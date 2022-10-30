

AUDSTATE_STOPPED		equ			0					; channel stopped in TICK
AUDSTATE_STARTING		equ			1					; play command issued, waiting for audio interrupt
AUDSTATE_PLAYING		equ			2					;
AUDSTATE_FINISHED		equ			3


aud_custom_base			equ			 0			;	ptr_t		CHIP_BASE + $10*index
aud_irq_mask			equ			 4			;	word		INTENA/INTREQ mask
aud_dma_mask			equ			 6			;	word		DMAEN mask
aud_pending_sample		equ			 8			;	ptr_t
aud_thing_owner			equ			12			;	ptr_t
aud_state				equ			16			;	byte		channel state	AUDSTATE_*
												;	byte		(unused)
aud_sizeof				equ			18


Aud_Channel_0:
		dc.l			$dff000
		dc.w			$0080
		dc.w			$0001
		dc.l			0
		dc.l			0
		dc.b			0
		dc.b			0
Aud_Channel_1:
		dc.l			$dff010
		dc.w			$0080
		dc.w			$0001
		dc.l			0
		dc.l			0
		dc.b			0
		dc.b			0
Aud_Channel_2:
		dc.l			$dff020
		dc.w			$0080
		dc.w			$0001
		dc.l			0
		dc.l			0
		dc.b			0
		dc.b			0
Aud_Channel_3:
		dc.l			$dff030
		dc.w			$0080
		dc.w			$0001
		dc.l			0
		dc.l			0
		dc.b			0
		dc.b			0



aud0_state:				dc.b		0
aud1_state:				dc.b		0
aud2_state:				dc.b		0
aud3_state:				dc.b		0

aud0_pending_sample:	dc.l		0
aud1_pending_sample:	dc.l		0
aud2_pending_sample:	dc.l		0
aud3_pending_sample:	dc.l		0

aud_channel_sequencer:	dc.w		0



;******************** AudChan_Buffer_Started ********************
;	d1		 (used)
;	a0		 (used)
;	a1		- CHIP_BASE					(saved)
;	a2		- Audio_Channel_x pointer
;
AudChan_Buffer_Started:
	move.b	aud_state(a2), d1
	cmp.b	#AUDSTATE_STARTING, d1				; starting? -> transition to "playing"
	bne.b	.c1
	move.b	#AUDSTATE_PLAYING, aud_state(a2)
	bra		.irqend
.c1:
														; other state -> kill the sound
	move.l	aud_custom_base(a2), a0
	move.w	aud_irq_mask(a2), INTENA(a1)				; disable audio buffer interrupt
	move.w	aud_dma_mask(a2), DMACON(a1)				; disable the audio DMA
	move.w	#0, AUD0VOL(a0)								; mute
	move.b	#AUDSTATE_FINISHED, aud_state(a2)

.irqend:
	move.w	aud_irq_mask(a2), INTREQ(a1)				; clear the request
	move.w	aud_irq_mask(a2), INTREQ(a1)				; (compatibility)

.end:
	rts



;******************** AudChan_Tick ********************
;	d1		 (used)
;	a1		- CHIP_BASE					(saved)
;	a2		- Audio_Channel_x pointer
;
AudChan_Tick:

	move.b	aud_state(a2), d1							; DMA already stopped?
	cmp.b	#AUDSTATE_FINISHED, d1
	beq.b	.killsound

	move.l	aud_pending_sample(a2), d1
	beq.b	.end
	
.killsound:												; prepare to play new sample
	move.w	aud_irq_mask(a2), INTENA(a1)				; disable audio buffer interrupt
	move.w	aud_dma_mask(a2), DMACON(a1)				; disable the audio DMA
	move.b	#AUDSTATE_STOPPED, aud_state(a2)			; channel state = breaking the DMA

.end:
	rts




;******************** AudChan_Tock ********************
;	d1		 (used)
;	a0		 (used)
;	a1		- CHIP_BASE					(saved)
;	a2		- Audio_Channel_x pointer
;	a3		 (used)
;
AudChan_Tock:

	move.l	aud_pending_sample(a2), d1					; new sample?
	beq.b	.end
	
	move.b	aud_state(a2), d1							; DMA already stopped?
	bne.b	.end

	move.l	aud_pending_sample(a2), a3					; play new sample
	move.l	#0, aud_pending_sample(a2)

	move.l	aud_custom_base(a2), a0
	move.w	(a3)+, AUD0LEN(a0)
	move.w	(a3)+, AUD0PER(a0)
	move.w	#64, AUD0VOL(a0)
	move.l	a3, AUD0LC(a0)

	move.b	#AUDSTATE_STARTING, aud_state(a2)			; channel state = starting

	move.w	aud_irq_mask(a2), d1
	move.w	d1, INTREQ(a1)								; clear any pending request
	move.w	d1, INTREQ(a1)								; (compatibility)
	or.w	#$8000, d1
	move.w	d1, INTENA(a1)								; setup the interrupt
	move.w	aud_dma_mask(a2), d1
	or.w	#$8000, d1
	move.w	d1, DMACON(a1)

.end:
	rts




;******************** AUDCHAN_BUFFER_STARTED ********************
;	d0		- INTREQR		(saved)
;	a1		- CHIP_BASE		(saved)
;
AUDCHAN_BUFFER_STARTED:		macro		; chan_index

	btst.l	#(\1+7), d0							; check AUDx interrupt
	beq.b	.end\@


	move.b	aud0_state+\1, d1
	cmp.b	#AUDSTATE_STARTING, d1				; starting? -> transition to "playing"
	bne.b	.c1\@
	move.b	#AUDSTATE_PLAYING, aud0_state+\1
	bra		.irqend\@
