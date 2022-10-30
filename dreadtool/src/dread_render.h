
#pragma once



class DreadSoftRenderer {
public:
	enum {
		// Engine limits
		ENGINE_X_MAX					= 160,
		ENGINE_Y_MAX					= 100,
		ENGINE_Y_MID					=  50,
		ENGINE_ZOOM						=  80,
		ENGINE_WIDTH					= 160,
		ENGINE_SUBVERTEX_PRECISION		=    3,

		ENGINE_ZOOM_CONSTANT			= (ENGINE_ZOOM*32)<<(8+ENGINE_SUBVERTEX_PRECISION),
		ENGINE_MIN_ZNEAR				= (ENGINE_ZOOM_CONSTANT/65536)+1,

		STRETCH_SCALING					=   1,

		// Internal limits
		NUM_SOFT_SIZES					= 2048,
		NUM_SCALERS						=  160,

	};

	struct EngineVertex {
		short	xp;
		short	yp;
		short	tr_x;		// transformed to camera space, fixed point		(0: not transformed yet)
		short	tr_y;
	};

	struct EngineLine {
		EngineVertex	*v1;			//
		EngineVertex	*v2;			//
		texture_id_t	tex_upper;
		texture_id_t	tex_lower;
		word			xcoord1;		//	starting texture coord
		word			xcoord2;		//	ending texture coord
		short			ycoord_u;		//	upper/normal texture Y offset
		short			ycoord_l;		//	lower texture Y offset
		short			y1;
		short			y2;
		short			y3;
		short			y4;
		byte			ceil_col;		//	Ceiling color
		byte			floor_col;		//	Floor color
		bool			draw_pixel;
		byte			pixel_col;
		//byte			modes[2];		//
	};

	struct EngineSubSector {
		const word		*vis;			
		//const word		*thingvis;		
		//EngineThing		*first_thing;	//	 8
		//word			visframe;
	};

	struct EngineVisLine {
		word		linemode;
		EngineLine	*line;
	};

	struct DataBSPNode {
		short	A;			// x*A + y*B + C > 0	- right side
		short	B;
		int		C;
		short	left;		// >=0: node index		<0: ~(vis order offset)
		short	right;
	};

	// public state
	short			view_pos_x;
	short			view_pos_y;
	short			view_pos_z;
	short			view_angle;
	short			view_rotate_dx;
	short			view_rotate_dy;
	int				max_outbound_top;
	int				max_outbound_bottom;


	void LoadMap(DreadMapGen &mapgen);
	void Draw(DataTexture &screen, const vec3 &cam_pos, float cam_angle);


protected:

	typedef struct {
		word	*func_addr;
		short	real_size;
		int		_offset;
	} ScalerInfo;

	typedef struct {
		struct {
			DWORD	*render_addr;
			word	ymin;
			word	ymax;
		} chunk1[ENGINE_X_MAX];
		struct {
			word	size_limit;
			word	thing_num;
			word	_pad1;
			word	_pad2;
		} chunk2[ENGINE_X_MAX];
	} RenderColumnsInfo;


	// runtime data
	RenderColumnsInfo			render_columns;
	EngineSubSector				*view_subsector;

	// loaded map data
	vector<EngineVertex>		e_vtx;
	vector<EngineLine>			e_lines;
	vector<word>				map_vis;
	vector<EngineSubSector>		e_subsectors;
	vector<EngineVisLine>		e_vislines;
	vector<DataBSPNode>			map_nodes;

	// map loader helpers
	ChunkyColorPacker			chunky_packer;
	vector<texture_id_t>		tex_list;

	// renderer internal precomputed
	ScalerInfo					FN_SCALERS[NUM_SOFT_SIZES];
	vector<word>				scaler_code;




	void Map_LoadVerts(DreadMapGen &mapgen);
	void Map_LoadLines(DreadMapGen &mapgen);
	void Map_LoadVis(DreadMapGen &mapgen);
	void Map_LoadSubSectors(DreadMapGen &mapgen);
	void Map_LoadBSP(DreadMapGen &mapgen);

	void Dread_Build_Scaler_One(int size);
	void Dread_Build_Scalers();

	void Draw_Line(EngineLine *line);
	void Draw_FillRange(word xpos, int y1, int y2, byte color);
	void Draw_TextureRange(word xpos, int y1, int y2, texture_id_t tex, int texu, int texv, word *scaler);
	void Draw_RenderLine(int xpos, int xcount, word s1, dword u1s, int du, short asm_line_ds, int texy1, int texy2, EngineLine *line);
	void Draw_RenderLine_Old(int xpos, int xcount, word s1, dword u1s, int du, short asm_line_ds, int texy1, int texy2, EngineLine *line);

};
