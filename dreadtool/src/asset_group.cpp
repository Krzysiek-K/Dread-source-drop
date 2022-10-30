
#include "common.h"
#include "app_rendertest.h"


const char GfxAssetGroup::TYPE_NAME[] = "asset group";




// ================================================================ GfxAssetGroup ================================================================



void GfxAssetGroup::Clear()
{
	for( int i=0; i<(int)assets.size(); i++ )
		delete assets[i];
	assets.clear();
}

bool GfxAssetGroup::ImportAssets()
{
	bool all_ok = true;

	for( int i=0; i<(int)assets.size(); i++ )
		if( !assets[i]->ImportAssets() )
			all_ok = false;

	return all_ok;
}

void GfxAssetGroup::Load(DefNode *node, GfxConverter *owner, GfxAssetGroup *parent, bool ignore_arg_count, bool overlay_mode, GfxAsset *gfxowner)
{
	bool light_levels_set = false;

	try
	{
		InitLoad(node, overlay_mode ? -1 : 0, parent, owner);
			
		if( !ignore_arg_count )
			node->RequireArgs(1);
		node->RequireSubnodes();

	}
	catch( const string &e )
	{
		elog.Error(NULL, e);
		return;
	}

	for( DefNode *n : node->subnodes )
	{
		n->type_name = "group option";

		try
		{
			if( n->name=="gfxgroup" || n->name=="group" )
			{
				GfxAssetGroup *ag = new GfxAssetGroup(*this);
				ag->parent = this;
				ag->name.clear();

				assets.push_back(ag);
				ag->Load(n, owner, this, false, false, NULL);
			}
			else if( n->name=="modgroup" )			// TBD: document
			{
				n->RequireArgs(1);

				string name = n->args[0];
				GfxAssetGroup *ag = NULL;
				for( int i=0; i<assets.size(); i++ )
					if( assets[i]->name == name && dynamic_cast<GfxAssetGroup*>(assets[i])!=NULL )
					{
						ag = dynamic_cast<GfxAssetGroup*>(assets[i]);
						break;
					}

				if( !ag )
					n->Error("No matching group for modgroup '"+name+"'");

				ag->Load(n, owner, this, false, true, NULL);
			}
			else if( n->name=="gfx" || n->name=="multipart" )
			{
				GfxAsset *a = new GfxAsset();
				a->is_multipart_gfx = (n->name=="multipart");
				if( a->is_multipart_gfx )
					n->RequireSubnodes();

				if( n->HasSubnodes() )
				{
					GfxAssetGroup *ag = new GfxAssetGroup(*this);
					ag->parent = this;
					ag->name.clear();

					assets.push_back(ag);
					ag->Load(n, owner, this, true, false, a);

					ag->assets.push_back(a);
					a->Load(n, ag, owner);
					ag->name = "__auto_"+a->name;
					ag->hidden = true;
				}
				else
				{
					assets.push_back(a);
					a->Load(n, this, owner);
				}
			}
			else if( n->name=="flat" )		// TBD: document
			{
				GfxFlatDesc *s = new GfxFlatDesc();
				assets.push_back(s);
				s->Load(n, this, owner);
			}
			else if( n->name=="texture_remap" )		// TBD: document
			{
				n->RequireArgs(2);
				n->ForbidSubnodes();
				converter_owner->texture_remap[n->args[0]] = n->args[1];
			}
			else if( n->name=="anim" )
			{
				GfxAnimation *a = new GfxAnimation();
				assets.push_back(a);
				a->Load(n, this, owner);
			}
			else if( n->name=="sound" )		// TBD: document
			{
				SoundAsset *s = new SoundAsset();
				if( n->HasSubnodes() )
				{
					GfxAssetGroup *ag = new GfxAssetGroup(*this);
					ag->parent = this;
					ag->name.clear();

					assets.push_back(ag);
					ag->Load(n, owner, this, true, false, NULL);

					ag->assets.push_back(s);
					s->Load(n, ag, owner);
					ag->name = "__autosfx_"+s->name;
					ag->hidden = true;
				}
				else
				{
					assets.push_back(s);
					s->Load(n, this, owner);
				}
			}
			else if( n->name=="trksound" )		// TBD: document
			{
				SoundAsset *s = new SoundAsset();
				assets.push_back(s);
				s->LoadTracked(n, this, owner);
			}
			else if( n->name=="-fontgroup" )	// TBD: document
			{
				n->ForbidArgs();
				n->ForbidSubnodes();
				converter_owner->AddAsset(this);
			}
			else if( n->name=="-verbose" )
			{
				n->ForbidSubnodes();
				n->RequireArgsOr(0, 1);
				opt_verbose = (n->GetIntegerArgDef(0, 1)!=0);
			}
			else if( n->name=="-path" )
			{
				n->ForbidSubnodes();
				n->RequireArgs(1);

				src_path = FilePathGetPart(n->parser->filenames[n->file_id].c_str(), true, false, false) + n->args[0].c_str();
			}
			else if( n->name=="-masked" )
			{
				n->ForbidSubnodes();
				n->RequireArgsOr(0, 1);
				opt_masked = (n->GetIntegerArgDef(0, 1)!=0);
			}
			else if( n->name=="-colorkey" )
			{
				n->ForbidSubnodes();
				n->RequireArgsOr(1, 2);

				if( n->args[0]=="off" )
				{
					if( n->args.size()!=1 )
						n->Error(format("'-colorkey off' can't have 2 arguments"));

					opt_colorkey_mode = COLORKEY_OFF;
					opt_masked = false;
				}
				else if( n->args[0]=="*" )
				{
					opt_colorkey_mode = COLORKEY_AUTO;
					opt_colormask = n->GetIntegerArgDef(1, 0xFFFFFF);
					opt_masked = true;
				}
				else
				{
					opt_colorkey_mode = COLORKEY_MASKED;
					opt_colorkey = n->GetIntegerArg(0);
					opt_colormask = n->GetIntegerArgDef(1, 0xFFFFFF);
					opt_masked = true;
				}

			}
			else if( n->name=="-scroll" )
			{
				n->ForbidSubnodes();
				n->RequireArgs(2);
				opt_scroll_x = n->GetIntegerArg(0);
				opt_scroll_y = n->GetIntegerArg(1);
			}
			else if( n->name=="-trim" )
			{
				n->ForbidSubnodes();
				n->RequireArgsOr(0, 1);
				opt_trim = (n->GetIntegerArgDef(0, 1)!=0);
			}
			else if( n->name=="-downscale" )
			{
				n->ForbidSubnodes();
				n->RequireArgsRange(0, 2);
				if( n->args.size()==1 && (n->args[0]=="hd" || n->args[0]=="HD" || n->args[0]=="-2") )
				{
					opt_resample_mode = RESAMPLE_DOWNSCALE_2Y;
					opt_pixel_pack = PIXELPACK_HD;
				}
				else
				{
					int dx = n->GetIntegerArgDef(0, 2);
					int dy = n->GetIntegerArgDef(1, dx);

					/**/ if( dx==2 && dy==2 ) opt_resample_mode = RESAMPLE_DOWNSCALE_2;
					else if( dx<=1 && dy==2 ) opt_resample_mode = RESAMPLE_DOWNSCALE_2Y;
					else	n->Error(format("Invalid -downscale argument combination %d %d", dx, dy));

					opt_pixel_pack = PIXELPACK_OFF;
				}
			}
			else if( n->name=="-lanczos" )
			{
				// -lanczos <x0> <y0> <xstep> [<ystep>]  [blur <bx> [<by>]]  [width <wx> [<wy>]]  [pack]
				n->ForbidSubnodes();
				n->RequireArgsMin(3);

				opt_resample_mode = RESAMPLE_LANCZOS;
				opt_pixel_pack = PIXELPACK_OFF;

				int apos = 0;
				opt_lanczos.pos_base.x = n->GetIntegerArg(apos++);
				opt_lanczos.pos_base.y = n->GetIntegerArg(apos++);
				opt_lanczos.pos_step.x = n->GetIntegerArg(apos++);
				opt_lanczos.pos_step.y = n->IsArgFloat(apos) ? n->GetIntegerArg(apos++) : opt_lanczos.pos_step.x;
				opt_lanczos.window_size.x = opt_lanczos.pos_step.x * 3;
				opt_lanczos.window_size.y = opt_lanczos.pos_step.y * 3;
				opt_lanczos.kernel_size.x = opt_lanczos.pos_step.x;
				opt_lanczos.kernel_size.y = opt_lanczos.pos_step.y;

				while( apos<n->args.size() )
				{
					if( n->args[apos]=="width" )
					{
						apos++;
						float wx = n->GetFloatArg(apos++);
						opt_lanczos.window_size.x = wx * opt_lanczos.kernel_size.x;
						opt_lanczos.window_size.y = (n->IsArgFloat(apos) ? n->GetFloatArg(apos++) : wx) * opt_lanczos.kernel_size.y;
					}
					else if( n->args[apos]=="blur" )
					{
						apos++;
						float bx = n->GetFloatArg(apos++);
						opt_lanczos.kernel_size.x = bx;
						opt_lanczos.kernel_size.y = (n->IsArgFloat(apos) ? n->GetFloatArg(apos++) : bx);
					}
					else if( n->args[apos]=="pack" )
					{
						apos++;
						opt_pixel_pack = PIXELPACK_HD;
					}
					else
						n->InvalidArg(apos);
				}
			}
			else if( n->name=="-remap" )			// TBD: document
			{
				n->ForbidSubnodes();
				n->RequireArgs(1);

				const char *s = n->args[0].c_str();
				opt_color_remap.clear();
				while( *s )
				{
					/**/ if( *s>='0' && *s<='9' ) opt_color_remap.push_back(*s-'0');
					else if( *s>='A' && *s<='Z' ) opt_color_remap.push_back(*s-'A'+10);
					else if( *s>='a' && *s<='z' ) opt_color_remap.push_back(*s-'a'+10);
					else if( *s=='-' ) opt_color_remap.push_back(opt_color_remap.size());
					else n->InvalidArg(0);
					s++;
				}
			}
			else if( n->name=="-pixelpack" )		// TBD: document
			{
				n->ForbidSubnodes();
				n->RequireArgs(1);

				/**/ if( n->args[0]=="off" ) opt_pixel_pack = PIXELPACK_OFF;
				else if( n->args[0]=="hd" ) opt_pixel_pack = PIXELPACK_HD;
				else if( n->args[0]=="hd_black" ) opt_pixel_pack = PIXELPACK_HD_BLACK;
				else if( n->args[0]=="hd_keephalf" ) opt_pixel_pack = PIXELPACK_HD_KEEPHALF;
				else if( n->args[0]=="xd" ) opt_pixel_pack = PIXELPACK_XD;
				else if( n->args[0]=="xd_wrap" ) opt_pixel_pack = PIXELPACK_XD_WRAP;
				else if( n->args[0]=="xd_narrow" ) opt_pixel_pack = PIXELPACK_XD_NARROW;
				else if( n->args[0]=="xd_wrap_narrow" ) opt_pixel_pack = PIXELPACK_XD_WRAP_NARROW;
				else									  n->InvalidArg(0);
			}
			else if( n->name=="-visualscale" )
			{
				n->ForbidSubnodes();
				n->RequireArgsOr(1, 2);
				opt_visualscale_x = n->GetFloatArg(0);
				opt_visualscale_y = n->GetFloatArgDef(1, opt_visualscale_x);
			}
			else if( n->name=="-collapse" )
			{
				n->ForbidSubnodes();
				n->RequireArgsOr(0, 1);
				opt_resample_mode = (n->GetIntegerArgDef(0, 1)!=0) ? RESAMPLE_COLLAPSE : RESAMPLE_OFF;
				opt_pixel_pack = PIXELPACK_OFF;
			}
			else if( n->name=="-texture" )
			{
				n->ForbidSubnodes();
				n->ForbidArgs();
				opt_usage = ASSET_USAGE_TEXTURE;
			}
			else if( n->name=="-graphics" )
			{
				n->ForbidSubnodes();
				n->ForbidArgs();
				opt_usage = ASSET_USAGE_GRAPHICS;
			}
			else if( n->name=="-sky" )			// TBD: document
			{
				n->ForbidSubnodes();
				n->ForbidArgs();
				opt_usage = ASSET_USAGE_SKY;
			}
			else if( n->name=="-flat" )
			{
				n->ForbidSubnodes();
				n->ForbidArgs();
				opt_usage = ASSET_USAGE_FLAT;
				opt_resample_mode = RESAMPLE_COLLAPSE;
				opt_pixel_pack = PIXELPACK_OFF;
			}
			else if( n->name=="-weapon" )
			{
				n->ForbidSubnodes();
				n->ForbidArgs();
				opt_usage = ASSET_USAGE_WEAPON;
			}
			else if( n->name=="-weaponsprite" )
			{
				n->ForbidSubnodes();
				n->ForbidArgs();
				opt_usage = ASSET_USAGE_WEAPON_SPRITE;
			}
			else if( n->name=="-object" )
			{
				n->ForbidSubnodes();
				n->ForbidArgs();
				opt_usage = ASSET_USAGE_OBJECT;
			}
			else if( n->name=="-palette" )			// TBD: option to make sure palette doesn't exceed specified number of colors
			{
				n->RequireArgs(1);
				if( !opt_palette || n->args[0]!="*" )
					opt_palette = owner->FindPalette(n->args[0].c_str());
				if( !opt_palette )
					n->Error(format("Undefined palette '%s'", n->args[0].c_str()));

				if( n->HasSubnodes() )
				{
					opt_palette = new GfxPalette(*opt_palette);
					owner->palette_pool.push_back(opt_palette);
					opt_palette->Load(n, owner, "__auto_"+n->args[0]+"_"+name);
				}
			}
			else if( n->name=="-autopal" )
			{
				n->RequireArgsOr(1, 2);

				GfxPalette *base_pal = NULL;
				int argpos = 0;
				if( n->args.size()>=2 )
				{
					if( n->args[argpos]!="*" )
						base_pal = opt_palette;
					else
						base_pal = owner->FindPalette(n->args[argpos].c_str());
					argpos++;
				}

				if( base_pal )
					opt_palette = new GfxPalette(*base_pal);
				else
					opt_palette = new GfxPalette();

				owner->palette_pool.push_back(opt_palette);
				opt_palette->Load(n, owner, "__autopal_"+name);
				opt_palette->opt_load_colors = n->GetIntegerArg(argpos);

			}
			else if( n->name=="-gamma" )
			{
				n->RequireArgsOr(1, 3);
				n->ForbidSubnodes();
				color_gamma.x = n->GetFloatArg(0);
				color_gamma.y = n->GetFloatArgDef(1, color_gamma.x);
				color_gamma.z = n->GetFloatArgDef(2, color_gamma.x);
			}
			else if( n->name=="-gain" )
			{
				n->RequireArgsOr(1, 3);
				n->ForbidSubnodes();
				color_gain.x = n->GetFloatArg(0);
				color_gain.y = n->GetFloatArgDef(1, color_gain.x);
				color_gain.z = n->GetFloatArgDef(2, color_gain.x);
			}
			else if( n->name=="-lift" )
			{
				n->RequireArgsOr(1, 3);
				n->ForbidSubnodes();
				color_lift.x = n->GetFloatArg(0);
				color_lift.y = n->GetFloatArgDef(1, color_lift.x);
				color_lift.z = n->GetFloatArgDef(2, color_lift.x);
			}
			else if( n->name=="-lightlevel" )
			{
				if( !light_levels_set )
				{
					light_levels.clear();
					light_levels_set = true;
				}

				n->ForbidSubnodes();
				n->RequireArgsOr(2, 3);

				LightLevel ll;
				ll.light = n->GetIntegerArg(0);
				ll.gain = vec3(1, 1, 1) * n->GetFloatArg(1);
				ll.path_ext = (n->args.size()>=3) ? n->args[2] : "";
				light_levels.push_back(ll);
			}
			else if( n->name=="-nolight" )
			{
				n->ForbidSubnodes();
				n->ForbidArgs();
				light_levels.clear();
				light_levels_set = true;
			}
			else if( n->name=="-export" || n->name=="-export_amiga" || n->name=="-export_st" )
			{
				ExportSettings &es = *((n->name=="-export_st") ? &export_st : &export_amiga);
				n->ForbidSubnodes();
				for( int argpos = 0; argpos<(int)n->args.size(); argpos++ )
				{
					const string &arg = n->args[argpos];
					if( arg=="off" )	es.export_mode = EXPORT_OFF;
					else if( arg=="bitplane" )	es.export_mode = EXPORT_BITPLANES;
					else if( arg=="bitplane_interleaved" )	es.export_mode = EXPORT_BITPLANES_INTERLEAVED;
					else if( arg=="bitplane_st" )	es.export_mode = EXPORT_BITPLANES_ST;
					else if( arg=="font_mask" )	es.export_mode = EXPORT_FONT_MASK;
					else if( arg=="header" )	es.write_header = true;
					else if( arg=="noheader" )	es.write_header = false;
					else if( arg=="0bpp" )	es.num_bitplanes = 0;
					else if( arg=="1bpp" )	es.num_bitplanes = 1;
					else if( arg=="2bpp" )	es.num_bitplanes = 2;
					else if( arg=="3bpp" )	es.num_bitplanes = 3;
					else if( arg=="4bpp" )	es.num_bitplanes = 4;
					else if( arg=="5bpp" )	es.num_bitplanes = 5;
					else if( arg=="6bpp" )	es.num_bitplanes = 6;
					else if( arg=="chipmem" )	es.chipmem = true;
					else if( arg=="index" )	es.write_index = INDEX_ASSET_LIST;				// TBD: document
					else if( arg=="info" )	es.write_info = true;							// TBD: document
					else n->InvalidArg(argpos);
				}

				if( n->name=="-export" )
					export_st = export_amiga;
			}
			else if( n->name=="-audio_resample" )		// TBD: document
			{
				n->ForbidSubnodes();
				n->RequireArgs(1);
				opt_audio_resample = n->GetFloatArg(0);
			}
			else if( n->name=="-audio_gain" )		// TBD: document
			{
				n->ForbidSubnodes();
				n->RequireArgs(1);
				opt_audio_gain = n->GetFloatArg(0);
			}
			else if( gfxowner )
			{
				gfxowner->extra_options.push_back(n);
			}
			else
				n->Unknown();
		}
		catch( const string &e )
		{
			elog.Error(NULL, e);
			return;
		}
	}
}

