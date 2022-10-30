
#include "common.h"
#include "app_rendertest.h"



// ================================================================ ChunkyColorPacker ================================================================

byte ChunkyColorPacker::Process(DWORD color)
{
	byte b = 0;

	if( mode == MODE_AMIGA )
	{
		// Amiga
		if( color & 0x0001 ) b |= 0x80;
		if( color & 0x0002 ) b |= 0x20;
		if( color & 0x0004 ) b |= 0x08;
		if( color & 0x0008 ) b |= 0x02;

		if( color & 0x0100 ) b |= 0x40;
		if( color & 0x0200 ) b |= 0x10;
		if( color & 0x0400 ) b |= 0x04;
		if( color & 0x0800 ) b |= 0x01;
	}
	else if( mode == MODE_ST_GATHER )
	{
		// Atari ST
		DWORD c1 =  color & 0x000F;			// left
		DWORD c2 = (color & 0x0F00)>>8;		// right
		b = c1<<2;

		color_stats[c2*16+c1] += 1.0;
	}
	else if( mode == MODE_ST_APPLY )
	{
		DWORD c1 =  color & 0x000F;			// left
		DWORD c2 = (color & 0x0F00)>>8;		// right
		b = st_map[c2*16+c1]<<2;
	}
	else if( mode == MODE_PREVIEW )
	{
		DWORD c1 =  color & 0x000F;			// left
		DWORD c2 = (color & 0x0F00)>>8;		// right
		b = (c1<<4) | c2;		// left + right nibbles
	}

	return b;
}

void ChunkyColorPacker::ST_StartGather()
{
	color_stats.clear();
	mode = MODE_ST_GATHER;
}

void ChunkyColorPacker::ST_StartApply()
{
	vector<pair<double, DWORD> > ordered;
	memset(st_map, 0xFF, sizeof(st_map));
	st_alloc = 0;
	st_color_table.Clear();

	// sort colors by usage frequency (descending)
	for( auto p = color_stats.begin(); p!=color_stats.end(); ++p )
		ordered.push_back(pair<double, DWORD>(-p->second, p->first));
	sort(ordered.begin(), ordered.end());

	// alloc primary colors
	for( int i=0; i<16; i++ )
		ST_AssureColor(i*0x11);

	// alloc extra colors
	double weight_all = 0;
	double weight_ok = 0;
	for( int i=0; i<(int)ordered.size(); i++ )
	{
		weight_all += -ordered[i].first;
		if( ST_AssureColor(ordered[i].second) )
			weight_ok += -ordered[i].first;
	}

	printf("Allocated %d/%d colors  (%.1f%% used pixels)\n", st_alloc, ordered.size(), float(weight_ok*100/weight_all));

	// fill missing colors
	for( int i=0; i<256; i++ )
		if( st_map[i]>=0xFF )
			st_map[i] = i&0x0F;

	while( st_color_table.data.size()<64 )
		st_color_table.data.push_back(0x00FF);

	// ready
	mode = MODE_ST_APPLY;
}

bool ChunkyColorPacker::ST_AssureColor(int color)
{
	if( st_map[color]>=0xFF && st_alloc<64 )
	{
		st_map[color] = st_alloc++;
		st_color_table.data.push_back(color);
	}

	return st_map[color] < 0xFF;
}




// ================================================================ GfxPalette ================================================================


void GfxPalette::Init()
{
	if( colorspace == NULL )
		colorspace = new RGBColorSpace();

	int basepal = colors.size();
	for( int i=0; i<basepal; i++ )
	{
		GfxColor &col = colors[i];
		col.code = i*0x0101;
		col.colorspace_color = colorspace->Convert(col.rgb);
	}
}

void GfxPalette::CheckBestColor(GfxBestColorCode &best, int pid)
{
	vec3 test = colors[pid].colorspace_color;
	int code = pid*0x0101;
	float err = colorspace->Distance(best.ref_color, test);
	err = ApplyErrorWeight(code, err);

	if( err < best.first_err )
	{
		best.second_color = best.first_color;
		best.second_code = best.first_code;
		best.second_err = best.first_err;

		best.first_color = test;
		best.first_code = code;
		best.first_err = err;
	}
	else if( err < best.second_err )
	{
		best.second_color = test;
		best.second_code = code;
		best.second_err = err;
	}
}

