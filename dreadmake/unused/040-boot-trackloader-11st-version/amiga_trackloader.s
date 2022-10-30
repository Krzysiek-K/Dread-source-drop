

TRACK_BUFFER_SIZE				= 12980




; ================================================================ Globals ================================================================



Track_Vars:
.trk_buffer_address			dc.l		0			;	Track buffer address - unused by the routine itself
.trk_req_trackhead			dc.w		$FFFF		;	Track/head number to load 
.trk_req_destination		dc.l		0			;	Load destination address
.trk_current_track			dc.w		0			;	The track head currently is over
.trk_current_head			dc.w		2			;	Currently selected head (0 or 1)
.trk_dma_trackhead			dc.w		$FFFF		;	Track/head number DMA was activated for, or $FFFF if DMA not active
.trk_loaded_trackhead		dc.w		$FFFF		;	Recently loaded track/head number



trk_buffer_address			= .trk_buffer_address		- Track_Vars
trk_req_trackhead			= .trk_req_trackhead		- Track_Vars
trk_req_destination			= .trk_req_destination		- Track_Vars
trk_current_track			= .trk_current_track		- Track_Vars
trk_current_head			= .trk_current_head			- Track_Vars
trk_dma_trackhead			= .trk_dma_trackhead		- Track_Vars
trk_loaded_trackhead		= .trk_loaded_trackhead		- Track_Vars





; ================================================================ Track_ISR_Poll ================================================================
;
;	Uses d0-d1/a0-a1, saves the rest
;
Track_ISR_Poll:
		lea.l		Track_Vars(pc), a1
														
																	; ================================ Check if anything is to load ================================

		move.w		trk_req_trackhead(a1), d0						;	if( req_trackhead >= 0x8000 )
		bmi			.end_isr										;		return;


																	; ================================ Select head (d0 & 1) ================================

		move.w		d0, d1											; save d0
		and.w		#1, d0											;	head &= 1;
		cmp.w		trk_current_head(a1), d0						;	if( head != current_head )
		beq			.head_selected									;	{
		bset.b		#2, CIAB_PRB									;		CIAB_PRB |=  (1<<2);	// head 0
		tst.w		d0												;		if( head )
		beq			.head_0											;		{
		bclr.b		#2, CIAB_PRB									;			CIAB_PRB &= ~(1<<2);	// head 1
.head_0:															;		}
		move.w		d0, trk_current_head(a1)						;		current_head = head;
		move.w		#2, d0											;		_WaitScans(2);
		bsr			Track_Wait_Scans								;
.head_selected:														;	}
		move.w		d1, d0											; restore d0


																	; ================================ Seek track ================================

		lsr.w		d0												;	word seek_track = req_trackhead >> 1;

		cmp.w		trk_current_track(a1), d0						;	if( seek_track != current_track )
		beq			.seek_done										;	{
		bls			.step_out										;		if( seek_track > current_track )
																	;		{
		bsr			Track_Drive_StepIn								;			_Drive_StepIn();
		add.w		#1, trk_current_track(a1)						;			current_track++;
		bra			.end_isr										;			return;
																	;		}
																	;		else // seek_track < current_track
.step_out:															;		{
		bsr			Track_Drive_StepOut								;			_Drive_StepOut();
		sub.w		#1, trk_current_track(a1)						;			current_track--;
		bra			.end_isr										;			return;
																	;		}
.seek_done:															;	}
		move.w		d1, d0											; restore d0



																	; ================================ EXECUTE DMA ================================
		lea.l		$dff000, a0
		cmp.w		trk_dma_trackhead(a1), d0						;	if( dma_trackhead != trackhead )
		beq			.dma_started									;	{
																	;		// ================ Start disk DMA ================
		move.w		#2, INTREQ(a0)									;		INTREQ = 2;					// Clear "disk DMA finished" flag
		move.l		trk_req_destination(a1), DSKPT(a0)				;		DSKPT = destination;
		move.w		#$8210, DMACON(a0)								;		DMACON = 0x8210;			// Enable disk DMA
		move.w		#$4489, DSKSYNC(a0)								;		DSKSYNC = 0x4489;			// MFM sync pattren
		move.w		#$9500, ADKCON(a0)								;		ADKCON = 0x9500;			// Configure disk for MFM
		move.w		#$4000, DSKLEN(a0)								;		DSKLEN = 0x4000;
		move.w		#$9900, DSKLEN(a0)								;		DSKLEN = 0x9900;
		move.w		#$9900, DSKLEN(a0)								;		DSKLEN = 0x9900;
		move.w		d0, trk_dma_trackhead(a1)						;		dma_trackhead = trackhead;
		move.w		#$FFFF, trk_buffer_trackhead(a1)				;		trk_buffer_trackhead = 0xFFFF;	// invalidate track buffer
		bra			.end_isr										;		return;
.dma_started:														;	}

																	; ================================ WAIT FOR DMA ================================
		move.w		INTREQR(a0), d1									;	if( !(INTREQR & (1<<1)) )		// if not done
		btst.l		#1, d1											;	{
		beq			.end_isr										;		return;
																	;	}

																	; ================================ DMA DONE ================================
		move.w		trk_dma_trackhead(a1), trk_loaded_trackhead(a1)
		move.w		#$FFFF, trk_dma_trackhead(a1)

																	; ================================ Request done ================================
		move.w		#$FFFF, trk_req_trackhead(a1)


