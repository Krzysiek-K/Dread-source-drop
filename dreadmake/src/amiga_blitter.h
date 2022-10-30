
#ifndef _AMIGA_BLITTER_H
#define _AMIGA_BLITTER_H



typedef struct _Plane {
	word	*base;
	word	stride;	// in words
} Plane;

#define BLIT_FUNC_COPY		0x9F0
#define BLIT_FUNC_OR		0xBFA
#define BLIT_FUNC_CUT		0xB0A

extern word blit_func;


void BlitClear(word *dst);
void BlitFill(word *dst, word bltsize, word data);
void Blit_Run(Plane *src, short sx, short sy, word w, word h, Plane *dst, short dx, short dy);

void Blit_Print32(word *font, word *dst, word dst_stride_words, word xp, word yp, const char *text);
void Blit_Print32_Center(word *font, word *dst, word dst_stride_words, word xp, word yp, const char *text);
void Blit_Image(word *img, word bitplane, word *dst, word dst_stride_words, short dx, short dy);

void Blit_Image_CpuCopy(word *img, word bitplane, word *dst, word dst_stride_words, short dx, short dy);
void Blit_Image_CpuCopyInvert(word *img, word bitplane, word *dst, word dst_stride_words, short dx, short dy);




#endif
