
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
	0x000,	// 
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



word COLBLIT_BUFFER[32000];	// overestimated
colblit_fn_t COLBLITS[101];



word *Dread_Build_ColBlit_One(word *ptr, int size)
{
	const short lrc_cam_pos_z = 40;
	short line_h1 = 64 - lrc_cam_pos_z;
	short line_h2 = 0 - lrc_cam_pos_z;

	int s1 = size << 8;
	int y1 = 50 - ((s1*line_h1)>>13);
	int y2 = 50 - ((s1*line_h2)>>13);
	if( y1<=0 ) y1 = 0;
	if( y2>=100 ) y2 = 100;

	//typedef void(*colblit_fn_t)(__a0 byte *dst, __a2 const byte *tex, __d0 byte ceil, __d1 byte floor);
	//typedef void(*colblit_fn_t)(__a0 byte *dst, __a2 const byte *tex, __a1 byte *sky, __d1 byte floor);
	for( int y=0; y<100; y++ )
	{
		int ty, tyy;
		if( size>=20 ) ty = ((y-50)*32/size - (lrc_cam_pos_z-64)) + 32, tyy = (ty-32)>>1;
		else if( size>=1 ) tyy = ty = ((y-50)*16/size - (lrc_cam_pos_z-64)/2);
		else tyy = (y<50) ? -1 : 32;
		if( tyy<0 )
		{
			//// ceil
			//*ptr++ = 0x1140;		// 0001 000 101 000 000			MOVE.b	D0, yyy(A0)							MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
			//						// 0001 0001 0100 0000
			//*ptr++ = 40*y;

			// sky
			*ptr++ = 0x1159;		// 0001 000 101 011 001			MOVE.b	(A1)+, yyy(A0)						MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
									// 0001 0001 0101 1001
			*ptr++ = 40*y;
		}
		else if( tyy>=32 )
		{
			// floor
			*ptr++ = 0x1141;		// 0001 000 101 000 000			MOVE.b	D1, yyy(A0)							MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
									// 0001 0001 0100 0000
			*ptr++ = 40*y;
		}
		else
		{
			// texture
			*ptr++ = 0x116A;		// 0001 000 101 101 010			MOVE.b	xxx(A2), yyy(A0)					MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
									// 0001 0001 0110 1010
			*ptr++ = ty<<6;
			*ptr++ = 40*y;
		}
	}

	*ptr++ = 0x4E75;

	return ptr;
}

void Dread_Build_ColBlits(void)
{
	word *ptr = COLBLIT_BUFFER;
	for( int size=0; size<=100; size++ )
	{
		if( size<=50 || (size&1)==0 )
		{
			COLBLITS[size] = (colblit_fn_t)ptr;
			ptr = Dread_Build_ColBlit_One(ptr, size);
		}
		else
			COLBLITS[size] = COLBLITS[size-1];
	}
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
		render_buffer = C2P_BeginScene();

		Dread_SoftTest();

		C2P_EndScene();

		MarkFrame();
	}
}
