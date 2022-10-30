
#include "common.h"
#include "app_rendertest.h"


extern short sincos_fix14[256];


// ================================================================ Helper Functions ================================================================


static int line_len(int dx, int dy)
{
	return (int)sqrt(dx*dx+dy*dy);
}






// ================================================================ DreadSoftRenderer ================================================================



void DreadSoftRenderer::LoadMap(DreadMapGen &mapgen)
{
	// Precompute rendering
	Dread_Build_Scalers();

	// Clear map data
	e_vtx.clear();
	e_lines.clear();
	e_subsectors.clear();
	e_vislines.clear();
	map_vis.clear();
	map_nodes.clear();

	// Init loader helpers
	chunky_packer.Preview_Init();
	tex_list.clear();

	// Load
	mapgen.ExportGather(tex_list);
	Map_LoadVerts(mapgen);
	Map_LoadLines(mapgen);
	Map_LoadVis(mapgen);
	Map_LoadSubSectors(mapgen);
	Map_LoadBSP(mapgen);
}

void DreadSoftRenderer::Draw(DataTexture &screen, const vec3 &cam_pos, float cam_angle)
{
	// Camera
	view_pos_x = (int)floor(cam_pos.x*(1<<ENGINE_SUBVERTEX_PRECISION)+0.5f);
	view_pos_y = (int)floor(-cam_pos.y*(1<<ENGINE_SUBVERTEX_PRECISION)+0.5f);
	view_pos_z = -(int)floor(cam_pos.z+0.5f);		// Z goes DOWN drom now on
	view_angle = int(floor(cam_angle/360*256+0.5f)) & 0xFF;
	view_rotate_dx = sincos_fix14[(view_angle+64)&0xFF];
	view_rotate_dy = sincos_fix14[(view_angle)&0xFF];

	// Init renderer
	screen.Resize(ENGINE_X_MAX, ENGINE_Y_MAX);
	screen.Clear(0x0F0E);
	max_outbound_top = 0;
	max_outbound_bottom = 0;

	for( int x=0; x<ENGINE_X_MAX; x++ )
	{
		render_columns.chunk1[x].render_addr = &screen.data[x];
		render_columns.chunk1[x].ymin = 0;
		render_columns.chunk1[x].ymax = ENGINE_Y_MAX;
	}

	// Clear vertex position cache
	for( int i=0; i<e_vtx.size(); i++ )
		e_vtx[i].tr_x = 0;

	// BSP
	if( map_nodes.size() <= 0 )
		return;
	short bsp = 0;
	do
	{
		const DataBSPNode *node = &map_nodes[bsp];
		int side = muls(view_pos_x, node->A) + muls(view_pos_y, node->B) + node->C;
		bsp = side>=0 ? node->right : node->left;
	} while( bsp>=0 );
	view_subsector = &e_subsectors[~bsp];


	// Draw lines
	const word *vis = view_subsector->vis;
	while( *vis!=0xC000 )
	{
		if( *vis <= 0x7FFF )
			Draw_Line(&e_lines[*vis>>1]);
		vis++;
	}

	Dev.DPrintF("TOP ERROR = %d", max_outbound_top);
	Dev.DPrintF("BOTTOM ERROR = %d", max_outbound_bottom);
}




void DreadSoftRenderer::Map_LoadVerts(DreadMapGen &mapgen)
{
	DreadMapGen::ExportState &exp = mapgen.exp;

	e_vtx.resize(exp.verts.size());

	for( int i=0; i<exp.verts.size(); i++ )
	{
		// Prepare EngineVertex structure
		DreadMapGen::Vertex *vsrc = exp.verts[i];
		EngineVertex *v = &e_vtx[i];
		v->xp = vsrc->xp;
		v->yp = vsrc->yp;
		v->tr_x = 0;
		v->tr_y = 0;
	}
}

