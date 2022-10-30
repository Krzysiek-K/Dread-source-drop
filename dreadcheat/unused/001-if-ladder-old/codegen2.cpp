
#include "common.h"


// ================================================================ Helper functions ================================================================


static string _tracer_name(int index)
{
	return index >= 0 ? format(".trace_y%d", index) : ".trace_s";
}




// ================================================================ VariableCodeGen::EstimatedRange ================================================================


bool VariableCodeGen::EstimatedRange::SplitWithComparison(bool comp_geq, int value, EstimatedRange &rtrue, EstimatedRange &rfalse) const
{
	rtrue = *this;
	rfalse = *this;

	if( comp_geq )
	{
		if( value<=loc_min || value>loc_max ) return false;
		rtrue.loc_min = value;
		rfalse.loc_max = value - 1;
	}
	else
	{
		if( value<loc_min || value>=loc_max ) return false;
		rtrue.loc_min = value+1;
		rfalse.loc_max = value;
	}

	return true;
}


// ================================================================ VariableCodeGen::CodeGenContext ================================================================


bool VariableCodeGen::CodeGenContext::AllocReg(EAddr &reg, bool is_addr_reg)
{
	if( reg.IsReg() )
		return true;

	for( int i=0; i<8; i++ )
	{
		int regnum = i + (is_addr_reg ? 8 : 0);
		if( free_regs & (1<<regnum) )
		{
			reg = is_addr_reg ? EAddr::a(i) : EAddr::d(i);
			free_regs &= ~(1<<regnum);
			return true;
		}
	}

	return false;
}

void VariableCodeGen::CodeGenContext::FreeReg(EAddr &reg)
{
	if( !reg.IsReg() || uint32_t(reg.regnum)>16 )
		return;

	reg.amode = AMODE_IGNORED;		// disable current register
	
	if( ymin_reg.IsReg() && ymin_reg.regnum==reg.regnum ) return;	// still used by <ymin>
	if( ymax_reg.IsReg() && ymax_reg.regnum==reg.regnum ) return;	// still used by <ymax>

	free_regs |= 1 << reg.regnum;
}

void VariableCodeGen::CodeGenContext::SetFreeReg(const EAddr &reg)
{
	if( !reg.IsReg() || uint32_t(reg.regnum)>16 )
		return;

	free_regs |= 1 << reg.regnum;
}




// ================================================================ VariableCodeGen - public ================================================================



void VariableCodeGen::AddSection(drawtype_t drawtype, int yend_index, int yend_offset )
{
	WallSection s;
	s.drawtype = drawtype;
	s.yend_index = yend_index;
	s.yend_offset = yend_offset;
	sections.push_back(s);
}


void VariableCodeGen::Initialize(const int *draw_list)
{
	sections.clear();
	max_yindex = -1;
	uses_sky = false;

	while( *draw_list>=0 )
	{
		if( *draw_list <= 99 )
		{
			yindex_source_height[++max_yindex] = *draw_list;
			sections.back().yend_index = max_yindex;
		}
		else
		{
			WallSection s;
			s.drawtype = (drawtype_t)*draw_list;
			s.yend_index = 99;
			s.yend_offset = 0;
			sections.push_back(s);

			if( s.drawtype == DRAW_SKY )
				uses_sky = true;
		}

		draw_list++;
	}
}

string VariableCodeGen::GetPrintName()
{
	string s;
	for( int i=0; i<(int)sections.size(); i++ )
	{
		if( s.size()>0 ) s += " ";
		s += SectionDebugString(sections[i], FCLIP_VISIBLE);
		if( sections[i].yend_index <= max_yindex )
			s += format(" y%d", yindex_source_height[sections[i].yend_index]);
	}
	return s;
}


void VariableCodeGen::GenerateIfLadder()
{
//	vector<DrawAction> dlist;
//
//	for( int i=0; i<(int)sections.size(); i++ )
//	{
//		DrawAction a;
//		a.section_index = i;
//		a.ymax_case = RANGE_CASE_ALL;
//		a.ymin_case = RANGE_CASE_ALL;
//		dlist.push_back(a);
//	}

	YRange yr;
	yr.ymin.loc_min = 0;
	yr.ymin.loc_max = max_yindex*2 + 2;
	yr.ymax.loc_min = 0;
	yr.ymax.loc_max = max_yindex*2 + 2;

	GenerateIfLadderRec(yr, "", "");
}


