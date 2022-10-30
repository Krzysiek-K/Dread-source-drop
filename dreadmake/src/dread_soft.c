
#include "demo.h"






short render_layout_delta[160];


const byte *e_skymap[128*3];
const byte **e_skyptr;


extern const dword GFXINFO_STBAR0[];		// TBD: remove from here



// Cheat codes:
//	ATFA	- 100% health, all weapons & ammo
//	ATKEY	- all keys
//	ATPOW	- 200% health + 200% armor
//	ATL#	- jump to location #
//			- noclip on/off
//

// Rules:
//	$00			- table end (reset)
//	$01..$7F	- move on if code matches, reset to start on mismatch
//	$80			- on keypress, look ahead until finding proper "$80 <keycode>" sequence or reset if $00 was reached
//	$81			- if keycode is 1..9, jump to that location and reset
//	$90..$9F	- activate cheat (send code as pulse) and reset
//
const byte CHEAT_STATE_MACHINE[] = {
	KEYSCAN_A, KEYSCAN_T,
	0x80, KEYSCAN_F, KEYSCAN_A, 0x90,
	0x80, KEYSCAN_K, KEYSCAN_E, KEYSCAN_Y, 0x91,
	0x80, KEYSCAN_P, KEYSCAN_O, KEYSCAN_W, 0x92,
	0x80, KEYSCAN_R, KEYSCAN_W, KEYSCAN_D, 0x93,
	0x80, KEYSCAN_L, 0x81,
	0x00
};




void Dread_SoftInit(void)
{
	//Dread_Init_Objects();


	for( int i=0; i<3*128; i++ )
		e_skymap[i] = SKYTEX_DATA + ((i&127)<<6);

#if AMIGA
	word _offs = 0;
	for( int i=0; i<160; i++ )
	{
		word offs = 0;
		if( i&1 ) offs += 8080;
		if( i&2 ) offs += 200;
		if( i&4 ) offs++;
		offs += (i>>3)*402;

		render_layout_delta[i] = offs - _offs;
		_offs = offs;
	}
	//	render_columns[i].render_delta = (short)render_columns[i].render_offs - (short)render_columns[i-1].render_offs;
	//render_columns[0].render_delta = 0;
#elif ATARI_ST
	word _offs = 0;
	for( int i=0; i<160; i++ )
	{
		word offs = (i>>1)*200;
		if( i&1 ) offs++;

		render_layout_delta[i] = offs - _offs;
		_offs = offs;
	}
	//	render_columns[i].render_delta = (short)render_columns[i].render_offs - (short)render_columns[i-1].render_offs;
	//render_columns[0].render_delta = 0;
#endif

	Engine_Reset();
	e_globals.skill = 2;
	//Map_LoadAll(2);

	intnum = 0;
}

void Dread_DrawSprite(__d2 int spx, __d1 int spy, __a3 const word *frame)
{
	// Draw
	const int WOLFSCREEN_W = 160;
	const int wolf_xoffs = 80;
	const int zoom_i = 80;
	const int clip = 161;	//161;	// fix.4

	if( spy < clip )
		return;

	int p1x = spx - (frame[1]<<ENGINE_SUBVERTEX_PRECISION);
	int p2x = p1x + (frame[0]<<ENGINE_SUBVERTEX_PRECISION);
	word tx2 = frame[0];


	// xp = px*zoom / py
	int xmax = divs(muls(p2x, zoom_i), spy) + wolf_xoffs;
	if( xmax<=0 ) return;
	int xmin = divs(muls(p1x, zoom_i), spy) + wolf_xoffs;
	if( xmin>=WOLFSCREEN_W ) return;
	if( xmin>=xmax ) return;
	
	long u1s = 0;
	// long u2s = ((long)tx2)<<(16+1);
	long du = (((long)tx2)<<(16+1))/(long)(xmax-xmin);									// result can be 32-bit!!
	if( xmin < 0 )
	{
		u1s = -du*xmin;																	// 32:16-bit multiply !!!
		xmin = 0;
	}
	if( xmax>WOLFSCREEN_W ) xmax = WOLFSCREEN_W;

	short s0 = divs((zoom_i*32)<<(8+ENGINE_SUBVERTEX_PRECISION), spy);		// won't overflow because of clipping p#y>=321
	asm_tex_base = (const byte*)frame + -40;

	Dread_LineCore_Sprite(xmin<<3, xmax-xmin-1, s0, u1s, du, frame);

}


