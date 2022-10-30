


; ================================================================ _memcpy ================================================================
	public	_memcpy
_memcpy:	
	move.l	4(sp),a0
	move.l	8(sp),a1
	move.l	12(sp),d1

	lsr.l	#4,d1
	move.l	d1,d0
	swap.w	d0
	subq.w	#1,d1	
	bmi.b	.1
.2:
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+
	dbf		d1,.2
.1:
	subq.w	#1,d0
	bpl.b	.2

	move.l	12(sp),d1
	and.w	#$f,d1
	subq.w	#1,d1
	bmi.b	.3
.4:
	move.b	(a1)+,(a0)+
	dbf	d1,.4
.3:
	move.l	4(sp),d0
	rts





TRAP_ICACHE_DISABLE: macro
	endm

TRAP_ICACHE_ENABLE: macro
	endm


	include	"dread_asm.s"
