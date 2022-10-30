
;
;	move.w	#$2400, d1									; disable Audio interrupts (and lower)
;	TRAP_SET_SR
;	move.w	#$2000, d1									; enable all interrupts
;	TRAP_SET_SR



AUDSTATE_STOPPED		equ		0			; DMA & Paula fully stopped
AUDSTATE_STARTING		equ		1			; Everything prepared, waiting for first interrupt
AUDSTATE_PLAYING		equ		2			; Currently playing
AUDSTATE_BREAK			equ		3			; Break issued



Aud0_irq_fn:			dc.l		0

Aud0_state:				dc.w		0
Aud0_pending_sample:	dc.l		0



AudFn_StartedOneShot:
	move.w	(a0)+, AUD0LEN(a1)
	move.w	(a0)+, AUD0PER(a1)
;	lea.l	AudFn_Stop, a0
	rts


AudFn_Stop:
	move.w	#$0080, INTENA(a1)		; disable the interrupt
	move.w	#$0001, DMACON(a1)		; disable the audio
	move.w	#0, AUD0VOL(a1)			; disable audio output
	move.w	#$0, a0
	rts



******************** Irq_AudioBufferStarted ********************

Irq_AudioBufferStarted:					; IV_LEVEL_4	$70		; audio channel 0..3
	movem.l	d0-d1/a0-a1,-(a7)
	
	lea.l	CHIP_BASE, a1

	move.w	INTREQR(a1), d0			; INTREQR
	and.w	#$0080, d0				; check AUD0 interrupt
	beq.b	.IrqEnd

		; Audio 0 interrupt
		move.l	Aud0_irq_fn, d0
		beq.b	.IrqEnd
		move.l	d0, a0
		jsr		(a0)
		move.l	a0, Aud0_irq_fn
		move.w	#$0080, INTREQ(a1)		; clear the request
		move.w	#$0080, INTREQ(a1)		; clear the request

	; end
.IrqEnd:
	movem.l	(a7)+,d0-d1/a0-a1
	rte



******************** Irq_CIAA ********************
Irq_CIAA:
	movem.l	d0-d1/a0-a1,-(a7)
	lea.l	CHIP_BASE, a1
	
	tst.b	CIAA_ICR
	move.w	#$0008,INTREQ(a1)
	move.w	#$0008,INTREQ(a1)

	move.w	#$f00, COLOR00(a1)
	WAIT_SCANLINE
	WAIT_SCANLINE
	WAIT_SCANLINE
	move.w	#$000, COLOR00(a1)

	move.b	#CIACRBF_START|CIACRBF_ONESHOT|CIACRBF_LOAD, CIAB_CRB 

	movem.l	(a7)+,d0-d1/a0-a1
	rte


******************** Irq_CIAB ********************

Irq_CIAB:
	movem.l	d0-d1/a0-a1,-(a7)
	lea.l	CHIP_BASE, a1
	
	tst.b	CIAB_ICR
	move.w	#$2000,INTREQ(a1)
	move.w	#$2000,INTREQ(a1)

	move.w	#$00f, COLOR00(a1)
	WAIT_SCANLINE
	WAIT_SCANLINE
	WAIT_SCANLINE
	move.w	#$000, COLOR00(a1)

	movem.l	(a7)+,d0-d1/a0-a1
	rte





******************** Sound_PlayThingSound ********************
; input:	a0	- sound pointer
;			a6	- thing pointer
	public _Sound_PlayThingSound
_Sound_PlayThingSound:
	lea.l	CHIP_BASE, a1

	move.w	#$2600, d1
	TRAP_CRITICAL_BEGIN
	move.l	a0, Aud0_pending_sample
	TRAP_CRITICAL_END

;	move.w	(a0)+, AUD0LEN(a1)
;	move.w	(a0)+, AUD0PER(a1)
;	move.w	#64, AUD0VOL(a1)
;	move.l	a0, AUD0LC(a1)
;
;	lea.l	AudFn_StartedOneShot, a0
;	move.l	a0, Aud0_irq_fn
;
;	move.w	#$0080, INTREQ(a1)		; clear any pending request
;	move.w	#$0080, INTREQ(a1)		; clear any pending request
;	move.w	#$8080, INTENA(a1)		; setup the interrupt
;	move.w	#$8001, DMACON(a1)

	rts



******************** Music_Init ********************

	public _Music_Init
_Music_Init:
	if MUSICPLAYER=1
		lea		P61Module,a0
		lea		P61Samples,a1
		sub.l	a2,a2
		moveq	#0,d0
		jsr		P61_Init
	endc
	rts
