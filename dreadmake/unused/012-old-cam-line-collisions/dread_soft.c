
#include "demo.h"


int cam_pos_x;
int cam_pos_y;
int cam_pos_z;
word cam_weapon_swing;
short lrc_cam_pos_x;							// camera
short lrc_cam_pos_y;
short lrc_cam_pos_z;
short lrc_cam_angle;
short lrc_cam_rotate_dx;
short lrc_cam_rotate_dy;
EngineSubSector *lrc_cam_subsector;


short render_layout_delta[160];

typedef struct {
	struct {
		word	*render_addr;
		word	ymin;
		word	ymax;
	} chunk1[160];
	struct {
		word	size_limit;
		word	_pad0;
		word	_pad1;
		word	_pad2;
	} chunk2[160];
} RenderColumnsInfo;

const byte *e_skymap[128*3];
const byte **e_skyptr;
RenderColumnsInfo render_columns;
short multab6[160*2];		// x/4*6

word weapon_frame;
word weapon_reload;




int line_len(int dx, int dy)
{
	// Length of(x, y) int vector as 24.8 fixpoint (1.12% max error):
	//	x=abs(x);
	//	y=abs(y);
	//	s=x+y;
	//	t=abs(x-y);
	//	return s*164 + t*72 + abs(s*15-t*34);

	if( dx<0 ) dx=-dx;
	if( dy<0 ) dy=-dy;
	int s = dx+dy;
	int t = dx-dy;
	if( t<0 ) t=-t;
	dx = s*15-t*34;
	if( dx<0 ) dx=-dx;
	int len = s*164 + t*72 + dx;
	return (len+128)>>8;
}

int line_len_fix8(int dx, int dy)
{
	if( dx<0 ) dx=-dx;
	if( dy<0 ) dy=-dy;
	int s = dx+dy;
	int t = dx-dy;
	if( t<0 ) t=-t;
	dx = s*15-t*34;
	if( dx<0 ) dx=-dx;
	return s*164 + t*72 + dx;
}


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

	Dread_Init_Objects();


	for( int i=0; i<3*128; i++ )
		e_skymap[i] = SKYTEX_DATA + ((i&127)<<6);

	word _offs = 0;
	for( int i=0; i<160; i++ )
	{
		word offs = 0;
		if( i&1 ) offs += 8080;
		if( i&2 ) offs += 200;
		if( i&4 ) offs++;
		offs += (i>>3)*402;

		//render_columns[i].render_offs = offs;	//(i>>2) + (i&3)*4000 + 50*40;
		render_layout_delta[i] = offs - _offs;
		_offs = offs;

		multab6[i<<1] = (i-50)*6;
	}
	//	render_columns[i].render_delta = (short)render_columns[i].render_offs - (short)render_columns[i-1].render_offs;
	//render_columns[0].render_delta = 0;

	Map_LoadAll();

	intnum = 0;
}

void Dread_DrawSprite(int spx, int spy, const word *frame)
{
	// Transform
	{
		spx = (spx<<4) - lrc_cam_pos_x;
		spy = (spy<<4) - lrc_cam_pos_y;
		int tx = (((int)spx)*lrc_cam_rotate_dx - ((int)spy)*lrc_cam_rotate_dy)>>14;
		spy = (((int)spx)*lrc_cam_rotate_dy + ((int)spy)*lrc_cam_rotate_dx)>>14;
		spx = tx;
	}

	// Draw
	const int WOLFSCREEN_W = 160;
	const int wolf_xoffs = 80;
	const int zoom_i = 80;
	const int clip = 161;	//161;	// fix.4

	if( spy < clip )
		return;

	int p1x = spx - (frame[1]<<4);
	int p2x = p1x + (frame[0]<<4);
	word tx1 = 0;
	word tx2 = frame[0];


	// xp = px*zoom / py
	int xmin = divs(muls(p1x, zoom_i), spy) + wolf_xoffs;
	int xmax = divs(muls(p2x, zoom_i), spy) + wolf_xoffs;
	if( xmin>=xmax ) return;

	short s0 = divs((zoom_i*32)<<12, spy);		// won't overflow because of clipping p#y>=321
	long u1s = ((long)tx1)<<(16+1);
	long u2s = ((long)tx2)<<(16+1);
	long du = (u2s-u1s)/(long)(xmax-xmin);			// result can be 32-bit!!

	if( xmin < 0 )
	{
		u1s -= du*xmin;
		xmin = 0;
	}
	if( xmax>WOLFSCREEN_W ) xmax = WOLFSCREEN_W;
	if( xmin>=xmax ) return;

	//															v line->y1
	asm_tex_base = (const byte*)frame + -40;

	Dread_LineCore_Sprite(xmin<<3, xmax-xmin-1, s0, u1s, du, frame);

}


