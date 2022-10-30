
#include "common.h"
#include "app_rendertest.h"


#define USE_TEXTURE_Y_OFFSETS	1


#define MAX_REGIONS_COUNT		10000
//#define MAX_REGIONS_COUNT		1000


LARGE_INTEGER perf_listresolve_time;
LARGE_INTEGER perf_subregion_time;



// ================================ Functions ================================



static float _point2line_dist(vec2 p, vec2 v1, vec2 v2)
{
	p -= v1;
	v2 -= v1;

	float f = v2.dot(p) / v2.dot(v2);
	if( f<0 ) f = 0;
	if( f>1 ) f = 1;
	return (p - v2*f).length();
}


static vec2 _line_intersection(vec2 v1, vec2 n1, vec2 v2, vec2 n2)
{
	vec3 l1(n1.x, n1.y, -n1.dot(v1));
	vec3 l2(n2.x, n2.y, -n2.dot(v2));
	vec3 pp = l1.cross(l2);
	return vec2(pp.x, pp.y)/pp.z;
}


// Rounds always down (to minus infinity)
//
static int _div_round_down(int a, int b)
{
	return int(floor(double(a)/double(b)));
}


// Adjusts texture ycoord so it shows best in the Y1..Y2 range.
//	ycoord		- initial y coord
//	texheight	- texture height
//	fudge		- warning border added by the exporter
//	y1..y2		- visibility range (y1<y2)
//
// Algorithm:
//	1. Move the offset up or down by [texheight+fudge] to find maximum ycoord, so that	"ycoord <= y1"
//	2. Check if fudge is visible at the bottom - if yes, then align the texture to Y2.
//
int adjust_y_offset(int ycoord, int texheight, int fudge, int y1, int y2)
{
	int uhf = texheight + fudge;
	int k = _div_round_down(y1 - ycoord, uhf);
	//if(k>0) k = -1;
	//if( k>0 ) printf("%d %d -> %d\n", y1 - ycoord, uhf, k);
	ycoord += k*uhf;

	if( ycoord + texheight < y2 )
		ycoord = y2 - texheight;

	return ycoord;
}




// ================================================================ DreadMapGen::Line ================================================================


void DreadMapGen::Line::LoadSectorHeights(int export_engine_version)
{
	if( other_sector )
	{
		h1 = sector->ceil_h;
		h2 = other_sector->ceil_h;
		h3 = other_sector->floor_h;
		h4 = sector->floor_h;

		if( !other_sector->tex_floor /*|| other_sector->floor_h<=other_sector->ceil_h */ )
		{
			if( export_engine_version == 1 )
			{
				// Sector to be removed (pure sky)
				tex_upper = tex_lower;
				tex_lower.clear();
				h1 = other_sector->floor_h;
				h2 = h3 = h4 = sector->floor_h;
				other_sector = NULL;
			}
			else
			{
				// Other sector to be removed
				bool has_upper = tex_upper.is_set();
				bool has_lower = tex_lower.is_set();
				if( h2 <= h1 ) h2 = h1, has_upper = false;
				if( h3 >= h4 ) h3 = h4, has_lower = false;

				if( has_upper && has_lower )
				{
					// both textures - extend upper to lower, closing the gap
					h2 = h3;
				}
				else if( has_upper )
				{
					// upper only - extend floor up to the upper texture edge
					h3 = h4 = h2;
				}
				else if( has_lower )
				{
					// lower only - extend ceiling down to the lower texture edge
					h1 = h2 = h3;
				}
				else
				{
					// no textures - extend ceiling down to floor
					h1 = h2 = h3 = h4;
				}

				other_sector = NULL;
			}
		}

		if( line_action>=101 && line_action<=108 )		// Facade height
			h1 = h4 - (line_action-100)*16;

		if( h2 <= h1 )
		{
			h2 = h1;
			if( !line_action )			// If not a special line (e.g. a door) - clear texture
				tex_upper.clear();
		}
		if( h3 >= h4 )
		{
			h3 = h4;
			tex_lower.clear();
		}
	}
	else
	{
		// One sided
		h1 = sector->ceil_h;
		h2 = h3 = h4 = sector->floor_h;
		tex_lower.clear();

		if( line_action>=101 && line_action<=108 )		// Facade height
			h1 = h4 - (line_action-100)*16;

		if( line_action==120 || line_action==121 )		// Y-wrap texture once	(assume texture height is 128)
		{
			int tex_height = 128;
			int total_height = tex_height + GfxConverter::FUDGE_BORDER;

			int texture_split = h1 - tex_offs_y;
			if( tex_unpegged_lower )
				texture_split = h2 - tex_offs_y;

			if( line_action==121 )				// Wrap according to bottom edge
				texture_split += tex_height;


			// Find first visible texture split
			int split_distance = texture_split - h1;
			split_distance = ((split_distance % total_height) + total_height) % total_height;

			texture_split = h1 + split_distance;
			if( texture_split == h1 )
				texture_split += tex_height;

			if( texture_split < h2 )
			{
				h4 = h2;
				h2 = h3 = texture_split;
				tex_lower = tex_upper;
				tex_unpegged_lower = tex_unpegged_upper;
			}
		}
	}
}

void DreadMapGen::Line::ComputeTextureOffsets()
{
	if( 1 )
	{
		// use Y coords
		if( other_sector )
		{
			if( tex_unpegged_upper )	_yoffs_u = h1;
			else						_yoffs_u = h2;
			if( tex_unpegged_lower )	_yoffs_l = h1;
			else						_yoffs_l = h3;
		}
		else
		{
			if( tex_unpegged_lower )	_yoffs_u = h2;
			else						_yoffs_u = h1;
			_yoffs_l = 0;	// unused
		}
		_yoffs_u -= tex_offs_y;
		_yoffs_l -= tex_offs_y;


		GfxTexture *utex = gfx_converter.FindTexture(tex_upper);
		int uh = (utex ? utex->orig_h : 128);
		_yoffs_u = adjust_y_offset(_yoffs_u, uh, GfxConverter::FUDGE_BORDER, h1, h2);

		GfxTexture *ltex = gfx_converter.FindTexture(tex_lower);
		int lh = (ltex ? ltex->orig_h : 128);
		_yoffs_l = adjust_y_offset(_yoffs_l, lh, GfxConverter::FUDGE_BORDER, h3, h4);
	}
	else
	{
		// legacy coords (ignoring Y alignment offsets)
		_yoffs_u = h1;
		_yoffs_l = h3;
	}
}


// ================================================================ DreadMapGen::VisLine ================================================================


void DreadMapGen::VisLine::IncludeVisInfo(VisLine *vsrc, DreadMapGen *mapgen)
{
	// Does not check if it's the same line - can be usd to copy the line visibility

	if( vsrc->range_y1_min < range_y1_min )
		range_y1_min = vsrc->range_y1_min;

	if( vsrc->range_y1_clip > range_y1_clip )
		range_y1_clip = vsrc->range_y1_clip;

	if( !vsrc->first_draw )
		first_draw = false;

	condition = mapgen->GetOrCondition(condition, vsrc->condition);
}

// ================================================================ DreadMapGen::VisSector ================================================================


void DreadMapGen::VisSector::IncludeVisInfo(VisSector *vsrc, DreadMapGen *mapgen)
{
	// Does not check if it's the same sector - can be used to copy the sector visibility

	condition = mapgen->GetOrCondition(condition, vsrc->condition);
}


// ================================================================ DreadMapGen::RegionPoly ================================================================




DreadMapGen::RegionPoly::RegionPoly(const RegionPoly &rp)
{
	// Don't copy parten/sub information
	sector = rp.sector;
	//poly = rp.poly;							// don't copy the poly
	bsp_subsector = rp.bsp_subsector;
	bsp_leaf = rp.bsp_leaf;
	marked = rp.marked;

	for( auto p=rp.vis.begin(); p!=rp.vis.end(); ++p )
		vis[p->first] = new VisLine(*p->second);

	for( auto p=rp.svis.begin(); p!=rp.svis.end(); ++p )
		svis[p->first] = new VisSector(*p->second);
}


DreadMapGen::RegionPoly::~RegionPoly()
{
	for( auto p=vis.begin(); p!=vis.end(); ++p )
		delete p->second;
	vis.clear();

	for( auto p=svis.begin(); p!=svis.end(); ++p )
		delete p->second;
	svis.clear();
}

void DreadMapGen::RegionPoly::Update()
{
	poly.UpdateBox();

	surface = 0;

	vec2 p1(0,0);
	for( int i=0; i<=(int)poly.edges.size(); i++ )
	{
		vec2 p2 = poly.edges[i%(int)poly.edges.size()].start.GetVec2();
		if( i )
			surface += (p2.x-p1.x)*(p1.y+p2.y);
		p1 = p2;
	}
	
	surface *= 0.5f;
}

vec2 DreadMapGen::RegionPoly::MidPoint()
{
	vec2 mid(0, 0);
	for( int i=0; i<(int)poly.edges.size(); i++ )
		mid += poly.edges[i].start.GetVec2();
	mid /= (int)poly.edges.size();
	
	return mid;
}




// ================================================================ DreadMapGen::SplitterBSP ================================================================


void DreadMapGen::SplitterBSP::Clear()
{
	active_lines.clear();

	for( int i=0; i<(int)all_nodes.size(); i++ )
		delete all_nodes[i];
	all_nodes.clear();
	
	root = NULL;
}

void DreadMapGen::SplitterBSP::BuildSplitterBSP()
{
	// Build BSP splits.
	Polygon2D area;
	area.CreateRectangle(-REGION_SIZE, -REGION_SIZE, REGION_SIZE, REGION_SIZE);
	root = BuildSplitterBSP_Rec(0, active_lines.size(), area);

	//// Compute leaft polys.
	//vector<LineRP2> splits;
	//root = ComputeLeafPolys_Rec(root, splits);
}


DreadMapGen::SplitterBSP::Node *DreadMapGen::SplitterBSP::NewNode()
{
	Node *node = new Node();
	all_nodes.push_back(node);
	return node;
}

DreadMapGen::SplitterBSP::Node *DreadMapGen::SplitterBSP::BuildSplitterBSP_Rec(int abegin, int aend, const Polygon2D &area)
{
	// Find best split line
	vector<RegionPoly*> cur_left;
	vector<RegionPoly*> cur_right;
	Line *best_split = NULL;
	float best_score = 1e38f;

	// Go through all active lines and find best split.
	for( int a=abegin; a<aend; a++ )
	{
		Line *line = active_lines[a];

		LineRP2 split_line = line->v1->point_rp2().LineTo(line->v2->point_rp2());

		if( area.CheckSide(split_line) != 0 )
			continue;	// whole region on one side - ignore this split

		int n_split = 0;
		int n_left = 0;
		int n_right = 0;

		for( int b=abegin; b<aend; b++ )
		{
			Line *test = active_lines[b];

			int s1 = split_line.CheckSide(test->v1->point_rp2());
			int s2 = split_line.CheckSide(test->v2->point_rp2());
			if( s1==0 && s2==0 )
			{
				ivec2 dtest = test->v2->point_ivec2() - test->v1->point_ivec2();
				ivec2 dsplit = line->v2->point_ivec2() - line->v1->point_ivec2();
				if( dtest.dot(dsplit) > 0 ) n_right++;
				if( dtest.dot(dsplit) < 0 ) n_left++;
			}
			else if( s1>=0 && s2>=0 ) n_right++;
			else if( s1<=0 && s2<=0 ) n_left++;
			else					  n_split++;
		}

		if( n_left<=0 || n_right<=0 )
			continue;	// whole subtree on one side - ignore this split

		float score = pow(fabs(float(cur_left.size()) - float(cur_right.size())), .5f) + n_split*2*10.f;
		if( score < best_score )
		{
			best_split = line;
			best_score = score;
		}
	}

	if( !best_split )
	{
		// Leaf node
		Node *node = NewNode();
		node->sector = active_lines[abegin]->sector;
		node->poly = area;

		for( int a=abegin; a<aend; a++ )
		{
			Line *line = active_lines[a];
			LineRP2 split_line = line->v1->point_rp2().LineTo(line->v2->point_rp2());
			node->poly.ClipBy(split_line);
		}

		return node;
	}


	// Partition lines into left and right groups.
	LineRP2 split_line = best_split->v1->point_rp2().LineTo(best_split->v2->point_rp2());
	lineset_right.clear();
	lineset_left.clear();

	for( int a=abegin; a<aend; a++ )
	{
		Line *test = active_lines[a];

		int s1 = split_line.CheckSide(test->v1->point_rp2());
		int s2 = split_line.CheckSide(test->v2->point_rp2());
		if( s1==0 && s2==0 )
		{
			ivec2 dtest = test->v2->point_ivec2() - test->v1->point_ivec2();
			ivec2 dsplit = best_split->v2->point_ivec2() - best_split->v1->point_ivec2();
			if( dtest.dot(dsplit) > 0 ) lineset_right.push_back(test);
			if( dtest.dot(dsplit) < 0 ) lineset_left.push_back(test);
		}
		else if( s1>=0 && s2>=0 ) lineset_right.push_back(test);
		else if( s1<=0 && s2<=0 ) lineset_left.push_back(test);
		else					  lineset_right.push_back(test), lineset_left.push_back(test);
	}

	// Move lines to stack.
	int sbegin = active_lines.size();
	active_lines.insert(active_lines.end(), lineset_right.begin(), lineset_right.end());
	int smid = active_lines.size();
	active_lines.insert(active_lines.end(), lineset_left.begin(), lineset_left.end());
	int send = active_lines.size();
	lineset_right.clear();
	lineset_left.clear();

	// Compute new regions
	Polygon2D right, left;
	area.SplitByLine(split_line, right, left);

	// Create node.
	Node *node = NewNode();
	node->line = split_line;
	node->left = BuildSplitterBSP_Rec(smid, send, left);
	active_lines.resize(smid);
	node->right = BuildSplitterBSP_Rec(sbegin, smid, right);
	active_lines.resize(sbegin);
}

DreadMapGen::SplitterBSP::Node *DreadMapGen::SplitterBSP::ComputeLeafPolys_Rec(Node *n, vector<LineRP2> &splits)
{
	if( n->sector )
	{
		// Leaf poly
		n->poly.CreateRectangle(-REGION_SIZE, -REGION_SIZE, REGION_SIZE, REGION_SIZE);
		for( const LineRP2 &split : splits )
			n->poly.ClipBy(split);

		return n->poly.IsEmpty() ? NULL : n;
	}

	// Inner node
	int nsplits = splits.size();
	
	splits.push_back(n->line);
	n->right = ComputeLeafPolys_Rec(n->right, splits);
	splits.resize(nsplits);

	splits.push_back(n->line.Reversed());
	n->left = ComputeLeafPolys_Rec(n->left, splits);
	splits.resize(nsplits);

	if( !n->left ) return n->right;
	if( !n->right ) return n->left;
	return n;
}



// ================================================================ DreadMapGen::RegionMap ================================================================


void DreadMapGen::RegionMap::Clear()
{
	//DWORD *ptr = _region_buffer;
	//while( ptr<_region_alloc )
	//{
	//	((RegionPoly*)ptr)->~RegionPoly();
	//	ptr += sizeof(RegionPoly)/sizeof(DWORD);
	//}
	_region_alloc = _region_buffer;
	_region_reuse.clear();

	for( auto p=all_polys.begin(); p!=all_polys.end(); ++p )
		(*p)->~RegionPoly();
	all_polys.clear();

	poly_set.clear();
	poly_set_snapshot.clear();


	for( int i=0; i<(int)nodes.size(); i++ )
		delete nodes[i];
	nodes.clear();

	bsproot = NULL;
}

DreadMapGen::RegionPoly *DreadMapGen::RegionMap::NewRegion()
{
	RegionPoly *p = new (AllocRegionMemory())RegionPoly();
	all_polys.push_back(p);
	poly_set.insert(p);

	return p;
}

DreadMapGen::RegionPoly	*DreadMapGen::RegionMap::NewRegion(const RegionPoly &reg)
{
	RegionPoly *p = new (AllocRegionMemory())RegionPoly(reg);
	all_polys.push_back(p);
	poly_set.insert(p);

	return p;
}

void DreadMapGen::RegionMap::OptimizeMemUsage()
{
	int num_before = (int)all_polys.size();

	// Unmark everything
	for( int i=0; i<(int)all_polys.size(); i++ )
		all_polys[i]->marked = false;

	// Mark poly set
	for( auto p=poly_set.begin(); p!=poly_set.end(); ++p )
		(*p)->marked = true;

	// Mark and optimize snapshot
	for( int i=0; i<(int)poly_set_snapshot.size(); i++ )
	{
		RegionPoly *reg = poly_set_snapshot[i];
		reg->marked = true;
		RebuildLeafList(reg->subregions);
	}

	// Delete unused regions
	int d = 0;
	for( int i=0; i<(int)all_polys.size(); i++ )
		if( all_polys[i]->marked )
			all_polys[d++] = all_polys[i];
		else
		{
			all_polys[i]->~RegionPoly();
			_region_reuse.push_back(all_polys[i]);
		}
	all_polys.resize(d);

	// Cleanup markers
	for( int i=0; i<(int)all_polys.size(); i++ )
		all_polys[i]->marked = false;

	int num_after = (int)all_polys.size();
	//printf("GC: Freed %d regions (%d -> %d, %.1f%% drop).\n", num_before-num_after, num_before, num_after, (num_before-num_after)*100.f/num_before);
}


void DreadMapGen::RegionMap::Init(int range)
{
	Clear();

	RegionPoly *p = NewRegion();
	p->poly.CreateRectangle(-range, -range, range, range);
}


// Make a snapshot of top-level poly list for SelectRegion
void DreadMapGen::RegionMap::SnapshotPolySet()
{
	poly_set_snapshot.clear();
	for( auto p=poly_set.begin(); p!=poly_set.end(); ++p )
		poly_set_snapshot.push_back(*p);
}

