
#include "demo.h"



word *screen_A[5];		// 5th bitplane used only in menu
word *screen_B[5];		// 5th bitplane used only in menu
byte *render_A;
byte *render_B;
word *temp_X;
word *temp_Y;
word *statbar_buffer;
word *copper_A;
word *copper_B;
word *copper_common;
#if WEAPONSPRITES
word *copper_sprite_update;
word *copper_sprite_update2;
#endif
byte *render_buffer;
word *render_copper;
word palette_index;



extern __chip word GFX_STBAR0[];
extern const dword GFXINFO_STBAR0[];
//extern const word PALETTE_MAIN[];
extern const word PALETTE_HUD[];









void Dread_SetupCopper(word *copper, word *s0, word *s1, word *s2, word *s3, word copnum)
{
	BPL1MOD = 2;
	BPL2MOD = 2;

	copper[-1] = 0xFFFF;

	cop_write = copper;
	Cop_Palette(e_map_palette, 16);
	Cop_StartBit4(s0, s1, s2, s3);
	if( opt_ntsc )
		cop_write_y = 44-17;


	if( copnum==0 )
		copper_common = cop_write;
	else
	{
		Cop_Write(COPJMP2, 0);
		debug_vars[4] = (byte*)cop_write - (byte*)copper;		// size of copper #1		TBD: remove
		return;
	}

	if( opt_screen_mode==SCREEN_MODE_SCANLINES )
	{
		Cop_Write(BPL1MOD, 44);
		Cop_Write(BPL2MOD, 44);
	}
	else if( opt_screen_mode==SCREEN_MODE_SHAKE )
	{
		Cop_Write(BPLCON1, 0x0011);
	}
	else if( opt_screen_mode==SCREEN_MODE_CHECKER )
	{
		Cop_Write(BPL1MOD, 2);
		Cop_Write(BPL2MOD, 2);
	}

	Cop_Write(DMACON, 0x8020);
	Cop_Write(COLOR17, 0x000);
	Cop_Write(COLOR18, 0x000);
	Cop_Write(COLOR19, 0xFFF);
	Cop_TickSprite();

	//Cop_Wait();
	//*cop_write++ = 0x096;	*cop_write++ = 0x0040;									//	Blitter OFF

	for( int y=0; y<ENGINE_Y_MAX; y++ )
	{
		if( opt_screen_mode==SCREEN_MODE_SCANLINES )
		{
			cop_write_y++;
			Cop_Wait();

			*cop_write++ = 0x100;	*cop_write++ = 0x0200;		// 0 bitplanes
			Cop_Write(BPL1DAT, 0x0000);

			cop_write_y++;
			Cop_Wait();
			*cop_write++ = 0x100;	*cop_write++ = 0x4200;		// 4 bitplanes
		}
		else if( opt_screen_mode==SCREEN_MODE_SHAKE )
		{
			// default
			Cop_Write(BPL1MOD, -40);
			Cop_Write(BPL2MOD, -40);
			cop_write_y++;
			Cop_Wait();
			Cop_Write(BPLCON1, 0x0000);

			Cop_Write(BPL1MOD, 44);
			Cop_Write(BPL2MOD, 44);
			cop_write_y++;
			Cop_Wait();
			Cop_Write(BPLCON1, 0x0011);
		}
		else if( opt_screen_mode==SCREEN_MODE_CHECKER )
		{
			cop_write_y++;
			cop_write_y++;
			if( y==2 || y==3 )
				Cop_Wait();
		}
		else
		{
			// default
			Cop_Write(BPL1MOD, -40);
			Cop_Write(BPL2MOD, -40);
			cop_write_y++;
			Cop_Wait();

			Cop_Write(BPL1MOD, 44);
			Cop_Write(BPL2MOD, 44);
			cop_write_y++;
			Cop_Wait();
		}

#if WEAPONSPRITES
		//if( y==2 )
		//{
		//	copper_sprite_update = cop_write + 1;
		//	//debug_vars[5] = (byte*)cop_write - (byte*)copper;		// offset of sprite settings (currently +260)				TBD: remove
		//
		//	for( int i=0; i<15; i++ )
		//		Cop_Write(COLOR[17+i], 0x000);
		//
		//	for( int i=0; i<8; i++ )
		//	{
		//		const word *ptr = NULL_SPRITE;
		//		Cop_Write((&SPR0PTL)[i<<1], (dword)ptr);
		//		Cop_Write((&SPR0PTH)[i<<1], ((dword)ptr)>>16);
		//		Cop_Write(SPR[i].POS, 0x0000);
		//		Cop_Write(SPR[i].CTL, 0xFF00);
		//	}
		//}

		if( y==2 )
		{
			copper_sprite_update = cop_write + 1;
			for( int i=0; i<15; i++ )
				Cop_Write(COLOR[17+i], 0x000);

			for( int i=0; i<2; i++ )
			{
				const word *ptr = NULL_SPRITE;
				Cop_Write((&SPR0PTL)[i<<1], (dword)ptr);
				Cop_Write((&SPR0PTH)[i<<1], ((dword)ptr)>>16);
				Cop_Write(SPR[i].POS, 0x0000);
				Cop_Write(SPR[i].CTL, 0xFF00);
			}
		}

		if( y==3 )
		{
			copper_sprite_update2 = cop_write + 1;

			for( int i=2; i<8; i++ )
			{
				const word *ptr = NULL_SPRITE;
				Cop_Write((&SPR0PTL)[i<<1], (dword)ptr);
				Cop_Write((&SPR0PTH)[i<<1], ((dword)ptr)>>16);
				Cop_Write(SPR[i].POS, 0x0000);
				Cop_Write(SPR[i].CTL, 0xFF00);
			}
		}
#endif
	}
	if( opt_screen_mode==SCREEN_MODE_CHECKER )
		Cop_Wait();
	Cop_SetBit0();
	Cop_Write(DMACON, 0x0020);
	if( opt_screen_mode==SCREEN_MODE_SHAKE )
		Cop_Write(BPLCON1, 0x0000);
	for( int i=0; i<8; i++ )
	{
		Cop_Write(SPR[i].DATA, 0x0000);
		Cop_Write(SPR[i].DATB, 0x0000);
	}

#if DEBUG_CONSOLE
	word color = 0xFFFF;
	Cop_Palette_NZ(&color, 1);
	Cop_Write(BPL1MOD, 0);
	cop_write_y++;
	Cop_Wait();
	Cop_SetBit1(DEBUG_CONSOLE_SCREEN);
	cop_write_y += 32;
	Cop_Wait256();
	Cop_SetBit0();
#else
	word *bar = statbar_buffer;
	Cop_Palette_NZ((word*)PALETTE_HUD+1, 16-1);
	Cop_Write(BPL1MOD, 60*2);
	Cop_Write(BPL2MOD, 60*2);

	cop_write_y++;
	Cop_Wait();
	Cop_SetBit4(bar, bar+20, bar+40, bar+60);
	cop_write_y += 32;
	Cop_Wait256();
	Cop_SetBit0();
#endif

	//*cop_write++ = 0x096;	*cop_write++ = 0x8040;									//	Blitter ON
	Cop_End();

	//static const word BLACK = 0x0000;
	//Cop_WaitFor(200);
	//Cop_SetBit0();
	//Cop_Palette((word*)&BLACK, 1);
	//Cop_End();

	debug_vars[3] = (byte*)cop_write - (byte*)copper;		// size of copper #0		TBD: remove
}





