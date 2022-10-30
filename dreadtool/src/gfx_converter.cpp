
#include "common.h"
#include "app_rendertest.h"



GfxConverter gfx_converter;


// ================================================================ Helper Functions ================================================================


static float sinc_pi(float x)
{
	if( x==0 ) return 1.f;
	x *= M_PI;
	return sin(x)/x;
}





// ================================================================ GfxLanczosResampler ================================================================




void GfxLanczosResampler::Process(DataTexture &tex)
{
	// prepare the kernel
	kernel_w = (int(ceil(window_size.x))-1)*2 + 1;
	kernel_h = (int(ceil(window_size.y))-1)*2 + 1;

	kernel_weights.resize(kernel_w*kernel_h);

	double ksum = 0;
	int d = 0;
	for( int y=-kernel_h/2; y<=kernel_h/2; y++ )
		for( int x=-kernel_w/2; x<=kernel_w/2; x++ )
		{
			float w = 1.f;
			if( kernel_size.x > 0 && window_size.x > 0 )	w *= sinc_pi(x/kernel_size.x) * sinc_pi(x/window_size.x);
			if( kernel_size.y > 0 && window_size.y > 0 )	w *= sinc_pi(y/kernel_size.y) * sinc_pi(y/window_size.y);

			kernel_weights[d++] = w;
			ksum += w;
		}

	// normalize
	for( int i=0; i<(int)kernel_weights.size(); i++ )
		kernel_weights[i] /= ksum;

	// load texture & convert to colorspace
	src_w = tex.width;
	src_h = tex.height;
	src_data.resize(src_w*src_h);
	for( int i=0; i<src_data.size(); i++ )
	{
		if( colorspace )
			*(vec3*)&src_data[i] = colorspace->Convert(vec3::from_rgb(tex.data[i]));
		else
			*(vec3*)&src_data[i] = vec3::from_rgb(tex.data[i]);

		src_data[i].w = (tex.data[i]>>24)/255.f;
		*(vec3*)&src_data[i] *= src_data[i].w;
	}

	// extents
	int x1 = -(pos_base.x / pos_step.x);
	int y1 = -(pos_base.y / pos_step.y);
	int x2 = (tex.width-pos_base.x + pos_step.x-1) / pos_step.x;
	int y2 = (tex.height-pos_base.y + pos_step.y-1) / pos_step.y;
	if( x2<x1 ) x2 = x1;
	if( y2<y1 ) y2 = y1;

	int new_w = x2 - x1;
	int new_h = y2 - y1;

	// process
	tex.data.resize(new_w*new_h);
	for( int y=0; y<new_h; y++ )
		for( int x=0; x<new_w; x++ )
		{
			int x0 = pos_base.x + (x+x1)*pos_step.x;
			int y0 = pos_base.y + (y+y1)*pos_step.y;
			vec4 acc = vec4(0, 0, 0, 0);

			int ki = 0;
			for( int ky=0; ky<kernel_h; ky++ )
				for( int kx=0; kx<kernel_w; kx++, ki++ )
				{
					int xs = x0 + (kx-kernel_w/2);
					int ys = y0 + (ky-kernel_h/2);

					if( uint32_t(xs) < uint32_t(src_w) && uint32_t(ys) < uint32_t(src_h) )
						acc += src_data[xs+ys*src_w] * kernel_weights[ki];
				}

			if( colorspace )
				*(vec3*)&acc = colorspace->Deconvert(*(vec3*)&acc);

			if( acc.x<0 ) acc.x = 0;
			if( acc.y<0 ) acc.y = 0;
			if( acc.z<0 ) acc.z = 0;
			if( acc.w<0 ) acc.w = 0;
			if( acc.x>1 ) acc.x = 1;
			if( acc.y>1 ) acc.y = 1;
			if( acc.z>1 ) acc.z = 1;
			if( acc.w>1 ) acc.w = 1;

			if( acc.w>0 )
			{
				acc.x /= acc.w;
				acc.y /= acc.w;
				acc.z /= acc.w;
			}

			tex.data[x+new_w*y] = ((vec3*)&acc)->make_rgba(acc.w);
		}
	tex.width = new_w;
	tex.height = new_h;
	tex.off_x = -x1;			// TBD: include original offset
	tex.off_y = -y1;
}




