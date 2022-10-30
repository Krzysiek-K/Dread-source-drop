


; ================================================================ Copper generator state ================================================================
;
;
;	d7		- Y coord
;
;	a6		- write pointer
;




; ================================================================ Copper generator ================================================================

; ------------------------------------------------ Copper_Start ------------------------------------------------
;
; Reset copper generator registers
;
Copper_Start:
		moveq.l		#0, d7
		lea.l		LocaImg_copper(pc), a6
		rts


; ------------------------------------------------ Copper_Wait ------------------------------------------------
;
; Draw 1 bit image
;
;	d0	- Y delay	(number of lines to wait)
;	d1	(used)
;
Copper_Wait:

		move.w		d7, d1
		add.w		d0, d7
		eor.w		d7, d1
		and.w		#$FF00, d1
		beq			.no_256
		move.l		#$FFDFFFFE, (a6)+		; wait for end of line 255
.no_256:
		move.w		d7, d1
		lsl.w		#8, d1					;	d0.w <<= 8;
		or.w		#$0F, d1				;	d0.w |= 0x0F;
		move.w		d1, (a6)+				;	(a0).w+ = d0.w;		(a0).w+ = 0xFFFE;		// wait
		move.w		#$FFFE, (a6)+			;
		rts

; ------------------------------------------------ Copper_Image_1 ------------------------------------------------
;
; Draw 1-bit image
;
;	d0	(used)
;	a0	- bitplane 0
;
Copper_Image_1:
		move.l		a0, d0
		move.w		#$0E2, (a6)+			;			(a0).w+ = 0x0E2;		(a0).w+ = d0.w;
		move.w		d0, (a6)+				;
		swap.w		d0						;swap d0;	(a0).w+ = 0x0E0;		(a0).w+ = d0.w;
		move.w		#$0E0, (a6)+			;
		move.w		d0, (a6)+				;
		move.w		#$100, (a6)+			;			(a0).w+ = 0x100;		(a0).w+ = 0x1200;
		move.w		#$1200, (a6)+			;
		rts

; ------------------------------------------------ Copper_Image_4 ------------------------------------------------
;
; Draw 4-bit image
;
;	d0	(used)
;	a0	- bitplane 0
;	a1	- bitplane 1
;	a2	- bitplane 2
;	a3	- bitplane 3
;
Copper_Image_4:
		move.l		a0, d0
		move.w		#$0E2, (a6)+			;	(a6).w+ = 0x0E2;
		move.w		d0, (a6)+				;	(a6).w+ = d0.w;
		swap.w		d0						;	swap d0;
		move.w		#$0E0, (a6)+			;	(a6).w+ = 0x0E0;
		move.w		d0, (a6)+				;	(a6).w+ = d0.w;

		move.l		a1, d0
		move.w		#$0E6, (a6)+			;	(a6).w+ = 0x0E6;
		move.w		d0, (a6)+				;	(a6).w+ = d0.w;
		swap.w		d0						;	swap d0;
		move.w		#$0E4, (a6)+			;	(a6).w+ = 0x0E4;
		move.w		d0, (a6)+				;	(a6).w+ = d0.w;

		move.l		a2, d0
		move.w		#$0EA, (a6)+			;	(a6).w+ = 0x0EA;
		move.w		d0, (a6)+				;	(a6).w+ = d0.w;
		swap.w		d0						;	swap d0;
		move.w		#$0E8, (a6)+			;	(a6).w+ = 0x0E8;
		move.w		d0, (a6)+				;	(a6).w+ = d0.w;

		move.l		a3, d0
		move.w		#$0EE, (a6)+			;	(a6).w+ = 0x0EE;
		move.w		d0, (a6)+				;	(a6).w+ = d0.w;
		swap.w		d0						;	swap d0;
		move.w		#$0EC, (a6)+			;	(a6).w+ = 0x0EC;
		move.w		d0, (a6)+				;	(a6).w+ = d0.w;

		move.w		#$100, (a6)+			;	(a6).w+ = 0x100;
		move.w		#$4200, (a6)+			;	(a6).w+ = 0x4200;

		rts



		; ================================================================ CONSOLE_LINE_COPPER ================================================================


