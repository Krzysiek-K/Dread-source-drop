
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


bool VariableCodeGen::CodeGenContext::IsFree(const EAddr &reg)
{
	if( !reg.IsReg() )
		return true;

	return (free_regs & (1<<reg.regnum)) != 0;
}

bool VariableCodeGen::CodeGenContext::AllocReg(EAddr &reg, bool is_addr_reg)
{
	if( reg.IsReg() )
		return true;

	for( int pass=0; pass<2; pass++ )
	{
		int mask = pass ? free_regs : free_regs & ~avoid_regs;

		for( int i=0; i<8; i++ )
		{
			int regnum = i + (is_addr_reg ? 8 : 0);
			if( mask & (1<<regnum) )
			{
				reg = is_addr_reg ? EAddr::a(i) : EAddr::d(i);
				free_regs &= ~(1<<regnum);
				UpdateInternalStats();
				return true;
			}
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

void VariableCodeGen::CodeGenContext::SetAvoidReg(const EAddr &reg)
{
	if( !reg.IsReg() || uint32_t(reg.regnum)>16 )
		return;

	avoid_regs |= 1 << reg.regnum;
}

void VariableCodeGen::CodeGenContext::UpdateInternalStats()
{
	int count = free_regs & 0xFFFF;
	count = ((count>>1) & 0x5555) + (count & 0x5555);
	count = ((count>>2) & 0x3333) + (count & 0x3333);
	count = ((count>>4) & 0x0F0F) + (count & 0x0F0F);

	int free_dregs = count & 0xFF;
	int free_aregs = (count >> 8) & 0xFF;
	if( free_dregs < min_free_dregs ) min_free_dregs = free_dregs;
	if( free_aregs < min_free_aregs ) min_free_aregs = free_aregs;
}

void VariableCodeGen::CodeGenContext::UpdateStats(int &free_dregs, int &free_aregs)
{
	if( min_free_dregs < free_dregs ) free_dregs = min_free_dregs;
	if( min_free_aregs < free_aregs ) free_aregs = min_free_aregs;
}

EAddr VariableCodeGen::CodeGenContext::get_column_value(int hardcoded_offset, const char *label_def, bool is_long)
{
	using namespace instructions;

	if( hardcoded_offset == render_column_incremented )
		return mem(a4);		// VariableCodeGen::reg_render_column

	if( !render_column_incremented )
		return mem(label_def, a4);

	return mem(format("%s%+d", label_def, -render_column_incremented), a4);
}






// ================================================================ VariableCodeGen - public ================================================================



void VariableCodeGen::Initialize(const int *draw_list, bool use_perspective)
{
	sections.clear();
	max_yindex = -1;
	uses_sky			= false;
	uses_texcoords		= false;
	uses_tex_upper		= false;
	uses_tex_lower		= false;
	uses_solid_ceil		= false;
	uses_solid_floor	= false;

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
			sections.push_back(s);

			if( s.drawtype == DRAW_SKY			) uses_sky				= true;
			if( s.drawtype == DRAW_SOLID_CEIL	) uses_solid_ceil		= true;
			if( s.drawtype == DRAW_SOLID_FLOOR	) uses_solid_floor		= true;
			if( s.drawtype == DRAW_TEX_LOWER	) uses_tex_lower = uses_texcoords = true;
			if( s.drawtype == DRAW_TEX_UPPER	) uses_tex_upper = uses_texcoords = true;
		}

		draw_list++;
	}


	uses_persp_correction = use_perspective && uses_texcoords;
	opt_texcoord_word = !uses_persp_correction;

	opt_keep_computed_texcoords = uses_texcoords && uses_tex_lower && uses_tex_upper;
}

int VariableCodeGen::EstimateMethodCost()
{
	int cost = 0;
	for( int i=0; i<(int)sections.size(); i++ )
	{
		switch( sections[i].drawtype )
		{
		case DRAW_SKY:				cost += 2;				break;
		case DRAW_SOLID_CEIL:		cost += 2;				break;
		case DRAW_SOLID_FLOOR:		cost += 2;				break;
		case DRAW_TEX_UPPER:		cost += 3;				break;
		case DRAW_TEX_LOWER:		cost += 3;				break;
		case DRAW_GAP:				cost += 1;				break;
		}
	}
	return cost;
}


bool VariableCodeGen::EnableOptimizations(int free_dregs, int free_aregs)
{
	bool opt = false;

	//if( free_dregs>0 && opt_yminmax_common )
	//{
	//	printf("a");
	//	opt_yminmax_common = false;
	//	free_dregs--;
	//	opt = true;
	//}
	
	if( free_dregs>0 && (uses_solid_floor || uses_solid_ceil) && opt_indirect_fill )
	{
		printf("b");
		opt_indirect_fill = false;
		free_dregs--;
		opt = true;
	}

	if( free_dregs>0 && opt_cmpa_loop )
	{
		printf("c");
		opt_cmpa_loop = false;
		free_dregs--;
		free_aregs++;
		opt = true;
	}

	if( free_aregs>0 && !opt_keep_fill_fn && (uses_solid_ceil || uses_solid_floor) )
	{
		printf("d");
		opt_keep_fill_fn = true;
		free_aregs--;
		opt = true;
	}

	if( free_aregs>0 && !opt_keep_fn_scalers_base && (uses_tex_upper || uses_tex_lower) )
	{
		printf("e");
		opt_keep_fn_scalers_base = true;
		free_aregs--;
		opt = true;
	}

	return opt;
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


bool VariableCodeGen::GenerateCode()
{
	using namespace instructions;

	try {

		// -------- Init assembler --------
		as.Clear();
		as.SetZone(ZONE_SETUP);
		free_dregs = 8;
		free_aregs = 8;
		label_counter = 1;


		// -------- Init context --------
		CodeGenContext ctx;
		ctx.SetFreeReg(d0);
		ctx.SetFreeReg(d1);
		ctx.SetFreeReg(d2);
		if( !uses_texcoords ) ctx.SetFreeReg(d6);
		if( !uses_texcoords ) ctx.SetFreeReg(d7);

		ctx.SetFreeReg(a0);
		ctx.SetFreeReg(a1);
		if( !uses_sky && !uses_texcoords ) ctx.SetFreeReg(a2);
		ctx.SetFreeReg(a3);



		// -------- Preamble --------
		as.Assemble("; ================================ "+GetPrintName()+" ================================");
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
			if( !ctx.AllocReg(ctx.reg_sky_columns, true) )
				Error("Can't allocate reg for sky source");

			as << lsr.w(imm(1), d2) << "init SKY";
			as << mov.l(label("_e_skyptr"), ctx.reg_sky_columns);
			as << add.w(d2, ctx.reg_sky_columns);
		}

		if( uses_texcoords )
		{
			if( opt_immediate_texstep )
			{
				if( opt_texcoord_word )
					as << mov.w(reg_texstep, label(".trace_tex", 2));
				else
					as << mov.l(reg_texstep, label(".trace_tex", 2));
				ctx.SetFreeReg(reg_texstep);
			}
		}

		if( opt_cmpa_loop )
		{
			if( !ctx.AllocReg(ctx.reg_cmpa_end, true) )
				Error("Can't allocate reg for loop end pointer");

			as << mov.l(reg_render_column, ctx.reg_cmpa_end) << "end = start + count*8";
			as << lsl.w(imm(3), d3);
			if( opt_step_column_reg )
				as << add.l(imm(1), d3);
			as << add.w(d3, ctx.reg_cmpa_end);
			ctx.SetFreeReg(d3);
		}


		// -------- Init textures --------
		for( int tex=0; tex<2; tex++ )
		{
			bool used = tex ? uses_tex_lower : uses_tex_upper;
			if( !used ) continue;
			//	move.l		line_tex_upper(a5), a1				;		asm_tex_base = line->tex_upper + ((-40 - line->y1)>>STRETCH_SCALING);
			//	move.w		_view_pos_z, d1						;
			//	asr.w		#1, d1
			//	add.w		d1, a1								;
			//	move.l		a1, _asm_tex_base					;

			EAddr temp;
			EAddr tempa;
			if( !ctx.AllocReg(temp, false) ) Error("Can't allocate D-reg for texture data source computation");
			if( !ctx.AllocReg(tempa, true) ) Error("Can't allocate A-reg for texture data source computation");

			if( tex )	as << mov.l(mem("line_tex_lower", reg_line_def), tempa) << "Lower texture";
			else		as << mov.l(mem("line_tex_upper", reg_line_def), tempa) << "Upper texture";
			as << mov.w(label("_view_pos_z"), temp);
			as << asr.w(imm(1), temp);
			as << add.w(temp, tempa);
			as << mov.l(tempa, label(tex ? ".asm_tex_base2" : ".asm_tex_base"));

			ctx.FreeReg(temp);
			ctx.FreeReg(tempa);
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
				mem(format("line_y%d",yindex_source_height[i]), reg_line_def),
				_tracer_name(i).c_str(),
				_tracer_name(i-1).c_str(),
				(i-1>=0) ? ctx.y_reg[i-1] : reg_cur_size,
				(i==max_yindex)
			);
		}
		as << mov.w(label("_asm_line_ds"), label(".trace_s", 4));


		// -------- Init flat colors --------
		if( !opt_indirect_fill )
		{
			if( uses_solid_ceil ) as << mov.b(mem("line_ceil_col", reg_line_def), reg_color_fill);
			if( uses_solid_ceil && uses_solid_floor ) as << swp.w(reg_color_fill);
			if( uses_solid_floor ) as << mov.b(mem("line_floor_col", reg_line_def), reg_color_fill);
			ctx.SetFreeReg(reg_line_def);
		}
		else
		{
			ctx.SetFreeReg(reg_color_fill);
			ctx.SetAvoidReg(reg_color_fill);
		}

		if( opt_keep_fill_fn )
			AssureColorFill(ctx);

		if( opt_keep_fn_scalers_base )
			if( ctx.AllocReg(ctx.reg_fn_scalers_base, true) )
				as << lea.l(label("_FN_SCALERS"), ctx.reg_fn_scalers_base);


		// -------- Header end --------
		as.Assemble(" TRAP_LINEDRAW_SETUP_DONE");
		as << bra(label(".start")) << "End of header";
		as.Assemble(";");
		as.Assemble(";");

		ctx.min_free_aregs = 8;
		ctx.min_free_dregs = 8;
		ctx.UpdateInternalStats();



		// -------- Loop start --------
		as.SetZone(ZONE_INCREMENT);
		as.Assemble(".loop: ; ======== LOOP START ========");
		if( !opt_step_column_reg )
			as << add.w(imm(8), reg_render_column);

		as.Assemble(".trace_s:");
		as << add.l(imm("$FF00FFFF"), reg_cur_size) << "s1 += ds";
		
		for( int i=0; i<=max_yindex; i++ )
		{
			EAddr temp;
			if( !ctx.AllocReg(temp, false) )
				Error(format("Can't allocate temp reg to step tracer y%d", i));

			as.Assemble(_tracer_name(i)+":");
			if( i==max_yindex )
			{
				as << mov.w(imm("$FF00"), temp);
				as << addx.w(temp, ctx.y_reg[i]);
			}
			else
			{
				as << mov.l(imm("$FF000000"), temp);
				as << addx.l(temp, ctx.y_reg[i]);
			}

			ctx.FreeReg(temp);
		}

		if( uses_texcoords )
		{
			as.Assemble(".trace_tex:");
			if( opt_immediate_texstep )
			{
				if( opt_texcoord_word )
					as << add.w(imm("$8000"), reg_texcoord) << "u += du";
				else
					as << add.l(imm("$80000000"), reg_texcoord) << "u += du";
			}
			else
			{
				if( opt_texcoord_word )
					as << add.w(reg_texstep, reg_texcoord) << "u += du";
				else
					as << add.l(reg_texstep, reg_texcoord) << "u += du";
			}
		}


		as.SetZone(ZONE_GENERIC_LOOP);
		as.Assemble(";");
		as.Assemble(".start: ; ======== FIRST ITERATION STARTS HERE ========");
		if( opt_step_column_reg )
		{
			as << mov.l(mem_postinc(reg_render_column), a7) << "byte *dst = render_buffer + offs[x];";
			ctx.render_column_incremented += 4;
		}
		else
			as << mov.l(ctx.get_column_drawptr(), a7) << "byte *dst = render_buffer + offs[x];";


		YRange yr;
		yr.ymin.loc_min = 0;
		yr.ymin.loc_max = max_yindex*2 + 2;
		yr.ymax.loc_min = 0;
		yr.ymax.loc_max = max_yindex*2 + 2;
		Gen_GenerateIfLadderRec(ctx, yr, NULL);


		// -------- END --------

		as.SetZone(ZONE_GENERIC_LOOP);
		as.Assemble(";");
		as.Assemble(";");
		as.Assemble(".end_loop: ; ======== LOOP END ========");
		if( opt_cmpa_loop )
		{
			as << cmp.l(ctx.reg_cmpa_end, reg_render_column) << "render_column ? render_end";
			as << blt(label(".loop")) << "continue if <";
		}
		else
			as << dbra.w(d3,label(".loop"));

		as.SetZone(ZONE_OUTRO);

		as.Assemble(".end: ; Cleanup & exit");
		as.Assemble(" move.l _a7_save, a7");

		if( save_regs )
			as.Assemble(" movem.l (a7)+, d2-d7/a2-a6");
		as.Assemble(" rts");

		// Extra pc-relative memory
		if( uses_tex_upper )
		{
			as.Assemble(".asm_tex_base:");
			as.Assemble(" move.w #0, d0 ; reserve 32-bit");
		}
		if( uses_tex_lower )
		{
			as.Assemble(".asm_tex_base2:");
			as.Assemble(" move.w #0, d0 ; reserve 32-bit");
		}


		// Allocate regs
		//	int free_regs = 0;
		//	free_regs |= 1<<0;	// d0
		//	free_regs |= 1<<1;	// d1
		//	free_regs |= 1<<2;	// d2
		//	//if( !ceil_mode && !floor_mode ) free_regs |= 1<<5;	// d5
		//	free_regs |= 0x100<<0;	// a0
		//	free_regs |= 0x100<<1;	// a1
		//	//if( !upper_mode && !lower_mode ) free_regs |= 0x100<<2;	// a2
		//	//if( !sky_mode ) free_regs |= 0x100<<3;	// a3

		//as.AllocateRegs(free_regs);
		as.AllocateRegs(0x3FFF);
		as.ConvertOpcodes();

		//as.Print();

		CycleStats stats;
		as.CountCycles(stats);
		printf(" - %4d/%4d %d/%d", stats.GetLoopMin(), stats.GetLoopMax(), free_dregs, free_aregs);
	}
	catch( const string &e )
	{
		as.Print();
		as.SaveFunction("error_code.s", "ERROR");
		printf("\nFatal ERROR: %s\n", e.c_str());
		return false;
	}
	return true;
}




