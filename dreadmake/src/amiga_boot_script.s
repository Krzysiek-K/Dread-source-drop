

;	Boot loader commands:

BCMD_END				=  0		;													- End	(enter infinite loop)

BCMD_ALLOC_CHIP			=  1		;	<index> <size><>								- Allocate section <index> as CHIP memory from the bottom
BCMD_ALLOC_CHIP_TOP		=  2		;	<index> <size><>								- Allocate section <index> as CHIP memory from the top
BCMD_ALLOC_FAST			=  3		;	<index> <size><>								- Allocate section <index> as FAST (fallback to CHIP if not possible) memory from the bottom
BCMD_ALLOC_FAST_TOP		=  4		;	<index> <size><>								- Allocate section <index> as FAST (fallback to CHIP if not possible) memory from the top

BCMD_TRACK_LOAD			=  5		;	<trackhead>										- Load track data from FDD to track buffer											[TBD: use double buffering]
BCMD_TRACK_WAIT			=  6		;													- Wait for trackloading completion
BCMD_TRACK_DECODE		=  7		;	<index> <offset><> <startsec> <numsec>			- Decode data from track buffer to section <index> at offset <offset>				[TBD: use double buffering]
BCMD_DEFLATE			=  8		;	<index> <dstoffs><> <srcoffs><>					- DEFLATE data in section <index>
BCMD_VERIFY_CRC			=  9		;	<index> <offset><> <size><> <crc><>				- Verify CRC of data in section <index>
BCMD_FILL				= 10		;	<index> <offset><> <size><> <value>				- Fill data in section <index>
BCMD_RELOC32			= 11		;	<index> <target> <buffer> <offset><> <count>	- Add base address of section <target> to data of section <index>
									;														<count> offsets are specified in section <buffer> at <offset>
BCMD_FINALIZE			= 12		;													- Free bootloader memory & initialize game memory system
BCMD_START				= 13		;	<index> <offset><>								- Set start address to specified location in section <index>

BCMD_PRINT_NEWLINE		= 14		;
BCMD_PRINT_STRING		= 15		;	<string>...										- Print string on console
BCMD_PRINT_ADDR			= 16		;	<index>											- Print base address of section <index> on console
BCMD_PRINT_DATA_DWORD	= 17		;	<index> <offset><>								- Print dword of data from section <index> from offset <offset><>
BCMD_PRINT_DATA_STRING	= 18		;	<index> <offset><>								- Print string from section <index> from offset <offset><>
BCMD_PRINT_MEM_STATS	= 19		;													- Print available memory regions
BCMD_PRINT_SECTION_CRC	= 20		;	<index>	<index> <size><>						- Print computed CRC of a section <index>

BCMD_PROGRESS			= 21		;	<width>											- Update progress bar position to <width> pixels
BCMD_WAIT				= 22		;	<time>											- Wait <time> vsyncs


BCMD_MACRO_TOTAL_SECTORS = 23		;	<total_sectors>
BCMD_MACRO_TRACK_LOAD	= 24		;	<index> <offset><> <flags> <trackhead> <startsec> <numsec>	<CRC><>
;																									- Make sure <trackhead> track is in the buffer
;																									- Wait if a request is in progress
;																									- Check if <trackhead> matches the loaded buffer
;																										- if NOT: request loading <trackhead> and wait until finish
;																									- Decode <startsec>/<numsec> to destination or temp		(bootloader should allocate 11*512 bytes for temp)
;																										- if ERROR: print the error, invalidate track buffer and start again
;																									- [optional] Verify decoded sectors CRC
;																									- [optional] Request loading <trackhead>+1
;																									- Add <numsec> to total loaded sector count
;																									- Update progressbar to position 111*<total_loaded_sectors>/<total_sectors>	(if pixel width differs from the current one)
;																									- [optional] Decompress (DEFLATE) the buffer to <index>/<offset> section location
;
;																									Flags:
;																									 0	- compressed		0: decode sectors directly to destination		1: decode sectors to temp and deflate
;																									 1	- CRC				0: no CRC check									1: verify CRC after decoding
;																									 2	- queue track		0: -											1: queue loading next track/head immediately
;





