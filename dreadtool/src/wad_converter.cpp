
#include "common.h"
#include "app_rendertest.h"



static string _doom2string(const char *s, int max_len)
{
	int len = 0;
	while( s[len] && len<max_len )
		len++;
	return string(s, s+len);
}






WadConverter::WadConverter()
{
}


// Returns relative order of v0-v1 and v0-v2 segments, results:
//	negative	- left (v1->v2 goes counter-clockwise around v0)
//	zero		- colinear
//	positive	- right (v1->v2 goes clockwise around v0)
int WadConverter::FanOrder(int v0, int v1, int v2)
{
	if( v0<0 || v0>=vertexes.size() ) throw string("Invalid vertex index");
	if( v1<0 || v1>=vertexes.size() ) throw string("Invalid vertex index");
	if( v2<0 || v2>=vertexes.size() ) throw string("Invalid vertex index");

	// ^ Y
	// |  X
	// +-->

	int dx1 = vertexes[v1].xpos - vertexes[v0].xpos;
	int dy1 = vertexes[v1].ypos - vertexes[v0].ypos;
	int dx2 = vertexes[v2].xpos - vertexes[v0].xpos;
	int dy2 = vertexes[v2].ypos - vertexes[v0].ypos;
	return dx2*dy1 - dx1*dy2;
}

// Returns direction in which v1-v2-v3 line bends, results:
//	negative	- left (concave)
//	zero		- colinear
//	positive	- right (convex)
int WadConverter::BendDirection(int v1, int v2, int v3)
{
	if( v1<0 || v1>=vertexes.size() ) throw string("Invalid vertex index");
	if( v2<0 || v2>=vertexes.size() ) throw string("Invalid vertex index");
	if( v3<0 || v3>=vertexes.size() ) throw string("Invalid vertex index");

	// ^ Y
	// |  X
	// +-->

	int dx1 = vertexes[v2].xpos - vertexes[v1].xpos;
	int dy1 = vertexes[v2].ypos - vertexes[v1].ypos;
	int dx2 = vertexes[v3].xpos - vertexes[v2].xpos;
	int dy2 = vertexes[v3].ypos - vertexes[v2].ypos;
	return dx2*dy1 - dx1*dy2;
}

// Returns next halfline index, for a line v1-v2 (only convex angles)
int WadConverter::FindNextHalfline(int v1, int v2)
{
	// Next line
	Vertex &vt2 = vertexes[v2];
	int best = -1;
	for( int i=0; i<vt2.outgoing.size(); i++ )
	{
		HalfLine &htest = halflines[vt2.outgoing[i]];
		if( BendDirection(v1, v2, htest.v2)<0 )
			continue;	// concave angle
		if( best>=0 && FanOrder(v2, halflines[best].v2, htest.v2)<0 )
			continue;	// left of currently best line
		if( htest.v2 == v1 )
			continue;	// other side of the same linedef
		best = vt2.outgoing[i];
	}

	return best;
}


// Returns whether a list of vertexes forms a convex shape
bool WadConverter::IsShapeConvex(const vector<int> &vtx)
{
	for( int i=0; i<(int)vtx.size(); i++ )
		if( BendDirection(vtx[i], vtx[(i+1)%vtx.size()], vtx[(i+2)%vtx.size()]) < 0 )
			return false;

	return true;
}


// Find vertex that lies in the interior of a convex shape, or -1
int WadConverter::FindVertexInConvexShape(const vector<int> &vtx)
{
	for( int i=0; i<(int)vertexes.size(); i++ )
	{
		bool found = true;
		for( int j=0; j<(int)vtx.size(); j++ )
			if( FanOrder(vtx[j], vtx[(j+1)%vtx.size()], i) <= 0 )
			{
				found = false;
				break;
			}

		if( found )
			return i;
	}
	return -1;
}


int WadConverter::LineLength(int v1, int v2)
{
	if( v1<0 || v1>=vertexes.size() ) throw string("Invalid vertex index");
	if( v2<0 || v2>=vertexes.size() ) throw string("Invalid vertex index");
	int dx = vertexes[v2].xpos - vertexes[v1].xpos;
	int dy = vertexes[v2].ypos - vertexes[v1].ypos;
	return int(sqrt(dx*dx+dy*dy)+0.5f);
}


