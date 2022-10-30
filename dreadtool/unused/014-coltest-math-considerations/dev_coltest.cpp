
#include "common.h"
#include "app_rendertest.h"




ivec2 CollisionTestTool::coord_process(ivec2 pos)
{
	pos -= coord_origin;
	pos *= coord_flip_sign;
	if( coord_swap )
		swap(pos.x, pos.y);
	return pos;
}

ivec2 CollisionTestTool::coord_unprocess(ivec2 pos)
{
	if( coord_swap )
		swap(pos.x, pos.y);
	pos *= coord_flip_sign;
	pos += coord_origin;
	return pos;
}

void CollisionTestTool::TraceSetup(ivec2 start, ivec2 end, int player_size)
{
	coord_origin = start;
	coord_flip_sign = ivec2(1, 1);
	coord_swap = false;

	trace_delta = end - start;
	trace_psize = player_size;

	if( trace_delta.x<0 )
	{
		trace_delta.x *= -1;
		coord_flip_sign.x = -1;
	}

	if( trace_delta.y<0 )
	{
		trace_delta.y *= -1;
		coord_flip_sign.y = -1;
	}

	if( trace_delta.y > trace_delta.x )
	{
		swap(trace_delta.x, trace_delta.y);
		coord_swap = true;
	}

	trace_cliplen = trace_delta.x;

	Dev.DPrintF("Trace %s%s %s%s",
				(coord_swap ? coord_flip_sign.y : coord_flip_sign.x) < 0 ? "-" : "",
				coord_swap ? "Y" : "X",
				(coord_swap ? coord_flip_sign.x : coord_flip_sign.y) < 0 ? "-" : "",
				coord_swap ? "X" : "Y"
				);
}

