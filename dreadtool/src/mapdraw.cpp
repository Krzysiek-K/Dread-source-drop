
#include "common.h"
#include "app_rendertest.h"





// Returns relative order of 0-d1 and 0-d2 segments, results:
//	negative	- left (v1->v2 goes counter-clockwise around v0)
//	zero		- colinear
//	positive	- right (v1->v2 goes clockwise around v0)
float _fan_order(const vec2 &d1, const vec2 &d2)
{
	// ^ Y
	// |  X
	// +-->
	return d2.x*d1.y - d1.x*d2.y;
}


int _float_to_dread(float coord)
{
	return int(floor(coord*(1<<DreadMapGen::ENGINE_SUBVERTEX_PRECISION)+0.5f));
}

float _dread_to_float(int coord)
{
	return float(coord)/(1<<DreadMapGen::ENGINE_SUBVERTEX_PRECISION);
}

vec2 _dread_to_vec2(ivec2 pos)
{
	return vec2(pos.x, pos.y)/(1<<DreadMapGen::ENGINE_SUBVERTEX_PRECISION);
}

vec2 _ivec2_to_vec2(ivec2 pos)
{
	return vec2(pos.x, pos.y);
}


// ---------------------------------------------------------------- MapDraw ----------------------------------------------------------------



void MapDraw::CenterRegion(vec2 vmin, vec2 vmax)
{
	vmin.y = -vmin.y;
	vmax.y = -vmax.y;
	swap(vmin.y, vmax.y);

	mv_zoom = (mv_size_y-32)/(vmax.y - vmin.y);
	mv_center = (vmin+vmax)/2;
}

void MapDraw::DrawVertex(vec2 v, DWORD color, float size, int layer)
{
	float ls = 0.5f/mv_zoom;	// line size
	v.y = -v.y;
	can.Draw(layer, "")()(color).line(v-vec2(size, size), v+vec2(size, size), ls);
	can.Draw(layer, "")()(color).line(v-vec2(size, -size), v+vec2(size, -size), ls);
}

void MapDraw::DrawLine(vec2 v1, vec2 v2, DWORD color, bool mark_direction)
{
	float ls = 0.5f/mv_zoom;	// line size
	v1.y = -v1.y;
	v2.y = -v2.y;

	can.Draw("")()(color).line(v1, v2, ls);
	if( mark_direction )
		can.Draw("")()(color).line((v1+v2)/2, (v1+v2)/2+(v1-v2).get_rotated90().get_normalized()*8, ls);
}

void MapDraw::DrawBoxAround(vec2 pos, float radius, DWORD color)
{
	float ls = 0.5f/mv_zoom;	// line size
	pos.y = -pos.y;
	can.Draw("")()(color).line(pos+vec2(-radius, -radius), pos+vec2(radius, -radius), ls);
	can.Draw("")()(color).line(pos+vec2(radius, -radius), pos+vec2(radius, radius), ls);
	can.Draw("")()(color).line(pos+vec2(radius, radius), pos+vec2(-radius, radius), ls);
	can.Draw("")()(color).line(pos+vec2(-radius, radius), pos+vec2(-radius, -radius), ls);
}

void MapDraw::DrawPolygon(Polygon2D &p, DWORD color, bool draw_vertexes, bool draw_line_markers)
{
	float ls = 0.5f/mv_zoom;	// line size
	float nl = 4.f/mv_zoom;

	for( int j=0; j<(int)p.edges.size(); j++ )
	{
		vec2 v1 = p.edges[j].start.GetVec2();
		vec2 v2 = p.edges[(j+1)%p.edges.size()].start.GetVec2();
		v1.y = -v1.y;
		v2.y = -v2.y;

		DWORD col = color;
		if( p.edges[j].line.CheckSide(p.edges[(j+2)%p.edges.size()].start)<=0 )
			col = 0xFF0000;
		can.Draw("")()(col).line(v1, v2, ls);

		if( draw_line_markers )
			can.Draw("")()(col).line((v1+v2)/2, (v1+v2)/2+(v1-v2).get_rotated90().get_normalized()*nl, ls);

		if( draw_vertexes )
		{
			vec2 v3 = p.edges[(j+2)%p.edges.size()].start.GetVec2();
			v3.y = -v3.y;
			vec2 d1 = (v2-v1).get_normalized();
			vec2 d2 = (v3-v2).get_normalized();
			vec2 n = (d1+d2).get_normalized().get_rotated90() * nl;
			can.Draw("")()(color).line(v2-n, v2+n, ls);
		}
	}
}

void MapDraw::DrawEdge(vec2 v1, vec2 v2, DWORD color)
{
	float ls = 0.5f/mv_zoom;	// line size
	float nl = 4.f/mv_zoom;
	v1.y = -v1.y;
	v2.y = -v2.y;

	vec2 right = (v1-v2).get_rotated90().get_normalized();
	vec2 mid = (v1+v2)/2;

	can.Draw("")()(color).line(v1, v2, ls);
	//can.Draw("")()(color).line(mid, mid+right*8, ls);
	can.Draw("")()(color).line(v1-right*4, v1+right*4, ls);
	can.Draw("")()(color).line(v2-right*4, v2+right*4, ls);
}


void MapDraw::DrawEye(vec2 eyepos, DWORD color)
{
	float ls = 0.5f/mv_zoom;	// line size
	eyepos.y = -eyepos.y;
	can.Draw("")()(0x0000FF).line(eyepos+vec2(16, 0), eyepos+vec2(0, 8), ls);
	can.Draw("")()(0x0000FF).line(eyepos+vec2(16, 0), eyepos-vec2(0, 8), ls);
	can.Draw("")()(0x0000FF).line(eyepos-vec2(16, 0), eyepos+vec2(0, 8), ls);
	can.Draw("")()(0x0000FF).line(eyepos-vec2(16, 0), eyepos-vec2(0, 8), ls);
	can.Draw("")()(0x0000FF).line(eyepos-vec2(1, 0), eyepos+vec2(1, 0), ls);
	can.Draw("")()(0x0000FF).line(eyepos-vec2(0, 1), eyepos+vec2(0, 1), ls);
}

