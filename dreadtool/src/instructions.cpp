
#include "common.h"
#include "script_compiler.h"


namespace instructions {


EAddr d0 = EAddr::d(0);
EAddr d1 = EAddr::d(1);
EAddr d2 = EAddr::d(2);
EAddr d3 = EAddr::d(3);
EAddr d4 = EAddr::d(4);
EAddr d5 = EAddr::d(5);
EAddr d6 = EAddr::d(6);
EAddr d7 = EAddr::d(7);
EAddr d[8] ={ EAddr::d(0), EAddr::d(1), EAddr::d(2), EAddr::d(3), EAddr::d(4), EAddr::d(5), EAddr::d(6), EAddr::d(7) };
EAddr a0 = EAddr::a(0);
EAddr a1 = EAddr::a(1);
EAddr a2 = EAddr::a(2);
EAddr a3 = EAddr::a(3);
EAddr a4 = EAddr::a(4);
EAddr a5 = EAddr::a(5);
EAddr a6 = EAddr::a(6);
EAddr a7 = EAddr::a(7);
EAddr a[8] ={ EAddr::a(0), EAddr::a(1), EAddr::a(2), EAddr::a(3), EAddr::a(4), EAddr::a(5), EAddr::a(6), EAddr::a(7) };

BinaryInstruction		mov	(OP_MOVE);			//	<src>, <dst>
UnaryInstruction		swp	(OP_SWAP);			//	<dst>
BinaryInstruction		lea	(OP_LEA);			//	<src>, <dst>
BinaryInstruction		add	(OP_ADD);			//	<src>, <dst>
BinaryInstruction		sub	(OP_SUB);			//	<src>, <dst>
BinaryInstruction		cmp	(OP_CMP);			//	<src>, <dst>
BinaryInstruction		lsr	(OP_LSR);			//	<src>, <dst>
BinaryInstruction		asr	(OP_ASR);			//	<src>, <dst>
BinaryInstruction		lsl	(OP_LSL);			//	<src>, <dst>
BinaryInstruction		muls_(OP_MULS);			//	<src>, <dst>
BinaryInstruction		mulu_(OP_MULU);			//	<src>, <dst>
BinaryInstruction		divs_(OP_DIVS);			//	<src>, <dst>
BinaryInstruction		divu_(OP_DIVU);			//	<src>, <dst>
BinaryInstruction		and	(OP_AND);			//	<src>, <dst>
BinaryInstruction		or	(OP_OR);			//	<src>, <dst>
BinaryInstruction		eor	(OP_EOR);			//	<src>, <dst>
UnsizedUnaryInstruction	jmp	(OP_JMP);			//	<dst>
UnsizedUnaryInstruction	jsr	(OP_JSR);			//	<dst>
UnsizedUnaryInstruction	bra	(OP_BRA);			//	<dst>
UnsizedUnaryInstruction	bsr	(OP_BSR);			//	<dst>
UnsizedUnaryInstruction	beq	(OP_BEQ);			//	<dst>
UnsizedUnaryInstruction	bge	(OP_BGE);			//	<dst>
UnsizedUnaryInstruction	ble	(OP_BLE);			//	<dst>
UnsizedUnaryInstruction	bmi	(OP_BMI);			//	<dst>
UnsizedUnaryInstruction	bpl	(OP_BPL);			//	<dst>
UnaryInstruction		ext (OP_EXT);			//	<dst>
UnaryInstruction		extb(OP_EXTB);			//	<dst>
BinaryInstruction		dbra(OP_DBRA);			//	<src>, <dst>



Instruction *UnaryInstruction::Gen::operator()(EAddr dst)
{
	Instruction *ins = new Instruction();
	ins->opcode = opcode;
	ins->width = size;
	ins->dst = dst;
	return ins;
}

Instruction *BinaryInstruction::Gen::operator()(EAddr src, EAddr dst)
{
	Instruction *ins = new Instruction();
	ins->opcode = opcode;
	ins->width = size;
	ins->src = src;
	ins->dst = dst;
	return ins;
}






};
