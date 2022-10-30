

	if AMIGA_AUDIO_ENGINE_VERSION = 2


AUDSTATE_STOPPED		equ			0					; channel stopped in TICK
AUDSTATE_STARTING		equ			1					; play command issued, waiting for audio interrupt
AUDSTATE_PLAYING		equ			2					;
AUDSTATE_FINISHED		equ			3


	xref		_Aud_Channels
	xref		_aud_channel_count
	xref		_aud_volume_update_request

	xref		_TRKSOUND_TKPISTOL
	xref		_TRKSOUND_TKPISTOL_SUP
	xref		_TRKSOUND_TKROCKT
	xref		_TRKSOUND_TKCLIP


aud_tick_count:					dc.w		0



;******************** General register usage ********************
;
;	d0		- temp
;	d1		- temp
;	d2
;	d3
;	d4
;	d5
;	d6
;	d7
;
;	a0		- current track row					(AudioTrackRow*)
;	a1		- CHIP_BASE
;	a2		- current virtual channel			(AudioChannel*)
;	a3
;	a4
;	a5
;	a6		- Aud_ComputeVolume thing pointer
;	a7
;
;
;	------- Function breakdown --------
;
;									d0	d1	d2	d3	d4	d5	d6	d7		a0	a1	a2	a3	a4	a5	a6			Subroutines
;	Aud_ComputeVolumes				IN	RV	RV	$$			in	in								in
;	Aud_ExecutePacket_Stop												ff	ff	ff
;	Aud_ExecutePacket_Play			##	##								ff	ff	ff	##	##	##	##			Aud_ComputeVolumes
;
;	AudChan_Buffer_Started			##	$$								FF	ff	ff	$$	$$	$$				Aud_ExecutePacket_Stop	Aud_ExecutePacket_Play
;	AudChan_Tick					##									FF	ff	ff							Aud_ExecutePacket_Stop
;	AudChan_Tock					##	$$								FF	ff	ff	##	$$	$$				ExecutePacket_Play
;
;	Sound_PlayThingSound			##	##	$$	$$				$$			##					$$			Aud_ComputeVolume		Aud_ComputeStereo
;
;	Irq_AudioBufferStarted			--	--	++							--	++	++	--	--	--				AudChan_Buffer_Started
;	Irq_CIAA						--				++					--	++	++							AudChan_Tick
;	Irq_CIAB						--	--			++					--	++	++	--	--	--				AudChan_Tock
;
;
; legend:
;	##		- trashed
;	$$		- trashed by subfunction			(NOT SAVED, used only indirectly by a subfunction)
;	IN		- custom argument					(NOT SAVED)
;	FF		- internally as fixed function		(NOT SAVED)
;	RV		- return value
;
;	++		- used, but saved					(saved)
;	--		- used by subroutine, but saved		(saved)
;	in		- custom argument					(saved)
;	ff		- inout fixed function				(saved)
;



;******************** Aud_ComputeVolumes ********************
; Returns:
;
;	d0	IN	- stereo mode					(NOT SAVED)
;	d1.b	- left volume 0..64				(OUT)
;	d2.b	- right volume 0..64			(OUT)
;	d3.b	- temp							(NOT SAVED)
;
;	d6	IN	- max volume 0..64				(saved)
;	d7	IN	- attenuation					(saved)			[ d7 = (attn*attn)>>8 ]
;
;	a6	IN	- thing pointer					(saved)
;
Aud_ComputeVolumes:
	tst.w		d7
	beq			.end_max							; no attenuation

	tst.b		d0
	bne			.compute_stereo

	move.w		_view_pos_x, d1						; 1 / (dx*dx + dy*dy)
	sub.w		thing_xp(a6), d1
	asr.w		#ENGINE_SUBVERTEX_PRECISION, d1
	muls.w		d1, d1

	move.w		_view_pos_y, d2
	sub.w		thing_yp(a6), d2
	asr.w		#ENGINE_SUBVERTEX_PRECISION, d2
	muls.w		d2, d2

	add.l		d2, d1

	move.l		d1, d0								; compute distance (square root)
	jsr			_lsqrt
	move.l		d0, d1


	cmp.l		#65536, d1							; 0 ? 65536
	bhs			.end_0								;	>=				(too far to hear)

	cmp.w		d7, d1								; d1 ? d7
	bls			.end_max							;	<=				(too close to attenuate)
	move.w		d6, d2								; volume = max volume * attn / distance			= d2 * d7 / d1
	mulu.w		d7, d2								;
	divu.w		d1, d2								; d2 = volume
	move.w		d2, d1								; d1 = volume
	rts

