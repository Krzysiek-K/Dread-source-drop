


NUM_RAM_ENTRIES					= 8					; should match mem.c

STACK_PAINT_VALUE				= $C7EF



		
; ================================================================ Mem_Detect - detect system memory regions ================================================================
;
; internal:
;
;	a2		- destination RAM_Info pointer
;	a3		- RAM_Info_end
;	d2		- RAM type
;
;	

Mem_Detect:
		move.l		$4.w, a6
		move.l		#$0002, d2
		lea.l		RAM_Info(pc), a2

		move.w		#$0002, (a2)+						; save the current program range as CHIP			(must be FIRST not to overwrite the program during memory test)
		lea.l		ProgStart(pc), a0
		move.l		a0, (a2)+
		adda.l		#ProgEnd-ProgStart, a0
		move.l		a0, (a2)+

		move.w		#$0002, (a2)+						; save $0000..$FFFF as CHIP
		move.l		#$0000, (a2)+
		move.l		#$10000, (a2)+

		lea.l		RAM_Info_end(pc), a3

.loop:
		cmpa.l		a3, a2								; a2 ? end
		bhs			.end

		move.l		d2, d1
		or.l		#$20000, d1
		jsr			sys_exec_AvailMem(a6)
		move.l		d0, 6(a2)
		beq			.out_of_mem_type

		move.w		d2, (a2)
		move.l		d2, d1
		jsr			sys_exec_AllocMem(a6)				; d0 already set
		move.l		d0, 2(a2)
		beq			.out_of_mem_type
		add.l		d0, 6(a2)

		adda.w		#10, a2
		bra			.loop

.out_of_mem_type:
		move.w		.mem_next_type(pc,d2.w), d2
		bpl			.loop

		clr.l		6(a2)

.end:
		rts

.mem_next_type:
		dc.w		-1, $0004, $0000



		

; ================================================================ [internal] Mem_Probe - test if the specified region indeed contains R/W memory ================================================================
;
;	[ Macro, because we will most likely overwrite the stack]
;
;	registers:
;
;		a0		- start address		[saved]
;		a1		- end address		[saved]		(dirst address after the region)
;		a3		- (used)
;		d0		- 0/1 result		[out]
;
Mem_Probe:	macro
		move.l		a0, a3
		move.l		#$4598D3AB, d0
.write_loop\@:
		move.l		d0, (a3)+
		add.l		#$9625EF51, d0
		cmpa.l		a1, a3					;	a3 ? a1
		blo			.write_loop\@

		move.l		a0, a3
		move.l		#$4598D3AB, d0
.verify_loop\@:
		cmp.l		(a3)+, d0
		bne			.mem_error\@
		add.l		#$9625EF51, d0
		cmpa.l		a1, a3					;	a3 ? a1
		blo			.verify_loop\@

		moveq.l		#1, d0
		bra			.end\@

.mem_error\@:
		moveq.l		#0, d0
.end\@:
	endm


; ================================================================ Mem_Check - merge memory regions and extend them by probing ================================================================
;
;	[ Macro, because we will most likely overwrite the stack]
;
; internal:
;
;		a2		- currently processed memory range
;		a3		- other memory range
;
;		d2.w	- current->type
;		a4		- current->start
;		a5		- current->end
;		a6		- RAM_Info_end
;	
;	
Mem_Check:		macro
		lea.l		RAM_Info(pc), a2
		lea.l		RAM_Info_end(pc), a6

.main_loop:
		move.l		6(a2), d0							;	current->end == NULL	-> skip
		beq			.main_next

		move.l		d0, a5								;	a5:	current->end
		move.w		(a2), d2							;	d2: current->type
		move.l		2(a2), a4							;	a4: current->start


		; ================ Include compatible overlapping ranges ================
.include_restart:
		lea.l		RAM_Info(pc), a3
