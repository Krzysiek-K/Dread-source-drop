
#include "common.h"


//	 loop regs:		<input>							<runtime>							<fill/texture>
//	
//		d0											(-)
//		d1											(-)
//		d2			xmin							(-)								
//		d3			xmax-xmin-1						column counter														
//		d4			->								s1																	
//		d5											X									fill color						data read from line structure, must be precached to free A5
//		d6			->								u1s																	only for texcoords
//		d7			->								du																	only for texcoords
//	
//		a0											(-)	
//		a1											(-)
//		a2											(-)									sky/texture pointer
//		a3											(-)	/ _e_skyptr														fixed if there is sky present (no other simple way to read sky map than (a3)+)
//		a4											LineCheat*															fixed, used until the very end (lookups)
//		a5			EngineLine *line				RenderColumnInfo*													fixed, in more complex cases used until the very end (ymin/ymax)
//		a6											X									return addr						fixed, used in all fillers
//		a7											render_buffer (dynamic)				write pointer					fixed, used in all fillers

// Optimization types:
//		- cached value
//		- reused value
//

// Automatic optimizations:
//
//	$ Precache memory source:
//
//		instr.#	<ea>, ANY		->		move.#	<ea>, x#				(header)						for instrs:		move add sub cmp and or eo		TBD: lea
//										instr.#	x#, ANY					(correct place)					for <ea>:		label label(pc) #imm
//
//


static EAddr _line_cheat(const string &name)
{
	using namespace instructions;
	string var = format("lc_%s", name.c_str());
	return mem(var, a4);
}

static EAddr _line_cheat(const string &name, int y1)
{
	using namespace instructions;
	string var;
	if( y1>=0 )	var = format("lc_%s_%d", name.c_str(), y1);
	else		var = format("lc_%s_m%d", name.c_str(), -y1);
	return mem(var, a4);
}

static EAddr _line_cheat(const string &name, int y1, int y2)
{
	using namespace instructions;
	string var;
	if( y1>=0 )	var = format("lc_%s_%d", name.c_str(), y1);
	else		var = format("lc_%s_m%d", name.c_str(), -y1);
	if( y2>=0 )	var = format("%s_%d", var.c_str(), y2);
	else		var = format("%s_m%d", var.c_str(), -y2);
	return mem(var, a4);
}

static string _vlabel(const string &name, int variant)
{
	if( variant ) return format("%s%d", name.c_str(), variant);
	return name;
}


static string _section(const char *name, genmode_t mode)
{
	const char *y1 = "?";
	const char *y2 = "?";
	switch(mode & GEN_Y1_MASK)
	{
	case 0:							y1 = "";				break;
	case GEN_Y1:					y1 = "[";				break;
	case GEN_Y1_CULL:				y1 = "[[";				break;
	case GEN_Y1_MUST_CLIP:			y1 = "!";				break;
	case GEN_Y1_MUST_CLIP_CULL:		y1 = "![";				break;
	}
	switch( mode & GEN_Y2_MASK )
	{
	case 0:							y2 = "";				break;
	case GEN_Y2:					y2 = "]";				break;
	case GEN_Y2_CULL:				y2 = "]]";				break;
	case GEN_Y2_MUST_CLIP:			y2 = "!";				break;
	case GEN_Y2_MUST_CLIP_CULL:		y2 = "]!";				break;
	}
	return format(";=== %s%s%s", y1, name, y2);
}



void CodeGen::GenCeil(const char *cull_label)
{
	using namespace instructions;

	if( !ceil_mode ) return;

	as.SetZone(ZONE_FLATS);
	string lb = _vlabel(".ceil_ret", variant);
	string lbcull = _vlabel(".ceil_culled", variant);

	as.Assemble(_section("Ceil", ceil_mode));
	if( ceil_mode )
		as << swp.w(d5);

	as << lea.l(label("_FUNC_COLORFILL+ENGINE_Y_MAX*2"), xr_ceil_fill_addr) << "Color filler func";
	if( ceil_mode & GEN_Y1 )
	{
		as << mov.w(column_ymin, xr_ceil_temp) << "jsr(fn1, d0) LUT.w:	Ceil length(x2)";
		as << add.w(xr_ceil_temp, a7) << "skip ymin";	// TBD: invalid/not required if ceil is culled	(check same in Sky)
		as << add.w(_line_cheat("ceil_len", upper_y1), xr_ceil_temp) << "shorten to end at texture edge";
		if( ISGEN_Y1_CULL(ceil_mode) )
		{
			as.SetZone(ZONE_CASE_SELECT);
			if( cull_label )
				as << bge(label(cull_label));	// jump to alternate path
			else
			{
				as << bge(label(lbcull.c_str()));	// jump to alternate path if required
			}
			as.SetZone(ZONE_FLATS);
		}
	}
	else
	{
		// unclipped
		as << mov.w(_line_cheat("ceil_len", upper_y1), xr_ceil_temp) << "jsr(fn1, d0) LUT.w:	Ceil length(x2)";
	}
	as << lea.l(pc_rel(".ceil_ret"), a6);
	as << jmp(mem_idx_w(0, xr_ceil_fill_addr, xr_ceil_temp));

	if( ISGEN_Y1_CULL(ceil_mode) && !cull_label )
	{
		as.Assemble(lbcull+":");
		as << mov.l(mem(a5), a7) << "byte *dst = render_buffer + offs[x];";
		as << add.w(_line_cheat("hole_end", lower_y1), a7);
		as << bra(label(_vlabel(".floor_start", variant)));
	}
	as.Assemble(lb+":");
}