bool MapDraw::DrawWarning(vec2 pos, DWORD color)
{
	float ls = 0.5f/mv_zoom;	// line size
	float sc = 8.f/mv_zoom;

	bool over = false;
	if( mouse_over && (pos-world_pos).length() < ls*30 )
		over = true;

	pos.y = -pos.y;
	for( int i=0; i<3; i++ )
	{
		vec2 v1 = pos + vec2(sin(i*M_PI*2/3), cos(i*M_PI*2/3))*sc;
		vec2 v2 = pos + vec2(sin((i+1)*M_PI*2/3), cos((i+1)*M_PI*2/3))*sc;
		can.Draw("")()(over ? 0xFFFFFF : color).line(v1, v2, ls);
	}

	return over;
}


void MapDraw::Run(float region_size)
{
	float ls = 0.5f/mv_zoom;	// line size
	//mv_size_x = 320*2;
	//mv_size_y = 240*2;
	{
		int sx, sy;
		Dev.GetScreenSize(sx, sy);
		mv_size_x = sx - MENU_WIDTH;
		mv_size_y = sy;
	}

	can.SetView(mv_center, mv_size_y/mv_zoom);
	can.SetScreenBox(vec2(0, 0), vec2(mv_size_x, mv_size_y));
	can.Draw("")()(0x00404040)(vec2(0, 0), 1000000);	// Background
	can.Draw("")()(0x00000000)(vec2(0, 0), region_size);	// Background

																		// Input
	mouse_pos = Dev.GetMousePosV();
	world_pos = ((mouse_pos-vec2(mv_size_x, mv_size_y)/2)/mv_zoom + mv_center)*vec2(1, -1);
	world_pos.x = _dread_to_float(_float_to_dread(world_pos.x));
	world_pos.y = _dread_to_float(_float_to_dread(world_pos.y));

	if( !mouse_over )
		prev_mouse_pos = mouse_pos;

	mouse_over = (mouse_pos.x<mv_size_x && mouse_pos.y<mv_size_y);
	mouse_delta = mouse_pos - prev_mouse_pos;
	world_delta = mouse_delta/mv_zoom*vec2(1, -1);
	prev_mouse_pos = mouse_pos;

	if( mouse_over )
	{
		Dev.DPrintF("MPos: %.0f, %.0f", world_pos.x, world_pos.y);

		float dz = Dev.GetMouseDeltaZ();
		if( dz )
		{
			float old_zoom = mv_zoom;
			mv_zoom *= pow(2.f, dz/4);
			if( mv_zoom<1.f/256 ) mv_zoom = 1.f/256;
			if( mv_zoom>256 ) mv_zoom = 256;
			mv_center = (mv_center - world_pos*vec2(1, -1))*old_zoom/mv_zoom + world_pos*vec2(1, -1);
		}

		if( Dev.GetKeyState(VK_MBUTTON) )
			mv_center += mouse_delta/mv_zoom*3;

		mouse_lmb = Dev.GetKeyState(VK_LBUTTON);
		mouse_rmb = Dev.GetKeyState(VK_RBUTTON);
	}
	else
	{
		mouse_lmb = false;
		mouse_rmb = false;
	}

	if( Dev.GetKeyStroke(VK_LEFT) )		mv_center.x -= 50.f/mv_zoom;
	if( Dev.GetKeyStroke(VK_RIGHT) )	mv_center.x += 50.f/mv_zoom;
	if( Dev.GetKeyStroke(VK_UP) )		mv_center.y -= 50.f/mv_zoom;
	if( Dev.GetKeyStroke(VK_DOWN) )		mv_center.y += 50.f/mv_zoom;

	// reset edit variables
	edit_vertex_present = false;
	edit_find_vertex = NULL;
	edit_distance = ls*20;
}

void MapDraw::EditVertex(ivec2 &pos, int resolution, DWORD color)
{
	float ls = 0.5f/mv_zoom;	// line size

	vec2 vpos = vec2(pos.x, pos.y)/resolution;
	if( !mouse_lmb )
	{
		float dist = (vpos - world_pos).length();
		if( dist <= edit_distance )
		{
			edit_find_vertex = &pos;
			edit_distance = dist;
		}
	}

	int layer = 1;
	if( &pos == edit_vertex )
	{
		color = 0xFFFFFF;
		layer = 2;
		edit_vertex_present = true;

		if( mouse_lmb )
		{
			vpos = world_pos;
			pos.x = (int)floor(vpos.x*resolution+0.5);
			pos.y = (int)floor(vpos.y*resolution+0.5);
			vpos = vec2(pos.x, pos.y)/resolution;
		}
	}

	DrawVertex(vpos, color, max(1, ls*5), layer);
}

void MapDraw::Flush()
{
	D3DVIEWPORT9 vp ={ 0, 0, mv_size_x, mv_size_y, 0.f, 1.f };
	Dev->SetViewport(&vp);
	Dev.SetRState(0);
	can.Flush();

	Dev.GetScreenSize(*(int*)&vp.Width, *(int*)&vp.Height);
	Dev->SetViewport(&vp);

	// Edit end
	if( !edit_vertex_present )
		edit_vertex = NULL;

	if( !mouse_lmb )
		edit_vertex = edit_find_vertex;
}
