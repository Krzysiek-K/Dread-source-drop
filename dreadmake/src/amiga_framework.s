


  if !WEAPONSPRITES
	xref	_Dread_BlitWeapon
  endif
	xref	_Dread_BlitStats

	xref	_intnum
	xref	_rawintnum
	xref	_debug_vars
	xref	_io_mouse_state
	xref	_io_mouse_xdelta
	xref	_io_mouse_ydelta
	xref	_io_key_last
	xref	_io_keystate
	xref	_palette_index
	xref	_e_map_palette
	xref	_e_map_palette_count
	xref	_opt_screen_mode


screen_A0:	equ	2
screen_A1:	equ	2+8400
screen_A2:	equ	2+8400*2
screen_A3:	equ	2+8400*3
render_A:	equ 2+8400*4
temp_X:		equ	2+8400*4+16160+40
copper_A:	equ 2+8400*4+16160+40+8040+4


	public _BlockAddr_Work
	public _BlockAddr_Temp
	public _C2P_Scene_Queue
_BlockAddr_Work:	dc.w	0
_BlockAddr_Temp:	dc.w	0
_C2P_Scene_Queue:	dc.w	0		; [2]
					dc.w	0

_Blit_IRQ_fn:
	dc.l	0

_Blit_IRQ_rep:
	dc.w	0
_Blit_IRQ_dptr:
	dc.w	0
_Blit_IRQ_aptr:
	dc.w	0

_Blit_C2P_Pass_1a:
	; assumes A1 = $dff000
	lea.l	_Blit_C2P_Pass_1a_rep(pc), a0
	move.l	a0, _Blit_IRQ_fn
	move.w	#20, _Blit_IRQ_rep
	moveq	#0, d0
	move.w	#$CCCC, BLTCDAT(a1)					;	BLTCDAT = 0xCCCC;								// data
	move.w	#$0DE4, BLTCON0(a1)					;	BLTCON0 = 0x0D00 | (_A&_C | _B&~_C);			// BLTCONx
	move.w	#$2000, BLTCON1(a1)						;	BLTCON1 = 0x2000;
	move.l	#$FFFFFFFF, BLTAWM(a1)				;	BLTAWM = 0xFFFFFFFF;							// mask
	move.w	d0, BLTAMOD(a1)						;	BLTAMOD = 0;									// modulos
	move.w	d0, BLTBMOD(a1)						;	BLTBMOD = 0;
	move.w	#38, BLTDMOD(a1)					;	BLTDMOD = 38;
	move.w	_BlockAddr_Work(pc), BLTAPTH(a1)	;	BLTAPT = render_A;								// pointers
	move.w	#render_A, BLTAPTL(a1)
;	move.w	#render_A, _Blit_IRQ_aptr
	move.w	_BlockAddr_Work(pc), BLTBPTH(a1)	;	BLTBPT = render_A+8080;
	move.w	#(render_A + 8080), BLTBPTL(a1)
	move.w	_BlockAddr_Work(pc), BLTDPTH(a1)	;	BLTDPT = temp_X;
	move.w	#(temp_X), BLTDPTL(a1)
	move.w	#(temp_X), _Blit_IRQ_dptr
	move.w	#((201<<6) | 1), BLTSIZE(a1)		;	BLTSIZE = COMPUTE_BLTSIZE(1, 200);				// start
	rts

_Blit_C2P_Pass_1a_rep:
	; assumes A1 = $dff000
	subq.w	#1, _Blit_IRQ_rep
	beq.b	_Blit_C2P_Pass_1b
	addq.w	#2, _Blit_IRQ_dptr
	move.w	_Blit_IRQ_dptr, BLTDPTL(a1)
;	move.w	_Blit_IRQ_aptr, d0
;	add.w	#402, d0
;	move.w	d0, _Blit_IRQ_aptr
;	move.w	d0, BLTAPTL(a1)
;	add.w	#8080, d0
;	move.w	d0, BLTBPTL(a1)
	move.w	#((201<<6) | 1), BLTSIZE(a1)		;	BLTSIZE = COMPUTE_BLTSIZE(1, 200);				// start
	rts

_Blit_C2P_Pass_1b:
	; assumes A1 = $dff000
	lea.l	_Blit_C2P_Pass_1b_rep(pc), a0
	move.l	a0, _Blit_IRQ_fn
	move.w	#20, _Blit_IRQ_rep
	moveq	#0, d0
	move.w	#$CCCC, BLTCDAT(a1)					;	BLTCDAT = 0xCCCC;								// data
	move.w	#$EDE4, BLTCON0(a1)					;	BLTCON0 = 0xED00 | (_A&_C | _B&~_C);			// BLTCONx
	move.w	d0, BLTCON1(a1)						;	BLTCON1 = 0x0000;
	move.w	#render_A, BLTAPTL(a1)				;	BLTAPT = render_A;								// pointers