void CodeGen::GenSky(const char *cull_label)
{
	using namespace instructions;

	if( !sky_mode ) return;

	string lb = _vlabel(".sky_ret", variant);

	as.SetZone(ZONE_SKY);
	as.Assemble(_section("Sky", sky_mode));

	as << lea.l(label("_FUNC_SKYCOPY+ENGINE_Y_MAX*2"), xr_sky_fill_addr) << "Fill func";
	as << mov.l(mem_postinc(a3), a2);
	as << mov.w(_line_cheat("ceil_len", upper_y1), xr_sky_temp) << "jsr(fn1, d0) LUT.w:	Ceil length(x2)";
	if( sky_mode & GEN_Y1 )
	{
		as << mov.w(column_ymin, xr_sky_temp2) << "ymin";
		as << add.w(xr_sky_temp2, a7) << "skip to ymin";
		as << add.w(xr_sky_temp2, xr_sky_temp) << "shorten by ymin";
		if( ISGEN_Y1_CULL(sky_mode) )
		{
			as.SetZone(ZONE_CASE_SELECT);
			as << bge(label(cull_label ? cull_label : lb.c_str()));		// jump to alternate path if required
			as.SetZone(ZONE_SKY);
		}
		as << lsr.w(imm(1), xr_sky_temp2);
		as << add.w(xr_sky_temp2, a2) << "skip sky texture to ymin";
	}
	as << lea.l(pc_rel(lb), a6);
	as << jmp(mem_idx_w(0, xr_sky_fill_addr, xr_sky_temp));
	as.Assemble(lb+":");
}

void CodeGen::GenTexcoord()
{
	using namespace instructions;

	if( !upper_mode && !lower_mode ) return;

	as.SetZone(ZONE_TEXCOORD);
	as.Assemble(_section("Texcoord", GEN_NOCLIP));

	if( tex_mode==TEXM_PERSPECTIVE )
	{
		as << mov.l(d6, xr_texcoord_temp) << "word tx = u1s /divu/ s1;";
		as << divu.w(d4, xr_texcoord_temp);
		as << and.w(immhex(0x1F80), xr_texcoord_temp) << "tx &= $1F80;";
		as << mov.l(pc_rel("_asm_tex_base"), a2);
		as << add.w(xr_texcoord_temp, a2) << "void *tex @ a2 = _asm_tex_base + tx;";
	}
	else
	{
		as << mov.l(d6, xr_texcoord_temp) << "word tx = u1s & $1F80;";
		as << and.w(immhex(0x1F80), xr_texcoord_temp) << "tx &= $1F80;";
		as << mov.l(pc_rel("_asm_tex_base"), a2);
		as << add.w(xr_texcoord_temp, a2) << "void *tex @ a2 = _asm_tex_base + tx;";
	}
}

