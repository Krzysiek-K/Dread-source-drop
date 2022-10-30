
#include "common.h"
#include "app_rendertest.h"


const char GfxAsset::TYPE_NAME[] = "gfx asset";




// ================================================================ GfxAsset ================================================================


bool GfxAsset::ImportAssets()
{
	bool all_files_ok = true;

	Clear();

	static const GfxAssetGroup::LightLevel default_light_level;
	const GfxAssetGroup::LightLevel *light_levels = &default_light_level;
	int num_light_levels = 1;

	if( asset_group->light_levels.size()>0 )
	{
		num_light_levels = (int)asset_group->light_levels.size();
		light_levels = &asset_group->light_levels[0];
	}

	if( is_multipart_gfx )
		num_light_levels = 0;

	for( int i=0; i<num_light_levels; i++ )
	{
		try
		{
			const GfxAssetGroup::LightLevel &ll = light_levels[i];
			string path = modmgr.GetResourcePath(asset_group->src_path + "/" + file_name + ll.path_ext, ".png;.bmp;.jpg;.tga;.dds;.dib;.hdr;.pfm;.ppm");

			GfxTexture *tex = new GfxTexture();
			if( !tex->LoadTexture(name.c_str(), path.c_str()) )
			{
				delete tex;
				Error(format("Can't load texture '%s'", path.c_str()));
				all_files_ok = false;
				continue;
			}

			converter_owner->WatchFile(tex->texture.tex.GetLoadPath(), this);

			tex->orig_w = (int)floor(tex->orig_w*asset_group->opt_visualscale_x+0.5f);
			tex->orig_h = (int)floor(tex->orig_h*asset_group->opt_visualscale_y+0.5f);
			tex->orig_ox = tex->texture.off_x = xoffs;
			tex->orig_oy = tex->texture.off_y = yoffs;

			// Keep
			textures.push_back(tex);
			tex->light_level = ll.light;

			// Color key
			if( asset_group->opt_colorkey_mode == GfxAssetGroup::COLORKEY_AUTO )
			{
				if( tex->texture.data.size()>0 )
					tex->texture.ColorMask(tex->texture.data[0], asset_group->opt_colormask);
			}
			else if( asset_group->opt_colorkey_mode == GfxAssetGroup::COLORKEY_MASKED )
			{
				tex->texture.ColorMask(asset_group->opt_colorkey, asset_group->opt_colormask);
			}

			// Scroll
			if( asset_group->opt_scroll_x || asset_group->opt_scroll_y )
				tex->texture.Scroll(asset_group->opt_scroll_x, asset_group->opt_scroll_y);

			// Resample (downscale)
			switch( asset_group->opt_resample_mode )
			{
			case GfxAssetGroup::RESAMPLE_COLLAPSE:				tex->texture.Collapse();			break;
			case GfxAssetGroup::RESAMPLE_DOWNSCALE_2:			tex->texture.Downscale();			break;
			case GfxAssetGroup::RESAMPLE_DOWNSCALE_2Y:			tex->texture.DownscaleY();			break;
			case GfxAssetGroup::RESAMPLE_LANCZOS: {
				GfxLanczosResampler lanczos;
				lanczos.pos_base = asset_group->opt_lanczos.pos_base;
				lanczos.pos_step = asset_group->opt_lanczos.pos_step;
				lanczos.kernel_size = asset_group->opt_lanczos.kernel_size;
				lanczos.window_size = asset_group->opt_lanczos.window_size;
				lanczos.Process(tex->texture);
				break;
			}
			}

			// Adjust
			tex->ApplyColorCorrections(
				asset_group->color_gamma.get_scaled_xyz(ll.gamma),
				asset_group->color_gain.get_scaled_xyz(ll.gain),
				asset_group->color_lift + ll.lift,
				asset_group->opt_palette->master_brightness);

			// Dither
			asset_group->opt_palette->DitherTexture(tex->texture, asset_group->opt_masked);

			// Pixel remap
			if( asset_group->opt_color_remap.size()>0 )
				tex->texture.ColorIndexRemap(asset_group->opt_color_remap);

			// Pixel process - horizontal packing
			switch( asset_group->opt_pixel_pack )
			{
			case GfxAssetGroup::PIXELPACK_HD:				tex->texture.IndexHorizPack();				tex->texture.FixTransparency(0);		break;
			case GfxAssetGroup::PIXELPACK_HD_BLACK:			tex->texture.IndexHorizPack();				tex->texture.FixTransparency(4);		break;
			case GfxAssetGroup::PIXELPACK_HD_KEEPHALF:		tex->texture.IndexHorizPack();														break;
			case GfxAssetGroup::PIXELPACK_XD:				tex->texture.IndexHorizScroll(false);		tex->texture.FixTransparency(2);		break;
			case GfxAssetGroup::PIXELPACK_XD_WRAP:			tex->texture.IndexHorizScroll(true);		tex->texture.FixTransparency(2);		break;
			case GfxAssetGroup::PIXELPACK_XD_NARROW:		tex->texture.IndexHorizScroll(false);		tex->texture.FixTransparency(1);		break;
			case GfxAssetGroup::PIXELPACK_XD_WRAP_NARROW:	tex->texture.IndexHorizScroll(true);		tex->texture.FixTransparency(1);		break;
			}

			// Overlay static images
			for( int i=0; i<overlays.size(); i++ )
				if( overlays[i].mode==Overlay::M_STATIC && overlays[i].gfx.get()!=NULL && overlays[i].gfx->textures.size()>0 )
				{
					Overlay &ov = overlays[i];
					tex->texture.IndexOverlay(ov.gfx->textures[0]->texture, ov.xoffs, ov.yoffs);
				}

			// Trim
			if( asset_group->opt_trim )
				tex->texture.IndexTrim();

			if( asset_group->opt_usage==ASSET_USAGE_WEAPON_SPRITE )
			{
				if( tex->texture.width>64 )
					Error(format("Weapon sprite '%s' is wider then 64px (currently %dpx)", name.c_str(), tex->texture.width));
			}

			// Upload
			tex->texture.Upload();

			// Check palette
			if( asset_group->opt_usage==ASSET_USAGE_WEAPON_SPRITE )
			{
				if( asset_group->opt_palette->colors.size() > 15 )
					Error(format("Weapon sprite '%s' uses palette with more than 15 colors (currently %d)", name.c_str(), asset_group->opt_palette->colors.size()));
			}

			if( asset_group->opt_verbose )
				printf("Imported %-12s (light %3d, offsets %+3d %+3d)\n", name.c_str(), tex->light_level, tex->texture.off_x, tex->texture.off_y);
		}
		catch( const string &e )
		{
			elog.Error(this, e);
		}
	}

	return all_files_ok;
}