void VariableCodeGen::GenerateCode()
{
	using namespace instructions;

	try {

		// -------- Init assembler --------
		as.Clear();
		as.BlockMode(0);


		// -------- Init context --------
		CodeGenContext ctx;
		ctx.SetFreeReg(d0);
		ctx.SetFreeReg(d1);
		ctx.SetFreeReg(d2);
		ctx.SetFreeReg(d6);
		ctx.SetFreeReg(d7);

		ctx.SetFreeReg(a0);
		ctx.SetFreeReg(a1);
		if( !uses_sky ) ctx.SetFreeReg(a2);
		ctx.SetFreeReg(a3);



		// -------- Preamble --------
		if( save_regs )
			as.Assemble(" movem.l d2-d7/a2-a6, -(a7)");
		as.Assemble(" move.l a7, _a7_save");


		// -------- Init loop --------
		as.Assemble(";");
		as.Assemble("; ======== Loop initialization ========");
		as << lea.l(label("_render_columns"), reg_render_column);
		as << add.w(d2, reg_render_column) << "add xmin*8 to column pointer";

		if( uses_sky )
		{
			if( !ctx.AllocReg(reg_sky_columns, true) )
				Error("Can't allocate reg for sky source");

			as << lsr.w(imm(1), d2) << "init SKY";
			as << mov.l(label("_e_skyptr"), reg_sky_columns);
			as << add.w(d2, reg_sky_columns);
		}

		if( opt_cmpa_loop )
		{
			if( !ctx.AllocReg(reg_cmpa_end, true) )
				Error("Can't allocate reg for loop end pointer");

			as << mov.l(reg_render_column, reg_cmpa_end) << "end = start + count*8";
			as << lsl.w(imm(3), d3);
			as << add.w(d3, reg_cmpa_end);
			ctx.SetFreeReg(d3);
		}


		// -------- Init tracers --------
		for( int i=0; i<=max_yindex; i++ )
		{
			if( !ctx.AllocReg(ctx.y_reg[i], false) )
				Error(format("Can't allocate reg for tracer y%d", i));

			Gen_InitTracer(
				ctx,
				ctx.y_reg[i], 
				//imm(i==0 ? -64 : 0),
				mem(format("line_y%d",yindex_source_height[i]), a5),
				_tracer_name(i).c_str(),
				_tracer_name(i-1).c_str(),
				(i-1>=0) ? ctx.y_reg[i-1] : reg_cur_size);
		}
		as << mov.w(label("_asm_line_ds(pc)"), label(".trace_s", 4));


		// -------- Init flat colors --------
		as << mov.b(mem("line_ceil_col", a5), reg_color_fill);
		as << swp.w(reg_color_fill);
		as << mov.b(mem("line_floor_col", a5), reg_color_fill);
		ctx.SetFreeReg(a5);


		// -------- Header end --------
		as << bra(label(".start")) << "End of header";
		as.Assemble(";");
		as.Assemble(";");


		// -------- Loop start --------
		as.BlockMode(1);
		as.Assemble(".loop: ; ======== LOOP START ========");
		as << add.w(imm(8), reg_render_column);
		//    add.l       d7, d6                              ;   u1s += du;

		as.Assemble(".trace_s:");
		as << add.l(imm("$FF00FFFF"), d4) << "s1 += ds";
		
		for( int i=0; i<=max_yindex; i++ )
		{
			EAddr temp;
			if( !ctx.AllocReg(temp, false) )
				Error(format("Can't allocate temp reg to step tracer y%d", i));

			as.Assemble(_tracer_name(i)+":");
			as << mov.l(imm("$FF000000"), temp);
			as << addx.l(temp, ctx.y_reg[i]);

			ctx.FreeReg(temp);
		}

		as.Assemble(";");
		as.Assemble(".start: ; ======== FIRST ITERATION STARTS HERE ========");
		as << mov.l(column_drawptr, a7) << "byte *dst = render_buffer + offs[x];";


		YRange yr;
		yr.ymin.loc_min = 0;
		yr.ymin.loc_max = max_yindex*2 + 2;
		yr.ymax.loc_min = 0;
		yr.ymax.loc_max = max_yindex*2 + 2;
		Gen_GenerateIfLadderRec(ctx, yr, NULL);


		// -------- END --------

		as.Assemble(";");
		as.Assemble(";");
		as.BlockMode(1);
		as.Assemble(".end_loop: ; ======== LOOP END ========");
		if( opt_cmpa_loop )
		{
			as << cmp.l(reg_cmpa_end, reg_render_column) << "render_column ? render_end";
			as << blt(label(".loop")) << "continue if <";
		}
		else
			as << dbra.w(d3,label(".loop"));

		as.Assemble(".end: ; Cleanup & exit");
		as.Assemble(" move.l _a7_save(pc), a7");

		if( save_regs )
			as.Assemble(" movem.l (a7)+, d2-d7/a2-a6");
		as.Assemble(" rts");

		// Allocate regs
		int free_regs = 0;
		free_regs |= 1<<0;	// d0
		free_regs |= 1<<1;	// d1
		free_regs |= 1<<2;	// d2
		//if( !ceil_mode && !floor_mode ) free_regs |= 1<<5;	// d5
		free_regs |= 0x100<<0;	// a0
		free_regs |= 0x100<<1;	// a1
		//if( !upper_mode && !lower_mode ) free_regs |= 0x100<<2;	// a2
		//if( !sky_mode ) free_regs |= 0x100<<3;	// a3

		as.AllocateRegs(free_regs);
		as.ConvertOpcodes();

		//as.Print();

		int cmin, cmax;
		as.CountCycles(cmin, cmax);
		printf(" - %4d .. %4d cycles\n", cmin, cmax);
	}
	catch( const string &e )
	{
		as.Print();
		printf("Fatal ERROR: %s\n", e.c_str());
	}
}




