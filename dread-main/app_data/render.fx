
float4x4 ViewProj;
float3 cam_pos;
float3 cam_right;
float2 ssize;
float3 bg_color;
float2 tex_size;
float2 tex_offs;



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
	float mask = 0;
	if( uv.y<0 || uv.y>=1 ) mask = 1;
	return tex2D(stex,uv) + float4(mask,0,0,0);
}

float4 PS_Sky(float2 uv : VPOS) : COLOR
{
	return tex2D(stex, (uv+tex_offs)/float2(128,64));
}


void VS_Sprites(
	float4 pos			: POSITION,
	float2 uv			: TEXCOORD0,
	out float4 hpos		: POSITION,
	out float2 _uv		: TEXCOORD0
	)
{
	pos.y = -pos.y;
	pos.xyz += cam_right*(uv.x*tex_size.x-tex_offs.x);
	pos.z += (1-uv.y)*tex_size.y-tex_size.y+tex_offs.y;
	hpos = mul(pos, ViewProj);
	//hpos.z -= 0.02;
	_uv = uv;
}

float4 PS_Sprites(	float2 uv : TEXCOORD0 ) : COLOR
{
	float4 col = tex2D(stex,uv);
	if(col.y*255>254) discard;
	return col;
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
	float3 rawcol = tex2D(stex, (px+0.5)/ssize).zyx;
	float3 col = floor(rawcol*255+0.5);
	if(mode>=1) col.x = fmod(vp.x+vp.y*(mode==4),2)<0.5 ? col.x : col.y;
	if( col.x >= 255 ) return float4(bg_color,1);
	
	float4 outcol = tex2D(palette,float2((col.x+0.5)/pal_size,0.5));
	float mask = 0.3;	//saturate(sin((vp.x+vp.y)/1.5)*10+4)*0.3;

	return lerp(outcol,float4(1,0,0,0),mask*rawcol.z);
}

float4 PS_SoundPreview(float2 uv : TEXCOORD0) : COLOR
{
	float4 info = tex2D(stex,float2(uv.x,0.5))*2-1;
	float y = (1-uv.y)*2-1;
	float v, g;

	if( y<info.z )
		v = 1-(info.z-y)/(info.z-info.x);
	else
		v = 1-(y-info.z)/(info.y-info.z);

	v = saturate(v);
	g = (y>info.x && y<info.y);

	return float4(v,(g+v*2)/3,v,0);
}

technique tech {
	pass {
		VertexShader = compile vs_3_0 VS();
		PixelShader = compile ps_3_0 PS();
	}
}

technique tech_sky {
	pass {
		VertexShader = compile vs_3_0 VS();
		PixelShader = compile ps_3_0 PS_Sky();
	}
}

technique sprites {
	pass {
		VertexShader = compile vs_3_0 VS_Sprites();
		PixelShader = compile ps_3_0 PS_Sprites();
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

technique sound_preview{
	pass {
		PixelShader = compile ps_3_0 PS_SoundPreview();
	}
}
