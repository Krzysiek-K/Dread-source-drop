
#include "common.h"
#include "app_rendertest.h"



void DataTexture::Upload()
{
	if( data.size()>0 )
		tex.LoadRaw2D(D3DFMT_A8R8G8B8, width, height, &data[0], false);
}


bool DataTexture::Load(string path)
{
	tex.Load(path.c_str());
	this->path = path;

	bool ok = tex.GetRawData(width, height, data);
	if( !ok )
	{
		Resize(1, 1);
		Upload();
	}

	if(config_id.size()>0)
		cfg.SetString(config_id.c_str(), this->path.c_str());

	return ok;
}

void DataTexture::CopyFrom(const DataTexture &src)
{
	data = src.data;
	width = src.width;
	height = src.height;
	off_x = src.off_x;
	off_y = src.off_y;

	Upload();
}

void DataTexture::InitConfig(const char *id)
{
	config_id = id;
	Load(cfg.GetString("texscale-tex-path", "").c_str());
}

void DataTexture::Draw(int x0, int y0, int scale)
{
	Upload();
	Dev->SetTexture(0, tex);
	Dev.SetSampler(0, TEXF_POINT);
	Dev.DrawScreenQuad(x0, y0, x0+width*scale, y0+height*scale);
}

bool DataTexture::MenuEdit(const char *desc, int height)
{
	ms.LineSetup(-1);
	ms.LineHeight(height);
	if( ms.TextureSelector(desc, tex, path) )
	{
		Load(string(path).c_str());
		return true;
	}
	return false;
}


void DataTexture::Crop(int x1, int y1, int x2, int y2)
{
	vector<DWORD> newdata;
	for( int y=y1; y<y2; y++ )
		for( int x=x1; x<x2; x++ )
			newdata.push_back(DWORD(x)<DWORD(width) && DWORD(y)<DWORD(height) ? data[x+y*width] : 0);
	data.swap(newdata);
	width = x2-x1;
	height = y2-y1;
	off_x -= x1;
	off_y -= y1;
}

bool DataTexture::IsColumnTransparent(int x)
{
	for( int y=0; y<height; y++ )
		if( (data[x+y*width]>>24) >= 128 )
			return false;
	return true;
}

bool DataTexture::IsRowTransparent(int y)
{
	for( int x=0; x<width; x++ )
		if( (data[x+y*width]>>24) >= 128 )
			return false;
	return true;
}

void DataTexture::Trim()
{
	int x1=0, y1=0, x2=width-1, y2=height-1;
	bool crop = false;

	while( x1<x2 && IsColumnTransparent(x2) )
		x2--, crop=true;

	while( x1<x2 && IsColumnTransparent(x1) )
		x1++, crop=true;

	while( y1<y2 && IsRowTransparent(y2) )
		y2--, crop=true;

	while( y1<y2 && IsRowTransparent(y1) )
		y1++, crop=true;

	if( crop )
		Crop(x1, y1, x2+1, y2+1);
}

void DataTexture::Collapse()
{
	double r=0;
	double g=0;
	double b=0;
	double a=0;

	for( int y=0; y<height; y++ )
		for( int x=0; x<width; x++ )
		{
			vec3 col = vec3::from_rgb(data[x+y*width]);
			r += col.x;
			g += col.y;
			b += col.z;
			a += (data[x+y*width]>>24)/255.f;
		}
	int n = width * height;
	data.clear();
	data.push_back(vec3(r/n, g/n, b/n).make_rgba(a/n));
	width = 1;
	height = 1;
	off_x = 0;
	off_y = 0;
}

void DataTexture::Downscale()
{
	if( (width|height|off_x|off_y)&1 )
	{
		// Width	Off		X1		X2
		//	0		0		0		W
		//	0		1		-1		W+1
		//	1		0		0		W+1
		//	1		1		-1		W
		int x1 = -(off_x&1);
		int y1 = -(off_y&1);
		int x2 = width + ((width^off_x)&1);
		int y2 = height + ((height^off_y)&1);
		Crop(x1, y1, x2, y2);
	}

	vector<DWORD> newdata;
	for( int y=0; y<height; y+=2 )
		for( int x=0; x<width; x+=2 )
		{
			vec3 scol = vec3(0, 0, 0);
			float sa = 0;
			for( int i=0; i<2; i++ )
				for( int j=0; j<2; j++ )
				{
					vec3 col = vec3::from_rgb(data[(x+i)+(y+j)*width]);
					float a = (data[(x+i)+(y+j)*width]>>24)/255.f;
					scol += col*a;
					sa += a;
				}

			if( sa>0 )
				scol /= sa;

			newdata.push_back(scol.make_rgba(sa/4));
		}
	data.swap(newdata);
	width /= 2;
	height /= 2;
	off_x /= 2;
	off_y /= 2;
}

