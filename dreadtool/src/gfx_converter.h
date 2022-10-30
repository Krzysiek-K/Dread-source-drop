
#pragma once



class SoundAsset;
class ScriptCompiler;





// ================================================================ GfxLanczosResampler ================================================================

class GfxLanczosResampler {
public:
	// parameters
	GfxColorSpace		*colorspace;
	ivec2				pos_base;
	ivec2				pos_step;
	vec2				kernel_size;		// should be equal to pos_step for best results
	vec2				window_size;		// Lanczos3:	window_size = kernel_size * 3


	GfxLanczosResampler() : colorspace(NULL), pos_base(0,0), pos_step(1,1), kernel_size(1,1), window_size(1,1) {}

	void Process(DataTexture &tex);

protected:
	// internal
	int				kernel_w;	// kernel sizes are always odd, centered on pixel in question
	int				kernel_h;
	vector<float>	kernel_weights;

	int				src_w;
	int				src_h;
	vector<vec4>	src_data;


};


// ================================================================ GfxTexture ================================================================


class GfxTexture {
public:
	string			name;
	int				light_level;
	DataTexture		texture;
	int				orig_w;
	int				orig_h;
	int				orig_ox;
	int				orig_oy;

	string GetOffsetName() {
		if( light_level==255 ) return format("TEX_%s_OFFS", name.c_str());
		if( light_level==128 ) return format("TEX_%s_DARK_OFFS", name.c_str());
		return format("TEX_%s__LL%d_OFFS", name.c_str(), light_level);
	}

	bool LoadTexture(const char *name, const char *path);
	void ApplyColorCorrections(const vec3 &gamma, const vec3 &gain, const vec3 &lift, float master_brightness);
};




// ================================================================ GfxConverter ================================================================

struct texture_id_t {
	int _internal_index = -1;

	void clear() {
		_internal_index = -1;
	}
	
	bool operator ==(const texture_id_t &t) const {
		return _internal_index == t._internal_index;
	}

	bool operator <(const texture_id_t &t) const {
		return _internal_index < t._internal_index;
	}

	bool operator !() const {
		return _internal_index < 0;
	}

	bool operator &&(bool arg2) const {
		return is_set() && arg2;
	}

	bool is_set() const {
		return _internal_index >= 0;
	}
};



class GfxConverter : public ToolObject {
public:
	enum {
		FUDGE_BORDER	= 16
	};

	struct TextureIdentifier {
		string	name;
		int		light_level = 255;

		bool operator <(const TextureIdentifier &t) const {
			if( name!=t.name ) return name < t.name;
			return light_level < t.light_level;
		}
	};

	struct File {
		string				path;
		unsigned long long	time;
		AssetBase			*owner_asset = NULL;
	};

	GfxPalette					*main_palette = NULL;
	GfxPalette					*hud_palette = NULL;

	vector<File>				watched_files;
	float						watched_files_suppress = 0;
	vector<DefNode*>			node_roots;

	GfxAssetGroup				*root_group = NULL;
	vector<GfxPalette*>			palette_pool;
	map<string, AssetBase*>		asset_index;
	vector<AssetBase*>			asset_list;
	map<string, string>			texture_remap;
	vector<string>				script_files;
	map<string, bool>			open_asset_groups;
	AssetBase					*sel_asset = NULL;
	string						sel_asset_name;
	int							sel_anim_frame = 0;
	float						sel_anim_delay = 0.f;
	map<string, bool>			sel_versions;
	map<string, bool>			asset_error_mute;

	// texture id dictionary
	vector<TextureIdentifier>	texid_list;			// id once assigned never changes
	map<TextureIdentifier, int>	texid_dict;



	GfxConverter();
	virtual ~GfxConverter();

	void Init();
	void ClearAssets(bool init);


	texture_id_t GetTextureId(const char *name, int light_level);


	template<class T>
	T				*FindAsset(const char *name, bool mute_errors);

	void			AddAsset(AssetBase *asset);

	GfxPalette		*FindPalette(const char *name);
	GfxAsset		*FindGfxAsset(const char *name, bool mute_errors)								{ return FindAsset<GfxAsset>(name, mute_errors);		}
	GfxAnimation	*FindAnimation(const char *name, bool mute_errors)								{ return FindAsset<GfxAnimation>(name, mute_errors);	}
	SoundAsset		*FindSound(const char *name, bool mute_errors)									{ return FindAsset<SoundAsset>(name, mute_errors);		}
	GfxTexture		*FindTexture(const char *name, int light_level, GfxAsset **out_asset=NULL);
	GfxTexture		*FindTexture(texture_id_t texindex, GfxAsset **out_asset=NULL);
	GfxTexture		*FindFlatTexture(const char *name, int light_level, bool mute_errors=false);
	GfxTexture		*FindFlatTexture(texture_id_t texindex);
	GfxTexture		*FindMapTexture(texture_id_t texindex);

	const char	*GetTextureName(texture_id_t tid);
	string		GetTextureNameEx(texture_id_t tid);
	string		GetTextureOffsetName(texture_id_t tid);
	DWORD		GetTextureColor(texture_id_t tid);

	int			ExportGraphics(FILE *fp, platform_t platform);
	void		ExportSounds(FILE *fp, ScriptCompiler &scomp, platform_t platform);
	void		SaveAllGfxPreviews(const char *path);
	void		BuildPK3Structure(const char *path);

	void		WatchFile(const char *path);
	void		WatchFile(const char *path, AssetBase *owner);
	
	bool WatchAssetFiles(bool force_reload=false);
	void MenuVersionSelector();
	void MenuAssetsList();


protected:

	void MenuAssetsListRec(GfxAssetGroup *group, const string &indent);

};


extern GfxConverter gfx_converter;




template<class T>
T *GfxConverter::FindAsset(const char *name, bool mute_errors)
{
	auto p = asset_index.find(name);
	if( p==asset_index.end() )
	{
		if( !asset_error_mute[name] && !mute_errors )
		{
			elog.Error(this, format("Missing %s '%s'", T::TYPE_NAME, name));
			asset_error_mute[name] = true;
		}
		return NULL;
	}

	T *typed = dynamic_cast<T*>(p->second);
	if( !typed )
	{
		if( !asset_error_mute[name] && !mute_errors )
		{
			elog.Error(this, format("Missing %s '%s' (found %s with this name)", T::TYPE_NAME, name, p->second->GetAssetClassName()));
			asset_error_mute[name] = true;
		}
	}

	return typed;
}