void DreadSoftRenderer::Map_LoadLines(DreadMapGen &mapgen)
{
	DreadMapGen::ExportState &exp = mapgen.exp;

	e_lines.resize(exp.lines.size());

	for( int i=0; i<exp.lines.size(); i++ )
	{
		EngineLine *line = &e_lines[i];
		DreadMapGen::Line *lsrc = exp.lines[i];
		//lsrc->ComputeTextureOffsets();

		int v1 = lsrc->v1->_number;
		int v2 = lsrc->v2->_number;
		int x = e_vtx[v2].xp - e_vtx[v1].xp;
		int y = e_vtx[v2].yp - e_vtx[v1].yp;
		int len = line_len(x, y);
		if( len<1 ) len = 1;

		// Prepare EngineLine structure
		line->v1 = &e_vtx[v1];
		line->v2 = &e_vtx[v2];
		line->y1 = lsrc->h1;
		line->y2 = lsrc->h2;
		line->y3 = lsrc->h3;
		line->y4 = lsrc->h4;

		line->tex_upper = lsrc->tex_upper;
		line->tex_lower = lsrc->tex_lower;
		line->xcoord1 = lsrc->tex_offs_x;
		line->xcoord2 = lsrc->tex_offs_x + (word)len;
		line->ycoord_u = lsrc->_yoffs_u;
		line->ycoord_l = lsrc->_yoffs_l;

		line->ceil_col = chunky_packer.Process(gfx_converter.GetTextureColor(lsrc->tex_ceil));
		line->floor_col = chunky_packer.Process(gfx_converter.GetTextureColor(lsrc->tex_floor));
		
		line->draw_pixel = (lsrc->other_line && line->y3==line->y4 && lsrc->other_line->h4>line->y4);
		line->pixel_col = line->floor_col - 0x11;

//		line->ceil_col = lsrc->ceil;
//		line->floor_col = lsrc->floor;
//		line->modes[0] = lsrc->modes[0];
//		line->modes[1] = lsrc->modes[1];

//		printf("%4d .. %4d        %4d .. %4d\n", line->y1, line->y2, line->y3, line->y4);
	}
}

void DreadSoftRenderer::Map_LoadVis(DreadMapGen &mapgen)
{
	DreadMapGen::ExportState &exp = mapgen.exp;

	map_vis.resize(exp.vis.size());

	for( int i=0; i<(int)exp.vis.size(); i++ )
		map_vis[i] = exp.vis[i].Encode();
}

void DreadSoftRenderer::Map_LoadSubSectors(DreadMapGen &mapgen)
{
	DreadMapGen::ExportState &exp = mapgen.exp;

	e_subsectors.resize(exp.subsectors.size());

	for( int i=0; i<(int)exp.subsectors.size(); i++ )
	{
		EngineSubSector *sub = &e_subsectors[i];
		sub->vis = &map_vis[exp.subsectors[i]->_vis_start];
//		sub->thingvis = MAP_VIS + (int)ssrc->thingvisoffset;
	}
}

void DreadSoftRenderer::Map_LoadBSP(DreadMapGen &mapgen)
{
	DreadMapGen::ExportState &exp = mapgen.exp;

	map_nodes.resize(exp.nodes.size());

	for( int i=0; i<(int)exp.nodes.size(); i++ )
	{
		DreadMapGen::BSPNode &b = *exp.nodes[i];
		DataBSPNode &n = map_nodes[i];

		int A = -b.line.normal.x;
		int B = -b.line.normal.y;
		int C = (-b.line.normal.z) << ENGINE_SUBVERTEX_PRECISION;	// input coords are fix.3

		//if( A<-32767 || A>32767 || B<-32767 || B>32767 )
		//	printf("ERROR: BSP line vector magnitude.\n");

		int left=b.left->_number, right=b.right->_number;
		//if( left<-32767 || right<-32767 )
		//	printf("ERROR: BSP line draw list overflow.\n");
		//if( left>32767 || right>32767 )
		//	printf("ERROR: BSP node count overflow.\n");

		n.A = A;
		n.B = B;
		n.C = C;
		n.left = left;
		n.right = right;
	}
}



