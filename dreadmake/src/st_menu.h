
#ifndef _ST_MENU_H
#define _ST_MENU_H



// ================================================================ Game options ================================================================

#define SCREEN_MODE_DEFAULT			0
#define SCREEN_MODE_SCANLINES		1

#define SCREEN_MODE_MAX				1


extern short opt_mouse_sens_index;
extern short opt_mouse_sens;
extern word opt_screen_mode;
extern word opt_sfx_enable;
extern word opt_msx_enable;



void Options_Reset(void);



// ================================================================ Menu structures ================================================================

#define MENU_FONT_SPACE_WIDTH			 4
#define MENU_FONT_SPACING				 1
#define MENU_FONT_LINESPACING			18
#define MENU_FONT_LINETOTAL				(11+2+2)
#define MENU_FONT_LINESHADOW			2
#define MENU_CURSOR_SHADOW				2

#define MENU_MAX_OPTIONS				12
#define MENU_OPTION_VALUE_MARGIN		36



typedef struct _MenuDirtyRect {
	word	x1;		// 0..20 in 16px blocks
	word	x2;		// 0..20 in 16px blocks
	word	y1;		// 0..236 in 1px lines
	word	y2;		// 0..236 in 1px lines
} MenuDirtyRect;


typedef struct _MenuFrameInfo {
	word			*screen;
	word			*copper;
	MenuDirtyRect	dirty[5];
	word			num_dirty;
	word			opt_drawn;
	word			opt_dirty;
} MenuFrameInfo;

typedef struct _MenuInfo {
	const word		*title[5];			// chipram copy of DREAD letters
	//word			*backrow[4];		// 320x48 x 4bpp image of a single row of tiles
	//word			*bloodmap;			// chipram copy
	//word			*cursor[2];			// chipram copy of the cursors
	//word			*textbuff;			// one-line 320x11 buffer for compositing text line
	//word			*fontmask;			// font data (only the data)
	word			textbuff_len;		// number of pixels filled in textbuff

	MenuFrameInfo	frame[2];
	word			frameflip;
	MenuFrameInfo	*cframe;
	word			view_start;

	word			opt_rowcount;
	short			opt_selection;
	word			opt_margin[MENU_MAX_OPTIONS];	// number of free pixels on each side
	word			opt_inactive;

	int				debug1;
	int				debug2;
	int				debug3;
} MenuInfo;



byte Menu_Run(void);



#endif
