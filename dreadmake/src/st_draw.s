

		xref		_NUMFONT


	SECTION_CODE



; ================================================================ Debug_WritePerf ================================================================
;
;	a0	- screenpos
;	d0	- value
;
	public _Debug_WritePerf
_Debug_WritePerf:
	movem.l		d2/a2-a3,-(a7)

	lea.l		_NUMFONT, a1

	divu.w		#10, d0
	moveq.l		#0, d1
	move.w		d0, d1			; d1 = value/10
	swap.w		d0
	mulu.w		#6, d0			; d0 = value%10*6
	lea.l		(a1,d0.w), a3	;						const byte *D3 = NUMFONT + value%10*6;		value/=10;

	divu.w		#10, d1
	moveq.l		#0, d0
	move.w		d1, d0			; d0 = value/10
	swap.w		d1
	mulu.w		#6, d1
	lea.l		(a1,d1.w), a2	;						const byte *D2 = NUMFONT + value%10*6;		value/=10;

	mulu.w		#6, d0
	lea.l		(a1,d0.w), a1	;						const byte *D1 = NUMFONT + value*6;




									; ----------------
									; ####_._####_####
									;		bitmap[0] = (((word)*D1++)<<12) | (((word)*D2++)<<5) | (((word)*D3++));
									;		bitmap[1] = (((word)*D1++)<<12) | (((word)*D2++)<<5) | (((word)*D3++));
									;		bitmap[2] = (((word)*D1++)<<12) | (((word)*D2++)<<5) | (((word)*D3++));
									;		bitmap[3] = (((word)*D1++)<<12) | (((word)*D2++)<<5) | (((word)*D3++));
									;		bitmap[4] = (((word)*D1++)<<12) | (((word)*D2++)<<5) | (((word)*D3++));
									;		bitmap[5] = (((word)*D1++)<<12) | (((word)*D2++)<<5) | (((word)*D3++)) | 0x0400;
									;		
									;		for( int y=0; y<6; y++ )
									;		{
									;			word *dst = screenpos + 80*y;
									;			dst[0] = 0;
									;			dst[1] = bitmap[y];
									;			dst[2] = bitmap[y];
									;			dst[3] = 0;
									;		}

	moveq.l		#0, d0			; d0 = 0
	moveq.l		#5, d2

.loop:
	move.b		(a1)+, d1		; ------------####
	lsl.w		#7, d1			; -----####-------
	or.b		(a2)+, d1		; -----####---####
	lsl.w		#5, d1			; ####---####-----
	or.b		(a3)+, d1		; ####---####-####
	
	move.w		d0, (a0)+
	move.w		d1, (a0)+
	move.w		d1, (a0)+
	move.w		d0, (a0)+

	adda.w		#(80-4)*2, a0

	dbra.w		d2, .loop

	ori.b		#$04, -160+2(a0)
	ori.b		#$04, -160+4(a0)


	movem.l		(a7)+,d2/a2-a3
	rts


; ================================================================ ST_ClearScreen ================================================================
;
;	void ST_ClearScreen(__a0 word *screen);
;
	public _ST_ClearScreen
_ST_ClearScreen:
	movem.l		d2-d7/a2-a6,-(a7)

	;	d0 - counter
	;	a0 - target
	;
	moveq.l		#0, d1
	move.l		d1, d2
	move.l		d1, d3
	move.l		d1, d4
	move.l		d1, d5
	move.l		d1, d6
	move.l		d1, d7
	move.l		d1, a1
	move.l		d1, a2
	move.l		d1, a3
	move.l		d1, a4
	move.l		d1, a5
	move.l		d1, a6
	
	adda.w		#32000, a0				; start from the end

	move.w		#615/15-1, d0
