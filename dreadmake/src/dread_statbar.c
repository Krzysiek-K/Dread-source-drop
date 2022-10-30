
#include "demo.h"


extern __chip word GFX_STBAR0[];

extern __chip word GFX_STTNUM0[];
extern __chip word GFX_STTNUM1[];
extern __chip word GFX_STTNUM2[];
extern __chip word GFX_STTNUM3[];
extern __chip word GFX_STTNUM4[];
extern __chip word GFX_STTNUM5[];
extern __chip word GFX_STTNUM6[];
extern __chip word GFX_STTNUM7[];
extern __chip word GFX_STTNUM8[];
extern __chip word GFX_STTNUM9[];
extern __chip word GFX_STTPRCNT[];
extern __chip word GFX_STYSNUM0[];
extern __chip word GFX_STYSNUM1[];
extern __chip word GFX_STYSNUM2[];
extern __chip word GFX_STYSNUM3[];
extern __chip word GFX_STYSNUM4[];
extern __chip word GFX_STYSNUM5[];
extern __chip word GFX_STYSNUM6[];
extern __chip word GFX_STYSNUM7[];
extern __chip word GFX_STYSNUM8[];
extern __chip word GFX_STYSNUM9[];
extern __chip word GFX_STGNUM0[];
extern __chip word GFX_STGNUM1[];
extern __chip word GFX_STGNUM2[];
extern __chip word GFX_STGNUM3[];
extern __chip word GFX_STGNUM4[];
extern __chip word GFX_STGNUM5[];
extern __chip word GFX_STGNUM6[];
extern __chip word GFX_STGNUM7[];
extern __chip word GFX_STGNUM8[];
extern __chip word GFX_STGNUM9[];
extern __chip word GFX_STKEYS0[];
extern __chip word GFX_STKEYS1[];
extern __chip word GFX_STKEYS2[];
//extern __chip word GFX_STKEYS3[];
//extern __chip word GFX_STKEYS4[];
//extern __chip word GFX_STKEYS5[];



const word *STAT_DIGITS[] = {
	NULL,
	GFX_STTNUM0+3,			//	1
	GFX_STTNUM1+3,
	GFX_STTNUM2+3,
	GFX_STTNUM3+3,
	GFX_STTNUM4+3,
	GFX_STTNUM5+3,
	GFX_STTNUM6+3,
	GFX_STTNUM7+3,
	GFX_STTNUM8+3,
	GFX_STTNUM9+3,
	GFX_STTPRCNT+3,			// 11
	GFX_STYSNUM0+3,			// 12
	GFX_STYSNUM1+3,
	GFX_STYSNUM2+3,
	GFX_STYSNUM3+3,
	GFX_STYSNUM4+3,
	GFX_STYSNUM5+3,
	GFX_STYSNUM6+3,
	GFX_STYSNUM7+3,
	GFX_STYSNUM8+3,
	GFX_STYSNUM9+3,
	GFX_STGNUM0+3,			// 22
	GFX_STGNUM1+3,
	GFX_STGNUM2+3,
	GFX_STGNUM3+3,
	GFX_STGNUM4+3,
	GFX_STGNUM5+3,
	GFX_STGNUM6+3,
	GFX_STGNUM7+3,
	GFX_STGNUM8+3,
	GFX_STGNUM9+3,
	GFX_STKEYS0+3,			// 32
	GFX_STKEYS1+3,
	GFX_STKEYS2+3,
//	GFX_STKEYS3+3,
//	GFX_STKEYS4+3,
//	GFX_STKEYS5+3,
};