.end_0:
	moveq.l		#0, d1								; d1 = silence
	moveq.l		#0, d2								; d2 = silence
	rts

.end_max:
	move.l		d6, d1								; d1 = max volume
	move.l		d6, d2								; d2 = max volume
	rts


	; -------------------------------- Stereo mode --------------------------------
.compute_stereo:
													; ============ d0 <- distance^2 >> 8 ============
													; ============ d1 <- dot_product ============
	move.w		thing_xp(a6), d0
	sub.w		_view_pos_x, d0
	asr.w		#ENGINE_SUBVERTEX_PRECISION, d0		;	dx = ( thing.xp - view_pos_x ) >> ENGINE_SUBVERTEX_PRECISION
	move.w		_view_rotate_dx, d1
	muls.w		d0, d1								;		d1 = dx*view_rotate_dx
	muls.w		d0, d0								;		d0 = dx*dx

	move.w		thing_yp(a6), d2
	sub.w		_view_pos_y, d2
	asr.w		#ENGINE_SUBVERTEX_PRECISION, d2		;	dy = ( thing.yp - view_pos_y ) >> ENGINE_SUBVERTEX_PRECISION
	move.w		_view_rotate_dy, d3
	muls.w		d2, d3								;		d3 = dy*view_rotate_dy
	muls.w		d2, d2								;		d2 = dy*dy

	add.l		d2, d0								;		d0 = ( dx*dx + dy*dy ) >> 8
	lsr.l		#8, d0
	beq			.end_max							;	at the player - max volume both channels

	cmp.l		#65536, d0							;		d0 ? 65536
	bhs			.end_0								;		   >=				(too far to hear)

	sub.l		d3, d1								;	d1 = ( dx*view_rotate_dx - dy*view_rotate_dy ) >> 14
	lsl.l		#2, d1								;
	swap.w		d1									;


	move.w		d6, d3								; ============ d3 <- attenuated volume ============
	cmp.w		d7, d0								;	d0 ? d7
	bls			.compute_pan						;	   <=				(too close to attenuate)
	mulu.w		d7, d3								;	d3 = max volume * attn / distance^2		= d3 * d7 / d0
	divu.w		d0, d3


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
	move.w		#256, d0							; d1 <- ( d3 * ( 256 - d1 ) ) >> 8
	sub.w		d1, d0
	mulu.w		d3, d0
	lsr.l		#8, d0
	move.w		d0, d1
	move.w		d3, d2								; d2 <- d3		[ max volume right volume ]
	rts
	



.pan_left:
	muls.w		d1, d1								; d1 = min( 256, dot_product*dot_product / (distance^2 >> 8) )
	divu.w		d0, d1
	cmp.w		#256, d1
	bls			.l_noclip
	move.w		#256, d1
.l_noclip:
	move.w		#256, d2							; d2 <- ( d3 * ( 256 - d1 ) ) >> 8
	sub.w		d1, d2
	mulu.w		d3, d2
	lsr.l		#8, d2
	move.w		d3, d1								; d1 <- d3		[ max volume left volume ]
	rts






