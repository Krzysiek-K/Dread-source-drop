
#include "common.h"
#include "script_compiler.h"



struct OpInfo {
	opcode_t	opcode;
	const char	*name;
	int			argnum;
	int			sizes;
};

const OpInfo OPINFO[] = {
	{ OP_EMPTY,			"",			0,	0		},		// no opcode, just comment
									 
	{ OP_MOVE,			"move",		2,	7		},		//	<src>, <dst>
	{ OP_MOVEQ,			"moveq",	2,	7		},		//	<src>, <dst>			automatically converted
	{ OP_MOVEM,			"movem",	2,	7		},		//	<src>, <dst>
	{ OP_SWAP,			"swap",		1,	2		},		//	<dst>
									 	
	{ OP_LEA,			"lea",		2,	7		},		//	<src>, <dst>
									 	
	{ OP_ADD,			"add",		2,	7		},		//	<src>, <dst>
	{ OP_ADDQ,			"addq",		2,	7		},		//	<src>, <dst>			automatically converted
	{ OP_SUB,			"sub",		2,	7		},		//	<src>, <dst>
	{ OP_SUBQ,			"subq",		2,	7		},		//	<src>, <dst>			automatically converted
	{ OP_CMP,			"cmp",		2,	7		},		//	<src>, <dst>
									 	
	{ OP_LSR,			"lsr",		2,	7		},		//	<src>, <dst>
	{ OP_ASR,			"asr",		2,	7		},		//	<src>, <dst>
	{ OP_LSL,			"lsl",		2,	7		},		//	<src>, <dst>
									 	
	{ OP_MULS,			"muls",		2,	7		},		//	<src>, <dst>
	{ OP_MULU,			"mulu",		2,	7		},		//	<src>, <dst>
	{ OP_DIVS,			"divs",		2,	7		},		//	<src>, <dst>
	{ OP_DIVU,			"divu",		2,	7		},		//	<src>, <dst>
									 	
	{ OP_AND,			"and",		2,	7		},		//	<src>, <dst>
	{ OP_OR,			"or",		2,	7		},		//	<src>, <dst>
	{ OP_EOR,			"eor",		2,	7		},		//	<src>, <dst>
									 	
	{ OP_JMP,			"jmp",		1,	0		},		//	<dst>
	{ OP_JSR,			"jsr",		1,	0		},		//	<dst>
	{ OP_BRA,			"bra",		1,	3		},		//	<dst>
	{ OP_BSR,			"bsr",		1,	3		},		//	<dst>
	{ OP_BEQ,			"beq",		1,	3		},		//	<dst>		==
	{ OP_BNE,			"bne",		1,	3		},		//	<dst>		!=
	{ OP_BGE,			"bge",		1,	3		},		//	<dst>		>=	(signed)
	{ OP_BGT,			"bgt",		1,	3		},		//	<dst>		>	(signed)
	{ OP_BLE,			"ble",		1,	3		},		//	<dst>		<=	(signed)
	{ OP_BLT,			"blt",		1,	3		},		//	<dst>		<	(signed)
	{ OP_BCC,			"bcc",		1,	3		},		//	<dst>		>=	(unsigned)
	{ OP_BHI,			"bhi",		1,	3		},		//	<dst>		>	(unsigned)
	{ OP_BLS,			"bls",		1,	3		},		//	<dst>		<=	(unsigned)
	{ OP_BCS,			"bcs",		1,	3		},		//	<dst>		<	(unsigned)
	{ OP_BMI,			"bmi",		1,	3		},		//	<dst>
	{ OP_BPL,			"bpl",		1,	3		},		//	<dst>
	{ OP_EXT,			"ext",		1,	6		},		//	<dst>
	{ OP_EXTB,			"extb",		1,	4		},		//	<dst>
	{ OP_DBRA,			"dbra",		2,	7		},		//	<src>, <dst>
	{ OP_RTS,			"rts",		0,	0		},		//	-

	{ OP_LABEL,			"<label>",	1,	0		},		//	<dst>
	{ OP_PREPROC_IF,	"if",		1,	0		},		//	<dst>
	{ OP_PREPROC_ELIF,	"elif",		1,	0		},		//	<dst>
	{ OP_PREPROC_ELSE,	"else",		0,	0		},		//	<dst>
	{ OP_PREPROC_ENDIF,	"endif",	0,	0		},		//	<dst>

	{ OP_INVALID }
};


static string _regname(int reg)
{
	if( reg<8 ) return format("d%d", reg);
	if( reg<16 ) return format("a%d", reg-8);
	return format("x%d", reg-16);
}

void EAddr::Cleanup()
{
	switch(amode)
	{
	default:
	case AMODE_IGNORED:
	case AMODE_MEM_LABEL:
	case AMODE_MEM_REL_PC:
	case AMODE_IMM:
	case AMODE_REGMASK:
	case AMODE_STRING:
		regnum = -1;
		index_reg = -1;
		break;

	case AMODE_REG:
	case AMODE_MEM_A:
	case AMODE_MEM_A_POSTINC:
	case AMODE_MEM_A_PREDEC:
		index_reg = -1;
		break;

	case AMODE_MEM_INDEX_W:
	case AMODE_MEM_INDEX_L:
		break;
	}
}

int EAddr::EACycles(bool _long)
{
	if( amode==AMODE_REG ) return 0;
	if( amode==AMODE_MEM_A && (offs || label.size()>0) ) return _long ? 12 : 8;
	if( amode==AMODE_MEM_A || amode==AMODE_MEM_A_POSTINC ) return _long ? 8 : 4;
	if( amode==AMODE_MEM_A_PREDEC ) return _long ? 10 : 6;
	if( amode==AMODE_MEM_INDEX_W || amode==AMODE_MEM_INDEX_L ) return _long ? 14 : 10;
	if( amode==AMODE_MEM_LABEL ) return _long ? 16 : 12;
	if( amode==AMODE_MEM_REL_PC ) return _long ? 12 : 8;
	if( amode==AMODE_IMM ) return _long ? 8 : 4;
	return -1000;
}