// ================================================================ VariableCodeGen - protected ================================================================


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

void VariableCodeGen::TryYMinMaxCommonLoad(CodeGenContext &ctx)
{
	using namespace instructions;

	if( !opt_yminmax_common || ctx.ymin_reg.IsReg() || ctx.ymax_reg.IsReg() )
		return;		// disabled or already (partially) done

	if( !ctx.AllocReg(ctx.ymin_reg, false) )
		Error("TryYMinMaxCommonLoad: no free D-regs");
	ctx.ymax_reg = ctx.ymin_reg;

	if( opt_step_column_reg && ctx.render_column_incremented==4 )
	{
		as << mov.l(mem_postinc(reg_render_column), ctx.ymin_reg) << "Load ymin/ymax";		// fast path
		ctx.render_column_incremented += 4;
	}
	else
		as << mov.l(ctx.get_column_ymin(), ctx.ymin_reg) << "Load ymin/ymax";

	ctx.ymax_selected = true;		// big endian load order
}

void VariableCodeGen::AssureYMin(CodeGenContext &ctx)
{
	using namespace instructions;

	if( opt_yminmax_common )
	{
		// ymin/ymax are kept in one D register
		TryYMinMaxCommonLoad(ctx);

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

	as << mov.w(ctx.get_column_ymin(), ctx.ymin_reg) << "Load ymin";
}

void VariableCodeGen::AssureYMax(CodeGenContext &ctx)
{
	using namespace instructions;

	if( opt_yminmax_common )
	{
		// ymin/ymax are kept in one D register
		TryYMinMaxCommonLoad(ctx);

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

	as << mov.w(ctx.get_column_ymax(), ctx.ymax_reg) << "Load ymax";
}

bool VariableCodeGen::AssureColorFill(CodeGenContext &ctx)
{
	using namespace instructions;

	if( ctx.reg_color_fill_addr.IsValid() )
		return true;

	if( !ctx.AllocReg(ctx.reg_color_fill_addr, true) )
		return false;

	as << lea.l(label("_FUNC_COLORFILL+ENGINE_Y_MAX*2"), ctx.reg_color_fill_addr) << "Color filler func";
	return true;
}


//VariableCodeGen::TracerInfo *VariableCodeGen::Tracer_AssureSubpixelComponent()
//{
//	if( tracers.size()>0 && !tracers.back().use_long )
//	{
//		tracers.back().handle_next_subpixel = true;
////		return;
//	}
//
//	return NULL;
//}
//
//VariableCodeGen::TracerInfo *VariableCodeGen::Tracer_AddGeneric(EAddr reg, EAddr step_source, bool bit32, const string &name)
//{
//	TracerInfo tracer;
//	tracer.mode = TRACER_GENERIC;
//	tracer.name = name;
//	tracer.label = ".tracer_" + name;
//	tracer.tracer_reg = reg;
//	tracer.step_source = step_source;
//	tracer.use_long = bit32;
//	tracers.push_back(tracer);
//	return &tracers.back();
//}
//
//VariableCodeGen::TracerInfo *VariableCodeGen::Tracer_AddYPos(CodeGenContext &ctx, int index)
//{
//	using namespace instructions;
//
//	TracerInfo tracer;
//	tracer.mode = TRACER_YPOS;
//	tracer.name = format("y%d", index);
//	tracer.label = format(".tracer_y%d", index);
//	tracer.step_source = imm(0);
//	tracer.use_long = false;
//
//	if( !ctx.AllocReg(ctx.y_reg[index], false) )
//		Error(format("Can't allocate reg for tracer y%d", index));
//	tracer.tracer_reg = ctx.y_reg[index];
//
//	tracers.push_back(tracer);
//	return &tracers.back();
//}

//void VariableCodeGen::Tracer_GenInit(CodeGenContext &ctx, int index)
//{
//	using namespace instructions;
//
////	TracerInfo &tr = tracers[index];
////	if( tr.mode == TRACER_YPOS )
////	{
////		Gen_InitTracer(
////			ctx,
////			tr.tracer_reg,
////			//imm(i==0 ? -64 : 0),
////			mem(format("line_y%d", yindex_source_height[i]), a5),
////			_tracer_name(i).c_str(),
////			_tracer_name(i-1).c_str(),
////			(i-1>=0) ? ctx.y_reg[i-1] : reg_cur_size);
////	}
//}


void VariableCodeGen::Gen_InitTracer(CodeGenContext &ctx, const EAddr &yreg, const EAddr &ysource, const char *name, const char *prev_name, const EAddr &prev_reg, bool word_size)
{
	using namespace instructions;
	as.Assemble(";");
	as.Assemble("; ======== init "+string(name)+" tracer ========");

	EAddr temp;
	EAddr slope = reg_color_fill;	// not yet initialized, so free to use

	if( !ctx.AllocReg(temp, false) ) Error("Can't alloc temp register for init "+string(name));
	//if( !ctx.AllocReg(slope, false) ) Error("Can't alloc slope register for init "+string(name));


	as << mov.w(reg_cur_size, yreg) << "compute starting Y";
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
	as << mov.w(slope, label(name, word_size ? 2 : 4)) << "pixel step";

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

void VariableCodeGen::Gen_ColorFill(CodeGenContext &ctx, int wall_section_index, int clip_flags, bool last_draw)
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
		if( !AssureColorFill(ctx) )
			Error("Gen_ColorFill: can't allocate color fill routine register");
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

		as << mov.l(mem_postinc(ctx.reg_sky_columns), reg_texture_source);
	}




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

			// free ymin/ymax early
			ctx.FreeReg(ctx.ymin_reg);
			ctx.FreeReg(ctx.ymax_reg);

			// check for register clash
			if( len_reg.regnum == reg_color_fill.regnum )
			{
				EAddr temp;
				if( !ctx.AllocReg(temp, false) ) Error("Can't alloc length register for color fill clash avoidance "+SectionDebugString(ws, clip_flags));
				as << mov.w(len_reg, temp) << "avoid color reg clash";
				ctx.FreeReg(len_reg);
				len_reg = temp;
			}
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

	if( ws.drawtype != DRAW_SKY )
	{
		if( opt_indirect_fill )
		{
			if( !ctx.IsFree(reg_color_fill) ) Error("Color fill register is taken "+SectionDebugString(ws, clip_flags));

			if( ws.drawtype == DRAW_SOLID_CEIL )
				as << mov.b(mem("line_ceil_col", a5), reg_color_fill);
			else
				as << mov.b(mem("line_floor_col", a5), reg_color_fill);
		}
		else
			if( ws.drawtype == DRAW_SOLID_CEIL && uses_solid_floor )
			{
				as << swp.w(reg_color_fill) << "select ceiling color";
				last_draw = false;	// can't optimize exit - has to unswap
			}
	}

	string return_label = last_draw ? Gen_DrawCore_Finalize(ctx) : format(".ret_%d", label_counter++);
	as << lea.l(pc_rel(return_label), a6);
	as << jmp(mem_idx_w(0, fill_routine, len_reg)) << ("-> "+return_label);
	if( !last_draw )
		as.Assemble(return_label+":");

	if( ws.drawtype == DRAW_SOLID_CEIL && uses_solid_floor && !opt_indirect_fill )
		as << swp.w(reg_color_fill) << "deselect ceiling color";

	// Free all temp registers used
	ctx.FreeReg(ctx.ymin_reg);					// ymin is always freed after first use
	ctx.FreeReg(len_reg);
	if( clip_flags & FCLIP_BOTTOM_ALWAYS )
		ctx.FreeReg(ctx.ymax_reg);				// free ymax when used

	ctx.FreeReg(ctx.reg_sky_fill_addr);
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
			as << mov.w(yend, ctx.get_column_ymax()) << "Update rc_ymax with yend";

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
			as << mov.w(ystart, ctx.get_column_ymin()) << "Update rc_ymin with ystart";
			as << mov.l(a7, ctx.get_column_drawptr()) << "Set future drawing start pointer to current pos";

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
			if( !ctx.AllocReg(len_reg, false) ) Error("Can't alloc length register for gap length "+SectionDebugString(ws, clip_flags));

			as << mov.w(ystart, ctx.get_column_ymin()) << "Update rc_ymin with ystart";
			as << mov.l(a7, ctx.get_column_drawptr()) << "Set future drawing start pointer to current pos";

			as << mov.w(yend, ctx.get_column_ymax()) << "Update rc_ymax with yend";
			
			as << mov.w(yend, len_reg) << "Add 2*(yend - ystart) to A7";
			as << sub.w(ystart, len_reg);
			as << add.w(len_reg, len_reg);
			as << add.w(len_reg, a7);

			ctx.FreeReg(len_reg);
		}

	}
