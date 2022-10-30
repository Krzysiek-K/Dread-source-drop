
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
static int adjust_y_offset(int ycoord, int texheight, int fudge, int y1, int y2)
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


void DreadMapGen::Line::ComputeTextureOffsets()
{
	if( 1 )
	{
		// use Y coords
		if( other_sector )
		{
			if( tex_unpegged_upper )	_yoffs_u = h1 - tex_offs_y;
			else						_yoffs_u = h2 - tex_offs_y;
			_yoffs_l = h3;		//lsrc->tex_offs_y;
		}
		else
		{
			if( tex_unpegged_lower )	_yoffs_u = h2 - tex_offs_y;
			else						_yoffs_u = h1 - tex_offs_y;
			_yoffs_l = 0;
		}

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
	printf("GC: Freed %d regions (%d -> %d, %.1f%% drop).\n", num_before-num_after, num_before, num_after, (num_before-num_after)*100.f/num_before);
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
	things.clear();
	locations.clear();
	machines.clear();
	vis_conditions.clear();
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

void DreadMapGen::LoadMap(MapFileHeader *msrc)
{
	Clear();

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
		l.v1 = AssureVertex(mv1.xp, mv1.yp);
		if( mline.other_sector!=0xFFFF )
			l.other_sector = sectors[mline.other_sector];
		l.tex_upper = gfx_converter.GetTextureId(mline.upper_tex.c_str(), mline.light_level);
		l.tex_lower = gfx_converter.GetTextureId(mline.lower_tex.c_str(), mline.light_level);
		l.line_action = mline.line_action;
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
		Line &l = *lines[i];

		if( l.other_sector )
		{
			l.h1 = l.sector->ceil_h;
			l.h2 = l.other_sector->ceil_h;
			l.h3 = l.other_sector->floor_h;
			l.h4 = l.sector->floor_h;

			if( !l.other_sector->tex_floor )
			{
				// Sector to be removed (pure sky)
				l.tex_upper = l.tex_lower;
				l.tex_lower.clear();
				l.h1 = l.other_sector->floor_h;
				l.h2 = l.h3 = l.h4 = l.sector->floor_h;
				l.other_sector = NULL;
			}

			if( l.line_action>=101 && l.line_action<=116 )
			{
				// Facade height
				l.h1 = l.h4 - (l.line_action-100)*16;
			}

			if( l.h2 <= l.h1 )
			{
				l.h2 = l.h1;
				if( !(l.line_action>=1 && l.line_action<=4) )	// If not a door
					l.tex_upper.clear();
			}
			if( l.h3 >= l.h4 )
			{
				l.h3 = l.h4;
				l.tex_lower.clear();
			}

			// Find other line
			if( !l.other_line )
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
		}
		else
		{
			// One sided
			l.h1 = l.sector->ceil_h;
			l.h2 = l.h3 = l.h4 = l.sector->floor_h;
			l.tex_lower.clear();

			if( l.line_action>=101 && l.line_action<=116 )
			{
				// Facade height
				l.h1 = l.h4 - (l.line_action-100)*16;
			}
		}
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
		if( line->other_sector )
		{
			if( !line->other_line || line->other_line->other_sector!=line->sector || line->other_line->other_line!=line )
				printf("ERROR: 2-sided line link mismatch.");
		}
	}
	
	// Process line special actions
	for( int i=0; i<(int)lines.size(); i++ )
	{
		Line *line = lines[i];
		if( line->line_action>=1 && line->line_action<=4 )
			MakeDoor(line);
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
				line->Error("ERROR: Line length >= 512!");
			else if( len + off >= 512 )
				line->Error("ERROR: Line texture offset moves texcoord over 512!");
			line->tex_offs_x = off;
		}
		else
			line->tex_offs_x = 0;
	}

	// Compute visibility
	SetDoorsPosition(0);	// open all doors
	ComputeAllVisibility();
	
	// Collisions
	SetDoorsPosition(0);	// open all doors
	ComputeCollisionMesh(collision_mesh, 0);
	ComputeCollisionMesh(collision_mesh, ENGINE_PLAYER_SIZE);

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
	printf("Door: ");
	if( !line->tex_upper ) { printf("ERROR: Can't make door (E1).\n"); return; }
	if( !line->other_sector ) { printf("ERROR: Can't make door (E2).\n"); return; }
	if( !line->other_line ) { printf("ERROR: Can't make door (E3).\n"); return; }

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
	if( door_trak->other_sector ) { printf("ERROR: Can't make door (E6).\n"); return; }
	printf("OK!\n");

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
	dr.line0 = door_edge;
	dr.line1 = door_front;
	dr.line2 = door_back;
	dr.line_x1 = front_out;
	dr.line_x2 = back_out;
	machines.push_back(dr);

	front_out->line_action = 0;
	front_in->line_action = 0;
	back_out->line_action = 0;
	back_in->line_action = 0;
}

