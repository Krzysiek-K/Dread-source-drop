#include <stdio.h>
#include <stdlib.h>

short sincos_fix14[256];		// MEM:	<1k	init

int swing_dx[256];
int swing_dy[256];

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


int main()
{
    init_sincos();

    for (int anim = 0; anim < 256; anim++)
    {
        swing_dx[anim] = (sincos_fix14[(anim + 64) & 0xFF] >> 12);
	    swing_dy[anim] = (sincos_fix14[(uint8_t)((anim<<1)+64)]>>12) + 20;
    }

    int anim = 0;
    while(anim<256)
    {
        int start = anim;
        while (anim < 256 && swing_dx[anim] == swing_dx[start] && swing_dy[anim] == swing_dy[start])
            anim++;

        if (anim - start < 12)
            anim = start + 12;
        if( anim > 256 ) anim = 256;

        if (start == 0)
            printf("\t\t\tframe %3d\tSP_PISGA0\t%3d %3d\n", (anim - start) * 2, swing_dx[start], swing_dy[start]);
        else
            printf("\t\t\tframe %3d\t*\t\t\t%3d %3d\n", (anim - start) * 2, swing_dx[start], swing_dy[start]);
    }

 //   getc(stdin);
    return 0;
}