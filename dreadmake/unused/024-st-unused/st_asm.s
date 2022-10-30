; custom, minimal startup for C code
;(c)2011 Mariusz Buras (all), Pawel Goralski (modifications for C)

REDIRECT_OUTPUT_TO_SERIAL	equ	0	;0-output to console,1-output to serial port

	public	_start
	public	_memcpy
	public	_test
    public	_set_supervisor
	public	_set_usermode
	public	_c2p_asm

    xref	_main


BASEPAGE_SIZE 	equ $100
STACK_SIZE 	equ	$10000		
		

		section	code,code

; --------------------------------------------------------------
_start:
		move.l	4(sp),a5				;address to basepage
		move.l	$0c(a5),d0				;length of text segment
		add.l	$14(a5),d0				;length of data segment
		add.l	$1c(a5),d0				;length of bss segment
		add.l	#STACK_SIZE+BASEPAGE_SIZE,d0		;length of stackpointer+basepage
		move.l	a5,d1					;address to basepage
		add.l	d0,d1					;end of program
		and.l	#$fffffff0,d1				;align stack
		move.l	d1,sp					;new stackspace

		move.l	d0,-(sp)				;mshrink()
		move.l	a5,-(sp)				;
		clr.w	-(sp)					;
		move.w	#$4a,-(sp)				;
		trap	#1					;
		lea.l	12(sp),sp				;
				
;########################## redirect output to serial		
	if (REDIRECT_OUTPUT_TO_SERIAL==1)  
		; redirect to serial
		move.w #2,-(sp)
		move.w #1,-(sp)
		move.w #$46,-(sp)
		trap #1
		addq.l #6,sp
	endif
	
		jsr	_main

exit:	
		move.w #1,-(sp)
		trap #1
		addq.l #2,sp
		
		clr.w -(sp)
		trap #1
		
		
_basepage:	ds.l	1
_len:	ds.l	1

; --------------------------------------------------------------
_memcpy:	
	move.l	4(sp),a0
	move.l	8(sp),a1
	move.l	12(sp),d1

	lsr.l	#4,d1
	move.l	d1,d0
	swap.w	d0
	subq.w	#1,d1	
	bmi.b	.1
.2:
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+
	dbf		d1,.2
.1:
	subq.w	#1,d0
	bpl.b	.2

	move.l	12(sp),d1
	and.w	#$f,d1
	subq.w	#1,d1
	bmi.b	.3
.4:
	move.b	(a1)+,(a0)+
	dbf	d1,.4
.3:
	move.l	4(sp),d0
	rts




; --------------------------------------------------------------
_test:
      movem.l	d0-d2/a0-2,-(sp)
      move.l	#hello,-(sp)
      move.w	#$09,-(sp)
      trap	#1
      addq.l	#6,sp 	
      movem.l	(sp)+,d0-d2/a0-2
      
      rts
      


; --------------------------------------------------------------
save_stack:
	dc.l	0

_set_supervisor:
	clr.l	-(sp)
	move.w	#32,-(sp)
    trap	#1
	addq	#6,sp
	move.l	d0, save_stack

	;move.b	#$00, $FFFF8001
	;move.l	sp, d0
	;and     #$0fff, SR
	;move.l	d0, sp
    rts

_set_usermode:
	move.l	save_stack,-(sp)
	move.w	#32,-(sp)
	trap	#1
	addq.l	#6,sp
    rts

    xdef	_reset
_reset:
	reset
    rts



; --------------------------------------------------------------

	public		_st_install_vblank
	public		_st_remove_vblank


	xref		_intnum
	xref		_rawintnum



stvar_nvbls			equ		$454
stvar__vblqueue		equ		$456


;save_vbl:				dc.l	0

_st_install_vblank:
	;lea		$70, a0
	;move.l	(a0), save_vbl
	;move.l	#_int_vblank, (a0)

	move.l		stvar__vblqueue, a0
	move.w		stvar_nvbls, d0
	subq.l		#1, d0
.loop:
	move.l		(a0)+, d1
	cmp.l		#0, d1
	beq			.found
	dbra.w		d0, .loop
	rts

.found:
	lea			_int_vblank, a1
	move.l		a1, -(a0)
	rts


_st_remove_vblank:
	; TBD
	;lea		$70, a0
	;move.l	save_vbl, (a0)
	rts
	

_int_vblank:
	add.w		#1, _intnum						;	intnum++;
	add.w		#1, _rawintnum					;	rawintnum++;
	rts


; --------------------------------------------------------------

	public		_ikbd_rxbuff
	public		_ikbd_install_handler
	public		_ikbd_remove_handler
	public		_ikbd_write
	public		_ikbd_qwrite

	xref		_debug_vars
	xref		_io_mouse_xdelta
	xref		_io_keystate
	xref		_io_key_last
	xref		_io_mouse_state


trap_xbios				equ		14
xbios_ikbdws			equ		25
xbios_kbdvbase			equ		34

kbdvbase_kb_statvec		equ		12
kbdvbase_kb_mouevec		equ		16
kbdvbase_kb_kbdsys		equ		32

