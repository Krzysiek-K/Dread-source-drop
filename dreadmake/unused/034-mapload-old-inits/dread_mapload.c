
#include "demo.h"


extern word SOUND_NONE[];


#if AMIGA
	#include "data/frame.inc"
#endif
#if ATARI_ST
	#include "data/frame_st.inc"
#endif


word map_memory_buffer[
	0 +
		ENGINE_MAX_VERTEXES * sizeof(EngineVertex) / 2 +
		ENGINE_MAX_LINES * sizeof(EngineLine) / 2 +
		ENGINE_MAX_SUBSECTORS * sizeof(EngineSubSector) / 2
];
EngineVertex vtx_buffer[ENGINE_MAX_VERTEXES];
word *map_memory_buffer_alloc;
word *map_memory_buffer_end;

// TODO:
//	- ST chunky config
//
// Later:
//	- PALETTE_MAIN
//




//#define BLOCK_LOAD(name)		((void*)&data_block_ ## name)


extern word memfile_START[];
//extern word data_block_BSPNODES[];
//extern word data_block_VISLIST[];



extern DataActor Actor_Door;


DataHeader *e_map_header;
DataBSPNode *e_map_bspnodes;
word *e_map_vislist;
DataCondition *e_map_conditions;
byte *e_map_textures;




// ================================================================ Map block system - definitions 1 ================================================================


typedef struct _MapDataBlock MapDataBlock;

struct _MapDataBlock {

	// runtime data
	word		*data;
	word		*data_end;
	const word	*init_data;
	const word	*init_data_end;
	word		element_count;

	// static info
	const word	_element_size;
	const word	_init_element_size;
	int			(*_init_fn)(MapDataBlock*);

};



// ================================================================ Map block system - init functions ================================================================


int Map_InitBlock_Header(MapDataBlock *block)
{
	e_map_header = (DataHeader*)block->init_data;
	return 0;
}

int Map_InitBlock_Textures(MapDataBlock *block)
{
	e_map_textures = (byte*)block->data;
	return 0;
}

int Map_InitBlock_Vertexes(MapDataBlock *block)
{
	EngineVertex *data = (EngineVertex*)block->data;
	DataVertex *init = (DataVertex*)block->init_data;

	// Vertexes load:
	//	- copy position data as new structures
	//	- structure also contain <tr_x/y> fields for vertex transformation cache
	//

	e_vtx = data;
	e_n_vtx = block->element_count;

	while( (word*)data < block->data_end )
	{
		if( (word*)init < block->init_data_end )
		{
			// init loaded fields
			data->xp = init->xp;
			data->yp = init->yp;
			init++;
		}

		// reset dynamic fields
		data->tr_x = 0;
		data->tr_y = 0;
		data++;
	}

	return 0;
}

int Map_InitBlock_Lines(MapDataBlock *block)
{
	EngineLine *data = (EngineLine*)block->data;
	DataLine *init = (DataLine*)block->init_data;

	// Lines load:
	//	- convert references to pointers (vertexes, texture)
	//	- compute X1/X2 texcoords
	//	- copy remaining data (line modes, floor/ceil colors)
	//

	e_lines = data;
	e_n_lines = block->element_count;

	while( (word*)data < block->data_end )
	{
		if( (word*)init < block->init_data_end )
		{
			// init loaded fields
			int v1 = init->v1;
			int v2 = init->v2;
			int x = e_vtx[v2].xp - e_vtx[v1].xp;
			int y = e_vtx[v2].yp - e_vtx[v1].yp;
			int len = line_len(x, y);
			if( len<1 ) len = 1;

			data->v1 = e_vtx + v1;
			data->v2 = e_vtx + v2;
			data->tex_upper = e_map_textures + (int)init->tex_upper;
			data->xcoord1 = init->offs_x;
			data->xcoord2 = init->offs_x + (word)len;
			data->ceil_col = init->ceil;
			data->floor_col = init->floor;
			data->modes[0] = init->modes[0];
			data->modes[1] = init->modes[1];

			init++;
		}

		data++;
	}

	return 0;
}

int Map_InitBlock_Vis(MapDataBlock *block)
{
	e_map_vislist = block->data;
	return 0;
}

