
#include "demo.h"





extern const word GFX_ST_STBAR0[];
extern const dword GFXINFO_ST_STBAR0[];
extern const word PALETTE_MAIN[];
extern const word PALETTE_MAIN_STe[];
extern const word PALETTE_HUD[];

extern const word SOUND_DSLEVEND[];
extern const word SOUND_DSWEAP1[];



//word SOUND_DSPLPAIN;
//word SOUND_DSPSTART;
//word SOUND_DSBAREXP;
//word SOUND_DSRLAUNC;
//word SOUND_DSWPNUP;
//word SOUND_DSDBLOAD;
//word SOUND_DSITEMUP;
//word SOUND_DSPISTOL;
//word SOUND_DSSHOTGN;
//word SOUND_DSPOPAIN;
//word SOUND_DSPODTH1;

//word SOUND_DSNOCARD;
//word SOUND_DSCHGN3;
//word SOUND_DSPOPAIN2;
////word SOUND_DSPODTH2;
//word SOUND_DSHEADATK;
//word SOUND_DSHEADPAIN1;
//word SOUND_DSHEADPAIN2;
//word SOUND_DSHEADDTH;
//word SOUND_DSCHGN3S;
//word SOUND_DSCHGN3E;


word _screen[320*200/16*4*2+256];
word *screen;
word *screen_active;

word *music_active;
word music_track;

word render_screen[160*100/2];


dword frame_time[16];
byte frame_time_pos;
word statbar_shadows[2][ENGINE_MAX_STATBAR_CACHE];

word map_data_buffer[MAP_MEMORY_BUFFER_SIZE/sizeof(word)];



// Machine types:
//	0x0000 	0x0000 	Atari ST(260 ST, 520 ST, 1040 ST, Mega ST, ...)
//	0x0000 	0x4D34 	Medusa T40 without SCSI
//	0x0001 	0x0000 	Atari STE(1040 STE, ST-Book)
//	0x0001 	0x0010 	Mega STE
//	0x0001 	0x0100 	Sparrow(Falcon pre-production machine)
//	0x0002 	0x0000 	Atari TT or Hades
//	0x0002 	0x4D34 	Medusa T40 with SCSI
//	0x0003 	0x0000 	Atari-Falcon030
//	0x0004 	0x0000 	Milan
//	0x0005 	0x0000 	ARAnyM >=v0.8.5beta
word st_pal_mode;
word st_machine_type;
word st_cpu_type;




typedef struct
{
	byte cookie_id[4];   /* Identification code */
	dword cookie_value;   /* Value of the cookie */
} TOS_COOKIE;

#define _p_cookies		(*(volatile TOS_COOKIE**)0x05a0)

static dword ST_GetCookie(char c0, char c1, char c2, char c3)
{
	set_supervisor();

	static TOS_COOKIE *cookie;
	cookie = _p_cookies;
	if( cookie )
	{
		//return *(dword*)cookie;
		while( cookie->cookie_id[0] | cookie->cookie_id[1] | cookie->cookie_id[2] | cookie->cookie_id[3] )
		{
			if( cookie->cookie_id[0]==(byte)c0 &&
				cookie->cookie_id[1]==(byte)c1 &&
				cookie->cookie_id[2]==(byte)c2 &&
				cookie->cookie_id[3]==(byte)c3 )
			{
				set_usermode();
				return cookie->cookie_value;
			}
	
			cookie++;
		}
	}

	set_usermode();
	return 0;
}








void Statbar_Init(word *screenbase, const dword *info, word *shadow)
{
	const word *bg = (const word*)(*info++);
	memcpy(screenbase, bg+2, ((bg[0]>>1)&~1)*bg[1]);
	while( 1 )
	{
		dword code = *info++;
		byte cmd = (byte)(code>>24);
		if( cmd>=1 && cmd<=3 )
		{
			info += 2;
			*shadow++ = 0xFFFF;
			while( cmd-->0 )
				*shadow++ = 0xFFFF;
		}
		else if( (cmd&0xFE)==0x10 )
		{
			info += 2;
			*shadow++ = 0xFFFF;
		}
		else
			break;
	}
}

