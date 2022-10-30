
#include "demo.h"

#include "data/frame.inc"

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




void Dread_SetupCopper(word *copper)
{
	BPL1MOD = 2;
	BPL2MOD = 2;

	cop_write = copper;
	Cop_StartBit4(screen_A[0], screen_A[1], screen_A[2], screen_A[3]);
	Cop_Palette(PAL, 16);

	//Cop_Wait();

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
	BPLCON1 = 0x0000;
	COPCON = 2;

	// Playback the scene
	SetCopper(BLANK_COPPER);

	screen_A[0]	= (word*)(blockmem1+2);
	screen_A[1]	= (word*)(blockmem1+2+8400);
	screen_A[2]	= (word*)(blockmem1+2+8400*2);
	screen_A[3]	= (word*)(blockmem1+2+8400*3);
	render_A	= (byte*)(blockmem1+2+8400*4);
	temp_X		= (word*)(blockmem1+2+8400*4+16000+2);

	screen_B[0]	= (word*)(blockmem2+2);
	screen_B[1]	= (word*)(blockmem2+2+8400);
	screen_B[2]	= (word*)(blockmem2+2+8400*2);
	screen_B[3]	= (word*)(blockmem2+2+8400*3);
	render_B	= (byte*)(blockmem2+2+8400*4);
	temp_Y		= (word*)(blockmem2+2+8400*4+16000+2);

	Dread_SetupCopper(copperlist_buff1);
	SetCopper(copperlist_buff1);


//#define _A	0xF0
//#define _B	0xCC
//#define _C	0xAA
//	BlitWait();
//	BLTCDAT = 0x5555;								// data
//	BLTCON0 = 0x1D00 | (_A&_C | _B&~_C);			// BLTCONx
//	BLTCON1 = 0xF000;
//	BLTAWM = 0xFFFFFFFF;							// mask
//	BLTAMOD = 320-80;								// modulos
//	BLTBMOD = 320-80;
//	BLTDMOD = 320-80;
//	BLTAPT = screen_planes;							// pointers
//	BLTBPT = screen_planes + 1;
//	BLTDPT = screen_planes + 80;
//	BLTSIZE = COMPUTE_BLTSIZE(40, 100);				// start

	//for( int y=0; y<100; y++ )
	//	for( int x=0; x<20; x++ )
	//	{
	//		word v;
	//		v = screen_planes[x+y*160];
	//		screen_planes[x+y*160+80] = ((v&0xAAAA)>>1) | ((v&0x5555)<<1);
	//		v = screen_planes[x+y*160+20];
	//		screen_planes[x+y*160+80+20] = ((v&0xAAAA)>>1) | ((v&0x5555)<<1);
	//		v = screen_planes[x+y*160+40];
	//		screen_planes[x+y*160+80+40] = ((v&0xAAAA)>>1) | ((v&0x5555)<<1);
	//		v = screen_planes[x+y*160+60];
	//		screen_planes[x+y*160+80+60] = ((v&0xAAAA)>>1) | ((v&0x5555)<<1);
	//	}

	volatile word foo = 0;
//	while( 1 )
	{
	//	BlitWait();
	//	BLTAPT = screen_planes + 40;					// pointers
	//	BLTBPT = screen_planes + 40 + 1;
	//	BLTDPT = screen_planes + 120;
	//	BLTSIZE = COMPUTE_BLTSIZE(40, 100);				// start
	//
	//	BlitWait();
//		foo = 54321;
//		foo /= 12345;
//		foo /= 12345;
//		foo /= 12345;
//		foo /= 12345;
//		foo /= 12345;


		//memset(blockmem1, 0, 0x10000);
		//const byte *src = TEST_FRAME;
		//word sh = 16;
		//int xx = 0;
		//for( int x=0; x<160; x++ )
		//{
		//	if( sh )
		//		sh -= 2;
		//	else
		//		sh = 14, xx++;
		//
		//	for( int y=0; y<100; y++ )
		//	{
		//		word b = *src++;
		//		screen_A[0][xx+y*42] |= ((b&0xC0)>>6)<<sh;
		//		screen_A[1][xx+y*42] |= ((b&0x30)>>4)<<sh;
		//		screen_A[2][xx+y*42] |= ((b&0x0C)>>2)<<sh;
		//		screen_A[3][xx+y*42] |= ((b&0x03)>>0)<<sh;
		//	}
		//}

		const byte *src = TEST_FRAME;
		//int xx = 0;
		for( int x=0; x<160; x++ )
		{
			byte *dst = render_A + (x>>2) + (x&3)*4000;
			for( int y=0; y<100; y++ )
			{
				*dst = *src++;
				dst += 40;
			}
		}

#if 0
		//temp_X[0] = 0xEE00;
		//temp_X[2000] = 0xC000;
		//temp_X[20] = 0xEE00;
		//temp_Y[0] = 0xEE00;
		render_A[3] = 0x9F;			// $DE -> 1101 1110		1001 1111
		render_A[40* 0] = 0xC0;
		render_A[40* 1] = 0x30;
		render_A[40* 2] = 0xF0;
		render_A[40* 3] = 0x0C;
		render_A[40* 4] = 0xCC;
		render_A[40* 5] = 0x3C;
		render_A[40* 6] = 0xFC;
		render_A[40* 7] = 0x03;
		render_A[40* 8] = 0xC3;
		render_A[40* 9] = 0x33;
		render_A[40*10] = 0xF3;
		render_A[40*11] = 0x0F;
		render_A[40*12] = 0xCF;
		render_A[40*13] = 0x3F;
		render_A[40*14] = 0xFF;

		render_A[1+40* 0] = 0x80;
		render_A[1+40* 1] = 0x20;
		render_A[1+40* 2] = 0xA0;
		render_A[1+40* 3] = 0x08;
		render_A[1+40* 4] = 0x88;
		render_A[1+40* 5] = 0x28;
		render_A[1+40* 6] = 0xA8;
		render_A[1+40* 7] = 0x02;
		render_A[1+40* 8] = 0x82;
		render_A[1+40* 9] = 0x22;
		render_A[1+40*10] = 0xA2;
		render_A[1+40*11] = 0x0A;
		render_A[1+40*12] = 0x8A;
		render_A[1+40*13] = 0x2A;
		render_A[1+40*14] = 0xAA;

		render_A[2+40* 0] = 0x80>>1;
		render_A[2+40* 1] = 0x20>>1;
		render_A[2+40* 2] = 0xA0>>1;
		render_A[2+40* 3] = 0x08>>1;
		render_A[2+40* 4] = 0x88>>1;
		render_A[2+40* 5] = 0x28>>1;
		render_A[2+40* 6] = 0xA8>>1;
		render_A[2+40* 7] = 0x02>>1;
		render_A[2+40* 8] = 0x82>>1;
		render_A[2+40* 9] = 0x22>>1;
		render_A[2+40*10] = 0xA2>>1;
		render_A[2+40*11] = 0x0A>>1;
		render_A[2+40*12] = 0x8A>>1;
		render_A[2+40*13] = 0x2A>>1;
		render_A[2+40*14] = 0xAA>>1;
#endif

#define _A	0xF0
#define _B	0xCC
#define _C	0xAA
		BlitWait();										// ======== Pass 1a
		BLTCDAT = 0xCCCC;								// data
		BLTCON0 = 0xED00 | (_A&_C | _B&~_C);			// BLTCONx
		BLTCON1 = 0x0000;
		BLTAWM = 0xFFFFFFFF;							// mask
		BLTAMOD = 0;									// modulos
		BLTBMOD = 0;
		BLTDMOD = 0;
		BLTAPT = render_A;								// pointers
		BLTBPT = render_A+4000-2;
		BLTDPT = temp_X+2000-1;
		BLTSIZE = COMPUTE_BLTSIZE(23, 87);				// start

		BlitWait();										// ======== Pass 1b
		BLTCON0 = 0xED00 | (_A&_C | _B&~_C);			// BLTCONx
		BLTCON1 = 0x0000;
		BLTAPT = render_A+8000;								// pointers
		BLTBPT = render_A+12000-2;
		BLTDPT = temp_Y+2000-1;
		BLTSIZE = COMPUTE_BLTSIZE(23, 87);				// start

		BlitWait();										// ======== Pass 1c
		BLTCON0 = 0x0D00 | (_A&_C | _B&~_C);			// BLTCONx
		BLTCON1 = 0x2000;
		BLTAPT = render_A;								// pointers
		BLTBPT = render_A+4000;
		BLTDPT = temp_X;
		BLTSIZE = COMPUTE_BLTSIZE(20, 100);				// start

		BlitWait();										// ======== Pass 1d
		BLTCON0 = 0x0D00 | (_A&_C | _B&~_C);			// BLTCONx
		BLTCON1 = 0x2000;
		BLTAPT = render_A+8000;							// pointers
		BLTBPT = render_A+12000;
		BLTDPT = temp_Y;
		BLTSIZE = COMPUTE_BLTSIZE(20, 100);				// start

		BlitWait();										// ======== Pass 2a
		BLTCDAT = 0xF0F0;								// data
		BLTCON0 = 0x0D00 | (_A&_C | _B&~_C);			// BLTCONx
		BLTCON1 = 0x4000;
		BLTAWM = 0xFFFFFFFF;							// mask
		BLTAMOD = 0;									// modulos
		BLTBMOD = 0;
		BLTDMOD = 44;
		BLTAPT = temp_X;								// pointers
		BLTBPT = temp_Y;
		BLTDPT = screen_A[0];
		BLTSIZE = COMPUTE_BLTSIZE(20, 200);				// start

		BlitWait();										// ======== Pass 2b
		BLTCDAT = 0xF0F0;								// data
		BLTCON0 = 0xCD00 | (_A&_C | _B&~_C);			// BLTCONx
		BLTCON1 = 0x0000;
		BLTAWM = 0xFFFFFFFF;							// mask
		BLTAMOD = -2;									// modulos
		BLTBMOD = -2;
		BLTDMOD = 42;
		BLTAPT = temp_X;								// pointers
		BLTBPT = temp_Y-1;
		BLTDPT = screen_A[2]-1;
		BLTSIZE = COMPUTE_BLTSIZE(21, 200);				// start

		BlitWait();										// ======== HQ dither
		BLTCDAT = 0x5555;								// data
		BLTCON0 = 0x1D00 | (_A&_C | _B&~_C);			// BLTCONx
		BLTCON1 = 0xF000;
		BLTAWM = 0xFFFFFFFF;							// mask
		BLTAMOD = 42;									// modulos
		BLTBMOD = 42;
		BLTDMOD = 42;
		BLTAPT = screen_A[0]-1;							// pointers
		BLTBPT = screen_A[0];
		BLTDPT = screen_A[0]+21-1;
		BLTSIZE = COMPUTE_BLTSIZE(21, 400);				// start
	}


	while( 1 )
	{
	}
}
