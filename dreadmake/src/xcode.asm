	xref		_screen1
	xref		_screen2
	public		_CopyRect_Cpu
_CopyRect_Cpu:
.__local0___auto_2:
.__local0___auto_1:
	move.w		d4, d1
.__local0___auto_0:
	move.w		(a0)+, (a1)+
	sub.w		#1, d1
	bne			.__local0___auto_0
	add.l		d2, a0
	add.l		d3, a1
	sub.l		#1, d1
	bne			.__local0___auto_1
	rts			


	xref		_minimap_zoom
	xref		_minimap_xpos
	xref		_minimap_ypos
	xref		_minimap_xmid
	xref		_minimap_ymid
	xref		_minimap_xscreen
	xref		_minimap_yscreen
	xref		_minimap_screen_stride
	xref		_minimap_xclip
	xref		_minimap_yclip
	public		_Minimap_DrawLine
_Minimap_DrawLine:
.__local1___auto_11:
	movem.l		d2-d5, -(a7)
	sub.w		d0, d2
	blt			.__local1___auto_6
	sub.w		d1, d3
	blt			.__local1___auto_4
	cmp.w		d3, d2
	bge			.__local1___auto_3
	move.w		#1, d4
	bra			.swap1
.__local1___auto_3:
	move.w		#17, d4
	bra			.noswap
.__local1___auto_4:
	neg.w		d3
	cmp.w		d3, d2
	bge			.__local1___auto_5
	move.w		#5, d4
	bra			.swap1
.__local1___auto_5:
	move.w		#25, d4
	bra			.noswap
.__local1___auto_6:
	neg.w		d2
	sub.w		d1, d3
	blt			.__local1___auto_8
	cmp.w		d3, d2
	bge			.__local1___auto_7
	move.w		#9, d4
	bra			.swap1
.__local1___auto_7:
	move.w		#21, d4
	bra			.noswap
.__local1___auto_8:
	neg.w		d3
	cmp.w		d3, d2
	bge			.__local1___auto_9
	move.w		#13, d4
	bra			.swap1
.__local1___auto_9:
	move.w		#29, d4
	bra			.noswap
.swap1:
	eor.w		d3, d2
	eor.w		d2, d3
	eor.w		d3, d2
.noswap:
	lsl.w		#2, d3
	lsl.w		#1, d2
	move.w		d3, $dff062
	sub.w		d2, d3
	bge			.__local1___auto_10
	or.w		#64, d4
.__local1___auto_10:
	move.w		d4, $dff042
	move.w		d3, $dff052
	move.w		d0, d4
	lsl.w		#8, d4
	lsl.w		#4, d4
	or.w		#$0bfa, d4
	move.w		d4, $dff040
	sub.w		d2, d3
	move.w		d3, $dff064
	move.w		#$ffff, d4
	move.w		#$8000, $dff074
	move.w		d4, $dff044
	move.w		d4, $dff046
	move.w		d4, $dff072
	lsr.w		#4, d0
	lsl.w		#1, d0
	muls.w		_minimap_screen_stride, d1
	add.w		d1, d0
	add.w		d0, d5
	move.l		d5, $dff048
	move.l		d5, $dff054
	move.w		_minimap_screen_stride, $dff060
	move.w		_minimap_screen_stride, $dff066
	lsl.w		#5, d2
	add.w		#66, d2
	move.w		d2, $dff058
	movem.l		(a7)+, d2-d5
	rts			


	public		_Minimap_DrawLineClip
_Minimap_DrawLineClip:
.__local2___auto_27:
	movem.l		d2-d7/a3, -(a7)
	sub.w		_minimap_xpos, d4
	muls.w		_minimap_zoom, d4
	lsr.l		#8, d4
	add.w		_minimap_xmid, d4
	sub.w		_minimap_ypos, d5
	muls.w		_minimap_zoom, d5
	lsr.l		#8, d5
	add.w		_minimap_ymid, d5
	sub.w		_minimap_xpos, d0
	muls.w		_minimap_zoom, d0
	lsr.l		#8, d0
	add.w		_minimap_xmid, d0
	sub.w		_minimap_ypos, d1
	muls.w		_minimap_zoom, d1
	lsr.l		#8, d1
	add.w		_minimap_ymid, d1
	cmp.w		#0, d5
	bge			.__local2___auto_12
	cmp.w		#0, d1
	blt			.__local2_ret