.include_loop:
		move.l		6(a3), d0							;	if( other->end == NULL )
		beq			.include_next						;		continue;
		cmpa.l		a3, a2								;	if( current == other )
		beq			.include_next						;		continue;
		cmp.w		(a3), d2							;	if( current->type != other->type )					// compatible types?
		bne			.include_next						;		continue;
		cmp.l		2(a3), a5							;	if( current->end < other->start )
		blo			.include_next						;		continue;
		cmp.l		6(a3), a4							;	if( current->start > other->end )
		bhi			.include_next						;		continue;

														;	// ranges DO overlap
		cmp.l		2(a3), a4							;	if( current->start < other->start )
		blo			.no_extend_start					;	else		// if < then nothing to extend
		move.l		2(a3), a4							;		current->start = other->start,
		move.l		a5, 2(a3)							;		other->start = current->end;
.no_extend_start:

		cmp.l		6(a3), a5							;	if( current->end > other->end )
		bhi			.no_extend_end						;	else		// if > then nothing to extend
		move.l		6(a3), a5							;		current->end = other->end,
		move.l		a4, 6(a3)							;		other->end = current->start;
.no_extend_end:

		move.l		2(a3), a0
		cmpa.l		6(a3), a0							;	if( other->start < other->end )
		blo			.other_survived						;	else		// if < then range survived
		clr.l		6(a3)								;		other->end = NULL,
		bra			.include_restart					;		restart loop;
.other_survived:


.include_next:
		adda.w		#10, a3
		cmpa.l		a6, a3								; a3 ? end
		blo			.include_loop
		
		
		move.l		a4, d0								;	try to extend down to 64k page boundary
		move.w		#0, d0
		cmpa.l		d0, a4
		beq			.no_page_start_extension
		move.l		d0, a0
		move.l		a4, a1
		Mem_Probe
		tst.w		d0
		beq			.no_page_start_extension
		move.l		a0, a4
		bra			.include_restart					;	restart merge loop
.no_page_start_extension:

		move.l		a5, d0								;	try to extend up to 64k page boundary
		add.l		#$FFFF, d0
		move.w		#0, d0
		cmpa.l		d0, a5
		beq			.no_page_end_extension
		move.l		a5, a0
		move.l		d0, a1
		Mem_Probe
		tst.w		d0
		beq			.no_page_end_extension
		move.l		a1, a5
		bra			.include_restart					;	restart merge loop
.no_page_end_extension:


		move.l		a4, 2(a2)							; write back
		move.l		a5, 6(a2)


.main_next:		
		adda.l		#10, a2
		cmpa.l		a6, a2								; a2 ? end
		blo			.main_loop

	endm



; ================================================================ Mem_Postprocess - cleanup the memory report ================================================================
;
;		a2		- currently processed memory range
;		a3		- write target memory range
;		a6		- RAM_Info_end
;
Mem_Postprocess:
		lea.l		RAM_Info(pc), a2
		lea.l		RAM_Info(pc), a3
		lea.l		RAM_Info_end(pc), a6

.copy_loop:
		move.l		6(a2), d0
		beq			.skip_region

		move.w		(a2), d0				; type
		move.l		2(a2), d1				; start
		bne			.not_zero
		move.w		#$200, d1								;	skip 68k vector table and part of user vector table
.not_zero:
		move.l		6(a2), d2				; end

		cmp.w		#$0004, d0
		bne			.do_copy				; not a FAST ram
		cmp.l		#$C00000, d1
		bne			.do_copy				; not starting at $C00000
		cmp.l		#$DC0000, d2
		bhi			.do_copy				; larger than 1.8M

		moveq.l		#$0000, d0

.do_copy:
		move.w		d0, (a3)+
		move.l		d1, (a3)+
		move.l		d2, (a3)+

.skip_region:
		adda.l		#10, a2
		cmpa.l		a6, a2								; a2 ? end
		blo			.copy_loop


.clear_loop:
		clr.w		(a3)+
		clr.l		(a3)+
		clr.l		(a3)+
		cmpa.l		a6, a3								; a3 ? end
		blo			.clear_loop

		rts



