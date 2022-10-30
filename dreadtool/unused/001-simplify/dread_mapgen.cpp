
#include "common.h"
#include "app_rendertest.h"


#define REGION_SIZE			1024



// ================================================================ DreadMapGen::RegionPoly ================================================================




DreadMapGen::RegionPoly::RegionPoly(const RegionPoly &rp)
{
	sector = rp.sector;
	poly = rp.poly;
	bsp_subsector = rp.bsp_subsector;
	bsp_leaf = rp.bsp_leaf;
	marked = rp.marked;

	for( auto p=rp.vis.begin(); p!=rp.vis.end(); ++p )
		vis[p->first] = new VisLine(*p->second);
}


DreadMapGen::RegionPoly::~RegionPoly()
{
	for( auto p=vis.begin(); p!=vis.end(); ++p )
		delete p->second;
	vis.clear();
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
	for( int i=0; i<(int)polys.size(); i++ )
		delete polys[i];
	polys.clear();

	for( int i=0; i<(int)nodes.size(); i++ )
		delete nodes[i];
	nodes.clear();

	bsproot = NULL;
}

void DreadMapGen::RegionMap::Init(int range)
{
	Clear();

	RegionPoly *p = new RegionPoly();
	polys.push_back(p);
	p->poly.CreateRectangle(-range, -range, range, range);
}

DreadMapGen::RegionPoly *DreadMapGen::RegionMap::SelectRegionFromList(const Polygon2D &pr, const vector<RegionPoly*> &list, bool cut_regions)
{
	RegionPoly *list_head = NULL;
	
	Polygon2D C, inside, outside;
	vector<Polygon2D> outA;

	for( int i=0; i<(int)list.size(); i++ )
	{
		// init polygon C and set outA
		C = list[i]->poly;
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
			// add outside fragments
			list[i]->poly.edges.clear();
			for( int j=0; j<(int)outA.size(); j++ )
			{
				RegionPoly *np = new RegionPoly(*list[i]);
				polys.push_back(np);
				np->poly = outA[j];
			}

			// the common fragment is C
			list[i]->poly = C;
			list[i]->next = list_head;
			list_head = list[i];
		}
		else
		{
			// the polygon overlaps the region - cutting disabled
			list[i]->next = list_head;
			list_head = list[i];
		}
	}

	return list_head;
}

void DreadMapGen::RegionMap::DeleteRegion(RegionPoly *reg)
{
	for( int i=0; i<(int)polys.size(); i++ )
		if( polys[i]==reg )
		{
			polys.erase(polys.begin()+i);
			break;
		}
	delete reg;
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
		l.tex_upper = mline.tex_upper;
		l.tex_lower = mline.tex_lower;
	}

	// Load sectors
	for( int i=0; i<msrc->n_sectors; i++ )
	{
		MapFileSector &msec = msrc->sectors[i];
		Sector &s = *sectors[i];
		s.first_line = lines[msec.first_line];
		s.ceil_h = -msec.ceil_h;
		s.floor_h = -msec.floor_h;
		s.tex_ceil = msec.ceil_col;
		s.tex_floor = msec.floor_col;

		for( int j=0; j<msec.num_lines; j++ )
		{
			int j2 = msec.first_line + (j+1)%msec.num_lines;

			Line &l = *lines[msec.first_line + j];
			l.sector = &s;
			l.next_line = lines[j2];
			lines[j2]->prev_line = &l;

			MapFileVertex &mv2 = msrc->vertexes[msrc->lines[j2].vertex];
			l.v2 = AssureVertex(mv2.xp, mv2.yp);

			l.tex_ceil = msec.ceil_col;
			l.tex_floor = msec.floor_col;
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
				l.tex_lower = 0;
				l.h1 = l.other_sector->floor_h;
				l.h2 = l.h3 = l.h4 = l.sector->floor_h;
				l.other_sector = NULL;
			}

			if( l.h2 <= l.h1 )
			{
				l.h2 = l.h1;
				l.tex_upper = 0;
			}
			if( l.h3 >= l.h4 )
			{
				l.h3 = l.h4;
				l.tex_lower = 0;
			}
		}
		else
		{
			// One sided
			l.h1 = l.sector->ceil_h;
			l.h2 = l.h3 = l.h4 = l.sector->floor_h;
			l.tex_lower = 0;
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

	// Compute visibility
	ComputeAllVisibility();
}