DreadMapGen::RegionPoly	*DreadMapGen::RegionMap::SelectRegion(const Polygon2D &pr, bool cut_regions)
{
	LARGE_INTEGER qp_start, qp_mid, qp_end;
	QueryPerformanceCounter(&qp_start);

	// Prepare list of leaf regions
	static vector<RegionPoly*> list;
	list.clear();
	
#if 1
	// Iterate through top-level snapshot regions
	for( int i=0; i<(int)poly_set_snapshot.size(); i++ )
		AddRegionToList(poly_set_snapshot[i], pr, list);
#else
	// Iterate through set of leaf regions
	for( auto p=poly_set.begin(); p!=poly_set.end(); ++p )
		if( pr.IsBoxCollision((*p)->poly) )
			list.push_back(*p);
#endif

	QueryPerformanceCounter(&qp_mid);

	RegionPoly *list_head = SelectRegionInternal(pr, list, cut_regions);

	QueryPerformanceCounter(&qp_end);
	perf_listresolve_time.QuadPart += qp_mid.QuadPart - qp_start.QuadPart;
	perf_subregion_time.QuadPart += qp_end.QuadPart - qp_mid.QuadPart;

	return list_head;
}

DreadMapGen::RegionPoly *DreadMapGen::RegionMap::SelectRegionFromList(const Polygon2D &pr, const vector<RegionPoly*> &list_in, bool cut_regions)
{
	LARGE_INTEGER qp_start, qp_mid, qp_end;
	QueryPerformanceCounter(&qp_start);

	// Prepare list of leaf regions
	static vector<RegionPoly*> list;
	list.clear();

	for( int i=0; i<(int)list_in.size(); i++ )
		AddRegionToList(list_in[i], pr, list);

	QueryPerformanceCounter(&qp_mid);

	RegionPoly *list_head = SelectRegionInternal(pr, list, cut_regions);

	QueryPerformanceCounter(&qp_end);
	perf_listresolve_time.QuadPart += qp_mid.QuadPart - qp_start.QuadPart;
	perf_subregion_time.QuadPart += qp_end.QuadPart - qp_mid.QuadPart;

	return list_head;
}

DreadMapGen::RegionPoly *DreadMapGen::RegionMap::SelectRegionInternal(const Polygon2D &pr, const vector<RegionPoly*> &list, bool cut_regions)
{
	RegionPoly *list_head = NULL;
	Polygon2D C, inside, outside;
	vector<Polygon2D> outA;
	vector<Edge2D> tmp_edges;

	for( int i=0; i<(int)list.size(); i++ )
	{
		RegionPoly *reg = list[i];

		//// That's already checked when building the list
		//if( !pr.IsBoxCollision(reg->poly) )
		//	continue;

		// init polygon C and set outA
		C = reg->poly;
		outA.clear();

		// cut polygon C with edges of polygon being selected
		int j = 0;
		while( !C.IsEmpty() && j<pr.edges.size() )
		{
			C.SplitByLine(pr.edges[j].line, inside, outside);
			C.edges.swap(inside.edges);
			if( !outside.IsEmpty() )
				outA.push_back(outside);
			j++;
		}

		// if C is left empty it means all parts of initial polygon
		// were added to set outA
		if( C.IsEmpty() )
		{
			// the polygon is not in the region
		}
		else if( cut_regions )
		{
			ShelveRegion(reg);
			reg->poly.edges.swap(tmp_edges);
			reg->subregions.clear();

			// add outside fragments
			for( int j=0; j<(int)outA.size(); j++ )
			{
				RegionPoly *np = NewRegion(*reg);
				reg->subregions.push_back(np);
				np->parent = reg;
				np->poly = outA[j];
				np->poly.UpdateBox();
			}

			// the common fragment is C
			{
				RegionPoly *np = NewRegion(*reg);
				reg->subregions.push_back(np);
				np->parent = reg;
				np->poly = C;
				np->poly.UpdateBox();
				np->next = list_head;
				list_head = np;
			}

			reg->poly.edges.swap(tmp_edges);
		}
		else
		{
			// the polygon overlaps the region - cutting disabled
			reg->next = list_head;
			list_head = reg;
		}
	}

	return list_head;
}

void DreadMapGen::RegionMap::ShelveRegion(RegionPoly *reg)
{
	auto p = poly_set.find(reg);
	if( p!=poly_set.end() )
		poly_set.erase(p);
}

DreadMapGen::RegionPoly *DreadMapGen::RegionMap::FindPointRegion(vec2 pos)
{
	for( auto p=poly_set.begin(); p!=poly_set.end(); ++p )
	{
		RegionPoly &r = **p;
		if( pos.x < (float)r.poly.box_min_x ) continue;
		if( pos.y < (float)r.poly.box_min_y ) continue;
		if( pos.x > (float)r.poly.box_max_x ) continue;
		if( pos.y > (float)r.poly.box_max_y ) continue;

		bool ok = true;
		for( int j=0; j<(int)r.poly.edges.size(); j++ )
			if( r.poly.edges[j].line.CheckSideVec2(pos) < 0 )
			{
				ok = false;
				break;
			}

		if( ok )
			return &r;
	}

	return NULL;
}


void DreadMapGen::RegionMap::UnmarkAll()
{
	for( auto p=poly_set.begin(); p!=poly_set.end(); ++p )
		(*p)->marked = false;
}

void DreadMapGen::RegionMap::AddRegionToList(RegionPoly *reg, const Polygon2D &pr, vector<RegionPoly*> &list)
{
	if( !pr.IsBoxCollision(reg->poly) )
		return;

	if( reg->subregions.size() <= 0 )
	{
		list.push_back(reg);
		return;
	}

	for( int i=0; i<(int)reg->subregions.size(); i++ )
		AddRegionToList(reg->subregions[i], pr, list);
}

void DreadMapGen::RegionMap::AddRegionToList(RegionPoly *reg, vector<RegionPoly*> &list)
{
	if( reg->subregions.size()<=0 && poly_set.find(reg)!=poly_set.end() )
	{
		list.push_back(reg);
		return;
	}

	for( int i=0; i<(int)reg->subregions.size(); i++ )
		AddRegionToList(reg->subregions[i], list);
}

void DreadMapGen::RegionMap::RebuildLeafList(vector<RegionPoly*> &list)
{
	static vector<RegionPoly*> buff;
	buff.clear();
	for( int i=0; i<(int)list.size(); i++ )
		AddRegionToList(list[i], buff);
	list.swap(buff);
}

void *DreadMapGen::RegionMap::AllocRegionMemory()
{
	if( _region_reuse.size()>0 )
	{
		void *ptr = _region_reuse.back();
		_region_reuse.pop_back();
		return ptr;
	}

	if( _region_alloc>=_region_buffer+sizeof(_region_buffer)/sizeof(DWORD) )
		throw string("Exceeded max vis region count");
	
	void *ptr = _region_alloc;
	
	_region_alloc += sizeof(RegionPoly)/sizeof(DWORD);

	return ptr;
}



// ================================================================ DreadMapGen::ExportVis ================================================================


word DreadMapGen::ExportVis::Encode()
{
	int value = 0;
	int minvalue = 0;
	int maxvalue = 0xFFFF;

	switch(type)
	{
	case EVIS_GP_GROUP_START:		value = ENGINE_GVIS_MARKER;																								break;			// index: 0
	case EVIS_GP_END:				value = ENGINE_GVIS_MARKER;																								break;			// index: 0
	case EVIS_GP_GROUP_CALL:		value = index + ENGINE_GVIS_GROUP_CALL_BASE;		minvalue = 1;		maxvalue = ENGINE_GVIS_LINE_BASE-1;				break;			// index: end pointer of the group called
	case EVIS_GP_LINE:				value = index + ENGINE_GVIS_LINE_BASE;									maxvalue = ENGINE_GVIS_SUBSECTOR_BASE-1;		break;			// index: line number
	case EVIS_GP_SUBSECTOR:			value = index + ENGINE_GVIS_SUBSECTOR_BASE;								maxvalue = ENGINE_GVIS_CONMODE_BASE-1;			break;			// index: subsector index
	case EVIS_GP_REGION_CONMODE:	value = index + ENGINE_GVIS_CONMODE_BASE;																				break;			// index: condition index

	case EVIS_OLD_LINEMODE:			value = index + ENGINE_OLDVIS_LINEMODE_BASE;							maxvalue = ENGINE_OLDVIS_SUBSECTOR_BASE-1;		break;
	case EVIS_OLD_SUBSECTOR:		value = index + ENGINE_OLDVIS_SUBSECTOR_BASE;							maxvalue = ENGINE_OLDVIS_ENDLIST-1;				break;
	case EVIS_OLD_END:				value = ENGINE_OLDVIS_ENDLIST;																							break;
	case EVIS_OLD_CONDITION:		value = index + ENGINE_OLDVIS_CONDITION_BASE;																			break;
	
	default:
		printf("ERROR: Invalid vis type during vis list encoding.\n");
		return 0;
	}

	if( value < minvalue )
		printf("ERROR: Vis index value underflow for vis type %d (%04X < %04X)\n", type, value, minvalue);

	if( value > maxvalue )
		printf("ERROR: Vis index value overflow for vis type %d (%04X > %04X)\n", type, value, maxvalue);

	return (word)value;
}

string DreadMapGen::ExportVis::ToString(int addr)
{
#define ADDR "%4d $%04X:\t"
	const char *fmt = ADDR "??? %d";
	switch( type )
	{
	case EVIS_GP_REGION_CONMODE:	fmt = ADDR "reg conmode %d";		break;
	case EVIS_GP_GROUP_CALL:		fmt = ADDR "group call %d";			break;
	case EVIS_GP_GROUP_START:		fmt = ADDR "group start %d";		break;
	case EVIS_GP_LINE:				fmt = ADDR "line %d";				break;
	case EVIS_GP_SUBSECTOR:			fmt = ADDR "subsector %d";			break;
	case EVIS_GP_END:				fmt = ADDR "end %d";				break;

	case EVIS_OLD_CONDITION:		fmt = ADDR "cond %d";				break;
	case EVIS_OLD_LINEMODE:			fmt = ADDR "linemode %d";			break;
	case EVIS_OLD_SUBSECTOR:		fmt = ADDR "subsector %d";			break;
	case EVIS_OLD_END:				fmt = ADDR "END %d";				break;
	}
#undef ADDR

	return format(fmt, addr, addr, index);
}



// ================================================================ DreadMapGen ================================================================

void DreadMapGen::Clear()
{
	for( int i=0; i<(int)verts.size(); i++ )
		delete verts[i];
	verts.clear();

	for( int i=0; i<(int)lines.size(); i++ )
		delete lines[i];
	lines.clear();

	for( int i=0; i<(int)sectors.size(); i++ )
		delete sectors[i];
	sectors.clear();

	regions.Clear();
	visibility_done = false;
	things.clear();
	locations.clear();
	machines.clear();
	vis_conditions.clear();
	for( int i=0; i<(int)vis_groups.size(); i++ )
		delete vis_groups[i];
	vis_groups.clear();
	work_mesh.Clear();
	collision_mesh.Clear();
	collision_bsp_mesh.Clear();
	col_edges.clear();

	for( int i=0; i<(int)col_nodes.size(); i++ )
		delete col_nodes[i];
	col_nodes.clear();

	col_bsp_root_psize = NULL;
	col_bsp_root_0 = NULL;
}

void DreadMapGen::LoadMap(MapFileHeader *msrc, WadConverter *wconv)
{
	Clear();

	if( wconv )
	{
		//wconv->DebugDumpInternals();
		//wconv->DebugDumpMapFile();
	}

	// Allocate objects
	for( int i=0; i<msrc->n_lines; i++ )
		lines.push_back(new Line());
	
	for( int i=0; i<msrc->n_sectors; i++ )
		sectors.push_back(new Sector());

	// Load lines
	for( int i=0; i<msrc->n_lines; i++ )
	{
		MapFileLine &mline = msrc->lines[i];
		Line &l = *lines[i];
		MapFileVertex &mv1 = msrc->vertexes[mline.vertex];
		int light_level = mline.light_level;

		l.v1 = AssureVertex(mv1.xp, mv1.yp);
		if( mline.other_sector!=0xFFFF )
			l.other_sector = sectors[mline.other_sector];

		if( mline.line_action>=110 && mline.line_action<=118 )		// force texture light level
		{
			light_level = (mline.line_action-110)*32;
			if( light_level > 255 )
				light_level = 255;
		}

		l.tex_upper = gfx_converter.GetTextureId(mline.upper_tex.c_str(), light_level);
		l.tex_lower = gfx_converter.GetTextureId(mline.lower_tex.c_str(), light_level);
		l.line_action = mline.line_action;
		l.line_tag = mline.line_tag;
		l.line_flags = mline.line_flags;
		l.tex_offs_x = mline.xcoord1;
		l.tex_offs_y = mline.ycoord;
		l.tex_unpegged_upper = ((mline.line_flags & WadFile::LineDef::UPPER_UNPEGGED) != 0);
		l.tex_unpegged_lower = ((mline.line_flags & WadFile::LineDef::LOWER_UNPEGGED) != 0);
	}

	// Load sectors
	for( int i=0; i<msrc->n_sectors; i++ )
	{
		MapFileSector &msec = msrc->sectors[i];
		Sector &s = *sectors[i];
		s.first_line = lines[msec.first_line];
		s.ceil_h = -msec.ceil_h;
		s.floor_h = -msec.floor_h;
		s.tex_ceil = gfx_converter.GetTextureId(msec.ceil_tex.c_str(), msec.light_level);
		s.tex_floor = gfx_converter.GetTextureId(msec.floor_tex.c_str(), msec.light_level);
		s.type = msec.type;
		s.tag = msec.tag;

		for( int j=0; j<msec.num_lines; j++ )
		{
			int j2 = msec.first_line + (j+1)%msec.num_lines;

			Line &l = *lines[msec.first_line + j];
			l.sector = &s;
			l.next_line = lines[j2];
			lines[j2]->prev_line = &l;

			MapFileVertex &mv2 = msrc->vertexes[msrc->lines[j2].vertex];
			l.v2 = AssureVertex(mv2.xp, mv2.yp);

			l.tex_ceil = s.tex_ceil;
			l.tex_floor = s.tex_floor;
		}
	}

	// Link lines
	for( int i=0; i<(int)lines.size(); i++ )
	{
		MapFileLine &mline = msrc->lines[i];
		Line &l = *lines[i];

		for( int ul=0; ul<2; ul++ )
		{
			const string &mtex = *(ul ? &mline.lower_tex : &mline.upper_tex);
			texture_id_t *tex = (ul ? &l.tex_floor : &l.tex_ceil);
			
			if( mtex.size()>=4 && mtex[0]=='$' && mtex[1]=='F' )
				*tex = gfx_converter.GetTextureId(format("F_FIX%c%c", mtex[2], mtex[3]).c_str(), 255);
		}

		// Find other line
		if( l.other_sector && !l.other_line )
			for( int j=i+1; j<(int)lines.size(); j++ )
			{
				Line &l2 = *lines[j];
				if( l2.other_sector && l2.v1==l.v2 && l2.v2==l.v1 && !l2.other_line )
				{
					l.other_line = &l2;
					l2.other_line = &l;
					break;
				}
			}

		l.LoadSectorHeights(export_engine_version);
	}

	// Remove helper lines
	int d = 0;
	for( int i=0; i<(int)lines.size(); i++ )
		if( !lines[i]->sector->tex_floor )
			delete lines[i];
		else
			lines[d++] = lines[i];
	lines.resize(d);

	// Remove helper sectors
	d = 0;
	for( int i=0; i<(int)sectors.size(); i++ )
		if( !sectors[i]->tex_floor )
			delete sectors[i];
		else
			sectors[d++] = sectors[i];
	sectors.resize(d);

	// Load things
	things.assign(msrc->things, msrc->things+msrc->n_things);

	// Check line completeness
	for( int i=0; i<(int)lines.size(); i++ )
	{
		Line *line = lines[i];
		line->_debug_number = i;

		if( line->other_sector )
		{
			if( !line->other_line ) 
				printf("ERROR: 2-sided line link mismatch (%d A).\n", i);
			else if( line->other_line->other_sector!=line->sector )
				printf("ERROR: 2-sided line link mismatch (%d B).\n", i);
			else if( line->other_line->other_line!=line )
				printf("ERROR: 2-sided line link mismatch (%d C).\n", i);
		}
	}
	
	// Process line special actions
	for( int i=0; i<(int)lines.size(); i++ )
	{
		Line *line = lines[i];
		if( line->line_action>=1 && line->line_action<=10 )			// Door
			MakeDoor(line);


		if( line->line_action>=21 && line->line_action<=51 )		// Switch/anim
		{
			Machine sw;
			sw.type = line->line_action | 0x1000;
			sw.tag = line->line_tag;
			sw.line0 = line;
			machines.push_back(sw);
		}
	}

	// Process line texcoords
	for( int i=0; i<(int)lines.size(); i++ )
	{
		Line *line = lines[i];
		if( line->IsTextureDrawn() )
		{
			int len = (int)ceil((line->v2->point_vec2() - line->v1->point_vec2()).length());
			int off = line->tex_offs_x & 63;
			if( len >= 512 )
			{
				//line->Error("ERROR: Line length >= 512!");
				elog.MapWarn(this, line->midpoint(), "Line length >= 512");
			}
			else if( len + off >= 512 )
			{
				line->Error("ERROR: Line texture offset moves texcoord over 512!");
				elog.MapWarn(this, line->midpoint(), "Line texture offset moves texcoord over 512!");
			}
			line->tex_offs_x = off;
		}
		else
			line->tex_offs_x = 0;

		line->ComputeTextureOffsets();
	}

	// Compute splitter BSP.
	if( map_optimized_bsp )
	{
		splitter_bsp.Clear();
		for( int i=0; i<(int)lines.size(); i++ )
		{
			Line *line = lines[i];
			if( line->IsRequired() )
				splitter_bsp.active_lines.push_back(line);
		}
		splitter_bsp.BuildSplitterBSP();
	}

	//// Compute visibility
	//ComputeAllVisibility();

	// Collisions
	SetDoorsPosition(0);	// open all doors
	ComputeCollisionMesh(collision_mesh, 0, COLLISION_MODE_HITSCAN);
	ComputeCollisionMesh(collision_mesh, ENGINE_PLAYER_SIZE, COLLISION_MODE_PLAYER);

	// Close doors
	SetDoorsPosition(100);
}

