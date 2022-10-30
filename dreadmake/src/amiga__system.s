


; ================================================================ Sys globals ================================================================

		SECTION_BSS

  if SYSTEM_PRESENT
base_graphics_library:	dcb.l	1
base_dos_library:		dcb.l	1
  endif		; SYSTEM_PRESENT

Save_ActiView:			dcb.l	1
Save_DMACON:			dcb.w	1
Save_INTENA:			dcb.w	1
Save_Stack:				dcb.l	1
Save_Irq_1:				dcb.l	1
Save_Irq_2:				dcb.l	1
Save_Irq_3:				dcb.l	1
Save_Irq_4:				dcb.l	1
Save_Irq_5:				dcb.l	1
Save_Irq_6:				dcb.l	1
Save_Trap_0:			dcb.l	1
Save_Trap_1:			dcb.l	1

Save_CIAA_ICR:			dcb.b	1
Save_CIAA_CRA:			dcb.b	1
Save_CIAA_CRB:			dcb.b	1
Save_CIAA_TALO:			dcb.b	1
Save_CIAA_TAHI:			dcb.b	1
Save_CIAA_TBLO:			dcb.b	1
Save_CIAA_TBHI:			dcb.b	1

Save_CIAB_ICR:			dcb.b	1
Save_CIAB_CRA:			dcb.b	1
Save_CIAB_CRB:			dcb.b	1
Save_CIAB_TALO:			dcb.b	1
Save_CIAB_TAHI:			dcb.b	1
Save_CIAB_TBLO:			dcb.b	1
Save_CIAB_TBHI:			dcb.b	1
				

						cnop	0, 4		; dword align
	public _file_info_block
_file_info_block:		dcb.b	260			; sizeof(FileInfoBlock)

							cnop	0, 4
StandardPacket_message:		dcb.b	20
StandardPacket_dospacket:	dcb.b	48



						even


		SECTION_DATA

  if SYSTEM_PRESENT
name_graphics_library:	dc.b	"graphics.library", 0, 0
name_dos_library:		dc.b	"dos.library", 0, 0
name_sys_device:		dc.b	"sys:", 0
  endif		; SYSTEM_PRESENT

						even


		SECTION_CODE


; ================================================================ _Sys_OpenLibraries ================================================================
;
  if SYSTEM_PRESENT
	public _Sys_OpenLibraries
_Sys_OpenLibraries:
	movem.l		a6, -(a7)
	move.l		$4.w, a6										; Retrieve Vector Base Register

	lea.l		name_graphics_library, a1						; graphics.library
	jsr			sys_exec_OldOpenLibrary(a6)
	move.l		d0, base_graphics_library

	lea.l		name_dos_library, a1							; dos.library
	jsr			sys_exec_OldOpenLibrary(a6)
	move.l		d0, base_dos_library

	movem.l		(a7)+, a6
	rts

  endif		; SYSTEM_PRESENT


; ================================================================ _Sys_CloseLibraries ================================================================
;
  if SYSTEM_PRESENT
	public _Sys_CloseLibraries
_Sys_CloseLibraries:
	movem.l		a6, -(a7)
	move.l		$4.w, a6										; Retrieve Vector Base Register

;	lea.l		base_graphics_library, a1						; graphics.library
;	jsr			sys_exec_CloseLibrary(a6)

;	lea.l		base_dos_library, a1							; dos.library
;	jsr			sys_exec_CloseLibrary(a6)

	movem.l		(a7)+, a6
	rts
  endif		; SYSTEM_PRESENT


; ================================================================ _Sys_InstallTrap1 ================================================================
	public _Sys_InstallTrap1
_Sys_InstallTrap1:
	move.l		#Trap1_Handler_disabled, IV_TRAP_1
	cmp.w		#20, _cpu_type
	blo			.no_trap_1
	move.l		#Trap1_Handler, IV_TRAP_1
.no_trap_1:
	rts


; ================================================================ _Sys_Install ================================================================
;
	public _Sys_Install
