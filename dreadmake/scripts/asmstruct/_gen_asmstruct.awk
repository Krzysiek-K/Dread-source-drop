
$1=="};" || $1=="}" {
	if(NAME)
	{
		printf("%-24sequ %3d\n",NAME "_sizeof",OFFSET);
		if( CONDITION )
			print "\tendif";
	}
	NAME=0
}

{ok=0}
/^[ \t]*\/\//{ok=1}
/^[ \t]*$/{ok=1}

NAME && $1=="struct"{ok=1}
NAME && $1=="typedef" && $2=="struct"{ok=1}
NAME && !ok{
	line=$0
	sub(/^[ \t]*const/,"",line);
	sub(/^[ \t]*/,"",line);
	size=0;
	if( sub(/^[a-zA-Z0-9_]+[ \t]+\*/,"",line) ) size=4;
	else if( sub(/^byte[ \t]+/,"",line) ) size=1;
	else if( sub(/^word[ \t]+/,"",line) ) size=2;
	else if( sub(/^dword[ \t]+/,"",line) ) size=4;
	else if( sub(/^char[ \t]+/,"",line) ) size=1;
	else if( sub(/^short[ \t]+/,"",line) ) size=2;
	else if( sub(/^int[ \t]+/,"",line) ) size=4;

	if(size && sub(/;.*$/,"",line))
	{
		if(size>=2 && OFFSET%2)
			print "Error: unaligned field";

		COUNT=1;
		arr=line;
		if( sub(/^.*\[/,"",arr) && sub(/\].*$/,"",arr) )
		{
			COUNT = strtonum(arr);
			sub(/[ \t]*\[.*$/,"",line);
		}
		
		if( line~/^[a-zA-Z0-9_]+$/ )
		{
			printf("%-23s equ %3d\n",NAME "_" line,OFFSET);
			OFFSET += size * COUNT;
			ok=1;
		}
	}	
}



$1=="//$asmstruct"{
	NAME=$2
	OFFSET=0
	CONDITION=0;
	print "";
	if( $3 )
	{
		print "\tif " $3;
		CONDITION = $3;
	}
	print "; struct " NAME
}

NAME && !ok{ print "Error: " $0 }