void GfxAsset::Load(DefNode *node, GfxAssetGroup *aowner, GfxConverter *owner)
{
	try
	{
		InitLoad(node, 0, aowner, owner);

		int argpos = 1;
		bool has_coords = false;
		while( argpos < node->args.size() )
		{
			if( node->args[argpos] == "-file" && argpos+1<node->args.size() )
			{
				argpos++;
				file_name = node->args[argpos++];
			}
			else if( argpos+1<node->args.size() && node->IsArgInteger(argpos) && node->IsArgInteger(argpos+1) && !has_coords )
			{
				xoffs = node->GetIntegerArg(argpos++);
				yoffs = node->GetIntegerArg(argpos++);
				has_coords = true;
			}
			else
				node->InvalidArg(argpos);
		}

		if( asset_group->opt_palette == NULL )
			Error(format("Can't load asset without palette selected for the group"));

		owner->AddAsset(this);
	}
	catch( const string &e )
	{
		elog.Error(this, e);
		return;
	}

	for( int i=0; i<extra_options.size(); i++ )
		if( extra_options[i].get() !=NULL )
			LoadOption(extra_options[i].get());
}

void GfxAsset::LoadOption(DefNode *n)
{
	if( n->name=="static" )
	{
		// static <gfx> <x> <y>
		if( is_multipart_gfx )
			n->Error("'static' components are not allowed in multipart graphics");
		n->ForbidSubnodes();
		n->RequireArgs(3);
		
		Overlay ov;
		ov.mode =  Overlay::M_STATIC;
		ov.gfx = converter_owner->FindGfxAsset(n->args[0].c_str(), false);
		ov.xoffs = n->GetIntegerArg(1);
		ov.yoffs = n->GetIntegerArg(2);
		overlays.push_back(ov);
	}
	else if( n->name=="icon" )
	{
		// icon <gfx> <x> <y> <varname>
		if( is_multipart_gfx )
			n->Error("'icon' components are not allowed in multipart graphics");
		n->ForbidSubnodes();
		n->RequireArgs(4);

		Overlay ov;
		ov.mode =  Overlay::M_ICON;
		ov.gfx = converter_owner->FindGfxAsset(n->args[0].c_str(), false);
		ov.xoffs = n->GetIntegerArg(1);
		ov.yoffs = n->GetIntegerArg(2);
		ov.var_name = n->args[3];
		overlays.push_back(ov);
	}
	else if( n->name=="frame" )						// TBD: document
	{
		// frame <font> <x> <y> <varname>
		if( is_multipart_gfx )
			n->Error("'frame' components are not allowed in multipart graphics");
		n->ForbidSubnodes();
		n->RequireArgs(4);

		Overlay ov;
		ov.mode =  Overlay::M_FRAME;
		ov.font = converter_owner->FindAsset<GfxAssetGroup>(n->args[0].c_str(), false);
		ov.xoffs = n->GetIntegerArg(1);
		ov.yoffs = n->GetIntegerArg(2);
		ov.var_name = n->args[3];
		ov.num_digits = 1;
		overlays.push_back(ov);
	}
	else if( n->name=="number" )
	{
		// number <font> <x> <y> <step> <varname> <max>
		if( is_multipart_gfx )
			n->Error("'number' components are not allowed in multipart graphics");
		n->ForbidSubnodes();
		n->RequireArgs(6);

		Overlay ov;
		ov.mode =  Overlay::M_NUMBER;
		ov.font = converter_owner->FindAsset<GfxAssetGroup>(n->args[0].c_str(), false);
		ov.xoffs = n->GetIntegerArg(1);
		ov.yoffs = n->GetIntegerArg(2);
		ov.font_step = n->GetIntegerArg(3);
		ov.var_name = n->args[4];
		ov.num_max = n->GetIntegerArg(5);
		ov.num_digits = 1;
		if( ov.num_max >= 10 ) ov.num_digits = 2;
		if( ov.num_max >= 100 ) ov.num_digits = 3;
		if( ov.num_max >= 1000 ) ov.num_digits = 4;
		if( ov.num_max >= 10000 ) ov.num_digits = 5;
		overlays.push_back(ov);
	}
	else if( n->name=="part" )						// TBD: document
	{
		// part <gfx> <x> <y>
		if( !is_multipart_gfx )
			n->Error("'part' components are only allowed in multipart graphics");
		n->ForbidSubnodes();
		n->RequireArgs(3);

		Overlay ov;
		ov.mode =  Overlay::M_PART;
		ov.gfx = converter_owner->FindGfxAsset(n->args[0].c_str(), false);
		ov.xoffs = n->GetIntegerArg(1);
		ov.yoffs = n->GetIntegerArg(2);
		overlays.push_back(ov);
	}
	else
		n->Unknown();
}


