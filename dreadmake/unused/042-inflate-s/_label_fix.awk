BEGIN {

	for(label=1;label<=99;label++)
	{
		if( !REPL[label "f"] ) REPLLIST[++NREPL] = label "f";
		REPL[label "f"] = "_inflate_" label "_" (INDEX[label]+1);
	}

}


/^[0-9]+:/{

	label = $0;
	sub(/:.*$/,"",label);

	newlabel = "_inflate_" label "_" (++INDEX[label]);
	
	sub(/^[0-9]+:/,newlabel MSYM ":");

	if( !REPL[label "b"] ) REPLLIST[++NREPL] = label "b";
	REPL[label "b"] = newlabel;
	
	if( !REPL[label "f"] ) REPLLIST[++NREPL] = label "f";
	REPL[label "f"] = "_inflate_" label "_" (INDEX[label]+1);

#	print label " -> " newlabel;
}

$1==".macro" {
	$0 = $2 ":macro";
	MSYM = "\\@";
}

$1==".endm" {
	$0 = "\tendm";
	MSYM = "";
}

$0!~/^#/{

	gsub(/\<0x/,"$");
	gsub(/\<jeq\>/,"beq");
	gsub(/\<jne\>/,"bne");
	gsub(/\<jcc\>/,"bcc");
	gsub(/\<jcs\>/,"bcs");
	gsub(/\<jls\>/,"bls");
	gsub(/\<jhi\>/,"bhi");
	gsub(/\<jpl\>/,"bpl");
	gsub(/\<jmi\>/,"bmi");
	gsub(/\<jra\>/,"bra");
	gsub(/\<jbsr\>/,"bsr");


	for(i=1;i<=NREPL;i++)
		gsub("\\<" REPLLIST[i] "\\>", REPL[REPLLIST[i]] MSYM);

	print;
}