void CollisionTestTool::TraceClip(ivec2 v1, ivec2 v2)
{
	// TBD: assure no mul-div sequence can overflow
	// TBD: bounding box check
	// TBD: standing on a line

	v1 = coord_process(v1);
	v2 = coord_process(v2);

	Dev.DPrintF("%d,%d --- %d,%d", v1.x, v1.y, v2.x, v2.y);
	trace_start.x = trace_psize;
	trace_start.y = trace_psize;

	Dev.DPrintF("TDX = %d", trace_delta.x);
	Dev.DPrintF("TDY = %d", trace_delta.y);

	if( trace_delta.x<=0 )
	{
		Dev.DPrintF("trace_delta.x<=0");
		trace_color = 0xFF0000;
		return;
	}

	if( v1.x > v2.x )
	{
		swap(v1.x, v2.x);
		swap(v1.y, v2.y);
		// TBD: remember we swapped left & right
	}

	// ------------------------------------------------ Line orientation: backslash  (v1x<v2x && v1y>v2y) ------------------------------------------------
	if( v1.y >= v2.y )
	{
		int A = v1.y - v2.y;
		int B = v2.x - v1.x;

		v1 -= trace_start;
		if( trace_cliplen < v1.x )
		{
			Dev.DPrintF("BS: line culled  (trace_cliplen < v1.x)");
			trace_color = 0x00C0C0;
			return;
		}
		v2 -= trace_start;
		if( trace_delta.y < v2.y )
		{
			Dev.DPrintF("BS: line culled  (trace_delta.y < v2.y)");
			trace_color = 0x00C0C0;
			return;
		}

		int C = muls(A, v1.x) + muls(B, v1.y);
		if( C<=0 )
		{
			Dev.DPrintF("BS: line behind trace start  (C<=0)");
			trace_color = 0xFF0000;
			return;
		}

		// at this point:	A>=0	B>=0	C>0
		//
		// trace line:		y = x*TDY/TDX
		// clip line:		x*A + y*B - C > 0

		//	x*A + y*B - C = 0
		//	x*A + x*TDY/TDX*B - C = 0
		//	x * (A + TDY/TDX*B) - C = 0
		//	x * (A + TDY/TDX*B) = C 
		//	x = C / (A + TDY/TDX*B)
		int div = A + divs(muls(trace_delta.y, B), trace_delta.x) + 1;		// TBD: all values are positive, use MULU/DIVU
																			//	trace_delta.y <= trace_delta.x assures division overflow won't happen
																			//	max line length is 512*8, so addition won't overflow either
		Dev.DPrintF("A = %d", A);
		Dev.DPrintF("B = %d", B);
		Dev.DPrintF("C = %d", C);
		Dev.DPrintF("div = %d", div);

		if( div <= 0 )
		{
			Dev.DPrintF("BS: div < 0  (impossible case)");		// This is impossible. All values to compute <div> are positive and we add 1 at the end.
			trace_color = 0xFF00FF;
			return;
		}

		int x = divs(C, div);			// TBD: both values are positive, use DIVU
		// Overflow considerations:		// TBD: division overflow!	(can bounding box check prevent that?)
		//	A <= 4096		max line length 512*8
		//	B <= 4096		-
		//	div > A			implied by <div> formula
		//	tdy < tdx		assured by clip space flips
		//	v1x < 65536		region size
		//	v1y < 65536		-
		//	v1x <= v2x		assured by line case
		//	v1y >= v2y		-
		//	v1x <= tdx		SHOULD be assured by clipping
		//	v2y <= tdy		-
		//
		//	A = v1y - v2y
		//	B = v2x - v1x
		//	C = A*v1x + B*v1y = A*v2x + B*v2y
		//	div = A + tdy*B/tdx + 1
		//
		//	C / div : worst case:
		//		(A*v1x + B*v1y) / (A+1)
		//		(0*v1x + 4096*v1y) / (0+1)
		//
		//		(A*v1x + B*v1y) / (A + tdy*B/tdx + 1)		<<< Prove it does not overflow!				<< it caps at slightly over 12000 when trace length is limited to 1000*8 (tested by random probing & improvement)
		//		B*v1y / (tdy*B/tdx + 1)					assuming worst case A = 0
		//
		//	Can we prove:
		//		B*v1y / (tdy*B/tdx + 1) <= 65535
		//		B*v1y <= 65535 * (tdy*B/tdx + 1)		assuming tdy>=v1y (v2y, but A=0)	worst case tdy=v1y
		//		B*v1y <= 65535 * (v1y*B/tdx + 1)
		//		B <= 65535 * (B/tdx + 1/v1y)
		//		B <= 65535 * B/tdx						ignoring 1/v1y
		//		1 <= 65535 / tdx
		//		tdx <= 65535							this is true
		//
		//	Used assumption:
		//		A/X < C		B/Y < C		->		(A+B)/(X+Y) < C
		//		A/X < 1		B/Y < 1		->		(A+B)/(X+Y) < 1			scaling everything to C=1
		//		A<X			B<Y			->		A+B < X+Y				this is true
		//
		//		(A*v1x + B*v1y) / (A + tdy*B/tdx + 1) <= 65535
		//		(A*v1x + B*v1y) / (A + tdy*B/tdx) <= 65535
		//		((v1y - v2y)*v1x + (v2x - v1x)*v1y) / ((v1y - v2y) + (v2x - v1x)*tdy/tdx) <= 65535
		//		((v1y - v2y)*v1x + (v2x - v1x)*v1y) / ((v1y - v2y) + (v2x - v1x)*v2y/tdx) <= 65535			lowering divider
		//		(v1y - v2y)*v1x + (v2x - v1x)*v1y <= 65535 * ( (v1y - v2y) + (v2x - v1x)*v2y/tdx )
		//		(v1y - v2y)*v1x + (v2x - v1x)*v1y <= 65535 * (v1y - v2y) + 65535 * (v2x - v1x)*v2y/tdx
		//		(v1y - v2y)*v1x + (v2x - v1x)*v1y <= 65535 * (v1y - v2y) + 65535 * (v2x - v1x)*(v1y - (v1y - v2y))/tdx
		//		(v1y - v2y)*v1x + (v2x - v1x)*v1y <= 65535 * (v1y - v2y) + 65535*(v2x - v1x)*v1y/tdx - 65535*(v2x - v1x)*(v1y - v2y)/tdx
		//		(v1y - v2y)*v1x + (v2x - v1x)*v1y <= 65535*(v1y - v2y) - 65535*(v1y - v2y)*(v2x - v1x)/tdx + 65535*(v2x - v1x)*v1y/tdx
		//		(v1y - v2y)*v1x + (v2x - v1x)*v1y <= 65535*(v1y - v2y)*(1 - (v2x - v1x)/tdx) + 65535*(v2x - v1x)*v1y/tdx
		//			(v1y - v2y)*v1x <= 65535*(v1y - v2y)*(1 - (v2x - v1x)/tdx)
		//				v1x <= 65535*(1 - (v2x - v1x)/tdx)
		//				v1x <= 65535*(1 - (v2x - v1x)/v1x)			culling: tdx >= v1x			dividing by less makes subtraction larger
		//				v1x <= 65535*(1 - v2x/v1x + v1x/v1x)
		//				v1x <= 65535*(2 - v2x/v1x)
		//				v1x <= 65535*2 - 65535*v2x/v1x
		//				v1x + 65535*v2x/v1x <= 65535*2
		//
		//				v1x <= 65535*(1 - (v2x - v1x)/tdx)
		//				v1x <= 65535*(1 - v2x/tdx + v1x/tdx)
		//				v1x <= 65535 - 65535*v2x/tdx + 65535*v1x/tdx
		//				v1x - 65535*v1x/tdx <= 65535 - 65535*v2x/tdx
		//				v1x*(1 - 65535/tdx) <= 65535*(1 - v2x/tdx)
		//				v1x*(1/65535 - 1/tdx) <= 1 - v2x/tdx
		//
		//			(v2x - v1x)*v1y <= 65535*(v2x - v1x)*v1y/tdx
		//				1 <= 65535/tdx
		//				tdx <= 65535
		//
		if( x >= trace_cliplen )
		{
			Dev.DPrintF("BS: trace doesn't reach the line  (unclipped)");
			trace_color = 0x00C0C0;
			return;
		}

		// int xmin = min(v1.x, v2.x);		xmin = v1.x
		if( x < v1.x )
		{
			// Fell short of the segment on X axis
			x = v1.x-1;		// x<v1.x already, so this can't decrease X (and it can't go negative)
			int y = divs(muls(x, trace_delta.y), trace_delta.x);	// TBD:  all values are positive here, use MULU/DIVU
																	//	trace_delta.y <= trace_delta.x assures division overflow won't happen
			// int ymax = max(v1.y, v2.y);		ymax = v1.y
			if( y-(trace_psize<<1) > v1.y )
			{
				// Passed over the segment, left side
				Dev.DPrintF("BS: passed left of the line  (in test space)");
				trace_color = 0x00C0C0;
				return;
			}

			// Known constraints here:
			//	old_x < trace_cliplen
			//	old_x < v1.x
			//	x = v1.x - 1
			//
			//	v1.x = x + 1
			//	old_x < x + 1
			//

			if( x < trace_cliplen )
			{
				trace_cliplen = x;
				Dev.DPrintF("BS: hit left vertex of the line  (V1)");
				trace_color = 0xFFFF00;
				return;
			}

			Dev.DPrintF("BS: unclipped  (didn't rech V1)");
			trace_color = 0x00C0C0;
			return;
		}

		// if( x < trace_cliplen )		// the opposite was already checked and handled
		int y = divs(muls(x, trace_delta.y), trace_delta.x);	// TBD:  all values are positive here, use MULU/DIVU
																//	trace_delta.y <= trace_delta.x assures division overflow won't happen
		// int ymin = min(v1.y, v2.y);		// ymin = v2.y
		if( y < v2.y )
		{
			// y = x*TDY/TDX
			// x = y*TDX/TDY
			if( trace_delta.y <= 0 )		// This can't really be negative, but can be zero and break division below
			{
				Dev.DPrintF("BS: passed right of the line  (V2, tdy=0)");
				trace_color = 0x00C0C0;
				return;
			}

			// y>=0 && y<v2.y	->	v2.y > 0
			x = divs(muls(v2.y-1, trace_delta.x), trace_delta.y);		// TBD:  all values are positive here, use MULU/DIVU		TBD: division by 0
																		// TBD:  overflow
			// int xmax = max(v1.x, v2.x);		// xmax = v2.x
			if( x-(trace_psize<<1) > v2.x )
			{
				// Passed under the segment, right side
				Dev.DPrintF("BS: passed right of the line  (V2)");
				trace_color = 0x00C0C0;
				return;
			}

			if( x < trace_cliplen )
			{
				trace_cliplen = x;
				Dev.DPrintF("BS: hit right vertex of the line  (V2)");
				trace_color = 0xFFFF00;
				return;
			}

			Dev.DPrintF("BS: unclipped  (didn't rech V2)");
			trace_color = 0x00C0C0;
			return;
		}

		trace_cliplen = x;
		Dev.DPrintF("BS: direct hit on the line  (clipped)");
		trace_color = 0xFFFF00;
		return;
	}


	// Trace point options:
	//	X left		trace_start.x -= 2*trace_psize		v1.x += 2*trace_psize		C += A*2*trace_psize
	//	Y down		trace_start.y -= 2*trace_psize		v1.y += 2*trace_psize		C += B*2*trace_psize
	//
	// minimize C
	//
	int A = v1.y - v2.y;
	int B = v2.x - v1.x;
	int C = muls(A, v1.x) + muls(B, v1.y);
	if( A <= B )
	{
		C += A*2*trace_psize;
		trace_start.x -= 2*trace_psize;
	}
	else
	{
		C += B*2*trace_psize;
		trace_start.y -= 2*trace_psize;
	}

	int div = A + divs(muls(trace_delta.y, B), trace_delta.x) + 1;

	//Dev.DPrintF("TDX = %d", trace_delta.x);
	//Dev.DPrintF("TDY = %d", trace_delta.y);
	//Dev.DPrintF("A = %d", A);
	//Dev.DPrintF("B = %d", B);
	//Dev.DPrintF("C = %d", C);
	//Dev.DPrintF("div = %d", div);

	if( div <= 0 )
	{
		Dev.DPrintF("div < 0  (case 2)");
		trace_color = 0xFF00FF;
		return;
	}

	int x = divs(C, div);
	if( x < trace_cliplen )
	{
		trace_cliplen = x;
		Dev.DPrintF("Clipped (case 2)");
		trace_color = 0xFFFF00;
		return;
	}

	Dev.DPrintF("unknown");
	trace_color = 0x00C0C0;
	return;
}

