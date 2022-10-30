
#include "demo.h"






short render_layout_delta[160];


const byte *e_skymap[128*3];
const byte **e_skyptr;


extern const dword GFXINFO_STBAR0[];		// TBD: remove from here






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






void Dread_CameraUpdateBSP(void)
{
	//short bsp = 0;
	//do
	//{
	//	const DataBSPNode *node = (const DataBSPNode*)(((const byte*)MAP_NODES) + bsp);
	//	int side = muls(view_pos_x, node->A) + muls(view_pos_y, node->B) + node->C;
	//	bsp = side>=0 ? node->right : node->left;
	//} while( bsp>=0 );
	//view_subsector = MAP_SUBSECTORS + ~bsp;

	view_subsector = Dread_FindSubSector(view_pos_x, view_pos_y);

	word type = MAP_SUBSECTORS[view_subsector - e_subsectors].type;
	switch(type)
	{
	case 1:
		e_globals.game_state = GAMESTATE_LEVEL_DONE;
		break;
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
		{
			word secmask = 1 << ((word)(type - 21));
			if( !(e_globals.levstat_secrets_done & secmask) )
			{
				e_globals.levstat_secrets++;
				e_globals.levstat_secrets_done |= secmask;
			}
		}
		break;
	}
}


static const word DELTA_MUL[] = {
	  0, 14, 28, 41, 53, 64, 75, 85, 95,104,112,120,128,135,142,148,
	154,160,165,171,175,180,184,188,192,196,199,202,205,208,211,213,
};

