
#include "demo.h"


/*const*/ word PAL[16] = {
	0x000,	// black			0000	OK
	0x222,	// gray ramp		0001	OK
	0x333,	// |
	0x555,	// |
	0x777,	// |
	0xBBB,	// |
	0xFFF,	// white
	0x343,	// green ramp
	0x573,	// |
	0x774,	// |
	0x004,	// blue				1010	OK
	0x320,	// brown
	0x431,	// brown
	0xCA5,	// yellow
	0xF00,	// 
	0xF0F,	// 
};


word *screen_A[4];
word *screen_B[4];
byte *render_A;
byte *render_B;
word *temp_X;
word *temp_Y;
word *copper_A;
word *copper_B;
byte *render_buffer;




extern const WeaponFrameInfo WEAPON_INFO[];


// (0xCC & 0xF0) | (0xAA & ~0xF0)	=	0xCA
const word BLIT2D_CON0[] ={ 0x0FCA, 0x2FCA, 0x4FCA, 0x6FCA, 0x8FCA, 0xAFCA, 0xCFCA, 0xEFCA };
const word BLIT2D_ALWM[] ={ 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };

void Dread_BlitWeapon(__d0 word plane)
{
	static byte anim;
	static byte frame;
	if( plane==0 )
	{
		// Animation logic
		if( weapon_frame )
		{
			mouse_click = 0;
			if( WEAPON_INFO[weapon_frame+1].data )
				weapon_frame++;
			cam_weapon_swing = 0;
		}
		else if( mouse_click || !(CIAA_PRA & (1<<6)) )
		{
			mouse_click = 0;
		
			if( weapon_frame == 0 )
			{
				weapon_frame++;
				weapon_reload = 0;
				cam_weapon_swing = 0;
			}
		}

		anim = (byte)cam_weapon_swing;
		frame = (byte)weapon_frame;
	}

	WeaponFrameInfo *weap = WEAPON_INFO + frame;

	byte *ptr_base = (byte*)((dword)BlockAddr_Work << 16);
	word ox = weap->off_x + (sincos_fix14[(anim+64)&0xFF]>>12);				// cos(x+1/8)		x=32
	word oy = weap->off_y + 16 + 4 + (sincos_fix14[(byte)((anim<<1)+64)]>>12);		// cos(2x)			x=32	x=96	x=160	x=224
	word w = (weap->width+7)>>3;
	if(ox&7) w++;

	BLTCON0 = BLIT2D_CON0[ox&7];
	BLTCON1 = BLIT2D_CON0[ox&7] & 0xF000;
	BLTAFWM = 0xFFFF;
	BLTALWM = BLIT2D_ALWM[ox&7];
	BLTAMOD = ox&7 ? -2 : 0;
	BLTBMOD = ox&7 ? -2 : 0;
	BLTCMOD = 42*2 - (w<<1);
	BLTDMOD = 42*2 - (w<<1);
	BLTAPT = weap->data;
	BLTBPT = (byte*)weap->data + (weap->plane_stride*(plane+1));

	//	screen_A0:	equ	2
	//	screen_A1 : equ	2+8400
	//	screen_A2 : equ	2+8400*2
	//	screen_A3 : equ	2+8400*3
	ptr_base += 2 + 8400*plane + (42*2)*oy + ((ox>>2)&~1);
	BLTCPT = ptr_base;
	BLTDPT = ptr_base;

	word h = weap->height;
	if( h+oy > 100 )
		h = 100 - oy;

	BLTSIZE = COMPUTE_BLTSIZE(w, h);
}




void Dread_SetupCopper(word *copper, word *s0, word *s1, word *s2, word *s3)
{
	BPL1MOD = 2;
	BPL2MOD = 2;

	cop_write = copper;
	Cop_StartBit4(s0, s1, s2, s3);
	//Cop_Palette(PAL, 16);
	Cop_Palette_NZ(PAL+1, 16-1);
	Cop_TickSprite();

	Cop_Wait();
	//*cop_write++ = 0x096;	*cop_write++ = 0x0040;									//	Blitter OFF

	cop_write_y += 200;
	Cop_Wait();
	Cop_SetBit0();
	//*cop_write++ = 0x096;	*cop_write++ = 0x8040;									//	Blitter ON
	Cop_End();

	//static const word BLACK = 0x0000;
	//Cop_WaitFor(200);
	//Cop_SetBit0();
	//Cop_Palette((word*)&BLACK, 1);
	//Cop_End();
}



void FX_DreadTest(void)
{
	COLOR00 = 0x000;
	COLOR01 = 0xFFF;
	COLOR17 = 0x000;
	COLOR18 = 0x000;
	COLOR19 = 0xFFF;
	BPLCON1 = 0x0000;
	COPCON = 2;
	INTENA = 0x8040;	// Enable blitter interrupts

	SetCopper(BLANK_COPPER);

	// Init memory pointers
	screen_A[0]	= (word*)(blockmem1+2);
	screen_A[1]	= (word*)(blockmem1+2+8400);
	screen_A[2]	= (word*)(blockmem1+2+8400*2);
	screen_A[3]	= (word*)(blockmem1+2+8400*3);
	render_A	= (byte*)(blockmem1+2+8400*4);
	temp_X		= (word*)(blockmem1+2+8400*4+16000+2);
	copper_A	= (word*)(blockmem1+2+8400*4+16000+2+8000);

	screen_B[0]	= (word*)(blockmem2+2);
	screen_B[1]	= (word*)(blockmem2+2+8400);
	screen_B[2]	= (word*)(blockmem2+2+8400*2);
	screen_B[3]	= (word*)(blockmem2+2+8400*3);
	render_B	= (byte*)(blockmem2+2+8400*4);
	temp_Y		= (word*)(blockmem2+2+8400*4+16000+2);
	copper_B	= (word*)(blockmem2+2+8400*4+16000+2+8000);

	Dread_Build_ColBlits();
	Dread_Build_Scalers();
	Dread_Build_Fillers();
	Dread_SoftInit();


	Dread_SetupCopper(copper_A, screen_A[0], screen_A[1], screen_A[2], screen_A[3]);
	Dread_SetupCopper(copper_B, screen_B[0], screen_B[1], screen_B[2], screen_B[3]);
	//SetCopper(copper_A);
	DMACON = 0x0000 | (1<< 7);	// disable copper
	COP1LC = copper_A;
	DMACON = 0x8000 | (1<< 7);	// enable copper

	BlockAddr_Work = (word)(((dword)blockmem1)>>16);
	BlockAddr_Temp = (word)(((dword)blockmem2)>>16);
	C2P_Scene_Queue[1] = (word)(((dword)blockmem1)>>16);
	C2P_Scene_Queue[0] = (word)(((dword)blockmem2)>>16);

	intnum = 0;

	word xanim = 0;
	while( 1 )
	{
		//word slow = rawintnum;
		render_buffer = C2P_BeginScene();

		Dread_SoftTest();

		C2P_EndScene();

		//while( rawintnum-slow<5 );

		MarkFrame();
	}
}
