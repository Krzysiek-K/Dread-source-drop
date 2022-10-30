
	xref	_e_skyptr
	xref	_render_buffer
	xref	_render_columns
	xref	_FN_SCALERS
	xref	_FN_COLORFILL_START
	xref	_FN_COLORFILL
	xref	_multab6
	xref	_SIZE_LINE_CHEAT




; ================================================================ _Dread_FrameReset ================================================================



	public _Dread_FrameReset
_Dread_FrameReset:
	move.w		#160-1, d1
	lea.l		_render_columns+4, a0
	;move.l		#$FF3800C8, d0				;	ymin = -50*4;			ymax =  50*4;
	move.l		#$0000018C, d0				;	ymin = 0;				ymax =  99*4;
.loop:
	move.l		d0, (a0)
	addq.w		#8, a0
	dbra.w		d1, .loop
	rts




; ================================================================ _Dread_LineCore2 ================================================================

	public _asm_line_h1
	public _asm_line_h2
	public _asm_line_h3
	public _asm_line_ds
	public _asm_tex_base
_asm_line_h1:
	dc.w	0
_asm_line_h2:
	dc.w	0
_asm_line_h3:
	dc.w	0
_asm_line_ds:
	dc.w	0
_asm_tex_base:
	dc.l	0




	public _Dread_LineCore2
_Dread_LineCore2:
	
	; loop regs:
	;
	;	d0		- (temp)
	;	d1		- (temp)
	;	d2		- (temp)						initially xmin
	;	d3		- column counter				xmax-xmin
	;	d4		- s1
	;	d5		- (temp)						ceil/floor color
	;	d6		- u1s
	;	d7		r du
	;
	;	a0		- render_buffer (dynamic)
	;	a1		- (temp)						
	;	a2		- (temp)						tex
	;	a3		-								ScalerInfo *si
	;	a4		- tex_base
	;	a5		r EngineLine *line
	;	a6		- render_columns				RenderColumnInfo*		pointer to current column

	movem.l	d2-d7/a2-a6,-(a7)

	lea.l		_render_columns, a6
	adda.w		d2, a6
	move.l		_render_buffer, a0
	adda.w		2(a6), a0

	bra.b		.start