int WadConverter::AssureVertex(int vid)
{
	auto p = vertex_remap.find(vid);
	if( p!=vertex_remap.end() )
		return p->second;

	if( vid<0 || vid>=wadmap->n_vertexes )
		throw string("Vertex index out of range");

	WadFile::Vertex *wv = wadmap->vertexes + vid;
	Vertex v;
	v.xpos = wv->x;
	v.ypos = wv->y;
	vertexes.push_back(v);
	vertex_remap[vid] = vertexes.size()-1;

	return vertexes.size()-1;
}

void WadConverter::LoadLine(int lid)
{
	WadFile::LineDef *line = wadmap->linedefs + lid;

	int pindex = -1;
	for( int side=0; side<2; side++ )
	{
		if( side && !(line->flags & WadFile::LineDef::TWOSIDED) )
			break;

		int sid = side ? line->sd_left : line->sd_right;
		if( sid<0 || sid>=wadmap->n_sidedefs )
			throw string("Sidedef index out of range");

		HalfLine hl = {};
		hl.v1 = AssureVertex(side ? line->end : line->start);
		hl.v2 = AssureVertex(side ? line->start : line->end);
		hl.other = pindex;
		hl.linedef = line;
		hl.sidedef = wadmap->sidedefs + sid;
		hl.sector = -1;
		hl.wad_sector = hl.sidedef->sector;
		hl.linedef_side = side;

//		if( side==0 )
//			printf("wad line %3d: %d-%d  sector %d", lid, hl.v1, hl.v2, hl.wad_sector);
//		else
//			printf("/%d", hl.wad_sector);

		// link sides
		int index = halflines.size();
		halflines.push_back(hl);
		if( pindex>=0 )
			halflines[pindex].other = index;
		pindex = index;

		// save in vertexes
		vertexes[hl.v1].outgoing.push_back(index);
		vertexes[hl.v2].incoming.push_back(index);
	}

//	printf("\n");
}

void WadConverter::LoadSector(int sid)
{
	WadFile::Sector *wsec = wadmap->sectors + sid;

	// Create sector
	Sector sec;
	sec.floor_h = wsec->floor_h;
	sec.ceil_h = wsec->ceil_h;
	sec.floor_tex = _doom2string(wsec->ftex, 8);
	sec.ceil_tex = _doom2string(wsec->ctex, 8);
	sec.sector = wsec;

	//printf("Sector %3d: %4d %4d  %-8s %-8s :", sid, sec.floor_h, sec.ceil_h, sec.floor_tex.c_str(), sec.ceil_tex.c_str());


	// Find first sector line
	int first_line = -1;
	for( int i=0; i<halflines.size(); i++ )
		if( halflines[i].wad_sector==sid )
		{
			if( halflines[i].used )
				throw format("First halfline in sector %d already used", sid);
			first_line = i;
			break;
		}

	if( first_line == -1 )
		throw format("Sector %d with no linedefs", sid);

	// Gather all sector lines
	int lineidx = first_line;
	while( 1 )
	{
		HalfLine &hline = halflines[lineidx];

		// Mark as used
		if( hline.used )
			throw format("Invalid sector %d (halfline in sector already used)", sid);
		hline.used = true;

		//printf(" %d%s", hline.linedef - wadmap->linedefs, hline.linedef_side ? "B" : "");

		// Save
		sec.halflines.push_back(lineidx);
		hline.sector = sectors.size();

		// Next line
		//Vertex &v2 = vertexes[hline.v2];
		//int best = -1;
		//for( int i=0; i<v2.outgoing.size(); i++ )
		//{
		//	HalfLine &htest = halflines[v2.outgoing[i]];
		//	if( BendDirection(hline.v1, hline.v2, htest.v2)<0 )
		//		continue;	// concave angle
		//	if( best>=0 && FanOrder(hline.v2, halflines[best].v2, htest.v2)<0 )
		//		continue;	// left of currently best line
		//	if( hline.linedef == htest.linedef )
		//		continue;	// other side of the same linedef
		//	best = v2.outgoing[i];
		//}

		lineidx = FindNextHalfline(hline.v1, hline.v2);
		if( lineidx<0 )
			throw format("Sector %d not convex or open", sid);

		//lineidx = best;

		// Done?
		if( lineidx==first_line )
			break;
	}

	// Save
	sectors.push_back(sec);

	//printf("\n");
}

