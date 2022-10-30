

function test(n1s,n2s,t1s,t2s)
{
	if( n1s<0 && n2s<0 ) return 1;		# no collision, OK
	if( n1s>0 && n2s>0 ) return 1;
	if( t1s<0 && t2s<0 ) return 1;
	if( t1s>0 && t2s>0 ) return 1;
	if( n1s<0 && n2s>0 ) return 0;
	if( t1s<0 && t2s>0 ) return 0;
	return -1;
}


END {
	ncases=0;
	unhandled=0;
	for(A=-1;A<=1;A++)
		for(B=-1;B<=1;B++)
			for(C=-1;C<=1;C++)
				for(D=-1;D<=1;D++)
				{
					res = test(A,B,C,D);
					if( res<0 )
					{
						if(unhandled==0)
							print "Unhandled case n1s=" A " n2s=" B " t1s=" C " t2s=" D;
						unhandled++;
					}
					ncases++;
				}
	print (ncases-unhandled) "/" ncases " cases handled."
}
