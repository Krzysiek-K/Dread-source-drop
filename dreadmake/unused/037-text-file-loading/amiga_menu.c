
#include "demo.h"


//#define COPYRIGHT_1		"X-MAS VERSION 0C24"
//#define COPYRIGHT_2		"---- DO NOT SPREAD ----"
//#define COPYRIGHT_3		"VER 1905 (C) KK/ALTAIR 2021"

//#define COPYRIGHT_2		"VER 1912 -- PERSONAL THANKS TO"
//#define COPYRIGHT_3		"@@@@NAME------------------------"

#define SHOW_TEXT_FILES		0


extern const word PALETTE_MENU[];

#if AMIGA_AUDIO_ENGINE_VERSION == 2
extern const AudioTrackRow TRKSOUND_DSRLAUNC[];
extern const AudioTrackRow TRKSOUND_DSWPNUP[];
extern const AudioTrackRow TRKSOUND_DSWEAP1[];
extern const AudioTrackRow TRKSOUND_DSAMMO[];
extern const AudioTrackRow TRKSOUND_DSSELECT[];
#endif





// ================================================================ Game options ================================================================

#define MOUSE_SENS_DEFAULT		10
#define MOUSE_SENS_MAX			15

short opt_mouse_sens_index;
short opt_mouse_sens;
word opt_screen_mode;
word opt_ntsc;

const short MOUSE_SENS_TABLE[] ={
	1,
	2,
	3,
	4,
	5,
	6,
	8,
	10,
	12,
	14,
	16,
	19,
	23,
	28,
	34,
	41,
	49,
	59,
	71,
	85,
	102,
	122,
	146,
	175,
	210,
	256,
};

const char *SCREEN_MODES_NAMES[] = {
	"DEFAULT",
	"SCANLINES",
	"SHAKE",
	"CHECKER",
};

const char *AUDIO_MODES_NAMES[] = {
	"SFX MONO",
	"SFX STEREO"
};

const char *AUDIO_FILTER_NAMES[] ={
	"OFF",
	"ON"
};


// ================================================================ Menu structures ================================================================

MenuInfo menu;


extern const word GFX_BLOOD_MAP[];
extern const word GFX_MENU_TEXTURE[];
extern const word GFX_TITLE1[];
extern const word GFX_TITLE2[];
extern const word GFX_TITLE3[];
extern const word GFX_TITLE4[];
extern const word GFX_TITLE5[];
extern const word GFX_CURSOR1[];
extern const word GFX_CURSOR2[];
extern const word GFX_MenuFont[];





// ================================================================ Helper functions ================================================================


word Menu_OptionComputeYP(word index)
{
	return ((236 - MENU_FONT_LINESPACING*menu.opt_rowcount)>>1) + index * MENU_FONT_LINESPACING + 10;
}

void Menu_SystemRestore(void)
{
	// Bring back the system
	Aud_Reset();
	Sys_Restore();
}

void Menu_GameRestore(void)
{
	// Shutdown the system & reinitialize (TBD: make common function for all that)
	Sys_Install();
	Main_Init();
	Menu_Init();
	Menu_FillBackgrounds();
	Menu_DrawLogo();
	SetCopper(menu.frame[0].copper);
	// TBD: draw copyright text
}



// ================================================================ Option functions ================================================================


void Options_Reset(void)
{
	opt_mouse_sens_index = MOUSE_SENS_DEFAULT;
	opt_mouse_sens = MOUSE_SENS_TABLE[opt_mouse_sens_index];
	opt_screen_mode = 0;
}



// ================================================================ Copper routines ================================================================


//					-*-	-0-	-1-	-2-	-3-	-4-	-5-	-6-	-7-	-8-	-9- -A- -*-
// Red font:	13	400
//				14	700										600	500
//				15	900
//
const word MENU_GRADIENT_RED[] = {
		0xD400, 0xE700, 0xF900,
	10, 0xE600,
	 1, 0xE500,
	 2,
	 0
};