; ================================================================ Mem_Alloc - get some memory ================================================================
;
;	All registers are saved, except d0 which contains the result
;
; Registers:
;	d0.l		OUT		Address of target memory
;	d1.l		IN		Size of memory
;	d2.l		IN		Alloc from top (hi word)	| Type of memory (low word)
;	d3					(temp - size)
;
;	a2					(RAM info pointer)
;	a3					(RAM info end)
;
;
;
Mem_Alloc:
		movem.l		d3/a2-a3, -(a7)

		lea.l		RAM_Info(pc), a2					;	a2: ram info pointer
		lea.l		RAM_Info_end(pc), a3				;	a3: ram info pointer

.search_loop:
		cmp.w		(a2), d2							;	check RAM type
		bne			.next
		move.l		6(a2), d3
		sub.l		2(a2), d3
		cmp.l		d1, d3								;	RAM size ? required size
		blo			.next								;		if < then skip

		swap.w		d2									;	Allocate from region top?
		tst.w		d2
		bne			.alloc_top

.alloc_bottom:
		move.l		2(a2), d0
		add.l		d1, 2(a2)
		bra			.end

.alloc_top:
		sub.l		d1, 6(a2)
		move.l		6(a2), d0
		bra			.end


.next:
		adda.l		#10, a2
		cmpa.l		a3, a2								; a2 ? end
		blo			.search_loop

		clr.l		d0									; not found

.end:
		movem.l		(a7)+, d3/a2-a3
		rts

		
; ================================================================ Mem_AllocChip ================================================================
;
; Registers:
;	d0			OUT		Address of target memory
;	d1.l		IN		Size of memory				(saved)
;	d2.l		USED	(internal)
;
;
Mem_AllocChip:
		move.l		#$00000002, d2
		bsr			Mem_Alloc
		rts


; ================================================================ Mem_AllocChipTop ================================================================
;
; Registers:
;	d0			OUT		Address of target memory
;	d1.l		IN		Size of memory				(saved)
;	d2.l		USED	(internal)
;
;
Mem_AllocChipTop:
		move.l		#$00010002, d2
		bsr			Mem_Alloc
		rts

; ================================================================ Mem_AllocFast / Mem_AllocFastTop ================================================================
;
; Registers:
;	d0			OUT		Address of target memory
;	d1.l		IN		Size of memory				(saved)
;	d2.l		USED	(internal)
;
;
Mem_AllocFast:
		moveq.l		#$00000000, d2			; alloc from bottom
		bra			Mem_AllocFastAny

Mem_AllocFastTop:
		move.l		#$00010000, d2			; alloc from top

Mem_AllocFastAny:
		move.w		#$0004, d2				; try Fast
		bsr			Mem_Alloc
		tst.l		d0
		bne			.end

		move.w		#$0000, d2				; try Slow
		bsr			Mem_Alloc
		tst.l		d0
		bne			.end

		move.w		#$0002, d2				; try Chip
		bsr			Mem_Alloc
.end:
		rts



; ================================================================ Mem_Free - free memory, if possible ================================================================
;
; Registers:
;	d0			IN		Freed memory size
;	a0			IN		Freed memory address
;
;	a1			USED
;	d1			USED
;
;
Mem_Free:
		lea.l		RAM_Info(pc), a1				;	a1: ram info pointer
		moveq.l		#NUM_RAM_ENTRIES-1, d1			;	d1: ram entry counter
.loop:
		cmpa.l		6(a1), a0						;	if( mem->end == ptr )
		bne			.next							;	{
		add.l		d0, 6(a1)						;		mem->end += size;
		bra			.end							;		return;
.next:												;	}
		adda.w		#10, a1
		dbra.w		d1, .loop
.end:
		rts




; ================================================================ Mem_Paint ================================================================
;
; Registers:
;	d0			IN		Address of target memory	(saved, NULL is safe)
;	d1.l		IN		Size of memory				(saved)
;	d2.w		IN		Paint value					(saved)
;
;	a0			USED
;	d7			USED
;
;
Mem_Paint_Macro:	macro
		tst.l		d0
		beq			.end\@
		move.l		d0, a0
		
		move.l		d1, d7
		lsr.l		#1, d7
		sub.l		#1, d7
		swap.w		d7
