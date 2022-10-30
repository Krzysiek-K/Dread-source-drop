
; All registers relative to $dff000
BLTCON0:	equ $040				;	word	  ~W   A       Blitter control register 0
BLTCON1:	equ $042				;	word	  ~W   A( E )  Blitter control register 1
BLTAWM:		equ $044				;	dword	  ~W   A       Blitter first word mask for source A
BLTAFWM:	equ $044				;	word	  ~W   A       Blitter first word mask for source A
BLTALWM:	equ $046				;	word	  ~W   A       Blitter last word mask for source A
BLTCPT:		equ $048				;	ptr_t	+ ~W   A       Blitter pointer to source C (high 3 bits)
BLTCPTH:	equ $048				;	word	+ ~W   A       Blitter pointer to source C (high 3 bits)
BLTCPTL:	equ $04A				;	word	+ ~W   A       Blitter pointer to source C (low 15 bits)
BLTBPT:		equ $04C				;	ptr_t	+ ~W   A       Blitter pointer to source B (high 3 bits)
BLTBPTH:	equ $04C				;	word	+ ~W   A       Blitter pointer to source B (high 3 bits)
BLTBPTL:	equ $04E				;	word	+ ~W   A       Blitter pointer to source B (low 15 bits)
BLTAPT:		equ $050				;	ptr_t	+ ~W   A( E )  Blitter pointer to source A (high 3 bits)
BLTAPTH:	equ $050				;	word	+ ~W   A( E )  Blitter pointer to source A (high 3 bits)
BLTAPTL:	equ $052				;	word	+ ~W   A       Blitter pointer to source A (low 15 bits)
BLTDPT:		equ $054				;	ptr_t	+ ~W   A       Blitter pointer to destination D (high 3 bits)
BLTDPTH:	equ $054				;	word	+ ~W   A       Blitter pointer to destination D (high 3 bits)
BLTDPTL:	equ $056				;	word	+ ~W   A       Blitter pointer to destination D (low 15 bits)
BLTSIZE:	equ $058				;	word	  ~W   A       Blitter start and size (window width,height)
BLTCON0L:	equ $05A				;	word	  ~W   A( E )  Blitter control 0, lower 8 bits (minterms)
BLTSIZV:	equ $05C				;	word	  ~W   A( E )  Blitter V size (for 15 bit vertical size)
BLTSIZH:	equ $05E				;	word	  ~W   A( E )  Blitter H size and start (for 11 bit H size)
BLTCMOD:	equ $060				;	word	  ~W   A       Blitter modulo for source C
BLTBMOD:	equ $062				;	word	  ~W   A       Blitter modulo for source B
BLTAMOD:	equ $064				;	word	  ~W   A       Blitter modulo for source A
BLTDMOD:	equ $066				;	word	  ~W   A       Blitter modulo for destination D
BLTCDAT:	equ $070				;	word	% ~W   A       Blitter source C data register
BLTBDAT:	equ $072				;	word	% ~W   A       Blitter source B data register
BLTADAT:	equ $074				;	word	% ~W   A       Blitter source A data register


; Blitter register load order:
;	BLTxDAT
;	BLTCONx
;	BLTAxWM
;	BLTxMOD
;	BLTxPT
;	BLTSIZE


	xref _Dread_BlitWeapon



screen_A0:	equ	2
screen_A1:	equ	2+8400
screen_A2:	equ	2+8400*2
screen_A3:	equ	2+8400*3
render_A:	equ 2+8400*4
temp_X:		equ	2+8400*4+16000+40


	public _BlockAddr_Work
	public _BlockAddr_Temp
	public _C2P_Scene_Queue
_BlockAddr_Work:	dc.w	0
_BlockAddr_Temp:	dc.w	0
_C2P_Scene_Queue:	dc.w	0		; [2]
					dc.w	0

_Blit_IRQ_fn:
	dc.l	0

