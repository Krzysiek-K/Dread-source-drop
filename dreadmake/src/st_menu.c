
#include "demo.h"


//#define COPYRIGHT_2			"PIXEL HEAVEN 2021 EDITION"
//#define COPYRIGHT_3			"VER 1A08 (C) KK/ALTAIR 2021"

//#define COPYRIGHT_3			"VER 1B04 (C) KK/ALTAIR 2021"

#define COPYRIGHT_2				"SILLY VENTURE 2021 DEATHMATCH"
#define COPYRIGHT_3				"VER SV21 (C) KK/ALTAIR 2021"



#define LOGO_FINAL_YPOS				8



extern word SOUND_DSSELECT[];
extern word SOUND_DSAMMO[];
extern word SOUND_DSWPNUP[];
extern word SOUND_DSRLAUNC[];
extern word SOUND_DSWEAP1[];




// ================================================================ Game options ================================================================

#define MOUSE_SENS_DEFAULT		10
#define MOUSE_SENS_MAX			25

short opt_mouse_sens_index;
short opt_mouse_sens;
word opt_screen_mode;
word opt_sfx_enable;
word opt_msx_enable;

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

const char *OFF_ON[] ={
	"OFF",
	"ON",
};

const char *SCREEN_MODES_NAMES[] ={
	"DEFAULT",
	"SCANLINES",
};



// ================================================================ Menu structures ================================================================

MenuInfo menu;


extern const word GFX_ST_MENUBACK[];
extern const word GFX_ST_TITLE1[];
extern const word GFX_ST_TITLE2[];
extern const word GFX_ST_TITLE3[];
extern const word GFX_ST_TITLE4[];
extern const word GFX_ST_TITLE5[];
extern const word GFX_ST_CURSOR1[];
extern const word GFX_ST_CURSOR2[];
extern const word GFX_MenuFont[];
extern const word PALETTE_MENU[];

///*const*/ word MENU_PAL[16] ={
//	0x000>>1,		//  0
//	0x200>>1,		//  1
//	0x222>>1,		//  2
//	0x444>>1,		//  3
//	0x888>>1,		//  4
//	0xAAA>>1,		//  5
//	0x242>>1,		//  6
//	0x664>>1,		//  7
//	0x644>>1,		//  8
//	0x400>>1,		//  9
//	0x420>>1,		// 10
//	0x600>>1,		// 11
//	0x800>>1,		// 12
//	0xc00>>1,		// 13
//	0x666>>1,		//
//	0xFFF>>1,		//
//};

//					Normal		Old sel		Inactive	Selected
//	Shadow			 9 400		 3 444		 2 222		14 666
//	Highlight		12 800		 4 888		14 666		 5 AAA
//	Face			11 600		14 666		 3 444		 4 888


//		Menu_TextLayer(frame, xp+2, yp+4, 0);	//	0x000,	//	 0				0x000>>1,		//  0
//		Menu_TextLayer(frame, xp, yp+2, 13);	//	0x400,	//	13				0x400>>1,		//  9		1001			 3	0011		2
//		Menu_TextLayer(frame, xp, yp, 15);		//	0x900,	//	15				0x800>>1,		// 12		1100			 4	0100		14
//		Menu_TextLayer(frame, xp, yp+1, 14);	//	0x700,	//	14				0x600>>1,		// 11		1011			14	0111		3




// ================================================================ Helper functions ================================================================


word Menu_OptionComputeYP(word index)
{
	return ((200 - MENU_FONT_LINESPACING*menu.opt_rowcount)>>1) + index * MENU_FONT_LINESPACING + 10;
}




// ================================================================ Option functions ================================================================


void Options_Reset(void)
{
	opt_mouse_sens_index = MOUSE_SENS_DEFAULT;
	opt_mouse_sens = MOUSE_SENS_TABLE[opt_mouse_sens_index];
	opt_screen_mode = 0;
	opt_sfx_enable = 1;
	opt_msx_enable = 0;		// TBD: 1
}




// ================================================================ Drawing ================================================================


// X coords are in 16px blocks	[0.. 20]
// Y coords are in 1px lines	[0..236]
void Menu_ResetBlock(word *screen, word x1, word y1, word x2, word y2)
{
	ST_DrawResetBlockAsm(screen, GFX_ST_MENUBACK, x1, y1, x2, y2);
}