string EAddr::ToString()
{
	if( amode==AMODE_IGNORED ) return "<ignored>";
	if( amode==AMODE_REG ) return _regname(regnum);

	if( amode==AMODE_REGMASK )
	{
		int mask = value;
		int start = 0;
		string s;
		while( 1 )
		{
			while( start<16 && !(mask & (1<<start)) ) start++;
			if( start>=16 ) break;
			int end = start;
			while( end<(start&~7)+8 && (mask & (1<<end)) ) end++;

			if( s.size()>0 ) s += "/";

			s += _regname(start);
			if( end>start+1 )
			{
				s += "-";
				s += _regname(end-1);
			}

			start = end;
		}
		return s;
	}

	if( amode==AMODE_STRING )
		return label;

	string s;
	int num = offs;
	if( amode==AMODE_IMM ) s += "#", num = value;

	s += label;
	if( num || (label.size()==0 && amode==AMODE_IMM) )
	{
		if( num>=0 && label.size()>0 ) s += "+";
		if( hex_value ) s += format("$%04X", num);
		else s += format("%d", num);
	}

	if( amode==AMODE_MEM_A ) s += "(" + _regname(regnum) + ")";
	if( amode==AMODE_MEM_A_POSTINC ) s += "(" + _regname(regnum) + ")+";
	if( amode==AMODE_MEM_A_PREDEC ) s += "-(" + _regname(regnum) + ")";
	if( amode==AMODE_MEM_REL_PC ) s += "(pc)";
	if( amode==AMODE_MEM_INDEX_L ) s += "(" + _regname(regnum) + "," + _regname(index_reg) + ")";
	if( amode==AMODE_MEM_INDEX_W ) s += "(" + _regname(regnum) + "," + _regname(index_reg) + ".w)";

	return s;
}



void Instruction::ConvertOpcode()
{
	if( opcode==OP_ADD && src.amode==AMODE_IMM && src.label.size()==0 && src.value>=1 && src.value<=8 )
		opcode = OP_ADDQ;

	if( opcode==OP_SUB && src.amode==AMODE_IMM && src.label.size()==0 && src.value>=1 && src.value<=8 )
		opcode = OP_SUBQ;

	if( opcode==OP_MOVE && width==SIZE_LONG && src.amode==AMODE_IMM && src.label.size()==0 && src.value>=-128 && src.value<=1217 && dst.amode==AMODE_REG && dst.regnum>=0 && dst.regnum<8 )
		opcode = OP_MOVEQ;
}

