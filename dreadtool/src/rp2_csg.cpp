
#include <math.h>

#include "rp2_csg.h"


using namespace std;




// **************** PointRP2 ****************

// Create line running from this point to the other point.
// Works only if both points were created by giving
// coordinates explicitly (i.e. doesn't work for intersection points).
LineRP2 PointRP2::LineTo(const PointRP2 &p) const
{
	return LineRP2(v.cross(p.v));
}


// **************** LineRP2 ****************

// Find intersection point of two lines.
PointRP2 LineRP2::Intersect(const LineRP2 &l) const
{
	PointRP2 out( normal.cross(l.normal) );
	if(out.v.z<0)
		out.v = -out.v;
	return out;
}

// Check which side of the line the point lies on:
//	result > 0		- right side
//	result = 0		- on the line
//	result < 0		- left side
coord_t LineRP2::CheckSide(const PointRP2 &p) const
{
	return -normal.dot(p.v);		// CCW winding: 0Y is on the left side of 0X
}

float LineRP2::CheckSideVec2(const vec2 &p) const
{
	return -float(double(normal.x)*p.x + double(normal.y)*p.y + double(normal.z));
}

float LineRP2::CheckDistanceVec2(const vec2 &p) const
{
	double nx = double(normal.x);
	double ny = double(normal.y);
	double nz = double(normal.z);
	double len = sqrt(nx*nx+ny*ny);

	return -float(nx*p.x + ny*p.y + nz)/len;
}



// **************** Polygon2D ****************

// Create polygon from list of vertices
// (vertices should be ordered clockwise,
//  should form a convex shape, no three
//  vertices can be colinear and no vertex
//  should be specified twice)
Polygon2D::Polygon2D(PointRP2 *vtx,int n_vtx) : _user(NULL)
{
	CreateFromPoints(vtx, n_vtx);
}

// Create polygon from list of vertices
// (vertices should be ordered clockwise,
//  should form a convex shape, no three
//  vertices can be colinear and no vertex
//  should be specified twice).
void Polygon2D::CreateFromPoints(PointRP2 *vtx, int n_vtx)
{
	Edge2D e;
	edges.clear();
	for( int i=0; i<n_vtx; i++ )
	{
		e.start = vtx[i];
		e.line = vtx[i].LineTo(vtx[(i+1)%n_vtx]);
		edges.push_back(e);
	}
	UpdateBox();
}

// Cleanup polygon in case colinear points were passed to CreateFromPoints().
void Polygon2D::CleanupColinear()
{
	for( int i=0; i<(int)edges.size(); i++ )
	{
		int j = (i+1)%edges.size();
		if( edges[j].line.CheckSide(edges[i].start) == 0 )
		{
			edges.erase(edges.begin()+j);
			i--;	// check index <i> again
		}
	}
}


// TBD: comment
void Polygon2D::CreateRectangle(int x1, int y1, int x2, int y2)
{
	PointRP2 pts[4] = {		// X-to-Y CCW winding
		PointRP2(x1, y1),
		PointRP2(x1, y2),
		PointRP2(x2, y2),
		PointRP2(x2, y1),
	};
	CreateFromPoints(pts, 4);
}


// Split polygon with a line.
// Resulting polygons can be empty if no part
// of initial polygon lies on given side.
void Polygon2D::SplitByLine(const LineRP2 &line, Polygon2D &right, Polygon2D &left) const
{
	int n_left = 0, n_right = 0;
	Edge2D e;

	// clear output polygons, in case they weren't empty
	right.edges.clear();
	right._user = _user;
	right.UpdateBox();

	left.edges.clear();
	left._user = _user;
	left.UpdateBox();

	if( IsEmpty() )
		return;

	// check if polygon lies completely on one side of the line
	for( unsigned int i=0; i<edges.size(); i++ )
	{
		coord_t s = line.CheckSide(edges[i].start);
		if( s>0 ) n_right++;
		if( s<0 ) n_left++;
	}

	if( n_left==0 )
	{
		right = *this;
		return;
	}

	if( n_right==0 )
	{
		left = *this;
		return;
	}

	// assign edges to output polygons and split them when neccessary,
	// edges closing polygons are thrown in when edge crossing the line is detected
	// (crossing left->right closes the left polygon and crossing right->left closes the right one)

	for( unsigned int i=0; i<edges.size(); i++ )
	{
		coord_t s1 = line.CheckSide(edges[i].start);
		coord_t s2 = line.CheckSide(edges[(i+1)%edges.size()].start);

		if( s1<0 && s2>0 )		// Crossing left to right
		{
			PointRP2 mid = line.Intersect(edges[i].line);

			// part of the edge on left side
			e.start = edges[i].start;
			e.line = edges[i].line;
			left.edges.push_back(e);

			// midpoint is start of the edge closing left polygon
			e.start = mid;
			e.line = line.Reversed();
			left.edges.push_back(e);

			// part of the edge on right side
			e.start = mid;
			e.line = edges[i].line;
			right.edges.push_back(e);
		}
		else if( s1>0 && s2<0 )
		{
			PointRP2 mid = line.Intersect(edges[i].line);

			// part of the edge on right side
			e.start = edges[i].start;
			e.line = edges[i].line;
			right.edges.push_back(e);

			// midpoint is start of the edge closing right polygon
			e.start = mid;
			e.line = line;
			right.edges.push_back(e);

			// part of the edge on left side
			e.start = mid;
			e.line = edges[i].line;
			left.edges.push_back(e);
		}
		else if( s1<0 || s2<0 )
		{
			// edge is completely of the left side
			left.edges.push_back(edges[i]);

			if( s1==0 )
			{
				// starting point lies on the line, so use it as
				// starting point of edge closing right polygon
				e.start = edges[i].start;
				e.line = line;
				right.edges.push_back(e);
			}
		}
		else if( s1>0 || s2>0 )
		{
			// edge is completely on the right side
			right.edges.push_back(edges[i]);

			if( s1==0 )
			{
				// starting point lies on the line, so use it as
				// starting point of edge closing left polygon
				e.start = edges[i].start;
				e.line = line.Reversed();
				left.edges.push_back(e);
			}
		}
		else
		{
			// remaining case: s1==0 && s2==0
			// this can happen only if initial polygon was incorrect
		}
	}

	// update bounding boxes of resulting polygons
	left.UpdateBox();
	right.UpdateBox();
}