; ================================================================ Boot commands table ================================================================
BootCmd_Table:
		dc.w		0													;	BCMD_END				=  0		;					
		dc.w		BootCmd_AllocChip			- BootCmd_Table			;	BCMD_ALLOC_CHIP			=  1		;	<index> <size><>
		dc.w		BootCmd_AllocChip_Top		- BootCmd_Table			;	BCMD_ALLOC_CHIP_TOP		=  2		;	<index> <size><>
		dc.w		BootCmd_AllocFast			- BootCmd_Table			;	BCMD_ALLOC_FAST			=  3		;	<index> <size><>
		dc.w		BootCmd_AllocFast_Top		- BootCmd_Table			;	BCMD_ALLOC_FAST_TOP		=  4		;	<index> <size><>
		dc.w		BootCmd_Track_Load			- BootCmd_Table			;	BCMD_TRACK_LOAD			=  5		;	<trackhead>										
		dc.w		BootCmd_Track_Wait			- BootCmd_Table			;	BCMD_TRACK_WAIT			=  6		;													
		dc.w		BootCmd_Track_Decode		- BootCmd_Table			;	BCMD_TRACK_DECODE		=  7		;	<index> <offset><> <startsec> <numsec>			
		dc.w		0													;	BCMD_DEFLATE			=  8		;	<index> <dstoffs><> <srcoffs><>					
		dc.w		0													;	BCMD_VERIFY_CRC			=  9		;	<index> <offset><> <size><> <crc><>				
		dc.w		BootCmd_Fill				- BootCmd_Table			;	BCMD_FILL				= 10		;	<index> <offset><> <size><> <value>
		dc.w		BootCmd_Reloc32				- BootCmd_Table			;	BCMD_RELOC32			= 11		;	<index> <target> <buffer> <offset><> <count>	
		dc.w		Mem_FinalPreparation		- BootCmd_Table			;	BCMD_FINALIZE			= 12		;
		dc.w		BootCmd_Start				- BootCmd_Table			;	BCMD_START				= 13		;	<index> <offset><>								
		dc.w		Print_Newline				- BootCmd_Table			;	BCMD_PRINT_NEWLINE		= 14		;
		dc.w		BootCmd_Print_String		- BootCmd_Table			;	BCMD_PRINT_STRING		= 15		;	<string>...										
		dc.w		BootCmd_Print_Addr			- BootCmd_Table			;	BCMD_PRINT_ADDR			= 16		;	<index>											
		dc.w		BootCmd_Print_Data_Dword	- BootCmd_Table			;	BCMD_PRINT_DATA_DWORD	= 17		;	<index> <offset><>								
		dc.w		0													;	BCMD_PRINT_DATA_STRING	= 18		;	<index> <offset><>								
		dc.w		Mem_Print					- BootCmd_Table			;	BCMD_PRINT_MEM_STATS	= 19		;
		dc.w		BootCmd_Print_CRC			- BootCmd_Table			;	BCMD_PRINT_SECTION_CRC	= 20		;	<index>
		dc.w		BootCmd_Progress			- BootCmd_Table			;	BCMD_PROGRESS			= 21		;	<width>											
		dc.w		BootCmd_Wait				- BootCmd_Table			;	BCMD_WAIT				= 22		;	<time>											
		dc.w		BootCmd_Macro_Init			- BootCmd_Table			;	BCMD_MACRO_TOTAL_SECTORS = 23		;	<total_sectors>
		dc.w		BootCmd_Macro_Track_Load	- BootCmd_Table			;	BCMD_MACRO_TRACK_LOAD	= 24		;	<index> <offset><> <flags> <trackhead> <startsec> <numsec> <CRC><>


BootCmd_Table_End:



; ================================================ Test script ================================================
Test_Script:
	if 1

		dc.b			">>>BOOTSCRIPT START<<<"
		even

		dcb.b			4300
		
		dc.b			">>>BOOTSCRIPT END<<<"
		even

;		dc.w			BCMD_PRINT_NEWLINE
;		dc.w			BCMD_PRINT_STRING
;		dc.b			">>> LOADING <<<",0
;		even
;		include "gen/dread_loadscript.inc"

	else

		dc.w			BCMD_PRINT_STRING
		dc.b			"\nPRINT \x01\nTEST \x02\nHELLO WORLD \x03",0
		even

