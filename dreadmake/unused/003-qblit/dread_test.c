
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



// Input:
//	A0	- target column
//	A1	- texture source
//
typedef void(*qblit_fn_t)(__a0 byte *dst, __a1 const byte *tex);

//word TEST_CODE[] = {
//	0x303C,		// 0011 0000 00 11 1100		MOVE.w	#42, D0					MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
//	42,
//	0x4E75,		// 0100 1110 0111 0101		RTS
//};

word QBLIT_FN[100*6+1];		// MOVE.B	xxx(A1), yyy(A0)			x100
							// RTS

word COLBLIT_BUFFER[32000];	// overestimated
colblit_fn_t COLBLITS[101];


void Dread_Build_QBlit(void)
{
	word *dst = QBLIT_FN;
	word ypos = 0;
	word ystep = 456;

	for( int y=0; y<100; y++ )
	{
		*dst++ = 0x1169;		// 0001 000 101 101 001			MOVE.b	xxx(A1), yyy(A0)					MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
								// 0001 0001 0110 1001
		*dst++ = ((ypos>>8)&31)<<5;
		*dst++ = 40*y;

		ypos += ystep;
	}
	*dst++ = 0x4E75;
}

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


	// copper blit 1
#define _A	0xF0
#define _B	0xCC
#define _C	0xAA
//	*cop_write++ = 0x0001;	*cop_write++ = 0x0000;									//	BlitWait();
//	*cop_write++ = 0x070;	*cop_write++ = 0x5555;									//	BLTCDAT = 0x5555;								// data
//	*cop_write++ = 0x040;	*cop_write++ = 0x1D00 | (_A&_C | _B&~_C);				//	BLTCON0 = 0x1D00 | (_A&_C | _B&~_C);			// BLTCONx
//	*cop_write++ = 0x042;	*cop_write++ = 0xF000;									//	BLTCON1 = 0xF000;
//	*cop_write++ = 0x044;	*cop_write++ = 0xFFFF;									//	BLTAWM = 0xFFFFFFFF;							// mask
//	*cop_write++ = 0x046;	*cop_write++ = 0xFFFF;
//	*cop_write++ = 0x064;	*cop_write++ = 320-80-2;								//	BLTAMOD = 320-80;								// modulos
//	*cop_write++ = 0x062;	*cop_write++ = 320-80-2;								//	BLTBMOD = 320-80;
//	*cop_write++ = 0x066;	*cop_write++ = 320-80-2;								//	BLTDMOD = 320-80;
//	*cop_write++ = 0x050;	*cop_write++ = (word)((dword)screen_planes >> 16);		//	BLTAPT = screen_planes;							// pointers
//	*cop_write++ = 0x052;	*cop_write++ = (word)(dword)screen_planes - 2;
//	*cop_write++ = 0x04C;	*cop_write++ = (word)((dword)screen_planes >> 16);		//	BLTBPT = screen_planes + 1;
//	*cop_write++ = 0x04E;	*cop_write++ = (word)(dword)(screen_planes + 1) - 2;
//	*cop_write++ = 0x054;	*cop_write++ = (word)((dword)screen_planes >> 16);		//	BLTDPT = screen_planes + 80;
//	*cop_write++ = 0x056;	*cop_write++ = (word)(dword)(screen_planes + 80) - 2;
//	*cop_write++ = 0x058;	*cop_write++ = COMPUTE_BLTSIZE(41, 100);				//	BLTSIZE = COMPUTE_BLTSIZE(40, 100);				// start
//	*cop_write++ = 0x180;	*cop_write++ = 0x040;									//	[GREEN]
//
//	// copper blit 2
//	*cop_write++ = 0x0001;	*cop_write++ = 0x0000;								//	BlitWait();
//	*cop_write++ = 0x052;	*cop_write++ = (word)(screen_planes + 40) - 2;		//	BLTAPTL = screen_planes + 40;					// pointers
//	*cop_write++ = 0x04E;	*cop_write++ = (word)(screen_planes + 40 + 1) - 2;	//	BLTBPTL = screen_planes + 40 + 1;
//	*cop_write++ = 0x056;	*cop_write++ = (word)(screen_planes + 120) - 2;		//	BLTDPTL = screen_planes + 120;
//	*cop_write++ = 0x058;	*cop_write++ = COMPUTE_BLTSIZE(41, 100);			//	BLTSIZE = COMPUTE_BLTSIZE(40, 100);				// start
//	*cop_write++ = 0x180;	*cop_write++ = 0x400;								//	[RED]
//	*cop_write++ = 0x0001;	*cop_write++ = 0x0000;								//	BlitWait();
//	*cop_write++ = 0x180;	*cop_write++ = 0x000;								//	[BLACK]


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

	Dread_Build_QBlit();
	Dread_Build_ColBlits();
	Dread_SoftInit();


	Dread_SetupCopper(copper_A, screen_A[0], screen_A[1], screen_A[2], screen_A[3]);
	Dread_SetupCopper(copper_B, screen_B[0], screen_B[1], screen_B[2], screen_B[3]);
	//SetCopper(copper_A);
	DMACON = 0x0000 | (1<< 7);	// disable copper
	COP1LC = copper_A;
	DMACON = 0x8000 | (1<< 7);	// enable copper

	//SetSR(0x0000);		// 0010 0III 000XNZVC

	BlockAddr_Work = (word)(((dword)blockmem1)>>16);
	BlockAddr_Temp = (word)(((dword)blockmem2)>>16);
	C2P_Scene_Queue[1] = (word)(((dword)blockmem1)>>16);
	C2P_Scene_Queue[0] = (word)(((dword)blockmem2)>>16);

	intnum = 0;

	word xanim = 0;
	while( 1 )
	{
		//const byte *src = TEST_FRAME;
		//for( int x=0; x<160; x++ )
		//{
		//	byte *dst = render_A + (x>>2) + (x&3)*4000;
		//	for( int y=0; y<100; y++ )
		//	{
		//		*dst = *src++;
		//		dst += 40;
		//	}
		//}
		
		render_buffer = C2P_BeginScene();

		//xanim = (-debug_vars[0])>>4;
		//for( int x=0; x<160; x++ )
		//{
		//	const byte *src = TEXTURE_DATA + (((x+xanim)>>1)&31);
		//	byte *dst = render_buffer + (x>>2) + (x&3)*4000;
		//	//for( int y=0; y<100; y++ )
		//	//{
		//	//	*dst = src[(y&31)<<5];
		//	//	dst += 40;
		//	//}
		//
		//	//((qblit_fn_t)QBLIT_FN)(dst, src);
		//	//((colblit_fn_t)QBLIT_FN)(dst, src, 0x40, 0xC0);
		//	COLBLITS[x>>1](dst, src, 0x40, 0xC0);		// 01000000	11000000
		//}

		//// Test sky
		//for( int x=0; x<160; x++ )
		//{
		//	const byte *src = SKYTEX_DATA + ((x&127)<<6);
		//	byte *dst = render_buffer + (x>>2) + (x&3)*4000;
		//	//for( int y=0; y<100; y++ )
		//	//{
		//	//	*dst = src[(y&31)<<5];
		//	//	dst += 40;
		//	//}
		//
		//	for( int y=0; y<64; y++ )
		//		dst[y*40] = *src++;
		//}

		Dread_SoftTest();


		//INTENA = 0xC000;	// Enable blitter interrupts
		//Blit_C2P_Start();
		//INTENA = 0x4000;	// Disable blitter interrupts
		C2P_EndScene();


#if 0
#define _A	0xF0
#define _B	0xCC
#define _C	0xAA
		BlitWait();										// ======== Pass 1a
		//BLTCDAT = 0xCCCC;								// data
		//BLTCON0 = 0xEDE4;//00 | (_A&_C | _B&~_C);			// BLTCONx
		//BLTCON1 = 0x0000;
		//BLTAWM = 0xFFFFFFFF;							// mask
		//BLTAMOD = 0;									// modulos
		//BLTBMOD = 0;
		//BLTDMOD = 0;
		//BLTAPT = render_A;								// pointers
		//BLTBPT = render_A+4000-2;
		//BLTDPT = temp_X+2000-1;
		//BLTSIZE = COMPUTE_BLTSIZE(23, 87);				// start
		Blit_C2P_Pass_1a((void*)0xdff000);

		BlitWait();										// ======== Pass 1b
		//BLTAPT = render_A+8000;							// pointers
		//BLTBPT = render_A+12000-2;
		//BLTDPT = temp_Y+2000-1;
		//BLTSIZE = COMPUTE_BLTSIZE(23, 87);				// start
		Blit_C2P_Pass_1b((void*)0xdff000);

		BlitWait();										// ======== Pass 1c
		//BLTCON0 = 0x0D00 | (_A&_C | _B&~_C);			// BLTCONx
		//BLTCON1 = 0x2000;
		//BLTAPT = render_A;								// pointers
		//BLTBPT = render_A+4000;
		//BLTDPT = temp_X;
		//BLTSIZE = COMPUTE_BLTSIZE(20, 100);				// start
		Blit_C2P_Pass_1c((void*)0xdff000);

		BlitWait();										// ======== Pass 1d
		//BLTAPT = render_A+8000;							// pointers
		//BLTBPT = render_A+12000;
		//BLTDPT = temp_Y;
		//BLTSIZE = COMPUTE_BLTSIZE(20, 100);				// start
		Blit_C2P_Pass_1d((void*)0xdff000);

		BlitWait();										// ======== Pass 2a
		//BLTCDAT = 0xF0F0;								// data
		//BLTCON0 = 0x0D00 | (_A&_C | _B&~_C);			// BLTCONx
		//BLTCON1 = 0x4000;
		//BLTDMOD = 44;									// modulos
		//BLTAPT = temp_X;								// pointers
		//BLTBPT = temp_Y;
		//BLTDPT = screen_A[0];
		//BLTSIZE = COMPUTE_BLTSIZE(20, 200);				// start
		Blit_C2P_Pass_2a((void*)0xdff000);

		BlitWait();										// ======== Pass 2b
		//BLTCON0 = 0xCD00 | (_A&_C | _B&~_C);			// BLTCONx
		//BLTCON1 = 0x0000;
		//BLTAMOD = -2;									// modulos
		//BLTBMOD = -2;
		//BLTDMOD = 42;
		//BLTAPT = temp_X;								// pointers
		//BLTBPT = temp_Y-1;
		//BLTDPT = screen_A[2]-1;
		//BLTSIZE = COMPUTE_BLTSIZE(21, 200);				// start
		Blit_C2P_Pass_2b((void*)0xdff000);

		BlitWait();										// ======== HQ dither
		//BLTCDAT = 0x5555;								// data
		//BLTCON0 = 0x1D00 | (_A&_C | _B&~_C);			// BLTCONx
		//BLTCON1 = 0xF000;
		//BLTAMOD = 42;									// modulos
		//BLTBMOD = 42;
		//BLTDMOD = 42;
		//BLTAPT = screen_A[0]-1;							// pointers
		//BLTBPT = screen_A[0];
		//BLTDPT = screen_A[0]+21-1;
		//BLTSIZE = COMPUTE_BLTSIZE(21, 400);				// start
		Blit_C2P_Pass_hqd((void*)0xdff000);
#endif

		MarkFrame();
	}
}
