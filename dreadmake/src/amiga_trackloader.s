

TRACK_BUFFER_SIZE				= 12980
TRACK_DECODED_SIZE				= 11*512



	if COMPILING_BOOTLOADER = 0
		xref		_trk
	endif



; ================================================================ Track_ISR_Poll ================================================================
;
;	Uses d0-d1/a0-a1, saves the rest
;
Track_ISR_Poll:
	if COMPILING_BOOTLOADER
		lea.l		Track_Vars(pc), a1
	else
		lea.l		_trk, a1
	endif

																	; ================================ Check if anything is to load ================================

		move.w		trk_req_trackhead(a1), d0						;	if( req_trackhead >= 0x8000 )
		bmi			.end_isr										;		return;


		move.w		d0, d1											; save d0

																	; ================================ Select head (d0 & 1) ================================

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


;	if COMPILING_BOOTLOADER = 1
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
		move.w		#$4000, DSKLEN(a0)								;		DSKLEN = 0x4000;			// 01	DMA off		write	length = 0
		move.w		#$9900, DSKLEN(a0)								;		DSKLEN = 0x9900;			// 10	DMA on		read	length = $1900
		move.w		#$9900, DSKLEN(a0)								;		DSKLEN = 0x9900;			// 10	DMA on		read	length = $1900
		move.w		d0, trk_dma_trackhead(a1)						;		dma_trackhead = trackhead;
		move.w		#$FFFF, trk_loaded_trackhead(a1)				;		loaded_trackhead = 0xFFFF;	// invalidate track buffer
		
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
		move.w		#$4000, DSKLEN(a0)								;		DSKLEN = 0x4000;			// 01	DMA off		write	length = 0

																	; ================================ Request done ================================
		move.w		#$FFFF, trk_req_trackhead(a1)
;	endif
	

.end_isr:
		rts



; ================================================================ Track_Init ================================================================
;
	public _Track_Init_asm
_Track_Init_asm:
Track_Init:
		movem.l		d2/a6, -(a7)
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

	if COMPILING_BOOTLOADER
		lea.l		Track_Vars(pc), a6
	else
		lea.l		_trk, a6
	endif
		clr.w		trk_current_track(a6)			;			current_track = 0;
		move.w		#2, trk_current_head(a6)		;			current_head = 2;	// force head change
													;			// _Drive_Head(0);	
													;		}
													;		

		move.w		#235, d0 						;		_WaitScans(235);	// settle time
		bsr			Track_Wait_Scans	

		movem.l		(a7)+, d2/a6
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






; ================================================================ Track_Decode ================================================================
;
;	__d0 word Track_Decode(__a1 void *track_data, __d0 word start_sec, __d1 word num_sec, __a0 void *target_buffer);
;
;	All registers SAVED except <d0> holding return value.
;
;	d0		start_sec							result
;	d1		num_sec								sector loop counter
;	d2		end_sector/num_sec
;	d3		#$55555555
;	d4		(temp)
;	d5		(temp)
;	d6		checksum
;	d7		checksum loop counter
;
;	a0		target buffer						(never modified)
;	a1		track_data							(main pointer)
;	a2		second sector half read pointer
;	a3		sector write destination
;	a4		-
;	a5		-
;	a6		-
;
;
;

MFMsync		= $4489

	public _Track_Decode
_Track_Decode:
Track_Decode:
		movem.l		d1-d7/a0-a6, -(sp)

		lea.l		TRACK_BUFFER_SIZE-2(a1), a6			;	src_end = src + (TRACK_BUFFER_SIZE-2);								[a6]
	
		move.w 		d0,	d2								;	end_sector = start_sec + num_sec;									[d2]
		add.w 		d1,	d2								;
		cmp.w 		#11, d2								;
		ble.s		.NoOvr								;	if( end_sector > 11 )
		moveq		#11, d2								;		end_sector = 11
.NoOvr:	
		sub.w 		d0, d2								;	num_sec = end_sector - start_sec - 1;								[d1: loop counter]
		move.w		d2, d1								;
		subq.w		#1, d1								; 

		move.l		#$55555555, d3						;	mask = 0x55555555;													[d3: and-const]

		move.w		#MFMsync, (a6)						;	*src_end = MFMsync;													make sure the following loop terminates