_Sys_Install:
	movem.l		a6, -(a7)
	
	lea.l		CHIP_BASE,a1
	move.w		INTENAR(a1), Save_INTENA
	move.w		DMACONR(a1), Save_DMACON

	move.w		#$3fdf, INTENA(a1)							; leave vblank running
	move.w		#$01ff, DMACON(a1)							; disable all DMA

	sub.l		a0, a0										; Init interrupts

	move.l		IV_LEVEL_1(a0), Save_Irq_1					; Level 1 - TBE, DSKBLK, soft
  if MULTIPLAYER_MODE
	lea.l		Irq_TBE, a1
	move.l		a1, IV_LEVEL_1(a0)
  endif

	move.l		IV_LEVEL_2(a0), Save_Irq_2					; Level 2 - PORTS
  if AUDIO_ENABLED
	lea.l		Irq_CIAA,a1
	move.l		a1, IV_LEVEL_2(a0)
  endif

	move.l		IV_LEVEL_3(a0), Save_Irq_3					; Level 3 - COPER, VERTB, BLIT
	lea.l		Irq_VideoHandler,a1
	move.l		a1, IV_LEVEL_3(a0)

	move.l		IV_LEVEL_4(a0), Save_Irq_4					; Level 4 - AUDx
  if AUDIO_ENABLED
	lea.l		Irq_AudioBufferStarted,a1
	move.l		a1, IV_LEVEL_4(a0)
  endif

	move.l		IV_LEVEL_5(a0), Save_Irq_5					; Level 5 - RBF
  if MULTIPLAYER_MODE
	lea.l		Irq_RBF, a1
	move.l		a1, IV_LEVEL_5(a0)
  endif

	move.l		IV_LEVEL_6(a0), Save_Irq_6					; Level 6 - external (CIAB)
  if AUDIO_ENABLED
	lea.l		Irq_CIAB,a1
	move.l		a1, IV_LEVEL_6(a0)
  endif

	lea.l		Trap0_Handler, a1							; Trap 0
	move.l		IV_TRAP_0(a0), Save_Trap_0
	move.l		a1, IV_TRAP_0(a0)

	move.l		IV_TRAP_1(a0), Save_Trap_1					; Trap 1
	jsr			_Sys_InstallTrap1


																	; ======== Save CIAA ========
  if SYSTEM_PRESENT
	move.l		$4.w, a6											;	retrieve Vector Base Register
	lea.l		.name_CIAA, a1										;	open CIA
	jsr			sys_exec_OpenResource(a6)
	
	move.l		d0, a6												;	save old ICR register	
	moveq.l		#0, d0
	jsr			sys_cia_AbleICR(a6)
	move.b		d0, Save_CIAA_ICR

	move.b		CIAA_CRA, Save_CIAA_CRA
	move.b		#CIACRAF_LOAD, CIAA_CRA								; stop the timer and load TALO/TAHI into counter register for readout
	move.b		CIAA_TALO,	Save_CIAA_TALO
	move.b		CIAA_TAHI,	Save_CIAA_TAHI

	move.b		CIAA_CRB, Save_CIAA_CRB
	move.b		#CIACRBF_LOAD, CIAA_CRB
	move.b		CIAA_TBLO,	Save_CIAA_TBLO
	move.b		CIAA_TBHI,	Save_CIAA_TBHI
  endif	; SYSTEM_PRESENT

	move.b		#$7F, CIAA_ICR										; Stop CIAA
	move.b		#0, CIAA_CRB
	tst.b		CIAA_ICR
	


																	; ======== Save CIAB ========
  if SYSTEM_PRESENT
	move.l		$4.w, a6											;	retrieve Vector Base Register
	lea.l		.name_CIAB, a1										;	open CIA
	jsr			sys_exec_OpenResource(a6)
	
	move.l		d0, a6												;	save old ICR register	
	moveq.l		#0, d0
	jsr			sys_cia_AbleICR(a6)
	move.b		d0, Save_CIAB_ICR

	move.b		CIAB_CRA, Save_CIAB_CRA
	move.b		#CIACRAF_LOAD, CIAB_CRA								; stop the timer and load TALO/TAHI into counter register for readout
	move.b		CIAB_TALO,	Save_CIAB_TALO
	move.b		CIAB_TAHI,	Save_CIAB_TAHI

	move.b		CIAB_CRB, Save_CIAB_CRB
	move.b		#CIACRBF_LOAD, CIAB_CRB
	move.b		CIAB_TBLO,	Save_CIAB_TBLO
	move.b		CIAB_TBHI,	Save_CIAB_TBHI
  endif ; SYSTEM_PRESENT

	move.b		#$7F, CIAB_ICR										; Stop CIAB
	move.b		#0, CIAB_CRB
	tst.b		CIAB_ICR


	
	move.w		#$8008, CHIP_BASE+INTENA							; Enable CIAA interrupt		TBLOAD = 14188 = $376C	(~50Hz)
	move.b		#$6C,	CIAA_TBLO
	move.b		#$37,	CIAA_TBHI
	move.b		#CIAICRF_SETCLR|CIAICRF_TB, CIAA_ICR
	move.b		#CIACRBF_START|CIACRBF_LOAD, CIAA_CRB

	move.w		#$a000, CHIP_BASE+INTENA							; Enable CIAB interrupt		TBLOAD = 682 = $2AA		(~15 scanlines)
	move.b		#$AA,	CIAB_TBLO
	move.b		#$02,	CIAB_TBHI
	move.b		#CIAICRF_SETCLR|CIAICRF_TB, CIAB_ICR
	; move.b	#CIACRBF_START|CIACRBF_LOAD, CIAB_CRB 

	move.w		#$8020, CHIP_BASE+INTENA							; enable vblank

	movem.l		(a7)+, a6
	rts

  if SYSTEM_PRESENT
