


CONSOLE_FONT_HEIGHT			=		8

CONSOLE_SCANLINES			=		CONSOLE_ROWS * CONSOLE_FONT_HEIGHT





; ================================================================ Print_Init ================================================================
;
;	All registers possibly used
;
Print_Init:
		lea.l		ProgStart(pc), a1						;	lea.l		ConsoleBuffer(pc), a1
		add.l		#ConsoleBuffer-ProgStart, a1

		move.l		a1, d0
		move.l		#40*CONSOLE_SCANLINES, d1
		moveq.l		#0, d2
		bsr			Mem_Paint

		lea.l		copper_console_screen(pc), a0
		moveq.l		#CONSOLE_ROWS-1, d1

.loop:
		move.l		a1, d0
		move.w		d0, 2(a0)
		swap.w		d0
		move.w		d0, 6(a0)
		adda.w		#copper_console_screen_1-copper_console_screen, a0
		adda.w		#CONSOLE_FONT_HEIGHT*40, a1
		dbra.w		d1, .loop

		rts


; ================================================================ Print_Newline ================================================================
;
;	Scroll console up, clear bottom line and prepare it for printing.
;	All registers saved.
;
;	d0 d1 d2 d3 d4 d5 d6 d7		a0 a1 a2 a3 a4 a5 a6 a7
;
Print_Newline:
		movem.l		d0-d2/a0-a1, -(a7)

		lea.l		Print_Vars(pc), a1

		lea.l		copper_console_screen(pc), a0			;	load topmost row address			[d0]
		move.w		6(a0), d0
		swap.w		d0
		move.w		2(a0), d0
		move.l		d0, prn_target_pointer(a1)
		add.l		#40, prn_target_pointer(a1)
		move.w		#8, prn_target_subbyte(a1)
		
		move.l		d0, a1									;	clear the line
		clr.l		d1
		moveq.l		#CONSOLE_FONT_HEIGHT*40/4-1, d2
.clear_loop:
		move.l		d1, (a1)+
		dbra.w		d2, .clear_loop


		moveq.l		#CONSOLE_ROWS-2, d2						; move copper list up
.scroll_loop:
		move.w		copper_console_screen_1-copper_console_screen+2(a0), 2(a0)
		move.w		copper_console_screen_1-copper_console_screen+6(a0), 6(a0)
		adda.w		#copper_console_screen_1-copper_console_screen, a0
		dbra.w		d2, .scroll_loop
		
		move.w		d0, 2(a0)								; put the current row last
		swap.w		d0
		move.w		d0, 6(a0)

		movem.l		(a7)+, d0-d2/a0-a1
		rts


; ================================================================ Print_Start ================================================================
;
;	Registers:
;	
;		d0		IN			Console line number
;
;		a2		saved		(temp)
;		a3		saved		(Print_Vars)
;	
;
Print_Start:
		movem.l		a2-a3, -(a7)

		lea.l		Print_Vars(pc), a3

		mulu.w		#CONSOLE_FONT_HEIGHT*40, d0
		lea.l		ProgStart(pc), a2						;	lea.l		ConsoleBuffer+40(pc), a2
		add.l		#ConsoleBuffer+40-ProgStart, a2
		lea.l		(a2,d0.w), a2
		move.l		a2, prn_target_pointer(a3)
		
		move.w		#8, prn_target_subbyte(a3)
		
		movem.l		(a7)+, a2-a3
		rts






; ================================================================ Print_Char ================================================================
;
;	Registers:
;
;		d0		IN			Character to print
;		d1		saved		(height)
;		d2		saved		(width)
;		d3		saved		Target sub-byte possition		8..1	(incremented by char width at the end)
;
;		a0		saved		(dst)
;		a1		saved		(src)
;		a2		saved		Target screen pointer					(incremented by char width at the end)
;
Print_Char:
		movem.l		d1-d3/a0-a3, -(a7)
		lea.l		Print_Vars(pc), a3
		move.l		prn_target_pointer(a3), a2
		move.w		prn_target_subbyte(a3), d3

		lea.l		Font_data(pc), a1					;												[a1: font data]
		move.b		3(a1), d2							;	d2: space width

		cmp.b		1(a1), d0							;	d0 ? first_code
		blo			.move_cursor
		cmp.b		2(a1), d0							;	d0 ? last_code
		bhi			.move_cursor

		sub.b		1(a1), d0							;	a1 += (d0-first_code)*height + 4			[a1: point to glyph]
		and.w		#$00FF, d0
		moveq.l		#0, d1								;	d1.w = height
		move.b		(a1), d1
		mulu.w		d1, d0
		lea.l		4(a1,d0.w), a1


		move.b		(a1)+, d2							;	d2: width

		move.l		a2, a0								;	a0: dst
		subq.l		#2, d1
