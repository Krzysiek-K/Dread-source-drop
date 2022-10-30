
#include "demo.h"


typedef struct _MenuDirtyRect {
	word	x1;		// 0..20 in 16px blocks
	word	x2;		// 0..20 in 16px blocks
	word	y1;		// 0..236 in 1px lines
	word	y2;		// 0..236 in 1px lines
} MenuDirtyRect;


typedef struct _MenuFrameInfo {
	word			**screen;
	word			*copper;
	MenuDirtyRect	dirty[5];
	word			num_dirty;
} MenuFrameInfo;

typedef struct _MenuInfo {
	word			*title[5];			// chipram copy of DREAD letters
	word			*backrow[4];		// 320x48 x 4bpp image of a single row of tiles
	word			*bloodmap;			// chipram copy
	word			*textbuff;			// one-line 320x11 buffer for compositing text line
	word			*fontmask;			// font data (only the data)
	word			textbuff_len;		// number of pixels filled in textbuff

	MenuFrameInfo	frame[2];
} MenuInfo;



MenuInfo menu;


extern const word GFX_BLOOD_MAP[];
extern const word GFX_MENU_TEXTURE[];
extern const word GFX_TITLE1[];
extern const word GFX_TITLE2[];
extern const word GFX_TITLE3[];
extern const word GFX_TITLE4[];
extern const word GFX_TITLE5[];
extern const word GFX_MenuFont[];


const word MENU_PAL[] = {
	0x000,	//	 0
	0x111,	//	 1
	0x333,	//	 2
	0x444,	//	 3
	0x555,	//	 4
	0x666,	//	 5
	0x777,	//	 6
	0x211,	//	 7
	0x322,	//	 8
	0x433,	//	 9
	0x332,	//	10
	0x221,	//	11
	0x222,	//	12
	0x400,	//	13
	0x700,	//	14
	0x900,	//	15
	0x100,	//	16
	0x200,	//	17
	0x422,	//	18
	0x533,	//	19
	0x544,	//	20
	0x644,	//	21
	0x655,	//	22
	0x300,	//	23
	0x411,	//	24
	0x522,	//	25
	0x421,	//	26
	0x311,	//	27
	0x311,	//	28
	0x766,	//	29
	0x888,	//	31
	0x999,	//	32
};


// ================================================================ Initialization helpers ================================================================



word *Menu_LoadImage(word **alloc, const word *image, word bpp)
{
	word size = 6 + image[1]*image[2]*bpp;		// h * stride * bpp
	word *dst = *alloc;
	memcpy(dst, image, size);
	*alloc += size>>1;
	return dst;
}

word *Menu_LoadTileRow(word **alloc, const word *image, word plane)
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

word *Menu_LoadFontMasks(word **alloc, const word *font)
{
	word size = font[0]*font[2];		// h * glyphs	in words
	const word *src = font + 3 + font[2];
	word *dst = *alloc;
	memcpy(dst, src, size<<1);
	*alloc += size;
	return dst;
}

word *Menu_GenCopper(word **alloc, word *screen[])
{
	word *copper = *alloc;
	cop_write = copper;
	Cop_StartBit5(screen[0], screen[1], screen[2], screen[3], screen[4]);
	Cop_NoSprites();
	cop_write_y += 236;
	Cop_Wait256();
	Cop_StartBit1(menu.textbuff);
	//	Cop_Write(COLOR01, 0xFFF);
	//	cop_write_y += 11;
	//	Cop_Wait();
	//	Cop_Write(COLOR01, 0x111);
	Cop_SetBit0();
	Cop_End();

	*alloc = cop_write;
	return copper;
}

void Menu_InitFrame(MenuFrameInfo *frame, word *screen[], word *copper)
{
	frame->num_dirty = 0;
	frame->screen = screen;
	frame->copper = copper;
}



// ================================================================ Drawing ================================================================


