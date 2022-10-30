
	xref	_view_pos_x
	xref	_view_pos_y
	xref	_view_rotate_dx
	xref	_view_rotate_dy
	xref	_e_lines
	xref	_e_condition_states

	public _Dread_LineWall_Fix2_asm


  if WALL_RENDERER_VERSION == 1
	xref	_CHEAT_MODES
  else
	xref	_RENDER2_LINEDEF_MODES
  endif


	;	d0
	;	d1
	;	d2		p1x
	;	d3		p1y
	;	d4		p2x
	;	d5		p2y
	;	d6
	;	d7		mode
	;
	;	a0		cheat_mode
	;	a1
	;	a2
	;	a3
	;	a4
	;	a5		line
	;	a6
	;


; ================================================================ _Dread_LineWall_Fix2_asm ================================================================

_Dread_LineWall_Fix2_asm:

	;movem.l		d2-d7,-(a7)

	move.l		line_v1(a5), a0					;	EngineVertex *v = line->v1;
	move.w		vtx_tr_x(a0), d2				;	if( !v->tr_x )
	bne.b		.v1_transformed					;	{
												
	move.w		vtx_xp(a0), d0					;		short xp = (v->xp<<ENGINE_SUBVERTEX_PRECISION) - view_pos_x;					[d0]
	lsl.w		#ENGINE_SUBVERTEX_PRECISION, d0	;																							// TBD: move this shift to precompute
	sub.w		_view_pos_x, d0
												
	move.w		vtx_yp(a0), d3					;		short yp = (v->yp<<ENGINE_SUBVERTEX_PRECISION) - view_pos_y;					[d3]
	lsl.w		#ENGINE_SUBVERTEX_PRECISION, d3	;
	sub.w		_view_pos_y, d3

	move.w		d0, d2							;		v->tr_x = (muls(xp, view_rotate_dx) - muls(yp, view_rotate_dy))>>14;			[d2]
	muls.w		_view_rotate_dx, d2
	move.w		d3, d1
	muls.w		_view_rotate_dy, d1
	sub.l		d1, d2
	lsl.l		#2, d2
	swap.w		d2

	muls.w		_view_rotate_dy, d0				;		v->tr_y = (muls(xp, view_rotate_dy) + muls(yp, view_rotate_dx))>>14;			[d3]
	muls.w		_view_rotate_dx, d3
	add.l		d0, d3
	lsl.l		#2, d3
	swap.w		d3

	move.w		d2, vtx_tr_x(a0)
	move.w		d3, vtx_tr_y(a0)
	bra.b		.v1_done

.v1_transformed:
	move.w		vtx_tr_y(a0), d3
.v1_done:


	move.l		line_v2(a5), a0					;	EngineVertex *v = line->v2;
	move.w		vtx_tr_x(a0), d4				;	if( !v->tr_x )
	bne.b		.v2_transformed					;	{
												
	move.w		vtx_xp(a0), d0					;		short xp = (v->xp<<ENGINE_SUBVERTEX_PRECISION) - view_pos_x;					[d0]
	lsl.w		#ENGINE_SUBVERTEX_PRECISION, d0	;
	sub.w		_view_pos_x, d0
												
	move.w		vtx_yp(a0), d5					;		short yp = (v->yp<<ENGINE_SUBVERTEX_PRECISION) - view_pos_y;					[d5]
	lsl.w		#ENGINE_SUBVERTEX_PRECISION, d5	;
	sub.w		_view_pos_y, d5

	move.w		d0, d4							;		v->tr_x = (muls(xp, view_rotate_dx) - muls(yp, view_rotate_dy))>>14;			[d4]
	muls.w		_view_rotate_dx, d4
	move.w		d5, d1
	muls.w		_view_rotate_dy, d1
	sub.l		d1, d4
	lsl.l		#2, d4
	swap.w		d4

												;		v->tr_y = (muls(xp, view_rotate_dy) + muls(yp, view_rotate_dx))>>14;			[d5]
	muls.w		_view_rotate_dy, d0
	muls.w		_view_rotate_dx, d5
	add.l		d0, d5
	lsl.l		#2, d5
	swap.w		d5

	move.w		d4, vtx_tr_x(a0)
	move.w		d5, vtx_tr_y(a0)
	bra.b		.v2_done

.v2_transformed:
	move.w		vtx_tr_y(a0), d5
.v2_done:

  if WALL_RENDERER_VERSION == 1
	lsl.w		#4, d7
	lea.l		_CHEAT_MODES, a0
	lea.l		(a0,d7.w), a0
  else
	; lea.l		_EXPERIMENTAL_LINEDEF_MODES, a0
	lsl.w		#4, d7
	lea.l		_RENDER2_LINEDEF_MODES, a0
	lea.l		(a0,d7.w), a0
  endif

	move.w		cheatmode_clip(a0), d0				;		int clip = MODE_CLIP[mode];							TBD: ENGINE_SUBVERTEX_PRECISION

													;		if( p1y<clip && p2y<clip ) return;
	cmp.w		d0, d3								; p1y ? clip
	bge.b		.visible
	cmp.w		d0, d5
	bge.b		.visible
	bra.w		.end_skip

.visible:

	and.w		#1, d0
	beq.w		__Dread_LineWall_Fix2_asm_notex		; alternative branch - no texcoords

	move.w		line_xcoord1(a5), d6				;		word tx1 = line->xcoord1;			[d6]
	move.w		line_xcoord2(a5), d7				;		word tx2 = line->xcoord2;			[d7]

	move.w		d2, d0								;		short d1 = p1y+p1x;					[d0]
	add.w		d3, d0
	bpl.b		.no_clip1							;		if( d1<0 ) {
	move.w		d4, d1								;			short d2 = p2y+p2x;				[d1]
	add.w		d5, d1
	bmi.w		.end_skip							;			if( d2<0 ) return;
	sub.w		d0, d1								;			d2 -= d1;
	
	move.w		d7, d3								;			tx1 -= divs(muls(tx2-tx1, d1), d2);				[d3 used as temp - p1y not needed now]
	sub.w		d6, d3								;
	muls.w		d0, d3								;
	divs.w		d1, d3								;
	sub.w		d3, d6								;

	move.w		d4, d3								;			p1x -= divs(muls(p2x-p1x, d1), d2);				[d3 used as temp]
	sub.w		d2, d3								;
	muls.w		d0, d3								;
	divs.w		d1, d3								;
	sub.w		d3, d2								;
	
	move.w		d2, d3								;			p1y = -p1x;
	neg.w		d3
.no_clip1:											;		}


	move.w		d5, d1								;		short d2 = p2y-p2x;									[d1]
	sub.w		d4, d1								;
	bpl.b		.no_clip2							;		if( d2<0 ) {
	move.w		d3, d0								;			short d1 = p1y-p1x;								[d0]
	sub.w		d2, d0								;
	bmi.w		.end_skip							;			if( d1<0 ) return;
	sub.w		d1, d0								;			d1 -= d2;
	
	move.w		d2, d5								;			p2x -= divs(muls(p1x-p2x, d2), d1);				[d5 used as temp - p2y not needed now]
	sub.w		d4, d5								;
	muls.w		d1, d5								;
	divs.w		d0, d5								;
	sub.w		d5, d4								;

	move.w		d6, d5								;			tx2 -= divs(muls(tx1-tx2, d2), d1);				[d5 used as temp]
	sub.w		d7, d5								;
	muls.w		d1, d5								;
	divs.w		d0, d5								;
	sub.w		d5, d7								;

	move.w		d4, d5								;			p2y = p2x;
.no_clip2:											;		}


	sub.w		cheatmode_clip(a0), d3				;		p1y -= clip;
	bpl.b		.clip3_case1						;		if( p1y<0 ) {
	sub.w		cheatmode_clip(a0), d5				;			p2y -= clip;
	bmi.w		.end_skip							;			if( p2y<0 ) return;

	move.w		d3, d1								;			p1y - p2y										[d1]
	sub.w		d5, d1								;
	
	move.w		d6, d0								;			tx1 -= divs(muls(p1y, tx1-tx2), p1y-p2y);
	sub.w		d7, d0								;
	muls.w		d3, d0								;
	divs.w		d1, d0								;
	sub.w		d0, d6								;

	move.w		d2, d0								;			p1x -= divs(muls(p1y, p1x-p2x), p1y-p2y);
	sub.w		d4, d0								;
	muls.w		d3, d0								;
	divs.w		d1, d0								;
	sub.w		d0, d2								;

	moveq.l		#0, d3								;			p1y = 0;
	bra.b		.clip3_end							;		}

.clip3_case1:										;		else {
	sub.w		cheatmode_clip(a0), d5				;			p2y -= clip;
	bpl.b		.clip3_end							;			if( p2y<0 ) {

	move.w		d3, d1								;			p1y - p2y										[d1]
	sub.w		d5, d1								;

	move.w		d6, d0								;			tx2 -= divs(muls(tx1-tx2, p2y), p1y-p2y);
	sub.w		d7, d0								;
	muls.w		d5, d0								;
	divs.w		d1, d0								;
	sub.w		d0, d7								;
													
	move.w		d2, d0								;			p2x -= divs(muls(p2y, p1x-p2x), p1y-p2y);
	sub.w		d4, d0								;
	muls.w		d5, d0								;
	divs.w		d1, d0								;
	sub.w		d0, d4								;


	moveq.l		#0, d5								;			p2y = 0;
.clip3_end:											;		}	}

	add.w		cheatmode_clip(a0), d3				;		p1y += clip;
	add.w		cheatmode_clip(a0), d5				;		p2y += clip;

	lsl.w		#7, d6								;		tx1 <<= 7;
	lsl.w		#7, d7								;		tx2 <<= 7;

	muls.w		#ENGINE_ZOOM, d2					;		int xmin = divs(muls(p1x, ENGINE_ZOOM), p1y) + ENGINE_ZOOM;			[d2]
	divs.w		d3, d2
	add.w		#ENGINE_ZOOM, d2
	bpl.w		.xmin_ok							;		if( xmin<0 ) xmin = 0;
	moveq.l		#0, d2
.xmin_ok:

	muls.w		#ENGINE_ZOOM, d4					;		int xmax = divs(muls(p2x, ENGINE_ZOOM), p2y) + ENGINE_ZOOM;			[d4]
	divs.w		d5, d4
	add.w		#ENGINE_ZOOM, d4
	cmp.w		#ENGINE_WIDTH, d4					;		if( xmax>ENGINE_WIDTH ) xmax = ENGINE_WIDTH;
	ble.w		.xmax_ok
	move.w		#ENGINE_WIDTH, d4
.xmax_ok:
	sub.w		d2, d4								;		if( xmin>=xmax ) return;
	ble.w		.end_skip							;		int xlen = xmax - xmin;												[d4]


	move.l		#ENGINE_ZOOM_CONSTANT, d0			;		word s1 = divu(ENGINE_ZOOM_CONSTANT, p1y);							[d0]
	move.l		d0, d1
	divu.w		d3, d0
	divu.w		d5, d1								;		word s2 = divu(ENGINE_ZOOM_CONSTANT, p2y);							[d1]
  if WALL_RENDERER_VERSION == 2
	move.w		d1, _asm_line_s2
	move.w		d4, _asm_line_xlen
  endif	

	moveq.l		#0, d3								;		asm_line_ds = (short)divs((int)s2-(int)s1, xmax-xmin);				[d3 & d5 no longer needed]
	move.w		d0, d3
	moveq.l		#0, d5
	move.w		d1, d5
	sub.l		d3, d5
	divs.w		d4, d5
	move.w		d5, _asm_line_ds

													; At this point:
													;	d0:		s1
													;	d1:		s2
													;	d2:		xmin
													;	d3:		-
													;	d4:		xlen
													;	d5:		-
													;	d6:		tx1
													;	d7:		tx2

	move.w		d6, d3								;		unsigned long u1s = mulu(tx1, s1);					[d3.l]
	mulu.w		d0, d3
	move.w		d7, d5								;		unsigned long u2s = mulu(tx2, s2);					[d5.l]
	mulu.w		d1, d5

	swap.w		d2									;	[d2 temp / hide xmin]							
	swap.w		d4									;	[d4 temp / hide xlen]

	move.w		d0, d2								;		word ss = (s1>>1) + (s2>>1);						[d2]
	add.w		d1, d2								;
	roxr.w		#1, d2								;

	move.l		d3, d1								;		dword uu = (u1s>>1) + (u2s>>1);						[d1 - s2 no longer needed]
	add.l		d5, d1								;
	roxr.l		#1, d1								;

	move.w		d6, d4								; ((tx1+tx2)>>1)											[d4]
	add.w		d7, d4								;
	roxr.w		#1, d4								;
	
	divu.w		d2, d1								;		int err = ((tx1+tx2)>>1) - (int)divu(uu, ss);		[d4]
	sub.w		d1, d4								;
	bge.b		.err_pos							;		if( err<0 ) err = -err;
	neg.w		d4
.err_pos:
 if PERSPECTIVE_CORRECTION < 2
  if PERSPECTIVE_CORRECTION == 1
	sub.w		#4*128, d4							;		int ref = 4*128 + divu(10000000, ss);		// max error in texels, smaller sizes allow more error
	bcs.b		.nopersp							;
	move.l		#10000000, d1						;
	divu.w		d2, d1								;
	sub.w		d1, d4								;
	bcc.b		.persp								;		if( err < ref )
  endif
.nopersp:											;		{
	addq.l		#4, a0								;			fn = CHEAT_MODES[mode].fn_nopersp;				[cheat_mode is moved - no used for other purposes]
	moveq.l		#0, d3								;			u1s = tx1;
	move.w		d6, d3								;
	moveq.l		#0, d5								;			u2s = tx2;
	move.w		d7, d5								;
  endif
.persp:												;		}

	swap.w		d2									;	[d2: restore xmin]							
	swap.w		d4									;	[d4: restore xlen]

	sub.l		d3, d5								;	u2s-u1s													[d5 - u2s no longer needed]
													;		long du;											[d7]
	bcs.b		.du_neg								;		if( u1s <= u2s )
													;			du = (u2s-u1s)/(word)(xlen);				// result can be 32-bit!!
	swap.w		d5									;	[ d5 = lo : hi ]
	moveq.l		#0, d7								;
	move.w		d5, d7								;	[ d7 = 0 : hi ]
	divu.w		d4, d7								;
	swap.w		d7									;	[ d7 = divhi : remhi ]
	move.w		d7, d5								;	[ d5 = lo : remhi ]
	swap.w		d5									;	[ d5 = remhi : lo ]
	divu.w		d4, d5								;	[ d5 = - : divlo ]
	move.w		d5, d7								;	[ d7 = divhi : divlo ]
	bra.b		.du_end

.du_neg:											;		else
	neg.l		d5									;			du = -((u1s-u2s)/(word)(xlen));
	swap.w		d5									;	[ d5 = lo : hi ]
	moveq.l		#0, d7								;
	move.w		d5, d7								;	[ d7 = 0 : hi ]
	divu.w		d4, d7								;
	swap.w		d7									;	[ d7 = divhi : remhi ]
	move.w		d7, d5								;	[ d5 = lo : remhi ]
	swap.w		d5									;	[ d5 = remhi : lo ]
	divu.w		d4, d5								;	[ d5 = - : divlo ]
	move.w		d5, d7								;	[ d7 = divhi : divlo ]
	neg.l		d7									;
.du_end:

	move.l		line_tex_upper(a5), a1				;		asm_tex_base = line->tex_upper + ((-40 - line->y1)>>STRETCH_SCALING);
 if STRETCH_SCALING == 0
	add.w		#-40, a1							;
	sub.w		line_y1(a5), a1						;
	move.l		a1, _asm_tex_base					;
 else
  if WALL_RENDERER_VERSION == 1
	move.w		#-40, d1							;
	asr.w		#1, d1
	add.w		d1, a1								;
	move.l		a1, _asm_tex_base					;
  else
;	move.w		_view_pos_z, d1						;
;	asr.w		#1, d1
;	add.w		d1, a1								;
;	move.l		a1, _asm_tex_base					;
  endc
 endc

													; used:
													;	d0 -- d2 d3 d4 -- -- d7

													;	void(*line_cheat_fn)(__d2 int xmin, __d3 int xcount, __d4 dword s1, __d6 dword u1s, __d7 int du, __a5 EngineLine *line);
	lsl.w		#3, d2								;		d2	->	d2:	xmin<<3
	move.l		d3, d6								;		d3	->	d6:	u1s
	move.w		d4, d3								;		d4	->	d3:	xcount-1
	subq.w		#1, d3								;
	move.w		d0, d4								;		d0	->	d4:	s1
													;		d7	->	d7: du
													;		a5	->	a5: line				d1 d5 d6 - not used
;	TRAP_ICACHE_DISABLE
	move.l		(a0), a0
	jsr			(a0)					; TBD
	TRAP_ICACHE_ENABLE

.end:
	;movem.l		(a7)+,d2-d7
	;moveq.l		#1, d0				; TBD
	rts

.end_skip:
	;movem.l		(a7)+,d2-d7
	;moveq.l		#0, d0				; TBD
	rts


; ================================================================ __Dread_LineWall_Fix2_asm_notex ================================================================

	public __Dread_LineWall_Fix2_asm_notex		; do not call from C
__Dread_LineWall_Fix2_asm_notex:



	move.w		d2, d0								;		int d1 = p1y+p1x;					[d0]
	add.w		d3, d0
	bpl.b		.no_clip1							;		if( d1<0 ) {
	move.w		d4, d1								;			int d2 = p2y+p2x;				[d1]
	add.w		d5, d1
	bmi.w		.end_skip							;			if( d2<0 ) return;
	sub.w		d0, d1								;			d2 -= d1;
	
	move.w		d4, d3								;			p1x -= divs(muls(p2x-p1x, d1), d2);				[d3 used as temp]
	sub.w		d2, d3								;
	muls.w		d0, d3								;
	divs.w		d1, d3								;
	sub.w		d3, d2								;
	
	move.w		d2, d3								;			p1y = -p1x;
	neg.w		d3
.no_clip1:											;		}


	move.w		d5, d1								;		int d2 = p2y-p2x;									[d1]
	sub.w		d4, d1								;
	bpl.b		.no_clip2							;		if( d2<0 ) {
	move.w		d3, d0								;			int d1 = p1y-p1x;								[d0]
	sub.w		d2, d0								;
	bmi.w		.end_skip							;			if( d1<0 ) return;
	sub.w		d1, d0								;			d1 -= d2;
	
	move.w		d2, d5								;			p2x -= divs(muls(p1x-p2x, d2), d1);				[d5 used as temp - p2y not needed now]
	sub.w		d4, d5								;
	muls.w		d1, d5								;
	divs.w		d0, d5								;
	sub.w		d5, d4								;

	move.w		d4, d5								;			p2y = p2x;
.no_clip2:											;		}


	sub.w		cheatmode_clip(a0), d3				;		p1y -= clip;
	bpl.b		.clip3_case1						;		if( p1y<0 ) {
	sub.w		cheatmode_clip(a0), d5				;			p2y -= clip;
	bmi.w		.end_skip							;			if( p2y<0 ) return;

	move.w		d3, d1								;			p1y - p2y										[d1]
	sub.w		d5, d1								;
	
	move.w		d2, d0								;			p1x -= divs(muls(p1y, p1x-p2x), p1y-p2y);
	sub.w		d4, d0								;
	muls.w		d3, d0								;
	divs.w		d1, d0								;
	sub.w		d0, d2								;

	moveq.l		#0, d3								;			p1y = 0;
	bra.b		.clip3_end							;		}

.clip3_case1:										;		else {
	sub.w		cheatmode_clip(a0), d5				;			p2y -= clip;
	bpl.b		.clip3_end							;			if( p2y<0 ) {

	move.w		d3, d1								;			p1y - p2y										[d1]
	sub.w		d5, d1								;

	move.w		d2, d0								;			p2x -= divs(muls(p2y, p1x-p2x), p1y-p2y);
	sub.w		d4, d0								;
	muls.w		d5, d0								;
	divs.w		d1, d0								;
	sub.w		d0, d4								;


	moveq.l		#0, d5								;			p2y = 0;
.clip3_end:											;		}

	add.w		cheatmode_clip(a0), d3				;		p1y += clip;
	add.w		cheatmode_clip(a0), d5				;		p2y += clip;

	muls.w		#80, d2								;		int xmin = divs(muls(p1x, zoom_i), p1y) + wolf_xoffs;				[d2]
	divs.w		d3, d2
	add.w		#80, d2
	bpl.w		.xmin_ok							;		if( xmin<0 ) xmin = 0;
	moveq.l		#0, d2
.xmin_ok:

	muls.w		#80, d4								;		int xmax = divs(muls(p2x, zoom_i), p2y) + wolf_xoffs;				[d4]
	divs.w		d5, d4
	add.w		#80, d4
	cmp.w		#160, d4							;		if( xmax>WOLFSCREEN_W ) xmax = WOLFSCREEN_W;
	ble.w		.xmax_ok
	move.w		#160, d4
.xmax_ok:
	sub.w		d2, d4								;		if( xmin>=xmax ) return;
	ble.w		.end_skip							;		int xlen = xmax - xmin;												[d4]


	move.l		#ENGINE_ZOOM_CONSTANT, d0			;		word s1 = divu((zoom_i*32)<<12, p1y);								[d0]
	move.l		d0, d1
	divu.w		d3, d0
	divu.w		d5, d1								;		word s2 = divu((zoom_i*32)<<12, p2y);								[d1]
  if WALL_RENDERER_VERSION == 2
	move.w		d1, _asm_line_s2
	move.w		d4, _asm_line_xlen
  endif

	moveq.l		#0, d3								;		asm_line_ds = (short)divs((int)s2-(int)s1, xmax-xmin);				[d3 & d5 no longer needed]
	move.w		d0, d3
	moveq.l		#0, d5
	move.w		d1, d5
	sub.l		d3, d5
	divs.w		d4, d5
	move.w		d5, _asm_line_ds

													; At this point:
													;	d0:		s1
													;	d1:		s2
													;	d2:		xmin
													;	d3:		-
													;	d4:		xlen
													;	d5:		-
													;	d6:		-
													;	d7:		-

													;	void(*line_cheat_fn)(__d2 int xmin, __d3 int xcount, __d4 dword s1, __d6 dword u1s, __d7 int du, __a5 EngineLine *line);
	lsl.w		#3, d2								;		d2	->	d2:	xmin<<3
	move.w		d4, d3								;		d4	->	d3:	xcount-1
	subq.w		#1, d3								;
	move.w		d0, d4								;		d0	->	d4:	s1
													;		a5	->	a5: line
	move.l		(a0), a0
	jsr			(a0)
	


.end:
	;movem.l		(a7)+,d2-d7
	;moveq.l		#1, d0
	rts

.end_skip:
	;movem.l		(a7)+,d2-d7
	;moveq.l		#0, d0
	rts