void WadConverter::LoadComplexSector(int sid)
{
	WadFile::Sector *wsec = wadmap->sectors + sid;

//	printf("Loading sector %d:\n", sid);

	// Gather all sector halflines
	vector<int> base_halflines;						// all initial halflines, their ends are used to index all used vertexes (might reuse same vertex multiple times)
	for( int i=0; i<halflines.size(); i++ )
		if( halflines[i].wad_sector==sid )
		{
			//halflines[i].used = false;
			base_halflines.push_back(i);
		}
	vector<int> active_halflines = base_halflines;		// all active halflines, including the newly added ones

	// Build subsectors
	while( true )
	{
		// Find first unused halfline
		int start_hl = -1;
		for( int i=0; i<base_halflines.size(); i++ )
			if( !halflines[base_halflines[i]].used )
			{
				start_hl = base_halflines[i];
				break;
			}

		if( start_hl<0 )
			break;
		
		// Prepare sector structure
		sectors.resize(sectors.size()+1);
		Sector &sec = sectors.back();
		sec.floor_h = wsec->floor_h;
		sec.ceil_h = wsec->ceil_h;
		sec.floor_tex = _doom2string(wsec->ftex, 8);
		sec.ceil_tex = _doom2string(wsec->ctex, 8);
		sec.sector = wsec;
		sec.halflines.clear();

		// Add first halfline
		const HalfLine *first_hl = &halflines[start_hl];
		sec.halflines.push_back(start_hl);
		halflines[start_hl].used = true;
		halflines[start_hl].sector = sectors.size()-1;

//		printf("  %d-%d", first_hl->v1, first_hl->v2);

		// Prepare active vertex list
		for( int i=0; i<base_halflines.size(); i++ )
		{
			int vid = halflines[base_halflines[i]].v2;
			if( vid != first_hl->v1 && vid != first_hl->v2 && BendDirection(first_hl->v1, first_hl->v2, vid) >= 0 )
				vertexes[vid].used = true;
			else
				vertexes[vid].used = false;
		}

		// Build the sector
		int current_hl = start_hl;
		while( true )
		{
			// Add the halfline found
			const HalfLine *hl = &halflines[current_hl];

			// Find potential next halfline - next halfline in the sector forming convex angle
			int next_hl = FindNextHalfline(hl->v1, hl->v2);
			const HalfLine *nhl = (next_hl>=0) ? &halflines[next_hl] : NULL;
			int best_vertex = nhl && vertexes[nhl->v2].used ? nhl->v2 : -1;

			if( nhl && BendDirection(nhl->v1, nhl->v2, first_hl->v1) < 0 )
				nhl = NULL;	// using this halfline won't let us close the sector

			// Go through all active vertexes
			for( int i=0; i<base_halflines.size(); i++ )
			{
				int vid = halflines[base_halflines[i]].v2;
				if( !vertexes[vid].used )
					continue;	// already culled
				if( nhl && BendDirection(nhl->v1, nhl->v2, vid) < 0 )
				{
					vertexes[vid].used = false;
					continue;	// can't be used - next halfline doesn't allow that; cull it
				}
				if( best_vertex<0 || BendDirection(best_vertex, vid, first_hl->v1) < 0 )
				{
					// Check if we can add halfline (hl->v2, vid) without intersecting any other halflines
					bool all_ok = true;
					for( int j=0; j<(int)active_halflines.size() && all_ok; j++ )
					{
						const HalfLine &test = halflines[active_halflines[j]];
						int n1s = BendDirection(test.v1, test.v2, hl->v2);
						int n2s = BendDirection(test.v1, test.v2, vid);
						int t1s = BendDirection(hl->v2, vid, test.v1);
						int t2s = BendDirection(hl->v2, vid, test.v2);
						if( n1s<0 && n2s<0 ) continue;
						if( n1s>0 && n2s>0 ) continue;
						if( t1s<0 && t2s<0 ) continue;
						if( t1s>0 && t2s>0 ) continue;
						if( n1s<0 && n2s>0 ) all_ok = false;
						if( n1s>0 && n2s<0 ) all_ok = false;
						if( t1s<0 && t2s>0 ) all_ok = false;
						if( t1s>0 && t2s<0 ) all_ok = false;
					}

					if( all_ok )
						best_vertex = vid;	// this vertex prevents forming the sector - use it instead		(or no best vertex found yet)
				}
			}

			// Close the sector if no next vertex found
			if( best_vertex < 0 )
				best_vertex = first_hl->v1;

			// Create edge
			if( !nhl || best_vertex!=nhl->v2 )
			{
				WadFile::LineDef *linedef = new WadFile::LineDef();			gen_linedefs.push_back(linedef);
				WadFile::SideDef *sidedef0 = new WadFile::SideDef();		gen_sidedefs.push_back(sidedef0);
				WadFile::SideDef *sidedef1 = new WadFile::SideDef();		gen_sidedefs.push_back(sidedef1);
				memset(linedef, 0, sizeof(*linedef));
				memset(sidedef0, 0, sizeof(*sidedef0));
				memset(sidedef1, 0, sizeof(*sidedef1));

				HalfLine h1 = {}, h2 = {};
				int nh1 = halflines.size();
				int nh2 = halflines.size() + 1;
				h1.v1 = h2.v2 = hl->v2;
				h1.v2 = h2.v1 = best_vertex;
				h1.other = nh2;
				h2.other = nh1;
				h1.wad_sector = h2.wad_sector = first_hl->wad_sector;
				h1.sector = h2.sector = -1;
				h1.linedef_side = 0;
				h2.linedef_side = 1;
				vertexes[h1.v1].outgoing.push_back(nh1);
				vertexes[h1.v2].incoming.push_back(nh1);
				vertexes[h2.v1].outgoing.push_back(nh2);
				vertexes[h2.v2].incoming.push_back(nh2);

//				printf("(%d-%d)", h1.v1, h1.v2);


				halflines.push_back(h1);
				halflines.push_back(h2);
				active_halflines.push_back(nh1);
				active_halflines.push_back(nh2);
				first_hl = &halflines[start_hl];		// refresh pointer
				current_hl = nh1;
			}
			else
			{
//				printf("-%d", halflines[next_hl].v2);
				current_hl = next_hl;
			}

			// Use the next halfline
			sec.halflines.push_back(current_hl);
			if( halflines[current_hl].used )
			{
				printf("ERROR: sector %d used halfline %d that is already used\n", sectors.size()-1, current_hl);
				//DebugDumpInternals();
				break;
			}
			if( halflines[current_hl].wad_sector != sid )
			{
				printf("ERROR: sector %d used halfline %d that faces wad sector %d (instead of %d)\n", sectors.size()-1, current_hl, halflines[current_hl].wad_sector, sid);
				//DebugDumpInternals();
				break;
			}
			halflines[current_hl].used = true;
			halflines[current_hl].sector = sectors.size()-1;

			// Done?
			hl = &halflines[current_hl];
			if( hl->v2 == first_hl->v1 )
				break;	// Done

			// Cull vertexes that can no longer affect us
			for( int i=0; i<base_halflines.size(); i++ )
			{
				int vid = halflines[base_halflines[i]].v2;
				if( BendDirection(hl->v1, hl->v2, vid) < 0 )
					vertexes[vid].used = false;
			}
			vertexes[hl->v2].used = false;
		}

//		printf("\n");
	}
}

