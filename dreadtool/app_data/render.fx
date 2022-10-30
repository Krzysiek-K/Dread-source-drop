
float4x4 ViewProj;
float3 cam_pos;
float2 ssize;
float3 bg_color;
float4 shadow_params;		// x: xoffs		y: yoffs	z: -	w: alpha



//texture tex;
//
//sampler stex = sampler_state {
//	Texture = <tex>;
//	MagFilter = Point;
//	MinFilter = Point;
//	MipFilter = None;
//	AddressU = Wrap;
//	AddressV = Wrap;
//};

sampler stex : register(s0);
sampler palette : register(s1);
sampler pstex : register(s2);

float pal_size : register(c0);


void VS(
	float4 pos			: POSITION,
	float2 uv			: TEXCOORD0,
	out float4 hpos		: POSITION,
	out float2 _uv		: TEXCOORD0
	)
{
	hpos = mul(pos, ViewProj);
	_uv = uv;
}



float4 PS(	float2 uv : TEXCOORD0 ) : COLOR
{
	return tex2D(stex,uv);
}

float4 PS_Resolve(uniform int mode, float2 uv : TEXCOORD0, float2 vp : VPOS) : COLOR
{
	float2 px = floor(uv*ssize);
	if( mode==3 && fmod(vp.y, 2)>0.5 )
		return 0;
	if(mode==2)
	{
		px.x -= fmod(vp.y, 2);
		vp.x -= fmod(vp.y, 2);
		if(px.x<0) return 0;
	}
	float2 col = floor(tex2D(stex,(px+0.5)/ssize).zy*255+0.5);
	if(mode>=1) col.x = fmod(vp.x+vp.y*(mode==4),2)<0.5 ? col.x : col.y;

	float2 pcol = floor(tex2D(pstex, (px+0.5-shadow_params.xy)/ssize).zy*255+0.5);
	if( mode>=1 ) pcol.x = fmod(vp.x+vp.y*(mode==4), 2)<0.5 ? pcol.x : pcol.y;
	float4 ppcol = tex2D(palette, float2((pcol.x+0.5)/pal_size, 0.5));
	ppcol = lerp(float4(bg_color, 1), ppcol, (pcol.x >= 255) ? 0 : shadow_params.w);
	if( pcol.x<255 && shadow_params.w>0 ) return ppcol;

	if( col.x >= 255 ) return ppcol;

	if( col.x >= 255 ) return float4(bg_color,1);
	return tex2D(palette,float2((col.x+0.5)/pal_size,0.5));
}

technique tech {
	pass {
		VertexShader = compile vs_3_0 VS();
		PixelShader = compile ps_3_0 PS();
	}
}

technique resolve0 {
	pass {
		PixelShader = compile ps_3_0 PS_Resolve(0);
	}
}

technique resolve1 {
	pass {
		PixelShader = compile ps_3_0 PS_Resolve(1);
	}
}

technique resolve2 {
	pass {
		PixelShader = compile ps_3_0 PS_Resolve(2);
	}
}

technique resolve3 {
	pass {
		PixelShader = compile ps_3_0 PS_Resolve(3);
	}
}

technique resolve4 {
	pass {
		PixelShader = compile ps_3_0 PS_Resolve(4);
	}
}
