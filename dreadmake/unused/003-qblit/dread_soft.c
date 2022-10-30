
#include "demo.h"


#include "data/frame.inc"


#define SOFT_MAX_VERT		10
#define SOFT_MAX_LINES		10

int cam_pos_x;
int cam_pos_y;
short lrc_cam_pos_x;							// camera
short lrc_cam_pos_y;
short lrc_cam_pos_z;
short lrc_cam_angle;
word  lrc_n_vert = 0;							// data
short lrc_vert_x[SOFT_MAX_VERT];
short lrc_vert_y[SOFT_MAX_VERT];
word  lrc_n_lines = 0;
word  lrc_lines_v1[SOFT_MAX_LINES];
word  lrc_lines_v2[SOFT_MAX_LINES];
word  lrc_lines_tex[SOFT_MAX_LINES];
word  lrc_lines_tx2[SOFT_MAX_LINES];
short lrc_vert_final_x[SOFT_MAX_VERT];			// internal
short lrc_vert_final_y[SOFT_MAX_VERT];


void Dread_SoftInit(void)
{
	cam_pos_x = 0;
	cam_pos_y = 0;
	lrc_cam_pos_x = 0;							// camera
	lrc_cam_pos_y = 0;
	lrc_cam_pos_z = 40;
	lrc_cam_angle = 0;

	// test data
	lrc_n_vert = 0;
	lrc_n_lines = 0;

//	lrc_vert_x[lrc_n_vert] = -192;		lrc_vert_y[lrc_n_vert++] =  192;
//	lrc_vert_x[lrc_n_vert] = -192;		lrc_vert_y[lrc_n_vert++] = -192;
//	lrc_vert_x[lrc_n_vert] =  192;		lrc_vert_y[lrc_n_vert++] = -192;
//	lrc_vert_x[lrc_n_vert] =  192;		lrc_vert_y[lrc_n_vert++] =  192;
//
//	lrc_lines_v1[lrc_n_lines] = 0;		lrc_lines_v2[lrc_n_lines] = 3;		lrc_lines_tex[lrc_n_lines++] = 0;
//	lrc_lines_v1[lrc_n_lines] = 3;		lrc_lines_v2[lrc_n_lines] = 2;		lrc_lines_tex[lrc_n_lines++] = 1;
//	lrc_lines_v1[lrc_n_lines] = 2;		lrc_lines_v2[lrc_n_lines] = 1;		lrc_lines_tex[lrc_n_lines++] = 0;
//	lrc_lines_v1[lrc_n_lines] = 1;		lrc_lines_v2[lrc_n_lines] = 0;		lrc_lines_tex[lrc_n_lines++] = 0;

	lrc_vert_x[lrc_n_vert] = -192;		lrc_vert_y[lrc_n_vert++] =  192;
	lrc_vert_x[lrc_n_vert] =  192;		lrc_vert_y[lrc_n_vert++] =  192;
	lrc_vert_x[lrc_n_vert] =  192;		lrc_vert_y[lrc_n_vert++] =   32;
	lrc_vert_x[lrc_n_vert] =  192;		lrc_vert_y[lrc_n_vert++] =  -32;
	lrc_vert_x[lrc_n_vert] =  192;		lrc_vert_y[lrc_n_vert++] = -192;
	lrc_vert_x[lrc_n_vert] = -192;		lrc_vert_y[lrc_n_vert++] = -192;

	lrc_lines_v1[lrc_n_lines] = 0;		lrc_lines_v2[lrc_n_lines] = 1;		lrc_lines_tex[lrc_n_lines++] = TEX_RW23_4_OFFS;
	lrc_lines_v1[lrc_n_lines] = 1;		lrc_lines_v2[lrc_n_lines] = 2;		lrc_lines_tex[lrc_n_lines++] = TEX_RW23_4_OFFS;
	lrc_lines_v1[lrc_n_lines] = 2;		lrc_lines_v2[lrc_n_lines] = 3;		lrc_lines_tex[lrc_n_lines++] = TEX_DOOR3_6_OFFS;
	lrc_lines_v1[lrc_n_lines] = 3;		lrc_lines_v2[lrc_n_lines] = 4;		lrc_lines_tex[lrc_n_lines++] = TEX_RW23_4_OFFS;
	lrc_lines_v1[lrc_n_lines] = 4;		lrc_lines_v2[lrc_n_lines] = 5;		lrc_lines_tex[lrc_n_lines++] = TEX_RW23_4_OFFS;
	lrc_lines_v1[lrc_n_lines] = 5;		lrc_lines_v2[lrc_n_lines] = 0;		lrc_lines_tex[lrc_n_lines++] = TEX_RW23_4_OFFS;

	for( int i=0; i<lrc_n_lines; i++ )
	{
		// Length of(x, y) int vector as 24.8 fixpoint (1.12% max error):
		//	x=abs(x);
		//	y=abs(y);
		//	s=x+y;
		//	t=abs(x-y);
		//	return s*164 + t*72 + abs(s*15-t*34);

		int v1 = lrc_lines_v1[i];
		int v2 = lrc_lines_v2[i];
		int x = lrc_vert_x[v2] - lrc_vert_x[v1];
		int y = lrc_vert_y[v2] - lrc_vert_y[v1];
		if( x<0 ) x=-x;
		if( y<0 ) y=-y;
		int s = x+y;
		int t = x-y;
		if( t<0 ) t=-t;
		x = s*15-t*34;
		if( x<0 ) x=-x;
		int len = s*164 + t*72 + x;
		len = (len+128)>>8;
		if(len<1) len = 1;

		lrc_lines_tx2[i] = (word)len;
	}

	intnum = 0;
}