DreadMapGen::Vertex *DreadMapGen::AssureVertex(int xp, int yp, bool special)
{
	if( !special )
	{
		for( int i=0; i<(int)verts.size(); i++ )
			if( verts[i]->xp==xp && verts[i]->yp==yp && !verts[i]->_special )
				return verts[i];
	}
	Vertex *v = new Vertex();
	verts.push_back(v);
	v->xp = xp;
	v->yp = yp;
	v->_special = special;
	return v;
}

void DreadMapGen::MakeDoor(Line *line)
{
	//
	//					back_out
	//		*---------------^---------------*
	//		|			back_in				|
	//		|door_edge 			   door_trak|
	//		|			front_in			|
	//		*---------------v---------------*
	//					front_out
	//
	//
	//
	printf("Door: ");
	if( !line->tex_upper ) { printf("ERROR: Can't make door (E1).\n"); return; }
	if( !line->other_sector ) { printf("ERROR: Can't make door (E2).\n"); return; }
	if( !line->other_line ) { printf("ERROR: Can't make door (E3).\n"); return; }
	if( !line->sector ) { printf("ERROR: Can't make door (E11).\n"); return; }
	if( !line->other_sector ) { printf("ERROR: Can't make door (E12).\n"); return; }

	Line *front_out = line;
	Line *front_in = line->other_line;
	Line *door_edge = front_in->next_line;
	Line *back_in = door_edge->next_line;
	Line *back_out = back_in->other_line;
	Line *door_trak = back_in->next_line;
	if( door_edge == front_in ) { printf("ERROR: Can't make door (E8).\n"); return; }
	if( back_in == front_in ) { printf("ERROR: Can't make door (E9).\n"); return; }
	if( door_trak == front_in ) { printf("ERROR: Can't make door (E10).\n"); return; }
	if( door_trak->next_line!=front_in ) { printf("ERROR: Can't make door (E7).\n"); return; }
	if( !back_out ) { printf("ERROR: Can't make door (E4).\n"); return; }
	if( door_edge->other_sector ) { printf("ERROR: Can't make door (E5).\n"); return; }
	//if( door_trak->other_sector ) { printf("ERROR: Can't make door (E6).\n"); return; }

	// fix sector heights
	front_out->other_sector->ceil_h = front_out->sector->ceil_h;
	front_out->LoadSectorHeights(export_engine_version);
	front_in->LoadSectorHeights(export_engine_version);
	door_edge->LoadSectorHeights(export_engine_version);
	back_in->LoadSectorHeights(export_engine_version);
	back_out->LoadSectorHeights(export_engine_version);
	door_trak->LoadSectorHeights(export_engine_version);

	//Vertex *vs1 = AssureVertex((front_out->v1->xp + front_out->v2->xp)/2, (front_out->v1->yp+front_out->v2->yp)/2, true);
	//Vertex *vs2 = AssureVertex((back_out->v1->xp + back_out->v2->xp)/2, (back_out->v1->yp + back_out->v2->yp)/2, true);
	Vertex *vs1 = AssureVertex(front_out->v1->xp, front_out->v1->yp, true);	// keep door open for computations
	Vertex *vs2 = AssureVertex(back_out->v2->xp, back_out->v2->yp, true);

	// make door front
	Line *door_front = new Line(*front_out);
	lines.push_back(door_front);
	{
		// link
		door_front->next_line = front_out;
		door_front->prev_line = front_out->prev_line;
		door_front->prev_line->next_line = door_front;
		front_out->prev_line = door_front;

		// setup
		door_front->v2 = vs1;
		door_front->other_sector = NULL;
		door_front->other_line = NULL;
		door_front->h2 = door_front->h3;
		door_front->line_action = 0;
		front_out->v1 = vs1;
		front_out->tex_upper.clear();
		front_in->v2 = vs1;
		door_edge->v1 = vs1;
	}

	// make back door
	Line *door_back = new Line(*back_out);
	lines.push_back(door_back);
	{
		// link
		door_back->prev_line = back_out;
		door_back->next_line = back_out->next_line;
		door_back->next_line->prev_line = door_back;
		back_out->next_line = door_back;

		// setup
		door_back->v1 = vs2;
		door_back->other_sector = NULL;
		door_back->other_line = NULL;
		door_back->h2 = door_back->h3;
		door_back->line_action = 0;
		door_back->tex_upper = door_front->tex_upper;
		back_out->v2 = vs2;
		back_out->tex_upper.clear();
		back_in->v1 = vs2;
		door_edge->v2 = vs2;
	}

	// visibility
	front_out->vis_require = true;
	back_out->vis_require = true;
	front_out->vis_condition = machines.size();
	back_out->vis_condition = machines.size();

	// save door existence
	Machine dr;
	dr.type = front_out->line_action;
	dr.tag = front_out->line_tag;
	dr.line0 = door_edge;
	dr.line1 = door_front;
	dr.line2 = door_back;
	dr.line_x1 = front_out;
	dr.line_x2 = back_out;
	dr.is_door = true;
	machines.push_back(dr);

	front_out->line_action = 0;
	front_in->line_action = 0;
	back_out->line_action = 0;
	back_in->line_action = 0;

	printf("OK (%d)!\n", dr.type);
}

void DreadMapGen::SetDoorsPosition(int prc)
{
	for( int i=0; i<(int)machines.size(); i++ )
	{
		Machine &m = machines[i];
		if( m.is_door )
		{
#define LERP(a,b,p)		((a)+((b)-(a))*(p)/100)
			m.line1->v2->xp = LERP(m.line1->v1->xp, m.line_x1->v2->xp, prc);
			m.line1->v2->yp = LERP(m.line1->v1->yp, m.line_x1->v2->yp, prc);
			m.line2->v1->xp = LERP(m.line2->v2->xp, m.line_x2->v1->xp, prc);
			m.line2->v1->yp = LERP(m.line2->v2->yp, m.line_x2->v1->yp, prc);
#undef LERP
		}
	}
}



DreadMapGen::Line *DreadMapGen::FindClosestLine(vec2 pos, float range)
{
	float min_dist = range;
	Line *best = NULL;

	for( int i=0; i<(int)lines.size(); i++ )
	{
		Line &l = *lines[i];
		vec2 dp = pos - vec2(l.v1->xp, l.v1->yp);
		vec2 lv = vec2(l.v2->xp, l.v2->yp) - vec2(l.v1->xp, l.v1->yp);
		if( l.other_sector )
		{
			if( lv.get_rotated90().dot(dp)<0 )
				continue;
		}
		float p = lv.dot(dp)/lv.dot(lv);
		if( p<0 ) p=0;
		if( p>1 ) p=1;
		vec2 lp = lv*p;
		float dist = (dp-lp).length();

		if( dist<min_dist )
		{
			min_dist = dist;
			best = &l;
		}
	}

	return best;
}

DreadMapGen::RegionPoly *DreadMapGen::FindCameraRegion(vec2 pos, bool brute_force)
{
	if( !brute_force )
	{
		// BSP version
		BSPNode *node = regions.bsproot;
		while( node && !node->region )
			node = (node->line.CheckSideVec2(pos) >= 0) ? node->right : node->left;

		if( node )
			return node->region;
	}
	else
	{
		// Brute force version
		return regions.FindPointRegion(pos);
	}

	return NULL;
}



void DreadMapGen::ComputeVisibility(Line *line, bool reset)
{
	if( reset )
	{
		ResetVisibility();
		regions.SnapshotPolySet();
		ComputeInitialBSPTree();
		regions.SnapshotPolySet();
	}

	if( !line->IsDrawn() && !line->vis_require )
	{
		// invisible line
		return;
	}

	if( line->IsDegenerated() )
		return;

	Beam beam;
	beam.p1s.push_back(line->v1->point_rp2());
	beam.p2s.push_back(line->v2->point_rp2());
	beam.portal = beam.p1s[0].LineTo(beam.p2s[0]);
	beam.l1 = beam.portal;
	beam.l2 = beam.portal;

	Polygon2D beam_poly;
	beam_poly.CreateRectangle(-REGION_SIZE, -REGION_SIZE, REGION_SIZE, REGION_SIZE);
	beam_poly.ClipBy(beam.portal);

	ComputeVisibility_Rec(line, line->sector, beam, beam_poly, 0);
}

void DreadMapGen::ComputeAllVisibility()
{
	SetDoorsPosition(0);	// open all doors

	// Reset
	printf("Vis: Reset...\n");
	ResetVisibility();

	// Initial sector split BSP
	printf("Vis: Initial BSP...\n");
	regions.SnapshotPolySet();
	ComputeInitialBSPTree();
//	ComputeFinalBSPTree(true);
//	visibility_done = true;
//	return;

	// Compute visibility for all lines
	LARGE_INTEGER qp_start, qp_end, qp_freq;
	printf("Vis: Line vis regions...\n");
	regions.SnapshotPolySet();

	perf_listresolve_time.QuadPart = 0;
	perf_subregion_time.QuadPart = 0;
	QueryPerformanceCounter(&qp_start);
	for( int i=0; i<(int)lines.size(); i++ )
	{
		ComputeVisibility(lines[i], false);

		if( regions.UsedRegionsCount() >= MAX_REGIONS_COUNT )
		{
			ComputeFinalBSPTree(false);
			regions.OptimizeMemUsage();
		}

		printf("\rVisibility %5.1f%%  /%7d regions", (i+1)*100.f/lines.size(), regions.all_polys.size());
	}
	printf("  -  DONE\n");
	QueryPerformanceCounter(&qp_end);
	QueryPerformanceFrequency(&qp_freq);

	printf("Vis: %d regions.\n", regions.poly_set.size());
	printf("Vis: %d ms (%d ms for list resolve, %d ms for subregioning)\n",
		int(double(qp_end.QuadPart - qp_start.QuadPart)*1000/qp_freq.QuadPart),
		int(double(perf_listresolve_time.QuadPart)*1000/qp_freq.QuadPart),
		int(double(perf_subregion_time.QuadPart)*1000/qp_freq.QuadPart)
	);

	regions.UnmarkAll();

	// Copy machine visibility
	for( int i=0; i<(int)machines.size(); i++ )
	{
		Machine &m = machines[i];
		if( m.is_door )
		{
			// door_front(1) includes visibility from front_out(x1), door_back(2) includes from back_out(x2), door_edge(0) includes from both(x1+x2)
			IncludeLineVisibility(m.line0, m.line_x1);
			IncludeLineVisibility(m.line0, m.line_x2);
			IncludeLineVisibility(m.line1, m.line_x1);
			IncludeLineVisibility(m.line2, m.line_x2);
		}
	}
	
	// Build final BSP
	if( !map_optimized_bsp )
	{
		printf("Vis: Final BSP...\n");
		ComputeFinalBSPTree(true);
	}

	regions.OptimizeMemUsage();

	
	// Compule line order lists
	SetDoorsPosition( 50);	// all doors half-opened
	printf("Vis: Line draw order...\n");
	BuildVisDrawLists();
	SetDoorsPosition(100);	// close all doors

	//// Compute collisions
	//printf("Vis: Collisions...\n");
	//for( auto p=regions.poly_set.begin(); p!=regions.poly_set.end(); ++p )
	//	ComputeCollisions(*p);

	printf("Vis: peak used %d regions (max %d)\n", regions.UsedRegionsPeak(), RegionMap::MAX_REGIONS);

	printf("Vis: Done.\n");
	visibility_done = true;
}

void DreadMapGen::AssureVisibility()
{
	if( !visibility_done )
		ComputeAllVisibility();
}


void DreadMapGen::ResetVisibility()
{
	// regions.Init(REGION_SIZE);

	regions.Clear();
	visibility_done = false;

	if( map_optimized_bsp )
	{
		for( int i=0; i<(int)splitter_bsp.all_nodes.size(); i++ )
		{
			SplitterBSP::Node *node = splitter_bsp.all_nodes[i];
			if( node->sector && !node->poly.IsEmpty() )
			{
				RegionPoly *p = regions.NewRegion();

				p->poly = node->poly;
				p->sector = node->sector;
			}
		}

	}
	else
	{
		for( int i=0; i<(int)sectors.size(); i++ )
		{
			RegionPoly *p = regions.NewRegion();

			MakePolySector(sectors[i], p->poly);
			p->sector = sectors[i];
		}
	}
}

DreadMapGen::VisLine *DreadMapGen::RegisterVisLine(RegionPoly *region, Line *line, const Beam &beam, const Polygon2D &visregion)
{
	if( beam.conditions.size() > MAX_SEE_THROUGH )
		return NULL;

	VisLine *vis = region->vis[line];
	bool new_vis = false;
	if( !vis )
	{
		vis = new VisLine();
		region->vis[line] = vis;
		vis->line = line;
		new_vis = true;
	}

	// range
	if( beam.lineportals.size()==0 )
	{
		vis->range_y1_min = -10000;	// full visibility
	}
	else
	{
		vis->first_draw = false;

		vec2 v1 = line->v1->point_vec2();
		vec2 v2 = line->v2->point_vec2();
		vec2 n = (v2-v1).get_rotated90().get_normalized();

		// poly range
		float pmin = 1e38;
		float pmax = 0;
		for( int i=0; i<(int)visregion.edges.size(); i++ )
		{
			float d = (visregion.edges[i].start.GetVec2()-v1).dot(n);
			if( d<pmin ) pmin=d;
			if( d>pmax ) pmax=d;
		}

		if( pmin<=0 )
			vis->range_y1_min = -10000;	// full visibility

										// process clippers
		const float cam_h = -40;
		float ymin  = -10000;
		for( int i=0; i<(int)beam.lineportals.size(); i++ )
		{
			Line *clip = beam.lineportals[i];
			float d1 = (clip->v1->point_vec2()-v1).dot(n);
			float d2 = (clip->v2->point_vec2()-v1).dot(n);
			if( d2<d1 ) swap(d1, d2);

			// ymin
			float ym;
			if( d2<=0 )			ym = ymin;		// error
			else if( d2>=pmin )	ym = ymin;		// don't care - camera camera can get past the line
			else				ym = cam_h - (cam_h - clip->h2)/(pmin-d2)*pmin;

			if( ym > ymin )
				ymin = ym;	// despite best efforts, limits the view

			// ycull
			float yc;
			if( d1<=0 )			yc = clip->h2;	// maximum clip
			else if( d1>=pmax )	yc = -1e38;		// error
			else				yc = cam_h - (cam_h - clip->h2)/(pmax-d1)*pmax;

			if( yc > vis->range_y1_clip )
				vis->range_y1_clip = yc;
		}

		if( ymin < vis->range_y1_min )
			vis->range_y1_min = ymin;
	}

	// conditional visibility
	if( new_vis || vis->condition>=0 )
	{
		int cond = -1;
		for( int i=(int)beam.conditions.size()-1; i>=0; i-- )
		{
			int mc = GetMachineCondition(beam.conditions[i]);
			cond = (cond>=0) ? GetAndCondition(cond, mc) : mc;
		}
		if( cond<0 )
			vis->condition = -1;		// unconditional visibility
		else if( new_vis )
			vis->condition = cond;		// first condition
		else
			vis->condition = GetOrCondition(vis->condition, cond);
	}

	return vis;
}

DreadMapGen::VisSector *DreadMapGen::RegisterVisSector(RegionPoly *region, Sector *sec, const Beam &beam)
{
	if( beam.conditions.size() > MAX_SEE_THROUGH )
		return NULL;
	
	VisSector *vis = region->svis[sec];
	bool new_vis = false;
	if( !vis )
	{
		vis = new VisSector();
		vis->sector = sec;
		region->svis[sec] = vis;
		new_vis = true;
	}

	// conditional visibility
	if( new_vis || vis->condition>=0 )
	{
		int cond = -1;
		for( int i=(int)beam.conditions.size()-1; i>=0; i-- )
		{
			int mc = GetMachineCondition(beam.conditions[i]);
			cond = (cond>=0) ? GetAndCondition(cond, mc) : mc;
		}
		if( cond<0 )
			vis->condition = -1;		// unconditional visibility
		else if( new_vis )
			vis->condition = cond;		// first condition
		else
			vis->condition = GetOrCondition(vis->condition, cond);
	}

	return vis;
}

int DreadMapGen::GetMachineCondition(int midx)
{
	for( int i=0; i<(int)vis_conditions.size(); i++ )
		if( vis_conditions[i].op == VisCondition::OP_MACHINE &&
			vis_conditions[i].c1 == midx )
			return i;

	VisCondition vc;
	vc.op = VisCondition::OP_MACHINE;
	vc.c1 = midx;
	vc._opt_numsources = 1;
	vc._opt_sources[0] = midx;
	vc._opt_mask = 1;
	vis_conditions.push_back(vc);
	return vis_conditions.size() - 1;
}