void CodeGen::GenUpper(const char *cull_label)
{
	using namespace instructions;

	if( !upper_mode ) return;

	as.SetZone(ZONE_TEXDRAW);
	string lb = _vlabel(".upper_ret", variant);
	string lb2 = _vlabel(".upper_skip", variant);
	
	as.Assemble(_section("Upper", upper_mode));

	if( ISGEN_Y1_MUST_CLIP(upper_mode) )
	{
		// xr_upper_temp2 = (ymin-ENGINE_Y_MID*2)*2
		as << mov.w(column_ymin, xr_upper_temp2) << "ymin";
		as << add.w(imm("-ENGINE_Y_MID*2"), xr_upper_temp2);
		as << add.w(xr_upper_temp2, xr_upper_temp2);

		// xr_upper_temp = upper_endpos
		as << mov.w(_line_cheat("upper_endpos", upper_y2), xr_upper_temp) << "$ fn[line_cheat.upper_len_m64_0] = $4ED6;  (L = -64..0 / -128..0)";
		if( ISGEN_Y1_CULL(upper_mode) )
		{
			// Make sure xr_upper_temp2 < xr_upper_temp
			as.SetZone(ZONE_CASE_SELECT);
			as << cmp.w(xr_upper_temp, xr_upper_temp2);
			as << bge(label(cull_label ? cull_label : lb2.c_str()));		// jump to alternate path if required
			as.SetZone(ZONE_TEXDRAW);
		}
		as << mov.l(_line_cheat("scaler_fn"), xr_upper_fn2) << "void *fn = line_cheat.upper_start_m64;";
		as << mov.w(immhex(0x4ED6), mem_idx_w(xr_upper_fn2, xr_upper_temp));
		as << lea.l(pc_rel(lb), a6);
		as << jmp(mem_idx_w(xr_upper_fn2, xr_upper_temp2)) << "$ call_a6(fn)	(Z = -64 / -128)";
		as.Assemble(lb+":");
		as << mov.w(immhex(0x1EEA), mem_idx_w(xr_upper_fn2, xr_upper_temp)) << "$ fn[line_cheat.upper_len_m64_0] = $1EEA;";
		if( ISGEN_Y1_CULL(upper_mode) )
			as.Assemble(lb2+":");
	}
	else
	{
		as << mov.l(_line_cheat("upper_start", upper_y1), xr_upper_fn) << "void *fn = line_cheat.upper_start_m64;";
		as << mov.w(_line_cheat("upper_len", upper_y1, upper_y2), xr_upper_temp) << "$ fn[line_cheat.upper_len_m64_0] = $4ED6;  (L = -64..0 / -128..0)";
		as << mov.w(immhex(0x4ED6), mem_idx_w(xr_upper_fn, xr_upper_temp));
		as << lea.l(pc_rel(lb), a6);
		as << jmp(mem(xr_upper_fn)) << "$ call_a6(fn)	(Z = -64 / -128)";
		as.Assemble(lb+":");
		as << mov.w(immhex(0x1EEA), mem_idx_w(xr_upper_fn, xr_upper_temp)) << "$ fn[line_cheat.upper_len_m64_0] = $1EEA;";
	}
}

void CodeGen::GenGap()
{
	using namespace instructions;

	if( !gap_mode ) return;


	as.SetZone(ZONE_GAP);
	string lb = _vlabel(".gap_skip", variant);

	as.Assemble(_section("Gap", gap_mode));
	
	if( ISGEN_Y1_MUST_CLIP(gap_mode) )
	{
		as << mov.l(mem(a5), a7) << "byte *dst = render_buffer + offs[x];";
		as << add.w(_line_cheat("hole_end", lower_y1), a7);
	}
	else if( gap_mode & GEN_Y1 )
	{
		// TBD: A7 invalid in second case (branch taken)
		as << mov.w(_line_cheat("ceil_ymin", upper_y2), xr_gap_temp) << "ymin";
		as.SetZone(ZONE_CASE_SELECT);
		as << cmp.w(column_ymin, xr_gap_temp);
		as << ble(label(lb));
		as.SetZone(ZONE_GAP);
		as << mov.w(xr_gap_temp, column_ymin);
		as << add.w(_line_cheat("hole_size", upper_y2, lower_y1), a7);
		as.Assemble(lb+":");
	}
	else
	{
		as << mov.w(_line_cheat("ceil_ymin", upper_y2), column_ymin) << "ymin";
		as << add.w(_line_cheat("hole_size", upper_y2, lower_y1), a7);
	}
}

void CodeGen::GenFloor()
{
	using namespace instructions;

	if( !floor_mode ) return;

	as.SetZone(ZONE_FLATS);
	string lb = _vlabel(".floor_ret", variant);

	string fill_offs = "-ENGINE_Y_MID*2";
	as.Assemble(_section("Floor", floor_mode));
	as.Assemble(_vlabel(".floor_start", variant)+":");
	if( ceil_mode )
		as << swp.w(d5);
	as << lea.l(label("_FUNC_COLORFILL+ENGINE_Y_MID*2"), xr_floor_fill_addr) << "Color filler func";
	as << mov.w(_line_cheat("floor_start", lower_y2), xr_floor_temp) << "jsr (fn1,d0)		LUT.w:	Fill func start		(x2)	(fills to screen end)";
	if( gap_mode )
		as << mov.w(xr_floor_temp, column_ymax) << "ymax = Gap end";
	if( floor_mode & GEN_Y2 )
	{
		as << sub.w(xr_ymax, xr_floor_temp) << "clip ymax";
		fill_offs = "(ENGINE_Y_MAX-ENGINE_Y_MID)*2";
	}
	as << lea.l(pc_rel(lb), a6);
	as << jmp(mem_idx_w(fill_offs, xr_floor_fill_addr, xr_floor_temp));
	as.Assemble(lb+":");
}