void DreadSoftRenderer::Dread_Build_Scaler_One(int size)
{
	// y1 = ENGINE_Y_MID + ((size*h1)>>8)				-  y1 is topmost (first) pixel which gets texcoord h1
	//
	//	for each Y>=y1	->	h>=h1

	for( int y=0; y<ENGINE_Y_MAX; y++ )
	{
		// (size*h)>>8 = y-ENGINE_Y_MID;				// given the <y>, choose largest <h> that satisfies the equation
		int ty;

		if( y>=ENGINE_Y_MID )
		{
			ty = (((y-ENGINE_Y_MID)<<8) | 0xFF)/size;
		}
		else
		{
			//	int yv = y-ENGINE_Y_MID;
			//	int hibitsneg = (-yv)-1;			// upper bits of multiplication result	(inverted)
			//	int target = hibitsneg<<8;			// obtain this value or higher	(pick lowest <minus_h>)
			//	
			//	// inverted multiplication result:
			//	//	size*minus_h-1 >= target
			//	//	minus_h >= (target+1)/size				// round up
			//	//	minus_h >= (target+1+size-1)/size
			//	//	minus_h >= target/size+1
			//	out[y] = -(target/size+1);


			ty = -1 - ((ENGINE_Y_MID-1-y)<<8)/size;
		}

		if( STRETCH_SCALING )
			ty >>= 1;


		// Input:
		//	A0		- return address
		//	A2		- texture pointer
		//	A7		- destination write pointer
		//

		// texture
		scaler_code.push_back(0x1EEA);		// 0001 111 011 101 010			MOVE.b	xxx(A2), (A7)+					MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
											// 0001 1110 1110 1010
		scaler_code.push_back(ty);
	}

	scaler_code.push_back(0x4ED6);		// 0100 111 01 1 010 110		JMP (A6)			0100 1110 1101 0000
}

void DreadSoftRenderer::Dread_Build_Scalers()
{
	memset(FN_SCALERS, 0, sizeof(FN_SCALERS));
	scaler_code.clear();


	int start = 1;
	int srem = 0;		// fixed.8
	while( start<NUM_SOFT_SIZES )
	{
		int regsize = (start<<8)/30 + srem;		// fixed.8
		if( regsize<256 ) regsize = 256;
		int ireg = regsize >> 8;
		srem = regsize - (ireg<<8);
		if( srem<0 ) srem = 0;

		int end = start + ireg;
		if( end > NUM_SOFT_SIZES )
			end = NUM_SOFT_SIZES;

		int size = (start+(end-1))/2;
		while( start<end )
		{
			FN_SCALERS[start]._offset = scaler_code.size();
			FN_SCALERS[start].real_size = size<<4;
			start++;
		}

		// Result:
		//	<size> is the real size value
		//	<ptr> points to start of the scaler routine
		//
		//	To compute screen screen Y coordinate use:
		//		y = ( ((-height - view_pos_z)*size)>>8 ) + ENGINE_Y_MID
		//	


		// Build scaler
		Dread_Build_Scaler_One(size);
	}

	scaler_code.push_back(0x4ED6);		// 0100 111 01 1 010 110		JMP (A6)			0100 1110 1101 0000

	for( int i=0; i<NUM_SOFT_SIZES; i++ )
		FN_SCALERS[i].func_addr = &scaler_code[FN_SCALERS[i]._offset];
}