int DreadMapGen::GetAndCondition(int c1, int c2)
{
	if( c1==c2 ) return c1;
	if( c2<0 ) return c1;
	if( c1<0 ) return c2;

	for( int i=0; i<(int)vis_conditions.size(); i++ )
		if( vis_conditions[i].op == VisCondition::OP_AND )
		{
			if( vis_conditions[i].c1 == c1 && vis_conditions[i].c2 == c2 ) return i;
			if( vis_conditions[i].c1 == c2 && vis_conditions[i].c2 == c1 ) return i;
		}
	VisCondition vc;
	vc.op = VisCondition::OP_AND;
	vc.c1 = c1;
	vc.c2 = c2;
	vis_conditions.push_back(vc);
	return vis_conditions.size() - 1;
}

int DreadMapGen::GetOrCondition(int c1, int c2)
{
	if( c1==c2 ) return c1;									// same
	if( c1<0 || c2<0 ) return -1;							// one is TRUE
	if( FindOrConditionComponentRec(c1, c2) ) return c1;	// <c1> includes <c2>
	if( FindOrConditionComponentRec(c2, c1) ) return c2;	// <c2> includes <c1>
	if( FindAndConditionComponentRec(c2, c1) ) return c1;	// <c1> is already required in <c2>
	if( FindAndConditionComponentRec(c1, c2) ) return c2;	// <c2> is already required in <c1>

	for( int i=0; i<(int)vis_conditions.size(); i++ )
		if( vis_conditions[i].op == VisCondition::OP_OR )
		{
			if( vis_conditions[i].c1 == c1 && vis_conditions[i].c2 == c2 ) return i;
			if( vis_conditions[i].c1 == c2 && vis_conditions[i].c2 == c1 ) return i;
		}
	VisCondition vc;
	vc.op = VisCondition::OP_OR;
	vc.c1 = c1;
	vc.c2 = c2;
	vis_conditions.push_back(vc);
	return vis_conditions.size() - 1;
}

// Returns 'true' is <searchcon> is part of AND condition <cid>.
bool DreadMapGen::FindAndConditionComponentRec(int cid, int searchcon)
{
	if( cid==searchcon ) return true;

	VisCondition &vc = vis_conditions[cid];
	if( vc.op!=VisCondition::OP_AND ) return false;
	if( FindAndConditionComponentRec(vc.c1, searchcon) ) return true;
	return FindAndConditionComponentRec(vc.c2, searchcon);
}

// Returns 'true' is <searchcon> is part of OR condition <cid>.
bool DreadMapGen::FindOrConditionComponentRec(int cid, int searchcon)
{
	if( cid==searchcon ) return true;

	VisCondition &vc = vis_conditions[cid];
	if( vc.op!=VisCondition::OP_OR ) return false;
	if( FindOrConditionComponentRec(vc.c1, searchcon) ) return true;
	return FindOrConditionComponentRec(vc.c2, searchcon);
}


void DreadMapGen::SelectVisLineCheat(VisLine *vis)
{
	Line *line = vis->line;

	if( export_engine_version == 2 )
	{
		bool is_two_sided = line->other_line && line->other_sector;
		bool has_sky = !line->tex_ceil;
		bool draw_ceiling = true;		// flat color or sky
		bool draw_upper = true;
		bool draw_lower = true;
		bool draw_floor = true;
		bool is_first = vis->first_draw;

		if( !is_two_sided )
		{
			// Disable lower if not needed
			if( line->h3>=line->h4 )
				draw_lower = false;
		}
		else	// 2-sided
		{
			// Disable ceiling & upper if both sectors are sky
			if( has_sky && line->tex_ceil==line->other_line->tex_ceil )
			{
				draw_ceiling = false;
				draw_upper = false;
			}

			// Disable upper if not needed
			if( line->h1>=line->h2 )
			{
				draw_upper = false;
				if( line->tex_ceil==line->other_line->tex_ceil && line->sector->ceil_h==line->other_sector->ceil_h )
					draw_ceiling = false;
			}

			// Disable lower if not needed
			if( line->h3>=line->h4 )
			{
				draw_lower = false;
				if( line->tex_floor==line->other_line->tex_floor && line->sector->floor_h==line->other_sector->floor_h )
					draw_floor = false;
			}
		}

		int cost = 100;
		int mode = 0;
#include "gen/render2_cheatselector.inc"
		vis->cheat_mode = mode;
	}
	else
	{
		// cheat function number
		int fm=-2, cm=-2;
#include "gen/cheatselector.inc"

		vis->cheat_mode = vis->first_draw ? fm : cm;
	}
}


void DreadMapGen::ComputeVisibility_Rec(Line *line, Sector *sec, Beam &beam, const Polygon2D &beam_poly, int depth)
{
	if( depth>=32 )
		return;

	int _beam_vislines = (int)beam.vislines.size();
	int _beam_conditions = (int)beam.conditions.size();

	// Process current sector
	{
		Polygon2D tmp, secpoly;
		MakePolySector(sec, tmp);

		if( !tmp.ComputeIntersection(beam_poly, secpoly) )
			return;
		
		DreadMapGen::RegionPoly *ip = regions.SelectRegion(secpoly, mapvis_allow_region_splitting);
		for( ; ip; ip=ip->next )
		{
			ip->marked = true;

			// Register line in the region
			VisLine *vis = RegisterVisLine(ip, line, beam, secpoly);

			// Register sectors in the region
			for( int j=0; j<(int)beam.sectors.size(); j++ )
				RegisterVisSector(ip, beam.sectors[j], beam);

			// Add self to the visibility list for further sectors
			if( vis )
				beam.vislines.push_back(vis);
		}
	}

	beam.sectors.push_back(sec);

	// Descend into next sectors
	{
		Line *sline = sec->first_line;
		LineRP2 _l1 = beam.l1;
		LineRP2 _l2 = beam.l2;
		LineRP2 _portal = beam.portal;
		size_t _p1s = beam.p1s.size();
		size_t _p2s = beam.p2s.size();
		size_t _lps = beam.lineportals.size();

		// Rotate to start from the portal
		do
		{
			if( _portal.CheckSide(sline->v2->point_rp2())==0 &&
				_portal.CheckSide(sline->v1->point_rp2())==0 )
				break;

			sline = sline->next_line;
		} while( sline!=sec->first_line );

		// Descend
		Line *endline = sline;
		do
		{
			if( sline->other_sector && !sline->IsDegenerated() )
			{
				PointRP2 p1 = sline->v2->point_rp2();	// consider other side of this line
				PointRP2 p2 = sline->v1->point_rp2();

				if( beam.portal.CheckSide(p1)>0 || beam.portal.CheckSide(p2)>0 )
				{
					beam.portal = p1.LineTo(p2);

					if( beam.l1.CheckSide(p1)>0 )
					{
						beam.l1 = p1.LineTo(beam.p2s[0]);
						for( int k=1; k<_p2s; k++ )
							if( beam.l1.CheckSide(beam.p2s[k]) < 0 )
								beam.l1 = p1.LineTo(beam.p2s[k]);

						beam.p1s.push_back(p1);
					}
					if( beam.l2.CheckSide(p2)>0 )
					{
						beam.l2 = beam.p1s[0].LineTo(p2);
						for( int k=1; k<_p1s; k++ )
							if( beam.l2.CheckSide(beam.p1s[k]) < 0 )
								beam.l2 = beam.p1s[k].LineTo(p2);

						beam.p2s.push_back(p2);
					}

					Polygon2D new_beam_poly = beam_poly;
					new_beam_poly.ClipBy(beam.portal);
					new_beam_poly.ClipBy(beam.l1);
					new_beam_poly.ClipBy(beam.l2);

					if( !new_beam_poly.IsEmpty() )
					{
						if( sline->other_line && sline->other_line->IsDrawn() )	// is drawn?
							beam.lineportals.push_back(sline);

						if( sline->vis_condition>=0 )
						{
							bool found = false;
							for( int i=0; i<(int)beam.conditions.size(); i++ )
								if( beam.conditions[i]==sline->vis_condition )
								{
									found = true;
									break;
								}
							if( !found )
								beam.conditions.push_back(sline->vis_condition);
						}

						ComputeVisibility_Rec(line, sline->other_sector, beam, new_beam_poly, depth+1);
					}

					beam.p1s.resize(_p1s);
					beam.p2s.resize(_p2s);
					beam.l1 = _l1;
					beam.l2 = _l2;
					beam.portal = _portal;
					beam.lineportals.resize(_lps);
					beam.conditions.resize(_beam_conditions);
				}
			}

			sline = sline->next_line;
		} while( sline!=endline );
	}

	beam.vislines.resize(_beam_vislines);
	beam.sectors.pop_back();
}

void DreadMapGen::IncludeLineVisibility(Line *line, Line *vis_source)
{
	if( !line || !vis_source )
		return;

	for( auto p=regions.poly_set.begin(); p!=regions.poly_set.end(); ++p )
	{
		RegionPoly *reg = *p;
		auto q = reg->vis.find(vis_source);
		if( q == reg->vis.end() )
			continue;
		VisLine *vsrc = q->second;

		VisLine *vis = NULL;
		auto v = reg->vis.find(line);
		if( v == reg->vis.end() )
		{
			vis = new VisLine(*vsrc);
			vis->line = line;
			reg->vis[line] = vis;
		}
		else
			vis = v->second;

		vis->IncludeVisInfo(vsrc, this);
	}
}

void DreadMapGen::MakePolySector(Sector *sec, Polygon2D &out_poly)
{
	vector<PointRP2> pts;

	Line *line = sec->first_line;
	do
	{
		pts.push_back(line->v1->point_rp2());
		line = line->next_line;
	} while( line!=sec->first_line );

	out_poly.CreateFromPoints(&pts[0], (int)pts.size());
	out_poly.CleanupColinear();
	out_poly.UpdateBox();
}


void DreadMapGen::Vis_DumpLocks(const char *path)
{
	FILE *fp = fopen(path, "wt");
	if( !fp ) return;

	for( auto p=regions.poly_set.begin(); p!=regions.poly_set.end(); ++p )
	{
		RegionPoly *reg = *p;
		fprintf(fp, "Region %p\n", reg);
		for( auto q=reg->_line_order_locks.begin(); q!=reg->_line_order_locks.end(); ++q )
		{
			Line *line = q->first;
			vector<VisLine*> &locked = q->second;
			for( int i=0; i<locked.size(); i++ )
				fprintf(fp, "  lock L%d -> L%d\n", line->_debug_number, locked[i]->line->_debug_number);
		}
	}

	fclose(fp);
}

void DreadMapGen::Vis_GatherSectors()
{
	printf("%d regions to consider\n", regions.poly_set.size());

	for( int i=0; i<sectors.size(); i++ )
		sectors[i]->regions.clear();

	for( auto p=regions.poly_set.begin(); p!=regions.poly_set.end(); ++p )
	{
		RegionPoly *reg = *p;
		reg->sector->regions.push_back(reg);
	}
}

void DreadMapGen::Vis_PrepareRegion(RegionPoly *reg)
{
	// Reset region
	reg->drawlist.clear();
	reg->grouplist.clear();
	reg->_vis_line_count = 0;
	reg->_vis_pending.clear();
	reg->_line_order_locks.clear();

	// Gather and reset lines
	for( auto p=reg->vis.begin(); p!=reg->vis.end(); ++p )
		if( p->first->IsDrawn() )
		{
			p->second->draw_order = -1;
			p->second->locknum = 0;

			SelectVisLineCheat(p->second);

			reg->_vis_pending.push_back(VisEntry(p->second));
		}

	// Find visibility restrictions
	Polygon2D poly;

	for( int pi=0; pi<reg->_vis_pending.size(); pi++ )
		for( int qi=0; qi<reg->_vis_pending.size(); qi++ )
		{
			VisLine *va = reg->_vis_pending[pi].visline;		// only vislines are in the pending list so far
			VisLine *vb = reg->_vis_pending[qi].visline;

			if( va >= vb )
				continue;

			Line *a = va->line;		// only vislines are in the pending list so far
			Line *b = vb->line;

			PointRP2 a1 = a->v1->point_rp2();
			PointRP2 a2 = a->v2->point_rp2();
			PointRP2 b1 = b->v1->point_rp2();
			PointRP2 b2 = b->v2->point_rp2();
			LineRP2 la = a1.LineTo(a2);
			LineRP2 lb = b1.LineTo(b2);
			int m = 0;
			if( la.CheckSide(b2) > 0 ) m |= 1;
			if( la.CheckSide(b1) > 0 ) m |= 2;
			if( lb.CheckSide(a2) > 0 ) m |= 4;
			if( lb.CheckSide(a1) > 0 ) m |= 8;

			static const int MODE[16] ={
				0, 5, 6, 7,
				2, 0, 0, 5,
				1, 0, 0, 6,
				3, 2, 1, 0,
			};
			m = MODE[m];
			if( !m ) continue;
			if( m&4 ) swap(a, b), swap(va, vb);
			if( b->sector==reg->sector ) continue;	// can't be covered

			poly = reg->poly;
			poly.ClipBy(la);
			poly.ClipBy(lb);

			if( m&1 ) poly.ClipBy(a1.LineTo(b2));
			if( m&2 ) poly.ClipBy(b1.LineTo(a2));

			if( !poly.IsEmpty() )
			{
//				printf("  line lock L%d -> L%d\n", a->_debug_number, b->_debug_number);
				reg->_line_order_locks[a].push_back(vb);
				vb->locknum++;
			}
		}

	// Make sure the <svis> includes the sector itself
	if( reg->svis.find(reg->sector) == reg->svis.end() )
	{
		VisSector *vis = new VisSector();
		vis->sector = reg->sector;
		reg->svis[reg->sector] = vis;
	}

	// Gather and reset subsectors
	for( auto p=reg->svis.begin(); p!=reg->svis.end(); ++p )
	{
		p->second->draw_order = -1;
		reg->_vis_pending.push_back(VisEntry(p->second));
	}
}

bool DreadMapGen::Vis_ComputeDrawOrder(RegionPoly *reg)
{
	// Compule valid order
	bool work			= true;		// something was done - keep running
	bool was_error		= false;	// ordering gone wrong - mute further error messages
	bool check_order	= true;		// force order restrictions
	bool picky			= true;		// consider only walls with same condition
	bool todo			= false;
	int conmode			= -1;


	while( work )
	{
		work = false;
		todo = false;

		for( int i=0; i<reg->_vis_pending.size(); i++ )
			if( reg->_vis_pending[i].IsPending() )
			{
				VisEntry &ve = reg->_vis_pending[i];
				if( check_order && ve.visline && ve.visline->locknum>0 )
				{
					todo = true;
					continue;
				}

				if( picky && ve.condition!=conmode )
				{
					todo = true;
					continue;
				}

				// Mark for draw
				if( ve.visline )
				{
					ve.visline->draw_order = ++reg->_vis_line_count;	//  (int)reg->drawlist.size();

					// Unlock lines
					vector<VisLine*> &locks = reg->_line_order_locks[ve.visline->line];
					for( int j=0; j<(int)locks.size(); j++ )
						locks[j]->locknum--;
					check_order = true;
					ve.visline->vis_done = true;
				}
				
				if( ve.vissector )
				{
					ve.vissector->draw_order = (int)reg->drawlist.size();
					ve.vissector->vis_done = true;
				}

				reg->drawlist.push_back(ve);

				work = true;
				picky = true;
				conmode = ve.condition;
			}

		if( !todo )
			break;

		if( picky && !work )
		{
			// Try again, allowing condition to change
			picky = false;
			work = true;
			continue;
		}

		if( !work )
		{
			if( !was_error )
			{
				was_error = true;

				vec2 mid = reg->MidPoint();
				//printf("WARNING: No valid line order for visibility region at (%.0f,%.0f)!!!\n", mid.x, mid.y);
				ErrorLog::LogEntry *e = elog.MapWarn(this, ivec2(mid.x, mid.y), "No valid line order for visibility region");

//				Vis_DumpLocks("debug/vis-locked.txt");		//TBD: remove
//				printf("Vislocked region %p\n", reg);

				// trace dependency chain
				map<Line*, int> visited;		// 0: not visited	1: on stack		2: done
				map<Line*, Line*> line_next;
				vector<Line*> line_stack;
				bool found = false;
				for( int i=0; !found && i<reg->_vis_pending.size(); i++ )
				{
					VisEntry &ve = reg->_vis_pending[i];
					if( ve.IsPending() && ve.visline && ve.visline->locknum>0 && !visited[ve.visline->line] )
					{
						Line *line = ve.visline->line;
						line_stack.clear();
						line_stack.push_back(line);
						visited[line] = 1;

						// Depth-first search
						while( !found && line_stack.size() > 0 )
						{
							line = line_stack.back();
							vector<VisLine*> &locks = reg->_line_order_locks[line];
							bool line_done = true;
							for( VisLine *lock : locks )
								if( lock->line && !lock->vis_done && lock->locknum>0 )
								{
									if( visited[lock->line] == 1 )
									{
										line_next[line] = lock->line;
										while( line_stack.size()>0 && line_stack.front()!=lock->line )
											line_stack.erase(line_stack.begin());
										found = true;
										break;
									}
									if( !visited[lock->line] )
									{
										line_next[line] = lock->line;
										line = lock->line;
										visited[line] = 1;
										line_stack.push_back(line);
										line_done = false;
										break;
									}
								}
							if( !found && line_done )
							{
								visited[line] = 2;
								line_stack.pop_back();
							}
						}
					}
				}

				if( found )
				{
					for( Line* line : line_stack )
					{
						printf("  vis lock: %s -> %s\n", line->ToString().c_str(), line_next[line]->ToString().c_str());
						if( e )
							e->map_debug.push_back(ErrorLog::MapDebugLine(line->v1->point_vec2(), line->v2->point_vec2(), 0xFFFF00));
					}
				}
				else
					printf("  No vis lock loop found while scanning.\n");



				break;
			}
			check_order = false;
			work = true;
		}
	}

	return !was_error;
}

void DreadMapGen::Vis_CleanupRegion(RegionPoly *reg)
{
	reg->_vis_pending.swap(vector<VisEntry>());
	reg->_line_order_locks.clear();
}