void GfxPalette::CheckBestColorChecker(GfxBestColorCode &best, int pid1, int pid2)
{
	vec3 test = (colors[pid1].colorspace_color + colors[pid2].colorspace_color)*0.5f;
	int code = pid1*0x0001 + pid2*0x0100;
	float err = colorspace->Distance(best.ref_color, test);
	err = ApplyErrorWeight(code, err);

	if( mix_penalty>0 )
		err += mix_penalty * colorspace->Distance(colors[pid1].colorspace_color.get_scaled_xyz(mix_penalty_components), colors[pid2].colorspace_color.get_scaled_xyz(mix_penalty_components));

	if( err < best.first_err )
	{
		best.second_color = best.first_color;
		best.second_code = best.first_code;
		best.second_err = best.first_err;

		best.first_color = test;
		best.first_code = code;
		best.first_err = err;
	}
	else if( err < best.second_err )
	{
		best.second_color = test;
		best.second_code = code;
		best.second_err = err;
	}
}

void GfxPalette::CheckBestColorMix(GfxBestColorCode &best, int code, int pid1, int pid2, float mix)
{
	vec3 test = colors[pid1].colorspace_color + (colors[pid2].colorspace_color - colors[pid1].colorspace_color)*mix;
	float err = colorspace->Distance(best.ref_color, test);
	err = ApplyErrorWeight(code, err);

	if( mix_penalty>0 )
		err += mix_penalty * colorspace->Distance(colors[pid1].colorspace_color.get_scaled_xyz(mix_penalty_components), colors[pid2].colorspace_color.get_scaled_xyz(mix_penalty_components));

	if( err < best.first_err )
	{
		best.second_color = best.first_color;
		best.second_code = best.first_code;
		best.second_err = best.first_err;

		best.first_color = test;
		best.first_code = code;
		best.first_err = err;
	}
	else if( err < best.second_err )
	{
		best.second_color = test;
		best.second_code = code;
		best.second_err = err;
	}
}



