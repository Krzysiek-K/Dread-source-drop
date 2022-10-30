
#include "demo.h"


#define MINIMAP_SCREEN_WIDTH			(336+112)
#define MINIMAP_SCREEN_HEIGHT			(233+112)
#define MINIMAP_SCREEN_WORDS			(MINIMAP_SCREEN_WIDTH/16*MINIMAP_SCREEN_HEIGHT)

#define MINIMAP_COPPER_OFFS_ADL_P0		1		// in words
#define MINIMAP_COPPER_OFFS_ADL_P1		5
#define MINIMAP_COPPER_OFFS_BPLCON1		13

#define MINIMAP_FETCH_PIXELS			(320+16)


short minimap_zoom = 64;
short minimap_xpos = 0;
short minimap_ypos = 0;
short minimap_xmid;
short minimap_ymid;
short minimap_xscreen;
short minimap_yscreen;
short minimap_screen_stride;
short minimap_xclip;
short minimap_yclip;

MinimapInfo minimap;




// returns copper end
word *Minimap_GenCopper(word **alloc, word *screen[])
{
	word *copper = *alloc;
	cop_write = copper;
	Cop_StartBit2(screen[0], screen[1]);
	Cop_Write(BPLCON1, 0x0000);
	Cop_NoSprites();

	word endy = cop_write_y + 233;

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

	Cop_SetBit0();
	Cop_End();

	*alloc = cop_write;

	return copper;
}


void Minimap_Init(void)
{
	memset(&minimap, 0, sizeof(minimap));
	minimap_zoom = 64;
	minimap_xpos = e_things[0].xp;
	minimap_ypos = e_things[0].yp;
	minimap_xscreen = MINIMAP_SCREEN_WIDTH;
	minimap_yscreen = MINIMAP_SCREEN_HEIGHT;
	minimap_xmid = minimap_xscreen/2;
	minimap_ymid = minimap_yscreen/2;
	minimap_screen_stride = minimap_xscreen/8;
	minimap_xclip = minimap_xscreen-1;
	minimap_yclip = minimap_yscreen-1;

	minimap.cam_xpos = e_things[0].xp;
	minimap.cam_ypos = e_things[0].yp;

	INTENA = 0x0040;			// disable blitter interrupts
	DMACON = 0x8000 | (1<<10);	// enable blitter nasty
	DMACON = 0x8000 | (1<< 7);	// enable copper
	DDFSTRT = 48;

	COLOR00 = 0x000;
	COLOR01 = 0xFFF;
	COLOR02 = 0x888;
	COLOR03 = 0x00F;

	BPL1MOD = (MINIMAP_SCREEN_WIDTH - MINIMAP_FETCH_PIXELS)/8;
	BPL2MOD = (MINIMAP_SCREEN_WIDTH - MINIMAP_FETCH_PIXELS)/8;

	// allocate chip memory
	word *alloc_A = (word*)blockmem1;
	word *alloc_B = (word*)blockmem2;

	for( int i=0; i<2; i++ )
	{
		screen_A[i] = alloc_A;
		alloc_A += MINIMAP_SCREEN_WORDS;

		screen_B[i] = alloc_B;
		alloc_B += MINIMAP_SCREEN_WORDS;

		BlitFill(screen_A[i], COMPUTE_BLTSIZE(MINIMAP_SCREEN_WIDTH/16, MINIMAP_SCREEN_HEIGHT), 0x0000);
		BlitFill(screen_B[i], COMPUTE_BLTSIZE(MINIMAP_SCREEN_WIDTH/16, MINIMAP_SCREEN_HEIGHT), 0x0000);
	}

	// create copper lists (in chip memory)
	copper_A = Minimap_GenCopper(&alloc_A, screen_A);
	copper_B = Minimap_GenCopper(&alloc_B, screen_B);

	minimap.frame[0].screen = screen_A;
	minimap.frame[0].copper = copper_A;

	minimap.frame[1].screen = screen_B;
	minimap.frame[1].copper = copper_B;

	minimap.drawframe = &minimap.frame[0];
	minimap.showframe = &minimap.frame[1];

	//Minimap_DrawLine(10, 10, 200, 100, screen_A[0]);
	//Minimap_DrawLineClip(10, 10, 200, 100, screen_A[0]);

}