void WadConverter::PrepareFileData()
{
	// Copy vertexes
	for( int i=0; i<vertexes.size(); i++ )
	{
		Vertex &v = vertexes[i];
		MapFileVertex fv = {};
		fv.xp = short(v.xpos);
		fv.yp = short(v.ypos);

		if( fv.xp != v.xpos || fv.yp != v.ypos )
			throw format("Vertex at %d,%d outside range limit", v.xpos, v.ypos);

		m_vertexes.push_back(fv);
	}

	// Generate sectors & their lines
	for( int i=0; i<sectors.size(); i++ )
	{
		Sector &sec = sectors[i];
		MapFileSector fs = {};
		int light = sec.sector->light;
		fs.first_line = word(m_lines.size());
		fs.num_lines = word(sec.halflines.size());		// unchecked, a single convex, closed sector with more than 65k lines is unlikely
		fs.floor_h = sec.floor_h;
		fs.ceil_h = sec.ceil_h;
		fs.light_level = light;
		fs.floor_tex = sectors[i].floor_tex.c_str();
		fs.ceil_tex = sectors[i].ceil_tex.c_str();
		fs.type = sec.sector->type;
		fs.tag = sec.sector->tag;

		// Sector lines
		for( int j=0; j<sec.halflines.size(); j++ )
		{
			HalfLine &hl = halflines[sec.halflines[j]];
			MapFileLine fl = {};
			fl.vertex = hl.v1;
			fl.light_level = light;
			if( hl.other>=0 )
			{
				fl.upper_tex = hl.sidedef ? _doom2string(hl.sidedef->upper, 8) : "";
				fl.lower_tex = hl.sidedef ? _doom2string(hl.sidedef->lower, 8) : "";
			}
			else
			{
				fl.upper_tex = hl.sidedef ? _doom2string(hl.sidedef->middle, 8) : "";
				fl.lower_tex = hl.sidedef ? _doom2string(hl.sidedef->lower, 8) : "";
			}
			// TBD: texture offsets
			fl.xcoord1 = hl.sidedef ? hl.sidedef->xoffs & 255 : 0;
			fl.xcoord2 = fl.xcoord1 + LineLength(hl.v1, hl.v2);
			fl.ycoord = hl.sidedef ? hl.sidedef->yoffs : 0;
			fl.line_action = (hl.linedef ? hl.linedef->type : 0) | word(hl.linedef_side<<15);
			fl.line_tag = hl.linedef ? hl.linedef->tag : 0;
			fl.line_flags = hl.linedef ? hl.linedef->flags : 0;
			fl.other_sector = (hl.other>=0 ? halflines[hl.other].sector : 0xFFFF);
			m_lines.push_back(fl);
		}

		m_sectors.push_back(fs);
	}

	//// Process BSP nodes
	//for( int i=0; i<wadmap->n_nodes; i++ )
	//{
	//	WadFile::Node *node = wadmap->nodes + i;
	//	MapFileNode fn = {};
	//	// Node line function:
	//	//	Ax + By - C
	//	//		negative		- left
	//	//		zero			- on line
	//	//		positive		- right
	//	// ^ Y
	//	// |  X
	//	// +-->
	//	fn.A = -node->dy;
	//	fn.B = node->dx;
	//	fn.C = int(node->xp)*fn.A + int(node->yp)*fn.B;
	//	
	//	for( int side=0; side<2; side++ )
	//	{
	//		short sub = (side ? node->lchild : node->rchild);
	//		if( sub<0 )
	//		{
	//			WadFile::Segment &seg = wadmap->segments[wadmap->subsectors[sub & 0x7FFF].first_seg];
	//			WadFile::LineDef &linedef = wadmap->linedefs[seg.linedef];
	//			WadFile::SideDef &sdef = wadmap->sidedefs[seg.dir ? linedef.sd_left : linedef.sd_right];
	//			sub = sdef.sector | 0x8000;
	//		}
	//		*(side ? &fn.left : &fn.right) = sub;
	//	}

	//	m_nodes.push_back(fn);

	//	//printf(" node %3d:  %4d,%4d   %4d,%4d    L%3d%c  R%3d%c\n",
	//	//	i,
	//	//	node->xp, node->yp,
	//	//	node->xp + node->dx, node->yp + node->dy,
	//	//	fn.left&0x7FFF, fn.left<0 ? '#' : ' ',
	//	//	fn.right&0x7FFF, fn.right<0 ? '#' : ' ');
	//}

	// Copy things
	// Load things
	for( int i=0; i<wadmap->n_things; i++ )
	{
		WadFile::Thing *wobj = wadmap->things + i;
		MapFileThing obj;
		obj.type = wobj->type;
		obj.xp = wobj->xp;
		obj.yp = wobj->yp;
		obj.flags = wobj->flags;
		obj.angle = wobj->angle;
		m_things.push_back(obj);
	}

	// Check limits
	if( m_nodes.size()	  > MAX_ENGINE_NODES ) throw string("MAX_ENGINE_NODES overflow");
	if( m_vertexes.size() > MAX_ENGINE_VERTEXES ) throw string("MAX_ENGINE_VERTEXES overflow");
	if( m_lines.size()	  > MAX_ENGINE_LINES ) throw string("MAX_ENGINE_LINES overflow");
	if( m_sectors.size()  > MAX_ENGINE_SECTORS ) throw string("MAX_ENGINE_SECTORS overflow");
	if( m_things.size()  > MAX_ENGINE_THINGS ) throw string("MAX_ENGINE_THINGS overflow");


	// Fill header
	m_header.format_marker = MAKEFOURCC('K', 'M', 'P', '1');

	m_header.n_nodes = m_nodes.size();
	m_header.nodes = (m_nodes.size()>0 ? &m_nodes[0] : NULL);

	m_header.n_vertexes = m_vertexes.size();
	m_header.vertexes = (m_vertexes.size()>0 ? &m_vertexes[0] : NULL);

	m_header.n_lines = m_lines.size();
	m_header.lines = (m_lines.size()>0 ? &m_lines[0] : NULL);

	m_header.n_sectors = m_sectors.size();
	m_header.sectors = (m_sectors.size()>0 ? &m_sectors[0] : NULL);

	m_header.n_things = m_things.size();
	m_header.things = (m_things.size()>0 ? &m_things[0] : NULL);
}


