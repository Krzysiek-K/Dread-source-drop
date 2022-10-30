
#include "common.h"
#include "app_rendertest.h"



void CollisionTestTool::TestCase::Serialize(TreeFileRef n)
{
	n.SerString("name", name, "");
	n.SerInt("v1.x", v1.x, 0);
	n.SerInt("v1.y", v1.y, 0);
	n.SerInt("v2.x", v2.x, 0);
	n.SerInt("v2.y", v2.y, 0);
	n.SerInt("start_pos.x", start_pos.x, 0);
	n.SerInt("start_pos.y", start_pos.y, 0);
	n.SerInt("end_pos.x", end_pos.x, 0);
	n.SerInt("end_pos.y", end_pos.y, 0);
}





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

	// ------------------------------------------------ Line orientation: backslash  (v1x<=v2x && v1y>=v2y) ------------------------------------------------
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
		//
		//	x*A + y*B - C = 0
		//	x*A + x*TDY/TDX*B - C = 0
		//	x * (A + TDY/TDX*B) - C = 0
		//	x * (A + TDY/TDX*B) = C 
		//	x = C / (A + TDY/TDX*B)
		int div = A + divu(mulu(trace_delta.y, B), trace_delta.x) + 1;		//	Overflow:	trace_delta.y <= trace_delta.x assures division overflow won't happen
																			//				max line length is 512*8, so addition won't overflow either
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

		int x = divu(C, div);			// Overflow:	it caps at slightly over 12000 when trace length is limited to 1000*8	(tested by random probing & improvement)
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
			int y = divu(mulu(x, trace_delta.y), trace_delta.x);	// Overflow:	trace_delta.y <= trace_delta.x assures division overflow won't happen
			if( y-(trace_psize<<1) > v1.y )		// ymax = v1.y
			{
				// Passed over the segment, left side
				Dev.DPrintF("BS: passed left of the line  (in test space)");
				trace_color = 0x00C0C0;
				return;
			}

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

		int y = divu(mulu(x, trace_delta.y), trace_delta.x);	// Overflow:	trace_delta.y <= trace_delta.x assures division overflow won't happen
		if( y < v2.y )		// ymin = v2.y
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
			x = divu(mulu(v2.y-1, trace_delta.x), trace_delta.y);		// TBD:  overflow
			if( x-(trace_psize<<1) > v2.x )		// xmax = v2.x
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

	// ------------------------------------------------ Line orientation: slash (v1x<=v2x && v1y<v2y) ------------------------------------------------
	
	//
	// Find which side of the front corner trace line v1 is
	//
	v1 -= trace_start;
	if( trace_cliplen < v1.x )
	{
		Dev.DPrintF("BS: line culled  (trace_cliplen < v1.x)");
		trace_color = 0x00C0C0;
		return;
	}
//	v2 -= trace_start;
//	if( trace_delta.y < v2.y )
//	{
//		Dev.DPrintF("BS: line culled  (trace_delta.y < v2.y)");
//		trace_color = 0x00C0C0;
//		return;
//	}

	int dot = v1.x*trace_delta.y - v1.y*trace_delta.x;
	if( dot >= 0 )
	{
		// Vertical (right) side is front
		trace_start.y = -trace_psize;
		v1.y += 2*trace_psize;
		dot -= 2*trace_psize*trace_delta.x;
		if( dot<0 )
		{
			if( v1.x <= 0 )
			{
				Dev.DPrintF("BS: V1 behind right face  (v1.x <= 0)");
				trace_color = 0x00C0C0;
				return;
			}
			trace_cliplen = v1.x-1;			// TBD: v1x == 0 ?
			Dev.DPrintF("BS: right face stopped at vertex (V1)");
			trace_color = 0xFFFF00;
			return;
		}

		v2 -= trace_start;
		int A = v2.y - v1.y;	// A >  0			this line equation has different sign than the backslash case version
		int B = v2.x - v1.x;	// B >= 0			also B is inverted

		int C = muls(A, v1.x) - muls(B, v1.y);
		if( C<=0 )
		{
			Dev.DPrintF("BS: line behind trace start  (C<=0, slash, right is front)");
			trace_color = 0xFF0000;
			return;
		}

		int div = A - divs(muls(trace_delta.y, B), trace_delta.x) + 1;		//	Overflow:	TBD				TBD: div/mul can be unsigned
		Dev.DPrintF("A = %d", A);
		Dev.DPrintF("B = %d", B);
		Dev.DPrintF("C = %d", C);
		Dev.DPrintF("div = %d", div);

		if( div <= 0 )
		{
			// Corner is moving away from the line (collision with V1 already checked)
			Dev.DPrintF("BS: moving away from the line  (div <= 0, slash, right is front)");
			trace_color = 0xFF00FF;
			return;
		}

		int x = divs(C, div);			// Overflow:	TBD				TBD: can be unsigned
		if( x >= trace_cliplen )
		{
			Dev.DPrintF("BS: trace doesn't reach the line  (slash, right is front)");
			trace_color = 0x00C0C0;
			return;
		}
		if( x >= v2.x )
		{
			Dev.DPrintF("BS: trace passes above V2  (slash, right is front)");
			trace_color = 0x00C0C0;
			return;
		}

		trace_cliplen = x;
		Dev.DPrintF("BS: hit left vertex of the line  (slash, right is front)");
		trace_color = 0xFFFF00;
		return;
	}
	else
	{
		// Horizontal (top) side is front		TBD
		trace_start.x = -trace_psize;
		v1.x += 2*trace_psize;
	}