// ================================================================ VariableCodeGen - protected ================================================================


void VariableCodeGen::GenerateIfLadderRec(const YRange &yr, const string &indent0, const string &indent)
{
	int cases = EstimateDifficulty(yr);
	if( cases <= 1 )
	{
		printf("%s %s\n", indent0.c_str(), SectionsDebugString(yr).c_str());
		return;
	}


	// find perfect split
	YRange ytrue, yfalse;
	int best_cost = 1000000000;
	int best_comp = 0;
	int best_yindex = 0;
	int best_cases1 = 0;
	int best_cases2 = 0;

	for( int comp=0; comp<4; comp++ )
		for( int yindex=0; yindex<=max_yindex; yindex++ )
		{
			if( !SplitWithComparison(yr, (compare_t)comp, yindex*2+1, ytrue, yfalse) )
				continue;

			int cases1 = EstimateDifficulty(ytrue);
			int cases2 = EstimateDifficulty(yfalse);
			int cost = max(cases1, cases2)*100 + cases1 + cases2;

			if( cost < best_cost )
			{
				best_cost = cost;
				best_comp = comp;
				best_yindex = yindex;
				best_cases1 = cases1;
				best_cases2 = cases2;
			}
		}

	if( best_cost >= 1000000000 )
	{
		printf("%s[%d] FAIL: %s\n", indent0.c_str(), cases, SectionsDebugString(yr).c_str());
		return;
	}


	//printf("%s[%d] SPLIT: %s %s y%d   (cost %d,  %d:%d)\n",
	//	indent0.c_str(),
	//	cases,
	//	best_comp>=2 ? "ymax" : "ymin",
	//	best_comp&1 ? ">=" : ">",
	//	best_yindex,
	//	best_cost, best_cases1, best_cases2
	//);
//	printf("%s%s%sy%d\n",
//		indent0.c_str(),
//		best_comp>=2 ? "ymax" : "ymin",
//		best_comp&1 ? ">=" : ">",
//		best_yindex);

	SplitWithComparison(yr, (compare_t)best_comp, best_yindex*2+1, ytrue, yfalse);
	//GenerateIfLadderRec(ytrue, indent+"  T -> ", indent+"  |    ");
	//GenerateIfLadderRec(yfalse, indent+"  F -> ", indent+"       ");
//	GenerateIfLadderRec(ytrue, indent+"  yes -> ", indent+"  |      ");
//	GenerateIfLadderRec(yfalse, indent+"  no  -> ", indent+"         ");


	string check = format("%s%sy%d",
		best_comp>=2 ? "ymax" : "ymin",
		best_comp&1 ? ">=" : ">",
		best_yindex);
	if( check.size()<8 )
		check += " ";

	GenerateIfLadderRec(ytrue, indent0+check+" -> ", indent+"    |       ");
	GenerateIfLadderRec(yfalse, indent+"   else  -> ", indent+"            ");
}

bool VariableCodeGen::SplitWithComparison(const YRange &yr, compare_t comp, int value, YRange &ytrue, YRange &yfalse)
{
	bool result = false;
	ytrue = yr;
	yfalse = yr;
	switch(comp)
	{
	case COMPARE_YMIN_GREATER:		result = yr.ymin.SplitWithComparison(false, value, ytrue.ymin, yfalse.ymin);		break;
	case COMPARE_YMIN_GEQ:			result = yr.ymin.SplitWithComparison(true, value, ytrue.ymin, yfalse.ymin);			break;
	case COMPARE_YMAX_GREATER:		result = yr.ymax.SplitWithComparison(false, value, ytrue.ymax, yfalse.ymax);		break;
	case COMPARE_YMAX_GEQ:			result = yr.ymax.SplitWithComparison(true, value, ytrue.ymax, yfalse.ymax);			break;
	}
	ytrue.Trim();
	yfalse.Trim();
	return result;
}

