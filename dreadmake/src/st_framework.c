
#include "demo.h"


// common with Amiga ramework
// dread_framework.c
word palette_index;
byte *render_buffer;

// main.c
int debug_vars[16];
word debug_target_index;
volatile byte io_keystate[128];
volatile byte io_key_last;
volatile byte io_mouse_state;
volatile short io_mouse_xdelta;
volatile word intnum;
volatile word rawintnum;
volatile const word *vblank_pal;
volatile const word *vblank_hudpal;
volatile const word *vblank_playsound[3];
volatile word vblank_playwait;



extern const word ST_CHUNKY_COLOR_PAIRS[];		// frame.inc



dword c2p_lut[0x10000];		// hhhhhh-- llllll--
dword *c2p_lut2;

word render_screen2[160*100/2];






void init_c2p_lut(void)
{
	c2p_lut2 = c2p_lut + 0x100/4;

#if 0
	for( int c0=0; c0<16; c0++ )
		for( int c1=0; c1<16; c1++ )
		{
			dword value = 0;

			if( c0&1 ) value |= 0xC0000000;
			if( c1&1 ) value |= 0x30000000;
			if( c0&2 ) value |= 0x00C00000;
			if( c1&2 ) value |= 0x00300000;
			if( c0&4 ) value |= 0x0000C000;
			if( c1&4 ) value |= 0x00003000;
			if( c0&8 ) value |= 0x000000C0;
			if( c1&8 ) value |= 0x00000030;

			//c2p_lut[(c0<<8) | c1] = value;
			//c2p_lut2[(c0<<8) | c1] = value>>4;

			int c0a = c0;	//((c0*0x11)>>2)&0x0F;
			int c1a = c1;	//((c1*0x11)>>2)&0x0F;
			word index = ((c0a<<8) + c1a + 0x2000)&0x3FFF;
			c2p_lut[index] = value;
			c2p_lut2[index] = value>>4;
		}
#else
	for( int c0=0; c0<64; c0++ )
		for( int c1=0; c1<64; c1++ )
		{
			dword value = 0;

			for( int i=0; i<2; i++ )
			{
				word dc = ST_CHUNKY_COLOR_PAIRS[i ? c1 : c0];
				if( dc&0x01 ) value |= 0x80000000>>(i*2);
				if( dc&0x02 ) value |= 0x00800000>>(i*2);
				if( dc&0x04 ) value |= 0x00008000>>(i*2);
				if( dc&0x08 ) value |= 0x00000080>>(i*2);

				if( dc&0x10 ) value |= 0x40000000>>(i*2);
				if( dc&0x20 ) value |= 0x00400000>>(i*2);
				if( dc&0x40 ) value |= 0x00004000>>(i*2);
				if( dc&0x80 ) value |= 0x00000040>>(i*2);
			}

			word index = ((c0<<8) + c1 + 0x2000)&0x3FFF;
			c2p_lut[index] = value;
			c2p_lut2[index] = value>>4;
		}
#endif
}


void perform_c2p(void)
{
	c2p_asm(render_screen, (byte*)screen, render_screen2, c2p_lut+0x8000/4, c2p_lut2+0x8000/4);
}
