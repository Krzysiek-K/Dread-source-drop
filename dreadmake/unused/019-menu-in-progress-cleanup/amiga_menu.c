
#include "demo.h"


typedef struct _MenuInfo {
	word	*title[5];			// chipram copy of DREAD letters
	word	*backrow[4];		// 320x48 x 4bpp image of a single row of tiles
	word	*bloodmap;			// chipram copy
	word	*textbuff;			// one-line 320x11 buffer for compositing text line
} MenuInfo;



MenuInfo menu;


extern const word GFX_BLOOD_MAP[];
extern const word GFX_MENU_TEXTURE[];
extern const word GFX_TITLE1[];
extern const word GFX_TITLE2[];
extern const word GFX_TITLE3[];
extern const word GFX_TITLE4[];
extern const word GFX_TITLE5[];


const word MENU_PAL[] = {
	0x000,
	0x111,
	0x333,
	0x444,
	0x555,
	0x666,
	0x777,
	0x211,
	0x322,
	0x433,
	0x332,
	0x221,
	0x222,
	0x400,
	0x700,
	0x900,
	0x100,
	0x200,
	0x422,
	0x533,
	0x544,
	0x644,
	0x655,
	0x300,
	0x411,
	0x522,
	0x421,
	0x311,
	0x311,
	0x766,
	0x888,
	0x999, 
};



//void Menu_DrawImage(word *screen[], const word *image, short xp, short yp)
//{
//	word w = (image[2]>>1)*16;
//	word h = image[1];
//	word imgstride = (image[2]>>1)*h;
//	Plane src, dst;
//	src.stride = image[2]>>1;
//	dst.stride = 20;
//
//	blit_func = 0x09F0;
//
//	for( int plane=0; plane<5; plane++ )
//	{
//		src.base = image + 3 + plane*imgstride;
//		dst.base = screen_A[plane];
//
//		Blit_Run(&src, 0, 0, w, h, &dst, xp, yp);
//	}
//}

#if 0
void Menu_DrawImage(word *screen[], const word *image, word xp, word yp)
{
	word width = image[2]>>1;			// in words
	word h = image[1];
	word planestride = width*h;			// in words

	BlitWait();


	// A = 11110000
	// B = 11001100
	// C = 10101010
	//	   11001010		D = A&B | ~A&C
	word func = 0x0FCA;

	const word *src = image + 3;
	word dstoff = (xp>>4) + yp*20;		// in words
	BLTAFWM = 0xFFFF;
	BLTALWM = 0xFFFF;
	BLTAMOD = 0;				// mask
	BLTBMOD = 0;				// data
	if( xp & 0x0F )
	{
		func |= (xp & 0x0F)<<12;
		width++;
		BLTALWM = 0;
		BLTAMOD = -2;
		BLTBMOD = -2;
	}

	BLTCON0 = func;
	BLTCON1 = func & 0xF000;

	BLTCMOD = 40 - (width<<1);	// screen
	BLTDMOD = 40 - (width<<1);	// screen

	BLTBPT = src;
	for( int plane=0; plane<5; plane++ )
	{
		BLTAPT = src + planestride*5;
		//BLTBPT = src + plane*planestride;
		BLTCPT = screen_A[plane] + dstoff;
		BLTDPT = screen_A[plane] + dstoff;

		BLTSIZE = (h<<6) | width;
		BlitWait();
	}
}
#endif

word *Menu_LoadImage(word **alloc, const word *image, word bpp)
{
	word size = 6 + image[1]*image[2]*bpp;		// h * stride * bpp
	word *dst = *alloc;
	memcpy(dst, image, size);
	*alloc += size>>1;
	return dst;
}

word *Menu_Load_TileRow(word **alloc, const word *image, word plane)
{
	word *dst = *alloc;
	menu.backrow[plane] = dst;
	for( int y=0; y<48; y++ )
	{
		const word *src = image + 3 + (plane*48 + y)*3;
		for( int x=0; x<20; x+=3 )
		{
			*dst++ = src[0];
			*dst++ = src[1];
			*dst++ = src[2];
		}
		dst--;
	}

	*alloc = dst;
	return menu.backrow[plane];
}

// X coords are in 16px blocks	[0.. 20]
// Y coords are in 1px lines	[0..236]
void Menu_ResetBlock(word *screen[], word x1, word y1, word x2, word y2)
{
	BlitWait();

	BLTAFWM = 0xFFFF;
	BLTALWM = 0xFFFF;
	BLTCON0 = 0x09F0;
	BLTCON1 = 0;
	BLTAMOD = 0;
	BLTDMOD = 0;

	// Clear blood plane
	word dstoffs0 = x1 + y1*20;
	BLTAPT = menu.bloodmap + dstoffs0;
	BLTDPT = screen[4] + dstoffs0;
	BLTSIZE = ((y2-y1)<<6) | (x2-x1);
	BlitWait();

	while( y1 < y2 )
	{
		word yend = y1/48*48+48;
		if( yend>y2 )
			yend = y2;

		// Clear texture planes
		word srcoffs = x1 + (y1%48)*20;
		word dstoffs = x1 + y1*20;
		word bsize = ((yend-y1)<<6) | (x2-x1);
		for( word plane=0; plane<4; plane++ )
		{
			BLTAPT = menu.backrow[plane] + srcoffs;
			BLTDPT = screen[plane] + dstoffs;
			BLTSIZE = bsize;
			BlitWait();
		}

		y1 = yend;
	}
}