int VariableCodeGen::ComputeSectionClipFlags(int wall_section_index, const YRange &yr)
{
	WallSection &ws = sections[wall_section_index];
	int y1 = (wall_section_index>0) ? sections[wall_section_index-1].yend_index*2 + 1 : -1;
	int y2 = ws.yend_index*2 + 1;

	if( y1 >= yr.ymax.loc_max ) return 0;		// below gap
	if( y2 <= yr.ymin.loc_min ) return 0;		// above gap

	int flags = FCLIP_VISIBLE;

	if( y1 >= yr.ymin.loc_max ) {}									// no top clipping
	else if( y1 <= yr.ymin.loc_min ) flags |= FCLIP_TOP_ALWAYS;		// top clipping can be always enabled
	else  flags |= FCLIP_TOP_UNDECIDED;								// undecided top clipping

	if( y2 <= yr.ymax.loc_min ) {}									// no bottom clipping
	else if( y2 >= yr.ymax.loc_max ) flags |= FCLIP_BOTTOM_ALWAYS;	// bottom clipping can be always enabled
	else flags |= FCLIP_BOTTOM_UNDECIDED;							// undecided bottom clipping

	if( yr.ymax.loc_min < y1 )								// entire section can potentially be culled
		flags |= FCLIP_OCCLUSION_POSSIBLE_TOP;

	if( yr.ymin.loc_max > y2 )								// entire section can potentially be culled
		flags |= FCLIP_OCCLUSION_POSSIBLE_BOTTOM;

	return flags;
}

int VariableCodeGen::EstimateDifficulty(const YRange &yr)
{
	int cases = 0;
	for( int i=0; i<(int)sections.size(); i++ )
	{
		WallSection &ws = sections[i];
		int flags = ComputeSectionClipFlags(i, yr);

		if( !flags )
			continue;

		int drawcases = 1;
		if( flags & FCLIP_TOP_UNDECIDED				) drawcases *= 2;
		if( flags & FCLIP_BOTTOM_UNDECIDED			) drawcases *= 2;
		if( flags & FCLIP_OCCLUSION_POSSIBLE_TOP	) drawcases++;
		if( flags & FCLIP_OCCLUSION_POSSIBLE_BOTTOM	) drawcases++;

		if( drawcases > 1 )
			cases += drawcases-1;
	}

	return cases;
}

string VariableCodeGen::SectionDebugString(WallSection &ws, int clip_flags)
{
	string s;

	if( clip_flags & FCLIP_TOP_ALWAYS ) s += "[";			// top clipping can be always enabled
	if( clip_flags & FCLIP_TOP_UNDECIDED ) s += "{";		// undecided top clipping

	switch( ws.drawtype )
	{
	case DRAW_SOLID_CEIL:	s += "Ceil";		break;
	case DRAW_SOLID_FLOOR:	s += "Floor";		break;
	case DRAW_SKY:			s += "Sky";			break;
	case DRAW_TEX_UPPER:	s += "Upper";		break;
	case DRAW_TEX_LOWER:	s += "Lower";		break;
	case DRAW_GAP:			s += "gap";			break;
	default:				s += "xxx";			break;
	}

	if( clip_flags & FCLIP_BOTTOM_ALWAYS ) s += "]";		// bottom clipping can be always enabled
	if( clip_flags & FCLIP_BOTTOM_UNDECIDED ) s += "}";		// undecided bottom clipping

	if( clip_flags & FCLIP_OCCLUSION_POSSIBLE_MASK )		// entire section can potentially be culled
		s += "?";

	if( !(clip_flags & FCLIP_VISIBLE) )						// entire section is hidden
		s += "-";

	return s;
}

string VariableCodeGen::SectionsDebugString(const YRange &yr)
{
	string s;
	for( int i=0; i<(int)sections.size(); i++ )
	{
		WallSection &ws = sections[i];
		int flags = ComputeSectionClipFlags(i, yr);
		string w = SectionDebugString(ws, flags);

		if( !flags )
			continue;

		if( s.size() )
			s += " ";

		s += w;
	}

	return s;
}