.__local2___auto_12:
	cmp.w		_minimap_yclip, d5
	ble			.__local2___auto_13
	cmp.w		_minimap_yclip, d1
	bgt			.__local2_ret
.__local2___auto_13:
	cmp.w		_minimap_xclip, d4
	ble			.__local2___auto_14
	cmp.w		_minimap_xclip, d0
	bgt			.__local2_ret
.__local2___auto_14:
	cmp.w		#0, d4
	bge			.__local2___auto_16
	cmp.w		#0, d0
	blt			.__local2_ret
	move.w		d5, d3
	sub.w		d1, d3
	muls.w		d4, d3
	move.w		d0, d7
	sub.w		d4, d7
	divs.w		d7, d3
	add.w		d3, d5
	move.w		#0, d4
	bra			.__local2___auto_17
.__local2___auto_16:
	cmp.w		#0, d0
	bge			.__local2___auto_15
	move.w		d5, d3
	sub.w		d1, d3
	muls.w		d0, d3
	move.w		d0, d7
	sub.w		d4, d7
	divs.w		d7, d3
	add.w		d3, d1
	move.w		#0, d0
.__local2___auto_15:
.__local2___auto_17:
	cmp.w		#0, d5
	bge			.__local2___auto_19
	cmp.w		#0, d1
	blt			.__local2_ret
	move.w		d4, d3
	sub.w		d0, d3
	muls.w		d5, d3
	move.w		d1, d7
	sub.w		d5, d7
	divs.w		d7, d3
	add.w		d3, d4
	move.w		#0, d5
	bra			.__local2___auto_20
.__local2___auto_19:
	cmp.w		#0, d1
	bge			.__local2___auto_18
	move.w		d4, d3
	sub.w		d0, d3
	muls.w		d1, d3
	move.w		d1, d7
	sub.w		d5, d7
	divs.w		d7, d3
	add.w		d3, d0
	move.w		#0, d1
.__local2___auto_18:
.__local2___auto_20:
	cmp.w		_minimap_xclip, d4
	ble			.__local2___auto_22
	cmp.w		_minimap_xclip, d0
	bgt			.__local2_ret
	move.w		d1, d3
	sub.w		d5, d3
	move.w		_minimap_xclip, d7
	sub.w		d4, d7
	muls.w		d7, d3
	move.w		d0, d7
	sub.w		d4, d7
	divs.w		d7, d3
	add.w		d3, d5
	move.w		_minimap_xclip, d4
	bra			.__local2___auto_23
.__local2___auto_22:
	cmp.w		_minimap_xclip, d0
	ble			.__local2___auto_21
	move.w		d1, d3
	sub.w		d5, d3
	move.w		_minimap_xclip, d7
	sub.w		d0, d7
	muls.w		d7, d3
	move.w		d0, d7
	sub.w		d4, d7
	divs.w		d7, d3
	add.w		d3, d1
	move.w		_minimap_xclip, d0
.__local2___auto_21:
.__local2___auto_23:
	cmp.w		_minimap_yclip, d5
	ble			.__local2___auto_25
	cmp.w		_minimap_yclip, d1
	bgt			.__local2_ret
	move.w		d0, d3
	sub.w		d4, d3
	move.w		_minimap_yclip, d7
	sub.w		d5, d7
	muls.w		d7, d3
	move.w		d1, d7
	sub.w		d5, d7
	divs.w		d7, d3
	add.w		d3, d4
	move.w		_minimap_yclip, d5
	bra			.__local2___auto_26
.__local2___auto_25:
	cmp.w		_minimap_yclip, d1
	ble			.__local2___auto_24
	move.w		d0, d3
	sub.w		d4, d3
	move.w		_minimap_yclip, d7
	sub.w		d1, d7
	muls.w		d7, d3
	move.w		d1, d7
	sub.w		d5, d7
	divs.w		d7, d3
	add.w		d3, d0
	move.w		_minimap_yclip, d1
