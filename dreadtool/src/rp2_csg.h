
#ifndef _CSG_H
#define _CSG_H


#include <base.h>
#include <vector>

using namespace base;



// This implrementation assumes:
//	- X-to-Y axis CCW order
//	- RIGHT side of polygon edges is the interior
//









// **************** Vector3D ****************

template<class _T>
class Vector3D {
public:
	_T		x, y, z;

	Vector3D() {}
	Vector3D(_T _x,_T _y,_T _z) : x(_x), y(_y), z(_z) {}

	Vector3D<_T> operator -() const { return Vector3D<_T>(-x,-y,-z); }

	_T dot(const Vector3D<_T> &v) const { return x*v.x + y*v.y + z*v.z; }

	Vector3D<_T> cross(const Vector3D<_T> &v) const
	{
		return Vector3D<_T>(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
	}

	void simplify()
	{
		coord_t a = (z>=0) ? z : -z;
		coord_t b = (x>=0) ? x : -x;
		while( b>0 )
		{
			coord_t t = b;
			b = a % b;
			a = t;
		}
		b = (y>=0) ? y : -y;
		while( b>0 )
		{
			coord_t t = b;
			b = a % b;
			a = t;
		}
		if( a>1 )
		{
			x /= a;
			y /= a;
			z /= a;
		}
	}
};


typedef long long	coord_t;





// **************** PointRP2 ****************

class LineRP2;

class PointRP2 {
public:
	Vector3D<coord_t>	v;


	PointRP2() {}
	PointRP2(int xr,int yr) : v(xr,yr,1) {}
	PointRP2(const Vector3D<coord_t> &_v) : v(_v) {}

	LineRP2 LineTo(const PointRP2 &p) const;

	double GetX() const { return double(v.x)/v.z; }
	double GetY() const { return double(v.y)/v.z; }
	vec2   GetVec2() const { return vec2(float(v.x)/v.z, float(v.y)/v.z); }

	bool IsSameAs(const PointRP2 &p) const
	{
		// v.x/v.z == p.v.x/p.v.z
		// v.y/v.z == p.v.y/p.v.z
		return v.x*p.v.z == p.v.x*v.z &&
			   v.y*p.v.z == p.v.y*v.z;
	}
};


// **************** LineRP2 ****************

class LineRP2 {
public:
	Vector3D<coord_t>	normal;


	LineRP2() {}
	LineRP2(const Vector3D<coord_t> &_normal) : normal(_normal) {}

	PointRP2 Intersect(const LineRP2 &l) const;
	coord_t  CheckSide(const PointRP2 &p) const;
	float CheckSideVec2(const vec2 &p) const;
	float CheckDistanceVec2(const vec2 &p) const;

	LineRP2  Reversed() const { return LineRP2(-normal); }
	
	void Flip() { normal = -normal; }
};




// **************** Edge2D ****************

class Edge2D {
public:
	PointRP2	start;	// start point of next edge is end point
	LineRP2		line;
};


// **************** Polygon2D ****************

class Polygon2D {
public:
	std::vector<Edge2D>	edges;
	int					box_min_x;
	int					box_min_y;
	int					box_max_x;
	int					box_max_y;
	void				*_user;


	Polygon2D() : _user(NULL) {}
	Polygon2D(PointRP2 *vtx, int n_vtx);
	
	void CreateFromPoints(PointRP2 *vtx, int n_vtx);
	void CleanupColinear();
	void CreateRectangle(int x1, int y1, int x2, int y2);

	void SplitByLine(const LineRP2 &line, Polygon2D &right, Polygon2D &left) const;
	bool ComputeIntersection(const Polygon2D &other, Polygon2D &out) const;
	bool ClipBy(const LineRP2 &line);
	int  CheckSide(const LineRP2 &line) const;

	void UpdateBox();

	bool IsEmpty() const
	{
		return (edges.size()==0);
	}

	bool IsBoxCollision(const Polygon2D &other) const
	{
		return	!IsEmpty() && !other.IsEmpty() &&
				box_min_x<=other.box_max_x &&
				box_max_x>=other.box_min_x &&
				box_min_y<=other.box_max_y &&
				box_max_y>=other.box_min_y;
	}
};


// **************** PolygonMesh2D ****************

class PolygonMesh2D {
public:
	std::vector<Polygon2D>	polys;

	void Clear() { polys.clear(); }
	void CSGAdd(const Polygon2D &p);
	void CSGSubtract(const Polygon2D &p);

};



#endif