//	int div = A + divs(muls(trace_delta.y, B), trace_delta.x) + 1;
//
//	//Dev.DPrintF("TDX = %d", trace_delta.x);
//	//Dev.DPrintF("TDY = %d", trace_delta.y);
//	//Dev.DPrintF("A = %d", A);
//	//Dev.DPrintF("B = %d", B);
//	//Dev.DPrintF("C = %d", C);
//	//Dev.DPrintF("div = %d", div);
//
//	if( div <= 0 )
//	{
//		Dev.DPrintF("div < 0  (case 2)");
//		trace_color = 0xFF00FF;
//		return;
//	}
//
//	int x = divs(C, div);
//	if( x < trace_cliplen )
//	{
//		trace_cliplen = x;
//		Dev.DPrintF("Clipped (case 2)");
//		trace_color = 0xFFFF00;
//		return;
//	}

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
	n.SerInt("active_case", active_case, -1);

	if( n.IsReading() )
	{
		int count = n.GetCloneArraySize("test_cases");
		test_cases.clear();
		test_cases.resize(count);
	}
	for( int i=0; i<test_cases.size(); i++ )
		test_cases[i].Serialize(n.SerChild("test_cases", i));
}

void CollisionTestTool::LoadTestCase(TestCase &t)
{
	start_pos = t.start_pos;
	end_pos = t.end_pos;
	v1 = t.v1;
	v2 = t.v2;
}

void CollisionTestTool::UpdateTestCase(TestCase &t)
{
	t.start_pos = start_pos;
	t.end_pos = end_pos;
	t.v1 = v1;
	t.v2 = v2;
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

	ms.EndPanel();
	ms.Panel();
	ms.Label("Test cases");
	ms.ListStart();
	for( int i=0; i<test_cases.size(); i++ )
		if( ms.ListItem(test_cases[i].name.c_str(), 0xFFFF80, (i==active_case)) )
			if( i!=active_case )
			{
				active_case = i;
				LoadTestCase(test_cases[i]);
			}
			else
				active_case = -1;
	if( ms.ListItem("+", 0xFFFF80, false) )
	{
		TestCase t;
		t.name = format("Case #%d", test_cases.size()+1);
		UpdateTestCase(t);
		test_cases.push_back(t);
	}
	ms.ListEnd();
	ms.LineSetup(-1, -1, -1, -1);
	if( ms.Button(NULL, "^") && active_case>0 )
	{
		swap(test_cases[active_case], test_cases[active_case-1]);
		active_case--;
	}
	if( ms.Button(NULL, "v") && active_case>=0 && active_case+1<test_cases.size() )
	{
		swap(test_cases[active_case], test_cases[active_case+1]);
		active_case++;
	}
	if( ms.Button(NULL, "Upd", 0, 2) && active_case>=0 )
		UpdateTestCase(test_cases[active_case]);
	if( ms.Button(NULL, "Del", 0, 4) && active_case>=0 )
	{
		test_cases.erase(test_cases.begin()+active_case);
		active_case = -1;
	}
	if( active_case>=0 )
		ms.EditBox(test_cases[active_case].name);
}
