
#pragma once


class GfxAsset;
class GfxAssetGroup;
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
	};

	mode_t				mode			= MODE_AMIGA;
	map<DWORD, double>	color_stats;
	byte				st_map[256];
	int					st_alloc		= 0;				// internal
	BinaryWordData		st_color_table;

	byte Process(DWORD color);

	void Amiga_Init() { mode = MODE_AMIGA; }
	void ST_StartGather();
	void ST_StartApply();

	bool ST_AssureColor(int color);

};



// ================================================================ LABColor ================================================================



struct LABColor {
	float l;
	float a;
	float b;

	LABColor() {}
	LABColor(float _l, float _a, float _b) : l(_l), a(_a), b(_b) {}
	LABColor operator *(float s) const { return LABColor(l*s, a*s, b*s); }
	LABColor operator +(const LABColor &lab) const { return LABColor(l+lab.l, a+lab.a, b+lab.b); }
	LABColor operator -(const LABColor &lab) const { return LABColor(l-lab.l, a-lab.a, b-lab.b); }
	void operator +=(const LABColor &lab) { l+=lab.l; a+=lab.a; b+=lab.b; }
	void operator -=(const LABColor &lab) { l-=lab.l; a-=lab.a; b-=lab.b; }


	// sRGB -> linear RGB
	static vec3 srgb_to_linear(const vec3 &rgb)
	{
		vec3 lrgb;
		lrgb.x = (rgb.x <= 0.04045f) ? rgb.x / 12.92f : pow((rgb.x + 0.055f) / 1.055f, 2.4f);
		lrgb.y = (rgb.y <= 0.04045f) ? rgb.y / 12.92f : pow((rgb.y + 0.055f) / 1.055f, 2.4f);
		lrgb.z = (rgb.z <= 0.04045f) ? rgb.z / 12.92f : pow((rgb.z + 0.055f) / 1.055f, 2.4f);
		return lrgb;
	}

	// linear RGB -> sRGB
	static vec3 linear_to_srgb(const vec3 &lrgb)
	{
		vec3 rgb;
		rgb.x = (lrgb.x > 0.0031308f) ? 12.92f * lrgb.x : 1.055f * pow(lrgb.x, 1/2.4f) - 0.055f;
		rgb.y = (lrgb.y > 0.0031308f) ? 12.92f * lrgb.y : 1.055f * pow(lrgb.y, 1/2.4f) - 0.055f;
		rgb.z = (lrgb.z > 0.0031308f) ? 12.92f * lrgb.z : 1.055f * pow(lrgb.z, 1/2.4f) - 0.055f;
		return rgb;
	}

	// linear RGB -> XYZ
	static vec3 linear_to_xyz(const vec3 &lrgb)
	{
		vec3 xyz;
		xyz.x = (lrgb.x * 0.41239080f + lrgb.y * 0.35758434f + lrgb.z + 0.18048079) / 0.950467f;
		xyz.y = (lrgb.x * 0.21263901f + lrgb.y * 0.71516868f + lrgb.z + 0.07219232) / 1.00000f;
		xyz.z = (lrgb.x * 0.01933082f + lrgb.y * 0.11919478f + lrgb.z + 0.95053215) / 1.088969f;
		return xyz;
	}

	// XYZ -> linear RGB
	static vec3 xyz_to_linear(const vec3 &xyz)
	{
		vec3 h = vec3(xyz.x*0.950467f, xyz.y*1.00000f, xyz.z*1.088969f);
		vec3 lrgb;
		lrgb.x = h.x *  3.24096994f + h.y * -1.53738318f + h.z * -0.49861076f;
		lrgb.y = h.x * -0.96924364f + h.y *  1.87596750f + h.z *  0.04155506f;
		lrgb.z = h.x *  0.05563008f + h.y * -0.20397696f + h.z *  1.05697151f;
		return lrgb;
	}

	// XYZ -> LAB
	static LABColor xyz_to_lab(const vec3 &xyz)
	{
		LABColor lab;
		vec3 h;
		h.x = H(xyz.x);
		h.y = H(xyz.y);
		h.z = H(xyz.z);
		lab.l = 116 * h.y - 16;
		lab.a = 500 * (h.x - h.y);
		lab.b = 200 * (h.y - h.z);
		return lab;
	}

	// LAB -> XYZ
	static vec3 lab_to_xyz(const LABColor &lab)
	{
		vec3 xyz;
		xyz.y = (lab.l + 16) / 116;
		xyz.x = lab.a / 500 + xyz.y;
		xyz.z = xyz.y - lab.b / 200;

		xyz.x = iH(xyz.x);
		xyz.y = iH(xyz.y);
		xyz.z = iH(xyz.z);
		return xyz;
	}