.end_isr:
		rts



; ================================================================ Track_Init ================================================================
;
Track_Init:
													;		// Motor on
		or.b		#$78, CIAB_PRB					;		CIAB_PRB |= 0x78;		// deselect all drives
		bclr.b		#7, CIAB_PRB					;		CIAB_PRB &= ~(1<<7);	// turn motor on
		bclr.b		#3, CIAB_PRB					;		CIAB_PRB &= ~(1<<3);	// select DF0

.wait_drive_ready:									;		// Wait for drive ready
		btst.b		#5, CIAA_PRA					;		while( CIAA_PRA & (1<<5) ){}
		bne			.wait_drive_ready				;

													;		
													;		// Seek track 0
													;		{
		move.w		#282, d2						;			word delay = 282;										[d2: delay]

.seek_track_zero:
		btst.b		#4, CIAA_PRA					;			while( CIAA_PRA & (1<<4) )
		beq			.track_zero						;			{
		bsr			Track_Drive_StepOut				;				_Drive_StepOut();
		move.w		d2, d0							;				_WaitScans(delay);
		bsr			Track_Wait_Scans				;
		move.w		#47, d2							;				delay = 47;
		bra			.seek_track_zero				;			}

.track_zero:

		lea.l		Track_Vars(pc), a6
		move.w		#$FFFF, trk_seek_track(a6)		;			seek_track = 0xFFFF;
		clr.w		trk_next_track(a6)				;			next_track = 0;
		clr.w		trk_current_track(a6)			;			current_track = 0;
		move.w		#2, trk_current_head(a6)		;			current_head = 2;	// force head change
													;			// _Drive_Head(0);	
													;		}
													;		

		move.w		#235, d0 						;		_WaitScans(235);	// settle time
		bsr			Track_Wait_Scans	

		rts





; ================================================================ Track_Wait_Scans ================================================================
;
Track_Wait_Scans:
		move		d1, -(a7)
		subq.l		#1, d0
.loop:												;	while( count-->0 )
													;	{
		move.b		$dff006, d1						;		byte pos = VHPOSR_B;

.scanwait:											;		while( VHPOSR_B == pos ){}
		cmp.b		$dff006, d1
		beq			.scanwait

		dbra.w		d0, .loop						;	}
		move		(a7)+, d1
		rts


; ================================================================ Track_Drive_StepOut ================================================================
;
Track_Drive_StepOut:
		bset.b		#1, CIAB_PRB					;	CIAB_PRB |=  (1<<1);	// direction: out
		bclr.b		#0, CIAB_PRB					;	CIAB_PRB &= ~(1<<0);	// step pulse LOW
		bset.b		#0, CIAB_PRB					;	CIAB_PRB |=  (1<<0);	//
		rts


; ================================================================ Track_Drive_StepIn ================================================================
;
Track_Drive_StepIn:
		bclr.b		#1, CIAB_PRB					;	CIAB_PRB &= ~(1<<1);	// direction: in
		bclr.b		#0, CIAB_PRB					;	CIAB_PRB &= ~(1<<0);	// step pulse LOW
		bset.b		#0, CIAB_PRB					;	CIAB_PRB |=  (1<<0);	//
		rts