DWORD GfxAssetGroup::GetColor()
{
	switch( opt_usage )
	{
	case ASSET_USAGE_TEXTURE:		return 0xFFC0C0C0;
	case ASSET_USAGE_FLAT:			return 0xFF808080;
	case ASSET_USAGE_OBJECT:		return 0xFFFFFF80;
	case ASSET_USAGE_WEAPON:		return 0xFF80FFFF;
	case ASSET_USAGE_WEAPON_SPRITE:	return 0xFF00FFFF;
	case ASSET_USAGE_GRAPHICS:		return 0xFF80FF80;
	case ASSET_USAGE_SKY:			return 0xFF8080FF;
	}
	return 0xFFFFFFFF;
}

void GfxAssetGroup::ExportGraphics(AssetExportContext &ctx)
{
	const GfxAssetGroup::ExportSettings &es = *(ctx.platform == PLATFORM_ST ? &export_st : &export_amiga);
	int info_start = ctx.info_table.size();

	for( AssetBase *a : assets )
		a->ExportGraphics(ctx);


	// Export tagged groups assets
	if( es.export_mode == GfxAssetGroup::EXPORT_FONT_MASK )
	{
		// gather
		map<int, GfxTexture*> glyphs;
		int mincode = 255;
		int maxcode = 0;
		for( int i=0; i<assets.size(); i++ )
		{
			GfxAsset *a = dynamic_cast<GfxAsset*>(assets[i]);
			if( !a || a->textures.size()<=0 || !a->textures[0] ) continue;

			const char *s = a->name.c_str();
			const char *e = s;
			while( *e ) e++;
			while( s<e && e[-1]>='0' && e[-1]<='9' ) e--;
			int code = ParseInt(e);

			if( code<mincode ) mincode = code;
			if( code>maxcode ) maxcode = code;

			glyphs[code] = a->textures[0];
		}

		if( glyphs.size() > 0 )
		{
			// generate
			BinaryWordData bin;
			bin.maxlen = 16;


			// header
			int height = glyphs.begin()->second->texture.height;
			bin.AddWord(height);
			bin.AddWord(mincode);
			bin.AddWord(maxcode - mincode + 1);
			bin.AddComment("height, start code, num codes");

			// widths
			for( int code=mincode; code<=maxcode; code++ )
			{
				GfxTexture *tex = glyphs[code];
				if( !tex )
				{
					bin.AddWord(0);
					bin.AddComment(format("%3d missing", code));
				}
				else
				{
					bin.AddWord(tex->texture.width);
					bin.AddComment(format("%3d", code));
				}
			}

			// data
			for( int code=mincode; code<=maxcode; code++ )
			{
				GfxTexture *tex = glyphs[code];
				if( !tex )
				{
					for( int i=0; i<height; i++ )
						bin.AddWord(0);
					bin.AddComment(format("%3d missing", code));
				}
				else
				{
					for( int i=0; i<height; i++ )
						bin.AddWord(tex->texture.GatherBitplaneWord(0, i, -1));
					bin.AddComment(format("%3d", code));
				}
			}

			// write
			ctx.exported_bytes += bin.WriteTable(ctx.fp, format("GFX_%s", name.c_str()).c_str(), es.chipmem);
		}
	}

	if( es.write_index==INDEX_ASSET_LIST && !hidden )
	{
		// info
		fprintf(ctx.fp, "\nconst word *GFXGROUP_%s[] = {\n", name.c_str());
		for( int i=info_start; i<ctx.info_table.size(); i++ )
		{
			fprintf(ctx.fp, "\t%s,\n", ctx.info_table[i].c_str());
			ctx.exported_bytes += 4;
		}
		fprintf(ctx.fp, "};\n");

		ctx.info_table.resize(info_start);
	}
}