int Map_InitBlock_BSP(MapDataBlock *block)
{
	e_map_bspnodes = (DataBSPNode*)block->data;
	return 0;
}

int Map_InitBlock_SubSectors(MapDataBlock *block)
{
	EngineSubSector *data = (EngineSubSector*)block->data;
	DataSubSector *init = (DataSubSector*)block->init_data;

	// Subsectors load:
	//	- convert reference to pointer to VIS data
	//	- structure also contains <first_thing> and <visframe> fields for tracking things
	//

	dword secrets_mask = 0;

	e_subsectors = data;
	e_n_subsectors = block->element_count;

	while( (word*)data < block->data_end )
	{
		if( (word*)init < block->init_data_end )
		{
			// init loaded fields
			data->vis = e_map_vislist + (int)init->visoffset;
			data->type = init->type;
			init++;
		}

		// reset dynamic fields
		data->first_thing = NULL;
		data->visframe = (word)(view_frame_number-2);

		if( (word)(data->type - 21) < 9 )
			secrets_mask |= ((dword)1) << ((word)(data->type - 21));

		data++;
	}

	while( secrets_mask )
	{
		e_globals.levstat_secrets_max += secrets_mask & 1;
		secrets_mask >>= 1;
	}

	return 0;
}

int Map_InitBlock_Things(MapDataBlock *block)
{
	DataThing *init = (DataThing*)block->init_data;

	// Things load:
	//	- spawn all actors in specified places
	//

	// Don't reset things count - default objects already created

	word skillmask = 0x002;
	if( e_globals.skill <= 1 ) skillmask = 0x001;
	if( e_globals.skill >= 3 ) skillmask = 0x004;

	while( (word*)init < block->init_data_end )
	{
		if( init->type < TABLE_LENGTH(MAP_ACTOR_DEFS) )
			if( MAP_ACTOR_DEFS[init->type] )
				if( init->flags & skillmask )
					Engine_SpawnActor(MAP_ACTOR_DEFS[init->type], init->xp << ENGINE_SUBVERTEX_PRECISION, init->yp << ENGINE_SUBVERTEX_PRECISION);

		init++;
	}

	return 0;
}


int Map_InitBlock_Machines(MapDataBlock *block)
{
	DataMachine *init = (DataMachine*)block->init_data;

	// Machines load:
	//	- spawn all machines & their actors
	//

	e_n_machines = 0;

	while( (word*)init < block->init_data_end )
	{
		EngineMachine *m = e_machines + e_n_machines++;
		m->type = init->type;
		m->line_id = init->line_id;
		m->position = 0x4000;
		m->direction = 1;

		EngineThing *th = Engine_AllocThing();
		th->actor = (DataActor*)&Actor_Door;
		th->health = init->type;
		th->machine_line_id = init->line_id;
		th->machine_position = 0x4000;
		th->machine_speed = 0;

		m->blocker = th;

		Machine_Update(m);

		th->blocker_size = 1;
		th->xp = ((int)th->block_x1 + th->block_x2)>>1;
		th->yp = ((int)th->block_y1 + th->block_y2)>>1;

		Engine_ThingSetSubsector(th, Dread_FindSubSector(th->xp, th->yp));

		Script_OnCreate(th);

		init++;
	}

	return 0;
}


int Map_InitBlock_Conditions(MapDataBlock *block)
{
	e_map_conditions = (DataCondition*)block->data;
	return 0;
}



// ================================================================ Map block system - definitions 2 ================================================================




#define DEF_MAP_DATA_BLOCK(esize, isize, ifn)			{NULL, NULL, NULL, NULL, 0, esize, isize, ifn }


#define	MAPBLOCK_ID_TEXTURES			 0
#define	MAPBLOCK_ID_VERTEXES			 1
#define	MAPBLOCK_ID_LINES				 2
#define	MAPBLOCK_ID_VIS					 3
#define	MAPBLOCK_ID_BSP					 4
#define	MAPBLOCK_ID_SUBSECTORS			 5
#define	MAPBLOCK_ID_LOCATIONS			 6
#define	MAPBLOCK_ID_THINGS				 7
#define	MAPBLOCK_ID_MACHINES			 8
#define	MAPBLOCK_ID_CONDITIONS			 9
#define	MAPBLOCK_ID_HEADER				10
#define	NUM_MAPBLOCK_ID					11