;******************** Aud_ExecutePacket_Stop ********************
;
;	a0	IN	- currect track row pointer		(saved)
;	a1	IN	- CHIP_BASE						(saved)
;	a2	IN	- AudioChannel*					(saved)
;
Aud_ExecutePacket_Stop:
	btst.b		#0, audtrak_flags(a0)							;	if( pk->flags & AudioTrackRow::F_STOP )
	beq			.no_stop										;	{
	move.w		aud_dma_mask(a2), DMACON(a1)					;		DMA_on = false;					// disable the audio DMA
	move.w		aud_irq_mask(a2), INTENA(a1)					;		IRQ_on = false;					// disable audio buffer interrupt
.no_stop:														;	}

	btst.b		#1, audtrak_flags(a0)							;	if( pk->flags & AudioTrackRow::F_END )
	beq			.no_end											;	{
	clr.l		aud_timer_cmd(a2)								;		timer_cmd = NULL;
	clr.l		aud_irq_cmd(a2)									;		irq_cmd = NULL;
.no_end:														;	}

	rts

;******************** Aud_ExecutePacket_Play ********************
;
;	d0		- temp							(NOT SAVED)
;	d1		- temp							(NOT SAVED)
;
;	a0	IN	- currect track row pointer		(saved)
;	a1	IN	- CHIP_BASE						(saved)
;	a2	IN	- AudioChannel*					(saved)
;
;	a3		- temp - sample pointer			(NOT SAVED)
;	a4		- temp - hardware pointer 1		(NOT SAVED)
;	a5		- temp - hardware pointer 2		(NOT SAVED)
;
Aud_ExecutePacket_Play:

	move.l		aud_reg_base_1(a2), a4
	move.l		aud_reg_base_2(a2), a5

																;	// Set sample (1)
	btst.b		#2, audtrak_flags(a0)							;	if( pk->flags & AudioTrackRow::F_SET_SAMPLE )
	beq			.no_sample1										;	{
	move.l		audtrak_sample(a0), a3							;		AUDxLEN = pk->sample[0];
	move.w		2(a3), d0										;		if( pk->sample[1] )
	beq.w		.no_sample1										;			base_period = pk->sample[1];
	move.w		d0, aud_base_period(a2)							;
.no_sample1:													;	}


																;	// Set volume
	btst.b		#3, audtrak_flags(a0)							;	if( pk->flags & AudioTrackRow::F_UPDATE_VOLUME )
	beq			.no_volume										;	{
	clr.l		d6
	move.b		audtrak_volume(a0), d6							;		d6 = new max volume
	move.b		d6, aud_volume(a2)								;			(save for later updates)
	move.b		aud_stereo_mode(a2), d0							;		d0 = stereo mode flag
	move.w		aud_attenuation(a2), d7							;		d7 = attenuation
	move.l		aud_thing_owner(a2), a6							;		a6 = thing

	jsr			Aud_ComputeVolumes								;		Aud_ComputeVolumes(...)
	move.b		d1, AUD0VOL(a4)									;
	move.b		d2, AUD0VOL(a5)									;

.no_volume:														;	}
																;
																;	// Set frequency
	btst.b		#4, audtrak_flags(a0)							;	if( pk->flags & AudioTrackRow::F_UPDATE_FREQUENCY )
	beq			.no_frequency									;	{
	move.w		aud_base_period(a2), d0							;		base_period															[d0, d1]
	move.w		d0, d1											;

	mulu.w		audtrak_period_mul(a0), d0						;		uint16_t period = uint16_t(mulu(base_period, pk->period_mul));
	mulu.w		audtrak_period_mul_fract(a0), d1				;		period += uint16_t(mulu(base_period, pk->period_mul_fract) >> 16);
	swap.w		d1												;
	add.w		d1, d0

																;
	cmp.w		#124, d0										;		if( period < 124 )
	bge			.period_ok										;			period = 124;
	move.w		#124, d0
.period_ok:
	move.w		d0, AUD0PER(a4)									;		AUDxPER = period;
	move.w		d0, AUD0PER(a5)									;
.no_frequency:													;	}
	
																;	// Set sample (2)
	btst.b		#2, audtrak_flags(a0)							;	if( pk->flags & AudioTrackRow::F_SET_SAMPLE )
	beq			.no_sample2										;	{

	move.l		audtrak_sample(a0), a3							;		AUDxLEN = pk->sample[0];
	move.w		(a3), AUD0LEN(a4)								;
	move.w		(a3), AUD0LEN(a5)								;
	adda.l		#4, a3											;
	move.l		a3, AUD0LC(a4)									;		AUDxLC = pk->sample + 2;
	move.l		a3, AUD0LC(a5)									;

	move.w		aud_dma_mask(a2), d0							;		DMA_on = true;
	or.w		#$8000, d0										;
	move.w		d0, DMACON(a1)									;
.no_sample2:														;	}

	rts



