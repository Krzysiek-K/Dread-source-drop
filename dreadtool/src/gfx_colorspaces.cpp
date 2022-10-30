
#include "common.h"
#include "gfx_colorspaces.h"




// ================================================================ RGBColorSpace ================================================================

GfxColorSpace *GfxColorSpace::CreateFromNode(DefNode *n)
{
	GfxColorSpace *colorspace = NULL;

	n->RequireArgs(1);

	/**/ if( n->args[0]=="rgb" )	colorspace = new RGBColorSpace();
	else if( n->args[0]=="xyz" )	colorspace = new XYZColorSpace();		// TBD: document
	else if( n->args[0]=="lab" )	colorspace = new LABColorSpace();
	else if( n->args[0]=="badlab" )	colorspace = new BadLABColorSpace();	// TBD: document
	else if( n->args[0]=="yuv" )	colorspace = new YUVColorSpace();		// TBD: document
	else
		n->Error(format("Invalid color space '%s' (supported options: rgb, xyz, yuv, lab, badlab)", n->args[0].c_str()));

	if( colorspace )
		colorspace->Load(n);

	return colorspace;
}

// ================================================================ RGBColorSpace ================================================================


vec3 RGBColorSpace::Convert(const vec3 &rgb)
{
	return vec3(
		pow(rgb.x, gamma_r),
		pow(rgb.y, gamma_g),
		pow(rgb.z, gamma_b)
	);
}

vec3 RGBColorSpace::Deconvert(const vec3 &color)
{
	return vec3(
		pow(color.x, 1.f/gamma_r),
		pow(color.y, 1.f/gamma_g),
		pow(color.z, 1.f/gamma_b)
	);
}


float RGBColorSpace::Distance(const vec3 &color1, const vec3 &color2)
{
	vec3 e_rgb = color2 - color1;
	float e_luma = color2.dot(luma_mix) - color1.dot(luma_mix);
	e_rgb.scale_xyz(rgb_weight);
	e_luma *= luma_weight;
	return sqrt(e_rgb.dot(e_rgb) + e_luma*e_luma);
}

void RGBColorSpace::Load(DefNode *node)
{
	for( DefNode *n : node->subnodes )
	{
		n->type_name = "RGB colorspace option";

		try
		{
			if( n->name=="-gamma" )
			{
				n->RequireArgsOr(1, 3);
				n->ForbidSubnodes();
				gamma_r = n->GetFloatArg(0);
				gamma_g = n->GetFloatArgDef(1, gamma_r);
				gamma_b = n->GetFloatArgDef(2, gamma_r);
			}
			else if( n->name=="-weights" )
			{
				n->RequireArgsOr(3, 4);
				n->ForbidSubnodes();
				rgb_weight.x = n->GetFloatArg(0);
				rgb_weight.y = n->GetFloatArg(1);
				rgb_weight.z = n->GetFloatArg(2);
				luma_weight = n->GetFloatArgDef(3, 0.f);
			}
			else if( n->name=="-luma" )
			{
				n->RequireArgs(3);
				n->ForbidSubnodes();
				luma_mix.x = n->GetFloatArg(0);
				luma_mix.y = n->GetFloatArg(1);
				luma_mix.z = n->GetFloatArg(2);
				luma_mix.normalize();
				if( !luma_weight )
					luma_weight = 1.f;
			}
			else
				n->Unknown();
		}
		catch( const string &e )
		{
			elog.Error(NULL, e);
		}
	}
}



// ================================================================ XYZColorSpace ================================================================

vec3 XYZColorSpace::Convert(const vec3 &rgb)
{
	return
		ColorConversions::linear_to_xyz(
			ColorConversions::srgb_to_linear(rgb)
		);
}

vec3 XYZColorSpace::Deconvert(const vec3 &color)
{
	return
		ColorConversions::xyz_to_linear(
			ColorConversions::lab_to_xyz(color)
		);
}


float XYZColorSpace::Distance(const vec3 &color1, const vec3 &color2)
{
	vec3 error = color2 - color1;
	error.x *= weight_x;
	error.y *= weight_y;
	error.z *= weight_z;
	error.scale_xyz(error);
	return sqrt(error.x + error.y + error.z);
}

void XYZColorSpace::Load(DefNode *node)
{
	for( DefNode *n : node->subnodes )
	{
		n->type_name = "XYZ colorspace option";

		try
		{
			if( n->name=="-weights" )			// TBD: document
			{
				n->ForbidSubnodes();
				n->RequireArgs(3);
				weight_x = n->GetFloatArg(0);
				weight_y = n->GetFloatArg(1);
				weight_z = n->GetFloatArg(2);
			}
			else
				n->Unknown();
		}
		catch( const string &e )
		{
			elog.Error(NULL, e);
		}
	}
}


