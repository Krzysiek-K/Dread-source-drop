
#include "common.h"

struct LineDefRenderDefinition {
	int			index;
	char		selector[8];
	int			method[12];
};

struct LineDefRenderInfo {
	string	print_name;
	string	selector;
	int		clip_min = 81;

	string	fn_persp;
	string	fn_nopersp;
};

#if 0
const LineDefRenderDefinition LINEDEF_METHODS[] = {
	{  0, "2s",		{ DRAW_SOLID_CEIL, 1,	DRAW_SOLID_FLOOR, 2,	DRAW_GAP, 3,	DRAW_SOLID_FLOOR, 4,	DRAW_SOLID_CEIL, -1 }},		// full 2-sided wall
	{  1, "2S",		{ DRAW_SKY, 1,			DRAW_SOLID_FLOOR, 2,	DRAW_GAP, 3,	DRAW_SOLID_FLOOR, 4,	DRAW_SOLID_CEIL, -1 }},		// full 2-sided wall					(sky)
//	{  2, "1s",		{ DRAW_SOLID_CEIL, 1,	DRAW_SOLID_FLOOR, 2,											DRAW_SOLID_CEIL, -1 }},		// 1-sided wall			y2==y3==y4
	{  2, "1s",		{ DRAW_SOLID_CEIL, 1,	DRAW_TEX_UPPER, 2,												DRAW_SOLID_CEIL, -1 }},		// 1-sided wall, textured		y2==y3==y4
	{  3, "1S",		{ DRAW_SKY, 1,			DRAW_SOLID_FLOOR, 2,											DRAW_SOLID_CEIL, -1 }},		// 1-sided wall			y2==y3==y4		(sky)
	{  4, "2cu",	{												DRAW_GAP, 3,	DRAW_SOLID_FLOOR, 4,	DRAW_SOLID_CEIL, -1 }},		// step only, ceiling not affected
	{  5, "2cul",	{												DRAW_GAP, 3,							DRAW_SOLID_CEIL, -1 }},		// lower ledge, ceiling not affected
//	{  6, "1s",		{ DRAW_SOLID_CEIL, 1,	DRAW_TEX_UPPER, 2,												DRAW_SOLID_CEIL, -1 }},		// 1-sided wall, textured		y2==y3==y4
};
#endif

const LineDefRenderDefinition LINEDEF_METHODS[] ={
	{  0, "2s",		{ DRAW_SOLID_CEIL, 1,	DRAW_TEX_UPPER, 2,		DRAW_GAP, 3,	DRAW_TEX_LOWER, 4,		DRAW_SOLID_FLOOR, -1 }},		// full 2-sided wall
	{  1, "2S",		{ DRAW_SKY, 1,			DRAW_TEX_UPPER, 2,		DRAW_GAP, 3,	DRAW_TEX_LOWER, 4,		DRAW_SOLID_FLOOR, -1 }},		// full 2-sided wall					(sky)
	{  2, "1sl",	{ DRAW_SOLID_CEIL, 1,	DRAW_TEX_UPPER, 2,												DRAW_SOLID_FLOOR, -1 }},		// 1-sided wall, textured		y2==y3==y4
	{  3, "1Sl",	{ DRAW_SKY, 1,			DRAW_TEX_UPPER, 2,												DRAW_SOLID_FLOOR, -1 }},		// 1-sided wall			y2==y3==y4		(sky)
	{  4, "2cu",	{												DRAW_GAP, 3,	DRAW_TEX_LOWER, 4,		DRAW_SOLID_FLOOR, -1 }},		// step only, ceiling not affected
	{  5, "2cul",	{												DRAW_GAP, 3,							DRAW_SOLID_FLOOR, -1 }},		// lower ledge, ceiling not affected
	{  6, "2su",	{ DRAW_SOLID_CEIL, 1,							DRAW_GAP, 3,	DRAW_TEX_LOWER, 4,		DRAW_SOLID_FLOOR, -1 }},		//
	{  7, "2sul",	{ DRAW_SOLID_CEIL, 1,							DRAW_GAP, 3,							DRAW_SOLID_FLOOR, -1 }},		//
	{  8, "1sL",	{ DRAW_SOLID_CEIL, 1,	DRAW_TEX_UPPER, 2,						DRAW_TEX_LOWER, 4,		DRAW_SOLID_FLOOR, -1 }},		// 1-sided wall, textured		y2==y3==y4
	{  9, "1SL",	{ DRAW_SKY, 1,			DRAW_TEX_UPPER, 2,						DRAW_TEX_LOWER, 4,		DRAW_SOLID_FLOOR, -1 }},		// 1-sided wall			y2==y3==y4		(sky)
	{ -1 }
};