MapDataBlock map_data_blockdefs[NUM_MAPBLOCK_ID] = {
	DEF_MAP_DATA_BLOCK( 0,							0,						Map_InitBlock_Textures		),		//	MAPBLOCK_ID_TEXTURES
	DEF_MAP_DATA_BLOCK( sizeof(EngineVertex),		sizeof(DataVertex),		Map_InitBlock_Vertexes		),		//	MAPBLOCK_ID_VERTEXES
	DEF_MAP_DATA_BLOCK( sizeof(EngineLine),			sizeof(DataLine),		Map_InitBlock_Lines			),		//	MAPBLOCK_ID_LINES	
	DEF_MAP_DATA_BLOCK( 0,							0,						Map_InitBlock_Vis			),		//	MAPBLOCK_ID_VIS		
	DEF_MAP_DATA_BLOCK( 0,							0,						Map_InitBlock_BSP			),		//	MAPBLOCK_ID_BSP		
	DEF_MAP_DATA_BLOCK( sizeof(EngineSubSector),	sizeof(DataSubSector),	Map_InitBlock_SubSectors	),		//	MAPBLOCK_ID_SUBSECTORS
	DEF_MAP_DATA_BLOCK( 0,							sizeof(DataLocation),	NULL						),		//	MAPBLOCK_ID_LOCATIONS
	DEF_MAP_DATA_BLOCK( 0,							sizeof(DataThing),		Map_InitBlock_Things		),		//	MAPBLOCK_ID_THINGS	
	DEF_MAP_DATA_BLOCK( 0,							sizeof(DataMachine),	Map_InitBlock_Machines		),		//	MAPBLOCK_ID_MACHINES
	DEF_MAP_DATA_BLOCK( 0,							sizeof(DataCondition),	Map_InitBlock_Conditions	),		//	MAPBLOCK_ID_CONDITIONS
	DEF_MAP_DATA_BLOCK( 0,							sizeof(DataHeader),		Map_InitBlock_Header		),		//	MAPBLOCK_ID_HEADER
};

#define MAPFILE_FN_END					0x0000	//			(all sizes are in word units)
#define MAPFILE_FN_BLOCK_MEM_RAW		0x0100	//	<block_id> <size><>							register following memory as raw block to use
#define MAPFILE_FN_BLOCK_MEM_INIT		0x0200	//	<block_id> <ielem_size> <count>				allocate <size> for block data and use <init_size> of the following memory as init data
#define MAPFILE_FN_BLOCK_MEM_SETUP		0x0300	//	<block_id> <init_size><>					use <init_size> of the following memory as init data





// ================================================================ Map block system - core ================================================================


#define _fetch_dword(ptr)			(*(*(const dword **)&ptr)++)


void Map_ResetAllBlocks(void)
{
	for( int i=0; i<NUM_MAPBLOCK_ID; i++ )
	{
		MapDataBlock *block = &map_data_blockdefs[i];
		block->data = NULL;
		block->data_end = NULL;
		block->init_data = NULL;
		block->init_data_end = NULL;
		block->element_count = 0;
	}

	map_memory_buffer_alloc = map_memory_buffer;
	map_memory_buffer_end = map_memory_buffer + sizeof(map_memory_buffer)/sizeof(map_memory_buffer[0]);
}

void Map_ParseMemoryData(const word *src)
{
	MapDataBlock *block;
	dword size;

	while( 1 )
	{
		word cmd = *src & 0xFF00;
		if( (*src&0xFF) >= NUM_MAPBLOCK_ID ) break;
		block = map_data_blockdefs + (*src&0xFF);
		src++;

		if( cmd == MAPFILE_FN_BLOCK_MEM_RAW )
		{
			size = _fetch_dword(src);
			block->data = (word*)src;
			src += size;
			block->data_end = (word*)src;
		}
		else if( cmd == MAPFILE_FN_BLOCK_MEM_INIT || cmd == MAPFILE_FN_BLOCK_MEM_SETUP )
		{
			if( *src++ != block->_init_element_size ) break;

			block->element_count = *src++;
			
			if( cmd == MAPFILE_FN_BLOCK_MEM_INIT )
			{
				// allocate data memory
				size = (block->element_count * block->_element_size) / sizeof(word);

				if( map_memory_buffer_alloc + size >= map_memory_buffer_end )
					break;

				block->data = map_memory_buffer_alloc;
				map_memory_buffer_alloc += size;
				block->data_end = map_memory_buffer_alloc;

				memset(block->data, 0, (byte*)block->data_end - (byte*)block->data);
			}

			// reference init region
			size = (block->element_count * block->_init_element_size) / sizeof(word);
			block->init_data = src;
			src += size;
			block->init_data_end = src;
		}
		else
			break;
	}
}