void GfxPalette::DitherTexture(DataTexture &tex, bool masked)
{
	//	jj jj	jk jj	jk jk	kk jk	kk kk
	//	jj jj	jj jk	jk jk	jk kk	kk kk

	//	jj jj	jk jj	jk jk	kk kj	kk kk
	//	jj jj	jj jk	kj kj	kj kk	kk kk
	static const char *CHECKER_PATTERN[2] ={
		//"jkjjjkjkkkjk",
		//"jjjkjkjkjkkk",

		//"jkjjjkjkkkkj",
		//"jjjkkjkjkjkk",

		"jkjj jkjj jkjk jkjk kkjk kkkj kkkk",
		"jjjj jjjk jjkj kjkj kjkj kjkk kjkk",
	};

	vector<vec3> error;
	error.resize(tex.data.size(), vec3(0, 0, 0));

	//int debug_i = -1;
	for( int i=0; i<tex.data.size(); i++ )
	{
		//if( masked && i<10 )
		//	printf(" %08X:", tex.data[i]);

		if( masked && (tex.data[i]>>24)<128 )
		{
			tex.data[i] = COLOR_TRANSPARENT;
			continue;
		}

		if( colors.size() < opt_load_colors )
		{
			DWORD quantized = tex.data[i] & 0xF0F0F0;
			quantized |= quantized>>4;

			bool found = false;
			for( int j=0; j<colors.size(); j++ )
				if( colors[j].color == quantized )
				{
					found = true;
					break;
				}

			if( !found )
			{
				colors.push_back(GfxColor(quantized));
				GfxColor &col = colors[colors.size()-1];
				col.code = i*0x0101;
				col.colorspace_color = colorspace->Convert(col.rgb);
			}
		}

		vec3 ideal_color = colorspace->Convert(vec3::from_rgb(tex.data[i]));
		int tx = i % tex.width;
		int ty = i / tex.width;
		bool odd_pos = ((tx&1)!=(ty&1));

		GfxBestColorCode bc;
		if( dither_mode == DITHER_HALF_CLOSEST )
		{
			// Find the best solid color (ignore error diffusion)
			GfxBestColorCode bc1;
			bc1.ref_color = ideal_color;
			for( int j=0; j<colors.size(); j++ )
				CheckBestColor(bc1, j);

			// Find color to mix taking error into account
			bc.ref_color = ideal_color + error[i];
			for( int j=0; j<colors.size(); j++ )
				CheckBestColorChecker(bc, bc1.first_code&0xFF, j);
		}
		else if( dither_mode == DITHER_FULL )
		{
			bc.ref_color = ideal_color + error[i];
			for( int j=0; j<colors.size(); j++ )
				for( int k=j; k<colors.size(); k++ )
					CheckBestColorChecker(bc, j, k);
		}
		else if( dither_mode == DITHER_CHECKER )
		{
			bc.ref_color = ideal_color + error[i];
			for( int j=0; j<colors.size(); j++ )
				for( int k=j; k<colors.size(); k++ )
					for( int p=0; p<7; p++ )
					{
						const char *cp = CHECKER_PATTERN[ty&1] + p*5 + (tx&1)*2;
						CheckBestColorMix(bc,
							(cp[0]=='j' ? j : k)*256 +
							(cp[1]=='j' ? j : k),
							j, k, (p+1)/8.f);
					}
		}
		else if( dither_mode == DITHER_CHECKER_CLOSEST )
		{
			// Find the best solid color (ignore error diffusion)
			GfxBestColorCode bc1;
			bc1.ref_color = ideal_color;
			for( int j=0; j<colors.size(); j++ )
				CheckBestColor(bc1, j);

			// Find color to mix taking error into account
			bc.ref_color = ideal_color + error[i];
			int j = bc1.first_code&0xFF;
			for( int k=j; k<colors.size(); k++ )
				for( int p=0; p<7; p++ )
				{
					const char *cp = CHECKER_PATTERN[ty&1] + p*5 + (tx&1)*2;
					CheckBestColorMix(bc,
						(cp[0]=='j' ? j : k)*256 +
						(cp[1]=='j' ? j : k),
						j, k, (p+1)/4.f);
				}
		}
		else // DITHER_SOLID
		{
			// Find best color
			bc.ref_color = ideal_color + error[i];

			for( int j=0; j<colors.size(); j++ )
				CheckBestColor(bc, j);
		}

		// Pair order
		if( pair_order != PAIR_ORDER_KEEP )
		{
			int pid1 = bc.first_code & 0xFF;
			int pid2 = bc.first_code >> 8;
			bool rev = false;		//	PAIR_ORDER_SORT - Color codes are always $0x0y where x>=y

			if( pair_order == PAIR_ORDER_REV_SORT )
				rev = true;
			else if( pair_order == PAIR_ORDER_CHECKER || pair_order == PAIR_ORDER_SOLID_CHECKER )
				rev = odd_pos;
			else if( pair_order == PAIR_ORDER_YCHECKER )
				rev = (ty&1) ? true : false;

			if( (pid1>pid2) != rev )
				bc.first_code = (pid1<<8) | pid2;

			if( pair_order == PAIR_ORDER_SOLID_CHECKER )
				bc.first_code = (bc.first_code&0xFF) * 0x0101;
		}


		tex.data[i] = bc.first_code;

		vec3 err = bc.ref_color - bc.first_color;
		err *= diffuse_components;
		if( i+1 < error.size() ) error[i+1] += err * diffuse_kernel.x;						// 7.f/16 * 0.75
		if( i+tex.width-1 < error.size() ) error[i+tex.width-1] += err * diffuse_kernel.y;	// 3.f/16 * 0.75
		if( i+tex.width+0 < error.size() ) error[i+tex.width+0] += err * diffuse_kernel.z;	// 5.f/16 * 0.75
		if( i+tex.width+1 < error.size() ) error[i+tex.width+1] += err * diffuse_kernel.w;	// 1.f/16 * 0.75
	}
}


DWORD GfxPalette::StringToColor(DefNode *user, const char *s)
{
	int col = ParseHex(s);
	if( *s )
		user->Invalid();

	int c2 = 0;
	c2 |= ((col>>8)&0x0F)*0x110000;
	c2 |= ((col>>4)&0x0F)*0x001100;
	c2 |= ((col)&0x0F)*0x000011;
	return c2;
}