//	as << add.w(len_reg, len_reg);

//	string return_label = format(".ret_%d", label_counter++);
//	as << lea.l(pc_rel(return_label), a6);
//	as << jmp(mem_idx_w(0, ctx.xr_ceil_fill_addr, len_reg));
//	as.Assemble(return_label+":");



	// Free all temp registers used
	ctx.FreeReg(ctx.ymin_reg);					// ymin is always freed after first use
}

void VariableCodeGen::Gen_TextureFill(CodeGenContext &ctx, int wall_section_index, int clip_flags, bool expect_next_texture)
{
	using namespace instructions;
	WallSection &ws = sections[wall_section_index];

	as.SetZone(ZONE_TEXDRAW);

	as.Assemble("; -------- "+SectionDebugString(ws, clip_flags)+" --------");
	if( !(clip_flags & FCLIP_VISIBLE) || (clip_flags & (FCLIP_TOP_UNDECIDED | FCLIP_BOTTOM_UNDECIDED | FCLIP_OCCLUSION_POSSIBLE_MASK)) )
	{
		Error(format("Gen_TextureFill: can't generate code for %s flag combination %02X", SectionDebugString(ws, clip_flags).c_str(), clip_flags));
		return;
	}

	// Fill routine
	EAddr fill_routine;
	{
		//    move.w      d4, d0                              ;1 d--2----- a01-3----   4   int size = (((word)s1)>>3) & ~3;
		//    lsr.w       #3, d0                              ;1 d--2----- a01-3----  12   
		//    and.w       #$FFFC, d0                          ;1 d--2----- a01-3----   8   
		//    lea.l       _SIZE_LINE_CHEAT, a4                ;1 d--2----- a01-3----  12   const LineCheat *lc = (LineCheat*)((byte*)SIZE_LINE_CHEAT + size);
		//    move.l      (a4,d0.w), a4                       ;1 d--2----- a01-3----  18   
		EAddr temp;
		if( !ctx.AllocReg(temp, false) )
			Error("Gen_TextureFill: can't alloc temp reg to fetch fill routine");
		
		if( !ctx.AllocReg(fill_routine, true) )
			Error("Gen_TextureFill: can't allocate scaler fill routine register");

		as.Assemble("; -- Fill routine --");
		as << mov.w(reg_cur_size, temp) << "int lut_offset = (((word)s1)>>3) & ~3;";
		as << lsr.w(imm(3), temp);
		as << and.w(imm("$1FFC"), temp);

		if( ctx.reg_fn_scalers_base.IsReg() )
		{
			as << mov.l(mem_idx_w(ctx.reg_fn_scalers_base, temp), fill_routine);
		}
		else
		{
			as << lea.l(label("_FN_SCALERS"), fill_routine);
			as << mov.l(mem_idx_w(fill_routine, temp), fill_routine);
		}

		ctx.FreeReg(temp);
	}

	// Texcoord
	as.SetZone(ZONE_TEXCOORD);
	if( !ctx.texcoords_saved )
	{
		if( !ctx.reg_texcoord.IsReg() )
		{
			as.Assemble(uses_persp_correction ? "; -- Texcoord (persp) --" : "; -- Texcoord (no persp) --");
			//    move.l      d6, d0                              ;1 d--2----- a01-3----   4   word tx = u1s /divu/ s1;
			//    divu.w      d4, d0                              ;1 d--2----- a01-3---- 140   
			//    and.w       #$1F80, d0                          ;1 d--2----- a01-3----   8   tx &= $1F80;
			//    move.l      _asm_tex_base(pc), a2               ;1 d--2----- a01-3----  16   
			//    add.w       d0, a2                              ;1 d--2----- a01-3----   8   void *tex @ a2 = _asm_tex_base + tx;
			if( !ctx.AllocReg(ctx.reg_texcoord, false) )
				Error("Gen_TextureFill: can't alloc texcoord reg");

			const char *cmt = uses_persp_correction ? "word tx = u1s /divu/ s1;" : "word tx = u1s;";
			if( opt_texcoord_word )
				as << mov.w(reg_texcoord, ctx.reg_texcoord) << cmt;
			else
				as << mov.l(reg_texcoord, ctx.reg_texcoord) << cmt;

			if( uses_persp_correction )
				as << divu.w(reg_cur_size, ctx.reg_texcoord);
			as << and.w(imm("$1F80"), ctx.reg_texcoord) << "tx &= $1F80;";

			if( opt_keep_computed_texcoords && expect_next_texture )
			{
				as << mov.w(ctx.reg_texcoord, label("_asm_tex_base")) << "Save texcoords";		// TBD: save 4 cycles by using zeropage
				ctx.texcoords_saved = true;
			}
		}
		as << mov.l(pc_rel(ws.drawtype==DRAW_TEX_LOWER ? ".asm_tex_base2" : ".asm_tex_base"), reg_texture_source) << "-- Texture source --";
		as << add.w(ctx.reg_texcoord, reg_texture_source);

		ctx.FreeReg(ctx.reg_texcoord);
	}
	else
	{
		as << mov.l(pc_rel(ws.drawtype==DRAW_TEX_LOWER ? ".asm_tex_base2" : ".asm_tex_base"), reg_texture_source) << "-- Texture source --";
		as << add.w(label("_asm_tex_base"), reg_texture_source);
	}
	as.SetZone(ZONE_TEXDRAW);


	//                                                    ;1 d0-2----- a01-3----       === Upper											[drawing setup code]
	//    move.l      lc_upper_start_m128(a4), a0         ;1 d0-2----- a-1-3----  16   void *fn = line_cheat.upper_start_m64;
	//    move.w      lc_upper_len_m128_m64(a4), d0       ;1 d--2----- a-1-3----  12   $ fn[line_cheat.upper_len_m64_0] = $4ED6;  (L = -64..0 / -128..0)
	//    move.w      #$4ED6, (a0,d0.w)                   ;1 d--2----- a-1-3----  18   
	//    lea.l       .upper_ret(pc), a6                  ;1 d--2----- a-1-3----   8   
	//    jmp         (a0)                                ;1 d--2----- a-1-3----   8   $ call_a6(fn)	(Z = -64 / -128)
	//.upper_ret:                                         ;1 d--2----- a-1-3----       
	//    move.w      #$1EEA, (a0,d0.w)                   ;1 d--2----- a-1-3----  18   $ fn[line_cheat.upper_len_m64_0] = $1EEA;


	EAddr ystart;
	EAddr yend;
	if( wall_section_index > 0 )
		ystart = ctx.y_reg[sections[wall_section_index-1].yend_index];
	if( sections[wall_section_index].yend_index>=0 && sections[wall_section_index].yend_index<=max_yindex )
		yend = ctx.y_reg[sections[wall_section_index].yend_index];

	// Compute bottom_offset = 4*ybottom
	EAddr bottom_offset;
	EAddr ytop_source;
	bool moving_fill_pointer = opt_texfill_moving_pointer;
	bool free_bottom = false;
	if( clip_flags & FCLIP_BOTTOM_ALWAYS )
	{
		AssureYMax(ctx);
		bottom_offset = ctx.ymax_reg;
		free_bottom = true;	// TBD: is it correct?
		as << add.w(bottom_offset, bottom_offset) << "bottom_offset = 4*ymax";
		as << add.w(bottom_offset, bottom_offset);
		moving_fill_pointer = false;
	}
	else
	{
		if( !yend.IsValid() ) Error("yend not valid");
		if( !ctx.AllocReg(bottom_offset, false) ) Error("Can't alloc bottom offset register for texturing "+SectionDebugString(ws, clip_flags));
		free_bottom = true;
		as << mov.w(yend, bottom_offset) << "bottom_offset = 4*yend";
		as << add.w(bottom_offset, bottom_offset);
		as << add.w(bottom_offset, bottom_offset);
		ytop_source = yend;
	}

	// #$4ED6 -> (fill_routine, bottom_offset)
	if( moving_fill_pointer )
	{
		as << add.w(bottom_offset, fill_routine);
		as << mov.w(imm("$4ED6"), mem(fill_routine)) << "#$4ED6 -> (fill_routine, bottom_offset)";
		if( free_bottom ) ctx.FreeReg(bottom_offset);
	}
	else
		as << mov.w(imm("$4ED6"), mem_idx_w(fill_routine, bottom_offset)) << "#$4ED6 -> (fill_routine, bottom_offset)";


	// Compute top_offset = 4*ytop
	EAddr top_offset;
	bool free_top = false;
	if( clip_flags & FCLIP_TOP_ALWAYS )
	{
		AssureYMin(ctx);
		top_offset = ctx.ymin_reg;
		if( moving_fill_pointer )
			as << sub.w(ytop_source, top_offset);
		as << add.w(top_offset, top_offset) << "top_offset = 4*ymin";
		as << add.w(top_offset, top_offset);
	}
	else
	{
		if( !ystart.IsValid() ) Error("ystart not valid");
		if( !ctx.AllocReg(top_offset, false) )
		{
			if( opt_yminmax_common && ctx.ymax_reg.IsReg() )
			{
				if( ctx.ymax_selected )
				{
					as << swp.w(ctx.ymin_reg) << "Select ymin";
					ctx.ymax_selected = false;
				}
				top_offset = ctx.ymax_reg;
			}
			else
				Error("Can't alloc top offset register for texturing "+SectionDebugString(ws, clip_flags));
		}
		else
			free_top = true;

		as << mov.w(ystart, top_offset) << "top_offset = 4*ystart";
		if( moving_fill_pointer )
			as << sub.w(ytop_source, top_offset);
		as << add.w(top_offset, top_offset);
		as << add.w(top_offset, top_offset);
	}

	// execute at (fill_routine, top_offset)
	string return_label = format(".ret_%d", label_counter++);
	as << lea.l(pc_rel(return_label), a6);
	as << jmp(mem_idx_w(fill_routine, top_offset))  << ("-> "+return_label);
	as.Assemble(return_label+":");

	// #$1EEA -> (fill_routine, bottom_offset)
	if( moving_fill_pointer )
	{
		as << mov.w(imm("$1EEA"), mem(fill_routine)) << "#$1EEA -> (fill_routine, bottom_offset)";
	}
	else
	{
		if( clip_flags & FCLIP_BOTTOM_ALWAYS )
			AssureYMax(ctx);
		as << mov.w(imm("$1EEA"), mem_idx_w(fill_routine, bottom_offset)) << "#$1EEA -> (fill_routine, bottom_offset)";
	}


	// Free all temp registers used
	ctx.FreeReg(ctx.ymin_reg);					// ymin is always freed after first use
	if( clip_flags & FCLIP_BOTTOM_ALWAYS )
		ctx.FreeReg(ctx.ymax_reg);				// free ymax when used
	if( free_top ) ctx.FreeReg(top_offset);
	if( free_bottom ) ctx.FreeReg(bottom_offset);
	ctx.FreeReg(fill_routine);
}




