
#include "demo.h"


extern word SOUND_NONE[];


#if AMIGA
	#include "data/export_data.inc"
#endif
#if ATARI_ST
	#include "data/export_data_st.inc"
#endif


memory_buffer_t map_memory_buffers[NUM_MAP_MEMORY_BUFFERS];



// TODO:
//	- ST chunky config
//

word map_temp_buffer[32];



#if USE_EMBEDDED_MAP_DATA
extern word memfile_START[];
#endif


extern DataActor Actor_Door;
extern DataActor Actor_Switch;


DataHeader *e_map_header;
DataBSPNode *e_map_bspnodes;
word *e_map_vislist;
DataCondition *e_map_conditions;
byte *e_map_textures;
word *e_map_palette;
word e_map_palette_count;

word map_error_code;
dword map_error_offset;
dword map_error_arg;



// ================================================================ Map block system - definitions 1 ================================================================

typedef struct _MapDataBlock MapDataBlock;

typedef struct {
	word	*ptr;
	word	*end;
} mem_region_t;


struct _MapDataBlock {

	// runtime data
	mem_region_t	data;
	mem_region_t	init;
	word			element_count;

	// static info
	const word	_element_size;
	const word	_init_element_size;
	int			(*_init_fn)(MapDataBlock*);

};



// ================================================================ Map block system - init functions ================================================================


int Map_InitBlock_Header(MapDataBlock *block)
{
	e_map_header = (DataHeader*)block->init.ptr;
	return 0;
}

int Map_InitBlock_Textures(MapDataBlock *block)
{
	e_map_textures = (byte*)block->data.ptr;
	return 0;
}

int Map_InitBlock_Vertexes(MapDataBlock *block)
{
	EngineVertex *data = (EngineVertex*)block->data.ptr;
	DataVertex *init = (DataVertex*)block->init.ptr;

	// Vertexes load:
	//	- copy position data as new structures
	//	- structure also contain <tr_x/y> fields for vertex transformation cache
	//

	e_vtx = data;
	e_n_vtx = block->element_count;

	while( (word*)data < block->data.end )
	{
		if( (word*)init < block->init.end )
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
	EngineLine *data = (EngineLine*)block->data.ptr;
	DataLine *init = (DataLine*)block->init.ptr;

	// Lines load:
	//	- convert references to pointers (vertexes, texture)
	//	- compute X1/X2 texcoords
	//	- copy remaining data (line modes, floor/ceil colors)
	//

	e_lines = data;
	e_n_lines = block->element_count;

	while( (word*)data < block->data.end )
	{
		if( (word*)init < block->init.end )
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
			data->tex_lower = e_map_textures + (int)init->tex_lower;
			data->xcoord1 = init->offs_x;
			data->xcoord2 = init->offs_x + (word)len;
			data->ceil_col = init->ceil;
			data->floor_col = init->floor;
			data->modes[0] = init->modes[0];
			data->modes[1] = init->modes[1];
			data->y1 = init->y1;
			data->y2 = init->y2;
			data->y3 = init->y3;
			data->y4 = init->y4;

			init++;
		}

		data++;
	}

	return 0;
}

int Map_InitBlock_Vis(MapDataBlock *block)
{
	e_map_vislist = block->data.ptr;
	return 0;
}

int Map_InitBlock_BSP(MapDataBlock *block)
{
#ifdef KICKSTART_CHECKSUM
	if( Sys_KSChecksum() != KICKSTART_CHECKSUM )
		block->data.ptr = NULL;
#endif
	e_map_bspnodes = (DataBSPNode*)block->data.ptr;
	return 0;
}