void DreadMapGen::Vis_AddUserRegion(RegionPoly *reg)
{
	for( auto q=reg->_vis_pending.begin(); q!=reg->_vis_pending.end(); ++q )
	{
		VisEntry *vis = &*q;
		if( !vis->IsPending() ) continue;
		if( reg->_vis_scan_has_condition && vis->condition!=reg->_vis_scan_condition ) continue;

		if( vis->visline )
		{
			if( reg->_vis_scan_has_first_draw && vis->visline->first_draw!=reg->_vis_scan_first_draw ) continue;

			if( !reg->_vis_scan_respect_order || vis->visline->locknum<=0 )
				vis->visline->line->_vis_users.push_back(reg);
		}

		if( vis->vissector )
			vis->vissector->sector->_vis_users.push_back(reg);
	}
}

void DreadMapGen::Vis_BuildUserLists(vector<RegionPoly*> *users)
{
	for( int i=0; i<(int)lines.size(); i++ )
		lines[i]->_vis_users.clear();

	for( int i=0; i<(int)sectors.size(); i++ )
		sectors[i]->_vis_users.clear();

	if( users )
	{
		for( RegionPoly *reg : *users )
			Vis_AddUserRegion(reg);
	}
	else
	{
		for( auto p=regions.poly_set.begin(); p!=regions.poly_set.end(); ++p )
			Vis_AddUserRegion(*p);
	}
}

bool DreadMapGen::Vis_BuildGroup()
{
	VisGroup *vis_group = new VisGroup();
	vector<RegionPoly*>	users;

	// Reset regions scan
	for( auto p=regions.poly_set.begin(); p!=regions.poly_set.end(); ++p )
	{
		RegionPoly *reg = *p;
		reg->_vis_scan_group_ref = NULL;	// clear group ref
		reg->_vis_scan_has_condition = false;
		reg->_vis_scan_has_first_draw = false;
	}


	Vis_BuildUserLists(NULL);
	vis_group->max_users = (int)users.size();

//	printf("VIS GROUP START\n");

	while(1)
	{
		// ================================ Find Line or sector with maximum number of users ================================
		int max_user_count = 0;
		VisGroupEntry max_entry;
//		int max_line_index = -1;

		for( int i=0; i<(int)lines.size(); i++ )
			if( lines[i]->_vis_users.size() > max_user_count )
			{
				max_user_count = lines[i]->_vis_users.size();
				max_entry.line = lines[i];
//				max_line_index = i;
			}

		for( int i=0; i<(int)sectors.size(); i++ )
			if( sectors[i]->_vis_users.size() > max_user_count )
			{
				max_user_count = sectors[i]->_vis_users.size();
				max_entry.line = NULL;
				max_entry.sector = sectors[i];
			}

		// ================================ Copy user list ================================
		if( max_entry.line )
		{
			users = max_entry.line->_vis_users;
//			printf("  line %d ->", max_line_index);
//			for( int i=0; i<(int)users.size(); i++ )
//				printf(" %p", users[i]);
//			printf("\n");
		}
		else if( max_entry.sector )
			users = max_entry.sector->_vis_users;
		else
			break;


		// ================================ Add entry to visibility group ================================
		int count = max_entry.sector ? max_entry.sector->regions.size() : 1;
		vis_group->vis.push_back(max_entry);
		vis_group->entry_count += count;
		vis_group->instances_used += count * users.size();

		if( max_entry.line )
		{
			vis_group->visline_count++;

			for( RegionPoly *reg : users )
			{
				// Unlock lines
				VisLine *visline = reg->vis[max_entry.line];
				vector<VisLine*> &locks = reg->_line_order_locks[max_entry.line];
				for( int j=0; j<(int)locks.size(); j++ )
					locks[j]->locknum--;
				locks.clear();
				reg->_vis_scan_respect_order = true;
				visline->vis_done = true;

				reg->_vis_scan_has_first_draw = true;
				reg->_vis_scan_first_draw = visline->first_draw;

				reg->_vis_scan_has_condition = true;
				reg->_vis_scan_condition = visline->condition;

				// Register
				if( !reg->_vis_scan_group_ref )
				{
					reg->grouplist.push_back(VisGroupReference());
					reg->_vis_scan_group_ref = &reg->grouplist.back();

					reg->_vis_scan_group_ref->group = vis_group;
					reg->_vis_scan_group_ref->condition = reg->_vis_scan_condition;
				}
				reg->_vis_scan_group_ref->stop_index = vis_group->entry_count;
				reg->_vis_scan_group_ref->first_draw = reg->_vis_scan_first_draw;	// always update first_draw because the group might have contained sectors only
			}
		}
		else if( max_entry.sector )
		{
			for( RegionPoly *reg : users )
			{
				VisSector *vissector = reg->svis[max_entry.sector];
				vissector->vis_done = true;
				reg->_vis_scan_has_condition = true;
				reg->_vis_scan_condition = vissector->condition;

				// Register
				if( !reg->_vis_scan_group_ref )
				{
					reg->grouplist.push_back(VisGroupReference());
					reg->_vis_scan_group_ref = &reg->grouplist.back();

					reg->_vis_scan_group_ref->group = vis_group;
					reg->_vis_scan_group_ref->condition = reg->_vis_scan_condition;
				}
				reg->_vis_scan_group_ref->stop_index = vis_group->entry_count;
			}
		}
	
		// ================================ Prepare for next pass ================================
		Vis_BuildUserLists(&users);
	}

	if( vis_group->vis.size() <= 0 )
	{
		delete vis_group;
		return false;
	}

	vis_groups.push_back(vis_group);

	return true;
}


void DreadMapGen::BuildVisDrawLists()
{
	Vis_GatherSectors();
//	Vis_DumpLocks("debug/vis-zero.txt");

	for( auto p=regions.poly_set.begin(); p!=regions.poly_set.end(); ++p )
		Vis_PrepareRegion(*p);
//	Vis_DumpLocks("debug/vis-initial.txt");

	if( export_vis_groups )
	{
		//printf("Vis: Building groups\n");
		while( Vis_BuildGroup() ){}
	}
	else
	{
		for( auto p=regions.poly_set.begin(); p!=regions.poly_set.end(); ++p )
			Vis_ComputeDrawOrder(*p);
	}

	for( auto p=regions.poly_set.begin(); p!=regions.poly_set.end(); ++p )
		Vis_CleanupRegion(*p);
}

DreadMapGen::BSPNode *DreadMapGen::CleanupBSPRec(BSPNode *node)
{
	if( !node ) return NULL;
	if( node->region ) return node;

	node->left = CleanupBSPRec(node->left);
	node->right = CleanupBSPRec(node->right);

	if( !node->left )
	{
		BSPNode *out = node->right;
		node->right = NULL;
		return out;
	}

	if( !node->right )
	{
		BSPNode *out = node->left;
		node->left = NULL;
		return out;
	}

	return node;
}


void DreadMapGen::ComputeInitialBSPTree()
{
	// Clear nodes
	for( int i=0; i<(int)regions.nodes.size(); i++ )
		delete regions.nodes[i];
	regions.nodes.clear();

	// Build
	Polygon2D poly;
	poly.CreateRectangle(-REGION_SIZE, -REGION_SIZE, REGION_SIZE, REGION_SIZE);

	regions.bsproot = BuildBSPForPoly(poly, true);

	// Save references
	for( auto p=regions.poly_set.begin(); p!=regions.poly_set.end(); ++p )
		(*p)->bsp_subsector = (*p)->bsp_leaf;


	printf("BSP built %d nodes.\n", regions.nodes.size());
}

void DreadMapGen::ComputeFinalBSPTree(bool build_bsp)
{
	// Build
	Polygon2D poly;
	poly.CreateRectangle(-REGION_SIZE, -REGION_SIZE, REGION_SIZE, REGION_SIZE);
	ComputeFinalBSPTreeRec(regions.bsproot, poly, build_bsp);

	if( !build_bsp )
		return;

#if 0		// TBD: reenable
	// Cleanup
	regions.bsproot = CleanupBSPRec(regions.bsproot);

	// Remove deleted nodes
	int ndel = 0;
	for( int i=0; i<(int)regions.nodes.size(); i++ )
	{
		BSPNode *node = regions.nodes[i];
		if( !node->left && !node->right && !node->region )
		{
			delete node;
			regions.nodes.erase(regions.nodes.begin()+i);
			i--;
			ndel++;
		}
	}

	printf("Cleanup %d.\n", ndel);
	printf("Final BSP has %d nodes (depth %d).\n", regions.nodes.size(), CheckBSPDepth(regions.bsproot));
#endif
}

void DreadMapGen::ComputeFinalBSPTreeRec(BSPNode *node, const Polygon2D &poly, bool build_bsp)
{
	if( !node->region )
	{
		Polygon2D left, right;
		poly.SplitByLine(node->line, right, left);
		ComputeFinalBSPTreeRec(node->left, left, build_bsp);
		ComputeFinalBSPTreeRec(node->right, right, build_bsp);
		return;
	}

	static vector<RegionPoly*> regs;

	if( build_bsp )
		node->region = NULL;		// outdated by now

	// Build list of regions to be separated
	regs.clear();
	double total_surface = 0;
	for( auto p=regions.poly_set.begin(); p!=regions.poly_set.end(); ++p )
		if( (*p)->bsp_subsector==node )
		{
			regs.push_back(*p);
			(*p)->Update();
			total_surface += (*p)->surface;
		}

	if( !regs.size() )
	{
		printf("Empty BSP subregion!\n");
		return;
	}

	//Polygon2D poly;
	////poly.CreateRectangle(-REGION_SIZE, -REGION_SIZE, REGION_SIZE, REGION_SIZE);
	//MakePolySector(regs[0]->sector, poly);

	int depth = 0;
//	if( total_surface >= 128*64 ) depth = 1;
//	if( total_surface >= 256*256 ) depth = 2;
	if( build_bsp ) depth++;
	if( depth>0 ) depth--;
	BSPNode *newroot = BuildVisibilityBSP(poly, regs, depth, build_bsp);

	// Delete old regions - they all were rebuilt
	for( int j=0; j<(int)regs.size(); j++ )
		regions.ShelveRegion(regs[j]);

	if( build_bsp )
	{
		if( newroot==NULL )
		{
			printf("ERROR: BSP subtree rebuild failed.\n");
			return;
		}

		// Turn subroot into newroot
		*node = *newroot;
		if( node->region )
			node->region->bsp_leaf = node;
		else
		{
			if( !node->left ) printf("A !!!!!!!!!!!!!!!!!!!!!!!\n");
			if( !node->right ) printf("B !!!!!!!!!!!!!!!!!!!!!!!\n");
		}

		// newroot is left unused
		newroot->left = NULL;
		newroot->right = NULL;
		newroot->region = NULL;
	}
}




// Build BSP tree for regions from the list, that are overlapping the polygon.
DreadMapGen::BSPNode *DreadMapGen::BuildBSPForPoly(const Polygon2D &poly, bool cut_regions)
{
	if( poly.IsEmpty() )
	{
		printf("ERROR: Empty subregion. Can't build BSP tree! [1]\n");
		return NULL;
	}

	// Collect regions
	vector<RegionPoly*> regs;
	for( RegionPoly *reg = regions.SelectRegion(poly, cut_regions); reg; reg = reg->next )
		regs.push_back(reg);

	// Handle leaves
	if( regs.size()==1 )
	{
		BSPNode *node = new BSPNode();
		regions.nodes.push_back(node);
		node->region = regs[0];
		regs[0]->bsp_leaf = node;
		if( !node->region ) printf("!!!!!!!!!!!!!!!!!!!!!!!\n");
		return node;
	}

	// Find best split line
	vector<RegionPoly*> cur_left;
	vector<RegionPoly*> cur_right;
	RegionPoly *best_reg = NULL;
	int best_edge = 0;
	float best_score = 1e38f;
	for( int i=0; i<(int)regs.size(); i++ )
	{
		RegionPoly *reg = regs[i];
		for( int e=0; e<(int)reg->poly.edges.size(); e++ )
		{
			const LineRP2 &edge = reg->poly.edges[e].line;
			int n_split = 0;
			cur_left.clear();
			cur_right.clear();

			for( int j=0; j<(int)regs.size(); j++ )
			{
				int side = regs[j]->poly.CheckSide(edge);
				if( side<=0 ) cur_left.push_back(regs[j]);
				if( side>=0 ) cur_right.push_back(regs[j]);
				if( side==0 ) n_split++;
			}

			if( cur_left.size()<=0 || cur_right.size()<=0 )
				continue;	// whole subtree on one side - ignore this split

			//float score = log2(float(cur_left.size())) + log2(float(cur_right.size())) + n_split*2*10.f;
			float score = pow(fabs(float(cur_left.size()) - float(cur_right.size())), .5f) + n_split*2*10.f;
			if( score < best_score )
			{
				best_reg = reg;
				best_edge = e;
				best_score = score;
			}
		}
	}

	if( !best_reg )
	{
		printf("ERROR: Can't find split edge. Can't build BSP tree!\n");
		return NULL;
	}

	BSPNode *node = new BSPNode();
	regions.nodes.push_back(node);
	node->line = best_reg->poly.edges[best_edge].line;
	
	Polygon2D left, right;
	poly.SplitByLine(node->line, right, left);

	node->left = BuildBSPForPoly(left, cut_regions);
	node->right = BuildBSPForPoly(right, cut_regions);
	if( !node->left ) printf("!!!!!!!!!!!!!!!!!!!!!!!\n");
	if( !node->right ) printf("!!!!!!!!!!!!!!!!!!!!!!!\n");

	return node;
}

DreadMapGen::BSPNode *DreadMapGen::BuildVisibilityBSP(const Polygon2D &poly, const vector<RegionPoly*> &reglist, int depth, bool build_bsp)
{
	if( poly.IsEmpty() )
	{
		printf("ERROR: Empty subregion. Can't build BSP tree! [2]\n");
		for( int i=0; i<reglist.size(); i++ )
			printf("  %5.0f %5.0f\n", reglist[i]->MidPoint().x, reglist[i]->MidPoint().y);
		return NULL;
	}

	// Collect regions
	vector<RegionPoly*> regs;
	for( RegionPoly *reg = regions.SelectRegionFromList(poly, reglist, false); reg; reg = reg->next )
		regs.push_back(reg);

	// Handle leaves
	if( regs.size()==1 || depth<=0 )
		return BuildVisibilityLeaf(poly, regs, build_bsp);

	// Find best split line
	vector<RegionPoly*> best_left;
	vector<RegionPoly*> best_right;
	vector<RegionPoly*> cur_left;
	vector<RegionPoly*> cur_right;
	set<Line*> vis_left;
	set<Line*> vis_right;
	RegionPoly *best_reg = NULL;
	int best_edge = 0;
	double best_score = 0;
	int num_checks = 0;
	
	static uint8_t hash_map[0x10000];
	memset(hash_map, 0, sizeof(hash_map));

	for( int i=0; i<(int)regs.size(); i++ )
	{
		RegionPoly *reg = regs[i];
		for( int e=0; e<(int)reg->poly.edges.size(); e++ )
		{
			LineRP2 edge = reg->poly.edges[e].line;
			edge.normal.simplify();
			{
				coord_t hash = 0x12345678;
				if( edge.normal.x >= 0 )
				{
					hash += edge.normal.x * 12345;
					hash *= 54321;
					hash += edge.normal.y * 12345;
				}
				else
				{
					hash += -edge.normal.x * 12345;
					hash *= 54321;
					hash += -edge.normal.y * 12345;
				}
				hash *= 54321;
				hash += edge.normal.z * 12345;
				uint32_t h = uint32_t(hash>>(64-16-3)) & 0x7FFFF;

				if( hash_map[h>>3] & (1<<(h&3)) )
					continue;
				hash_map[h>>3] |= 1<<(h&3);
			}

			int n_split = 0;
			cur_left.clear();
			cur_right.clear();

			for( int j=0; j<(int)regs.size(); j++ )
			{
				int side = regs[j]->poly.CheckSide(edge);
				if( side<=0 ) cur_left.push_back(regs[j]);
				if( side>=0 ) cur_right.push_back(regs[j]);
				if( side==0 ) n_split++;
				num_checks++;
			}

			if( cur_left.size()<=0 || cur_right.size()<=0 )
				continue;	// whole subtree on one side - ignore this split

			// Compute visibility sets
			double surf_left = 0;
			double surf_right = 0;
			vec2 mid_left(0, 0);
			vec2 mid_right(0, 0);
			vis_left.clear();
			vis_right.clear();
			for( int j=0; j<(int)cur_left.size(); j++ )
			{
				RegionPoly *r = cur_left[j];
				for( auto p=r->vis.begin(); p!=r->vis.end(); ++p )
					vis_left.insert(p->first);
				surf_left += r->surface;
				mid_left += r->MidPoint() * float(r->surface);
			}
			for( int j=0; j<(int)cur_right.size(); j++ )
			{
				RegionPoly *r = cur_right[j];
				for( auto p=r->vis.begin(); p!=r->vis.end(); ++p )
					vis_right.insert(p->first);
				surf_right += r->surface;
				mid_right += r->MidPoint() * float(r->surface);
			}
			mid_left /= float(surf_left);
			mid_right /= float(surf_right);

			// Compute score
			double score = 0;	//log2(float(cur_left.size())) + log2(float(cur_right.size())) + n_split*2/10.f;
			for( auto p=vis_left.begin(); p!=vis_left.end(); p++ )
				if( vis_right.find(*p)==vis_right.end() )
				{
					// Split would introduce line *p to the right side
					vec2 v1 = ((*p)->v1->point_vec2() - mid_right).get_normalized();
					vec2 v2 = ((*p)->v2->point_vec2() - mid_right).get_normalized();
					score += fabs(v1.cross(v2)) * surf_right;
				}
			for( auto p=vis_right.begin(); p!=vis_right.end(); p++ )
				if( vis_left.find(*p)==vis_left.end() )
				{
					// Split would introduce line *p to the left side
					vec2 v1 = ((*p)->v1->point_vec2() - mid_left).get_normalized();
					vec2 v2 = ((*p)->v2->point_vec2() - mid_left).get_normalized();
					score += fabs(v1.cross(v2)) * surf_left;
				}
			score /= sqrt(surf_left+surf_right);

			if( score > best_score )
			{
				best_reg = reg;
				best_edge = e;
				best_score = score;
				best_left.swap(cur_left);
				best_right.swap(cur_right);
			}
		}
	}

	//printf("%d ", num_checks);

	if( !best_reg || best_score < 128*(build_bsp ? 1.25f : 0.25f) )		// Split factor
	{
		//if( !best_reg )
		//	printf("WARNING: Can't split visibility region.\n");
		return BuildVisibilityLeaf(poly, reglist, build_bsp);
	}

	Polygon2D left, right;
	poly.SplitByLine(best_reg->poly.edges[best_edge].line, right, left);

	if( build_bsp )
	{
		BSPNode *node = new BSPNode();
		regions.nodes.push_back(node);
		node->line = best_reg->poly.edges[best_edge].line;

		node->left = BuildVisibilityBSP(left, best_left, depth-1, build_bsp);
		node->right = BuildVisibilityBSP(right, best_right, depth-1, build_bsp);
		return node;
	}
	else
	{
		BuildVisibilityBSP(left, best_left, depth-1, build_bsp);
		BuildVisibilityBSP(right, best_right, depth-1, build_bsp);
	}

	return NULL;
}

