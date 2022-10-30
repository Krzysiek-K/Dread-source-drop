
#pragma once



//struct _EngineBSPNode;
//struct _EngineVertex;
//struct _EngineLine;
//struct _EngineSector;
//
//typedef struct _EngineBSPNode EngineBSPNode;
//typedef struct _EngineVertex EngineVertex;
//typedef struct _EngineLine EngineLine;
//typedef struct _EngineSector EngineSector;




// ------------------------------------------------ File structs ----------------------------------------------------------------

typedef struct {
	short	A;
	short	B;
	long	C;
	short	right;
	short	left;
} MapFileNode;

typedef struct {
	short	xp;
	short	yp;
} MapFileVertex;

typedef struct {
	word		vertex;
	string		upper_tex;
	string		lower_tex;
	word		xcoord1;
	word		xcoord2;
	short		ycoord;
	word		other_sector;		// 0xFFFF is none
	word		line_action;
	word		line_tag;
	word		light_level;
	word		line_flags;
} MapFileLine;

typedef struct {
	word		first_line;
	word		num_lines;
	short		floor_h;
	short		ceil_h;
	word		light_level;
	string		floor_tex;
	string		ceil_tex;
	word		type;
	word		tag;
} MapFileSector;

typedef struct {
	word	type;
	short	xp;
	short	yp;
	word	flags;
	word	angle;
} MapFileThing;

typedef struct {
	dword			format_marker;
	word			n_nodes;
	word			n_vertexes;
	word			n_lines;
	word			n_sectors;
	word			n_things;
	MapFileNode		*nodes;
	MapFileVertex	*vertexes;
	MapFileLine		*lines;
	MapFileSector	*sectors;
	MapFileThing	*things;
} MapFileHeader;





// ------------------------------------------------ Engine structs ----------------------------------------------------------------

//struct _EngineBSPNode {
//	short	A;
//	short	B;
//	long	C;
//	void	*right;		// pointer to node or sector	(compare to e_mapdata.sectors[] - nodes have lower addresses)
//	void	*left;
//};
//
//
//struct _EngineVertex {
//	short	xp;
//	short	yp;
//	short	tr_x;		// transformed to camera space, fixed point		(0: not transformed yet)
//	short	tr_y;
//};
//
//// one side of a line
//struct _EngineLine {
//	EngineVertex	*vertex;
//	EngineLine		*next;
//	void			*other_sector;	// sector on the other side for 2-sided lines
//	word			tex_upper;		// the only texture for one-sided walls		(texture index includes light level)
//	word			tex_lower;
//	word			xcoord1;		// starting texture coord
//	word			xcoord2;		// ending texture coord
//};
//
//
//struct _EngineSector {
//	EngineLine		*first_line;	// updated to point to first line encountered this frame
//	short			floor_h;
//	short			ceil_h;
//	byte			floor_col;
//	byte			ceil_col;
//	byte			_pad[2];
//
//	// run-time
//	EngineLine		*active_line;
//	// TBD: interpolation values of active line
//
//};


#define MAX_ENGINE_NODES		4096
#define MAX_ENGINE_VERTEXES		4096
#define MAX_ENGINE_LINES		4096
#define MAX_ENGINE_SECTORS		4096
#define MAX_ENGINE_THINGS		4096

//typedef struct {
//	EngineBSPNode	bsp_nodes[MAX_ENGINE_NODES];
//	EngineVertex	vertexes[MAX_ENGINE_VERTEXES];
//	EngineLine		lines[MAX_ENGINE_LINES];
//	EngineSector	sectors[MAX_ENGINE_SECTORS];
//
//	word			n_bsp_nodes;
//	word			n_vertexes;
//	word			n_lines;
//	word			n_sectors;
//
//} EngineMapData;