// 1-sided:		sky/ceil  x  lower/nolower
// 2-sided:		sky/ceil



vector<LineDefRenderInfo> linedef_info;

//const int LINEDEF[] ={
//	DRAW_SOLID_CEIL, 1, DRAW_SOLID_FLOOR, 2, DRAW_GAP, 3, DRAW_SOLID_FLOOR, 4, DRAW_SOLID_CEIL, -1
//};



#if 1
	#define DREAD_SOURCE_PATH		"../dreadmake/gen"
	#define TOOL_SOURCE_PATH		"../dreadtool/gen"
#else
	#define DREAD_SOURCE_PATH		"out"
	#define TOOL_SOURCE_PATH		"out"
#endif


vector<LineDefRenderDefinition> gen_methods;


void Method_1sided(bool use_sky, bool use_lower, bool is_first)
{
	LineDefRenderDefinition def;
	char *sel = def.selector;
	int *gen = def.method;
	def.index = gen_methods.size();
	*sel++ = '1';

	if( use_sky )
	{
		*sel++ = 'S';
		*gen++ = DRAW_SKY;
		*gen++ = 1;
	}
	else
	{
		*sel++ = 's';
		*gen++ = DRAW_SOLID_CEIL;
		*gen++ = 1;
	}
	*gen++ = DRAW_TEX_UPPER;
	*gen++ = 2;

	if( use_lower )
	{
		*sel++ = 'L';
		*gen++ = DRAW_TEX_LOWER;
		*gen++ = 4;
	}
	else
	{
		*sel++ = 'l';
	}

	*gen++ = DRAW_SOLID_FLOOR;
	*gen++ = -1;

	*sel = 0;
	gen_methods.push_back(def);
}

void Method_2sided(bool use_sky, bool draw_ceil, bool draw_upper, bool draw_lower, bool draw_floor, bool is_first)
{
	LineDefRenderDefinition def;
	char *sel = def.selector;
	int *gen = def.method;
	def.index = gen_methods.size();
	*sel++ = '2';

	if( draw_ceil )
	{
		if( use_sky )
		{
			*sel++ = 'S';
			*gen++ = DRAW_SKY;
			*gen++ = 1;
		}
		else
		{
			*sel++ = 's';
			*gen++ = DRAW_SOLID_CEIL;
			*gen++ = 1;
		}
	}
	else
		*sel++ = 'c';

	if( draw_upper )
	{
		*gen++ = DRAW_TEX_UPPER;
		*gen++ = 2;
	}
	else
		*sel++ = 'u';

	*gen++ = DRAW_GAP;
	*gen++ = 3;

	if( draw_lower )
	{
		*gen++ = DRAW_TEX_LOWER;
		*gen++ = 4;
	}
	else
		*sel++ = 'l';

	if( draw_floor )
	{
		*gen++ = DRAW_SOLID_FLOOR;
		*gen++ = -1;
	}
	else
		*sel++ = 'f';

	*--gen = -1;
	*sel = 0;
	gen_methods.push_back(def);
}