.name_CIAA:	dc.b	"ciaa.resource",0,0
.name_CIAB:	dc.b	"ciab.resource",0,0
	cnop	0, 2
  endif ; SYSTEM_PRESENT

; ================================================================ _Sys_Restore ================================================================
;
  if SYSTEM_PRESENT
	public _Sys_Restore
_Sys_Restore:

	sub.l		a0, a0

	move.l		Save_Irq_1, IV_LEVEL_1(a0)							; Restore interrupts
	move.l		Save_Irq_2, IV_LEVEL_2(a0)
	move.l		Save_Irq_3, IV_LEVEL_3(a0)
	move.l		Save_Irq_4, IV_LEVEL_4(a0)
	move.l		Save_Irq_5, IV_LEVEL_5(a0)
	move.l		Save_Irq_6, IV_LEVEL_6(a0)
	move.l		Save_Trap_0, IV_TRAP_0(a0)
	move.l		Save_Trap_1, IV_TRAP_1(a0)


	move.b		Save_CIAA_TALO, CIAA_TALO							; Restore CIAA
	move.b		Save_CIAA_TAHI, CIAA_TAHI
	move.b		Save_CIAA_CRA, d0
	or.b		#CIACRAF_LOAD, d0
	move.b		d0, CIAA_CRA

	move.b		Save_CIAA_TBLO, CIAA_TBLO
	move.b		Save_CIAA_TBHI, CIAA_TBHI
	move.b		Save_CIAA_CRB, d0
	or.b		#CIACRBF_LOAD, d0
	move.b		d0, CIAA_CRB

	tst.b		CIAA_ICR
	move.b		#$7F, CIAA_ICR
	move.b		Save_CIAA_ICR, d0
	or.b		#$80, d0
	move.b		d0, CIAA_ICR
	
	tst.b		CIAA_ICR
	move.w		#$0008, CHIP_BASE + INTREQ
	move.w		#$0008, CHIP_BASE + INTREQ


	move.b		Save_CIAB_TALO, CIAB_TALO							; Restore CIAB
	move.b		Save_CIAB_TAHI, CIAB_TAHI
	move.b		Save_CIAB_CRA, d0
	or.b		#CIACRAF_LOAD, d0
	move.b		d0, CIAB_CRA

	move.b		Save_CIAB_TBLO, CIAB_TBLO
	move.b		Save_CIAB_TBHI, CIAB_TBHI
	move.b		Save_CIAB_CRB, d0
	or.b		#CIACRBF_LOAD, d0
	move.b		d0, CIAB_CRB

	tst.b		CIAB_ICR
	move.b		#$7F, CIAB_ICR
	move.b		Save_CIAB_ICR, d0
	or.b		#$80, d0
	move.b		d0, CIAB_ICR

	tst.b		CIAB_ICR
	move.w		#$2000, CHIP_BASE + INTREQ
	move.w		#$2000, CHIP_BASE + INTREQ

	lea.l		CHIP_BASE, a1
	
	move.w		#$0000, INTENA(a1)
	move.w		Save_INTENA, d0
	or.w		#$8000, d0
	move.w		d0, INTENA(a1)

	move.w		#$0000, DMACON(a1)
	move.w		Save_DMACON, d0
	or.w		#$8000, d0
	move.w		d0, DMACON(a1)

	rts
  endif ; SYSTEM_PRESENT


; ================================================================ _Sys_Flush ================================================================
;
  if SYSTEM_PRESENT
	public _Sys_Flush