.__local2___auto_24:
.__local2___auto_26:
	move.w		d1, d3
	move.w		d0, d2
	move.w		d5, d1
	move.w		d4, d0
	move.l		a2, d5
	jsr			_Minimap_DrawLine
.__local2_ret:
	movem.l		(a7)+, d2-d7/a3
	rts			


	xref		_TICK_SPR_POINTER
	xref		_NULL_SPR_POINTER
	xref		_cop_write
	xref		_cop_write_y
	public		_Cop_StartBit0
_Cop_StartBit0:
.__local3___auto_28:
	move.l		_cop_write, a0
	move.w		#256, (a0)+
	move.w		#512, (a0)+
	move.w		#384, (a0)+
	move.w		#0, (a0)+
	move.l		a0, _cop_write
	move.w		#44, _cop_write_y
	rts			


	public		_Cop_StartBit1
_Cop_StartBit1:
.__local4___auto_29:
	move.l		_cop_write, a0
	move.w		#226, (a0)+
	move.w		d0, (a0)+
	swap		d0
	move.w		#224, (a0)+
	move.w		d0, (a0)+
	move.w		#256, (a0)+
	move.w		#$1200, (a0)+
	move.w		#384, (a0)+
	move.w		#0, (a0)+
	move.l		a0, _cop_write
	move.w		#44, _cop_write_y
	rts			


	public		_Cop_StartBit2
_Cop_StartBit2:
.__local5___auto_30:
	move.l		_cop_write, a0
	move.w		#226, (a0)+
	move.w		d0, (a0)+
	swap		d0
	move.w		#224, (a0)+
	move.w		d0, (a0)+
	move.w		#230, (a0)+
	move.w		d1, (a0)+
	swap		d1
	move.w		#228, (a0)+
	move.w		d1, (a0)+
	move.w		#256, (a0)+
	move.w		#$2200, (a0)+
	move.w		#384, (a0)+
	move.w		#0, (a0)+
	move.l		a0, _cop_write
	move.w		#44, _cop_write_y
	rts			


	public		_Cop_StartBit3
_Cop_StartBit3:
.__local6___auto_31:
	move.l		_cop_write, a0
	move.w		#226, (a0)+
	move.w		d0, (a0)+
	swap		d0
	move.w		#224, (a0)+
	move.w		d0, (a0)+
	move.w		#230, (a0)+
	move.w		d1, (a0)+
	swap		d1
	move.w		#228, (a0)+
	move.w		d1, (a0)+
	move.l		d2, d1
	move.w		#234, (a0)+
	move.w		d1, (a0)+
	swap		d1
	move.w		#232, (a0)+
	move.w		d1, (a0)+
	move.w		#256, (a0)+
	move.w		#$3200, (a0)+
	move.w		#384, (a0)+
	move.w		#0, (a0)+
	move.l		a0, _cop_write
	move.w		#44, _cop_write_y
	rts			


	public		_Cop_StartBit4
_Cop_StartBit4:
.__local7___auto_32:
	move.l		_cop_write, a0
	move.w		#226, (a0)+
	move.w		d0, (a0)+
	swap		d0
	move.w		#224, (a0)+
	move.w		d0, (a0)+
	move.w		#230, (a0)+
	move.w		d1, (a0)+
	swap		d1
	move.w		#228, (a0)+
	move.w		d1, (a0)+
	move.l		d2, d1
	move.w		#234, (a0)+
	move.w		d1, (a0)+
	swap		d1
	move.w		#232, (a0)+
	move.w		d1, (a0)+
	move.l		d3, d1
	move.w		#238, (a0)+
	move.w		d1, (a0)+
	swap		d1
	move.w		#236, (a0)+
	move.w		d1, (a0)+
	move.w		#256, (a0)+
	move.w		#$4200, (a0)+
	move.w		#384, (a0)+
	move.w		#0, (a0)+
	move.l		a0, _cop_write
	move.w		#44, _cop_write_y
	rts			


	public		_Cop_StartBit5
