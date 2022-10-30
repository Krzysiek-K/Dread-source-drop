
	xref	_e_skyptr
	xref	_render_buffer
	xref	_render_columns
	xref	_FN_SCALERS
	xref	_FN_COLORFILL_START
	xref	_FN_COLORFILL
	xref	_multab6
	xref	_SIZE_LINE_CHEAT



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
	;	a0		- return addr
	;	a1		-								_FN_COLORFILL_START
	;	a2		- (temp)						texture pointer
	;	a3		- LineCheat*
	;	a4		-								fn2
	;	a5		r EngineLine *line
	;	a6		- render_columns				RenderColumnInfo*		pointer to current column
	;	a7		- render_buffer (dynamic)

	movem.l	d2-d7/a2-a6,-(a7)
	move.l		a7, _a7_save

	lea.l		_render_columns, a6
	adda.w		d2, a6
	move.l		_FN_COLORFILL_START, a1

	bra.b		.start

.loop:										;	for( int x=xmin; x<xmax; x++ )
											;	{

	add.w		_asm_line_ds(pc), d4		;		s1 += ds;
	add.l		d7, d6						;		u1s += du;
	addq.w		#8, a6

.start:										;		// FIRST ITERATION STARTS HERE
	move.l		_render_buffer, a7			;		byte *dst = render_buffer + (x>>2) + (x&3)*4000;
	adda.w		2(a6), a7

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
	move.w		2(a3), d0					;	jsr (fn1,d0)		LUT.w:	Ceil length			(x4)									(Z = -64 / -128)
	lea.l		.ceil_ret(pc), a0
	jmp			(a1,d0.w)
.ceil_ret:

											;					== Texcoord
	move.l		d6, d5						;						int tx = (u1s / s1)&63;
	divu.w		d4, d5
	and.w		#63, d5
	move.l		_asm_tex_base(pc), a2
	lea.l		(a2,d5.w), a2				;						const byte *tex = tex_base + tx;

											;					== Upper
	move.l		6(a3), a4					;	fn2					LUT.l:	Scaler func			(jump directly to proper Y offset)		(Z = -64 / -128)
	move.w		14(a3), d0					;	trim (fn2,d0)		LUT.w:	Tex length			(x6)									(L = -64..0 / -128..0)
	move.w		#$4ED0,(a4,d0.w)			;	$4E75 -> $4ED0
	lea.l		.upper_ret(pc), a0
	jmp			(a4)						;	jsr (fn2)
.upper_ret:
	move.w		#$1EEA,(a4,d0.w)			;	untrim (fn2,d0)		$116A -> $1EEA

											;					== Floor
	move.b		35(a5), d5					;						floor color
	move.w		20(a3), d0					;	jsr (fn1,d0)		LUT.w:	Fill func start		(x4)	(fills to screen end)
	lea.l		.floor_ret(pc), a0
	jmp			(a1,d0.w)
.floor_ret:
											;

	move.w		#0, 6(a6)					;	ymax = 0										(block further visibility)



	dbra.w		d3, .loop					; LOOP END

.end:
	move.l		_a7_save, a7
	movem.l		(a7)+,d2-d7/a2-a6
	rts




; ================================================================ _Dread_LineCoreCheat_1 ================================================================


_a7_save:
	dc.l		0

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