// TBD: comments
bool Polygon2D::ComputeIntersection(const Polygon2D &other, Polygon2D &out) const
{
	Polygon2D inner, outer;

	if( box_max_x <= other.box_min_x ||
		box_min_x >= other.box_max_x ||
		box_max_y <= other.box_min_y ||
		box_min_y >= other.box_max_y )
		return false;

	out = other;

	for( int i=0; i<(int)edges.size() && !out.IsEmpty(); i++ )
	{
		out.SplitByLine(edges[i].line, inner, outer);
		out.edges.swap(inner.edges);
	}

	out.UpdateBox();

	return !out.IsEmpty();
}

// TBD: comments
bool Polygon2D::ClipBy(const LineRP2 &line)
{
	Polygon2D inner, outer;
	SplitByLine(line, inner, outer);
	*this = inner;
	return !IsEmpty();
}

// Returns which side of the line the polygon is on:
//	-1	- completely on left side
//	 0	- partially on both sides	(or on the line, if the polygon is degenerated)
//	 1	- completely on right side
int Polygon2D::CheckSide(const LineRP2 &line) const
{
	int pside = 0;
	for( int i=0; i<(int)edges.size(); i++ )
	{
		coord_t side = line.CheckSide(edges[i].start);
		if( !side ) continue;
		
		if( side<0 )
		{
			if( pside>0 ) return 0;
			pside = -1;
		}
		else // side>0
		{
			if( pside<0 ) return 0;
			pside = 1;
		}
	}
	return pside;
}



// update bounding box of the polygon,
// this function is called automatically
void Polygon2D::UpdateBox()
{
	if( IsEmpty() )
	{
		box_min_x = box_max_x = 0;
		box_min_y = box_max_y = 0;
		return;
	}

	double min_x =  1e20;
	double min_y =  1e20;
	double max_x = -1e20;
	double max_y = -1e20;

	for(int i=0;i<(int)edges.size();i++)
	{
		double x = edges[i].start.GetX();
		if(x<min_x) min_x = x;
		if(x>max_x) max_x = x;

		double y = edges[i].start.GetY();
		if(y<min_y) min_y = y;
		if(y>max_y) max_y = y;
	}

	box_min_x = int(floor(min_x)) - 1;
	box_min_y = int(floor(min_y)) - 1;
	box_max_x = int(ceil (max_x)) + 1;
	box_max_y = int(ceil (max_y)) + 1;
}


// **************** PolygonMesh2D ****************

// add polygon to the mesh
void PolygonMesh2D::CSGAdd(const Polygon2D &p)
{
	// make hole for new polygon
	CSGSubtract(p);

	// add the polygon
	polys.push_back(p);
}

// make hole in the mesh
void PolygonMesh2D::CSGSubtract(const Polygon2D &p)
{
	vector<Polygon2D> result;
	vector<Polygon2D> outA;
	Polygon2D C, left, right;

	for(int i=0;i<polys.size();i++)
	{
		// if bounding boxes don't touch, polygon lies outside
		// of the polygon being subtracted, so add it to the solution
		if( !polys[i].IsBoxCollision(p) )
		{
			result.push_back(polys[i]);
			continue;
		}

		// init polygon C and set outA
		C = polys[i];
		outA.clear();

		// cur polygon C with edges of polygon being subtracted
		int j = 0;
		while( !C.IsEmpty() && j<p.edges.size() )
		{
			C.SplitByLine(p.edges[j].line, right, left);
			C = right;
			if( !left.IsEmpty() )
				outA.push_back(left);

			j++;
		}

		// if C is left empty it means all parts of initial polygon
		// were added to set outA, so add entire polygon instead of
		// set outA to save some splits
		if( C.IsEmpty() )
			result.push_back(polys[i]);
		else
			result.insert(result.end(), outA.begin(), outA.end());
	}

	// replace old mesh with new result
	polys.swap(result);
}