void DataTexture::DownscaleY()
{
	if( (width|height|off_x|off_y)&1 )
	{
		// Width	Off		X1		X2
		//	0		0		0		W
		//	0		1		-1		W+1
		//	1		0		0		W+1
		//	1		1		-1		W
		int x1 = -(off_x&1);
		int y1 = -(off_y&1);
		int x2 = width + ((width^off_x)&1);
		int y2 = height + ((height^off_y)&1);
		Crop(x1, y1, x2, y2);
	}

	vector<DWORD> newdata;
	for( int y=0; y<height; y+=2 )
		for( int x=0; x<width; x++ )
		{
			vec3 scol = vec3(0, 0, 0);
			float sa = 0;
			for( int j=0; j<2; j++ )
			{
				vec3 col = vec3::from_rgb(data[x+(y+j)*width]);
				float a = (data[x+(y+j)*width]>>24)/255.f;
				scol += col*a;
				sa += a;
			}

			if( sa>0 )
				scol /= sa;

			newdata.push_back(scol.make_rgba(sa/2));
		}
	data.swap(newdata);
	height /= 2;
	off_y /= 2;
}

void DataTexture::ColorKey(int aref, DWORD key)
{
	for( int i=0; i<data.size(); i++ )
		if( (data[i]>>24)<aref )
			data[i] = key;
}

//void DataTexture::OverlayColorKey(DataTexture &tex1, DataTexture &tex2, int x0, int y0, DWORD key)
//{
//	// <this> can be one of tex1/tex2 !!!
//	vector<DWORD> newdata;
//	int new_w = tex1.width;
//	int new_h = tex1.height;
//	int new_ox = tex1.off_x;
//	int new_oy = tex1.off_y;
//
//	// Rescale to fit tex2
//
//	// TBD
//
//}

void DataTexture::ColorMask(DWORD color, DWORD mask)
{
	color &= mask;
	for( int i=0; i<data.size(); i++ )
		if( (data[i]&mask)==color )
			data[i] = 0;
		else
			data[i] |= 0xFF000000;
}


void DataTexture::Scroll(int dx, int dy)
{
	if( width<=1 && height<=1 )
		return;

	vector<DWORD> newdata;
	dx = (dx%width+width)%width;
	dy = (dy%height+height)%height;

	for( int y=0; y<height; y++ )
		for( int x=0; x<width; x++ )
		{
			int xs = (x+dx)%width;
			int ys = (y+dy)%height;
			newdata.push_back(data[xs+ys*width]);
		}

	data.swap(newdata);
}

void DataTexture::ColorIndexRemap(vector<int> remap)
{
	for( int i=0; i<data.size(); i++ )
	{
		uint32_t c1 = uint32_t(data[i] & 0xFF);
		uint32_t c2 = uint32_t((data[i]>>8) & 0xFF);
		if( c1 < remap.size() ) c1 = uint32_t(remap[c1]) & 0xFF;
		if( c2 < remap.size() ) c2 = uint32_t(remap[c2]) & 0xFF;

		data[i] = c1 | (c2<<8) | (data[i] & 0xFFFF0000);
	}
}

bool DataTexture::IsIndexColumnTransparent(int x)
{
	for( int y=0; y<height; y++ )
		if( !COLOR_IS_TRANSPARENT(data[x+y*width]) )
			return false;
	return true;
}

bool DataTexture::IsIndexRowTransparent(int y)
{
	for( int x=0; x<width; x++ )
		if( !COLOR_IS_TRANSPARENT(data[x+y*width]) )
			return false;
	return true;
}

void DataTexture::IndexTrim()
{
	int x1=0, y1=0, x2=width-1, y2=height-1;
	bool crop = false;

	while( x1<x2 && IsIndexColumnTransparent(x2) )
		x2--, crop=true;

	while( x1<x2 && IsIndexColumnTransparent(x1) )
		x1++, crop=true;

	while( y1<y2 && IsIndexRowTransparent(y2) )
		y2--, crop=true;

	while( y1<y2 && IsIndexRowTransparent(y1) )
		y1++, crop=true;

	if( crop )
		Crop(x1, y1, x2+1, y2+1);
}

void DataTexture::IndexHorizPack()
{
	vector<DWORD> newdata;
	newdata.resize((width+1)/2*height);

	int d = 0;
	for( int y=0; y<height; y++ )
		for( int x=0; x<width; x+=2 )
		{
			DWORD code = data[x+y*width] & 0xFF;
			if( x+1 < width )
				code |= (data[(x+1)+y*width] & 0xFF)<<8;
			else
				code |= (COLOR_TRANSPARENT & 0xFF)<<8;

			newdata[d++] = code;
		}

	data.swap(newdata);
	width = (width+1)/2;
	off_x /= 2;
}

void DataTexture::IndexHorizScroll(bool wrap)
{
	if( width<=0 ) return;

	for( int y=0; y<height; y++ )
	{
		DWORD rowstart = data[0+y*width];

		for( int x=0; x<width; x++ )
		{
			DWORD c1 = data[x+y*width] & 0xFF;
			DWORD c2;

			if( x+1<width ) c2 = data[(x+1)+y*width] & 0xFF;
			else if( wrap )		 c2 = rowstart & 0xFF;
			else				 c2 = COLOR_TRANSPARENT & 0xFF;

			data[x+y*width] = c1 | (c2<<8);
		}
	}
}


