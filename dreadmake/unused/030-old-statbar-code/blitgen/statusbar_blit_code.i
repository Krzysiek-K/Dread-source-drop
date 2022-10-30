
.check_ammo_100:
	move.w	_e_status_bar+statbar_ammo_100, d0	; check ammo_100
	cmp.w	_e_status_bar_shadow+statbar_ammo_100, d0
	beq		.check_ammo_10

	; Clear: ammo_100, ammo_10, ammo_1
												; Stat_Clear( 4, 4, 39, 16 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$0FFFFFE0, BLTAFWM(a1)				; BLTAFWM = 0x0FFF
												; BLTALWM = 0xFFE0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00220022, BLTCMOD(a1)				; BLTCMOD = 34
												; BLTBMOD = 34
	move.w	#34, BLTDMOD(a1)					; BLTDMOD = 34
	move.l	#_GFX_STBAR0 + 646, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 646
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 640
	add.l	#640, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 3), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 3)

	lea.l	.draw_ammo_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_ammo_10:
	move.w	_e_status_bar+statbar_ammo_10, d0	; check ammo_10
	cmp.w	_e_status_bar_shadow+statbar_ammo_10, d0
	beq		.check_ammo_1

	; Clear: ammo_10, ammo_1
												; Stat_Clear( 17, 4, 26, 16 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$7FFFFFE0, BLTAFWM(a1)				; BLTAFWM = 0x7FFF
												; BLTALWM = 0xFFE0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 648, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 648
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 642
	add.l	#642, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 2)

	lea.l	.draw_ammo_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_ammo_1:
	move.w	_e_status_bar+statbar_ammo_1, d0	; check ammo_1
	cmp.w	_e_status_bar_shadow+statbar_ammo_1, d0
	beq		.check_health_prc

	; Clear: ammo_1
												; Stat_Clear( 30, 4, 13, 16 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$0003FFE0, BLTAFWM(a1)				; BLTAFWM = 0x0003
												; BLTALWM = 0xFFE0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 648, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 648
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 642
	add.l	#642, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 2)

	lea.l	.draw_ammo_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_ammo_100:
	move.w	_e_status_bar+statbar_ammo_100, d0	; draw ammo_100
	move.w	d0, _e_status_bar_shadow+statbar_ammo_100
	beq		.draw_ammo_10
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 4, 4, 13, 16 );
	move.l	#$4FCA4000, BLTCON0(a1)				; BLTCON0 = 0x4FCA
												; BLTCON1 = 0x4000
	move.l	#$FFFF0000, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0x0000
	move.l	#$0024FFFE, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = -2
	move.l	#$FFFE0024, BLTAMOD(a1)				; BLTAMOD = -2
												; BLTDMOD = 36
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#128, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 640
	add.l	#640, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 2)

	lea.l	.draw_ammo_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_ammo_10:
	move.w	_e_status_bar+statbar_ammo_10, d0	; draw ammo_10
	move.w	d0, _e_status_bar_shadow+statbar_ammo_10
	beq		.draw_ammo_1
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 17, 4, 13, 16 );
	move.l	#$1FCA1000, BLTCON0(a1)				; BLTCON0 = 0x1FCA
												; BLTCON1 = 0x1000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#128, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 642
	add.l	#642, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 1)

	lea.l	.draw_ammo_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_ammo_1:
	move.w	_e_status_bar+statbar_ammo_1, d0	; draw ammo_1
	move.w	d0, _e_status_bar_shadow+statbar_ammo_1
	beq		.check_health_prc
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 30, 4, 13, 16 );
	move.l	#$EFCAE000, BLTCON0(a1)				; BLTCON0 = 0xEFCA
												; BLTCON1 = 0xE000
	move.l	#$FFFF0000, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0x0000
	move.l	#$0024FFFE, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = -2
	move.l	#$FFFE0024, BLTAMOD(a1)				; BLTAMOD = -2
												; BLTDMOD = 36
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#128, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 642
	add.l	#642, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 2)

	lea.l	.check_health_prc(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_health_prc:
	move.w	_e_status_bar+statbar_health_prc, d0; check health_prc
	cmp.w	_e_status_bar_shadow+statbar_health_prc, d0
	beq		.check_health_100

	; Clear: health_prc, health_100, health_10, health_1
												; Stat_Clear( 51, 4, 52, 16 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$1FFFFE00, BLTAFWM(a1)				; BLTAFWM = 0x1FFF
												; BLTALWM = 0xFE00
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00200020, BLTCMOD(a1)				; BLTCMOD = 32
												; BLTBMOD = 32
	move.w	#32, BLTDMOD(a1)					; BLTDMOD = 32
	move.l	#_GFX_STBAR0 + 652, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 652
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 646
	add.l	#646, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 4), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 4)

	lea.l	.draw_health_prc(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_health_100:
	move.w	_e_status_bar+statbar_health_100, d0; check health_100
	cmp.w	_e_status_bar_shadow+statbar_health_100, d0
	beq		.check_health_10

	; Clear: health_100, health_10, health_1
												; Stat_Clear( 51, 4, 39, 16 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$1FFFFFC0, BLTAFWM(a1)				; BLTAFWM = 0x1FFF
												; BLTALWM = 0xFFC0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00220022, BLTCMOD(a1)				; BLTCMOD = 34
												; BLTBMOD = 34
	move.w	#34, BLTDMOD(a1)					; BLTDMOD = 34
	move.l	#_GFX_STBAR0 + 652, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 652
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 646
	add.l	#646, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 3), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 3)

	lea.l	.draw_health_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_health_10:
	move.w	_e_status_bar+statbar_health_10, d0	; check health_10
	cmp.w	_e_status_bar_shadow+statbar_health_10, d0
	beq		.check_health_1

	; Clear: health_10, health_1
												; Stat_Clear( 64, 4, 26, 16 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$FFFFFFC0, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFC0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 654, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 654
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 648
	add.l	#648, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 2)

	lea.l	.draw_health_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_health_1:
	move.w	_e_status_bar+statbar_health_1, d0	; check health_1
	cmp.w	_e_status_bar_shadow+statbar_health_1, d0
	beq		.check_armor_prc

	; Clear: health_1
												; Stat_Clear( 77, 4, 13, 16 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$0007FFC0, BLTAFWM(a1)				; BLTAFWM = 0x0007
												; BLTALWM = 0xFFC0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 654, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 654
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 648
	add.l	#648, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 2)

	lea.l	.draw_health_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_health_prc:
	move.w	_e_status_bar+statbar_health_prc, d0; draw health_prc
	move.w	d0, _e_status_bar_shadow+statbar_health_prc
	beq		.draw_health_100
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 90, 4, 13, 16 );
	move.l	#$AFCAA000, BLTCON0(a1)				; BLTCON0 = 0xAFCA
												; BLTCON1 = 0xA000
	move.l	#$FFFF0000, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0x0000
	move.l	#$0024FFFE, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = -2
	move.l	#$FFFE0024, BLTAMOD(a1)				; BLTAMOD = -2
												; BLTDMOD = 36
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#128, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 650
	add.l	#650, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 2)

	lea.l	.draw_health_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_health_100:
	move.w	_e_status_bar+statbar_health_100, d0; draw health_100
	move.w	d0, _e_status_bar_shadow+statbar_health_100
	beq		.draw_health_10
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 51, 4, 13, 16 );
	move.l	#$3FCA3000, BLTCON0(a1)				; BLTCON0 = 0x3FCA
												; BLTCON1 = 0x3000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#128, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 646
	add.l	#646, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 1)

	lea.l	.draw_health_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_health_10:
	move.w	_e_status_bar+statbar_health_10, d0	; draw health_10
	move.w	d0, _e_status_bar_shadow+statbar_health_10
	beq		.draw_health_1
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 64, 4, 13, 16 );
	move.l	#$0FCA0000, BLTCON0(a1)				; BLTCON0 = 0x0FCA
												; BLTCON1 = 0x0000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#128, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 648
	add.l	#648, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 1)

	lea.l	.draw_health_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_health_1:
	move.w	_e_status_bar+statbar_health_1, d0	; draw health_1
	move.w	d0, _e_status_bar_shadow+statbar_health_1
	beq		.check_armor_prc
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 77, 4, 13, 16 );
	move.l	#$DFCAD000, BLTCON0(a1)				; BLTCON0 = 0xDFCA
												; BLTCON1 = 0xD000
	move.l	#$FFFF0000, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0x0000
	move.l	#$0024FFFE, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = -2
	move.l	#$FFFE0024, BLTAMOD(a1)				; BLTAMOD = -2
												; BLTDMOD = 36
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#128, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 648
	add.l	#648, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 2)

	lea.l	.check_armor_prc(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_armor_prc:
	move.w	_e_status_bar+statbar_armor_prc, d0	; check armor_prc
	cmp.w	_e_status_bar_shadow+statbar_armor_prc, d0
	beq		.check_armor_100

	; Clear: armor_prc, armor_100, armor_10, armor_1
												; Stat_Clear( 182, 4, 52, 16 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$03FFFFC0, BLTAFWM(a1)				; BLTAFWM = 0x03FF
												; BLTALWM = 0xFFC0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00200020, BLTCMOD(a1)				; BLTCMOD = 32
												; BLTBMOD = 32
	move.w	#32, BLTDMOD(a1)					; BLTDMOD = 32
	move.l	#_GFX_STBAR0 + 668, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 668
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 662
	add.l	#662, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 4), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 4)

	lea.l	.draw_armor_prc(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_armor_100:
	move.w	_e_status_bar+statbar_armor_100, d0	; check armor_100
	cmp.w	_e_status_bar_shadow+statbar_armor_100, d0
	beq		.check_armor_10

	; Clear: armor_100, armor_10, armor_1
												; Stat_Clear( 182, 4, 39, 16 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$03FFFFF8, BLTAFWM(a1)				; BLTAFWM = 0x03FF
												; BLTALWM = 0xFFF8
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00220022, BLTCMOD(a1)				; BLTCMOD = 34
												; BLTBMOD = 34
	move.w	#34, BLTDMOD(a1)					; BLTDMOD = 34
	move.l	#_GFX_STBAR0 + 668, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 668
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 662
	add.l	#662, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 3), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 3)

	lea.l	.draw_armor_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_armor_10:
	move.w	_e_status_bar+statbar_armor_10, d0	; check armor_10
	cmp.w	_e_status_bar_shadow+statbar_armor_10, d0
	beq		.check_armor_1

	; Clear: armor_10, armor_1
												; Stat_Clear( 195, 4, 26, 16 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$1FFFFFF8, BLTAFWM(a1)				; BLTAFWM = 0x1FFF
												; BLTALWM = 0xFFF8
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 670, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 670
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 664
	add.l	#664, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 2)

	lea.l	.draw_armor_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_armor_1:
	move.w	_e_status_bar+statbar_armor_1, d0	; check armor_1
	cmp.w	_e_status_bar_shadow+statbar_armor_1, d0
	beq		.check_bullets_100

	; Clear: armor_1
												; Stat_Clear( 208, 4, 13, 16 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$FFFFFFF8, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFF8
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00260026, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 38
	move.w	#38, BLTDMOD(a1)					; BLTDMOD = 38
	move.l	#_GFX_STBAR0 + 672, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 672
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 666
	add.l	#666, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 1)

	lea.l	.draw_armor_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_armor_prc:
	move.w	_e_status_bar+statbar_armor_prc, d0	; draw armor_prc
	move.w	d0, _e_status_bar_shadow+statbar_armor_prc
	beq		.draw_armor_100
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 221, 4, 13, 16 );
	move.l	#$DFCAD000, BLTCON0(a1)				; BLTCON0 = 0xDFCA
												; BLTCON1 = 0xD000
	move.l	#$FFFF0000, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0x0000
	move.l	#$0024FFFE, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = -2
	move.l	#$FFFE0024, BLTAMOD(a1)				; BLTAMOD = -2
												; BLTDMOD = 36
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#128, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 666
	add.l	#666, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 2)

	lea.l	.draw_armor_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_armor_100:
	move.w	_e_status_bar+statbar_armor_100, d0	; draw armor_100
	move.w	d0, _e_status_bar_shadow+statbar_armor_100
	beq		.draw_armor_10
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 182, 4, 13, 16 );
	move.l	#$6FCA6000, BLTCON0(a1)				; BLTCON0 = 0x6FCA
												; BLTCON1 = 0x6000
	move.l	#$FFFF0000, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0x0000
	move.l	#$0024FFFE, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = -2
	move.l	#$FFFE0024, BLTAMOD(a1)				; BLTAMOD = -2
												; BLTDMOD = 36
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#128, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 662
	add.l	#662, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 2)

	lea.l	.draw_armor_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_armor_10:
	move.w	_e_status_bar+statbar_armor_10, d0	; draw armor_10
	move.w	d0, _e_status_bar_shadow+statbar_armor_10
	beq		.draw_armor_1
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 195, 4, 13, 16 );
	move.l	#$3FCA3000, BLTCON0(a1)				; BLTCON0 = 0x3FCA
												; BLTCON1 = 0x3000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#128, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 664
	add.l	#664, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 1)

	lea.l	.draw_armor_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_armor_1:
	move.w	_e_status_bar+statbar_armor_1, d0	; draw armor_1
	move.w	d0, _e_status_bar_shadow+statbar_armor_1
	beq		.check_bullets_100
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 208, 4, 13, 16 );
	move.l	#$0FCA0000, BLTCON0(a1)				; BLTCON0 = 0x0FCA
												; BLTCON1 = 0x0000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#128, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 666
	add.l	#666, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((64<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((64<<6) | 1)

	lea.l	.check_bullets_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_bullets_100:
	move.w	_e_status_bar+statbar_bullets_100, d0; check bullets_100
	cmp.w	_e_status_bar_shadow+statbar_bullets_100, d0
	beq		.check_bullets_10

	; Clear: bullets_100, bullets_10, bullets_1
												; Stat_Clear( 279, 5, 12, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$01FFE000, BLTAFWM(a1)				; BLTAFWM = 0x01FF
												; BLTALWM = 0xE000
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 840, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 840
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 834
	add.l	#834, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_bullets_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_bullets_10:
	move.w	_e_status_bar+statbar_bullets_10, d0; check bullets_10
	cmp.w	_e_status_bar_shadow+statbar_bullets_10, d0
	beq		.check_bullets_1

	; Clear: bullets_10, bullets_1
												; Stat_Clear( 283, 5, 8, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$001FE000, BLTAFWM(a1)				; BLTAFWM = 0x001F
												; BLTALWM = 0xE000
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 840, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 840
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 834
	add.l	#834, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_bullets_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_bullets_1:
	move.w	_e_status_bar+statbar_bullets_1, d0	; check bullets_1
	cmp.w	_e_status_bar_shadow+statbar_bullets_1, d0
	beq		.check_shells_100

	; Clear: bullets_1
												; Stat_Clear( 287, 5, 4, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$0001E000, BLTAFWM(a1)				; BLTAFWM = 0x0001
												; BLTALWM = 0xE000
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 840, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 840
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 834
	add.l	#834, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_bullets_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_bullets_100:
	move.w	_e_status_bar+statbar_bullets_100, d0; draw bullets_100
	move.w	d0, _e_status_bar_shadow+statbar_bullets_100
	beq		.draw_bullets_10
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 279, 5, 4, 6 );
	move.l	#$7FCA7000, BLTCON0(a1)				; BLTCON0 = 0x7FCA
												; BLTCON1 = 0x7000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 834
	add.l	#834, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_bullets_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_bullets_10:
	move.w	_e_status_bar+statbar_bullets_10, d0; draw bullets_10
	move.w	d0, _e_status_bar_shadow+statbar_bullets_10
	beq		.draw_bullets_1
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 283, 5, 4, 6 );
	move.l	#$BFCAB000, BLTCON0(a1)				; BLTCON0 = 0xBFCA
												; BLTCON1 = 0xB000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 834
	add.l	#834, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_bullets_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_bullets_1:
	move.w	_e_status_bar+statbar_bullets_1, d0	; draw bullets_1
	move.w	d0, _e_status_bar_shadow+statbar_bullets_1
	beq		.check_shells_100
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 287, 5, 4, 6 );
	move.l	#$FFCAF000, BLTCON0(a1)				; BLTCON0 = 0xFFCA
												; BLTCON1 = 0xF000
	move.l	#$FFFF0000, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0x0000
	move.l	#$0024FFFE, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = -2
	move.l	#$FFFE0024, BLTAMOD(a1)				; BLTAMOD = -2
												; BLTDMOD = 36
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 834
	add.l	#834, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.check_shells_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_shells_100:
	move.w	_e_status_bar+statbar_shells_100, d0; check shells_100
	cmp.w	_e_status_bar_shadow+statbar_shells_100, d0
	beq		.check_shells_10

	; Clear: shells_100, shells_10, shells_1
												; Stat_Clear( 279, 11, 12, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$01FFE000, BLTAFWM(a1)				; BLTAFWM = 0x01FF
												; BLTALWM = 0xE000
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 1800, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 1800
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 1794
	add.l	#1794, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_shells_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_shells_10:
	move.w	_e_status_bar+statbar_shells_10, d0	; check shells_10
	cmp.w	_e_status_bar_shadow+statbar_shells_10, d0
	beq		.check_shells_1

	; Clear: shells_10, shells_1
												; Stat_Clear( 283, 11, 8, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$001FE000, BLTAFWM(a1)				; BLTAFWM = 0x001F
												; BLTALWM = 0xE000
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 1800, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 1800
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 1794
	add.l	#1794, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_shells_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_shells_1:
	move.w	_e_status_bar+statbar_shells_1, d0	; check shells_1
	cmp.w	_e_status_bar_shadow+statbar_shells_1, d0
	beq		.check_rockets_100

	; Clear: shells_1
												; Stat_Clear( 287, 11, 4, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$0001E000, BLTAFWM(a1)				; BLTAFWM = 0x0001
												; BLTALWM = 0xE000
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 1800, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 1800
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 1794
	add.l	#1794, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_shells_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_shells_100:
	move.w	_e_status_bar+statbar_shells_100, d0; draw shells_100
	move.w	d0, _e_status_bar_shadow+statbar_shells_100
	beq		.draw_shells_10
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 279, 11, 4, 6 );
	move.l	#$7FCA7000, BLTCON0(a1)				; BLTCON0 = 0x7FCA
												; BLTCON1 = 0x7000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 1794
	add.l	#1794, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_shells_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_shells_10:
	move.w	_e_status_bar+statbar_shells_10, d0	; draw shells_10
	move.w	d0, _e_status_bar_shadow+statbar_shells_10
	beq		.draw_shells_1
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 283, 11, 4, 6 );
	move.l	#$BFCAB000, BLTCON0(a1)				; BLTCON0 = 0xBFCA
												; BLTCON1 = 0xB000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 1794
	add.l	#1794, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_shells_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_shells_1:
	move.w	_e_status_bar+statbar_shells_1, d0	; draw shells_1
	move.w	d0, _e_status_bar_shadow+statbar_shells_1
	beq		.check_rockets_100
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 287, 11, 4, 6 );
	move.l	#$FFCAF000, BLTCON0(a1)				; BLTCON0 = 0xFFCA
												; BLTCON1 = 0xF000
	move.l	#$FFFF0000, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0x0000
	move.l	#$0024FFFE, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = -2
	move.l	#$FFFE0024, BLTAMOD(a1)				; BLTAMOD = -2
												; BLTDMOD = 36
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 1794
	add.l	#1794, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.check_rockets_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_rockets_100:
	move.w	_e_status_bar+statbar_rockets_100, d0; check rockets_100
	cmp.w	_e_status_bar_shadow+statbar_rockets_100, d0
	beq		.check_rockets_10

	; Clear: rockets_100, rockets_10, rockets_1
												; Stat_Clear( 279, 17, 12, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$01FFE000, BLTAFWM(a1)				; BLTAFWM = 0x01FF
												; BLTALWM = 0xE000
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 2760, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 2760
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 2754
	add.l	#2754, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_rockets_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_rockets_10:
	move.w	_e_status_bar+statbar_rockets_10, d0; check rockets_10
	cmp.w	_e_status_bar_shadow+statbar_rockets_10, d0
	beq		.check_rockets_1

	; Clear: rockets_10, rockets_1
												; Stat_Clear( 283, 17, 8, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$001FE000, BLTAFWM(a1)				; BLTAFWM = 0x001F
												; BLTALWM = 0xE000
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 2760, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 2760
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 2754
	add.l	#2754, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_rockets_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_rockets_1:
	move.w	_e_status_bar+statbar_rockets_1, d0	; check rockets_1
	cmp.w	_e_status_bar_shadow+statbar_rockets_1, d0
	beq		.check_cells_100

	; Clear: rockets_1
												; Stat_Clear( 287, 17, 4, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$0001E000, BLTAFWM(a1)				; BLTAFWM = 0x0001
												; BLTALWM = 0xE000
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 2760, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 2760
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 2754
	add.l	#2754, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_rockets_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_rockets_100:
	move.w	_e_status_bar+statbar_rockets_100, d0; draw rockets_100
	move.w	d0, _e_status_bar_shadow+statbar_rockets_100
	beq		.draw_rockets_10
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 279, 17, 4, 6 );
	move.l	#$7FCA7000, BLTCON0(a1)				; BLTCON0 = 0x7FCA
												; BLTCON1 = 0x7000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 2754
	add.l	#2754, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_rockets_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_rockets_10:
	move.w	_e_status_bar+statbar_rockets_10, d0; draw rockets_10
	move.w	d0, _e_status_bar_shadow+statbar_rockets_10
	beq		.draw_rockets_1
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 283, 17, 4, 6 );
	move.l	#$BFCAB000, BLTCON0(a1)				; BLTCON0 = 0xBFCA
												; BLTCON1 = 0xB000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 2754
	add.l	#2754, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_rockets_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_rockets_1:
	move.w	_e_status_bar+statbar_rockets_1, d0	; draw rockets_1
	move.w	d0, _e_status_bar_shadow+statbar_rockets_1
	beq		.check_cells_100
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 287, 17, 4, 6 );
	move.l	#$FFCAF000, BLTCON0(a1)				; BLTCON0 = 0xFFCA
												; BLTCON1 = 0xF000
	move.l	#$FFFF0000, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0x0000
	move.l	#$0024FFFE, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = -2
	move.l	#$FFFE0024, BLTAMOD(a1)				; BLTAMOD = -2
												; BLTDMOD = 36
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 2754
	add.l	#2754, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.check_cells_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_cells_100:
	move.w	_e_status_bar+statbar_cells_100, d0	; check cells_100
	cmp.w	_e_status_bar_shadow+statbar_cells_100, d0
	beq		.check_cells_10

	; Clear: cells_100, cells_10, cells_1
												; Stat_Clear( 279, 23, 12, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$01FFE000, BLTAFWM(a1)				; BLTAFWM = 0x01FF
												; BLTALWM = 0xE000
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 3720, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 3720
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 3714
	add.l	#3714, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_cells_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_cells_10:
	move.w	_e_status_bar+statbar_cells_10, d0	; check cells_10
	cmp.w	_e_status_bar_shadow+statbar_cells_10, d0
	beq		.check_cells_1

	; Clear: cells_10, cells_1
												; Stat_Clear( 283, 23, 8, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$001FE000, BLTAFWM(a1)				; BLTAFWM = 0x001F
												; BLTALWM = 0xE000
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 3720, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 3720
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 3714
	add.l	#3714, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_cells_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_cells_1:
	move.w	_e_status_bar+statbar_cells_1, d0	; check cells_1
	cmp.w	_e_status_bar_shadow+statbar_cells_1, d0
	beq		.check_cap_bullets_100

	; Clear: cells_1
												; Stat_Clear( 287, 23, 4, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$0001E000, BLTAFWM(a1)				; BLTAFWM = 0x0001
												; BLTALWM = 0xE000
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 3720, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 3720
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 3714
	add.l	#3714, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_cells_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_cells_100:
	move.w	_e_status_bar+statbar_cells_100, d0	; draw cells_100
	move.w	d0, _e_status_bar_shadow+statbar_cells_100
	beq		.draw_cells_10
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 279, 23, 4, 6 );
	move.l	#$7FCA7000, BLTCON0(a1)				; BLTCON0 = 0x7FCA
												; BLTCON1 = 0x7000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 3714
	add.l	#3714, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_cells_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_cells_10:
	move.w	_e_status_bar+statbar_cells_10, d0	; draw cells_10
	move.w	d0, _e_status_bar_shadow+statbar_cells_10
	beq		.draw_cells_1
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 283, 23, 4, 6 );
	move.l	#$BFCAB000, BLTCON0(a1)				; BLTCON0 = 0xBFCA
												; BLTCON1 = 0xB000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 3714
	add.l	#3714, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_cells_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_cells_1:
	move.w	_e_status_bar+statbar_cells_1, d0	; draw cells_1
	move.w	d0, _e_status_bar_shadow+statbar_cells_1
	beq		.check_cap_bullets_100
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 287, 23, 4, 6 );
	move.l	#$FFCAF000, BLTCON0(a1)				; BLTCON0 = 0xFFCA
												; BLTCON1 = 0xF000
	move.l	#$FFFF0000, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0x0000
	move.l	#$0024FFFE, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = -2
	move.l	#$FFFE0024, BLTAMOD(a1)				; BLTAMOD = -2
												; BLTDMOD = 36
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 3714
	add.l	#3714, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.check_cap_bullets_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_cap_bullets_100:
	move.w	_e_status_bar+statbar_cap_bullets_100, d0; check cap_bullets_100
	cmp.w	_e_status_bar_shadow+statbar_cap_bullets_100, d0
	beq		.check_cap_bullets_10

	; Clear: cap_bullets_100, cap_bullets_10, cap_bullets_1
												; Stat_Clear( 302, 5, 12, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$0003FFC0, BLTAFWM(a1)				; BLTAFWM = 0x0003
												; BLTALWM = 0xFFC0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 842, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 842
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 836
	add.l	#836, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_cap_bullets_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_cap_bullets_10:
	move.w	_e_status_bar+statbar_cap_bullets_10, d0; check cap_bullets_10
	cmp.w	_e_status_bar_shadow+statbar_cap_bullets_10, d0
	beq		.check_cap_bullets_1

	; Clear: cap_bullets_10, cap_bullets_1
												; Stat_Clear( 306, 5, 8, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$3FFFFFC0, BLTAFWM(a1)				; BLTAFWM = 0x3FFF
												; BLTALWM = 0xFFC0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00260026, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 38
	move.w	#38, BLTDMOD(a1)					; BLTDMOD = 38
	move.l	#_GFX_STBAR0 + 844, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 844
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 838
	add.l	#838, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_cap_bullets_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_cap_bullets_1:
	move.w	_e_status_bar+statbar_cap_bullets_1, d0; check cap_bullets_1
	cmp.w	_e_status_bar_shadow+statbar_cap_bullets_1, d0
	beq		.check_cap_shells_100

	; Clear: cap_bullets_1
												; Stat_Clear( 310, 5, 4, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$03FFFFC0, BLTAFWM(a1)				; BLTAFWM = 0x03FF
												; BLTALWM = 0xFFC0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00260026, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 38
	move.w	#38, BLTDMOD(a1)					; BLTDMOD = 38
	move.l	#_GFX_STBAR0 + 844, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 844
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 838
	add.l	#838, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_cap_bullets_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_cap_bullets_100:
	move.w	_e_status_bar+statbar_cap_bullets_100, d0; draw cap_bullets_100
	move.w	d0, _e_status_bar_shadow+statbar_cap_bullets_100
	beq		.draw_cap_bullets_10
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 302, 5, 4, 6 );
	move.l	#$EFCAE000, BLTCON0(a1)				; BLTCON0 = 0xEFCA
												; BLTCON1 = 0xE000
	move.l	#$FFFF0000, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0x0000
	move.l	#$0024FFFE, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = -2
	move.l	#$FFFE0024, BLTAMOD(a1)				; BLTAMOD = -2
												; BLTDMOD = 36
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 836
	add.l	#836, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_cap_bullets_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_cap_bullets_10:
	move.w	_e_status_bar+statbar_cap_bullets_10, d0; draw cap_bullets_10
	move.w	d0, _e_status_bar_shadow+statbar_cap_bullets_10
	beq		.draw_cap_bullets_1
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 306, 5, 4, 6 );
	move.l	#$2FCA2000, BLTCON0(a1)				; BLTCON0 = 0x2FCA
												; BLTCON1 = 0x2000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 838
	add.l	#838, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_cap_bullets_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_cap_bullets_1:
	move.w	_e_status_bar+statbar_cap_bullets_1, d0; draw cap_bullets_1
	move.w	d0, _e_status_bar_shadow+statbar_cap_bullets_1
	beq		.check_cap_shells_100
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 310, 5, 4, 6 );
	move.l	#$6FCA6000, BLTCON0(a1)				; BLTCON0 = 0x6FCA
												; BLTCON1 = 0x6000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 838
	add.l	#838, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.check_cap_shells_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_cap_shells_100:
	move.w	_e_status_bar+statbar_cap_shells_100, d0; check cap_shells_100
	cmp.w	_e_status_bar_shadow+statbar_cap_shells_100, d0
	beq		.check_cap_shells_10

	; Clear: cap_shells_100, cap_shells_10, cap_shells_1
												; Stat_Clear( 302, 11, 12, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$0003FFC0, BLTAFWM(a1)				; BLTAFWM = 0x0003
												; BLTALWM = 0xFFC0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 1802, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 1802
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 1796
	add.l	#1796, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_cap_shells_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_cap_shells_10:
	move.w	_e_status_bar+statbar_cap_shells_10, d0; check cap_shells_10
	cmp.w	_e_status_bar_shadow+statbar_cap_shells_10, d0
	beq		.check_cap_shells_1

	; Clear: cap_shells_10, cap_shells_1
												; Stat_Clear( 306, 11, 8, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$3FFFFFC0, BLTAFWM(a1)				; BLTAFWM = 0x3FFF
												; BLTALWM = 0xFFC0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00260026, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 38
	move.w	#38, BLTDMOD(a1)					; BLTDMOD = 38
	move.l	#_GFX_STBAR0 + 1804, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 1804
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 1798
	add.l	#1798, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_cap_shells_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_cap_shells_1:
	move.w	_e_status_bar+statbar_cap_shells_1, d0; check cap_shells_1
	cmp.w	_e_status_bar_shadow+statbar_cap_shells_1, d0
	beq		.check_cap_rockets_100

	; Clear: cap_shells_1
												; Stat_Clear( 310, 11, 4, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$03FFFFC0, BLTAFWM(a1)				; BLTAFWM = 0x03FF
												; BLTALWM = 0xFFC0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00260026, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 38
	move.w	#38, BLTDMOD(a1)					; BLTDMOD = 38
	move.l	#_GFX_STBAR0 + 1804, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 1804
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 1798
	add.l	#1798, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_cap_shells_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_cap_shells_100:
	move.w	_e_status_bar+statbar_cap_shells_100, d0; draw cap_shells_100
	move.w	d0, _e_status_bar_shadow+statbar_cap_shells_100
	beq		.draw_cap_shells_10
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 302, 11, 4, 6 );
	move.l	#$EFCAE000, BLTCON0(a1)				; BLTCON0 = 0xEFCA
												; BLTCON1 = 0xE000
	move.l	#$FFFF0000, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0x0000
	move.l	#$0024FFFE, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = -2
	move.l	#$FFFE0024, BLTAMOD(a1)				; BLTAMOD = -2
												; BLTDMOD = 36
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 1796
	add.l	#1796, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_cap_shells_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_cap_shells_10:
	move.w	_e_status_bar+statbar_cap_shells_10, d0; draw cap_shells_10
	move.w	d0, _e_status_bar_shadow+statbar_cap_shells_10
	beq		.draw_cap_shells_1
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 306, 11, 4, 6 );
	move.l	#$2FCA2000, BLTCON0(a1)				; BLTCON0 = 0x2FCA
												; BLTCON1 = 0x2000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 1798
	add.l	#1798, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_cap_shells_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_cap_shells_1:
	move.w	_e_status_bar+statbar_cap_shells_1, d0; draw cap_shells_1
	move.w	d0, _e_status_bar_shadow+statbar_cap_shells_1
	beq		.check_cap_rockets_100
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 310, 11, 4, 6 );
	move.l	#$6FCA6000, BLTCON0(a1)				; BLTCON0 = 0x6FCA
												; BLTCON1 = 0x6000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 1798
	add.l	#1798, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.check_cap_rockets_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_cap_rockets_100:
	move.w	_e_status_bar+statbar_cap_rockets_100, d0; check cap_rockets_100
	cmp.w	_e_status_bar_shadow+statbar_cap_rockets_100, d0
	beq		.check_cap_rockets_10

	; Clear: cap_rockets_100, cap_rockets_10, cap_rockets_1
												; Stat_Clear( 302, 17, 12, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$0003FFC0, BLTAFWM(a1)				; BLTAFWM = 0x0003
												; BLTALWM = 0xFFC0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 2762, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 2762
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 2756
	add.l	#2756, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_cap_rockets_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_cap_rockets_10:
	move.w	_e_status_bar+statbar_cap_rockets_10, d0; check cap_rockets_10
	cmp.w	_e_status_bar_shadow+statbar_cap_rockets_10, d0
	beq		.check_cap_rockets_1

	; Clear: cap_rockets_10, cap_rockets_1
												; Stat_Clear( 306, 17, 8, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$3FFFFFC0, BLTAFWM(a1)				; BLTAFWM = 0x3FFF
												; BLTALWM = 0xFFC0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00260026, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 38
	move.w	#38, BLTDMOD(a1)					; BLTDMOD = 38
	move.l	#_GFX_STBAR0 + 2764, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 2764
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 2758
	add.l	#2758, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_cap_rockets_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_cap_rockets_1:
	move.w	_e_status_bar+statbar_cap_rockets_1, d0; check cap_rockets_1
	cmp.w	_e_status_bar_shadow+statbar_cap_rockets_1, d0
	beq		.check_cap_cells_100

	; Clear: cap_rockets_1
												; Stat_Clear( 310, 17, 4, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$03FFFFC0, BLTAFWM(a1)				; BLTAFWM = 0x03FF
												; BLTALWM = 0xFFC0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00260026, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 38
	move.w	#38, BLTDMOD(a1)					; BLTDMOD = 38
	move.l	#_GFX_STBAR0 + 2764, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 2764
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 2758
	add.l	#2758, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_cap_rockets_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_cap_rockets_100:
	move.w	_e_status_bar+statbar_cap_rockets_100, d0; draw cap_rockets_100
	move.w	d0, _e_status_bar_shadow+statbar_cap_rockets_100
	beq		.draw_cap_rockets_10
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 302, 17, 4, 6 );
	move.l	#$EFCAE000, BLTCON0(a1)				; BLTCON0 = 0xEFCA
												; BLTCON1 = 0xE000
	move.l	#$FFFF0000, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0x0000
	move.l	#$0024FFFE, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = -2
	move.l	#$FFFE0024, BLTAMOD(a1)				; BLTAMOD = -2
												; BLTDMOD = 36
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 2756
	add.l	#2756, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_cap_rockets_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_cap_rockets_10:
	move.w	_e_status_bar+statbar_cap_rockets_10, d0; draw cap_rockets_10
	move.w	d0, _e_status_bar_shadow+statbar_cap_rockets_10
	beq		.draw_cap_rockets_1
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 306, 17, 4, 6 );
	move.l	#$2FCA2000, BLTCON0(a1)				; BLTCON0 = 0x2FCA
												; BLTCON1 = 0x2000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 2758
	add.l	#2758, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_cap_rockets_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_cap_rockets_1:
	move.w	_e_status_bar+statbar_cap_rockets_1, d0; draw cap_rockets_1
	move.w	d0, _e_status_bar_shadow+statbar_cap_rockets_1
	beq		.check_cap_cells_100
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 310, 17, 4, 6 );
	move.l	#$6FCA6000, BLTCON0(a1)				; BLTCON0 = 0x6FCA
												; BLTCON1 = 0x6000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 2758
	add.l	#2758, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.check_cap_cells_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_cap_cells_100:
	move.w	_e_status_bar+statbar_cap_cells_100, d0; check cap_cells_100
	cmp.w	_e_status_bar_shadow+statbar_cap_cells_100, d0
	beq		.check_cap_cells_10

	; Clear: cap_cells_100, cap_cells_10, cap_cells_1
												; Stat_Clear( 302, 23, 12, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$0003FFC0, BLTAFWM(a1)				; BLTAFWM = 0x0003
												; BLTALWM = 0xFFC0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 3722, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 3722
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 3716
	add.l	#3716, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_cap_cells_100(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_cap_cells_10:
	move.w	_e_status_bar+statbar_cap_cells_10, d0; check cap_cells_10
	cmp.w	_e_status_bar_shadow+statbar_cap_cells_10, d0
	beq		.check_cap_cells_1

	; Clear: cap_cells_10, cap_cells_1
												; Stat_Clear( 306, 23, 8, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$3FFFFFC0, BLTAFWM(a1)				; BLTAFWM = 0x3FFF
												; BLTALWM = 0xFFC0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00260026, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 38
	move.w	#38, BLTDMOD(a1)					; BLTDMOD = 38
	move.l	#_GFX_STBAR0 + 3724, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 3724
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 3718
	add.l	#3718, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_cap_cells_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_cap_cells_1:
	move.w	_e_status_bar+statbar_cap_cells_1, d0; check cap_cells_1
	cmp.w	_e_status_bar_shadow+statbar_cap_cells_1, d0
	beq		.check_key_0

	; Clear: cap_cells_1
												; Stat_Clear( 310, 23, 4, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$03FFFFC0, BLTAFWM(a1)				; BLTAFWM = 0x03FF
												; BLTALWM = 0xFFC0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00260026, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 38
	move.w	#38, BLTDMOD(a1)					; BLTDMOD = 38
	move.l	#_GFX_STBAR0 + 3724, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 3724
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 3718
	add.l	#3718, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_cap_cells_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_cap_cells_100:
	move.w	_e_status_bar+statbar_cap_cells_100, d0; draw cap_cells_100
	move.w	d0, _e_status_bar_shadow+statbar_cap_cells_100
	beq		.draw_cap_cells_10
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 302, 23, 4, 6 );
	move.l	#$EFCAE000, BLTCON0(a1)				; BLTCON0 = 0xEFCA
												; BLTCON1 = 0xE000
	move.l	#$FFFF0000, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0x0000
	move.l	#$0024FFFE, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = -2
	move.l	#$FFFE0024, BLTAMOD(a1)				; BLTAMOD = -2
												; BLTDMOD = 36
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 3716
	add.l	#3716, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_cap_cells_10(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_cap_cells_10:
	move.w	_e_status_bar+statbar_cap_cells_10, d0; draw cap_cells_10
	move.w	d0, _e_status_bar_shadow+statbar_cap_cells_10
	beq		.draw_cap_cells_1
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 306, 23, 4, 6 );
	move.l	#$2FCA2000, BLTCON0(a1)				; BLTCON0 = 0x2FCA
												; BLTCON1 = 0x2000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 3718
	add.l	#3718, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_cap_cells_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_cap_cells_1:
	move.w	_e_status_bar+statbar_cap_cells_1, d0; draw cap_cells_1
	move.w	d0, _e_status_bar_shadow+statbar_cap_cells_1
	beq		.check_key_0
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 310, 23, 4, 6 );
	move.l	#$6FCA6000, BLTCON0(a1)				; BLTCON0 = 0x6FCA
												; BLTCON1 = 0x6000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 3718
	add.l	#3718, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.check_key_0(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_key_0:
	move.w	_e_status_bar+statbar_key_0, d0		; check key_0
	cmp.w	_e_status_bar_shadow+statbar_key_0, d0
	beq		.check_key_1

	; Clear: key_0
												; Stat_Clear( 239, 3, 7, 5 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$0001FC00, BLTAFWM(a1)				; BLTAFWM = 0x0001
												; BLTALWM = 0xFC00
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 514, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 514
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 508
	add.l	#508, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((20<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((20<<6) | 2)

	lea.l	.draw_key_0(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_key_0:
	move.w	_e_status_bar+statbar_key_0, d0		; draw key_0
	move.w	d0, _e_status_bar_shadow+statbar_key_0
	beq		.check_key_1
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 239, 3, 7, 5 );
	move.l	#$FFCAF000, BLTCON0(a1)				; BLTCON0 = 0xFFCA
												; BLTCON1 = 0xF000
	move.l	#$FFFF0000, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0x0000
	move.l	#$0024FFFE, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = -2
	move.l	#$FFFE0024, BLTAMOD(a1)				; BLTAMOD = -2
												; BLTDMOD = 36
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#40, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 508
	add.l	#508, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((20<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((20<<6) | 2)

	lea.l	.check_key_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_key_1:
	move.w	_e_status_bar+statbar_key_1, d0		; check key_1
	cmp.w	_e_status_bar_shadow+statbar_key_1, d0
	beq		.check_key_2

	; Clear: key_1
												; Stat_Clear( 239, 13, 7, 5 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$0001FC00, BLTAFWM(a1)				; BLTAFWM = 0x0001
												; BLTALWM = 0xFC00
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 2114, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 2114
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 2108
	add.l	#2108, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((20<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((20<<6) | 2)

	lea.l	.draw_key_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_key_1:
	move.w	_e_status_bar+statbar_key_1, d0		; draw key_1
	move.w	d0, _e_status_bar_shadow+statbar_key_1
	beq		.check_key_2
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 239, 13, 7, 5 );
	move.l	#$FFCAF000, BLTCON0(a1)				; BLTCON0 = 0xFFCA
												; BLTCON1 = 0xF000
	move.l	#$FFFF0000, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0x0000
	move.l	#$0024FFFE, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = -2
	move.l	#$FFFE0024, BLTAMOD(a1)				; BLTAMOD = -2
												; BLTDMOD = 36
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#40, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 2108
	add.l	#2108, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((20<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((20<<6) | 2)

	lea.l	.check_key_2(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_key_2:
	move.w	_e_status_bar+statbar_key_2, d0		; check key_2
	cmp.w	_e_status_bar_shadow+statbar_key_2, d0
	beq		.check_weapon_0

	; Clear: key_2
												; Stat_Clear( 239, 23, 7, 5 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$0001FC00, BLTAFWM(a1)				; BLTAFWM = 0x0001
												; BLTALWM = 0xFC00
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 3714, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 3714
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 3708
	add.l	#3708, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((20<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((20<<6) | 2)

	lea.l	.draw_key_2(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_key_2:
	move.w	_e_status_bar+statbar_key_2, d0		; draw key_2
	move.w	d0, _e_status_bar_shadow+statbar_key_2
	beq		.check_weapon_0
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 239, 23, 7, 5 );
	move.l	#$FFCAF000, BLTCON0(a1)				; BLTCON0 = 0xFFCA
												; BLTCON1 = 0xF000
	move.l	#$FFFF0000, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0x0000
	move.l	#$0024FFFE, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = -2
	move.l	#$FFFE0024, BLTAMOD(a1)				; BLTAMOD = -2
												; BLTDMOD = 36
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#40, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 3708
	add.l	#3708, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((20<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((20<<6) | 2)

	lea.l	.check_weapon_0(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_weapon_0:
	move.w	_e_status_bar+statbar_weapon_0, d0	; check weapon_0
	cmp.w	_e_status_bar_shadow+statbar_weapon_0, d0
	beq		.check_weapon_1

	; Clear: weapon_0
												; Stat_Clear( 111, 4, 4, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$0001E000, BLTAFWM(a1)				; BLTAFWM = 0x0001
												; BLTALWM = 0xE000
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 658, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 658
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 652
	add.l	#652, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_weapon_0(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_weapon_0:
	move.w	_e_status_bar+statbar_weapon_0, d0	; draw weapon_0
	move.w	d0, _e_status_bar_shadow+statbar_weapon_0
	beq		.check_weapon_1
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 111, 4, 4, 6 );
	move.l	#$FFCAF000, BLTCON0(a1)				; BLTCON0 = 0xFFCA
												; BLTCON1 = 0xF000
	move.l	#$FFFF0000, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0x0000
	move.l	#$0024FFFE, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = -2
	move.l	#$FFFE0024, BLTAMOD(a1)				; BLTAMOD = -2
												; BLTDMOD = 36
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 652
	add.l	#652, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.check_weapon_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_weapon_1:
	move.w	_e_status_bar+statbar_weapon_1, d0	; check weapon_1
	cmp.w	_e_status_bar_shadow+statbar_weapon_1, d0
	beq		.check_weapon_2

	; Clear: weapon_1
												; Stat_Clear( 123, 4, 4, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$001FFFFE, BLTAFWM(a1)				; BLTAFWM = 0x001F
												; BLTALWM = 0xFFFE
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00260026, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 38
	move.w	#38, BLTDMOD(a1)					; BLTDMOD = 38
	move.l	#_GFX_STBAR0 + 660, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 660
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 654
	add.l	#654, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_weapon_1(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_weapon_1:
	move.w	_e_status_bar+statbar_weapon_1, d0	; draw weapon_1
	move.w	d0, _e_status_bar_shadow+statbar_weapon_1
	beq		.check_weapon_2
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 123, 4, 4, 6 );
	move.l	#$BFCAB000, BLTCON0(a1)				; BLTCON0 = 0xBFCA
												; BLTCON1 = 0xB000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 654
	add.l	#654, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.check_weapon_2(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_weapon_2:
	move.w	_e_status_bar+statbar_weapon_2, d0	; check weapon_2
	cmp.w	_e_status_bar_shadow+statbar_weapon_2, d0
	beq		.check_weapon_3

	; Clear: weapon_2
												; Stat_Clear( 135, 4, 4, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$01FFFFE0, BLTAFWM(a1)				; BLTAFWM = 0x01FF
												; BLTALWM = 0xFFE0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00260026, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 38
	move.w	#38, BLTDMOD(a1)					; BLTDMOD = 38
	move.l	#_GFX_STBAR0 + 662, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 662
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 656
	add.l	#656, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_weapon_2(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_weapon_2:
	move.w	_e_status_bar+statbar_weapon_2, d0	; draw weapon_2
	move.w	d0, _e_status_bar_shadow+statbar_weapon_2
	beq		.check_weapon_3
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 135, 4, 4, 6 );
	move.l	#$7FCA7000, BLTCON0(a1)				; BLTCON0 = 0x7FCA
												; BLTCON1 = 0x7000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 656
	add.l	#656, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.check_weapon_3(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_weapon_3:
	move.w	_e_status_bar+statbar_weapon_3, d0	; check weapon_3
	cmp.w	_e_status_bar_shadow+statbar_weapon_3, d0
	beq		.check_weapon_4

	; Clear: weapon_3
												; Stat_Clear( 111, 14, 4, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$0001E000, BLTAFWM(a1)				; BLTAFWM = 0x0001
												; BLTALWM = 0xE000
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00240024, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = 36
	move.w	#36, BLTDMOD(a1)					; BLTDMOD = 36
	move.l	#_GFX_STBAR0 + 2258, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 2258
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 2252
	add.l	#2252, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.draw_weapon_3(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_weapon_3:
	move.w	_e_status_bar+statbar_weapon_3, d0	; draw weapon_3
	move.w	d0, _e_status_bar_shadow+statbar_weapon_3
	beq		.check_weapon_4
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 111, 14, 4, 6 );
	move.l	#$FFCAF000, BLTCON0(a1)				; BLTCON0 = 0xFFCA
												; BLTCON1 = 0xF000
	move.l	#$FFFF0000, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0x0000
	move.l	#$0024FFFE, BLTCMOD(a1)				; BLTCMOD = 36
												; BLTBMOD = -2
	move.l	#$FFFE0024, BLTAMOD(a1)				; BLTAMOD = -2
												; BLTDMOD = 36
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 2252
	add.l	#2252, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 2), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 2)

	lea.l	.check_weapon_4(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_weapon_4:
	move.w	_e_status_bar+statbar_weapon_4, d0	; check weapon_4
	cmp.w	_e_status_bar_shadow+statbar_weapon_4, d0
	beq		.check_weapon_5

	; Clear: weapon_4
												; Stat_Clear( 123, 14, 4, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$001FFFFE, BLTAFWM(a1)				; BLTAFWM = 0x001F
												; BLTALWM = 0xFFFE
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00260026, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 38
	move.w	#38, BLTDMOD(a1)					; BLTDMOD = 38
	move.l	#_GFX_STBAR0 + 2260, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 2260
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 2254
	add.l	#2254, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_weapon_4(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_weapon_4:
	move.w	_e_status_bar+statbar_weapon_4, d0	; draw weapon_4
	move.w	d0, _e_status_bar_shadow+statbar_weapon_4
	beq		.check_weapon_5
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 123, 14, 4, 6 );
	move.l	#$BFCAB000, BLTCON0(a1)				; BLTCON0 = 0xBFCA
												; BLTCON1 = 0xB000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 2254
	add.l	#2254, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.check_weapon_5(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.check_weapon_5:
	move.w	_e_status_bar+statbar_weapon_5, d0	; check weapon_5
	cmp.w	_e_status_bar_shadow+statbar_weapon_5, d0
	beq		.end

	; Clear: weapon_5
												; Stat_Clear( 135, 14, 4, 6 );
	move.l	#$07CA0000, BLTCON0(a1)				; BLTCON0 = 0x07CA
												; BLTCON1 = 0x0000
	move.l	#$01FFFFE0, BLTAFWM(a1)				; BLTAFWM = 0x01FF
												; BLTALWM = 0xFFE0
	move.w	#$FFFF, BLTADAT(a1)					; BLTADAT = 0xFFFF
	move.l	#$00260026, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 38
	move.w	#38, BLTDMOD(a1)					; BLTDMOD = 38
	move.l	#_GFX_STBAR0 + 2262, BLTBPT(a1)		; BLTBPT = _GFX_STBAR0 + 2262
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 2256
	add.l	#2256, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

	lea.l	.draw_weapon_5(pc), a0
	move.l	a0, _Blit_IRQ_fn
	rts		


.draw_weapon_5:
	move.w	_e_status_bar+statbar_weapon_5, d0	; draw weapon_5
	move.w	d0, _e_status_bar_shadow+statbar_weapon_5
	beq		.end
	lsl.w	#2, d0
	lea.l	_STAT_DIGITS, a0
	move.l	(a0,d0.w), a0

												; Stat_Draw( 135, 14, 4, 6 );
	move.l	#$7FCA7000, BLTCON0(a1)				; BLTCON0 = 0x7FCA
												; BLTCON1 = 0x7000
	move.l	#$FFFFFFFF, BLTAFWM(a1)				; BLTAFWM = 0xFFFF
												; BLTALWM = 0xFFFF
	move.l	#$00260000, BLTCMOD(a1)				; BLTCMOD = 38
												; BLTBMOD = 0
	move.l	#$00000026, BLTAMOD(a1)				; BLTAMOD = 0
												; BLTDMOD = 38
	move.l	a0, BLTBPT(a1)						; BLTBPT = a0
	add.l	#48, a0
	move.l	a0, BLTAPT(a1)						; BLTAPT = a0
	move.l	_statbar_buffer, d0					; d0 = _statbar_buffer + 2256
	add.l	#2256, d0
	move.l	d0, BLTCPT(a1)						; BLTCPT = d0
	move.l	d0, BLTDPT(a1)						; BLTDPT = d0
	move.w	#((24<<6) | 1), BLTSIZE(a1)			; BLTSIZE = ((24<<6) | 1)

.end:
	moveq.l	#0, d0
	move.l	d0, _Blit_IRQ_fn
	rts		