;	move.w	#render_A, _Blit_IRQ_aptr
	move.w	#(render_A + 8080-2), BLTBPTL(a1)	;	BLTBPT = render_A+8080-2;
	move.w	_BlockAddr_Temp(pc), BLTDPTH(a1)	;	BLTDPT = temp_Y-40;
	move.w	#(temp_X-40), BLTDPTL(a1)
	move.w	#(temp_X-40), _Blit_IRQ_dptr
	move.w	#((201<<6) | 1), BLTSIZE(a1)		;	BLTSIZE = COMPUTE_BLTSIZE(1, 201);				// start
	rts

_Blit_C2P_Pass_1b_rep:
	; assumes A1 = $dff000
	subq.w	#1, _Blit_IRQ_rep
	beq.b	_Blit_C2P_Pass_2a
	addq.w	#2, _Blit_IRQ_dptr
	move.w	_Blit_IRQ_dptr, BLTDPTL(a1)
;	move.w	_Blit_IRQ_aptr, d0
;	add.w	#402, d0
;	move.w	d0, _Blit_IRQ_aptr
;	move.w	d0, BLTAPTL(a1)
;	add.w	#8080-2, d0
;	move.w	d0, BLTBPTL(a1)
	move.w	#((201<<6) | 1), BLTSIZE(a1)		;	BLTSIZE = COMPUTE_BLTSIZE(1, 201);				// start
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
	move.w	#(temp_X+4000), BLTBPTL(a1)			;	BLTBPT = temp_X + 4000;
	move.w	_BlockAddr_Work(pc), BLTDPTH(a1)	;	BLTDPT = screen_A[0];
	move.w	#(screen_A0), BLTDPTL(a1)
	move.w	#((100<<6) | 20), BLTSIZE(a1)		;	BLTSIZE = COMPUTE_BLTSIZE(20, 100);				// start
	rts

_Blit_C2P_Pass_2b:
	; assumes A1 = $dff000
	lea.l	_Blit_C2P_Pass_2c(pc),a0
	move.l	a0, _Blit_IRQ_fn
	move.w	_BlockAddr_Work(pc), _C2P_Scene_Queue+2		; return processed render buffer to the queue
	move.w	_BlockAddr_Temp(pc), BLTAPTH(a1)	;	BLTAPT = temp_Y;								// pointers
	move.w	#(temp_X), BLTAPTL(a1)			
	move.w	_BlockAddr_Temp(pc), BLTBPTH(a1)	;	BLTBPT = temp_Y + 4000;
	move.w	#(temp_X+4000), BLTBPTL(a1)		
	move.w	#(screen_A1), BLTDPTL(a1)			;	BLTDPT = screen_A[1];
	move.w	#((100<<6) | 20), BLTSIZE(a1)		;	BLTSIZE = COMPUTE_BLTSIZE(20, 100);				// start
	rts


_Blit_C2P_Pass_2c:
	; assumes A1 = $dff000
	lea.l	_Blit_C2P_Pass_2d(pc),a0
	move.l	a0, _Blit_IRQ_fn
	move.w	_BlockAddr_Work(pc), _C2P_Scene_Queue+2		; return processed render buffer to the queue
	move.w	#$CDE4, BLTCON0(a1)					;	BLTCON0 = 0xCD00 | (_A&_C | _B&~_C);			// BLTCONx
	move.w	#$0000, BLTCON1(a1)					;	BLTCON1 = 0x0000;
	move.w	#-2, BLTAMOD(a1)					;	BLTAMOD = -2;									// modulos
	move.w	#-2, BLTBMOD(a1)					;	BLTBMOD = -2;
	move.w	#42, BLTDMOD(a1)					;	BLTDMOD = 42;
	move.w	#(temp_X), BLTAPTL(a1)				;	BLTAPT = temp_Y;								// pointers
	move.w	#(temp_X+4000-2), BLTBPTL(a1)		;	BLTBPT = temp_Y + 4000 - 2;
	move.w	#(screen_A3-2), BLTDPTL(a1)			;	BLTDPT = screen_A[2] - 2;
	move.w	#((100<<6) | 21), BLTSIZE(a1)		;	BLTSIZE = COMPUTE_BLTSIZE(21, 100);				// start
	rts