// ================================================================ GfxTexture ================================================================




bool GfxTexture::LoadTexture(const char *_name, const char *path)
{
	name = _name;

	if( !texture.Load(path) )
		return false;
	
	orig_w = texture.width;
	orig_h = texture.height;
	orig_ox = texture.off_x;
	orig_oy = texture.off_y;

	return true;
}

void GfxTexture::ApplyColorCorrections(const vec3 &gamma, const vec3 &gain, const vec3 &lift, float master_brightness)
{
	// Apply brightness
	for( int j=0; j<texture.data.size(); j++ )
	{
		vec3 color = vec3::from_rgb(texture.data[j]);
		float a = (texture.data[j]>>24)/255.f;
		color.x = pow(color.x, gamma.x);
		color.y = pow(color.y, gamma.y);
		color.z = pow(color.z, gamma.z);
		color *= gain;
		color += lift;
		color *= master_brightness;
		color.saturate();
		texture.data[j] = color.make_rgba(a);
	}
}






// ================================================================ GfxConverter ================================================================


GfxConverter::GfxConverter()
{
}

GfxConverter::~GfxConverter()
{
	ClearAssets(false);
}


void GfxConverter::Init()
{
}

void GfxConverter::ClearAssets(bool init)
{
	// clear audio channels
	ami_paula.RemoveAllChannels(true);

	// remove assets
	delete root_group;
	root_group = NULL;

	if( init )
		root_group = new GfxAssetGroup();

	asset_index.clear();
	asset_list.clear();
	texture_remap.clear();
	sel_asset = NULL;
	sel_anim_frame = 0;
	sel_anim_delay = 0.f;

	// delete palettes
	for( int i=0; i<palette_pool.size(); i++ )
		delete palette_pool[i];
	palette_pool.clear();

	main_palette = NULL;
	hud_palette = NULL;

	// reset markers
	asset_error_mute.clear();
	asset_error_mute[""] = true;
	asset_error_mute["-"] = true;

	// clear errors
	elog.RemoveMessagesOf(NULL);
	elog.RemoveMessagesOf(this);

	// clear info
	watched_files.clear();

	for( int i=0; i<node_roots.size(); i++ )
		delete node_roots[i];
	node_roots.clear();

	script_files.clear();
}




texture_id_t GfxConverter::GetTextureId(const char *name, int light_level)
{
	if( !*name || strcmp(name, "-")==0 || strcmp(name, "F_SKY1")==0 || name[0]=='$' )
		return texture_id_t();

	TextureIdentifier key;
	key.name = name;
	key.light_level = light_level;

	texture_id_t tid;
	auto p = texid_dict.find(key);
	if( p!=texid_dict.end() )
		tid._internal_index = p->second;
	else
	{
		tid._internal_index = (int)texid_list.size();
		texid_list.push_back(key);
		texid_dict[key] = tid._internal_index;
	}
	return tid;
}

void GfxConverter::AddAsset(AssetBase *asset)
{
	if( asset_index.find(asset->name) != asset_index.end() )
		asset->Error(format("Asset name '%s' used more than once", asset->name.c_str()));

	asset_index[asset->name] = asset;
	asset_list.push_back(asset);
}

GfxPalette *GfxConverter::FindPalette(const char *name)
{
	for( int i=0; i<(int)palette_pool.size(); i++ )
		if( palette_pool[i]->name==name || palette_pool[i]->name_alias==name )
			return palette_pool[i];
	return NULL;
}

