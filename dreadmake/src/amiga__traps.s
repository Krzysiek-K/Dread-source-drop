



;******************** Trap 0 - jump to (a0), function should return with 'rte' ********************
Trap0_Handler:
	jmp		(a0)
	;	rte


;******************** Trap 1 - cache control ********************
Trap1_Handler:
	mc68020			
	movec		cacr, d5			; d1 - enable mask | disable mask
	and.w		d1, d5				; d5 - used
	swap.w		d1
	or.w		d1, d5
	movec		d5, cacr
	mc68000				
Trap1_Handler_disabled:
	rte



;********************  Trap function - SetSR  ********************
;	d1.w	- new value of SR register
;
TRAP_SET_SR: macro
	lea.l		T0_SetSR, a0
	trap		#0
 endm

T0_SetSR:
	move.w		d1, (a7)
	rte


;********************  Trap function - critical begin ********************
; out:	d0.w	- old value of SR register
;		a0		 (used)
;
T0_CriticalBegin:
	move.w		(a7), d0
	or.w		#$0700, (a7)
	rte

	public _Critical_Begin
_Critical_Begin:
	lea.l		T0_CriticalBegin, a0
	trap		#0
	rts

CRITICAL_BEGIN: macro
	lea.l		T0_CriticalBegin, a0
	trap		#0
	endm



;********************  Trap function - critical end ********************
; in:	d0.w	- old value of SR register
;		a0		 (used)
;

T0_CriticalEnd:
	move.w		d0, (a7)
	rte

	public _Critical_End
_Critical_End:
	lea.l		T0_CriticalEnd, a0
	trap		#0
	rts

CRITICAL_END: macro
	lea.l		T0_CriticalEnd, a0
	trap		#0
	endm


; Disable 68020+ instruction cache
;	CACR bit  0		EnableI			Enable instruction cache
;	CACR bit  1		FreezeI			Freeze instruction cache
;	CACR bit  3		ClearI			Clear instruction cache
;	CACR bit  4		IBE				Instruction burst enable
;	CACR bit  8		EnableD			Enable data cache		(68030)
;	CACR bit  9		FreezeD			Freeze data cache		(68030)
;	CACR bit 11		ClearD			Clear data cache		(68030)
;	CACR bit 12		DBE				Data burst enable		(68030)
;	CACR bit 13		WriteAllocate3	Write-Allocate mode		(68030 - must always be set)


;********************  Trap function - ICache_Disable ********************
;
;	destroys: d0, a0
;
TRAP_ICACHE_DISABLE: macro
;	cmp.w		#20, _cpu_type
;	blo			.\@skip
;	lea.l		T0_ICache_Disable, a0
;	trap		#0
;.\@skip:

									; d1 - enable mask | disable mask
	move.l		#$000AEEFE, d1		; freeze instruction cache + disable data cache + disable data burst
	trap		#1

 endm

TRAP_ICACHE_FLUSH: macro
									; d1 - enable mask | disable mask
	move.l		#$0008FFFF, d1		; clear instruction cache
	trap		#1
 endm

TRAP_LINEDRAW_SETUP_DONE: macro
	move.l		d1, -(a7)
	move.l		d5, -(a7)
	move.l		#$080AFFFF, d1		; clear instruction & data caches + freeze instruction cache
	trap		#1
	move.l		(a7)+, d5
	move.l		(a7)+, d1
 endm


T0_ICache_Disable:
	mc68020			
	movec		cacr, d0
;	and.w		#$FFFE, d0
	or.w		#$0002, d0		; TBD: test
	movec		d0, cacr
	mc68000				
	rte


;********************  Trap function - ICache_Enable ********************
;
;	destroys: d0, a0
;
TRAP_ICACHE_ENABLE: macro
;	cmp.w		#20, _cpu_type
;	blo			.\@skip
;	lea.l		T0_ICache_Enable, a0			; TBD: reenable
;	trap		#0
;.\@skip:

									; d1 - enable mask | disable mask
	move.l		#$0001FFFD, d1		; enable + unfreeze instruction cache
	trap		#1

 endm