_Sys_Flush:
	rts
	movem.l		d4/a2/a6,-(a7)

																	;			;get filesystem message port
	move.l		base_dos_library, a6								;				move.l	osDosBase(a4),a6		; DeviceProc(d1: "sys:") - get system process to d4
	move.l		#name_sys_device, d1								;				lea	name_sys_device(pc),a0
																	;				move.l	a0,d1
	jsr			sys_dos_DeviceProc(a6)								;				jsr	sys_dos_DeviceProc(a6)
	move.l		d0, d4												;				move.l	d0,d4
	beq			.end												;				beq.b	.exit
																			

																	;			;get our message port
	move.l		$4.w, a6		; base_exec_library					;				move.l	osExecBase(a4),a6
	move.l		sys_exec_ThisTask(a6), a2							;				move.l	sys_exec_ThisTask(a6),a2		;ThisTask
	lea.l		sys_task_pr_MsgPort(a2), a2							;				lea	sys_task_pr_MsgPort(a2),a2			;pr_MsgPort
																	;		
																	;			;fill in packet
	lea	StandardPacket_message, a1									;				lea	StandardPacket_message, a1
	lea	StandardPacket_dospacket, a0								;				lea	StandardPacket_dospacket, a0
																	;			
	move.l		a0, sys_message_ln_Name(a1)							;				move.l	a0,sys_message_ln_Name(a1)	;message->mn_Node.ln_Name = dospacket
	move.l		a2, sys_message_mn_ReplyPort(a1)					;				move.l	a2,sys_message_mn_ReplyPort(a1)	;mn_ReplyPort = pr_MsgPort
																	;		
	move.l		a1,(a0)+											;				move.l	a1,(a0)+	;dp_link = message
	move.l		a2,(a0)+											;				move.l	a2,(a0)+	;dp_Port = pr_MsgPort
	move.l		#dos_ACTION_FLUSH,(a0)								;				move.l	#27,(a0)	;dp_Type = ACTION_FLUSH
																	;		
																	;			;sends packet 
	move.l		d4, a0												;				move.l	d4,a0
	jsr			sys_exec_PutMsg(a6)									;				jsr	sys_exec_PutMsg(a6)	;exec PutMsg(a0 - port,a1 - msg)
																	;		
																	;			;wait for reply
	move.l		a2, a0												;				move.l	a2,a0
	jsr			sys_exec_WaitPort(a6)								;				jsr	sys_exec_WaitPort(a6)	;exec WaitPort(a0 - port)
	move.l		a2, a0												;				move.l	a2,a0
	jsr			sys_exec_GetMsg(a6)									;				jsr	sys_exec_GetMsg(a6)	;exec GetMsg(a0 - port)
																	;		
																	;		
																	;		filename:	dc.b	"sys:",0
																	;			cnop 0,4
																	;			; struct StandardPacket
																	;		message:	dcb.b 20,0
																	;		dospacket:	dcb.b 48,0

.end:
	movem.l		(a7)+, d4/a2/a6
	rts
  endif ; SYSTEM_PRESENT



; ================================================================ _Sys_WaitTOF ================================================================
;
  if SYSTEM_PRESENT
	public _Sys_WaitTOF
_Sys_WaitTOF:
	movem.l		a6,-(a7)

	move.l		base_graphics_library, a6				; Get graphics.library
	jsr			sys_graphics_WaitTOF(a6)

	movem.l		(a7)+,a6
	rts
  endif ; SYSTEM_PRESENT


; ================================================================ _Sys_Open ================================================================
;
  if SYSTEM_PRESENT
	public _Sys_Open
_Sys_Open:
	movem.l		d2/a6,-(a7)

	move.l		base_dos_library, a6						; Open (path already in <d1>)
	move.l		#dos_MODE_OLDFILE, d2
	jsr			sys_dos_Open(a6)

	movem.l		(a7)+, d2/a6								; result in <d0>
	rts
  endif ; SYSTEM_PRESENT

; ================================================================ _Sys_Close ================================================================
;
  if SYSTEM_PRESENT
	public _Sys_Close
_Sys_Close:
	movem.l		a6,-(a7)

	move.l		base_dos_library, a6						; Close (file handle already in <d1>)
	jsr			sys_dos_Close(a6)

	movem.l		(a7)+, a6
	rts
  endif ; SYSTEM_PRESENT