void Instruction::CountCycles()
{
	cycles = 0;

	if( opcode==OP_MOVE )
	{
		if( width==SIZE_BYTE || width==SIZE_WORD )
		{
			// 	Dn / An 	(An) / (An)+ / -(An) 	d(An) / xxx.W 	d(An,ix) 	xxx.L
			//	
			//				+4						+8				+10			+12
			//	4	 		8 						12 				14 			16			Dn / An 			
			//	8	 		12 						16 				18 			20			(An) / (An)+ / #xxx 		
			//	10			14 						18 				20 			22			-(An) 		
			//	12			16 						20 				22 			24			d(An) / xxx.W / d(PC) 		
			//	14			18 						22 				24 			26			d(An,ix) / d(PC,ix)	
			//	16			20 						24 				26 			28			xxx.L 		
			//
			// unsupported:		xxx.W	d(PC,ix)
			//
			int sc = -1;
			switch( src.amode )
			{
			case AMODE_REG:					sc =  4;										break;
			case AMODE_MEM_A:				sc = src.offs || src.label.size()>0 ? 12 : 8;	break;
			case AMODE_MEM_A_POSTINC:		sc =  8;										break;
			case AMODE_MEM_A_PREDEC:		sc = 10;										break;
			case AMODE_MEM_LABEL:			sc = 16;										break;
			case AMODE_MEM_REL_PC:			sc = 12;										break;
			case AMODE_MEM_INDEX_W:			sc = 14;										break;
			case AMODE_MEM_INDEX_L:			sc = 14;										break;
			case AMODE_IMM:					sc =  8;										break;
			}
			int dc = -1;
			switch( dst.amode )
			{
			case AMODE_REG:					dc =  0;										break;
			case AMODE_MEM_A:				dc = dst.offs || src.label.size()>0 ? 8 : 4;	break;
			case AMODE_MEM_A_POSTINC:		dc =  4;										break;
			case AMODE_MEM_A_PREDEC:		dc =  4;										break;
			case AMODE_MEM_LABEL:			dc = 12;										break;
			case AMODE_MEM_INDEX_W:			dc = 10;										break;
			case AMODE_MEM_INDEX_L:			dc = 10;										break;
			}
			if( sc>=0 && dc>=0 )
				cycles = sc + dc;
		}
		else if( width==SIZE_LONG )
		{
			//	Dn /An		(An) / (An)+ / -(An) 	d(An) / xxx.W 		d(An,ix) 	xxx.L
			//				+8						+12					+14			+16
			//	4 			12						16					18			20			Dn / An 		
			//	12 			20						24					26			28			(An) / (An)+ / #xxx 	
			//	14 			22						26					28			30			-(An) 	
			//	16 			24						28					30			32			d(An) / xxx.W / d(PC) 	
			//	18 			26						30					32			34			d(An,ix) / d(PC,ix)
			//	20 			28						32					34			36			xxx.L 	
			//
			// unsupported:		xxx.W	d(PC,ix)
			//
			int sc = -1;
			switch( src.amode )
			{
			case AMODE_REG:					sc =  4;										break;
			case AMODE_MEM_A:				sc = src.offs || src.label.size()>0 ? 16 : 12;	break;
			case AMODE_MEM_A_POSTINC:		sc = 12;										break;
			case AMODE_MEM_A_PREDEC:		sc = 14;										break;
			case AMODE_MEM_LABEL:			sc = 20;										break;
			case AMODE_MEM_REL_PC:			sc = 16;										break;
			case AMODE_MEM_INDEX_W:			sc = 18;										break;
			case AMODE_MEM_INDEX_L:			sc = 18;										break;
			case AMODE_IMM:					sc = 12;										break;
			}
			int dc = -1;
			switch( dst.amode )
			{
			case AMODE_REG:					dc =  0;										break;
			case AMODE_MEM_A:				dc = dst.offs || src.label.size()>0 ? 12 : 8;	break;
			case AMODE_MEM_A_POSTINC:		dc =  8;										break;
			case AMODE_MEM_A_PREDEC:		dc =  8;										break;
			case AMODE_MEM_LABEL:			dc = 16;										break;
			case AMODE_MEM_INDEX_W:			dc = 14;										break;
			case AMODE_MEM_INDEX_L:			dc = 14;										break;
			}
			if( sc>=0 && dc>=0 )
				cycles = sc + dc;
		}

		if( cycles<=0 ) throw "Invalid instruction operands:\n" + ToString();
		return;
	}

	//	                                  d(an                   d(pc
	//	         (an)  (an)+  -(an) d(an) .ri) abs.s abs.l d(pc) .ri)
	//	
	//	lea      4     -      -     8     12   8     12    8     12
	//	jmp      8     -      -     10    14   10    12    10    14
	//	pea      12    -      -     16    20   16    20    16    20
	//	jsr      16    -      -     18    22   18    20    18    22
	//
	if( opcode==OP_LEA )
	{
		if( src.amode==AMODE_MEM_A ) cycles = (src.offs || src.label.size()>0) ? 8 : 4;
		if( src.amode==AMODE_MEM_INDEX_W ||
			src.amode==AMODE_MEM_INDEX_L ) cycles = 12;
		if( src.amode==AMODE_MEM_LABEL ) cycles = 12;
		if( src.amode==AMODE_MEM_REL_PC ) cycles = 8;
	}
	else if( opcode==OP_JMP )
	{
		if( dst.amode==AMODE_MEM_A ) cycles = (src.offs || src.label.size()>0) ? 10 : 8;
		if( dst.amode==AMODE_MEM_INDEX_W ||
			dst.amode==AMODE_MEM_INDEX_L ) cycles = 14;
		if( dst.amode==AMODE_MEM_LABEL ) cycles = 12;
		if( dst.amode==AMODE_MEM_REL_PC ) cycles = 10;
	}
	else if( opcode==OP_RTS )
	{
		cycles = 16;
	}
	else if( opcode==OP_SWAP )
	{
		cycles = 4;
	}
	//	.b.w/.l  dn,dn    m,m
	//	
	//	addx      4/8    18/30
	//	cmpm       -     12/20
	//	subx      4/8    18/30
	//	abcd       6      18      .b only
	//	sbcd       6      18      .b only
	//	Bcc      .b/.w   10/10      8/12
	//	bra      .b/.w   10/10       -
	//	bsr      .b/.w   18/18       -
	//	DBcc      t/f      10      12/14
	//	chk        -       40 max    8
	//	trap       -       34        -
	//	trapv      -       34        4
	else if( opcode==OP_BRA || opcode==OP_BEQ || opcode==OP_BNE ||
		opcode==OP_BGE || opcode==OP_BGT || opcode==OP_BLE || opcode==OP_BLT ||
		opcode==OP_BCC || opcode==OP_BHI || opcode==OP_BLS || opcode==OP_BCS ||
		opcode==OP_BMI || opcode==OP_BPL ||
		opcode==OP_DBRA )
	{
		cycles = 10;
	}
	else if( src.amode==AMODE_IMM )
	{
		// Immediates
		//	.b.w/.l  #,dn  #,an  #,mem
		//	
		//	addi     8/16   -    12/20
		//	andi     8/16   -    12/20   nbcd+tas.b only
		//	ori      8/16   -    12/20   add effective address times from above for mem addresses single operand instructions
		//	subi     8/16   -    12/20   
		//	eori     8/16   -    12/20   scc false/true
		//
		//	cmpi     8/14  8/14   8/12
		//
		//	addq     4/8   8/8    8/12   Moveq.l only
		//	subq     4/8   8/8    8/12   
		//
		//	moveq     4     -      -
		//
		//	clr      4/6   4/6   8/12    
		//	neg      4/6   4/6   8/12
		//	negx     4/6   4/6   8/12
		//	not      4/6   4/6   8/12
		//	scc      4/6   4/6   8/8
		//
		//	tas       4     4    10
		//	tst      4/4   4/4   4/4
		//	nbcd      6     6     8
		bool _bw = (width==SIZE_BYTE || width==SIZE_WORD);

		if( src.label.size()<=0 &&
			(opcode==OP_LSL || opcode==OP_LSR || opcode==OP_ASR) )			// shifts
		{
			cycles = (width==SIZE_LONG || dst.IsMem()) ? 8 : 6;
			cycles += dst.EACycles(width==SIZE_LONG);
			cycles += 2*src.value;
		}
		else if( dst.amode==AMODE_REG && dst.regnum>=0 && dst.regnum<8 )	// #, Dn
		{
			if( opcode==OP_ADD ||
				opcode==OP_AND ||
				opcode==OP_OR ||
				opcode==OP_SUB ||
				opcode==OP_EOR )	cycles = _bw ? 8 : 16;

			if( opcode==OP_CMP )	cycles = _bw ? 8 : 14;

			if( opcode==OP_ADDQ ||
				opcode==OP_SUBQ )	cycles = _bw ? 4 : 8;

			if( opcode==OP_MOVEQ && !_bw )	cycles = 4;
		}
		else if( dst.amode==AMODE_REG && dst.regnum>=8 && dst.regnum<16 )	// #, An
		{
			if( opcode==OP_ADD ||
				opcode==OP_SUB )	cycles = _bw ? 8 : 16;

			if(	opcode==OP_CMP )	cycles = _bw ? 8 : 14;

			if( opcode==OP_ADDQ ||
				opcode==OP_SUBQ )	cycles = 8;
		}
		else if( dst.IsMem() )												// #, MEM
		{
			if( opcode==OP_ADD ||
				opcode==OP_AND ||
				opcode==OP_OR ||
				opcode==OP_SUB ||
				opcode==OP_EOR )	cycles = (_bw ? 8 : 16) + dst.EACycles(!_bw);

			if( opcode==OP_CMP ||
				opcode==OP_ADDQ ||
				opcode==OP_SUBQ )	cycles = (_bw ? 8 : 12) + dst.EACycles(!_bw);
		}
	}
	else
	{
		//	.b.w/.l   ea,an   ea,dn   dn,mem
		//	
		//	add       8/6(8)  4/6(8)  8/12    (8) time if effective address is direct
		//	sub       8/6(8)  4/6(8)  8/12
		//	and        -      4/6(8)  8/12    
		//	or         -      4/6(8)  8/12
		//	cmp       6/6     4/6      -
		//	eor        -      4/8     8/12        
		//	divs       -      158max   -          Add effective address times from above for memory addresses.
		//	divu       -      140max   -          
		//	muls       -      70max    -
		//	mulu       -      70max    -

		if( dst.amode==AMODE_REG && dst.regnum>=8 && dst.regnum<16 )		// <ea>, An
		{
			bool _bw = (width==SIZE_BYTE || width==SIZE_WORD);
			bool _direct = (src.amode==AMODE_REG);

			if( opcode==OP_ADD ||
				opcode==OP_SUB )	cycles = (_bw ? 8 : (_direct ? 8 : 6)) + src.EACycles(!_bw);

			if( opcode==OP_CMP )	cycles = 6 + src.EACycles(!_bw);
		}
		else if( dst.amode==AMODE_REG && dst.regnum>=0 && dst.regnum<8 )		// <ea>, Dn
		{
			bool _bw = (width==SIZE_BYTE || width==SIZE_WORD);
			bool _direct = (src.amode==AMODE_REG);

			if( opcode==OP_ADD ||
				opcode==OP_SUB ||
				opcode==OP_AND ||
				opcode==OP_OR )		cycles = (_bw ? 4 : (_direct ? 8 : 6)) + src.EACycles(!_bw);

			if( opcode==OP_CMP )	cycles = (_bw ? 4 : 6) + src.EACycles(!_bw);
			if( opcode==OP_EOR )	cycles = (_bw ? 4 : 8) + src.EACycles(!_bw);

			if( opcode==OP_DIVS )	cycles = 158 + src.EACycles(!_bw);
			if( opcode==OP_DIVU )	cycles = 140 + src.EACycles(!_bw);
			if( opcode==OP_MULS )	cycles =  70 + src.EACycles(!_bw);
			if( opcode==OP_MULU )	cycles =  70 + src.EACycles(!_bw);
		}
		else if( src.amode==AMODE_REG && src.regnum>=0 && src.regnum<8 && dst.IsMem() )		// Dn, MEM
		{
			bool _bw = (width==SIZE_BYTE || width==SIZE_WORD);

			if( opcode==OP_ADD ||
				opcode==OP_SUB ||
				opcode==OP_AND ||
				opcode==OP_OR ||
				opcode==OP_EOR )	cycles = (_bw ? 8 : 12) + dst.EACycles(!_bw);
		}
	}

}