T0_ICache_Enable:
	mc68020			
	movec		cacr, d0
;	or.w		#$0001, d0
	and.w		#$FFFD, d0
	movec		d0, cacr
	mc68000				
	rte



;********************  Trap function - atomic pointer write ********************
T0_Atomic_WritePointer:
	move.w		sr, d1				; disable interrupts
	or.w		#$0700, d1
	move.w		d1, sr
	move.l		d0, (a1)			; write

	rte								; return and restore interrupts


	public _Atomic_WritePointer			; a1: target	d0: value
_Atomic_WritePointer:
	lea.l		T0_Atomic_WritePointer, a0
	trap		#0
	rts




;********************  Trap function - DetectCPU  ********************
	public	_DetectCPU
_DetectCPU:
	lea.l		T0_DetectCPU, a0
	trap		#0
	rts

T0_DetectCPU:
	movem.l		d0-d3/a1-a3, -(a7)
	move.w		sr, d0
	move.w		#$2700, d1
	move.w		d1, sr

	; Supervisor mode

	; Registers:
	;
	;	d0	- old SR
	;	d1	- CPU type
	;	d2	- old vector 4 pointer				(both 68000 and 68010+)
	;	d3	- VBR / old vector 11 pointer		(68010+)
	;
	;	a0	- (temp)
	;	a1	- vector 4 address					(both 68000 and 68010+)
	;	a2	- vector 11 address					(68010+)
	;	a3	- saved stack pointer
	;

	moveq		#0, d1					; CPU = 68000
	moveq		#$10, d2				; A1 = $000010: vector 4 - illegal instruction (68000 only)
	move.l		d2, a1					;
	move.l		(a1), d2				; D2 = old vector 4
	lea			.vect4_00(pc), a0		; change to .vect4_00
	move.l		a0, (a1)				;

	move.l		a7, a3					; save stack pointer in A3

	dc.l		$4E7A3801				; !!! MOVEC VBR, D3 - load VBR		(invokes vector 4 on 68000 only)
	
	; CPU 68010+

	move.l		d2, (a1)				; restore "68000" vector 4

	add.l		d3, a1					; A1 = VBR+$10: vector 4 - illegal instruction (68010+)
	move.l		(a1), d2				; D2 = old vector 4
	lea			.vect4_10plus(pc), a0	; change to .vect4_10plus
	move.l		a0, (a1)				;

	move.l		d3, a2					; A2 = VBR+$2C: vector 11 - line 1111 emulator	(invoked when CPU hits an unknown instruction starting with $Fxxx)
	moveq		#$2C, d3				;
	add.l		d3, a2					;
	move.l		(a2), d3				; D3 = old vector 11
	lea			.vect11(pc), a0			; change to .vect11
	move.l		a0, (a2)				;

	moveq		#10, d1					; CPU = 68010
	dc.l		$4E7A1002				; !!! MOVEC CACR, D1				(supported only in 68020/30/40/60)
	moveq		#20, d1					; CPU = 68020/68030
	dc.l		$4E7A1004				; !!! MOVEC ITT0, D1				(supported only in 68040/60)
	moveq		#40, d1					; CPU = 68040
	dc.l		$4E7A1808				; !!! MOVEC PCR, D1					(supported only in 68060)
	moveq		#60, d1					; CPU = 68060

.vect11:
	move.l		d3, (a2)				; restore vector 11

.vect4_00:
	move.l		d2, (a1)				; restore vector 4
	move.l		a3, a7					; restore stack pointer
	move.w		d1, _cpu_type			; save CPU type

.end:
	move.w		d0, sr
	movem.l		(a7)+, d0-d3/a1-a3
	rte


.vect4_10plus:
	cmp.b		#20, d1					; detect CPU 68020 from 68030
	bne.s		.vect11

	dc.w		$F02F,$6200,$FFFE		; !!! PMOVE.W PSR,-2(A7)			(supported only in 68030+)
	moveq		#30, d1					; CPU = 68030
	bra.s		.vect11