_Blit_C2P_Pass_1a:
	; assumes A1 = $dff000
	lea.l	_Blit_C2P_Pass_1b(pc), a0
	move.l	a0, _Blit_IRQ_fn
	moveq	#0, d0
	move.w	#$CCCC, BLTCDAT(a1)					;	BLTCDAT = 0xCCCC;								// data
	move.w	#$EDE4, BLTCON0(a1)					;	BLTCON0 = 0xED00 | (_A&_C | _B&~_C);			// BLTCONx
	move.w	d0, BLTCON1(a1)						;	BLTCON1 = 0x0000;
	move.l	#$FFFFFFFF, BLTAWM(a1)				;	BLTAWM = 0xFFFFFFFF;							// mask
	move.w	d0, BLTAMOD(a1)						;	BLTAMOD = 0;									// modulos
	move.w	d0, BLTBMOD(a1)						;	BLTBMOD = 0;
	move.w	d0, BLTDMOD(a1)						;	BLTDMOD = 0;
	move.w	_BlockAddr_Work(pc), BLTAPTH(a1)	;	BLTAPT = render_A;								// pointers
	move.w	#render_A, BLTAPTL(a1)
	move.w	_BlockAddr_Work(pc), BLTBPTH(a1)	;	BLTBPT = render_A+4000-2;
	move.w	#(render_A + 4000-2), BLTBPTL(a1)
	move.w	_BlockAddr_Work(pc), BLTDPTH(a1)	;	BLTDPT = temp_X+2000-1;
	move.w	#(temp_X + 4000-2), BLTDPTL(a1)
	move.w	#((87<<6) | 23), BLTSIZE(a1)		;	BLTSIZE = COMPUTE_BLTSIZE(23, 87);				// start
	rts

_Blit_C2P_Pass_1b:
	; assumes A1 = $dff000
	lea.l	_Blit_C2P_Pass_1c(pc),a0
	move.l	a0, _Blit_IRQ_fn
	move.w	#(render_A + 8000), BLTAPTL(a1)		;	BLTAPT = render_A+8000;							// pointers
	move.w	#(render_A + 12000-2), BLTBPTL(a1)	;	BLTBPT = render_A+12000-2;
	move.w	_BlockAddr_Temp(pc), BLTDPTH(a1)	;	BLTDPT = temp_Y+2000-1;
	move.w	#(temp_X + 4000-2), BLTDPTL(a1)
	move.w	#((87<<6) | 23), BLTSIZE(a1)		;	BLTSIZE = COMPUTE_BLTSIZE(23, 87);				// start
	rts

_Blit_C2P_Pass_1c:
	; assumes A1 = $dff000
	lea.l	_Blit_C2P_Pass_1d(pc),a0
	move.l	a0, _Blit_IRQ_fn
	move.w	#$0DE4, BLTCON0(a1)					;	BLTCON0 = 0x0D00 | (_A&_C | _B&~_C);			// BLTCONx
	move.w	#$2000, BLTCON1(a1)					;	BLTCON1 = 0x2000;
	move.w	#(render_A), BLTAPTL(a1)			;	BLTAPT = render_A;								// pointers
	move.w	#(render_A + 4000), BLTBPTL(a1)		;	BLTBPT = render_A+4000;
	move.w	_BlockAddr_Work(pc), BLTDPTH(a1)	;	BLTDPT = temp_X;
	move.w	#(temp_X), BLTDPTL(a1)
	move.w	#((100<<6) | 20), BLTSIZE(a1)		;	BLTSIZE = COMPUTE_BLTSIZE(20, 100);				// start
	rts


_Blit_C2P_Pass_1d:
	; assumes A1 = $dff000
	lea.l	_Blit_C2P_Pass_2a(pc),a0
	move.l	a0, _Blit_IRQ_fn
	move.w	#(render_A + 8000), BLTAPTL(a1)		;	BLTAPT = render_A+8000;							// pointers
	move.w	#(render_A + 12000), BLTBPTL(a1)	;	BLTBPT = render_A+12000;
	move.w	_BlockAddr_Temp(pc), BLTDPTH(a1)	;	BLTDPT = temp_Y
	move.w	#(temp_X), BLTDPTL(a1)
	move.w	#((100<<6) | 20), BLTSIZE(a1)		;	BLTSIZE = COMPUTE_BLTSIZE(20, 100);				// start
	rts

