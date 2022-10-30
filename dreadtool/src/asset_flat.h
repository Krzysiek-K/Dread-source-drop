
#pragma once




// ================================================================ GfxFlatDesc ================================================================


class GfxFlatDesc : public AssetBase {
public:
	struct LightLevel {
		int			level = 255;
		DWORD		pixel =   0;

		GfxTexture	texture;

		void GenerateTexture();
	};

	static const char TYPE_NAME[];

	vector<LightLevel*>	levels;



	virtual ~GfxFlatDesc() { Clear(); }
	virtual const char *GetAssetClassName() { return TYPE_NAME; }

	void Clear() {
		for( int i=0; i<(int)levels.size(); i++ )
			delete levels[i];
		levels.clear();
	}

	virtual bool ImportAssets();

	void Load(DefNode *node, GfxAssetGroup *aowner, GfxConverter *owner);
	LightLevel *FindBestLightLevel(int light_level);

	void ExportPK3Flat(const char *path);
};
