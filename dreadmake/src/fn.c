
#include "demo.h"

short sincos_fix14[256];		// MEM:	<1k	init

static char itoa_buff[12];
static char time_buff[32];

// Parsing C : /LocalProjects/_amiga/dreadmake/out/Dread.exe :
// 	Hunk 0 is code, 39360 bytes
// 	Hunk 1 is code, 352660 bytes
// 	Hunk 2 is code, 348 bytes
// 	Hunk 3 is chip, 46656 bytes
// 	Hunk 4 is code, 198816 bytes
// 	Hunk 5 is chip, 13100 bytes
// 	Code : 591184 bytes
// 	Chip : 59756 bytes

void init_sincos()
{
	int cos = 1<<14;
	int sin = 0;
	for( int i=0; i<64; i++ )
	{
		sincos_fix14[i] = (short)cos;
		sincos_fix14[i+64] = (short)-sin;
		sincos_fix14[i+128] = (short)-cos;
		sincos_fix14[i+192] = (short)sin;
		sin += (cos*1608)>>16;
		cos -= (sin*1608)>>16;
	}
}



const char *_itoa(int value)
{
	char *bpos = itoa_buff + (sizeof(itoa_buff) - 1);
	bpos[0] = 0;

	word sign = (value<0);
	if( sign )
		value = -value;

	do {
		*--bpos = value%10 + '0';
		value /= 10;
	} while( value );

	if( sign )
		*--bpos = '-';

	return bpos;
}

const char *_itoa_hex(dword value)
{
	char *bpos = itoa_buff + (sizeof(itoa_buff) - 1);
	bpos[0] = 0;

	do {
		*--bpos = "0123456789ABCDEF"[value&15];
		value >>= 4;
	} while( value );

	*--bpos = '$';

	return bpos;
}

char *_append_s(char *s, const char *text)
{
	while( *text )
		*s++ = *text++;
	*s = 0;
	return s;
}


// "x:xx.xx" time from 100s
const char *_time_100s_a(int time)
{
	char *s = time_buff;
	if( time<0 )
	{
		*s++ = '-';
		time = -time;
	}

	word s100 = (word)(time%100);
	time /= 100;
	word sec = (word)(time%60);
	time /= 60;

	s = _append_s(s, _itoa(time));
	*s++ = ':';
	if( sec<10 ) *s++ = '0';
	s = _append_s(s, _itoa(sec));
	*s++ = '.';
	if( s100<10 ) *s++ = '0';
	s = _append_s(s, _itoa(s100));

	return time_buff;
}
