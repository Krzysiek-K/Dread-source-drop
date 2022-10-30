
END{
	#for(i=0;i<256;i++)
	#	print int(rand()*256);

	# $xy
	x = 0;
	y = 0;

	seed = 31;
	while(1)
	{
		srand(seed);
		printf("%3d: ",seed)  >"/dev/stderr";

		for(i=0;i<16;i++)
		{
			SIZE[i] = 16;
			for(j=0;j<16;j++)
				NUM[i,j] = j;
		}

		for(i=0;i<256;i++)
		{
			v = x*16+y;
			RNG[i] = v;
			printf("\t0x%02X,\t// %3d %s\n", v, v, substr(" .*#",rshift(v,6)+1,1));
			x = y;
			if(SIZE[x]<=0)
			{
				print "FAILED after number " i >"/dev/stderr";
				break;
			}
			p = int(rand()*SIZE[x]);
			if(p<0 || p>=SIZE[x])
			{
				print "RNG FAILED after number " i >"/dev/stderr";
				break;
			}
			y = NUM[x,p];
			NUM[x,p] = NUM[x,SIZE[x]-1];
			SIZE[x]--;
		}

		if(i>=256)
		{
			print "OK!"  >"/dev/stderr";
			break;
		}

		seed++;
	}


	// CRC test
	N=0;
	PACKET[N++] = 0x01;
	PACKET[N++] = 0xEA;
	PACKET[N++] = 0x80;
	PACKET[N++] = 0xD0;
	PACKET[N++] = 0x00;
	crc = 0xA5;
	for(i=0;i<N;i++)
		crc = RNG[and(PACKET[i]+crc,0xFF)];
	printf("CRC = 0x%02X\n", crc)  >"/dev/stderr";
}