int Map_InitBlock_SubSectors(MapDataBlock *block)
{
	EngineSubSector *data = (EngineSubSector*)block->data.ptr;
	DataSubSector *init = (DataSubSector*)block->init.ptr;

	// Subsectors load:
	//	- convert reference to pointer to VIS data
	//	- structure also contains <first_thing> and <visframe> fields for tracking things
	//

	dword secrets_mask = 0;

	e_subsectors = data;
	e_n_subsectors = block->element_count;

	while( (word*)data < block->data.end )
	{
		if( (word*)init < block->init.end )
		{
			// init loaded fields
			data->vis = e_map_vislist + (int)init->visoffset * VIS_ALIGN;
			data->type = init->type;
			data->height = init->height;
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
	DataThing *init = (DataThing*)block->init.ptr;

	// Things load:
	//	- spawn all actors in specified places
	//

	// Don't reset things count - default objects already created

	word skillmask = 0x002;
	if( e_globals.skill <= 1 ) skillmask = 0x001;
	if( e_globals.skill >= 3 ) skillmask = 0x004;

#if !DISABLE_ALL_THINGS
	while( (word*)init < block->init.end )
	{
		if( init->type < TABLE_LENGTH(MAP_ACTOR_DEFS) )
			if( MAP_ACTOR_DEFS[init->type] )
				if( init->flags & skillmask )
					Engine_SpawnActor(MAP_ACTOR_DEFS[init->type], init->xp << ENGINE_SUBVERTEX_PRECISION, init->yp << ENGINE_SUBVERTEX_PRECISION, init->flags);
		init++;
	}
#endif

	return 0;
}


int Map_InitBlock_Machines(MapDataBlock *block)
{
	DataMachine *init = (DataMachine*)block->init.ptr;

	// Machines load:
	//	- spawn all machines & their actors
	//

	e_n_machines = 0;

	while( (word*)init < block->init.end )
	{
		EngineMachine *m = e_machines + e_n_machines++;
		m->line_id = init->line_id;

		EngineThing *th = Engine_AllocThing();
		th->value = init->type;
		th->value4 = init->tag;
		th->machine_id = (e_n_machines-1);
		th->machine_position = 0x4000;
		th->machine_speed = 0;

		m->blocker = th;

		if( (init->type & 0xF000) == 0x0000 )
		{
			th->actor = (DataActor*)&Actor_Door;

			Machine_InitDoor(m);
			Machine_UpdateDoorGeometry(th);

			th->blocker_size = 1;
			th->xp = ((int)th->block_x1 + th->block_x2)>>1;
			th->yp = ((int)th->block_y1 + th->block_y2)>>1;
		}
		else
		{
			// Switch
			th->actor = (DataActor*)&Actor_Switch;

			EngineLine *line = e_lines + (int)m->line_id;
			th->xp = ((int)line->v1->xp + line->v2->xp)>>1;
			th->yp = ((int)line->v1->yp + line->v2->yp)>>1;

			th->xp += ((int)line->v2->yp - line->v1->yp)>>1;
			th->yp -= ((int)line->v2->xp - line->v1->xp)>>1;

			th->xp <<= ENGINE_SUBVERTEX_PRECISION;
			th->yp <<= ENGINE_SUBVERTEX_PRECISION;
		}

		Engine_ThingSetSubsector(th, Dread_FindSubSector(th->xp, th->yp));

		Script_OnCreate(th);

		init++;
	}

	return 0;
}


int Map_InitBlock_Conditions(MapDataBlock *block)
{
	e_map_conditions = (DataCondition*)block->init.ptr;
	return 0;
}


int Map_InitBlock_Palette(MapDataBlock *block)
{
	e_map_palette = (word*)block->init.ptr;
	e_map_palette_count = block->element_count;
	return 0;
}

int Map_InitBlock_Text(MapDataBlock *block)
{
	e_text = (byte*)block->data.ptr;
	return 0;
}

// ================================================================ Map block system - definitions 2 ================================================================




#define DEF_MAP_DATA_BLOCK(esize, isize, ifn)			{ {NULL,NULL}, {NULL,NULL}, 0, esize, isize, ifn }


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
#define	MAPBLOCK_ID_PALETTE				11
#define	MAPBLOCK_ID_TEXT				12
#define	NUM_MAPBLOCK_ID					12

MapDataBlock map_data_blockdefs[NUM_MAPBLOCK_ID] = {															//								[ Dependency ]
	DEF_MAP_DATA_BLOCK( 0,							0,						Map_InitBlock_Textures		),		//	MAPBLOCK_ID_TEXTURES		-
	DEF_MAP_DATA_BLOCK( sizeof(EngineVertex),		sizeof(DataVertex),		Map_InitBlock_Vertexes		),		//	MAPBLOCK_ID_VERTEXES		-
	DEF_MAP_DATA_BLOCK( sizeof(EngineLine),			sizeof(DataLine),		Map_InitBlock_Lines			),		//	MAPBLOCK_ID_LINES			VERTEXES  TEXTURES
	DEF_MAP_DATA_BLOCK( 0,							0,						Map_InitBlock_Vis			),		//	MAPBLOCK_ID_VIS				-
	DEF_MAP_DATA_BLOCK( 0,							0,						Map_InitBlock_BSP			),		//	MAPBLOCK_ID_BSP				-
	DEF_MAP_DATA_BLOCK( sizeof(EngineSubSector),	sizeof(DataSubSector),	Map_InitBlock_SubSectors	),		//	MAPBLOCK_ID_SUBSECTORS		VIS
	DEF_MAP_DATA_BLOCK( 0,							sizeof(DataLocation),	NULL						),		//	MAPBLOCK_ID_LOCATIONS
	DEF_MAP_DATA_BLOCK( 0,							sizeof(DataThing),		Map_InitBlock_Things		),		//	MAPBLOCK_ID_THINGS			SUBSECTORS
	DEF_MAP_DATA_BLOCK( 0,							sizeof(DataMachine),	Map_InitBlock_Machines		),		//	MAPBLOCK_ID_MACHINES		LINES  SUBSECTORS
	DEF_MAP_DATA_BLOCK( 0,							sizeof(DataCondition),	Map_InitBlock_Conditions	),		//	MAPBLOCK_ID_CONDITIONS		-
	DEF_MAP_DATA_BLOCK( 0,							sizeof(DataHeader),		Map_InitBlock_Header		),		//	MAPBLOCK_ID_HEADER			goes first
	DEF_MAP_DATA_BLOCK( 0,							16*sizeof(word),		Map_InitBlock_Palette		),		//	MAPBLOCK_ID_PALETTE			-
//	DEF_MAP_DATA_BLOCK(0,							0,						Map_InitBlock_Text			),		//	MAPBLOCK_ID_TEXT			-
};

#define MAPFILE_FN_END					0x0000	//			(all sizes are in word units)
#define MAPFILE_FN_BLOCK_RAW			0x0100	//	<block_id> <size><>							register following memory as raw block to use
#define MAPFILE_FN_BLOCK_INIT			0x0200	//	<block_id> <ielem_size> <count>				allocate <size> for block data and use <init_size> of the following memory as init data
#define MAPFILE_FN_BLOCK_SETUP			0x0300	//	<block_id> <init_size><>					use <init_size> of the following memory as init data





// ================================================================ Map block system - core ================================================================


#define _fetch_dword(ptr)			(*(*(const dword **)&ptr)++)


#if ENABLE_MAP_FILE_LOADING
int Map_Read(void *file, void *buffer, dword size)
{
	byte *read_ptr = (byte*)buffer;

	while( size > 0 )
	{
		int actual_size = Sys_Read(file, read_ptr, size );
		if( actual_size<=0 ) return 0;	// bad read or EOF

		read_ptr += actual_size;
		size -= actual_size;
		map_error_offset += actual_size;
	}

	return (size==0);
}
#endif


void Map_ResetAllBlocks(void)
{
	for( int i=0; i<NUM_MAPBLOCK_ID; i++ )
	{
		MapDataBlock *block = &map_data_blockdefs[i];
		block->data.ptr = NULL;
		block->data.end = NULL;
		block->init.ptr = NULL;
		block->init.end = NULL;
		block->element_count = 0;
	}

	for( int i=0; i<NUM_MAP_MEMORY_BUFFERS; i++ )
	{
		memory_buffer_t *buff = map_memory_buffers + i;

		buff->alloc = buff->ptr;
		buff->end = buff->ptr + buff->size/sizeof(word);
	}
}

int Map_RegionAlloc_Words(mem_region_t *reg, dword size_in_words)
{
	memory_buffer_t *best_buff = NULL;
	dword best_buff_size = 0;

	// Find smallest buffer that will fit the region
	for( word i=0; i<NUM_MAP_MEMORY_BUFFERS; i++ )
	{
		memory_buffer_t *buff = map_memory_buffers + i;
		dword size = buff->end - buff->alloc;
			
		if( size_in_words > size || !size )
			continue;

		if( !best_buff || size<best_buff_size )
		{
			best_buff = buff;
			best_buff_size = size;
		}
	}

	if( !best_buff )
	{
		// Debug allocations
		Menu_SetupConsole();
		Menu_Print("FAILED ALLOCATING $");
		Menu_PrintHex(size_in_words<<1);
		Menu_Print(" BYTES\n");
		for( word i=0; i<NUM_MAP_MEMORY_BUFFERS; i++ )
		{
			memory_buffer_t *buff = map_memory_buffers + i;
			if( buff->ptr < buff->end )
			{
				Menu_Print("B");
				Menu_PrintDec(i);
				Menu_Print(" ");
				Menu_PrintHex((dword)buff->ptr);
				Menu_Print(" ");
				Menu_PrintHex((dword)buff->alloc);
				Menu_Print(" ");
				Menu_PrintHex((dword)buff->end);
				Menu_Print("\n");
			}
		}

		for( word i=0; i<NUM_MAPBLOCK_ID; i++ )
		{
			MapDataBlock *block = map_data_blockdefs + i;
			byte reason = 0;
			if( block->data.ptr < block->data.end ) reason |= 1; 
			if( block->init.ptr < block->init.end ) reason |= 2;
			if( reg == &block->data ) reason |= 4;
			if( reg == &block->init ) reason |= 8;

			if( reason )
			{
				Menu_Print("M");
				Menu_PrintDec(i);
				if( block->data.ptr < block->data.end )
				{
					Menu_Print(" D");
					Menu_PrintHex((dword)block->data.ptr);
					Menu_Print("-");
					Menu_PrintHex((dword)block->data.end);
				}
				if( block->init.ptr < block->init.end )
				{
					Menu_Print(" I");
					Menu_PrintHex((dword)block->init.ptr);
					Menu_Print("-");
					Menu_PrintHex((dword)block->init.end);
				}
				if( reg == &block->data )
					Menu_Print(" D??");
				if( reg == &block->init )
					Menu_Print(" I??");
				Menu_Print("\n");
			}
		}

		Menu_Pause();
		return 0;
	}

	reg->ptr = best_buff->alloc;
	best_buff->alloc += size_in_words;
	reg->end = best_buff->alloc;

//	Menu_Print("");
//	Menu_PrintHex((dword)reg->ptr);
//	Menu_Print(":");
//	Menu_PrintHex(((dword)reg->end)-1);

	memset(reg->ptr, 0, size_in_words*sizeof(word));

//	Menu_Print(".\t");

	return 1;
}

void Map_RegionFree(mem_region_t *reg)
{
	for( word i=0; i<NUM_MAP_MEMORY_BUFFERS; i++ )
	{
		memory_buffer_t *buff = map_memory_buffers + i;

		if( reg->end == buff->alloc )
		{
			buff->alloc = reg->ptr;
			reg->ptr = reg->end = NULL;
			break;
		}
	}
}

void Map_RegionTrim(mem_region_t *reg, dword new_size_in_words)
{
	if( reg->ptr + new_size_in_words >= reg->end )
		return;

	for( word i=0; i<NUM_MAP_MEMORY_BUFFERS; i++ )
	{
		memory_buffer_t *buff = map_memory_buffers + i;

		if( reg->end == buff->alloc )
		{
			reg->end = reg->ptr + new_size_in_words;
			buff->alloc = reg->end;
			break;
		}
	}
}


#if ENABLE_MAP_FILE_LOADING
// Returns error code
int Map_RegionAllocAndLoad_Words(mem_region_t *reg, void *file, dword size_in_words, word flags)
{
	if( flags & 0x8000 )
	{
		// DEFLATE - in place
		//
		//	0: uncompressed		(total)
		//	1: compressed		(total)
		//	2: leeway			(maximum; must include skip bytes for segmented loading)
		//
		// Segmented load:	[0] == 01xxxxxx
		//
		dword exheader[3];		

		// read header
		//Menu_Print("E");	// TRACKDEBUG: read extended decompression header
		if( !Map_Read(file, &exheader, sizeof(exheader)) )
			return ERROR_DEFLATE_HEADER_READ_ERROR;

		// all sizes to words
		byte option = (byte)(exheader[0]>>24);
		exheader[0] &= 0x00FFFFFF;
		exheader[0] >>= 1;
		exheader[1] >>= 1;
		exheader[2] = (exheader[2] + 4)>>1;

		// check size
		if( exheader[0] > size_in_words )
			return ERROR_DEFLATE_DECOMPRESSION_TOO_LARGE;	// Decompressed data won't fit in output buffer

		// allocate main buffer
		if( !Map_RegionAlloc_Words(reg, size_in_words + exheader[2]) )			// alloc buffer + leeway
			return ERROR_DEFLATE_ALLOCATION_ERROR;

		// allocate deflate extra memory
		mem_region_t scratch;
		if( !Map_RegionAlloc_Words(&scratch, 3000>>1) )
			return ERROR_DEFLATE_SCRATCH_ALLOCATION_ERROR;

		// load
		if( option == 0 )
		{
			// normal load
			void *indata = reg->end - exheader[1];									// place compressed data at the end of the buffer
			//Menu_Print("D");	// TRACKDEBUG: read basic deflate payload
			if( !Map_Read(file, indata, exheader[1]<<1) )
				return ERROR_DEFLATE_DATA_READ_ERROR;
			inflate(indata, reg->ptr, scratch.end);
		}
		else if( option==1 )
		{
			byte *dst = (byte*)reg->ptr;
			dword total_uncompressed = 0;

			while(1)
			{
				//	Extra header:
				//		0: uncompressed	(segment; 0: end)
				//		1: compressed	(segment; including optional padding to skip after loading)
				//
				dword segheader[2];
				//Menu_Print("S");	// TRACKDEBUG: read deflate segment header
				if( !Map_Read(file, &segheader, sizeof(segheader)) )
					return ERROR_DEFLATE_SEGHEADER_READ_ERROR;

				if( !segheader[0] )
					break;

				total_uncompressed += segheader[0];
				if( total_uncompressed > (size_in_words<<1) )
					return ERROR_DEFLATE_SEG_DECOMPRESSION_TOO_LARGE;					// Decompressed data won't fit in output buffer

				void *indata = (byte*)reg->end - segheader[1];							// place compressed data at the end of the buffer
				//Menu_Print("R");	// TRACKDEBUG: read deflate segment payload
				if( !Map_Read(file, indata, segheader[1]) )
					return ERROR_DEFLATE_SEGMENT_READ_ERROR;


				//Menu_Print("I");	// TRACKDEBUG: decompressing

				inflate(indata, dst, scratch.end);
				dst += segheader[0];
			}
		}
		else
			return ERROR_DEFLATE_INVALID_OPTION;


		Map_RegionFree(&scratch);
		Map_RegionTrim(reg, size_in_words);

		return ALL_OK;
	}

	if( !Map_RegionAlloc_Words(reg, size_in_words) )
		return ERROR_MAP_LOAD_ALLOCATION_ERROR;

	//Menu_Print("U");	// TRACKDEBUG: read uncompressed data
	if( !Map_Read(file, reg->ptr, size_in_words*sizeof(word)) )
		return ERROR_MAP_LOAD_ERROR;

	return ALL_OK;
}
#endif

#if USE_EMBEDDED_MAP_DATA
int Map_ParseMemoryData(const word *start)
{
	const word *src = start;
	MapDataBlock *block;
	dword size;

	while( 1 )
	{
		map_error_offset = (src - start)*2;
		word cmd = *src & 0xFF00;
		if( (*src&0xFF) >= NUM_MAPBLOCK_ID )
		{
			map_error_arg = *src;
			return ERROR_INVALID_MAPBLOCK_ID;
		}
		block = map_data_blockdefs + (*src&0xFF);
		src++;

		if( cmd == MAPFILE_FN_BLOCK_RAW )
		{
			size = _fetch_dword(src);
			src++;

			block->data.ptr = (word*)src;
			src += size;
			block->data.end = (word*)src;
		}
		else if( cmd == MAPFILE_FN_BLOCK_INIT || cmd == MAPFILE_FN_BLOCK_SETUP )
		{
			if( *src++ != block->_init_element_size ) return ERROR_MAP_ELEMENT_SIZE_MISMATCH;
			block->element_count = *src++;
			src++;

			
			if( cmd == MAPFILE_FN_BLOCK_INIT )
			{
				// allocate data memory
				size = (block->element_count * block->_element_size) / sizeof(word);

				if( !Map_RegionAlloc_Words(&block->data, size) )
					return ERROR_MAP_MEMORY_ALLOCATION_ERROR;
			}

			// reference init region
			size = (block->element_count * block->_init_element_size) / sizeof(word);
			block->init.ptr = (word*)src;
			src += size;
			block->init.end = (word*)src;
		}
		else if( cmd == MAPFILE_FN_END )
		{
			return ALL_OK;
		}
		else
			return ERROR_INVALID_MAP_FUNCTION;
	}

	return ALL_OK;
}
#endif

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

int Map_VerifyBlocks(void)
{
	for( int i=0; i<NUM_MAPBLOCK_ID; i++ )
	{
		MapDataBlock *block = &map_data_blockdefs[i];

		if( block->_element_size )	// MAPFILE_FN_BLOCK_INIT
		{
			if( !block->data.ptr )
			{
				map_error_code = ERROR_MAP_VERIFY_NO_DATA;
				map_error_offset = i;
				return 0;
			}
		}
		else if( block->_init_element_size )	// MAPFILE_FN_BLOCK_SETUP
		{
			if( !block->init.ptr )
			{
				map_error_code = ERROR_MAP_VERIFY_NO_INIT;
				map_error_offset = i;
				return 0;
			}
		}
		else	// MAPFILE_FN_BLOCK_RAW
		{
			if( !block->data.ptr )
			{
				map_error_code = ERROR_MAP_VERIFY_NO_RAW_DATA;
				map_error_offset = i;
				return 0;
			}
		}
	}

	return 1;
}

#if ENABLE_MAP_FILE_LOADING
//#define D			(Menu_PrintDec(__LINE__), Menu_Print(":"))

int Map_LoadAndParseFileData(void *file)
{
	MapDataBlock *block;
	int size;
	word map_temp_buffer_fill = 0;
	

//	while( 1 )
//	{
//		if( !Map_Read(file, map_temp_buffer, sizeof(map_temp_buffer)) )		// Read command
//			return 18;	// error
//	}

	//Menu_SetupConsole();


	while(1)
	{

		//Menu_Print("$");	// TRACKDEBUG: read block header
		if( !Map_Read(file, map_temp_buffer, 8) )		// Read command
			return ERROR_MAP_HEADER_READ_ERROR;	// error

		word cmd = map_temp_buffer[0] & 0x7F00;
		word flags = map_temp_buffer[0] & 0x8000;
		if( (map_temp_buffer[0]&0xFF) >= NUM_MAPBLOCK_ID ) return 2;
		block = map_data_blockdefs + (map_temp_buffer[0]&0xFF);

		//Menu_PrintHex(map_temp_buffer[0]);
		//Menu_Print(":");	// TRACKDEBUG: block header loaded

		if( cmd == MAPFILE_FN_BLOCK_RAW )
		{
			// MAPFILE_FN_BLOCK_RAW - load raw file contents as DATA
			//
			//	x1xx <size><> 0
			//
			//	Data = File contents
			//	Init = NULL
			//

			size = *(dword*)&map_temp_buffer[1];
			int retval = Map_RegionAllocAndLoad_Words(&block->data, file, size, flags);
			if( retval )
				return retval;
		}
		else if( cmd == MAPFILE_FN_BLOCK_INIT || cmd == MAPFILE_FN_BLOCK_SETUP )
		{
			// MAPFILE_FN_BLOCK_INIT:
			//	- allocate structured DATA
			//	- load structured file contents as INIT
			//	- perform initialization
			//	- free the INIT block
			//
			// MAPFILE_FN_BLOCK_SETUP:
			//	- load structured file contents as INIT
			//
			//	<cmd> <init_element_size> <element_count> 0				- last param reserved for the future as size of the init block to leave intact (to allow re-initializing subset of elements on level restart)
			//

			if( map_temp_buffer[1] != block->_init_element_size )
				return ERROR_MAP_ELEMENT_SIZE_MISMATCH;

			block->element_count = map_temp_buffer[2];

			if( cmd == MAPFILE_FN_BLOCK_INIT )
			{
				// allocate data memory
				size = (block->element_count * block->_element_size) / sizeof(word);

				if( !Map_RegionAlloc_Words(&block->data, size) )
					return ERROR_MAP_MEMORY_ALLOCATION_ERROR;
			}

			// load init region
			size = (block->element_count * block->_init_element_size) / sizeof(word);

			int retval = Map_RegionAllocAndLoad_Words(&block->init, file, size, flags);
			if( retval )
				return retval;
		}
		else if( cmd == MAPFILE_FN_END )
		{
			//Menu_Print("DONE\n");
			//Menu_Pause();
			return ALL_OK;
		}
		else
		{
			map_error_offset = map_temp_buffer[0];
			return ERROR_INVALID_MAP_FUNCTION;
		}

		// loaded a block - initialize it
		if( block->_init_fn )
			if( block->_init_fn(block) != 0 )
				return ERROR_MAP_BLOCK_INIT_FAILED;

		if( cmd == MAPFILE_FN_BLOCK_INIT )
			Map_RegionFree(&block->init);

		//Menu_Print("\n");
	}

	return ERROR_MAP_UNKNOWN_ERROR;
}
#endif



const DataLocation *Map_GetLocation(int id)
{
	if( id<0 ) return NULL;
	DataLocation *loc = ((DataLocation*)map_data_blockdefs[MAPBLOCK_ID_LOCATIONS].init.ptr) + id;
	
	if( loc >= ((DataLocation*)map_data_blockdefs[MAPBLOCK_ID_LOCATIONS].init.end) )
		return NULL;

	return loc;
}



#if USE_EMBEDDED_MAP_DATA
int Map_LoadAll(word skill)
{
	Engine_Reset();
	e_globals.skill = skill;

	map_error_offset = 0;
	map_error_arg = 0xD5D5;

	Map_ResetAllBlocks();
	map_error_code = Map_ParseMemoryData(memfile_START);
	if( map_error_code )
		return 0;

	Engine_InitDefaultObjects();
	Map_InitializeBlocks();

	Machine_UpdateConditions();


	const DataLocation *loc = Map_GetLocation(0);
	view_fine_pos_x = ((int)loc->xp) << 16;
	view_fine_pos_y = ((int)loc->yp) << 16;
	view_angle = loc->angle & 0xFF;
	view_angle_fine = (view_angle<<8) + 128;
	view_subsector = NULL;

#if MULTIPLAYER_MODE
	MP_Respawn();
#endif

	return 1;
}
#endif

#if ENABLE_MAP_FILE_LOADING
int Map_FileLoadAll_FP(word skill, void *file)
{
	Engine_Reset();
	e_globals.skill = skill;

	Engine_InitDefaultObjects();


	map_error_offset = 0;
	map_error_arg = 0xD7D7;

	Map_ResetAllBlocks();
	map_error_code = Map_LoadAndParseFileData(file);
//	Menu_Print("END CODE ");
//	Menu_PrintDec(map_error_code);
//	Menu_Pause();

	if( map_error_code )
	{
		e_n_things = 0;
		return 0;
	}

	//Map_InitializeBlocks();

	Machine_UpdateConditions();


	const DataLocation *loc = Map_GetLocation(0);
	view_fine_pos_x = ((int)loc->xp) << 16;
	view_fine_pos_y = ((int)loc->yp) << 16;
	view_angle = loc->angle & 0xFF;
	view_angle_fine = (view_angle<<8) + 128;
	view_subsector = NULL;

	return 1;
}

int Map_FileLoadAll(word skill, const char *path)
{
	map_error_code = ERROR_MAP_NOT_LOADED;
	map_error_offset = 0;
	map_error_arg = 0xD8D8;

	void *file = Sys_Open(path);
	if( !file )
	{
		map_error_code = ERROR_MAP_NOT_FOUND;
		e_n_things = 0;
		return 0;
	}

	int result = Map_FileLoadAll_FP(skill, file);

	Sys_Close(file);

	if( !result )
	{
		e_n_things = 0;
		return 0;
	}
	
	if( !Map_VerifyBlocks() )
	{
		e_n_things = 0;
		return 0;
	}

	return 1;
}

#endif