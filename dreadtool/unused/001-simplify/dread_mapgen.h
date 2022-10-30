
#pragma once


#include <set>

#include "rp2_csg.h"




// ================================================================ DreadMapGen ================================================================


class DreadMapGen {
public:
	struct Sector;

	struct Vertex {
		int		xp, yp;
		int		_number = -1;

		PointRP2 point_rp2() { return PointRP2(xp, yp); }
		vec2 point_vec2() { return vec2(xp, yp); }
	};
	struct Line {
		Vertex		*v1 = NULL;
		Vertex		*v2 = NULL;
		Line		*next_line = NULL;
		Line		*prev_line = NULL;
		int			h1, h2, h3, h4;
		int			tex_upper = 0;
		int			tex_lower = 0;
		int			tex_ceil = 0;
		int			tex_floor = 0;
		Sector		*sector = NULL;
		Sector		*other_sector = NULL;

		int			_mark = 0;
		int			_locknum = 0;
		int			_number = -1;
	};
	struct Sector {
		Line		*first_line;
		int			ceil_h = 0;
		int			floor_h = 0;
		int			tex_ceil = 0;
		int			tex_floor = 0;
	};


	struct VisLine {
		Line	*line = NULL;
		int		cheat_mode = -1;
		int		draw_order = -1;
	};
	
	class BSPNode;

	class RegionPoly {
	public:
		Sector						*sector = NULL;
		Polygon2D					poly;
		BSPNode						*bsp_subsector = NULL;
		BSPNode						*bsp_leaf = NULL;
		bool						marked = false;
		map<Line*, VisLine*>		vis;
		vector<VisLine*>			drawlist;				// NOT owned	(refers to elements in <vis>)

		RegionPoly					*next = NULL;			// linked list for query results
		double						surface = 0;
		int							_vis_start = -1;
		int							_priority = 1;

		RegionPoly() {}
		RegionPoly(const RegionPoly &rp);
		~RegionPoly();

		void Update();
		vec2 MidPoint();
	};

	class BSPNode {
	public:
		LineRP2		line;				// inner nodes only
		BSPNode		*left = NULL;		// inner nodes only
		BSPNode		*right = NULL;		// inner nodes only
		RegionPoly	*region = NULL;		// leaf nodes only
		int			_number = -1;
	};

	class RegionMap {
	public:
		vector<RegionPoly*>	polys;
		vector<BSPNode*>	nodes;
		BSPNode				*bsproot = NULL;		// not owned

		~RegionMap() { Clear(); }

		void		Clear();
		void		Init(int range);
		RegionPoly	*SelectRegion(const Polygon2D &pr, bool cut_regions=true) { return SelectRegionFromList(pr, polys, cut_regions); }
		RegionPoly	*SelectRegionFromList(const Polygon2D &pr, const vector<RegionPoly*> &list, bool cut_regions=true);
		void		DeleteRegion(RegionPoly *reg);
	};


	vector<Vertex*>				verts;
	vector<Line*>				lines;
	vector<Sector*>				sectors;
	DreadMapGen::RegionMap		regions;


	DreadMapGen() { Clear(); }
	~DreadMapGen() { Clear(); }

	void Clear();
	void LoadMap(MapFileHeader *msrc);

	Line *FindClosestLine(vec2 pos, float range);
	RegionPoly *FindCameraRegion(vec2 pos);

	void ComputeVisibility(Line *line, bool reset=true);
	void ComputeAllVisibility();
	void ExportGather(vector<int> &tex_list);
	void ExportWrite(FILE *fp, GfxConverter &gconv);

	void MarkSimplifyNeighbors(RegionPoly *region);

protected:

	class Beam {
	public:
		LineRP2				l1;
		LineRP2				l2;
		LineRP2				portal;
		vector<PointRP2>	p1s;
		vector<PointRP2>	p2s;
		vector<Line*>		lineportals;		// the line is visible through the backsides of these lines, most recent is first
	};

	struct RegionEdge {
		RegionPoly	*other;
		int			a;			// own edge index
		int			b;			// <other> edge index

		bool operator <(const RegionEdge &re) const
		{ return other->surface < re.other->surface; }
	};

	struct ExportState {
		vector<Vertex*>		verts;
		vector<Line*>		lines;
		vector<BSPNode*>	nodes;
		vector<int>			vis;
		set<int>			tex_set;

		void Clear() {
			verts.clear();
			lines.clear();
			nodes.clear();
			vis.clear();
			tex_set.clear();
		}
	};

	ExportState exp;



	Vertex *AssureVertex(int xp, int yp);

	void ResetVisibility();
	void RegisterVisLine(RegionPoly *region, VisLine *vis, const Beam &beam);
	void ComputeVisibility_Rec(Line *line, Sector *sec, Beam &beam, int depth);		// <beam> is not "const", but restored to original state
	void MakePolySector(Sector *sec, Polygon2D &out_poly);
	bool ComputeLineOrderRec(RegionPoly *reg, const vector<VisLine*> &pending, bool check=false);
	bool ComputeLineOrder(RegionPoly *reg, bool check=false);
	void SimplifyRegions();
	BSPNode *CleanupBSPRec(BSPNode *node);
	void ComputeInitialBSPTree();
	void ComputeFinalBSPTree();
	BSPNode *BuildBSPForPoly(const Polygon2D &poly, const vector<RegionPoly*> reglist, bool cut_regions);
	BSPNode *BuildVisibilityBSP(const Polygon2D &poly, const vector<RegionPoly*> reglist, int depth);
	BSPNode *BuildVisibilityLeaf(const Polygon2D &poly, const vector<RegionPoly*> reglist);
	int CheckBSPDepth(BSPNode *node);

	void ExportBSPRec(BSPNode *node, ExportState &exp);
};