void Dread_LineWall_Fix(int p1x, int p1y, int p2x, int p2y, int tx1, int tx2, const byte *texture)
{
	const int WOLFSCREEN_W = 160;
	const int wolf_xoffs = 80;
	const int clip = 300;	// fix.4
	p1y -= clip;
	p2y -= clip;
	if( p1y<0 )
	{
		if( p2y<0 ) return;
		tx1 -= p1y*(tx1-tx2)/(p1y-p2y);
		p1x -= p1y*(p1x-p2x)/(p1y-p2y);
		p1y = 0;
		tx2 -= tx1&~63;
		tx1 &= 63;
	}
	else if( p2y<0 )
	{
		tx2 -= p2y*(tx1-tx2)/(p1y-p2y);
		p2x -= p2y*(p1x-p2x)/(p1y-p2y);
		p2y = 0;
	}
	p1y += clip;
	p2y += clip;

	const int zoom_i = 70;
	//const int xoffs = WOLFSCREEN_W/2;

	// xp = px*zoom / py
	//int xmin = (int)ceil( float(p1x)/p1y );
	//int xmax = (int)ceil( float(p2x)/p2y );
	int xmin = p1x*zoom_i/p1y + wolf_xoffs;
	int xmax = p2x*zoom_i/p2y + wolf_xoffs;
	if( xmin>=xmax ) return;

	short s1 = ((zoom_i*32)<<12)/p1y;		// won't overflow because of clipping p#y>=300
	short s2 = ((zoom_i*32)<<12)/p2y;
	short ds = (s2-s1)/(xmax-xmin);			// s# is signed word
	int u1s = tx1*s1;
	int u2s = tx2*s2;
	int du = (u2s-u1s)/(xmax-xmin);			// result can be 32-bit!!

	if( xmin<0 )
	{
		int d = -xmin;
		s1+=(s2-s1)*d/(xmax-xmin);
		u1s+=(u2s-u1s)*d/(xmax-xmin);
		xmin = 0;
	}
	if( xmax>WOLFSCREEN_W ) xmax = WOLFSCREEN_W;
	if( xmin>=xmax ) return;

	//tex <<= 6;

	short line_h1 = 64 - lrc_cam_pos_z;
	short line_h2 = 0 - lrc_cam_pos_z;
	for( int x=xmin; x<xmax; x++ )
	{
		int size = s1>>8;
		if( size<=1 ) size = 1;
		if( size>=100 ) size = 100;
		int tx = (u1s / s1)&63;
		
		const byte *sky = SKYTEX_DATA + (((x+(lrc_cam_angle<<1))&127)<<6);
		const byte *src = texture + tx;
		byte *dst = render_buffer + (x>>2) + (x&3)*4000;

		//COLBLITS[size](dst, src, 0x40, 0xC0);		// 01000000	11000000
		COLBLITS[size](dst, src, sky, 0xC0);


		s1 += ds;
		u1s += du;
	}
}

void Dread_SoftTest(void)
{
	// camera
	word ticks = intnum*16;
	intnum = 0;

	lrc_cam_angle = debug_vars[0]>>4;
	lrc_cam_angle &= 0xFF;
	short mwalk = 0;
	short mside = 0;
	if( keystate[0x11] ) mwalk += ticks;
	if( keystate[0x21] ) mwalk -= ticks;
	if( keystate[0x20] ) mside -= ticks;
	if( keystate[0x22] ) mside += ticks;
	cam_pos_x += ((int)sincos_fix14[lrc_cam_angle])*mwalk;
	cam_pos_y += ((int)sincos_fix14[(lrc_cam_angle+64)&0xFF])*mwalk;

	cam_pos_x += ((int)sincos_fix14[(lrc_cam_angle+64)&0xFF])*mside;
	cam_pos_y -= ((int)sincos_fix14[lrc_cam_angle])*mside;

	lrc_cam_pos_x = (short)(cam_pos_x>>12);
	lrc_cam_pos_y = (short)(cam_pos_y>>12);

	// precompute vertexes
	{
		short dx = sincos_fix14[(lrc_cam_angle+64)&0xFF];
		short dy = sincos_fix14[(lrc_cam_angle)&0xFF];
		for( int id=0; id<lrc_n_vert; id++ )
		{
			short xp = (lrc_vert_x[id]<<4) - lrc_cam_pos_x;
			short yp = (lrc_vert_y[id]<<4) - lrc_cam_pos_y;

			lrc_vert_final_x[id] = (((int)xp)*dx - ((int)yp)*dy)>>14;
			lrc_vert_final_y[id] = (((int)xp)*dy + ((int)yp)*dx)>>14;
		}
	}

	// draw lines
	{
		for( int id=0; id<lrc_n_lines; id++ )
		{
			word v1 = lrc_lines_v1[id];
			word v2 = lrc_lines_v2[id];
			const byte *texture = TEXTURE_DATA + lrc_lines_tex[id];

			Dread_LineWall_Fix(
				lrc_vert_final_x[v1],
				lrc_vert_final_y[v1],
				lrc_vert_final_x[v2],
				lrc_vert_final_y[v2],
				0,						// tx1
				lrc_lines_tx2[id],		// tx2
				texture );
		}
	}
}
