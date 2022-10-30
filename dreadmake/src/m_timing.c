
#include "demo.h"


#if MULTIPLAYER_MODE
	#define DEBUG_VAR			(((word)e_globals.mp_frags << 8) | (byte)e_globals.mp_deaths)
	#define DEBUG_SHOW_HEX		1
#elif WALL_ORDER_DEBUG == 1
	#define DEBUG_VAR			( debug_vars[2] )
	#define DEBUG_SHOW_HEX		0
#else
	//#define DEBUG_VAR			e_globals.debug_var
	//#define DEBUG_VAR			opt_ntsc
	//#define DEBUG_VAR			e_n_things
	//#define DEBUG_VAR			(e_delthings_end - e_delthings)
	//#define DEBUG_VAR			(e_visthings_end - e_visthings)
	#define DEBUG_SHOW_HEX		0
#endif


dword demo_frame;
dword tick_total;
int tick_frame;
dword tick_count[10];
word tick_show;
short tick_visfilter;


const byte NUMFONT[] ={
#if !DEBUG_SHOW_HEX
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
#else
	2,	2,	2,	7,	5,	7,	7,	7,	7,	7,	2,	6,	2,	6,	7,	7,	//	-#-	-#-	-#-	###	#-#	###	###	###	###	###	-#-	##-	-#-	##-	###	###
	5,	6,	5,	1,	5,	4,	4,	1,	5,	5,	5,	5,	5,	5,	4,	4,	//	#-#	##-	#-#	--#	#-#	#--	#--	--#	#-#	#-#	#-#	#-#	#-#	#-#	#--	#--
	5,	2,	1,	3,	7,	7,	7,	1,	7,	7,	7,	6,	4,	5,	6,	6,	//	#-#	-#-	--#	-##	###	###	###	--#	###	###	###	##-	#--	#-#	##-	##-
	5,	2,	2,	1,	1,	1,	5,	2,	5,	1,	5,	5,	5,	5,	4,	4,	//	#-#	-#-	-#-	--#	--#	--#	#-#	-#-	#-#	--#	#-#	#-#	#-#	#-#	#--	#--
	2,	2,	7,	7,	1,	7,	7,	2,	7,	7,	5,	6,	2,	6,	7,	4,	//	-#-	-#-	###	###	--#	###	###	-#-	###	###	#-#	##-	-#-	##-	###	#--
#endif
};