// X coords are in 16px blocks	[0.. 20]
// Y coords are in 1px lines	[0..236]
void Menu_ResetBlock(word *screen[], word x1, word y1, word x2, word y2)
{
	BlitWait();

	BLTAFWM = 0xFFFF;
	BLTALWM = 0xFFFF;
	BLTCON0 = 0x09F0;
	BLTCON1 = 0;
	BLTAMOD = 40 - ((x2-x1)<<1);
	BLTDMOD = 40 - ((x2-x1)<<1);

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

void Menu_ResetFrame(MenuFrameInfo *frame)
{
	for( word d=0; d<frame->num_dirty; d++ )
	{
		Menu_ResetBlock(
			frame->screen,
			frame->dirty[d].x1,
			frame->dirty[d].y1,
			frame->dirty[d].x2,
			frame->dirty[d].y2);
	}
	frame->num_dirty = 0;
}

void Menu_FrameDrawImage(MenuFrameInfo *frame, word *image, word xp, word yp)
{
	Blit_DrawImage(frame->screen, image, xp, yp);

	MenuDirtyRect *dirty = &frame->dirty[frame->num_dirty++];
	dirty->x1 = xp/16;
	dirty->x2 = (xp + image[0] + 15)/16;
	dirty->y1 = yp;
	dirty->y2 = yp + image[1];
}

void Menu_DrawText(const char *s)
{
	word h = GFX_MenuFont[0];
	BlitWait();

	BLTCON1 = 0;
	BLTAFWM = 0xFFFF;

	// clear
	if( menu.textbuff_len > 0 )
	{
		word width = (menu.textbuff_len+15)/16;
		BLTALWM = 0xFFFF;
		BLTCON0 = 0x0100;
		BLTDMOD = 40 - (width<<1);	// screen
		BLTDPT = menu.textbuff;
		BLTSIZE = (h<<6) | width;
	}

	// print
	word xp = 0;
	while(*s)
	{
		word func = 0x0BFA;
		word ch = *s++ - GFX_MenuFont[1];

		if( ch >= GFX_MenuFont[2] )
		{
			xp += 4;	// space width
			continue;
		}

		word dstoff = (xp>>4);		// in words
		word width = 1;
		BlitWait();
		BLTALWM = 0xFFFF;
		BLTAMOD = 0;
		if( xp & 0x0F )
		{
			func |= (xp & 0x0F)<<12;
			width++;
			BLTALWM = 0;
			BLTAMOD = (word)-2;
		}

		BLTCON0 = func;

		BLTCMOD = 40 - (width<<1);	// screen
		BLTDMOD = 40 - (width<<1);	// screen

		BLTAPT = menu.fontmask + ch*h;
		BLTCPT = menu.textbuff + dstoff;
		BLTDPT = menu.textbuff + dstoff;
		BLTSIZE = (h<<6) | width;

		xp += GFX_MenuFont[3+ch] + 1;	// + spacing
	}

	menu.textbuff_len = xp;
}

void Menu_TextLayer(MenuFrameInfo *frame, word xp, word yp, word color)
{
	word h = GFX_MenuFont[0];

	for( word i=0; i<5; i++ )
	{
		BlitWait();

		word func = (color&1) ? 0x0BFA : 0x0B0A;
		word width =  ((xp+menu.textbuff_len+15)>>4) - (xp>>4);
		word dstoff = (xp>>4) + yp*20;		// in words
		BLTALWM = 0xFFFF;
		if( xp & 0x0F )
		{
			func |= (xp & 0x0F)<<12;
			width++;
			BLTALWM = 0;
		}

		BLTCON0 = func;
		BLTCON1 = 0;

		word mod = 40 - (width<<1);
		BLTAMOD = mod;
		BLTCMOD = mod;
		BLTDMOD = mod;

		BLTAPT = menu.textbuff;
		BLTCPT = frame->screen[i] + dstoff;
		BLTDPT = frame->screen[i] + dstoff;
		BLTSIZE = (h<<6) | width;

		color >>= 1;
	}
}

void Menu_PlaceText(MenuFrameInfo *frame, word xp, word yp)
{
	Menu_TextLayer(frame, xp+2, yp+4, 0);	//	0x000,	//	 0
	Menu_TextLayer(frame, xp, yp+2, 13);	//	0x400,	//	13
	Menu_TextLayer(frame, xp, yp, 15);		//	0x900,	//	15
	Menu_TextLayer(frame, xp, yp+1, 14);	//	0x700,	//	14

	//	0x100,	//	16
	//	0x200,	//	17
	//	0x300,	//	23

	//	0x111,	//	 1
	//	0x333,	//	 2
	//	0x444,	//	 3
	//	0x555,	//	 4
	//	0x666,	//	 5
	//	0x777,	//	 6
	//	0x211,	//	 7
	//	0x322,	//	 8
	//	0x433,	//	 9
	//	0x332,	//	10
	//	0x221,	//	11
	//	0x222,	//	12
	//	0x422,	//	18
	//	0x533,	//	19
	//	0x544,	//	20
	//	0x644,	//	21
	//	0x655,	//	22
	//	0x411,	//	24
	//	0x522,	//	25
	//	0x421,	//	26
	//	0x311,	//	27
	//	0x311,	//	28
	//	0x766,	//	29
	//	0x888,	//	31
	//	0x999,	//	32

}

void Menu_DrawLogo(void)
{
	for( int i=0; i<5; i++ )
	{
		// Logo placement: 64+39, 20	(39x32)
		Blit_DrawImage(screen_A, menu.title[i], 64+39*i, 20);
		Blit_DrawImage(screen_B, menu.title[i], 64+39*i, 20);
	}
}


// ================================================================ Logic ================================================================



void Menu_Init(void)
{
	INTENA = 0x0040;			// disable blitter interrupts
	DMACON = 0x8000 | (1<<10);	// enable blitter nasty
	DMACON = 0x8000 | (1<< 7);	// enable copper

	for( int i=0; i<32; i++ )
		COLOR[i] = MENU_PAL[i];

	BPL1MOD = 0;
	BPL2MOD = 0;

	// allocate chip memory
	word *alloc_A = (word*)blockmem1;
	word *alloc_B = (word*)blockmem2;

	for( int i=0; i<5; i++ )
	{
		screen_A[i] = alloc_A;
		alloc_A += 4720;	// 320x236 in words

		screen_B[i] = alloc_B;
		alloc_B += 4720;	// 320x236 in words
	}

	menu.bloodmap = Menu_LoadImage(&alloc_B, GFX_BLOOD_MAP, 1) + 3;		// skip header
	menu.title[0] = Menu_LoadImage(&alloc_A, GFX_TITLE1, 6);
	menu.title[1] = Menu_LoadImage(&alloc_A, GFX_TITLE2, 6);
	menu.title[2] = Menu_LoadImage(&alloc_A, GFX_TITLE3, 6);
	menu.title[3] = Menu_LoadImage(&alloc_A, GFX_TITLE4, 6);
	menu.title[4] = Menu_LoadImage(&alloc_A, GFX_TITLE5, 6);
	menu.fontmask = Menu_LoadFontMasks(&alloc_A, GFX_MenuFont);

	for( int plane=0; plane<4; plane++ )
		menu.backrow[plane] = Menu_LoadTileRow(&alloc_B, GFX_MENU_TEXTURE, plane);

	menu.textbuff = alloc_B;
	alloc_B += 440/2;


	// create copper lists (in chip memory)
	copper_A = Menu_GenCopper(&alloc_A, screen_A);
	copper_B = Menu_GenCopper(&alloc_A, screen_B);


	// reset internal structures
	Menu_InitFrame(&menu.frame[0], screen_A, copper_A);
	Menu_InitFrame(&menu.frame[1], screen_B, copper_B);

	menu.textbuff_len = 320;

	// fill background
	Menu_ResetBlock(screen_A, 0, 0, 20, 236);
	Menu_ResetBlock(screen_B, 0, 0, 20, 236);
}


void Menu_Run(void)
{
	Menu_Init();
	Menu_DrawLogo();

	SetCopper(copper_A);


	word xp[5] ={ 64, 64+39*1, 64+39*2, 64+39*3, 64+39*4 };
	word yp[5] ={ 20, 20, 20, 20, 20 };
	short dx[5] ={ -2, -1, 1, -1, 2 };
	short dy[5] ={ 1,  2,-2,  1,-1 };


	io_key_last = 0;
	intnum = 0;
	while( intnum<1 ) {}

	word flip = 0;
	while( io_key_last != KEYSCAN_ESC )
	{
		intnum = 0;
		//while( VHPOSR<0x4000 ) {}
//		COLOR00 = 0xF00;

		MenuFrameInfo *frame = &menu.frame[flip];
		//Menu_ResetBlock(screen_A, 4, 20, 13, 20+32);
		Menu_ResetFrame(frame);


//		COLOR00 = 0x0F0;
		//for( int i=0; i<5; i++ )
		//{
		//	Menu_FrameDrawImage(frame, menu.title[i], xp[i], yp[i]);
		//	//Menu_FrameDrawImage(frame, menu.title[i], 64+39*i, 20);
		//	if( io_keystate[KEYSCAN_F1] )
		//	{
		//		if( (word)(xp[i]+dx[i]) > 320-39 ) dx[i] = -dx[i];
		//		if( (word)(yp[i]+dy[i]) > 236-32 ) dy[i] = -dy[i];
		//		xp[i] += dx[i];
		//		yp[i] += dy[i];
		//	}
		//}

		Menu_DrawText("HELLO WORLD!");
		//Menu_TextLayer(frame, 128, 127, 15);
		Menu_PlaceText(frame, (320-menu.textbuff_len)>>1, 100);
//		COLOR00 = 0x000;

		//while( intnum<1 ) {}
		SetCopper(frame->copper);
		while( intnum<1 ) {}
		flip = 1-flip;
	}

	// exit
	SetCopper(BLANK_COPPER);
}