void CodeGen::StartVariant(const string &lb)
{
	using namespace instructions;
	
	as.SetZone(ZONE_GENERIC_LOOP);
	as << bra(label(".end")) << "End of variant";

	as.Assemble(lb+":");
	variant = variant_counter++;
	as << format("Variant %d", variant);
}

void CodeGen::GenDrawingCode()
{
	using namespace instructions;

	CheatState _cs = *(CheatState*)this;

	as.SetZone(ZONE_GENERIC_LOOP);

	switch(stage)
	{
	case STAGE_CEIL_SKY:
		stage = STAGE_TEXCOORD;			// next
		if( ceil_mode || sky_mode )
		{
			if( upper_mode & GEN_Y1 )		// texture can be clipped - generate variant
			{
				string lb = _vlabel(".skyceil_culled", variant);
				if( ceil_mode ) GenCeil(lb.c_str());
				if( sky_mode ) GenSky(lb.c_str());
				genmode_t mode = upper_mode;
				upper_mode = genmode_t(mode & ~GEN_Y1_MASK);				// switch to new variant
				if( !upper_mode ) upper_mode = GEN_NOCLIP;
				GenDrawingCode();											// finish current variant during recursion
				upper_mode = genmode_t(mode | GEN_Y1_MUST_CLIP);			// switch to new variant
				StartVariant(lb);
			}
			else if( !upper_mode && (gap_mode & GEN_Y1) )	// gap can be clipped - generate variant
			{
				string lb = _vlabel(".skyceil_culled", variant);
				if( ceil_mode ) GenCeil(lb.c_str());
				if( sky_mode ) GenSky(lb.c_str());
				genmode_t mode = gap_mode;
				gap_mode = genmode_t(mode & ~GEN_Y1_MASK);				// switch to new variant
				if( !gap_mode ) gap_mode = GEN_NOCLIP;
				GenDrawingCode();										// finish current variant during recursion
				gap_mode = genmode_t(mode | GEN_Y1_MUST_CLIP);			// switch to new variant
				StartVariant(lb);
			}
			else
			{
				// no variants
				if( ceil_mode ) GenCeil();
				if( sky_mode ) GenSky();
			}
		}

	case STAGE_TEXCOORD:
		stage = STAGE_UPPER;			// next
		if( upper_mode || lower_mode ) GenTexcoord();

	case STAGE_UPPER:
		stage = STAGE_GAP_SKIP;			// next
		if( upper_mode )
		{
			//GenUpper();	// TBD: variants for the Gap is Upper is completely culled	(bad floor rendering in zone 5)

			if( gap_mode & GEN_Y1 )		// gap can be clipped - generate variant
			{
				string lb = _vlabel(".upper_culled", variant);
				GenUpper(lb.c_str());
				genmode_t mode = gap_mode;
				gap_mode = genmode_t(mode & ~GEN_Y1_MASK);				// switch to new variant
				if( !gap_mode ) gap_mode = GEN_NOCLIP;
				GenDrawingCode();										// finish current variant during recursion
				gap_mode = genmode_t(mode | GEN_Y1_MUST_CLIP);			// switch to new variant
				StartVariant(lb);
			}
			else
			{
				// no variants
				GenUpper();
			}
		}

	case STAGE_GAP_SKIP:
		stage = STAGE_FLOOR;			// next
		if( gap_mode ) GenGap();

	case STAGE_FLOOR:
		stage = STAGE_GAP_LIMITS;		// next
		if( floor_mode ) GenFloor();

	case STAGE_GAP_LIMITS:
		stage = STAGE_DONE;				// next
		if( gap_mode )
		{
			//as << mov.w(_line_cheat("floor_start", lower_y2), column_ymax) << "ymax = Gap end";		moved to floor
		}
		else
		{
			as.SetZone(ZONE_GENERIC_LOOP);
			as << mov.w(imm(0), column_ymax) << "ymax = 0		(block further visibility)";
		}

	case STAGE_DONE:
		break;
	}

	as.SetZone(ZONE_GENERIC_LOOP);
	if( !variant && !sky_mode )
		as.Assemble(".hidden_skip:");
	as << dbra.w(d3, label(".loop")) << "LOOP END";

	*(CheatState*)this = _cs;
}