void WadConverter::Clear()
{
	m_nodes.clear();
	m_vertexes.clear();
	m_lines.clear();
	m_sectors.clear();
	m_things.clear();
	memset(&m_header, 0, sizeof(m_header));

	// Clear internals
	vertexes.clear();
	halflines.clear();
	sectors.clear();
	vertex_remap.clear();
	tex_index.clear();

	for( int i=0; i<(int)gen_linedefs.size(); i++ )
		delete gen_linedefs[i];
	gen_linedefs.clear();

	for( int i=0; i<(int)gen_sidedefs.size(); i++ )
		delete gen_sidedefs[i];
	gen_sidedefs.clear();
}

MapFileHeader *WadConverter::Convert(WadFile::MapInfo *_wadmap)
{
	wadmap = _wadmap;

	// Reset map
	Clear();

	// Assure all vertexes	(TBD: remove)
	for( int i=0; i<wadmap->n_vertexes; i++ )
		AssureVertex(i);

	// Load lines (and used vertexes)
	for( int i=0; i<wadmap->n_linedefs; i++ )
		LoadLine(i);

	// Load sectors
	for( int i=0; i<wadmap->n_sectors; i++ )
		LoadComplexSector(i);

	// Debug dump
	printf("Converted %d sectors\n", sectors.size());

	// Check if there are still unused halflines
	{
		int unused = 0;
		for( int i=0; i<halflines.size(); i++ )
			if( !halflines[i].used )
				unused++;
		if( unused>0 )
			throw format("%d halflines still unused after sector build", unused);
	}

	// Generate file structures
	PrepareFileData();

	printf("Final file:\n");
	printf(" - %d nodes\n",m_header.n_nodes);
	printf(" - %d vertexes\n", m_header.n_vertexes);
	printf(" - %d lines\n", m_header.n_lines);
	printf(" - %d sectors\n", m_header.n_sectors);
	printf(" - %d things\n", m_header.n_things);


	return &m_header;
}