DreadMapGen::BSPNode *DreadMapGen::BuildVisibilityLeaf(const Polygon2D &poly, const vector<RegionPoly*> &reglist, bool build_bsp)
{
	if( poly.IsEmpty() )
	{
		printf("ERROR: Empty subregion. Can't build BSP leaf! [3]\n");
		return NULL;
	}

	if( reglist.size()<=0 )
	{
		printf("ERROR: Empty region list. Can't build BSP leaf!\n");
		return NULL;
	}

	BSPNode *leaf = NULL;
	if( build_bsp )
	{
		leaf = new BSPNode();
		regions.nodes.push_back(leaf);
	}

	RegionPoly *newreg = regions.NewRegion();
	newreg->sector = reglist[0]->sector;
	
	Polygon2D secpoly;
	MakePolySector(newreg->sector, secpoly);
	secpoly.ComputeIntersection(poly, newreg->poly);
	
	newreg->bsp_subsector = reglist[0]->bsp_subsector;
	if( build_bsp )
		newreg->bsp_leaf = leaf;
	else
	{
		newreg->bsp_subsector->region->subregions.push_back(newreg);	// new region must be accessible for next passes
	}

	for( int i=0; i<(int)reglist.size(); i++ )
	{
		RegionPoly *reg = reglist[i];

		// VisLines
		for( auto p=reg->vis.begin(); p!=reg->vis.end(); ++p )
		{
			if( !newreg->vis[p->first] )
				newreg->vis[p->first] = new VisLine(*p->second);
			else
			{
				VisLine *newvis = newreg->vis[p->first];

				// Merge VisLines:	newvis = newvis + p->second
				newvis->IncludeVisInfo(p->second, this);
			}
		}

		// VisSectors
		for( auto p=reg->svis.begin(); p!=reg->svis.end(); ++p )
		{
			if( !newreg->svis[p->first] )
				newreg->svis[p->first] = new VisSector(*p->second);
			else
			{
				VisSector *newvis = newreg->svis[p->first];

				// Merge VisSectors:	newvis = newvis + p->second
				newvis->IncludeVisInfo(p->second, this);
			}
		}
	}

	newreg->Update();

	if( build_bsp )
		leaf->region = newreg;
	return leaf;
}

int DreadMapGen::CheckBSPDepth(BSPNode *node)
{
	if( !node ) return 0;
	int a = CheckBSPDepth(node->left) + 1;
	int b = CheckBSPDepth(node->right) + 1;
	return a>b ? a : b;
}



void DreadMapGen::ComputeCollisionMesh(PolygonMesh2D &mesh, int offset, collision_mode_t col_mode)
{
	mag_AB = 0;
	mag_C = 0;

	Polygon2D newpoly;
	vector<PointRP2> pvtx;
	newpoly._user = this;	// any non-NULL is solid

	// Add free space to target mesh
	Polygon2D init;
	init.CreateRectangle(-REGION_SIZE, -REGION_SIZE, REGION_SIZE, REGION_SIZE);
	init._user = this;	// any non-NULL is solid


	mesh.Clear();
	//mesh.polys.push_back(init);
	for( int i=0; i<(int)sectors.size(); i++ )
	{
		Polygon2D poly;
		MakePolySector(sectors[i], poly);
		mesh.polys.push_back(poly);
		//mesh.CSGAdd(poly);
	}

	// Gather vertex->line mapping
	for( int i=0; i<(int)verts.size(); i++ )
	{
		verts[i]->_line_in.clear();
		verts[i]->_line_out.clear();
		verts[i]->_has_offpos = false;
	}

	for( int i=0; i<(int)lines.size(); i++ )
	{
		Line *line = lines[i];
		if( !line->IsCollision(col_mode) )
			continue;

		line->v1->_line_out.push_back(line);
		line->v2->_line_in.push_back(line);
	}

	// Process vertexes
	for( int i=0; i<(int)verts.size(); i++ )
	{
		Vertex *v = verts[i];
		if( v->_line_in.size()==1 && v->_line_out.size()==1 )
		{
			vec2 v0 = v->_line_in[0]->v1->point_vec2();
			vec2 v1 = v->point_vec2();
			vec2 v2 = v->_line_out[0]->v2->point_vec2();
			vec2 n0 = (v1-v0).get_rotated90().get_normalized();	// Y, -X		- make normals point outward (left side)
			vec2 n2 = (v2-v1).get_rotated90().get_normalized();
			v0 = v1 + n0*(offset+0.5f);
			v2 = v1 + n2*(offset+0.5f);
			vec2 v1b = (n0.dot(n2) < 0.99f) ? _line_intersection(v0, n0, v2, n2) : (v0+v2)*0.5f;
			float miss = (v1b-v1).length();

			if( offset > 0 && map_col_round_corners && miss >= offset*1.3f )
			{
				vec2 n1 = (n0+n2).get_normalized();
				v1b = v1 + n1*(offset+0.5f);
				v0 = _line_intersection(v0, n0, v1b, n1);
				v2 = _line_intersection(v2, n2, v1b, n1);

				v->_offpos_in = v0;
				v->_offpos_out = v2;

				pvtx.clear();
				vec2 vv;
				vv = v0;			pvtx.push_back(PointRP2((int)floor(vv.x+0.5f), (int)floor(vv.y+0.5f)));
				vv = v1;			pvtx.push_back(PointRP2((int)floor(vv.x+0.5f), (int)floor(vv.y+0.5f)));
				vv = v2;			pvtx.push_back(PointRP2((int)floor(vv.x+0.5f), (int)floor(vv.y+0.5f)));
				newpoly.CreateFromPoints(&pvtx[0], (int)pvtx.size());
				mesh.CSGSubtract(newpoly);
			}
			else
				v->_offpos_in = v->_offpos_out = v1b;
			
			v->_has_offpos = true;
		}
	}
	
	// Subtract blocking lines
	for( int i=0; i<(int)lines.size(); i++ )
	{
		Line *line = lines[i];
		if( !line->IsCollision(col_mode) )
			continue;

		pvtx.clear();

		vec2 v1 = line->v1->point_vec2();
		vec2 v2 = line->v2->point_vec2();
		vec2 dir = v2 - v1;
		vec2 offs = dir.get_rotated90().get_normalized()*(offset+0.5f);
		dir.normalize();

		vec2 vv;
		if( line->v1->_has_offpos )
		{
			vv = line->v1->_offpos_out;		pvtx.push_back(PointRP2((int)floor(vv.x+0.5f), (int)floor(vv.y+0.5f)));
			vv = v1;						pvtx.push_back(PointRP2((int)floor(vv.x+0.5f), (int)floor(vv.y+0.5f)));
		}
		else
		{
			vv = v1-dir+offs;				pvtx.push_back(PointRP2((int)floor(vv.x+0.5f), (int)floor(vv.y+0.5f)));
			vv = v1-dir;					pvtx.push_back(PointRP2((int)floor(vv.x+0.5f), (int)floor(vv.y+0.5f)));
		}
		if( line->v2->_has_offpos )
		{
			vv = v2;						pvtx.push_back(PointRP2((int)floor(vv.x+0.5f), (int)floor(vv.y+0.5f)));
			vv = line->v2->_offpos_in;		pvtx.push_back(PointRP2((int)floor(vv.x+0.5f), (int)floor(vv.y+0.5f)));
		}
		else
		{
			vv = v2+dir;					pvtx.push_back(PointRP2((int)floor(vv.x+0.5f), (int)floor(vv.y+0.5f)));
			vv = v2+dir+offs;				pvtx.push_back(PointRP2((int)floor(vv.x+0.5f), (int)floor(vv.y+0.5f)));
		}

		newpoly.CreateFromPoints(&pvtx[0], (int)pvtx.size());

		mesh.CSGSubtract(newpoly);
	}

	// Gather collision edges
	col_edges.clear();
	for( int i=0; i<(int)mesh.polys.size(); i++ )
	{
		Polygon2D &poly = mesh.polys[i];
		int N = (int)poly.edges.size();
		for( int j=0; j<N; j++ )
		{
			ColEdge e;
			e.p1 = poly.edges[j].start;
			e.p2 = poly.edges[(j+1)%N].start;
			e.line = poly.edges[j].line;
			e.limit1 = poly.edges[(j+N-1)%N].line;
			e.limit2 = poly.edges[(j+1)%N].line;
			col_edges.push_back(e);
		}
	}

	// Remove doubled edge fragments
	GlueCollisionEdges(false);
	GlueCollisionEdges(true);

	// Build BSP tree
	collision_bsp_mesh.Clear();

	vector<ColEdge*> elist;
	for( int i=0; i<(int)col_edges.size(); i++ )
		elist.push_back(&col_edges[i]);

	ColBSPNode * root = BuildBSPForColEdges(init, elist, true);
	*(offset>0 ? &col_bsp_root_psize : &col_bsp_root_0) = root;
	printf("%d collision BSP nodes for %s mesh (depth %d)\n", col_nodes.size(), offset ? "offset" : "original", CheckBSPDepth(root));
	printf("  - magnitude AB: %20lld\n", mag_AB);
	printf("  - magnitude C:  %20lld\n", mag_C);

	//col_bsp_root = new ColBSPNode();
	//col_bsp_root->A = -96;
	//col_bsp_root->B = -63;
	//col_bsp_root->C = 215280;
	//col_bsp_root->left = new ColBSPNode();
	//col_bsp_root->left->is_leaf = true;
	//col_bsp_root->left->is_solid = true;
	//col_bsp_root->right = new ColBSPNode();
	//col_bsp_root->right->is_leaf = true;
	//col_bsp_root->right->is_solid = false;
}


void DreadMapGen::GlueCollisionEdges(bool allow_merge)
{
	bool work = true;
	while( work )
	{
		work = false;

		for( int i=0; i<(int)col_edges.size(); i++ )
		{
			ColEdge *e = &col_edges[i];
			LineRP2 line = e->line;

			for( int j=i+1; j<(int)col_edges.size(); j++ )
			{
				ColEdge *f = &col_edges[j];
				if( line.CheckSide(f->p1)!=0 || line.CheckSide(f->p2)!=0 )
					continue;	// not colinear

				coord_t a1 = e->limit1.CheckSide(f->p1);
				coord_t a2 = e->limit1.CheckSide(f->p2);
				coord_t b1 = e->limit2.CheckSide(f->p1);
				coord_t b2 = e->limit2.CheckSide(f->p2);

				if( line.normal.x*f->line.normal.x + line.normal.y*f->line.normal.y >= 0 )
				{
					// same facing
					if( !allow_merge )
						continue;

					if( a2==0 )
					{
						// extend 1, remove F
						e->p1 = f->p1;
						e->limit1 = f->limit1;
						col_edges[j--] = col_edges.back();
						col_edges.pop_back();
						work = true;
					}
					else if( b1==0 )
					{
						// extend 2, remove F
						e->p2 = f->p2;
						e->limit2 = f->limit2;
						col_edges[j--] = col_edges.back();
						col_edges.pop_back();
						work = true;
					}
					continue;	
				}

				if( a1<=0 ) continue;		// the lines have opposite directions
				if( b2<=0 ) continue;

				work = true;

				if( a2<0 )
				{
					if( b1>0 )
					{
						// Swap 1's
						swap(e->p1, f->p1);
						swap(e->limit1, f->limit1);
						e->limit1.Flip();
						f->limit1.Flip();
					}
					else if( b1==0 )
					{
						// Swap 1's, remove E
						f->p1 = e->p1;
						f->limit1 = e->limit1.Reversed();
						col_edges[i--] = col_edges.back();
						col_edges.pop_back();
						break;
					}
					else
					{
						// Swap 1's, flip E, restart check loop
						swap(e->p1, f->p1);
						swap(e->limit1, f->limit1);
						f->limit1.Flip();
						e->line.Flip();
						e->limit2.Flip();
						j = i;	// +1 -1
					}
				}
				else if( a2==0 )
				{
					if( b1>0 )
					{
						// Swap 1's, remove F
						e->p1 = f->p1;
						e->limit1 = f->limit1.Reversed();
						col_edges[j--] = col_edges.back();
						col_edges.pop_back();
					}
					else if( b1==0 )
					{
						// Remove both
						col_edges[j] = col_edges.back();		// j>i, so goes first
						col_edges.pop_back();
						col_edges[i--] = col_edges.back();
						col_edges.pop_back();
						break;
					}
					else
					{
						// Swap 2's, remove E
						f->p2 = e->p2;
						f->limit2 = e->limit2.Reversed();
						col_edges[i--] = col_edges.back();
						col_edges.pop_back();
						break;
					}
				}
				else
				{
					if( b1>0 )
					{
						// Swap 1's, flip F
						swap(e->p1, f->p1);
						swap(e->limit1, f->limit1);
						e->limit1.Flip();
						f->line.Flip();
						f->limit2.Flip();
					}
					else if( b1==0 )
					{
						// Swap 2's, remove F
						e->p2 = f->p2;
						e->limit2 = f->limit2.Reversed();
						col_edges[j--] = col_edges.back();
						col_edges.pop_back();
					}
					else
					{
						// Swap 2's
						swap(e->p2, f->p2);
						swap(e->limit2, f->limit2);
						e->limit2.Flip();
						f->limit2.Flip();
					}
				}
			}
			// no more code here - edge might have been removed
		}
	}
}

DreadMapGen::ColBSPNode *DreadMapGen::BuildBSPForColEdges(const Polygon2D &poly, const vector<ColEdge*> &edges, bool is_right_side)
{
	if( edges.size()<=0 )
	{
		// Leaf node
		ColBSPNode *bsp = new ColBSPNode();
		col_nodes.push_back(bsp);
		bsp->is_leaf = true;
		bsp->is_solid = !is_right_side;
		bsp->poly = poly;
		bsp->poly._user = bsp->is_solid ? this : NULL;
		collision_bsp_mesh.polys.push_back(bsp->poly);
		return bsp;
	}

	// Find best split
	ColEdge *best_split = NULL;
	float best_score = 1e38;
	for( int i=0; i<(int)edges.size(); i++ )
	{
		int n_left=0, n_right=0, n_split=0, n_online=0;
		LineRP2 split = edges[i]->line;

		for( int j=0; j<(int)edges.size(); j++ )
		{
			coord_t s1 = split.CheckSide(edges[j]->p1);
			coord_t s2 = split.CheckSide(edges[j]->p2);
			if( s1==0 && s2==0 ) n_online++;
			else if( s1>=0 && s2>=0 ) n_right++;
			else if( s1<=0 && s2<=0 ) n_left++;
			else n_split++;
		}

		int n_diff = n_right-n_left;
		if( n_diff<0 )
			n_diff = -n_diff;

		float score = n_right + n_left + n_split*2 + n_diff;
		if( score < best_score )
		{
			best_split = edges[i];
			best_score = score;
		}
	}

	// Split
	vector<ColEdge*> e_right;
	vector<ColEdge*> e_left;
	vector<ColEdge*> extras;
	LineRP2 split = best_split->line;
	for( int i=0; i<(int)edges.size(); i++ )
	{
		coord_t s1 = split.CheckSide(edges[i]->p1);
		coord_t s2 = split.CheckSide(edges[i]->p2);
		if( s1==0 && s2==0 ) continue;	// on edge
		
		if( s1>=0 && s2>=0 )
			e_right.push_back(edges[i]);
		else if( s1<=0 && s2<=0 )
			e_left.push_back(edges[i]);
		else
		{
			// Split
			ColEdge *er = new ColEdge(*edges[i]);
			ColEdge *el = new ColEdge(*edges[i]);
			extras.push_back(er);
			extras.push_back(el);
			if( s1<0 )
				el->p2 = er->p1 = split.Intersect(er->line);		// don't care about limit1/2
			else
				el->p1 = er->p2 = split.Intersect(er->line);
			e_right.push_back(er);
			e_left.push_back(el);
		}
	}


	ColBSPNode *bsp = new ColBSPNode();
	col_nodes.push_back(bsp);
	bsp->is_leaf = false;
	bsp->line = split;
//	bsp->line.normal.simplify();
	bsp->A = -bsp->line.normal.x;
	bsp->B = -bsp->line.normal.y;
	bsp->C = (-bsp->line.normal.z) << ENGINE_SUBVERTEX_PRECISION;	// input coords are fixed point

	if( bsp->A >  mag_AB )	mag_AB =  bsp->A;
	if( bsp->A < -mag_AB )	mag_AB = -bsp->A;
	if( bsp->B >  mag_AB )	mag_AB =  bsp->B;
	if( bsp->B < -mag_AB )	mag_AB = -bsp->B;
	if( bsp->C >  mag_C  )	mag_C  =  bsp->C;
	if( bsp->C < -mag_C  )	mag_C  = -bsp->C;

	Polygon2D p_right, p_left;
	poly.SplitByLine(bsp->line, p_right, p_left);

	bsp->right = BuildBSPForColEdges(p_right, e_right, true);
	bsp->left = BuildBSPForColEdges(p_left, e_left, false);

	for( int i=0; i<(int)extras.size(); i++ )
		delete extras[i];

	return bsp;
}