GfxTexture *GfxConverter::FindTexture(const char *name, int light_level, GfxAsset **out_asset)
{
	auto remap = texture_remap.find(name);
	if( remap!=texture_remap.end() )
		name = remap->second.c_str();

	GfxAsset *asset = FindGfxAsset(name, true);
	if( out_asset )
		*out_asset = NULL;

	if( !asset || asset->textures.size()<=0 )
		return NULL;

	//if( !asset || asset->textures.size()<=0 )
	//{
	//	// Find flat instead
	//	GfxFlatDesc *flat = FindAsset<GfxFlatDesc>(name, false);
	//	if( !flat ) return NULL;

	//	GfxFlatDesc::LightLevel *level = flat->FindBestLightLevel(light_level);
	//	if( !light_level ) return NULL;

	//	return &level->texture;
	//}

	if( out_asset )
		*out_asset = asset;

	if( light_level<0 )
		light_level = 255;

	GfxTexture *tex = asset->textures[0];
	int best_err = 1000000;
	for( int i=0; i<(int)asset->textures.size(); i++ )
	{
		int err = asset->textures[i]->light_level - light_level;
		if( err<0 ) err = -err;
		if( err<best_err )
		{
			tex = asset->textures[i];
			best_err = err;
		}
	}
	return tex;
}

GfxTexture *GfxConverter::FindTexture(texture_id_t texindex, GfxAsset **out_asset)
{
	if( texindex._internal_index<0 || texindex._internal_index>=texid_list.size() )
		return NULL;
	TextureIdentifier &tid = texid_list[texindex._internal_index];
	return FindTexture(tid.name.c_str(), tid.light_level, out_asset);
}

GfxTexture *GfxConverter::FindFlatTexture(const char *name, int light_level, bool mute_errors)
{
	// Find flat
	GfxFlatDesc *flat = FindAsset<GfxFlatDesc>(name, mute_errors);
	if( !flat ) return NULL;

	GfxFlatDesc::LightLevel *level = flat->FindBestLightLevel(light_level);
	if( !light_level ) return NULL;

	return &level->texture;
}

GfxTexture *GfxConverter::FindFlatTexture(texture_id_t texindex)
{
	if( texindex._internal_index<0 || texindex._internal_index>=texid_list.size() )
		return NULL;
	TextureIdentifier &tid = texid_list[texindex._internal_index];
	return FindFlatTexture(tid.name.c_str(), tid.light_level);
}

GfxTexture *GfxConverter::FindMapTexture(texture_id_t texindex)
{
	if( texindex._internal_index<0 || texindex._internal_index>=texid_list.size() )
		return NULL;
	TextureIdentifier &tid = texid_list[texindex._internal_index];
	GfxTexture *flat = FindFlatTexture(tid.name.c_str(), tid.light_level, true);
	if( flat )
		return flat;
	return FindTexture(tid.name.c_str(), tid.light_level);
}



const char *GfxConverter::GetTextureName(texture_id_t tid)
{
	if( !tid ) return "-";
	const char *name = texid_list[tid._internal_index].name.c_str();

	auto remap = texture_remap.find(name);
	if( remap!=texture_remap.end() )
		name = remap->second.c_str();
	return name;
}

string GfxConverter::GetTextureNameEx(texture_id_t tid)
{
	if( !tid ) return "-";
	const TextureIdentifier &tx = texid_list[tid._internal_index];
	
	return format("%s:%d", tx.name.c_str(), tx.light_level);
}

string GfxConverter::GetTextureOffsetName(texture_id_t tid)
{
	if( !tid ) return "0";

	//if( texid_list[tid._internal_index].name == "F_SKY1" )
	//	return "0";

	GfxTexture *tex = FindTexture(tid);
	if( !tex ) return format("___MISSING_TEXTURE_%s___", texid_list[tid._internal_index].name.c_str());

	return tex->GetOffsetName();
}

