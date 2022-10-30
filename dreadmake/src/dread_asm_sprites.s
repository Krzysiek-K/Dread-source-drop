
	xref	_Dread_DrawSprite
	xref	_view_frame_number
	xref	_e_visthings
	xref	_e_visthings_end
	xref	_e_visthings_max


	public _Dread_DrawSprites
	public _Dread_DrawSprite_asm
	public _Dread_LineCore_Sprite



sprite_y_offs:
	dc.w		0

sprite_tex_offs:
	dc.w		0






; ================================================================ _Dread_DrawSprites ================================================================


_Dread_DrawSprites:
	movem.l		d2-d7/a2-a6,-(a7)


												; ============ Pass 1: remove sprites no longer visible

	move.w		_view_frame_number, d4				;	[d4: view_frame_number]

												;
	lea.l		_e_visthings, a0				;	EngineThing **read_ptr = e_visthings;												[ a0: read_ptr ]
	move.l		a0, a1							;	EngineThing **write_ptr = e_visthings;												[ a1: write_ptr ]
	move.l		_e_visthings_end, a2			;																						[ a2: e_visthings_end ]
	
	cmp.l		a2, a0							;	while( read_ptr < e_visthings_end )
	bhs.b		.p1_done
.p1_loop:										;	{
	move.l		(a0)+, a3						;		EngineThing *th = *read_ptr++;													[ a3: th ]
	move.l		thing_subsector(a3), d0			;		if( !th->subsector || th->subsector->visframe != view_frame_number )
	beq			.p1_invisible
	move.l		d0, a4							;																						[ a4: th->subsector ]
	cmp.w		subsector_visframe(a4), d4
	beq.b		.p1_visible

.p1_invisible:
	move.b		#0, thing_visible(a3)			;			th->visible = 0;
	bra.b		.p1_endif						;		else

.p1_visible:									;		{
	move.l		a3, (a1)+						;			*write_ptr++ = th;
												;
												;			// Compute thing transforms and stuff
	move.w		thing_xp(a3), d0				;			short spx = th->xp - view_pos_x;											[d0]
	sub.w		_view_pos_x, d0
												
	move.w		thing_yp(a3), d3				;			short spy = th->yp - view_pos_y;											[d3]
	sub.w		_view_pos_y, d3

	move.w		d0, d2							;			th->tr_x = (((int)spx)*view_rotate_dx - ((int)spy)*view_rotate_dy)>>14;		[d2]
	muls.w		_view_rotate_dx, d2
	move.w		d3, d1
	muls.w		_view_rotate_dy, d1
	sub.l		d1, d2
	lsl.l		#2, d2
	swap.w		d2
	move.w		d2, thing_tr_x(a3)
												
	muls.w		_view_rotate_dy, d0				;			th->tr_y = (((int)spx)*view_rotate_dy + ((int)spy)*view_rotate_dx)>>14;		[d3]
	muls.w		_view_rotate_dx, d3
	add.l		d0, d3
	lsl.l		#2, d3
	swap.w		d3
	move.w		d3, thing_tr_y(a3)

.p1_endif:										;		}

	cmp.l		a2, a0							;	}
	blt.b		.p1_loop