int DreadMapGen::CheckBSPDepth(ColBSPNode *node)
{
	if( !node ) return 0;
	int a = CheckBSPDepth(node->left) + 1;
	int b = CheckBSPDepth(node->right) + 1;
	return a>b ? a : b;
}


string DreadMapGen::GetConditionName(int cid, bool mark_op, bool and_level)
{
	if( cid==-1 ) return "TRUE";
	if( cid<0 || cid>=(int)vis_conditions.size() )
		return "???";
	
	VisCondition &vc = vis_conditions[cid];
	if( vc.op == VisCondition::OP_MACHINE )
		return format("%c", 'A'+vc.c1);

	if( vc.op==VisCondition::OP_AND )
		return GetConditionName(vc.c1, false, true) + (mark_op ? "&" : "") + GetConditionName(vc.c2, false, true);

	// vc.op==VisCondition::OP_OR
	return (and_level ? "(" : "") + GetConditionName(vc.c1, false, false) + (mark_op ? "|" : "+") + GetConditionName(vc.c2, false, false) + (and_level ? ")" : "");
}





// Gather
int DreadMapGen::ExportVertex(Vertex *v, ExportState &exp)
{
	if( v->_number >= 0 )
		return v->_number;

	v->_number = (int)exp.verts.size();
	exp.verts.push_back(v);
	return v->_number;
}

// Gather
//	request_mode == -1	-> don't assign mode nor line number
int DreadMapGen::ExportLine(Line *line, ExportState &exp, int request_mode, bool first_draw)
{
	if( line->_number < 0 )
	{
		line->_number = (int)exp.lines.size();
		exp.lines.push_back(line);

		ExportVertex(line->v1, exp);
		ExportVertex(line->v2, exp);
	}

	if( request_mode < 0 )
		return 0;
		
	//TBD: signal error for non-height maps

	for( int i=0; i<(int)line->_modes.size(); i++ )
		if( line->_modes[i]==request_mode )
			return (line->_number << VIS_PACKED_SHIFT) | (i & VIS_PACKED_MASK);
	
	int mode = first_draw ? 1 : 0;
	line->_modes.resize(2, -1);
	if( line->_modes[mode]<0 || line->_modes[mode]==request_mode )
		line->_modes[mode] = request_mode;
	else
	{
		int mode = (int)line->_modes.size();
		line->_modes.push_back(request_mode);
	}

	if( mode > VIS_PACKED_MASK )
	{
		static const char *NAMES_LC[] ={
#include "gen/cheatnames.inc"
		};
		static const char *NAMES_LC2[] ={
#include "gen/render2_cheatnames.inc"
		};

		const char **LC = (export_engine_version == 2) ? NAMES_LC2 : NAMES_LC;
		int num_modes = (export_engine_version == 2) ? sizeof(NAMES_LC2)/sizeof(NAMES_LC2[0]) : sizeof(NAMES_LC)/sizeof(NAMES_LC[0]);


		printf("ERROR: Line mode table limit exceeded. (line midpoint %d, %d)\n", (line->v1->xp+line->v2->xp)/2, (line->v1->yp+line->v2->yp)/2);
		for( int i=0; i<line->_modes.size(); i++ )
			printf("    %3d: %s\n",
				line->_modes[i],
				uint32_t(line->_modes[i]) < num_modes ? LC[line->_modes[i]] : "???"
			);
	}
	return (line->_number << VIS_PACKED_SHIFT) | (mode & VIS_PACKED_MASK);
}

// Gather pass 1
void DreadMapGen::ExportRegionGather(RegionPoly *reg, ExportState &exp)
{
	if( reg->_number >= 0 )
		return;

	reg->_number = (int)exp.subsectors.size();
	exp.subsectors.push_back(reg);

//	printf("Region %p -> #%d\n", reg, reg->_number);
}

// Gather pass 2
void DreadMapGen::ExportRegionProcess(RegionPoly *reg, ExportState &exp, bool full_export)
{
	if( !full_export )
	{
		for( auto p=reg->vis.begin(); p!=reg->vis.end(); ++p )
		{
			VisLine *vl = p->second;
			ExportLine(vl->line, exp, vl->cheat_mode, vl->first_draw);
		}
		return;
	}

	if( reg->_vis_start >= 0 )
		return;

	// Visibility
	while(exp.vis.size() % export_vis_align != 0)
		exp.vis.push_back(ExportVis(EVIS_GP_END, 0));

	reg->_vis_start = (int)exp.vis.size();

	if( export_vis_groups )
	{
		// Export using visibility groups
		int conmode = 1;
		int vislines = 0;
		for( int i=0; i<(int)reg->grouplist.size(); i++ )
		{
			VisGroupReference &vg = reg->grouplist[i];
			int vg_conmode = (vg.condition+1)*2 + (vg.first_draw ? 1 : 0);

			//if( pass == 1 )
			//{
			//	if( vg.condition!=-1 )
			//	{
			//		vec2 regpos = reg->MidPoint();
			//		string err = format("First draw with condition %d! (region %.0f, %.0f)", vg.condition, regpos.x, regpos.y);
			//		if( !vg.group )
			//			printf("ERROR (no group!): %s\n", err.c_str());
			//		else if( vg.group->vis.size()<0 )
			//			printf("ERROR (group empty!): %s\n", err.c_str());
			//		else
			//		{
			//			Line *line = NULL;
			//			for( int j=0; j<vg.group->vis.size(); j++ )
			//				if( vg.group->vis[j].line )
			//				{
			//					line = vg.group->vis[j].line;
			//					break;
			//				}
			//			if( !line )
			//				printf("ERROR (no lines in group of %d elements): %s\n", vg.group->vis.size(), err.c_str());
			//			else
			//				line->Error(err);
			//		}
			//	}
			//}
			//else


			if( vg_conmode!=conmode )
			{
				exp.vis.push_back(ExportVis(EVIS_GP_REGION_CONMODE, vg_conmode));
				conmode = vg_conmode;
			}

			if( vg.group->_base_offset >= 0 )
			{
				exp.vis.push_back(ExportVis(EVIS_GP_GROUP_CALL, vg.group->_base_offset + vg.stop_index));
				//printf("[conmode %3d] Export group call region #%d -> group at %d, stop index %d/%d\n", conmode, reg->_number, vg.group->_base_offset, vg.stop_index, vg.group->vis.size());
			}
			else
			{
				// inline group
				ExportVisGroup(vg.group, exp, true, vg.stop_index);
			}

			vislines += vg.group->visline_count;
		}

		exp.vis.push_back(ExportVis(EVIS_GP_END, 0));

		if( vislines > exp.max_vis_lines )
			exp.max_vis_lines = vislines;
	}
	else
	{
		// Old direct list export
		int conmode = -1;
		int vislines = 0;
		for( int i=0; i<(int)reg->drawlist.size(); i++ )
		{
			VisEntry &ve = reg->drawlist[i];

			if( ve.condition!=conmode )
			{
				exp.vis.push_back(ExportVis(EVIS_OLD_CONDITION, ve.condition));
				conmode = ve.condition;
			}

			if( ve.visline )
			{
				Line *line = ve.visline->line;
				int vismode = ExportLine(line, exp, ve.visline->cheat_mode, ve.visline->first_draw);

				exp.vis.push_back(ExportVis(EVIS_OLD_LINEMODE, vismode));

				if( line->tex_upper.is_set() ) exp.tex_set.insert(line->tex_upper);
				if( line->tex_lower.is_set() ) exp.tex_set.insert(line->tex_lower);

				vislines++;
			}

			if( ve.vissector )
			{
				// Thing (subsector) visibility - include all subsectors
				Sector *sec = ve.vissector->sector;
				for( int j=0; j<sec->regions.size(); j++ )
					exp.vis.push_back(ExportVis(EVIS_OLD_SUBSECTOR, sec->regions[j]->_number));
			}
		}

		exp.vis.push_back(ExportVis(EVIS_OLD_END, 0));

		if( vislines > exp.max_vis_lines )
			exp.max_vis_lines = vislines;
	}

	//	// Fold visibility
	//	int pat_start = reg->_vis_start;
	//	int pat_len = exp.vis.size() - reg->_vis_start;
	//	for(int i = 0; i < pat_start - pat_len; i++)
	//	{
	//		bool found = true;
	//		for(int j=0; j<pat_len; j++)
	//			if(exp.vis[i+j] != exp.vis[pat_start+j])
	//			{
	//				found = false;
	//				break;
	//			}
	//		if(found)
	//		{
	//			reg->_vis_start = i;
	//			exp.vis.resize(pat_start);
	//			break;
	//		}
	//	}
}

void DreadMapGen::ExportVisGroup(VisGroup *vis_group, ExportState &exp, bool region_inline, int stop_entry)
{
	if( !region_inline )
	{
		int entries_grouped = vis_group->max_users*2 + (1 + vis_group->entry_count);
		int entries_ungrouped = vis_group->instances_used;
		if( entries_grouped >= entries_ungrouped )
			return;

		exp.vis.push_back(ExportVis(EVIS_GP_GROUP_START, 0));
		vis_group->_base_offset = (int)exp.vis.size();

		//printf("  - group %4d entries (%5.1f avg. users)\n", vis_group->vis.size(), vis_group->instances_used*1.f/vis_group->vis.size());
	}


	for( int i=0; i<(int)vis_group->vis.size() && i<stop_entry; i++ )
	{
		VisGroupEntry &ve = vis_group->vis[i];
		if( ve.line )
		{
			exp.vis.push_back(ExportVis(EVIS_GP_LINE, ve.line->_number));

			if( ve.line->tex_upper.is_set() ) exp.tex_set.insert(ve.line->tex_upper);
			if( ve.line->tex_lower.is_set() ) exp.tex_set.insert(ve.line->tex_lower);
		}

		if( ve.sector )
		{
			// Thing (subsector) visibility - include all subsectors
			for( int j=0; j<ve.sector->regions.size(); j++ )
				exp.vis.push_back(ExportVis(EVIS_GP_SUBSECTOR, ve.sector->regions[j]->_number));
		}
	}
}

// Gather
void DreadMapGen::ExportBSPRec(BSPNode *node, ExportState &exp)
{
	if( !node ) return;
	
	if( node->region == NULL )
	{
		// Export node
		node->_number = (int)exp.nodes.size();
		exp.nodes.push_back(node);

		ExportBSPRec(node->left, exp);
		ExportBSPRec(node->right, exp);
		return;
	}

	// Export region
	RegionPoly *reg = node->region;
	ExportRegionGather(reg, exp);

	node->_number = ~reg->_number;
}

// Gather
void DreadMapGen::ExportColBSPRec(ColBSPNode *node, ExportState &exp)
{
	if( !node ) return;

	if( node->is_leaf )
	{
		node->_number = ~(node->is_solid ? 1 : 0);
		return;
	}

	// Export node
	node->_number = ((int)exp.colnodes.size() + (int)exp.nodes.size())*12;
	exp.colnodes.push_back(node);

	ExportColBSPRec(node->left, exp);
	ExportColBSPRec(node->right, exp);
}


void DreadMapGen::ExportGather(vector<texture_id_t> &tex_list)
{
	exp.Clear();
	tex_list.clear();

	for( int i=0; i<(int)verts.size(); i++ )
		verts[i]->_number = -1;

	for( int i=0; i<(int)lines.size(); i++ )
	{
		lines[i]->_number = -1;
		lines[i]->_modes.clear();
	}

	for( int i=0; i<(int)regions.nodes.size(); i++ )
		regions.nodes[i]->_number = -1;

	for( auto p=regions.poly_set.begin(); p!=regions.poly_set.end(); ++p )
	{
		(*p)->_number = -1;
		(*p)->_vis_start = -1;
	}

	// Gather machines first	(assure sequential numbering of machine elements)
	for( int i=0; i<(int)machines.size(); i++ )
	{
		Machine &m = machines[i];
		if( m.is_door )
		{
			ExportVertex(m.line0->v1, exp);		// v+0:	first moving vert
			ExportVertex(m.line0->v2, exp);		// v+1: second moving vert
			ExportVertex(m.line1->v1, exp);		// v+2: first min pos
			ExportVertex(m.line2->v2, exp);		// v+3: second min pos
			ExportVertex(m.line_x1->v2, exp);	// v+4: first max pos
			ExportVertex(m.line_x2->v1, exp);	// v+5: second max pos

			// export lines - after vertexes
			ExportLine(m.line0, exp, -1, false);	// edge
			ExportLine(m.line1, exp, -1, false);	// front
			ExportLine(m.line2, exp, -1, false);	// back
		}
		else
		{
			ExportLine(m.line0, exp, -1, false);	// edge
		}
	}

	// Gather
	ExportBSPRec(regions.bsproot, exp);

	if( export_vis_groups )
	{
		for( int i=0; i<(int)exp.subsectors.size(); i++ )
			ExportRegionProcess(exp.subsectors[i], exp, false);

		for( int i=0; i<vis_groups.size(); i++ )
			ExportVisGroup(vis_groups[i], exp, false, 1000000000);
	}

	for( int i=0; i<(int)exp.subsectors.size(); i++ )
		ExportRegionProcess(exp.subsectors[i], exp, true);
	
	for( auto p=exp.tex_set.begin(); p!=exp.tex_set.end(); ++p )
	{
		printf("Texture: %d (%s)\n", p->_internal_index, gfx_converter.GetTextureNameEx(*p).c_str());
		tex_list.push_back(*p);
	}

	ExportColBSPRec(col_bsp_root_psize, exp);
	ExportColBSPRec(col_bsp_root_0, exp);
}




void DreadMapGen::ExportTextures(FILE *fp, vector<texture_id_t> &tex_list, map<texture_id_t, int> &tex_offsets, ChunkyColorPacker &chunky_packer)
{
	// tex_buffer is a virtual texture <width>x128, column order
	vector<byte> &tex_buffer = exp.tex_buffer;
	vector<DWORD> colbuffer;
	vector<int> lane_fill;		// already used vertical size in pixels (one entry = 64px width)
	map<GfxTexture*, bool> tex_done;

	tex_buffer.clear();
	tex_offsets.clear();

	// Texture offset defines
	for( int t=0; t<(int)tex_list.size(); t++ )
	{
		if( !tex_list[t] )
			continue;

		GfxTexture *gtex = gfx_converter.FindTexture(tex_list[t]);
		if( !gtex ) printf("Texture missing for %d\n", tex_list[t]._internal_index);
		if( !gtex || tex_done[gtex] ) continue;
		tex_done[gtex] = true;

		DataTexture &tex = gtex->texture;
		int th = tex.height;

		// Allocate space
		int xoff = -1;
		int yoff = 0;
		for( int i=0; i<lane_fill.size(); i++ )
			if( lane_fill[i]+th <= 128 )
			{
				xoff = i*64;
				yoff = lane_fill[i];
				lane_fill[i] += th;
				break;
			}
		if( xoff<0 )
		{
			xoff = lane_fill.size()*64;
			yoff = 0;
			lane_fill.push_back(th);
			tex_buffer.resize(lane_fill.size()*64*128);
			colbuffer.resize(tex_buffer.size());
		}

		int offset = xoff*128 + yoff;
		tex_offsets[tex_list[t]] = offset;

		//if( fp )
		//{
		//	fprintf(fp, "#define %-32s\t%d", gtex->GetOffsetName().c_str(), offset);
		//	fprintf(fp, "\t// %s  %3dx%-3d (%3d)  [ %s ]\n", gtex->name.c_str(), tex.width, th, gtex->light_level, gtex->name.c_str());
		//}

		for( int y=0; y<th; y++ )
		{
			for( int x=0; x<tex.width; x++ )
			{
				tex_buffer[offset + x*128 + y] = chunky_packer.Process(tex.data[x+y*tex.width]);
				colbuffer[offset + x*128 + y] = tex.data[x+y*tex.width];
			}
		}
	}

	// Texture data
	if( fp )
	{
		fprintf(fp, "\nconst byte TEXTURE_DATA[] = {\n");
		for( int i=0; i<tex_buffer.size(); i += 64 )
		{
			fprintf(fp, "\t");
			for( int j=0; j<64; j++ )
				fprintf(fp, "0x%02X,", tex_buffer[i+j]);
			fprintf(fp, "\n");
		}
		fprintf(fp, "};\n");
	}


	printf("Export: %d texture bytes.\n", tex_buffer.size());


	// Dump texture buffer
	if( gfx_converter.main_palette )
	{
		vector<DWORD> texdata(colbuffer.size()*4);
		int w = colbuffer.size()/128;
		for( int y=0; y<128; y++ )
			for( int x=0; x<w; x++ )
			{
				DWORD c0 = gfx_converter.main_palette->GetColor(colbuffer[x*128+y]&0xFF).color_out;
				DWORD c1 = gfx_converter.main_palette->GetColor((colbuffer[x*128+y]>>8)&0xFF).color_out;
				texdata[(x*2+0)+(y*2+0)*w*2] = c0;
				texdata[(x*2+0)+(y*2+1)*w*2] = c0;
				texdata[(x*2+1)+(y*2+0)*w*2] = c1;
				texdata[(x*2+1)+(y*2+1)*w*2] = c1;
			}
		DevTexture tex;
		tex.LoadRaw2D(D3DFMT_X8R8G8B8, w*2, 128*2, &texdata[0], false);

		string export_path = proj_cfg.GetString("export-directory", "export") + "/export_tex_atlas.png";
		D3DXSaveTextureToFile(export_path.c_str(), D3DXIFF_PNG, tex, NULL);
	}
}