save_kbdvbase_addr:		dc.l	0
save_kb_statvec:		dc.l	0
save_kb_mousevec:		dc.l	0
save_kb_kbdsys:			dc.l	0

ikbd_command_buff:		dc.l	0

_ikbd_rxbuff:			dc.b	0,0,0,0,0,0,0,0
_ikbd_rxbuff_pos:		dc.w	0

_ikbd_install_handler:
	move.w	#xbios_kbdvbase, -(a7)		; A0 = address of IKBD vector table (KBDVBASE)
	trap	#trap_xbios
	add		#2, a7
	move.l	d0, a0
	
	move.l	a0, save_kbdvbase_addr							; save address and old value
	move.l	kbdvbase_kb_statvec(a0), save_kb_statvec
	move.l	kbdvbase_kb_mouevec(a0), save_kb_mousevec
	move.l	kbdvbase_kb_kbdsys(a0), save_kb_kbdsys

	move.l	#ikbd_recv, kbdvbase_kb_statvec(a0)
	move.l	#ikbd_mouse, kbdvbase_kb_mouevec(a0)
	move.l	#ikbd_handler, kbdvbase_kb_kbdsys(a0)

	rts


_ikbd_remove_handler:
	move.l	save_kbdvbase_addr, a0
	move.l	save_kb_statvec, kbdvbase_kb_statvec(a0)
	move.l	save_kb_mousevec, kbdvbase_kb_mouevec(a0)
	move.l	save_kb_kbdsys, kbdvbase_kb_kbdsys(a0)
	rts

_ikbd_qwrite:
	lea		ikbd_command_buff, a0
	move.l	d1, (a0)

_ikbd_write:
	move.l	a0, -(a7)				; A0: data address
	subq.w	#1, d0					; D0: data count
	move.w	d0, -(a7)
	move.w	#xbios_ikbdws, -(a7)	; write string to keyboatd
	trap	#trap_xbios
	add		#8, a7
	rts

ikbd_recv:
	move.w	#7, d0
	lea		_ikbd_rxbuff, a1
.loop:
	move.b	(a0)+, (a1)+
	dbra.w	d0, .loop
	rts

ikbd_mouse:
											; 0xC0 - click
											; 0x30 - current state

	move.b	(a0), d0						;	d0[7..6] = current state
	lsl.b	#5, d0							;		d0 = ?ab-----
	move.b	d0, d1							;		d1 =			?ab-----
	and.b	#$40, d1						;		d1 =			-a------
	lsl.b	#2, d0							;		d0 = b-------
	or.b	d1, d0							;		d0 = ba------
	
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


	move.b	1(a0), d0						; 1(a0)		- mouse X step
	ext.w	d0
	ext.l	d0
	lsl.l	#2, d0
	add.w	d0, _io_mouse_xdelta

	;bra		ikbd_recv				; save buffer contents
	move.w	#7, d0
	lea		_ikbd_rxbuff, a1
.loop:
	move.b	(a0)+, (a1)+
	dbra.w	d0, .loop
	rts

.prev_mouse_state:
	dc.w	0


ikbd_handler:
	move.b		$fffc02, d0

	cmp.b		#$F8, d0
	blo			.key_event
	cmp.b		#$FC, d0
	blo			.mouse_event
	; unknown event
	rts

.key_event:
	cmp.b		#$80, d0
	bhs			.key_event_release

	and.w		#$007F, d0					; keypress
	lea			_io_keystate, a0
	move.b		#1, (a0,d0.w)
	move.b		d0, _io_key_last
	rts

.key_event_release:
	and.w		#$007F, d0
	lea			_io_keystate, a0
	move.b		#0, (a0,d0.w)
	rts


.save_d0_to_buff:
	move.w		_ikbd_rxbuff_pos, d1		; save d0 in rxbuff
	lea			_ikbd_rxbuff, a0
	move.b		d0, (a0,d1.w)
	addq.l		#1, d1						; increment buffer write pos (mod 8)
	and.w		#7, d1
	move.w		d1, _ikbd_rxbuff_pos
	rts



.mouse_event:
											; 0xC0 - click
											; 0x30 - current state

											;	d0[7..6] = current state
	lsl.b	#5, d0							;		d0 = ?ab-----
	move.b	d0, d1							;		d1 =			?ab-----
	and.b	#$40, d1						;		d1 =			-a------
	lsl.b	#2, d0							;		d0 = b-------
	or.b	d1, d0							;		d0 = ba------
	
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

	move.l		save_kbdvbase_addr, a0
	move.l		#ikbd_handler_mouse_dx, kbdvbase_kb_kbdsys(a0)
	rts

.prev_mouse_state:
	dc.w	0


ikbd_handler_mouse_dx:
	move.b		$fffc02, d0					; 1(a0)		- mouse X step
	ext.w		d0
	ext.l		d0
	lsl.l		#2, d0
	add.w		d0, _io_mouse_xdelta

	move.l		save_kbdvbase_addr, a0
	move.l		#ikbd_handler_mouse_dy, kbdvbase_kb_kbdsys(a0)
	rts

