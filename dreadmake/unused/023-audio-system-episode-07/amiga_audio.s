

AUDSTATE_STOPPED		equ			0					; channel stopped in TICK
AUDSTATE_STARTING		equ			1					; play command issued, waiting for audio interrupt
AUDSTATE_PLAYING		equ			2					;
AUDSTATE_FINISHED		equ			3


	xref		_Aud_Channels
	xref		_aud_channel_count
	xref		_aud_volume_update_request


aud_tick_count:					dc.w		0




;******************** Aud_ComputeVolume ********************
; Returns:
;	- d0.l	= distance^2 / 256		( except when attenuation d7==0 )
;	- d1.b	= volume 0..64
;
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
	moveq.l		#0, d1								; d1 = silence
	rts

.end_64:
	moveq.l		#64, d1								; d1 = max volume
	rts


;******************** Aud_ComputeStereo ********************
; Returns:
;	- d0.b	= volume 0..64		left
;	- d1.b	= volume 0..64		right
;
;	d0		 (used)
;	d1		 (used)
;	d2		 (used)
;	d3		 (used)
;	d7		- attenuation		(saved)			[ d7 = (attn*attn)>>8 ]
;	a6		- thing pointer		(saved)
;
	public _Aud_ComputeStereo
_Aud_ComputeStereo:
	tst.w		d7
	beq			.end_64								; no attenuation

	move.w		thing_xp(a6), d0
	sub.w		_view_pos_x, d0
	asr.w		#ENGINE_SUBVERTEX_PRECISION, d0		;	dx = ( thing.xp - view_pos_x ) >> ENGINE_SUBVERTEX_PRECISION
	move.w		_view_rotate_dx, d1
	muls.w		d0, d1								; d1 = dx*view_rotate_dx
	muls.w		d0, d0								; d0 = dx*dx

	move.w		thing_yp(a6), d2
	sub.w		_view_pos_y, d2
	asr.w		#ENGINE_SUBVERTEX_PRECISION, d2		;	dy = ( thing.yp - view_pos_y ) >> ENGINE_SUBVERTEX_PRECISION
	move.w		_view_rotate_dy, d3
	muls.w		d2, d3								; d3 = dy*view_rotate_dy
	muls.w		d2, d2								; d2 = dy*dy

	add.l		d2, d0								; d0 = ( dx*dx + dy*dy ) >> 8
	lsr.l		#8, d0
	beq			.end_64								;	at the player - max volume both channels

	cmp.l		#65536, d0							; d0 ? 65536
	bhs			.end_0								;	>=				(too far to hear)

	sub.l		d3, d1								; d1 = ( dx*view_rotate_dx - dy*view_rotate_dy ) >> 14
	lsl.l		#2, d1
	swap.w		d1


	moveq.l		#64, d2
	cmp.w		d7, d0								; d0 ? d7
	bls			.compute_pan						;	<=				(too close to attenuate)
	mulu.w		d7, d2								; d2 = volume = max volume * attn / distance^2		= d2 * d7 / d0
	divu.w		d0, d2


.compute_pan:
	tst.w		d1
	bmi			.pan_left

.pan_right:
	muls.w		d1, d1								; d1 = min( 256, dot_product*dot_product / (distance^2 >> 8) )
	divu.w		d0, d1
	cmp.w		#256, d1
	bls			.r_noclip
	move.w		#256, d1
.r_noclip:
	move.w		#256, d0							; d0 = ( d2 * ( 256 - d1 ) ) >> 8
	sub.w		d1, d0								; d1 = d2		[ max volume right volume ]
	mulu.w		d2, d0
	lsr.l		#8, d0
	move.w		d2, d1
	rts
	



.pan_left:
	muls.w		d1, d1								; d1 = min( 256, dot_product*dot_product / (distance^2 >> 8) )
	divu.w		d0, d1
	cmp.w		#256, d1
	bls			.l_noclip
	move.w		#256, d1
.l_noclip:
	move.w		#256, d0							; d1 = ( d2 * ( 256 - d1 ) ) >> 8
	sub.w		d1, d0								; d0 = d2		[ max volume left volume ]
	mulu.w		d2, d0
	lsr.l		#8, d0
	move.w		d0, d1
	move.w		d2, d0
	rts



.end_0:
	moveq.l		#0, d1								; d1 = silence
	move.l		d1, d0
	rts

.end_64:
	moveq.l		#64, d1								; d1 = max volume
	move.l		d1, d0
	rts