void DreadMapGen::ExportWrite(FILE *fp, FILE *asm_fp, DataBlockSet &bset, ScriptCompiler &scomp, ChunkyColorPacker &chunky_packer, map<texture_id_t, int> &tex_offsets)
{
	if( !fp || !asm_fp )
	{
		// estimate colors only
		for( int i=0; i<(int)exp.lines.size(); i++ )
		{
			Line &l = *exp.lines[i];
			chunky_packer.Process(gfx_converter.GetTextureColor(l.tex_ceil));
			chunky_packer.Process(gfx_converter.GetTextureColor(l.tex_floor));
		}
			
		return;
	}

	// Example file:
	//
	//	   $00 HEADER                8     16
    //	   $10 PALETTE             352    360
    //	  $178 TEXTURES          57344  22710  40%
    //	 $5A2E VERTEXES           9152   2664  58%
    //	 $6496 LINES             52000  10372  25%
    //	 $8D1A BSPNODES          33900  33908
    //	$1118E VISLIST           85474  26846  31% ERROR: out of memory (32116 bytes)
    //	$17A6C SUBSECTORS         7266   1390  44%
    //	$17FDA THINGS             1240   1248
    //	$184BA LOCATIONS            54     62
    //	$184F8 MACHINES            162    170
    //	$185A2 CONDITIONS          426    434


	//	HEADER                8
	//	VISLIST           85474		moved
	//	TEXTURES          57344
	//	VERTEXES           9152
	//	LINES             52000
	//	BSPNODES          33900
	//	SUBSECTORS         7266
	//	THINGS             1240
	//	PALETTE             352		moved
	//	LOCATIONS            54
	//	MACHINES            162
	//	CONDITIONS          426


	// ================================================================ Write header ================================================================
	{
		DataBlock *block = bset.NewBlock("HEADER");
		block->ConfigSetupBlock(MAPBLOCK_ID_HEADER, 8, 1);

		block->AddDWord(col_bsp_root_psize ? col_bsp_root_psize->_number : 0);
		block->AddComment("MAP_COLLISION_BSP_ROOT");
		block->AddLineBreak();

		block->AddDWord(col_bsp_root_0 ? col_bsp_root_0->_number : 0);
		block->AddComment("MAP_HITSCAN_BSP_ROOT");
		block->AddLineBreak();
	}

	// ================================================================ Write test ================================================================
	//	{
	//		DataBlock *block = bset.NewBlock("TEXT");
	//		block->ConfigRawBlock(MAPBLOCK_ID_TEXT);
	//	
	//		block->AddString("HELLO DEFLATE WORLD!");
	//		for( int i=0; i<10; i++ )
	//			block->AddWord(0);
	//		//block->AddWord(0x1234);
	//		//block->AddWord(0x5678);
	//	
	//		if(export_deflate) block->Deflate();
	//	}


	// ================================================================ Write textures ================================================================
	{
		DataBlock *block = bset.NewBlock("TEXTURES");
		block->ConfigRawBlock(MAPBLOCK_ID_TEXTURES);

		for( int i=0; i<(int)exp.tex_buffer.size(); i+=2 )
		{
			word value = exp.tex_buffer[i] << 8;
			if( i+1<(int)exp.tex_buffer.size() ) value |= exp.tex_buffer[i+1];

			block->AddWord(value);
		}
		
		if( export_deflate ) block->Deflate();
	}


	// ================================================================ Write vertexes ================================================================
	{
		DataBlock *block = bset.NewBlock("VERTEXES");
		block->ConfigInitBlock(MAPBLOCK_ID_VERTEXES, 4, exp.verts.size());
		block->map_block_engine_element_size = 8;

		for( int i=0; i<(int)exp.verts.size(); i++ )
		{
			block->AddWord(exp.verts[i]->xp);
			block->AddWord(exp.verts[i]->yp);
			block->AddCommentF("%d", i);
			block->AddLineBreak();
		}
	
		if( export_deflate ) block->Deflate();
	}


	// ================================================================ Write lines ================================================================
	tex_offsets[texture_id_t()] = 0;
	{
		DataBlock *block = bset.NewBlock("LINES");
		block->ConfigInitBlock(MAPBLOCK_ID_LINES, 26, exp.lines.size());
		block->map_block_engine_element_size = 32;

		for( int i=0; i<(int)exp.lines.size(); i++ )
		{
			Line &l = *exp.lines[i];
			byte ceil_col = 0, floor_col = 0;
			//if( export_engine_version == 2 )
			//{
			//	int dir = 0;
			//	int dx = l.v2->xp - l.v1->xp;
			//	int dy = l.v2->yp - l.v1->yp;
			//	if( abs(dx)>abs(dy) )
			//		dir = (dx>=0) ? 1 : 3;
			//	else
			//		dir = (dy>=0) ? 2 : 4;
			//	ceil_col = chunky_packer.Process(5*0x0101);
			//	floor_col = chunky_packer.Process(dir*0x0101);
			//}
			//else
			{
				ceil_col = chunky_packer.Process(gfx_converter.GetTextureColor(l.tex_ceil));
				floor_col = chunky_packer.Process(gfx_converter.GetTextureColor(l.tex_floor));
			}

			word modes = 0;
			if( l._modes.size()>=1 ) modes |= l._modes[0] << 8;
			if( l._modes.size()>=2 ) modes |= l._modes[1];
			if( tex_offsets.find(l.tex_upper)==tex_offsets.end() ) printf("Texoffset missing for %d (%s)!\n", l.tex_upper._internal_index, gfx_converter.GetTextureNameEx(l.tex_upper).c_str());
			if( tex_offsets.find(l.tex_lower)==tex_offsets.end() ) printf("Texoffset missing for %d (%s)!\n", l.tex_lower._internal_index, gfx_converter.GetTextureNameEx(l.tex_lower).c_str());


			int texoff_u = tex_offsets[l.tex_upper];
			int texoff_l = tex_offsets[l.tex_lower];
#if USE_TEXTURE_Y_OFFSETS
			texoff_u += -(l._yoffs_u>>1);
			texoff_l += -(l._yoffs_l>>1);
#endif

			block->AddWord(l.v1->_number);
			block->AddWord(l.v2->_number);
			block->AddCommentF("%d", i);
			block->AddDWord(texoff_u);
			block->AddWord(l.tex_offs_x);
			block->AddWord((ceil_col<<8) | floor_col);
			block->AddWord(modes);
			block->AddDWord(texoff_l);		// tex_lower
			block->AddWord(l.h1);	// y1
			block->AddWord(l.h2);	// y2
			block->AddWord(l.h3);	// y3
			block->AddWord(l.h4);	// y4
			block->AddLineBreak();
		}
	
		if( export_deflate ) block->Deflate();
	}

	// ================================================================ Write visibilisty (draw list) ================================================================
	{
		DataBlock *block = bset.NewBlock("VISLIST");
		block->ConfigRawBlock(MAPBLOCK_ID_VIS);

		// Gather debug info
		map<int, string> reg_start_cmt;
		for( int i=0; i<(int)exp.subsectors.size(); i++ )
		{
			RegionPoly *reg = exp.subsectors[i];
			reg_start_cmt[reg->_vis_start] += format(" %d", i);
		}
		
		// Export
		for( int i=0; i<(int)exp.vis.size(); i++ )
		{
			string cmt = exp.vis[i].ToString(i);
			const string &regcmt = reg_start_cmt[i];
			if( regcmt.size()>0 )
				cmt = format("%-28s<<<%s", cmt.c_str(), regcmt.c_str());

			block->AddWord(exp.vis[i].Encode());
			block->AddComment(cmt.c_str());
			block->AddLineBreak();
		}

		if( export_deflate ) block->Deflate();
	}

	// ================================================================ Write subsectors ================================================================
	{
		DataBlock *block = bset.NewBlock("SUBSECTORS");
		block->ConfigInitBlock(MAPBLOCK_ID_SUBSECTORS, 6, exp.subsectors.size());
		block->map_block_engine_element_size = 14;

		for( int i=0; i<(int)exp.subsectors.size(); i++ )
		{
			RegionPoly *reg = exp.subsectors[i];
			int type = reg->sector ? reg->sector->type | (reg->sector->tag << 8) : 0;
			int height = reg->sector ? reg->sector->floor_h : 0;

			block->AddWord(reg->_vis_start / export_vis_align);
			block->AddWord(type);
			block->AddWord(height);
			if( export_vis_groups )
				block->AddCommentF("%4d -> %6X", i, reg->_vis_start);
			else
				block->AddCommentF("%3d", i);
			block->AddLineBreak();
		}
	
		if( export_deflate ) block->Deflate();
	}



	// ================================================================ Write BSP nodes ================================================================
	{
		DataBlock *block = bset.NewBlock("BSPNODES");
		block->ConfigRawBlock(MAPBLOCK_ID_BSP);

		for( int i=0; i<(int)exp.nodes.size(); i++ )
		{
			BSPNode &b = *exp.nodes[i];
			int A = -b.line.normal.x;
			int B = -b.line.normal.y;
			int C = (-b.line.normal.z) << ENGINE_SUBVERTEX_PRECISION;	// input coords are fix.3

			if( A<-32767 || A>32767 || B<-32767 || B>32767 )
				printf("ERROR: BSP line vector magnitude.\n");

			int left=b.left->_number, right=b.right->_number;
			if( left<-32767 || right<-32767 )
				printf("ERROR: BSP line draw list overflow.\n");
			if( left*12>32767 || right*12>32767 )
				printf("ERROR: BSP node count overflow.\n");

			block->AddEmptyLine();
			block->AddLineCommentF("BSP node %d", i);
			block->AddWord((word)A);
			block->AddWord((word)B);
			block->AddDWord(C);
			if( left>=0 )	block->AddWord(left*12);
			else			block->AddWord(left);
			if( right>=0 )	block->AddWord(right*12);
			else			block->AddWord(right);
			block->AddLineBreak();
		}

		for( int i=0; i<(int)exp.colnodes.size(); i++ )
		{
			ColBSPNode &b = *exp.colnodes[i];
			block->AddEmptyLine();
			block->AddLineCommentF("collision BSP node %d", i);
			block->AddWord((word)b.A);
			block->AddWord((word)b.B);
			block->AddDWord(b.C);
			block->AddWord(b.left->_number);
			block->AddWord(b.right->_number);
			block->AddLineBreak();
		}

		if( export_deflate ) block->Deflate();
	}


	// ================================================================ Write actor list ================================================================
	fprintf(fp, "\nconst DataActor *MAP_ACTOR_DEFS[] ={\n");
	{
		vector<string> actor_table;

		for( int j=0; j<(int)scomp.actors.size(); j++ )
			if( scomp.actors[j]->thing_id && scomp.actors[j]->is_used )
			{
				string name = "&Actor_" + scomp.idents[scomp.actors[j]->name_id].name + ",";

				if (scomp.actors[j]->thing_id >= actor_table.size())
					actor_table.resize(scomp.actors[j]->thing_id + 1, "NULL,");
				else if (actor_table[scomp.actors[j]->thing_id] != "NULL,")
					printf("ERROR: Thing number #%d used for both %s and %s!\n", name.c_str(), actor_table[scomp.actors[j]->thing_id].c_str());
				actor_table[scomp.actors[j]->thing_id] = name;
			}

		for( int i=0; i<(int)actor_table.size(); i++ )
			fprintf(fp, "\t%-24s// %3d\n", actor_table[i].c_str(), i);
	}
	fprintf(fp, "};\n");



	// ================================================================ Write things ================================================================
	{
		DataBlock *block = bset.NewBlock("THINGS");
		block->ConfigSetupBlock(MAPBLOCK_ID_THINGS, 8, 0);

		for( int i=0; i<(int)things.size(); i++ )
		{
			MapFileThing &t = things[i];
			const char *s = NULL;

			if( t.type>=1 && t.type<=9 )
			{
				Location *loc = &locations[t.type-1];
				loc->xp = t.xp;
				loc->yp = t.yp;
				loc->angle = t.angle;
			}
			else
			{
				block->map_block_element_count++;
				block->AddWord(t.type);
				block->AddWord(t.xp);
				block->AddWord(t.yp);
				block->AddWord(t.flags);
				block->AddCommentF("%3d: type %3d", i, t.type);
				block->AddLineBreak();
			}
		}
	}


	// ================================================================ Write palette ================================================================
	do {
		GfxPalette *pal = gfx_converter.FindPalette("Main");
		if( !pal )
		{
			elog.Error(NULL, "Missing palette 'Main'");
			break;
		}

		DataBlock *block = bset.NewBlock("PALETTE");
		if( !pal->ExportPaletteBlock(*block) )
			break;

	} while( 0 );

	// ================================================================ Write locations ================================================================
	{
		DataBlock *block = bset.NewBlock("LOCATIONS");
		block->ConfigSetupBlock(MAPBLOCK_ID_LOCATIONS, 6, 9);

		for( int i=0; i<9; i++ )
		{
			Location &loc = locations[i];
			block->AddWord(loc.xp);
			block->AddWord(loc.yp);
			block->AddWord((-loc.angle/45*32)&0xFF);
			block->AddCommentF("Location %d (angle %d)", i, loc.angle);
			block->AddLineBreak();
		}
	}


	// ================================================================ Write machines ================================================================
	{
		DataBlock *block = bset.NewBlock("MACHINES");
		block->ConfigSetupBlock(MAPBLOCK_ID_MACHINES, 6, machines.size());

		for( int i=0; i<(int)machines.size(); i++ )
		{
			Machine &m = machines[i];
			block->AddWord(m.type);
			block->AddWord(m.line0->_number);
			block->AddWord(m.tag);
			block->AddCommentF("%3d", i);
			block->AddLineBreak();
		}
	}

	// ================================================================ Write conditions ================================================================
	{
		DataBlock *block = bset.NewBlock("CONDITIONS");
		block->ConfigSetupBlock(MAPBLOCK_ID_CONDITIONS, 6, vis_conditions.size()+1);

		for( int i=0; i<(int)vis_conditions.size(); i++ )
		{
			VisCondition &vc = vis_conditions[i];
			block->AddWord(vc.op);
			block->AddWord(vc.c1);
			block->AddWord(vc.c2);
			block->AddCommentF("%5d:  %s", i, GetConditionName(i, true).c_str());
			block->AddLineBreak();
		}

		block->AddWord(0xFFFF);
		block->AddWord(0);
		block->AddWord(0);
		block->AddComment("end");
		block->AddLineBreak();
	}


	// ================================================================ Done ================================================================

	//bset.DumpBlockStats();
	DataMemTracker mem;
	for( int i=1; i<=3; i++ )
	{
		int size = proj_cfg.GetInt(format("export-a500-mem-%d", i), 0);
		mem.AddBank(size/2);
	}
	bset.VeryfyLoad(mem);

	bset.ExportAllAsAsm(asm_fp);

	
	// ================================================================ Summary ================================================================
	
	printf("Export:                             %7d vertexes.\n", exp.verts.size());
	printf("Export:                             %7d lines.\n", exp.lines.size());
	printf("Export:                             %7d BSP nodes.\n", exp.nodes.size() + exp.colnodes.size());
	printf("Export:                             %7d subsectors.\n", exp.subsectors.size());
	printf("Export: ENGINE_MAX_THINGS      160  %7d things.\n", things.size()+machines.size()+1);
	printf("Export: ENGINE_MAX_MACHINES     64  %7d machines.\n", machines.size());
	printf("Export: ENGINE_MAX_CONDITIONS  128  %7d conditions.\n", vis_conditions.size());
	printf("Export: ENGINE_MAX_VISLINES    256  %7d max vislines\n", exp.max_vis_lines);
	printf("Export:                             %7d vis bytes.\n", exp.vis.size()*2);
}


void DreadMapGen::ExportToSim(DreadSim &sim)
{
	// ================================================================ Write header ================================================================
	sim.e_map_header->map_collision_bsp_root = col_bsp_root_psize ? col_bsp_root_psize->_number : 0;
	sim.e_map_header->map_hitscan_bsp_root = col_bsp_root_0 ? col_bsp_root_0->_number : 0;

	// ================================================================ Write BSP nodes ================================================================
	sim.e_map_bspnodes.clear();

	for( int i=0; i<(int)exp.nodes.size(); i++ )
	{
		BSPNode &b = *exp.nodes[i];
		int A = -b.line.normal.x;
		int B = -b.line.normal.y;
		int C = (-b.line.normal.z) << ENGINE_SUBVERTEX_PRECISION;	// input coords are fix.3

		int left=b.left->_number, right=b.right->_number;

		DreadSim::DataBSPNode out;
		out.A = (short)A;
		out.B = (short)B;
		out.C = C;

		if( left>=0 )	out.left = left*12;
		else			out.left = left;
		if( right>=0 )	out.right = right*12;
		else			out.right = right;

		sim.e_map_bspnodes.push_back(out);
	}

	for( int i=0; i<(int)exp.colnodes.size(); i++ )
	{
		ColBSPNode &b = *exp.colnodes[i];
		
		DreadSim::DataBSPNode out;
		out.A = (short)b.A;
		out.B = (short)b.B;
		out.C = b.C;
		out.left = b.left->_number;
		out.right = b.right->_number;
	
		sim.e_map_bspnodes.push_back(out);
	}
}
