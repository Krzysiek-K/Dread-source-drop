
#pragma once


#include <vector>
#include <deque>
#include <algorithm>

#include <base.h>



typedef unsigned int	dword;
typedef unsigned short	word;
typedef unsigned char	byte;





class WadFile {
public:
	struct Header {
		dword	magic;
		int		n_lumps;
		int		lump_ofs;
	};

	struct Lump {
		int		fpos;
		int		fsize;
		char	name[8];
	};

	struct Thing {
		short	xp;
		short	yp;
		word	angle;
		word	type;
		word	flags;
	};

	struct ZThing {
		word	tid;
		short	xp;
		short	yp;
		short	zp;
		word	angle;
		word	type;
		word	flags;
		byte	action;
		byte	args[5];
	};

	struct LineDef {
		enum {	TWOSIDED		= 0x0004,
				UPPER_UNPEGGED	= 0x0008,
				LOWER_UNPEGGED	= 0x0010  };
		word	start;
		word	end;
		word	flags;
		word	type;
		word	tag;
		word	sd_right;
		word	sd_left;
	};

	struct ZLineDef {
		word	start;
		word	end;
		word	flags;
		byte	type;
		byte	args[5];
		word	sd_right;
		word	sd_left;
	};

	struct SideDef {
		short	xoffs;
		short	yoffs;
		char	upper[8];
		char	lower[8];
		char	middle[8];
		word	sector;
	};

	struct Vertex {
		short	x;
		short	y;
	};

	struct Sector {
		short	floor_h;
		short	ceil_h;
		char	ftex[8];
		char	ctex[8];
		word	light;
		word	type;
		word	tag;
	};

	struct NodeBox {
		short	y1;
		short	y2;
		short	x1;
		short	x2;
	};

	struct Node {
		short	xp;			//	0 	2 	Partition line x coordinate
		short	yp;			//	2 	2 	Partition line y coordinate
		short	dx;			//	4 	2 	Change in x to end of partition line
		short	dy;			//	6 	2 	Change in y to end of partition line
		NodeBox	rbox;		//	8 	8 	Right bounding box
		NodeBox	lbox;		//	16 	8 	Left bounding box
		short	rchild;		//	24 	2 	Right child				(MSB:	0:subnode		1:bits 0..14 index subsector)
		short	lchild;		//	26 	2 	Left child
	};

	struct SubSector {
		word	num_segs;	//	0-1 	Short 	Number of segments in this sub-sector
		word	first_seg;	//	2-3 	Short 	First segment in this sub-Sector
	};

	struct Segment {
		word	v1;			//	0-1 	Unsigned short 	Beginning vertex number
		word	v2;			//	2-3 	Unsigned short 	Ending vertex number
		short	angle;		//	4-5 	Signed short 	Angle : 0x0000 = East, 0x4000=North, 0x8000=West, 0xC000=South
		word	linedef;	//	6-7 	Unsigned short 	Linedef number
		short	dir;		//	8-9 	Signed short 	Direction : 0 (same as linedef) or 1 (opposite of linedef)
		short	offset;		//	10-11 	Signed short 	Offset : distance along the linedef to the start of the seg
	};

	struct MapInfo {
		char		name[9];
		int			n_things;
		int			n_linedefs;
		int			n_sidedefs;
		int			n_vertexes;
		int			n_sectors;
		int			n_nodes;
		int			n_subsectors;
		int			n_segments;

		Thing		*things;
		LineDef		*linedefs;
		SideDef		*sidedefs;
		Vertex		*vertexes;
		Sector		*sectors;
		Node		*nodes;
		SubSector	*subsectors;
		Segment		*segments;
	};

	struct ZMapInfo {
		char		name[9];
		int			n_things;
		int			n_linedefs;
		int			n_sidedefs;
		int			n_vertexes;
		int			n_sectors;

		ZThing		*things;
		ZLineDef	*linedefs;
		SideDef		*sidedefs;
		Vertex		*vertexes;
		Sector		*sectors;
	};

	bool Load(const char *path,bool merge=true);
	void GetMap(const char *name,MapInfo *m);
	bool GetZMap(const char *name,ZMapInfo *m);
	bool HasLump(const char *name)						{ return FindLump(name) != NULL; }

	bool ReadPatch(const char *name,int &width,int &height,int &offs_x,int &offs_y,std::vector<int> &data);
	bool ReadTexture(const char *name,int &width,int &height,std::vector<int> &data);
	bool ReadFlat(const char *name,int &width,int &height,std::vector<int> &data);

	int *GetPalette() { LoadPalette(); return &palette[0]; }

private:
	std::vector<byte>	file_data;
	std::vector<int>	palette;
	std::vector<Lump>	lumps;

	Lump *FindLump(const char *name,const char *dir=NULL,const char *dir_end=NULL);
	void LoadPalette();
};
