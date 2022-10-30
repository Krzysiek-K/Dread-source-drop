

#pragma once



// ================================================================ GfxAnimation ================================================================


class GfxAnimation : public AssetBase {
public:
	struct Frame {
		GfxAsset	*asset = NULL;		// not owned
		float		delay = 1.0f;
		int			add_off_x = 0;
		int			add_off_y = 0;
		bool		walkswing = false;
		string		play_sound;
	};

	static const char TYPE_NAME[];


	vector<Frame>	frames;
	int				loop_pos = 0;


	virtual const char *GetAssetClassName() { return TYPE_NAME; }

	void Load(DefNode *node, GfxAssetGroup *aowner, GfxConverter *owner);

	bool SavePreviewEmulation(FILE *fp);
};