;******************** AudChan_Buffer_Started ********************
; Called for each HARDWARE channel once DMA started streaming a buffer.
;
;	d0		- temp									(NOT SAVED)
;	d1		- subroutine temp						(NOT SAVED)
;
;	a0		- temp - currect track row pointer		(NOT SAVED)
;	a1	IN	- CHIP_BASE								(saved)
;	a2	IN	- AudioChannel*							(saved)
;
;				subroutine temp						(NOT SAVED)
;				subroutine temp						(NOT SAVED)
;
AudChan_Buffer_Started:

	move.l		aud_irq_cmd(a2), d0
	beq			.irq_end


	move.l		d0, a0											;	AudioTrackRow *cmd = irq_cmd++;				[a0]		// shadow, because F_STOP might NULL it
	add.l		#audtrak_sizeof, d0
	move.l		d0, aud_irq_cmd(a2)

																
	jsr			Aud_ExecutePacket_Stop							;	ExecutePacket_Stop(cmd);
	jsr			Aud_ExecutePacket_Play							;	ExecutePacket_Play(cmd);

	
	btst.b		#6, audtrak_flags(a0)							;	if( cmd->flags & AudioTrackRow::F_DONE )
	beq			.no_done										;	{
	clr.l		aud_irq_cmd(a2)									;		irq_cmd = NULL;
	move.w		aud_irq_mask(a2), INTENA(a1)					;		IRQ_on = false;					// disable audio buffer interrupt
.no_done:														;	}

.irq_end:
	move.w		aud_irq_mask(a2), INTREQ(a1)					; clear the request
	move.w		aud_irq_mask(a2), INTREQ(a1)					; (compatibility)
	rts



;******************** AudChan_Tick ********************
; Called for each virtual channel every frame (by Irq_CIAA).
;
;	d0		- temp						(NOT SAVED)
;
;	a0		- timer_cmd					(NOT SAVED)
;	a1	IN	- CHIP_BASE					(saved)
;	a2	IN	- AudioChannel*				(saved)
;
AudChan_Tick:

	move.l		aud_timer_cmd(a2), d0							;	if( !timer_cmd )								[a0]
	beq			.end											;		return;
	move.l		d0, a0

	add.w		#1, aud_t1_entry_count(a2)						;	t1_entry_count++;

	move.w		aud_t1_entry_count(a2), d0						;	if( t1_entry_count < timer_cmd->delay )
	cmp.w		audtrak_delay(a0), d0							;		return;
	blo			.end

	jsr			Aud_ExecutePacket_Stop							;	ExecutePacket_Stop(timer_cmd);

.end:
	rts



