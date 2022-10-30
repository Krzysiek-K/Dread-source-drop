
#pragma once


#include <set>

#include "rp2_csg.h"




// ================================================================ DreadMapGen ================================================================

class ChunkyColorPacker;
class DreadSim;


class DreadMapGen : public ToolObject
{
public:
	enum {
		// Engine limits/constants
		REGION_SIZE					= 4096,	//2048,
		
		ENGINE_SUBVERTEX_PRECISION	= 3,
		ENGINE_PLAYER_SIZE			= 15,

		VIS_PACKED_SHIFT			= 1,
		VIS_PACKED_MASK				= (1<<VIS_PACKED_SHIFT)-1,

		MAX_SEE_THROUGH				= 2,
	};

	enum collision_mode_t {
		COLLISION_MODE_PLAYER	= 0,
		COLLISION_MODE_HITSCAN,
	};


	struct Line;
	struct Sector;
	class RegionPoly;

	struct Vertex {
		int		xp, yp;
		bool	_special = false;
		int		_number = -1;

		// helpers
		vector<Line*>	_line_in;
		vector<Line*>	_line_out;
		bool			_has_offpos = false;
		vec2			_offpos_in;
		vec2			_offpos_out;

		PointRP2 point_rp2() { return PointRP2(xp, yp); }
		vec2 point_vec2() { return vec2(xp, yp); }
		ivec2 point_ivec2() { return ivec2(xp, yp); }
	};
	struct Line {
		Vertex				*v1 = NULL;
		Vertex				*v2 = NULL;
		Line				*next_line = NULL;
		Line				*prev_line = NULL;
		Line				*other_line = NULL;
		int					h1, h2, h3, h4;
		texture_id_t		tex_upper;
		texture_id_t		tex_lower;
		texture_id_t		tex_ceil;
		texture_id_t		tex_floor;
		int					tex_offs_x = 0;
		int					tex_offs_y = 0;
		bool				tex_unpegged_upper = false;
		bool				tex_unpegged_lower = false;
		Sector				*sector = NULL;
		Sector				*other_sector = NULL;
		int					line_action = 0;
		int					line_tag = 0;
		int					line_flags = 0;
		bool				vis_require = false;
		int					vis_condition = -1;		// index of the machine (door) responsible

		int					_mark = 0;
		int					_debug_mark = 0;
		int					_debug_number = -1;
		int					_number = -1;
		vector<int>			_modes;
		int					_yoffs_u = 0;
		int					_yoffs_l = 0;
		vector<RegionPoly*>	_vis_users;

		bool IsDrawn() {
			if( other_sector &&	other_line &&						// line is two-sided
				(!sector->tex_ceil || h1 == h2) &&					// ceiling is sky  OR  upper texture is not drawn		(ignores upper textures when sky level changes)
				h3 == h4 &&											// lower texture is not drawn
				(!sector->tex_ceil || h2 == other_line->h1) &&		// ceiling height matches  OR  upper texture is not drawn		(ignores sky level changes)
				(h3 == other_line->h4 ) &&							// ceiling height matches  OR  upper texture is not drawn		(ignores sky level changes)
				sector->tex_ceil == other_sector->tex_ceil &&		// ceiling textures match
				sector->tex_floor == other_sector->tex_floor )		// floor textures match
				return false;
			return true;
		}

		bool IsTextureDrawn() {
			if( other_sector &&
				(!sector->tex_ceil || h1 == h2) &&
				h3 == h4 )
				return false;
			return true;
		}

		bool IsRequired() {
			if( line_action == 200 ) return true;
			if( other_sector )
				if( sector->type!=other_sector->type || sector->tag!=other_sector->tag ) return true;
			return IsDrawn();
		}

		bool IsDegenerated() { return v1->xp == v2->xp && v1->yp == v2->yp; }
		bool IsCollision(collision_mode_t col_mode) {
			if( IsDegenerated() ) return false;		// degererated
			if( !other_sector ) return true;		// 1-sided
			if( col_mode==COLLISION_MODE_PLAYER && (line_flags&1) ) return true;		// "impassable" flag for players and monsters
			return false;		// normal 2-sided line
		}

		void LoadSectorHeights(int export_engine_version);
		void ComputeTextureOffsets();

		void Error(const string &s, const char *prefix="ERROR on line ")
		{
			if( v1 && v2 )
				printf("%s (%d,%d): %s\n", prefix, (v1->xp+v2->xp)/2, (v1->yp+v2->yp)/2, s.c_str());
			else
				printf("%s (?,?): %s\n", prefix, s.c_str());
		}

