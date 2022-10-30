
#include "demo.h"


#include "data/frame.inc"


#define SOFT_MAX_VERT		10
#define SOFT_MAX_LINES		10

int cam_pos_x;
int cam_pos_y;
int cam_pos_z;
word cam_weapon_swing;
short lrc_cam_pos_x;							// camera
short lrc_cam_pos_y;
short lrc_cam_pos_z;
short lrc_cam_angle;
word  lrc_n_vert = 0;							// data
word  lrc_n_lines = 0;
short lrc_vert_final_x[SOFT_MAX_VERT];			// internal
short lrc_vert_final_y[SOFT_MAX_VERT];

EngineVertex e_vtx[SOFT_MAX_VERT];
EngineLine e_lines[SOFT_MAX_LINES];

const byte *e_skymap[128*3];
const byte **e_skyptr;
word render_offsets[160*2];
byte clip_y1[160];
byte clip_y2[160];

word weapon_frame;
word weapon_reload;



// typedef struct {
// 	short	xp;
// 	short	yp;
// } DataVertex;
static const DataVertex MAP_VTX[] = {
	//
	//	   8--9
	//	   |  |
	//	0--6--7-1
	//	|       |
	//	|       2
	//	|       |
	//	|       3
	//	|       |
	//	5-------4
	//
	//
	{ -192,  192 },		//	0
	{  192,  192 },		//	1
	{  192,   32 },		//	2
	{  192,  -32 },		//	3
	{  192, -192 },		//	4
	{ -192, -192 },		//	5
	{ -128,  192 },		//	6
	{  128,  192 },		//	7
	{ -128,  320 },		//	8
	{  128,  320 },		//	9
	{ 0x7FFF, 0x7FFF }
};

//typedef struct {
//	word	v1;
//	word	v2;
//	word	tex_upper;
//	short	y1;
//	short	y2;
//	byte	ceil;
//	byte	floor;
//} DataLine;
static const DataLine MAP_LINES[] = {
	{  0,  6,	TEX_RW23_4_OFFS,   -128,  0, 0x40, 0xC0 },	// North - left
	{  6,  7,	TEX_RW23_4_OFFS,   -128,-64, 0x40, 0xC0 },	// North - middle
	{  7,  1,	TEX_RW23_4_OFFS,   -128,  0, 0x40, 0xC0 },	// North - right

	{  1,  2,	TEX_RW23_4_OFFS,	-64,  0, 0x40, 0xC0 },	// East - left
	{  2,  3,	TEX_DOOR3_6_OFFS,	-64,  0, 0x40, 0xC0 },	// East - door
	{  3,  4,	TEX_RW23_4_OFFS,	-64,  0, 0x40, 0xC0 },	// East - right
	{  4,  5,	TEX_RW23_4_OFFS,   -128,  0, 0x40, 0xC0 },
	{  5,  0,	TEX_RW23_4_OFFS,	-64,  0, 0x40, 0xC0 },

	{  6,  8,	TEX_RW23_4_DARK_OFFS,    -64,  0, 0x40, 0x40 },	// Pocket - left
	{ 8,  9,	TEX_RW23_4_DARK_OFFS,    -64,  0, 0x40, 0x40 },	// Pocket - middle
	{ 9,  7,	TEX_RW23_4_DARK_OFFS,    -64,  0, 0x40, 0x40 },	// Pocket - right

	{ 0xFFFF, 0, 0, 0, 0, 0, 0 }
};



