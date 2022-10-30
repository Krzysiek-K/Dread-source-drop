
#pragma once


enum opcode_t {
	OP_EMPTY,			// no opcode, just comment

	OP_MOVE,			//	<src>, <dst>
	OP_MOVEQ,			//	<src>, <dst>			automatically converted
	OP_MOVEM,			//	<src>, <dst>
	OP_SWAP,			//	<dst>

	OP_LEA,				//	<src>, <dst>

	OP_ADD,				//	<src>, <dst>
	OP_ADDQ,			//	<src>, <dst>			automatically converted
	OP_SUB,				//	<src>, <dst>
	OP_SUBQ,			//	<src>, <dst>			automatically converted
	OP_CMP,				//	<src>, <dst>

	OP_LSR,				//	<src>, <dst>
	OP_ASR,				//	<src>, <dst>
	OP_LSL,				//	<src>, <dst>

	OP_MULS,			//	<src>, <dst>
	OP_MULU,			//	<src>, <dst>
	OP_DIVS,			//	<src>, <dst>
	OP_DIVU,			//	<src>, <dst>

	OP_AND,				//	<src>, <dst>
	OP_OR,				//	<src>, <dst>
	OP_EOR,				//	<src>, <dst>

	OP_JMP,				//	<dst>
	OP_JSR,				//	<dst>
	OP_BRA,				//	<dst>
	OP_BSR,				//	<dst>
	OP_BEQ,				//	<dst>		==
	OP_BNE,				//	<dst>		!=
	OP_BGE,				//	<dst>		>=	(signed)
	OP_BGT,				//	<dst>		>	(signed)
	OP_BLE,				//	<dst>		<=	(signed)
	OP_BLT,				//	<dst>		<	(signed)
	OP_BCC,				//	<dst>		>=	(unsigned)
	OP_BHI,				//	<dst>		>	(unsigned)
	OP_BLS,				//	<dst>		<=	(unsigned)
	OP_BCS,				//	<dst>		<	(unsigned)
	OP_BMI,				//	<dst>
	OP_BPL,				//	<dst>
	OP_EXT,				//	<dst>
	OP_EXTB,			//	<dst>
	OP_DBRA,			//	<src>, <dst>
	OP_RTS,				//	-

	OP_LABEL,			//	<dst>
	OP_PREPROC_IF,		//	<dst>
	OP_PREPROC_ELIF,	//	<dst>
	OP_PREPROC_ELSE,	//	<dst>
	OP_PREPROC_ENDIF,	//	<dst>

	OP_INVALID,
};

enum width_t {
	SIZE_BYTE,
	SIZE_WORD,
	SIZE_LONG,
	SIZE_UNSIZED,
};

enum addrmode_t {
	AMODE_IGNORED,			// unused
	AMODE_REG,				//   d#		/	  a#					D			A
	AMODE_MEM_A,			//  (a#)	/	###(a#)					(A)			0(A)		-0(A)		I(A,D)		I+0(A,D)	I-0(A,D)
	AMODE_MEM_A_POSTINC,	//  (a#)+								(A)+
	AMODE_MEM_A_PREDEC,		// -(a#)								-(A)
	AMODE_MEM_LABEL,		// ####.l								I			I+0			I-0
	AMODE_MEM_REL_PC,		// ####(pc)								I(pc)		I+0(pc)		I-0(pc)
	AMODE_MEM_INDEX_W,		// ##(a#,d#)							I(A,D)		I+0(A,D)	I-0(A,D)
	AMODE_MEM_INDEX_L,		// ##(a#,d#.w)							I(A,D.w)	I+0(A,D.w)	I-0(A,D.w)
	AMODE_IMM,				//  #imm								#0			#I			#I+0		#I-0
	AMODE_REGMASK,			//	reg/reg/reg...						<special>
	AMODE_STRING,			//	<special>							<special>

							// DReg AReg Ident Number ( ) - + . / #
};

struct EAddr {
	addrmode_t	amode = AMODE_IGNORED;	// regnums:		0-7: d0-d7		8-15: a0-a7		16+: x0-x999


