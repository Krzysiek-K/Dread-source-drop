

#include "demo.h"


#define NUM_TRAMPOLINES		4
#define COP_TRAMP_WORDS		6



__chip word BLANK_COPPER[8];
__chip word WHITE_COPPER[8];

word BLANK_COPPER_DATA[] = {
	0x0100,	0x0201,		// no bitplanes
	0x0096,	0x0020,		// no sprites
	0x0180, 0x0000,		// black color 0
	0xFFFF, 0xFFFE		// end
};
word WHITE_COPPER_DATA[] = {
	0x0100,	0x0201,		// no bitplanes
	0x0096,	0x0020,		// no sprites
	0x0180, 0x0FFF,		// white color 0
	0xFFFF, 0xFFFE		// end
};


word *cop_write;
word cop_write_y;


__chip word copper_trapolines[(NUM_TRAMPOLINES*2-1)*COP_TRAMP_WORDS];
word *copper_tramp_base;
word copper_use_tramp;




void SetCopper(__a0 word *coplist)
{
	if( ++copper_use_tramp >= NUM_TRAMPOLINES )
		copper_use_tramp = 0;

	volatile word *tramp = copper_tramp_base + copper_use_tramp*COP_TRAMP_WORDS;
	tramp[1] = HIWORD(coplist);
	tramp[3] = LOWORD(coplist);
	COP1LC = tramp;
}


void Init_Copper(void)
{
	// init blank coppers
	for(int i=0;i<8;i++) BLANK_COPPER[i] = BLANK_COPPER_DATA[i];
	for(int i=0;i<8;i++) WHITE_COPPER[i] = WHITE_COPPER_DATA[i];

	copper_tramp_base = copper_trapolines;
	if( HIWORD( copper_tramp_base ) != HIWORD( copper_tramp_base+(NUM_TRAMPOLINES-1)*COP_TRAMP_WORDS ) )
		copper_tramp_base = copper_trapolines + (sizeof(copper_trapolines)/sizeof(copper_trapolines[0]) - NUM_TRAMPOLINES*COP_TRAMP_WORDS);

	word *p = copper_tramp_base;
	for(int i=0;i<NUM_TRAMPOLINES;i++)
	{
		*p++ = 0x084;						// COP2LCH   (*(volatile word*)0xdff084)
		*p++ = HIWORD( BLANK_COPPER );
		*p++ = 0x086;						// COP2LCL   (*(volatile word*)0xdff086)
		*p++ = LOWORD( BLANK_COPPER );
		*p++ = 0x08A;						// COPJMP2   (*(volatile word*)0xdff08A)
		*p++ = 0;
	}

	COP1LC = copper_tramp_base;
	copper_use_tramp = 0;
}
