

#include "demo.h"


extern DataActor Actor_Pistol;		// TBD: generate header with all that
extern DataActor Actor_Player;
extern DataActor Actor_SmokePuff;
extern DataActor Actor_BloodSplat;



// Vertexes
EngineVertex	*e_vtx;
word			e_n_vtx = 0;

// Lines
EngineLine		*e_lines;
word			e_n_lines = 0;

// Subsectors
EngineSubSector	*e_subsectors;
word			e_n_subsectors = 0;

// Things
__chip EngineThing		e_things_buffer[ENGINE_MAX_THINGS];			// TBD: __chip here is just a hack to load TsakBigMap
EngineThing		*e_things = e_things_buffer;
word			e_n_things = 0;
word			e_max_things = ENGINE_MAX_THINGS;
EngineThing		*e_delthings[ENGINE_MAX_THINGS];
EngineThing		**e_delthings_end;

// Machines
EngineMachine	e_machines[ENGINE_MAX_MACHINES];
word			e_n_machines = 0;

// Conditions
byte			e_condition_states[ENGINE_MAX_CONDITIONS];

// Vislines
EngineVisLine	e_vislines[ENGINE_MAX_VISLINES+1];

// VisThings
EngineThing		*e_visthings[ENGINE_MAX_VISTHINGS];
EngineThing		**e_visthings_end;
EngineThing		**e_visthings_max;

// DmgThings
EngineThing		*e_dmgthings[ENGINE_MAX_DMGTHINGS];
EngineThing		**e_dmgthings_end;
EngineThing		**e_dmgthings_max;

// Globals
EngineGlobals	e_globals;

// Render data
RenderColumnsInfo render_columns;

// Text
const byte		*e_text;


// View variables (not in struct for easier access from asm)
int				view_fine_pos_x;	// Logic
int				view_fine_pos_y;
int				view_fine_pos_z;
int				view_delta_x;
int				view_delta_y;
int				view_delta_z;
word			view_weapon_swing;
short			view_weapon_drop;
word			view_movement;
EngineThing		*view_weapon;
EngineThing		*view_player;
EngineThing		*mp_avatar;

short			view_pos_x;			// Rendering
short			view_pos_y;
short			view_pos_z;
word			view_angle;
word			view_angle_fine;
short			view_rotate_dx;
short			view_rotate_dy;
EngineSubSector	*view_subsector;
word			view_frame_number;
short			view_prev_pos_x;
short			view_prev_pos_y;
short			view_prev_pos_z;
short			view_prev_rotate_dx;
short			view_prev_rotate_dy;





void Engine_Reset(void)
{
	// Globals
	memset(&e_globals, 0, sizeof(e_globals));
	e_globals.cheat_code_pos = 10;
	e_globals.mp_mode = MULTIPLAYER_MODE;


	// Map elements
	e_n_vtx			= 0;
	e_n_lines		= 0;
	e_n_subsectors	= 0;
	e_n_things		= 0;
	e_n_machines	= 0;

	e_delthings_end = e_delthings;


	// Conditions
	memset(e_condition_states, 0, sizeof(e_condition_states));
	e_condition_states[0] = 1;

	// Vis tables
	e_visthings_end = e_visthings;
	e_visthings_max = e_visthings + ENGINE_MAX_VISTHINGS;

	// Dmg tables
	e_dmgthings_end = e_dmgthings;
	e_dmgthings_max = e_dmgthings + ENGINE_MAX_DMGTHINGS;

	// View
	view_fine_pos_x		= 0;			// Logic
	view_fine_pos_y		= 0;
	view_fine_pos_z		= -40<<16;
	view_delta_x		= 0;
	view_delta_y		= 0;
	view_delta_z		= 0;
	view_weapon_swing	= 0;
	view_weapon_drop	= 0;
	//view_weapon			= NULL;

	view_pos_x			= 0;			// Rendering
	view_pos_y			= 0;
	view_pos_z			= -40;		// TBD: make fixed.6
	view_angle			= 0;
	view_angle_fine		= 0;
	view_rotate_dx		= 0;
	view_rotate_dy		= 0;
	view_subsector		= NULL;
	view_frame_number	= 0;

	view_prev_pos_x			= view_pos_x;
	view_prev_pos_y			= view_pos_y;
	view_prev_pos_z			= view_pos_z;
	view_prev_rotate_dx		= view_rotate_dx;
	view_prev_rotate_dy		= view_rotate_dy;

	mp_avatar			= NULL;
}

