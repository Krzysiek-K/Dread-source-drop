
#pragma once




// ================================================================ GfxAsset ================================================================


// Effects of asset usage:
//	- preview zoom
//	- position (0,0) in the middle of the screen and draw marker
//	- color on the item list
//

enum asset_usage_t {
	ASSET_USAGE_TEXTURE = 0,
	ASSET_USAGE_FLAT,
	ASSET_USAGE_OBJECT,
	ASSET_USAGE_WEAPON,
	ASSET_USAGE_WEAPON_SPRITE,
	ASSET_USAGE_GRAPHICS,
	ASSET_USAGE_SKY,
};


class GfxAsset : public AssetBase {
public:
	struct Overlay {
		enum mode_t {
			M_STATIC	= 0,
			M_ICON,
			M_FRAME,
			M_NUMBER,
			M_PART,
		};

		mode_t						mode = M_STATIC;
		weak_ptr<GfxAsset>			gfx;
		weak_ptr<GfxAssetGroup>		font;
		int							xoffs = 0;
		int							yoffs = 0;
		int							font_step = 0;
		int							num_max = 0;
		int							num_digits = 1;
		string						var_name;
	};

	static const char TYPE_NAME[];

	int					xoffs = 0;
	int					yoffs = 0;
	vector<Overlay>		overlays;
	bool				is_multipart_gfx = false;

	vector<GfxTexture*>	textures;


	virtual ~GfxAsset() { Clear(); }
	virtual const char *GetAssetClassName() { return TYPE_NAME; }


	void Clear() {
		for( int i=0; i<(int)textures.size(); i++ )
			delete textures[i];
		textures.clear();
	}

	virtual bool ImportAssets();

	void Load(DefNode *node, GfxAssetGroup *aowner, GfxConverter *owner);
	void LoadOption(DefNode *n);

	void SavePreview(const char *path, bool save_light_levels);
	void ExportPK3Texture(const char *path, int fudge_border);
	void ExportPK3Sprite(const char *path);
		
	virtual void ExportGraphics(AssetExportContext &ctx);
};