void Dread_SoftInit(void)
{
	cam_pos_x = 0;
	cam_pos_y = 0;
	cam_pos_z = -40<<16;
	lrc_cam_pos_x = 0;							// camera
	lrc_cam_pos_y = 0;
	lrc_cam_pos_z = -40;
	lrc_cam_angle = 0;

	weapon_frame = 0;
	weapon_reload = 255;


	for( int i=0; i<3*128; i++ )
		e_skymap[i] = SKYTEX_DATA + ((i&127)<<6);

	for( int i=0; i<160; i++ )
		render_offsets[i<<1] = (i>>2) + (i&3)*4000;

	for( int i=0; MAP_VTX[i].xp!=0x7FFF; i++ )
	{
		// Prepare EngineVertex structure
		EngineVertex *v = e_vtx + i;
		v->xp = MAP_VTX[i].xp;
		v->yp = MAP_VTX[i].yp;
		v->tr_x = 0;
		v->tr_y = 0;
		lrc_n_vert = i+1;
	}

	for( int i=0; MAP_LINES[i].v1!=0xFFFF; i++ )
	{
		// Length of(x, y) int vector as 24.8 fixpoint (1.12% max error):
		//	x=abs(x);
		//	y=abs(y);
		//	s=x+y;
		//	t=abs(x-y);
		//	return s*164 + t*72 + abs(s*15-t*34);

		int v1 = MAP_LINES[i].v1;
		int v2 = MAP_LINES[i].v2;
		int x = e_vtx[v2].xp - e_vtx[v1].xp;
		int y = e_vtx[v2].yp - e_vtx[v1].yp;
		if( x<0 ) x=-x;
		if( y<0 ) y=-y;
		int s = x+y;
		int t = x-y;
		if( t<0 ) t=-t;
		x = s*15-t*34;
		if( x<0 ) x=-x;
		int len = s*164 + t*72 + x;
		len = (len+128)>>8;
		if( len<1 ) len = 1;

		// Prepare EngineLine structure
		EngineLine *line = e_lines + i;
		line->v1 = e_vtx + v1;
		line->v2 = e_vtx + v2;
		line->next = NULL;		//e_lines + (i+1)%lrc_n_lines;
		line->other_sector = NULL;
		line->tex_upper = TEXTURE_DATA + MAP_LINES[i].tex_upper;	// lrc_lines_tex[i];
		line->tex_lower = NULL;
		line->xcoord1 = 0;
		line->xcoord2 = (word)len;
		line->y1 = MAP_LINES[i].y1;
		line->y2 = MAP_LINES[i].y2;
		line->y3 = 0;
		line->ceil = MAP_LINES[i].ceil;
		line->floor = MAP_LINES[i].floor;

		lrc_n_lines = i+1;
	}

	intnum = 0;
}



//  int p1x, int p1y, int p2x, int p2y, int tx1, int tx2, const byte *texture
void Dread_LineWall_Fix(EngineLine *line)
{
	const int WOLFSCREEN_W = 160;
	const int wolf_xoffs = WOLFSCREEN_W/2;
	const int clip = 321;	// fix.4
	const int zoom_i = 80;

	int p1x = line->v1->tr_x;
	int p1y = line->v1->tr_y - clip;
	int p2x = line->v2->tr_x;
	int p2y = line->v2->tr_y - clip;
	word tx1 = line->xcoord1;
	word tx2 = line->xcoord2;
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

	// xp = px*zoom / py
	int xmin = p1x*zoom_i/p1y + wolf_xoffs;
	int xmax = p2x*zoom_i/p2y + wolf_xoffs;
	if( xmin>=xmax ) return;

	short s1 = divs((zoom_i*32)<<12, p1y);		// won't overflow because of clipping p#y>=300
	short s2 = divs((zoom_i*32)<<12, p2y);
	short ds = divs(s2-s1, xmax-xmin);			// s# is signed word
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

	//for( int x=xmin; x<xmax; x++ )
	//{
	//	int size = s1>>8;
	//	if( size<=1 ) size = 1;
	//	if( size>=100 ) size = 100;
	//	int tx = (u1s / s1)&63;
	//	
	//	const byte *sky = e_skyptr[x];
	//	const byte *src = line->tex_upper + tx;
	//	byte *dst = render_buffer + (x>>2) + (x&3)*4000;
	//
	//	//FN_COLBLITS[size](dst, src, sky, 0xC0);
	//	Dread_LineCore_Simple(src, size<<2, x<<2);
	//
	//	s1 += ds;
	//	u1s += du;
	//}

	Dread_LineCore(xmin<<2, xmax<<2, s1, ds, u1s, du, line->tex_upper);
}