string VariableCodeGen::VariableRangeDebugString(const EstimatedRange &er, const char *varname)
{
	// == yindex*2 + 1			-->  on yindex split
	//	0	1	2	3	4	5	6	7	8
	//	-	y0	*	y1	*	y2	*	y3	-
	string s;
	if( er.loc_min > 0 )
	{
		if( er.loc_min & 1 )
			s = format("y%d <= %s", (er.loc_min-1)/2, varname);
		else
			s = format("y%d < %s", (er.loc_min-1)/2, varname);
	}

	if( er.loc_max <= max_yindex*2+1 )
	{
		if( s.size()==0 )
			s = varname;

		if( er.loc_max & 1 )
			s += format(" <= y%d", er.loc_max/2);
		else
			s += format(" < y%d", er.loc_max/2);
	}

	return s;	// + format(" [%d;%d]", er.loc_min, er.loc_max);
}

string VariableCodeGen::RangeDebugString(const YRange &yr)
{
	string s1 = VariableRangeDebugString(yr.ymin, "ymin");
	string s2 = VariableRangeDebugString(yr.ymax, "ymax");
	if( s1.size()==0 && s2.size()==0 )
		return "ymin/ymax not checked";

	if( s1.size()!=0 && s2.size()!=0 )
		return s1 + " && " + s2;

	return s1 + s2;		// only one is filled
}

bool VariableCodeGen::Error(const string &err)
{
	throw "ERROR: "+err;
	return false;
}

void VariableCodeGen::AssureYMin(CodeGenContext &ctx)
{
	using namespace instructions;

	if( opt_yminmax_common )
	{
		// ymin/ymax are kept in one D register

		if( ctx.ymin_reg.IsReg() )			// already allocated?
		{
			if( ctx.ymax_selected )			// not selected?
			{
				as << swp.w(ctx.ymin_reg) << "Select ymin";
				ctx.ymax_selected = false;
			}
			return;
		}

		if( ctx.ymax_reg.IsReg() )					// ymin is not ready, but is ymax allocated?
		{
			ctx.ymin_reg = ctx.ymax_reg;
			if( ctx.ymax_selected )			// not selected?
			{
				as << swp.w(ctx.ymin_reg) << "Select ymin";
			}
		}

		ctx.ymax_selected = false;
		// use old code to allocate (if not present) and load ymin
	}
	else
	{
		if( ctx.ymin_reg.IsReg() )
			return;
	}

	if( !ctx.AllocReg(ctx.ymin_reg, false) )
		Error("AssureYMin: no free D-regs");

	as << mov.w(column_ymin, ctx.ymin_reg) << "Load ymin";
}

void VariableCodeGen::AssureYMax(CodeGenContext &ctx)
{
	using namespace instructions;

	if( opt_yminmax_common )
	{
		// ymin/ymax are kept in one D register

		if( ctx.ymax_reg.IsReg() )			// already allocated?
		{
			if( !ctx.ymax_selected )		// not selected?
			{
				as << swp.w(ctx.ymax_reg) << "Select ymax";
				ctx.ymax_selected = true;
			}
			return;
		}

		if( ctx.ymin_reg.IsReg() )					// ymax is not ready, but is ymin allocated?
		{
			ctx.ymax_reg = ctx.ymin_reg;
			if( !ctx.ymax_selected )			// not selected?
			{
				as << swp.w(ctx.ymax_reg) << "Select ymax";
			}
		}

		ctx.ymax_selected = true;
		// use old code to allocate (if not present) and load ymax
	}
	else
	{
		if( ctx.ymax_reg.IsReg() )
			return;
	}

	if( !ctx.AllocReg(ctx.ymax_reg, false) )
		Error("AssureYMin: no free D-regs");

	as << mov.w(column_ymax, ctx.ymax_reg) << "Load ymax";
}

void VariableCodeGen::Gen_InitTracer(CodeGenContext &ctx, const EAddr &yreg, const EAddr &ysource, const char *name, const char *prev_name, const EAddr &prev_reg)
{
	using namespace instructions;
	as.Assemble(";");
	as.Assemble("; ======== init "+string(name)+" tracer ========");

	EAddr scale_reg = d4;
	EAddr temp;
	EAddr slope = reg_color_fill;	// not yet initialized, so free to use

	if( !ctx.AllocReg(temp, false) ) Error("Can't alloc temp register for init "+string(name));
	//if( !ctx.AllocReg(slope, false) ) Error("Can't alloc slope register for init "+string(name));


	as << mov.w(scale_reg, yreg) << "compute starting Y";
	as << lsr.w(imm(5), yreg);
	as << mov.w(ysource, temp);
	as << sub.w(label("_view_pos_z"), temp);
	as << muls.w(temp, yreg);

	as << mov.w(label("_asm_line_s2"), slope) << "compute Y slope";
	as << lsr.w(imm(5), slope);
	as << muls.w(temp, slope);
	as << sub.l(yreg, slope);
	as << divs.w(label("_asm_line_xlen"), slope);

	as << mov.b(slope, label(prev_name, 2)) << "subpixel step";
	as << lsr.w(imm(8), slope);
	as << ext.w(slope);
	as << mov.w(slope,label(name,4)) << "pixel step";

	as << add.l(imm("ENGINE_Y_MID*256+128"), yreg) << "24.8 fixed";
	as << lsl.l(imm(8), yreg) << "16.16 fixed";

	as << swp.w(prev_reg);
	as << mov.w(yreg, prev_reg);
	as << swp.w(prev_reg);

	as << sub.w(yreg, yreg);
	as << swp.w(yreg) << "current Y value";

	ctx.FreeReg(temp);
	//ctx.FreeReg(slope);
}

