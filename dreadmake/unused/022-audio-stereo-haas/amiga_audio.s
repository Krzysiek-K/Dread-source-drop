

AUDSTATE_STOPPED		equ			0					; channel stopped in TICK
AUDSTATE_STARTING		equ			1					; play command issued, waiting for audio interrupt
AUDSTATE_PLAYING		equ			2					;
AUDSTATE_FINISHED		equ			3


	xref		_Aud_Channels

	public		_aud_channel_count
	public		_aud_stereo_mode
	public		_aud_volume_update_request

_aud_channel_count:				dc.w		2
_aud_stereo_mode:				dc.w		1

_aud_volume_update_request:		dc.w		0

aud_tick_count:					dc.w		0




;******************** Aud_ComputeVolume ********************
; returns volume in <d1>
;	d0		 (used)
;	d1		 (used)
;	d7		- attenuation		(saved)			[ d7 = (attn*attn)>>8 ]
;	a6		- thing pointer		(saved)
;
	public _Aud_ComputeVolume
_Aud_ComputeVolume:
	tst.w		d7
	beq			.end_64								; no attenuation

	move.w		_view_pos_x, d0						; 1 / (dx*dx + dy*dy)
	sub.w		thing_xp(a6), d0
	asr.w		#ENGINE_SUBVERTEX_PRECISION, d0
	muls.w		d0, d0

	move.w		_view_pos_y, d1
	sub.w		thing_yp(a6), d1
	asr.w		#ENGINE_SUBVERTEX_PRECISION, d1
	muls.w		d1, d1

	add.l		d1, d0
	lsr.l		#8, d0
	cmp.l		#65536, d0							; d0 ? 65536
	bhs			.end_0								;	>=				(too far to hear)

	cmp.w		d7, d0								; d0 ? d7
	bls			.end_64								;	<=				(too close to attenuate)
	moveq.l		#64, d1								; volume = max volume * attn / distance^2		= d1 * d7 / d0
	mulu.w		d7, d1								;
	divu.w		d0, d1
	rts

.end_0:
	moveq.l		#0, d1								; d0 = silence
	rts

.end_64:
	moveq.l		#64, d1								; d1 = max volume
	rts


;******************** AudChan_Buffer_Started ********************
;	d1		 (used)
;	a0		 (used)
;	a1		- CHIP_BASE					(saved)
;	a2		- Audio_Channel_x pointer	(saved)
;
AudChan_Buffer_Started:
	move.b	aud_state(a2), d1
	cmp.b	#AUDSTATE_STARTING, d1				; starting? -> transition to "playing"
	bne.b	.c1
	move.b	#AUDSTATE_PLAYING, aud_state(a2)
	bra		.irqend
.c1:
														; other state -> kill the sound
	move.w	aud_irq_mask(a2), INTENA(a1)				; disable audio buffer interrupt
	move.w	aud_dma_mask(a2), DMACON(a1)				; disable the audio DMA
	move.b	#AUDSTATE_FINISHED, aud_state(a2)

.irqend:
	move.w	aud_irq_mask(a2), INTREQ(a1)				; clear the request
	move.w	aud_irq_mask(a2), INTREQ(a1)				; (compatibility)

.end:
	rts



;******************** AudChan_Tick ********************
;	a0		 (used)
;	d0		 (used)
;	d1		 (used)
;	a1		- CHIP_BASE					(saved)
;	a2		- Audio_Channel_x pointer	(saved)
;
AudChan_Tick:

	move.l		aud_pending_sample(a2), d1					; new sample pending?	-> kill sound
	bne			.killsound

	move.b		aud_state(a2), d1							; DMA already stopped?	-> kill sound
	cmp.b		#AUDSTATE_FINISHED, d1
	beq			.killsound


	move.w		_aud_volume_update_request, d1				; no update request?	-> done
	beq			.end
	cmp.b		#AUDSTATE_PLAYING, d1						; not playing?			-> done
	bne			.end
	move.l		aud_thing_owner(a2), d1						; no thing owner?		-> done
	beq			.end

															; update volume
	movem.l		d7/a6, -(a7)

	move.l		d1, a6
	move.w		aud_attenuation(a2), d7						; attenuation
	jsr			_Aud_ComputeVolume
	move.b		d1, aud_volume(a2)							; save & update volume
	move.l		aud_custom_base(a2), a0
	move.b		d1, AUD0VOL(a0)

	movem.l		(a7)+, d7/a6