void Method_2sided_gen(int index)
{
	bool is_first = ((index & 0x20) != 0);
	bool use_sky = ((index & 0x10) != 0);
	bool draw_ceil = ((index & 0x08) != 0);
	bool draw_upper = ((index & 0x04) != 0);
	bool draw_lower = ((index & 0x02) != 0);
	bool draw_floor = ((index & 0x01) != 0);

	if( !draw_ceil && use_sky ) return;
	if( draw_upper && !draw_ceil ) return;
	if( draw_lower && !draw_floor ) return;
	if( (index&0x0F)==0 ) return;

	Method_2sided(use_sky, draw_ceil, draw_upper, draw_lower, draw_floor, is_first);
}


LineDefRenderDefinition *GenerateAllMethods()
{
	// 1-sided
	Method_1sided(false, false, false);
	Method_1sided(false, true, false);
	Method_1sided(true, false, false);
	Method_1sided(true, true, false);

	// 2-sided
	for( int i=0; i<=0x1F; i++ )
		Method_2sided_gen(i);

	gen_methods.push_back(LineDefRenderDefinition());
	gen_methods.back().index = -1;

	return &gen_methods[0];
}


int main()
{
	//return main_old();

	vector<string> text_inc;
	vector<string> file_cheatnames;
	vector<string> file_selector;
	vector<string> stats_csv;
	
	try
	{
		const LineDefRenderDefinition *methods = LINEDEF_METHODS;
		methods = GenerateAllMethods();


		for( int i=0; methods[i].index>=0; i++ )
		{
			const LineDefRenderDefinition &method = methods[i];
			linedef_info.push_back(LineDefRenderInfo());
			LineDefRenderInfo &info = linedef_info.back();

			if( method.index!=i )
			{
				printf("ERROR: index out of sequence\n");
				break;
			}

			bool uses_texcoords = false;
			int cost = 100;
			for( int pass = 0; pass<2; pass++ )
			{
				string name = format("x%03d%s", method.index, pass ? "" : "p");
				string fn_name = "_Dread_LineCore_" + name;
				int charpos = 0;
				charpos += printf("$$$ %-5s", name.c_str());

				//// Estimate
				//{
				//	VariableCodeGen vc1;
				//	vc1.Initialize(method.method);
				//	vc1.uses_persp_correction = (pass==0);
				//	charpos += printf(" - %s ", vc1.GetPrintName().c_str());
				//	while( charpos<53 )
				//		charpos += printf(" ");
				//	if( !vc1.GenerateCode() ) throw 0;
				//
				//	free_dregs = vc1.free_dregs;
				//	free_aregs = vc1.free_aregs;
				//}

				VariableCodeGen vc;
				vc.Initialize(method.method, (pass==0));
				uses_texcoords = vc.uses_texcoords;
				cost = vc.EstimateMethodCost();

				charpos += printf(" - %s ", vc.GetPrintName().c_str());
				while( charpos<53 )
					charpos += printf(" ");

				for( int pass=0; pass<4; pass++ )
				{
					if( !vc.GenerateCode() ) throw 0;

					if( !vc.EnableOptimizations(vc.free_dregs, vc.free_aregs) )
						break;
				}

				vc.as.SaveFunction((DREAD_SOURCE_PATH "/render2_linedef_")+name+".s", fn_name);
				text_inc.push_back(format("\tinclude\t\"gen/render2_linedef_%s.s\"", name.c_str()));

				printf("\n");

				if( pass==0 )	info.fn_persp = fn_name;
				else			info.fn_nopersp = fn_name;

				info.print_name = vc.GetPrintName();

				// CSV stats
				if( stats_csv.size()<=0 )
					stats_csv.push_back(","+vc.as.StatCSVHeader());
				CycleStats cycles;
				vc.as.CountCycles(cycles);
				stats_csv.push_back(vc.as.StatCSVLine(name+","+vc.GetPrintName(), cycles));

				// End
				if( !uses_texcoords )
				{
					info.fn_nopersp = info.fn_persp;
					break;
				}
			}



			// Cheat selector
			const char *s = method.selector;
			info.selector = format("\tif( %2d<cost", cost);
			while( *s )
			{
				switch( *s++ )
				{
				case '0':	info.selector += " && false";				break;
				case '1':	info.selector += " && !is_two_sided";		break;
				case '2':	info.selector += " &&  is_two_sided";		break;
				case 's':	info.selector += " && !has_sky";			break;
				case 'S':	info.selector += " &&  has_sky";			break;
				case 'c':	info.selector += " && !draw_ceiling";		break;
				case 'C':	info.selector += " &&  draw_ceiling";		break;
				case 'u':	info.selector += " && !draw_upper";			break;
				case 'U':	info.selector += " &&  draw_upper";			break;
				case 'l':	info.selector += " && !draw_lower";			break;
				case 'L':	info.selector += " &&  draw_lower";			break;
				case 'f':	info.selector += " && !draw_floor";			break;
				case 'F':	info.selector += " &&  draw_floor";			break;
				case 'N':	info.selector += " && is_first";			break;
				}
			}
			info.selector += format(" ) mode=%3d, cost=%d;\t//\t%s", method.index, cost, info.print_name.c_str());


			// Clip table
			const float CLIP_CONST = 1.6f * (1<<ENGINE_SUBVERTEX_PRECISION);		// 4: 25.6
			const int CLIP_MIN = (((80*32)<<(8+ENGINE_SUBVERTEX_PRECISION)) / 65536) + 1;
			int clipdist = 0;	//(int)floor(CLIP_CONST * (-40 - CHEAT_TABLE[i].upper_y2));
			int clipdist2 = 0;	//(int)floor(CLIP_CONST * (0 - -40));
			if( clipdist > clipdist2 )
				clipdist = clipdist2;
			if( clipdist < CLIP_MIN )
				clipdist = CLIP_MIN;
			if( (clipdist&1) != (uses_texcoords ? 1 : 0) )
				clipdist++;
			info.clip_min = clipdist;

			//string clipentry = format("\t%3d,\t//%4d\t%s", clipdist, i, cheatname.c_str());
			//clip_table[CHEAT_TABLE[i].__name_index] = clipdist;


			// add to files
			file_cheatnames.push_back(format("\t\"%s\",", info.print_name.c_str()));
			file_selector.push_back(info.selector);
		}


		// Make table
		text_inc.push_back("");
		text_inc.push_back("\tpublic _RENDER2_LINEDEF_MODES");
		text_inc.push_back("_RENDER2_LINEDEF_MODES:");
		for( int i=0; i<linedef_info.size(); i++ )
		{
			LineDefRenderInfo &info = linedef_info[i];

			text_inc.push_back(format("\tdc.l\t%s", info.fn_persp.c_str()));
			text_inc.push_back(format("\tdc.l\t%s", info.fn_nopersp.c_str()));
			text_inc.push_back(format("\tdc.w\t%d\t; clip distance", info.clip_min));
			text_inc.push_back("\tdc.w\t0, 0, 0");
		}

		// Write files
		NFS.WriteFileLines(DREAD_SOURCE_PATH "/render2_linedefs.s", text_inc);
		NFS.WriteFileLines(TOOL_SOURCE_PATH "/render2_cheatnames.inc", file_cheatnames);
		NFS.WriteFileLines(TOOL_SOURCE_PATH "/render2_cheatselector.inc", file_selector);
		NFS.WriteFileLines("render2_stats.csv", stats_csv);
	}
	catch( int )
	{
	}
	//VariableCodeGen vc;
	//vc.Initialize(LINEDEF);
	//vc.GenerateCode();
	//vc.as.SaveFunction("../dreadmake/gen" "/line_experimental.s", "_Dread_LineCoreCheat_experimental");

	printf("\n\n\n");
	system("pause");
	return 0;
}
