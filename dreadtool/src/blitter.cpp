
#include "common.h"


void Blitter::InitDefaults()
{
	pool_a.SetMem(NULL,0);
	pool_b.SetMem(NULL,0);
	pool_c.SetMem(NULL,0);
	pool_d.SetMem(NULL,0);
	cycle_counter = 0;

	BLTCON0 = 0;
	BLTCON1 = 0;
	BLTAFWM = 0xFFFF;
	BLTALWM = 0xFFFF;
	BLTCPT = 0;
	BLTBPT = 0;
	BLTAPT = 0;
	BLTDPT = 0;
	BLTSIZE = 0;
	BLTCMOD = 0;
	BLTBMOD = 0;
	BLTAMOD = 0;
	BLTDMOD = 0;
	BLTCDAT = 0;
	BLTBDAT = 0;
	BLTADAT = 0;
}



void Blitter::Run()
{
	int height = BLTSIZE >> 6;
	int width  = BLTSIZE & 63;	// in words
	if(!width) width = 64;
	BLTSIZE = 0;

	int ashift = BLTCON0>>BC0_ASH_SHIFT;
	int bshift = BLTCON1>>BC1_BSH_SHIFT;
	word sparebits_a = 0;
	word sparebits_b = 0;
	for(int y=0;y<height;y++)
	{
		for(int x=0;x<width;x++)
		{
			if( BLTCON0 & BC0_USEA )
			{
				BLTADAT = pool_a.Read( BLTAPT );
				BLTAPT += 2;
				word tmp = (BLTADAT>>ashift) | sparebits_a;
				sparebits_a = int(BLTADAT)<<(16-ashift);
				BLTADAT = tmp;
				if(x==0) BLTADAT &= BLTAFWM;
				if(x==width-1) BLTADAT &= BLTALWM;
			}
			if( BLTCON0 & BC0_USEB )
			{
				BLTBDAT = pool_b.Read( BLTBPT );
				BLTBPT += 2;
				word tmp = (BLTBDAT>>bshift) | sparebits_b;
				sparebits_b = int(BLTBDAT)<<(16-bshift);
				BLTBDAT = tmp;

				cycle_counter += 2;
			}
			if( BLTCON0 & BC0_USEC )
			{
				BLTCDAT = pool_c.Read( BLTCPT );
				BLTCPT += 2;
			}

			// fill BLTDDAT
			word BLTDDAT = 0;
			for(int b=0;b<16;b++)
			{
				int mint = 0;
				int bit = 1<<b;
				if( BLTADAT & bit ) mint |= 4;
				if( BLTBDAT & bit ) mint |= 2;
				if( BLTCDAT & bit ) mint |= 1;
				if( BLTCON0 & (1<<mint) )
					BLTDDAT |= bit;
			}

			if( BLTCON0 & BC0_USED )
			{
				pool_d.Write( BLTDPT, BLTDDAT );
				BLTDPT += 2;
			}

			cycle_counter += 4;
			if( (BLTCON0 & (BC0_USEC|BC0_USED)) == (BC0_USEC|BC0_USED) )
				cycle_counter += 2;
		}

		if( BLTCON0 & BC0_USEA )	BLTAPT += BLTAMOD;
		if( BLTCON0 & BC0_USEB )	BLTBPT += BLTBMOD;
		if( BLTCON0 & BC0_USEC )	BLTCPT += BLTCMOD;
		if( BLTCON0 & BC0_USED )	BLTDPT += BLTDMOD;
	}
}