string Instruction::ToString()
{
	string s;
	if( opcode==OP_LABEL && dst.amode==AMODE_MEM_LABEL )
	{
		s += dst.label + ":";
	}
	else
	{
		s += "    ";
		const OpInfo *info = NULL;
		for( int i=0; OPINFO[i].opcode!=OP_INVALID; i++ )
			if( OPINFO[i].opcode==opcode )
			{
				info = OPINFO + i;
				break;
			}
		if( !info )
			s += "<illegal>";
		else
		{
			s += info->name;
			if( width!=SIZE_UNSIZED )
			{
				s += ".";
				if( width==SIZE_BYTE ) s += "b";
				if( width==SIZE_WORD ) s += "w";
				if( width==SIZE_LONG ) s += "l";
			}

			while( s.size()<16 )
				s += " ";

			if( src.amode!=AMODE_IGNORED ) s += src.ToString() + ", ";
			if( dst.amode!=AMODE_IGNORED ) s += dst.ToString();
		}
	}

	//if( cycles || comment.size()>0 )
	{
		while( s.size()<52 )
			s += " ";
		s += ";";

		s += format("%d", block_mode);

		s += " d";
		for( int i=0; i<8; i++ )
			s += (free_regs & (1<<i)) ? format("%d", i) : "-";

		s += " a";
		for( int i=0; i<8; i++ )
			s += (free_regs & (1<<(i+8))) ? format("%d", i) : "-";

		if( cycles ) s += format(" %3d   ", cycles);
		else s += "       ";
		s += comment;
	}

	return s;
}







void Assembler::Clear()
{
	for( int i=0; i<(int)instr.size(); i++ )
		delete instr[i];
	instr.clear();
	block_mode = 0;
}