void GfxPalette::Load(DefNode *node, GfxConverter *owner, const string &_name)
{
	src_node = node;

	try
	{
		if( owner->FindPalette(_name.c_str()) )
			node->Error(format("Palette name '%s' used more than once", _name.c_str()));
	}
	catch( const string &e )
	{
		elog.Error(NULL, e);
	}

	name = _name;

	// clear non-inheriting options
	opt_export_amiga.enabled = false;
	opt_export_st.enabled = false;
	name_alias.clear();


	for( DefNode *n : node->subnodes )
	{
		n->type_name = "palette option";

		try
		{
			if( n->name.size()==4 && n->name[0]=='$' )
			{
				n->type_name = "palette color";
				n->RequireArgsOr(0, 1);
				n->ForbidSubnodes();

				colors.push_back(GfxColor(StringToColor(n, n->name.c_str()+1)));

				if( n->args.size()==1 )
				{
					if( n->args[0].size()==4 && n->args[0][0]=='$' )
						colors.back().color_out = StringToColor(n, n->args[0].c_str()+1);
					else
						n->Invalid();
				}
			}
			else if( n->name=="-clear" )	// TBD: document
			{
				n->ForbidArgs();
				n->ForbidSubnodes();
				colors.clear();
			}
			else if( n->name=="-ramp" )
			{
				n->ForbidSubnodes();
				n->RequireArgsMin(2);

				if( weights.size()==0 )
					for( int c1=0; c1<32; c1++ )
						for( int c2=0; c2<32; c2++ )
							weights[c1+c2*256] = (c1==c2) ? 1 : 0;


				int prevcol = -1;
				for( int i=0; i<n->args.size(); i++ )
				{
					int col = n->GetIntegerArg(i);
					if( col<0 || col>=colors.size() )
						n->Error(format("Color index %d exceeds max palette color index %d", col, colors.size()-1));

					if( prevcol>=0 )
						weights[col+prevcol*256] = weights[prevcol+col*256] = 1;

					prevcol = col;
				}
			}
			else if( n->name=="-weight" )
			{
				n->ForbidSubnodes();
				n->RequireArgs(3);

				int c1 = n->GetIntegerArg(0);
				float weight = n->GetFloatArg(2);
				if( n->args[1]=="*" )
				{
					for( int c2=0; c2<32; c2++ )
						weights[c1+c2*256] = weights[c2+c1*256] = weight;
				}
				else
				{
					int c2 = n->GetIntegerArg(1);
					weights[c1+c2*256] = weights[c2+c1*256] = weight;
				}
			}
			else if( n->name=="-mainpal" )
			{
				n->ForbidArgs();
				n->ForbidSubnodes();
				owner->main_palette = this;
			}
			else if( n->name=="-hudpal" )
			{
				n->ForbidArgs();
				n->ForbidSubnodes();
				owner->hud_palette = this;
			}
			else if( n->name=="-masterbrightness" )
			{
				n->RequireArgs(1);
				n->ForbidSubnodes();
				master_brightness = n->GetFloatArg(0);
			}
			else if( n->name=="-colorspace" )
			{
				colorspace = GfxColorSpace::CreateFromNode(n);
			}
			else if( n->name=="-dither" )
			{
				n->RequireArgs(1);
				n->ForbidSubnodes();
				if( n->args[0]=="solid" ) dither_mode = DITHER_SOLID;
				else if( n->args[0]=="half_closest" ) dither_mode = DITHER_HALF_CLOSEST;
				else if( n->args[0]=="full" ) dither_mode = DITHER_FULL;
				else if( n->args[0]=="checker" ) dither_mode = DITHER_CHECKER;
				else if( n->args[0]=="checker_closest" ) dither_mode = DITHER_CHECKER_CLOSEST;
				else
					n->Error(format("Invalid dither mode '%s' (use 'solid', 'half_closest', 'full', 'checker' or 'checker_closest')", n->args[0].c_str()));
			}
			else if( n->name=="-pairorder" )
			{
				n->RequireArgs(1);
				n->ForbidSubnodes();
				if( n->args[0]=="keep" ) pair_order = PAIR_ORDER_KEEP;
				else if( n->args[0]=="sort" ) pair_order = PAIR_ORDER_SORT;
				else if( n->args[0]=="rev_sort" ) pair_order = PAIR_ORDER_REV_SORT;
				else if( n->args[0]=="checker" ) pair_order = PAIR_ORDER_CHECKER;
				else if( n->args[0]=="y_checker" ) pair_order = PAIR_ORDER_YCHECKER;
				else if( n->args[0]=="solid_checker" ) pair_order = PAIR_ORDER_SOLID_CHECKER;
				else
					n->Error(format("Invalid pair order '%s' (use 'keep', 'sort', 'rev_sort' or 'checker')", n->args[0].c_str()));
			}
			else if( n->name=="-mixpenalty" )
			{
				n->RequireArgsOr(1, 4);
				n->ForbidSubnodes();
				mix_penalty = n->GetFloatArg(0);
				mix_penalty_components.x = n->GetFloatArgDef(1, 1.f);
				mix_penalty_components.y = n->GetFloatArgDef(2, 1.f);
				mix_penalty_components.z = n->GetFloatArgDef(3, 1.f);
			}
			else if( n->name=="-errordiffuse" )
			{
				n->RequireArgs(4);
				n->ForbidSubnodes();
				diffuse_kernel.x = n->GetFloatArg(0);
				diffuse_kernel.y = n->GetFloatArg(1);
				diffuse_kernel.z = n->GetFloatArg(2);
				diffuse_kernel.w = n->GetFloatArg(3);
			}
			else if( n->name=="-diffuseweights" )
			{
				n->RequireArgs(3);
				n->ForbidSubnodes();
				diffuse_components.x = n->GetFloatArg(0);
				diffuse_components.y = n->GetFloatArg(1);
				diffuse_components.z = n->GetFloatArg(2);
			}
			else if( n->name=="-palexport_amiga" || n->name=="-palexport_st" || n->name=="-palexport_ste" )		// TBD: document
			{
				PalExportOptions &pe = *(n->name=="-palexport_amiga" ? &opt_export_amiga : &opt_export_st);
				n->RequireArgs(2);
				n->ForbidSubnodes();

				pe.enabled = true;
				pe.mode_STe = (n->name=="-palexport_ste");
				pe.name = n->args[0];
				pe.color_count = n->GetIntegerArgRange(1, 2, (n->name=="-palexport_amiga" ? 32 : 16));
			}
			else if( n->name=="-colormap" )			// TBD: document
			{
				// colormap <number> [ <argb> | <rmul> <gmul> <bmul> <radd> <gadd> badd> | { ... ]
				n->RequireArgsOr(1, 2, 7);
				if( n->GetIntegerArg(0) != colormaps.size()+1 )
					n->Error(format("Invalid colormap index number (expected %d, given %d)", colormaps.size()+1, n->GetIntegerArg(0)));

				GfxPalette *pal = new GfxPalette();
				owner->palette_pool.push_back(pal);

				if( n->args.size() == 1 )
				{
					n->RequireSubnodes();
					// Load will do the rest
				}
				else
				{
					n->ForbidSubnodes();
					
					vec3 cmul, cadd;
					if( n->args.size() == 2 )
					{
						DWORD color = (DWORD)n->GetIntegerArg(1);
						float alpha = (color>>24)/255.f;
						cadd = vec3::from_rgb(color) * alpha;
						cmul = vec3(1,1,1) * (1-alpha);
					}
					else
					{
						cmul.x = n->GetFloatArg(1);
						cmul.y = n->GetFloatArg(2);
						cmul.z = n->GetFloatArg(3);
						cadd.x = n->GetFloatArg(4);
						cadd.y = n->GetFloatArg(5);
						cadd.z = n->GetFloatArg(6);
					}

					for( int i=0; i<colors.size(); i++ )
						pal->colors.push_back(GfxColor(colors[i].rgb.get_scaled_xyz(cmul) + cadd));
				}

				pal->Load(n, owner, format("%s_colormap_%d", name.c_str(), colormaps.size()+1));
				colormaps.push_back(pal);
			}
			else if( n->name=="-namealias" )			// TBD: document
			{
				n->RequireArgs(1);
				n->ForbidSubnodes();

				if( owner->FindPalette(n->args[0].c_str()) )
					n->Error(format("Palette alias name '%s' used more than once", n->args[0].c_str()));

				name_alias = n->args[0];
			}
			else if( n->name=="palette" )
			{
				GfxPalette *pal = new GfxPalette(*this);
				owner->palette_pool.push_back(pal);
				n->RequireArgs(1);

				pal->Load(n, owner, n->args[0]);
			}
			else
				n->Unknown();
		}
		catch( const string &e )
		{
			elog.Error(this, e);
		}
	}

	Init();
}



