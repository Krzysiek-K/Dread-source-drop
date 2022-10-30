
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



// ================================================================ RGBColorSpace ================================================================


vec3 RGBColorSpace::Convert(const vec3 &rgb)
{
	return vec3(
		pow(rgb.x, gamma_r),
		pow(rgb.y, gamma_g),
		pow(rgb.z, gamma_b)
	);
}

vec3 RGBColorSpace::Deconvert(const vec3 &color)
{
	return vec3(
		pow(color.x, 1.f/gamma_r),
		pow(color.y, 1.f/gamma_g),
		pow(color.z, 1.f/gamma_b)
	);
}


float RGBColorSpace::Distance(const vec3 &color1, const vec3 &color2)
{
	vec3 e_rgb = color2 - color1;
	float e_luma = color2.dot(luma_mix) - color1.dot(luma_mix);
	e_rgb.scale_xyz(e_rgb);
	e_luma *= e_luma;
	return e_rgb.dot(rgb_weight) + e_luma*luma_weight;
}

void RGBColorSpace::Load(DefNode *node)
{
	for( DefNode *n : node->subnodes )
	{
		n->type_name = "RGB colorspace option";

		try
		{
			if( n->name=="-gamma" )
			{
				n->RequireArgsOr(1, 3);
				n->ForbidSubnodes();
				gamma_r = n->GetFloatArg(0);
				gamma_g = n->GetFloatArgDef(1, gamma_r);
				gamma_b = n->GetFloatArgDef(2, gamma_r);
			}
			else if( n->name=="-weights" )
			{
				n->RequireArgsOr(3, 4);
				n->ForbidSubnodes();
				rgb_weight.x = n->GetFloatArg(0);
				rgb_weight.y = n->GetFloatArg(1);
				rgb_weight.z = n->GetFloatArg(2);
				luma_weight = n->GetFloatArgDef(3, 0.f);
			}
			else if( n->name=="-luma" )
			{
				n->RequireArgs(3);
				n->ForbidSubnodes();
				luma_mix.x = n->GetFloatArg(0);
				luma_mix.y = n->GetFloatArg(1);
				luma_mix.z = n->GetFloatArg(2);
				if( !luma_weight )
					luma_weight = 1.f;
			}
			else
				n->Unknown();
		}
		catch( const string &e )
		{
			elog.Error(NULL, e);
		}
	}
}



// ================================================================ LABColorSpace ================================================================

vec3 LABColorSpace::Convert(const vec3 &rgb)
{
	LABColor lab = LABColor::from_rgb(rgb);

	return vec3(lab.l, lab.a, lab.b);
}

vec3 LABColorSpace::Deconvert(const vec3 &color)
{
	LABColor lab;
	lab.l = color.x;
	lab.a = color.y;
	lab.b = color.z;
	return lab.to_rgb();
}


float LABColorSpace::Distance(const vec3 &color1, const vec3 &color2)
{
	vec3 error = color2 - color1;
	error.scale_xyz(error);
	return sqrt(error.x*weight_l + error.y*weight_a + error.z*weight_b);
}

void LABColorSpace::Load(DefNode *node)
{
	for( DefNode *n : node->subnodes )
	{
		n->type_name = "LAB colorspace option";

		try
		{
			if( n->name=="-weights" )
			{
				n->ForbidSubnodes();
				n->RequireArgs(3);
				weight_l = n->GetFloatArg(0);
				weight_a = n->GetFloatArg(1);
				weight_b = n->GetFloatArg(2);
			}
			else
				n->Unknown();
		}
		catch( const string &e )
		{
			elog.Error(NULL, e);
		}
	}
}

// ================================================================ YUVColorSpace ================================================================

vec3 YUVColorSpace::Convert(const vec3 &rgb)
{
	vec3 yuv;
	yuv.x = 0.299f * rgb.x + 0.587f * rgb.y + 0.114f * rgb.z;
	yuv.y = 0.492f * (rgb.z - yuv.x);
	yuv.z = 0.877f * (rgb.x - yuv.x);
	return yuv;
}

vec3 YUVColorSpace::Deconvert(const vec3 &color)
{
	vec3 rgb;
	rgb.x = color.x + 1.13983f * color.z;
	rgb.y = color.x - 0.39465f * color.y - 0.58060f * color.z;
	rgb.z = color.x + 2.03211f * color.y;
	if( rgb.x<0 ) rgb.x = 0;
	if( rgb.x>1 ) rgb.x = 1;
	if( rgb.y<0 ) rgb.y = 0;
	if( rgb.y>1 ) rgb.y = 1;
	if( rgb.z<0 ) rgb.z = 0;
	if( rgb.z>1 ) rgb.z = 1;
	return rgb;
}


float YUVColorSpace::Distance(const vec3 &color1, const vec3 &color2)
{
	vec3 error = color2 - color1;
	error.scale_xyz(error);
	return sqrt(error.x*weight_y + error.y*weight_u + error.z*weight_v);
}

void YUVColorSpace::Load(DefNode *node)
{
	for( DefNode *n : node->subnodes )
	{
		n->type_name = "YUV colorspace option";

		try
		{
			if( n->name=="-weights" )
			{
				n->ForbidSubnodes();
				n->RequireArgs(3);
				weight_y = n->GetFloatArg(0);
				weight_u = n->GetFloatArg(1);
				weight_v = n->GetFloatArg(2);
			}
			else
				n->Unknown();
		}
		catch( const string &e )
		{
			elog.Error(NULL, e);
		}
	}
}





// ================================================================ GfxPalette ================================================================


void GfxPalette::Init()
{
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
	try
	{
		if( owner->FindPalette(_name.c_str()) )
			node->Error(format("Palette name '%s' used more than once", node->args[0].c_str()));
	}
	catch( const string &e )
	{
		elog.Error(NULL, e);
	}

	name = _name;

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
				n->RequireArgs(1);
				if( n->args[0]=="rgb" )
				{
					colorspace = &rgb_colorspace;
					rgb_colorspace.Load(n);
				}
				else if( n->args[0]=="lab" )
				{
					colorspace = &lab_colorspace;
					lab_colorspace.Load(n);
				}
				else if( n->args[0]=="yuv" )		// TBD: document
				{
					colorspace = &yuv_colorspace;
					yuv_colorspace.Load(n);
				}
				else
					n->Error(format("Invalid color space '%s' (only 'rgb', 'yuv' or 'lab' supported)", n->args[0].c_str()));
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