_Cop_StartBit5:
.__local8___auto_33:
	move.l		_cop_write, a0
	move.w		#226, (a0)+
	move.w		d0, (a0)+
	swap		d0
	move.w		#224, (a0)+
	move.w		d0, (a0)+
	move.w		#230, (a0)+
	move.w		d1, (a0)+
	swap		d1
	move.w		#228, (a0)+
	move.w		d1, (a0)+
	move.l		d2, d1
	move.w		#234, (a0)+
	move.w		d1, (a0)+
	swap		d1
	move.w		#232, (a0)+
	move.w		d1, (a0)+
	move.l		d3, d1
	move.w		#238, (a0)+
	move.w		d1, (a0)+
	swap		d1
	move.w		#236, (a0)+
	move.w		d1, (a0)+
	move.l		d4, d1
	move.w		#242, (a0)+
	move.w		d1, (a0)+
	swap		d1
	move.w		#240, (a0)+
	move.w		d1, (a0)+
	move.w		#256, (a0)+
	move.w		#$5200, (a0)+
	move.w		#384, (a0)+
	move.w		#0, (a0)+
	move.l		a0, _cop_write
	move.w		#44, _cop_write_y
	rts			


	public		_Cop_SetBit0
_Cop_SetBit0:
.__local9___auto_34:
	move.l		_cop_write, a0
	move.w		#256, (a0)+
	move.w		#512, (a0)+
	move.l		a0, _cop_write
	rts			


	public		_Cop_SetBit1
_Cop_SetBit1:
.__local10___auto_35:
	move.l		_cop_write, a0
	move.w		#226, (a0)+
	move.w		d0, (a0)+
	swap		d0
	move.w		#224, (a0)+
	move.w		d0, (a0)+
	move.w		#256, (a0)+
	move.w		#$1200, (a0)+
	move.l		a0, _cop_write
	rts			


	public		_Cop_SetBit4
_Cop_SetBit4:
.__local11___auto_36:
	move.l		_cop_write, a0
	move.w		#226, (a0)+
	move.w		d0, (a0)+
	swap		d0
	move.w		#224, (a0)+
	move.w		d0, (a0)+
	move.w		#230, (a0)+
	move.w		d1, (a0)+
	swap		d1
	move.w		#228, (a0)+
	move.w		d1, (a0)+
	move.l		d2, d1
	move.w		#234, (a0)+
	move.w		d1, (a0)+
	swap		d1
	move.w		#232, (a0)+
	move.w		d1, (a0)+
	move.l		d3, d1
	move.w		#238, (a0)+
	move.w		d1, (a0)+
	swap		d1
	move.w		#236, (a0)+
	move.w		d1, (a0)+
	move.w		#256, (a0)+
	move.w		#$4200, (a0)+
	move.l		a0, _cop_write
	rts			


	public		_Cop_NoSprites
_Cop_NoSprites:
.__local12___auto_37:
	move.l		_cop_write, a0
	move.w		#150, (a0)+
	move.w		#32, (a0)+
	move.l		a0, _cop_write
	rts			


	public		_Cop_TickSprite
_Cop_TickSprite:
.__local13___auto_38:
	move.l		_cop_write, a0
	move.l		_TICK_SPR_POINTER, d1
	move.w		#290, (a0)+
	move.w		d1, (a0)+
	swap		d1
	move.w		#288, (a0)+
	move.w		d1, (a0)+
	move.l		_NULL_SPR_POINTER, d1
	move.w		#294, (a0)+
	move.w		d1, (a0)+
	move.w		#298, (a0)+
	move.w		d1, (a0)+
	move.w		#302, (a0)+
	move.w		d1, (a0)+
	move.w		#306, (a0)+
	move.w		d1, (a0)+
	move.w		#310, (a0)+
	move.w		d1, (a0)+
	move.w		#314, (a0)+
	move.w		d1, (a0)+
	move.w		#318, (a0)+
	move.w		d1, (a0)+
	swap		d1
	move.w		#292, (a0)+
	move.w		d1, (a0)+
	move.w		#296, (a0)+
	move.w		d1, (a0)+
	move.w		#300, (a0)+
	move.w		d1, (a0)+
	move.w		#304, (a0)+
	move.w		d1, (a0)+
	move.w		#308, (a0)+
	move.w		d1, (a0)+
	move.w		#312, (a0)+
	move.w		d1, (a0)+
	move.w		#316, (a0)+
	move.w		d1, (a0)+
	move.w		#150, (a0)+
	move.w		#$8020, (a0)+
	move.l		a0, _cop_write
	rts			


	public		_Cop_BitMod