;******************** AudChan_Buffer_Started ********************
; Called for each HARDWARE channel once DMA started streaming a buffer.
;
;	- STARTING		-> PLAYING
;	- PLAYING		-> disable interrups and DMA, FINISHED
;
;	d1		 (used)
;	a0		 (used)
;	a1		- CHIP_BASE						(saved)
;	a2		- AudioHardwareChannel*			(saved)
;
AudChan_Buffer_Started:
	move.l	aud_reg_base_1(a2), a0
	move.b	aud_state_1(a2), d1
	cmp.b	#AUDSTATE_STARTING, d1						; already started? -> disable DMA
	bne.b	.killsound

	move.b	#AUDSTATE_PLAYING, aud_state_1(a2)			; starting -> transition to "playing" & queue "zero sample"
	move.l	#ZeroSample, AUD0LC(a0)
	move.w	#2, AUD0LEN(a0)
	bra		.irqend

.killsound:
														; other state -> kill the sound
	move.w	aud_irq_mask_1(a2), INTENA(a1)				; disable audio buffer interrupt
	move.w	aud_dma_mask_1(a2), DMACON(a1)				; disable the audio DMA
	move.b	#AUDSTATE_FINISHED, aud_state_1(a2)
	move.w	#0, AUD0VOL(a0)

.irqend:
	move.w	aud_irq_mask_1(a2), INTREQ(a1)				; clear the request
	move.w	aud_irq_mask_1(a2), INTREQ(a1)				; (compatibility)
	
	rts



;******************** AudChan_Tick ********************
; Called for each virtual channel every frame.
;
;	- pending sample  OR  state FINISHED		-> stop IRQ, DMA and state STOPPED
;	- update_request & PLAYING & has owner		-> update volume
;
;	d0		 (used)
;	d1		 (used)
;	d2		 (used)
;	d3		 (used)
;	a0		 (used)
;	a1		- CHIP_BASE					(saved)
;	a2		- AudioVirtualChannel*		(saved)
;
AudChan_Tick:

	move.l		aud_pending_sample(a2), d1					; new sample pending?	-> kill sound
	bne			.killsound

	move.b		aud_state_1(a2), d1							; DMA already stopped? (checking only one HW channel)	-> kill sound
	cmp.b		#AUDSTATE_FINISHED, d1
	beq			.killsound

	movem.l		d7/a6, -(a7)

;	move.w		_aud_volume_update_request, d1				; no update request?	-> done				TBD: reenable the test
;	beq			.end
	cmp.b		#AUDSTATE_PLAYING, d1						; not playing?			-> done
	bne			.end
	move.l		aud_thing_owner(a2), d1						; no thing owner?		-> done
	beq			.end

															; update volume

	move.l		d1, a6
	move.w		aud_attenuation(a2), d7						; attenuation
	tst.l		aud_reg_base_2(a2)
	bne			.stereo

.mono:
	jsr			_Aud_ComputeVolume
	move.l		aud_reg_base_1(a2), a0						; save & update volume
	move.b		d1, AUD0VOL(a0)
	move.b		d1, aud_volume_1(a2)
	
	bra			.end
	

.stereo:
	move.l		d0, a0
	jsr			_Aud_ComputeStereo
	move.l		aud_reg_base_1(a2), a0						; save & update volume
	move.b		d0, AUD0VOL(a0)
	move.b		d0, aud_volume_1(a2)

	move.l		aud_reg_base_2(a2), a0						; save & update volume
	move.b		d1, AUD0VOL(a0)
	move.b		d1, aud_volume_2(a2)

.end:
	movem.l		(a7)+, d7/a6
	rts


.killsound:													; prepare to play new sample
	move.w		aud_combined_irq_mask(a2), INTENA(a1)		; disable audio buffer interrupt
	move.w		aud_combined_dma_mask(a2), DMACON(a1)		; disable the audio DMA
	move.b		#AUDSTATE_STOPPED, aud_state_1(a2)			; channel state = breaking the DMA
	move.b		#AUDSTATE_STOPPED, aud_state_2(a2)			; channel state = breaking the DMA
	rts