		ivec2 midpoint() { return (v1->point_ivec2() + v2->point_ivec2())/2; }

		string ToString()
		{
			if( !this ) return "NULL line";
			if( v1 && v2 )
				return format("line L%d (%d,%d)", _debug_number, (v1->xp+v2->xp)/2, (v1->yp+v2->yp)/2);
			return "line (?,?)";
		}
	};
	struct Sector {
		Line			*first_line;
		int				ceil_h = 0;
		int				floor_h = 0;
		texture_id_t	tex_ceil;
		texture_id_t	tex_floor;
		int				type = 0;
		int				tag = 0;

		vector<RegionPoly*>	regions;

		vector<RegionPoly*>	_vis_users;
	};
	struct Location {
		int			xp = 0;
		int			yp = 0;
		word		angle = 0;
	};
	struct Machine {
		int			type = 0;		// TBD
		int			tag = 0;		// TBD
		bool		is_door = false;
		Line		*line0 = NULL;			// door_edge
		Line		*line1 = NULL;			// door_front
		Line		*line2 = NULL;			// door_back
		Line		*line_x1 = NULL;		// front_out		(not numbered sequentially)
		Line		*line_x2 = NULL;		// back_out			(not numbered sequentially)
	};

	struct VisLine {
		Line	*line = NULL;
		float	range_y1_min	=  10000;		// enable
		float	range_y1_clip	= -10000;		// clip (max)
		bool	first_draw		= true;
		int		condition		= -1;

		int		cheat_mode	= -1;
		int		draw_order	= -1;
		int		locknum		= 0;
		bool	vis_done	= false;

		void IncludeVisInfo(VisLine *vsrc, DreadMapGen *mapgen);
	};

	struct VisSector {
		Sector	*sector		= NULL;
		int		condition	= -1;
		int		draw_order	= -1;
		bool	vis_done	= false;

		void IncludeVisInfo(VisSector *vsrc, DreadMapGen *mapgen);
	};

	struct VisEntry {
		VisLine		*visline	= NULL;
		VisSector	*vissector	= NULL;
		int			condition	= -1;		// copied value from whatever entry is provided

		VisEntry(VisLine *vline) : visline(vline), condition(vline->condition) {}
		VisEntry(VisSector *vsector) : vissector(vsector), condition(vsector->condition) {}

		bool IsPending() { return (visline && !visline->vis_done) || (vissector && !vissector->vis_done); }
	};

	struct VisGroupEntry {
		Line		*line		= NULL;
		Sector		*sector		= NULL;
	};

	struct VisGroup {
		vector<VisGroupEntry>	vis;
		int						max_users = 0;
		int						instances_used = 0;
		int						entry_count = 0;
		int						visline_count = 0;
		int						_base_offset = -1;
	};

	struct VisGroupReference {
		VisGroup	*group = NULL;
		int			stop_index	= 0;		// first element index in the group NOT to be drawn
		int			condition	= -1;
		bool		first_draw	= false;
	};

	struct VisCondition {
		enum vcop_t {
			OP_MACHINE	= 0,
			OP_AND		= 1,
			OP_OR		= 2,
		};
		vcop_t	op;
		int		c1 = 0;
		int		c2 = 0;

		// optimizer
		int			_opt_numsources = 0;		// 0: optimization disabled
		int			_opt_sources[5] = {};
		uint32_t	_opt_mask = 0;
	};

	class ColBSPNode {
	public:
		bool		is_leaf  = false;
		bool		is_solid = false;
		LineRP2		line;				// inner nodes only
		ColBSPNode	*left = NULL;		// inner nodes only
		ColBSPNode	*right = NULL;		// inner nodes only
		Polygon2D	poly;				// leaf nodes only

		short		A = 0;
		short		B = 0;
		int			C = 0;

		int			_number = -1;
	};

	struct ColEdge {
		PointRP2	p1;
		PointRP2	p2;
		LineRP2		line;		// p1->p2, right side empty, left side solid
		LineRP2		limit1;		// line through p1, the edge is on its right side
		LineRP2		limit2;		// line through p2, the edge is on its right side
	};

	class BSPNode;

	class RegionPoly {
	public:
		RegionPoly					*parent = NULL;
		vector<RegionPoly*>			subregions;				// only when the region was split & shelved

		Sector						*sector = NULL;
		Polygon2D					poly;
		BSPNode						*bsp_subsector = NULL;
		BSPNode						*bsp_leaf = NULL;
		bool						marked = false;
		map<Line*, VisLine*>		vis;					// leaf items owned
		map<Sector*, VisSector*>	svis;					// leaf items owned
		vector<VisEntry>			drawlist;				// refers to elements in <vis> or <svis>
		vector<VisGroupReference>	grouplist;

