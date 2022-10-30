
#pragma once



// ---------------------------------------------------------------- MapDraw ----------------------------------------------------------------


class MapDraw
{
public:
	DevCanvas	can;
	int			mv_size_x = 1;
	int			mv_size_y = 1;
	float		mv_zoom = 0.3f;
	vec2		mv_center = vec2(0, 0);

	vec2		mouse_pos = vec2(0, 0);
	vec2		prev_mouse_pos = vec2(0, 0);
	vec2		mouse_delta = vec2(0, 0);
	vec2		world_pos = vec2(0, 0);
	vec2		world_delta = vec2(0, 0);
	bool		mouse_over = false;
	bool		mouse_lmb = false;
	bool		mouse_rmb = false;

	ivec2		*edit_vertex = NULL;
	bool		edit_vertex_present = false;
	ivec2		*edit_find_vertex = NULL;
	float		edit_distance;



	void CenterRegion(vec2 vmin, vec2 vmax);
	void DrawVertex(vec2 v, DWORD color, float size=0.25f, int layer=0);
	void DrawLine(vec2 v1, vec2 v2, DWORD color, bool mark_direction=false);
	void DrawBoxAround(vec2 pos, float radius, DWORD color);
	void DrawPolygon(Polygon2D &p, DWORD color, bool draw_vertexes, bool draw_line_markers);
	void DrawEdge(vec2 v1, vec2 v2, DWORD color);
	void DrawEye(vec2 eyepos, DWORD color);
	bool DrawWarning(vec2 pos, DWORD color);

	void Run(float region_size);
	void EditVertex(ivec2 &pos, int resolution, DWORD color);

	void Flush();
};



float _fan_order(const vec2 &d1, const vec2 &d2);
int _float_to_dread(float coord);
float _dread_to_float(int coord);
vec2 _dread_to_vec2(ivec2 pos);
vec2 _ivec2_to_vec2(ivec2 pos);
