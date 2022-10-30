
#ifndef _DREAD_ENGINE_H
#define _DREAD_ENGINE_H


typedef struct {
	short	xp;
	short	yp;
} DataVertex;

typedef struct {
	word	v1;
	word	v2;
	word	tex_upper;
	short	y1;
	short	y2;
	byte	ceil;
	byte	floor;
	word	mode;
} DataLine;





struct _EngineVertex;
struct _EngineLine;


typedef struct _EngineVertex EngineVertex;
typedef struct _EngineLine EngineLine;


struct _EngineVertex {
	short	xp;
	short	yp;
	short	tr_x;		// transformed to camera space, fixed point		(0: not transformed yet)
	short	tr_y;
};

// one side of a line
struct _EngineLine {
	EngineVertex	*v1;			//	@	  +0
	EngineVertex	*v2;			//	@	  +4
	EngineLine		*next;			//	-	  +8
	void			*other_sector;	//	-	 +12	sector on the other side for 2-sided lines
	const byte		*tex_upper;		//	@	 +16	the only texture for one-sided walls		(texture index includes light level)
	const byte		*tex_lower;		//	-	 +20
	word			xcoord1;		//	+	 +24	starting texture coord
	word			xcoord2;		//	+	 +26	ending texture coord
	short			y1;				//	@	 +28	Upper edge
	short			y2;				//	@	 +30	Lower edge
	short			y3;				//	+	 +32	Floor edge
	byte			ceil;			//	@	 +34	Ceiling color
	byte			floor;			//	@	 +35	Floor color
	word			mode;			//		 +36
									//			38	TOTAL
};


// player weapon frame info
typedef struct {
	const word	*data;
	word		width;
	word		height;
	word		off_x;
	word		off_y;
	word		plane_stride;		// in bytes
} WeaponFrameInfo;



// cheat struct

typedef struct {
	word	_pad;						//	 +0
	short	ceil_len_m64;				//	 +2		$0			$3
	short	ceil_len_m128;				//	 +4			$1	$2
	word	*upper_start_m64;			//	 +6		$0			$3
	word	*upper_start_m128;			//	+10			$1	$2
	short	upper_len_m64_0;			//	+14		$0			$3
	short	upper_len_m128_0;			//	+16			$1
	short	upper_len_m128_m64;			//	+18				$2
	short	floor_start_0;				//	+20		$0	$1	$2	$3
	short	other_ceil_start_m64;		//	+22				$2
										// 24 bytes
} LineSizeCheat;





// asm functons

extern short asm_line_h1;
extern short asm_line_h2;
extern short asm_line_h3;
extern short asm_line_ds;
extern word *asm_tex_base;

typedef void(*line_cheat_fn)(__d2 int xmin, __d3 int xcount, __d4 dword s1, __d6 dword u1s, __d7 int du, __a4 const byte *tex_base, __a5 EngineLine *line);


void Dread_FrameReset(void);

void Dread_LineCore2(__d2 int xmin, __d3 int xcount, __d4 dword s1, __d6 dword u1s, __d7 int du, __a4 const byte *tex_base, __a5 EngineLine *line);
void Dread_LineCoreCheat_0(__d2 int xmin, __d3 int xcount, __d4 dword s1, __d6 dword u1s, __d7 int du, __a4 const byte *tex_base, __a5 EngineLine *line);
void Dread_LineCoreCheat_1(__d2 int xmin, __d3 int xcount, __d4 dword s1, __d6 dword u1s, __d7 int du, __a4 const byte *tex_base, __a5 EngineLine *line);
void Dread_LineCoreCheat_2(__d2 int xmin, __d3 int xcount, __d4 dword s1, __d6 dword u1s, __d7 int du, __a4 const byte *tex_base, __a5 EngineLine *line);
void Dread_LineCoreCheat_3(__d2 int xmin, __d3 int xcount, __d4 dword s1, __d6 dword u1s, __d7 int du, __a4 const byte *tex_base, __a5 EngineLine *line);
int Dread_TestA7(__a0 void *ptr,__d0 byte value);


#endif