_Blit_C2P_Pass_2a:
	; assumes A1 = $dff000
	lea.l	_Blit_C2P_Pass_2b(pc),a0
	move.l	a0, _Blit_IRQ_fn
	move.w	_BlockAddr_Work(pc), _C2P_Scene_Queue+2		; return processed render buffer to the queue
	move.w	#$F0F0, BLTCDAT(a1)					;	BLTCDAT = 0xF0F0;								// data
	move.w	#$0DE4, BLTCON0(a1)					;	BLTCON0 = 0x0D00 | (_A&_C | _B&~_C);			// BLTCONx
	move.w	#$4000, BLTCON1(a1)					;	BLTCON1 = 0x4000;
	move.w	#44, BLTDMOD(a1)					;	BLTDMOD = 44;									// modulos
	move.w	#(temp_X), BLTAPTL(a1)				;	BLTAPT = temp_X;								// pointers
	move.w	_BlockAddr_Temp(pc), BLTBPTH(a1)	;	BLTBPT = temp_Y;
	move.w	#(temp_X), BLTBPTL(a1)
	move.w	_BlockAddr_Work(pc), BLTDPTH(a1)	;	BLTDPT = screen_A[0];
	move.w	#(screen_A0), BLTDPTL(a1)
	move.w	#((200<<6) | 20), BLTSIZE(a1)		;	BLTSIZE = COMPUTE_BLTSIZE(20, 200);				// start
	rts

_Blit_C2P_Pass_2b:
	; assumes A1 = $dff000
	lea.l	_Blit_C2P_Pass_weapon0(pc),a0
	move.l	a0, _Blit_IRQ_fn
	move.w	#$CDE4, BLTCON0(a1)					;	BLTCON0 = 0xCD00 | (_A&_C | _B&~_C);			// BLTCONx
	move.w	#$0000, BLTCON1(a1)					;	BLTCON1 = 0x0000;
	move.w	#-2, BLTAMOD(a1)					;	BLTAMOD = -2;									// modulos
	move.w	#-2, BLTBMOD(a1)					;	BLTBMOD = -2;
	move.w	#42, BLTDMOD(a1)					;	BLTDMOD = 42;
	move.w	#(temp_X), BLTAPTL(a1)				;	BLTAPT = temp_X;								// pointers
	move.w	#(temp_X-2), BLTBPTL(a1)			;	BLTBPT = temp_Y-1;
	move.w	#(screen_A2-2), BLTDPTL(a1)			;	BLTDPT = screen_A[2]-1;
	move.w	#((200<<6) | 21), BLTSIZE(a1)		;	BLTSIZE = COMPUTE_BLTSIZE(21, 200);				// start
	rts

_Blit_C2P_Pass_weapon0:
	lea.l	_Blit_C2P_Pass_weapon1(pc),a0
	move.l	a0, _Blit_IRQ_fn
	moveq.l	#0, d0
	jsr		_Dread_BlitWeapon
	rts

_Blit_C2P_Pass_weapon1:
	lea.l	_Blit_C2P_Pass_weapon2(pc),a0
	move.l	a0, _Blit_IRQ_fn
	moveq.l	#1, d0
	jsr		_Dread_BlitWeapon
	rts

_Blit_C2P_Pass_weapon2:
	lea.l	_Blit_C2P_Pass_weapon3(pc),a0
	move.l	a0, _Blit_IRQ_fn
	moveq.l	#2, d0
	jsr		_Dread_BlitWeapon
	rts

_Blit_C2P_Pass_weapon3:
;	lea.l	_Blit_C2P_Pass_hqd(pc),a0
	lea.l	_Blit_C2P_Pass_end(pc),a0
	move.l	a0, _Blit_IRQ_fn
	moveq.l	#3, d0
	jsr		_Dread_BlitWeapon
	rts

