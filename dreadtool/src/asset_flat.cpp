
#include "common.h"
#include "app_rendertest.h"


const char GfxFlatDesc::TYPE_NAME[] = "flat asset";




// ================================================================ GfxFlatDesc::LightLevel ================================================================


void GfxFlatDesc::LightLevel::GenerateTexture()
{
	texture.texture.Resize(64, 64);
	texture.texture.Clear(pixel);
	texture.texture.Upload();
}


// ================================================================ GfxFlatDesc ================================================================



bool GfxFlatDesc::ImportAssets()
{
	for(LightLevel *lv : levels)
		lv->GenerateTexture();

	return true;
}

void GfxFlatDesc::Load(DefNode *node, GfxAssetGroup *aowner, GfxConverter *owner)
{
	try
	{
		node->RequireArgsMin(2);
		InitLoad(node, 0, aowner, owner);
		owner->AddAsset(this);
		node->ForbidSubnodes();

		for( int i=1; i<node->args.size(); i++ )
		{
			const char *s = node->args[i].c_str();
			ParseWhitespace(s);
			const char *b = s;
			int light_level = 255;
			int flat_color = ParseInt(s);

			if( s<=b || flat_color<0 || flat_color>=16 ) node->InvalidArg(i, "Expected color index");
			flat_color *= 0x0101;

			ParseWhitespace(s);
			if( *s==':' )
			{
				s++;
				ParseWhitespace(s);
				b = s;
				int value = ParseInt(s);
				if( s<=b || value<0 || value>=16 ) node->InvalidArg(i, "Expected second color index");
				flat_color = (flat_color&0xFF) | (value<<8);
			}

			ParseWhitespace(s);
			if( *s=='@' )
			{
				s++;
				ParseWhitespace(s);
				b = s;
				light_level = ParseInt(s);
				if( s<=b || light_level<0 || light_level>255 ) node->InvalidArg(i, "Expected light level value");
			}

			ParseWhitespace(s);
			if( *s ) node->InvalidArg(i, "Unexpected characters at the end of the argument");
			
			
			LightLevel *lv = new LightLevel();
			lv->level = light_level;
			lv->pixel = flat_color;
			levels.push_back(lv);
		}
	}
	catch( const string &e )
	{
		elog.Error(this, e);
	}
}

GfxFlatDesc::LightLevel *GfxFlatDesc::FindBestLightLevel(int light_level)
{
	int best_level = -1;
	int best_error = 1000;
	for( int i=0; i<levels.size(); i++ )
	{
		int error = light_level - levels[i]->level;
		if( error<0 ) error = -error;
		if( error < best_error )
		{
			best_level = i;
			best_error = error;
		}
	}
	return best_level>=0 ? levels[best_level] : NULL;
}

void GfxFlatDesc::ExportPK3Flat(const char *path)
{
	if( levels.size()<=0 || !asset_group )
		return;

	GfxPalette *pal = asset_group->opt_palette;
	if( !pal ) pal = converter_owner->main_palette;
	if( !pal ) return;

	LightLevel *level = levels[0];
	vector<DWORD> texdata(64*64);
	DWORD *dst = &texdata[0];

	for( int y=0; y<64; y++ )
		for( int x=0; x<64; x+=2 )
		{
			DWORD c0 = level->pixel & 0xFF;
			DWORD c1 = (level->pixel >> 8) & 0xFF;
			*dst++ = pal->GetColor(c0).color_out | 0xFF000000;
			*dst++ = pal->GetColor(c1).color_out | 0xFF000000;
		}

	DevTexture dtex;
	dtex.LoadRaw2D(D3DFMT_X8R8G8B8, 64, 64, &texdata[0], false);

	D3DXSaveTextureToFile(format(path, name.c_str()).c_str(), D3DXIFF_PNG, dtex, NULL);
}