; ================================================================ Track_Load_Track ================================================================
;
;	Uses d0-d1/a0-a1, saves the rest
;
; Input:
;	d0		- trackhead = track * 2 + head
;
; Output:
;	d0		- 1: correct track is in the buffer now		(else zero)
;
Track_Load_Track:
		lea.l		Track_Vars(pc), a1

		cmp.w		trk_buffer_trackhead(a1), d0
		beq			.all_done


		move.w		d0, d1								; ================================ SEEK TRACK (d0>>1) ================================
		lsr.w		#1, d1
		move.w		d1, trk_seek_track(a1)
		cmp.w		trk_current_track(a1), d1
		bne			.not_done_seeking

														; ================================ SELECT HEAD (d0&1) ================================
		move.w		d0, d1								; save d0
		and.w		#1, d0								;	head &= 1;
		cmp.w		trk_current_head(a1), d0			;	if( head != current_head )
		beq			.head_selected						;	{
		bset.b		#2, CIAB_PRB						;		CIAB_PRB |=  (1<<2);	// head 0
		tst.w		d0									;		if( head )
		beq			.head_0								;		{
		bclr.b		#2, CIAB_PRB						;			CIAB_PRB &= ~(1<<2);	// head 1
.head_0:												;		}
		move.w		d0, trk_current_head(a1)			;		current_head = head;
		move.w		#2, d0								;		_WaitScans(2);
		bsr			Track_Wait_Scans					;
.head_selected:											;	}
		move.w		d1, d0								; restore d0
		
		
														; ================================ EXECUTE DMA ================================
		lea.l		$dff000, a0
		cmp.w		trk_dma_trackhead(a1), d0			;	if( dma_trackhead != trackhead )
		beq			.dma_started						;	{
														;		// ================ Start disk DMA ================
		move.w		#2, INTREQ(a0)						;		INTREQ = 2;					// Clear "disk DMA finished" flag
		move.l		trk_buffer_address(a1), DSKPT(a0)	;		DSKPT = destination;
		move.w		#$8210, DMACON(a0)					;		DMACON = 0x8210;			// Enable disk DMA
		move.w		#$4489, DSKSYNC(a0)					;		DSKSYNC = 0x4489;			// MFM sync pattren
		move.w		#$9500, ADKCON(a0)					;		ADKCON = 0x9500;			// Configure disk for MFM
		move.w		#$4000, DSKLEN(a0)					;		DSKLEN = 0x4000;
		move.w		#$9900, DSKLEN(a0)					;		DSKLEN = 0x9900;
		move.w		#$9900, DSKLEN(a0)					;		DSKLEN = 0x9900;
		move.w		d0, trk_dma_trackhead(a1)			;		dma_trackhead = trackhead;
		move.w		#$FFFF, trk_buffer_trackhead(a1)	;		trk_buffer_trackhead = 0xFFFF;	// invalidate track buffer
		bra			.not_done_dmastart					;		return 0;
.dma_started:											;	}

														; ================================ WAIT FOR DMA ================================
		move.w		INTREQR(a0), d1						;	if( !(INTREQR & (1<<1)) )		// if not done
		btst.l		#1, d1								;	{
		beq			.not_done_dmawait					;		return 0;
														;	}

														; ================================ DMA DONE ================================
		move.w		trk_dma_trackhead(a1), trk_buffer_trackhead(a1)
		move.w		#$FFFF, trk_dma_trackhead(a1)


.all_done:												; ================================ ALL OK ================================
		moveq.l		#1, d0
		bra			.end

.not_done:
		moveq.l		#0, d0
.end:
		rts

.not_done_seeking:
		lea.l		.str_seeking(pc), a0
		bsr			Print_String
		bra			.not_done

.not_done_dmastart:
		lea.l		.str_dmastart(pc), a0
		bsr			Print_String
		bra			.not_done

.not_done_dmawait:
		lea.l		.str_dmawait(pc), a0
		bsr			Print_String
		bra			.not_done


.str_seeking		dc.b		"SEEKING...", 0
.str_dmastart		dc.b		"DMA START...", 0
.str_dmawait		dc.b		"DMA WAIT...", 0
					even