_Blit_C2P_Pass_hqd:
	; assumes A1 = $dff000
	lea.l	_Blit_C2P_Pass_end(pc),a0
	move.l	a0, _Blit_IRQ_fn
	move.w	#$5555, BLTCDAT(a1)					;	BLTCDAT = 0x5555;								// data
	move.w	#$1DE4, BLTCON0(a1)					;	BLTCON0 = 0x1D00 | (_A&_C | _B&~_C);			// BLTCONx
	move.w	#$F000, BLTCON1(a1)					;	BLTCON1 = 0xF000;
	move.l	#$FFFFFFFF, BLTAWM(a1)				;	BLTAWM = 0xFFFFFFFF;							// mask
	move.w	#42, BLTAMOD(a1)					;	BLTAMOD = 42;									// modulos
	move.w	#42, BLTBMOD(a1)					;	BLTBMOD = 42;
	move.w	#42, BLTDMOD(a1)					;	BLTDMOD = 42;
	move.w	_BlockAddr_Work(pc), BLTAPTH(a1)	;	BLTAPT = screen_A[0]-1;							// pointers
	move.w	#(screen_A0-2), BLTAPTL(a1)		
	move.w	_BlockAddr_Work(pc), BLTBPTH(a1)	;	BLTBPT = screen_A[0];
	move.w	#(screen_A0), BLTBPTL(a1)			
	move.w	#(screen_A0+42-2), BLTDPTL(a1)		;	BLTDPT = screen_A[0]+21-1;
	move.w	#((400<<6) | 21), BLTSIZE(a1)		;	BLTSIZE = COMPUTE_BLTSIZE(21, 400);				// start
	rts

_Blit_C2P_Pass_end:
	move.l	#0, _Blit_IRQ_fn
	move.l	_BlockAddr_Work(pc), d0

												; swap work with temp
	swap.w	d0
	move.l	d0, _BlockAddr_Work

												; switch copper program
	move.w	d0, $dff080							; COP1LCH = Work


	; TBD: check if next c2p pass is already pending
	rts


	public _C2P_BeginScene
_C2P_BeginScene:
	move.l	_C2P_Scene_Queue(pc), d0					; wait until a buffer is ready
	beq.b	_C2P_BeginScene

	move.w	#$2300, d1									; disable Blitter/VSync interrupts							; TBD: disable for A7 test
	trap	#0
	move.l	_C2P_Scene_Queue(pc), d0					; read again (with IRQ off)
	move.w	d0, d1
	bne.b	.c2pbs_use_slot0
	swap.w	d0
.c2pbs_use_slot0:
														; D0 =	<pending>	<to-use>
	move.l	d0, d1
	move.w	#0, d1
	swap.w	d1											; D1 =	0000		<pending>
	
	move.l	d1, _C2P_Scene_Queue						; write queue back
	move.w	#$2000, d1									; enable Blitter/VSync interrupts									; TBD: disable for A7 test
	trap	#0
	 
	swap.w	d0											; prepare address in D0
	move.w	#render_A, d0
	rts

	public _C2P_EndScene
_C2P_EndScene:

	; DMACONR   =	$dff002			;  *R   AP      DMA control (and blitter status) read
	tst		$dff002					; for compatibility

.wait:
	btst	#6, $dff002				; wait for blitter done
	bne.s	.wait
	
	move.l	_Blit_IRQ_fn(pc), d0	; wait until blitter queue is empty
	bne.b	.wait

	lea.l	$dff000,a1
	jsr		_Blit_C2P_Pass_1a

	rts


InterruptHandler:
	movem.l	d0-d1/a0-a1,-(a7)

	lea.l	$dff000,a1
	move.w	$01e(a1), d0			; INTREQR
	and.w	#$0040, d0
	beq.b	.VSync

	; Blitter interrupt
	move.w	#$0040,$09c(a1)
	move.w	#$0040,$09c(a1)			; COMPATIBILITY >= 1

	move.l	_Blit_IRQ_fn(pc), d0
	beq.b	.BlitEnd
	
	move.l	d0, a0
	jsr		(a0)

	; end
.BlitEnd:
	movem.l	(a7)+,d0-d1/a0-a1,
	rte


.VSync:
	movem.l	d2-d7/a2-a6,-(a7)

	if MUSICPLAYER=1
		if MUSICPLAYERUSEVBLANK<>0
			lea		$dff000,a6
			bsr		P61_Music
		endc
	endc

	bsr.w	_InterruptMain

	lea.l	$dff000,a1

	move.w	#$0020,$09c(a1)
	move.w	#$0020,$09c(a1)			; COMPATIBILITY >= 1

	movem.l	(a7)+,d2-d7/a2-a6,
	movem.l	(a7)+,d0-d1/a0-a1,

	rte