int Map_InitializeBlocks(void)
{
	for( int i=0; i<NUM_MAPBLOCK_ID; i++ )
	{
		MapDataBlock *block = &map_data_blockdefs[i];
		
		if( block->_init_fn )
			if( block->_init_fn(block) != 0 )
				return 0;
	}

	return 1;
}



// ================================================================ Old map loader ================================================================

//void Map_LoadVerts(const DataVertex *vsrc)
//{
//	// Vertexes load:
//	//	- copy position data as new structures
//	//	- structure also contain <tr_x/y> fields for vertex transformation cache
//	//
//	
//	//e_vtx = (EngineVertex*)map_memory_buffer;
//	e_vtx = vtx_buffer;
//	e_n_vtx = 0;
//
//	for( ; vsrc->xp!=0x7FFF; vsrc++ )
//	{
//		// Prepare EngineVertex structure
//		EngineVertex *v = e_vtx + e_n_vtx++;
//		v->xp = vsrc->xp;
//		v->yp = vsrc->yp;
//		v->tr_x = 0;
//		v->tr_y = 0;
//	}
//}
//
//void Map_LoadLines(const DataLine *lsrc)
//{
//	// Lines load:
//	//	- convert references to pointers (vertexes, texture)
//	//	- compute X1/X2 texcoords
//	//	- copy remaining data (line modes, floor/ceil colors)
//	//
//
//	e_lines = e_buff_lines;
//	e_n_lines = 0;
//
//	for( ; lsrc->v1!=0xFFFF; lsrc++ )
//	{
//		int v1 = lsrc->v1;
//		int v2 = lsrc->v2;
//		int x = e_vtx[v2].xp - e_vtx[v1].xp;
//		int y = e_vtx[v2].yp - e_vtx[v1].yp;
//		int len = line_len(x, y);
//		if( len<1 ) len = 1;
//
//		// Prepare EngineLine structure
//		EngineLine *line = e_lines + (int)e_n_lines++;
//		line->v1 = e_vtx + v1;
//		line->v2 = e_vtx + v2;
//		//line->next = NULL;
//		//line->other_sector = NULL;
//		line->tex_upper = e_map_textures + (int)lsrc->tex_upper;	// lrc_lines_tex[i];
//		//line->tex_lower = NULL;
//		line->xcoord1 = lsrc->offs_x;
//		line->xcoord2 = lsrc->offs_x + (word)len;
//		//line->y1 = lsrc->y1;
//		//line->y2 = lsrc->y2;
//		//line->y3 = 0;
//		line->ceil_col = lsrc->ceil;
//		line->floor_col = lsrc->floor;
//		line->modes[0] = lsrc->modes[0];
//		line->modes[1] = lsrc->modes[1];
//	}
//}

//void Map_LoadSubSectors(const DataSubSector *ssrc)
//{
//	// Subsectors load:
//	//	- convert reference to pointer to VIS data
//	//	- structure also contains <first_thing> and <visframe> fields for tracking things
//	//
//
//	dword secrets_mask = 0;
//
//	e_subsectors = e_buff_subsectors;
//	e_n_subsectors = 0;
//
//	for( ; ssrc->visoffset!=0xFFFF; ssrc++ )
//	{
//		EngineSubSector *sub = e_subsectors + e_n_subsectors++;
//		sub->vis = e_map_vislist + (int)ssrc->visoffset;
//		sub->first_thing = NULL;
//		sub->visframe = (word)(view_frame_number-2);
//		sub->type = ssrc->type;
//
//		if( (word)(sub->type - 21) < 9 )
//			secrets_mask |= ((dword)1)<< ((word)(sub->type - 21));
//	}
//
//	while( secrets_mask )
//	{
//		e_globals.levstat_secrets_max += secrets_mask & 1;
//		secrets_mask >>= 1;
//	}
//}