void Statbar_Update(word *screenbase, const dword *info, word *shadow)
{
	const word *bg = (const word*)(*info++) + 2;

	while( 1 )
	{
		dword code = *info++;
		byte cmd = (byte)(code>>24);
		if( cmd>=1 && cmd<=3 )
		{
			// draw value
			const word **font = (const word**)(*info++);
			word value = *(word*)(*info++);

			if( value == *shadow )
			{
				shadow++;
				shadow += cmd;
				continue;
			}
			*shadow++ = value;

			word xstep = (word)(code>>17) & 0x007F;
			word xpos = (word)(code>>8) & 0x01FF;
			word ypos = (byte)code;
			byte first = 1;
			cmd--;
			xpos += cmd * xstep;
			do
			{
				word glyphnum = 0;
				if( value || first )
				{
					glyphnum = value%10 + 1;
					first = 0;
				}
				value /= 10;

				if( *shadow != glyphnum )
				{
					*shadow = glyphnum;

					ST_DrawClearMaskedNarrow(xpos, ypos, screenbase, font[glyphnum], bg);
				}
				
				xpos -= xstep;
				shadow++;
			} while( cmd-->0 );
		}
		else if( cmd==0x10 )
		{
			// draw frame
			const word **frames = (const word**)(*info++);
			word value = *(word*)(*info++);

			if( value == *shadow )
			{
				shadow++;
				continue;
			}
			*shadow++ = value;

			word xpos = (word)(code>>8) & 0x01FF;
			word ypos = (byte)code;
			ST_DrawClearMaskedNarrow(xpos, ypos, screenbase, frames[value], bg);
		}
		else if( cmd==0x11 )
		{
			// draw frame
			const word **frames = (const word**)(*info++);
			word value = *(word*)(*info++);

			if( value == *shadow )
			{
				shadow++;
				continue;
			}
			*shadow++ = value;

			word xpos = (word)(code>>8) & 0x01FF;
			word ypos = (byte)code;
			ST_DrawOpaqueAligned(xpos, ypos, screenbase, frames[value]);
		}
		else
			break;
	}
}