.loop:
	movem.l		d1-d7/a1-a6,-(a0)
	movem.l		d1-d7/a1-a6,-(a0)
	movem.l		d1-d7/a1-a6,-(a0)
	movem.l		d1-d7/a1-a6,-(a0)
	movem.l		d1-d7/a1-a6,-(a0)
	movem.l		d1-d7/a1-a6,-(a0)
	movem.l		d1-d7/a1-a6,-(a0)
	movem.l		d1-d7/a1-a6,-(a0)
	movem.l		d1-d7/a1-a6,-(a0)
	movem.l		d1-d7/a1-a6,-(a0)
	movem.l		d1-d7/a1-a6,-(a0)
	movem.l		d1-d7/a1-a6,-(a0)
	movem.l		d1-d7/a1-a6,-(a0)
	movem.l		d1-d7/a1-a6,-(a0)
	movem.l		d1-d7/a1-a6,-(a0)
	dbra.w		d0, .loop

	movem.l		d1-d5,-(a0)				; clear leftover


	movem.l		(a7)+,d2-d7/a2-a6
	rts



; ================================================================ ST_DrawWeaponAsm ================================================================
;
;	a0 - word *screen
;	a1 - const WeaponFrameInfoST *info
;
	public _ST_DrawWeaponAsm
_ST_DrawWeaponAsm:
;
;	d0	- stride
;	d1	- Y counter
;	d2	- X counter
;	d3	- mask
;	d4	- tmp
;
;	a0	- dst			word*
;	a1	- info			WeaponFrameInfoST*
;	a2	- src			word*
;
	movem.l		d2-d4/a2,-(a7)


	adda.w		st_wpinfo_screen_start(a1), a0		;		word *dst = screen + info->screen_start + (ENGINE_Y_MAX-100)*40*4;
	suba.w		#(100-ENGINE_Y_MAX)*40*8, a0


	moveq.l		#40, d0								;		word stride = 40*4 - info->width*4;
	sub.w		st_wpinfo_width(a1), d0
	lsl.w		#3, d0

	move.l		st_wpinfo_data(a1), a2				;		const word *src = info->data;
	
	move.w		st_wpinfo_height(a1), d1			;		for( word y=0; y<info->height; y++ )
	subq.l		#1, d1								;		{
.y_loop:											
	move.w		st_wpinfo_width(a1), d2				;			for( word x=0; x<info->width; x++ )
	subq.l		#1, d2								;			{
.x_loop:
	move.w		(a2)+, d3							;				word mask = *src++;

	beq			.empty								;				if( mask )
													;				{
	not.w		d3									;					mask ^= 0xFFFF;
	move.w		d3, d4
	swap.w		d3
	move.w		d4, d3

	move.l		(a0), d4							;					*dst = (*dst & mask) | *src++;		dst++;
	and.l		d3, d4								;					*dst = (*dst & mask) | *src++;		dst++;
	or.l		(a2)+, d4
	move.l		d4, (a0)+

	move.l		(a0), d4							;					*dst = (*dst & mask) | *src++;		dst++;
	and.l		d3, d4								;					*dst = (*dst & mask) | *src++;		dst++;
	or.l		(a2)+, d4
	move.l		d4, (a0)+

													;				}
	bra			.cont_x								;				else

.empty:
	adda.w		#8, a0								;					dst+=4;
.cont_x:
	dbra		d2, .x_loop							;			}

	adda.w		d0, a0								;			dst += stride;

	dbra		d1, .y_loop							;		}

	movem.l		(a7)+,d2-d4/a2
	rts