ikbd_handler_mouse_dy:
	move.b		$fffc02, d0
	move.l		save_kbdvbase_addr, a0
	move.l		#ikbd_handler, kbdvbase_kb_kbdsys(a0)
	rts



; --------------------------------------------------------------

C2P_FIX:	macro
	;lsl.w		#2, \1
	;and.w		#$3C3C, \1
  endm

C2P_CORE:	macro	line_num
	move.l		(a4)+, d1
	C2P_FIX		d1
	move.l		(a6,d1.w), d0
	move.l		(a3)+, d4
	C2P_FIX		d4
	or.l		(a5,d4.w), d0
	movep.l		d0, 1+320*\1+320(a1)
	movep.l		d0, 161+320*\1+320(a1)

	move.l		(a2)+, d5
	C2P_FIX		d5
	move.l		(a6,d5.w), d0
	move.l		(a0)+, d6
	C2P_FIX		d6
	or.l		(a5,d6.w), d0
	movep.l		d0, 0+320*\1+320(a1)
	movep.l		d0, 160+320*\1+320(a1)

	swap		d1
	C2P_FIX		d1
	move.l		(a6,d1.w), d0
	swap		d4
	C2P_FIX		d4
	or.l		(a5,d4.w), d0
	movep.l		d0, 1+320*\1(a1)
	movep.l		d0, 161+320*\1(a1)

	swap		d5
	C2P_FIX		d5
	move.l		(a6,d5.w), d0
	swap		d6
	C2P_FIX		d6
	or.l		(a5,d6.w), d0
	movep.l		d0, 0+320*\1(a1)
	movep.l		d0, 160+320*\1(a1)
endm


; --------------------------------------------------------------
;	a0	- render_screen
;	a1	- screen
;	a2	- (render_screen+200)
;	a3	- (render_screen+400)
;	a4	- (render_screen+600)
;	a5	- c2p_lut
;	a6	- c2p_lut2
;
;	d0	- (temp)
;	d1	- (temp)
;	d2	- X counter
;	d3	- Y counter
;
_c2p_asm:
	movem.l		d0-d7/a0-a6,-(a7)
	
	move.l		#$FFFF0000, d0

;	add.l		#16, a0

	move.w		#19, d2
.xloop:

	lea			200(a0), a2
	lea			400(a0), a3
	lea			600(a0), a4

;	move.w		#99, d3
;.yloop:
;
;	move.w		600(a0), d1
;	move.l		(a6,d1.w), d0
;	move.w		400(a0), d1
;	or.l		(a5,d1.w), d0
;	movep.l		d0, 1(a1)
;	movep.l		d0, 161(a1)
;
;	move.w		200(a0), d1
;	move.l		(a6,d1.w), d0
;	move.w		(a0)+, d1
;	or.l		(a5,d1.w), d0
;	movep.l		d0, (a1)
;	movep.l		d0, 160(a1)
;
;
;	add.l		#320, a1			; (320/16)*4*4
;
;	; --- next Y ---
;	dbra.w		d3, .yloop


	C2P_CORE	 0
	C2P_CORE	 2
	C2P_CORE	 4
	C2P_CORE	 6
	C2P_CORE	 8
	C2P_CORE	10
	C2P_CORE	12
	C2P_CORE	14
	C2P_CORE	16
	C2P_CORE	18
	C2P_CORE	20
	C2P_CORE	22
	C2P_CORE	24
	C2P_CORE	26
	C2P_CORE	28
	C2P_CORE	30
	C2P_CORE	32
	C2P_CORE	34
	C2P_CORE	36
	C2P_CORE	38
	C2P_CORE	40
	C2P_CORE	42
	C2P_CORE	44
	C2P_CORE	46
	C2P_CORE	48
	C2P_CORE	50
	C2P_CORE	52
	C2P_CORE	54
	C2P_CORE	56
	C2P_CORE	58
	C2P_CORE	60
	C2P_CORE	62
	C2P_CORE	64
	C2P_CORE	66
	C2P_CORE	68
	C2P_CORE	70
	C2P_CORE	72
	C2P_CORE	74
	C2P_CORE	76
	C2P_CORE	78
	C2P_CORE	80
	C2P_CORE	82
	C2P_CORE	84
	C2P_CORE	86
	C2P_CORE	88
	C2P_CORE	90
	C2P_CORE	92
	C2P_CORE	94
	C2P_CORE	96
	C2P_CORE	98


	; --- next X ---
	;add.l		#600+32, a0
	add.l		#600, a0
	add.l		#8, a1
	dbra.w		d2, .xloop


	movem.l		(a7)+,d0-d7/a0-a6
	rts




	public _Sound_PlayThingSound
_Sound_PlayThingSound:
	rts



     section	data
hello: 
      dc.b 'Hello world from m68k asm',$0d,$0a,$00 



TRAP_ICACHE_DISABLE: macro
	endm

TRAP_ICACHE_ENABLE: macro
	endm


	include	"dread_asm.s"