DWORD GfxConverter::GetTextureColor(texture_id_t tid)
{
#if 1
	if( tid._internal_index<0 || tid._internal_index>=texid_list.size() )
		return COLOR_TRANSPARENT;		// flat not loaded, return transparent
	TextureIdentifier &texid = texid_list[tid._internal_index];

	const char *name = GetTextureName(tid);
	auto remap = texture_remap.find(name);
	if( remap!=texture_remap.end() )
		name = remap->second.c_str();

	GfxFlatDesc *flat = FindAsset<GfxFlatDesc>(name, false);
	if( !flat ) return COLOR_TRANSPARENT;		// flat not loaded, return transparent

	GfxFlatDesc::LightLevel *level = flat->FindBestLightLevel(texid.light_level);
	if( !level ) return COLOR_TRANSPARENT;		// flat not loaded, return transparent

	return level->pixel;
#else
	GfxTexture *tex = FindTexture(tid);
	if( !tex || tex->texture.data.size()<=0 )
		return COLOR_TRANSPARENT;		// texture not loaded, return transparent
	return tex->texture.data[0];
#endif
}


int GfxConverter::ExportGraphics(FILE *fp, platform_t platform)
{
	AssetExportContext ctx;
	ctx.fp = fp;
	ctx.platform = platform;

	for( int i=0; i<(int)palette_pool.size(); i++ )
		palette_pool[i]->ExportGraphics(ctx);
	
	if( root_group )
		root_group->ExportGraphics(ctx);

	return ctx.exported_bytes;
}

void GfxConverter::ExportSounds(FILE *fp, ScriptCompiler &scomp, platform_t platform)
{
	// Export sounds
	int size = 0;
	map<SoundAsset*, bool> export_done;

	for( int j=0; j<(int)scomp.idents.size(); j++ )
		if( scomp.idents[j].is_sound )
		{
			const char *name = scomp.idents[j].name.c_str();

			SoundAsset *snd = FindSound(name, -1);
			if( !snd ) continue;

			export_done[snd] = true;

			for( int i=0; i<snd->tracker_data.size(); i++ )
			{
				SoundAsset *sample = snd->tracker_data[i].sample;
				if( sample && !export_done[sample] )
				{
					export_done[sample] = true;
					size += sample->ExportSound(fp, platform);
				}
			}


			size += snd->ExportSound(fp, platform);

		}

	printf("Export: %d sound bytes.\n", size);
}

void GfxConverter::SaveAllGfxPreviews(const char *path)
{
	ami_paula.RemoveAllChannels(true);

	for( AssetBase *a : asset_list )
	{
		GfxAsset *gfx = dynamic_cast<GfxAsset*>(a);
		if( !gfx ) continue;

		gfx->SavePreview((string(path)+"/%s.png").c_str(), true);
	}

	for( AssetBase *a : asset_list )
	{
		SoundAsset *sound = dynamic_cast<SoundAsset*>(a);
		if( !sound) continue;

		if( sound->is_tracked_sample )
			sound->SavePreview((string(path)+"/%s.wav").c_str());
	}

	FILE *fp = fopen(format("%s/Animations.nut", path).c_str(), "wt");
	if(fp)
	{
		fprintf(fp, "\nclass AnimationBank {\n");
		for( AssetBase *a : asset_list )
		{
			GfxAnimation *anim = dynamic_cast<GfxAnimation*>(a);
			if( !anim ) continue;

			anim->SavePreviewEmulation(fp);
		}
		fprintf(fp, "\n}\n");

		fclose(fp);
	}
}