void DreadMapGen::SetDoorsPosition(int prc)
{
	for( int i=0; i<(int)machines.size(); i++ )
	{
		Machine &m = machines[i];
#define LERP(a,b,p)		((a)+((b)-(a))*(p)/100)
		m.line1->v2->xp = LERP(m.line1->v1->xp, m.line_x1->v2->xp, prc);
		m.line1->v2->yp = LERP(m.line1->v1->yp, m.line_x1->v2->yp, prc);
		m.line2->v1->xp = LERP(m.line2->v2->xp, m.line_x2->v1->xp, prc);
		m.line2->v1->yp = LERP(m.line2->v2->yp, m.line_x2->v1->yp, prc);
#undef LERP
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
	// Reset
	printf("Vis: Reset...\n");
	ResetVisibility();

	// Initial sector split BSP
	printf("Vis: Initial BSP...\n");
	regions.SnapshotPolySet();
	ComputeInitialBSPTree();

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
	}
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

		// door_front(1) includes visibility from front_out(x1), door_back(2) includes from back_out(x2), door_edge(0) includes from both(x1+x2)
		IncludeLineVisibility(m.line0, m.line_x1);
		IncludeLineVisibility(m.line0, m.line_x2);
		IncludeLineVisibility(m.line1, m.line_x1);
		IncludeLineVisibility(m.line2, m.line_x2);
	}
	
	// Build final BSP
	printf("Vis: Final BSP...\n");
	ComputeFinalBSPTree(true);

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
}