void Dread_CheckCheatCodes(byte key)
{
	//if( e_globals.skill >= 4 ) return;		// no cheats on NIGHTMARE
	if( !key || (key&0x80) ) return;

	byte cmd = CHEAT_STATE_MACHINE[e_globals.cheat_code_pos];

	// multi-scan
	if( cmd==0x80 )
	{
		word scanpos = e_globals.cheat_code_pos;
		while( 1 )
		{
			cmd = CHEAT_STATE_MACHINE[scanpos];
			if( cmd==0 )
			{
				// not found
				e_globals.cheat_code_pos = 0;
				return;
			}
			else if( cmd==0x80 && key==CHEAT_STATE_MACHINE[scanpos+1] )
			{
				e_globals.cheat_code_pos = scanpos+1;
				cmd = key;
				break;		// next part will resolve the key press
			}
			else
				scanpos++;
		}
	}

	// location jump?
	if( cmd==0x81 )
	{
		if( key>=KEYSCAN_1 && key<=KEYSCAN_9 )
		{
			const DataLocation *loc = Map_GetLocation(key-KEYSCAN_1);
			view_fine_pos_x = ((int)loc->xp) << 16;
			view_fine_pos_y = ((int)loc->yp) << 16;
			view_angle = loc->angle & 0xFF;
			view_angle_fine = (view_angle<<8) + 128;
			view_subsector = NULL;
			view_delta_x = 0;
			view_delta_y = 0;
			view_delta_z = 0;
			e_globals.cheat_flags |= 1;
		}
		e_globals.cheat_code_pos = 0;
		return;
	}

	// check key
	if( key!=cmd )
	{
		if( key == KEYSCAN_A )
			e_globals.cheat_code_pos = 1;
		else
			e_globals.cheat_code_pos = 0;
		return;
	}

	// correct - process it
	e_globals.cheat_code_pos++;
	io_key_last = 0;

	// action invoked?
	cmd = CHEAT_STATE_MACHINE[e_globals.cheat_code_pos];
	if( cmd>=0x90 )
	{
		e_globals.cheat_code_pos = 0;
		e_globals.pulse = cmd;
		Script_OnPulse(view_player);
	}
}



void Dread_CameraUpdateBSP(void)
{
	view_subsector = Dread_FindSubSector(view_pos_x, view_pos_y);
	
	word type = view_subsector->type;
	if( type != e_globals.player_sector_type )
	{
		e_globals.player_sector_type = type;
		e_globals.player_floor_timer = 0;
	}
}


static const word DELTA_MUL[] = {
	  0, 14, 28, 41, 53, 64, 75, 85, 95,104,112,120,128,135,142,148,
	154,160,165,171,175,180,184,188,192,196,199,202,205,208,211,213,
};