void Menu_DrawLogo(void)
{
	for( int i=0; i<5; i++ )
	{
		// Logo placement: 64+39, 20	(39x32)
		ST_DrawMasked(screen, menu.title[i], 64+39*i, LOGO_FINAL_YPOS);
		ST_DrawMasked(screen_active, menu.title[i], 64+39*i, LOGO_FINAL_YPOS);
	}
}

void Menu_FillBackgrounds(void)
{
	// fill background
	memcpy(menu.frame[0].screen, GFX_ST_MENUBACK, 320*200*4/8);
	memcpy(menu.frame[1].screen, GFX_ST_MENUBACK, 320*200*4/8);
	//asm_memcpy(menu.frame[0].screen, GFX_ST_MENUBACK, 320*200*4/8);
	//asm_memcpy(menu.frame[1].screen, GFX_ST_MENUBACK, 320*200*4/8);
	menu.frame[0].num_dirty = 0;
	menu.frame[1].num_dirty = 0;
}



// ================================================================ Text ================================================================


// Color sets:
//	0	- normal
//	1	- selected
//	2	- inactive
//
void Menu_DrawText(const char *s, MenuFrameInfo *frame, word xp, word yp, word justify, word color_set)
{
	word h = GFX_MenuFont[0];
	const word *font_base = GFX_MenuFont + 3 + GFX_MenuFont[2];

	// compute size
	const char *begin = s;
	word len = 0;
	while( *s )
	{
		word ch = *s++ - GFX_MenuFont[1];

		if( ch >= GFX_MenuFont[2] )
			len += MENU_FONT_SPACE_WIDTH;
		else
			len += GFX_MenuFont[3+ch] + MENU_FONT_SPACING;
	}
	menu.textbuff_len = len;

	if( justify == 1 ) xp -= len>>1;
	if( justify == 2 ) xp -= len;

	// print
	s = begin;
	while( *s )
	{
		word ch = *s++ - GFX_MenuFont[1];

		if( ch >= GFX_MenuFont[2] )
		{
			xp += MENU_FONT_SPACE_WIDTH;
			continue;
		}

		const word *glyph = font_base + ch*h;

		ST_DrawDreadGlyphShadow(frame->screen, glyph, xp+2, yp+4, h);
		ST_DrawDreadGlyph(frame->screen, glyph, xp, yp, h, color_set);

		xp += GFX_MenuFont[3+ch] + MENU_FONT_SPACING;
	}
}





// ================================================================ Frame handling ================================================================


void Menu_InitFrame(MenuFrameInfo *frame, word *screen, word *copper)
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

void Menu_FrameDrawImage(MenuFrameInfo *frame, const word *image, word xp, short yp)
{
	ST_DrawMasked(frame->screen, image, xp, yp);

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
	// Screen swap
	set_supervisor();
	VID_SET_SCREEN(menu.cframe->screen);
	set_usermode();

	word waitnum = intnum;
	while( intnum==waitnum ) {}
	menu.frameflip ^= 1;
}



// ================================================================ Initialization ================================================================


void Menu_Init(void)
{
	memset(&menu, 0, sizeof(menu));
	menu.textbuff_len = 320;
	menu.frameflip = 0;
	menu.opt_selection = 0;
	menu.opt_inactive = 0;
	menu.view_start = 2;

	vblank_pal = PALETTE_MENU;
	vblank_hudpal = NULL;

//	menu.bloodmap = Menu_LoadImage(&alloc_A, GFX_BLOOD_MAP, 1) + 3;		// skip header
	menu.title[0] = GFX_ST_TITLE1;
	menu.title[1] = GFX_ST_TITLE2;
	menu.title[2] = GFX_ST_TITLE3;
	menu.title[3] = GFX_ST_TITLE4;
	menu.title[4] = GFX_ST_TITLE5;
//	menu.cursor[0] = Menu_LoadImage(&alloc_A, GFX_CURSOR1, 6);
//	menu.cursor[1] = Menu_LoadImage(&alloc_B, GFX_CURSOR2, 6);
//	menu.fontmask = Menu_LoadFontMasks(&alloc_A, GFX_MenuFont);
//
//	for( int plane=0; plane<4; plane++ )
//		menu.backrow[plane] = Menu_LoadTileRow(&alloc_B, GFX_MENU_TEXTURE, plane);
//
//	menu.textbuff = alloc_A;	alloc_A += 440/2;

	// reset internal structures
	Menu_InitFrame(&menu.frame[0], screen, NULL);
	Menu_InitFrame(&menu.frame[1], screen_active, NULL);

//	// create copper lists (in chip memory)
//	alloc_A = Menu_GenCopper(&menu.frame[0]);
//	alloc_B = Menu_GenCopper(&menu.frame[1]);
//
//	menu.debug3 = (alloc_A - (word*)blockmem1)<<1;

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
			menu.frame[0].opt_drawn &= ~(3<<menu.opt_selection);
			menu.frame[1].opt_drawn &= ~(3<<menu.opt_selection);
			Sound_PlayThingSound(NULL, SOUND_DSSELECT, 0);
		}
		io_key_last = 0;
	}

	if( io_key_last==KEYSCAN_DOWN || io_key_last==KEYSCAN_S )
	{
		if( menu.opt_selection < rowcount-1 )
		{
			menu.frame[0].opt_drawn &= ~(3<<menu.opt_selection);
			menu.frame[1].opt_drawn &= ~(3<<menu.opt_selection);
			menu.opt_selection++;
			Sound_PlayThingSound(NULL, SOUND_DSSELECT, 0);
		}
		io_key_last = 0;
	}

	if( io_key_last == KEYSCAN_ESC )
	{
		Sound_PlayThingSound(NULL, SOUND_DSAMMO, 0);
		io_key_last = 0;
		return 0;
	}

	return 1;
}

