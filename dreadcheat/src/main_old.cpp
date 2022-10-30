
#include "common.h"


// Required line cheats:
//		 0				Ceil	 -64	Upper	  0			Floor				Ceil  64   0 		front
//		 1				[Ceil	 -64	Upper	  0			Floor]				Ceil  64   0 		CLIPPED
//		 2				Sky		 -64	Upper	  0			Floor				Sky   64   0 		front
//		 3				[Sky	 -64	Upper	  0			Floor]				Sky   64   0 		CLIPPED
//		 4				Ceil	-128	Upper	  0			Floor				Ceil 128   0 		front
//		 5				[[Ceil	-128	[Upper	  0			Floor]				Ceil 128   0 		CLIPPED
//		 6				Sky		-128	Upper	  0			Floor				Sky	 128   0 		front
//		 7				[[Sky	-128	[Upper	  0			Floor]				Sky  128   0 		CLIPPED
//		 8				Ceil	-128	Upper	-64 % 0		Floor				Ceil 128  64 Gap	front
//		 9				[[Ceil	-128	[Upper	-64 % 0		Floor]				Ceil 128  64 Gap	CLIPPED
//		10				Sky		-128	Upper	-64 % 0		Floor				Sky  128  64 Gap	front
//		11				[[Sky	-128	[Upper	-64 % 0		Floor]				Sky  128  64 Gap	CLIPPED
//		12				Ceil	 -64	%		  0			Floor				Ceil  64  64 Gap	front
//		13				[Ceil	 -64	%		  0			Floor]				Ceil  64  64 Gap	CLIPPED
//		14				Ceil	 -96	Upper	  0			Floor				Ceil  96   0		front			// 96px one sided walls
//		15				[[Ceil	 -96	[Upper	  0			Floor				Ceil  96   0		CLIPPED
//		16				Sky		 -96	Upper	  0			Floor				Sky   96   0		front
//		17				[[Sky	 -96	[Upper	  0			Floor				Sky   96   0		CLIPPED

//						Ceil	 -96	Upper	-64 % 0		Floor				Ceil  96  64 Gap	front			// 96px walls with 64px openings
//						[[Ceil	 -96	[Upper	-64 % 0		Floor]				Ceil  96  64 Gap	CLIPPED
//						Sky		 -96	Upper	-64 % 0		Floor				Sky   96  64 Gap	front
//						[[Sky	 -96	[Upper	-64 % 0		Floor]				Sky   96  64 Gap	CLIPPED
//						Ceil	-128	Upper	-96 % 0		Floor				Ceil 128  96 Gap	front			// 128px walls with 96px gaps
//						[[Ceil	-128	[Upper	-96 % 0		Floor]				Ceil 128  96 Gap	CLIPPED
//						Sky		-128	Upper	-96 % 0		Floor				Sky  128  96 Gap	front
//						[[Sky	-128	[Upper	-96 % 0		Floor]				Sky  128  96 Gap	CLIPPED
//						Ceil	 -96	%		  0			Floor				Ceil  96  96 Gap	front			// 96px tunnel exit / light transition
//						[Ceil	 -96	%		  0			Floor]				Ceil  96  96 Gap	CLIPPED
//						Ceil	-128	%		  0			Floor				Ceil 128 128 Gap	front			// 128px light transition
//						[Ceil	-128	%		  0			Floor]				Ceil 128 128 Gap	CLIPPED


//						[[Ceil	 -96	%		  0		Floor]				culled version of #27
//						[[Ceil	-128	[[Upper	-96	% 0	Floor]				more clipped version of #23

//						%		 -64	Lower	  0			Floor										// low wall, with gap above
//						Sky]	-128	Upper]]															// high wall, side of the construction, seen only above a low wall
//						Sky]																			// sky shoving above low wall
//	