;******************** AudChan_Tock ********************
; Called for each virtual channel every frame (15 scanlines after tick).
;
;	- pending sample & state STOPPED		-> play new sample
;
;
;	d1		 (used)
;	a0		 (used)
;	a1		- CHIP_BASE					(saved)
;	a2		- Audio_Channel_x pointer	(saved)
;	a3		 (used)
;	a4		 (used)
;
AudChan_Tock:

	tst.l	aud_pending_sample(a2)						; new sample?
	beq		.end
	
	tst.b	aud_state_1(a2)								; DMA already stopped?		(checking only first channel, because both are always stopped at the same time)
	bne		.end

	move.l	aud_pending_sample(a2), a3					; play new sample
	move.l	#0, aud_pending_sample(a2)

	move.l	aud_reg_base_1(a2), a0
	move.l	aud_reg_base_2(a2), d1
	bne		.stereo

	; MONO
	move.w	(a3)+, AUD0LEN(a0)
	move.w	(a3)+, AUD0PER(a0)
	move.b	aud_volume_1(a2), AUD0VOL(a0)
	move.l	a3, AUD0LC(a0)
	move.b	#AUDSTATE_STARTING, aud_state_1(a2)			; channel state = starting
	bra		.play
	
	; STEREO
.stereo:
	move.l	d1, a4
	move.w	(a3), AUD0LEN(a0)
	move.w	(a3)+, AUD0LEN(a4)
	move.w	(a3), AUD0PER(a0)
	move.w	(a3)+, AUD0PER(a4)
	move.b	aud_volume_1(a2), AUD0VOL(a0)
	move.b	aud_volume_2(a2), AUD0VOL(a4)
	move.l	a3, AUD0LC(a0)
	move.l	a3, AUD0LC(a4)
	move.b	#AUDSTATE_STARTING, aud_state_1(a2)			; channel state = starting
	move.b	#AUDSTATE_STARTING, aud_state_2(a2)			;


.play:
	move.w	aud_combined_irq_mask(a2), d1				; starting IRQs and DMAs
	move.w	d1, INTREQ(a1)									; clear any pending request
	move.w	d1, INTREQ(a1)									; (compatibility)
	or.w	#$8000, d1
	move.w	d1, INTENA(a1)									; setup the interrupt
	move.w	aud_combined_dma_mask(a2), d1
	or.w	#$8000, d1
	move.w	d1, DMACON(a1)

	move.w	aud_tick_count, aud_last_time(a2)

.end:
	rts




;******************** Sound_PlayThingSound ********************
; input:
;		d7	- sound attenuation
;		a0	- sound pointer
;		a6	- thing pointer
	public _Sound_PlayThingSound
_Sound_PlayThingSound:
	movem.l		d2-d3/a2, -(a7)
	
													; a0	 (sample)
	lea.l		_Aud_Channels, a1					; a1	- channel considered
	move.w		_aud_channel_count, d1				; d1	- remaining channels to scan
	subq.w		#1, d1								;
	move.l		a1, a2								; a2	- best channel
	moveq.l		#0, d2								; d2	- best channel score	(bigger = better)
													; d0	 (temp)
.loop:
	move.l		aud_thing_owner(a1), d0			; already allocated to this thing?
	cmp.l		a6, d0
	beq			.channel_found

	move.l		aud_pending_sample(a1), d0
	bne			.not_free
	move.b		aud_state_1(a1), d0
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

	tst.l		aud_reg_base_2(a2)
	bne			.stereo

.mono:
	jsr			_Aud_ComputeVolume
	tst.b		d1												; volume == 0
	beq			.end

	move.l		a0, a2
	CRITICAL_BEGIN
	move.l		a2, aud_pending_sample(a1)						; sound pointer
	move.l		a6, aud_thing_owner(a1)							; owner
	move.b		d1, aud_volume_1(a1)							; volume
	move.b		d1, aud_volume_2(a1)							; volume
	move.w		d7, aud_attenuation(a1)							; attenuation
	CRITICAL_END
.end:
	movem.l	(a7)+, d2-d3/a2
	rts


.stereo:
	jsr			_Aud_ComputeStereo
	tst.b		d0												; volume == 0
	bne			.play_stereo
	tst.b		d1												; volume == 0
	beq			.end

.play_stereo:
	move.l		a0, a2											; move sample pointer to A2
	move.b		d0, d2											; move left volume to D2
	CRITICAL_BEGIN
	move.l		a2, aud_pending_sample(a1)						; sound pointer
	move.l		a6, aud_thing_owner(a1)							; owner
	move.b		d2, aud_volume_1(a1)							; volume
	move.b		d1, aud_volume_2(a1)							; volume
	move.w		d7, aud_attenuation(a1)							; attenuation
	CRITICAL_END

	movem.l	(a7)+, d2-d3/a2
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