void CollisionTestTool::TraceEnd()
{
	trace_delta.y = divs(muls(trace_cliplen, trace_delta.y), trace_delta.x);
	trace_delta.x = trace_cliplen;
}



void CollisionTestTool::Serialize(TreeFileRef n)
{
	n.SerInt("v1.x", v1.x, 0);
	n.SerInt("v1.y", v1.y, 0);
	n.SerInt("v2.x", v2.x, 0);
	n.SerInt("v2.y", v2.y, 0);
	n.SerInt("start_pos.x", start_pos.x, 0);
	n.SerInt("start_pos.y", start_pos.y, 0);
	n.SerInt("end_pos.x", end_pos.x, 0);
	n.SerInt("end_pos.y", end_pos.y, 0);
}

void CollisionTestTool::RunView()
{
	static bool init = true;
	const int region_size = 256;

	mv.Run(region_size);

	if( init )
	{
		init = false;

		mv.CenterRegion(vec2(-region_size, -region_size), vec2(region_size, region_size));
	}

	mv.EditVertex(v1, 1, 0x00FF00);
	mv.EditVertex(v2, 1, 0x00FF00);
	mv.EditVertex(start_pos, 1<<DreadMapGen::ENGINE_SUBVERTEX_PRECISION, 0x0000FF);
	mv.EditVertex(end_pos, 1<<DreadMapGen::ENGINE_SUBVERTEX_PRECISION, 0xFF0000);

	int ps = DreadMapGen::ENGINE_PLAYER_SIZE << DreadMapGen::ENGINE_SUBVERTEX_PRECISION;
	mv.DrawLine(_dread_to_vec2(start_pos+ivec2(ps, ps)), _dread_to_vec2(end_pos+ivec2(ps, ps)), 0x202020);
	mv.DrawLine(_dread_to_vec2(start_pos+ivec2(ps, -ps)), _dread_to_vec2(end_pos+ivec2(ps, -ps)), 0x202020);
	mv.DrawLine(_dread_to_vec2(start_pos+ivec2(-ps, ps)), _dread_to_vec2(end_pos+ivec2(-ps, ps)), 0x202020);
	mv.DrawLine(_dread_to_vec2(start_pos+ivec2(-ps, -ps)), _dread_to_vec2(end_pos+ivec2(-ps, -ps)), 0x202020);

	mv.DrawBoxAround(_dread_to_vec2(start_pos), DreadMapGen::ENGINE_PLAYER_SIZE, 0x808040);
	mv.DrawLine(_ivec2_to_vec2(v1), _ivec2_to_vec2(v2), 0x404040, true);
	mv.DrawLine(_dread_to_vec2(start_pos), _dread_to_vec2(end_pos), 0x808080);

	TraceSetup(start_pos, end_pos, DreadMapGen::ENGINE_PLAYER_SIZE<<DreadMapGen::ENGINE_SUBVERTEX_PRECISION);
	TraceClip(v1*(1<<DreadMapGen::ENGINE_SUBVERTEX_PRECISION), v2*(1<<DreadMapGen::ENGINE_SUBVERTEX_PRECISION));
	TraceEnd();

	mv.DrawLine(_dread_to_vec2(coord_unprocess(trace_start)), _dread_to_vec2(coord_unprocess(trace_start+trace_delta)), trace_color);

	mv.DrawBoxAround(_dread_to_vec2(coord_unprocess(trace_delta)), DreadMapGen::ENGINE_PLAYER_SIZE, 0x808040);

	mv.Flush();
}

void CollisionTestTool::RunMenu()
{
	ms.Label("Collision test");
}