//#define TEX_MODE		TEXM_LINEAR
#define TEX_MODE		TEXM_PERSPECTIVE
CheatInfo CHEAT_TABLE[] ={
	//		sky_mode	ceil_mode	upper_y1	upper_mode	upper_y2	gap_mode	lower_y1	lower_mode	lower_y2	floor_mode	
	{ 0,	GEN_OFF,	GEN_NOCLIP,	  -64,		GEN_NOCLIP,     0,		GEN_OFF,	    0,		GEN_OFF,	    0,		GEN_NOCLIP,		TEX_MODE },		//	  Ceil	 -64	 Upper	  0		Floor
	{ 1,	GEN_OFF,	GEN_Y1,		  -64,		GEN_NOCLIP,     0,		GEN_OFF,	    0,		GEN_OFF,	    0,		GEN_Y2,			TEX_MODE },		//	 [Ceil	 -64	 Upper	  0		Floor]
	{ 2,	GEN_NOCLIP,	GEN_OFF,	  -64,		GEN_NOCLIP,     0,		GEN_OFF,	    0,		GEN_OFF,	    0,		GEN_NOCLIP,		TEX_MODE },		//	  Sky	 -64	 Upper	  0		Floor
	{ 3,	GEN_Y1,		GEN_OFF,	  -64,		GEN_NOCLIP,	    0,		GEN_OFF,	    0,		GEN_OFF,	    0,		GEN_Y2,			TEX_MODE },		//	 [Sky	 -64	 Upper	  0		Floor]
	{ 4,	GEN_OFF,	GEN_NOCLIP,	 -128,		GEN_NOCLIP,     0,		GEN_OFF,	    0,		GEN_OFF,	    0,		GEN_NOCLIP,		TEX_MODE },		//	  Ceil	-128	 Upper	  0		Floor
	{ 5,	GEN_OFF,	GEN_Y1_CULL, -128,		GEN_Y1,		    0,		GEN_OFF,	    0,		GEN_OFF,	    0,		GEN_Y2,			TEX_MODE },		//	[[Ceil	-128	[Upper	  0		Floor]
	{ 6,	GEN_NOCLIP,	GEN_OFF,	 -128,		GEN_NOCLIP,     0,		GEN_OFF,	    0,		GEN_OFF,	    0,		GEN_NOCLIP,		TEX_MODE },		//	  Sky	-128	 Upper	  0		Floor
	{ 7,	GEN_Y1_CULL,GEN_OFF,	 -128,		GEN_Y1,		    0,		GEN_OFF,	    0,		GEN_OFF,	    0,		GEN_Y2,			TEX_MODE },		//	[[Sky	-128	[Upper	  0		Floor]
	{ 8,	GEN_OFF,	GEN_NOCLIP,	 -128,		GEN_NOCLIP,   -64,		GEN_NOCLIP,	    0,		GEN_OFF,	    0,		GEN_NOCLIP,		TEX_MODE },		//	  Ceil	-128	 Upper	-64	% 0	Floor
	{ 9,	GEN_OFF,	GEN_Y1_CULL, -128,		GEN_Y1,		  -64,		GEN_NOCLIP,	    0,		GEN_OFF,	    0,		GEN_Y2,			TEX_MODE },		//	[[Ceil	-128	[Upper	-64	% 0	Floor]
	{ 10,	GEN_NOCLIP,	GEN_OFF,	 -128,		GEN_NOCLIP,   -64,		GEN_NOCLIP,	    0,		GEN_OFF,	    0,		GEN_NOCLIP,		TEX_MODE },		//	  Sky	-128	 Upper	-64	% 0	Floor
	{ 11,	GEN_Y1_CULL,GEN_OFF,	 -128,		GEN_Y1,		  -64,		GEN_NOCLIP,	    0,		GEN_OFF,	    0,		GEN_Y2,			TEX_MODE },		//	[[Sky	-128	[Upper	-64	% 0	Floor]
	{ 12,	GEN_OFF,	GEN_NOCLIP,	  -64,		GEN_OFF,	  -64,		GEN_NOCLIP,	    0,		GEN_OFF,	    0,		GEN_NOCLIP,		TEX_MODE },		//	  Ceil	 -64	%		  0		Floor
	{ 13,	GEN_OFF,	GEN_Y1,		  -64,		GEN_OFF,	  -64,		GEN_NOCLIP,	    0,		GEN_OFF,	    0,		GEN_Y2,			TEX_MODE },		//	 [Ceil	 -64	%		  0		Floor]

	{ 14,	GEN_OFF,	GEN_NOCLIP,	  -96,		GEN_NOCLIP,     0,		GEN_OFF,	    0,		GEN_OFF,	    0,		GEN_NOCLIP,		TEX_MODE },		//	  Ceil	 -96	 Upper	  0		Floor
	{ 15,	GEN_OFF,	GEN_Y1_CULL,  -96,		GEN_Y1,		    0,		GEN_OFF,	    0,		GEN_OFF,	    0,		GEN_Y2,			TEX_MODE },		//	[[Ceil	 -96	[Upper	  0		Floor]
	{ 16,	GEN_NOCLIP,	GEN_OFF,	  -96,		GEN_NOCLIP,     0,		GEN_OFF,	    0,		GEN_OFF,	    0,		GEN_NOCLIP,		TEX_MODE },		//	  Sky	 -96	 Upper	  0		Floor
	{ 17,	GEN_Y1_CULL,GEN_OFF,	  -96,		GEN_Y1,		    0,		GEN_OFF,	    0,		GEN_OFF,	    0,		GEN_Y2,			TEX_MODE },		//	[[Sky	 -96	[Upper	  0		Floor]

	{ 18,	GEN_OFF,	GEN_NOCLIP,	  -96,		GEN_NOCLIP,   -64,		GEN_NOCLIP,	    0,		GEN_OFF,	    0,		GEN_NOCLIP,		TEX_MODE },		//	  Ceil	 -96	 Upper	-64	% 0	Floor
	{ 19,	GEN_OFF,	GEN_Y1_CULL,  -96,		GEN_Y1_CULL,  -64,		GEN_NOCLIP,		0,		GEN_OFF,	    0,		GEN_Y2,			TEX_MODE },		//	[[Ceil	 -96	[[Upper	-64	[% 0 Floor]
	{ 20,	GEN_NOCLIP,	GEN_OFF,	  -96,		GEN_NOCLIP,   -64,		GEN_NOCLIP,	    0,		GEN_OFF,	    0,		GEN_NOCLIP,		TEX_MODE },		//	  Sky	 -96	 Upper	-64	% 0	Floor
	{ 21,	GEN_Y1_CULL,GEN_OFF,	  -96,		GEN_Y1_CULL,  -64,		GEN_NOCLIP,		0,		GEN_OFF,	    0,		GEN_Y2,			TEX_MODE },		//	[[Sky	 -96	[[Upper	-64	[% 0 Floor]

	{ 22,	GEN_OFF,	GEN_NOCLIP,	 -128,		GEN_NOCLIP,   -96,		GEN_NOCLIP,	    0,		GEN_OFF,	    0,		GEN_NOCLIP,		TEX_MODE },		//	  Ceil	-128	 Upper	-96	% 0	Floor
	{ 23,	GEN_OFF,	GEN_Y1_CULL, -128,		GEN_Y1_CULL,  -96,		GEN_Y1,		    0,		GEN_OFF,	    0,		GEN_Y2,			TEX_MODE },		//	[[Ceil	-128	[[Upper	-96	[% 0 Floor]
	{ 24,	GEN_NOCLIP,	GEN_OFF,	 -128,		GEN_NOCLIP,   -96,		GEN_NOCLIP,	    0,		GEN_OFF,	    0,		GEN_NOCLIP,		TEX_MODE },		//	  Sky	-128	 Upper	-96	% 0	Floor
	{ 25,	GEN_Y1_CULL,GEN_OFF,	 -128,		GEN_Y1_CULL,  -96,		GEN_Y1,		    0,		GEN_OFF,	    0,		GEN_Y2,			TEX_MODE },		//	[[Sky	-128	[[Upper	-96	[% 0 Floor]

	{ 26,	GEN_OFF,	GEN_NOCLIP,	  -96,		GEN_OFF,	  -96,		GEN_NOCLIP,	    0,		GEN_OFF,	    0,		GEN_NOCLIP,		TEX_MODE },		//	  Ceil	 -96	%		  0		Floor
	{ 27,	GEN_OFF,	GEN_Y1_CULL,  -96,		GEN_OFF,	  -96,		GEN_Y1,		    0,		GEN_OFF,	    0,		GEN_Y2,			TEX_MODE },		//	 [[Ceil	 -96	[%		  0		Floor]

	{ 28,	GEN_OFF,	GEN_NOCLIP,	 -128,		GEN_OFF,	 -128,		GEN_NOCLIP,	    0,		GEN_OFF,	    0,		GEN_NOCLIP,		TEX_MODE },		//	  Ceil	-128	%		  0		Floor
	{ 29,	GEN_OFF,	GEN_Y1_CULL, -128,		GEN_OFF,	 -128,		GEN_NOCLIP,	    0,		GEN_OFF,	    0,		GEN_Y2,			TEX_MODE },		//	 [[Ceil	-128	%		  0		Floor]

	{ 30,	GEN_NOCLIP,	GEN_OFF,	 -128,		GEN_OFF,	 -128,		GEN_NOCLIP,	    0,		GEN_OFF,	    0,		GEN_NOCLIP,		TEX_MODE },		//	  Sky	-128	%		  0		Floor
	{ 31,	GEN_Y1_CULL,GEN_OFF,	 -128,		GEN_OFF,	 -128,		GEN_Y1,		    0,		GEN_OFF,	    0,		GEN_Y2,			TEX_MODE },		//	 [[Sky	-128	%		  0		Floor]
	{ 32,	GEN_NOCLIP,	GEN_OFF,	  -96,		GEN_OFF,	  -96,		GEN_NOCLIP,	    0,		GEN_OFF,	    0,		GEN_NOCLIP,		TEX_MODE },		//	  Sky	 -96	%		  0		Floor
	{ 33,	GEN_Y1_CULL,GEN_OFF,	  -96,		GEN_OFF,	  -96,		GEN_Y1,		    0,		GEN_OFF,	    0,		GEN_Y2,			TEX_MODE },		//	 [[Sky	 -96	%		  0		Floor]
	{ 34,	GEN_NOCLIP,	GEN_OFF,	  -64,		GEN_OFF,	  -64,		GEN_NOCLIP,	    0,		GEN_OFF,	    0,		GEN_NOCLIP,		TEX_MODE },		//	  Sky	 -64	%		  0		Floor
	{ 35,	GEN_Y1_CULL,GEN_OFF,	  -64,		GEN_OFF,	  -64,		GEN_NOCLIP,	    0,		GEN_OFF,	    0,		GEN_Y2,			TEX_MODE },		//	 [[Sky	 -64	%		  0		Floor]

	{ -1 }
};