	static LABColor from_rgb(const vec3 &rgb) {
		//const float adapt0 = 0.950467f;
		//const float adapt1 = 1.000000f;
		//const float adapt2 = 1.088969f;
		//float R = rgb.x;// * 0.003922f;
		//float G = rgb.y;// * 0.003922f;
		//float B = rgb.z;// * 0.003922f;
		//float X = 0.412424f  * R + 0.357579f * G + 0.180464f  * B;
		//float Y = 0.212656f  * R + 0.715158f * G + 0.0721856f * B;
		//float Z = 0.0193324f * R + 0.119193f * G + 0.950444f  * B;
		//LABColor lab;
		//lab.l = 116 * H(Y / adapt1) - 16;
		//lab.a = 500 * (H(X / adapt0) - H(Y / adapt1));
		//lab.b = 200 * (H(Y / adapt1) - H(Z / adapt2));

		//LABColor lab;
		//vec3 lrgb = srgb_to_linear(rgb);
		//vec3 xyz = linear_to_xyz(rgb);
		//lab.l = xyz.x;
		//lab.a = xyz.y;
		//lab.b = xyz.z;
		//return lab;

		return xyz_to_lab(linear_to_xyz(srgb_to_linear(rgb)));
	}

	vec3 to_rgb() {
		return linear_to_srgb(xyz_to_linear(lab_to_xyz(*this)));
	}


	float error_to(const LABColor &lab, float color_coeff=1.0f)
	{
		float dl = lab.l - l;
		float da = (lab.a - a)*color_coeff;
		float db = (lab.b - b)*color_coeff;
		return dl*dl + da*da + db*db;
	}

protected:
	static float H(float q)
	{
		return q > 0.00885645f ? pow(q, 1/3.f) : 7.78703f*q + 0.137931f;
	}

	static float iH(float q)
	{
		return (q*q*q > 0.00885645f) ? q*q*q : (q - 0.137931f) / 7.78703f;
	}
};


// ================================================================ GfxColorSpace ================================================================


class GfxColorSpace {
public:
	virtual vec3  Convert(const vec3 &rgb) = 0;
	virtual vec3  Deconvert(const vec3 &color) = 0;
	virtual float Distance(const vec3 &color1, const vec3 &color2) = 0;
};


class RGBColorSpace : public GfxColorSpace {
public:
	float	gamma_r = 1.f;
	float	gamma_g = 1.f;
	float	gamma_b = 1.f;
	vec3	rgb_weight	= vec3(1, 1, 1);
	vec3	luma_mix	= vec3(1, 1, 1);
	float	luma_weight	= 0;


	virtual vec3  Convert(const vec3 &rgb);
	virtual vec3  Deconvert(const vec3 &color);
	virtual float Distance(const vec3 &color1, const vec3 &color2);

	void Load(DefNode *node);
};

class LABColorSpace : public GfxColorSpace {
public:
	float	weight_l = 1.0f;
	float	weight_a = 1.0f;
	float	weight_b = 1.0f;

	virtual vec3  Convert(const vec3 &rgb);
	virtual vec3  Deconvert(const vec3 &color);
	virtual float Distance(const vec3 &color1, const vec3 &color2);

	void Load(DefNode *node);
};

class YUVColorSpace : public GfxColorSpace {
public:
	float	weight_y = 1.0f;
	float	weight_u = 1.0f;
	float	weight_v = 1.0f;

	virtual vec3  Convert(const vec3 &rgb);
	virtual vec3  Deconvert(const vec3 &color);
	virtual float Distance(const vec3 &color1, const vec3 &color2);

	void Load(DefNode *node);
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


class GfxPalette : public ToolObject {
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

	// colors
	string				name;
	vector<GfxColor>	colors;
	map<int, float>		weights;
	int					opt_load_colors = 0;

	// color space
	GfxColorSpace		*colorspace = &yuv_colorspace;
	RGBColorSpace		rgb_colorspace;
	LABColorSpace		lab_colorspace;
	YUVColorSpace		yuv_colorspace;

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

	void Clear() {
		colors.clear();
		weights.clear();
	}

	void CheckBestColor(GfxBestColorCode &best, int pid);
	void CheckBestColorChecker(GfxBestColorCode &best, int pid1, int pid2);
	void CheckBestColorMix(GfxBestColorCode &best, int code, int pid1, int pid2, float mix);

	void DitherTexture(DataTexture &tex, bool masked);

	DWORD StringToColor(DefNode *user, const char *s);
	void Load(DefNode *node, GfxConverter *owner, const string &_name);


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