void DreadMapGen::ResetVisibility()
{
	// regions.Init(REGION_SIZE);

	regions.Clear();
	for( int i=0; i<(int)sectors.size(); i++ )
	{
		RegionPoly *p = regions.NewRegion();

		MakePolySector(sectors[i], p->poly);
		p->sector = sectors[i];
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

	// cheat function number
	int fm=-2, cm=-2;
#include "gen/cheatselector.inc"

	vis->cheat_mode = vis->first_draw ? fm : cm;
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
		
		DreadMapGen::RegionPoly *ip = regions.SelectRegion(secpoly, true);
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
	bool picky			= true;
	bool todo			= false;
	int conmode			= -1;


	while( work )
	{
		work = false;
		todo = false;

		for( int i=0; i<reg->_vis_pending.size(); i++ )
			if( !reg->_vis_pending[i].done )
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
				}
				
				if( ve.vissector )
					ve.vissector->draw_order = (int)reg->drawlist.size();

				ve.done = true;
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
				printf("WARNING: No valid line order for visibility region at (%.0f,%.0f)!!!\n", mid.x, mid.y);
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

void DreadMapGen::BuildVisDrawLists()
{
	//for( auto p=regions.poly_set.begin(); p!=regions.poly_set.end(); ++p )
	//	ComputeLineOrder(*p);

	Vis_GatherSectors();

	for( auto p=regions.poly_set.begin(); p!=regions.poly_set.end(); ++p )
		Vis_PrepareRegion(*p);

	for( auto p=regions.poly_set.begin(); p!=regions.poly_set.end(); ++p )
		Vis_ComputeDrawOrder(*p);

	for( auto p=regions.poly_set.begin(); p!=regions.poly_set.end(); ++p )
		Vis_CleanupRegion(*p);
}



//bool DreadMapGen::ComputeLineOrder(RegionPoly *reg, bool check)
//{
//	reg->drawlist.clear();
//
//	// Reset / selct cheat modes
//	int numlines = 0;
//	for( auto p=reg->vis.begin(); p!=reg->vis.end(); ++p )
//		if( p->first->IsDrawn() )
//		{
//			p->second->draw_order = -1;
//			p->second->locknum = 0;
//			numlines++;
//
//			SelectVisLineCheat(p->second);
//		}
//
//	// Find visibility restrictions
//	Polygon2D poly;
//
//	for( auto p=reg->vis.begin(); p!=reg->vis.end(); ++p )
//		for( auto q=reg->vis.begin(); q!=reg->vis.end(); ++q )
//			if( p->first < q->first )
//			{
//				Line *a = p->first;
//				Line *b = q->first;
//				if( !a->IsDrawn() || !b->IsDrawn() )
//					continue;
//				PointRP2 a1 = a->v1->point_rp2();
//				PointRP2 a2 = a->v2->point_rp2();
//				PointRP2 b1 = b->v1->point_rp2();
//				PointRP2 b2 = b->v2->point_rp2();
//				LineRP2 la = a1.LineTo(a2);
//				LineRP2 lb = b1.LineTo(b2);
//				int m = 0;
//				if( la.CheckSide(b2) > 0 ) m |= 1;
//				if( la.CheckSide(b1) > 0 ) m |= 2;
//				if( lb.CheckSide(a2) > 0 ) m |= 4;
//				if( lb.CheckSide(a1) > 0 ) m |= 8;
//
//				static const int MODE[16] ={
//					0, 5, 6, 7,
//					2, 0, 0, 5,
//					1, 0, 0, 6,
//					3, 2, 1, 0,
//				};
//				m = MODE[m];
//				if( !m ) continue;
//				if( m&4 ) swap(a, b);
//				if( b->sector==reg->sector ) continue;	// can't be covered
//
//				poly = reg->poly;
//				poly.ClipBy(la);
//				poly.ClipBy(lb);
//
//				if( m&1 ) poly.ClipBy(a1.LineTo(b2));
//				if( m&2 ) poly.ClipBy(b1.LineTo(a2));
//
//				if( !poly.IsEmpty() )
//				{
//					line_order[a].push_back(q->second);
//					q->second->locknum++;
//				}
//			}
//
//	// Compule valid order
//	bool warned = false;
//	bool nocheck = false;
//	bool picky = true;
//	int conmode = -1;
//	while( numlines>0 )
//	{
//		bool work = false;
//
//		for( auto p=reg->vis.begin(); p!=reg->vis.end(); ++p )
//			if( p->first->IsDrawn() && p->second->draw_order<0 && (nocheck || !p->second->locknum) )
//			{
//				if( picky && p->second->condition!=conmode )
//					continue;
//
//				p->second->draw_order = (int)reg->drawlist.size();
//				reg->drawlist.push_back(p->second);
//				p->first->_mark = 1;
//				numlines--;
//				work = true;
//				nocheck = false;
//
//				vector<VisLine*> &locks = line_order[p->first];
//				for( int i=0; i<(int)locks.size(); i++ )
//					locks[i]->locknum--;
//
//				conmode = p->second->condition;
//				picky = true;
//			}
//
//		if( picky && !work )
//		{
//			picky = false;
//			continue;
//		}
//
//		if( !work )
//		{
//			if( check ) return false;
//
//			if( !warned )
//			{
//				warned = true;
//			
//				vec2 mid = reg->MidPoint();
//				printf("WARNING: No valid line order for visibility region at (%.0f,%.0f)!!!\n", mid.x, mid.y);
//				break;
//			}
//			nocheck = true;
//		}
//	}
//
//	return !warned;
//}

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
	if( total_surface >= 128*64 ) depth = 1;
	if( total_surface >= 256*256 ) depth = 2;
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



void DreadMapGen::ComputeCollisionMesh(PolygonMesh2D &mesh, int offset)
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
		if( !line->IsCollision() )
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

			if( offset > 0 && miss >= offset*1.3f )
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
		if( !line->IsCollision() )
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
int DreadMapGen::ExportLine(Line *line, ExportState &exp, int request_mode)
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
	
	int mode = (int)line->_modes.size();
	line->_modes.push_back(request_mode);
	if( mode > VIS_PACKED_MASK )
	{
		static const char *LC[] ={
#include "gen/cheatnames.inc"
		};

		printf("ERROR: Line mode table limit exceeded. (line midpoint %d, %d)\n", (line->v1->xp+line->v2->xp)/2, (line->v1->yp+line->v2->yp)/2);
		for( int i=0; i<line->_modes.size(); i++ )
			printf("    %3d: %s\n",
				line->_modes[i],
				uint32_t(line->_modes[i]) < sizeof(LC)/sizeof(LC[0]) ? LC[line->_modes[i]] : "???"
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
}

// Gather pass 2
void DreadMapGen::ExportRegionProcess(RegionPoly *reg, ExportState &exp)
{
	if( reg->_vis_start >= 0 )
		return;
	
	// Visibility
	reg->_vis_start = (int)exp.vis.size();


	int conmode = -1;
	int vislines = 0;
	for( int i=0; i<(int)reg->drawlist.size(); i++ )
	{
		VisEntry &ve = reg->drawlist[i];

		if( ve.condition!=conmode )
		{
			exp.vis.push_back(ExportVis(ve.condition + ENGINE_VIS_CONDITION_BASE, EVIS_CONDITION));
			conmode = ve.condition;
		}

		if( ve.visline )
		{
			Line *line = ve.visline->line;
			int vismode = ExportLine(line, exp, ve.visline->cheat_mode);

			exp.vis.push_back(ExportVis(vismode + ENGINE_VIS_LINEMODE_BASE, EVIS_LINEMODE));

			if( line->tex_upper.is_set() ) exp.tex_set.insert(line->tex_upper);
			if( line->tex_lower.is_set() ) exp.tex_set.insert(line->tex_lower);

			vislines++;
		}

		if( ve.vissector )
		{
			// Thing (subsector) visibility - include all subsectors
			Sector *sec = ve.vissector->sector;
			for( int j=0; j<sec->regions.size(); j++ )
				exp.vis.push_back(ExportVis(sec->regions[j]->_number + ENGINE_VIS_SUBSECTOR_BASE, EVIS_SUBSECTOR));
		}
	}

	reg->_thingvis_start = (int)exp.vis.size();		// TBD: remove
	exp.vis.push_back(ExportVis(ENGINE_VIS_ENDLIST, EVIS_MARKER));

	if( vislines > exp.max_vis_lines )
		exp.max_vis_lines = vislines;
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
		ExportVertex(m.line0->v1, exp);		// v+0:	first moving vert
		ExportVertex(m.line0->v2, exp);		// v+1: second moving vert
		ExportVertex(m.line1->v1, exp);		// v+2: first min pos
		ExportVertex(m.line2->v2, exp);		// v+3: second min pos
		ExportVertex(m.line_x1->v2, exp);	// v+4: first max pos
		ExportVertex(m.line_x2->v1, exp);	// v+5: second max pos

		// export lines - after vertexes
		ExportLine(m.line0, exp, -1);	// edge
		ExportLine(m.line1, exp, -1);	// front
		ExportLine(m.line2, exp, -1);	// back
	}

	// Gather
	ExportBSPRec(regions.bsproot, exp);

	for( int i=0; i<(int)exp.subsectors.size(); i++ )
		ExportRegionProcess(exp.subsectors[i], exp);
	
	for( auto p=exp.tex_set.begin(); p!=exp.tex_set.end(); ++p )
		tex_list.push_back(*p);

	ExportColBSPRec(col_bsp_root_psize, exp);
	ExportColBSPRec(col_bsp_root_0, exp);
}

void DreadMapGen::ExportWrite(FILE *fp, FILE *asm_fp, ScriptCompiler &scomp, ChunkyColorPacker &chunky_packer, map<texture_id_t, int> &tex_offsets)
{
	if( !fp )
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


	DataBlockSet bset;



	// ================================================================ Write header ================================================================

	//if( col_bsp_root_psize ) fprintf(fp, "const int MAP_COLLISION_BSP_ROOT = %d;\n", col_bsp_root_psize->_number);
	//if( col_bsp_root_0 ) fprintf(fp, "const int MAP_HITSCAN_BSP_ROOT = %d;\n", col_bsp_root_0->_number);

	// Again, as asm
	if( asm_fp )
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

	// ================================================================ Write vertexes ================================================================
	
	//fprintf(fp, "\nconst DataVertex MAP_VTX[] ={\n");
	//for( int i=0; i<(int)exp.verts.size(); i++ )
	//	fprintf(fp, "\t{ %5d, %5d },\t// %4d\n", exp.verts[i]->xp, exp.verts[i]->yp, i);
	//fprintf(fp, "\t{ 0x7FFF, 0x7FFF }");
	//fprintf(fp, "};\n");

	// Again, as asm
	if( asm_fp )
	{
		DataBlock *block = bset.NewBlock("VERTEXES");
		block->ConfigInitBlock(MAPBLOCK_ID_VERTEXES, 4, exp.verts.size());

		for( int i=0; i<(int)exp.verts.size(); i++ )
		{
			block->AddWord(exp.verts[i]->xp);
			block->AddWord(exp.verts[i]->yp);
			block->AddCommentF("%d", i);
			block->AddLineBreak();
		}
	}


	// ================================================================ Write lines ================================================================
	
//	fprintf(fp, "\nconst DataLine MAP_LINES[] ={\n");
//	for( int i=0; i<(int)exp.lines.size(); i++ )
//	{
//		// { 2, 3, TEX_RW23_4_OFFS, -128, -64, 0x40, 0xC0, 6 },
//
//		Line &l = *exp.lines[i];
//		fprintf(fp, "\t{ %5d, %5d,", l.v1->_number, l.v2->_number);
//
//		string tname = gfx_converter.GetTextureOffsetName(l.tex_upper);
//		l.ComputeTextureOffsets();
//#if USE_TEXTURE_Y_OFFSETS
//		fprintf(fp, " %-32s %+3d,", tname.c_str(), -(l._yoffs_u>>1));
//#else
//		fprintf(fp, " %-32s,", tname.c_str());
//#endif
//
//		//fprintf(fp, "%5d,%5d,", l.h1, l.h2);
//
//		fprintf(fp, "%4d,", l.tex_offs_x);
//
//		byte ceil_col = chunky_packer.Process(gfx_converter.GetTextureColor(l.tex_ceil));
//		byte floor_col = chunky_packer.Process(gfx_converter.GetTextureColor(l.tex_floor));
//		fprintf(fp, " 0x%02X,", ceil_col);
//		fprintf(fp, " 0x%02X,", floor_col);
//
//		fprintf(fp, " {");
//		for( int j=0; j<=VIS_PACKED_MASK; j++ )
//			fprintf(fp, " %3d,", (j<l._modes.size()) ? l._modes[j] : 0);
//
//		fprintf(fp, " } },\t// %4d\n", i);
//	}
//	fprintf(fp, "\t{ 0xFFFF, 0, 0, 0, 0, 0, 0 }\n");
//	fprintf(fp, "};\n");


	// Again, as asm
	if( asm_fp )
	{
		DataBlock *block = bset.NewBlock("LINES");
		block->ConfigInitBlock(MAPBLOCK_ID_LINES, 14, exp.lines.size());

		for( int i=0; i<(int)exp.lines.size(); i++ )
		{
			Line &l = *exp.lines[i];
			byte ceil_col = chunky_packer.Process(gfx_converter.GetTextureColor(l.tex_ceil));
			byte floor_col = chunky_packer.Process(gfx_converter.GetTextureColor(l.tex_floor));
			word modes = 0;
			if( l._modes.size()>=1 ) modes |= l._modes[0] << 8;
			if( l._modes.size()>=2 ) modes |= l._modes[1];

			int texture_offset = tex_offsets[l.tex_upper];
#if USE_TEXTURE_Y_OFFSETS
			texture_offset += -(l._yoffs_u>>1);
#endif

			block->AddWord(l.v1->_number);
			block->AddWord(l.v2->_number);
			block->AddCommentF("%d", i);
			block->AddDWord(texture_offset);
			block->AddWord(l.tex_offs_x);
			block->AddWord((ceil_col<<8) | floor_col);
			block->AddWord(modes);
			block->AddLineBreak();
		}
	}

	// ================================================================ Write BSP nodes ================================================================
	//fprintf(fp, "\nconst DataBSPNode MAP_NODES[] ={\n");
	//for( int i=0; i<(int)exp.nodes.size(); i++ )
	//{
	//	BSPNode &b = *exp.nodes[i];
	//	int A = -b.line.normal.x;
	//	int B = -b.line.normal.y;
	//	int C = (-b.line.normal.z) << ENGINE_SUBVERTEX_PRECISION;	// input coords are fix.3
	//
	//	if( A<-32767 || A>32767 || B<-32767 || B>32767 )
	//		printf("ERROR: BSP line vector magnitude.\n");
	//
	//	int left=b.left->_number, right=b.right->_number;
	//	if( left<-32767 || right<-32767 )
	//		printf("ERROR: BSP line draw list overflow.\n");
	//	if( left>32767/12 || right>32767/12 )
	//		printf("ERROR: BSP node count overflow.\n");
	//
	//	fprintf(fp, "\t{ %6d, %6d, %8d,", A, B, C);
	//	if( left>=0 )	fprintf(fp, " %6d,", left*12);
	//	else			fprintf(fp, " %6s,", format("~%d", ~left).c_str());
	//	if( right>=0 )	fprintf(fp, " %6d,", right*12);
	//	else			fprintf(fp, " %6s,", format("~%d", ~right).c_str());
	//	fprintf(fp, " },\t// %4d\n", i*12);
	//}
	//fprintf(fp, "// Collision BSP\n");
	//for( int i=0; i<(int)exp.colnodes.size(); i++ )
	//{
	//	ColBSPNode &b = *exp.colnodes[i];
	//	fprintf(fp, "\t{ %6d, %6d, %8d, %6d, %6d },\t// %4d\n", b.A, b.B, b.C, b.left->_number, b.right->_number, ((int)exp.nodes.size()+i)*12);
	//}
	//fprintf(fp, "};\n");


	// Again, as asm
	if( asm_fp )
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
			if( left>32767/12 || right>32767/12 )
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
	}


	// ================================================================ Write visibilisty (draw list) ================================================================

	//fprintf(fp, "\nconst word MAP_VIS[] ={\n");
	//for( int i=0; i<(int)exp.vis.size(); i++ )
	//{
	//	if( exp.vis[i].type & EVIS_BIT_PRINT_HEX )
	//		fprintf(fp, "\t0x%04X,\t\t\t// %4d\n", exp.vis[i].value, i);
	//	else
	//		fprintf(fp, "\t%6d,\t\t\t// %4d\n", exp.vis[i].value, i);
	//}
	//fprintf(fp, "};\n");

	// Again, as asm
	if( asm_fp )
	{
		DataBlock *block = bset.NewBlock("VISLIST");
		block->ConfigRawBlock(MAPBLOCK_ID_VIS);

		for( int i=0; i<(int)exp.vis.size(); i++ )
		{
			block->AddWord(exp.vis[i].value);

			if( exp.vis[i].type & EVIS_BIT_PRINT_HEX )
				block->AddCommentF("0x%04X\t%4d", exp.vis[i].value, i);
			else
				block->AddCommentF("%6d\t%4d", exp.vis[i].value, i);
			block->AddLineBreak();
		}
	}


	// ================================================================ Write subsectors ================================================================
	
	//fprintf(fp, "\nconst DataSubSector MAP_SUBSECTORS[] ={\n");
	//for( int i=0; i<(int)exp.subsectors.size(); i++ )
	//{
	//	RegionPoly *reg = exp.subsectors[i];
	//	int type = reg->sector ? reg->sector->type : 0;
	//	//fprintf(fp, "\t{ %5d, %5d, %5d },\n", reg->_vis_start, reg->_thingvis_start, type);
	//	fprintf(fp, "\t{ %5d, %5d },\n", reg->_vis_start, type);
	//}
	//fprintf(fp, "\t{ 0xFFFF, 0 }\n");
	//fprintf(fp, "};\n");


	// Again, as asm
	if( asm_fp )
	{
		DataBlock *block = bset.NewBlock("SUBSECTORS");
		block->ConfigInitBlock(MAPBLOCK_ID_SUBSECTORS, 4, exp.subsectors.size());

		for( int i=0; i<(int)exp.subsectors.size(); i++ )
		{
			RegionPoly *reg = exp.subsectors[i];
			int type = reg->sector ? reg->sector->type : 0;

			block->AddWord(reg->_vis_start);
			block->AddWord(type);
			block->AddCommentF("%d", i);
			block->AddLineBreak();
		}
	}



	// ================================================================ Write actor list ================================================================
	fprintf(fp, "\nconst DataActor *MAP_ACTOR_DEFS[] ={\n");
	{
		vector<string> actor_table;

		for( int j=0; j<(int)scomp.actors.size(); j++ )
			if( scomp.actors[j]->thing_id )
			{
				if( scomp.actors[j]->thing_id >= actor_table.size() )
					actor_table.resize(scomp.actors[j]->thing_id+1, "NULL,");
				actor_table[scomp.actors[j]->thing_id] = "&Actor_" + scomp.idents[scomp.actors[j]->name_id].name + ",";
			}

		for( int i=0; i<(int)actor_table.size(); i++ )
			fprintf(fp, "\t%-24s// %3d\n", actor_table[i].c_str(), i);
	}
	fprintf(fp, "};\n");



	// ================================================================ Write things ================================================================
	//fprintf(fp, "\nconst DataThing MAP_THINGS[] ={\n");
	//for( int i=0; i<(int)things.size(); i++ )
	//{
	//	MapFileThing &t = things[i];
	//	const char *s = NULL;
	//	Location *loc = NULL;
	//
	//	switch( t.type )
	//	{
	//	case  1:	loc = &locations[0];	break;		// Player 1 start
	//	case  2:	loc = &locations[1];	break;		// Location 2
	//	case  3:	loc = &locations[2];	break;		// Location	3
	//	case  4:	loc = &locations[3];	break;		// Location	4
	//	case  5:	loc = &locations[4];	break;		// Location	5
	//	case  6:	loc = &locations[5];	break;		// Location	6
	//	case  7:	loc = &locations[6];	break;		// Location	7
	//	case  8:	loc = &locations[7];	break;		// Location	8
	//	case  9:	loc = &locations[8];	break;		// Location	9
	//	}
	//
	//	for( int j=0; j<(int)scomp.actors.size(); j++ )
	//		if( scomp.actors[j]->thing_id && scomp.actors[j]->thing_id == t.type )
	//			s = scomp.idents[scomp.actors[j]->name_id].name.c_str();
	//
	//	if( loc )
	//	{
	//		loc->xp = t.xp;
	//		loc->yp = t.yp;
	//		loc->angle = t.angle;
	//	}
	//	else if( s )
	//		fprintf(fp, "\t{ %d, %5d, %5d, 0x%04X },\t// %3d: type %3d\n", t.type, t.xp, t.yp, t.flags, i, t.type);
	//	else
	//		fprintf(fp, "\t\t\t// unknown object type %3d\n", t.type);
	//}
	//fprintf(fp, "\t{ 0, 0, 0 }\n");
	//fprintf(fp, "};\n");


	// Again, as asm
	if( asm_fp )
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


	// ================================================================ Write locations ================================================================

	//fprintf(fp, "\nconst DataLocation MAP_LOCATIONS[] ={\n");
	//for( int i=0; i<9; i++ )
	//{
	//	Location &loc = locations[i];
	//	fprintf(fp, "\t{ %5d, %5d, %5d },\t// Location %d\n", loc.xp, loc.yp, loc.angle, i);
	//}
	//fprintf(fp, "};\n");
	
	// Again, as asm
	if( asm_fp )
	{
		DataBlock *block = bset.NewBlock("LOCATIONS");
		block->ConfigSetupBlock(MAPBLOCK_ID_LOCATIONS, 6, 9);

		for( int i=0; i<9; i++ )
		{
			Location &loc = locations[i];
			block->AddWord(loc.xp);
			block->AddWord(loc.yp);
			block->AddWord((-loc.angle/45*32)&0xFF);
			block->AddCommentF("Location %d", i);
			block->AddLineBreak();
		}
	}


	// ================================================================ Write machines ================================================================
	
	//fprintf(fp, "\nconst DataMachine MAP_MACHINES[] ={\n");
	//for( int i=0; i<(int)machines.size(); i++ )
	//{
	//	Machine &m = machines[i];
	//	fprintf(fp, "\t{ %5d, %5d },\n", m.type, m.line0->_number);
	//}
	//fprintf(fp, "\t{ 0xFFFF, 0 }\n");
	//fprintf(fp, "};\n");

	// Again, as asm
	if( asm_fp )
	{
		DataBlock *block = bset.NewBlock("MACHINES");
		block->ConfigSetupBlock(MAPBLOCK_ID_MACHINES, 4, machines.size());

		for( int i=0; i<(int)machines.size(); i++ )
		{
			Machine &m = machines[i];
			block->AddWord(m.type);
			block->AddWord(m.line0->_number);
			block->AddCommentF("%3d", i);
			block->AddLineBreak();
		}
	}

	// ================================================================ Write conditions ================================================================
	
	//fprintf(fp, "\nconst DataCondition MAP_CONDITIONS[] ={\n");
	//for( int i=0; i<(int)vis_conditions.size(); i++ )
	//{
	//	VisCondition &vc = vis_conditions[i];
	//	fprintf(fp, "\t{ %1d, %5d, %5d },\t// %5d:  %s\n", vc.op, vc.c1, vc.c2, i, GetConditionName(i, true).c_str());
	//}
	//fprintf(fp, "\t{ 0xFFFF, 0, 0 }\n");
	//fprintf(fp, "};\n");

	// Again, as asm
	if( asm_fp )
	{
		DataBlock *block = bset.NewBlock("CONDITIONS");
		block->ConfigSetupBlock(MAPBLOCK_ID_CONDITIONS, 6, vis_conditions.size());

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

	if( asm_fp )
	{
		bset.ExportAllAsAsm(asm_fp);
	}

	
	// ================================================================ Summary ================================================================
	
	printf("Export: ENGINE_MAX_VERTEXES    1024 %d vertexes.\n", exp.verts.size());
	printf("Export: ENGINE_MAX_LINES       1536 %d lines.\n", exp.lines.size());
	printf("Export:                             %d BSP nodes.\n", exp.nodes.size() + exp.colnodes.size());
	printf("Export: ENGINE_MAX_SUBSECTORS  512  %d subsectors.\n", exp.subsectors.size());
	printf("Export: ENGINE_MAX_THINGS      144  %d things.\n", things.size()+machines.size()+1);
	printf("Export: ENGINE_MAX_MACHINES    64   %d machines.\n", machines.size());
	printf("Export: ENGINE_MAX_CONDITIONS  80   %d conditions.\n", vis_conditions.size());
	printf("Export: ENGINE_MAX_VISLINES    160  %d max vislines\n", exp.max_vis_lines);
	printf("Export:                             %d vis bytes.\n", exp.vis.size()*2);
}
