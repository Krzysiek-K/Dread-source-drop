BEGIN {
	NREGS = 0

	for(i=0;i<=7;i++)	REGS[NREGS++] = "d" i;
	REGS[NREGS++] = "";
	for(i=0;i<=6;i++)	REGS[NREGS++] = "a" i;

	printf("%-20s\t", "");
	for(i=0;i<NREGS;i++)
		printf("%-2s ",REGS[i]);
	print "";

	#printf("%-20s\t", "");
	#print "[------D------]   [------A------]";
	#
	#printf("%-20s\t", "");
	#print "0 1 2 3 4 5 6 7   0 1 2 3 4 5 6 7";
}

function end_label()
{
	printf("%-20s\t", LABEL);
	for(i=0;i<NREGS;i++)
	{
		reg = REGS[i];
	    if( FN_WRITE[LABEL,reg]	)
		{
			if( FN_SUBWRITE[LABEL,reg]	)	printf("## ");
			else							printf("@@ ");
		}
		else if( FN_SUBWRITE[LABEL,reg]	)	printf("$$ ");
		else if( FN_READ[LABEL,reg]		)	printf(".. ");
		else								printf("   ");
	}

	for(i=0;i<FN_NCALL[LABEL];i++)
		printf(" %s", FN_CALL[i]);
	print "";
}

function start_label(l)
{
	if( LABEL )
		end_label();

	sub(/:$/,"",l);
	LABEL = l;
	FN_NCALL[LABEL] = 0;
}


{
	sub(/[ \t]*;.*$/,"");
}

/^[a-zA-Z0-9_]+:/{
	start_label($1);
	#print "-> " $1;
}

{
	for(i=0;i<NREGS;i++)
	{
		reg = REGS[i];
		if( reg && $0 ~ ("[, \t]" reg "$") ) FN_WRITE[LABEL,reg] = 1;
		if( reg && $0 ~ ("[, \t(]" reg "[, \t)]") ) FN_READ[LABEL,reg] = 1;
	}
}

$1=="jsr" {
	for(i=0;i<NREGS;i++)
	{
		reg = REGS[i];
		if( FN_WRITE[$2,reg] || FN_SUBWRITE[$2,reg] )
			FN_SUBWRITE[LABEL,reg] = 1;
	}
	if( !FN_ISCALL[LABEL,$2] )
	{
		FN_ISCALL[LABEL,$2] = 1;
		FN_CALL[FN_NCALL[LABEL]++] = $2;
	}
}

END{
	end_label();
}