; ================================================================ _Sys_Read ================================================================
;
  if SYSTEM_PRESENT
	public _Sys_Read
_Sys_Read:
	movem.l		d2-d3/a6,-(a7)

	move.l		base_dos_library, a6						; Read (file, buffer and length already in <d1-d3>)
	jsr			sys_dos_Read(a6)

	movem.l		(a7)+, d2-d3/a6								; result in <d0>
	rts
  endif ; SYSTEM_PRESENT


; ================================================================ _Sys_Lock ================================================================
;
  if SYSTEM_PRESENT
	public _Sys_Lock
_Sys_Lock:
	movem.l		d2/a6, -(a7)

	move.l		base_dos_library, a6						; Lock	(path already in <d1>)
	move.l		#dos_ACCESS_READ, d2
	jsr			sys_dos_Lock(a6)

	movem.l		(a7)+, d2/a6									; result in <d0>
	rts
  endif ; SYSTEM_PRESENT

; ================================================================ _Sys_UnLock ================================================================
;
  if SYSTEM_PRESENT
	public _Sys_UnLock
_Sys_UnLock:
	movem.l		a6, -(a7)

	move.l		base_dos_library, a6						; UnLock	(lock already in <d1>)
	jsr			sys_dos_UnLock(a6)

	movem.l		(a7)+, a6
	rts
  endif ; SYSTEM_PRESENT

; ================================================================ _Sys_Examine ================================================================
;
  if SYSTEM_PRESENT
	public _Sys_Examine
_Sys_Examine:
	movem.l		a6,-(a7)

	move.l		base_dos_library, a6						; Examine	(lock and file info block already in <d1> and <d2>)
	jsr			sys_dos_Examine(a6)

	movem.l		(a7)+,a6
	rts
  endif ; SYSTEM_PRESENT


; ================================================================ _Sys_ExNext ================================================================
;
  if SYSTEM_PRESENT
	public _Sys_ExNext
_Sys_ExNext:
	movem.l		a6,-(a7)

	move.l		base_dos_library, a6						; _Sys_ExNext	(lock and file info block already in <d1> and <d2>)
	jsr			sys_dos_ExNext(a6)

	movem.l		(a7)+,a6
	rts
  endif ; SYSTEM_PRESENT



; ================================================================ _Sys_AllocAbs ================================================================
;input: 	A1 - requested memory address
;			D0 - memory block length
;result: 	D0 - memory pointer or NULL.
;
  if SYSTEM_PRESENT
	public _Sys_AllocAbs
_Sys_AllocAbs:
	movem.l		a6,-(a7)
	move.l		4, a6
	jsr			sys_exec_AllocAbs(a6)
	movem.l		(a7)+,a6
	rts
  endif ; SYSTEM_PRESENT



; ================================================================ _Sys_AllocMem ================================================================
;input: 	D0 - allocated memory size
;			D1 - memory attributes MEMF_... (e.g. MEMF_CLEAR=$10000, MEMF_CHIP=$00002, MEMF_FAST=$00004 )
;result: 	D0 - memory pointer or NULL. Address is aligned to dword.
;
  if SYSTEM_PRESENT
	public _Sys_AllocMem
_Sys_AllocMem
	; move.l		#$10002, d1
	movem.l		a6,-(a7)
	move.l		4, a6
	jsr			sys_exec_AllocMem(a6)
	movem.l		(a7)+,a6
	rts
  endif ; SYSTEM_PRESENT



; ================================================================ _Sys_FreeMem ================================================================
;input: 	A1 - memory address to be freed
;			D0 - freed memory size
;
  if SYSTEM_PRESENT
	public _Sys_FreeMem
_Sys_FreeMem
	movem.l		a6,-(a7)
	move.l		4, a6
	jsr			sys_exec_FreeMem(a6)
	movem.l		(a7)+,a6
	rts
  endif ; SYSTEM_PRESENT



; ================================================================ _Sys_KSChecksum ================================================================
;
	public _Sys_KSChecksum
_Sys_KSChecksum:
	move.l		d2, -(a7)

	move.l		#$FC0000, a0
	moveq.l		#0, d0
	lsl.l		d0						; clear X

	move.l		#255, d1
.loop:
	move.l		(a0)+, d2
	roxl.l		d0
	addx.l		d2, d0
	dbra.w		d1, .loop

	moveq.l		#0, d2
	addx.l		d2, d0

	move.l		(a7)+, d2
	rts