_Blit_C2P_Pass_2d:
	; assumes A1 = $dff000
  if WEAPONSPRITES
	lea.l	_Blit_C2P_Pass_end(pc),a0

	moveq.l	#3, d0
	cmp.w	_opt_screen_mode, d0
	bne.b	.nohd
	lea.l	_Blit_C2P_Pass_hqd(pc),a0
.nohd:
  else
	lea.l	_Blit_C2P_Pass_weapon0(pc),a0
  endif
	move.l	a0, _Blit_IRQ_fn
	move.w	_BlockAddr_Work(pc), BLTAPTH(a1)	;	BLTAPT = temp_X;								// pointers
	move.w	#(temp_X), BLTAPTL(a1)			
	move.w	_BlockAddr_Work(pc), BLTBPTH(a1)	;	BLTBPT = temp_X + 4000 - 2;
	move.w	#(temp_X+4000-2), BLTBPTL(a1)		
	move.w	#(screen_A2-2), BLTDPTL(a1)			;	BLTDPT = screen_A[3] - 2;
	move.w	#((100<<6) | 21), BLTSIZE(a1)		;	BLTSIZE = COMPUTE_BLTSIZE(21, 100);				// start
	rts



  if !WEAPONSPRITES
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
  endif

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
	;move.l	#0, _Blit_IRQ_fn
	move.l	_BlockAddr_Work(pc), d0

												; swap work with temp
	swap.w	d0
	move.l	d0, _BlockAddr_Work

												; switch copper program
	move.w	d0, $dff080							; COP1LCH = Work


_Blit_C2P_Pass_Stats:

	bra		_Blit_Pass_StatusBar




	public _C2P_BeginScene
_C2P_BeginScene:
	move.l	_C2P_Scene_Queue(pc), d0					; wait until a buffer is ready
	beq.b	_C2P_BeginScene

	move.w	#$0300, d1									; disable Blitter/VSync interrupts									; TBD: disable for A7 test
	TRAP_SET_SR
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
	move.w	#$0000, d1									; enable Blitter/VSync interrupts									; TBD: disable for A7 test
	TRAP_SET_SR
	 
	swap.w	d0											; prepare address in D0
	move.w	#render_A, d0
	rts



	public _C2P_EndScene
_C2P_EndScene:

										; update palette if changed

	move.w	_BlockAddr_Work(pc), d0		; a0 = copper_A	(active copper)
	swap.w	d0							;
	move.w	#copper_A, d0				;
	move.l	d0, a0						;

	move.w	_palette_index, d0
	cmp.w	-2(a0), d0
	beq		.skip_palette
	move.w	d0, -2(a0)
	cmp.w	_e_map_palette_count, d0	; d0 ? e_map_palette_count
	bge		.skip_palette

	lsl.w	#5, d0
	move.l	_e_map_palette, a1
	lea.l	(a1,d0.w), a1

	add.l	#2, a0
	move.w	#15, d0
.pal_loop:
	move.w	(a1)+, (a0)
	add.l	#4, a0
	dbra	d0, .pal_loop

.skip_palette:

	; DMACONR   =	$dff002			;  *R   AP      DMA control (and blitter status) read
	tst		CHIP_BASE+DMACONR		; for compatibility

.wait:
	btst	#6, $dff002				; wait for blitter done
	bne.s	.wait
	
	move.l	_Blit_IRQ_fn(pc), d0	; wait until blitter queue is empty
	bne.b	.wait

	lea.l	CHIP_BASE,a1
	jsr		_Blit_C2P_Pass_1a

	rts


	public _C2P_WaitAll
_C2P_WaitAll:

.wait:
	btst	#6, $dff002				; wait for blitter done
	bne.s	.wait
	
	move.l	_Blit_IRQ_fn(pc), d0	; wait until blitter queue is empty
	bne.b	.wait

	rts