; ================================================================ Track_Load_Sector ================================================================
;
;	Uses d0-d1/a0-a1, saves the rest
;
;
;	Input:
;		d2			Sector number
;		a2			Target buffer
;
;	Output:
;		d0			0: in progress
;					1: done
;					2: error
;
Track_Load_Sector:
		clr.l		d0
		move.w		d2, d0
		divu.w		#11, d0
		bsr			Track_Load_Track
		tst.w		d0
		beq			.not_done


															;	__d0 word Track_Decode(__a1 void *track_data, __d0 word start_sec, __d1 word num_sec, __a0 void *target_buffer);
		lea.l		Track_Vars+trk_buffer_address(pc), a1	;		a1 = target buffer
		clr.l		d0										;		start_sec = sector_number % 11
		move.w		d2, d0
		divu.w		#11, d0
		swap.w		d0
		move.w		#1, d1									;		num_sec = 1
		move.l		a2, a0									;		target buffer
		bsr			Track_Decode
		tst.w		d0
		beq			.error


.all_done:
		moveq.l		#1, d0
		bra			.end

.error:	
		moveq.l		#2, d0
		bra			.end

.not_done:
		moveq.l		#0, d0
.end:
		rts




; ================================================================ Track_Decode ================================================================
;
;	__d0 word Track_Decode(__a1 void *track_data, __d0 word start_sec, __d1 word start_sec, __a0 void *target_buffer);
;

MFMsync		= $4489

Track_Decode:
		movem.l		d1-d7/a0-a6, -(sp)
		lea			$dff000, a6
		lea 		$bfd100, a4
		lea			MFMchk(pc), a5				;	(a5): MFMchk
	
		move.w 		d0, -(sp)					; save startsec
		move.w 		d1, -(sp)					; save numsec
	
		move.w 		d0,	d2						; d0: start_sec
		add.w 		d1,	d2						; d2: end_sector = start_sec + start_sec
		cmp.w 		#11, d2						;
		ble.s		.NoOvr						;	if( end_sector > 11 )
		moveq		#11, d2						;		end_sector = 11
.NoOvr:	
		sub.w 		d0, d2						; num_sec = end_sector - start_sec - 1		[d1: loop counter]
		move.w		d2, d1						;
		subq.w		#1, d1						; 

		move.l		#$55555555, d3				; and-const

.FindS:	
		cmp.w 		#MFMsync, (a1)+				;search for a sync word
		bne.s 		.FindS
		cmp.b 		(a1), d3					;search for 0-nibble
		bne.s 		.FindS
		move.l 		(a1)+, d4					;decode fmtbyte/trk#,sec#,eow#
		move.l 		(a1)+, d5
		and.w 		d3, d4
		and.w 		d3, d5
		add.w 		d4, d4
		or.w 		d5, d4
		lsr.w 		#8, d4						;sec#
		sub.w 		d0, d4						;do we want this sec?
		bmi.s 		.Skip
		cmp.w 		d2, d4
		blt.s 		.DeCode
.Skip:	
		lea 		48+1024(a1), a1				;nope
		bra.s 		.FindS

.DeCode:
		lea 		40(a1), a1					;found a sec,skip unnecessary data
		clr.l 		(a5)
		move.l 		(a1)+, d6					;decode data chksum.L
		move.l 		(a1)+, d5
		and.l 		d3, d6
		and.l 		d3, d5
		add.l 		d6, d6
		or.l 		d5, d6						;chksum
		lea 		512(a1), a2
		add.w 		d4, d4						;x512
		lsl.w 		#8, d4
		lea 		(a0,d4.w), a3				;dest addr for this sec
		moveq 		#127, d7
.DClup:	
		move.l 		(a1)+, d4
		move.l 		(a2)+, d5
		and.l 		d3, d4
		and.l 		d3, d5
		eor.l 		d4, d6						;EOR with checksum
		eor.l 		d5, d6						;EOR with checksum
		add.l 		d4, d4
		or.l 		d5, d4
		move.l 		d4, (a3)+
		dbf 		d7, .DClup					;chksum should now be 0 if correct
		or.l 		d6, (a5)					;or with track total chksum
		move.l 		a2, a1
		dbf 		d1, .FindS					;decode next sec
		move.w 		(sp)+, d1
		move.w 		(sp)+, d0
		tst.l 		(a5)						;track total chksum OK?
		bne.w 		.end_error					;no,retry
		moveq 		#0, d0						;set to start of track
		move.w 		d2, d3
		add.w 		d3, d3
		lsl.w 		#8, d3
		add.w 		d3, a0
		sub.w 		d2, d1						;sub #secs loaded

.end_ok:
		moveq 		#1, d0
		bra.s		.end

.end_error:
		moveq 		#0,d0

.end:
		movem.l		(sp)+, d1-d7/a0-a6
		rts
	
MFMchk:		dc.l 0  