void GfxConverter::BuildPK3Structure(const char *path)
{
	// Build structure / remove existing files
	CreateDirectory(path, NULL);

	const char *DIRS[] = {
		"%s\\flats",
		"%s\\textures",
		"%s\\sprites",
		NULL
	};

	for( int d=0; DIRS[d]; d++ )
	{
		string dir = format(DIRS[d], path);
		CreateDirectory(dir.c_str(), NULL);
		
		vector<string> files;
		if( NFS.GetFileList((dir+"\\*.*").c_str(), files) )
		{
			for( int i=0; i<files.size(); i++ )
			{
				//printf("File: %s\\%s\n", dir.c_str(), files[i].c_str());
				DeleteFile((dir+"\\"+files[i]).c_str());
			}
		}
	}

	// Export "empty" texture  -  Doom Builder 2 needs it, or otherwise first texture won't show in previews
	{
		//NFS.WriteFileLines(format("%s/textures/-.png", path).c_str(), vector<string>());
		DevTexture dtex;
		vector<DWORD> texdata(64*64, 0xFFFF00FF);
		dtex.LoadRaw2D(D3DFMT_X8R8G8B8, 64, 64, &texdata[0], false);

		D3DXSaveTextureToFile(format("%s/textures/-.png", path).c_str(), D3DXIFF_PNG, dtex, NULL);
	}

	// Export PK3 assets
	for( AssetBase *a : asset_list )
	{
		GfxAsset *gfx = dynamic_cast<GfxAsset*>(a);
		if( gfx && gfx->asset_group )
		{
			if( gfx->asset_group->opt_usage == ASSET_USAGE_TEXTURE )
				gfx->ExportPK3Texture((string(path)+"/textures/%s.png").c_str(), FUDGE_BORDER);

			if( gfx->asset_group->opt_usage == ASSET_USAGE_FLAT )
				gfx->SavePreview((string(path)+"/flats/%s.png").c_str(), false);

			if( gfx->asset_group->opt_usage == ASSET_USAGE_OBJECT )
			{
				ScriptCompiler::Ident *id = app_rendertest.script_compiler.GetIdent(gfx->name.c_str());
				if( id && id->is_editor_frame )
					gfx->ExportPK3Sprite((string(path)+"/sprites/%s.png").c_str());
			}

			if( gfx->asset_group->opt_usage == ASSET_USAGE_SKY )
				gfx->ExportPK3Texture((string(path)+"/flats/F_SKY1.png").c_str(), 0);
		}

		GfxFlatDesc *flat = dynamic_cast<GfxFlatDesc*>(a);
		if( flat )
			flat->ExportPK3Flat((string(path)+"/flats/%s.png").c_str());
	}
}




void GfxConverter::WatchFile(const char *path)
{
	File f;
	f.path = path;
	f.time = GetFileTime(path);
	if( f.time>0 )
	{
		for( int i=0; i<watched_files.size(); i++ )
			if( watched_files[i].path == f.path && !watched_files[i].owner_asset )
				return;
		watched_files.push_back(f);
	}
}

void GfxConverter::WatchFile(const char *path, AssetBase *owner)
{
	File f;
	f.path = path;
	f.time = GetFileTime(path);
	f.owner_asset = owner;
	if( f.time>0 )
	{
		for( int i=0; i<watched_files.size(); i++ )
			if( watched_files[i].path == f.path && watched_files[i].owner_asset == owner )
				return;
		watched_files.push_back(f);
	}
}