;		dc.w			BCMD_ALLOC_CHIP, 0				;	Allocate Chip
;		dc.l			$40000
;
;		dc.w			BCMD_PRINT_NEWLINE				;	Print
;		dc.w			BCMD_PRINT_STRING
;		dc.b			"LOADING TH 10",0
;		even
;
;		dc.w			BCMD_MACRO_TRACK_LOAD, 0		;	<index> <offset><> <flags> <trackhead> <startsec> <numsec>	<CRC><>
;		dc.l			0
;		dc.w			$0000, 10, 0, 11
;		dc.l			0
;
;
;		dc.w			BCMD_PRINT_NEWLINE				;	Print
;		dc.w			BCMD_PRINT_STRING
;		dc.b			"LOADING TH 11",0
;		even
;
;		dc.w			BCMD_MACRO_TRACK_LOAD, 0		;	<index> <offset><> <flags> <trackhead> <startsec> <numsec>	<CRC><>
;		dc.l			0
;		dc.w			$0000, 11, 0, 11
;		dc.l			0
;
;		dc.w			BCMD_PRINT_NEWLINE				;	Print
;		dc.w			BCMD_PRINT_STRING
;		dc.b			"DONE",0
;		even

		dc.w			BCMD_END
	endif

		

; ================================================ Globals ================================================
;
Section_Pointers:		dcb.l			16

BootError:				dc.w			0
Boot_StartAddr:			dc.l			0
Boot_TrackScratch:		dc.l			0				; 11*512 bytes of RAM (any type) to decode sectors before decompression



; ================================================================ Boot script ================================================================
;
;	a6		- script code pointer
;
Boot_Script_Execute:
		lea.l		Test_Script(pc), a6
.loop:
		btst.b		#6, CIAA_PRA
		beq			.loop
		move.w		(a6)+, d0
		beq			.end
		move.w		d0, d1
		add.w		d1, d1
		cmp.w		#BootCmd_Table_End-BootCmd_Table, d1
		bhs			.error

		lea.l		BootCmd_Table(pc), a0
		move.w		(a0,d1.w), d1
		beq			.error

		lea.l		(a0,d1.w), a0
		jsr			(a0)

		move.w		BootError(pc), d0
		beq			.loop

.end:
		rts

.error:
		bsr			Print_Newline
		lea.l		.str_error(pc), a0
		bsr			Print_String
		bsr			Print_Hex_4
		lea.l		.str_at(pc), a0
		bsr			Print_String

		move.l		a6, d0
		lea.l		Test_Script(pc), a0
		sub.l		a0, d0
		bsr			Print_Hex_6
		rts

.str_error		dc.b		"INVALID BOOT SCRIPT COMMAND $", 0
.str_at			dc.b		" AT $", 0
		even





; ================================================ BootCmd_Alloc* ================================================
;
;	<index> <size><>
;
BootCmd_AllocChip:
		lea.l		str_memerr2a(pc), a4
		lea.l		Mem_AllocChip(pc), a5
		bra			BootCmd_AllocAny

BootCmd_AllocChip_Top:
		lea.l		str_memerr2b(pc), a4
		lea.l		Mem_AllocChipTop(pc), a5
		bra			BootCmd_AllocAny

BootCmd_AllocFast:
		lea.l		str_memerr2c(pc), a4
		lea.l		Mem_AllocFast(pc), a5
		bra			BootCmd_AllocAny

BootCmd_AllocFast_Top:
		lea.l		str_memerr2d(pc), a4
		lea.l		Mem_AllocFastTop(pc), a5
		bra			BootCmd_AllocAny



; ================================================ BootCmd_AllocAny ================================================
;
;	a4		- memory type error string pointer
;	a5		- allocation function pointer (Mem_Alloc*)
;
BootCmd_AllocAny:
		move.w		(a6)+, d3
		move.l		(a6)+, d1
		jsr			(a5)
		tst.l		d0
		beq			.error

		lea.l		Section_Pointers(pc), a0
		lsl.w		#2, d3
		move.l		d0, (a0,d3)

										; At this point:
										;	d0.l		- memory address
										;	d1.l		- memory size
;			move.l		d0, a0
;			lea.l		(a0,d1.l), a1
;			move.l		#$5A5A5A5A, d2
;			move.l		#$A5A5A5A5, d3
;			move.l		#0, d4
;	
;	.memtest:
;			move.l		d2, (a0)
;			cmp.l		(a0), d2
;			bne			.memerror
;	
;			move.l		d3, (a0)
;			cmp.l		(a0), d3
;			bne			.memerror
;	
;	;		move.l		d4, (a0)
;	;		cmp.l		(a0), d4
;	;		bne			.memerror
;	
;			addq.l		#4, a0
;			cmp.l		a1, a0
;			blt			.memtest

		rts