										//	d#/a#/(a#)/(a#)+/-(a#)		###(a#)/###(pc)		###.l		##(a#,d#)/##(a#,d#.w)		#imm		reg/reg/reg...
	int			regnum = -1;			//			X						   X			  -					 A#					 -				-
	int			offs = 0;				//			-						   X			  Opt				 X					 -				-
	int			index_reg = -1;			//			-						   -			  -					 D#					 -				-
	int			value = 0;				//			-						   -			  -					 -					 X				X
	string		label;					//			-						   Opt			  X					 Opt				 X				-
	bool		hex_value = false;		// offs / value

	static EAddr d(int reg) { EAddr ea; ea.amode=AMODE_REG; ea.regnum=reg;		return ea; }
	static EAddr a(int reg) { EAddr ea; ea.amode=AMODE_REG; ea.regnum=reg+8;	return ea; }
	static EAddr x(int reg) { EAddr ea; ea.amode=AMODE_REG; ea.regnum=reg+16;	return ea; }

	void Cleanup();
	int EACycles(bool _long);
	bool IsMem() {
		return amode==AMODE_MEM_A || amode==AMODE_MEM_A_POSTINC || amode==AMODE_MEM_A_PREDEC || amode==AMODE_MEM_LABEL ||
				amode==AMODE_MEM_REL_PC || amode==AMODE_MEM_INDEX_W || amode==AMODE_MEM_INDEX_L;
	}
	string ToString();
};

class Instruction {
public:
	opcode_t	opcode;
	width_t		width = SIZE_UNSIZED;
	EAddr		src;
	EAddr		dst;
	int			movem_bitmask = 0;

	int			cycles = 0;
	int			tree_cycles_min = 0;
	int			tree_cycles_max = 0;
	string		comment;

	// allocator variables
	int						block_mode = 0;
	int						free_regs = 0;
	vector<Instruction*>	link_prev;
	vector<Instruction*>	link_next;
	int						marker;

	void Cleanup() { src.Cleanup(); dst.Cleanup(); }

	void ConvertOpcode();
	void CountCycles();

	string ToString();
};


class Assembler {
public:
	vector<Instruction*>	instr;

	~Assembler() { Clear(); }

	void Clear();
	void BlockMode(int mode)		{ block_mode = mode; }
	void Assemble(const char *s);
	void Assemble(const string &s) { Assemble(s.c_str()); }
	Instruction *Assemble(Instruction *ins) { instr.push_back(ins); ins->Cleanup(); ins->block_mode = block_mode; return ins; }
	Assembler &operator <<(Instruction *ins) { Assemble(ins); return *this; }
	void operator <<(const string &s) { if( instr.size()>0 ) instr.back()->comment += s; }
	void Print();
	void SaveFunction(const string &path, const string &fn_name);

	void AllocateRegs(int free_regs);
	void ConvertOpcodes();
	void CountCycles(int &cmin, int &cmax);

protected:

	struct LexSymbol {
		char	code;
		string	text;
		LexSymbol(char c, const string &t) : code(c), text(t) {}
	};

	struct LiveRangeInfo {
		vector<Instruction*>	live;	// all instructions during execution of which the register is live
		//int  i_start = 0;					// first instruction that uses the register
		//int  i_end = 0;						// first instruction that does NOT use the register
		bool valid = false;
		bool req_dreg = false;
		bool req_areg = false;
	};

	int block_mode = 0;
	map<int, LiveRangeInfo> live_ranges;

	EAddr ParseEAddr(const char *&s, const char *_line);

	LiveRangeInfo &UseReg(int instr_index, int regnum);
	void LinkInstr(Instruction *from, Instruction *to);
	void LinkInstructions();
	void ClearMarkers(const vector<Instruction*> &list);
	void RecurseLiveList(LiveRangeInfo &lr, vector<Instruction*> &live, Instruction *ins, int reg);
	void StartRecurseLiveList(Instruction *ins, int reg);
	void FindLiveLists();
	void FindLiveRagnes();
	void ReserveLiveRanges();
	void RenumberRegister(int i_start, int i_end, int reg_from, int reg_to);
	void RenumberRegister(const vector<Instruction*> &list, int reg_from, int reg_to);
	void CountCyclesRec(Instruction *instr);
};