DreadMapGen::Vertex *DreadMapGen::AssureVertex(int xp, int yp)
{
	for( int i=0; i<(int)verts.size(); i++ )
		if( verts[i]->xp==xp && verts[i]->yp==yp )
			return verts[i];
	Vertex *v = new Vertex();
	verts.push_back(v);
	v->xp = xp;
	v->yp = yp;
	return v;
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

DreadMapGen::RegionPoly *DreadMapGen::FindCameraRegion(vec2 pos)
{
#if 1
	// BSP version
	BSPNode *node = regions.bsproot;
	while( node && !node->region )
		node = (node->line.CheckSideVec2(pos) >= 0) ? node->right : node->left;

	if( node )
		return node->region;

#else
	// Brute force version
	for( int i=0; i<(int)regions.polys.size(); i++ )
	{
		RegionPoly &r = *regions.polys[i];
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
#endif

	return NULL;
}



void DreadMapGen::ComputeVisibility(Line *line, bool reset)
{
	if( reset )
		ResetVisibility();

	if( line->other_sector &&
		line->h1 == line->h2 &&
		line->h3 == line->h4 &&
		line->sector->tex_ceil == line->other_sector->tex_ceil &&
		line->sector->tex_floor == line->other_sector->tex_floor )
	{
		// invisible line
		return;
	}

	Beam beam;
	beam.p1s.push_back(line->v1->point_rp2());
	beam.p2s.push_back(line->v2->point_rp2());
	beam.portal = beam.p1s[0].LineTo(beam.p2s[0]);
	beam.l1 = beam.portal;
	beam.l2 = beam.portal;

	//beam.region.CreateRectangle(-REGION_SIZE, -REGION_SIZE, REGION_SIZE, REGION_SIZE);
	//beam.region.ClipBy(beam.p1.LineTo(beam.p2));

	ComputeVisibility_Rec(line, line->sector, beam, 0);
}

void DreadMapGen::ComputeAllVisibility()
{
	// Reset
	ResetVisibility();

	// Initial sector split BSP
	ComputeInitialBSPTree();

	// Compute visibility for all lines
	for( int i=0; i<(int)lines.size(); i++ )
		ComputeVisibility(lines[i], false);

	// Cleanup regions
	//SimplifyRegions();

	// Build final BSP
	ComputeFinalBSPTree();
	
	// Compule line order lists
	for( int i=0; i<(int)regions.polys.size(); i++ )
		ComputeLineOrder(regions.polys[i]);
}

void DreadMapGen::ResetVisibility()
{
	// regions.Init(REGION_SIZE);

	regions.Clear();
	for( int i=0; i<(int)sectors.size(); i++ )
	{
		RegionPoly *p = new RegionPoly();
		regions.polys.push_back(p);

		MakePolySector(sectors[i], p->poly);
		p->sector = sectors[i];
	}
}

void DreadMapGen::RegisterVisLine(RegionPoly *region, VisLine *vis, const Beam &beam)
{
	Line *line = vis->line;
	int fm=-2, cm=-2;
	if( line->tex_ceil==0 )
	{
		// SKY
		//	 4		Sky   64  0		front
		//	 8		Sky   64  0		CLIPPED
		if( line->h1== -64 && line->h2==  0 ) fm= 4, cm= 8;
		//	 5		Sky	 128  0		front
		//	 9		Sky  128  0		CLIPPED
		if( line->h1==-128 && line->h2==  0 ) fm= 5, cm= 9;
		//	 6		Sky  128 64		front
		//	10		Sky  128 64		CLIPPED
		if( line->h1==-128 && line->h2==-64 ) fm= 6, cm=10;
	}
	else
	{
		// Ceil
		//	 0		Ceil  64  0		front
		//	 3		Ceil  64  0		CLIPPED
		if( line->h1== -64 && line->h2==  0 ) fm= 0, cm= 3;
		//	 7		Ceil  64 64		CLIPPED		// no unclipped version
		if( line->h1== -64 && line->h2==-64 ) fm= 7, cm= 7;
		//	 1		Ceil 128  0		front
		if( line->h1==-128 && line->h2==  0 ) fm= 1, cm=-1;
		//	 2		Ceil 128 64		front
		if( line->h1==-128 && line->h2==-64 ) fm= 2, cm=-1;
	}

	if( beam.lineportals.size()==0 )
	{
		if( vis->cheat_mode==-1 )
			vis->cheat_mode = fm;
		else if( vis->cheat_mode==fm || vis->cheat_mode==cm )
		{
			// already done
		}
		else
			vis->cheat_mode = -2;
	}
	else
	{
		if( vis->cheat_mode==-1 || vis->cheat_mode==fm || vis->cheat_mode==cm )
			vis->cheat_mode = cm;
		else
			vis->cheat_mode = -2;
	}
}

void DreadMapGen::ComputeVisibility_Rec(Line *line, Sector *sec, Beam &beam, int depth)
{
	if( depth>=8 )
		return;

	// Process current sector
	{
		Polygon2D secpoly;
		MakePolySector(sec, secpoly);
		secpoly.ClipBy(beam.portal);
		secpoly.ClipBy(beam.l1);
		secpoly.ClipBy(beam.l2);
		
		if( secpoly.IsEmpty() )
			return;

		//if( !beam.region.ComputeIntersection(secpoly, poly) )
		//	return;

		DreadMapGen::RegionPoly *ip = regions.SelectRegion(secpoly, true);
		for( ; ip; ip=ip->next )
		{
			ip->marked = true;

			VisLine *vis = ip->vis[line];
			if( !vis )
			{
				vis = new VisLine();
				ip->vis[line] = vis;
				vis->line = line;
			}
			RegisterVisLine(ip, vis, beam);
		}
	}

	// Descend into next sectors
	{
		Line *sline = sec->first_line;
		LineRP2 _l1 = beam.l1;
		LineRP2 _l2 = beam.l2;
		LineRP2 _portal = beam.portal;
		size_t _p1s = beam.p1s.size();
		size_t _p2s = beam.p2s.size();
		size_t _lps = beam.lineportals.size();

		do
		{
			if( sline->other_sector )
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

					if( sline->sector->ceil_h == sline->other_sector->ceil_h &&
						sline->sector->floor_h == sline->other_sector->floor_h &&
						sline->sector->tex_ceil == sline->other_sector->tex_ceil &&
						sline->sector->tex_floor == sline->other_sector->tex_floor )
					{
						// line is invisible
					}
					else
						beam.lineportals.push_back(sline);

					ComputeVisibility_Rec(line, sline->other_sector, beam, depth+1);

					beam.p1s.resize(_p1s);
					beam.p2s.resize(_p2s);
					beam.l1 = _l1;
					beam.l2 = _l2;
					beam.portal = _portal;
					beam.lineportals.resize(_lps);
				}
			}

			sline = sline->next_line;
		} while( sline!=sec->first_line );
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
}

bool DreadMapGen::ComputeLineOrderRec(RegionPoly *reg, const vector<VisLine*> &pending, bool check)
{
	vector<VisLine*> left, right, colinear;

	if( pending.size()<=1 )
	{
		if( pending.size()==1 )
		{
			pending[0]->draw_order = (int)reg->drawlist.size();
			reg->drawlist.push_back(pending[0]);
		}
		return true;
	}


	// Check every possible split line
	for( int i=0; i<(int)pending.size(); i++ )
	{
		left.clear();
		right.clear();
		colinear.clear();

		LineRP2 line = pending[i]->line->v1->point_rp2().LineTo(pending[i]->line->v2->point_rp2());
		bool failed = false;
		for( int j=0; j<(int)pending.size(); j++ )
		{
			coord_t s1 = line.CheckSide(pending[j]->line->v1->point_rp2());
			coord_t s2 = line.CheckSide(pending[j]->line->v2->point_rp2());
			if( !s1 && !s2 )
				colinear.push_back(pending[j]);
			else if( s1<=0 && s2<=0 )
				left.push_back(pending[j]);
			else if( s1>=0 && s2>=0 )
				right.push_back(pending[j]);
			else
			{
				failed = true;
				break;
			}
		}
	
		if( failed ) continue;

		if( !ComputeLineOrderRec(reg, right, check) ) return false;
		for( int j=0; j<(int)colinear.size(); j++ )
		{
			colinear[j]->draw_order = (int)reg->drawlist.size();
			reg->drawlist.push_back(colinear[j]);
		}
		if( !ComputeLineOrderRec(reg, left, check) ) return false;

		return true;
	}

	if( !check )
	{
		vec2 mid = reg->MidPoint();
		printf("WARNING: No valid line order for visibility region at (%.0f,%.0f)!!!\n", mid.x, mid.y);
	}

	return false;
}

bool DreadMapGen::ComputeLineOrder(RegionPoly *reg, bool check)
{
	reg->drawlist.clear();

	// Reset
	vector<VisLine*> pending;
	int numlines = 0;
	for( auto p=reg->vis.begin(); p!=reg->vis.end(); ++p )
	{
		p->first->_mark = 0;
		p->first->_locknum = 0;
		pending.push_back(p->second);
		numlines++;
	}

#if 1

	return ComputeLineOrderRec(reg, pending, check);

#else
	// Find visibility restrictions
	map<Line*, vector<Line*> > line_order;
	for( auto p=reg->vis.begin(); p!=reg->vis.end(); ++p )
		for( auto q=reg->vis.begin(); q!=reg->vis.end(); ++q )
			if( p->first < q->first )
			{
				Line *a = p->first;
				Line *b = q->first;
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
				if( m&4 ) swap(a, b);
				if( b->sector==reg->sector ) continue;	// can't be covered
				if( m&1 ) la = a1.LineTo(b2);
				if( m&2 ) lb = b1.LineTo(a2);

				int mask = m&3;
				for( int i=0; mask && i<(int)reg->poly.edges.size(); i++ )
				{
					Edge2D &e = reg->poly.edges[i];

					if( (mask&1) && la.CheckSide(e.start) > 0 )
						mask &= ~1;

					if( (mask&2) && lb.CheckSide(e.start) > 0 )
						mask &= ~2;

					//coord_t sa1 = e.line.CheckSide(a1);
					//coord_t sa2 = e.line.CheckSide(a2);
					//coord_t sb1 = e.line.CheckSide(b1);
					//coord_t sb2 = e.line.CheckSide(b2);
					//if( sa1<=0 && sa2<=0 && (sa1 || sa2) )
					//{
					//	if( sb1>=0 && sb2>=0 && (sb1 || sb2) )
					//	{
					//		// Sector edge is separation line - there's no way the edges considered can overshadow
					//		mask = 100;
					//		break;
					//	}
					//}
					//if( sa1>=0 && sa2>=0 && (sa1 || sa2) )
					//{
					//	if( sb1<=0 && sb2<=0 && (sb1 || sb2) )
					//	{
					//		// Sector edge is separation line - there's no way the edges considered can overshadow
					//		mask = 100;
					//		break;
					//	}
					//}
				}

				if( !mask )
				{
					line_order[a].push_back(b);
					b->_locknum++;
				}
			}

	// Compule valid order
	bool warned = false;
	bool nocheck = false;
	while( numlines>0 )
	{
		bool work = false;

		for( auto p=reg->vis.begin(); p!=reg->vis.end(); ++p )
			if( !p->first->_mark && (nocheck || !p->first->_locknum) )
			{
				p->second->draw_order = (int)reg->drawlist.size();
				reg->drawlist.push_back(p->second);
				p->first->_mark = 1;
				numlines--;
				work = true;
				nocheck = false;

				vector<Line*> &locks = line_order[p->first];
				for( int i=0; i<(int)locks.size(); i++ )
					locks[i]->_locknum--;
			}

		if( !work )
		{
			if( check ) return false;

			if( !warned )
			{
				warned = true;
			
				vec2 mid = reg->MidPoint();
				printf("WARNING: No valid line order for visibility region at (%.0f,%.0f)!!!\n", mid.x, mid.y);
				break;
			}
			nocheck = true;
		}
	}

	return !warned;
#endif
}

void DreadMapGen::MarkSimplifyNeighbors(RegionPoly *region)
{
	for( int i=0; i<(int)regions.polys.size(); i++ )
	{
		regions.polys[i]->Update();
		regions.polys[i]->marked = false;
	}

	int N = (int)region->poly.edges.size();

	// Scan all neighboring regions
	for( int i=0; i<(int)regions.polys.size(); i++ )
	{
		RegionPoly *reg = regions.polys[i];
		if( reg==region || !reg->poly.IsBoxCollision(region->poly) )
			continue;

		if( reg->sector != region->sector ) continue;
		if( reg->bsp_subsector != region->bsp_subsector ) continue;

		// Find connecting edges
		int N2 = (int)reg->poly.edges.size();
		int a, b;
		for( a=0; a<N; a++ )
			for( b=0; b<N2; b++ )
				if( region->poly.edges[a].start.IsSameAs(reg->poly.edges[(b+1)%N2].start) &&
					region->poly.edges[(a+1)%N].start.IsSameAs(reg->poly.edges[b].start) )
				{
					goto found;
				}
	found:
		if( a>=N ) continue;	// no common edge

								// Check if merge result would be convex
		if( region->poly.edges[(a+N-1)%N].line.CheckSide(reg->poly.edges[(b+2)%N2].start) < 0 ) continue;
		if( region->poly.edges[(a+1)%N].line.CheckSide(reg->poly.edges[(b+N2-1)%N2].start) < 0 ) continue;
		if( reg->poly.edges[(b+N2-1)%N2].line.CheckSide(region->poly.edges[(a+2)%N].start) < 0 ) continue;
		if( reg->poly.edges[(b+1)%N2].line.CheckSide(region->poly.edges[(a+N-1)%N].start) < 0 ) continue;

		reg->marked = true;
	}
}

void DreadMapGen::SimplifyRegions()
{
	// Update / reset
	double total_surface = 0;
	for( int i=0; i<(int)regions.polys.size(); i++ )
	{
		regions.polys[i]->Update();
		regions.polys[i]->marked = false;
		regions.polys[i]->_priority = 1;
		total_surface += regions.polys[i]->surface;
	}

	vector<RegionEdge> reged;
	int max_regions = sectors.size();
	max_regions += (int)floor(total_surface/(256*256)+0.5f);
	//max_regions = 1;

	int scancount = 0;
	while( regions.polys.size() > max_regions)
	{
		// Find smallest unmarked region
		RegionPoly *region = NULL;
		int region_i = 0;
		{
			double best_score = 1e38f;
			for( int i=0; i<(int)regions.polys.size(); i++ )
			{
				RegionPoly *reg = regions.polys[i];
				double score = reg->surface;
				if( reg->marked || score >= best_score )
					continue;

				region = reg;
				region_i = i;
				best_score = score;
			}
		}

		if( !region )
		{
			printf("No more regions to simplify (scancount %d).\n", scancount);
			break;
		}

		int N = (int)region->poly.edges.size();

		// Scan all neighboring regions
		reged.clear();
		for( int i=0; i<(int)regions.polys.size(); i++ )
		{
			RegionPoly *reg = regions.polys[i];
			if( reg==region || !reg->poly.IsBoxCollision(region->poly) )
				continue;

			if( reg->sector != region->sector ) continue;
			if( reg->bsp_subsector != region->bsp_subsector ) continue;

			// Find connecting edges
			int N2 = (int)reg->poly.edges.size();
			int a, b;
			for( a=0; a<N; a++ )
				for( b=0; b<N2; b++ )
					if( region->poly.edges[a].start.IsSameAs(reg->poly.edges[(b+1)%N2].start) &&
						region->poly.edges[(a+1)%N].start.IsSameAs(reg->poly.edges[b].start) )
					{
						goto found;
					}
		found:
			if( a>=N ) continue;	// no common edge
		
			// Check if merge result would be convex
			if( region->poly.edges[(a+N-1)%N].line.CheckSide(reg->poly.edges[(b+2)%N2].start) < 0 ) continue;
			if( region->poly.edges[(a+1)%N].line.CheckSide(reg->poly.edges[(b+N2-1)%N2].start) < 0 ) continue;
			if( reg->poly.edges[(b+N2-1)%N2].line.CheckSide(region->poly.edges[(a+2)%N].start) < 0 ) continue;
			if( reg->poly.edges[(b+1)%N2].line.CheckSide(region->poly.edges[(a+N-1)%N].start) < 0 ) continue;

			RegionEdge re;
			re.other = reg;
			re.a = a;
			re.b = b;
			reged.push_back(re);
		}
		//printf("%3d: %d\n", region_i, reged.size());

		//vec2 mid = region->MidPoint();
		//printf("Region at (%.0f,%.0f) -> %d neighbors\n", mid.x, mid.y, reged.size());

		// Sort neighbors by size
		sort(reged.begin(), reged.end());

		// Try merging with neighbors
		bool merged = false;
		for( int i=0; i<(int)reged.size(); i++ )
		{
			const RegionEdge &ree = reged[i];
			RegionPoly *newreg = new RegionPoly(*region);	// copy self
			RegionPoly *other = ree.other;

			// Merge visibility lists
			bool ok = true;
			for( auto p=other->vis.begin(); ok && p!=other->vis.end(); ++p )
			{
				if( !newreg->vis[p->first] )
					newreg->vis[p->first] = new VisLine(*p->second);
				else
					if( newreg->vis[p->first]->cheat_mode != p->second->cheat_mode )
					{
						ok = false;
						break;
					}
			}

			if( !ok )
			{
				//printf("?????????????????????\n");
				delete newreg;
				continue;
			}

			// Erase common edge
			newreg->poly.edges.erase(newreg->poly.edges.begin()+ree.a);
																			
			// Insert other polygon
			{
				int N2 = (int)other->poly.edges.size();
				int ip = ree.a;
				for( int e = (ree.b+1)%N2; e!=ree.b; e=(e+1)%N2 )
					newreg->poly.edges.insert(newreg->poly.edges.begin()+(ip++), other->poly.edges[e]);

				newreg->poly.CleanupColinear();
			}

			// Check line visibility restrictions
			//if( !ComputeLineOrder(newreg, true) )
			//{
			//	delete newreg;
			//	continue;
			//}
			
			
			// Done
			if( 0 )
			{
				vec2 mid = region->MidPoint();
				vec2 mid2 = other->MidPoint();
				printf("MERGED: (%.0f,%.0f) with (%.0f,%.0f)\n", mid.x, mid.y, mid2.x, mid2.y);
			}

			newreg->_priority = region->_priority + other->_priority;

			regions.DeleteRegion(region);
			regions.DeleteRegion(other);

			newreg->Update();
			regions.polys.push_back(newreg);
			merged = true;
			//printf("!!!!\n");
			break;
		}

		if( !merged )
		{
			region->marked = true;	// don't try again
			scancount++;
		}
		else
		{
			// Clear all markers
			for( int i=0; i<(int)regions.polys.size(); i++ )
			{
				//regions.polys[i]->Update();
				regions.polys[i]->marked = false;
			}
			scancount = 0;
		}
	}
	// } while(0);

	printf("Simplified to %d regions.\n", regions.polys.size());
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
	Polygon2D poly;
	
	poly.CreateRectangle(-REGION_SIZE, -REGION_SIZE, REGION_SIZE, REGION_SIZE);
	regions.bsproot = BuildBSPForPoly(poly, regions.polys, true);

	for( int i=0; i<(int)regions.polys.size(); i++ )
		regions.polys[i]->bsp_subsector = regions.polys[i]->bsp_leaf;


	printf("BSP built %d nodes.\n", regions.nodes.size());
}

void DreadMapGen::ComputeFinalBSPTree()
{
	vector<RegionPoly*> regs;

	int num_nodes = (int)regions.nodes.size();
	int nsub = 0;
	for( int i=0; i<num_nodes; i++ )
	{
		BSPNode *subroot = regions.nodes[i];
		if( !subroot->region )
			continue;	// Not a subroot

		subroot->region = NULL;		// outdated by now

		// Build list of regions to be separated
		regs.clear();
		double total_surface = 0;
		for( int j=0; j<(int)regions.polys.size(); j++ )
			if( regions.polys[j]->bsp_subsector==subroot )
			{
				regs.push_back(regions.polys[j]);
				regions.polys[j]->Update();
				total_surface += regions.polys[j]->surface;
			}

		if( !regs.size() )
		{
			printf("Empty BSP subregion!\n");
			continue;
		}

		Polygon2D poly;
		//poly.CreateRectangle(-REGION_SIZE, -REGION_SIZE, REGION_SIZE, REGION_SIZE);
		MakePolySector(regs[0]->sector, poly);

		//BSPNode *newroot = BuildBSPForPoly(poly, regs, false);
		int depth = 0;
		if( total_surface >= 128*64  ) depth = 1;
		if( total_surface >= 256*256 ) depth = 2;
		BSPNode *newroot = BuildVisibilityBSP(poly, regs, depth);

		// Delete old regions
		for( int j=0; j<(int)regs.size(); j++ )
			regions.DeleteRegion(regs[j]);

		if( newroot==NULL )
		{
			printf("ERROR: BSP subtree rebuild failed.\n");
			continue;
		}

		// Turn subroot into newroot
		*subroot = *newroot;
		if( subroot->region )
			subroot->region->bsp_leaf = subroot;
		else
		{
			if( !subroot->left ) printf("A !!!!!!!!!!!!!!!!!!!!!!!\n");
			if( !subroot->right ) printf("B !!!!!!!!!!!!!!!!!!!!!!!\n");
		}

		// newroot is left unused
		newroot->left = NULL;
		newroot->right = NULL;
		newroot->region = NULL;
		nsub++;
	}

	regions.bsproot = CleanupBSPRec(regions.bsproot);

	// Cleanup
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

	printf("Cleanup %d (expected %d).\n", ndel, nsub);
	printf("Final BSP has %d nodes (depth %d).\n", regions.nodes.size(), CheckBSPDepth(regions.bsproot));
}

// Build BSP tree for regions from the list, that are overlapping the polygon.
DreadMapGen::BSPNode *DreadMapGen::BuildBSPForPoly(const Polygon2D &poly, const vector<RegionPoly*> reglist, bool cut_regions)
{
	if( poly.IsEmpty() )
	{
		printf("ERROR: Empty subregion. Can't build BSP tree!\n");
		return NULL;
	}

	// Collect regions
	vector<RegionPoly*> regs;
	for( RegionPoly *reg = regions.SelectRegionFromList(poly, reglist, cut_regions); reg; reg = reg->next )
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
	vector<RegionPoly*> best_left;
	vector<RegionPoly*> best_right;
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

			float score = log2(float(cur_left.size())) + log2(float(cur_right.size())) + n_split*2/10.f;
			if( score < best_score )
			{
				best_reg = reg;
				best_edge = e;
				best_score = score;
				best_left.swap(cur_left);
				best_right.swap(cur_right);
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

	node->left = BuildBSPForPoly(left, best_left, cut_regions);
	node->right = BuildBSPForPoly(right, best_right, cut_regions);
	if( !node->left ) printf("!!!!!!!!!!!!!!!!!!!!!!!\n");
	if( !node->right ) printf("!!!!!!!!!!!!!!!!!!!!!!!\n");

	return node;
}

DreadMapGen::BSPNode *DreadMapGen::BuildVisibilityBSP(const Polygon2D &poly, const vector<RegionPoly*> reglist, int depth)
{
	if( poly.IsEmpty() )
	{
		printf("ERROR: Empty subregion. Can't build BSP tree!\n");
		return NULL;
	}

	// Collect regions
	vector<RegionPoly*> regs;
	for( RegionPoly *reg = regions.SelectRegionFromList(poly, reglist, false); reg; reg = reg->next )
		regs.push_back(reg);

	// Handle leaves
	if( regs.size()==1 || depth<=0 )
		return BuildVisibilityLeaf(poly, reglist);

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

	if( !best_reg || best_score < 128*128 )
	{
		return BuildVisibilityLeaf(poly, reglist);
	}

	BSPNode *node = new BSPNode();
	regions.nodes.push_back(node);
	node->line = best_reg->poly.edges[best_edge].line;

	Polygon2D left, right;
	poly.SplitByLine(node->line, right, left);

	node->left = BuildVisibilityBSP(left, best_left, depth-1);
	node->right = BuildVisibilityBSP(right, best_right, depth-1);

	return node;
}

DreadMapGen::BSPNode *DreadMapGen::BuildVisibilityLeaf(const Polygon2D &poly, const vector<RegionPoly*> reglist)
{
	if( poly.IsEmpty() )
	{
		printf("ERROR: Empty subregion. Can't build BSP leaf!\n");
		return NULL;
	}

	if( reglist.size()<=0 )
	{
		printf("ERROR: Empty region list. Can't build BSP leaf!\n");
		return NULL;
	}

	BSPNode *leaf= new BSPNode();
	regions.nodes.push_back(leaf);

	RegionPoly *newreg = new RegionPoly();
	regions.polys.push_back(newreg);
	newreg->sector = reglist[0]->sector;
	newreg->poly = poly;
	newreg->bsp_subsector = reglist[0]->bsp_subsector;
	newreg->bsp_leaf = leaf;

	bool ok = true;
	for( int i=0; i<(int)reglist.size(); i++ )
	{
		RegionPoly *reg = reglist[i];
		for( auto p=reg->vis.begin(); p!=reg->vis.end(); ++p )
		{
			if( !newreg->vis[p->first] )
				newreg->vis[p->first] = new VisLine(*p->second);
			else
				if( newreg->vis[p->first]->cheat_mode != p->second->cheat_mode )
				{
					if( ok ) printf("ERROR: Bad visibility list merge!\n");
					ok = false;
				}
		}
	}

	newreg->Update();

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
	if( reg->_vis_start < 0 )
	{
		reg->_vis_start = (int)exp.vis.size();
	
		for( int i=0; i<(int)reg->drawlist.size(); i++ )
		{
			VisLine *vis = reg->drawlist[i];

			Line *line = vis->line;
			if( line->_number < 0 )
			{
				line->_number = (int)exp.lines.size();
				exp.lines.push_back(line);

				if( line->v1->_number < 0 )
				{
					line->v1->_number = (int)exp.verts.size();
					exp.verts.push_back(line->v1);
				}

				if( line->v2->_number < 0 )
				{
					line->v2->_number = (int)exp.verts.size();
					exp.verts.push_back(line->v2);
				}
			}

			exp.vis.push_back(line->_number);
			exp.vis.push_back(vis->cheat_mode);

			if( line->tex_upper ) exp.tex_set.insert(line->tex_upper);
			if( line->tex_lower ) exp.tex_set.insert(line->tex_lower);
		}

		exp.vis.push_back(0xFFFF);
	}

	node->_number = ~reg->_vis_start;
}

void DreadMapGen::ExportGather(vector<int> &tex_list)
{
	exp.Clear();
	tex_list.clear();

	for( int i=0; i<(int)verts.size(); i++ )
		verts[i]->_number = -1;

	for( int i=0; i<(int)lines.size(); i++ )
		lines[i]->_number = -1;

	for( int i=0; i<(int)regions.nodes.size(); i++ )
		regions.nodes[i]->_number = -1;

	for( int i=0; i<(int)regions.polys.size(); i++ )
		regions.polys[i]->_vis_start = -1;

	// Gather
	ExportBSPRec(regions.bsproot, exp);
	
	for( auto p=exp.tex_set.begin(); p!=exp.tex_set.end(); ++p )
		tex_list.push_back(*p);
}

void DreadMapGen::ExportWrite(FILE *fp, GfxConverter &gconv)
{
	// Write vertexes
	fprintf(fp, "\nconst DataVertex MAP_VTX[] ={\n");
	for( int i=0; i<(int)exp.verts.size(); i++ )
		fprintf(fp, "\t{ %5d, %5d },\t// %4d\n", exp.verts[i]->xp, exp.verts[i]->yp, i);
	fprintf(fp, "\t{ 0x7FFF, 0x7FFF }");
	fprintf(fp, "};\n");

	// Write lines
	fprintf(fp, "\nconst DataLine MAP_LINES[] ={\n");
	for( int i=0; i<(int)exp.lines.size(); i++ )
	{
		// { 2, 3, TEX_RW23_4_OFFS, -128, -64, 0x40, 0xC0, 6 },

		Line &l = *exp.lines[i];
		fprintf(fp, "\t{ %5d, %5d,", l.v1->_number, l.v2->_number);

		string tname = "0,";
		if( l.tex_upper )
			tname = gconv.textures[l.tex_upper]->GetOffsetName() + ",";
		fprintf(fp, " %-32s", tname.c_str());

		fprintf(fp, "%5d,%5d,", l.h1, l.h2);

		fprintf(fp, " 0x%02X,", l.tex_ceil ? _color2chunky(gconv.textures[l.tex_ceil]->texture.data[0]) : 0);
		fprintf(fp, " 0x%02X,", l.tex_floor ? _color2chunky(gconv.textures[l.tex_floor]->texture.data[0]) : 0);

		fprintf(fp, "  0, },\t// %4d\n", i);
	}
	fprintf(fp, "\t{ 0xFFFF, 0, 0, 0, 0, 0, 0 }\n");
	fprintf(fp, "};\n");

	// Write BSP nodes
	fprintf(fp, "\nconst DataBSPNode MAP_NODES[] ={\n");
	for( int i=0; i<(int)exp.nodes.size(); i++ )
	{
		BSPNode &b = *exp.nodes[i];
		int A = -b.line.normal.x;
		int B = -b.line.normal.y;
		int C = (-b.line.normal.z) << 4;	// input coords are fix.4

		if( A<-32767 || A>32767 || B<-32767 || B>32767 )
			printf("ERROR: BSP line vector magnitude.\n");

		int left=b.left->_number, right=b.right->_number;
		if( left<-32767 || right<-32767 )
			printf("ERROR: BSP line draw list overflow.\n");
		if( left>32767 || right>32767 )
			printf("ERROR: BSP node count overflow.\n");

		fprintf(fp, "\t{ %6d, %6d, %8d,", A, B, C);
		if( left>=0 )	fprintf(fp, " %6d,", left);
		else			fprintf(fp, " %6s,", format("~%d", ~left).c_str());
		if( right>=0 )	fprintf(fp, " %6d,", right);
		else			fprintf(fp, " %6s,", format("~%d", ~right).c_str());
		fprintf(fp, " },\t// %4d\n", i);
	}
	fprintf(fp, "};\n");

	// Write draw list
	fprintf(fp, "\nconst word MAP_VIS[] ={\n");
	for( int i=0; i<(int)exp.vis.size(); i++ )
	{
		if( (i+1)<(int)exp.vis.size() && exp.vis[i]!=0xFFFF )
		{
			fprintf(fp, "\t%6d,%6d,\t// %4d\n", exp.vis[i], exp.vis[i+1], i);
			i++;
		}
		else
			fprintf(fp, "\t%6d,\t\t\t// %4d\n", exp.vis[i], i);
	}
	fprintf(fp, "};\n");
}
