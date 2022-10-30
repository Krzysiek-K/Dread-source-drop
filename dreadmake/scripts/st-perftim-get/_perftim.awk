
function reset()
{
	DATA = 1;
	PENDING = 0;
	HZ = 5;
	STEPNUM = 0;
	RESSTRING = "";
	PCHARS = 0;
}

function step(prn)
{
	if( STEPNUM==TICKSTEP )
	{
		printf(".");
		PCHARS++;
		if( --DATA <= 0 )
		{
			DATA = 192;
			PENDING = 1;
		}
	}
	if( STEPNUM==IRQSTEP )
	{
		printf("!");
		PCHARS++;
		if( PENDING )
		{
			PENDING = 0;
			HZ++;
		}
	}
	STEPNUM++;
	if(!prn)
	{
		printf("%s",substr("ABCDEFGHIJKL",STEPNUM,1));
		PCHARS++;
	}
}

function read_data()
{
	step();
	RESSTRING = RESSTRING sprintf(" %03d",DATA);
	return DATA;
}

function read_hz()
{
	step();
	RESSTRING = RESSTRING sprintf(" %d",HZ);
	return HZ;
}

function read_pending()
{
	step();
	RESSTRING = RESSTRING (PENDING ? " *" : " -");
	return PENDING;
}

function algo()
{
#	t1 = read_data();
#	h1 = read_hz();
#	p  = read_pending();
#	h2 = read_hz();
#	t2 = read_data();
#
#	res = h1*192 - t2;
#	if( h1!=h2 || p || t1<t2 ) res += 192;
#	return res;


#	t2 = read_data();
	h1 = read_hz();
#	p  = read_pending();
	t1 = read_data();
#	h2 = read_hz();
	 
	res = h1*192 - t1;
	#if( h1!=h2 ) res += 192;
	return res;
}

END {
	TICKSTEP = -1;
	IRQSTEP = -1;
	reset();
	algo();
	BASEVAL = 5*192 - 1;
	printf("\n");
	MAXSTEP = STEPNUM;

	for(TICKSTEP=0;TICKSTEP<=MAXSTEP;TICKSTEP++)
		for(IRQSTEP=TICKSTEP;IRQSTEP<=MAXSTEP;IRQSTEP++)
		{
			reset();
			value = algo();
			step(1);
			while(PCHARS++<10) printf(" ");
			printf("%-20s -> %5d  [%+5d]\n",RESSTRING,value,value-BASEVAL);
		}

	print "";
	print "";
	print "";
}