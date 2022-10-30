
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
	int				_clip_flags;		// runtime
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
	bool					uses_persp_correction = false;
	bool					save_regs = false;

	// filled by Initialize()
	int						max_yindex = 0;
	bool					uses_sky			= false;
	bool					uses_texcoords		= false;
	bool					uses_tex_upper		= false;
	bool					uses_tex_lower		= false;
	bool					uses_solid_ceil		= false;
	bool					uses_solid_floor	= false;

	// Available optiomizations:
	//
	//															Gain			   Loss			   Cycles		   Description
	bool	opt_cmpa_loop					= true;			//		1 D-reg			1 A-reg			?				Replaces DBRA looping with address compare
	bool	opt_yminmax_common				= true;			//		1 D-reg			-				?				Keeps ymin & ymax values in a single register
	bool	opt_immediate_texstep			= true;			//		1 D-reg			-				?				Uses immediate add.l for texture coords
	bool	opt_texcoord_word				= false;		//		-				-				?				Uses add.w for texcoords instead of add.l
	bool	opt_keep_computed_texcoords		= false;		//
	bool	opt_indirect_fill				= true;			//
	bool	opt_keep_fill_fn				= false;		//
	bool	opt_keep_fn_scalers_base		= false;		//
	bool	opt_direct_return_to_end_loop	= true;			//
	bool	opt_step_column_reg				= true;			//
	bool	opt_texfill_moving_pointer		= true;			//
	bool	opt_assume_first_draw			= false;		//
															//
														
														
	// internal
	Assembler	as;
	int			free_dregs = 0;
	int			free_aregs = 0;


	void	Initialize(const int *draw_list, bool use_perspective);
	int		EstimateMethodCost();
	bool	EnableOptimizations(int free_dregs, int free_aregs);
	string	GetPrintName();

	bool GenerateCode();


protected:

	enum compare_t {
		COMPARE_YMIN_GREATER	= 0,
		COMPARE_YMIN_GEQ,
		COMPARE_YMAX_GREATER,
		COMPARE_YMAX_GEQ,
	};

	enum tracertype_t {
		TRACER_GENERIC = 0,
		TRACER_YPOS
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

	//struct TracerInfo {
	//	tracertype_t	mode;
	//	string			name;
	//	string			label;
	//	bool			use_long = false;
	//	bool			use_addx = false;
	//	bool			handle_next_subpixel = false;
	//
	//	// runtime
	//	EAddr			tracer_reg;
	//	EAddr			step_source;
	//};


	class CodeGenContext {
	public:
		int			free_regs = 0;
		int			avoid_regs = 0;
		EAddr		ymin_reg;
		EAddr		ymax_reg;
		EAddr		y_reg[MAX_Y_SPLITS];
		bool		ymax_selected;				// valid only with <opt_yminmax_common> enabled

		EAddr		reg_color_fill_addr;
		EAddr		reg_sky_fill_addr;
		EAddr		reg_texcoord;
		EAddr		reg_cmpa_end;
		EAddr		reg_sky_columns;
		EAddr		reg_fn_scalers_base;
		int			min_free_aregs = 8;
		int			min_free_dregs = 8;
		bool		texcoords_saved = false;

		bool		drawcore_had_gap = false;
		bool		drawcore_had_sky = false;
		bool		drawcore_finalized = false;
		int			render_column_incremented = 0;


		bool IsFree(const EAddr &reg);
		bool AllocReg(EAddr &reg, bool is_addr_reg);
		void FreeReg(EAddr &reg);
		void SetFreeReg(const EAddr &reg);
		void SetAvoidReg(const EAddr &reg);

		void UpdateInternalStats();
		void UpdateStats(int &free_dregs, int &free_aregs);

		EAddr get_column_value(int hardcoded_offset, const char *label_def, bool is_long);
		EAddr get_column_drawptr()			{ return get_column_value(	 0, "0",			 true ); }
		EAddr get_column_ymin()				{ return get_column_value(	 4, "rc_ymin",		 false); }
		EAddr get_column_ymax()				{ return get_column_value(	 6, "rc_ymax",		 false); }
		EAddr get_column_size_limit()		{ return get_column_value(1280, "rc_size_limit", false); }
	};


	// internals
	//vector<TracerInfo>	tracers;
	const EAddr				reg_texture_source	= instructions::a2;
	const EAddr				reg_render_column	= instructions::a4;		// hardcoded
	const EAddr				reg_line_def		= instructions::a5;
	//const EAddr				column_drawptr		= instructions::mem(reg_render_column);
	//const EAddr				column_ymin			= instructions::mem("rc_ymin", reg_render_column);
	//const EAddr				column_ymax			= instructions::mem("rc_ymax", reg_render_column);
	//const EAddr				column_size_limit	= instructions::mem("rc_size_limit", reg_render_column);
	const EAddr				reg_cur_size		= instructions::d4;
	const EAddr				reg_color_fill		= instructions::d5;
	const EAddr				reg_texcoord		= instructions::d6;		// input & counter
	const EAddr				reg_texstep			= instructions::d7;		// input

	int		label_counter		= 1;





	bool	SplitWithComparison(const YRange &yr, compare_t comp, int value, YRange &ytrue, YRange &yfalse);
	int		ComputeSectionClipFlags(int wall_section_index, const YRange &yr);
	int		EstimateDifficulty(const YRange &yr);
	string	SectionDebugString(WallSection &ws, int clip_flags);
	string	SectionsDebugString(const YRange &yr);
	string	VariableRangeDebugString(const EstimatedRange &er, const char *varname);
	string	RangeDebugString(const YRange &yr);

	bool	Error(const string &err);
	void	TryYMinMaxCommonLoad(CodeGenContext &ctx);
	void	AssureYMin(CodeGenContext &ctx);
	void	AssureYMax(CodeGenContext &ctx);
	bool	AssureColorFill(CodeGenContext &ctx);

	//TracerInfo	*Tracer_AssureSubpixelComponent();
	//TracerInfo	*Tracer_AddGeneric(EAddr reg, EAddr step_source, bool bit32, const string &name);
	//TracerInfo	*Tracer_AddYPos(CodeGenContext &ctx, int index);
	//void		Tracer_GenInit(CodeGenContext &ctx, int index);
	void		Gen_InitTracer(CodeGenContext &ctx, const EAddr &yreg, const EAddr &ysource, const char *name, const char *prev_name, const EAddr &prev_reg, bool word_size);

	void	Gen_ColorFill(CodeGenContext &ctx, int wall_section_index, int clip_flags, bool last_draw);
	void	Gen_GapSkip(CodeGenContext &ctx, int wall_section_index, int clip_flags);
	void	Gen_TextureFill(CodeGenContext &ctx, int wall_section_index, int clip_flags, bool expect_next_texture);
	string	Gen_DrawCore_Finalize(CodeGenContext &ctx);
	void	Gen_DrawCore(CodeGenContext &ctx, const YRange &yr);
	void	Gen_GenerateIfLadderRec(CodeGenContext &ctx, const YRange &yr, const char *label_name);
};
