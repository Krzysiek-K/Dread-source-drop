
#pragma once

namespace instructions {

	struct UnsizedUnaryInstruction {
		opcode_t opcode;
		UnsizedUnaryInstruction(opcode_t op) : opcode(op) {}
		Instruction *operator()(EAddr dst)
		{
			Instruction *ins = new Instruction();
			ins->opcode = opcode;
			ins->width = SIZE_UNSIZED;
			ins->dst = dst;
			return ins;
		}
	};

	struct UnaryInstruction {
		struct Gen {
			opcode_t opcode;
			width_t	size;
			Gen(opcode_t op, width_t w) : opcode(op), size(w) {}
			Instruction *operator()(EAddr dst);
		};
		Gen		b, w, l;
		UnaryInstruction(opcode_t op) : b(op, SIZE_BYTE), w(op, SIZE_WORD), l(op, SIZE_LONG) {}
	};

	struct BinaryInstruction {
		struct Gen {
			opcode_t opcode;
			width_t	size;
			Gen(opcode_t op, width_t w) : opcode(op), size(w) {}
			Instruction *operator()(EAddr src, EAddr dst);
		};
		Gen		b, w, l;
		BinaryInstruction(opcode_t op) : b(op,SIZE_BYTE), w(op, SIZE_WORD), l(op, SIZE_LONG) {}
	};

	extern EAddr d0;
	extern EAddr d1;
	extern EAddr d2;
	extern EAddr d3;
	extern EAddr d4;
	extern EAddr d5;
	extern EAddr d6;
	extern EAddr d7;
	extern EAddr d[8];
	extern EAddr a0;
	extern EAddr a1;
	extern EAddr a2;
	extern EAddr a3;
	extern EAddr a4;
	extern EAddr a5;
	extern EAddr a6;
	extern EAddr a7;
	extern EAddr a[8];
	inline EAddr xr(int num) { return EAddr::x(num); }
	inline EAddr mem(EAddr ea) {
		if( ea.amode!=AMODE_REG ) throw string("Register expected for <ea>, got: "+ea.ToString());
		ea.amode = AMODE_MEM_A;
		return ea;
	}
	inline EAddr mem_postinc(EAddr ea) {
		if( ea.amode!=AMODE_REG ) throw string("Register expected for <ea>, got: "+ea.ToString());
		ea.amode = AMODE_MEM_A_POSTINC;
		return ea;
	}
	inline EAddr mem_predec(EAddr ea) {
		if( ea.amode!=AMODE_REG ) throw string("Register expected for <ea>, got: "+ea.ToString());
		ea.amode = AMODE_MEM_A_PREDEC;
		return ea;
	}
	inline EAddr mem(const string &label, EAddr ea) {
		if( ea.amode!=AMODE_REG ) throw string("Register expected for <ea>, got: "+ea.ToString());
		ea.amode = AMODE_MEM_A;
		ea.label = label;
		return ea;
	}
	inline EAddr mem(int offs, EAddr ea) {
		if( ea.amode!=AMODE_REG ) throw string("Register expected for <ea>, got: "+ea.ToString());
		ea.amode = AMODE_MEM_A;
		ea.offs = offs;
		return ea;
	}
	inline EAddr mem_idx_w(EAddr ea, EAddr idx) {
		if( ea.amode!=AMODE_REG ) throw string("Register expected for <ea>, got: "+ea.ToString());
		if( idx.amode!=AMODE_REG ) throw string("Register expected for <index>, got: "+idx.ToString());
		ea.amode = AMODE_MEM_INDEX_W;
		ea.index_reg = idx.regnum;
		return ea;
	}
	inline EAddr mem_idx_w(int offs, EAddr ea, EAddr idx) {
		if( ea.amode!=AMODE_REG ) throw string("Register expected for <ea>, got: "+ea.ToString());
		if( idx.amode!=AMODE_REG ) throw string("Register expected for <index>, got: "+idx.ToString());
		ea.amode = AMODE_MEM_INDEX_W;
		ea.index_reg = idx.regnum;
		ea.offs = offs;
		return ea;
	}
	inline EAddr mem_idx_w(string offs, EAddr ea, EAddr idx) {
		if( ea.amode!=AMODE_REG ) throw string("Register expected for <ea>, got: "+ea.ToString());
		if( idx.amode!=AMODE_REG ) throw string("Register expected for <index>, got: "+idx.ToString());
		ea.amode = AMODE_MEM_INDEX_W;
		ea.index_reg = idx.regnum;
		ea.offs = 0;
		ea.label = offs;
		return ea;
	}
	inline EAddr label(const string &label, int offs=0)		{ EAddr ea; ea.amode=AMODE_MEM_LABEL; ea.label=label; ea.offs=offs; return ea; }
	inline EAddr pc_rel(const string &label, int offs=0)	{ EAddr ea; ea.amode=AMODE_MEM_REL_PC; ea.label=label; ea.offs=offs; return ea; }
	inline EAddr imm(int value)								{ EAddr ea; ea.amode=AMODE_IMM; ea.value=value; return ea; }
	inline EAddr imm(string value)							{ EAddr ea; ea.amode=AMODE_IMM; ea.label=value; return ea; }
	inline EAddr immhex(int value)							{ EAddr ea; ea.amode=AMODE_IMM; ea.value=value; ea.hex_value=true; return ea; }
	extern BinaryInstruction		mov;
	extern UnaryInstruction			swp;
	extern UnaryInstruction			ext;
	extern UnaryInstruction			neg;
	extern UnaryInstruction			clr;
	extern BinaryInstruction		lea;
	extern BinaryInstruction		add;
	extern BinaryInstruction		addx;
	extern BinaryInstruction		sub;
	extern BinaryInstruction		cmp;
	extern BinaryInstruction		lsr;
	extern BinaryInstruction		asr;
	extern BinaryInstruction		lsl;
	extern BinaryInstruction		muls;
	extern BinaryInstruction		mulu;
	extern BinaryInstruction		divs;
	extern BinaryInstruction		divu;
	extern BinaryInstruction		and;
	extern BinaryInstruction		or;
	extern BinaryInstruction		eor;
	extern UnsizedUnaryInstruction	jmp;
	extern UnsizedUnaryInstruction	bra;
	extern UnsizedUnaryInstruction	beq;
	extern UnsizedUnaryInstruction	bge;
	extern UnsizedUnaryInstruction	bgt;
	extern UnsizedUnaryInstruction	ble;
	extern UnsizedUnaryInstruction	blt;
	extern BinaryInstruction		dbra;


};