EngineThing *Engine_AllocThing(void)
{
	EngineThing *th = NULL;

	// Try reusing old thing
	EngineThing **scan = e_delthings_end;
	while( scan > e_delthings )
	{
		th = *--scan;
		if( !th->visible )
		{
			// Found a thing to reuse
			*scan = *--e_delthings_end;			// remove from deleted things list

			// initialize
			word index = th->index;
			Engine_ThingSetSubsector(th, NULL);
			memset(th, 0, sizeof(EngineThing));
			th->index = index;

			return th;
		}
	}

	if( e_n_things >= e_max_things )
		return NULL;		// sorry, no more things available
	
	th = e_things + e_n_things;

	memset(th, 0, sizeof(EngineThing));
	th->index = e_n_things++;

	return th;
}

EngineThing *Engine_SpawnActor(const DataActor *actor, short xp, short yp, word flags)
{
	EngineThing *th = Engine_AllocThing();
	if( !th )
		return NULL;

	th->actor = (DataActor*)actor;
	th->value = flags;

	Engine_SetThingPosition(th, xp, yp);

	if( th->subsector )
		th->value2 = th->subsector->type;

	Script_OnCreate(th);

	return th;
}

EngineThing *Engine_SpawnActor_NoPosition(const DataActor *actor)
{
	EngineThing *th = Engine_AllocThing();

	th->actor = (DataActor*)actor;

	Script_OnCreate(th);

	return th;
}

void Engine_SetThingPosition(EngineThing *th, short xp, short yp)
{
	th->unstuck_x = th->xp = xp;
	th->unstuck_y = th->yp = yp;

	EngineSubSector *ss = Dread_FindSubSector(th->xp, th->yp);
	Engine_ThingSetSubsector(th, ss);
}


void Engine_SpawnWeaponPuff(__d0 short column, __a0 EngineThing *hit)
{
	column >>= 3;
	word size = render_columns.chunk2[column].size_limit;

	//	int xmin = divs(muls(p1x, zoom_i), p1y) + wolf_xoffs;
	//	word s1 = divu((zoom_i*32)<<12, p1y);
	DataActor *actor = &Actor_SmokePuff;
	int p1y;
	if( hit )
	{
		p1y = hit->tr_y - (1<<ENGINE_SUBVERTEX_PRECISION);
		actor = &Actor_BloodSplat;
	}
	else
		p1y = divu((80*32)<<11, size) - (1<<ENGINE_SUBVERTEX_PRECISION);
	int p1x = divs(muls(column - 80,p1y),80);
	int xp = ((((int)p1x)*view_prev_rotate_dx + ((int)p1y)*view_prev_rotate_dy)>>14) + view_prev_pos_x;
	int yp = ((((int)p1y)*view_prev_rotate_dx - ((int)p1x)*view_prev_rotate_dy)>>14) + view_prev_pos_y;

	EngineThing *th = Engine_SpawnActor(actor, xp, yp, 0);
	if( !th )
		return;

	th->zp = -28<<ENGINE_THING_Z_PRECISION;
	Engine_ThingSetVisible(th);
}