void Dread_Camera(word ticks)
{
	static int delta_x = 0;
	static int delta_y = 0;

	// rotate
	view_angle += (io_mouse_xdelta*opt_mouse_sens)>>8;
	io_mouse_xdelta = 0;
	view_angle &= 0xFF;

	// walk
	word move = (16<<8)/6;
	short mwalk = 0;
	short mside = 0;
	if( io_keystate[KEYSCAN_W] ) mwalk += move;
	if( io_keystate[KEYSCAN_S] ) mwalk -= move;
	if( io_keystate[KEYSCAN_A] ) mside -= move;
	if( io_keystate[KEYSCAN_D] ) mside += move;
	if( io_keystate[KEYSCAN_Q] ) view_fine_pos_z += move<<14;
	if( io_keystate[KEYSCAN_E] ) view_fine_pos_z -= move<<14;
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
	if( io_key_last>=KEYSCAN_F1 && io_key_last<=KEYSCAN_F9 )
	{
		if( e_globals.player_bullets_cap > 200 )
		{
			const DataLocation *loc = Map_GetLocation(io_key_last-KEYSCAN_F1);
			view_fine_pos_x = ((int)loc->xp) << 16;
			view_fine_pos_y = ((int)loc->yp) << 16;
			view_angle = (-loc->angle/45*32) & 0xFF;
			view_subsector = NULL;

			mwalk = 0;
			mside = 0;
			delta_x = 0;
			delta_y = 0;
		}

		io_key_last = 0;
	}
	if( io_key_last == KEYSCAN_LBRACE )
	{
		debug_vars[2] = (debug_vars[2]+999)%1000;
		io_key_last = 0;
	}
	if( io_key_last == KEYSCAN_RBRACE )
	{
		debug_vars[2] = (debug_vars[2]+1)%1000;
		io_key_last = 0;
	}
	//if( io_key_last == KEYSCAN_ESC )
	//{
	//	io_key_last = 0;
	//	Map_LoadAll(e_globals.skill);
	//}



	int force_x = ( ((int)sincos_fix14[view_angle])*mwalk + ((int)sincos_fix14[(view_angle+64)&0xFF])*mside )>>8;
	int force_y = ( ((int)sincos_fix14[(view_angle+64)&0xFF])*mwalk - ((int)sincos_fix14[view_angle])*mside )>>8;
	if( ticks>0 )
	{
		if( ticks<32 )
		{
			delta_x += ((force_x-delta_x)*DELTA_MUL[ticks])>>8;
			delta_y += ((force_y-delta_y)*DELTA_MUL[ticks])>>8;
		}
		else
		{
			delta_x = force_x;
			delta_y = force_y;
		}
	}

	view_pos_z = (short)(view_fine_pos_z>>16);
	view_rotate_dx = sincos_fix14[(view_angle+64)&0xFF];
	view_rotate_dy = sincos_fix14[(view_angle)&0xFF];

#if 1
	if( (delta_x | delta_y) & ~255 )
	{
		short startx = (short)(view_fine_pos_x>>(16-ENGINE_SUBVERTEX_PRECISION));
		short starty = (short)(view_fine_pos_y>>(16-ENGINE_SUBVERTEX_PRECISION));
		view_fine_pos_x += delta_x*ticks;
		view_fine_pos_y += delta_y*ticks;
		short endx = (short)(view_fine_pos_x>>(16-ENGINE_SUBVERTEX_PRECISION));
		short endy = (short)(view_fine_pos_y>>(16-ENGINE_SUBVERTEX_PRECISION));
		
		if( Physics_WalkSlide(startx, starty, endx, endy) )
		{
			view_fine_pos_x = ((int)physics_endx)<<(16-ENGINE_SUBVERTEX_PRECISION);
			view_fine_pos_y = ((int)physics_endy)<<(16-ENGINE_SUBVERTEX_PRECISION);
			delta_x = force_x;
			delta_y = force_y;
		}
		view_pos_x = physics_endx;
		view_pos_y = physics_endy;

		Dread_CameraUpdateBSP();
	}
	else
	{
		view_pos_x = (short)(view_fine_pos_x>>(16-ENGINE_SUBVERTEX_PRECISION));
		view_pos_y = (short)(view_fine_pos_y>>(16-ENGINE_SUBVERTEX_PRECISION));

		if( !view_subsector )
			Dread_CameraUpdateBSP();
	}

#else
	view_fine_pos_x += delta_x*ticks;
	view_fine_pos_y += delta_y*ticks;
	view_pos_x = (short)(view_fine_pos_x>>12);
	view_pos_y = (short)(view_fine_pos_y>>12);
	Dread_CameraUpdateBSP();
#endif

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


void Stat_Set_asm(__a0 word *stat, __d0 word value, __d2 word num_base);

void Dread_WeaponLogic(void)
{
	if( io_mouse_state )
	{
		e_globals.pulse = io_mouse_state;
		io_mouse_state &= 0x30;
		Script_OnPulse(view_weapon);
	}

	if( view_weapon->timer_attack )
		view_weapon_swing = 0;
}


void Dread_SoftTest(void)
{
	word ticks = intnum*6;			// TBD: PAL NTSC
	intnum = 0;
	view_frame_number++;
	e_globals.levstat_time += ticks;

	// Update status bar before anything else because rendering is already 1 frame late
#if AMIGA
#if AMIGA_USE_NEW_STATBAR
	Statbar_Update(statbar_buffer, GFXINFO_STBAR0);
#else
	Stat_Set_asm(&e_status_bar.ammo, e_globals.player_ammo, 1);
	Stat_Set_asm(&e_status_bar.health, e_globals.player_health, 1);
	Stat_Set_asm(&e_status_bar.armor, e_globals.player_armor, 1);
	Stat_Set_asm(&e_status_bar.bullets, e_globals.player_bullets, 12);
	Stat_Set_asm(&e_status_bar.shells, e_globals.player_shells, 12);
	Stat_Set_asm(&e_status_bar.rockets, e_globals.player_rockets, 12);
	Stat_Set_asm(&e_status_bar.cells, e_globals.player_cells, 12);
	Stat_Set_asm(&e_status_bar.cap_bullets, e_globals.player_bullets_cap, 12);
	Stat_Set_asm(&e_status_bar.cap_shells, e_globals.player_shells_cap, 12);
	Stat_Set_asm(&e_status_bar.cap_rockets, e_globals.player_rockets_cap, 12);
	Stat_Set_asm(&e_status_bar.cap_cells, e_globals.player_cells_cap, 12);
	e_status_bar.key_0 = e_globals.statbar_key_0;
	e_status_bar.key_1 = e_globals.statbar_key_1;
	e_status_bar.key_2 = e_globals.statbar_key_2;
	e_status_bar.weapon_0 = e_globals.statbar_weapon_0;
	e_status_bar.weapon_1 = e_globals.statbar_weapon_1;
	e_status_bar.weapon_2 = e_globals.statbar_weapon_2;
	e_status_bar.weapon_3 = e_globals.statbar_weapon_3;
	e_status_bar.weapon_4 = e_globals.statbar_weapon_4;
	e_status_bar.weapon_5 = e_globals.statbar_weapon_5;
#endif
#endif

	e_globals.io_mouse_state = io_mouse_state;

//#define TEX_SUBSCR02_OFFS               	32768	// SUBSCR02  64x128 (255)
//#define TEX_SUBSCR01_OFFS               	40960	// SUBSCR01  64x128 (255)

	if( io_key_last==KEYSCAN_SPACE )	// Space
	{
		io_key_last = 0;
		Machine_Activate(view_pos_x>>ENGINE_SUBVERTEX_PRECISION, view_pos_y>>ENGINE_SUBVERTEX_PRECISION);
	}
//	else if( io_key_last==KEYSCAN_Q )	// Q
//	{
//		EngineLine *line = e_lines;
//		EngineLine *end = e_lines + e_n_lines;
//		for( ; line < end; line++ )
//		{
//			if( line->tex_upper == TEXTURE_DATA + TEX_SUBSCR01_OFFS )
//				line->tex_upper += 64;
//		}
//	}
//	else if( io_key_last==KEYSCAN_E )	// Q
//	{
//		EngineLine *line = e_lines;
//		EngineLine *end = e_lines + e_n_lines;
//		for( ; line < end; line++ )
//		{
//			if( line->tex_upper == TEXTURE_DATA + TEX_SUBSCR02_OFFS )
//				line->tex_upper += 64;
//		}
//	}

	// camera
	Dread_Camera(ticks);
	
	Script_TickObjects(ticks);
	Machine_TickAll(ticks);

#if AMIGA
	Aud_VolumeUpdateRequest();
#endif

	// init clip buffer
	Dread_FrameReset(render_buffer);

	// init drawing state
	static word skyanim = 0;
	skyanim += ticks;
	e_skyptr = e_skymap + (((view_angle<<1)-(skyanim>>5))&127) + 16;
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

		if( e_globals.player_health > 0 )
		{
			e_globals.player_red_glow -= ticks;
		}
		else
		{
			e_globals.player_red_glow += ticks;
			if( e_globals.player_red_glow > (8<<4) )
				e_globals.player_red_glow = (8<<4);
		}
	}
	else
		palette_index = 0;

}