.c1\@:
												; other state -> kill the sound
	move.w	#1<<(\1+7), INTENA(a1)				; disable audio buffer interrupt
	move.w	#$0001<<\1, DMACON(a1)				; disable the audio DMA
	move.w	#0, AUD0VOL+\1*$10(a1)				; mute
	move.b	#AUDSTATE_FINISHED, aud0_state+\1

.irqend\@:
	move.w	#$0080<<\1, INTREQ(a1)				; clear the request
	move.w	#$0080<<\1, INTREQ(a1)				; (compatibility)

.end\@:
	endm


;******************** Irq_AudioBufferStarted ********************

Irq_AudioBufferStarted:					; IV_LEVEL_4	$70		; audio channel 0..3
	movem.l	d0-d1/a0-a2, -(a7)
	lea.l	CHIP_BASE, a1

	move.w	INTREQR(a1), d0						; INTREQR
	
	AUDCHAN_BUFFER_STARTED	0
	AUDCHAN_BUFFER_STARTED	1
	AUDCHAN_BUFFER_STARTED	2
	AUDCHAN_BUFFER_STARTED	3

	; end
	movem.l	(a7)+, d0-d1/a0-a2
	rte


;******************** AUDCHAN_TICK ********************
;	d0			(used)
;	a1		- CHIP_BASE
;
AUDCHAN_TICK:		macro		; chan_index

	move.b	aud0_state+\1, d1					; DMA already stopped?
	cmp.b	#AUDSTATE_FINISHED, d1
	beq.b	.killsound\@

	move.l	aud0_pending_sample+\1*4, d0
	beq.b	.end\@
	
.killsound\@:									; prepare to play new sample
	move.w	#1<<(\1+7), INTENA(a1)				; disable audio buffer interrupt
	move.w	#$0001<<\1, DMACON(a1)				; disable the audio DMA
	move.b	#AUDSTATE_STOPPED, aud0_state+\1	; channel state = breaking the DMA

.end\@:
	endm


;******************** Irq_CIAA ********************
Irq_CIAA:
	movem.l	d0-d1/a0-a1,-(a7)
	lea.l	CHIP_BASE, a1

	tst.b	CIAA_ICR
	move.w	#$0008,INTREQ(a1)
	move.w	#$0008,INTREQ(a1)

	AUDCHAN_TICK	0
	AUDCHAN_TICK	1
	AUDCHAN_TICK	2
	AUDCHAN_TICK	3
	
	move.b	#CIACRBF_START|CIACRBF_ONESHOT|CIACRBF_LOAD, CIAB_CRB 

	movem.l	(a7)+,d0-d1/a0-a1
	rte


;******************** AUDCHAN_TOCK ********************
;	d0			(used)
;	a1		- CHIP_BASE
;
AUDCHAN_TOCK:		macro		; chan_index

	move.l	aud0_pending_sample+\1*4, d0		; new sample?
	beq.b	.end\@
	
	move.b	aud0_state+\1, d0					; DMA already stopped?
	bne.b	.end\@

	move.l	aud0_pending_sample+\1*4, a0		; play new sample
	move.l	#0, aud0_pending_sample+\1*4

	move.w	(a0)+, AUD0LEN+\1*$10(a1)
	move.w	(a0)+, AUD0PER+\1*$10(a1)
	move.w	#64, AUD0VOL+\1*$10(a1)
	move.l	a0, AUD0LC+\1*$10(a1)

	move.b	#AUDSTATE_STARTING, aud0_state+\1	; channel state = starting

	move.w	#$0080<<\1, INTREQ(a1)				; clear any pending request
	move.w	#$0080<<\1, INTREQ(a1)				; (compatibility)
	move.w	#$8000+($0080<<\1), INTENA(a1)		; setup the interrupt
	move.w	#$8000+($0001<<\1), DMACON(a1)

.end\@:
	endm


;******************** Irq_CIAB ********************

Irq_CIAB:
	movem.l	d0-d1/a0-a1,-(a7)
	lea.l	CHIP_BASE, a1
	
	tst.b	CIAB_ICR
	move.w	#$2000,INTREQ(a1)
	move.w	#$2000,INTREQ(a1)

	AUDCHAN_TOCK	0
	AUDCHAN_TOCK	1
	AUDCHAN_TOCK	2
	AUDCHAN_TOCK	3

	movem.l	(a7)+,d0-d1/a0-a1
	rte



;******************** AUDCHAN_CHECK_EMPTY ********************
;	checks if channel is free
;	-> loads channel index to D0 and jumps to .channel_found if so
AUDCHAN_CHECK_EMPTY:		macro		; chan_index
	move.l	aud0_pending_sample+\1*4, d0
	or.b	aud0_active+\1, d0
	bne.b	.end\@
	moveq.l	#\1, d0
	bra		.channel_found
.end\@:
	endm

;******************** Sound_PlayThingSound ********************
; input:	a0	- sound pointer
;			a6	- thing pointer
	public _Sound_PlayThingSound
_Sound_PlayThingSound:

;	; try to find an empty channel
;	AUDCHAN_CHECK_EMPTY	0
;	AUDCHAN_CHECK_EMPTY	1
;	AUDCHAN_CHECK_EMPTY	2
;	AUDCHAN_CHECK_EMPTY	3
;	bra		.end


	move.w	aud_channel_sequencer, d0
	addq.w	#1, d0
	and.w	#3, d0
	move.w	d0, aud_channel_sequencer


.channel_found:
	lea.l	aud0_pending_sample, a1
	lsl.w	#2, d0
	lea.l	0(a1, d0.w), a1

	move.l	a0, (a1)						; sound pointer

.end:
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
