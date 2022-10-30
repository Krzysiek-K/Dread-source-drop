
#include "demo.h"

#if DEBUG_CONSOLE
__chip word DEBUG_CONSOLE_SCREEN[20*8*DEBUG_CONSOLE];
#endif



const byte HEXFONT[] ={
	0x6,	//	-##-
	0x9,	//	#--#
	0x9,	//	#--#
	0x9,	//	#--#
	0x9,	//	#--#
	0x6,	//	-##-

	0x2,	//	--#-
	0x6,	//	-##-
	0x2,	//	--#-
	0x2,	//	--#-
	0x2,	//	--#-
	0x7,	//	-###

	0x6,	//	-##-
	0x9,	//	#--#
	0x1,	//	---#
	0x2,	//	--#-
	0x4,	//	-#--
	0xF,	//	####

	0x6,	//	-##-
	0x9,	//	#--#
	0x2,	//	--#-
	0x1,	//	---#
	0x9,	//	#--#
	0x6,	//	-##-

	0x3,	//	--##
	0x5,	//	-#-#
	0x9,	//	#--#
	0xF,	//	####
	0x1,	//	---#
	0x1,	//	---#

	0xF,	//	####
	0x8,	//	#---
	0xE,	//	###-
	0x1,	//	---#
	0x9,	//	#--#
	0x6,	//	-##-

	0x7,	//	-###
	0x8,	//	#---
	0xE,	//	###-
	0x9,	//	#--#
	0x9,	//	#--#
	0x6,	//	-##-

	0xF,	//	####
	0x1,	//	---#
	0x1,	//	---#
	0x2,	//	--#-
	0x2,	//	--#-
	0x2,	//	--#-

	0x6,	//	-##-
	0x9,	//	#--#
	0x6,	//	-##-
	0x9,	//	#--#
	0x9,	//	#--#
	0x6,	//	-##-

	0x6,	//	-##-
	0x9,	//	#--#
	0x9,	//	#--#
	0x7,	//	-###
	0x1,	//	---#
	0x6,	//	-##-

	0x6,	//	-##-
	0x9,	//	#--#
	0x9,	//	#--#
	0xF,	//	####
	0x9,	//	#--#
	0x9,	//	#--#

	0xE,	//	###-
	0x9,	//	#--#
	0xE,	//	###-
	0x9,	//	#--#
	0x9,	//	#--#
	0xE,	//	###-

	0x7,	//	-###
	0x8,	//	#---
	0x8,	//	#---
	0x8,	//	#---
	0x8,	//	#---
	0x7,	//	-###

	0xE,	//	###-
	0x9,	//	#--#
	0x9,	//	#--#
	0x9,	//	#--#
	0x9,	//	#--#
	0xE,	//	###-

	0xF,	//	####
	0x8,	//	#---
	0xF,	//	####
	0x8,	//	#---
	0x8,	//	#---
	0xF,	//	####

	0xF,	//	####
	0x8,	//	#---
	0xF,	//	####
	0x8,	//	#---
	0x8,	//	#---
	0x8,	//	#---
};


void debug_write_hexbuff(word *screenpos, int len, byte *data)
{
	word bitmap[6];

	while( len>0 )
	{
		const byte *D1 = HEXFONT + (*data>>4)*6;
		const byte *D2 = HEXFONT + (*data&0x0F)*6;

		// ####-####-------
		bitmap[0] = (((word)*D1++)<<12) | (((word)*D2++)<<7);
		bitmap[1] = (((word)*D1++)<<12) | (((word)*D2++)<<7);
		bitmap[2] = (((word)*D1++)<<12) | (((word)*D2++)<<7);
		bitmap[3] = (((word)*D1++)<<12) | (((word)*D2++)<<7);
		bitmap[4] = (((word)*D1++)<<12) | (((word)*D2++)<<7);
		bitmap[5] = (((word)*D1++)<<12) | (((word)*D2++)<<7);

		for( int y=0; y<6; y++ )
		{
			word *dst = screenpos + 80*y;
			dst[0] = 0;
			dst[1] = bitmap[y];
			dst[2] = bitmap[y];
			dst[3] = 0;
		}

		len--;
		data++;
		screenpos += 4;
	}
}

void debug_write_dec(word *screenpos, byte value)
{
	const byte *D1 = HEXFONT + (value/100)*6;
	const byte *D2 = HEXFONT + (value/10%10)*6;
	const byte *D3 = HEXFONT + (value%10)*6;
	word bitmap[6];

	// -####-####-####-
	if( value >= 100 )
	{
		bitmap[0] = (((word)*D1++)<<11) | (((word)*D2++)<<6) | (((word)*D3++)<<1);
		bitmap[1] = (((word)*D1++)<<11) | (((word)*D2++)<<6) | (((word)*D3++)<<1);
		bitmap[2] = (((word)*D1++)<<11) | (((word)*D2++)<<6) | (((word)*D3++)<<1);
		bitmap[3] = (((word)*D1++)<<11) | (((word)*D2++)<<6) | (((word)*D3++)<<1);
		bitmap[4] = (((word)*D1++)<<11) | (((word)*D2++)<<6) | (((word)*D3++)<<1);
		bitmap[5] = (((word)*D1++)<<11) | (((word)*D2++)<<6) | (((word)*D3++)<<1);
	}
	else if( value>= 10 )
	{
		bitmap[0] = (((word)*D2++)<<6) | (((word)*D3++)<<1);
		bitmap[1] = (((word)*D2++)<<6) | (((word)*D3++)<<1);
		bitmap[2] = (((word)*D2++)<<6) | (((word)*D3++)<<1);
		bitmap[3] = (((word)*D2++)<<6) | (((word)*D3++)<<1);
		bitmap[4] = (((word)*D2++)<<6) | (((word)*D3++)<<1);
		bitmap[5] = (((word)*D2++)<<6) | (((word)*D3++)<<1);
	}
	else
	{
		bitmap[0] = (((word)*D3++)<<1);
		bitmap[1] = (((word)*D3++)<<1);
		bitmap[2] = (((word)*D3++)<<1);
		bitmap[3] = (((word)*D3++)<<1);
		bitmap[4] = (((word)*D3++)<<1);
		bitmap[5] = (((word)*D3++)<<1);
	}

	for( int y=0; y<6; y++ )
	{
		word *dst = screenpos + 80*y;
		dst[0] = 0;
		dst[1] = bitmap[y];
		dst[2] = bitmap[y];
		dst[3] = 0;
	}
}
