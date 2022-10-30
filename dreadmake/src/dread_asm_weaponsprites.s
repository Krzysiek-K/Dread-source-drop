

  if WEAPONSPRITES


	xref		_IO_CheckMouse
	xref		_io_mouse_state
	xref		_view_weapon_swing			; word
	xref		_view_weapon_drop			; short
	xref		_view_movement				; word
	xref		_view_weapon				; EngineThing*
	xref		_weaponsprite_anim			; WeaponSpriteAnimFrame*
	xref		_weaponsprite_delay			; short
	xref		_copper_sprite_update		; word*
	xref		_copper_sprite_update2		; word*
	xref		_sincos_fix14				; short[256]


	public	Update_Weapon_Sprite
Update_Weapon_Sprite:
	;		movem.l		d2-d7/a2-a6,-(a7)		[already saved]


WEAPONSPRITE_UPDATE_TICK		equ			6*2				;	ticks		TBD: PAL NTSC


															;	// interrupt weapon logic
	jsr			_IO_CheckMouse								;	IO_CheckMouse();			//|| !(CIAA_PRA & (1<<6)) )

	lea.l		_e_globals, a5
	move.w		gv_game_state(a5), d0
	cmp.w		#1, d0
	bne			.end

	move.w		gv_player_health(a5), d0
	cmp.w		#0, d0
	beq			.nofire
	

	moveq.l		#0, d0
	move.b		_io_mouse_state, d0							;	if( io_mouse_state )
	or.b		d0, d0
	beq.b		.nofire										;	{
	move.w		gv_pulse(a5), d1							;												[save global pulse in case we interrupt already running script code]
	move.w		d0, gv_pulse(a5)							;		e_globals.pulse = io_mouse_state;
	and.b		#$30, d0
	move.b		d0, _io_mouse_state							;		io_mouse_state &= 0x30;

	move.l		_view_weapon, a6							;		Script_OnPulse(view_weapon);			[inlined,  a5/a6 already set up]
	move.l		thing_actor(a6), a4
	move.l		actor_cb_pulse(a4), d0
	beq.b		.nofire
	move.l		d0, a3
	jsr			(a3)

.nofire:													;	}
	move.w		d1, gv_pulse(a5)							;												[restore global pulse]


	;	move.w		thing_timer_attack(a6), d0					;	if( view_weapon->timer_attack )
	;	or.w		d0, d0
	;	beq.b		.swing_test_run
	;	move.w		#0, _view_weapon_swing						;		view_weapon_swing = 0;
	;	bra			.swing_end									

.swing_test_run:											;	else if( view_movement )
	move.w		_view_movement, d0
	or.w		d0, d0
	beq			.swing_return_neutral
	add.w		#WEAPONSPRITE_UPDATE_TICK>>1, _view_weapon_swing		;		view_weapon_swing += 6>>1;		// weapon swing
	bra			.swing_end									

.swing_return_neutral:										;	else if( view_weapon_swing & 0x7F )
	move.w		_view_weapon_swing, d0
	andi.b		#$7F, d0
	beq			.swing_end									;	{
	
	move.w		_view_weapon_swing, d0						;		word pswing = view_weapon_swing;										[ d0 = view_weapon_swing ]
	move.w		d0, d1										;																				[ d1 = pswing ]
	move.w		d0, d2										;																				[ d2 = test copy ]

	add.w		#WEAPONSPRITE_UPDATE_TICK, d0				;		if( view_weapon_swing & 0x40 )	view_weapon_swing += 6;
	andi.b		#$40, d2									;
	bne.b		.swing_return_end							;
	sub.w		#WEAPONSPRITE_UPDATE_TICK*2, d0				;		else							view_weapon_swing -= 6;
						
.swing_return_end:											;		if( (pswing^view_weapon_swing) & 0x80 )
	move.w		d0, _view_weapon_swing						
	eor.b		d1, d0
	bpl.b		.swing_end
	move.w		#0, _view_weapon_swing						;			view_weapon_swing = 0;

.swing_end:													;	}



															;	if( !copper_sprite_update || !weaponsprite_anim )	return;
	move.l		_copper_sprite_update, a2					;																				[ a2 = copper_sprite_update ]
	cmpa.l		#0, a2
	beq			.end
	move.l		_weaponsprite_anim, a3						;																				[ a3 = weaponsprite_anim ]
	cmpa.l		#0, a3
	beq			.end

	move.w		_weaponsprite_delay, d2						;	weaponsprite_delay += 6;													[ d2 = weaponsprite_delay ]
	add.w		#WEAPONSPRITE_UPDATE_TICK, d2				;
	

.anim_loop_init:
	move.w		weaponanim_delay(a3), d0					;																				[ d0 = weaponsprite_anim->delay ]