void MarkFrame(void)
{
#ifdef SHOW_TICKS
	//word in = rawintnum;
	//tick_total = in - tick_count[tick_frame];
	//tick_count[tick_frame] = in;
	//if( ++tick_frame>=10 )
	//	tick_frame=0;
	//word tt = tick_total;


	static word _ticks;
	static word _ints;
	
	// Get ticks
	word ticks = GetCIAA_TimerA();
	dword tt = (word)(_ticks - ticks);		// reverse, because it's downcounting
	_ticks = ticks;

	// Get interrupts
	word ints = rawintnum;
	dword ii = (dword)(word)(ints-_ints) * 14186;		// 141.8758 * 100;
	_ints = ints;

	// Fix higher word of the ticks
	{
		dword t1 = (ii & 0xFFFF0000) | tt;
		if( ii >= t1 )
		{
			dword t2 = t1 + 0x10000;
			dword e1 = ii - t1;
			dword e2 = t2 - ii;
			tt = (e1<=e2) ? t1 : t2;
		}
		else if( t1 >= 0x10000 )
		{
			dword t2 = t1 - 0x10000;
			dword e1 = t1 - ii;
			dword e2 = ii - t2;
			tt = (e1<=e2) ? t1 : t2;
		}
		// else - tt is already OK
	}

	//if( tt<250*142 && cpu_type<20 ) tt += 0x10000;		// with all due respect for 68000/010, I don't believe that ;)
	//if( tt>999*142 ) tt = 999*142;

	tick_total -= tick_count[tick_frame];
	tick_total += tt;
	tick_count[tick_frame] = tt;
	if( ++tick_frame>=10 )
		tick_frame=0;
	int t1 = (tick_total+1419/2)/1419;		// 141.8758 * 10
	if( t1>9999 ) t1 = 9999;
	tt = (word)t1;

	if( tt < tick_show )
	{
		if( --tick_visfilter <= -3 )
		{
			tick_show = tt;
			tick_visfilter = -3;
		}
	}
	else if( tt > tick_show )
	{
		if( ++tick_visfilter >= 3 )
		{
			tick_show = tt;
			tick_visfilter = 3;
		}
	}
	tt = tick_show;

#ifndef DEBUG_VAR
	if( tt <= 999 )
	{
		const byte *D3 = NUMFONT + tt%10*6;		tt/=10;
		const byte *D2 = NUMFONT + tt%10*6;		tt/=10;
		const byte *D1 = NUMFONT + tt*6;

		// ----------------
		// ####_._####_####
		TICK_SHOW[2] = (((word)*D1++)<<12) | (((word)*D2++)<<5) | (((word)*D3++));
		TICK_SHOW[4] = (((word)*D1++)<<12) | (((word)*D2++)<<5) | (((word)*D3++));
		TICK_SHOW[6] = (((word)*D1++)<<12) | (((word)*D2++)<<5) | (((word)*D3++));
		TICK_SHOW[8] = (((word)*D1++)<<12) | (((word)*D2++)<<5) | (((word)*D3++));
		TICK_SHOW[10] = (((word)*D1++)<<12) | (((word)*D2++)<<5) | (((word)*D3++));
		TICK_SHOW[12] = (((word)*D1++)<<12) | (((word)*D2++)<<5) | (((word)*D3++)) | 0x0400;
	}
	else
	{
		tt = (tt+5)/10;
		const byte *D3 = NUMFONT + tt%10*6;		tt/=10;
		const byte *D2 = NUMFONT + tt%10*6;		tt/=10;
		const byte *D1 = NUMFONT + tt*6;

		// ----------------
		// ####_####_._####
		TICK_SHOW[2] = (((word)*D1++)<<12) | (((word)*D2++)<<7) | (((word)*D3++));
		TICK_SHOW[4] = (((word)*D1++)<<12) | (((word)*D2++)<<7) | (((word)*D3++));
		TICK_SHOW[6] = (((word)*D1++)<<12) | (((word)*D2++)<<7) | (((word)*D3++));
		TICK_SHOW[8] = (((word)*D1++)<<12) | (((word)*D2++)<<7) | (((word)*D3++));
		TICK_SHOW[10] = (((word)*D1++)<<12) | (((word)*D2++)<<7) | (((word)*D3++));
		TICK_SHOW[12] = (((word)*D1++)<<12) | (((word)*D2++)<<7) | (((word)*D3++)) | 0x0020;
	}
#else
	tt = DEBUG_VAR;

#if !DEBUG_SHOW_HEX
	const byte *D3 = NUMFONT + tt%10*6;		tt/=10;
	const byte *D2 = NUMFONT + tt%10*6;		tt/=10;
	const byte *D1 = NUMFONT + tt%10*6;		tt/=10;

	// ----------------
	// #_####_####_####
	TICK_SHOW[ 2] = (tt>=1 ? 0x8000 : 0x0000) | (((word)*D1++)<<10) | (((word)*D2++)<<5) | (((word)*D3++));
	TICK_SHOW[ 4] = (tt>=2 ? 0x8000 : 0x0000) | (((word)*D1++)<<10) | (((word)*D2++)<<5) | (((word)*D3++));
	TICK_SHOW[ 6] = (tt>=3 ? 0x8000 : 0x0000) | (((word)*D1++)<<10) | (((word)*D2++)<<5) | (((word)*D3++));
	TICK_SHOW[ 8] = (tt>=4 ? 0x8000 : 0x0000) | (((word)*D1++)<<10) | (((word)*D2++)<<5) | (((word)*D3++));
	TICK_SHOW[10] = (tt>=5 ? 0x8000 : 0x0000) | (((word)*D1++)<<10) | (((word)*D2++)<<5) | (((word)*D3++));
	TICK_SHOW[12] = (tt>=6 ? 0x8000 : 0x0000) | (((word)*D1++)<<10) | (((word)*D2++)<<5) | (((word)*D3++));
#else
	const byte *H3 = NUMFONT + ((tt>>12)&0xF);
	const byte *H2 = NUMFONT + ((tt>> 8)&0xF);
	const byte *H1 = NUMFONT + ((tt>> 4)&0xF);
	const byte *H0 = NUMFONT + ((tt    )&0xF);
	TICK_SHOW[ 2] = (H3[ 0]<<13) | (H2[ 0]<<9) | (H1[ 0]<<5) | (H0[ 0]<<1);
	TICK_SHOW[ 4] = (H3[16]<<13) | (H2[16]<<9) | (H1[16]<<5) | (H0[16]<<1);
	TICK_SHOW[ 6] = (H3[32]<<13) | (H2[32]<<9) | (H1[32]<<5) | (H0[32]<<1);
	TICK_SHOW[ 8] = (H3[48]<<13) | (H2[48]<<9) | (H1[48]<<5) | (H0[48]<<1);
	TICK_SHOW[10] = (H3[64]<<13) | (H2[64]<<9) | (H1[64]<<5) | (H0[64]<<1);
	TICK_SHOW[12] = 0;
#endif

#endif

#endif
}