.loop:										;	for( int x=xmin; x<xmax; x++ )
											;	{

	add.w		_asm_line_ds(pc), d4		;		s1 += ds;
	add.l		d7, d6						;		u1s += du;
	addq.w		#8, a6
	adda.w		(a6), a0					;		byte *dst = render_buffer + (x>>2) + (x&3)*4000;

.start:										;		// FIRST ITERATION STARTS HERE

	move.l		d4, d0						;		int size = (((word)s1)>>2) & ~7;
	lsr.w		#2, d0
	and.w		#$FFF8, d0

	cmp.w		#1200<<3, d0				;		if( size>=NUM_SOFT_SIZES*8 ) size = NUM_SOFT_SIZES*8-1;
	ble.b		.size_noclamp
	move.w		#1200<<3, d0
.size_noclamp:

	lea.l		_FN_SCALERS, a3				;		const ScalerInfo *si = (ScalerInfo*)((byte*)FN_SCALERS + size);
	adda.w		d0, a3

										
	;===================== Draw ceiling
	move.w		4(a6), d0				; d0 = ymin		(column top)
	move.w		6(a6), d2				; d2 = ymax		(column bottom)
	move.w		_asm_line_h1(pc), d1	; d1 = y1		(current span end)
	muls.w		4(a3), d1
	swap.w		d1
	and.w		#$FFFC, d1
	cmp.w		d0, d1
	ble.w		.no_ceil				; y1 <= ymin

	move.b		34(a5), d5				; ceil color						(at this point we know ceiling will be filled)
	move.l		_FN_COLORFILL, a1		; fn = (word*)FN_COLORFILL;

	cmp.w		d2, d1					; y1 >= ymax
	bge.w		.ceil_only

										; fill d0..d1
	move.w		#$4E75,(a1,d1.w)		;		fn[y1*2] = 0x4E75;		// RTS
	jsr			(a1,d0.w)				;		((colorfill_fn_t)(fn+2*ymin))(dst, line->ceil);
	move.w		#$1145,(a1,d1.w)		;		fn[y1*2] = 0x1141;		// 0001 000 101 000 101			MOVE.b	D5, yyy(A0)							MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>

	move.w		d1, d0				

	;===================== Draw upper texture
.no_ceil:								; Current state:
										; d0 = ymin		(remaining column top)
										; d2 = ymax		(column bottom)

	move.w		_asm_line_h2(pc), d1	; d1 = y2		(span end)
	muls.w		4(a3), d1
	swap.w		d1
	and.w		#$FFFC, d1
	cmp.w		d0, d1
	ble.w		.no_texupper			; y2 <= previous span end		(e.g. when )


											; (at this point we know upper texture will be drawn will be filled, starting from d0..d1/d2)
	move.l		d6, d5						;		int tx = (u1s / s1)&63;
	divu.w		d4, d5
	and.w		#63, d5
	lea.l		(a4,d5.w), a2				;		const byte *tex = tex_base + tx;

	move.l		#_multab6+50*4, a1
	move.w		(a1,d0.w), d0

	cmp.w		d2, d1						; y2 >= ymax
	bge.w		.texupper_only

	move.w		(a1,d1.w), d5				; Save: d1, d2
	move.l		(a3), a1					;		fn = si->func_addr;
	move.w		#$4E75,(a1,d5.w)			;		fn[ymax*2] = 0x4E75;		// RTS
	jsr			(a1,d0.w)					;		((scaler_fn_t)(fn))(dst, tex_base + tx);				typedef void(*scaler_fn_t)(__a0 byte *dst, __a2 const byte *tex);
	move.w		#$116A,(a1,d5.w)			;		fn[y2*3] = 0x116A;		// 0001 000 101 101 010			MOVE.b	xxx(A2), yyy(A0)					MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>

	move.w		d1, d0				

	;===================== Gap for the other sector
.no_texupper:
	move.w		d0, 4(a6)					; ymin = d0		(new column top)

	move.w		_asm_line_h3(pc), d1		; d1 = y3		(gap end / floor start)
	muls.w		4(a3), d1
	swap.w		d1
	and.w		#$FFFC, d1
	cmp.w		d0, d1
	bge.b		.gap_exists					; y3 <= previous span end			(gap_exists - not really, jump is taken also when d1==d0)
	move.w		d0, d1
.gap_exists:
	cmp.w		d2, d1						; y3 >= ymax
	bge.b		.no_floor
	move.w		d1, 6(a6)					; ymax = d1		(new column bottom)

	move.b		35(a5), d5				; floor color
	move.l		_FN_COLORFILL, a1		; fn = (word*)FN_COLORFILL;

										; fill d1..d2
	move.w		#$4E75,(a1,d2.w)		;		fn[y1*2] = 0x4E75;		// RTS
	jsr			(a1,d1.w)				;		((colorfill_fn_t)(fn+2*ymin))(dst, line->ceil);
	move.w		#$1145,(a1,d2.w)		;		fn[y1*2] = 0x1141;		// 0001 000 101 000 101			MOVE.b	D5, yyy(A0)							MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>

.no_floor:
	dbra.w		d3, .loop				; LOOP END
	bra.w		.end


	;===================== Draw ceiling - special case when ceiling fills entire column
	; extra code at the end
	; - fill d0..d2 with ceil color
	; - continue loop
.ceil_only:
										; fill d0..d2
	move.w		#$4E75,(a1,d2.w)		;		fn[y1*2] = 0x4E75;		// RTS
	jsr			(a1,d0.w)				;		((colorfill_fn_t)(fn+2*ymin))(dst, line->ceil);
	move.w		#$1145,(a1,d2.w)		;		fn[y1*2] = 0x1141;		// 0001 000 101 000 101			MOVE.b	D5, yyy(A0)							MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
	move.w		d2, 4(a6)				; ymin = d2		(ymax, close the column)
	dbra.w		d3, .loop				; LOOP END
	bra.w		.end

	;===================== Draw upper texture - special case when upper texture fills entire column
	; extra code at the end
	; - fill d0..d2 with upper texture
	; - continue loop
.texupper_only:
	move.w		(a1,d2.w), d5				; Save: d1, d2
	move.l		(a3), a1					;		fn = si->func_addr;
	move.w		#$4E75,(a1,d5.w)			;		fn[ymax*2] = 0x4E75;		// RTS
	jsr			(a1,d0.w)					;		((scaler_fn_t)(fn))(dst, tex_base + tx);				typedef void(*scaler_fn_t)(__a0 byte *dst, __a2 const byte *tex);
	move.w		#$116A,(a1,d5.w)			;		fn[y2*3] = 0x116A;		// 0001 000 101 101 010			MOVE.b	xxx(A2), yyy(A0)					MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
	move.w		d2, 4(a6)					; ymin = d2		(ymax, close the column)
	dbra.w		d3, .loop				; LOOP END
	bra.w		.end

	;=====================

.end:
	movem.l		(a7)+,d2-d7/a2-a6
	rts



; ================================================================ _Dread_LineCoreCheat_0 ================================================================


		public _Dread_LineCoreCheat_0
_Dread_LineCoreCheat_0:
	
	; loop regs:
	;
	;	d0		- (temp)
	;	d1		- (temp)
	;	d2		- (temp)						initially xmin
	;	d3		- column counter				xmax-xmin
	;	d4		- s1
	;	d5		- (temp)						ceil/floor color
	;	d6		- u1s
	;	d7		r du
	;
	;	a0		- render_buffer (dynamic)
	;	a1		-								_FN_COLORFILL_START
	;	a2		- (temp)						texture pointer
	;	a3		- LineCheat*
	;	a4		-								fn2
	;	a5		r EngineLine *line
	;	a6		- render_columns				RenderColumnInfo*		pointer to current column

	movem.l	d2-d7/a2-a6,-(a7)

	lea.l		_render_columns, a6
	adda.w		d2, a6
	move.l		_render_buffer, a0
	adda.w		2(a6), a0
	move.l		_FN_COLORFILL_START, a1

	bra.b		.start

.loop:										;	for( int x=xmin; x<xmax; x++ )
											;	{

	add.w		_asm_line_ds(pc), d4		;		s1 += ds;
	add.l		d7, d6						;		u1s += du;
	addq.w		#8, a6
	adda.w		(a6), a0					;		byte *dst = render_buffer + (x>>2) + (x&3)*4000;

.start:										;		// FIRST ITERATION STARTS HERE

	move.l		d4, d0						;		int size = (((word)s1)>>3) & ~3;
	lsr.w		#3, d0
	and.w		#$FFFC, d0

	cmp.w		#1200<<2, d0				;		if( size>=NUM_SOFT_SIZES*4 ) size = NUM_SOFT_SIZES*4-1;
	ble.b		.size_noclamp
	move.w		#1200<<2, d0
.size_noclamp:

	lea.l		_SIZE_LINE_CHEAT, a3		;		const LineCheat *lc = (LineCheat*)((byte*)SIZE_LINE_CHEAT + size);
	move.l		(a3,d0), a3


											;					== Ceil
	move.b		34(a5), d5					;						ceil color
											;	fn1					CONST:	Fill func
	move.w		2(a3), d0					;	trim (fn1,d0)		LUT.w:	Ceil length			(x4)									(Z = -64 / -128)
	move.w		#$4E75,(a1,d0.w)
	jsr			(a1)						;	jsr (fn1)
	move.w		#$1145,(a1,d0.w)			;	untrim (fn1,d0)

											;					== Texcoord
	move.l		d6, d5						;						int tx = (u1s / s1)&63;
	divu.w		d4, d5
	and.w		#63, d5
	move.l		_asm_tex_base(pc), a2
	lea.l		(a2,d5.w), a2				;						const byte *tex = tex_base + tx;

											;					== Upper
	move.l		6(a3), a4					;	fn2					LUT.l:	Scaler func			(jump directly to proper Y offset)		(Z = -64 / -128)
	move.w		14(a3), d0					;	trim (fn2,d0)		LUT.w:	Tex length			(x6)									(L = -64..0 / -128..0)
	move.w		#$4E75,(a4,d0.w)
	jsr			(a4)						;	jsr (fn2)
	move.w		#$116A,(a4,d0.w)			;	untrim (fn2,d0)

											;					== Floor
	move.b		35(a5), d5					;						floor color
	move.w		20(a3), d0					;	jsr (fn1,d0)		LUT.w:	Fill func start		(x4)	(fills to screen end)
	jsr			(a1,d0.w)
											;

	move.w		#0, 6(a6)					;	ymax = 0										(block further visibility)



	dbra.w		d3, .loop				; LOOP END

.end:
	movem.l		(a7)+,d2-d7/a2-a6
	rts




; ================================================================ _Dread_LineCoreCheat_1 ================================================================


		public _Dread_LineCoreCheat_1
_Dread_LineCoreCheat_1:

	; loop regs:
	;
	;	d0		- (temp)
	;	d1		- (temp)
	;	d2		- (temp)						initially xmin
	;	d3		- column counter				xmax-xmin
	;	d4		- s1
	;	d5		- (temp)						ceil/floor color
	;	d6		- u1s
	;	d7		r du
	;
	;	a0		- render_buffer (dynamic)
	;	a1		-								_FN_COLORFILL_START
	;	a2		- (temp)						texture pointer
	;	a3		- LineCheat*
	;	a4		-								fn2
	;	a5		r EngineLine *line
	;	a6		- render_columns				RenderColumnInfo*		pointer to current column

	movem.l	d2-d7/a2-a6,-(a7)

	lea.l		_render_columns, a6
	adda.w		d2, a6
	move.l		_render_buffer, a0
	adda.w		2(a6), a0
	move.l		_FN_COLORFILL_START, a1

	bra.b		.start

.loop:										;	for( int x=xmin; x<xmax; x++ )
											;	{

	add.w		_asm_line_ds(pc), d4		;		s1 += ds;
	add.l		d7, d6						;		u1s += du;
	addq.w		#8, a6
	adda.w		(a6), a0					;		byte *dst = render_buffer + (x>>2) + (x&3)*4000;

.start:										;		// FIRST ITERATION STARTS HERE

	move.l		d4, d0						;		int size = (((word)s1)>>3) & ~3;
	lsr.w		#3, d0
	and.w		#$FFFC, d0

	cmp.w		#1200<<2, d0				;		if( size>=NUM_SOFT_SIZES*4 ) size = NUM_SOFT_SIZES*4-1;
	ble.b		.size_noclamp
	move.w		#1200<<2, d0
.size_noclamp:

	lea.l		_SIZE_LINE_CHEAT, a3		;		const LineCheat *lc = (LineCheat*)((byte*)SIZE_LINE_CHEAT + size);
	move.l		(a3,d0), a3


											;					== Ceil
	move.b		34(a5), d5					;						ceil color
											;	fn1					CONST:	Fill func
	move.w		4(a3), d0					;	trim (fn1,d0)		LUT.w:	Ceil length			(x4)									(Z = -64 / -128)
	move.w		#$4E75,(a1,d0.w)
	jsr			(a1)						;	jsr (fn1)
	move.w		#$1145,(a1,d0.w)			;	untrim (fn1,d0)

											;					== Texcoord
	move.l		d6, d5						;						int tx = (u1s / s1)&63;
	divu.w		d4, d5
	and.w		#63, d5
	move.l		_asm_tex_base(pc), a2
	lea.l		(a2,d5.w), a2				;						const byte *tex = tex_base + tx;

											;					== Upper
	move.l		10(a3), a4					;	fn2					LUT.l:	Scaler func			(jump directly to proper Y offset)		(Z = -64 / -128)
	move.w		16(a3), d0					;	trim (fn2,d0)		LUT.w:	Tex length			(x6)									(L = -64..0 / -128..0)
	move.w		#$4E75,(a4,d0.w)
	jsr			(a4)						;	jsr (fn2)
	move.w		#$116A,(a4,d0.w)			;	untrim (fn2,d0)

											;					== Floor
	move.b		35(a5), d5					;						floor color
	move.w		20(a3), d0					;	jsr (fn1,d0)		LUT.w:	Fill func start		(x4)	(fills to screen end)
	jsr			(a1,d0.w)
											;
	move.w		#0, 6(a6)					;	ymax = 0										(block further visibility)



	dbra.w		d3, .loop				; LOOP END

.end:
	movem.l		(a7)+,d2-d7/a2-a6
	rts




; ================================================================ _Dread_LineCoreCheat_2 ================================================================


		public _Dread_LineCoreCheat_2
_Dread_LineCoreCheat_2:

	; loop regs:
	;
	;	d0		- (temp)
	;	d1		- (temp)
	;	d2		- (temp)						initially xmin
	;	d3		- column counter				xmax-xmin
	;	d4		- s1
	;	d5		- (temp)						ceil/floor color
	;	d6		- u1s
	;	d7		r du
	;
	;	a0		- render_buffer (dynamic)
	;	a1		-								_FN_COLORFILL_START
	;	a2		- (temp)						texture pointer
	;	a3		- LineCheat*
	;	a4		-								fn2
	;	a5		r EngineLine *line
	;	a6		- render_columns				RenderColumnInfo*		pointer to current column

	movem.l	d2-d7/a2-a6,-(a7)

	lea.l		_render_columns, a6
	adda.w		d2, a6
	move.l		_render_buffer, a0
	adda.w		2(a6), a0
	move.l		_FN_COLORFILL_START, a1

	bra.b		.start

.loop:										;	for( int x=xmin; x<xmax; x++ )
											;	{

	add.w		_asm_line_ds(pc), d4		;		s1 += ds;
	add.l		d7, d6						;		u1s += du;
	addq.w		#8, a6
	adda.w		(a6), a0					;		byte *dst = render_buffer + (x>>2) + (x&3)*4000;

.start:										;		// FIRST ITERATION STARTS HERE

	move.l		d4, d0						;		int size = (((word)s1)>>3) & ~3;
	lsr.w		#3, d0
	and.w		#$FFFC, d0

	cmp.w		#1200<<2, d0				;		if( size>=NUM_SOFT_SIZES*4 ) size = NUM_SOFT_SIZES*4-1;
	ble.b		.size_noclamp
	move.w		#1200<<2, d0
.size_noclamp:

	lea.l		_SIZE_LINE_CHEAT, a3		;		const LineCheat *lc = (LineCheat*)((byte*)SIZE_LINE_CHEAT + size);
	move.l		(a3,d0), a3


											;					== Ceil
	move.b		34(a5), d5					;						ceil color
											;	fn1					CONST:	Fill func
	move.w		4(a3), d0					;	trim (fn1,d0)		LUT.w:	Ceil length			(x4)									(Z = -64 / -128)
	move.w		#$4E75,(a1,d0.w)
	jsr			(a1)						;	jsr (fn1)
	move.w		#$1145,(a1,d0.w)			;	untrim (fn1,d0)

											;					== Texcoord
	move.l		d6, d5						;						int tx = (u1s / s1)&63;
	divu.w		d4, d5
	and.w		#63, d5
	move.l		_asm_tex_base(pc), a2
	lea.l		(a2,d5.w), a2				;						const byte *tex = tex_base + tx;

											;					== Upper
	move.l		10(a3), a4					;	fn2					LUT.l:	Scaler func			(jump directly to proper Y offset)		(Z = -64 / -128)
	move.w		18(a3), d0					;	trim (fn2,d0)		LUT.w:	Tex length			(x6)									(L = -128..-64)
	move.w		#$4E75,(a4,d0.w)
	jsr			(a4)						;	jsr (fn2)
	move.w		#$116A,(a4,d0.w)			;	untrim (fn2,d0)
	move.w		2(a3), 4(a6)				;	ymin				LUT.w:	Opening ceil start	(x4)


											;					== Floor
	move.b		35(a5), d5					;						floor color
	move.w		20(a3), d0					;	jsr (fn1,d0)		LUT.w:	Fill func start		(x4)	(fills to screen end)
	jsr			(a1,d0.w)
											;
	move.w		d0, 6(a6)					;	ymax = d0												(fill start is fill end for the next sector)




	dbra.w		d3, .loop				; LOOP END

.end:
	movem.l		(a7)+,d2-d7/a2-a6
	rts




; ================================================================ _Dread_LineCoreCheat_3 ================================================================


		public _Dread_LineCoreCheat_3
_Dread_LineCoreCheat_3:
	
	; loop regs:
	;
	;	d0		- (temp)
	;	d1		- (temp)
	;	d2		- (temp)						initially xmin
	;	d3		- column counter				xmax-xmin
	;	d4		- s1
	;	d5		- (temp)						ceil/floor color
	;	d6		- u1s
	;	d7		r du
	;
	;	a0		- render_buffer (dynamic)
	;	a1		-								_FN_COLORFILL_START
	;	a2		- (temp)						texture pointer
	;	a3		- LineCheat*
	;	a4		-								fn2
	;	a5		r EngineLine *line
	;	a6		- render_columns				RenderColumnInfo*		pointer to current column

	movem.l	d2-d7/a2-a6,-(a7)

	lea.l		_render_columns, a6
	adda.w		d2, a6
	move.l		_render_buffer, a0
	adda.w		2(a6), a0
	move.l		_FN_COLORFILL_START, a1

	bra.b		.start

.loop:										;	for( int x=xmin; x<xmax; x++ )
											;	{

	add.w		_asm_line_ds(pc), d4		;		s1 += ds;
	add.l		d7, d6						;		u1s += du;
	addq.w		#8, a6
	adda.w		(a6), a0					;		byte *dst = render_buffer + (x>>2) + (x&3)*4000;

.start:										;		// FIRST ITERATION STARTS HERE

	move.w		6(a6), d1
	beq.w		.continue

	move.l		d4, d0						;		int size = (((word)s1)>>3) & ~3;
	lsr.w		#3, d0
	and.w		#$FFFC, d0

	cmp.w		#1200<<2, d0				;		if( size>=NUM_SOFT_SIZES*4 ) size = NUM_SOFT_SIZES*4-1;
	ble.b		.size_noclamp
	move.w		#1200<<2, d0
.size_noclamp:

	lea.l		_SIZE_LINE_CHEAT, a3		;		const LineCheat *lc = (LineCheat*)((byte*)SIZE_LINE_CHEAT + size);
	move.l		(a3,d0), a3


											;					== Ceil
	move.b		34(a5), d5					;						ceil color
											;	fn1					CONST:	Fill func
	move.w		2(a3), d0					;	trim (fn1,d0)		LUT.w:	Ceil length			(x4)									(Z = -64 / -128)
	move.w		#$4E75,(a1,d0.w)
	move.w		4(a6), d1					;	jsr (fn1,ymin)
	jsr			(a1,d1.w)
	move.w		#$1145,(a1,d0.w)			;	untrim (fn1,d0)

											;					== Texcoord
	move.l		d6, d5						;						int tx = (u1s / s1)&63;
	divu.w		d4, d5
	and.w		#63, d5
	move.l		_asm_tex_base(pc), a2
	lea.l		(a2,d5.w), a2				;						const byte *tex = tex_base + tx;

											;					== Upper
	move.l		6(a3), a4					;	fn2					LUT.l:	Scaler func			(jump directly to proper Y offset)		(Z = -64 / -128)
	move.w		14(a3), d0					;	trim (fn2,d0)		LUT.w:	Tex length			(x6)									(L = -64..0 / -128..0)
	move.w		#$4E75,(a4,d0.w)
	jsr			(a4)						;	jsr (fn2)
	move.w		#$116A,(a4,d0.w)			;	untrim (fn2,d0)

											;					== Floor
	move.b		35(a5), d5					;						floor color
	move.w		6(a6), d1					;	trim (fn1,ymax)
	move.w		#$4E75,(a1,d1.w)
	move.w		20(a3), d0					;	jsr (fn1,d0)		LUT.w:	Fill func start		(x4)	(fills to screen end)
	jsr			(a1,d0.w)
	move.w		#$1145,(a1,d1.w)			;	untrim (fn1,ymax)
											;

	move.w		#0, 6(a6)					;	ymax = 0										(block further visibility)


.continue:
	dbra.w		d3, .loop				; LOOP END

.end:
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
