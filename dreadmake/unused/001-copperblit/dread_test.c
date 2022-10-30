
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


word *screen_planes;




void Dread_SetupCopper(word *copper)
{
	BPL1MOD = 3*40;
	BPL2MOD = 3*40;

	cop_write = copper;
	Cop_StartBit4(screen_planes, screen_planes+20, screen_planes+40, screen_planes+60);
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
//	*cop_write++ = 0x064;	*cop_write++ = 320-80;									//	BLTAMOD = 320-80;								// modulos
//	*cop_write++ = 0x062;	*cop_write++ = 320-80;									//	BLTBMOD = 320-80;
//	*cop_write++ = 0x066;	*cop_write++ = 320-80;									//	BLTDMOD = 320-80;
//	*cop_write++ = 0x050;	*cop_write++ = (word)((dword)screen_planes >> 16);		//	BLTAPT = screen_planes;							// pointers
//	*cop_write++ = 0x052;	*cop_write++ = (word)(dword)screen_planes;
//	*cop_write++ = 0x04C;	*cop_write++ = (word)((dword)screen_planes >> 16);		//	BLTBPT = screen_planes + 1;
//	*cop_write++ = 0x04E;	*cop_write++ = (word)(dword)(screen_planes + 1);
//	*cop_write++ = 0x054;	*cop_write++ = (word)((dword)screen_planes >> 16);		//	BLTDPT = screen_planes + 80;
//	*cop_write++ = 0x056;	*cop_write++ = (word)(dword)(screen_planes + 80);
//	*cop_write++ = 0x058;	*cop_write++ = COMPUTE_BLTSIZE(40, 100);				//	BLTSIZE = COMPUTE_BLTSIZE(40, 100);				// start
//	*cop_write++ = 0x180;	*cop_write++ = 0x040;									//	[GREEN]
//
//	// copper blit 2
//	*cop_write++ = 0x0001;	*cop_write++ = 0x0000;							//	BlitWait();
//	*cop_write++ = 0x052;	*cop_write++ = (word)(screen_planes + 40);		//	BLTAPTL = screen_planes + 40;					// pointers
//	*cop_write++ = 0x04E;	*cop_write++ = (word)(screen_planes + 40 + 1);	//	BLTBPTL = screen_planes + 40 + 1;
//	*cop_write++ = 0x056;	*cop_write++ = (word)(screen_planes + 120);		//	BLTDPTL = screen_planes + 120;
//	*cop_write++ = 0x058;	*cop_write++ = COMPUTE_BLTSIZE(40, 100);		//	BLTSIZE = COMPUTE_BLTSIZE(40, 100);				// start
//	*cop_write++ = 0x180;	*cop_write++ = 0x400;							//	[RED]
//	*cop_write++ = 0x0001;	*cop_write++ = 0x0000;							//	BlitWait();
//	*cop_write++ = 0x180;	*cop_write++ = 0x000;							//	[BLACK]


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

	screen_planes = (word*)blockmem1;

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
	while( 1 )
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


		memset(blockmem1, 0, 0x10000);
		const byte *src = TEST_FRAME;
		word sh = 16;
		int xx = 0;
		for( int x=0; x<160; x++ )
		{
			if( sh )
				sh -= 2;
			else
				sh = 14, xx++;

			for( int y=0; y<100; y++ )
			{
				word b = *src++;
				screen_planes[xx+y*160] |= ((b&0xC0)>>6)<<sh;
				screen_planes[xx+y*160+20] |= ((b&0x30)>>4)<<sh;
				screen_planes[xx+y*160+40] |= ((b&0x0C)>>2)<<sh;
				screen_planes[xx+y*160+60] |= ((b&0x03)>>0)<<sh;
			}
		}
	}

}
