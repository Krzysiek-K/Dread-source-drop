
#ifndef _DREAD_DATA_H
#define _DREAD_DATA_H



// ================================================================ Data* types - file structures ================================================================

typedef struct {

	int		map_collision_bsp_root;
	int		map_hitscan_bsp_root;

} DataHeader;


typedef struct {
	short	xp;			// 0xFFFF - end of list
	short	yp;
} DataVertex;

typedef struct {
	word	v1;			// 0xFFFF - end of list
	word	v2;
	int		tex_upper;
	short	offs_x;		// texcoord
	byte	ceil;
	byte	floor;
	byte	modes[2];
	int		tex_lower;		//	
	short	y1;				//	ceiling -> upper
	short	y2;				//	upper -> gap
	short	y3;				//	gap -> lower
	short	y4;				//	lower -> floor
} DataLine;
//const int foo = sizeof(DataLine);			// 16

//$asmstruct node
typedef struct {
	short	A;			// x*A + y*B + C > 0	- right side
	short	B;
	int		C;
	short	left;		// >=0: node index		<0: ~(vis order offset)
	short	right;
} DataBSPNode;

typedef struct {
	word	visoffset;			// 0xFFFF - end of list
	word	type;
	word	height;
} DataSubSector;

//$asmstruct actor
typedef struct {
	word		num;
	void		*cb_create;
	void		*cb_damage;
	void		*cb_pulse;
} DataActor;

typedef struct {
	word		type;
	short		xp;
	short		yp;
	word		flags;
} DataThing;

typedef struct {
	short		xp;
	short		yp;
	word		angle;
} DataLocation;

typedef struct {
	word		type;			// 0xFFFF - end of list
	word		line_id;
	word		tag;
} DataMachine;

typedef struct {
	word		op;				// 0xFFFF - end of list			0: machine,	1: and, 2: or
	word		c1;
	word		c2;
} DataCondition;



// ================================================================ Misc structures ================================================================


// player weapon frame info
typedef struct {
	const word	*data;
	word		width;
	word		height;
	word		off_x;
	word		off_y;
	word		plane_stride;		// in bytes
	word		delay;
} WeaponFrameInfo;

// player weapon frame info (new format)
typedef struct {
	const word	*mask;
	const word	*plane0;
	const word	*plane1;
	const word	*plane2;
	const word	*plane3;
	word		width;
	word		height;
	word		off_x;
	word		off_y;
} WeaponFrameInfoX;

// player weapon frame info (ST format)
//$asmstruct st_wpinfo
typedef struct {
	const word	*data;				// +0
	word		screen_start;		// +4
	word		width;				// +6
	word		height;				// +8
} WeaponFrameInfoST;

// player weapon sprite info - single sprite data
typedef struct {
	short		ydelta;
	const word	*data0;
	const word	*data1;
} WeaponSpriteInfoElement;

// player weapon sprite info
typedef struct {
	word					palette[15];
	WeaponSpriteInfoElement	sprites[4];
} WeaponSpriteInfo;

// player weapon sprite animation frame
//$asmstruct weaponanim
typedef struct {
	short		delay;
	const WeaponSpriteInfo	*gfx;
	short		xpos;
	short		ypos;
	word		flags;
} WeaponSpriteAnimFrame;


#endif