.error:
		bsr			Print_Newline
		lea.l		str_memerr1(pc), a0		; CANT ALLOC $
		bsr			Print_String
		move.l		d1, d0					; xxxxxx
		bsr			Print_Hex_4
		move.l		a4, a0					; TOP CHIP
		bsr			Print_String
		lea.l		str_memerr3(pc), a0		; BYTES FOR SEC $
		bsr			Print_String
		moveq.l		#0, d0
		move.w		d3, d0					; xx
		bsr			Print_Hex_2

		lea.l		BootError(pc), a0
		move.w		#1, (a0)
		rts

.memerror:
		move.l		a0, d0
		bsr			Print_Newline
		lea.l		str_testerr1(pc), a0	; MEMORY ERROR AT $
		bsr			Print_String
		bsr			Print_Hex_6				; xxxxxx

		lea.l		BootError(pc), a0
		move.w		#1, (a0)
		rts


str_memerr1		dc.b		"CANT ALLOC $", 0
str_memerr2a	dc.b		" CHIP", 0
str_memerr2b	dc.b		" TOP CHIP", 0
str_memerr2c	dc.b		" FAST", 0
str_memerr2d	dc.b		" TOP FAST", 0
str_memerr3		dc.b		" BYTES FOR SEC $", 0
str_testerr1	dc.b		"MEMORY ERROR AT $", 0
				even


; ================================================ BootCmd_Track_Load ================================================
;		
;	<trackhead>										
;
BootCmd_Track_Load:
		lea.l		Track_Vars(pc), a1
.loop:
		move.w		trk_req_trackhead(a1), d0								; wait for the previous request to end
		bpl			.loop

		move.l		trk_buffer_address(a1), trk_req_destination(a1)			; destination pointer
		move.w		(a6)+, trk_req_trackhead(a1)							; requested track/head

		rts


; ================================================ BootCmd_Track_Wait ================================================
;
;
;								
BootCmd_Track_Wait:
		lea.l		Track_Vars(pc), a1
.loop:
		move.w		trk_req_trackhead(a1), d0								; wait for the previous request to end
		bpl			.loop
		rts


; ================================================ BootCmd_Track_Decode ================================================
;
;	<index> <offset><> <startsec> <numsec>			
;
BootCmd_Track_Decode:
		lea.l		Track_Vars(pc), a1

													;	__d0 word Track_Decode(__a1 void *track_data, __d0 word start_sec, __d1 word num_sec, __a0 void *target_buffer);

		move.w		(a6)+, d0						;		a0:		target_buffer = sections[index] + offset
		lsl.w		#2, d0
		lea.l		Section_Pointers(pc), a0
		move.l		(a0,d0.w), a0
		adda.l		(a6)+, a0

		move.w		(a6)+, d0						;		d0:		start_sec
		move.w		(a6)+, d1						;		d1:		num_sec

		move.l		trk_buffer_address(a1), a1		;		a1:		track_data

		bsr			Track_Decode
		cmp.w		#0, d0
		bne			.error

		rts

.error:
		lea.l		.str_error(pc), a0
		moveq.l		#0, d1
		move.w		trk_loaded_trackhead(a1), d1
		bsr			Print_String
.lock:
		bra			.lock
		rts

.str_error		dc.b	"\nTRACK DECODE ERROR \x01", 0
		even


; ================================================ BootCmd_Fill ================================================
;
;	<index> <offset><> <size><> <value>
;
BootCmd_Fill:
		move.w		(a6)+, d0						;	d0			IN		Address of target memory	(saved, NULL is safe)
		lsl.w		#2, d0							;							src = sections[index] + offset
		lea.l		Section_Pointers(pc), a0
		move.l		(a0,d0.w), d0
		add.l		(a6)+, d0
		
		move.l		(a6)+, d1						;	d1.l		IN		Size of memory				(saved)
		move.w		(a6)+, d2						;	d2.w		IN		Paint value					(saved)

		bsr			Mem_Paint

		rts