void Assembler::Assemble(const char *s)
{
	Instruction *ins = new Instruction();
	instr.push_back(ins);
	ins->block_mode = block_mode;

	const char *_s= s;
	ParseWhitespace(s);
	if( *s==';' )
	{
		// Comment
	}
	else if( s==_s )
	{
		// Label
		ParseStringT(s, ins->dst.label, ":");
		if( *s!=':' || ins->dst.label.size()<=0 ) throw format("Bad label.\n>%s", _s);
		ins->opcode = OP_LABEL;
		ins->dst.amode = AMODE_MEM_LABEL;
		s++;
	}
	else
	{
		// Instruction
		string cmd;
		ParseStringT(s, cmd, ".");
		const OpInfo *opinfo = NULL;
		for( int i=0; OPINFO[i].opcode!=OP_INVALID; i++ )
			if( cmd==OPINFO[i].name )
			{
				opinfo = &OPINFO[i];
				break;
			}
		if( !opinfo ) throw format("Invalid opcode '%s'.\n>%s", cmd.c_str(), _s);
		ins->opcode = opinfo->opcode;

		if( opinfo->sizes )
		{
			if( *s++!='.' ) throw format("Data size expected.\n>%s", _s);
			if( *s=='b' && (opinfo->sizes&1) ) ins->width = SIZE_BYTE;
			else if( *s=='w' && (opinfo->sizes&2) ) ins->width = SIZE_WORD;
			else if( *s=='l' && (opinfo->sizes&4) ) ins->width = SIZE_LONG;
			else throw format("'%c' is not a valid size.\n>%s", *s, _s);
			s++;
		}
		else
			ins->width = SIZE_UNSIZED;

		// Arguments
		if( opinfo->argnum == 1 )
			ins->dst = ParseEAddr(s, _s);
		else if( opinfo->argnum == 2 )
		{
			ins->src = ParseEAddr(s, _s);
			if( *s++!=',' ) throw format("',' expected.\n>%s", _s);
			ins->dst = ParseEAddr(s, _s);
		}
	}
	ParseWhitespace(s);
	if( *s==';' )
	{
		s++;
		ParseWhitespace(s);
		ins->comment = s;
	}
	else if( *s )
		throw format("Extra characters after instruction.\n>%s", _s);

	ins->Cleanup();
}


EAddr Assembler::ParseEAddr(const char *&s, const char *_line)
{
	vector<LexSymbol> lex;
	EAddr ea;
	const char *_s = s;

	// Lexer
	while( 1 )
	{
		ParseWhitespace(s);
		if( *s=='#' || *s=='-' || *s=='+' || *s=='.' || *s=='/' || *s=='(' || *s==')' || *s=='"' || *s=='=')
		{
			lex.push_back(LexSymbol(*s, string(s, s+1)));
			s++;
			continue;
		}
		if( *s>='0' && *s<='9' )
		{
			const char *b = s;
			while( *s>='0' && *s<='9' ) s++;
			lex.push_back(LexSymbol('0', string(b, s)));
			continue;
		}
		if( *s=='$' )
		{
			const char *b = s;
			while( (*s>='0' && *s<='9') || (*s>='a' && *s<='f') || (*s>='A' && *s<='F') ) s++;
			lex.push_back(LexSymbol('0', string(b, s)));
			continue;
		}
		string id;
		const char *b = s;
		while( (*s>='0' && *s<='9') || (*s>='a' && *s<='z') || (*s>='A' && *s<='Z') || *s=='_' || *s=='.' ) s++;
		id = string(b, s);
		if( id.size()<=0 ) break;
		char type = 'I';
		if( id.size()==2 && id[0]=='d' && id[1]>='0' && id[1]<='7' ) type = 'D';
		if( id.size()==2 && id[0]=='a' && id[1]>='0' && id[1]<='7' ) type = 'A';
		lex.push_back(LexSymbol(type, id));
	}

	lex.push_back(LexSymbol('E', ""));

	// Parser - bitmask
	{
		int pos = 0;
		int mask = 0;
		int nreg = 0;
		while( 1 )
		{
			int reg = -1;
			if( lex[pos].code=='D' ) reg = lex[pos].text[1] - '0';
			if( lex[pos].code=='A' ) reg = lex[pos].text[1] - '0' + 8;
			if( reg<0 ) break;
			pos++;

			if( lex[pos].code=='-' )
			{
				pos++;
				int ereg = -1;
				if( lex[pos].code=='D' ) ereg = lex[pos].text[1] - '0';
				if( lex[pos].code=='A' ) ereg = lex[pos].text[1] - '0' + 8;
				if( ereg<0 ) break;
				pos++;
				for( int i=reg; i<=ereg; i++ )
					mask |= 1<<i, nreg++;
			}

			if( lex[pos].code!='/' ) break;
			pos++;
		}
		if( lex[pos].code=='E' && nreg>=2 )
		{
			ea.amode = AMODE_REGMASK;
			ea.value = mask;
			return ea;
		}
	}

	// Parser - string
	if( lex.size()>=3 && lex[0].code=='"' && lex[lex.size()-2].code=='"' )
	{
		const char *b = _s;
		const char *e = s;
		ParseWhitespace(b);
		if( *b=='"' ) b++;
		while( e>b && (e[-1]==' ' || e[-1]=='\t' || e[-1]=='"') ) e--;
		ea.amode = AMODE_STRING;
		ea.label = string(b, e);
		return ea;
	}

	// Parser - <ea>
	int pos = 0;
	if( lex[pos].code=='D' )
	{
		//	d#
		ea.amode = AMODE_REG;
		ea.regnum = lex[pos].text[1]-'0';
		pos++;
		if( lex[pos].code!='E' ) throw format("Can't parse argument '%s'.\n>%s", string(_s, s).c_str(), _line);
		return ea;
	}
	if( lex[pos].code=='A' )
	{
		//	a#
		ea.amode = AMODE_REG;
		ea.regnum = lex[pos].text[1]-'0'+8;
		pos++;
		if( lex[pos].code!='E' ) throw format("Can't parse argument '%s'.\n>%s", string(_s, s).c_str(), _line);
		return ea;
	}

	if(lex[pos].code=='#')
	{
		//	#imm
		ea.amode = AMODE_IMM;
		pos++;
	}

	if( lex[pos].code=='I' )
	{
		ea.label = lex[pos++].text;
	}

	if( lex[pos].code=='0' || (lex[pos].code=='-' && lex[pos+1].code=='0') )
	{
		int sign = 1;
		if( lex[pos].code=='-' ) sign=-1, pos++;
		int value = 0;
		if( lex[pos].text[0]=='$' )
		{
			const char *p = lex[pos].text.c_str() + 1;
			value = ParseHex(p) * sign;
			ea.hex_value = true;
			if( *p )throw format("Can't parse argument '%s'.\n>%s", string(_s, s).c_str(), _line);
		}
		else
		{
			const char *p = lex[pos].text.c_str();
			value = ParseInt(p) * sign;
			if( *p )throw format("Can't parse argument '%s'.\n>%s", string(_s, s).c_str(), _line);
		}
		pos++;

		if( ea.amode==AMODE_IMM )
		{
			ea.value = value;
			if( lex[pos].code!='E' ) throw format("Can't parse argument '%s'.\n>%s", string(_s, s).c_str(), _line);
			return ea;
		}

		ea.offs = value;
	}

	if( lex[pos].code=='E' && ea.label.size()>0 )
	{
		ea.amode = AMODE_MEM_LABEL;
		return ea;
	}

	bool predec = false;
	if( lex[pos].code=='-' ) predec = true, pos++;

	// one of the () modes expected at this point
	if( lex[pos++].code!='(' ) throw format("Can't parse argument '%s'.\n>%s", string(_s, s).c_str(), _line);

	if( lex[pos].text=="pc" )
	{
		pos++;
		ea.amode = AMODE_MEM_REL_PC;
	}
	else if( lex[pos].code=='A' )
	{
		ea.regnum = lex[pos++].text[1] - '0' + 8;
		ea.amode = AMODE_MEM_A;
		if( lex[pos].code==',' )
		{
			pos++;
			if( lex[pos].code!='D' ) throw format("Can't parse argument '%s'.\n>%s", string(_s, s).c_str(), _line);
			ea.index_reg = lex[pos++].text[1] - '0';
			ea.amode = AMODE_MEM_INDEX_L;
			if( lex[pos].code=='.' && lex[pos+1].text=="w" )
			{
				ea.amode = AMODE_MEM_INDEX_W;
				pos += 2;
			}
		}
	}
	else
		throw format("Can't parse argument '%s'.\n>%s", string(_s, s).c_str(), _line);

	if( lex[pos++].code!=')' ) throw format("Can't parse argument '%s'.\n>%s", string(_s, s).c_str(), _line);

	bool postinc = false;
	if( lex[pos].code=='+' ) postinc = true, pos++;

	if( (predec || postinc) && ea.amode!=AMODE_MEM_A ) throw format("Can't parse argument '%s'.\n>%s", string(_s, s).c_str(), _line);
	if( predec && postinc ) throw format("Can't parse argument '%s'.\n>%s", string(_s, s).c_str(), _line);
	if( predec ) ea.amode = AMODE_MEM_A_PREDEC;
	if( postinc ) ea.amode = AMODE_MEM_A_POSTINC;

	if( lex[pos++].code!='E' ) throw format("Can't parse argument '%s'.\n>%s", string(_s, s).c_str(), _line);

	return ea;
}