; ================================================================ ST_DrawClearMaskedNarrow ================================================================
;
;	Draws narrow (up to 16px) bitmap clearing the background behind it.
;	Background bitmap is assumed to be headerless ST bitmap data, 320px wide.
;
;	d2	IN	- X position
;	d3	IN	- Y position
;
;	a0	IN	- screen
;	a1	IN	- glyph					(masked bitmap with width/height header)
;	a2	IN	- background			(raw bitmap 320px wide, aligned to screen)
;
CORE_ST_DrawClearMaskedNarrow:	macro
	move.w		(a1)+, d5			;			dword data = (dword)*glyph++ << (16 - (xpos&15));
	swap.w		d5					
	eor.w		d5, d5				
	lsr.l		d2, d5				

	move.w		(a0), d0			;			*dst = (*dst & ~HIWORD(backmask)) | (*back & HIWORD(backmask));
	swap.w		d0					;
	move.w		8(a0), d0			;
	and.l		d6, d0				;	d0 = (*dst & ~HIWORD(backmask))

	move.w		(a2)+, d7
	swap.w		d7
	move.w		6(a2), d7
	and.l		d1, d7
	or.l		d7, d0				;	d0 = (*dst & ~HIWORD(backmask)) | (*back & HIWORD(backmask));

	and.l		d4, d0				;			*dst = (*dst & HIWORD(mask)) | HIWORD(data);
	or.l		d5, d0				;

	move.w		d0, 8(a0)
	swap.w		d0
	move.w		d0, (a0)+
  endm


	public _ST_DrawClearMaskedNarrow
_ST_DrawClearMaskedNarrow:
;
;	d0	- temp
;	d1	- backmask
;	d2	- xpos & 15
;	d3	- temp initially, then height counter
;	d4	- mask
;	d5	- glyph data
;	d6	- ~backmask
;	d7	- temp 2
;
;	a0	- dst
;	a1	- glyph
;	a2	- back
;

	movem.l		d2-d7/a2,-(a7)

	mulu.w		#160, d3			; d3 = ypos*160 + ((xpos>>1)&~7)
	move.w		d2, d0
	lsr.w		#1, d0
	andi.w		#$FFF8, d0
	add.w		d0, d3

	lea.l		(a0,d3.w), a0		;	word *dst = screenbase + (xpos>>4)*4 + 80*ypos;
	lea.l		(a2,d3.w), a2		;	word *back = bg + (xpos>>4)*4 + 80*ypos;

	andi.w		#15, d2

	move.w		(a1)+, d0			;	word width = *glyph++;			// width - assume 1 word
	move.l		#$FFFF0000, d1		;	word backmask0 = 0xFFFF<<(16-width);
	lsr.l		d0, d1				;
	swap.w		d1					;	dword backmask = backmask0 << (16 - (xpos&15));
	eor.w		d1, d1
	lsr.l		d2, d1
	move.l		d1, d6
	not.l		d6

	move.w		(a1)+, d3			;	word height = *glyph++ - 1;
	subq.l		#1, d3				;

.loop:								;	do {
	move.w		(a1)+, d4			;		dword mask = ~((dword)*glyph++ << (16 - (xpos&15)));
	swap.w		d4					;
	eor.w		d4, d4				;
	lsr.l		d2, d4				;
	not.l		d4					;


	CORE_ST_DrawClearMaskedNarrow
	CORE_ST_DrawClearMaskedNarrow
	CORE_ST_DrawClearMaskedNarrow
	CORE_ST_DrawClearMaskedNarrow

									;
	adda.w		#160-8, a0			;		dst += 80 - 4;
	adda.w		#160-8, a2			;		back += 80 - 4;
	dbra.w		d3, .loop			;	} while( height-->0 );

	movem.l		(a7)+,d2-d7/a2
	rts



; ================================================================ ST_DrawOpaqueAligned ================================================================
;
;	Draws opaque, 16px-aligned bitmap, 16px or 32px wide (any height).
;
;	d2	IN	- X position
;	d3	IN	- Y position
;
;	a0	IN	- screen
;	a1	IN	- bitmap				(opaque bitmap width width/height header)
;
	public _ST_DrawOpaqueAligned