void GfxAsset::SavePreview(const char *path, bool save_light_levels)
{
	if( !asset_group || !asset_group->opt_palette )
		return;


	for( int level=0; level<textures.size(); level++ )
	{
		if( !textures[level] )
			continue;

		if( level>0 && !save_light_levels )
			break;

		DataTexture &tex = textures[level]->texture;
		if( tex.width<1 || tex.height<1 )
			continue;

		int w = tex.width;
		int yscale = 2;
		if( asset_group->opt_resample_mode == GfxAssetGroup::RESAMPLE_DOWNSCALE_2Y )
			yscale = 4;

		vector<DWORD> texdata(w*2*tex.height*yscale);
		DWORD *dst = &texdata[0];

		for( int y=0; y<tex.height; y++ )
			for( int yrep=0; yrep<yscale; yrep++ )
				for( int x=0; x<w; x++ )
				{
					DWORD c0 = tex.data[x+y*w]&0xFF;
					DWORD c1 = (tex.data[x+y*w]>>8)&0xFF;

					for( int half=0; half<2; half++, c0=c1 )
					{
						if( c0==(COLOR_TRANSPARENT&0xFF) )
							*dst++ = 0x00000000;
						else if( asset_group->export_amiga.export_mode == GfxAssetGroup::EXPORT_FONT_MASK )
							*dst++ = 0xFFFFFFFF;
						else
							*dst++ = asset_group->opt_palette->GetColor(c0).color_out | 0xFF000000;
					}
				}

		DevTexture dtex;
		dtex.LoadRaw2D(D3DFMT_A8R8G8B8, w*2, tex.height*yscale, &texdata[0], false);

		string dumpname = name;
		if( textures[level]->light_level < 255 && save_light_levels )
			dumpname = format("%s_%d", name.c_str(), textures[level]->light_level);

		D3DXSaveTextureToFile(format(path, dumpname.c_str()).c_str(), D3DXIFF_PNG, dtex, NULL);
	}
}