_Cop_BitMod:
.__local14___auto_39:
	move.l		_cop_write, a0
	move.w		#264, (a0)+
	move.w		d0, (a0)+
	move.w		#266, (a0)+
	move.w		d1, (a0)+
	move.l		a0, _cop_write
	rts			


	public		_Cop_ColorMap
_Cop_ColorMap:
.__local15___auto_41:
	move.w		(a1)+, d0
	move.w		#384, d1
	move.l		_cop_write, a0
.__local15___auto_40:
	move.w		d1, (a0)+
	move.w		(a1)+, (a0)+
	add.w		#2, d1
	sub.w		#1, d0
	bne			.__local15___auto_40
	move.l		a0, _cop_write
	rts			


	public		_Cop_Palette
_Cop_Palette:
.__local16___auto_43:
	move.w		#384, d1
	move.l		_cop_write, a0
.__local16___auto_42:
	move.w		d1, (a0)+
	move.w		(a1)+, (a0)+
	add.w		#2, d1
	sub.w		#1, d0
	bne			.__local16___auto_42
	move.l		a0, _cop_write
	rts			


	public		_Cop_Palette_NZ
_Cop_Palette_NZ:
.__local17___auto_45:
	move.w		#386, d1
	move.l		_cop_write, a0
.__local17___auto_44:
	move.w		d1, (a0)+
	move.w		(a1)+, (a0)+
	add.w		#2, d1
	sub.w		#1, d0
	bne			.__local17___auto_44
	move.l		a0, _cop_write
	rts			


	public		_Cop_Palette_Fade
_Cop_Palette_Fade:
.__local18___auto_49:
	movem.l		d2-d4/d6, -(a7)
	move.w		#384, d1
	move.l		_cop_write, a0
	cmp.w		#17, d7
	bcc			.__local18___auto_47
.__local18___auto_46:
	move.w		d1, (a0)+
	move.w		(a1)+, d2
	move.w		d2, d3
	and.w		#$0f0f, d3
	mulu.w		d7, d3
	and.w		#$f0f0, d3
	and.w		#240, d2
	mulu.w		d7, d2
	and.w		#$0f00, d2
	add.w		d2, d3
	lsr.w		#4, d3
	move.w		d3, (a0)+
	add.w		#2, d1
	sub.w		#1, d0
	bne			.__local18___auto_46
	bra			.__local18_end
.__local18___auto_47:
	move.w		#32, d6
	sub.w		d7, d6
.__local18___auto_48:
	move.w		d1, (a0)+
	move.w		(a1)+, d2
	eor.w		#$0fff, d2
	move.w		d2, d3
	and.w		#$0f0f, d3
	mulu.w		d6, d3
	and.w		#$f0f0, d3
	and.w		#240, d2
	mulu.w		d6, d2
	and.w		#$0f00, d2
	add.w		d2, d3
	lsr.w		#4, d3
	eor.w		#$0fff, d3
	move.w		d3, (a0)+
	add.w		#2, d1
	sub.w		#1, d0
	bne			.__local18___auto_48
.__local18_end:
	move.l		a0, _cop_write
	movem.l		(a7)+, d2-d4/d6
	rts			


	public		_ColorMap_Fade
_ColorMap_Fade:
.__local19___auto_53:
	movem.l		d2-d4/d6, -(a7)
	move.l		#$dff180, a0
	cmp.w		#17, d7
	bcc			.__local19___auto_51
.__local19___auto_50:
	move.w		(a1)+, d2
	move.w		d2, d3
	and.w		#$0f0f, d3
	mulu.w		d7, d3
	and.w		#$f0f0, d3
	and.w		#240, d2
	mulu.w		d7, d2
	and.w		#$0f00, d2
	add.w		d2, d3
	lsr.w		#4, d3
	move.w		d3, (a0)+
	sub.w		#1, d0
	bne			.__local19___auto_50
	bra			.__local19_end
