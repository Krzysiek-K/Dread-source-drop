


; ================================================================ C2P_CORE ================================================================
; 2-scanline mode:		24 * 4 * YMAX/2 * 20 cycles	= 96000
;
; MOVEM patch:
;		movem.l		(a#)+, /.../		12+8n cycles
;		movem.l		/.../, (a#)			 8+8n cycles
;		movem.l		d(a#), /.../		16+8n cycles
;		movem.l		/.../, d(a#)		12+8n cycles		-> 28+16n cycles
;
;		100 lines x 40 longs
;		/d0-d7/a0-a6/	- 15		(40 = 15+15+10 = 13+13+14)
;
;		1 line -> 28*3 + 16*40 = 724
;		100 lines -> 72400
;
;



C2P_CORE:	macro	line_num
  if (\1 < ENGINE_Y_MAX)
	move.l		(a4)+, d1
	move.l		(a6,d1.w), d0
	move.l		(a3)+, d4
	or.l		(a5,d4.w), d0
	movep.l		d0, 1+320*\1+320(a1)
;	movep.l		d0, 161+320*\1+320(a1)

	move.l		(a2)+, d5
	move.l		(a6,d5.w), d0
	move.l		(a0)+, d6
	or.l		(a5,d6.w), d0
	movep.l		d0, 0+320*\1+320(a1)
;	movep.l		d0, 160+320*\1+320(a1)

	swap		d1
	move.l		(a6,d1.w), d0
	swap		d4
	or.l		(a5,d4.w), d0
	movep.l		d0, 1+320*\1(a1)
;	movep.l		d0, 161+320*\1(a1)

	swap		d5
	move.l		(a6,d5.w), d0
	swap		d6
	or.l		(a5,d6.w), d0
	movep.l		d0, 0+320*\1(a1)
;	movep.l		d0, 160+320*\1(a1)
  endif
endm


; ================================================================ _c2p_asm ================================================================
;	a0	- render_screen
;	a1	- screen
;	a2	- (render_screen+200)
;	a3	- (render_screen+400)
;	a4	- (render_screen+600)
;	a5	- c2p_lut
;	a6	- c2p_lut2
;
;	d0	- (temp)
;	d1	- (temp)
;	d2	- X counter
;	d3	- Y counter
;
	public	_c2p_asm
_c2p_asm:
	movem.l		d0-d7/a0-a6,-(a7)
	
	move.l		#$FFFF0000, d0

;	add.l		#16, a0

	move.w		#19, d2
.xloop:

	lea			200(a0), a2
	lea			400(a0), a3
	lea			600(a0), a4

	C2P_CORE	 0
	C2P_CORE	 2
	C2P_CORE	 4
	C2P_CORE	 6
	C2P_CORE	 8
	C2P_CORE	10
	C2P_CORE	12
	C2P_CORE	14
	C2P_CORE	16
	C2P_CORE	18
	C2P_CORE	20
	C2P_CORE	22
	C2P_CORE	24
	C2P_CORE	26
	C2P_CORE	28
	C2P_CORE	30
	C2P_CORE	32
	C2P_CORE	34
	C2P_CORE	36
	C2P_CORE	38
	C2P_CORE	40
	C2P_CORE	42
	C2P_CORE	44
	C2P_CORE	46
	C2P_CORE	48
	C2P_CORE	50
	C2P_CORE	52
	C2P_CORE	54
	C2P_CORE	56
	C2P_CORE	58
	C2P_CORE	60
	C2P_CORE	62
	C2P_CORE	64
	C2P_CORE	66
	C2P_CORE	68
	C2P_CORE	70
	C2P_CORE	72
	C2P_CORE	74
	C2P_CORE	76
	C2P_CORE	78
	C2P_CORE	80
	C2P_CORE	82
	C2P_CORE	84
	C2P_CORE	86
	C2P_CORE	88
	C2P_CORE	90
	C2P_CORE	92
	C2P_CORE	94
	C2P_CORE	96
	C2P_CORE	98


	; --- next X ---
	add.w		#600, a0
	add.w		#8, a1
	add.w		#(100-ENGINE_Y_MAX)*2, a4
	add.w		#(100-ENGINE_Y_MAX)*2, a3
	add.w		#(100-ENGINE_Y_MAX)*2, a2
	add.w		#(100-ENGINE_Y_MAX)*2, a0
	dbra.w		d2, .xloop


	movem.l		(a7)+,d0-d7/a0-a6
	rts



; ================================================================ _c2p_fix ================================================================
;	a0	- screen
	public _c2p_fix
_c2p_fix:
	movem.l		d2-d7/a2-a6,-(a7)
	move.l		a7, .save_a7

	move.w		#ENGINE_Y_MAX-2, d0
.loop:
	movem.l		(a0), d1-d7/a1-a7
	movem.l		d1-d7/a1-a7, 160(a0)
	movem.l		56(a0), d1-d7/a1-a7
	movem.l		d1-d7/a1-a7, 56+160(a0)
	movem.l		112(a0), d1-d7/a1-a5
	movem.l		d1-d7/a1-a5, 112+160(a0)
	add.w		#320, a0
	dbra.w		d0, .loop


	move.l		.save_a7, a7
	movem.l		(a7)+,d2-d7/a2-a6
	rts

.save_a7:		dc.l	0


; ================================================================ _c2p_fix_blit ================================================================
;	a0	- screen
	public _c2p_fix_blit
_c2p_fix_blit:
	move.b		BLIT_LINE_NUM, d0			; wait
	bmi			_c2p_fix_blit

	move.l		a0, BLIT_SOURCE_ADDR
	add.l		#160, a0
	move.l		a0, BLIT_DEST_ADDR
	move.w		#$FFFF, d0
	move.w		d0, BLIT_ENDMASK_1
	move.w		d0, BLIT_ENDMASK_2
	move.w		d0, BLIT_ENDMASK_3

	move.w		#2, BLIT_SOURCE_INC_X
	move.w		#2, BLIT_DEST_INC_X
	move.w		#160+2, BLIT_SOURCE_INC_Y
	move.w		#160+2, BLIT_DEST_INC_Y
	move.w		#80, BLIT_WORDS_PER_LINE
	move.w		#ENGINE_Y_MAX-1, BLIT_LINES_PER_BLOCK
	
	move.b		#2, BLIT_HALFTONE_OP		;   |     |10 - Source --------------------------------------+-+|
	move.b		#3, BLIT_LOGIC_OP			;	|     |0011 Source ----------------------------------+-+-+-+|
	move.b		#0, BLIT_SKEW
	move.b		#$80, BLIT_LINE_NUM			; start

	rts