void Dread_Camera(word ticks, word real_ticks)
{
	short mwalk = 0;
	short mside = 0;
	short mfly = 0;

	Dread_CheckCheatCodes(io_key_last);

	if( e_globals.player_health > 0 )
	{
		// rotate
		view_angle_fine += io_mouse_xdelta*opt_mouse_sens;
		view_angle = view_angle_fine >> 8;		// "& 0xFF" not needed after the shift
		io_mouse_xdelta = 0;

		// walk
		word move = (16<<8)/6;
		if( io_keystate[KEYSCAN_W] || io_keystate[KEYSCAN_UP]    ) mwalk += move;
		if( io_keystate[KEYSCAN_S] || io_keystate[KEYSCAN_DOWN]  ) mwalk -= move;
		if( io_keystate[KEYSCAN_A] || io_keystate[KEYSCAN_LEFT]  ) mside -= move;
		if( io_keystate[KEYSCAN_D] || io_keystate[KEYSCAN_RIGHT] ) mside += move;
		if( io_keystate[KEYSCAN_Q] ) mfly += move;
		if( io_keystate[KEYSCAN_E] ) mfly -= move;

		// weapon selection
		if( io_key_last>=KEYSCAN_1 && io_key_last<=KEYSCAN_9 )
		{
			e_globals.pulse = io_key_last - KEYSCAN_1 + 1;
			io_key_last = 0;
			e_globals.weapon_send_pulse = 0;
			Script_OnPulse(view_weapon);
		}
		else if( e_globals.weapon_send_pulse )
		{
			e_globals.pulse = e_globals.weapon_send_pulse;
			e_globals.weapon_send_pulse = 0;
			Script_OnPulse(view_weapon);
		}
	}
	else
	{
		io_mouse_xdelta = 0;
	
		// rotate to damage source
		word step = (real_ticks+15) << 4;
		for(byte pass=0;pass<2;pass++)
		{
			view_rotate_dx = sincos_fix14[(view_angle+64)&0xFF];
			view_rotate_dy = sincos_fix14[(view_angle)&0xFF];
			short dx = e_globals.player_damage_source_x - view_pos_x;
			short dy = e_globals.player_damage_source_y - view_pos_y;
		
			if( dx*view_rotate_dx - dy*view_rotate_dy < 0 )
				view_angle_fine -= step;
			else
				view_angle_fine += step;
			
			view_angle = view_angle_fine >> 8;
		}

		view_rotate_dx = sincos_fix14[(view_angle+64)&0xFF];
		view_rotate_dy = sincos_fix14[(view_angle)&0xFF];
	}


	// cheat/test keys
	if( io_key_last>=KEYSCAN_F1 && io_key_last<=KEYSCAN_F9 )
	{
		if( e_globals.cheat_flags )
		{
			const DataLocation *loc = Map_GetLocation(io_key_last-KEYSCAN_F1);
			view_fine_pos_x = ((int)loc->xp) << 16;
			view_fine_pos_y = ((int)loc->yp) << 16;
			view_fine_pos_z = -40 << 16;
			view_angle = loc->angle & 0xFF;
			view_angle_fine = (view_angle<<8) + 128;
			view_subsector = NULL;

			mwalk = 0;
			mside = 0;
			mfly = 0;
			view_delta_x = 0;
			view_delta_y = 0;
			view_delta_z = 0;
		}

		io_key_last = 0;
	}
	else if( io_key_last == KEYSCAN_LBRACE )
	{
		debug_vars[2] = (debug_vars[2]+999)%1000;
		io_key_last = 0;
	}
	else if( io_key_last == KEYSCAN_RBRACE )
	{
		debug_vars[2] = (debug_vars[2]+1)%1000;
		io_key_last = 0;
	}
	//if( io_key_last == KEYSCAN_ESC )
	//{
	//	io_key_last = 0;
	//	Map_LoadAll(e_globals.skill);
	//}


	if( ticks > 0 )
	{
		int force_x = (((int)sincos_fix14[view_angle])*mwalk + ((int)sincos_fix14[(view_angle+64)&0xFF])*mside)>>8;
		int force_y = (((int)sincos_fix14[(view_angle+64)&0xFF])*mwalk - ((int)sincos_fix14[view_angle])*mside)>>8;
		if( ticks<32 )
		{
			view_delta_x += ((force_x-view_delta_x)*DELTA_MUL[ticks])>>8;
			view_delta_y += ((force_y-view_delta_y)*DELTA_MUL[ticks])>>8;
		}
		else
		{
			view_delta_x = force_x;
			view_delta_y = force_y;
		}

#if 1
		// dT = ticks/300
		// V/300 += 8000<<16/300/300 * ticks
		// Z += V/300 * ticks
		view_delta_z += 500 * ticks;						//	yvel -= 8000 * Dev.GetTimeDelta();
		view_fine_pos_z += view_delta_z * ticks;			//	real_cam_pos.z += yvel * Dev.GetTimeDelta();
		view_pos_z = (short)(view_fine_pos_z>>16) & 0xFFFE;
#else
		view_fine_pos_z += (mfly<<(14-8))*ticks;
#endif

		view_pos_z = (short)(view_fine_pos_z>>16) & 0xFFFE;
		view_rotate_dx = sincos_fix14[(view_angle+64)&0xFF];
		view_rotate_dy = sincos_fix14[(view_angle)&0xFF];

		if( (view_delta_x | view_delta_y) & ~255 )
		{
			short startx = (short)(view_fine_pos_x>>(16-ENGINE_SUBVERTEX_PRECISION));
			short starty = (short)(view_fine_pos_y>>(16-ENGINE_SUBVERTEX_PRECISION));
			view_fine_pos_x += view_delta_x*ticks;
			view_fine_pos_y += view_delta_y*ticks;
			short endx = (short)(view_fine_pos_x>>(16-ENGINE_SUBVERTEX_PRECISION));
			short endy = (short)(view_fine_pos_y>>(16-ENGINE_SUBVERTEX_PRECISION));

			if( Physics_WalkSlide(startx, starty, endx, endy) )
			{
				view_fine_pos_x = ((int)physics_endx)<<(16-ENGINE_SUBVERTEX_PRECISION);
				view_fine_pos_y = ((int)physics_endy)<<(16-ENGINE_SUBVERTEX_PRECISION);
				view_delta_x = force_x;
				view_delta_y = force_y;
			}
			view_pos_x = physics_endx;
			view_pos_y = physics_endy;

			view_subsector = NULL;
		}
		else
		{
			view_pos_x = (short)(view_fine_pos_x>>(16-ENGINE_SUBVERTEX_PRECISION));
			view_pos_y = (short)(view_fine_pos_y>>(16-ENGINE_SUBVERTEX_PRECISION));
		}
	}

	if( !view_subsector )
		Dread_CameraUpdateBSP();

	if( view_subsector )
	{
		short cam_z = view_subsector->height - 40;
		if( view_pos_z >= cam_z )
		{
			view_fine_pos_z = (int)cam_z << 16;
			view_pos_z = (short)(view_fine_pos_z>>16) & 0xFFFE;
			view_delta_z = 0;
		}
	}

	if( e_globals.player_sector_type && e_globals.player_floor_timer < 0xFFFF )
	{
		if( ticks >= e_globals.player_floor_timer )
		{
			word old_floor_timer = e_globals.player_floor_timer;
			e_globals.pulse = 0xFF;
			Script_OnPulse(view_player);

			if( e_globals.player_floor_timer == old_floor_timer )
				e_globals.player_floor_timer = 0xFFFF;
			else if( ticks < e_globals.player_floor_timer )
				e_globals.player_floor_timer -= ticks;

			if( !view_subsector )
				Dread_CameraUpdateBSP();
		}
		else
			e_globals.player_floor_timer -= ticks;
	}

#if !WEAPONSPRITES
	// weapon swing
	if( mwalk | mside )
		view_weapon_swing += ticks>>1;
	else if( view_weapon_swing & 0x7F )
	{
		word pswing = view_weapon_swing;
		if( view_weapon_swing & 0x40 )	view_weapon_swing += ticks;
		else							view_weapon_swing -= ticks;
		if( (pswing^view_weapon_swing) & 0x80 )
			view_weapon_swing = 0;
	}
#else
	view_movement = (word)(mwalk | mside);
#endif
}


