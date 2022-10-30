BEGIN {
	MAX_ONES	= 1;
	MAX_ZEROES	= 5;
	MAX_ENDZ	= 3;
	BITS = 10;
	IN_STATE = -2;
}

function is_mfm(n,	b,st)
{
	b = lshift(1,BITS-1);
	st = IN_STATE;
	while( b >= 1 )
	{
		if( and(n,b) )
		{
			if( st<0 ) st = 0;
			st++;
			if( st>MAX_ONES ) return 0;
		}
		else
		{
			if( st>0 ) st = 0;
			st--;
			if( st<-MAX_ZEROES ) return 0;
		}

		b = rshift(b,1);
	}

	if( st<0 && -st>MAX_ENDZ ) return 0;

	return 1;
}

function binary(v,	b,s)
{
	s = "";
	for(b=lshift(1,BITS-1);b>0;b=rshift(b,1))
		if( and(v,b) )
			s = s "1";
		else
			s = s "0";
	return s;
}

function experiment(s)
{
	IN_STATE = s;
	count=0;

	for(n=0;n<lshift(1,BITS);n++)
		if( is_mfm(n) )
		{
			printf("   %s (%6d) -> %6d\n",binary(n),n,count);
			count++;
		}

	if(count<min_data) min_data = count;
	printf("%4d %2d: %6d\n",BITS,IN_STATE,count);
}

function full_experiment()
{
	printf("==== Testing %d bits, max %d ones, %d zeroes ====\n", BITS, MAX_ONES, MAX_ZEROES);
	min_data = 65536;
	max_st_zeroes = MAX_ZEROES;
	if( MAX_ENDZ<max_st_zeroes ) max_st_zeroes = MAX_ENDZ

	for(stb=-max_st_zeroes;stb<0;stb++) experiment(stb);
	for(stb=1;stb<=MAX_ONES;stb++)	 experiment(stb);

	DATA_BITS = 1;
	while( lshift(1,DATA_BITS+1)<=min_data )
		DATA_BITS++;

	printf("Efficiency: %4.1f%%  (%d bits / %d)\n",DATA_BITS*100/BITS,DATA_BITS,BITS);
	print "";
}

END {
#	for(BITS=4;BITS<=12;BITS++)
		full_experiment();
}