		RegionPoly					*next = NULL;			// linked list for query results
		double						surface = 0;
		int							_number = -1;
		int							_vis_start = -1;
		int							_priority = 1;

		// visibility order temps
		int								_vis_line_count;
		vector<VisEntry>				_vis_pending;				// refers to elements in <vis> or <svis>
		map<Line*, vector<VisLine*> >	_line_order_locks;			// locking Line -> list of locked VisLines
		
		bool							_vis_scan_has_condition = false;
		int								_vis_scan_condition = -1;
		bool							_vis_scan_has_first_draw = false;
		bool							_vis_scan_first_draw = false;
		bool							_vis_scan_respect_order = true;
		VisGroupReference				*_vis_scan_group_ref = NULL;


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

	class SplitterBSP {
	public:
		class Node {
		public:
			LineRP2		line;				// inner nodes only
			Node		*left = NULL;		// inner nodes only
			Node		*right = NULL;		// inner nodes only
			Sector		*sector = NULL;		// leaf nodes only
			Polygon2D	poly;				// leaf nodes only
		};

		vector<Line*>	active_lines;
		vector<Node*>	all_nodes;
		Node			*root = NULL;

		~SplitterBSP() { Clear(); }

		void Clear();
		void BuildSplitterBSP();

	protected:
		vector<Line*>	lineset_right;		// Temp, not owned.
		vector<Line*>	lineset_left;		// Temp, not owned.

		Node *NewNode();
		Node *BuildSplitterBSP_Rec(int abegin, int aend, const Polygon2D &area);
		Node *ComputeLeafPolys_Rec(Node *n, vector<LineRP2> &splits);
	};

	class RegionMap {
	public:
		enum {
			MAX_REGIONS = 1000000,
		};
		vector<RegionPoly*>	all_polys;				// all polys
		set<RegionPoly*>	poly_set;				// current leaf regions
		vector<RegionPoly*>	poly_set_snapshot;		// snapshot - starting point of SelectRegion* search
		vector<BSPNode*>	nodes;
		BSPNode				*bsproot = NULL;		// not owned

		DWORD			_region_buffer[MAX_REGIONS*sizeof(RegionPoly)/sizeof(DWORD)];
		DWORD			*_region_alloc;
		vector<void*>	_region_reuse;

		RegionMap() { _region_alloc = _region_buffer; }
		~RegionMap() { Clear(); }

		void		Clear();
		RegionPoly	*NewRegion();
		RegionPoly	*NewRegion(const RegionPoly &reg);
		void		OptimizeMemUsage();
		void		Init(int range);
		void		SnapshotPolySet();
		RegionPoly	*SelectRegion(const Polygon2D &pr, bool cut_regions=true);
		RegionPoly	*SelectRegionFromList(const Polygon2D &pr, const vector<RegionPoly*> &list, bool cut_regions=true);
		void		ShelveRegion(RegionPoly *reg);
		RegionPoly	*FindPointRegion(vec2 pos);
		void		UnmarkAll();

		int			UsedRegionsCount() { return (RegionPoly*)_region_alloc - (RegionPoly*)_region_buffer - _region_reuse.size(); }
		int			UsedRegionsPeak() { return (RegionPoly*)_region_alloc - (RegionPoly*)_region_buffer; }

	protected:
		RegionPoly	*SelectRegionInternal(const Polygon2D &pr, const vector<RegionPoly*> &list, bool cut_regions=true);
		void		AddRegionToList(RegionPoly *reg, const Polygon2D &pr, vector<RegionPoly*> &list);
		void		AddRegionToList(RegionPoly *reg, vector<RegionPoly*> &list);
		void		RebuildLeafList(vector<RegionPoly*> &list);
		void		*AllocRegionMemory();
	};

	enum exportvistype_t {
		//EVIS_MARKER			= 0x10,		//	marker
		//EVIS_SUBSECTOR		= 0x11,		//	subsector				(for things)
		//EVIS_LINEMODE		= 0x02,		//	packed line/modeindex
		//EVIS_CONDITION		= 0x13,		//	vis condition
		//
		//EVIS_MASK_TYPE		= 0x0F,
		//EVIS_BIT_PRINT_HEX	= 0x10,

		EVIS_INVALID				= 0,		//
		