void Assembler::Print()
{
	for( int i=0; i<(int)instr.size(); i++ )
		printf("%s\n", instr[i]->ToString().c_str());
}

void Assembler::SaveFunction(const string &path, const string &fn_name)
{
	FILE *fp = fopen(path.c_str(), "wt");
	if( !fp ) return;

	fprintf(fp, "\n");
	fprintf(fp, "    public %s\n", fn_name.c_str());
	fprintf(fp, "%s:\n", fn_name.c_str());
	fprintf(fp, "\n");
	for( int i=0; i<(int)instr.size(); i++ )
		fprintf(fp, "%s\n", instr[i]->ToString().c_str());

	fclose(fp);
}



//Assembler::LiveRangeInfo &Assembler::UseReg(int instr_index, int regnum)
//{
//	LiveRangeInfo &lr = live_ranges[regnum];
//	if( !lr.valid )
//	{
//		lr.valid = true;
//		lr.i_start = instr_index;
//		lr.i_end = instr_index+1;
//	}
//	if( lr.i_start > instr_index ) lr.i_start = instr_index;
//	if( lr.i_end < instr_index+1 ) lr.i_end = instr_index+1;
//	return lr;
//}

void Assembler::LinkInstr(Instruction *from, Instruction *to)
{
	bool found = false;
	for( int i=0; i<(int)from->link_next.size(); i++ )
		if( from->link_next[i]==to )
		{
			found = true;
			break;
		}
	if( !found )
		from->link_next.push_back(to);

	found = false;
	for( int i=0; i<(int)to->link_prev.size(); i++ )
		if( to->link_prev[i]==from )
		{
			found = true;
			break;
		}
	if( !found )
		to->link_prev.push_back(from);
}