byte ST_Game_MainLoop(void)
{
	static dword timer_value;
	byte exit_reason = 0;

	if( opt_msx_enable && music_track!=2 )
	{
		set_supervisor();
		init_music(2);
		set_usermode();
	}


	intnum = 0;
	io_key_last = 0;
	io_mouse_state = 0;
	e_globals.game_state = GAMESTATE_IN_GAME;

	asm_memclear(screen, 320*200*4/8);
	asm_memclear(screen_active, 320*200*4/8);
		
	vblank_pal = st_machine_type ? PALETTE_MAIN_STe : PALETTE_MAIN;
	vblank_hudpal = PALETTE_HUD;
	Statbar_Init(screen+80*(200-32), GFXINFO_ST_STBAR0, statbar_shadows[0]);
	Statbar_Init(screen_active+80*(200-32), GFXINFO_ST_STBAR0, statbar_shadows[1]);
	io_mouse_xdelta = 0;

#if MULTIPLAYER_MODE
	byte comm_data[4] = { 1,2,3,4 };
#endif

	byte stat_shadow__index = 0;
	while( !exit_reason )
	{
		render_buffer = (byte*)render_screen;

		Dread_WeaponLogic();

#if MULTIPLAYER_MODE
		Parse_MP_Packets();
#endif

		Dread_RunLogic();

#if MULTIPLAYER_MODE
		Comm_Write_Byte(0x01);
		Comm_Write_Word(view_pos_x);
		Comm_Write_Word(view_pos_y);
		Comm_Write_Byte(e_globals.mp_deaths);
		Comm_Packet_End();

		if( e_globals.mp_commstate == 1 )
		{
			Comm_Write_Byte(0x05);
			Comm_Packet_End();
		}

		Parse_MP_Packets();
#endif

		Dread_Render();

		perform_c2p();
		if( view_weapon && view_weapon->sprite )
			ST_DrawWeaponAsm(screen, view_weapon->sprite);
		if( opt_screen_mode==0 )
		{
			if( st_machine_type==1 )
			{
				set_supervisor();
				c2p_fix_blit(screen);
				set_usermode();
			}
			else
				c2p_fix(screen);
		}

		int perf = timer_value - frame_time[frame_time_pos];
		frame_time[frame_time_pos] = timer_value;
		frame_time_pos = (frame_time_pos+1) & 15;

		// 192*200 = 38400 Hz  -->  5000 Hz
		// 192 Hz  -->  25 Hz
		perf = (perf*25 + 192/2)/192/16;
		if( perf<0 ) perf = 0;
		if( perf>999 ) perf = 999;
		Debug_WritePerf(screen, perf);

		//debug_write_hexbuff(screen, 4, &timer_value);
		//debug_write_hexbuff(screen, 8, ikbd_rxbuff);
		//debug_write_hexbuff(screen+80*6, 4, (byte*)&debug_vars);
		//debug_write_hexbuff(screen+80*6*2, 4, (byte*)&debug_vars[1]);
		//debug_write_hexbuff(screen+80*6*3, 1, (byte*)&io_mouse_state);
		//debug_write_hexbuff(screen+80*7, 2, (byte*)&st_pal_mode);
		//debug_write_hexbuff(screen+80*7, 2, (byte*)&st_machine_type);
		//debug_write_hexbuff(screen+80*7*2, 2, (byte*)&st_cpu_type);
		//debug_write_hexbuff(screen+2*4, 8, (byte*)&e_globals.debug_var);
																							// 1:
//		debug_write_hexbuff(screen+12*1, 2, (byte*)&comm_rx_put);							//		read pos
//		debug_write_hexbuff(screen+12*2, 2, (byte*)&comm_rx_get);							//		parse pos
//		debug_write_hexbuff(screen+12*3, 2, (byte*)&comm_debug1);							//		debug1
//		debug_write_hexbuff(screen+12*4, 2, (byte*)&comm_debug2);							//		debug2
//		debug_write_hexbuff(screen+80*7, 8, (byte*)&comm_scan_buff);						// 2: scan buff
//		debug_write_hexbuff(screen+80*7*2, 16, (byte*)&comm_rx_buff);						// 3: RX buff
//		
//																							// 4:
//		debug_write_hexbuff(screen+80*7*3+12*0, 2, (byte*)&view_pos_x);						//		view pos X
//		debug_write_hexbuff(screen+80*7*3+12*1, 2, (byte*)&view_pos_y);						//		view pos Y
//		if( mp_avatar )
//		{
//			debug_write_hexbuff(screen+80*7*3+12*2, 2, (byte*)&mp_avatar->xp);				//		avatar pos X
//			debug_write_hexbuff(screen+80*7*3+12*3, 2, (byte*)&mp_avatar->yp);				//		avatar pos Y
//		}
//		debug_write_hexbuff(screen+80*7*4+12*0, 2, (byte*)&e_n_things);						// 4: things count
		
		debug_write_dec(screen+80-8, e_globals.mp_frags);
		debug_write_dec(screen+80-4, e_globals.mp_deaths);



		Statbar_Update(screen+80*(200-32), GFXINFO_ST_STBAR0, statbar_shadows[stat_shadow__index]);
		stat_shadow__index ^= 1;

		word *tmp = screen;
		screen = screen_active;
		screen_active = tmp;

		set_supervisor();
		//word sr = ST_CriticalBegin();
		VID_SET_SCREEN(screen_active);
		//ST_CriticalEnd(sr);
		timer_value = ST_GetPerfTimer();
		set_usermode();

		vblank_pal = (st_machine_type ? PALETTE_MAIN_STe : PALETTE_MAIN) + (palette_index<<4);
		vblank_hudpal = PALETTE_HUD + (palette_index<<4);

		debug_vars[1] += 1<<4;

		/**/ if( e_globals.game_state == GAMESTATE_LEVEL_DONE	) exit_reason = 1;
		else if( io_key_last==KEYSCAN_ESC						) exit_reason = 2;
		//else if( io_key_last==KEYSCAN_TAB						) exit_reason = 3;
	}

	if( exit_reason == 1 )
	{
		Sound_PlayThingSound(NULL, SOUND_DSLEVEND, 0);
	}
	else if( exit_reason == 2 )
	{
		Sound_PlayThingSound(NULL, SOUND_DSWEAP1, 0);
	}

#if !MULTIPLAYER_MODE
	if( e_globals.game_state==GAMESTATE_LEVEL_DONE || e_globals.player_health<=0 )
		e_n_things = 0;	// disable going back to the map
#endif

	io_key_last = 0;

	return exit_reason;
}