.loop_hi\@:
		swap.w		d7
.loop\@:
		move.w		d2, (a0)+
		dbra.w		d7, .loop\@
		swap.w		d7
		dbra.w		d7, .loop_hi\@

.end\@:
	endm


Mem_Paint:
	Mem_Paint_Macro
	rts



; ================================================================ Mem_CRC ================================================================
;
; Registers:
;	a0			IN		Memory address		(must be word-aligned)
;	d0			IN		Memory size			(must be dword-aligned, greater than 0)
;
;	d0			OUT		Computed CRC
;
;		All registers saved (except result in <d0>)
;
Mem_CRC:
		movem.l		d1-d2/a0, -(a7)
		move.l		d0, d1
		lsr.l		#2, d1
		subq.l		#1, d1

		moveq.l		#0, d0
		lsl.l		d0						; clear X

		swap.w		d1
.loop_hi:
		swap.w		d1
.loop:
		move.l		(a0)+, d2
		roxl.l		d0
		addx.l		d2, d0
		dbra.w		d1, .loop
		swap.w		d1
		dbra.w		d1, .loop_hi

		moveq.l		#0, d2
		addx.l		d2, d0

.end:
		movem.l		(a7)+, d1-d2/a0
		rts

; ================================================================ Initial memory allocation routine ================================================================
;
;	[ Macro, because we will change the stack pointer]
;
;	Allocate:
;		- supervisor stack			(destroys trap #0)
;		- user stack
;		- 
;
;
Mem_AllocateCoreRegions:	macro

		lea.l		Memdef_Region(pc), a6

		move.l		8(a6), d1									; Allocate supervisor stack			[a2]
		bsr			Mem_AllocFastTop
		move.l		d0, a2
		add.l		d1, a2
		move.w		#STACK_PAINT_VALUE, d2
					Mem_Paint_Macro
		
		move.l		12(a6), d1									; Allocate user stack				[a3]
		bsr			Mem_AllocFastTop
		move.l		d0, a3
		add.l		d1, a3
		move.w		#STACK_PAINT_VALUE, d2
					Mem_Paint_Macro

		move.l		#ProgEnd - ProgStart, d1					; Allocate space for this image		[a4]
		bsr			Mem_AllocChipTop
		move.l		d0, a4


		move.l		a2, d0
		beq			.error
		move.l		a3, d0
		beq			.error
		move.l		a4, d0
		beq			.error


		sub.l		a0, a0								; Init trap #0
		lea.l		.supervisor_return(pc), a1
		move.l		a1, IV_TRAP_0(a0)

        and.w		#$DFFF, sr							; User mode
		move.l		a3, a7

		trap		#0									; Supervisor mode
.supervisor_return:
		move.l		a2, a7


		move.l		#(ProgEnd - ProgStart)/32-1, d0				; Relocate bootloader
		lea.l		ProgStart(pc), a0
		move.l		a4, a1
		swap.w		d0
.copy_loop_hi:
		swap.w		d0
.copy_loop:
		movem.l		(a0)+, d1-d7/a2
		movem.l		d1-d7/a2, (a1)
		adda.w		#32, a1
		dbra.w		d0, .copy_loop
		swap.w		d0
		dbra.w		d0, .copy_loop_hi

		jmp			.relocate_jump-ProgStart(a4)
.relocate_jump:



		move.l		#TRACK_BUFFER_SIZE, d1						; Allocate track buffer
		bsr			Mem_AllocChipTop
		lea.l		Track_Vars(pc), a0
		move.l		d0, trk_buffer_address(a0)

		lea.l		Section_Pointers(pc), a0					; stored as section #15
		move.l		d0, 15*4(a0)

  if SHOW_TRACK_BITSTREAM
;		move.l		d0, a0
;		lea.l		LocalImg_copper_trkbits(pc), a0
;		move.w		d0, 2(a0)
;		swap.w		d0
;		move.w		d0, 6(a0)
  endif


		move.l		#TRACK_DECODED_SIZE, d1						; Allocate track decode buffer
		bsr			Mem_AllocFastTop
		lea.l		Track_Vars(pc), a0
		move.l		d0, trk_decoded_address(a0)

		lea.l		Section_Pointers(pc), a0					; stored as section #14
		move.l		d0, 14*4(a0)


.error:
		

	endm



; ================================================================ Finalize memory - free regions used by bootloader & copy memory info to $100 ================================================================
;
Mem_FinalPreparation:

		lea.l		.str(pc), a0
		move.l		Track_Vars+trk_decoded_address(pc), d1
		move.l		Track_Vars+trk_buffer_address(pc), d2
		lea.l		ProgStart(pc), a1
		move.l		a1, d3
		bsr			Print_String

		move.l		Track_Vars+trk_decoded_address(pc), a0		; Free decoded track buffer
		move.l		#TRACK_DECODED_SIZE, d0
		bsr			Mem_Free

		move.l		Track_Vars+trk_buffer_address(pc), a0		; Free track buffer
		move.l		#TRACK_BUFFER_SIZE, d0
		bsr			Mem_Free

		lea.l		ProgStart(pc), a0							; Free bootloader memory
		move.l		#ProgEnd - ProgStart, d0
		bsr			Mem_Free

																; ================================ Copy memory table ================================

		lea.l		RAM_Info(pc), a4							;	a4: ram info pointer
		lea.l		$100.w, a5									;	a5: target
		moveq.l		#NUM_RAM_ENTRIES-1, d5						;	d5: ram entry counter
.copy_loop:
		move.w		(a4)+, (a5)+
		move.l		(a4)+, (a5)+
		move.l		(a4)+, (a5)+
		dbra.w		d5, .copy_loop
		
		rts


.str	dc.b		"\nFREEING $\x01 $\x02 $\x03", 0
		even


; ================================================================ Memory report routine ================================================================
;
Mem_Print:
		lea.l		RAM_Info(pc), a4				;	a4: ram info pointer
		moveq.l		#0, d5							;	d5: ram entry index

.loop:
		move.l		6(a4), d0
		beq			.skip_row

		move.w		(a4), d0						;	READ MEMORY TYPE

		lea.l		str_Mem_Chip(pc), a0
		cmp.w		#$0002, d0
		beq			.print_mem_type

		lea.l		str_Mem_Fast(pc), a0
		cmp.w		#$0004, d0
		beq			.print_mem_type

		lea.l		str_Mem_Slow(pc), a0

.print_mem_type:

		move.l		2(a4), d1						;	READ MEMORY START
		move.l		6(a4), d2						;	READ MEMORY END
		subq.l		#1, d2
		move.l		6(a4), d3						;	COMPUTE MEMORY SIZE
		sub.l		2(a4), d3

		bsr			Print_String

.skip_row:
		adda.w		#10, a4

.cont:
		addq.l		#1, d5
		cmp.b		#NUM_RAM_ENTRIES, d5
		bhs			.end
		cmp.b		#CONSOLE_ROWS, d5
		blo			.loop
		
.end:
		rts






; ================================================================ RAM info buffer ================================================================

RAM_Info:
		dcb.b		NUM_RAM_ENTRIES*(2+4+4)		; array of: { type.w, start.l, end.l }				<end> is the first byte NOT in the memory range
												; Memory types:
												;	$0000		- Slow RAM
												;	$0002		- Chip RAM			(MEMF_CHIP)
												;	$0004		- Fast RAM			(MEMF_FAST)

RAM_Info_end:



; ================================================================ Text strings ================================================================

str_Mem_Slow:	dc.b		"\nSLOW RAM AT $\x01 .. $\x02  SIZE $\x03", 0
str_Mem_Chip:	dc.b		"\nCHIP RAM AT $\x01 .. $\x02  SIZE $\x03", 0
str_Mem_Fast:	dc.b		"\nFAST RAM AT $\x01 .. $\x02  SIZE $\x03", 0
str_Mem_To:		dc.b		" .. $", 0
str_Mem_Size:	dc.b		"  SIZE $", 0

				even