void Dread_CameraUpdateBSP(void)
{
	//short bsp = 0;
	//do
	//{
	//	const DataBSPNode *node = (const DataBSPNode*)(((const byte*)MAP_NODES) + bsp);
	//	int side = muls(lrc_cam_pos_x, node->A) + muls(lrc_cam_pos_y, node->B) + node->C;
	//	bsp = side>=0 ? node->right : node->left;
	//} while( bsp>=0 );
	//lrc_cam_subsector = MAP_SUBSECTORS + ~bsp;

	lrc_cam_subsector = Dread_FindSubSector(lrc_cam_pos_x, lrc_cam_pos_y);
}

int Dread_CamLineCollision(EngineLine *line)
{
	const int psize = 15 << 4;

	short xp = lrc_cam_pos_x - (line->v1->xp << 4);
	short yp = lrc_cam_pos_y - (line->v1->yp << 4);
	short lx = (line->v2->xp - line->v1->xp) << 4;
	short ly = (line->v2->yp - line->v1->yp) << 4;

	if( !ly )	// horizontal line
	{
		if( yp<=-psize || yp>=psize ) return 0;

		if( lx>0 )	// direction: right
		{
			if( xp<=0 )
			{
				// collide against (0,0)
				if( xp<=-psize ) return 0;
				goto _point_case;
			}
			else if( xp>=lx )
			{
				// collide against (lx,0)
				xp -= lx;
				if( xp>=psize ) return 0;
				goto _point_case;
			}
			// collides with line body	- move yp to -psize
			lrc_cam_pos_y += -psize - yp;
			return 1;
		}
		else		// direction: left
		{
			if( xp>=0 )
			{
				// collide against (0,0)
				if( xp>=psize ) return 0;
				goto _point_case;
			}
			else if( xp<=lx )
			{
				// collide against (lx,0)
				xp -= lx;
				if( xp<=-psize ) return 0;
				goto _point_case;
			}
			// collides with line body	- move yp to -psize
			lrc_cam_pos_y += psize - yp;
			return 1;
		}
	}

	if( !lx )	// vertical line
	{
		if( xp<=-psize || xp>=psize ) return 0;
	
		if( ly>0 )	// direction: up
		{
			if( yp<=0 )
			{
				// collide against (0,0)
				if( yp<=-psize ) return 0;
				goto _point_case;
			}
			else if( yp>=ly )
			{
				// collide against (0,ly)
				yp -= ly;
				if( yp>=psize ) return 0;
				goto _point_case;
			}
			// collides with line body	- move yp to -psize
			lrc_cam_pos_x += psize - xp;
			return 1;
		}
		else	// direction: down
		{
			if( yp>=0 )
			{
				// collide against (0,0)
				if( yp>=psize ) return 0;
				goto _point_case;
			}
			else if( yp<=ly )
			{
				// collide against (0,ly)
				yp -= ly;
				if( yp<=-psize ) return 0;
				goto _point_case;
			}
			// collides with line body	- move yp to -psize
			lrc_cam_pos_x += -psize - xp;
			return 1;
		}
	}

	if( 0 )
	{
		int len;
		// Point case - collide (xp,yp) with point (0,0)		(coarse checks already done)
	_point_case:
		len = line_len_fix8(xp, yp);
		if( len < (psize<<8) && (len>=256) )
		{
			lrc_cam_pos_x += xp*((psize+4)<<8)/len - xp;
			lrc_cam_pos_y += yp*((psize+4)<<8)/len - yp;
		}
		return 1;
	}

	return 0;
}

