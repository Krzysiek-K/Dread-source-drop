
#pragma once





class WadConverter {
public:

	WadConverter();
	~WadConverter() { Clear(); }

	void Clear();
	MapFileHeader *Convert(WadFile::MapInfo *_wadmap);
	void DebugDumpInternals();
	void DebugDumpMapFile();

protected:

	struct Vertex {
		int				xpos;		// short integer range
		int				ypos;
		vector<int>		outgoing;	// outgoing halflines
		vector<int>		incoming;	// incoming halflines

		bool			used;		// mark
	};

	struct HalfLine {
		int					v1;				// start vertex
		int					v2;				// end vertex
		int					other;			// halfline on the other side (-1 if none)
		int					sector;			// index to sectors list
		int					wad_sector;		// shortcut, same as sidedef->sector, but frequently used
		WadFile::LineDef	*linedef;
		WadFile::SideDef	*sidedef;
		int					linedef_side;

		bool				used;		// mark
	};

	struct Sector {
		short			floor_h = 0;
		short			ceil_h = 0;
		string			floor_tex;
		string			ceil_tex;
		WadFile::Sector	*sector;
		vector<int>		halflines;
	};


	// input file
	WadFile::MapInfo		*wadmap;

	// temp structures
	vector<Vertex>				vertexes;
	vector<HalfLine>			halflines;
	vector<Sector>				sectors;
	map<int, int>				vertex_remap;
	map<string, int>			tex_index;
	vector<WadFile::LineDef*>	gen_linedefs;
	vector<WadFile::SideDef	*>	gen_sidedefs;

	// output map
	vector<MapFileNode>		m_nodes;
	vector<MapFileVertex>	m_vertexes;
	vector<MapFileLine>		m_lines;
	vector<MapFileSector>	m_sectors;
	vector<MapFileThing>	m_things;
	MapFileHeader			m_header;

	int  FanOrder(int v0, int v1, int v2);
	int  BendDirection(int v1, int v2, int v3);
	int  FindNextHalfline(int v1, int v2);
	bool IsShapeConvex(const vector<int> &vtx);
	int  FindVertexInConvexShape(const vector<int> &vtx);
	int  LineLength(int v1, int v2);
	int  AssureVertex(int vid);
	void LoadLine(int lid);
	void LoadSector(int sid);
	void LoadComplexSector(int sid);
	void PrepareFileData();
};
