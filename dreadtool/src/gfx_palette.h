
#pragma once


class GfxConverter;





// ================================================================ Macros ================================================================

#define COLOR_IS_TRANSPARENT(c)			((c)==0xFFFF)
#define COLOR_TRANSPARENT				(0xFFFF)
#define COLOR_IS_TRANSPARENT_HIGH(c)	(((c)&0xFF00)==0xFF00)
#define COLOR_IS_TRANSPARENT_LOW(c)		(((c)&0x00FF)==0x00FF)



// ================================================================ ChunkyColorPacker ================================================================


class ChunkyColorPacker {
public:
	enum mode_t {
		MODE_AMIGA	= 0,
		MODE_ST_GATHER,			// gathers stats & returns just the left half
		MODE_ST_APPLY,
		MODE_PREVIEW,
	};

	mode_t				mode			= MODE_AMIGA;
	map<DWORD, double>	color_stats;
	byte				st_map[256];
	int					st_alloc		= 0;				// internal
	BinaryWordData		st_color_table;

	byte Process(DWORD color);

	void Amiga_Init()		{ mode = MODE_AMIGA; }
	void ST_StartGather();
	void ST_StartApply();
	void Preview_Init()		{ mode = MODE_PREVIEW; }

	bool ST_AssureColor(int color);

};



// ================================================================ GfxColor ================================================================


struct GfxColor {
	vec3		rgb;
	vec3		colorspace_color;
	DWORD		color;
	DWORD		color_out;
	DWORD		code;

	GfxColor(DWORD col) : color(col), color_out(col) {
		rgb = vec3::from_rgb(col);
	}

	GfxColor(const vec3 _rgb) : rgb(_rgb) {
		if( rgb.x < 0 ) rgb.x = 0;
		if( rgb.y < 0 ) rgb.y = 0;
		if( rgb.z < 0 ) rgb.z = 0;
		if( rgb.x > 1 ) rgb.x = 1;
		if( rgb.y > 1 ) rgb.y = 1;
		if( rgb.z > 1 ) rgb.z = 1;
		color = color_out = rgb.make_rgb();
	}
};

struct GfxBestColorCode {
	// All 'vec3' values in chosen color space
	vec3		ref_color;

	vec3		first_color;
	int			first_code = 0;
	float		first_err = 1e38f;

	vec3		second_color;
	int			second_code = 0;
	float		second_err = 1e38f;
};


// ================================================================ GfxPalette ================================================================


class GfxPalette : public AssetBase {
public:

	enum dither_mode_t {
		// Dithering modes:		(optional error diffusion is applied to all methods)
		//
		DITHER_SOLID = 0,				// best color is used for entire texel
		DITHER_HALF_CLOSEST,			// one half is always the color closest to the original (picked not using the diffusion), other is chosen to produce best mix
		DITHER_FULL,					// all color mixes are considered
		DITHER_CHECKER,					// finds two colors to mix and uses patterns to mix them in 0%, 25%, 50%, 75%, 100% proportions (producing three extra intermediate colors)
		DITHER_CHECKER_CLOSEST,			// same as CHECKERED, but one of the colors is always the color closest to the original (picked not using the diffusion)
	};

	enum pair_order_t {
		// Color ordering within the pair
		PAIR_ORDER_KEEP = 0,			// Keep whatever the dithering algorithm produced
		PAIR_ORDER_SORT,				// Sort texel halves
		PAIR_ORDER_REV_SORT,			// Sort texel halves in reverse order
		PAIR_ORDER_CHECKER,				// Sort colors with order depending on checkerboard pattern
		PAIR_ORDER_YCHECKER,			// Sort colors with order depending on vertical position
		PAIR_ORDER_SOLID_CHECKER,		// Pick one of the colors and use it for full texel depending on checkerboard pattern
	};

	struct PalExportOptions {
		bool		enabled = false;
		bool		mode_STe = false;
		string		name;
		int			color_count = 0;
	};


	// general
	string				name_alias;

	// colors
	vector<GfxColor>	colors;
	map<int, float>		weights;
	int					opt_load_colors = 0;

	// color space
	shared_ptr<GfxColorSpace>	colorspace;

	// colormap export
	PalExportOptions	opt_export_amiga;
	PalExportOptions	opt_export_st;
	vector<GfxPalette*>	colormaps;			// not owned


	// dithering
	dither_mode_t		dither_mode = DITHER_SOLID;
	pair_order_t		pair_order = PAIR_ORDER_KEEP;
	float				mix_penalty  = 0;
	vec3				mix_penalty_components = vec3(1, 1, 1);
	vec3				diffuse_components = vec3(1, 1, 1);
	vec4				diffuse_kernel = vec4(0, 0, 0, 0);
	float				master_brightness = 1.0f;

	virtual void Focus() { printf("Click %s!\n", name.c_str()); }


	int size() { return colors.size(); }
	GfxColor &operator[](int i) { return colors[i]; }
	GfxColor &GetColor(int i) { return colors[i]; }


	void Init();

	//void Clear() {
	//	colors.clear();
	//	weights.clear();
	//}

	void CheckBestColor(GfxBestColorCode &best, int pid);
	void CheckBestColorChecker(GfxBestColorCode &best, int pid1, int pid2);
	void CheckBestColorMix(GfxBestColorCode &best, int code, int pid1, int pid2, float mix);

	void DitherTexture(DataTexture &tex, bool masked);

	DWORD StringToColor(DefNode *user, const char *s);
	void Load(DefNode *node, GfxConverter *owner, const string &_name);


	virtual void ExportGraphics(AssetExportContext &ctx);
	
	bool ExportPaletteBlock(DataBlock &block);


protected:
	float ApplyErrorWeight(int code, float error)
	{
		auto p = weights.find(code);
		if( p!=weights.end() )
		{
			if( p->second<=0 )
				error = 1e38f;
			else
				error /= p->second;
		}
		return error;
	}
};