void GfxPalette::ExportGraphics(AssetExportContext &ctx)
{
	PalExportOptions &pe = *(ctx.platform == PLATFORM_ST ? &opt_export_st : &opt_export_amiga);
	if( !pe.enabled ) return;

	BinaryWordData bin;
	bin.maxlen = pe.color_count;

	for( int cmap=0; cmap<=colormaps.size(); cmap++ )
	{
		vector<GfxColor> &export_colors = *(cmap ? &colormaps[cmap-1]->colors : &colors);

		for( int i=0; i<pe.color_count; i++ )
		{
			DWORD color = (i<export_colors.size() ? export_colors[i].color_out : 0x00000000);
			word value = 0;

			if( ctx.platform == PLATFORM_ST )
			{										//	color		RRR-----GGG-----BBB-----
				value |= (color>>13) & 0x700;		//				>>>>>>>>>>>>>RRR-----GGG-----BBB-----
				value |= (color>> 9) & 0x070;		//				>>>>>>>>>RRR-----GGG-----BBB-----
				value |= (color>> 5) & 0x007;		//				>>>>>RRR-----GGG-----BBB-----
													//				        -----rrr-ggg-bbb
				if( pe.mode_STe )
				{									//	color		rrrR----gggG----bbbB----
					value |= (color>> 9) & 0x800;	//				>>>>>>>>>rrrR----gggG----bbbB----
					value |= (color>> 5) & 0x080;	//				>>>>>rrrR----gggG----bbbB----
					value |= (color>> 1) & 0x008;	//				>rrrR----gggG----bbbB----
				}									//				        ----R...G...B...
			}
			else
			{										//	color		RRRR----GGGG----BBBB----
				value |= (color>>12) & 0xF00;		//				>>>>>>>>>>>>RRRR----GGGG----BBBB----
				value |= (color>> 8) & 0x0F0;		//				>>>>>>>>RRRR----GGGG----BBBB----
				value |= (color>> 4) & 0x00F;		//				>>>>RRRR----GGGG----BBBB----
			}										//				        ----rrrrggggbbbb


			bin.AddWord(value);
		}

		bin.AddComment(format("colormap %2d", cmap));
	}

	ctx.exported_bytes += bin.WriteTable(ctx.fp, pe.name.c_str(), false);
}