.end:
	rts


.killsound:												; prepare to play new sample
	move.w		aud_irq_mask(a2), INTENA(a1)				; disable audio buffer interrupt
	move.w		aud_dma_mask(a2), DMACON(a1)				; disable the audio DMA
	move.b		#AUDSTATE_STOPPED, aud_state(a2)			; channel state = breaking the DMA
	rts




;******************** AudChan_Tock ********************
;	d1		 (used)
;	a0		 (used)
;	a1		- CHIP_BASE					(saved)
;	a2		- Audio_Channel_x pointer	(saved)
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
	move.b	aud_volume(a2), AUD0VOL(a0)
	move.l	a3, AUD0LC(a0)

	move.b	#AUDSTATE_STARTING, aud_state(a2)			; channel state = starting
	move.w	aud_tick_count, aud_last_time(a2)

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




;******************** Sound_PlayThingSound ********************
; input:
;		d7	- sound attenuation
;		a0	- sound pointer
;		a6	- thing pointer
	public _Sound_PlayThingSound
_Sound_PlayThingSound:
	movem.l		d2/a2, -(a7)
	
													; a0	 (sample)
	lea.l		_Aud_Channels, a1					; a1	- channel considered
	move.w		_aud_channel_count, d1				; d1	- remaining channels to scan
	subq.w		#1, d1								;
	move.l		a1, a2								; a2	- best channel
	moveq.l		#0, d2								; d2	- best channel score	(bigger = better)
													; d0	 (temp)
.loop:
	move.l		aud_thing_owner(a1), d0				; already allocated to this thing?
	cmp.l		a6, d0
	beq			.channel_found

	move.l		aud_pending_sample(a1), d0
	bne			.not_free
	move.b		aud_state(a1), d0
	cmp.b		#AUDSTATE_STOPPED, d0
	beq			.free
	cmp.b		#AUDSTATE_FINISHED, d0
	bne			.not_free
.free:
	move.l		a1, a2								; save as best possible channel
	move.w		#$ffff, d2							;
	bra			.next

.not_free:
	move.w		aud_tick_count, d0					; score = tick count
	sub.w		aud_last_time(a1), d0
	cmp.w		d0, d2								; d2 ? d0
	bhs			.next
	move.l		a1, a2								; save as best channel found yet
	move.w		d0, d2								;


.next:
	adda.l		#aud_sizeof, a1
	dbra.w		d1, .loop

	move.l		a2, a1


.channel_found:													; found channel in A1


	mulu.w		d7, d7
	lsr.l		#8, d7
	jsr			_Aud_ComputeVolume
	tst.b		d1												; volume == 0
	beq			.end

	move.l		a0, a2
	CRITICAL_BEGIN
	move.l		a2, aud_pending_sample(a1)						; sound pointer
	move.l		a6, aud_thing_owner(a1)							; owner
	move.b		d1, aud_volume(a1)								; volume
	move.w		d7, aud_attenuation(a1)							; attenuation

	tst.w		_aud_stereo_mode
	beq			.no_stereo
	adda.l		#aud_sizeof*2, a1								; stereo channel
	move.l		a2, aud_pending_sample(a1)						; sound pointer
	move.l		a6, aud_thing_owner(a1)							; owner
	move.b		d1, aud_volume(a1)								; volume
	move.w		d7, aud_attenuation(a1)							; attenuation
.no_stereo:

	CRITICAL_END



.end:
	movem.l	(a7)+, d2/a2
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
