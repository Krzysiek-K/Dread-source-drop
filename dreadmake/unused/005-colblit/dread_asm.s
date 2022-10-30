
	xref	_e_skyptr
	xref	_render_buffer
	xref	_render_offsets
	xref	_FN_COLBLITS


	public _Dread_LineCore_Simple
_Dread_LineCore_Simple:
	movem.l	a3/a5/a6,-(a7)

	moveq.l		#-64, d1				; 0xC0 = floor color
	move.l		_e_skyptr, a3
	lea.l		_render_offsets, a6

	move.l		(a3,d2), a1					;		const byte *sky = e_skyptr[x];
	move.l		_render_buffer, a0
	adda.w		(a6,d2), a0
	;adda.w		(a6,d2), a0					;		byte *dst = render_buffer + (x>>2) + (x&3)*4000;

	lea.l		_FN_COLBLITS, a5			;		FN_COLBLITS[size](dst, src, sky, 0xC0);				void(__a0 byte *dst, __a2 const byte *tex, __a1 byte *sky, __d1 byte floor)
	move.l		(a5, d0), a5
	jsr			(a5)			

	movem.l	(a7)+,a3/a5/a6
	rts




	public _Dread_LineCore
_Dread_LineCore:
	
	; loop regs:
	;
	;	d0		- (temp)
	;	d1		- floor color
	;	d2		- x, xmin
	;	d3		r xmax
	;	d4		- s1
	;	d5		r ds
	;	d6		- u1s
	;	d7		r du
	;
	;	a0		- render_buffer (dynamic)
	;	a1		- (temp)
	;	a2		- (temp)
	;	a3		- e_skyptr
	;	a4		r line->tex_upper
	;	a5		- (temp)
	;	a6		- render_offsets

	movem.l	d2/d4/d6/a0-a3/a5/a6,-(a7)

	moveq.l		#-64, d1					; 0xC0 = floor color
	move.l		_e_skyptr, a3
	move.l		_render_buffer, a0
	lea.l		_render_offsets, a6


.loop:										;	for( int x=xmin; x<xmax; x++ )
											;	{
										
	move.l		d6, d0						;		int tx = (u1s / s1)&63;
	divu.w		d4, d0
	and.l		#63, d0
	lea.l		(a4,d0), a2					;		const byte *src = line->tex_upper + tx;

	move.l		d4, d0						;		int size = s1>>8;					<size> is shifted 2 bits left here
	lsr.l		#6, d0
	and.w		#$FFFC, d0
	bne.b		.size_1						;		if( size<1 ) size = 1;
	moveq.l		#4, d0

.size_1:
	cmp.w		#400, d0					;		if( size>100 ) size = 100;
	ble.b		.size_100
	move.w		#400, d0

.size_100:
										
	move.l		(a3,d2), a1					;		const byte *sky = e_skyptr[x];
	move.l		_render_buffer, a0
	adda.w		(a6,d2), a0
	;move.w		(a6,d2), a0					;		byte *dst = render_buffer + (x>>2) + (x&3)*4000;
										
	lea.l		_FN_COLBLITS, a5			;		COLBLITS[size](dst, src, sky, 0xC0);				void(__a0 byte *dst, __a2 const byte *tex, __a1 byte *sky, __d1 byte floor)
	move.l		(a5, d0), a5
	jsr			(a5)			

	add.w		d5, d4						;		s1 += ds;
	add.l		d7, d6						;		u1s += du;
	
	addq.w		#4, d2						;	}
	cmp.w		d3, d2
	blt.b		.loop

	movem.l		(a7)+,d2/d4/d6/a0-a3/a5/a6
	rts
