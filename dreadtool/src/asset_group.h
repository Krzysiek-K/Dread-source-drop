
#pragma once



// ================================================================ GfxAssetGroup ================================================================


class GfxAssetGroup : public AssetBase {
public:
	static const char TYPE_NAME[];


	enum colorkey_mode_t {
		COLORKEY_OFF	= 0,
		COLORKEY_MASKED,
		COLORKEY_AUTO,
	};

	enum resample_mode_t {
		RESAMPLE_OFF		= 0,
		RESAMPLE_DOWNSCALE_2,
		RESAMPLE_DOWNSCALE_2Y,
		RESAMPLE_COLLAPSE,
		RESAMPLE_LANCZOS,
	};

	enum pixelpack_mode_t {
		PIXELPACK_OFF		= 0,
		PIXELPACK_HD,				// packs pairs of pixels into halves		(half-transparent pixels become fully solid)
		PIXELPACK_HD_BLACK,			// packs pairs of pixels into halves		(half-transparent pixels are padded with black color)
		PIXELPACK_HD_KEEPHALF,		// packs pairs of pixels into halves		(half-transparent pixels are kept as they are)
		PIXELPACK_XD,				// shifts the texture left by 0.5 pixel		(transparent pixels stay transparent, transparent pixels don't shift in)
		PIXELPACK_XD_WRAP,			// rolls the texture left by 0.5 pixel		(transparent pixels stay transparent, transparent pixels don't shift in)
		PIXELPACK_XD_NARROW,		// shifts the texture left by 0.5 pixel		(half-transparent pixels become fully transparent)
		PIXELPACK_XD_WRAP_NARROW,	// rolls the texture left by 0.5 pixel		(half-transparent pixels become fully transparent)
	};

	enum export_mode_t {
		EXPORT_OFF						= 0,
		EXPORT_BITPLANES,
		EXPORT_BITPLANES_INTERLEAVED,
		EXPORT_BITPLANES_ST,
		EXPORT_FONT_MASK,
	};

	enum index_mode_t {
		INDEX_OFF			= 0,
		INDEX_ASSET_LIST,
	};

	struct LanczosParams {
		ivec2	pos_base = ivec2(0, 0);
		ivec2	pos_step = ivec2(1, 1);
		vec2	window_size = vec2(1, 1);
		vec2	kernel_size = vec2(1, 1);
	};

	struct LightLevel {
		int		light		= 255;				// 0..255
		vec3	gamma		= vec3(1, 1, 1);
		vec3	gain		= vec3(1, 1, 1);
		vec3	lift		= vec3(0, 0, 0);
		string	path_ext;
	};

	struct ExportSettings {
		export_mode_t	export_mode = EXPORT_OFF;

		bool			write_header = false;		// width, height, stride/maskoffs
		bool			chipmem = false;
		index_mode_t	write_index = INDEX_OFF;
		bool			write_info = false;
		int				num_bitplanes = 0;
	};


	bool					hidden = false;
	vector<AssetBase*>		assets;
	string					src_path;
	GfxAssetGroup			*parent = NULL;

	// graphics processing options
	GfxPalette			*opt_palette = NULL;
	bool				opt_masked = false;

	colorkey_mode_t		opt_colorkey_mode = COLORKEY_OFF;			// Color key
	int					opt_colorkey = 0;
	int					opt_colormask = 0;
	int					opt_scroll_x = 0;							// Scroll
	int					opt_scroll_y = 0;
	resample_mode_t		opt_resample_mode = RESAMPLE_OFF;			// Resample (downscale)
	LanczosParams		opt_lanczos;
	vector<LightLevel>	light_levels;								// Adjust
	vec3				color_gamma = vec3(1, 1, 1);
	vec3				color_gain = vec3(1, 1, 1);
	vec3				color_lift = vec3(0, 0, 0);
	// Dither
	vector<int>			opt_color_remap;
	pixelpack_mode_t	opt_pixel_pack = PIXELPACK_OFF;				// Pixel process - horizontal packing
	bool				opt_trim = false;							// Trim

	asset_usage_t		opt_usage = ASSET_USAGE_TEXTURE;
	bool				opt_verbose = false;
	float				opt_visualscale_x = 1.0f;
	float				opt_visualscale_y = 1.0f;

	// audio processing options
	float				opt_audio_resample = 0.f;
	float				opt_audio_gain = 1.f;

	// export options
	ExportSettings		export_amiga;
	ExportSettings		export_st;


	GfxAssetGroup() {}
	GfxAssetGroup(const GfxAssetGroup &ag) {
		name					= ag.name;
		hidden					= ag.hidden;
		src_path				= ag.src_path;
		parent					= ag.parent;

		opt_palette				= ag.opt_palette;
		opt_masked				= ag.opt_masked;

		opt_colorkey_mode		= ag.opt_colorkey_mode;
		opt_colorkey			= ag.opt_colorkey;
		opt_colormask			= ag.opt_colormask;
		opt_scroll_x			= ag.opt_scroll_x;
		opt_scroll_y			= ag.opt_scroll_y;
		opt_resample_mode 		= ag.opt_resample_mode;
		opt_lanczos				= ag.opt_lanczos;
		light_levels			= ag.light_levels;
		color_gamma				= ag.color_gamma;
		color_gain 				= ag.color_gain;
		color_lift 				= ag.color_lift;

		opt_color_remap			= ag.opt_color_remap;
		opt_pixel_pack			= ag.opt_pixel_pack;
		opt_trim 				= ag.opt_trim;

		opt_usage 				= ag.opt_usage;
		opt_verbose 			= ag.opt_verbose;
		opt_visualscale_x 		= ag.opt_visualscale_x;
		opt_visualscale_y 		= ag.opt_visualscale_y;

		opt_audio_resample		= ag.opt_audio_resample;
		opt_audio_gain			= ag.opt_audio_gain;

		export_amiga			= ag.export_amiga;
		export_st				= ag.export_st;

	}

	virtual ~GfxAssetGroup() { Clear(); }
	virtual const char *GetAssetClassName() { return TYPE_NAME; }


	void Clear();
	virtual bool ImportAssets();
	void Load(DefNode *node, GfxConverter *owner, GfxAssetGroup *parent, bool ignore_arg_count, bool overlay_mode, GfxAsset *gfxowner);
	DWORD GetColor();
	bool IsHidden() { return hidden && assets.size()<=1; }

	virtual void ExportGraphics(AssetExportContext &ctx);

};