void VariableCodeGen::Gen_ColorFill(CodeGenContext &ctx, int wall_section_index, int clip_flags)
{
	using namespace instructions;
	WallSection &ws = sections[wall_section_index];

	as.Assemble("; -------- "+SectionDebugString(ws, clip_flags)+" --------");
	if( !(clip_flags & FCLIP_VISIBLE) || (clip_flags & (FCLIP_TOP_UNDECIDED | FCLIP_BOTTOM_UNDECIDED | FCLIP_OCCLUSION_POSSIBLE_MASK)) )
	{
		Error(format("Gen_ColorFill: can't generate code for %s flag combination %02X", SectionDebugString(ws, clip_flags).c_str(), clip_flags));
		return;
	}

	EAddr fill_routine;
	if( ws.drawtype != DRAW_SKY )
	{
		// Solid color
		if( !ctx.reg_color_fill_addr.IsValid() )
		{
			if( !ctx.AllocReg(ctx.reg_color_fill_addr, true) )
				Error("Gen_ColorFill: can't allocate color fill routine register");
			as << lea.l(label("_FUNC_COLORFILL+ENGINE_Y_MAX*2"), ctx.reg_color_fill_addr) << "Color filler func";
		}
		fill_routine = ctx.reg_color_fill_addr;
	}
	else
	{
		// Sky
		if( !ctx.reg_sky_fill_addr.IsValid() )
		{
			if( !ctx.AllocReg(ctx.reg_sky_fill_addr, true) )
				Error("Gen_ColorFill: can't allocate sky copy routine register");
			as << lea.l(label("_FUNC_SKYCOPY+ENGINE_Y_MAX*2"), ctx.reg_sky_fill_addr) << "Sky copy func";
		}
		fill_routine = ctx.reg_sky_fill_addr;

		as << mov.l(mem_postinc(reg_sky_columns), reg_texture_source);
	}



	if( ws.drawtype == DRAW_SOLID_CEIL )
		as << swp.w(reg_color_fill) << "select ceiling color";


	// Compute double negative length of the color stripe = 2*(ytop - ybottom)
	EAddr len_reg;
	EAddr ystart;
	EAddr yend;

	if( wall_section_index > 0 )
		ystart = ctx.y_reg[sections[wall_section_index-1].yend_index];

	if( sections[wall_section_index].yend_index>=0 && sections[wall_section_index].yend_index<=max_yindex)
		yend = ctx.y_reg[sections[wall_section_index].yend_index];

	if( clip_flags & FCLIP_TOP_ALWAYS )
	{
		AssureYMin(ctx);
		len_reg = ctx.ymin_reg;

		if( ws.drawtype == DRAW_SKY )
			as << add.w(ctx.ymin_reg, reg_texture_source) << "sky offset by ymin";

		if( clip_flags & FCLIP_BOTTOM_ALWAYS )
		{
			if( opt_yminmax_common )
			{
				// we can't compute ymin-ymax difference because they are in one register - use temp
				len_reg = EAddr();
				if( !ctx.AllocReg(len_reg, false) ) Error("Can't alloc length register for color fill "+SectionDebugString(ws, clip_flags));
				as << mov.w(ctx.ymin_reg, len_reg);
			}
			AssureYMax(ctx);
			as << sub.w(ctx.ymax_reg, len_reg);
		}
		else
		{
			if( !yend.IsValid() ) Error("yend not valid");
			as << sub.w(yend, len_reg);
		}
	}
	else
	{
		if( !ystart.IsValid() ) Error("ystart not valid");

		if( ws.drawtype == DRAW_SKY )
			as << add.w(ystart, reg_texture_source) << "sky offset by ystart";

		if( clip_flags & FCLIP_BOTTOM_ALWAYS )
		{
			// use bottom register
			AssureYMax(ctx);
			len_reg = ctx.ymax_reg;
			as << sub.w(ystart, len_reg);
			as << neg.w(len_reg);
		}
		else
		{
			if( !yend.IsValid() ) Error("ystart not valid");
			if( !ctx.AllocReg(len_reg, false) ) Error("Can't alloc length register for color fill "+SectionDebugString(ws, clip_flags));

			as << mov.w(ystart, len_reg);
			as << sub.w(yend, len_reg);
		}

	}
	as << add.w(len_reg, len_reg);

	string return_label = format(".ret_%d", label_counter++);
	as << lea.l(label(return_label), a6);
	as << jmp(mem_idx_w(0, fill_routine, len_reg));
	as.Assemble(return_label+":");

	if( ws.drawtype == DRAW_SOLID_CEIL )
		as << swp.w(reg_color_fill) << "deselect ceiling color";

	// Free all temp registers used
	ctx.FreeReg(ctx.ymin_reg);					// ymin is always freed after first use
	ctx.FreeReg(len_reg);
	if( clip_flags & FCLIP_BOTTOM_ALWAYS )
		ctx.FreeReg(ctx.ymax_reg);				// free ymax when used
}