; ================================================ BootCmd_Reloc32 ================================================
;
;	<index> <target> <buffer> <offset><> <count>
;
BootCmd_Reloc32:
		lea.l		Section_Pointers(pc), a0		;	a0: section table

		move.w		(a6)+, d0						;	a1:	dst = sections[index]
		lsl.w		#2, d0
		move.l		(a0,d0.w), a1

		move.w		(a6)+, d0						;	d1:	base = sections[target]
		lsl.w		#2, d0
		move.l		(a0,d0.w), d1

		move.w		(a6)+, d0						;	a2:	offset_table = sections[target] + offs
		lsl.w		#2, d0
		move.l		(a0,d0.w), a2
		adda.l		(a6)+, a2

		move.w		(a6)+, d2						;	d2: count
		subq.w		#1, d2
.loop:
		move.l		(a2)+, d0
		add.l		d1, (a1,d0.l)
		dbra.w		d2, .loop

		rts


; ================================================ BootCmd_Start ================================================
;
;	<index> <offset><>
;
BootCmd_Start:
		move.w		(a6)+, d0						;	src = sections[index] + offset
		lsl.w		#2, d0
		lea.l		Section_Pointers(pc), a0
		move.l		(a0,d0.w), d0
		add.l		(a6)+, d0

		lea.l		Boot_StartAddr(pc), a0
		move.l		d0, (a0)

		rts


; ================================================ BootCmd_Print_String ================================================
;
;	<string>...
;
BootCmd_Print_String:
		move.l		a6, a0
		bsr			Print_String
		
		move.l		a0, d0				; a6 = a0 aligned to next word
		addq.l		#1, d0
		and.b		#$FE, d0
		move.l		d0, a6
		rts


; ================================================ BootCmd_Print_Addr ================================================
;
;	<index>
;
BootCmd_Print_Addr:
		move.w		(a6)+, d0
		lsl.w		#2, d0
		lea.l		Section_Pointers(pc), a0
		move.l		(a0,d0.w), d0
		bsr			Print_Hex_6
		rts


; ================================================ BootCmd_Print_Data_Dword ================================================
;
;	<index> <offset><>								
;
BootCmd_Print_Data_Dword:
		move.w		(a6)+, d0						;		a0:		src = sections[index] + offset
		lsl.w		#2, d0
		lea.l		Section_Pointers(pc), a0
		move.l		(a0,d0.w), a0
		adda.l		(a6)+, a0

		move.l		(a0), d0
		bsr			Print_Hex_8

		rts


; ================================================ BootCmd_Print_CRC ================================================
;
;	<index> <size><>
;
BootCmd_Print_CRC:
		move.w		(a6)+, d0						;		a0:		src = sections[index]
		lsl.w		#2, d0
		lea.l		Section_Pointers(pc), a0
		move.l		(a0,d0.w), a0

;		moveq.l		#0, d0
;		lsl.l		d0						; clear X
;
;		move.l		(a6)+, d1
;		lsr.l		#2, d1
;		subq.l		#1, d1
;
;		swap.w		d1
;.loop_hi:
;		swap.w		d1
;.loop:
;		move.l		(a0)+, d2
;		roxl.l		d0
;		addx.l		d2, d0
;		dbra.w		d1, .loop
;		swap.w		d1
;		dbra.w		d1, .loop_hi
;
;		moveq.l		#0, d2
;		addx.l		d2, d0


		move.l		(a6)+, d0
		bsr			Mem_CRC
		bsr			Print_Hex_8

		rts
		

; ================================================ BootCmd_Progress ================================================
;
;	<width>
;
;
;	110x3+105+116
;
;	Empty:		 3 10 10		0011 1010 1010
;	Full:		15 14 14		1111 1110 1110
;								++11 1+10 1+10
;
;
;	d0 d1 d2 d3 d4 d5 d6 d7		a0 a1 a2 a3 a4 a5 a6
;	++ ++ ++ .. .. .. .. IN		++ ++ .. .. .. .. IN
;
BootCmd_Progress:
		move.w		(a6)+, d7
BootCmd_Progress_Arg:
		add.w		#9, d7
		lea.l		LoadImg_data+12+116*160(pc), a0
		lea.l		.mask(pc), a1

