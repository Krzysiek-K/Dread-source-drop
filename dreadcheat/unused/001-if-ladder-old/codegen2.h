
#pragma once


// ================================================================ types ================================================================

enum drawtype_t {
	// Normal drawing - fill texture, sky or solid color from max(y1,ymin) to min(y2,ymax)
	//	Cases:
	//		y2 <= ymin						culled (above visible region)
	//		ymax <= y1						culled (below visible region)
	//		ymin <= y1 <= y2 <= ymax		draw normally
	//		y1 <= ymin <= y2 <= ymax		clipped top
	//		ymin <= y1 <= ymax <= y2		clipped bottom
	//		y1 <= ymin <= ymax <= y2		clipped both sides
	DRAW_SOLID_CEIL		= 101,
	DRAW_SOLID_FLOOR,
	DRAW_SKY,
	DRAW_TEX_UPPER,
	DRAW_TEX_LOWER,
	DRAW_GAP,
};



// ================================================================ WallSection ================================================================


struct WallSection {
	drawtype_t		drawtype;			// what to draw
	int				yend_index;			// index of interpolator for final position		(0..3 for h1..h4)
	int				yend_offset;		// offset added to the final position
};



// ================================================================ VariableCodeGen ================================================================


class VariableCodeGen {
public:
	enum {
		MAX_Y_SPLITS		= 4,
	};

	// settings
	vector<WallSection>		sections;
	int						yindex_source_height[MAX_Y_SPLITS] ={ 1, 2, 3, 4 };
	int						max_yindex = 0;
	bool					uses_sky	= false;
	bool					use_persp_correction = false;
	bool					save_regs	= false;

	// internal
	Assembler	as;


	void AddSection(drawtype_t drawtype, int yend_index, int yend_offset = 0);
	void Initialize(const int *draw_list);
	string GetPrintName();
	void GenerateIfLadder();

	void GenerateCode();


protected:

	enum compare_t {
		COMPARE_YMIN_GREATER	= 0,
		COMPARE_YMIN_GEQ,
		COMPARE_YMAX_GREATER,
		COMPARE_YMAX_GEQ,
	};

	enum {
		FCLIP_VISIBLE					= 1<<0,		// section can be visible and must be rendered				(flag missing: definitely not visible; all flags are 0 in this case)
		FCLIP_OCCLUSION_POSSIBLE_TOP	= 1<<1,		// section can be completely below the gap					(not allowed in final rendering - if ladder should separate these cases)
		FCLIP_OCCLUSION_POSSIBLE_BOTTOM	= 1<<2,		// section can be completely above the gap					(not allowed in final rendering - if ladder should separate these cases)
		FCLIP_TOP_ALWAYS				= 1<<3,		// top rendering starts from ymin
		FCLIP_TOP_UNDECIDED				= 1<<4,		// top can start from either ymin or normal Y level			(not allowed in final rendering - if ladder should separate these cases)
		FCLIP_BOTTOM_ALWAYS				= 1<<5,		// bottom rendering ends at ymax
		FCLIP_BOTTOM_UNDECIDED			= 1<<6,		// bottom can end at either ymax or normal Y level			(not allowed in final rendering - if ladder should separate these cases)

		FCLIP_OCCLUSION_POSSIBLE_MASK	= FCLIP_OCCLUSION_POSSIBLE_TOP | FCLIP_OCCLUSION_POSSIBLE_BOTTOM,	// ORed mask for both cases
	};

	struct EstimatedRange {		// range, where ymin/ymax can be found
		int		loc_min;		// == yindex*2 + 1			-->  on yindex split
		int		loc_max;

		bool value_can_be_leq(int value)		const	{ return value <= loc_max;	}
		bool value_can_be_less(int value)		const	{ return value <  loc_max;	}
		bool value_can_be_geq(int value)		const	{ return value >= loc_min;	}
		bool value_can_be_greater(int value)	const	{ return value >  loc_min; }