void VariableCodeGen::Gen_GapSkip(CodeGenContext &ctx, int wall_section_index, int clip_flags)
{
	using namespace instructions;
	WallSection &ws = sections[wall_section_index];

	as.Assemble("; -------- "+SectionDebugString(ws, clip_flags)+" --------");
	if( !(clip_flags & FCLIP_VISIBLE) || (clip_flags & (FCLIP_TOP_UNDECIDED | FCLIP_BOTTOM_UNDECIDED | FCLIP_OCCLUSION_POSSIBLE_MASK)) )
	{
		Error(format("Gen_GapSkip: can't generate code for %s flag combination %02X", SectionDebugString(ws, clip_flags).c_str(), clip_flags));
		return;
	}

	// Write current A7 to render column table
	// Update rc_ymin/rc_ymax with the current range
	// Add 2*(ybottom - ytop) to A7
	//
	EAddr ystart;
	EAddr yend;

	if( wall_section_index > 0 )
		ystart = ctx.y_reg[sections[wall_section_index-1].yend_index];

	if( sections[wall_section_index].yend_index>=0 && sections[wall_section_index].yend_index<=max_yindex )
		yend = ctx.y_reg[sections[wall_section_index].yend_index];

	if( clip_flags & FCLIP_TOP_ALWAYS )
	{
		if( clip_flags & FCLIP_BOTTOM_ALWAYS )
		{
			as.Assemble("; nothing to be done");
			ctx.FreeReg(ctx.ymax_reg);				// no longer needed
		}
		else
		{
			// Don't update rc_ymin, because it clipped and stays
			
			if( !yend.IsValid() ) Error("yend not valid");
			AssureYMin(ctx);
			as << mov.w(yend, column_ymax) << "Update rc_ymax with yend";

			as << sub.w(yend, ctx.ymin_reg) << "Add 2*(yend - rc_ymin) to A7";
			as << neg.w(ctx.ymin_reg);
			as << add.w(ctx.ymin_reg, ctx.ymin_reg);
			as << add.w(ctx.ymin_reg, a7);
		}
	}
	else
	{
		if( clip_flags & FCLIP_BOTTOM_ALWAYS )
		{
			// Don't update rc_ymax, because it clipped and stays

			if( !ystart.IsValid() ) Error("ystart not valid");
			AssureYMax(ctx);
			as << mov.w(ystart, column_ymin) << "Update rc_ymin with ystart";
			as << mov.l(a7, column_drawptr) << "Set future drawing start pointer to current pos";

			//// Don't update A7 - no further drawing
			// as << sub.w(ystart, ctx.ymax_reg) << "Add 2*(rc_ymax - ystart) to A7";
			// as << add.w(ctx.ymax_reg, ctx.ymax_reg);
			// as << add.w(ctx.ymax_reg, a7);

			ctx.FreeReg(ctx.ymax_reg);				// destroyed in the process
		}
		else
		{
			EAddr len_reg;

			if( !ystart.IsValid() ) Error("ystart not valid");
			if( !yend.IsValid() ) Error("ystart not valid");
			if( !ctx.AllocReg(len_reg, false) ) Error("Can't alloc length register for color fill "+SectionDebugString(ws, clip_flags));

			as << mov.w(ystart, column_ymin) << "Update rc_ymin with ystart";
			as << mov.l(a7, column_drawptr) << "Set future drawing start pointer to current pos";

			as << mov.w(yend, column_ymax) << "Update rc_ymax with yend";
			
			as << mov.w(yend, len_reg) << "Add 2*(yend - ystart) to A7";
			as << sub.w(ystart, len_reg);
			as << add.w(len_reg, len_reg);
			as << add.w(len_reg, a7);

			ctx.FreeReg(len_reg);
		}

	}
//	as << add.w(len_reg, len_reg);

//	string return_label = format(".ret_%d", label_counter++);
//	as << lea.l(label(return_label), a6);
//	as << jmp(mem_idx_w(0, ctx.xr_ceil_fill_addr, len_reg));
//	as.Assemble(return_label+":");



	// Free all temp registers used
	ctx.FreeReg(ctx.ymin_reg);					// ymin is always freed after first use
}