.FindS:	
		cmp.w 		#MFMsync, (a1)+						;	while( *src++ != MFMsync ) {}										search for a sync word
		bne.s 		.FindS

		cmpa.l		a6, a1								;	if( src >= src_end )
		bhs			.end_error_1						;		return 1;					// error 1 - scanned past end of the track while looking for MFMsync

		cmp.b 		(a1), d3							;	check odd bits of the first byte of track header in encoded form		($FF value, checked mask -1-1-1-1)
		bne.s 		.FindS

		move.l 		(a1)+, d4							;	dword odd_bits = *src++;											[d4]		decode fmtbyte/trk#,sec#,eow#
		move.l 		(a1)+, d5							;	dword even_bits = *src++;											[d5]
		and.w 		d3, d4								;	dword decoded = ((odd_bits & mask)<<1) | (even_bits & mask);					TBD: verify header checksum
		and.w 		d3, d5								;
		add.w 		d4, d4								;
		or.w 		d5, d4								;

		lsr.w 		#8, d4								;	word sector = (word(decoded) >> 8) - start_sec;						[d4]
		sub.w 		d0, d4								;
		bmi.s 		.Skip								;	if( short(sector) < 0 ) skip;
		cmp.w 		d2, d4								;	if( sector < num_sec )
		blt.s 		.DeCode								;		decode;
.Skip:	
		lea 		48+1024(a1), a1						;	src += 48+1024;														skip this sector
		cmpa.l		a6, a1								;	if( src >= src_end )
		bhs			.end_error_2						;		return 0;					// error 2 - scanned past end of the track when skipping a sector
		bra.s 		.FindS

.DeCode:
		lea 		40(a1), a1							;	src += 40;																		found a sec,skip unnecessary data
		move.l 		(a1)+, d6							;	dword odd_bits = *src++;											[d6]		decode data chksum.L
		move.l 		(a1)+, d5							;	dword even_bits = *src++;											[d5]
		and.l 		d3, d6								;	dword chksum = ((odd_bits & mask)<<1) | (even_bits & mask);			[d6: checksum]
		and.l 		d3, d5
		add.l 		d6, d6
		or.l 		d5, d6

		lea 		512(a1), a2							;	dword *src_even = src + 512;										[a2]
		add.w 		d4, d4								;	dword *dst = target_buffer + (sector << 9);		// x512
		lsl.w 		#8, d4								;
		lea 		(a0,d4.w), a3						;

		moveq 		#127, d7							;	for(word count=127; count>=0; count--)								[d7]
.DClup:													;	{
		move.l 		(a1)+, d4							;		dword odd_bits = *src++;										[d4]
		move.l 		(a2)+, d5							;		dword even_bits = *src++;										[d5]
		and.l 		d3, d4								;		dword decoded = ((odd_bits & mask)<<1) | (even_bits & mask);
		and.l 		d3, d5								;
		eor.l 		d4, d6								;		chksum ^= (odd_bits & mask);			EOR with checksum
		eor.l 		d5, d6								;		chksum ^= (even_bits & mask);			EOR with checksum
		add.l 		d4, d4								;
		or.l 		d5, d4								;
		move.l 		d4, (a3)+							;		*dst++ = decoded
		dbf 		d7, .DClup							;	}										// chksum should now be 0 if correct

		tst.l 		d6									;	track total chksum OK?
		bne 		.end_error_3						;	no,retry						// error 3 - bad track data checksum


		move.l 		a2, a1								;	src = src_even;
		dbf 		d1, .FindS							;											// decode next sec
.end_ok:
		moveq.l		#0, d0
		bra			.end


.end_error_1:
		moveq.l		#1, d0
		bra			.end
.end_error_2:
		moveq.l		#2, d0
		bra			.end
.end_error_3:
		moveq.l		#3, d0
.end:
		movem.l		(sp)+, d1-d7/a0-a6
		rts
	
MFMchk:		dc.l 0  
