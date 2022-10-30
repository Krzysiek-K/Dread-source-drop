
;
;	move.w	#$2400, d1									; disable Audio interrupts (and lower)
;	TRAP_SET_SR
;	move.w	#$2000, d1									; enable all interrupts
;	TRAP_SET_SR



Aud0_active:			dc.w		0
Aud0_pending_sample:	dc.l		0




******************** Irq_AudioBufferStarted ********************

Irq_AudioBufferStarted:					; IV_LEVEL_4	$70		; audio channel 0..3
	movem.l	d0-d1/a0-a1,-(a7)
	lea.l	CHIP_BASE, a1

	move.w	INTREQR(a1), d0			; INTREQR
	and.w	#$0080, d0				; check AUD0 interrupt
	beq.b	.IrqEnd

		; Audio 0 interrupt
		move.w	#$0080, INTENA(a1)			; disable the interrupt
		move.l	#ZeroSample, AUD0LC(a1)		; next buffer - play silence
		move.w	#2, AUD0LEN(a1)
		
	
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

	
	move.l	Aud0_pending_sample, d0
	beq.b	.no_new_sample
											; prepare to play new sample
	move.w	#$0080, INTENA(a1)				; disable audio buffer interrupt
	move.w	#$0001, DMACON(a1)				; disable the audio DMA
	move.w	#0, Aud0_active					; channel state = breaking the DMA

	move.w	#$ff0, COLOR00(a1)
	bra		.wait_scanlines


.no_new_sample:
	move.w	#$800, COLOR00(a1)
.wait_scanlines:
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


	move.l	Aud0_pending_sample, d0			; new sample?
	beq.b	.no_new_sample
	
	move.w	Aud0_active, d0					; DMA already stopped?
	bne.b	.no_new_sample

	move.l	Aud0_pending_sample, a0			; play new sample
	move.l	#0, Aud0_pending_sample

	move.w	(a0)+, AUD0LEN(a1)
	move.w	(a0)+, AUD0PER(a1)
	move.w	#64, AUD0VOL(a1)
	move.l	a0, AUD0LC(a1)

	move.w	#1, Aud0_active					; channel state = active

	move.w	#$0080, INTREQ(a1)				; clear any pending request
	move.w	#$0080, INTREQ(a1)				; clear any pending request
	move.w	#$8080, INTENA(a1)				; setup the interrupt
	move.w	#$8001, DMACON(a1)


	move.w	#$0f0, COLOR00(a1)
	bra		.wait_scanlines


.no_new_sample:
	move.w	#$00f, COLOR00(a1)
.wait_scanlines:
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
	move.l	a0, a1

	move.w	#$2600, d1
	TRAP_CRITICAL_BEGIN
	move.l	a1, Aud0_pending_sample
	TRAP_CRITICAL_END

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