void GfxAsset::ExportPK3Texture(const char *path, int fudge_border)
{
	if( !asset_group || !asset_group->opt_palette || textures.size()<=0 )
		return;

	DataTexture &tex = textures[0]->texture;
	if( tex.width<1 || tex.height<1 )
		return;

	int exp_height = tex.height*2 + fudge_border;
	vector<DWORD> texdata(tex.width*exp_height);
	DWORD *dst = &texdata[0];

	for( int ey=0; ey<exp_height; ey++ )
	{
		int y = ey/2;
		if( y<tex.height )
		{
			for( int x=0; x<tex.width; x++ )
			{
				DWORD c = tex.data[x+y*tex.width]&0xFF;
				*dst++ = asset_group->opt_palette->GetColor(c).color_out | 0xFF000000;
			}
		}
		else
			for( int x=0; x<tex.width; x++ )
				*dst++ = 0xFFFF00FF;
	}

	DevTexture dtex;
	dtex.LoadRaw2D(D3DFMT_X8R8G8B8, tex.width, exp_height, &texdata[0], false);

	D3DXSaveTextureToFile(format(path, name.c_str()).c_str(), D3DXIFF_PNG, dtex, NULL);
}

void GfxAsset::ExportPK3Sprite(const char *path)
{
	if( !asset_group || !asset_group->opt_palette || textures.size()<=0 )
		return;

	DataTexture &tex = textures[0]->texture;
	if( tex.width<1 || tex.height<1 )
		return;

	vector<DWORD> texdata(tex.width*tex.height*2);
	DWORD *dst = &texdata[0];

	for( int y=0; y<tex.height; y++ )
		for( int rep=0; rep<2; rep++ )
			for( int x=0; x<tex.width; x++ )
			{
				DWORD c = tex.data[x+y*tex.width]&0xFF;
				if( c==(COLOR_TRANSPARENT&0xFF) )
					*dst++ = 0x00000000;
				else
					*dst++ = asset_group->opt_palette->GetColor(c).color_out | 0xFF000000;
			}

	DevTexture dtex;
	dtex.LoadRaw2D(D3DFMT_A8R8G8B8, tex.width, tex.height*2, &texdata[0], false);

	D3DXSaveTextureToFile(format(path, name.c_str()).c_str(), D3DXIFF_PNG, dtex, NULL);
}