;******************** AudChan_Tock ********************
; Called for each virtual channel every frame (by Irq_CIAB, 15 scanlines after AudChan_Tick).
;
;	d0		- temp						(NOT SAVED)
;
;	a0		- timer_cmd					(NOT SAVED)
;	a1	IN	- CHIP_BASE					(saved)
;	a2	IN	- AudioChannel*				(saved)
;	a3		- temp						(NOT SAVED)
;
AudChan_Tock:

	move.l		aud_timer_cmd(a2), d0							;	if( !timer_cmd )								[a0]
	beq			.end											;		return;
	move.l		d0, a0


	move.w		aud_t1_entry_count(a2), d0						;	if( t1_entry_count < timer_cmd->delay )
	cmp.w		audtrak_delay(a0), d0							;		return;
	blo			.end
	clr.w		aud_t1_entry_count(a2)							;	t1_entry_count = 0;


																;	// Schedule IRQ program
	btst.b		#7, audtrak_flags(a0)							;	if( timer_cmd->flags & AUD_ROW_FLAG_IRQ_ENABLE )
	beq			.no_irq											;	{
	
	lea.l		audtrak_sizeof(a0), a3							;		irq_cmd = timer_cmd + 1;
	move.l		a3, aud_irq_cmd(a2)								;

	move.w		aud_irq_mask(a2), d0							;		IRQ_on = true;
	move.w		d0, INTREQ(a1)									;			clear any pending request
	move.w		d0, INTREQ(a1)									;			(compatibility)
	or.w		#$8000, d0										;
	move.w		d0, INTENA(a1)									;			setup the interrupt
.no_irq:														;	}


	jsr			Aud_ExecutePacket_Play							;	ExecutePacket_Play(timer_cmd);


	btst.b		#6, audtrak_flags(a0)							;	if( timer_cmd->flags & AUD_ROW_FLAG_DONE )
	beq			.find_next_row									;	{
	clr.l		aud_timer_cmd(a2)								;		timer_cmd = NULL;
																;	}
	bra			.end											;	else
																;	{
.find_next_row:													;		do {
	lea.l		audtrak_sizeof(a0), a0							;			timer_cmd++;
	tst.w		audtrak_delay(a0)								;		} while( timer_cmd->delay==0 );
	beq			.find_next_row									;	}
	move.l		a0, aud_timer_cmd(a2)

.end:
	rts




;******************** Sound_PlayThingSound ********************
;
;	d0		- temp						(NOT SAVED)
;	d1		- sound counter				(NOT SAVED)
;	d2		- best error				(saved)
;	d7	IN	- sound attenuation			(saved)
;
;	a0	IN	- sound pointer				(NOT SAVED)
;	a1		- channel pointer			(NOT SAVED)
;	a2		- best channel				(saved)
;	a6	IN	- thing pointer				(saved)
;
	public _Sound_PlayThingSound
_Sound_PlayThingSound:
	movem.l		d2-d3/d7/a2, -(a7)
	
													; a0	 (sample)
	lea.l		_Aud_Channels, a1					; a1	- channel considered
	move.w		_aud_channel_count, d1				; d1	- remaining channels to scan
	subq.w		#1, d1								;
	move.l		a1, a2								; a2	- best channel
	moveq.l		#0, d2								; d2	- best channel score	(bigger = better)
													; d0	 (temp)
.loop:
	cmpa.l		aud_thing_owner(a1), a6				; already allocated to this thing?
	beq			.channel_found

	move.l		aud_timer_cmd(a1), d0
	or.l		aud_irq_cmd(a1), d0
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


.channel_found:										; found channel in A1


	mulu.w		d7, d7
	lsr.l		#8, d7

	move.l		a0, a2											; move sample pointer to A2
	;move.l		#_TRKSOUND_TKPISTOL, a2
	CRITICAL_BEGIN								; uses d0/a0
	move.l		a2, aud_timer_cmd(a1)							; sound track pointer
	clr.l		aud_irq_cmd(a1)									; sound track pointer
	move.l		a6, aud_thing_owner(a1)							; owner
	move.w		d7, aud_attenuation(a1)							; attenuation
	clr.w		aud_t1_entry_count(a1)							; delay counter
	move.w		aud_tick_count, aud_last_time(a1)
	CRITICAL_END

.end:
	movem.l	(a7)+, d2-d3/d7/a2
	rts




	endif		; AMIGA_AUDIO_ENGINE_VERSION = 2