_ST_DrawOpaqueAligned:
;
;	d0	- temp
;	d1	- temp
;	d2	- temp initially, then width, then temp
;	d3	- temp initially, then height counter
;	d4	- temp
;
;	a0	- dst
;	a1	- bitmap
;

	movem.l		d2-d4,-(a7)

	mulu.w		#160, d3				; d3 = ypos*160 + ((xpos>>1)&~7)
	lsr.w		#1, d2
	andi.w		#$FFF8, d2
	add.w		d2, d3

	lea.l		(a0,d3.w), a0			;	word *dst = screen + (xpos>>4)*4 + 80*ypos;
	
	move.w		(a1)+, d2				;	word width = (*bitmap++) >> 4;
	move.w		(a1)+, d3				;	word height = *bitmap++ - 1;
	subq.w		#1, d3

	cmp.w		#32, d2					; width == 32 ?
	beq			.loop_32				;	-> jump

.loop_16:								;	copy 16px	(8 words)					
	movem.l		(a1)+, d0-d1
	movem.l		d0-d1, (a0)
	adda.w		#160, a0
	dbra.w		d3, .loop_16
	bra			.end
	
.loop_32:								;	copy 32px	(16 words)
	movem.l		(a1)+, d0-d2/d4
	movem.l		d0-d2/d4, (a0)
	adda.w		#160, a0
	dbra.w		d3, .loop_32

.end:
	movem.l		(a7)+,d2-d4
	rts

		
;		; ================================================================ ST_CopyQWordRectBlock ================================================================
;		;
;		;	Copies rectangular block of data
;		;
;		;	d2		- number of qwords in each line
;		;	d3		- number of lines
;		;	d4		- stride (in qwords)
;		;
;		;	a0		- destination
;		;	a1		- source data
;		;
;			public _ST_CopyQWordRectBlock
;		_ST_CopyQWordRectBlock:
;		;
;		;	d0	- X counter
;		;	d1	- (copy temp)
;		;	d2	- IN, then number of qwords per line - 1
;		;	d3	- IN, then line counter
;		;	d4	- IN, then stride IN BYTES
;		;
;		;	a0	- destination
;		;	a1	- source
;		;
;		.end:
;			movem.l		d2-d4,-(a7)
;		
;			movem.l		(a7)+,d2-d4
;			rts
;		



; ================================================================ ST_DrawResetBlockAsm ================================================================
;
;	void ST_DrawResetBlockAsm(__a0 word *screen, __a1 word *background, __d0 word x1, __d1 word y1, __d2 word x2, __d3 word y2)
;
	public _ST_DrawResetBlockAsm
_ST_DrawResetBlockAsm:
;
;			[IN]			[SETUP]			[LOOP]
;	d0	-	x1				baseoffs		xcount
;	d1	-	y1								(tmp)
;	d2	-	x2				width			xlimit
;	d3	-	y2				linecount		ycount
;	d4	-					[		stride			]
;	d5	-									(tmp)
;	d6	-									-
;	d7	-									-
;
;	a0	-	[				screen					]
;	a1	-	[				background				]
;	a2	-									-
;	a3	-									-
;	a4	-									-
;	a5	-									-
;	a6	-
;

	movem.l		d2-d7/a2-a7,-(a7)

	sub.w		d0, d2									;	width = (x2-x1)							[d2]
	sub.w		d1, d3									;	linecount = (y2-y1)						[d3]

	ext.l		d0										;	baseoffs = x1*8 + y1*160				[d0.long]
	lsl.l		#3, d0
	mulu.w		#160, d1
	add.l		d1, d0

	add.l		d0, a0									;	screen += baseoffs						[a0]
	add.l		d0, a1									;	background += baseoffs					[a1]


	move.w		d2, d1									;	stride = 320*4/8 - width*8				[d4]		[d1 - temp]
	lsl.w		#3, d1
	move.w		#160, d4
	sub.w		d1, d4

	sub.w		#1, d2									;	xlimit = width - 1						[d2]
	bmi			.end
	sub.w		#1, d3									;	ycount = linecount - 1					[d3]
	bmi			.end

.yloop:													;	do {
	move.w		d2, d0									;		xcount = ylimit						[d0]

.xloop:													;		do {
	movem.l		(a1)+, d1/d5
	movem.l		d1/d5, (a0)
	adda.w		#8, a0
	dbra.w		d0, .xloop								;		} while( xcount-->0 )

	add.w		d4, a0									;		screen += stride
	add.w		d4, a1									;		background += stride
	dbra.w		d3, .yloop								;	} while( ycount-->0 )