void Dread_LineWall_Fix2(EngineLine *line)
{
	const int WOLFSCREEN_W = 160;
	const int wolf_xoffs = 80;
	const int zoom_i = 80;
	const int clip = 321;	// fix.4

	int p1x = line->v1->tr_x;
	int p1y = line->v1->tr_y - clip;
	int p2x = line->v2->tr_x;
	int p2y = line->v2->tr_y - clip;
	word tx1 = line->xcoord1;
	word tx2 = line->xcoord2;

	if( p1y<clip && p2y<clip )
		return;

	// Clip:	Y+X > 0
	{
		int d1 = p1y+p1x;
		int d2 = p2y+p2x;
		if( d1<0 )
		{
			if( d2<0 ) return;
			d2 -= d1;
			p1x -= (p2x-p1x)*d1/d2;
			p1y -= (p2y-p1y)*d1/d2;
			tx1 -= (tx2-tx1)*d1/d2;
		}
	}

	p1y -= clip;
	p2y -= clip;
	if( p1y<0 )
	{
		if( p2y<0 ) return;
		tx1 -= divs(muls(p1y, tx1-tx2), p1y-p2y);
		p1x -= divs(muls(p1y, p1x-p2x), p1y-p2y);
		p1y = 0;
	}
	else if( p2y<0 )
	{
		tx2 -= divs(muls(p2y, tx1-tx2), p1y-p2y);
		p2x -= divs(muls(p2y, p1x-p2x), p1y-p2y);
		p2y = 0;
	}
	p1y += clip;
	p2y += clip;
	tx2 -= tx1&~63;
	tx1 &= 63;


	// xp = px*zoom / py
	int xmin = divs(muls(p1x, zoom_i), p1y) + wolf_xoffs;
	int xmax = divs(muls(p2x, zoom_i), p2y) + wolf_xoffs;
	if( xmin>=xmax ) return;

	short s1 = divs((zoom_i*32)<<12, p1y);		// won't overflow because of clipping p#y>=300
	short s2 = divs((zoom_i*32)<<12, p2y);
	short ds = divs(s2-s1, xmax-xmin);			// s# is signed word
	int u1s = mulu(tx1, s1);
	int u2s = mulu(tx2, s2);
	int du = (u2s-u1s)/(xmax-xmin);			// result can be 32-bit!!

	if( xmin<0 ) xmin = 0;
	if( xmax>WOLFSCREEN_W ) xmax = WOLFSCREEN_W;
	if( xmin>=xmax ) return;

	short line_h1 = line->y1 - lrc_cam_pos_z;
	short line_h2 = line->y2 - lrc_cam_pos_z;
	short line_h3 = line->y3 - lrc_cam_pos_z;
	byte *tex_base = line->tex_upper + ((lrc_cam_pos_z - line->y1)<<6);
	for( int x=xmin; x<xmax; x++ )
	{
		int size = ((word)s1)>>5;
		if( size>=NUM_SOFT_SIZES ) size = NUM_SOFT_SIZES-1;
		const ScalerInfo *si = FN_SCALERS + size;
		size = si->real_size;

		int ymin = clip_y1[x];
		int ymax = clip_y2[x];
		int y1 = 50 + ((size*line_h1)>>8);
		int y2 = 50 + ((size*line_h2)>>8);
		int y3 = 50 + ((size*line_h3)>>8);
		if( y1<=ymin ) y1 = ymin;
		if( y1>=ymax ) y1 = ymax;
		if( y2<=ymin ) y2 = ymin;
		if( y2>=ymax ) y2 = ymax;
		if( y3<=ymin ) y3 = ymin;
		if( y3>=ymax ) y3 = ymax;

		byte *dst = render_buffer + render_offsets[x<<1];
		word *fn;


		// // 0..y1	Sky
		// fn = (word*)FN_SKYCOPY;
		// fn[y1*2] = 0x4E75;		// RTS
		// ((skycopy_fn_t)(fn+2*ymin))(dst, e_skyptr[x]+ymin);
		// fn[y1*2] = 0x1159;		// 0001 000 101 011 001			MOVE.b	(A1)+, yyy(A0)						MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>

		// 0..y1	Ceil color
		fn = (word*)FN_COLORFILL;
		fn[y1*2] = 0x4E75;		// RTS
		((colorfill_fn_t)(fn+2*ymin))(dst, line->ceil);
		fn[y1*2] = 0x1141;		// 0001 000 101 000 001			MOVE.b	D1, yyy(A0)							MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>


		// y1..y2	Upper texture
		int tx = (u1s / s1)&63;

		fn = si->func_addr;
		fn[y2*3] = 0x4E75;		// RTS
		((scaler_fn_t)(fn+3*y1))(dst, tex_base + tx);
		fn[y2*3] = 0x116A;		// 0001 000 101 101 010			MOVE.b	xxx(A2), yyy(A0)					MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>

		// y2..y3	Next sector
		clip_y1[x] = y2;
		clip_y2[x] = y3;

		// y3..100	Floor color
		fn = (word*)FN_COLORFILL;
		fn[ymax*2] = 0x4E75;		// RTS
		((colorfill_fn_t)(fn+2*y3))(dst, line->floor);
		fn[ymax*2] = 0x1141;		// 0001 000 101 000 001			MOVE.b	D1, yyy(A0)							MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>


		s1 += ds;
		u1s += du;
	}
}