// ================================================================ LABColorSpace ================================================================

vec3 LABColorSpace::Convert(const vec3 &rgb)
{
	return
		ColorConversions::xyz_to_lab(
			ColorConversions::linear_to_xyz(
				ColorConversions::srgb_to_linear(rgb)
			)
		);
}

vec3 LABColorSpace::Deconvert(const vec3 &color)
{
	return
		ColorConversions::linear_to_srgb(
			ColorConversions::xyz_to_linear(
				ColorConversions::lab_to_xyz(color)
			)
		);
}


float LABColorSpace::Distance(const vec3 &color1, const vec3 &color2)
{
	vec3 error = color2 - color1;
	error.x *= weight_l;
	error.y *= weight_a;
	error.z *= weight_b;
	error.scale_xyz(error);
	return sqrt(error.x + error.y + error.z);
}

void LABColorSpace::Load(DefNode *node)
{
	for( DefNode *n : node->subnodes )
	{
		n->type_name = "LAB colorspace option";

		try
		{
			if( n->name=="-weights" )
			{
				n->ForbidSubnodes();
				n->RequireArgs(3);
				weight_l = n->GetFloatArg(0);
				weight_a = n->GetFloatArg(1);
				weight_b = n->GetFloatArg(2);
			}
			else
				n->Unknown();
		}
		catch( const string &e )
		{
			elog.Error(NULL, e);
		}
	}
}



// ================================================================ BadLABColorSpace ================================================================

vec3 BadLABColorSpace::Convert(const vec3 &rgb)
{
	return ColorConversions::rgb_to_badlab(rgb);
}

vec3 BadLABColorSpace::Deconvert(const vec3 &color)
{
	return vec3(0, 0, 0);	// TBD: implement
}


float BadLABColorSpace::Distance(const vec3 &color1, const vec3 &color2)
{
	vec3 error = color2 - color1;
	error.x *= weight_l;
	error.y *= weight_a;
	error.z *= weight_b;
	error.scale_xyz(error);
	return sqrt(error.x + error.y + error.z);
}

void BadLABColorSpace::Load(DefNode *node)
{
	for( DefNode *n : node->subnodes )
	{
		n->type_name = "BadLAB colorspace option";

		try
		{
			if( n->name=="-weights" )		// TBD: document
			{
				n->ForbidSubnodes();
				n->RequireArgs(3);
				weight_l = n->GetFloatArg(0);
				weight_a = n->GetFloatArg(1);
				weight_b = n->GetFloatArg(2);
			}
			else
				n->Unknown();
		}
		catch( const string &e )
		{
			elog.Error(NULL, e);
		}
	}
}


// ================================================================ YUVColorSpace ================================================================

vec3 YUVColorSpace::Convert(const vec3 &rgb)
{
	vec3 yuv;
	yuv.x = 0.299f * rgb.x + 0.587f * rgb.y + 0.114f * rgb.z;
	yuv.y = 0.492f * (rgb.z - yuv.x);
	yuv.z = 0.877f * (rgb.x - yuv.x);
	return yuv;
}

vec3 YUVColorSpace::Deconvert(const vec3 &color)
{
	vec3 rgb;
	rgb.x = color.x + 1.13983f * color.z;
	rgb.y = color.x - 0.39465f * color.y - 0.58060f * color.z;
	rgb.z = color.x + 2.03211f * color.y;
	rgb.saturate();
	return rgb;
}


float YUVColorSpace::Distance(const vec3 &color1, const vec3 &color2)
{
	vec3 error = color2 - color1;
	error.x *= weight_y;
	error.y *= weight_u;
	error.z *= weight_v;
	error.scale_xyz(error);
	return sqrt(error.x + error.y + error.z);
}

void YUVColorSpace::Load(DefNode *node)
{
	for( DefNode *n : node->subnodes )
	{
		n->type_name = "YUV colorspace option";

		try
		{
			if( n->name=="-weights" )
			{
				n->ForbidSubnodes();
				n->RequireArgs(3);
				weight_y = n->GetFloatArg(0);
				weight_u = n->GetFloatArg(1);
				weight_v = n->GetFloatArg(2);
			}
			else
				n->Unknown();
		}
		catch( const string &e )
		{
			elog.Error(NULL, e);
		}
	}
}
