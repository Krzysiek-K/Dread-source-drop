
	xref	_e_skyptr
	xref	_render_buffer
	xref	_render_columns
	xref	_FN_SCALERS
	xref	_FN_COLORFILL_START
	xref	_FN_COLORFILL_MID
	xref	_FN_COLORFILL_END
	xref	_FN_SKYCOPY
	xref	_multab6
	xref	_SIZE_LINE_CHEAT
	xref	_FUNC_COLORFILL
	xref	_FUNC_SKYCOPY
	xref	_render_layout_delta



NUM_SOFT_SIZES:		equ 1200


	public _asm_line_ds
	public _asm_tex_base
	public _asm_fn_colorfill_mid
	public _asm_fn_skycopy
	public _asm_render_buffer
_asm_line_ds:
	dc.w	0
_asm_tex_base:
	dc.l	0
_a7_save:
	dc.l	0
_asm_fn_colorfill_mid:
	dc.l	0
_asm_fn_skycopy:
	dc.l	0
_asm_render_buffer:
	dc.l	0


lc_ceil_len_m64				equ	  2
lc_ceil_len_m128			equ	  4
lc_upper_start_m64			equ	  6
lc_upper_start_m128			equ  10
lc_upper_len_m64_0			equ	 14
lc_upper_len_m128_0			equ	 16
lc_upper_len_m128_m64		equ	 18
lc_floor_start_0			equ  20
lc_hole_size_m64_0			equ	 22
lc_ceil_ymin_m64			equ	 24
lc_floor_length_0			equ	 26
lc_real_size				equ	 28
lc_scaler_fn				equ	 30
lc_upper_endpos_0			equ	 34
lc_upper_endpos_m64			equ	 36
lc_upper_len_m96_0			equ	 38
lc_upper_start_m96			equ	 40
lc_ceil_len_m96				equ  44
lc_upper_len_m96_m64		equ  46
lc_upper_endpos_m96			equ  48
lc_hole_size_m96_0			equ  50
lc_ceil_ymin_m96			equ  52
lc_upper_len_m128_m96		equ  54
lc_hole_size_m128_0			equ  56
lc_ceil_ymin_m128			equ  58

cheatmode_fn_persp			equ	  0
cheatmode_fn_nopersp		equ	  4
cheatmode_clip				equ	  8

vtx_xp						equ	  0
vtx_yp						equ	  2
vtx_tr_x					equ	  4
vtx_tr_y					equ	  6

line_v1						equ	  0
line_v2						equ	  4
line_tex_upper				equ  16
line_xcoord1				equ	 24
line_xcoord2				equ	 26
line_y1						equ	 28
line_ceil_col				equ  34
line_floor_col				equ  35
line_sizeof					equ  38

rc_ymin						equ   4
rc_ymax						equ   6
rc_size_limit				equ   (160*8+0)





	include	"gen/line_cheats.s"
	include	"dread_fix2_asm.s"



; ================================================================ _Dread_FrameReset ================================================================



	public _Dread_FrameReset
_Dread_FrameReset:
	movem.l		a2, -(a7)

	move.w		#160-1, d1
	lea.l		_render_columns, a0
	lea.l		_render_layout_delta, a1
	move.l		#$000000C8, d0				;	ymin = 0;				ymax =  100*2;

.loop:
	add.w		(a1)+, a2
	move.l		a2, (a0)+
	move.l		d0, (a0)+
	dbra.w		d1, .loop

	movem.l		(a7)+, a2
	rts





; ================================================================ _Dread_LineCoreCheat_0 ================================================================


		public _Dread_LineCoreCheat_0