//					-*-	-0-	-1-	-2-	-3-	-4-	-5-	-6-	-7-	-8-	-9- -A- -*-
// Selected:	13	332
//				14	999		AAA	999	888	777	666	555	454	544		444
//				15	888
//
const word MENU_GRADIENT_SELECTED[] ={
	0xD332, 0xE999, 0xF888,
	2, 0xEAAA,
	1, 0xE999,
	1, 0xE888,
	1, 0xE777,
	1, 0xE666,
	1, 0xE555,
	1, 0xE454,
	1, 0xE544,
	2, 0xE444,
	2,
	0
};

//					-*-	-0-	-1-	-2-	-3-	-4-	-5-	-6-	-7-	-8-	-9- -A- -*-
// Inactive:	13	210
//				14	310
//				15	520
//	
const word MENU_GRADIENT_INACTIVE[] ={
	0xD210, 0xE310, 0xF520,
	13,
	0
};

const word MENU_GRADIENT_COPYRIGHT[] ={
	0xD222, 0xE555, 0xF777,
	0
};

// returns copper end
word *Menu_GenCopper(MenuFrameInfo *frame)
{
	word *copper = frame->copper;
	word data_offs = menu.view_start * 20;
	cop_write = copper;
	Cop_StartBit5(
		frame->screen[0]+data_offs,
		frame->screen[1]+data_offs,
		frame->screen[2]+data_offs,
		frame->screen[3]+data_offs,
		frame->screen[4]+data_offs);
	Cop_NoSprites();

	word endy = cop_write_y + 233;

	if( menu.opt_rowcount )
	{
		word ybase = cop_write_y - menu.view_start;
		for( word i=0; i<menu.opt_rowcount; i++ )
		{
			cop_write_y = ybase + Menu_OptionComputeYP(i);
			Cop_Wait();
			if( menu.opt_inactive & (1<<i) )
				Cop_Gradient((word*)MENU_GRADIENT_INACTIVE);
			else if( i==menu.opt_selection )
				Cop_Gradient((word*)MENU_GRADIENT_SELECTED);
			else
				Cop_Gradient((word*)MENU_GRADIENT_RED);
		}
	}

	Cop_Gradient((word*)MENU_GRADIENT_COPYRIGHT);

	if( (cop_write_y^endy)&0xFF00 )
	{
		cop_write_y = endy;
		Cop_Wait256();
	}
	else
	{
		cop_write_y = endy;
		Cop_Wait();
	}

	//Cop_StartBit1(menu.textbuff);
	//	Cop_Write(COLOR01, 0xFFF);
	//	cop_write_y += 11;
	//	Cop_Wait();
	//	Cop_Write(COLOR01, 0x111);
	Cop_SetBit0();
	Cop_End();

	return cop_write;
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


void Menu_DrawLogo(void)
{
	for( int i=0; i<5; i++ )
	{
		// Logo placement: 64+39, 20	(39x32)
		Blit_DrawImage(screen_A, menu.title[i], 64+39*i, 20);
		Blit_DrawImage(screen_B, menu.title[i], 64+39*i, 20);
	}
}

void Menu_FillBackgrounds(void)
{
	// fill background
	Menu_ResetBlock(screen_A, 0, 0, 20, 236);
	Menu_ResetBlock(screen_B, 0, 0, 20, 236);
	menu.frame[0].num_dirty = 0;
	menu.frame[1].num_dirty = 0;
}


// ================================================================ Text ================================================================


void Menu_PrepareTextMask(const char *s)
{
	word h = GFX_MenuFont[0];
	word decode = 0;

	if( *s==5 )
	{
		decode = 0x5B;
		s++;
	}

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
	while( *s )
	{
		word func = 0x0BFA;
		word ch = (*s++ ^ decode) - GFX_MenuFont[1];

		if( decode )
		{
			decode += 0x1A;
		}

		if( ch >= GFX_MenuFont[2] )
		{
			xp += MENU_FONT_SPACE_WIDTH;
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

		xp += GFX_MenuFont[3+ch] + MENU_FONT_SPACING;
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
}



// ================================================================ Frame handling ================================================================


void Menu_InitFrame(MenuFrameInfo *frame, word *screen[], word *copper)
{
	frame->num_dirty = 0;
	frame->screen = screen;
	frame->copper = copper;
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

void Menu_FrameDrawImage(MenuFrameInfo *frame, word *image, word xp, short yp)
{
	Blit_DrawImage(frame->screen, image, xp, yp);

	MenuDirtyRect *dirty = &frame->dirty[frame->num_dirty++];
	dirty->x1 = xp/16;
	dirty->x2 = (xp + image[0] + 15)/16;
	dirty->y1 = yp;
	dirty->y2 = yp + image[1];
	if( (short)dirty->y1 < 0 ) dirty->y1 = 0;
	if( (short)dirty->y2 <= 0 ) frame->num_dirty--;
}

word Menu_FrameBegin(void)
{
	word delta = intnum;
	intnum = 0;
	menu.cframe = menu.frame + menu.frameflip;
	menu.view_start = 2;

	Menu_ResetFrame(menu.cframe);
	return delta;
}

void Menu_FrameEnd(void)
{
	//COLOR00 = 0xFF0;
	Menu_GenCopper(menu.cframe);
	//COLOR00 = 0x000;

	SetCopper(menu.cframe->copper);
	word waitnum = intnum;
	while( intnum==waitnum ) {}
	menu.frameflip ^= 1;
}



// ================================================================ Initialization ================================================================



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






void Menu_Init(void)
{
	memset(&menu, 0, sizeof(menu));
	menu.textbuff_len = 320;
	menu.frameflip = 0;
	menu.opt_selection = 0;
	menu.opt_inactive = 0;
	menu.view_start = 2;


	INTENA = 0x0040;			// disable blitter interrupts
	DMACON = 0x8000 | (1<<10);	// enable blitter nasty
	DMACON = 0x8000 | (1<< 7);	// enable copper

	for( int i=0; i<32; i++ )
		COLOR[i] = PALETTE_MENU[i];

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

	menu.bloodmap = Menu_LoadImage(&alloc_A, GFX_BLOOD_MAP, 1) + 3;		// skip header
	menu.title[0] = Menu_LoadImage(&alloc_A, GFX_TITLE1, 6);
	menu.title[1] = Menu_LoadImage(&alloc_B, GFX_TITLE2, 6);
	menu.title[2] = Menu_LoadImage(&alloc_B, GFX_TITLE3, 6);
	menu.title[3] = Menu_LoadImage(&alloc_B, GFX_TITLE4, 6);
	menu.title[4] = Menu_LoadImage(&alloc_B, GFX_TITLE5, 6);
	menu.cursor[0] = Menu_LoadImage(&alloc_A, GFX_CURSOR1, 6);
	menu.cursor[1] = Menu_LoadImage(&alloc_B, GFX_CURSOR2, 6);
	menu.fontmask = Menu_LoadFontMasks(&alloc_A, GFX_MenuFont);

	for( int plane=0; plane<4; plane++ )
		menu.backrow[plane] = Menu_LoadTileRow(&alloc_B, GFX_MENU_TEXTURE, plane);

	menu.textbuff = alloc_A;	alloc_A += 440/2;

	menu.debug1 = (alloc_A - (word*)blockmem1)<<1;
	menu.debug2 = (alloc_B - (word*)blockmem2)<<1;

	// reset internal structures
	Menu_InitFrame(&menu.frame[0], screen_A, alloc_A);
	Menu_InitFrame(&menu.frame[1], screen_B, alloc_B);

	// create copper lists (in chip memory)
	alloc_A = Menu_GenCopper(&menu.frame[0]);
	alloc_B = Menu_GenCopper(&menu.frame[1]);

	menu.debug3 = (alloc_A - (word*)blockmem1)<<1;

}




// ================================================================ Options system ================================================================


void Menu_OptionHide(word index)
{
	if( menu.cframe->opt_dirty & (1<<index) )
	{
		word y = Menu_OptionComputeYP(index);
		Menu_ResetBlock(menu.cframe->screen, 0, y, 20, y + MENU_FONT_LINETOTAL);

		menu.cframe->opt_dirty &= ~(1<<index);
	}
}

void Menu_OptionReset()
{
	MenuFrameInfo *_cframe = menu.cframe;

	menu.cframe = &menu.frame[0];
	for( word i=0; i<menu.opt_rowcount; i++ ) Menu_OptionHide(i);
	menu.cframe = &menu.frame[1];
	for( word i=0; i<menu.opt_rowcount; i++ ) Menu_OptionHide(i);
	menu.cframe = _cframe;

	io_key_last = 0;
	menu.frame[0].opt_drawn = 0;
	menu.frame[0].opt_dirty = 0;
	menu.frame[1].opt_drawn = 0;
	menu.frame[1].opt_dirty = 0;
	menu.opt_selection = 0;
	menu.opt_inactive = 0;

	memset(menu.opt_margin, 0, sizeof(menu.opt_margin));
}

int Menu_OptionBegin(word rowcount)
{
	Menu_FrameBegin();

	menu.opt_rowcount = rowcount;


	if( io_key_last==KEYSCAN_UP || io_key_last==KEYSCAN_W )
	{
		if( menu.opt_selection > 0 )
		{
			menu.opt_selection--;
			Sound_PlayThingSound(NULL, TRKSOUND_DSSELECT, 0);
		}
		io_key_last = 0;
	}

	if( io_key_last==KEYSCAN_DOWN || io_key_last==KEYSCAN_S )
	{
		if( menu.opt_selection < rowcount-1 )
		{
			menu.opt_selection++;
			Sound_PlayThingSound(NULL, TRKSOUND_DSSELECT, 0);
		}
		io_key_last = 0;
	}

	if( io_key_last == KEYSCAN_ESC )
	{
		Sound_PlayThingSound(NULL, TRKSOUND_DSAMMO, 0);
		io_key_last = 0;
		return 0;
	}

	return 1;
}

void Menu_OptionEnd()
{
	if( menu.opt_selection>=0 && menu.opt_margin[menu.opt_selection]>=menu.cursor[0][0]+(2+16+MENU_CURSOR_SHADOW) )
	{
		word yp = Menu_OptionComputeYP(menu.opt_selection) + (MENU_FONT_LINETOTAL-MENU_FONT_LINESHADOW)/2 - ((menu.cursor[0][1]-MENU_CURSOR_SHADOW+1)>>1);
		word anim = (rawintnum>>2)&3;
		if( anim==3 ) anim = 1;

		word x1 = menu.opt_margin[menu.opt_selection] - 16 - anim - menu.cursor[0][0];
		word x2 = 320 - (menu.opt_margin[menu.opt_selection] - 16 - anim - MENU_CURSOR_SHADOW);

		Menu_FrameDrawImage(menu.cframe, menu.cursor[0], x1, yp);
		Menu_FrameDrawImage(menu.cframe, menu.cursor[1], x2, yp);
	}

	Menu_FrameEnd();
}



int Menu_Option(word index, const char *text)
{
	if( !(menu.cframe->opt_drawn & (1<<index)) )
	{
		Menu_OptionHide(index);

		Menu_PrepareTextMask(text);

		word yp = Menu_OptionComputeYP(index);
		word xp = (320-menu.textbuff_len)>>1;
		Menu_PlaceText(menu.cframe, xp, yp);

		menu.opt_margin[index] = xp;
		xp = 320 - (xp+menu.textbuff_len);
		if( xp < menu.opt_margin[index] )
			menu.opt_margin[index] = xp;

		menu.cframe->opt_drawn |= (1<<index);
		menu.cframe->opt_dirty |= (1<<index);
	}

	if( index == menu.opt_selection )
		if( io_key_last == KEYSCAN_ENTER || io_key_last == KEYSCAN_SPACE )
			if( (menu.opt_inactive & (1<<index))==0 )
			{
				io_key_last = 0;
				Sound_PlayThingSound(NULL, TRKSOUND_DSWPNUP, 0);
				return 1;
			}

	return 0;
}

// Option core
int Menu_Option_V(word index, const char *text, const char *value)
{
	int step = 0;

	if( !(menu.cframe->opt_drawn & (1<<index)) )
	{
		Menu_OptionHide(index);

		word yp = Menu_OptionComputeYP(index);

		Menu_PrepareTextMask(text);
		Menu_PlaceText(menu.cframe, MENU_OPTION_VALUE_MARGIN, yp);

		Menu_PrepareTextMask(value);
		Menu_PlaceText(menu.cframe, 320 - (MENU_OPTION_VALUE_MARGIN) - menu.textbuff_len, yp);

		menu.opt_margin[index] = MENU_OPTION_VALUE_MARGIN;
		menu.cframe->opt_drawn |= (1<<index);
		menu.cframe->opt_dirty |= (1<<index);
	}

	if( index == menu.opt_selection )
	{
		if( io_key_last == KEYSCAN_ENTER ||
			io_key_last == KEYSCAN_SPACE ||
			io_key_last == KEYSCAN_RIGHT || 
			io_key_last == KEYSCAN_D )
		{
			io_key_last = 0;
			menu.frame[0].opt_drawn &= ~(1<<index);
			menu.frame[1].opt_drawn &= ~(1<<index);
			step += 1;
		}

		if( io_key_last == KEYSCAN_LEFT ||
			io_key_last == KEYSCAN_A )
		{
			io_key_last = 0;
			menu.frame[0].opt_drawn &= ~(1<<index);
			menu.frame[1].opt_drawn &= ~(1<<index);
			step -= 1;
		}

		if( step )
		{
			Sound_PlayThingSound(NULL, TRKSOUND_DSAMMO, 0);
		}
	}

	return step;
}

// Integer option
int Menu_Option_VI(word index, const char *text, short *value, int min, int max)
{
	int delta = Menu_Option_V(index, text, _itoa(*value));
	delta += *value;
	if( delta<min ) delta = min;
	if( delta>max ) delta = max;
	if( (short)delta != *value )
	{
		*value = (short)delta;
		return 1;
	}
	return 0;
}

// Enum option
int Menu_Option_VE(word index, const char *text, short *value, const char *VTAB[], int max)
{
	int delta = Menu_Option_V(index, text, VTAB[*value]);
	delta += *value;
	if( delta<0 ) delta = max;
	if( delta>max ) delta = 0;
	if( (short)delta != *value )
	{
		*value = (short)delta;
		return 1;
	}
	return 0;
}

// Level stat "option"
int Menu_Option_Stat(word index, const char *text, int count, int max)
{
	char buff[11+11+3+1];
	char *s = buff;
	s = _append_s(s, _itoa(count));
	s = _append_s(s, " / ");
	s = _append_s(s, _itoa(max));
	return Menu_Option_V(index, text, buff);
}



// ================================================================ Logic ================================================================

#if ENABLE_MAP_FILE_LOADING
void Menu_MapLoadError(void)
{
	Menu_OptionReset();
	menu.opt_selection = -1;

	while( Menu_OptionBegin(4) )
	{
		word pos = 0;
		Menu_Option(pos++, "MAP LOAD ERROR");
		pos++;
		Menu_Option_V(pos++, "CODE", _itoa(map_error_code));
		Menu_Option_V(pos++, "OFFSET", _itoa_hex(map_error_offset));
		Menu_OptionEnd();
	}
}
#endif


word Menu_SelectSkill(void)
{
	word action = 0;
	Menu_OptionReset();
	menu.opt_selection = e_globals.skill;
	while( Menu_OptionBegin(5) )
	{
		action = 0;
		if( Menu_Option(0, "VERY EASY"	) ) e_globals.skill = 0, action = 1;
		if( Menu_Option(1, "EASY"		) ) e_globals.skill = 1, action = 1;
		if( Menu_Option(2, "NORMAL"		) ) e_globals.skill = 2, action = 1;
		if( Menu_Option(3, "HARD"		) ) e_globals.skill = 3, action = 1;
		if( Menu_Option(4, "NIGHTMARE"	) ) e_globals.skill = 4, action = 1;
		Menu_OptionEnd();

		if( action )
			break;
	}

	return action;
}

void Menu_RunOptions(void)
{
	word action = 0;
	Menu_OptionReset();
	while( Menu_OptionBegin(5) )
	{
		action = 0;
		
		Menu_Option_VE(0, "SCREEN MODE", &opt_screen_mode, SCREEN_MODES_NAMES, SCREEN_MODE_MAX);
		
		if( Menu_Option_VE(1, "SOUND MODE", &aud_mode, AUDIO_MODES_NAMES, AUD_MODE_MAX) )
		{
			Aud_Reset();
			Sound_PlayThingSound(NULL, TRKSOUND_DSAMMO, 0);
		}

		if( Menu_Option_VE(2, "SOUND FILTER", &aud_filter_mode, AUDIO_FILTER_NAMES, 1) )
			Aud_Filter(aud_filter_mode);

		if( Menu_Option_VI(3, "MOUSE SENS", &opt_mouse_sens_index, 0, MOUSE_SENS_MAX) )
			opt_mouse_sens = MOUSE_SENS_TABLE[opt_mouse_sens_index];

		if( Menu_Option(4, "BACK") ) action = 1;
		
		Menu_OptionEnd();


		if( action==1 )
			break;
	}
}

void Menu_RunCredits(void)
{
	Menu_OptionReset();
	menu.opt_selection = -1;

	while( Menu_OptionBegin(8) )
	{
		word pos = 0;
		pos++;
		Menu_Option(pos++, "THE DREAD IS CREATED BY:");
		pos++;
		Menu_Option_V(pos++, "CODE/DESIGN", "KK/ALTAIR");
		Menu_Option_V(pos++, "GFX", "JOHN TSAKIRIS");
		Menu_Option_V(pos++, "", "DENNIS RAMBERG");
		Menu_Option_V(pos++, "", "THE FREEDOOM PROJECT");
		Menu_OptionEnd();
	}
}

void Menu_ShowTextFile(void)
{
	Menu_OptionReset();
	menu.opt_selection = -1;

	while( Menu_OptionBegin(8) )
	{
		word pos = 0;

		char *s = temp_text_buffer;
		while( *s && pos<8 )
		{
			const char *b = s;
			while( *s>=' ' ) s++;
			char fix = *s;

			*s = 0;
			if( s>b ) Menu_Option(pos++, b);
			*s = fix;

			while( *s && *s<' ' ) s++;
		}

		Menu_OptionEnd();
	}
}

#if ENABLE_MAP_FILE_LOADING
int Menu_SelectAndLoadMap(void)
{
	Menu_OptionReset();
	menu.opt_selection = -1;

	word files_show = n_files_dir;
	word scroll_pos = 0;
	word map_loaded = 0;

	if( files_show > 7 )
		files_show = 7;

	while( Menu_OptionBegin(files_show+1) )
	{
		word pos = 0;
		short action = -1;
		Menu_Option(pos++, "DATA DIRECTORY:");

		for( int i=0; i<files_show; i++ )
		{
			const char *s = files_dir[i].path;
			char *d = temp_text_buffer;
		
			while( *s )
			{
				if( *s>='a' && *s<='z' )	*d++ = *s++ - 'a' + 'A';
				else						*d++ = *s++;
			}
			*d = 0;
		
			if( Menu_Option(pos++, temp_text_buffer) )
				action = i;
		}
		Menu_OptionEnd();

		if( action >= 0 )
		{
			// Bring back the system
			Menu_SystemRestore();

			// Prepare file path
			const char *path = File_CreatePath("data/", files_dir[action].path);

			// Read the file
#if SHOW_TEXT_FILES
			const char *text = File_ReadTextFile(path);
#else
			// load game maps
			if( Map_FileLoadAll(e_globals.skill, path) )
				map_loaded = 1;
#endif

			// Shutdown the system & reinitialize
			Menu_GameRestore();

#if SHOW_TEXT_FILES
			// Show file contents
			if( text[0] )
			{
				Menu_ShowTextFile();
				Menu_OptionReset();
			}
#else
			if( map_loaded )
				break;
			else
			{
				Menu_MapLoadError();
				Menu_OptionReset();
			}
#endif
		}
	}

	return map_loaded;
}
#endif


int Menu_RunMainMenu(void)
{
	word action = 255;

	Menu_OptionReset();

	while( 1 )
	{
		word run = Menu_OptionBegin(5);

		if( action == 255 )
		{
			//menu.opt_inactive = 0x0010;	// no EXIT
			if( e_n_things ) menu.opt_selection = 1;
			else			 menu.opt_inactive |= 0x0002;	// no RESUME
		}

		action = 0;
		if( Menu_Option(0, "NEW GAME"	) ) action = 1;
		if( Menu_Option(1, "RESUME"		) ) action = 2;
		if( Menu_Option(2, "OPTIONS"	) ) action = 3;
		if( Menu_Option(3, "CREDITS"	) ) action = 4;
		if( Menu_Option(4, "EXIT"		) ) action = 5;
		Menu_OptionEnd();

		// NEW GAME
		if( action==1 )
		{
			if( !Menu_SelectSkill() )
				continue;

#if ENABLE_MAP_FILE_LOADING
			if( Menu_SelectAndLoadMap() )
				break;		// done - run the map
#else
			if( Map_LoadAll(e_globals.skill) )
				break;		// done - run the map
#endif
			// Map loaded with error
			Menu_MapLoadError();
			Menu_OptionReset();
			action = 255;
		}

		// RESUME
		if( (action==2 || !run) && e_n_things )
			break;

		// OPTIONS
		if( action==3 )
		{
			Menu_RunOptions();
			Menu_OptionReset();
			action = 255;
		}

		// CREDITS
		if( action==4 )
		{
			Menu_RunCredits();
			Menu_OptionReset();
			action = 255;
		}

		// EXIT
		if( action==5 )
			break;
	}

	//Menu_Option_V(5, "BANK A", _itoa(menu.debug1));
	//Menu_Option_V(6, "BANK B", _itoa(menu.debug2));

	return (action==5) ? 1 : 0;		// action==5 - EXIT
}

void Menu_LevelStats(void)
{
	Menu_OptionReset();
	menu.opt_selection = -1;

	while( Menu_OptionBegin(10) )
	{
		word pos = 0;
		pos++;
		Menu_Option(pos++, "FINISHED");
		Menu_Option(pos++, "THE DEMO BASE");
		pos++;
		Menu_Option_Stat(pos++, "MONSTERS", e_globals.levstat_monsters, e_globals.levstat_monsters_max);
		Menu_Option_Stat(pos++, "ITEMS", e_globals.levstat_items, e_globals.levstat_items_max);
		Menu_Option_Stat(pos++, "SECRETS", e_globals.levstat_secrets, e_globals.levstat_secrets_max);
		pos++;
		Menu_Option_V(pos++, "TIME", _time_100s_a(e_globals.levstat_time/3));
		Menu_Option_V(pos++, "PAR", "1:30");
		Menu_OptionEnd();
	}
}



const byte INTRO_SHAKEPOS[] = {
	0, 1,
	3, 2,
	1, 1,
	2
};

void Menu_Intro(void)
{
	word time = 0;
	short shakepos = 100;
	short soundplay = -1;

	Menu_FillBackgrounds();

	while(time<=16)
	{
		for( int i=0; i<32; i++ )
		{
			word rb = PALETTE_MENU[i] & 0x0F0F;
			word g  = PALETTE_MENU[i] & 0x00F0;
			COLOR[i] = ((rb*time)&0xF0F0 | (g*time)&0x0F00)>>4;
		}
		SetCopper(menu.frame[0].copper);
		intnum = 0;
		while(!intnum){}
		time++;
	}

	intnum = 0;
	time = 0;

	const short ystart = -32;
	const short ymid = (233-27)/2+2;
	const short tdrop = 17;					// TBD: PAL NTSC
	const short trisestart = tdrop*6;
	const short triseend = trisestart + tdrop*2;
	const short texit = triseend + tdrop;

	io_key_last = 0;

	while( !io_key_last && time<texit )
	{
		word delta = Menu_FrameBegin();

		time += delta;
		shakepos += delta;
		if( shakepos >= TABLE_LENGTH(INTRO_SHAKEPOS) )
			shakepos = TABLE_LENGTH(INTRO_SHAKEPOS)-1;

		for( word i=0; i<5; i++ )
		{
			short yp = ystart;
			
			if( time < trisestart )
			{
				// drop & shake
				short t = time - i*tdrop;
				if( t<0 ) continue;

				if( t>=tdrop ) yp = ymid;
				else yp = (short)(((long)(t*t))*(ymid+32)/(tdrop*tdrop))-32;

				if( t-tdrop>=0 && t-tdrop<shakepos )
					shakepos = t-tdrop;

				if( t-tdrop>=0 && soundplay<(short)i )
				{
					Sound_PlayThingSound(NULL, TRKSOUND_DSRLAUNC, 0);
					soundplay = (short)i;
				}
			}
			else if( time < triseend )
			{
				const short trisetime = triseend - trisestart;
				long t = time - trisestart;
				t = (3*trisetime-2*t)*t*t;
				yp = ymid + (20-ymid)*t/trisetime/trisetime/trisetime;

				if( soundplay<6 )
				{
					Sound_PlayThingSound(NULL, TRKSOUND_DSWEAP1, 0);
					soundplay = 6;
				}
			}
			else
				yp = 20;

			// Logo placement: 64+39, 20	(39x32)
			Menu_FrameDrawImage(menu.cframe, menu.title[i], 64+39*i, yp);
		}

		menu.view_start = INTRO_SHAKEPOS[shakepos];

		Menu_FrameEnd();
	}

	if( io_key_last )
	{
		Sound_PlayThingSound(NULL, TRKSOUND_DSWEAP1, 0);
		io_key_last = 0;
	}

	if( time >= triseend )
	{
		menu.frame[0].num_dirty = 0;
		menu.frame[1].num_dirty = 0;
	}
	else
	{
		Menu_ResetFrame(&menu.frame[0]);
		Menu_ResetFrame(&menu.frame[1]);
		Menu_DrawLogo();
	}
}

#if 0
void Menu_MakeText()
{
	//COLOR31 = 0xF0F;
	Menu_FillBackgrounds();
	Menu_OptionReset();
	menu.opt_selection = -1;

	while( Menu_OptionBegin(0) )
	{
		//word pos = 0;
		//Menu_Option(pos++, "DISK 1");
		Menu_OptionEnd();
	}
}
#endif

int Menu_Run(void)
{
	Menu_Init();
	//Menu_MakeText();

	if( e_globals.game_state == GAMESTATE_NONE )
	{
		Menu_Intro();
	}
	else
	{
		Menu_FillBackgrounds();
		Menu_DrawLogo();
		SetCopper(menu.frame[0].copper);
	}

	if( e_globals.game_state == GAMESTATE_LEVEL_DONE )
	{
		e_globals.game_state = GAMESTATE_NONE;		// show only once
		Menu_LevelStats();
	}

#ifdef COPYRIGHT_1
	Menu_PrepareTextMask(COPYRIGHT_1);
	Menu_PlaceText(&menu.frame[0], (320-menu.textbuff_len)>>1, 233-MENU_FONT_LINETOTAL*3);
	Menu_PlaceText(&menu.frame[1], (320-menu.textbuff_len)>>1, 233-MENU_FONT_LINETOTAL*3);
#endif

#ifdef COPYRIGHT_2
	Menu_PrepareTextMask(COPYRIGHT_2);
	Menu_PlaceText(&menu.frame[0], (320-menu.textbuff_len)>>1, 233-MENU_FONT_LINETOTAL*2);
	Menu_PlaceText(&menu.frame[1], (320-menu.textbuff_len)>>1, 233-MENU_FONT_LINETOTAL*2);
#endif

#ifdef COPYRIGHT_3
	Menu_PrepareTextMask(COPYRIGHT_3);
	Menu_PlaceText(&menu.frame[0], (320-menu.textbuff_len)>>1, 233-MENU_FONT_LINETOTAL);
	Menu_PlaceText(&menu.frame[1], (320-menu.textbuff_len)>>1, 233-MENU_FONT_LINETOTAL);
#endif


	int status = Menu_RunMainMenu();

	// exit
	SetCopper(BLANK_COPPER);

	return status;
}