.p1_done:
	move.l		a1, _e_visthings_end			;	e_visthings_end = write_ptr;						[a1]


	lea.l		_e_visthings, a0				;	EngineThing **ptr = e_visthings;					[a0]
	cmp.l		a1, a0							;	if( e_visthings_end > e_visthings )
	bge			.end							;	{

												; ============ Pass 2: sort up
												;
;	moveq.l		#0, d1							;		int mod = 0;						TBD: implement second pass if required

;													; [invariants:
;													;	a0: ptr
;													;	a1: _e_visthings_end
;													;	a2: ptr[-1]
;													;	a3: ptr[0]
;													; ]
;		move.l		(a0)+, a3
;		cmp.l		a1, a0							;		for( EngineThing **ptr = e_visthings+1; ptr<e_visthings_end; ptr++ )
;		bge.b		.p2_nosort						;		{
;	.p2_loop:
;		move.l		a3, a2
;		move.l		(a0)+, a3
;							
;		move.l		thing_tr_y(a2), d0				;			if( ptr[-1]->tr_y > ptr[0]->tr_y )
;		cmp.l		thing_tr_y(a3), d0
;		ble.b		.p2_noswap						;			{
;		move.l		a2, -4(a0)						;				EngineThing *th = ptr[-1];					TBD: can be optimizing by keeping the moved element
;		move.l		a3, -8(a0)						;				ptr[-1] = ptr[0];
;		move.l		a2, a3							;				ptr[0] = th;								[update the cached version as well]
;	;	moveq.l		#1, d1							;				mod = 1;									TBD: implement second pass if required
;	
;	.p2_noswap:										;			}
;		cmp.l		a1, a0
;		blt.b		.p2_loop		
;	.p2_nosort:										;		}
;													;	
;	

												; ============ Pass 3: sort down
												
												; [invariants:
												;	a0: _e_visthings_end
												;	a1: ptr					(initially _e_visthings_end)
												;	a2: ptr[0]
												;	a3: ptr[1]
												; ]

;	add.w		d1, d1							;		if( mod )							TBD: implement second pass if required
;	beq.b		.p3_done

	move.l		-(a1), a2
	cmp.l		a0, a1							;			for( EngineThing **ptr = e_visthings_end-2; ptr>=e_visthings; ptr-- )
	ble.b		.p3_done						;			{
.p3_loop:
	move.l		a2, a3
	move.l		-(a1), a2
	move.l		thing_tr_y(a2), d0				;				if( ptr[0]->tr_y > ptr[1]->tr_y )
	cmp.l		thing_tr_y(a3), d0
	ble.b		.p3_noswap						;				{
	move.l		a2, 4(a1)						;					EngineThing *th = ptr[0];					TBD: can be optimizing by keeping the moved element
	move.l		a3, (a1)						;					ptr[0] = ptr[1];
	move.l		a3, a2							;					ptr[1] = th;								[update the cached version as well]
.p3_noswap:										;				}
	cmp.l		a0, a1							;			}
	bgt.b		.p3_loop		
.p3_done:


												; ============ Pass 4: draw all things
	move.l		_e_visthings_end, a2
	cmp.l		#_e_visthings, a2
	ble.b		.draw_done

	TRAP_ICACHE_DISABLE
	
.draw_loop:										;		for( EngineThing **ptr = e_visthings_end; ptr>e_visthings; )
												;		{
	move.l		-(a2), a0						;			EngineThing *th = *--ptr;
	move.l		thing_sprite(a0), d1			;																						[a3: frame]
	beq.b		.draw_skip
	move.l		d1, a3
	move.w		thing_tr_x(a0), d2				;			Dread_DrawSprite(th->tr_x, th->tr_y, th->sprite);							[d2: spx]
	ext.l		d2
	move.w		thing_tr_y(a0), d1				;																						[d1: spy]
	ext.l		d1
	move.w		#-40<<ENGINE_THING_Z_PRECISION, d0					; TBD: view_pos_z													[sprite_tex_offs / sprite_y_offs]
	sub.w		thing_zp(a0), d0									;thing_zp(a0)	;	sub.w		#(0-40)<<6, d0		(fixed.6)
	asr.w		#ENGINE_THING_Z_PRECISION+STRETCH_SCALING, d0
	move.w		d0, sprite_tex_offs
	lsl.w		#6, d0
	move.w		d0, sprite_y_offs

	move.b		thing_pickup(a0), d5
	beq			.draw_skip_pickup_check
	cmp.w		#ENGINE_PICKUP_DISTANCE, d1		;	pickup check (in drawing loop, because we all love hacks)
	bgt			.draw_skip_pickup_check
	cmp.w		#-ENGINE_PICKUP_DISTANCE, d1
	blt			.draw_skip_pickup_check
	cmp.w		#ENGINE_PICKUP_DISTANCE, d2
	bgt			.draw_skip_pickup_check
	cmp.w		#-ENGINE_PICKUP_DISTANCE, d2
	bge			.draw_pickup_hack
	
.draw_skip_pickup_check:
	move.w		thing_hitnum(a0), d5

	move.l		a2, -(a7)						;	SAVE: a2
	jsr			_Dread_DrawMultiSprite_asm
	move.l		(a7)+, a2
.draw_skip:
	cmp.l		#_e_visthings, a2				;		}
	bgt.b		.draw_loop

	TRAP_ICACHE_ENABLE

.draw_done:
												;	}


.end:
	movem.l		(a7)+,d2-d7/a2-a6
	rts


.draw_pickup_hack:
	lea.l		_e_globals, a5
	move.w		#0, gv_pulse(a5)
	move.l		a0, a6
	jsr			_Script_OnPulse
	bra			.draw_skip

.a2_save:
	dc.l		0
	


; ================================================================ _Dread_DrawMultiSprite_asm ================================================================
;
;	void Dread_DrawSprite_asm(__d2 int spx, __d1 int spy, __a3 const word *frame, __d5 int hitnum)
;
;	parameters passed via globals:
;		sprite_tex_offs
;		sprite_y_offs
;
;	Registers - all not saved
;
_Dread_DrawMultiSprite_asm:
	move.l		4(a3), d0
	beq			_Dread_DrawSprite_asm			; not a multi-sprite
	
	move.w		sprite_tex_offs, d4				; keep sprite_tex_offs in d4

.draw_loop:
	movem.l		d1/d2/d4/a3, -(a7)						; save state
	move.w		(a3), d3								;	spx -= (frame[0]<<ENGINE_SUBVERTEX_PRECISION);
	lsl.w		#ENGINE_SUBVERTEX_PRECISION, d3			;
	sub.w		d3, d2

	add.w		2(a3), d4								;	off_y
	move.w		d4, sprite_tex_offs
	lsl.w		#6, d4
	move.w		d4, sprite_y_offs


	move.l		d0, a3								; a3 = frame
	jsr			_Dread_DrawSprite_asm
	movem.l		(a7)+, d1/d2/d4/a3						; restore state

	addq.l		#8, a3
	move.l		4(a3), d0
	bne			.draw_loop

	rts


; ================================================================ _Dread_DrawSprite_asm ================================================================
;
;	void Dread_DrawSprite_asm(__d2 int spx, __d1 int spy, __a3 const word *frame, __d5 int hitnum)
;
;	parameters passed via globals:
;		sprite_tex_offs
;		sprite_y_offs
;
;	Registers:
;		d0								(temp, not saved)
;		d1		IN		spy				(not saved)
;		d2		IN		spx				(not saved)
;		d3								(temp, not saved)
;		d4								(temp, not saved)
;		d5		IN		hitnum			(likely not modified)
;		d6								(temp, not saved)
;		d7								(temp, not saved)
;
;		a0								(temp, not saved)
;		a1								(temp, not saved)
;		a2								(temp, not saved)
;		a3		IN		frame			(not saved)
;		a4								(temp, not saved)
;		a5								(temp, not saved)
;		a6								(temp, not saved)
;		a7				stack/(temp)
;
;

_Dread_DrawSprite_asm:
												;		// Draw
												;		const int WOLFSCREEN_W = 160;
												;		const int wolf_xoffs = 80;
												;		const int zoom_i = 80;
												;		const int clip = 161;	//161;	// fix.4
												;	
	cmp.w		#ENGINE_MIN_ZNEAR, d1			;		if( spy < clip )
	blt.w		.end							;			return;
						
	move.w		2(a3), d3						;		int p1x = spx - (frame[1]<<ENGINE_SUBVERTEX_PRECISION);			[d2]	[d3: temp]
	lsl.w		#ENGINE_SUBVERTEX_PRECISION, d3	;
	sub.w		d3, d2
	
	moveq.l		#0, d0							;		word tx2 = frame[0];											[d0]
	move.w		(a3), d0
	move.w		d0, d3							;		int p2x = p1x + (frame[0]<<ENGINE_SUBVERTEX_PRECISION);			[d3]
	lsl.w		#ENGINE_SUBVERTEX_PRECISION, d3	;
	add.w		d2, d3

	
	muls.w		#ENGINE_ZOOM, d3				;		int xmax = divs(muls(p2x, zoom_i), spy) + wolf_xoffs;			[d3]
	divs.w		d1, d3							;
	add.w		#ENGINE_ZOOM, d3				;
	bmi.b		.end							;		if( xmax < 0 ) return;

	muls.w		#ENGINE_ZOOM, d2				;		int xmin = divs(muls(p1x, zoom_i), spy) + wolf_xoffs;			[d2]
	divs.w		d1, d2							;
	add.w		#ENGINE_ZOOM, d2				;

	cmp.w		#ENGINE_WIDTH, d2				;		if( xmin>=WOLFSCREEN_W ) return;
	bge.b		.end							;


	move.w		d3, d4							;		if( xmin>=xmax ) return;										[d4: xmax-xmin]
	sub.w		d2, d4
	ble.b		.end							;	

	moveq.l		#0, d6							;		long u1s = 0;													[d6]

												;		long du = (u2s=((long)tx2)<<(16+1)) / (word)(xmax-xmin)			[d7]					// result can be 32-bit!!
	add.w		d0, d0							;	[ d0 = 0 : hi ]
	divu.w		d4, d0							;	[ d0 = remhi : divhi ]				dividing by d4.w
	move.w		d0, d7							;	[ d7 = ? : divhi ]
	swap.w		d7								;	[ d7 = divhi : ? ]
	move.w		d6, d0							;	[ d0 = remhi : 0 ]					[ d6 == 0 ]
	divu.w		d4, d0							;	[ d0 = remlo : divlo ]
	move.w		d0, d7							;	[ d7 = divhi : divlo ]

	cmp.w		d6, d2							;		if( xmin < 0 )
	bpl.b		.xmin_noclip					;		{
	neg.w		d2								;			xmin = -xmin;
												;			u1s = du*xmin;				// d7.l * d2.w --> d6.l
												;	---d2---	---d6---	---d7---
												;	- : xmin	  0 : 0		duH : duL
	move.w		d2, d6							;				0 : xmin
	mulu.w		d7, d6							;				 H1 : L
	swap.w		d7								;							duL : duH
	mulu.w		d7, d2							;	 - : H2
	swap.w		d7								;							duH : duL
	swap.w		d2								;	H2 : -
	eor.w		d2, d2							;	H2 : 0								xmin = 0;
	add.l		d2, d6							;				  H : L
.xmin_noclip:									;		}


	cmp.w		#ENGINE_WIDTH, d3				;		if( xmax>WOLFSCREEN_W ) xmax = WOLFSCREEN_W;
	ble.w		.xmax_ok
	move.w		#ENGINE_WIDTH, d3
.xmax_ok:


	move.l		#ENGINE_ZOOM_CONSTANT, d4		;		short s0 = divs((zoom_i*32)<<(8+ENGINE_SUBVERTEX_PRECISION), spy);		[d4]	// won't overflow because of clipping p#y>=161
	divs.w		d1, d4

	move.w		sprite_tex_offs, d0
	lea.l		(a3,d0.w), a0					;		asm_tex_base = (const byte*)frame + -40;
	move.l		a0, _asm_tex_base				;	

												;		Dread_LineCore_Sprite(xmin<<3, xmax-xmin-1, s0, u1s, du, frame);
	sub.w		d2, d3							;		d3.w:		xmax-xmin-1
	ble			.end							;
	subq.l		#1, d3							;
	lsl.w		#3, d2							;		d2.w:		xmin<<3
												;		d4:		s0
												;		d6:		u1s
												;		d7:		du
												;		a3:		frame
												;	

	move.w		d5, d5
	bne			.hitbox	
	bra			_Dread_LineCore_Sprite

.hitbox:
	bra			_Dread_LineCore_Sprite_hitbox


.end:
	rts


; ================================================================ _Dread_LineCore_Sprite ================================================================


	; loop regs:	<input>							<runtime>							<fill/texture>
	;
	;	d0											(temp)
	;	d1											(-)
	;	d2			xmin							(-)								
	;	d3			xmax-xmin-1						column counter						
	;	d4											s0
	;	d5			thing_num (*_hitbox)			(-)
	;	d6											u1s
	;	d7											du
	;
	;	a0											(temp)	
	;	a1											(temp)		(span read stream)
	;	a2											(temp)								texture pointer
	;	a3			frame							col_table														// [opt: _asm_tex_base]
	;	a4											LineCheat*								
	;	a5											RenderColumnInfo*
	;	a6																				return addr
	;	a7											render_buffer (dynamic)				write pointer


_Dread_LineCore_Sprite:

	move.l		a7, _a7_save


	lea.l		_render_columns, a5
	adda.w		d2, a5

	adda.w		#10, a3						; point to col_table

	move.l		d4, d0						;		int size = (((word)s1)>>3) & ~3;
	lsr.w		#3, d0
	and.w		#$FFFC, d0
	lea.l		_SIZE_LINE_CHEAT, a4		;		const LineCheat *lc = (LineCheat*)((byte*)SIZE_LINE_CHEAT + size);
	move.l		(a4,d0.w), a4

	bra.b		.start


.loop:										;	for( int x=xmin; x<xmax; x++ )
											;	{

	add.l		d7, d6						;		u1s += du;
	addq.w		#8, a5

.start:										;		// FIRST ITERATION STARTS HERE

	cmp.w		rc_size_limit(a5), d4		;	size0 ? size_limit
	blo			.continue

											;					== Texcoord
	move.l		d6, d0						;						int tx = u1s >> 16;
	swap.w		d0
	and.w		#$FFFE, d0
											

	; a3	= &span_offsets[1]
	; a1	= _tex_base
	; d0.w	= tx<<1
	move.l		a3, a1						; a1 = pointer to first span info
	add.w		(a3,d0.w), a1

.start_span:
	move.w		(a1)+, d2
	beq.b		.continue

	move.l		_asm_tex_base(pc), a2		; a2 = pointer to texture data		(TBD: compute correctly for each span)
	add.w		d2, a2						; (first span pixel)
	
											;					=== Span range
	move.w		(a1)+, d0					; y1 = (a1)+
	sub.w		sprite_y_offs, d0
	muls.w		lc_real_size(a4), d0
	swap.w		d0
	and.w		#$FFFC, d0

	move.w		(a1)+, d1					; y2 = (a1)+
	sub.w		sprite_y_offs, d1
	muls.w		lc_real_size(a4), d1
	swap.w		d1
	and.w		#$FFFC, d1



	cmp.w		#-ENGINE_Y_MID*4, d0					;	if( y0 < -ENGINE_Y_MID ) y0 = -ENGINE_Y_MID;
	bge.w		.y0_noclip
	move.w		#-ENGINE_Y_MID*4, d0
.y0_noclip:
	cmp.w		#(ENGINE_Y_MAX-ENGINE_Y_MID)*4, d1		;	if( y1 >  ENGINE_Y_MAX-ENGINE_Y_MID ) y1 =  ENGINE_Y_MAX-ENGINE_Y_MI;
	ble.w		.y1_noclip
	move.w		#(ENGINE_Y_MAX-ENGINE_Y_MID)*4, d1
.y1_noclip:
	cmp.w		d1, d0						;	if( y0 >= y1 ) continue;
	bge.w		.start_span


	move.l		rc_sprite_render_addr(a5), a7			;		byte *dst = render_buffer + (x>>2) + (x&3)*4000;
	add.w		#ENGINE_Y_MID*2, a7
	move.w		d0, d2
	asr.w		#1, d2
	add.w		d2, a7						;	skip



	move.l		30(a4), a0					;	fn2					LUT.l:	Scaler func			(jump directly to proper Y offset)		(Z = -64 / -128)
	move.w		#$4ED6,(a0,d1.w)			;	trim (fn2,d0)		LUT.w:	Tex length			(x4)									(L = -64..0 / -128..0)
	lea.l		.upper_ret(pc), a6
	jmp			(a0,d0.w)						;	jsr (fn2)
.upper_ret:
	move.w		#$1EEA,(a0,d1.w)			;	untrim (fn2,d0)		$116A -> $1EEA

	bra.b		.start_span

.continue:
	dbra.w		d3, .loop					; LOOP END

.end:
	move.l		_a7_save, a7
	rts




_Dread_LineCore_Sprite_hitbox:

	move.l		a7, _a7_save


	lea.l		_render_columns, a5
	adda.w		d2, a5

	adda.w		#10, a3						; point to col_table

	move.l		d4, d0						;		int size = (((word)s1)>>3) & ~3;
	lsr.w		#3, d0
	and.w		#$FFFC, d0
	lea.l		_SIZE_LINE_CHEAT, a4		;		const LineCheat *lc = (LineCheat*)((byte*)SIZE_LINE_CHEAT + size);
	move.l		(a4,d0.w), a4

	bra.b		.start


.loop:										;	for( int x=xmin; x<xmax; x++ )
											;	{

	add.l		d7, d6						;		u1s += du;
	addq.w		#8, a5

.start:										;		// FIRST ITERATION STARTS HERE

	cmp.w		rc_size_limit(a5), d4		;	size0 ? size_limit
	blo			.continue
	move.w		d5, rc_thing_num(a5)		; thing index

											;					== Texcoord
	move.l		d6, d0						;						int tx = u1s >> 16;
	swap.w		d0
	and.w		#$FFFE, d0
											

	; a3	= &span_offsets[1]
	; a1	= _tex_base
	; d0.w	= tx<<1
	move.l		a3, a1						; a1 = pointer to first span info
	add.w		(a3,d0.w), a1

.start_span:
	move.w		(a1)+, d2
	beq.b		.continue

	move.l		_asm_tex_base(pc), a2		; a2 = pointer to texture data		(TBD: compute correctly for each span)
	add.w		d2, a2						; (first span pixel)
	
											;					=== Span range
	move.w		(a1)+, d0					; y1 = (a1)+
	sub.w		sprite_y_offs, d0
	muls.w		lc_real_size(a4), d0
	swap.w		d0
	and.w		#$FFFC, d0

	move.w		(a1)+, d1					; y2 = (a1)+
	sub.w		sprite_y_offs, d1
	muls.w		lc_real_size(a4), d1
	swap.w		d1
	and.w		#$FFFC, d1



	cmp.w		#-ENGINE_Y_MID*4, d0					;	if( y0 < -ENGINE_Y_MID ) y0 = -ENGINE_Y_MID;
	bge.w		.y0_noclip
	move.w		#-ENGINE_Y_MID*4, d0
.y0_noclip:
	cmp.w		#(ENGINE_Y_MAX-ENGINE_Y_MID)*4, d1		;	if( y1 >  ENGINE_Y_MAX-ENGINE_Y_MID ) y1 =  ENGINE_Y_MAX-ENGINE_Y_MI;
	ble.w		.y1_noclip
	move.w		#(ENGINE_Y_MAX-ENGINE_Y_MID)*4, d1
.y1_noclip:
	cmp.w		d1, d0						;	if( y0 >= y1 ) continue;
	bge.w		.start_span


	move.l		rc_sprite_render_addr(a5), a7					;		byte *dst = render_buffer + (x>>2) + (x&3)*4000;
	add.w		#ENGINE_Y_MID*2, a7
	move.w		d0, d2
	asr.w		#1, d2
	add.w		d2, a7						;	skip



	move.l		30(a4), a0					;	fn2					LUT.l:	Scaler func			(jump directly to proper Y offset)		(Z = -64 / -128)
	move.w		#$4ED6,(a0,d1.w)			;	trim (fn2,d0)		LUT.w:	Tex length			(x4)									(L = -64..0 / -128..0)
	lea.l		.upper_ret(pc), a6
	jmp			(a0,d0.w)						;	jsr (fn2)
.upper_ret:
	move.w		#$1EEA,(a0,d1.w)			;	untrim (fn2,d0)		$116A -> $1EEA

	bra.b		.start_span

.continue:
	dbra.w		d3, .loop					; LOOP END

.end:
	move.l		_a7_save, a7
	rts
