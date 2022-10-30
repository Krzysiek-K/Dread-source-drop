
function run()
{
	A = lshift(t1,1);
	B = rshift(t2,1);
	C = t3;
	return xor(xor(A,B),C);
}


END {
	NVALUES = 0;

	t1 = 0x0000;
	t2 = 0x0001;
	t3 = 0x0002;

	system("cls");

	LASTCYCLE = 0;
	for(CYCLE=0;CYCLE<128*1024;CYCLE++)
	{
		v = and( run(), 0xFFFF);
		t1 = t2;
		t2 = t3;
		t3 = v;

		if( !VALUES[v] )
		{
			NVALUES++;
			VALUES[v] = 1;
			LASTCYCLE = CYCLE;
			#printf("%6d @ %9d\n",NVALUES,CYCLE);
		}
		if( CYCLE==65536 )
			printf("%6d @ %9d\n",NVALUES,CYCLE);
	}
	
	printf("%6d @ %9d\n",NVALUES,LASTCYCLE);


	print "";
}