_Dread_LineCoreCheat_0:
	
	; loop regs:	<input>							<runtime>							<fill/texture>
	;
	;	d0											(temp)
	;	d1											(-)
	;	d2			xmin							(-)								
	;	d3			xmax-xmin-1						column counter						
	;	d4											s1
	;	d5			ceil/floor color				floor/ceil color  (swap.w)			fill color
	;	d6											u1s
	;	d7											du
	;
	;	a0											(temp)	
	;	a1											_FN_COLORFILL_MID
	;	a2											(temp)								texture pointer
	;	a3											[opt: _asm_tex_base]
	;	a4											LineCheat*								
	;	a5			EngineLine *line				RenderColumnInfo*
	;	a6																				return addr
	;	a7											render_buffer (dynamic)				write pointer

	movem.l		d2-d7/a2-a6,-(a7)
	move.l		a7, _a7_save

											


	move.b		35(a5), d5					;						floor color
	swap.w		d5
	move.b		34(a5), d5					;						ceil color

	lea.l		_render_columns, a5
	adda.w		d2, a5
	move.l		_FN_COLORFILL_MID, a1
	move.l		_asm_tex_base(pc), a3

	bra.b		.start


.loop:										;	for( int x=xmin; x<xmax; x++ )
											;	{

	add.w		_asm_line_ds(pc), d4		;		s1 += _asm_line_ds;
	add.l		d7, d6						;		u1s += du;
	addq.w		#8, a5						;		RenderColumnInfo *column_info += 8;
	swap.w		d5

.start:										;		// FIRST ITERATION STARTS HERE
	move.l		_render_buffer, a7			;		void *write_ptr = _render_buffer + column_info.render_offs;				// 2(column_info)
	adda.w		2(a5), a7

	move.w		d4, d0						;		word size = s1 >> 3;
	lsr.w		#3, d0						;		
	and.w		#$FFFC, d0					;		size &= $FFFC;

	lea.l		_SIZE_LINE_CHEAT, a4		;		LineCheat *line_cheat = _SIZE_LINE_CHEAT + size;
	move.l		(a4,d0), a4


											;					== Ceil
	move.w		2(a4), d0					;	$ call_a6( _FN_COLORFILL_MID + line_cheat.ceil_len_m64 + 100 )		// "ceil"			(Z = -64 / -128)
	lea.l		.ceil_ret(pc), a6
	jmp			100(a1,d0.w)
.ceil_ret:

											;					== Texcoord
	move.l		d6, d0						;	word tx = u1s /divu/ s1;
	divu.w		d4, d0						;
	and.w		#$1F80, d0					;	tx &= $1F80;
	lea.l		(a3,d0.w), a2				;	void *tex @ a2 = _asm_tex_base + tx;

											;					== Upper
	move.l		6(a4), a0					;	void *fn = line_cheat.upper_start_m64;
	move.w		14(a4), d0					;	$ fn[line_cheat.upper_len_m64_0] = $4ED6;			(L = -64..0 / -128..0)
	move.w		#$4ED6,(a0,d0.w)
	lea.l		.upper_ret(pc), a6
	jmp			(a0)						;	$ call_a6(fn)				(Z = -64 / -128)
.upper_ret:
	move.w		#$1EEA,(a0,d0.w)			;	$ fn[line_cheat.upper_len_m64_0] = $1EEA;

											;					== Floor
	swap.w		d5
	move.w		20(a4), d0					;	jsr (fn1,d0)		LUT.w:	Fill func start		(x2)	(fills to screen end)
	lea.l		.floor_ret(pc), a6
	jmp			-100(a1,d0.w)
.floor_ret:
											;

	move.w		#0, 6(a5)					;	ymax = 0										(block further visibility)



	dbra.w		d3, .loop					; LOOP END

.end:
	move.l		_a7_save, a7
	movem.l		(a7)+,d2-d7/a2-a6
	rts



; ================================================================ _Dread_LineCoreCheat_1 ================================================================


		public _Dread_LineCoreCheat_1
_Dread_LineCoreCheat_1:
	
	; loop regs:	<input>							<runtime>							<fill/texture>
	;
	;	d0											(temp)
	;	d1											(-)
	;	d2			xmin							(-)								
	;	d3			xmax-xmin-1						column counter						
	;	d4											s1
	;	d5			ceil/floor color				floor/ceil color  (swap.w)			fill color
	;	d6											u1s
	;	d7											du
	;
	;	a0											(temp)	
	;	a1											_FN_COLORFILL_MID
	;	a2											(temp)								texture pointer
	;	a3											[opt: _asm_tex_base]
	;	a4											LineCheat*								
	;	a5			EngineLine *line				RenderColumnInfo*
	;	a6																				return addr
	;	a7											render_buffer (dynamic)				write pointer

	movem.l		d2-d7/a2-a6,-(a7)
	move.l		a7, _a7_save

	move.b		35(a5), d5					;						floor color
	swap.w		d5
	move.b		34(a5), d5					;						ceil color

	lea.l		_render_columns, a5
	adda.w		d2, a5
	move.l		_FN_COLORFILL_MID, a1
	;move.l		_asm_tex_base(pc), a3

	lsr.w		#1, d2				; SKY
	move.l		_e_skyptr, a3
	add.w		d2, a3

	bra.b		.start


.loop:										;	for( int x=xmin; x<xmax; x++ )
											;	{

	add.w		_asm_line_ds(pc), d4		;		s1 += ds;
	add.l		d7, d6						;		u1s += du;
	addq.w		#8, a5
	swap.w		d5

.start:										;		// FIRST ITERATION STARTS HERE
	move.l		_render_buffer, a7			;		byte *dst = render_buffer + (x>>2) + (x&3)*4000;
	adda.w		2(a5), a7

	move.l		d4, d0						;		int size = (((word)s1)>>3) & ~3;
	lsr.w		#3, d0
	and.w		#$FFFC, d0

	lea.l		_SIZE_LINE_CHEAT, a4		;		const LineCheat *lc = (LineCheat*)((byte*)SIZE_LINE_CHEAT + size);
	move.l		(a4,d0), a4


											;					== Ceil
											;	fn1					CONST:	Fill func
	move.w		4(a4), d0					;	jsr (fn1,d0)		LUT.w:	Ceil length			(x2)									(Z = -64 / -128)
	move.l		_FN_SKYCOPY, a0			; SKY [a1]
	move.l		(a3)+, a2
	lea.l		.ceil_ret(pc), a6
	jmp			100(a0,d0.w)
.ceil_ret:

											;					== Texcoord
	move.l		d6, d0						;						int tx = (u1s / s1)&63;
	divu.w		d4, d0
	and.w		#$1F80, d0
	move.l		_asm_tex_base(pc), a2	; SKY [a3]
	lea.l		(a2,d0.w), a2				;						const byte *tex = tex_base + tx;

											;					== Upper
	move.l		10(a4), a0					;	fn2					LUT.l:	Scaler func			(jump directly to proper Y offset)		(Z = -64 / -128)
	move.w		16(a4), d0					;	trim (fn2,d0)		LUT.w:	Tex length			(x4)									(L = -64..0 / -128..0)
	move.w		#$4ED6,(a0,d0.w)			;	$4E75 -> $4ED6
	lea.l		.upper_ret(pc), a6
	jmp			(a0)						;	jsr (fn2)
.upper_ret:
	move.w		#$1EEA,(a0,d0.w)			;	untrim (fn2,d0)		$116A -> $1EEA

											;					== Floor
	swap.w		d5
	move.w		20(a4), d0					;	jsr (fn1,d0)		LUT.w:	Fill func start		(x2)	(fills to screen end)
	lea.l		.floor_ret(pc), a6
	jmp			-100(a1,d0.w)
.floor_ret:
											;

	move.w		#0, 6(a5)					;	ymax = 0										(block further visibility)



	dbra.w		d3, .loop					; LOOP END

.end:
	move.l		_a7_save, a7
	movem.l		(a7)+,d2-d7/a2-a6
	rts








; ================================================================ _Dread_LineCoreCheat_2 ================================================================


		public _Dread_LineCoreCheat_2
_Dread_LineCoreCheat_2:
	
	; loop regs:	<input>							<runtime>							<fill/texture>
	;
	;	d0											(temp)
	;	d1											(-)
	;	d2			xmin							(-)								
	;	d3			xmax-xmin-1						column counter						
	;	d4											s1
	;	d5			ceil/floor color				floor/ceil color  (swap.w)			fill color
	;	d6											u1s
	;	d7											du
	;
	;	a0											(temp)	
	;	a1											_FN_COLORFILL_MID											
	;	a2											(temp)								texture pointer
	;	a3											[opt: _asm_tex_base]
	;	a4											LineCheat*								
	;	a5			EngineLine *line				RenderColumnInfo*
	;	a6																				return addr
	;	a7											render_buffer (dynamic)				write pointer

	movem.l		d2-d7/a2-a6,-(a7)
	move.l		a7, _a7_save

	move.b		35(a5), d5					;						floor color
	swap.w		d5
	move.b		34(a5), d5					;						ceil color

	lea.l		_render_columns, a5
	adda.w		d2, a5
	move.l		_FN_COLORFILL_MID, a1
	;move.l		_asm_tex_base(pc), a3

	lsr.w		#1, d2				; SKY
	move.l		_e_skyptr, a3
	add.w		d2, a3

	bra.b		.start


.loop:										;	for( int x=xmin; x<xmax; x++ )
											;	{

	add.w		_asm_line_ds(pc), d4		;		s1 += ds;
	add.l		d7, d6						;		u1s += du;
	addq.w		#8, a5
	swap.w		d5

.start:										;		// FIRST ITERATION STARTS HERE
	move.l		_render_buffer, a7			;		byte *dst = render_buffer + (x>>2) + (x&3)*4000;
	adda.w		2(a5), a7

	move.l		d4, d0						;		int size = (((word)s1)>>3) & ~3;
	lsr.w		#3, d0
	and.w		#$FFFC, d0

	lea.l		_SIZE_LINE_CHEAT, a4		;		const LineCheat *lc = (LineCheat*)((byte*)SIZE_LINE_CHEAT + size);
	move.l		(a4,d0), a4


											;					== Ceil
											;	fn1					CONST:	Fill func
	move.w		4(a4), d0					;	jsr (fn1,d0)		LUT.w:	Ceil length			(x2)									(Z = -64 / -128)
	move.l		_FN_SKYCOPY, a0			; SKY [a1]
	move.l		(a3)+, a2
	lea.l		.ceil_ret(pc), a6
	jmp			100(a0,d0.w)
.ceil_ret:

											;					== Texcoord
	move.l		d6, d0						;						int tx = (u1s / s1)&63;
	divu.w		d4, d0
	and.w		#$1F80, d0
	move.l		_asm_tex_base(pc), a2	; SKY [a3]
	lea.l		(a2,d0.w), a2				;						const byte *tex = tex_base + tx;

											;					== Upper
	move.l		10(a4), a0					;	fn2					LUT.l:	Scaler func			(jump directly to proper Y offset)		(Z = -64 / -128)
	move.w		18(a4), d0					;	trim (fn2,d0)		LUT.w:	Tex length			(x4)									(L = -64..0 / -128..0)
	move.w		#$4ED6,(a0,d0.w)			;	$4E75 -> $4ED6
	lea.l		.upper_ret(pc), a6
	jmp			(a0)						;	jsr (fn2)
.upper_ret:
	move.w		#$1EEA,(a0,d0.w)			;	untrim (fn2,d0)		$116A -> $1EEA

	move.w		24(a4), 4(a5)				;	ymin

											;					== Floor
	swap.w		d5
	add.w		22(a4), a7
	move.w		20(a4), d0					;	jsr (fn1,d0)		LUT.w:	Fill func start		(x2)	(fills to screen end)
	move.w		20(a4), 6(a5)				;	ymax
	lea.l		.floor_ret(pc), a6
	jmp			-100(a1,d0.w)
.floor_ret:
											;




	dbra.w		d3, .loop					; LOOP END

.end:
	move.l		_a7_save, a7
	movem.l		(a7)+,d2-d7/a2-a6
	rts



; ================================================================ _Dread_LineCoreCheat_3 ================================================================


		public _Dread_LineCoreCheat_3
_Dread_LineCoreCheat_3:
	
	; loop regs:	<input>							<runtime>							<fill/texture>
	;
	;	d0											(temp)
	;	d1											(-)
	;	d2			xmin							(-)								
	;	d3			xmax-xmin-1						column counter						
	;	d4											s1
	;	d5			ceil/floor color				floor/ceil color  (swap.w)			fill color
	;	d6											u1s
	;	d7											du
	;
	;	a0											(temp)	
	;	a1											_FN_COLORFILL_MID											
	;	a2											(temp)								texture pointer
	;	a3											[opt: _asm_tex_base]
	;	a4											LineCheat*								
	;	a5			EngineLine *line				RenderColumnInfo*
	;	a6																				return addr
	;	a7											render_buffer (dynamic)				write pointer

	movem.l		d2-d7/a2-a6,-(a7)
	move.l		a7, _a7_save

	move.b		35(a5), d5					;						floor color
	swap.w		d5
	move.b		34(a5), d5					;						ceil color

	lea.l		_render_columns, a5
	adda.w		d2, a5
	move.l		_FN_COLORFILL_MID, a1
	move.l		_asm_tex_base(pc), a3

	bra.b		.start


.loop:										;	for( int x=xmin; x<xmax; x++ )
											;	{

	add.w		_asm_line_ds(pc), d4		;		s1 += ds;
	add.l		d7, d6						;		u1s += du;
	addq.w		#8, a5
	swap.w		d5

.start:										;		// FIRST ITERATION STARTS HERE

	move.w		6(a5), d1
	beq.w		.continue

	move.l		_render_buffer, a7			;		byte *dst = render_buffer + (x>>2) + (x&3)*4000;
	adda.w		2(a5), a7

	move.l		d4, d0						;		int size = (((word)s1)>>3) & ~3;
	lsr.w		#3, d0
	and.w		#$FFFC, d0

	lea.l		_SIZE_LINE_CHEAT, a4		;		const LineCheat *lc = (LineCheat*)((byte*)SIZE_LINE_CHEAT + size);
	move.l		(a4,d0), a4


											;					== Ceil
											;	fn1					CONST:	Fill func
	move.w		2(a4), d0					;	jsr (fn1,d0)		LUT.w:	Ceil length			(x2)									(Z = -64 / -128)
	add.w		4(a5), a7					;		skip ymin
	add.w		4(a5), d0					;		shorten by ymin
	lea.l		.ceil_ret(pc), a6
	jmp			100(a1,d0.w)
.ceil_ret:

											;					== Texcoord
	move.l		d6, d0						;						int tx = (u1s / s1)&63;
	divu.w		d4, d0
	and.w		#$1F80, d0
	lea.l		(a3,d0.w), a2				;						const byte *tex = tex_base + tx;

											;					== Upper
	move.l		6(a4), a0					;	fn2					LUT.l:	Scaler func			(jump directly to proper Y offset)		(Z = -64 / -128)
	move.w		14(a4), d0					;	trim (fn2,d0)		LUT.w:	Tex length			(x4)									(L = -64..0 / -128..0)
	move.w		#$4ED6,(a0,d0.w)			;	$4E75 -> $4ED6
	lea.l		.upper_ret(pc), a6
	jmp			(a0)						;	jsr (fn2)
.upper_ret:
	move.w		#$1EEA,(a0,d0.w)			;	untrim (fn2,d0)		$116A -> $1EEA

											;					== Floor
	swap.w		d5
	move.w		20(a4), d2					;	jsr (fn1,d0)		LUT.w:	Fill func start		(x2)	(fills to screen end)
	sub.w		d1, d2
	lea.l		.floor_ret(pc), a6
	jmp			100(a1,d2.w)
.floor_ret:
											;

	move.w		#0, 6(a5)					;	ymax = 0										(block further visibility)



.continue:
	dbra.w		d3, .loop					; LOOP END

.end:
	move.l		_a7_save, a7
	movem.l		(a7)+,d2-d7/a2-a6
	rts



; ================================================================ _Dread_LineCoreCheat_4 ================================================================


		public _Dread_LineCoreCheat_4
_Dread_LineCoreCheat_4:
	
	; loop regs:	<input>							<runtime>							<fill/texture>
	;
	;	d0											(temp)
	;	d1											(-)
	;	d2			xmin							(-)								
	;	d3			xmax-xmin-1						column counter						
	;	d4											s1
	;	d5			ceil/floor color				floor/ceil color  (swap.w)			fill color
	;	d6											u1s
	;	d7											du
	;
	;	a0											(temp)	
	;	a1											_FN_COLORFILL_MID
	;	a2											(temp)								texture pointer
	;	a3											Sky map
	;	a4											LineCheat*								
	;	a5			EngineLine *line				RenderColumnInfo*
	;	a6																				return addr
	;	a7											render_buffer (dynamic)				write pointer

	movem.l		d2-d7/a2-a6,-(a7)
	move.l		a7, _a7_save

	move.b		35(a5), d5					;						floor color
	swap.w		d5
	move.b		34(a5), d5					;						ceil color

	lea.l		_render_columns, a5
	adda.w		d2, a5
	move.l		_FN_COLORFILL_MID, a1

	lsr.w		#1, d2
	move.l		_e_skyptr, a3
	add.w		d2, a3

	bra.b		.start


.loop:										;	for( int x=xmin; x<xmax; x++ )
											;	{

	add.w		_asm_line_ds(pc), d4		;		s1 += ds;
	add.l		d7, d6						;		u1s += du;
	addq.w		#8, a5
	swap.w		d5

.start:										;		// FIRST ITERATION STARTS HERE
	move.l		_render_buffer, a7			;		byte *dst = render_buffer + (x>>2) + (x&3)*4000;
	adda.w		2(a5), a7

	move.l		d4, d0						;		int size = (((word)s1)>>3) & ~3;
	lsr.w		#3, d0
	and.w		#$FFFC, d0

	lea.l		_SIZE_LINE_CHEAT, a4		;		const LineCheat *lc = (LineCheat*)((byte*)SIZE_LINE_CHEAT + size);
	move.l		(a4,d0), a4


											;					== Sky
											;	fn1					CONST:	Fill func
	move.w		2(a4), d0					;	jsr (fn1,d0)		LUT.w:	Ceil length			(x2)									(Z = -64 / -128)
	move.l		_FN_SKYCOPY, a0
	move.l		(a3)+, a2
	lea.l		.ceil_ret(pc), a6
	jmp			100(a0,d0.w)
.ceil_ret:

											;					== Texcoord
	move.l		d6, d0						;						int tx = (u1s / s1)&63;
	divu.w		d4, d0
	and.w		#$1F80, d0
	move.l		_asm_tex_base(pc), a2
	lea.l		(a2,d0.w), a2				;						const byte *tex = tex_base + tx;

											;					== Upper
	move.l		6(a4), a0					;	fn2					LUT.l:	Scaler func			(jump directly to proper Y offset)		(Z = -64 / -128)
	move.w		14(a4), d0					;	trim (fn2,d0)		LUT.w:	Tex length			(x4)									(L = -64..0 / -128..0)
	move.w		#$4ED6,(a0,d0.w)			;	$4E75 -> $4ED6
	lea.l		.upper_ret(pc), a6
	jmp			(a0)						;	jsr (fn2)
.upper_ret:
	move.w		#$1EEA,(a0,d0.w)			;	untrim (fn2,d0)		$116A -> $1EEA

											;					== Floor
	swap.w		d5
	move.w		20(a4), d0					;	jsr (fn1,d0)		LUT.w:	Fill func start		(x2)	(fills to screen end)
	lea.l		.floor_ret(pc), a6
	jmp			-100(a1,d0.w)
.floor_ret:
											;

	move.w		#0, 6(a5)					;	ymax = 0										(block further visibility)



	dbra.w		d3, .loop					; LOOP END

.end:
	move.l		_a7_save, a7
	movem.l		(a7)+,d2-d7/a2-a6
	rts





; ================================================================ _Dread_LineCore_Sprite ================================================================


		public _Dread_LineCore_Sprite
_Dread_LineCore_Sprite:
	
	; loop regs:	<input>							<runtime>							<fill/texture>
	;
	;	d0											(temp)
	;	d1											(-)
	;	d2			xmin							(-)								
	;	d3			xmax-xmin-1						column counter						
	;	d4											s0
	;	d5											(-)
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

	movem.l		d2-d7/a2-a6,-(a7)
	move.l		a7, _a7_save


	lea.l		_render_columns, a5
	adda.w		d2, a5

	adda.w		#10, a3						; point to col_table

	move.l		d4, d0						;		int size = (((word)s1)>>3) & ~3;
	lsr.w		#3, d0
	and.w		#$FFFC, d0
	lea.l		_SIZE_LINE_CHEAT, a4		;		const LineCheat *lc = (LineCheat*)((byte*)SIZE_LINE_CHEAT + size);
	move.l		(a4,d0), a4

	bra.b		.start


.loop:										;	for( int x=xmin; x<xmax; x++ )
											;	{

	add.l		d7, d6						;		u1s += du;
	addq.w		#8, a5
	swap.w		d5

.start:										;		// FIRST ITERATION STARTS HERE

	cmp.w		rc_size_limit(a5), d4		;	size0 ? size_limit
	blt.b		.continue


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
	move.w		(a1)+, d5
	beq.b		.continue
	;lea.l		(a1,d0.w), a2

	move.l		_asm_tex_base(pc), a2		; a2 = pointer to texture data		(TBD: compute correctly for each span)
	add.w		d5, a2					; (first span pixel)
	
											;					=== Span range
	move.w		(a1)+, d0					; y1 = (a1)+
	sub.w		#(0-40)<<6, d0
	muls.w		28(a4), d0
	swap.w		d0
	and.w		#$FFFC, d0

	move.w		(a1)+, d1					; y2 = (a1)+
	sub.w		#(0-40)<<6, d1
	muls.w		28(a4), d1
	swap.w		d1
	and.w		#$FFFC, d1



	cmp.w		#-50*4, d0					;	if( y0 < -50 ) y0 = -50;
	bge.w		.y0_noclip
	move.w		#-50*4, d0
.y0_noclip:
	cmp.w		#50*4, d1					;	if( y1 >  50 ) y1 =  50;
	ble.w		.y1_noclip
	move.w		#50*4, d1
.y1_noclip:
	cmp.w		d1, d0						;	if( y0 >= y1 ) continue;
	bge.w		.start_span


	move.l		(a5), a7					;		byte *dst = render_buffer + (x>>2) + (x&3)*4000;
	add.w		#50*2, a7
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
	movem.l		(a7)+,d2-d7/a2-a6
	rts


; ================================================================ _Dread_TestA7 ================================================================


	public _Dread_TestA7
_Dread_TestA7:
	move.l		a7, a1
	move.l		a0, a7

	move.b		d0, (a7)+		; draw 8 pixels
	move.b		d0, (a7)+
	move.b		d0, (a7)+
	move.b		d0, (a7)+
	move.b		d0, (a7)+
	move.b		d0, (a7)+
	move.b		d0, (a7)+
	move.b		d0, (a7)+
	move.l		a7, d0
	
	move.l		a1, a7
	rts