bool GfxPalette::ExportPaletteBlock(DataBlock &block)
{
	if( colors.size()!=16 )
	{
		elog.Error(this, "Palette PALETTE_MAIN should have exactly 16 colors");
		return false;
	}

	block.ConfigSetupBlock(MAPBLOCK_ID_PALETTE, 16*sizeof(word), colormaps.size()+1);

	for( int cmap=0; cmap<=colormaps.size(); cmap++ )
	{
		vector<GfxColor> &export_colors = *(cmap ? &colormaps[cmap-1]->colors : &colors);

		for( int i=0; i<16; i++ )
		{
			DWORD color = (i<export_colors.size() ? export_colors[i].color_out : 0x00000000);
			word value = 0;

			//if( ctx.platform == PLATFORM_ST )
			//{										//	color		RRR-----GGG-----BBB-----
			//	value |= (color>>13) & 0x700;		//				>>>>>>>>>>>>>RRR-----GGG-----BBB-----
			//	value |= (color>> 9) & 0x070;		//				>>>>>>>>>RRR-----GGG-----BBB-----
			//	value |= (color>> 5) & 0x007;		//				>>>>>RRR-----GGG-----BBB-----
			//										//				        -----rrr-ggg-bbb
			//	if( pe.mode_STe )
			//	{									//	color		rrrR----gggG----bbbB----
			//		value |= (color>> 9) & 0x800;	//				>>>>>>>>>rrrR----gggG----bbbB----
			//		value |= (color>> 5) & 0x080;	//				>>>>>rrrR----gggG----bbbB----
			//		value |= (color>> 1) & 0x008;	//				>rrrR----gggG----bbbB----
			//	}									//				        ----R...G...B...
			//}
			//else
			{										//	color		RRRR----GGGG----BBBB----
				value |= (color>>12) & 0xF00;		//				>>>>>>>>>>>>RRRR----GGGG----BBBB----
				value |= (color>> 8) & 0x0F0;		//				>>>>>>>>RRRR----GGGG----BBBB----
				value |= (color>> 4) & 0x00F;		//				>>>>RRRR----GGGG----BBBB----
			}										//				        ----rrrrggggbbbb


			block.AddWord(value);
		}

		block.AddComment(format("colormap %2d", cmap).c_str());
	}

	return true;
}
