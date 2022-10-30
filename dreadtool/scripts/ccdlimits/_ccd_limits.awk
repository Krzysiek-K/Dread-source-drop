

#	A <= 4096		max line length 512*8
#	B <= 4096		-
#	tdy < tdx		assured by clip space flips
#	v1x <= v2x		assured by line case
#	v1y >= v2y		-
#	v1x <= tdx		SHOULD be assured by clipping
#	v2y <= tdy		-
#
#
#	(A*v1x + B*v1y) / (A + tdy*B/tdx + 1)		<<< Prove it does not overflow!
#


function dump()
{
	printf("A = %d\n", A);
	printf("B = %d\n", B);
	printf("C = %d\n", C);
	printf("div = %d\n", div);
}

function init()
{
	#v1x=  7492;  v1y=  6070;  v2x= 10796;  v2y=  2339;  tdx=  7540;  tdy=  2533;
	#v1x=  7891;  v1y=  5656;  v2x= 11792;  v2y=  1656;  tdx=  7982;  tdy=  1680;
	#v1x=  7946;  v1y=  4703;  v2x= 12041;  v2y=   658;  tdx=  7953;  tdy=   718;
	#v1x=  7929;  v1y=  3808;  v2x= 11999;  v2y=   138;  tdx=  7934;  tdy=   159;
	# v1x=  7997;  v1y=  3885;  v2x= 12092;  v2y=    71;  tdx=  7999;  tdy=    75;
	#v1x=  7997;  v1y=  3971;  v2x= 12089;  v2y=    22;  tdx=  7999;  tdy=    25;
	#v1x=  7994;  v1y=  3982;  v2x= 12081;  v2y=     7;  tdx=  7997;  tdy=     7;			# 12074
	v1x=  7994;  v1y=  3991;  v2x= 12090;  v2y=     0;  tdx=  7997;  tdy=     0;
}

END {

	REGION = 4096*8;
	MAX_TDX = 1000*8;
	MAX_LINE = 512*8;

	initmode = 10;

	srand();
	while(1)
	{
		if( ++loops % 1000000 == 0 )
			print loops "!";

		if( !initmode )
		{
			plx  = int(rand()*REGION*2-REGION);
			ply  = int(rand()*REGION*2-REGION);
			v1x = int(rand()*REGION*2-REGION) - plx;
			v1y = int(rand()*REGION*2-REGION) - ply;
			v2x = v1x + int(rand()*MAX_LINE*2-MAX_LINE);
			v2y = v1y + int(rand()*MAX_LINE*2-MAX_LINE);
			tdx = int(rand()*MAX_TDX+1);
			tdy = int(rand()*tdx);
		}
		else
		{
			if( !update ) init();
			if( loops > 1 )
			{
				v1x += int(rand()*initmode*2-initmode);
				v1y += int(rand()*initmode*2-initmode);
				v2x += int(rand()*initmode*2-initmode);
				v2y += int(rand()*initmode*2-initmode);
				tdx += int(rand()*initmode*2-initmode);
				tdy += int(rand()*initmode*2-initmode);
			}
			update = 0;

			if( tdx > MAX_TDX ) continue;
			if( tdx <= 0 ) continue;
			if( tdy > tdx ) continue;
			if( tdy < 0 ) continue;
			if( v1x - v2x >  MAX_LINE ) continue;
			if( v1x - v2x < -MAX_LINE ) continue;
			if( v1y - v2y >  MAX_LINE ) continue;
			if( v1y - v2y < -MAX_LINE ) continue;
			if( v1x >  REGION*2 ) continue;
			if( v1x < -REGION*2 ) continue;
			if( v1y >  REGION*2 ) continue;
			if( v1y < -REGION*2 ) continue;
		}

		if( v1x > v2x )
		{
			t=v1x; v1x=v2x; v2x=t;
			t=v1y; v1y=v2y; v2y=t;
		}

		if( v1y<v2y ) continue;

		if( tdx < v1x ) continue;
		if( tdy < v2y ) continue;

		A = v1y - v2y;
		B = v2x - v1x;
		C = A*v1x + B*v1y;
		if( C<=0 ) continue;

		div = A + tdy*B/tdx + 1;
		if( div <= 0 ) { dump(); exit(1); }

		value = C / div;
		if( value > vmax )
		{
			vmax = value;
			update = 1;
			printf("vmax=%10d;    v1x=%6d;  v1y=%6d;  v2x=%6d;  v2y=%6d;  tdx=%6d;  tdy=%6d;\n", value, v1x, v1y, v2x, v2y, tdx, tdy);
		}
	}
}