.anim_loop:
	cmp.w		d0, d2										;	while( weaponsprite_delay >= weaponsprite_anim->delay )
	blo			.anim_loop_end								;	{

	sub.w		d0, d2										;		weaponsprite_delay -= weaponsprite_anim->delay;
	add.l		#weaponanim_sizeof, a3						;		weaponsprite_anim++;
	move.w		weaponanim_delay(a3), d0					;																				[ d0 = weaponsprite_anim->delay ]
	
	tst.w		d0											;		if( weaponsprite_anim->delay < 0 )
	bpl			.anim_loop									
	adda.w		d0, a3										;			weaponsprite_anim += weaponsprite_anim->delay;
	bra			.anim_loop_init								;	}

.anim_loop_end:


	move.w		d2, _weaponsprite_delay						; write back
	move.l		a3, _weaponsprite_anim


	move.w		weaponanim_flags(a3), d0					;	if(  !weaponsprite_anim->flags )
	bne			.update_sprite								;
	move.w		#0, _view_weapon_swing						;		view_weapon_swing = 0;


.update_sprite:												;	// update sprite
	move.l		weaponanim_gfx(a3), a4						;	const WeaponSpriteInfo *ws = weaponsprite_anim->gfx;						[ a4 = ws ]
															;	word *dst = copper_sprite_update;											[ already in a2 ]
	move.w		(a4)+, (a2)									;	for( int i=0; i<15; i++ )
	move.w		(a4)+, 4(a2)								;	{
	move.w		(a4)+, 8(a2)								;		*dst = ws->palette[i];
	move.w		(a4)+, 12(a2)								;		dst += 2;
	move.w		(a4)+, 16(a2)								;	}
	move.w		(a4)+, 20(a2)
	move.w		(a4)+, 24(a2)
	move.w		(a4)+, 28(a2)
	move.w		(a4)+, 32(a2)
	move.w		(a4)+, 36(a2)
	move.w		(a4)+, 40(a2)
	move.w		(a4)+, 44(a2)
	move.w		(a4)+, 48(a2)
	move.w		(a4)+, 52(a2)
	move.w		(a4)+, 56(a2)
	add.l		#15*4, a2


	lea.l		_sincos_fix14, a1							;																				[ a1 = sincos_fix14 ]

	moveq.l		#0, d0										;	word xp = weaponsprite_anim->xpos + (word)(sincos_fix14[(view_weapon_swing+64)&0xFF]>>11);				[ d2 = xp ]
	move.b		_view_weapon_swing+1, d0
	move.l		d0, d1
	add.b		#64, d0
	add.w		d0, d0
	move.w		(a1,d0.w), d2
	asr.w		#8, d2
	asr.w		#3, d2
	add.w		weaponanim_xpos(a3), d2

	add.b		d1, d1										;	word yp = weaponsprite_anim->ypos + (word)(sincos_fix14[(byte)((view_weapon_swing<<1)+64)]>>11);		[ d3 = yp ]
	add.b		#64, d1	
	add.w		d1, d1
	move.w		(a1,d1.w), d3
	asr.w		#8, d3
	asr.w		#3, d3
	add.w		weaponanim_ypos(a3), d3
	add.w		_view_weapon_drop, d3
	cmp.w		#200, d3
	blo			.noyclamp
	move.l		#200, d3
.noyclamp:

	moveq.l		#3, d1										;	for( int i=0; i<4; i++ )
.sprite_loop:												;	{

	cmp.w		#2, d1
	bne.b		.samecopper
	move.l		_copper_sprite_update2, a2					;																				[ a2 = copper_sprite_update2 ]
.samecopper:
	add.w		(a4)+, d3									;		yp += ws->sprites[i].ydelta;
															;
															;		const word *ptr = ws->sprites[i].data0;
	move.w		(a4)+, 4(a2)								;		dst[2] = ((dword)ptr)>>16;						
	move.w		(a4)+, 0(a2)								;		dst[0] = (dword)ptr;
	move.b		d3, 8(a2)									;		dst[4] = (yp<<8) | (xp>>1);
	move.w		d2, d0
	lsr.w		#1, d0
	move.b		d0, 9(a2)
	move.w		d2, d0										;		dst[6] = 0xFF00 | (xp&1);
	andi.b		#1, d0
	move.b		d0, 13(a2)
															;
															;		ptr = ws->sprites[i].data1;
	move.w		(a4)+, 20(a2)								;		dst[10] = ((dword)ptr)>>16;
	move.w		(a4)+, 16(a2)								;		dst[8] = (dword)ptr;
	move.b		d3, 24(a2)									;		dst[12] = (yp<<8) | (xp>>1);
	move.w		d2, d0
	lsr.w		#1, d0
	move.b		d0, 25(a2)
	move.w		d2, d0										;		dst[14] = 0xFF80 | (xp&1);
	andi.b		#1, d0
	ori.b		#$80, d0
	move.b		d0, 29(a2)
	
	adda.l		#32, a2										;		dst += 16;
															;
	add.w		#16, d2										;		xp += 16;
	dbra.w		d1, .sprite_loop							;	}


.end:
	;		movem.l		(a7)+,d2-d7/a2-a6		[already saved]
	rts


  endif