.__local19___auto_51:
	move.w		#32, d6
	sub.w		d7, d6
.__local19___auto_52:
	move.w		(a1)+, d2
	eor.w		#$0fff, d2
	move.w		d2, d3
	and.w		#$0f0f, d3
	mulu.w		d6, d3
	and.w		#$f0f0, d3
	and.w		#240, d2
	mulu.w		d6, d2
	and.w		#$0f00, d2
	add.w		d2, d3
	lsr.w		#4, d3
	eor.w		#$0fff, d3
	move.w		d3, (a0)+
	sub.w		#1, d0
	bne			.__local19___auto_52
.__local19_end:
	move.l		a0, _cop_write
	movem.l		(a7)+, d2-d4/d6
	rts			


	public		_Cop_Rainbow2
_Cop_Rainbow2:
.__local20___auto_56:
	movem.l		a2, -(a7)
	move.l		_cop_write, a0
	move.w		_cop_write_y, d0
	lsl.w		#8, d0
	or.w		#15, d0
	add.w		d1, _cop_write_y
.__local20___auto_55:
	move.w		d0, (a0)+
	move.w		#$fffe, (a0)+
	move.w		d2, (a0)+
	move.w		(a1)+, (a0)+
	move.w		d3, (a0)+
	move.w		(a2)+, (a0)+
	add.w		#256, d0
	cmp.w		#15, d0
	bne			.__local20___auto_54
	move.l		#$ffdffffe, (a0)+
.__local20___auto_54:
	sub.w		#1, d1
	cmp.w		#0, d1
	bhi			.__local20___auto_55
	move.l		a0, _cop_write
	movem.l		(a7)+, a2
	rts			


	public		_Cop_Rainbow4
_Cop_Rainbow4:
.__local21___auto_59:
	movem.l		a2-a4, -(a7)
	move.l		_cop_write, a0
	move.w		_cop_write_y, d0
	lsl.w		#8, d0
	or.w		#15, d0
	add.w		d5, _cop_write_y
.__local21___auto_58:
	move.w		d0, (a0)+
	move.w		#$fffe, (a0)+
	move.w		d1, (a0)+
	move.w		(a1)+, (a0)+
	move.w		d2, (a0)+
	move.w		(a2)+, (a0)+
	move.w		d3, (a0)+
	move.w		(a3)+, (a0)+
	move.w		d4, (a0)+
	move.w		(a4)+, (a0)+
	add.w		#256, d0
	cmp.w		#15, d0
	bne			.__local21___auto_57
	move.l		#$ffdffffe, (a0)+
.__local21___auto_57:
	sub.w		#1, d5
	cmp.w		#0, d5
	bhi			.__local21___auto_58
	move.l		a0, _cop_write
	movem.l		(a7)+, a2-a4
	rts			


	public		_Cop_Rainbow4x
_Cop_Rainbow4x:
.__local22___auto_62:
	movem.l		d5/a2-a5, -(a7)
	move.l		_cop_write, a0
	move.w		_cop_write_y, d0
	lsl.w		#8, d0
	or.w		#15, d0
	add.w		d5, _cop_write_y
.__local22___auto_61:
	move.w		d0, (a0)+
	move.w		#$fffe, (a0)+
	move.w		#392, (a0)+
	move.w		(a1), (a0)+
	move.w		#394, (a0)+
	move.w		(a1), (a0)+
	move.w		#396, (a0)+
	move.w		(a1), (a0)+
	move.w		#398, (a0)+
	move.w		(a1)+, (a0)+
	move.w		d1, (a0)+
	move.w		(a1)+, (a0)+
	move.w		d2, (a0)+
	move.w		(a2)+, (a0)+
	move.w		d3, (a0)+
	move.w		(a3)+, (a0)+
	move.w		d4, (a0)+
	move.w		(a4)+, (a0)+
	add.w		#256, d0
	cmp.w		#15, d0
	bne			.__local22___auto_60
	move.l		#$ffdffffe, (a0)+
.__local22___auto_60:
	sub.w		#1, d5
	cmp.w		#0, d5
	bhi			.__local22___auto_61
	move.l		a0, _cop_write
	movem.l		(a7)+, d5/a2-a5
	rts			


	public		_Cop_RainbowWildcat_Init