//void Map_LoadThings(const DataThing *tsrc)
//{
//	// Things load:
//	//	- spawn all actors in specified places
//	//
//
//	// Don't reset things count - default objects already created
//
//	word skillmask = 0x002;
//	if( e_globals.skill <= 1 ) skillmask = 0x001;
//	if( e_globals.skill >= 3 ) skillmask = 0x004;
//
//	for( ; tsrc->type; tsrc++ )
//		if( tsrc->type < TABLE_LENGTH(MAP_ACTOR_DEFS) )
//			if( MAP_ACTOR_DEFS[tsrc->type] )
//				if( tsrc->flags & skillmask )
//					Engine_SpawnActor(MAP_ACTOR_DEFS[tsrc->type], tsrc->xp << ENGINE_SUBVERTEX_PRECISION, tsrc->yp << ENGINE_SUBVERTEX_PRECISION);
//}

void Map_LoadMachines(const DataMachine *msrc)
{
	// Machines load:
	//	- spawn all machines & their actors
	//

	e_n_machines = 0;
	for( ; msrc->type!=0xFFFF; msrc++ )
	{
		EngineMachine *m = e_machines + e_n_machines++;
		m->type = msrc->type;
		m->line_id = msrc->line_id;
		m->position = 0x4000;
		m->direction = 1;

		//EngineThing *b = Engine_AllocThing();
		//m->blocker = b;
		//
		//Machine_Update(m);
		//
		//b->blocker_size = 1;
		//b->xp = ((int)b->block_x1 + b->block_x2)>>1;
		//b->yp = ((int)b->block_y1 + b->block_y2)>>1;
		//
		//EngineSubSector *ss = Dread_FindSubSector(b->xp, b->yp);
		//b->subsector = ss;
		//b->ss_next = ss->first_thing;
		//b->ss_prev = NULL;
		//if( ss->first_thing )
		//	ss->first_thing->ss_prev = b;
		//ss->first_thing = b;

		EngineThing *th = Engine_AllocThing();
		th->actor = (DataActor*)&Actor_Door;
		th->health = msrc->type;
		th->machine_line_id = msrc->line_id;
		th->machine_position = 0x4000;
		th->machine_speed = 0;
		
		m->blocker = th;
		
		Machine_Update(m);
		
		th->blocker_size = 1;
		th->xp = ((int)th->block_x1 + th->block_x2)>>1;
		th->yp = ((int)th->block_y1 + th->block_y2)>>1;
		
		Engine_ThingSetSubsector(th, Dread_FindSubSector(th->xp, th->yp));
		
		Script_OnCreate(th);
	}
}

const DataLocation *Map_GetLocation(int id)
{
	//return MAP_LOCATIONS + id;
	return ((DataLocation*)map_data_blockdefs[MAPBLOCK_ID_LOCATIONS].init_data) + id;
}


void Map_LoadAll(word skill)
{
//	e_map_bspnodes = (DataBSPNode*)BLOCK_LOAD(BSPNODES);
//	e_map_vislist = (word*)BLOCK_LOAD(VISLIST);

	Engine_Reset();
	e_globals.skill = skill;

	Map_ResetAllBlocks();
	Map_ParseMemoryData(memfile_START);

	Engine_InitDefaultObjects();
	Map_InitializeBlocks();
	


	//Map_LoadVerts(MAP_VTX);
	//Map_LoadLines(MAP_LINES);
	//Map_LoadSubSectors(MAP_SUBSECTORS);
	//Map_LoadThings(MAP_THINGS);
	//Map_LoadMachines(MAP_MACHINES);

	Machine_UpdateConditions();


	const DataLocation *loc = Map_GetLocation(0);
	view_fine_pos_x = ((int)loc->xp) << 16;
	view_fine_pos_y = ((int)loc->yp) << 16;
	view_angle = loc->angle & 0xFF;
	view_angle_fine = (view_angle<<8) + 128;
	view_subsector = NULL;
}