.end:
	movem.l		(a7)+,d2-d7/a2-a7
	rts



; ================================================================ ST_DrawMasked ================================================================
;
;	void ST_DrawMasked(__a0 word *screen, __a1 const word *bitmap, __d0 word xpos, __d2 word ypos)
;
;

;
; Load <dstreg> with shifted data.
;	Remainder left in upper word.
;	
DRAW_PREPARE_FIRST:		macro	dstreg
	moveq.l		#0, \1									; clear both words
	move.w		(a1)+, \1								; load
	ror.l		d0, \1									; roll		
endm

;
; Load <dstreg> with shifted data, overlaying upper word from <oldreg>.
;	Remainder left in upper word.
;	
DRAW_PREPARE_MIDDLE:	macro	dstreg, oldreg
	moveq.l		#0, \1									; clear target
	move.w		(a1)+, \1								; load
	ror.l		d0, \1									; roll
	swap.w		\2										; fetch upper word
	or.w		\2, \1									; add the mask
endm

;
; Prepare remainder of <dstreg> for drawing.
;	
DRAW_PREPARE_LAST:		macro	dstreg
	swap.w		\1										; just swap
endm

;
; Draw data.
;
DRAW_MASKED_DRAW:		macro	mask, plane0, plane1, plane2, plane3
	not.w		\1						; invert mask
	and.w		\1, (a0)
	or.w		\2, (a0)+
	and.w		\1, (a0)
	or.w		\3, (a0)+
	and.w		\1, (a0)
	or.w		\4, (a0)+
	and.w		\1, (a0)
	or.w		\5, (a0)+
endm



	public _ST_DrawMasked
_ST_DrawMasked:
	movem.l		d2-d7,-(a7)

														;	xpos									[d0]
														;	ypos									[d2]


	ext.l		d0										;	screen += xpos/16 * 8
	ror.l		#4, d0
	lsl.w		#3, d0
	add.w		d0, a0
	eor.w		d0, d0									;	offs = xpos & 15						[d0]
	rol.l		#4, d0
	
	
	move.w		(a1)+, d3								;	width = (*bitmap++ + 15)/16				[d3]
	add.w		#15, d3
	lsr.w		#4, d3

	move.w		(a1)+, d1								;	ycount = *bitmap++ - 1					[d1]
	sub.w		#1, d1


	tst.w		d2
	bmi			.ypos_negative
														;	if( ypos >= 0 )
	mulu.w		#160, d2								;		screen += ypos * 160
	add.l		d2, a0
	bra			.cont

.ypos_negative:											;	else {
	neg.w		d2										;		ypos = -ypos;
	sub.w		d2, d1									;		ycount -= ypos	
	bmi			.end									;		if( ycount < 0 ) return;
	mulu.w		#10, d2									;		source += ypos * width * 10
	mulu.w		d3, d2									;
	adda.w		d2, a1									;
.cont:													;	}


	cmp.w		#3, d3
	beq			.yloop_w3

.yloop_w1:												;	do {

											; d0	shift count
											; d1	ycount
											;
											; a0	target
											; a1	source
	; core - width=1
	DRAW_PREPARE_FIRST		d2
	DRAW_PREPARE_FIRST		d3
	DRAW_PREPARE_FIRST		d4
	DRAW_PREPARE_FIRST		d5
	DRAW_PREPARE_FIRST		d6
	DRAW_MASKED_DRAW		d2, d3, d4, d5, d6

	DRAW_PREPARE_LAST		d2
	DRAW_PREPARE_LAST		d3
	DRAW_PREPARE_LAST		d4
	DRAW_PREPARE_LAST		d5
	DRAW_PREPARE_LAST		d6
	DRAW_MASKED_DRAW		d2, d3, d4, d5, d6

	adda.w					#160-2*8, a0				;		screen += stride

	dbra.w		d1, .yloop_w1							;	} while( ycount-->0 )

	bra			.end



