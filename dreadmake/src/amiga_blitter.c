
#include "demo.h"


//#include "gfx/font32x32.inc"

word blit_func;




void BlitClear(word *dst)
{
	BlitWait();
	BLTCON1 = 0;
	BLTCON0 = 0x0100;
	BLTDPT = dst;
	BLTDMOD = 0;
	BLTSIZE = (256<<6) | 22;
	BlitWait();
}

void BlitFill(word *dst, word bltsize, word data)
{
	BlitWait();

	BLTAFWM = 0xFFFF;
	BLTALWM = 0xFFFF;
	BLTCON0 = 0x01F0;
	BLTCON1 = 0;

	BLTADAT = data;
	BLTDPT  = dst;
	BLTDMOD = 0;
	BLTSIZE = bltsize;

	BlitWait();
}


//void Blit_InstallFont32x32(word *dst)
//{
//	for(int i=0;i<sizeof(FONT_32x32)/2;i++)
//		dst[i] = FONT_32x32[i];
//}

void Blit_Run(Plane *src, short sx, short sy, word w, word h, Plane *dst, short dx, short dy)
{
	word *src_addr = src->base + (sx>>4) + sy*src->stride;
	word *dst_addr = dst->base + (dx>>4) + dy*dst->stride;
	sx &= 0xF;
	dx &= 0xF;

	word func = blit_func;
	word width = w>>4;
	w &= 0xF;

	BlitWait();
	// assert( !sx )	- source X aligned to 16 pixels
	// assert( !w )		- width is multiply of 16 pixels
	BLTAFWM = 0xFFFF;
	BLTALWM = 0xFFFF;
	if( dx )
	{
		func |= dx<<12;
		width++;
		BLTALWM = 0;
	}

	BLTCON0 = func;
	BLTCON1 = 0;
	BLTAPT = src_addr;
	BLTCPT = dst_addr;
	BLTDPT = dst_addr;
	BLTAMOD = (src->stride - width)<<1;
	BLTCMOD = (dst->stride - width)<<1;
	BLTDMOD = (dst->stride - width)<<1;

	BLTSIZE = (h<<6) | width;
	BlitWait();
}

// it's really 22x26
void Blit_Print32(word *font, word *dst, word dst_stride_words, word xp, word yp, const char *text)
{
	xp -= 3;
	yp -= 3;

	Plane fontp, dstp;
	fontp.base = font+2;
	fontp.stride = 16;
	dstp.base = dst;
	dstp.stride = dst_stride_words;

	while( *text )
	{
		char c = *text++;
		word index = 0xFFFF;
		if( c>='0' && c<='9' ) index = c-'0';
		if( c>='A' && c<='Z' ) index = c-('A'-10);
		if( index!=0xFFFF )
		{
			word fx = (index&7)<<5;
			word fy = ((index>>3)<<5)+3;

			Blit_Run(&fontp, fx, fy, 32, 26, &dstp, xp, yp);
		}
		xp += 24;
	}
}

void Blit_Print32_Center(word *font, word *dst, word dst_stride_words, word xp, word yp, const char *text)
{
	const char *s = text;
	while( *s++ ) xp -= 12;
	xp--;
	yp -= 13;
	Blit_Print32(font, dst, dst_stride_words, xp, yp, text);
}


void Blit_Image(word *img, word bitplane, word *dst, word dst_stride_words, short dx, short dy)
{
	word w = *img++;
	word h = *img++;
	img += bitplane * ((w>>4)*h);
	Plane srcp, dstp;
	srcp.base = img;
	srcp.stride = w>>4;
	dstp.base= dst;
	dstp.stride = dst_stride_words;
	Blit_Run(&srcp, 0, 0, w, h, &dstp, dx, dy);
}

// image width and <dx> must be multiplies of 16
void Blit_Image_CpuCopy(word *img, word bitplane, word *dst, word dst_stride_words, short dx, short dy)
{
	word w = *img++ >> 4;
	word h = *img++;
	img += bitplane * (w*h);
	dx >>= 4;
	dst += dx + dy*dst_stride_words;

	word stride = dst_stride_words-w;
	for( int y=0; y<h; y++ )
	{
		for( int x=0; x<w; x++ )
			*dst++ = *img++;
		dst += stride;
	}

	//CopyRect_Cpu( img, 0, dst, (dst_stride_words-w)<<1, w, h );
}

// image width and <dx> must be multiplies of 16
void Blit_Image_CpuCopyInvert(word *img, word bitplane, word *dst, word dst_stride_words, short dx, short dy)
{
	word w = *img++ >> 4;
	word h = *img++;
	img += bitplane * (w*h);
	dx >>= 4;
	dst += dx + dy*dst_stride_words;

	word stride = dst_stride_words-w;
	for( int y=0; y<h; y++ )
	{
		for( int x=0; x<w; x++ )
			*dst++ = ~*img++;
		dst += stride;
	}

	//CopyRect_CpuInvert( img, 0, dst, (dst_stride_words-w)<<1, w, h );
}