string VariableCodeGen::Gen_DrawCore_Finalize(CodeGenContext &ctx)
{
	using namespace instructions;
	zone_t zone = as.current_zone;

	ctx.drawcore_finalized = true;

	if( !ctx.drawcore_had_sky && ctx.reg_sky_columns.IsValid() )
	{
		as.SetZone(ZONE_SKY);
		as << add.l(imm(4), ctx.reg_sky_columns) << "SKY was skipped";
	}

	if( !ctx.drawcore_had_gap )
	{
		as.SetZone(ZONE_GAP);

		EAddr tmp;
		if( ctx.AllocReg(tmp, false) )
		{
			as << mov.l(imm(0), tmp) << "Disable further rendering - clear ymin/ymax";
			as << mov.l(tmp, ctx.get_column_ymin());
			ctx.FreeReg(tmp);
		}
		else
			as << clr.l(ctx.get_column_ymin()) << "Disable further rendering - clear ymin/ymax";

		as << mov.w(reg_cur_size, ctx.get_column_size_limit());
	}

	if( opt_step_column_reg && ctx.render_column_incremented<8 )
	{
		as.SetZone(ZONE_INCREMENT);
		as << add.w(imm(8-ctx.render_column_incremented), reg_render_column);
		ctx.render_column_incremented = 8;
	}


	as.SetZone(zone);

	return ".end_loop";
}