void Engine_SpawnMissile(__a0 DataActor *actor, __a6 EngineThing *owner)
{
	EngineThing *th = Engine_AllocThing();
	if( !th )
		return;

	th->actor = (DataActor*)actor;
	if( owner == view_weapon )
	{
		th->unstuck_x = th->xp = view_pos_x;
		th->unstuck_y = th->yp = view_pos_y;
	}
	else
	{
		th->unstuck_x = th->xp = owner->xp;
		th->unstuck_y = th->yp = owner->yp;
	}
	th->vector_x = owner->vector_x;
	th->vector_y = owner->vector_y;
	th->owner = owner;	

	EngineSubSector *ss = Dread_FindSubSector(th->xp, th->yp);
	th->ss_next = ss->first_thing;
	th->ss_prev = NULL;
	if( ss->first_thing )
		ss->first_thing->ss_prev = th;
	ss->first_thing = th;

	th->subsector = ss;

	Script_OnCreate(th);

	Engine_ThingSetVisible(th);
}

void Engine_InitDefaultObjects(void)
{
	view_player = Engine_AllocThing();
	view_player->actor = &Actor_Player;
	Script_OnCreate(view_player);

	view_weapon = Engine_AllocThing();
	view_weapon->actor = &Actor_Pistol;
	Script_OnCreate(view_weapon);
}

void Engine_ThingSetSubsector(__a6 EngineThing *th, __a0 EngineSubSector *ss)
{
	if( ss != th->subsector )
	{
		if( th->subsector )
		{
			// unlink
			if( th->ss_next )
				th->ss_next->ss_prev = th->ss_prev;

			if( th->ss_prev )
				th->ss_prev->ss_next = th->ss_next;
			else
				th->subsector->first_thing = th->ss_next;
		}

		if( ss )
		{
			// link
			th->ss_next = ss->first_thing;
			th->ss_prev = NULL;
			if( ss->first_thing )
				ss->first_thing->ss_prev = th;
			ss->first_thing = th;
		}

		th->subsector = ss;
	}
}

void Engine_ThingSetVisible(EngineThing *th)
{
	if( !th->visible && e_visthings_end < e_visthings_max )
	{
		th->visible = 1;
		*e_visthings_end++ = th;
	}
}

void Engine_ExplosionDamage(__a6 EngineThing *explo, __d7 short damage)
{
	short x0 = explo->xp;
	short y0 = explo->yp;
	for( EngineThing **ptr = e_visthings_end-1; ptr>=e_visthings; )
	{
		EngineThing *th = *ptr--;
		if( th->hitnum )
		{
			short dx = th->xp - x0;
			if( dx<0 ) dx=-dx;
			short dy = th->yp - y0;
			if( dy<0 ) dy=-dy;
			dx = damage - ((dx+dy)>>ENGINE_SUBVERTEX_PRECISION) + th->blocker_size;
			if( dx <= 0 )
				continue;

			th->damage += dx;
			if( e_dmgthings_end < e_dmgthings_max )
				*e_dmgthings_end++ = th;
		}
	}

	// Player
	{
		short dx = view_pos_x - x0;
		if( dx<0 ) dx=-dx;
		short dy = view_pos_y - y0;
		if( dy<0 ) dy=-dy;
		dx = damage - ((dx+dy)>>ENGINE_SUBVERTEX_PRECISION);
		if( dx > 0 )
			Script_PlayerDamaged(dx);
	}
}

int Engine_CheckRadiusBlocked(__a6 EngineThing *center, __d7 short radius)
{
	radius <<= ENGINE_SUBVERTEX_PRECISION;
	short x1 = center->block_x1 - radius;
	short y1 = center->block_y1 - radius;
	short x2 = center->block_x2 + radius;
	short y2 = center->block_y2 + radius;
	for( EngineThing **ptr = e_visthings_end-1; ptr>=e_visthings; )
	{
		EngineThing *th = *ptr--;
		if( th->blocker_size )
		{
			short x = th->xp;
			if( x>=x1 && x<=x2 )
			{
				short y = th->yp;
				if( y>=y1 && y<=y2 && th!=center )
					return 1;
			}
		}
	}
	return 0;
}