void VScreen_Init(void)
{
	//word sr = Critical_Begin();		// Make sure nothing triggers inbetween.
	INTENA = 0x0060;	// Disable blitter & vsync interrupts
	BPLCON1 = 0x0000;
	COPCON = 2;
	DMACON = 0x0000 | (1<<10);	// disable blitter nasty

	SetCopper(BLANK_COPPER);

	memset(blockmem1, 0, 0x10000);
	memset(blockmem2, 0, 0x10000);

	// Init memory pointers
	screen_A[0]	= (word*)(blockmem1+2);
	screen_A[1]	= (word*)(blockmem1+2+8400);
	screen_A[2]	= (word*)(blockmem1+2+8400*2);
	screen_A[3]	= (word*)(blockmem1+2+8400*3);
	render_A	= (byte*)(blockmem1+2+8400*4);
	temp_X		= (word*)(blockmem1+2+8400*4+16160+40);
	copper_A	= (word*)(blockmem1+2+8400*4+16160+40+8040+4);

	screen_B[0]	= (word*)(blockmem2+2);
	screen_B[1]	= (word*)(blockmem2+2+8400);
	screen_B[2]	= (word*)(blockmem2+2+8400*2);
	screen_B[3]	= (word*)(blockmem2+2+8400*3);
	render_B	= (byte*)(blockmem2+2+8400*4);
	temp_Y		= (word*)(blockmem2+2+8400*4+16160+40);
	copper_B	= (word*)(blockmem2+2+8400*4+16160+40+8040+4);

	statbar_buffer	= copper_B + 512/2;

	Statbar_Init(statbar_buffer, GFXINFO_STBAR0);


	Dread_SetupCopper(copper_A, screen_A[0], screen_A[1], screen_A[2], screen_A[3], 0);
	Dread_SetupCopper(copper_B, screen_B[0], screen_B[1], screen_B[2], screen_B[3], 1);
	DMACON = 0x0000 | (1<< 7);	// disable copper
	COP1LC = copper_A;
	COP2LC = copper_common;
	DMACON = 0x8000 | (1<< 7);	// enable copper

	BlockAddr_Work = (word)(((dword)blockmem1)>>16);
	BlockAddr_Temp = (word)(((dword)blockmem2)>>16);
	C2P_Scene_Queue[1] = (word)(((dword)blockmem1)>>16);
	C2P_Scene_Queue[0] = (word)(((dword)blockmem2)>>16);

//	COLOR00 = 0x000;
//	COLOR01 = 0xFFF;
//	COLOR17 = 0x000;
//	COLOR18 = 0x000;
//	COLOR19 = 0xFFF;

	io_mouse_xdelta = 0;
	INTENA = 0x8060;	// Enable blitter & vsync interrupts
	//Critical_End(sr);		// Done, resume interrupts.
}

void VScreen_Shutdown(void)
{
#if WEAPONSPRITES
	copper_sprite_update = NULL;		// TBD: disable sprite update properly
	copper_sprite_update2 = NULL;
#endif

	C2P_WaitAll();

	DMACON = 0x0000 | (1<< 7);	// disable copper
	DMACON = 0x0000 | (1<< 5);	// disable sprites
	Init_Copper();
	SetCopper(BLANK_COPPER);
	DMACON = 0x8000 | (1<< 7);	// enable copper
	COPJMP1 = 0;

	for( int i=0; i<8; i++ )
	{
		SPR[i].DATA = 0;
		SPR[i].DATB = 0;
	}
	
	//intnum = 0;
	//while( intnum<2 ) {}
}


void VScreen_BeginFrame(void)
{
	render_buffer = C2P_BeginScene();
}

void VScreen_EndFrame(void)
{
	C2P_EndScene();

	//while( rawintnum-slow<5 );

	MarkFrame();
}