.yloop_w3:												;	do {

											; d0	shift count
											; d1	ycount
											;
											; a0	target
											; a1	source
	; core - width=3
	DRAW_PREPARE_FIRST		d2
	DRAW_PREPARE_FIRST		d3
	DRAW_PREPARE_FIRST		d4
	DRAW_PREPARE_FIRST		d5
	DRAW_PREPARE_FIRST		d6
	DRAW_MASKED_DRAW		d2, d3, d4, d5, d6

	DRAW_PREPARE_MIDDLE		d7, d2
	DRAW_PREPARE_MIDDLE		d2, d3
	DRAW_PREPARE_MIDDLE		d3, d4
	DRAW_PREPARE_MIDDLE		d4, d5
	DRAW_PREPARE_MIDDLE		d5, d6
	DRAW_MASKED_DRAW		d7, d2, d3, d4, d5

	DRAW_PREPARE_MIDDLE		d6, d7
	DRAW_PREPARE_MIDDLE		d7, d2
	DRAW_PREPARE_MIDDLE		d2, d3
	DRAW_PREPARE_MIDDLE		d3, d4
	DRAW_PREPARE_MIDDLE		d4, d5
	DRAW_MASKED_DRAW		d6, d7, d2, d3, d4

	DRAW_PREPARE_LAST		d6
	DRAW_PREPARE_LAST		d7
	DRAW_PREPARE_LAST		d2
	DRAW_PREPARE_LAST		d3
	DRAW_PREPARE_LAST		d4
	DRAW_MASKED_DRAW		d6, d7, d2, d3, d4

	adda.w					#160-4*8, a0				;		screen += stride

	dbra.w		d1, .yloop_w3							;	} while( ycount-->0 )


.end:
	movem.l		(a7)+,d2-d7
	rts



; ================================================================ ST_DrawDreadGlyph ================================================================
;
;	void ST_DrawDreadGlyph(__a0 word *screen, __a1 const word *glyph, __d0 word xpos, __d2 word ypos, __d1 word height, __d3 word color_set);
;
DRAW_GLYPH_DRAW_PLANE:	macro	col0, col1, col2, plane
  if ( \1 & \4 )
	or.w		d6, (a0)
  else 
	and.w		d7, (a0)
  endif
  if ( \2 & \4 )
	or.w		d2, (a0)
  else 
	and.w		d3, (a0)
  endif
  if ( \3 & \4 )
	or.w		d4, (a0)+
  else 
	and.w		d5, (a0)+
  endif
endm

DRAW_GLYPH_DRAW:		macro	col0, col1, col2
	DRAW_GLYPH_DRAW_PLANE	\1, \2, \3, 1
	DRAW_GLYPH_DRAW_PLANE	\1, \2, \3, 2
	DRAW_GLYPH_DRAW_PLANE	\1, \2, \3, 4
	DRAW_GLYPH_DRAW_PLANE	\1, \2, \3, 8
endm

DRAW_GLYPH_SWAP:		macro
	swap.w		d2
	swap.w		d3
	swap.w		d4
	swap.w		d5
	swap.w		d6
	swap.w		d7
endm

DRAW_GLYPH_SWAP_SHIFT:		macro
	swap.w		d2										;			setup next
	swap.w		d3
	swap.w		d4
	swap.w		d5
	move.l		d4, d6
	move.l		d5, d7
	move.l		d2, d4
	move.l		d3, d5
endm

