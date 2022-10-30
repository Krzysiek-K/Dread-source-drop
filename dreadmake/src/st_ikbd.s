



; ================================================================ Keyboard vars ================================================================


save_kbdvbase_addr:		dc.l	0
save_kb_statvec:		dc.l	0
save_kb_mousevec:		dc.l	0
save_kb_kbdsys:			dc.l	0

ikbd_command_buff:		dc.l	0

	public		_ikbd_rxbuff
_ikbd_rxbuff:			dc.b	0,0,0,0,0,0,0,0
_ikbd_rxbuff_pos:		dc.w	0

; ================================================================ _ikbd_install_handler ================================================================
	public		_ikbd_install_handler
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



; ================================================================ _ikbd_remove_handler ================================================================
	public		_ikbd_remove_handler
_ikbd_remove_handler:
	move.l	save_kbdvbase_addr, a0
	move.l	save_kb_statvec, kbdvbase_kb_statvec(a0)
	move.l	save_kb_mousevec, kbdvbase_kb_mouevec(a0)
	move.l	save_kb_kbdsys, kbdvbase_kb_kbdsys(a0)
	rts

; ================================================================ _ikbd_qwrite / _ikbd_write ================================================================
	public		_ikbd_qwrite
_ikbd_qwrite:
	lea		ikbd_command_buff, a0
	move.l	d1, (a0)

	public		_ikbd_write
_ikbd_write:
	move.l	a0, -(a7)				; A0: data address
	subq.w	#1, d0					; D0: data count
	move.w	d0, -(a7)
	move.w	#xbios_ikbdws, -(a7)	; write string to keyboatd
	trap	#trap_xbios
	add		#8, a7
	rts



; ================================================================ ikbd_recv ================================================================
ikbd_recv:
	move.w	#7, d0
	lea		_ikbd_rxbuff, a1
.loop:
	move.b	(a0)+, (a1)+
	dbra.w	d0, .loop
	rts



; ================================================================ ikbd_mouse ================================================================
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