void CodeGen::Generate()
{
	using namespace instructions;

	as.Clear();
	as.SetZone(ZONE_SETUP);
	variant_counter = 0;
	variant = variant_counter++;
	stage = STAGE_CEIL_SKY;


	if( SAVE_REGS )
		as.Assemble(" movem.l d2-d7/a2-a6, -(a7)");
	as.Assemble(" move.l a7, _a7_save");

	as.Assemble("; Loop initialization");

	// Global - ceil / floor
	if( ceil_mode || floor_mode )
	{
		if( ceil_mode )					as << mov.b(mem("line_ceil_col", a5), d5) << "Prepare ceil";
		if( ceil_mode && floor_mode )	as << swp.w(d5);
		if( floor_mode )				as << mov.b(mem("line_floor_col", a5), d5) << "Prepare floor";

	}

	// Column pointer
	as << lea.l(label("_render_columns"), a5) << "Setup column pointer";
	as << add.w(d2, a5);


	// Global - sky
	if( sky_mode )
	{
		as << lsr.w(imm(1), d2) << "SKY";
		as << mov.l(label("_e_skyptr"), a3);
		as << add.w(d2, a3);
	}

	as << bra(label(".start")) << "End of header";


	// Next iteration start
	as.SetZone(ZONE_INCREMENT);

	as.Assemble(".loop:");
	as << add.w(pc_rel("_asm_line_ds"), d4) << "s1 += ds;";
	if( upper_mode || lower_mode )
		as << add.l(d7, d6) << "u1s += du;";			// texcoord interpolation
	as << add.w(imm(8), a5);							// next column
	// if( ceil_mode && floor_mode ) as << swp.w(d5);


	// First iteration start
	as.SetZone(ZONE_GENERIC_LOOP);
	as.Assemble(".start: ; // FIRST ITERATION STARTS HERE");
	// - check if column has ended / preload ymax
	//if( (floor_mode & GEN_Y2) )		//TBD: use only when required
	{
		as.SetZone(ZONE_CASE_SELECT);
		as << mov.w(column_ymax, xr_ymax);
		as << beq(label(".hidden_skip"));
		as.SetZone(ZONE_GENERIC_LOOP);
	}

	//	- render buffer
	as << mov.l(mem(a5), a7) << "byte *dst = render_buffer + offs[x];";
	//	- size/line cheat
	if( 1 )		// TBD: size is not required in some cases		(only sky or single color fills entire visible area)
	{
		as << mov.w(d4, xr_size_temp)  << "int size = (((word)s1)>>3) & ~3;";
		if( !gap_mode )
			as << mov.w(xr_size_temp, column_size_limit) << "end of column - clip sprites";
		as << lsr.w(imm(3), xr_size_temp);
		as << and.w(immhex(0xFFFC), xr_size_temp);
		as << lea.l(label("_SIZE_LINE_CHEAT"), a4) << "const LineCheat *lc = (LineCheat*)((byte*)SIZE_LINE_CHEAT + size);";
		as << mov.l(mem_idx_w(a4, xr_size_temp), a4);
	}


	GenDrawingCode();



	as.SetZone(ZONE_GENERIC_LOOP);
	if( sky_mode )
	{
		as << bra(label(".end")) << "End of variant";
		as.Assemble(".hidden_skip:");
		as.SetZone(ZONE_SKY);
		as << add.l(imm(4), a3);
		as.SetZone(ZONE_GENERIC_LOOP);
		as << dbra.w(d3, label(".loop")) << "LOOP END";
	}

	as.SetZone(ZONE_OUTRO);

	as.Assemble(".end: ; Cleanup & exit");
	as.Assemble(" move.l _a7_save(pc), a7");
	if( SAVE_REGS )
		as.Assemble(" movem.l (a7)+, d2-d7/a2-a6");
	as.Assemble(" rts");

	// Allocate regs
	int free_regs = 0;
	free_regs |= 1<<0;	// d0
	free_regs |= 1<<1;	// d1
	free_regs |= 1<<2;	// d2
	if( !ceil_mode && !floor_mode ) free_regs |= 1<<5;	// d5
	free_regs |= 0x100<<0;	// a0
	free_regs |= 0x100<<1;	// a1
	if( !upper_mode && !lower_mode ) free_regs |= 0x100<<2;	// a2
	if( !sky_mode ) free_regs |= 0x100<<3;	// a3

	as.AllocateRegs(free_regs);
	as.ConvertOpcodes();

	CycleStats stats;
	as.CountCycles(stats);
	printf("                - timing  %4d/%4d\n", stats.GetLoopMin(), stats.GetLoopMax());
}