void DreadSoftRenderer::Draw_Line(EngineLine *line)
{
	EngineVertex *v = line->v1;
	if( !v->tr_x )
	{
		short xp = (v->xp<<ENGINE_SUBVERTEX_PRECISION) - view_pos_x;
		short yp = (v->yp<<ENGINE_SUBVERTEX_PRECISION) - view_pos_y;
		v->tr_x = (muls(xp, view_rotate_dx) - muls(yp, view_rotate_dy))>>14;
		v->tr_y = (muls(xp, view_rotate_dy) + muls(yp, view_rotate_dx))>>14;
	}
	short p1x = v->tr_x;
	short p1y = v->tr_y;

	v = line->v2;
	if( !v->tr_x )
	{
		short xp = (v->xp<<ENGINE_SUBVERTEX_PRECISION) - view_pos_x;
		short yp = (v->yp<<ENGINE_SUBVERTEX_PRECISION) - view_pos_y;
		v->tr_x = (muls(xp, view_rotate_dx) - muls(yp, view_rotate_dy))>>14;
		v->tr_y = (muls(xp, view_rotate_dy) + muls(yp, view_rotate_dx))>>14;
	}
	short p2x = v->tr_x;
	short p2y = v->tr_y;

	int clip = 81;	//MODE_CLIP[mode];
	if( p1y<clip && p2y<clip ) return;


	word tx1 = line->xcoord1;
	word tx2 = line->xcoord2;

	short d1 = p1y+p1x;

	if( d1<0 ) {
		short d2 = p2y+p2x;
		if( d2<0 ) return;
		d2 -= d1;
		tx1 -= divs(muls(tx2-tx1, d1), d2);
		p1x -= divs(muls(p2x-p1x, d1), d2);
		p1y = -p1x;
	}

	short d2 = p2y-p2x;
	if( d2<0 ) {
		short d1 = p1y-p1x;
		if( d1<0 ) return;
		d1 -= d2;
		p2x -= divs(muls(p1x-p2x, d2), d1);
		tx2 -= divs(muls(tx1-tx2, d2), d1);
		p2y = p2x;
	}


	p1y -= clip;
	if( p1y<0 ) {
		p2y -= clip;
		if( p2y<0 ) return;
		tx1 -= divs(muls(p1y, tx1-tx2), p1y-p2y);
		p1x -= divs(muls(p1y, p1x-p2x), p1y-p2y);
		p1y = 0;
	}
	else {
		p2y -= clip;
		if( p2y<0 ) {
			tx2 -= divs(muls(tx1-tx2, p2y), p1y-p2y);
			p2x -= divs(muls(p2y, p1x-p2x), p1y-p2y);
			p2y = 0;
		}
	}

	p1y += clip;
	p2y += clip;

	tx1 <<= 7;
	tx2 <<= 7;

	int xmin = divs(muls(p1x, ENGINE_ZOOM), p1y) + ENGINE_ZOOM;
	if( xmin<0 ) xmin = 0;
	
	int xmax = divs(muls(p2x, ENGINE_ZOOM), p2y) + ENGINE_ZOOM;
	if( xmax>ENGINE_WIDTH ) xmax = ENGINE_WIDTH;

	if( xmin>=xmax ) return;
	int xlen = xmax - xmin;

	word s1 = divu(ENGINE_ZOOM_CONSTANT, p1y);
	word s2 = divu(ENGINE_ZOOM_CONSTANT, p2y);

	short asm_line_ds = (short)divs((int)s2-(int)s1, xmax-xmin);

	unsigned long u1s = mulu(tx1, s1);
	unsigned long u2s = mulu(tx2, s2);
	word ss = (s1>>1) + (s2>>1);
	dword uu = (u1s>>1) + (u2s>>1);

	int err = ((tx1+tx2)>>1) - (int)divu(uu, ss);
	if( err<0 ) err = -err;

	int ref = 4*128 + divu(10000000, ss);
	//bool use_perspective = true;
	//if( err < ref )
	//{
	//	use_perspective = false;
	//	u1s = tx1;
	//	u2s = tx2;
	//}

	long du;
	if( u1s <= u2s )
		du = (u2s-u1s)/(word)(xlen);
	else
		du = -((u1s-u2s)/(word)(xlen));

	int asm_tex_base1 = ((view_pos_z - line->ycoord_u)>>STRETCH_SCALING);
	int asm_tex_base2 = ((view_pos_z - line->ycoord_l)>>STRETCH_SCALING);

	//void(*line_cheat_fn)(__d2 int xmin, __d3 int xcount, __d4 dword s1, __d6 dword u1s, __d7 int du, __a5 EngineLine *line);
	
	Draw_RenderLine(xmin, xmax-xmin, s1, u1s, du, asm_line_ds, asm_tex_base1, asm_tex_base2, line);
}

