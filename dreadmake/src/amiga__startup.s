


; Things to find and kill:
;
;	MUSICPLAYER
;	MUSICPLAYERUSEVBLANK
;	SETUPCIAINTERRUPT
;	LEAVETHESEDMACHANNELSALONE
;	LEAVETHESEINTERRUPTSALONE
;	SHOWMUSICRASTERLINES
;	usecode
;	Music_Init
;	MUSIC_PLAYER
;	DMA_OFF
;	DMA_ON
;	COPPER
;	BITPLANE
;	BLITTER
;	SPRITE
;	P6111-Play.i
;




		xref	_Aud_Reset
		xref	_opt_ntsc			; word







; Entry point

	SECTION_CODE

Startup:

		jsr		_Precalc

  if SYSTEM_PRESENT
		move.l	$4.w,a6											; Retrieve Vector Base Register
		suba.l	a2,a2
		btst.b	#0, sys_exec_eb_AttnFlags+1(a6)
		beq.b	.no010
		lea.l	GetVBR(pc),a5
		jsr		sys_exec_Supervisor(a6)
.no010:

		jsr		_Sys_OpenLibraries

		move.l	base_graphics_library, a6					; Get graphics.library

															; Clear display
		move.l	sys_graphics_gb_ActiView(a6), Save_ActiView
		suba.l	a1,a1
		jsr		sys_graphics_LoadView(a6)
		lea.l	CHIP_BASE,a3
		move.w	#$0020, $1dc(a3)							; Ensure PAL
		jsr		sys_graphics_WaitTOF(a6)
		jsr		sys_graphics_WaitTOF(a6)

		jsr		_Sys_Flush

  else
												; exit supervisor mode
        and.w		#$D8FF,sr					; set lowest priority level

  endif ; SYSTEM_PRESENT

		jsr		_Sys_Install


		move.w	#$0200,$100(a3)								; make sure we dont have any ugly flashes on startup: clear colors and set 0 bitplanes
		move.w	#$180,d0
	.clearColors:
		clr.w	0(a3,d0.w)
		addq.w	#2,d0
		cmp.w	#$1c0,d0
		bne		.clearColors


		jsr		_Aud_Reset
		jsr		_DetectCPU									; Detect CPU
		jsr		_Sys_InstallTrap1
		
															; Detect NTSC
	;	clr.w	_opt_ntsc
	;	move.l	$4, a1
	;	cmp.b	#60, $213(a1)
	;	bne		.pal_mode
	;	move.w	#1, _opt_ntsc
	;.pal_mode:
		
	
	if TRACKLOADER
		xref	_Track_Init
		jsr		_Track_Init
	else
	    or.b    #$f8, $bfd100								; Disable all drives
	    nop
	    and.b   #$87, $bfd100
	    nop
	    or.b    #$78, $bfd100
	    nop
	endif


;.test_loop:
;		add.w	#1235, d0
;		move.w	d0, $dff180
;		bra		.test_loop
	
		; ================================ Launch application ================================
	
		jsr		_Main
	
	
	
	
	
		; ================================ Cleanup ================================
	
	
	StartupEnd:
		
  if SYSTEM_PRESENT
		jsr		_Sys_Restore

		lea.l	CHIP_BASE,a3									; Restore display
		move.w	#$a00c,$09a(a3)
		move.w	#$001f,$096(a3)
		move.w	#$81e0,$096(a3)

		move.l	base_graphics_library, a6						; Get graphics.library

		move.l	sys_graphics_gb_copinit(a6),$080(a3)			; restore copper
		move.l	Save_ActiView, a1
		jsr		sys_graphics_LoadView(a6)

		jsr		_Sys_CloseLibraries

		jmp		_Exit
  else

		bra		StartupEnd						; no system to return to - lock

  endif ; SYSTEM_PRESENT

GetVBR:	
	mc68020			
	movec	vbr,a2
	mc68000				
	rte


	even

	public _cpu_type
_cpu_type:
	dc.w	0



	even

	SECTION_CODE

		mc68000				