void Dread_WeaponLogic(void)
{
	if( io_mouse_state && e_globals.player_health>0 )
	{
		e_globals.pulse = io_mouse_state;
		io_mouse_state &= 0x30;
		Script_OnPulse(view_weapon);
	}

	if( view_weapon->timer_attack )
		view_weapon_swing = 0;
}


void Dread_RunLogic(void)
{
	word ticks = intnum*6;			// TBD: PAL NTSC
	intnum = 0;
	view_frame_number++;

	word runticks = (e_globals.player_health > 0 || MULTIPLAYER_MODE) ? ticks : 0;

	e_globals.levstat_time += runticks;

	// Update status bar before anything else because rendering is already 1 frame late
#if AMIGA
	Statbar_Update(statbar_buffer, GFXINFO_STBAR0);
#endif

	e_globals.io_mouse_state = io_mouse_state;

#define TEX_SUBSCR02_OFFS               	32768	// SUBSCR02   64x128 (255)  [ SUBSCR02 ]
#define TEX_SUBSCR01_OFFS               	49152	// SUBSCR01   64x128 (255)  [ SUBSCR01 ]

	if( io_key_last==KEYSCAN_SPACE )	// Space
	{
		if( e_globals.player_health > 0 )
		{
			io_key_last = 0;
			Machine_Activate(view_pos_x>>ENGINE_SUBVERTEX_PRECISION, view_pos_y>>ENGINE_SUBVERTEX_PRECISION);
		}
#if MULTIPLAYER_MODE
		else if( MULTIPLAYER_MODE )
		{
			// Respawn
			MP_Respawn();
		}
#endif
		else
			io_key_last = KEYSCAN_ESC;
	}
//	else if( io_key_last==KEYSCAN_Q )	// Q
//	{
//		EngineLine *line = e_lines;
//		EngineLine *end = e_lines + e_n_lines;
//		for( ; line < end; line++ )
//		{
//			if( line->tex_upper == e_map_textures + TEX_SUBSCR01_OFFS )
//				line->tex_upper += 64;
//		}
//	}
//	else if( io_key_last==KEYSCAN_E )	// E
//	{
//		EngineLine *line = e_lines;
//		EngineLine *end = e_lines + e_n_lines;
//		for( ; line < end; line++ )
//		{
//			if( line->tex_upper == e_map_textures + TEX_SUBSCR02_OFFS )
//				line->tex_upper += 64;
//		}
//	}


	// animate palette & weapon drop
	if( e_globals.player_health > 0 )
	{
		view_weapon_drop = 0;
		if( e_globals.player_red_glow >= ticks )
			e_globals.player_red_glow -= ticks;
		else
			e_globals.player_red_glow = 0;
	}
	else
	{
		e_globals.player_red_glow += ticks;
		if( e_globals.player_red_glow > (8<<4) )
			e_globals.player_red_glow = (8<<4);

		view_weapon_drop += ticks >> 2;
		if( view_weapon_drop > 96 )
			view_weapon_drop = 96;
	}

	// player timers
	if( ticks < e_globals.player_boots_timer )
		e_globals.player_boots_timer -= ticks;
	else
		e_globals.player_boots_timer = 0;

	// camera
	Dread_Camera(runticks, ticks);
	if( runticks > 0 )
	{
		Script_TickObjects(runticks);

		if( e_globals.update_conditions )
		{
			Machine_UpdateConditions();
			e_globals.update_conditions = 0;
		}

#if AMIGA
		Aud_VolumeUpdateRequest();
#endif
	}


	// animate sky
	static word skyanim = 0;
	skyanim += runticks;
	e_skyptr = e_skymap + (((view_angle<<1)-(skyanim>>7))&127) + 16;
}


void Dread_Render(void)
{
	// init clip buffer
	Dread_FrameReset(render_buffer);
#if WALL_RENDERER_VERSION == 2
	if( cpu_type >= 20 )
		memset(render_buffer, 0xF7, 16160);	// 11:15
#endif

	// init drawing state
	asm_fn_skycopy = FN_SKYCOPY;
	asm_render_buffer = render_buffer;


	// draw lines
	Dread_DrawVisible(view_subsector->vis);
	Dread_DrawLinesTable();

	Dread_DrawSprites();

	view_prev_pos_x			= view_pos_x;
	view_prev_pos_y			= view_pos_y;
	view_prev_pos_z			= view_pos_z;
	view_prev_rotate_dx		= view_rotate_dx;
	view_prev_rotate_dy		= view_rotate_dy;


	// select palette
	if( e_globals.player_pal_flash )
	{
		palette_index = e_globals.player_pal_flash;
		e_globals.player_pal_flash = 0;
	}
	else if( e_globals.player_red_glow > 0 )
	{
		int pal = e_globals.player_red_glow >> 4;
		if( pal>8 )
			pal = 8;
		palette_index = pal;
	}
	else if( e_globals.player_boots_timer )
		palette_index = 10;
	else
		palette_index = 0;

}

