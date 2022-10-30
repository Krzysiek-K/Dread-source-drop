
#ifndef _DREAD_ESTRUCTS_H
#define _DREAD_ESTRUCTS_H



// ================================================================ Engine* types - run-time structures ================================================================


typedef struct _EngineVertex	EngineVertex;
typedef struct _EngineLine		EngineLine;
typedef struct _EngineSubSector	EngineSubSector;
typedef struct _EngineThing		EngineThing;
typedef struct _EngineMachine	EngineMachine;
typedef struct _EngineVisLine	EngineVisLine;
typedef struct _EngineGlobals	EngineGlobals;





// ================================================ EngineVertex ================================================

//$asmstruct vtx
struct _EngineVertex {
	short	xp;
	short	yp;
	short	tr_x;		// transformed to camera space, fixed point		(tr_x==0: not transformed yet)
	short	tr_y;
};
//const int foo = sizeof(EngineVertex);



// ================================================ EngineLine ================================================

// one side of a line
//$asmstruct line
struct _EngineLine {
	EngineVertex	*v1;			//	
	EngineVertex	*v2;			//	
	const byte		*tex_upper;		//	the only texture for one-sided walls		(texture index includes light level)
	word			xcoord1;		//	starting texture coord
	word			xcoord2;		//	ending texture coord
	byte			ceil_col;		//	Ceiling color
	byte			floor_col;		//	Floor color
	byte			modes[2];		//	

	const byte		*tex_lower;		//	
	short			y1;				//	ceiling -> upper
	short			y2;				//	upper -> gap
	short			y3;				//	gap -> lower
	short			y4;				//	lower -> floor
};
//const int foo = sizeof(EngineLine);		// 20 32					Demo 566: 11320 18112			Jammer 1224: 24480 39168

// ================================================ EngineSubSector ================================================

// BSP leaf
//$asmstruct subsector
struct _EngineSubSector {
	const word		*vis;
	EngineThing		*first_thing;
	word			visframe;
	word			type;
	word			height;
};
//const int foo = sizeof(EngineSubSector);



// ================================================ EngineThing ================================================

//$asmstruct thing
struct _EngineThing {
	EngineThing		*ss_next;			//
	EngineThing		*ss_prev;			//
	EngineSubSector	*subsector;			//
	byte			visible;			//	0: not in <visthings>	1: in <visthings>
	byte			blocker_size;		//
	byte			pickup;				//
	char			gravity;			//
	byte			move_collide;		//
	byte			_pad;				//
	short			block_x1;			//
	short			block_y1;			//
	short			block_x2;			//
	short			block_y2;			//

	DataActor		*actor;				//
	const word		*sprite;			//
	short			xp;					//
	short			yp;					//
	short			zp;					//
	short			tr_x;				//
	short			tr_y;				//
	short			vel_z;				//
	short			unstuck_x;			//
	short			unstuck_y;			//
	word			hitnum;				//
	word			index;				//
	word			machine_id;			//
	short			machine_position;	//
	short			machine_speed;		//
	EngineThing		*owner;				//

	void			*script_fn;			//
	word			timer_delay;		//
	word			timer_attack;		//
	word			timer_walk;			//
	word			health;				//
	word			damage;				//
	short			vector_x;			//
	short			vector_y;			//
	word			distance;			//
	short			step_xp;			//	movement delta during 8 ticks
	short			step_yp;			//
	word			step_count;			//
	short			step_endx;			//
	short			step_endy;			//
	short			state;				//
	short			value;				//
	short			value2;				//
	short			value3;				//
	short			value4;				//
};


// ================================================ EngineMachine ================================================


struct _EngineMachine {
	word		line_id;
	EngineThing	*blocker;
	word		line_len;
};


// ================================================ EngineVisLine ================================================

struct _EngineVisLine {
	word		linemode;		// 0xFFFF - end of list
	EngineLine	*line;
};


// ================================================ EngineGlobals ================================================