CONSOLE_LINE_COPPER:	macro		row_index
	if (\1 < CONSOLE_ROWS)
			dc.w		((44+200-CONSOLE_SCANLINES +  \1*CONSOLE_FONT_HEIGHT+0)<<8) | $0F, $FFFE			; ======== wait for line 200-32
		if (\1 == 0)
			dc.w		$100,	$1200							; BPLCON0
			dc.w		$180,	$000							; COLOR0
			dc.w		$108,	0								; BPL1MOD
			dc.w		$10A,	0								; BPL2MOD
copper_console_screen:
		endif
		if (\1 == 1)
copper_console_screen_1:
		endif
			dc.w		$0E2,	$0								; BPL0PT
			dc.w		$0E0,	$0								;
			dc.w		$182,	$888							; COLOR1
			dc.w		((44+200-CONSOLE_SCANLINES +  \1*CONSOLE_FONT_HEIGHT+1)<<8) | $0F, $FFFE			; next line
			dc.w		$182,	$888							; COLOR1
			dc.w		((44+200-CONSOLE_SCANLINES +  \1*CONSOLE_FONT_HEIGHT+2)<<8) | $0F, $FFFE			; next line
			dc.w		$182,	$CCC							; COLOR1
			dc.w		((44+200-CONSOLE_SCANLINES +  \1*CONSOLE_FONT_HEIGHT+3)<<8) | $0F, $FFFE			; next line
			dc.w		$182,	$FFF							; COLOR1
			dc.w		((44+200-CONSOLE_SCANLINES +  \1*CONSOLE_FONT_HEIGHT+4)<<8) | $0F, $FFFE			; next line
			dc.w		$182,	$DDD							; COLOR1
			dc.w		((44+200-CONSOLE_SCANLINES +  \1*CONSOLE_FONT_HEIGHT+5)<<8) | $0F, $FFFE			; next line
			dc.w		$182,	$BBB							; COLOR1
			dc.w		((44+200-CONSOLE_SCANLINES +  \1*CONSOLE_FONT_HEIGHT+6)<<8) | $0F, $FFFE			; next line
			dc.w		$182,	$999							; COLOR1
			dc.w		((44+200-CONSOLE_SCANLINES +  \1*CONSOLE_FONT_HEIGHT+7)<<8) | $0F, $FFFE			; next line
			dc.w		$182,	$777							; COLOR1
	endif
  endm




		; ================================================================ Copper list ================================================================
LocaImg_copper:
			dc.w		$0E2,	$0								; BPL0PT
			dc.w		$0E0,	$0								;
			dc.w		$0E6,	$0								; BPL1PT
			dc.w		$0E4,	$0								;
			dc.w		$0EA,	$0								; BPL2PT
			dc.w		$0E8,	$0								;
			dc.w		$0EE,	$0								; BPL3PT
			dc.w		$0EC,	$0								;
			dc.w		$180,	$000							; COLOR0
			dc.w		$182,	$FFF							; COLOR1
			dc.w		$100,	$4200							; BPLCON0
			dc.w		$108,	40*3							; BPL1MOD
			dc.w		$10A,	40*3							; BPL2MOD

  if SHOW_TRACK_BITSTREAM
			dc.w		$100,	$1200							; BPLCON0
			dc.w		$108,	0								; BPL1MOD
			dc.w		$10A,	0								; BPL2MOD
			dc.w		$180,	$000							; COLOR0
			dc.w		$182,	$FFF							; COLOR1
LocalImg_copper_trkbits:
			dc.w		$0E2,	$200							; BPL0PT
			dc.w		$0E0,	$0								;
  endif

			CONSOLE_LINE_COPPER		 0
			CONSOLE_LINE_COPPER		 1
			CONSOLE_LINE_COPPER		 2
			CONSOLE_LINE_COPPER		 3
			CONSOLE_LINE_COPPER		 4
			CONSOLE_LINE_COPPER		 5
			CONSOLE_LINE_COPPER		 6
			CONSOLE_LINE_COPPER		 7
			CONSOLE_LINE_COPPER		 8
			CONSOLE_LINE_COPPER		 9
			CONSOLE_LINE_COPPER		10
			CONSOLE_LINE_COPPER		11
			CONSOLE_LINE_COPPER		12
			CONSOLE_LINE_COPPER		13
			CONSOLE_LINE_COPPER		14
			CONSOLE_LINE_COPPER		15

			;dc.l		$FFDFFFFE								; wait 256
			dc.w		((44+200)<<8) | $0F, $FFFE				; ======== wait for line 200
			dc.w		$100,	$0200
			dc.w		$180,	$000

			dc.l		$FFFFFFFE							; END