Irq_VideoHandler:					; IV_LEVEL_3	$6c		; graphics coprocessor, vertical blank interval, blitter finished
	movem.l	d0-d1/a0-a1,-(a7)

	lea.l	CHIP_BASE,a1
	move.w	INTREQR(a1), d0			; INTREQR
	and.w	#$0040, d0
	beq.b	.VSync

	; Blitter interrupt
	move.w	#$0040,INTREQ(a1)
	move.w	#$0040,INTREQ(a1)			; COMPATIBILITY >= 1

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


	; ================ Interrupt main ================

	add.w	#1, _intnum						;	intnum++;
	add.w	#1, _rawintnum					;	rawintnum++;


	lea.l	CHIP_BASE, a1

											;	// Handle mouse
	move.w	JOY0DAT(a1), d0					;	word joydat = JOY0DAT;
	move.w	d0, d1
	sub.b	.prev_joy0dat+1, d0				;	signed char xdelta = (signed char)joydat - (signed char)prev_joydat;
	ext.w	d0								;
	add.w	d0, _io_mouse_xdelta			;	io_mouse_xdelta += xdelta;

	move.w	d1, d0							;	signed char ydelta = (signed char)(joydat>>8) - (signed char)(prev_joydat>>8);
	lsr.w	#8, d0							;
	sub.b	.prev_joy0dat, d0				;
	ext.w	d0								;
	add.w	d0, _io_mouse_ydelta			;	io_mouse_ydelta += ydelta;

	move.w	d1, .prev_joy0dat				;	prev_joydat = joydat;

	; move.b	CIAA_PRA, d0				;	io_mouse_state |= !(CIAA_PRA & (1<<6));
	; and.b	#$40, d0						;	
	; eor.b	#$40, d0						;
	; or.b	d0, _io_mouse_state				;	


	jsr		_IO_CheckMouse

											;	// Handle keyboard
	move.l	#0, d0							;
	move.b	CIAA_SDR, d0					;	byte data = CIAA_SDR;
	cmp.b	.prev_keydat, d0				;	if( data!=_data )
	beq		.no_keys						;	{
	move.b	d0, .prev_keydat				;		_data = data;
	
											;		// acknowledge
	or.b	#$40, CIAA_CRA					;		CIAA_CRA |= (1<<6);

	move.l	#2, d1							;		for( word n=0; n<3; n++ )
.key_ack_delay:								;		{
	move.b	$dff006, d2						;			word pos = VHPOSR_B;
.key_ack_wait:								;					
	cmp.b	$dff006, d2						;			while( VHPOSR_B == pos ) {}
	beq		.key_ack_wait					;
	dbra.w	d1, .key_ack_delay				;		}

	and.b	#$FF-$40, CIAA_CRA				;		CIAA_CRA &= ~(1<<6);

											;	
											;		// process keycode
	move.l	#0, d2							;
	move.b	d0, d1							;		data = ~((data>>1) | (data<<7));
	lsr.b	#1, d0							;
	lsl.b	#7, d1							;
	or.b	d1, d0							;
	eor.b	#$FF, d0						;
	bmi		.key_up							;
	move.l	#1, d2							;
.key_up:
	move.b	d0, _io_key_last				;		io_key_last = data;
											;
												
	and.b	#$7F, d0						;		if( data&0x80 ) io_keystate[data&0x7F] = 0;
	lea.l	_io_keystate, a0				;		else			io_keystate[data] = 1;
	move.b	d2, (a0, d0.w)					;

.no_keys:									;	}
	;	
	;	//Track_ISR_Poll();
  if TRACKLOADER
	jsr		Track_ISR_Poll
  endif

  if WEAPONSPRITES
	eori.w		#1, .weapon_update_toggle
	beq.b		.no_weapon_update
	jsr			Update_Weapon_Sprite
.no_weapon_update:
  endif
	;jsr		_InterruptMain


	; ======== cleanup ========


	lea.l	CHIP_BASE,a1

	move.w	#$0020,INTREQ(a1)
	move.w	#$0020,INTREQ(a1)			; COMPATIBILITY >= 1

	movem.l	(a7)+,d2-d7/a2-a6
	movem.l	(a7)+,d0-d1/a0-a1

	rte

.prev_joy0dat:
		dc.w	0

.prev_keydat:
		dc.w	$FFFF

  if WEAPONSPRITES
.weapon_update_toggle:
		dc.w	0
  endif


	public	_IO_CheckMouse
_IO_CheckMouse:
											; 0xC0 - click
											; 0x30 - current state

	move.b	CIAA_PRA, d0						;	d0[7..6] = current state
	and.b	#$C0, d0						;
	eor.b	#$C0, d0						;
	
	move.b	d0, d1							;	.prev_mouse_state = d0	[deferred]
	lsr.b	#2, d0							;	d0[5..4] = d0[7..6]
	or.b	d1, d0							;
	eor.b	#$C0, d0						;	d0[7..6] = d0[7..6] & ~.prev_mouse_state[7..6]		= ~(~d0[7..6] | .prev_mouse_state[7..6])
	or.b	.prev_mouse_state, d0			;
	eor.b	#$C0, d0						;
	move.b	d1, .prev_mouse_state			;	[deferred - do now]

	move.b	_io_mouse_state, d1				;	io_mouse_state = (io_mouse_state & 0xC0) | d0;
	and.b	#$C0, d1						;
	or.b	d0, d1							;
	move.b	d1, _io_mouse_state				;

	rts

.prev_mouse_state:
		dc.w	0