void Dread_SoftTest(void)
{
	word ticks = intnum;
	intnum = 0;

	// camera
	{
		lrc_cam_angle = debug_vars[0]>>4;
		lrc_cam_angle &= 0xFF;
		word move = ticks*16;
		short mwalk = 0;
		short mside = 0;
		if( keystate[0x11] ) mwalk += move;
		if( keystate[0x21] ) mwalk -= move;
		if( keystate[0x20] ) mside -= move;
		if( keystate[0x22] ) mside += move;
		if( keystate[0x10] ) cam_pos_z += move<<14;
		if( keystate[0x12] ) cam_pos_z -= move<<14;
		if( mwalk | mside )
			cam_weapon_swing += ticks*3;
		else if( cam_weapon_swing & 0x7F )
		{
			word pswing = cam_weapon_swing;
			if( cam_weapon_swing & 0x40 )	cam_weapon_swing += ticks*6;
			else							cam_weapon_swing -= ticks*6;
			if( (pswing^cam_weapon_swing) & 0x80 )
				cam_weapon_swing = 0;
		}

		cam_pos_x += ((int)sincos_fix14[lrc_cam_angle])*mwalk;
		cam_pos_y += ((int)sincos_fix14[(lrc_cam_angle+64)&0xFF])*mwalk;

		cam_pos_x += ((int)sincos_fix14[(lrc_cam_angle+64)&0xFF])*mside;
		cam_pos_y -= ((int)sincos_fix14[lrc_cam_angle])*mside;

		const int psize = 28;
		if( cam_pos_x > ((192-psize)<<16) ) cam_pos_x = ((192-psize)<<16);
		if( cam_pos_x <-((192-psize)<<16) ) cam_pos_x =-((192-psize)<<16);
		if( cam_pos_y >((192-psize)<<16) ) cam_pos_y = ((192-psize)<<16);
		if( cam_pos_y <-((192-psize)<<16) ) cam_pos_y =-((192-psize)<<16);

		lrc_cam_pos_x = (short)(cam_pos_x>>12);
		lrc_cam_pos_y = (short)(cam_pos_y>>12);
		lrc_cam_pos_z = (short)(cam_pos_z>>16);

		// weapon
		if( weapon_reload < 50 )
			weapon_reload += ticks;
		if( weapon_reload >= 50 )
			weapon_frame = 0;

	}

	// precompute vertexes
	{
		short dx = sincos_fix14[(lrc_cam_angle+64)&0xFF];
		short dy = sincos_fix14[(lrc_cam_angle)&0xFF];
		for( int id=0; id<lrc_n_vert; id++ )
		{
			EngineVertex *v = e_vtx + id;
			short xp = (v->xp<<4) - lrc_cam_pos_x;
			short yp = (v->yp<<4) - lrc_cam_pos_y;

			v->tr_x = (((int)xp)*dx - ((int)yp)*dy)>>14;
			v->tr_y = (((int)xp)*dy + ((int)yp)*dx)>>14;
		}
	}

	// init clip buffer
	for(int i=0;i<160;i++)
	{
		clip_y1[i] = 0;
		clip_y2[i] = 100;
	}


	// draw lines
	e_skyptr = e_skymap + ((lrc_cam_angle<<1)&127);

	for( int id=0; id<lrc_n_lines; id++ )
		Dread_LineWall_Fix2(e_lines + id);
	//Dread_LineCore(0<<2, 160<<2, 20<<8, 0, 0, 0, TEXTURE_DATA);

}