word *Menu_GenCopper(word **alloc, word *screen[])
{
	word *copper = *alloc;
	cop_write = copper;
	//Cop_StartBit5(menu.backrow[0], menu.backrow[1], menu.backrow[2], menu.backrow[3], screen[4]);
	Cop_StartBit5(screen[0], screen[1], screen[2], screen[3], screen[4]);
	//Cop_StartBit1(screen[4]);
	Cop_NoSprites();
	cop_write_y += 236;
	Cop_Wait256();
	Cop_StartBit1(menu.textbuff);
	Cop_Write(COLOR01, 0xFFF);
	cop_write_y += 11;
	Cop_Wait();
	Cop_Write(COLOR01, 0x111);
	Cop_SetBit0();
	Cop_End();

	*alloc = cop_write;
	return copper;
}



void Menu_Run(void)
{
	for( int i=0; i<32; i++ )
		COLOR[i] = MENU_PAL[i];

	// init
	{
		word *alloc_A = (word*)blockmem1;
		word *alloc_B = (word*)blockmem2;
		
		for( int i=0; i<5; i++ )
		{
			screen_A[i] = alloc_A;
			alloc_A += 4720+1;	// 320x236 in words + 1 word fudge

			screen_B[i] = alloc_B;
			alloc_B += 4720+1;	// 320x236 in words + 1 word fudge
		}
		
		menu.bloodmap = Menu_LoadImage(&alloc_B, GFX_BLOOD_MAP, 1) + 3;		// skip header
		menu.title[0] = Menu_LoadImage(&alloc_A, GFX_TITLE1, 6);
		menu.title[1] = Menu_LoadImage(&alloc_A, GFX_TITLE2, 6);
		menu.title[2] = Menu_LoadImage(&alloc_A, GFX_TITLE3, 6);
		menu.title[3] = Menu_LoadImage(&alloc_A, GFX_TITLE4, 6);
		menu.title[4] = Menu_LoadImage(&alloc_A, GFX_TITLE5, 6);

		for( int plane=0; plane<4; plane++ )
			menu.backrow[plane] = Menu_Load_TileRow(&alloc_B, GFX_MENU_TEXTURE, plane);

		menu.textbuff = alloc_B;
		alloc_B += 440/2;


		copper_A = Menu_GenCopper(&alloc_A, screen_A);
	}

	//for( int plane=0; plane<4; plane++ )
	//	for( int y=0; y<236; y++ )
	//	{
	//		const word *src = GFX_MENU_TEXTURE + 3 + (plane*48 + y%48)*3;
	//		word *dst = screen_A[plane] + y*20;
	//		for( int x=0; x<20; x+=3 )
	//		{
	//			*dst++ = src[0];
	//			*dst++ = src[1];
	//			*dst++ = src[2];
	//		}
	//	}
	//
	//memcpy(screen_A[4], GFX_BLOOD_MAP+3, GFX_BLOOD_MAP[1]*GFX_BLOOD_MAP[2]);
	Menu_ResetBlock(screen_A, 0, 0, 20, 236);

	BPL1MOD = 0;
	BPL2MOD = 0;

	SetCopper(copper_A);
	DMACON = 0x8000 | (1<< 7);	// enable copper

	//intnum = 0;
	//while( intnum<10 ) {}


	//Menu_DrawImage(screen_A, menu.title[0], 0, 0);



	//COLOR00 = 0x000;
	//COLOR01 = 0xF00;
	io_key_last = 0;
	word pos = 0;
	intnum = 0;
	while( io_key_last != KEYSCAN_ESC )
	{
		intnum = 0;
		while( !intnum ) {}
		while( VHPOSR<0x4000 ) {}
		COLOR00 = 0xF00;

		Menu_ResetBlock(screen_A, 4, 20, 13, 20+32);

		COLOR00 = 0x0F0;
		// Logo placement: 64+39, 20	(39x32)
		//Menu_DrawImage(screen_A, menu.title[0], pos*101%(320-39), pos*113%(236-32));
		for( int i=0; i<5; i++ )
			Blit_DrawImage(screen_A, menu.title[i], 64+39*i, 20);

		COLOR00 = 0x000;
		pos++;
	}

	//intnum = 0;
	//while( intnum<50 ) {}

	// exit
	SetCopper(BLANK_COPPER);
	//intnum = 0;
	//while( intnum<2 ) {}
}