void Assembler::LinkInstructions()
{
	for( int i=0; i<(int)instr.size(); i++ )
	{
		instr[i]->link_prev.clear();
		instr[i]->link_next.clear();
	}

	for( int i=0; i<(int)instr.size(); i++ )
	{
		Instruction *ins = instr[i];
		bool link_next = true;
		bool link_label = false;

		if( ins->opcode==OP_BRA ||
			ins->opcode==OP_RTS ||
			ins->opcode==OP_PREPROC_ELIF ||
			ins->opcode==OP_PREPROC_ELSE )
			link_next = false;		// JMP is assumed to behave like JSR in this case

		if( ins->opcode==OP_BRA ||
			ins->opcode==OP_BSR ||
			ins->opcode==OP_BEQ ||
			ins->opcode==OP_BNE ||
			ins->opcode==OP_BGE ||
			ins->opcode==OP_BGT ||
			ins->opcode==OP_BLE ||
			ins->opcode==OP_BLT ||
			ins->opcode==OP_BCC ||
			ins->opcode==OP_BHI ||
			ins->opcode==OP_BLS ||
			ins->opcode==OP_BCS ||
			ins->opcode==OP_BMI ||
			ins->opcode==OP_BPL ||
			ins->opcode==OP_DBRA )
			link_label = true;

		if( i+1<(int)instr.size() && link_next )
		{
			// Follow to next instruction
			LinkInstr(ins, instr[i+1]);
		}

		if( link_label )
		{
			// Potential jump to label
			for( int j=0; j<(int)instr.size(); j++ )
				if( instr[j]->opcode==OP_LABEL && instr[j]->dst.label==ins->dst.label )
				{
					LinkInstr(ins, instr[j]);
					break;
				}
		}

		if( ins->opcode==OP_PREPROC_IF || ins->opcode==OP_PREPROC_ELIF || ins->opcode==OP_PREPROC_ELSE )
		{
			// link conditional preprocessor instructions
			//

			int level = 0;
			for( int j=i+1; j<(int)instr.size(); j++ )
			{
				if( instr[j]->opcode==OP_PREPROC_IF )
					level++;

				if( level==0 )
				{
					if( ins->opcode==OP_PREPROC_IF )
					{
						// link IF to first instruction in every other ELIF/ELSE block
						if( instr[j]->opcode==OP_PREPROC_ELIF || instr[j]->opcode==OP_PREPROC_ELSE )
							if( j+1<(int)instr.size() )
								LinkInstr(ins, instr[j+1]);
				
						if( instr[j]->opcode==OP_PREPROC_ENDIF )
							break;
					}
					else
					{
						// link ELIF/ELSE to ENDIF instruction
						if( instr[j]->opcode==OP_PREPROC_ENDIF )
						{
							LinkInstr(ins, instr[j]);
							break;
						}
					}
				}

				if( instr[j]->opcode==OP_PREPROC_ENDIF )
					level--;
			}
		}

	}
}

void Assembler::ClearMarkers(const vector<Instruction*> &list)
{
	for( int i=0; i<(int)list.size(); i++ )
		list[i]->marker = 0;
}


void Assembler::RecurseLiveList(Assembler::LiveRangeInfo &lr, vector<Instruction*> &live, Instruction *ins, int reg)
{
	while(1)
	{
		if( ins->marker ) return;

		live.push_back(ins);
		ins->marker = 1;

		// Mark usage
		for( int a=0; a<2; a++ )
		{
			EAddr &ea = *(a ? &ins->dst : &ins->src);
			if( ea.regnum==reg )
				if( ea.amode==AMODE_MEM_A || ea.amode==AMODE_MEM_A_POSTINC || ea.amode==AMODE_MEM_A_PREDEC || ea.amode==AMODE_MEM_INDEX_W || ea.amode==AMODE_MEM_INDEX_L )
					lr.req_areg = true;

			if( ea.index_reg==reg )
				if( ea.amode==AMODE_MEM_INDEX_W || ea.amode==AMODE_MEM_INDEX_L )
					lr.req_dreg = true;
		}

		// End live range?
		if( ins->opcode==OP_MOVE || ins->opcode==OP_LEA )
			if( ins->dst.amode==AMODE_REG && ins->dst.regnum==reg )
				return;

		// Propagate
		if( ins->link_prev.size() <= 0 )
			break;

		for( int i=1; i<(int)ins->link_prev.size(); i++ )
			RecurseLiveList(lr, live, ins->link_prev[i], reg);
		
		ins = ins->link_prev[0];	// optimize simple cases
	}
}

void Assembler::StartRecurseLiveList(Instruction *ins, int reg)
{
	if( reg<0 ) return;

	static vector<Instruction*> live;
	static vector<Instruction*> _live;
	live.clear();

	LiveRangeInfo &lr = live_ranges[reg];
	lr.valid = true;

	RecurseLiveList(lr, live, ins, reg);
	ClearMarkers(live);

	sort(live.begin(), live.end());

	_live.swap(lr.live);
	lr.live.clear();

	int p1=0, p2=0;
	while( 1 )
	{
		Instruction *i1 = (p1<(int)live.size() ? live[p1] : NULL);
		Instruction *i2 = (p2<(int)_live.size() ? _live[p2] : NULL);
		if( !i1 && !i2 ) break;
		if( (i1 && i1<i2) || !i2 )	lr.live.push_back(i1), p1++;
		else						lr.live.push_back(i2), p2++;
	}
}

void Assembler::FindLiveLists()
{
	live_ranges.clear();

	ClearMarkers(instr);

	for( int i=0; i<(int)instr.size(); i++ )
	{
		Instruction *ins = instr[i];
		StartRecurseLiveList(ins, ins->src.regnum);
		StartRecurseLiveList(ins, ins->src.index_reg);
		StartRecurseLiveList(ins, ins->dst.regnum);
		StartRecurseLiveList(ins, ins->dst.index_reg);
	}
}

#if 0
void Assembler::FindLiveRagnes()
{
	live_ranges.clear();

	for( int i=0; i<instr.size(); i++ )
	{
		Instruction *ins = instr[i];
		if( ins->block_mode==0 )
		{
			// Header: consider only registers that are explicitly written
			if( ins->opcode!=OP_MOVE && ins->opcode!=OP_LEA ) continue;
			if( ins->dst.amode!=AMODE_REG ) continue;

			LiveRangeInfo &lr = live_ranges[ins->dst.regnum];
			if( !lr.valid )
			{
				lr.valid = true;
				lr.i_start = i;
			}
			lr.i_end = instr.size();
			if( ins->opcode==OP_LEA )
				lr.req_areg = true;
			continue;
		}

		// Loop body - look at all registers use
		for( int a=0; a<2; a++ )
		{
			EAddr &ea = (a==0 ? ins->src : ins->dst);
			if( ea.amode==AMODE_REG )
				UseReg(i, ea.regnum);
			if( ea.amode==AMODE_MEM_A || ea.amode==AMODE_MEM_A_POSTINC || ea.amode==AMODE_MEM_A_PREDEC )
				UseReg(i, ea.regnum).req_areg = true;
			if( ea.amode==AMODE_MEM_INDEX_W || ea.amode==AMODE_MEM_INDEX_L )
			{
				UseReg(i, ea.regnum).req_areg = true;
				UseReg(i, ea.index_reg).req_dreg = true;
			}
		}

		// specials
		if( ins->opcode==OP_LEA )
			UseReg(i, ins->dst.regnum).req_areg = true;
	}
}
#endif