void DreadSoftRenderer::Draw_FillRange(word xpos, int y1, int y2, byte color)
{
	DWORD *dst = render_columns.chunk1[xpos].render_addr;
	word ymin = render_columns.chunk1[xpos].ymin;
	word ymax = render_columns.chunk1[xpos].ymax;

	if( y1<ymin ) y1 = ymin;
	if( y2>ymax ) y2 = ymax;

	for( int y=y1; y<y2; y++ )
		dst[ENGINE_WIDTH*y] = ((color&0x0F)<<8) | ((color&0xF0)>>4);
}

void DreadSoftRenderer::Draw_TextureRange(word xpos, int y1, int y2, texture_id_t tex, int texu, int texv, word *scaler)
{
	DWORD *dst = render_columns.chunk1[xpos].render_addr;
	word ymin = render_columns.chunk1[xpos].ymin;
	word ymax = render_columns.chunk1[xpos].ymax;

	GfxTexture *gtex = gfx_converter.FindTexture(tex);

	if( y1<ymin ) y1 = ymin;
	if( y2>ymax ) y2 = ymax;

	for( int y=y1; y<y2; y++ )
	{
		if( !gtex )
			dst[ENGINE_WIDTH*y] = 0x0505;
		else
		{
			int tx = texu & 63;
			int ty = texv + (short)scaler[y*2+1];
			if( uint32_t(tx)>=gtex->texture.width )
			{
				dst[ENGINE_WIDTH*y] = 0x0B0F;
			}
			else if( uint32_t(ty)>=gtex->texture.height )
			{
				if( ty<0 )
				{
					int err = -ty;
					if( err>max_outbound_top ) max_outbound_top = err;

					if( err <= 2 )
					{
						dst[ENGINE_WIDTH*y] = gtex->texture.data[tx + gtex->texture.width*0];
						continue;
					}
				}

				if( ty>=gtex->texture.height )
				{
					int err = ty - (gtex->texture.height-1);
					if( err>max_outbound_bottom ) max_outbound_bottom = err;

					if( err <= 2 )
					{
						dst[ENGINE_WIDTH*y] = gtex->texture.data[tx + gtex->texture.width*(gtex->texture.width-1)];
						continue;
					}
				}

				dst[ENGINE_WIDTH*y] = 0x0B0F;
			}
			else
				dst[ENGINE_WIDTH*y] = gtex->texture.data[tx + gtex->texture.width*ty];
		}
	}
}