		EVIS_GP_REGION_CONMODE,					// index: (condition+1)*2 + first_fraw					[range:	0 .. MAX_CONDITIONS ~= 255]
		EVIS_GP_GROUP_CALL,						// index: end pointer of the group called				[range: 0 .. huge ]
		EVIS_GP_GROUP_START,					// index: 0												[marker value]
		EVIS_GP_LINE,							// index: line number									[range: 0 .. 0x7FFF]
		EVIS_GP_SUBSECTOR,						// index: subsector index								[range: 0 .. 0x4FFF]
		EVIS_GP_END,							// index: 0												[marker value, may be the same as EVIS_GP_GROUP_START]

		EVIS_OLD_CONDITION,						// index: condition index								[export: +ENGINE_OLDVIS_CONDITION_BASE	]
		EVIS_OLD_LINEMODE,						// index: line/mode value								[export: +ENGINE_OLDVIS_LINEMODE_BASE	]
		EVIS_OLD_SUBSECTOR,						// index: subsector index								[export: +ENGINE_OLDVIS_SUBSECTOR_BASE	]
		EVIS_OLD_END,							// index: 0												[export:  ENGINE_OLDVIS_ENDLIST			]

		// Export values	(not really values of this enum)
		ENGINE_OLDVIS_LINEMODE_BASE		= 0x0000,
		ENGINE_OLDVIS_SUBSECTOR_BASE	= 0x8000,
		ENGINE_OLDVIS_CONDITION_OFF		= 0xC001,
		ENGINE_OLDVIS_CONDITION_BASE	= 0xC002,
		ENGINE_OLDVIS_ENDLIST			= 0xC000,

		ENGINE_GVIS_MARKER				= 0x0000,
		ENGINE_GVIS_GROUP_CALL_BASE		= 0x0000,
		ENGINE_GVIS_LINE_BASE			= 0x8000,
		ENGINE_GVIS_SUBSECTOR_BASE		= 0xC000,
		ENGINE_GVIS_CONMODE_BASE		= 0xF000,
	};

	struct ExportVis {
		exportvistype_t		type = EVIS_INVALID;
		int					index = 0;

		ExportVis() {}
		ExportVis(exportvistype_t _type, int _index) : type(_type), index(_index) {}

		word Encode();
		string ToString(int addr);

		bool operator ==(const ExportVis &v) const { return type==v.type && index==v.index; }
		bool operator !=(const ExportVis &v) const { return type!=v.type || index!=v.index; }
	};

	struct ExportState {
		vector<Vertex*>			verts;
		vector<Line*>			lines;
		vector<BSPNode*>		nodes;
		vector<ColBSPNode*>		colnodes;
		vector<RegionPoly*>		subsectors;
		vector<ExportVis>		vis;
		set<texture_id_t>		tex_set;
		vector<byte>			tex_buffer;
		int						max_vis_lines = 0;	// per subsector

		void Clear() {
			verts.clear();
			lines.clear();
			nodes.clear();
			colnodes.clear();
			subsectors.clear();
			vis.clear();
			tex_set.clear();
			tex_buffer.clear();
			max_vis_lines = 0;
		}
	};



	vector<Vertex*>				verts;
	vector<Line*>				lines;
	vector<Sector*>				sectors;
	RegionMap					regions;
	SplitterBSP					splitter_bsp;
	bool						visibility_done = false;
	vector<MapFileThing>		things;
	map<int, Location>			locations;
	vector<Machine>				machines;
	vector<VisCondition>		vis_conditions;
	vector<VisGroup*>			vis_groups;
	PolygonMesh2D				work_mesh;
	PolygonMesh2D				collision_mesh;
	PolygonMesh2D				collision_bsp_mesh;
	vector<ColEdge>				col_edges;
	vector<ColBSPNode*>			col_nodes;
	ColBSPNode					*col_bsp_root_psize;
	ColBSPNode					*col_bsp_root_0;

	ExportState					exp;

	// settings
	bool						mapvis_allow_region_splitting = false;			// not an option
	bool						map_col_round_corners = false;
	bool						map_optimized_bsp = false;
	int							export_engine_version = 1;
	bool						export_vis_groups = true;
	int							export_vis_align = 2;							// not an option
	bool						export_deflate = true;


	DreadMapGen() { Clear(); }
	~DreadMapGen() { Clear(); }

	void Clear();
	void LoadMap(MapFileHeader *msrc, WadConverter *wconv);

	Line *FindClosestLine(vec2 pos, float range);
	RegionPoly *FindCameraRegion(vec2 pos, bool brute_force);

	void ComputeVisibility(Line *line, bool reset);
	void ComputeAllVisibility();
	void AssureVisibility();
	void ComputeCollisionMesh(PolygonMesh2D &mesh, int offset, collision_mode_t col_mode);
	