_Cop_RainbowWildcat_Init:
.__local23___auto_65:
	movem.l		d2-d5/a2-a6, -(a7)
	move.l		_cop_write, a0
	move.w		_cop_write_y, d0
	lsl.w		#8, d0
	or.w		#15, d0
	add.w		d5, _cop_write_y
	move.w		#0, d1
.__local23___auto_64:
	move.w		d0, (a0)+
	move.w		#$fffe, (a0)+
	move.w		#392, (a0)+
	move.l		a0, d2
	sub.l		d7, d2
	lsr.l		#1, d2
	move.w		d2, (a1)+
	move.w		d1, (a0)+
	move.w		#394, (a0)+
	move.w		d1, (a0)+
	move.w		#396, (a0)+
	move.w		d1, (a0)+
	move.w		#398, (a0)+
	move.w		d1, (a0)+
	move.w		#408, (a0)+
	move.w		d1, (a0)+
	move.w		#410, (a0)+
	move.w		d1, (a0)+
	move.w		#412, (a0)+
	move.w		d1, (a0)+
	move.w		#414, (a0)+
	move.w		d1, (a0)+
	add.w		#256, d0
	cmp.w		#15, d0
	bne			.__local23___auto_63
	move.l		#$ffdffffe, (a0)+
.__local23___auto_63:
	sub.w		#1, d5
	cmp.w		#0, d5
	bhi			.__local23___auto_64
	move.l		a0, _cop_write
	movem.l		(a7)+, d2-d5/a2-a6
	rts			


	public		_Cop_Wait
_Cop_Wait:
.__local24___auto_67:
	move.l		_cop_write, a0
	move.w		_cop_write_y, d0
	lsl.w		#8, d0
	or.w		#15, d0
	move.w		d0, (a0)+
	move.w		#$fffe, (a0)+
	add.w		#256, d0
	cmp.w		#15, d0
	bne			.__local24___auto_66
	move.l		#$ffdffffe, (a0)+
.__local24___auto_66:
	move.l		a0, _cop_write
	rts			


	public		_Cop_Wait256
_Cop_Wait256:
.__local25___auto_68:
	move.l		_cop_write, a0
	move.w		_cop_write_y, d0
	lsl.w		#8, d0
	or.w		#15, d0
	move.l		#$ffdffffe, (a0)+
	move.w		d0, (a0)+
	move.w		#$fffe, (a0)+
	move.l		a0, _cop_write
	rts			


	public		_Cop_End
_Cop_End:
.__local26___auto_69:
	move.l		_cop_write, a0
	move.l		#-2, (a0)+
	move.l		a0, _cop_write
	rts			


	public		_Cop_SplitList
_Cop_SplitList:
.__local27___auto_85:
	movem.l		d2/a2, -(a7)
	move.w		#258, (a0)+
	move.w		#0, (a0)+
	move.w		#264, (a0)+
	move.w		#0, (a0)+
	move.w		#266, (a0)+
	move.w		#0, (a0)+
	move.l		(a3)+, d0
	cmp.l		#0, d0
	beq			.__local27___auto_70
	move.w		#290, (a0)+
	move.w		d0, (a0)+
	swap		d0
	move.w		#288, (a0)+
	move.w		d0, (a0)+
	move.l		(a3)+, d0
	move.w		#294, (a0)+
	move.w		d0, (a0)+
	swap		d0
	move.w		#292, (a0)+
	move.w		d0, (a0)+
	move.l		(a3)+, d0
	move.w		#298, (a0)+
	move.w		d0, (a0)+
	swap		d0
	move.w		#296, (a0)+
	move.w		d0, (a0)+
	move.l		(a3)+, d0
	move.w		#302, (a0)+
	move.w		d0, (a0)+
	swap		d0
	move.w		#300, (a0)+
	move.w		d0, (a0)+
	move.l		(a3)+, d0
	move.w		#306, (a0)+
	move.w		d0, (a0)+
	swap		d0
	move.w		#304, (a0)+
	move.w		d0, (a0)+
	move.l		(a3)+, d0
	move.w		#310, (a0)+
	move.w		d0, (a0)+
	swap		d0
	move.w		#308, (a0)+
	move.w		d0, (a0)+
	move.l		(a3)+, d0
	move.w		#314, (a0)+
	move.w		d0, (a0)+
	swap		d0
	move.w		#312, (a0)+
	move.w		d0, (a0)+
	move.l		(a3)+, d0
	move.w		#318, (a0)+
	move.w		d0, (a0)+
	swap		d0
	move.w		#316, (a0)+
	move.w		d0, (a0)+
	move.w		#150, (a0)+
	move.w		#$8020, (a0)+
	bra			.__local27___auto_71