void DreadSoftRenderer::Draw_RenderLine(int xpos, int xcount, word s1, dword u1s, int du, short asm_line_ds, int texy1, int texy2, EngineLine *line)
{
	int y1, y2, y3, y4;
	int d1, d2, d3, d4;
	{
		// start
		int size = s1 >> 5;
		ScalerInfo &scaler = FN_SCALERS[size];
		size = scaler.real_size >> 4;
		y1 = ((((line->y1 - view_pos_z)*size)>>8) + ENGINE_Y_MID)<<16;
		y2 = ((((line->y2 - view_pos_z)*size)>>8) + ENGINE_Y_MID)<<16;
		y3 = ((((line->y3 - view_pos_z)*size)>>8) + ENGINE_Y_MID)<<16;
		y4 = ((((line->y4 - view_pos_z)*size)>>8) + ENGINE_Y_MID)<<16;
	}
	if( xcount>1 )
	{
		int size = (s1 + asm_line_ds*(xcount-1)) >> 5;
		ScalerInfo &scaler = FN_SCALERS[size];
		size = scaler.real_size >> 4;
		int e1 = ((((line->y1 - view_pos_z)*size)>>8) + ENGINE_Y_MID)<<16;
		int e2 = ((((line->y2 - view_pos_z)*size)>>8) + ENGINE_Y_MID)<<16;
		int e3 = ((((line->y3 - view_pos_z)*size)>>8) + ENGINE_Y_MID)<<16;
		int e4 = ((((line->y4 - view_pos_z)*size)>>8) + ENGINE_Y_MID)<<16;
		int cnt = xcount - 1;
		d1 = (e1 - y1)/cnt;
		d2 = (e2 - y2)/cnt;
		d3 = (e3 - y3)/cnt;
		d4 = (e4 - y4)/cnt;

		//if( line->y1 < line->y2 )
		//{
		//	y1 += 0xFFFF;
		//}
		//else
		//{
			y1 += 0x8000;
			y2 += 0x8000;
		//}

		//if( line->y3 != line->y4 )
		//{
		//	y3 += 0xFFFF;
		//}
		//else
		//{
			y3 += 0x8000;
			y4 += 0x8000;
		//}
	}
	else
		d1=d2=d3=d4=0;


	int ytop = 0;
	int yend = ENGINE_Y_MAX;

	//u1s >>= 7;
	//du >>= 7;
	while( xcount-->0 )
	{
		int size = s1 >> 5;
		ScalerInfo &scaler = FN_SCALERS[size];
		word tx = (divu(u1s, s1) & 0x1F80) >> 7;
		//word tx = divu(u1s,s1) & 0x3F;

		Draw_FillRange(xpos, ytop, y1>>16, line->ceil_col);
		Draw_TextureRange(xpos, y1>>16, y2>>16, line->tex_upper, tx, texy1, scaler.func_addr);
		Draw_TextureRange(xpos, y3>>16, y4>>16, line->tex_lower, tx, texy2, scaler.func_addr);
		if( line->draw_pixel )
		{
			Draw_FillRange(xpos, (y4>>16), (y4>>16)+1, line->pixel_col);
			Draw_FillRange(xpos, (y4>>16)+1, yend, line->floor_col);
		}
		else
			Draw_FillRange(xpos, (y4>>16)+0, yend, line->floor_col);

		if( (y2>>16)>render_columns.chunk1[xpos].ymin )
			render_columns.chunk1[xpos].ymin = (y2>>16);

		if( (y3>>16)<render_columns.chunk1[xpos].ymax )
			render_columns.chunk1[xpos].ymax = (y3>>16);


		xpos++;
		s1 += asm_line_ds;
		u1s += du;
		y1 += d1;
		y2 += d2;
		y3 += d3;
		y4 += d4;
	}
}


void DreadSoftRenderer::Draw_RenderLine_Old(int xpos, int xcount, word s1, dword u1s, int du, short asm_line_ds, int texy1, int texy2, EngineLine *line)
{
	while( xcount-->0 )
	{
		int size = s1 >> 5;
		ScalerInfo &scaler = FN_SCALERS[size];
		// FN_SCALERS[start].real_size = size<<4;
		size = scaler.real_size >> 4;
		int ytop = 0;
		int yend = ENGINE_Y_MAX;
		int y1 = (((line->y1 - view_pos_z)*size)>>8) + ENGINE_Y_MID;
		int y2 = (((line->y2 - view_pos_z)*size)>>8) + ENGINE_Y_MID;
		int y3 = (((line->y3 - view_pos_z)*size)>>8) + ENGINE_Y_MID;
		int y4 = (((line->y4 - view_pos_z)*size)>>8) + ENGINE_Y_MID;
		word tx = (divu(u1s, s1) & 0x1F80) >> 7;

		Draw_FillRange(xpos, ytop, y1, line->ceil_col);
		Draw_TextureRange(xpos, y1, y2, line->tex_upper, tx, texy1, scaler.func_addr);
		Draw_TextureRange(xpos, y3, y4, line->tex_lower, tx, texy1, scaler.func_addr);
		Draw_FillRange(xpos, y4, yend, line->floor_col);

		if( y2>render_columns.chunk1[xpos].ymin )
			render_columns.chunk1[xpos].ymin = y2;

		if( y3<render_columns.chunk1[xpos].ymax )
			render_columns.chunk1[xpos].ymax = y3;


		xpos++;
		s1 += asm_line_ds;
		u1s += du;
	}
}