		bool SplitWithComparison(bool comp_geq, int value, EstimatedRange &rtrue, EstimatedRange &rfalse) const;
	};

	struct YRange {
		EstimatedRange	ymin;
		EstimatedRange	ymax;

		void Trim()
		{
			if( ymax.loc_max < ymin.loc_max ) ymin.loc_max = ymax.loc_max;
			if( ymin.loc_min > ymax.loc_min ) ymax.loc_min = ymin.loc_min;
		}
	};

	struct DrawAction {
		int		section_index = 0;		// section to draw
		int		ymin_case;				// rc_ymin in relation to the section
		int		ymax_case;				// rc_ymax in relation to the section
	};

	struct TracerInfo {
		string		name;
		string		label;
		bool		use_long = false;
		bool		use_addx = false;
		uint32_t	init_step = 0;

		// runtime
		EAddr		tracer_reg;
	};


	class CodeGenContext {
	public:
		int			free_regs = 0;
		EAddr		ymin_reg;
		EAddr		ymax_reg;
		EAddr		y_reg[MAX_Y_SPLITS];
		bool		ymax_selected;				// valid only with <opt_yminmax_common> enabled

		EAddr		reg_color_fill_addr;
		EAddr		reg_sky_fill_addr;

		bool AllocReg(EAddr &reg, bool is_addr_reg);
		void FreeReg(EAddr &reg);
		void SetFreeReg(const EAddr &reg);
	};


	// internals
	vector<TracerInfo>	tracers;
	EAddr				reg_texture_source	= instructions::a2;
	EAddr				reg_render_column	= instructions::a4;
	EAddr				column_drawptr		= instructions::mem(reg_render_column);
	EAddr				column_ymin			= instructions::mem("rc_ymin", reg_render_column);
	EAddr				column_ymax			= instructions::mem("rc_ymax", reg_render_column);
	EAddr				reg_cur_size		= instructions::d4;
	EAddr				reg_color_fill		= instructions::d5;
	EAddr				reg_cmpa_end;		// allocated when needed
	EAddr				reg_sky_columns;	// allocated when needed

	int		label_counter		= 1;

												// Available optiomizations:
												//
												//	   Gain			   Loss			   Cycles		   Description
	bool	opt_cmpa_loop		= true;			//		1 D-reg			1 A-reg			?				Replaces DBRA looping with address compare
	bool	opt_yminmax_common	= false;		//		1 D-reg			?				?				Keeps ymin & ymax values in a single register
												//




	void	GenerateIfLadderRec(const YRange &yr, const string &indent0, const string &indent);

	bool	SplitWithComparison(const YRange &yr, compare_t comp, int value, YRange &ytrue, YRange &yfalse);
	int		ComputeSectionClipFlags(int wall_section_index, const YRange &yr);
	int		EstimateDifficulty(const YRange &yr);
	string	SectionDebugString(WallSection &ws, int clip_flags);
	string	SectionsDebugString(const YRange &yr);
	string	VariableRangeDebugString(const EstimatedRange &er, const char *varname);
	string	RangeDebugString(const YRange &yr);

	bool	Error(const string &err);
	void	AssureYMin(CodeGenContext &ctx);
	void	AssureYMax(CodeGenContext &ctx);
	void	Gen_InitTracer(CodeGenContext &ctx, const EAddr &yreg, const EAddr &ysource, const char *name, const char *prev_name, const EAddr &prev_reg);

	void	Gen_ColorFill(CodeGenContext &ctx, int wall_section_index, int clip_flags);
	void	Gen_GapSkip(CodeGenContext &ctx, int wall_section_index, int clip_flags);
	void	Gen_DrawCore(CodeGenContext &ctx, const YRange &yr);
	void	Gen_GenerateIfLadderRec(CodeGenContext &ctx, const YRange &yr, const char *label_name);
};