void Menu_OptionEnd()
{
	if( menu.opt_selection>=0 && menu.opt_margin[menu.opt_selection]>=GFX_ST_CURSOR1[0]+(2+16+MENU_CURSOR_SHADOW) )
	{
		word yp = Menu_OptionComputeYP(menu.opt_selection) + (MENU_FONT_LINETOTAL-MENU_FONT_LINESHADOW)/2 - ((GFX_ST_CURSOR1[1]-MENU_CURSOR_SHADOW+1)>>1);
		word anim = (rawintnum>>2)&3;
		if( anim==3 ) anim = 1;

		word x1 = menu.opt_margin[menu.opt_selection] - 16 - anim - GFX_ST_CURSOR1[0];
		word x2 = 320 - (menu.opt_margin[menu.opt_selection] - 16 - anim - MENU_CURSOR_SHADOW);

		Menu_FrameDrawImage(menu.cframe, GFX_ST_CURSOR1, x1, yp);
		Menu_FrameDrawImage(menu.cframe, GFX_ST_CURSOR2, x2, yp);
	}

	Menu_FrameEnd();
}



int Menu_Option(short index, const char *text)
{
	word color_set = 0;
	if( index<0 )
	{
		color_set = 1;
		index = ~index;
	}

	if( !(menu.cframe->opt_drawn & (1<<index)) )
	{
		Menu_OptionHide(index);

		word yp = Menu_OptionComputeYP(index);
		if( menu.opt_selection==index ) color_set = 1;
		if( menu.opt_inactive & (1<<index) ) color_set = 2;

		Menu_DrawText(text, menu.cframe, 320/2, yp, 1, color_set);

		menu.opt_margin[index] = (320-menu.textbuff_len)>>1;
		word xp = 320 - (xp+menu.textbuff_len);
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
				Sound_PlayThingSound(NULL, SOUND_DSWPNUP, 0);
				return 1;
			}

	return 0;
}