void Dread_Camera(word ticks)
{
	// rotate
	lrc_cam_angle = debug_vars[0]>>4;
	lrc_cam_angle &= 0xFF;

	// walk
	word move = 16;
	short mwalk = 0;
	short mside = 0;
	if( keystate[0x11] ) mwalk += move;
	if( keystate[0x21] ) mwalk -= move;
	if( keystate[0x20] ) mside -= move;
	if( keystate[0x22] ) mside += move;
	if( keystate[0x10] ) cam_pos_z += move<<14;
	if( keystate[0x12] ) cam_pos_z -= move<<14;
	if( key_last>=0x01 && key_last<=0x09 )
	{
		const DataLocation *loc = Map_GetLocation(key_last-1);
		cam_pos_x = ((int)loc->xp) << 16;
		cam_pos_y = ((int)loc->yp) << 16;
		lrc_cam_angle = (-loc->angle/45*32) & 0xFF;
		
		debug_vars[0] = lrc_cam_angle << 4;
		key_last = 0;
	}

	int delta_x = ((int)sincos_fix14[lrc_cam_angle])*mwalk + ((int)sincos_fix14[(lrc_cam_angle+64)&0xFF])*mside;
	int delta_y =  ((int)sincos_fix14[(lrc_cam_angle+64)&0xFF])*mwalk - ((int)sincos_fix14[lrc_cam_angle])*mside;

	lrc_cam_pos_z = (short)(cam_pos_z>>16);
	lrc_cam_rotate_dx = sincos_fix14[(lrc_cam_angle+64)&0xFF];
	lrc_cam_rotate_dy = sincos_fix14[(lrc_cam_angle)&0xFF];

#if 0
	if( delta_x || delta_y )
	{
		word moved;
		for( int step=0; step<ticks; step++ )
		{
			cam_pos_x += delta_x;
			cam_pos_y += delta_y;

			lrc_cam_pos_x = (short)(cam_pos_x>>12);
			lrc_cam_pos_y = (short)(cam_pos_y>>12);

			Dread_CameraUpdateBSP();

			// line collisions
			short col = lrc_cam_bspvis;
			moved = 0;

			while( 1 )
			{
				word line = MAP_VIS[--col];
				if( line==0xFFFF ) break;

				moved |= Dread_CamLineCollision(e_lines + line);
			}

			if( moved )
			{
				cam_pos_x = ((int)lrc_cam_pos_x)<<12;
				cam_pos_y = ((int)lrc_cam_pos_y)<<12;
			}
		}

		if( moved )
			Dread_CameraUpdateBSP();
	}
	else
	{
		lrc_cam_pos_x = (short)(cam_pos_x>>12);
		lrc_cam_pos_y = (short)(cam_pos_y>>12);
		Dread_CameraUpdateBSP();
	}
#else
	cam_pos_x += delta_x*ticks;
	cam_pos_y += delta_y*ticks;
	lrc_cam_pos_x = (short)(cam_pos_x>>12);
	lrc_cam_pos_y = (short)(cam_pos_y>>12);
	Dread_CameraUpdateBSP();
#endif


	// weapon swing
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

	// weapon reload timer
	if( weapon_reload < 50 )
		weapon_reload += ticks;
	if( weapon_reload >= 50 )
		weapon_frame = 0;
}

void Dread_SoftTest(void)
{
	word ticks = intnum;
	intnum = 0;

	//if( !keystate[0x01] )
	{
		// camera
		Dread_Camera(ticks);
	}

	// init clip buffer
	Dread_FrameReset(render_buffer);

	// init drawing state
	e_skyptr = e_skymap + ((lrc_cam_angle<<1)&127) + 16;
	asm_fn_colorfill_mid = FN_COLORFILL_MID;
	asm_fn_skycopy = FN_SKYCOPY;
	asm_render_buffer = render_buffer;


	// draw lines
	//short vis = lrc_cam_bspvis;
	//while(1)
	//{
	//	word line = MAP_VIS[vis++];
	//	if( line==0xFFFF ) break;
	//	word mode = MAP_VIS[vis++];
	//
	//	//Dread_LineWall_Fix2_wip(e_lines + line, mode);
	//	Dread_LineWall_Fix2_asm(e_lines + line, mode);
	//}
	Dread_DrawLines(lrc_cam_subsector->vis);

	//// cleanup verts
	//vis = lrc_cam_bspvis;
	//while( 1 )
	//{
	//	word line = MAP_VIS[vis++];
	//	if( line==0xFFFF ) break;
	//	vis++;
	//	e_lines[line].v1->tr_x = 0;
	//	e_lines[line].v2->tr_x = 0;
	//}

//	 Dread_Handle_Objects(ticks*6);
//	Dread_Draw_Test_Objects();
//	Dread_Draw_Test_Objects2();
}