void GfxAsset::ExportGraphics(AssetExportContext &ctx)
{
	if( !asset_group || textures.size()<1 || !textures[0] ) return;

	GfxTexture *tex = textures[0];


	GfxAssetGroup::ExportSettings &es = *(ctx.platform == PLATFORM_ST ? &asset_group->export_st : &asset_group->export_amiga);
	if( es.export_mode == GfxAssetGroup::EXPORT_OFF )
		return;

	BinaryWordData bin;


	if( es.export_mode == GfxAssetGroup::EXPORT_BITPLANES )
	{
		string aname = "GFX_"+ name;
		int round_width = (tex->texture.width+15)/16*16;
		int stride = round_width/16*2;
		int num_bitplanes = es.num_bitplanes;
		int mask_bitplane = -1;
		if( asset_group->opt_masked )
			mask_bitplane = num_bitplanes++;

		DataBlock *db = ctx.dbs ? ctx.dbs->NewBlock(aname.c_str()) : NULL;

		if( es.write_header )
		{
			bin.AddWord((word)tex->texture.width);
			bin.AddWord((word)tex->texture.height);
			bin.AddWord((word)stride);
			bin.AddComment("width, height, stride");

			if( db )
			{
				db->AddWord((word)tex->texture.width);
				db->AddWord((word)tex->texture.height);
				db->AddWord((word)stride);
				db->AddComment("width, height, stride");
			}
		}
		bin.maxlen = stride/2;

		for( int plane=0; plane<num_bitplanes; plane++ )
		{
			if( plane==mask_bitplane )
			{
				bin.LineComment("mask");
				if( db ) db->AddLineComment("mask");
			}
			else
			{
				bin.LineComment(format("bitplane %d", plane));
				if( db ) db->AddLineCommentF("bitplane %d", plane);
			}

			for( int y=0; y<tex->texture.height; y++ )
			{
				for( int x0=0; x0<round_width; x0+=16 )
				{
					word value = 0;
					for( int x=0; x<16; x++ )
					{
						DWORD color = asset_group->opt_masked ? COLOR_TRANSPARENT : 0;
						if( x0+x<tex->texture.width )
							color = tex->texture(x0+x, y);

						if( plane==mask_bitplane )
							color = !COLOR_IS_TRANSPARENT(color);
						else
							color = (color>>plane) & 1;

						if( color )
							value |= 0x8000>>x;
					}
					bin.AddWord(value);
					if( db ) db->AddWord(value);
				}
				bin.NewLine();
				if( db ) db->AddLineBreak();
			}
		}

		ctx.info_table.push_back(aname);
		ctx.exported_bytes += bin.WriteTable(ctx.fp, aname.c_str(), es.chipmem);
	}

	if( es.export_mode == GfxAssetGroup::EXPORT_BITPLANES_INTERLEAVED )			// TBD: document
	{
		string aname = "GFX_" + name;
		int maskoffs = asset_group->opt_masked ? ((tex->texture.width+15)/16)*tex->texture.height*4 : 0;

		DataBlock *db = ctx.dbs ? ctx.dbs->NewBlock(aname.c_str()) : NULL;
		bin.Clear();

		if( es.write_header )
		{
			bin.AddWord((word)tex->texture.width);
			bin.AddWord((word)tex->texture.height);
			bin.AddWord((word)maskoffs);
			bin.AddComment("width, height, mask offset");
			if( db )
			{
				db->AddWord((word)tex->texture.width);
				db->AddWord((word)tex->texture.height);
				db->AddWord((word)maskoffs);
				db->AddComment("width, height, mask offset");
			}
		}
		bin.maxlen = (tex->texture.width+15)/16;

		for( int pass=0; pass<=(asset_group->opt_masked ? 1 : 0); pass++ )
		{
			if( pass )
			{
				bin.AddComment("Mask");
				if( db ) db->AddComment("Mask");
			}

			for( int y=0; y<tex->texture.height; y++ )
				for( int plane=0; plane<4; plane++ )
				{
					for( int x0=0; x0<tex->texture.width; x0+=16 )
					{
						word w = 0;
						for( int x=0; x<16 && x0+x<tex->texture.width; x++ )
						{
							DWORD col = tex->texture.data[(x0+x)+y*tex->texture.width];
							word bits = 0;
							if( pass==1 )
								bits = COLOR_IS_TRANSPARENT(col) ? 0 : 1;			// prepare graphics mask
							else if( !COLOR_IS_TRANSPARENT(col) )					// prepare graphics data
							{
								if( col&(0x0001<<plane) ) bits |= 1;
							}
							w |= bits<<(15-x);
						}
						bin.AddWord(w);
						if( db ) db->AddWord(w);
					}
					if( plane==0 )
					{
						bin.AddComment(format("%3d", y));
						if( db ) db->AddCommentF("%3d", y);
					}
				}
		}

		ctx.info_table.push_back(aname);
		ctx.exported_bytes += bin.WriteTable(ctx.fp, aname.c_str(), es.chipmem);
	}


	if( es.export_mode == GfxAssetGroup::EXPORT_BITPLANES_ST )
	{
		string aname = "GFX_" + name;
		int round_width = (tex->texture.width+15)/16*16;
		
		DataBlock *db = ctx.dbs ? ctx.dbs->NewBlock(aname.c_str()) : NULL;
		bin.Clear();

		if( es.write_header )
		{
			bin.AddWord((word)tex->texture.width);
			bin.AddWord((word)tex->texture.height);
			bin.AddComment("width, height");
			if( db )
			{
				db->AddWord((word)tex->texture.width);
				db->AddWord((word)tex->texture.height);
				db->AddComment("width, height");
			}
		}
		bin.maxlen = (asset_group->opt_masked ? 1 : 0) + es.num_bitplanes;

		for( int y=0; y<tex->texture.height; y++ )
		{
			for( int x0=0; x0<round_width; x0+=16 )
			{
				word mask = 0;
				word p1   = 0;
				word p2   = 0;
				word p3   = 0;
				word p4   = 0;
				for( int x=0; x<16; x++ )
				{
					DWORD color = asset_group->opt_masked ? COLOR_TRANSPARENT : 0;
					if( x0+x<tex->texture.width )
						color = tex->texture(x0+x, y);

					if( !COLOR_IS_TRANSPARENT(color) )
					{
						mask |= 0x8000>>x;
						if( color & 1 ) p1 |= 0x8000>>x;
						if( color & 2 ) p2 |= 0x8000>>x;
						if( color & 4 ) p3 |= 0x8000>>x;
						if( color & 8 ) p4 |= 0x8000>>x;
					}
				}
				if( asset_group->opt_masked )
					bin.AddWord(mask);
				if( es.num_bitplanes>=1 ) bin.AddWord(p1);
				if( es.num_bitplanes>=2 ) bin.AddWord(p2);
				if( es.num_bitplanes>=3 ) bin.AddWord(p3);
				if( es.num_bitplanes>=4 ) bin.AddWord(p4);
				bin.AddComment(format("%3d..%3d, %3d", x0, min(x0+15, tex->texture.width-1), y));
			}
			bin.NewLine();
		}

		ctx.info_table.push_back(aname);
		ctx.exported_bytes += bin.WriteTable(ctx.fp, aname.c_str(), false);
	}

	if( es.write_info )
	{
		// dword table 
		//	
		//	- start:		address of the main graphics
		//	- number:		0000ddd sssssssx xxxxxxxx yyyyyyyy
		//					<font_addr>
		//	- end:			NULL
		fprintf(ctx.fp, "\nconst dword GFXINFO_%s[] = {\n", name.c_str());
		fprintf(ctx.fp, "\t(dword)GFX_%s,\n", name.c_str());
		ctx.exported_bytes += 4;
		for( int i=0; i<overlays.size(); i++ )
		{
			const GfxAsset::Overlay &ov = overlays[i];
			if( ov.mode == GfxAsset::Overlay::M_FRAME  && ov.font.get()!=NULL )
			{
				DWORD code = (ov.font->opt_masked ? 0x10 : 0x11) << 24;
				code |= ov.xoffs << 8;
				code |= ov.yoffs;
				fprintf(ctx.fp, "\t0x%08X, (dword)GFXGROUP_%s, (dword)&e_globals.%s,\n", code, ov.font->name.c_str(), ov.var_name.c_str());
				ctx.exported_bytes += 8;
			}
			else if( ov.mode == GfxAsset::Overlay::M_NUMBER && ov.font.get()!=NULL && ov.num_digits>=1 && ov.num_digits<=3 )
			{
				DWORD code = ov.num_digits << 24;
				code |= ov.font_step << 17;
				code |= ov.xoffs << 8;
				code |= ov.yoffs;
				fprintf(ctx.fp, "\t0x%08X, (dword)GFXGROUP_%s, (dword)&e_globals.%s,\n", code, ov.font->name.c_str(), ov.var_name.c_str());
				ctx.exported_bytes += 8;
			}
		}
		fprintf(ctx.fp, "\t0\n");
		fprintf(ctx.fp, "};\n");
		ctx.exported_bytes += 4;
	}
}