.loop:
		move.w		(a1)+, d1
		beq			.end
		move.w		d1, d0
		not.w		d0

		cmp.w		#16, d7					; d7 ? 16
		bge			.draw					;	>	-> draw full bar
		tst.w		d7
		bpl			.draw_partial
		moveq.l		#0, d1					;		-> draw empty
		bra			.draw

.draw_partial:
		move.l		#$FFFF0000, d2
		lsr.l		d7, d2
		and.w		d2, d1

.draw:
		; d0	- old pixels mask
		; d1	- new hot pixels mask
		;
		and.w		d0, 80(a0)				; 0/1
		or.w		d1, 80(a0)
		and.w		d0, 120(a0)				; 0/1
		or.w		d1, 120(a0)
		and.w		d0, 160+80(a0)			; 0/1
		or.w		d1, 160+80(a0)
		and.w		d0, 320+80(a0)			; 0/1
		or.w		d1, 320+80(a0)
		
		addq.l		#2, a0
		sub.w		#16, d7

;		move.w		#$FFFF, (a0)
;		move.w		#$FFFF, 40(a0)
;		move.w		#$FFFF, 80(a0)
;		move.w		#$FFFF, 120(a0)
;		move.w		#$0000, 160(a0)
;		move.w		#$FFFF, 160+40(a0)
;		move.w		#$FFFF, 160+80(a0)
;		move.w		#$FFFF, 160+120(a0)
;		move.w		#$0000, 320(a0)
;		move.w		#$FFFF, 320+40(a0)
;		move.w		#$FFFF, 320+80(a0)
;		move.w		#$FFFF, 320+120(a0)

		bra			.loop

.end:
		rts

.mask:
		dc.w		$007F, $FFFF, $FFFF, $FFFF, $FFFF, $FFFF, $FFFF, $FE00, 0

; ================================================ BootCmd_Wait ================================================
;
;	<time>
;
BootCmd_Wait:
		lea.l		scan_timer(pc), a0
		move.w		(a6)+, (a0)
.wait:
		move.w		(a0), d0
		bne			.wait
		rts


; ================================================ BootCmd_Macro_Init ================================================
;
;	<total_sectors>
;
BootCmd_Macro_Init:
		lea.l		Bootloader_Vars(pc), a5
		move.w		(a6)+, load_total_sectors(a5)
		rts


; ================================================ BootCmd_Macro_Track_Load ================================================
;
;	<index> <offset><> <flags> <trackhead> <startsec> <numsec> <CRC><>
;
;												[ Track_Decode ]
;	d0		(temp)								start_sec
;	d1											num_sec
;	d2
;	d3
;	d4
;	d5
;	d6
;	d7
;
;	a0		(temp)								target_buffer
;	a1											track_data
;	a2
;	a3
;	a4		
;	a5		Bootloader_Vars / Track_Vars
;	a6		VM code source
;
;
BootCmd_Macro_Track_Load:
		lea.l		Bootloader_Vars(pc), a5

		lea.l		load_index(a5), a0									; Load all arguments
		move.w		(a6)+, (a0)+
		move.l		(a6)+, (a0)+
		move.w		(a6)+, (a0)+
		move.w		(a6)+, (a0)+
		move.w		(a6)+, (a0)+
		move.w		(a6)+, (a0)+
		move.l		(a6)+, (a0)+

		move.w		load_index(a5), d0									; Compute final destination
		lsl.w		#2, d0
		lea.l		Section_Pointers(pc), a0
		move.l		(a0,d0.w), d0
		add.l		load_offset(a5), d0
		move.l		d0, load_final_dst(a5)

																		; ================ Make sure <trackhead> track is in the buffer ================
.trk_wait:																; -------- Wait if a request is in progress --------
		move.w		trk_req_trackhead(a5), d0
		bpl			.trk_wait

																		; -------- Check if <trackhead> matches the loaded buffer --------
		move.w		load_trackhead(a5), d0
		cmp.w		trk_loaded_trackhead(a5), d0
		beq			.trk_loaded


;		lea.l		.str_neq(pc), a0									; DEBUG
;		moveq.l		#0, d1												;
;		move.w		trk_loaded_trackhead(a5), d1						;
;		moveq.l		#0, d2												;
;		move.w		load_trackhead(a5), d2								;
;		bsr			Print_String										;
;		bsr			Print_Hex_4											;

		move.l		trk_buffer_address(a5), trk_req_destination(a5)		; destination pointer
		move.w		load_trackhead(a5), d0								;
		move.w		d0, trk_req_trackhead(a5)							;	if NOT: request loading <trackhead> and wait until finish
		bra			.trk_wait