void WadConverter::DebugDumpInternals()
{
	map<int, int> line2sector;

	printf("WadConverter internal dump:\n");

	for( int i=0; i<(int)sectors.size(); i++ )
	{
		const Sector &sec = sectors[i];
		printf("  sector %3d:\n", i);
		for( int j=0; j<(int)sec.halflines.size(); j++ )
		{
			const HalfLine &hl = halflines[sec.halflines[j]];
			printf("    line %3d: %3d - %3d  sector %3d   other %3d   wadsector %3d", sec.halflines[j], hl.v1, hl.v2, hl.sector, hl.other, hl.wad_sector);
			auto p = line2sector.find(sec.halflines[j]);
			if( p!=line2sector.end() )
				printf("  (used in sector %d)", p->second);
			else
				line2sector[sec.halflines[j]] = i;
			printf("\n");
		}
	}

	for( int i=0; i<(int)halflines.size(); i++ )
	{
		auto p = line2sector.find(i);
		if( p == line2sector.end() )
		{
			const HalfLine &hl = halflines[i];
			printf("  unused line %3d: %3d - %3d  sector %3d   other %3d\n", i, hl.v1, hl.v2, hl.sector, hl.other);
		}
	}

}

void WadConverter::DebugDumpMapFile()
{
	printf("WadConverter export dump:\n");

	for( int i=0; i<(int)m_lines.size(); i++ )
	{
		const MapFileLine &ml = m_lines[i];
		printf("  line %3d: vtx %3d, other sec %3d\n", i, ml.vertex, ml.other_sector);
	}

	for( int i=0; i<(int)m_sectors.size(); i++ )
	{
		const MapFileSector &ms = m_sectors[i];
		printf("  sector %3d: lines %3d .. %3d\n", i, ms.first_line, ms.first_line+ms.num_lines-1);
	}
}