//$asmstruct gv
struct _EngineGlobals {
	word		randpos;			// must stay 0..255
	word		io_mouse_state;
	word		skill;
	word		pulse;
	word		player_ammo;
	word		player_health;
	word		player_armor;
	word		player_bullets;
	word		player_shells;
	word		player_rockets;
	word		player_cells;
	word		player_bullets_cap;
	word		player_shells_cap;
	word		player_rockets_cap;
	word		player_cells_cap;
	short		player_red_glow;
	word		player_pal_flash;
	word		player_face;
	word		weapon_send_pulse;
	word		statbar_key_0;
	word		statbar_key_1;
	word		statbar_key_2;
	word		statbar_weapon_0;
	word		statbar_weapon_1;
	word		statbar_weapon_2;
	word		statbar_weapon_3;
	word		statbar_weapon_4;
	word		statbar_weapon_5;
	word		update_conditions;
	word		temp1;
	word		temp2;
	word		temp3;
	short		levstat_monsters;
	short		levstat_monsters_max;
	short		levstat_items;
	short		levstat_items_max;
	short		levstat_secrets;
	short		levstat_secrets_max;
	int			levstat_time;
	int			levstat_secrets_done;
	word		game_state;
	byte		cheat_code_pos;
	byte		cheat_flags;
	word		debug_var;
	word		debug_var2;
	word		debug_var3;
	word		debug_var4;
	short		player_damage_source_x;
	short		player_damage_source_y;
	word		player_floor_timer;
	word		player_sector_type;
	word		player_sector_tag;
	word		player_boots_timer;
	dword		switch_flags;
	byte		special_monsters_blue_key;
	byte		special_monsters_yellow_key;
	byte		special_monsters_red_key;
	byte		mp_mode;
	byte		mp_commstate;
	byte		mp_spawnresponse;
	byte		mp_deaths;
	byte		mp_frags;
};



// ================================================ RenderColumnsInfo ================================================

typedef struct {
	struct {
		word	*render_addr;
		word	ymin;
		word	ymax;
	} chunk1[160];
	struct {
		word	size_limit;
		word	thing_num;
		word	*sprite_render_addr;
	} chunk2[160];
} RenderColumnsInfo;





// ================================================


// Vertexes
extern EngineVertex		*e_vtx;
extern word				e_n_vtx;

// Lines
extern EngineLine		*e_lines;
extern word				e_n_lines;

// Subsectors
extern EngineSubSector	*e_subsectors;
extern word				e_n_subsectors;

// Things
extern __chip EngineThing		e_things_buffer[ENGINE_MAX_THINGS];
extern EngineThing		*e_things;
extern word				e_n_things;
extern word				e_max_things;
extern EngineThing		*e_delthings[ENGINE_MAX_THINGS];
extern EngineThing		**e_delthings_end;

// Machines
extern EngineMachine	e_machines[ENGINE_MAX_MACHINES];
extern word				e_n_machines;

// Conditions
extern byte				e_condition_states[ENGINE_MAX_CONDITIONS];

// Vislines
extern EngineVisLine	e_vislines[ENGINE_MAX_VISLINES+1];

// VisThings
extern EngineThing		*e_visthings[ENGINE_MAX_VISTHINGS];
extern EngineThing		**e_visthings_end;
extern EngineThing		**e_visthings_max;

// Map data
extern DataHeader		*e_map_header;
extern DataBSPNode		*e_map_bspnodes;
extern word				*e_map_vislist;
extern DataCondition	*e_map_conditions;
extern byte				*e_map_textures;
extern word				*e_map_palette;
extern word				e_map_palette_count;

extern word				map_error_code;
extern dword			map_error_offset;
extern dword			map_error_arg;



// Globals
extern EngineGlobals	e_globals;

// Render data
extern RenderColumnsInfo render_columns;

// Text
extern const byte		*e_text;




// View variables (not in struct for easier access from asm)
extern int				view_fine_pos_x;	// Logic
extern int				view_fine_pos_y;
extern int				view_fine_pos_z;
extern int				view_delta_x;
extern int				view_delta_y;
extern int				view_delta_z;
extern word				view_weapon_swing;
extern short			view_weapon_drop;
extern word				view_movement;
extern EngineThing		*view_weapon;
extern EngineThing		*view_player;
extern EngineThing		*mp_avatar;

extern short			view_pos_x;			// Rendering
extern short			view_pos_y;
extern short			view_pos_z;
extern word				view_angle;
extern word				view_angle_fine;
extern short			view_rotate_dx;
extern short			view_rotate_dy;
extern EngineSubSector	*view_subsector;
extern word				view_frame_number;
extern short			view_prev_pos_x;
extern short			view_prev_pos_y;
extern short			view_prev_pos_z;
extern short			view_prev_rotate_dx;
extern short			view_prev_rotate_dy;




void Engine_Reset(void);
EngineThing *Engine_AllocThing(void);
EngineThing *Engine_SpawnActor(const DataActor *actor, short xp, short yp, word flags);
EngineThing *Engine_SpawnActor_NoPosition(const DataActor *actor);
void Engine_SetThingPosition(EngineThing *th, short xp, short yp);
void Engine_SpawnWeaponPuff(__d0 short column, __a0 EngineThing *hit);
void Engine_InitDefaultObjects(void);
void Engine_ThingSetSubsector(__a6 EngineThing *th, __a0 EngineSubSector *ss);
void Engine_ThingSetVisible(EngineThing *th);



#endif
