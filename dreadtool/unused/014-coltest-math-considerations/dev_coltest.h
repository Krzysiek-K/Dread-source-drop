
#pragma once


class MapDraw;


class CollisionTestTool
{
public:
	// test editor
	ivec2		start_pos;
	ivec2		end_pos;
	ivec2		v1, v2;
	MapDraw		mv;

	// collision routine
	ivec2		coord_origin;			// (0,0) point in trace space
	ivec2		coord_flip_sign;		// 1/-1 multiplier per component
	bool		coord_swap;				// swap X<->Y for tracing			( tracing assumes:  X>=0  Y>=0  X>=Y )
	ivec2		trace_delta;			// move offset in trace space
	int			trace_psize;			// player size
	int			trace_cliplen;			// length of clipped movement	(X coord, in trace space)

	ivec2		trace_start;			// [debug] origin point of traced line, in trace space
	DWORD		trace_color;			// [debug] debug line drawing color


	ivec2 coord_process(ivec2 pos);
	ivec2 coord_unprocess(ivec2 pos);
	void  TraceSetup(ivec2 start, ivec2 end, int player_size);
	void  TraceClip(ivec2 v1, ivec2 v2);
	void  TraceEnd();


	void Serialize(TreeFileRef n);
	void RunView();
	void RunMenu();
};
