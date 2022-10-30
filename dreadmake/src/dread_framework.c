
#include "demo.h"


#if AMIGA_AUDIO_ENGINE_VERSION == 2
extern const AudioTrackRow TRKSOUND_DSRLAUNC[];
extern const AudioTrackRow TRKSOUND_DSWPNUP[];
extern const AudioTrackRow TRKSOUND_DSWEAP1[];
extern const AudioTrackRow TRKSOUND_DSAMMO[];
extern const AudioTrackRow TRKSOUND_DSSELECT[];
extern const AudioTrackRow TRKSOUND_DSLEVEND[];
#endif




// (0xCC & 0xF0) | (0xAA & ~0xF0)	=	0xCA
const word BLIT2D_CON0[] ={ 0x0FCA, 0x2FCA, 0x4FCA, 0x6FCA, 0x8FCA, 0xAFCA, 0xCFCA, 0xEFCA };
const word BLIT2D_ALWM[] ={ 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };



#if !WEAPONSPRITES && AMIGA
void Dread_BlitWeapon(__d0 word plane)
{
	static byte anim;

	if( plane==0 )
	{
		IO_CheckMouse();			//|| !(CIAA_PRA & (1<<6)) )
		Dread_WeaponLogic();
		
		anim = (byte)view_weapon_swing;
	}


	const WeaponFrameInfoX *weap = (const WeaponFrameInfoX *)view_weapon->sprite;
	byte *ptr_base = (byte*)((dword)BlockAddr_Work << 16);
	word ox = weap->off_x + view_weapon->xp + (sincos_fix14[(anim+64)&0xFF]>>12);			// cos(x+1/8)		x=32
	word oy = weap->off_y + view_weapon->yp + (sincos_fix14[(byte)((anim<<1)+64)]>>12);		// cos(2x)			x=32	x=96	x=160	x=224
	word w = (weap->width+7)>>3;
	if( ox&7 ) w++;

	BLTCON0 = BLIT2D_CON0[ox&7];
	BLTCON1 = BLIT2D_CON0[ox&7] & 0xF000;
	BLTAFWM = 0xFFFF;
	BLTALWM = BLIT2D_ALWM[ox&7];
	BLTAMOD = ox&7 ? -2 : 0;
	BLTBMOD = ox&7 ? -2 : 0;
	BLTCMOD = 42*2 - (w<<1);
	BLTDMOD = 42*2 - (w<<1);
	BLTAPT = weap->mask;
	BLTBPT = ((word**)&weap->plane0)[plane];

	//	screen_A0:	equ	2
	//	screen_A1 : equ	2+8400
	//	screen_A2 : equ	2+8400*2
	//	screen_A3 : equ	2+8400*3
	ptr_base += 2 + 8400*plane + (42*2)*oy + ((ox>>2)&~1);
	BLTCPT = ptr_base;
	BLTDPT = ptr_base;

	word h = weap->height;
	if( h+oy > ENGINE_Y_MAX )
		h = ENGINE_Y_MAX - oy;

	BLTSIZE = COMPUTE_BLTSIZE(w, h);
}
#endif




byte Game_MainLoop(void)
{
	byte exit_reason = 0;

	VScreen_Init();

	intnum = 0;
	io_key_last = 0;
	io_mouse_state = 0;
	e_globals.game_state = GAMESTATE_IN_GAME;
	
	while( !exit_reason )
	{
		VScreen_BeginFrame();

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

		VScreen_EndFrame();

		/**/ if( e_globals.game_state == GAMESTATE_LEVEL_DONE	) exit_reason = 1;
		else if( io_key_last==KEYSCAN_ESC						) exit_reason = 2;
		//else if( io_key_last==KEYSCAN_TAB						) exit_reason = 3;
	}

	VScreen_Shutdown();
	Aud_Reset();

	if( exit_reason == 1 )
	{
		Sound_PlayThingSound(NULL, TRKSOUND_DSLEVEND, 0);
	}
	else if( exit_reason == 2 )
	{
		Sound_PlayThingSound(NULL, TRKSOUND_DSWEAP1, 0);
	}

#if !MULTIPLAYER_MODE
	if( e_globals.game_state==GAMESTATE_LEVEL_DONE || e_globals.player_health<=0 )
		e_n_things = 0;	// disable going back to the map
#endif

	io_key_last = 0;

	return exit_reason;
}


void FX_DreadTest(void)
{
	Options_Reset();
	Dread_Build_Fillers();
	Dread_Build_Scalers();
	Dread_SoftInit();

	byte state = 0;
	byte exit_reason;
	//Map_LoadAll(2);
	while( state != 255 )
	{
		switch( state )
		{
		case 0:				// 0 - menu
			if( Menu_Run() )
				state = 255;
			else
				state = 1;
			break;

		case 1:				// 1 - game
			exit_reason	= Game_MainLoop();
			if( exit_reason == 3 ) state = 2;
			else state = 0;
			break;

		case 2:				// 2 - minimap
			Minimap_Run();
			state = 1;
			break;
		}
	}

}