int main(int argc, char **argv)
{
//	byte cnt = 0;
//	while( 1 )
//	{
//		ST_Comm_Write_Byte(cnt++);
//	}

	// system setup
	set_supervisor();
	//while(1){}

	st_pal_mode = *(word*)0x000448;

	// screen setup
	static byte _vid_screen_high, _vid_screen_med, _vid_resolution, _vid_sync;
	_vid_screen_high = VID_SCREEN_HIGH;
	_vid_screen_med = VID_SCREEN_MED;
	_vid_resolution = VID_RESOLUTION;
	_vid_sync = VID_SYNC_MODE;
	screen = (word*)( (((int)_screen)|0xFF)+1 );
	screen_active = screen + (320*200*4/16);
	VID_SET_SCREEN(screen_active);
	VID_RESOLUTION = 0;
	VID_SYNC_MODE = 2;


	// engine setup
	init_sincos();
	init_c2p_lut();
	map_memory_buffers[0].ptr = map_data_buffer;
	map_memory_buffers[0].size = sizeof(map_data_buffer);

	Options_Reset();
	Dread_Build_Fillers();
	Dread_Build_Scalers();
	Dread_SoftInit();

	if( opt_msx_enable )
		init_music(1);

	vblank_pal = st_machine_type ? PALETTE_MAIN_STe : PALETTE_MAIN;
	vblank_hudpal = PALETTE_HUD;
	st_install_vblank();
#if MULTIPLAYER_MODE
	Comm_Init();
#endif
	set_usermode();

	// detect system
	st_machine_type = (word)(ST_GetCookie('_', 'M', 'C', 'H') >> 16);
	st_cpu_type = (word)ST_GetCookie('_','C','P','U');


	ikbd_install_handler();

	debug_vars[1] = 0;
	debug_vars[2] = 0;

	ikbd_qwrite(0x08000000, 1);			// mouse in relative mode
	ikbd_qwrite(0x0B040400, 3);			// set threshold to 4 pulses
	ikbd_qwrite(0x07000000, 2);			// send mouse buttons as flags


	// dread
	static byte state;
	static byte exit_reason;
	state = 0;
	//Map_LoadAll(2);
	while( state != 255 )
	{
		switch( state )
		{
		case 0:				// 0 - menu
			exit_reason = Menu_Run();
			if( exit_reason == 1 )	state = 255;
			else					state = 1;
			break;

		case 1:				// 1 - game
			exit_reason	= ST_Game_MainLoop();
			if( exit_reason == 3 )	state = 2;
			else					state = 0;
			break;

		case 2:				// 2 - minimap
			//Minimap_Run();
			state = 1;
			break;
		}
	}


	set_supervisor();
	init_music(0);
	st_remove_vblank();
	VID_SCREEN_HIGH = _vid_screen_high;
	VID_SCREEN_MED = _vid_screen_med;
	VID_RESOLUTION = _vid_resolution;
	VID_SYNC_MODE = _vid_sync;
	set_usermode();

	ikbd_remove_handler();


	return 0;
}