// Option core
int Menu_Option_V(short index, const char *text, const char *value)
{
	word color_set = 0;
	int step = 0;

	if( index<0 )
	{
		color_set = 1;
		index = ~index;
	}

	if( !(menu.cframe->opt_drawn & (1<<index)) )
	{
		Menu_OptionHide(index);

		word yp = Menu_OptionComputeYP(index);
		if( menu.opt_selection==index ) color_set = 1;
		if( menu.opt_inactive & (1<<index) ) color_set = 2;

		Menu_DrawText(text, menu.cframe, MENU_OPTION_VALUE_MARGIN, yp, 0, color_set);
		Menu_DrawText(value, menu.cframe, 320 - (MENU_OPTION_VALUE_MARGIN), yp, 2, color_set);

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
			Sound_PlayThingSound(NULL, SOUND_DSAMMO, 0);
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


void Menu_MapLoadError(void)
{
	Menu_OptionReset();
	menu.opt_selection = -1;

	while( Menu_OptionBegin(5) )
	{
		word pos = 0;
		Menu_Option(pos++, "MAP LOAD ERROR");
		pos++;
		Menu_Option_V(pos++, "CODE", _itoa(map_error_code));
		Menu_Option_V(pos++, "OFFSET", _itoa_hex(map_error_offset));
		Menu_Option_V(pos++, "ARG", _itoa_hex(map_error_arg));
		Menu_OptionEnd();
	}
}


word Menu_SelectSkill(void)
{
	word action = 0;
	Menu_OptionReset();
	menu.opt_selection = e_globals.skill;
	while( Menu_OptionBegin(5) )
	{
		action = 0;
		if( Menu_Option(0, "VERY EASY") ) e_globals.skill = 0, action = 1;
		if( Menu_Option(1, "EASY") ) e_globals.skill = 1, action = 1;
		if( Menu_Option(2, "NORMAL") ) e_globals.skill = 2, action = 1;
		if( Menu_Option(3, "HARD") ) e_globals.skill = 3, action = 1;
		if( Menu_Option(4, "NIGHTMARE") ) e_globals.skill = 4, action = 1;
		Menu_OptionEnd();

		if( action )
			break;
	}

	return action;
}

void Menu_RunOptions(void)
{
	word action = 0;
	word option;
	Menu_OptionReset();
	while( Menu_OptionBegin(5) )
	{
		action = 0;

		Menu_Option_VE(0, "SCREEN MODE", &opt_screen_mode, SCREEN_MODES_NAMES, SCREEN_MODE_MAX);

		if( Menu_Option_VI(1, "MOUSE SENS", &opt_mouse_sens_index, 0, MOUSE_SENS_MAX) )
			opt_mouse_sens = MOUSE_SENS_TABLE[opt_mouse_sens_index];

		// Little trick to make the changing sound use the new value
		option = opt_sfx_enable;
		opt_sfx_enable = 1-opt_sfx_enable;
		Menu_Option_VE(2, "SOUND FX", &option, OFF_ON, 1);
		opt_sfx_enable = option;

		if( Menu_Option_VE(3, "MUSIC", &opt_msx_enable, OFF_ON, 1) )
		{
			set_supervisor();
			if( opt_msx_enable )	init_music(1);
			else					init_music(0);
			set_usermode();
		}

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
		Menu_Option(~pos++, "THE DREAD IS CREATED BY:");
		pos++;
		Menu_Option_V(~pos++, "CODE/DESIGN", "KK/ALTAIR");
		Menu_Option_V(~pos++, "GFX", "JOHN TSAKIRIS");
		Menu_Option_V(~pos++, "", "DENNIS RAMBERG");
		Menu_Option_V(~pos++, "", "FREEDOOM");
		Menu_Option_V(~pos++, "MSX", "DMA-SC");
		Menu_OptionEnd();
	}
}


byte Menu_RunMainMenu(void)
{
	word action = 255;

	Menu_OptionReset();

	if( ENABLE_MAP_AUTOSTART )
		goto start_game_hack;

	while( 1 )
	{
		word run = Menu_OptionBegin(5);

		if( action == 255 )
		{
			menu.opt_inactive = 0x0010;	// no EXIT
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
		start_game_hack:
#if ENABLE_MAP_AUTOSTART >= 100
			e_globals.skill = ENABLE_MAP_AUTOSTART - 100;
#else
			if( !Menu_SelectSkill() )
			{
				Menu_OptionReset();
				continue;
			}
#endif

			if( Map_LoadAll(e_globals.skill) )
				break;		// done - run the map

			if( map_error_code )
			{
				// Map loaded with error
				Menu_MapLoadError();
				Menu_OptionReset();
				action = 255;
			}
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
			return 1;
	}

	return 0;
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


const byte INTRO_SHAKEPOS[] ={
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
	word TEMP_PAL[16];


	memset(TEMP_PAL, 0, sizeof(TEMP_PAL));
	vblank_pal = TEMP_PAL;
	vblank_hudpal = NULL;

	Menu_FillBackgrounds();

	while( time<=16 )
	{
		intnum = 0;
		while( !intnum ) {}
		for( int i=0; i<16; i++ )
		{
			word rb = PALETTE_MENU[i] & 0x0F0F;
			word g  = PALETTE_MENU[i] & 0x00F0;
			TEMP_PAL[i] = ((rb*time)&0xF0F0 | (g*time)&0x0F00)>>4;
		}
		time++;
	}

	intnum = 0;
	while( !intnum ) {}
	vblank_pal = PALETTE_MENU;
	time = 0;

	const short ystart = -32;
	const short ymid = (200-27)/2+2;
	const short tdrop = 17;					// TBD: PAL NTSC
	const short trisestart = tdrop*6;
	const short triseend = trisestart + tdrop*2;
	const short texit = triseend + tdrop;

	while( io_key_last!=KEYSCAN_ESC && time<texit )
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
					Sound_PlayThingSound(NULL, SOUND_DSRLAUNC, 0);
					soundplay = (short)i;
				}
			}
			else if( time < triseend )
			{
				const short trisetime = triseend - trisestart;
				long t = time - trisestart;
				t = (3*trisetime-2*t)*t*t;
				yp = ymid + (LOGO_FINAL_YPOS-ymid)*t/trisetime/trisetime/trisetime;

				if( soundplay<6 )
				{
					Sound_PlayThingSound(NULL, SOUND_DSWEAP1, 0);
					soundplay = 6;
				}
			}
			else
				yp = LOGO_FINAL_YPOS;

			// Logo placement: 64+39, 20	(39x32)
			Menu_FrameDrawImage(menu.cframe, menu.title[i], 64+39*i, yp);
		}

		menu.view_start = INTRO_SHAKEPOS[shakepos];

		Menu_FrameEnd();
	}

	if( io_key_last == KEYSCAN_ESC )
	{
		Sound_PlayThingSound(NULL, SOUND_DSWEAP1, 0);
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



byte Menu_Run(void)
{
	Menu_Init();

	if( e_n_things == 0 && opt_msx_enable )
	{
		set_supervisor();
		init_music(1);
		set_usermode();
	}


	if( e_globals.game_state == GAMESTATE_NONE && !ENABLE_MAP_AUTOSTART )
	{
		Menu_Intro();
	}
	else
	{
		Menu_FillBackgrounds();
		Menu_DrawLogo();
	}

	if( e_globals.game_state == GAMESTATE_LEVEL_DONE )
	{
		e_globals.game_state = GAMESTATE_NONE;		// show only once
		Menu_LevelStats();
	}

	// Color sets:
	//	0	- normal
	//	1	- selected
	//	2	- inactive
#ifdef COPYRIGHT_2
	Menu_DrawText(COPYRIGHT_2, &menu.frame[0], 320/2, 200-MENU_FONT_LINETOTAL*2, 1, 1);
	Menu_DrawText(COPYRIGHT_2, &menu.frame[1], 320/2, 200-MENU_FONT_LINETOTAL*2, 1, 1);
#endif

#ifdef COPYRIGHT_3
	Menu_DrawText(COPYRIGHT_3, &menu.frame[0], 320/2, 200-MENU_FONT_LINETOTAL, 1, 1);
	Menu_DrawText(COPYRIGHT_3, &menu.frame[1], 320/2, 200-MENU_FONT_LINETOTAL, 1, 1);
#endif

	byte result = Menu_RunMainMenu();

	//static const word GLYPH[] = { 0xFF00, 0xFF80, 0xF3C0, 0xF1E0, 0xF1E0, 0xF1E0, 0xFFE0, 0xFFE0, 0xFFE0, 0xF1E0, 0xF1E0 };
	//
	//word pos = 0;
	//while( io_key_last!=KEYSCAN_ESC )
	//{
	//	word delta = Menu_FrameBegin();
	//
	//	Menu_FrameDrawImage(menu.cframe, GFX_ST_TITLE1, pos%192, pos%101);
	//	Menu_FrameDrawImage(menu.cframe, GFX_ST_TITLE2, (pos+50)%192, (pos+40)%101);
	//	pos++;
	//	//Menu_FrameDrawImage(menu.cframe, GFX_ST_TITLE2, 123, -10);
	//
	//	//ST_DrawDreadGlyphShadow(menu.cframe->screen, GLYPH, 12+2, 12+4, 11);
	//	//ST_DrawDreadGlyph(menu.cframe->screen, GLYPH, 12, 12, 11);
	//	//ST_DrawDreadGlyph(menu.cframe->screen, GLYPH, (pos+50)%192, (pos+40)%101, 11);
	//
	//	//Menu_DrawText("HELLO WORLD!", menu.cframe, 0, 0, 0);
	//
	//	Menu_FrameEnd();
	//}

	io_key_last = 0;

	return result;
}