bool GfxConverter::WatchAssetFiles(bool force_reload)
{
	bool reload = (watched_files.size()<=0) || force_reload;

	if( watched_files_suppress > 0 )
		watched_files_suppress -= Dev.GetTimeDelta();

	for( int i=0; i<(int)watched_files.size(); i++ )
	{
		unsigned long long ftime = GetFileTime(watched_files[i].path.c_str());
		if( watched_files[i].time < ftime )
		{
			if( watched_files[i].owner_asset )
			{
				if( watched_files_suppress <= 0 )
				{
					if( watched_files[i].owner_asset->ImportAssets() )
					{
						watched_files[i].time = ftime;
					}
					else
					{
						watched_files_suppress = 0.25f;
					}
				}
			}
			else
			{
				reload = true;
				break;
			}
		}
	}

	if( !reload )
		return false;

	printf("Reloading assets...\n");

	// clear assets
	ClearAssets(true);


	// ================================ Load ================================

	DefParser dp;

	try {

		// -------- Parse files --------

		dp.Reset();
		dp.LoadFile(proj_cfg.GetString("assets-definition", "assets.txt").c_str());

		for( int i=0; i<(int)modmgr.mod_dir_list.size(); i++ )
			if( modmgr.mod_dir_list[i].enabled )
				dp.AppendModFile((modmgr.mod_dir_list[i].path+"/extra.txt").c_str());


		// -------- Filter versions --------
		
		map<string, bool> new_versions;
		dp.root->GatherVersions(new_versions);
		for( auto &p : new_versions )
			p.second = sel_versions[p.first];
		sel_versions.swap(new_versions);
		dp.root->FilterVersions(new_versions);

		
		// -------- Create assets --------

		for( DefNode *n : dp.root->subnodes )
		{
			n->type_name = "asset definition";

			try
			{
				if( n->name=="palette" )
				{
					GfxPalette *pal = new GfxPalette();
					palette_pool.push_back(pal);
					n->RequireArgs(1);
					pal->Load(n, this, n->args[0]);
				}
				else if( n->name=="gfxgroup" || n->name=="group" )			// TBD: document
				{
					GfxAssetGroup *ag = new GfxAssetGroup();
					root_group->assets.push_back(ag);
					ag->Load(n, this, root_group, false, false, NULL);
				}
				else if( n->name=="script" )
				{
					n->RequireArgs(1);
					n->ForbidSubnodes();
					script_files.push_back(modmgr.GetResourcePath(n->args[0]));
				}
				else
					n->Invalid();
			}
			catch( const string &err )
			{
				elog.Error(this, err);
			}
		}

		
		// -------- Load assets data --------
		root_group->ImportAssets();

	}
	catch( const string &err )
	{
		elog.Error(this, err);
		return false;
	}

	for( int i=0; i<(int)dp.filenames.size(); i++ )
		WatchFile(dp.filenames[i].c_str());
		
	printf("  Asset reload done.\n");

	return true;
}


void GfxConverter::MenuAssetsListRec(GfxAssetGroup *group, const string &indent)
{
	string newindent = indent;

	if( group!=root_group && !group->IsHidden() )
	{
		bool open = open_asset_groups[group->full_name];

		if( ms.ListItem((indent+(open ? "- " : "+ ")+group->name).c_str(), group->GetColor(), false) )
		{
			open = !open;
			open_asset_groups[group->full_name] = open;
		}

		if( !open )
			return;

		newindent += "     ";
	}

	for( AssetBase *a : group->assets )
	{
		GfxAssetGroup *subgroup = dynamic_cast<GfxAssetGroup*>(a);

		if( subgroup )
			MenuAssetsListRec(subgroup, newindent);
		else if( ms.ListItem((newindent + a->display_name).c_str(), group->GetColor(), (a==sel_asset)) )
		{
			// asset clicked
			sel_asset = a;
			sel_asset_name = sel_asset->name;
			sel_anim_delay = 0;
			sel_anim_frame = 1000000;

			sel_asset->OnClick();
		}
	}

}

void GfxConverter::MenuVersionSelector()
{
	if( sel_versions.size()>0 )
	{
		static vector<int> sizes;
		if( sizes.size()!=sel_versions.size()+1 )
		{
			sizes.clear();
			sizes.resize(sel_versions.size(), -1);
			sizes.push_back(0);
		}
		ms.LineSetup(&sizes[0]);
		ms.Group();

		bool mod = false;
		for( auto &p : sel_versions )
		{
			if( ms.Button(NULL, p.first.c_str(), (p.second ? 1 : 0)) )
			{
				//if( !Dev.GetKeyState(VK_CONTROL) )
				//	for( auto &q : sel_versions )
				//		if( p.first!=q.first )
				//			q.second = false;
				p.second = !p.second;
				mod = true;
			}
		}

		if( mod )
			WatchAssetFiles(true);
	}
}

void GfxConverter::MenuAssetsList()
{
	ms.ListStart();
	MenuAssetsListRec(root_group,"");
	ms.ListEnd();
}