DRAW_GLYPH_CORE:	macro		col0, col1, col2
.yloop\@:												;	do {
	moveq.l		#0, d2									;			d2 = shifted mask
	move.w		(a1)+, d2
	ror.l		d0, d2
	move.l		d2, d3									;			d3 = inverted shifted mask
	not.l		d3

	DRAW_GLYPH_DRAW		\1, \2, \3
	DRAW_GLYPH_SWAP
	DRAW_GLYPH_DRAW		\1, \2, \3
	DRAW_GLYPH_SWAP_SHIFT

	adda.w		#160-16, a0
	dbra.w		d1, .yloop\@							;	} while( ycount-->0 )


	moveq.l		#0, d2									; two more rows
	moveq.l		#0, d3
	not.l		d3
	DRAW_GLYPH_DRAW		\1, \2, \3
	DRAW_GLYPH_SWAP
	DRAW_GLYPH_DRAW		\1, \2, \3
	DRAW_GLYPH_SWAP_SHIFT
	adda.w		#160-16, a0

	DRAW_GLYPH_DRAW		\1, \2, \3
	DRAW_GLYPH_SWAP
	DRAW_GLYPH_DRAW		\1, \2, \3
	DRAW_GLYPH_SWAP_SHIFT
endm


	public _ST_DrawDreadGlyph
_ST_DrawDreadGlyph:
	movem.l		d2-d7,-(a7)
														;	xpos									[d0]
														;	ypos									[d2]


	ext.l		d0										;	screen += xpos/16 * 8
	ror.l		#4, d0
	lsl.w		#3, d0
	add.w		d0, a0
	eor.w		d0, d0									;	offs = xpos & 15						[d0]
	rol.l		#4, d0

	mulu.w		#160, d2								;		screen += ypos * 160
	add.l		d2, a0
	
	sub.w		#1, d1									;	ycount = height - 1						[d1]
	
						

											;	d0	shift count
											;	d1	ycount

											;	d2	OR	latest
											;	d3	AND	latest
	moveq.l		#0, d4						;	d4	OR	delayed
	moveq.l		#0, d5						;	d5	AND	delayed
	not.l		d5
	moveq.l		#0, d6						;	d6	OR	delayed+2
	move.l		d5, d7						;	d7	AND delayed+2
											;
											;	a0	target
											;	a1	source

	sub.w		#1, d3
	beq			.draw_selected
	sub.w		#1, d3
	beq			.draw_inactive
.draw_normal:
	DRAW_GLYPH_CORE		9, 12, 11
	bra			.end

.draw_selected:
	DRAW_GLYPH_CORE		14, 5, 4
	bra			.end

.draw_inactive:
	DRAW_GLYPH_CORE		2, 14, 3
	bra			.end

.end:
	movem.l		(a7)+,d2-d7
	rts



; ================================================================ ST_DrawDreadGlyphShadow ================================================================
;
;	void ST_DrawDreadGlyphShadow(__a0 word *screen, __a1 const word *glyph, __d0 word xpos, __d2 word ypos, __d1 word height);
;
	public _ST_DrawDreadGlyphShadow
_ST_DrawDreadGlyphShadow:
	movem.l		d2-d7,-(a7)
														;	xpos									[d0]
														;	ypos									[d2]


	ext.l		d0										;	screen += xpos/16 * 8
	ror.l		#4, d0
	lsl.w		#3, d0
	add.w		d0, a0
	eor.w		d0, d0									;	offs = xpos & 15						[d0]
	rol.l		#4, d0

	mulu.w		#160, d2								;		screen += ypos * 160
	add.l		d2, a0
	
	sub.w		#1, d1									;	ycount = height - 1						[d1]
	

.yloop:													;	do {
	moveq.l		#0, d2									;			d2 = inverted shifted mask
	move.w		(a1)+, d2
	ror.l		d0, d2
	not.l		d2
	and.w		d2, (a0)+								;			draw first word
	and.w		d2, (a0)+
	and.w		d2, (a0)+
	and.w		d2, (a0)+
	swap.w		d2										;			draw second word
	and.w		d2, (a0)+
	and.w		d2, (a0)+
	and.w		d2, (a0)+
	and.w		d2, (a0)+

	adda.w		#160-16, a0
	dbra.w		d1, .yloop								;	} while( ycount-->0 )

.end:
	movem.l		(a7)+,d2-d7
	rts