void VariableCodeGen::Gen_DrawCore(CodeGenContext &ctx, const YRange &yr)
{
	using namespace instructions;

	as.SetZone(ZONE_GENERIC_LOOP);
	as.Assemble("; ======== "+SectionsDebugString(yr)+" ========");

	ctx.drawcore_had_gap = false;
	ctx.drawcore_had_sky = false;
	ctx.drawcore_finalized = false;

	for( int i=0; i<(int)sections.size(); i++ )
	{
		WallSection &ws = sections[i];
		ws._clip_flags = ComputeSectionClipFlags(i, yr);
	}

	for( int i=0; i<(int)sections.size(); i++ )
	{
		WallSection &ws = sections[i];
		if( !ws._clip_flags ) continue;

		bool last_draw = opt_direct_return_to_end_loop;
		for( int j=i+1; j<(int)sections.size(); j++ )
			if( ws._clip_flags )
			{
				last_draw = false;
				break;
			}

		if( ws.drawtype==DRAW_SKY )
		{
			as.SetZone(ZONE_SKY);
			ctx.drawcore_had_sky = true;
			Gen_ColorFill(ctx, i, ws._clip_flags, last_draw);
		}
		if( ws.drawtype==DRAW_SOLID_CEIL || ws.drawtype==DRAW_SOLID_FLOOR )
		{
			as.SetZone(ZONE_FLATS);
			Gen_ColorFill(ctx, i, ws._clip_flags, last_draw);
		}
		else if( ws.drawtype==DRAW_GAP )
		{
			as.SetZone(ZONE_GAP);
			ctx.drawcore_had_gap = true;
			Gen_GapSkip(ctx, i, ws._clip_flags);
		}
		else if( ws.drawtype==DRAW_TEX_UPPER || ws.drawtype==DRAW_TEX_LOWER )
		{
			as.SetZone(ZONE_TEXDRAW);
			bool expect_next_texture = false;
			for( int j=i+1; j<(int)sections.size(); j++ )
				if( sections[j].drawtype==DRAW_TEX_UPPER || sections[j].drawtype==DRAW_TEX_LOWER )
					if( sections[j]._clip_flags )
						expect_next_texture = true;

			Gen_TextureFill(ctx, i, ws._clip_flags, expect_next_texture);
		}
	}

	as.SetZone(ZONE_GENERIC_LOOP);
	if( !ctx.drawcore_finalized )
		as << bra(label(Gen_DrawCore_Finalize(ctx)));
}

void VariableCodeGen::Gen_GenerateIfLadderRec(CodeGenContext &ctx, const YRange &yr, const char *label_name)
{
	using namespace instructions;

	as.SetZone(ZONE_CASE_SELECT);
	as.Assemble(";");
	if( label_name )
		as.Assemble(string(label_name)+": ; -------- "+RangeDebugString(yr)+" --------");
	else
		as.Assemble("; -------- "+RangeDebugString(yr)+" --------");

	int cases = EstimateDifficulty(yr);
	if( cases <= 1 )
	{
		Gen_DrawCore(ctx, yr);
		ctx.UpdateStats(free_dregs, free_aregs);
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