// Fix modes:
//	0	- make solid
//	1	- make transparent
//	2	- use left side for entire pixel
//	3	- use right side for entire pixel
//	4	- pad with black (index 0)
void DataTexture::FixTransparency(int mode)
{
	for( int i=0; i<data.size(); i++ )
	{
		DWORD code = data[i];
		int action = mode << 4;
		if( COLOR_IS_TRANSPARENT_LOW(code) ) action |= 1;		// left
		if( COLOR_IS_TRANSPARENT_HIGH(code) ) action |= 2;		// right

		switch( action )
		{
		case 0x11:
		case 0x12:
			// become transparent
			code = COLOR_TRANSPARENT;
			break;

		case 0x02:
		case 0x21:
		case 0x22:
			// copy left -> right
			code = (code&0xFF) * 0x0101;
			break;

		case 0x01:
		case 0x31:
		case 0x32:
			// copy right -> left
			code = ((code>>8)&0xFF) * 0x0101;
			break;

		case 0x41:
			// black left
			code &= 0xFF00;
			break;

		case 0x42:
			// black right
			code &= 0x00FF;
			break;
		}

		data[i] = code;
	}
}



void DataTexture::IndexOverlay(DataTexture &tex, int x0, int y0)
{
	for( int y=0; y<tex.height; y++ )
		for( int x=0; x<tex.width; x++ )
			if( uint32_t(x+x0)<width && uint32_t(y+y0)<height )
			{
				DWORD tcol = tex.data[x + y*tex.width];
				DWORD &col = data[(x+x0) + (y+y0)*width];
				if( !COLOR_IS_TRANSPARENT_HIGH(tcol) ) col = (col & 0xFFFF00FF) | (tcol & 0xFF00);
				if( !COLOR_IS_TRANSPARENT_LOW(tcol) ) col = (col & 0xFFFFFF00) | (tcol & 0x00FF);
			}
}



void DataTexture::PrepareObjectData(BinaryWordData &bindata, ChunkyColorPacker &chunky_packer)
{
	bindata.Clear();

	vector<word> &edata = bindata.data;
	vector<byte> pixels;
	map<int, bool> &lf = bindata.newline;
	lf[0] = true;

	edata.push_back(width);			// width
	edata.push_back(off_x);			// off_x
	edata.push_back(0);				// multisprite pointer (HI)
	edata.push_back(0);				// multisprite pointer (LO)
	int col_table = edata.size()+1;
	edata.resize(edata.size()+width+2);	// reserve <width>+2 column entries
	lf[col_table-1]=true;
	lf[col_table]=true;
	lf[col_table+width]=true;
	lf[col_table+width+1]=true;

	// guards
	edata[col_table-1] = edata.size()*2;
	edata[col_table+width] = edata.size()*2;
	edata.push_back(0);

	// spans
	for( int x=0; x<width; x++ )
	{
		lf[edata.size()] = true;
		edata[col_table+x] = edata.size()*2 - 10;	// fill address

		bool is_span = false;
		for( int y=0; y<=height; y++ )
		{
			DWORD col = (y<height) ? data[x+y*width] : COLOR_TRANSPARENT;			// get object gfx data (plus one transparent pixel at the end)
			if( !COLOR_IS_TRANSPARENT(col) )										// check object gfx transparency
			{
				// Solid pixel
				if( !is_span )
				{
					// Start span
					int dy = y - off_y;
					edata.push_back(pixels.size() + 0x4000 - dy);
					edata.push_back(dy<<6);
					is_span = true;
				}
				pixels.push_back(chunky_packer.Process(col));
			}
			else if( is_span )
			{
				// End span
				edata.push_back((y-off_y)<<6);
				is_span = false;
			}
		}
		edata.push_back(0);	// No more spans
	}

	// fixup pixel addresses
	word pixbase = edata.size()*2;
	lf[pixbase/2] = true;
	for( int x=0; x<width; x++ )
	{
		int addr = edata[col_table+x] + 10;
		while( edata[addr>>1] )
		{
			edata[addr>>1] += pixbase - 0x4000;
			addr += 6;
		}
	}

	// move pixels to data
	if( pixels.size()&1 ) pixels.push_back(0);
	for( int i=0; i<pixels.size(); i+=2 )
		edata.push_back(pixels[i]*256 + pixels[i+1]);
}

word DataTexture::GatherBitplaneWord(int xp, int yp, int bitplane)
{
	if( uint32_t(yp)>=height )
		return 0;

	word value = 0;
	for( int b=0; b<16; b++ )
		if( uint32_t(xp+b)<width )
		{
			DWORD col = data[(xp+b)+width*yp];
			if( !COLOR_IS_TRANSPARENT(col) )
				if( bitplane<0 || (col & (1<<bitplane)) )
					value |= 0x8000>>b;
		}

	return value;
}
