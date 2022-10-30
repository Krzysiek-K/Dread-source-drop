

;to stop here in WinUAE, enter "w 4 4 4 w" in the debugger window (shift+f12) to place the breakpoint, and enter "w 4" to remove it



; ================================================================ SECTION_CODE ================================================================
SECTION_CODE:	macro
	if	FASTMEM
		section	code,code_f
	else
		section	code,code
	endc
  endm


; ================================================================ SECTION_DATA ================================================================
SECTION_DATA:	macro
	section	data, data
  endm


; ================================================================ SECTION_DATA_CHIP ================================================================
SECTION_DATA_CHIP:	macro
	section	datachip,data_c
  endm


; ================================================================ SECTION_BSS ================================================================
SECTION_BSS:	macro
	section	bss,bss
  endm


; ================================================================ SECTION_BSS_CHIP ================================================================
SECTION_BSS_CHIP:	macro
	section	bsschip,bss_c
  endm



; ================================================================ WinUAEBreakpoint ================================================================
WinUAEBreakpoint:macro
		move.l	4.w, 4.w
	endm


; ================================================================ DEAD_LOOP ================================================================
DEAD_LOOP:macro
.dead_loop\@:
		addq.l	#1, d0
		move.w	d0, $dff180
		bra		.dead_loop\@
	endm


; ================================================================ CACHEFLUSH ================================================================
; Cache flush macro
; Potentially trashes D0-D1/A0-A1/A6
CACHEFLUSH:	macro
				move.l	$4.w,a6
				cmp.w	#37, sys_exec_lib_Version(a6)
				blo.b	.noflush\@
				jsr		sys_exec_CacheClearU(a6)
.noflush\@:	
			endm




; ================================================================ WAIT_SCANLINE ================================================================
; uses d0
WAIT_SCANLINE:	macro
		move.b		CHIP_BASE+VHPOSR, d0
.wait\@:
		cmp.b		CHIP_BASE+VHPOSR, d0
		beq.b		.wait\@
	endm