void VariableCodeGen::Gen_DrawCore(CodeGenContext &ctx, const YRange &yr)
{
	using namespace instructions;

	as.Assemble("; ======== "+SectionsDebugString(yr)+" ========");
	
	bool had_gap = false;
	bool had_sky = false;
	for( int i=0; i<(int)sections.size(); i++ )
	{
		WallSection &ws = sections[i];

		int clip_flags = ComputeSectionClipFlags(i, yr);
		if( !clip_flags ) continue;

		if( ws.drawtype==DRAW_SKY )
		{
			Gen_ColorFill(ctx, i, clip_flags);
			had_sky = true;
		}
		if( ws.drawtype==DRAW_SOLID_CEIL || ws.drawtype==DRAW_SOLID_FLOOR )
		{
			Gen_ColorFill(ctx, i, clip_flags);
		}
		else if( ws.drawtype==DRAW_GAP )
		{
			Gen_GapSkip(ctx, i, clip_flags);
			had_gap = true;
		}
	}

	if( !had_sky && reg_sky_columns.IsValid() )
		as << add.l(imm(4), reg_sky_columns) << "SKY was skipped";

	if( !had_gap )
		as << clr.l(column_ymin) << "Disable further rendering - clear ymin/ymax";



	as << bra(label(".end_loop"));
}

void VariableCodeGen::Gen_GenerateIfLadderRec(CodeGenContext &ctx, const YRange &yr, const char *label_name)
{
	using namespace instructions;

	as.Assemble(";");
	if( label_name )
		as.Assemble(string(label_name)+": ; -------- "+RangeDebugString(yr)+" --------");
	else
		as.Assemble("; -------- "+RangeDebugString(yr)+" --------");

	int cases = EstimateDifficulty(yr);
	if( cases <= 1 )
	{
		Gen_DrawCore(ctx, yr);
		return;
	}


	// find perfect split
	YRange ytrue, yfalse;
	int best_cost = 1000000000;
	int best_comp = 0;
	int best_yindex = 0;
	int best_cases1 = 0;
	int best_cases2 = 0;

	for( int comp=0; comp<4; comp++ )
		for( int yindex=0; yindex<=max_yindex; yindex++ )
		{
			if( !SplitWithComparison(yr, (compare_t)comp, yindex*2+1, ytrue, yfalse) )
				continue;

			int cases1 = EstimateDifficulty(ytrue);
			int cases2 = EstimateDifficulty(yfalse);
			int cost = max(cases1, cases2)*100 + cases1 + cases2;

			if( cost<best_cost || (cost==best_cost && comp>best_comp) )
			{
				best_cost = cost;
				best_comp = comp;
				best_yindex = yindex;
				best_cases1 = cases1;
				best_cases2 = cases2;
			}
		}

	if( best_cost >= 1000000000 )
	{
		Error("trying to draw "+SectionsDebugString(yr));
		return;
	}


	SplitWithComparison(yr, (compare_t)best_comp, best_yindex*2+1, ytrue, yfalse);

	EAddr yreg;
	if( best_comp < 2 )
	{
		AssureYMin(ctx);
		yreg = ctx.ymin_reg;
	}
	else
	{
		AssureYMax(ctx);
		yreg = ctx.ymax_reg;
	}

	as << cmp.w(ctx.y_reg[best_yindex], yreg) << format("%s%sy%d",
	  	best_comp>=2 ? "ymax" : "ymin",
	  	best_comp&1 ? ">=" : ">",
	 	best_yindex
	 );

	string else_label = format(".cmp_%d", label_counter++);
	if( best_comp & 1 )
		as << bge(label(else_label));
	else
		as << bgt(label(else_label));

	CodeGenContext else_ctx = ctx;
	Gen_GenerateIfLadderRec(ctx, yfalse, NULL);
	Gen_GenerateIfLadderRec(else_ctx, ytrue, else_label.c_str());
}
