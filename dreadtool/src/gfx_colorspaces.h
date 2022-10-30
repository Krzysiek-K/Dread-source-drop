
#pragma once



// ================================================================ ColorConversions ================================================================



struct ColorConversions {

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
	static vec3 xyz_to_lab(const vec3 &xyz)
	{
		vec3 lab;
		vec3 h;
		h.x = lab_H(xyz.x);
		h.y = lab_H(xyz.y);
		h.z = lab_H(xyz.z);
		lab.x = 116 * h.y - 16;
		lab.y = 500 * (h.x - h.y);
		lab.z = 200 * (h.y - h.z);
		return lab;
	}

	// LAB -> XYZ
	static vec3 lab_to_xyz(const vec3 &lab)
	{
		vec3 xyz;
		xyz.y = (lab.x + 16) / 116;
		xyz.x = lab.y / 500 + xyz.y;
		xyz.z = xyz.y - lab.z / 200;

		xyz.x = lab_iH(xyz.x);
		xyz.y = lab_iH(xyz.y);
		xyz.z = lab_iH(xyz.z);
		return xyz;
	}

	static vec3 rgb_to_badlab(const vec3 &rgb) {
		const float adapt0 = 0.950467f;
		const float adapt1 = 1.000000f;
		const float adapt2 = 1.088969f;
		float R = rgb.x * 0.003922f;
		float G = rgb.y * 0.003922f;
		float B = rgb.z * 0.003922f;
		float X = 0.412424f  * R + 0.357579f * G + 0.180464f  * B;
		float Y = 0.212656f  * R + 0.715158f * G + 0.0721856f * B;
		float Z = 0.0193324f * R + 0.119193f * G + 0.950444f  * B;
		vec3 lab;
		lab.x = 116 * lab_H(Y / adapt1) - 16;
		lab.y = 500 * (lab_H(X / adapt0) - lab_H(Y / adapt1));
		lab.z = 200 * (lab_H(Y / adapt1) - lab_H(Z / adapt2));
		return lab;
	}


protected:
	static float lab_H(float q)
	{
		return q > 0.00885645f ? pow(q, 1/3.f) : 7.78703f*q + 0.137931f;
	}

	static float lab_iH(float q)
	{
		return (q*q*q > 0.00885645f) ? q*q*q : (q - 0.137931f) / 7.78703f;
	}
};


// ================================================================ GfxColorSpace ================================================================


class GfxColorSpace : public Collectable {
public:
	virtual vec3  Convert(const vec3 &rgb) = 0;
	virtual vec3  Deconvert(const vec3 &color) = 0;
	virtual float Distance(const vec3 &color1, const vec3 &color2) = 0;
	virtual void  Load(DefNode *node) = 0;

	static GfxColorSpace *CreateFromNode(DefNode *node);
};



// ================================================================ RGBColorSpace ================================================================

class RGBColorSpace : public GfxColorSpace {
public:
	float	gamma_r = 1.f;
	float	gamma_g = 1.f;
	float	gamma_b = 1.f;
	vec3	rgb_weight	= vec3(1, 1, 1);
	vec3	luma_mix	= vec3(1/3.f, 1/3.f, 1/3.f);
	float	luma_weight	= 0;


	virtual vec3  Convert(const vec3 &rgb);
	virtual vec3  Deconvert(const vec3 &color);
	virtual float Distance(const vec3 &color1, const vec3 &color2);
	virtual void  Load(DefNode *node);
};



// ================================================================ XYZColorSpace ================================================================

class XYZColorSpace : public GfxColorSpace {
public:
	float	weight_x = 1.0f;
	float	weight_y = 1.0f;
	float	weight_z = 1.0f;

	virtual vec3  Convert(const vec3 &rgb);
	virtual vec3  Deconvert(const vec3 &color);
	virtual float Distance(const vec3 &color1, const vec3 &color2);
	virtual void  Load(DefNode *node);
};



// ================================================================ LABColorSpace ================================================================

class LABColorSpace : public GfxColorSpace {
public:
	float	weight_l = 1.0f;
	float	weight_a = 1.0f;
	float	weight_b = 1.0f;

	virtual vec3  Convert(const vec3 &rgb);
	virtual vec3  Deconvert(const vec3 &color);
	virtual float Distance(const vec3 &color1, const vec3 &color2);
	virtual void  Load(DefNode *node);
};



// ================================================================ BadLABColorSpace ================================================================

class BadLABColorSpace : public GfxColorSpace {
public:
	float	weight_l = 1.0f;
	float	weight_a = 1.0f;
	float	weight_b = 1.0f;

	virtual vec3  Convert(const vec3 &rgb);
	virtual vec3  Deconvert(const vec3 &color);
	virtual float Distance(const vec3 &color1, const vec3 &color2);
	virtual void  Load(DefNode *node);
};



// ================================================================ YUVColorSpace ================================================================

class YUVColorSpace : public GfxColorSpace {
public:
	float	weight_y = 1.0f;
	float	weight_u = 1.0f;
	float	weight_v = 1.0f;

	virtual vec3  Convert(const vec3 &rgb);
	virtual vec3  Deconvert(const vec3 &color);
	virtual float Distance(const vec3 &color1, const vec3 &color2);
	virtual void  Load(DefNode *node);
};