.trk_loaded:
																		; ================ Decode <startsec>/<numsec> to destination or temp ================
																		; (bootloader should have allocated 11*512 bytes for temp)
																		;	__d0 word Track_Decode(__a1 void *track_data, __d0 word start_sec, __d1 word num_sec, __a0 void *target_buffer);
																
		move.l		load_final_dst(a5), a0								;		a0:		target_buffer
		btst.b		#0, load_flags+1(a5)								;			Flag 0 set -> write to temp buffer address
		beq			.decode_dst_set										;
		move.l		trk_decoded_address(a5), a0
.decode_dst_set:
		move.l		a0, load_decoded_track(a5)

		move.w		load_startsec(a5), d0								;		d0:		start_sec
		move.w		load_numsec(a5), d1									;		d1:		num_sec
		move.l		trk_buffer_address(a5), a1							;		a1:		track_data
		bsr			Track_Decode
		cmp.w		#0, d0
		beq			.trk_decoded										;	if ERROR: print the error, invalidate track buffer and start again

		lea.l		.str_decode_error(pc), a0							;		print the error
		moveq.l		#0, d1
		moveq.l		#0, d2
		move.w		trk_loaded_trackhead(a5), d1
		move.w		d0, d2
		bsr			Print_String

.trk_load_again:
		move.w		#$FFFF, trk_loaded_trackhead(a5)					;		invalidate track buffer
		bra			.trk_wait											;		start again

.trk_decoded:
		btst.b		#1, load_flags+1(a5)								;	- [optional] Verify decoded sectors CRC
		beq			.crc_check_done

		clr.l		d0													;	d0 = loaded track size
		move.w		load_numsec(a5), d0
		lsl.l		#7, d0
		lsl.l		#2, d0
		move.l		load_decoded_track(a5), a0							;	a0 = track data
		bsr			Mem_CRC												;	Compute CRC

		cmp.l		load_crc(a5), d0									;	Verify
		beq			.crc_check_done

		lea.l		.str_crc_error(pc), a0								;	Print CRC error
		moveq.l		#0, d1
		move.w		trk_loaded_trackhead(a5), d1
		move.l		d0, d2
		move.l		load_crc(a5), d3
		bsr			Print_String
		bra			.trk_load_again										; Retry

.crc_check_done:
		btst.b		#2, load_flags+1(a5)								;	- [optional] Request loading <trackhead>+1
		beq			.next_request_done
		move.w		load_trackhead(a5), d0
		addq.l		#1, d0
		move.w		d0, trk_req_trackhead(a5)

.next_request_done:

		move.w		load_numsec(a5), d0									;	- Add <numsec> to total loaded sector count
		add.w		d0, load_loaded_sectors(a5)

		move.w		#PROGRESS_BAR_WIDTH, d7								;	- Update progressbar to position 111*<total_loaded_sectors>/<total_sectors>	(if pixel width differs from the current one)
		mulu.w		load_loaded_sectors(a5), d7
		divu.w		load_total_sectors(a5), d7
		cmp.w		#PROGRESS_BAR_WIDTH, d7
		bls			.progress_ok
		move.w		#PROGRESS_BAR_WIDTH, d7
.progress_ok:
		cmp.w		load_progress_pixels(a5), d7
		beq			.progress_skip
		move.w		d7, load_progress_pixels(a5)
		bsr			BootCmd_Progress_Arg								;		Draw progress
.progress_skip:

		btst.b		#0, load_flags+1(a5)								;	- [optional] Decompress (DEFLATE) the buffer to <index>/<offset> section location
		beq			.no_decompress

		move.l		load_final_dst(a5), a4								; a4 = output, a5 = input, all regs preserved
		move.l		trk_decoded_address(a5), a5							; a6 = *end* of storage area (only if OPT_STORAGE_OFFSTACK)
		bsr			_inflate											;

																		;		TBD
.no_decompress:


		rts



.str_decode_error	dc.b	"\nTRACK \x01 DECODE ERROR \x02", 0
.str_neq			dc.b	"\n\x01 != \x02", 0
.str_crc_error		dc.b	"\nTRACK $\x01 CRC ERROR $\x02 != $\x03", 0
					even