	string GetConditionName(int cid, bool mark_op, bool and_level=false);
	

	void ExportGather(vector<texture_id_t> &tex_list);
	void ExportTextures(FILE *fp, vector<texture_id_t> &tex_list, map<texture_id_t, int> &tex_offsets, ChunkyColorPacker &chunky_packer);
	void ExportWrite(FILE *fp, FILE *asm_fp, DataBlockSet &bset, ScriptCompiler &scomp, ChunkyColorPacker &chunky_packer, map<texture_id_t, int> &tex_offsets);
	void ExportToSim(DreadSim &sim);

	void SetDoorsPosition(int prc);
protected:

	class Beam {
	public:
		LineRP2				l1;
		LineRP2				l2;
		LineRP2				portal;
		vector<PointRP2>	p1s;
		vector<PointRP2>	p2s;
		vector<Line*>		lineportals;		// the line is visible through the backsides of these lines, most recent is first
		vector<Sector*>		sectors;			// the beam goes through these sectors
		vector<VisLine*>	vislines;
		vector<int>			conditions;
	};


	coord_t		mag_AB;
	coord_t		mag_C;



	Vertex *AssureVertex(int xp, int yp, bool special = false);
	void MakeDoor(Line *line);

	void ResetVisibility();
	VisLine *RegisterVisLine(RegionPoly *region, Line *line, const Beam &beam, const Polygon2D &visregion);
	VisSector *RegisterVisSector(RegionPoly *region, Sector *sec, const Beam &beam);
	int  GetMachineCondition(int midx);
	int  GetAndCondition(int c1, int c2);
	int  GetOrCondition(int c1, int c2);
	bool FindAndConditionComponentRec(int cid, int searchcon);
	bool FindOrConditionComponentRec(int cid, int searchcon);
	void SelectVisLineCheat(VisLine *vis);
	void ComputeVisibility_Rec(Line *line, Sector *sec, Beam &beam, const Polygon2D &beam_poly, int depth);		// <beam> is not "const", but restored to original state
	void IncludeLineVisibility(Line *line, Line *vis_source);
	void MakePolySector(Sector *sec, Polygon2D &out_poly);
	void Vis_DumpLocks(const char *path);									// vislist
	void Vis_GatherSectors();												// vislist
	void Vis_PrepareRegion(RegionPoly *reg);								// vislist
	bool Vis_ComputeDrawOrder(RegionPoly *reg);								// vislist
	void Vis_CleanupRegion(RegionPoly *reg);								// vislist
	void Vis_AddUserRegion(RegionPoly *reg);								// vislist
	void Vis_BuildUserLists(vector<RegionPoly*> *users);					// vislist
	bool Vis_BuildGroup();													// vislist
	void BuildVisDrawLists();												// vislist
	//bool ComputeLineOrder(RegionPoly *reg, bool check=false);
	BSPNode *CleanupBSPRec(BSPNode *node);
	void ComputeInitialBSPTree();
	void ComputeFinalBSPTree(bool build_bsp);
	void ComputeFinalBSPTreeRec(BSPNode *node, const Polygon2D &poly, bool build_bsp);
	BSPNode *BuildBSPForPoly(const Polygon2D &poly, bool cut_regions);
	BSPNode *BuildVisibilityBSP(const Polygon2D &poly, const vector<RegionPoly*> &reglist, int depth, bool build_bsp);
	BSPNode *BuildVisibilityLeaf(const Polygon2D &poly, const vector<RegionPoly*> &reglist, bool build_bsp);
	int CheckBSPDepth(BSPNode *node);
	
	//void ComputeCollisions(RegionPoly *reg);


	void GlueCollisionEdges(bool allow_merge);
	ColBSPNode *BuildBSPForColEdges(const Polygon2D &poly, const vector<ColEdge*> &edges, bool is_right_side);
	int CheckBSPDepth(ColBSPNode *node);


	int  ExportVertex(Vertex *v, ExportState &exp);
	int  ExportLine(Line *line, ExportState &exp, int request_mode, bool first_draw);
	void ExportRegionGather(RegionPoly *reg, ExportState &exp);
	void ExportRegionProcess(RegionPoly *reg, ExportState &exp, bool full_export);
	void ExportVisGroup(VisGroup *vis_group, ExportState &exp, bool region_inline, int stop_entry);
	void ExportBSPRec(BSPNode *node, ExportState &exp);
	void ExportColBSPRec(ColBSPNode *node, ExportState &exp);
};



int adjust_y_offset(int ycoord, int texheight, int fudge, int y1, int y2);