#if 1
	#define DREAD_SOURCE_PATH		"../dreadmake/gen"
	#define TOOL_SOURCE_PATH		"../dreadtool/gen"
#else
	#define DREAD_SOURCE_PATH		"out"
	#define TOOL_SOURCE_PATH		"out"
#endif




int main_old()
{
	vector<string> text_inc;
	map<int, string> cheat_table;
	map<int, int> clip_table;
	vector<string> file_cheatnames;
	vector<string> file_clipdist;
	vector<string> file_selector;
	vector<string> stats_csv;
	int max_cheat = 0;

	try
	{
		text_inc.push_back("");

		for( int i=0; CHEAT_TABLE[i].__name_index==i; i++ )
		{
			// Cheat name
			string cheatname;
			if( CHEAT_TABLE[i].sky_mode ) cheatname += "Sky ";
			if( CHEAT_TABLE[i].ceil_mode ) cheatname += "Ceil";
			cheatname += format("%4d%4d", -CHEAT_TABLE[i].upper_y1, -CHEAT_TABLE[i].upper_y2);
			if( CHEAT_TABLE[i].floor_mode==GEN_NOCLIP )
				cheatname += "  front";
			else
				cheatname += "  CLIPPED";

			for( int pass=0; pass<2; pass++ )
			{
				CodeGen cg;
				*(CheatInfo*)&cg = CHEAT_TABLE[i];
				string name = format("x%03d%s", cg.__name_index, pass ? "" : "p");
				cg.tex_mode = pass ? TEXM_LINEAR : TEXM_PERSPECTIVE;

				printf("$$$ Generating %s\n", name.c_str());
				cg.Generate();
				cg.as.SaveFunction(format(DREAD_SOURCE_PATH "/line_cheat_%s.s", name.c_str()), format("_Dread_LineCoreCheat_%s", name.c_str()));

				text_inc.push_back(format("\tinclude\t\"gen/line_cheat_%s.s\"", name.c_str()));

				int mode = cg.__name_index*2 + pass;
				cheat_table[mode] = name;
				if( mode>max_cheat )
					max_cheat = mode;
				if( pass==0 && !cg.upper_mode && !cg.lower_mode )
				{
					mode++;
					cheat_table[mode] = name;
					if( mode>max_cheat )
						max_cheat = mode;
					break;
				}

				//if( CHEAT_TABLE[i+1].__name_index<0 )
				//{
				//	printf("\n\n\n");
				//	cg.as.Print();
				//	break;
				//}

				// CSV stats
				if( stats_csv.size()<=0 )
					stats_csv.push_back(","+cg.as.StatCSVHeader());
				CycleStats cycles;
				cg.as.CountCycles(cycles);
				stats_csv.push_back(cg.as.StatCSVLine(name+","+cheatname, cycles));
			}


			file_cheatnames.push_back(format("\t\"%s\",", cheatname.c_str()));

			// Cheat selector
			string selector = format("\tif( %cline->tex_ceil && line->h1==%4d && line->h2==%4d ) %cm=%3d;\t//\t%s",
				CHEAT_TABLE[i].sky_mode ? '!' : ' ',
				CHEAT_TABLE[i].upper_y1,
				CHEAT_TABLE[i].upper_y2,
				CHEAT_TABLE[i].floor_mode==GEN_NOCLIP ? 'f' : 'c',
				i,
				cheatname.c_str());
			file_selector.push_back(selector);

			// Clip table
			const float CLIP_CONST = 1.6f * (1<<ENGINE_SUBVERTEX_PRECISION);		// 4: 25.6
			const int CLIP_MIN = (((80*32)<<(8+ENGINE_SUBVERTEX_PRECISION)) / 65536) + 1;
			int clipdist = (int)floor(CLIP_CONST * (-40 - CHEAT_TABLE[i].upper_y2));
			int clipdist2 = (int)floor(CLIP_CONST * (0 - -40));
			if( clipdist > clipdist2 )
				clipdist = clipdist2;
			if( clipdist < CLIP_MIN )
				clipdist = CLIP_MIN;
			int usetex = (CHEAT_TABLE[i].upper_mode || CHEAT_TABLE[i].lower_mode) ? 1 : 0;
			if( (clipdist&1) != usetex )
				clipdist++;

			string clipentry = format("\t%3d,\t//%4d\t%s", clipdist, i, cheatname.c_str());
			file_clipdist.push_back(clipentry);
			clip_table[CHEAT_TABLE[i].__name_index] = clipdist;
		}

		// Make table
		text_inc.push_back("");
		text_inc.push_back("\tpublic _CHEAT_MODES");
		text_inc.push_back("_CHEAT_MODES:");
		for( int i=0; i<=max_cheat; i+=2 )
		{
			if( cheat_table[i].size()>0 )
				text_inc.push_back(format("\tdc.l\t_Dread_LineCoreCheat_%s", cheat_table[i].c_str()).c_str());
			else
				text_inc.push_back("\tdc.l\t0");

			if( cheat_table[i+1].size()>0 )
				text_inc.push_back(format("\tdc.l\t_Dread_LineCoreCheat_%s", cheat_table[i+1].c_str()).c_str());
			else
				text_inc.push_back("\tdc.l\t0");

			text_inc.push_back(format("\tdc.w\t%d\t; clip distance", clip_table[i/2]));
			text_inc.push_back("\tdc.w\t0, 0, 0");
		}

		// Write files
		NFS.WriteFileLines(DREAD_SOURCE_PATH "/line_cheats.s", text_inc);
		NFS.WriteFileLines(TOOL_SOURCE_PATH "/cheatnames.inc", file_cheatnames);
		NFS.WriteFileLines(TOOL_SOURCE_PATH "/cheatselector.inc", file_selector);
		NFS.WriteFileLines(DREAD_SOURCE_PATH "/line_cliptable.inc", file_clipdist);
		NFS.WriteFileLines("render_stats.csv", stats_csv);
	}
	catch( const string &e )
	{
		fprintf(stderr, "ERROR: %s", e.c_str());
	}
	printf("\n\n\n");
	system("pause");
	return 0;
}