void Minimap_UpdateCopper(MinimapFrame *frame)
{
	short x0 = (short)((((int)(minimap.cam_xpos - frame->xpos))*minimap_zoom)>>8) + (MINIMAP_SCREEN_WIDTH-320)/2;
	short y0 = (short)((((int)(minimap.cam_ypos - frame->ypos))*minimap_zoom)>>8) + (MINIMAP_SCREEN_HEIGHT-233)/2;
	if( x0<0 ) x0 = 0;
	if( x0>MINIMAP_SCREEN_WIDTH-320 ) x0 = MINIMAP_SCREEN_WIDTH-320;
	if( y0<0 ) y0 = 0;
	if( y0>MINIMAP_SCREEN_HEIGHT-233 ) y0 = MINIMAP_SCREEN_HEIGHT-233;

	word offs = ((x0>>4)<<1) + y0*(MINIMAP_SCREEN_WIDTH/8);
	frame->copper[MINIMAP_COPPER_OFFS_ADL_P0] = ((word)(int)frame->screen[0]) + offs;
	frame->copper[MINIMAP_COPPER_OFFS_ADL_P1] = ((word)(int)frame->screen[1]) + offs;

	x0 = (~x0)&15;
	frame->copper[MINIMAP_COPPER_OFFS_BPLCON1] = (word)(x0 | (x0<<4));
}

int Minimap_SoftInt(void)
{
	intnum = 0;

	IO_CheckMouse();
	minimap.cam_xpos += (((long)io_mouse_xdelta)<<8)/minimap_zoom;	io_mouse_xdelta = 0;
	minimap.cam_ypos += (((long)io_mouse_ydelta)<<8)/minimap_zoom;	io_mouse_ydelta = 0;


	Minimap_UpdateCopper(minimap.showframe);


	if( io_key_last == KEYSCAN_W )
	{
		if( minimap_zoom>1 ) minimap_zoom >>= 1;
		io_key_last = 0;
		return 1;
	}
	if( io_key_last == KEYSCAN_S )
	{
		if( minimap_zoom<256 ) minimap_zoom <<= 1;
		io_key_last = 0;
		return 1;
	}

	return 0;
}

void Minimap_Run(void)
{
	// init
	Minimap_Init();

	// loop
	io_key_last = 0;
	io_mouse_state = 0;
	while( 1 )
	{
		if( io_key_last==KEYSCAN_ESC || io_key_last==KEYSCAN_TAB ) break;

		MinimapFrame *frame = &minimap.frame[minimap.frameflip];
		minimap.drawframe = frame;

		word **screen = frame->screen;

		word linesdrawn = 0;
		while( linesdrawn < e_n_lines )
		{
			Minimap_SoftInt();

			linesdrawn = 0;
			minimap_xpos = frame->xpos = minimap.cam_xpos;
			minimap_ypos = frame->ypos = minimap.cam_ypos;

			BlitFill(screen[0], COMPUTE_BLTSIZE(MINIMAP_SCREEN_WIDTH/16, MINIMAP_SCREEN_HEIGHT), 0x0000);
			BlitFill(screen[1], COMPUTE_BLTSIZE(MINIMAP_SCREEN_WIDTH/16, MINIMAP_SCREEN_HEIGHT), 0x0000);

			for( ; linesdrawn<e_n_lines; linesdrawn++ )
			{
				if( intnum )
					if( Minimap_SoftInt() )
					{
						// restart
						linesdrawn = 0;
						break;
					}

				EngineLine *line = e_lines + linesdrawn;

				//if( CHEAT_MODES[line->modes[0]].clip==81 )
				//{
				//	Minimap_DrawLineClip(
				//		line->v1->xp,
				//		-line->v1->yp,
				//		line->v2->xp,
				//		-line->v2->yp,
				//		screen[0]);
				//}
				//else if( line->v1 < line->v2 )
				//{
				//	Minimap_DrawLineClip(
				//		line->v1->xp,
				//		-line->v1->yp,
				//		line->v2->xp,
				//		-line->v2->yp,
				//		screen[1]);
				//}
			}
		}

		Minimap_UpdateCopper(frame);
		SetCopper(frame->copper);
		intnum = 0;
		while( !intnum ) {}
		minimap.showframe = frame;

		minimap.frameflip ^= 1;
	}


	// exit
	SetCopper(BLANK_COPPER);
	io_mouse_state = 0;
	DDFSTRT = 56;
}