.loop:													;	while( d1-->0 ) {
		moveq.l		#0, d0								;		pixels = *src++ & 0x00FF;
		move.b		(a1)+, d0
		lsl.w		d3, d0								;		pixels <<= xpos;
		or.b		d0, 1(a0)							;		dst[1] |= pixels & 0x00FF;
		lsr.w		#8, d0								;		dst[0] |= pixels >> 8;
		or.b		d0, (a0)
		adda.w		#40, a0
		dbra.w		d1, .loop							;	}

.move_cursor:
		sub.b		d2, d3								;	d3 ? d2
		bhi			.end
		addq.l		#8, d3
		addq.l		#1, a2

.end:
		lea.l		Print_Vars(pc), a3
		move.l		a2, prn_target_pointer(a3)
		move.w		d3, prn_target_subbyte(a3)
		movem.l		(a7)+, d1-d3/a0-a3
		rts



; ================================================================ Print_String ================================================================
;
;	Registers:
;
;		a0		IN			string to print		(OUT: one byte after the string trailing zero)
;
;		d0		saved		(temp)
;
;	Special characters:
;		\n		- start new line
;		\x01	- print d1 as 4+ digit hex number
;		\x02	- print d1 as 4+ digit hex number
;		\x03	- print d1 as 4+ digit hex number
;
;
Print_String:
		move.l		d0, -(a7)
.loop:
		move.b		(a0)+, d0
		beq			.end
		cmp.b		#10, d0
		ble			.special_char
		bsr			Print_Char
		bra			.loop
.end:
		move.l		(a7)+, d0
		rts

.special_char:
		move.l		a0, -(a7)
		and.w		#$00FF, d0
		lsl.w		#1, d0
		lea.l		.print_special_codes(pc), a0
		move.w		(a0,d0.w), d0
		beq			.skip_special
		adda.w		d0, a0
		jsr			(a0)
.skip_special:
		move.l		(a7)+, a0
		bra			.loop
		

.print_special_codes:
		dc.w		0
		dc.w		.print_d1		- .print_special_codes
		dc.w		.print_d2		- .print_special_codes
		dc.w		.print_d3		- .print_special_codes
		dc.w		0
		dc.w		0
		dc.w		0
		dc.w		0
		dc.w		0
		dc.w		0
		dc.w		.print_newline	- .print_special_codes

.print_d1:
		move.l		d1, d0
		bra			Print_Hex_4

.print_d2:
		move.l		d2, d0
		bra			Print_Hex_4

.print_d3:
		move.l		d3, d0
		bra			Print_Hex_4

.print_newline:
		bra			Print_Newline


; ================================================================ Print_Hex ================================================================
;
;	Registers:
;
;		d0		saved		value to print
;		d1		IN			character count
;		d2		saved		(temp digit)
;
;		a0		saved		(string buffer)
;
Print_Hex:
		movem.l		d0/d2/a0, -(a7)
		lea.l		.buffer+8(pc), a0
		sub.l		#1, d1
.loop:
		move.w		d0, d2
		and.w		#$000F, d2
		move.b		.hex(pc,d2.w), -(a0)
		lsr.l		#4, d0
		dbra.w		d1, .loop
		bne			.loop_again
		bsr			Print_String
		movem.l		(a7)+, d0/d2/a0
		rts

.loop_again:
		moveq.l		#0, d1
		bra			.loop


.buffer:
		dc.b		"        ", 0
.hex:
		dc.b		"0123456789ABCDEF"
		even


;	Registers:
;		d0		IN			value to print			(all registers saved)
Print_Hex_2:
		move.l		d1, -(a7)
		moveq.l		#2, d1
		bsr			Print_Hex
		move.l		(a7)+, d1
		rts

;	Registers:
;		d0		IN			value to print			(all registers saved)
Print_Hex_4:
		move.l		d1, -(a7)
		moveq.l		#4, d1
		bsr			Print_Hex
		move.l		(a7)+, d1
		rts

;	Registers:
;		d0		IN			value to print			(all registers saved)
Print_Hex_6:
		move.l		d1, -(a7)
		moveq.l		#6, d1
		bsr			Print_Hex
		move.l		(a7)+, d1
		rts

;	Registers:
;		d0		IN			value to print			(all registers saved)
Print_Hex_8:
		move.l		d1, -(a7)
		moveq.l		#8, d1
		bsr			Print_Hex
		move.l		(a7)+, d1
		rts




		; ================================================================ Font data ================================================================

Font_data:
		; Format:
		;	<height>
		;	<first_code>
		;	<last_code>
		;	<space_width>
		;	<glyph_data>[]
		;		<width>
		;		<pixels>	[x <height>]			Rows of pixels
		;

		incbin		"data/smallfont.bin"
		even