.__local27___auto_70:
	move.w		#150, (a0)+
	move.w		#32, (a0)+
.__local27___auto_71:
	move.l		#0, d0
	move.l		#0, d1
.__local27_loop:
	move.b		(a1), d1
	move.w		(a1)+, d0
	btst		#1, d0
	beq			.__local27___auto_83
	btst		#0, d0
	bne			.__local27___auto_72
	move.w		#258, (a0)+
	move.w		d1, (a0)+
	bra			.__local27___auto_73
.__local27___auto_72:
	move.w		d0, (a0)+
	move.w		#$fffe, (a0)+
.__local27___auto_73:
	bra			.__local27___auto_84
.__local27___auto_83:
	btst		#7, d0
	beq			.__local27___auto_81
	btst		#6, d0
	beq			.__local27___auto_74
	and.w		#28, d0
	move.l		(a4,d0), a2
	move.w		(a1)+, d2
	ext.l		d2
	add.l		(a2), d2
	move.w		d1, (a0)+
	move.w		d2, (a0)+
	sub.b		#2, d1
	swap		d2
	move.w		d1, (a0)+
	move.w		d2, (a0)+
	bra			.__local27___auto_75
.__local27___auto_74:
	add.w		d0, d0
	move.w		d0, (a0)+
	move.w		(a1)+, (a0)+
.__local27___auto_75:
	bra			.__local27___auto_82
.__local27___auto_81:
	btst		#0, d0
	beq			.__local27___auto_79
	move.w		#256, (a0)+
	move.w		d0, (a0)+
	bra			.__local27___auto_80
.__local27___auto_79:
	btst		#2, d0
	beq			.__local27___auto_77
	move.l		(a5,d1), a2
	move.w		(a2), d0
	add.w		d0, d0
	lea.l		(a6,d0), a2
	move.w		(a2)+, d1
	move.w		#384, d0
.__local27___auto_76:
	move.w		d0, (a0)+
	move.w		(a2)+, (a0)+
	add.w		#2, d0
	sub.w		#1, d1
	bne			.__local27___auto_76
	bra			.__local27___auto_78
.__local27___auto_77:
	bra			.__local27_end
.__local27___auto_78:
.__local27___auto_80:
.__local27___auto_82:
.__local27___auto_84:
	bra			.__local27_loop
.__local27_end:
	move.l		#-2, (a0)+
	movem.l		(a7)+, d2/a2
	rts			


	public		_Cop_Gradient
_Cop_Gradient:
.__local28___auto_90:
	move.l		_cop_write, a0
	move.w		(a1)+, d0
.__local28___auto_89:
	cmp.w		#$1000, d0
	bcs			.__local28___auto_87
	move.w		d0, d1
	lsr.w		#8, d1
	lsr.w		#4, d1
	lsl.w		#1, d1
	add.w		#384, d1
	move.w		d1, (a0)+
	move.w		d0, (a0)+
	bra			.__local28___auto_88
.__local28___auto_87:
	move.w		_cop_write_y, d1
	add.w		d1, d0
	move.w		d0, _cop_write_y
	eor.w		d0, d1
	and.w		#$ff00, d1
	beq			.__local28___auto_86
	move.l		#$ffdffffe, (a0)+
.__local28___auto_86:
	lsl.w		#8, d0
	or.w		#15, d0
	move.w		d0, (a0)+
	move.w		#$fffe, (a0)+
.__local28___auto_88:
	move.w		(a1)+, d0
	cmp.w		#0, d0
	bne			.__local28___auto_89
	move.l		a0, _cop_write
	rts			