void Assembler::ReserveLiveRanges()
{
	for( int r=0; r<16; r++ )
	{
		LiveRangeInfo &lr = live_ranges[r];
		if( !lr.valid ) continue;

		//for( int i=lr.i_start; i<lr.i_end; i++ )
		//	instr[i]->free_regs &= ~(1<<r);

		for( int i=0; i<(int)lr.live.size(); i++ )
			lr.live[i]->free_regs &= ~(1<<r);
	}
}

void Assembler::RenumberRegister(int i_start, int i_end, int reg_from, int reg_to)
{
	for( int i=i_start; i<i_end; i++ )
	{
		Instruction *ins = instr[i];

		for( int a=0; a<2; a++ )
		{
			EAddr &ea = *(a==0 ? &ins->src : &ins->dst);
			if( ea.amode==AMODE_REG || ea.amode==AMODE_MEM_A || ea.amode==AMODE_MEM_A_POSTINC || ea.amode==AMODE_MEM_A_PREDEC || ea.amode==AMODE_MEM_INDEX_W || ea.amode==AMODE_MEM_INDEX_L )
				if( ea.regnum==reg_from )
					ea.regnum = reg_to;
			
			if( ea.amode==AMODE_MEM_INDEX_W || ea.amode==AMODE_MEM_INDEX_L )
				if( ea.index_reg==reg_from )
					ea.index_reg = reg_to;
		}
	}
}

void Assembler::RenumberRegister(const vector<Instruction*> &list, int reg_from, int reg_to)
{
	for( int i=0; i<(int)list.size(); i++ )
	{
		Instruction *ins = list[i];

		for( int a=0; a<2; a++ )
		{
			EAddr &ea = *(a==0 ? &ins->src : &ins->dst);
			if( ea.amode==AMODE_REG || ea.amode==AMODE_MEM_A || ea.amode==AMODE_MEM_A_POSTINC || ea.amode==AMODE_MEM_A_PREDEC || ea.amode==AMODE_MEM_INDEX_W || ea.amode==AMODE_MEM_INDEX_L )
				if( ea.regnum==reg_from )
					ea.regnum = reg_to;

			if( ea.amode==AMODE_MEM_INDEX_W || ea.amode==AMODE_MEM_INDEX_L )
				if( ea.index_reg==reg_from )
					ea.index_reg = reg_to;
		}
	}
}




void Assembler::AllocateRegs(int free_regs)
{
	for( int i=0; i<(int)instr.size(); i++ )
		instr[i]->free_regs = free_regs;

	LinkInstructions();
	//FindLiveRagnes();
	FindLiveLists();
	ReserveLiveRanges();

	// Find register to be allocated
	for( auto p=live_ranges.begin(); p!=live_ranges.end(); ++p )
	{
		if( p->first<16 || !p->second.valid )
			continue;

		// Find available register mask
		LiveRangeInfo &lr = p->second;
		int free_mask = 0xFFFF;
		//for( int i=lr.i_start; i<lr.i_end; i++ )
		//	free_mask &= instr[i]->free_regs;
		for( int i=0; i<(int)lr.live.size(); i++ )
			free_mask &= lr.live[i]->free_regs;

		if( lr.req_dreg ) free_mask &= 0x00FF;
		if( lr.req_areg ) free_mask &= 0xFF00;
		if( !free_mask )
			throw format("Can't allocate register x%d", p->first-16);

		// Pick first available register
		int reg = 0;
		while( !(free_mask & (1<<reg)) )
			reg++;

		//RenumberRegister(lr.i_start, lr.i_end, p->first, reg);
		RenumberRegister(lr.live, p->first, reg);

		// Reserve
		//for( int i=lr.i_start; i<lr.i_end; i++ )
		//	instr[i]->free_regs &= ~(1<<reg);
		for( int i=0; i<(int)lr.live.size(); i++ )
			lr.live[i]->free_regs &= ~(1<<reg);


		//LOG: printf("$ Allocated x%d -> %s\n", p->first-16, _regname(reg).c_str());
	}
}


void Assembler::ConvertOpcodes()
{
	for( int i=0; i<(int)instr.size(); i++ )
		instr[i]->ConvertOpcode();
}

// Set min/max to minimum/maximum number of cycles taken to execute code from given instruction
void Assembler::CountCyclesRec(Instruction *instr)
{
	if( instr->marker )
		return;

	instr->tree_cycles_min = 0;
	instr->tree_cycles_max = 0;
	instr->marker = 1;

	for( int i=0; i<(int)instr->link_next.size(); i++ )
	{
		Instruction *next = instr->link_next[i];
		
		if( instr->opcode==OP_BEQ && next->dst.label==".continue" )
			continue;	// ignore fast exit path

		CountCyclesRec(next);

		if( instr->tree_cycles_min==0 && instr->tree_cycles_max==0 )
		{
			instr->tree_cycles_min = next->tree_cycles_min;
			instr->tree_cycles_max = next->tree_cycles_max;
		}
		else
		{
			if( next->tree_cycles_min<instr->tree_cycles_min ) instr->tree_cycles_min = next->tree_cycles_min;
			if( next->tree_cycles_max>instr->tree_cycles_max ) instr->tree_cycles_max = next->tree_cycles_max;
		}
	}

	if( instr->block_mode )
	{
		instr->tree_cycles_min += instr->cycles;
		instr->tree_cycles_max += instr->cycles;
	}
}

void Assembler::CountCycles(int &cmin, int &cmax)
{
	for( int i=0; i<(int)instr.size(); i++ )
		instr[i]->CountCycles();


	ClearMarkers(instr);

	CountCyclesRec(instr[0]);
	cmin = instr[0]->tree_cycles_min;
	cmax = instr[0]->tree_cycles_max;
}
