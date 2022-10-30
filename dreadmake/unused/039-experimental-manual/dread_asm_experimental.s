
		xref		_view_pos_z


	;	void(*line_cheat_fn)(__d2 int xmin, __d3 int xcount, __d4 dword s1, __d6 dword u1s, __d7 int du, __a5 EngineLine *line);
	;		d2:	xmin<<3
	;		d6:	u1s
	;		d3:	xcount-1
	;
	;		d4:	s1
	;		d7: du
	;		a5: line


;	move.w      d4, d2                              ;  int size = (((word)s1)>>3) & ~3;			// sssssssssss-----
;	lsr.w       #5, d2                              ;	0..2047
;	move.w		#-64, d0
;	sub.w		_view_pos_z, d0
;	muls.w		d0, d2								;		m128 = (-88*size)>>8;			// int m128 = ((-128 - (view_pos_z=-40) )*size)>>8;
;	asr.l       #8, d2
;	add.w		#ENGINE_Y_MID, d2



;
;	Register usage:
;
;			Input				Runtime							New
;
;	d0	-														
;	d1	-														
;	d2	-	xmin												y0 (tracer)
;	d3	-	xcount												Loop counter
;	d4	-	s1													s1 (current)
;	d5	-						Fill color						[fill color]
;	d6	-	u1s													
;	d7	-	du													
;
;	a0	-														(temp)
;	a1	-
;	a2	-						Texture/sky source
;	a3	-
;	a4	-														render columns table
;	a5	-	line
;	a6	-						Draw return ptr
;	a7	-						Render destination
;
;
;	Draw code:
;
;		Textured		MOVE.b	xxx(A2), (A7)+
;		Color			MOVE.b	D5, (A7)+
;		Sky				MOVE.b	(A2)+, (A7)+
;		End				JMP (A6)
;


    public _Dread_LineCoreCheat_experimental
_Dread_LineCoreCheat_experimental:

;    move.l      a7, _a7_save                        ;   
;                                                    ;   Loop initialization
;    move.b      line_ceil_col(a5), d5               ;   Prepare ceil
;    swap.w      d5                                  ;   
;    move.b      line_floor_col(a5), d5              ;   Prepare floor
;    lea.l       _render_columns, a4                 ;   Setup column pointer
;    add.w       d2, a4                              ;   
;
;													; compute starting Y0  -> d2
;	move.w      d4, d2                              ;	Y = ( ((-height) - view_pos_z) * (s>>5) ) >> 8   + ENGINE_Y_MID		[ d2 ]			| d4:s1	d0:temp
;	lsr.w       #5, d2                              ;
;	move.w		#-64, d0
;	sub.w		_view_pos_z, d0
;	muls.w		d0, d2								;
;
;													; compute Y0 slope  -> d1
;	move.w      _asm_line_s2, d1					;
;	lsr.w       #5, d1                              ;
;	muls.w		d0, d1								;		d0: -height - view_pos_z
;	sub.l		d2, d1								;	d1 = (yend - ystart)*256 / asm_line_xlen
;	divs.w		_asm_line_xlen, d1
;
;
;													; initialize y0 line tracer
;													;	
;	move.b		d1, .trace_s+2						;	subpixel step
;	lsr.w		#8, d1
;	ext.w		d1
;	move.w		d1, .trace_y0+4						;	pixel step
;
;	add.l		#ENGINE_Y_MID*256+128, d2			; d2 = $xxxxxxff		(24.8 fixed)
;	lsl.l		#8, d2								; d2 = $xxxxff00		(16.16 fixed)
;	swap.w		d4									; d4 = subpixel value of d2  |  s			$xx00ssss
;	move.w		d2, d4
;	swap.w		d4
;	
;	
;	sub.w		d2, d2
;	swap.w		d2									; d2 = current Y value

;
;	move.w		_asm_line_ds(pc), .trace_s+4
;
;
;
;    bra         .start                              ;   End of header
;
;
;
;.loop:                                              ;   
;	addq.w      #8, a4                              ;
;    add.l       d7, d6                              ;   u1s += du;
;
;.trace_s:		add.l		#$FF00FFFF, d4			;   s1 += ds
;.trace_y0:		move.l		#$FF000000, d0			;	$000000xx
;				addx.l		d0, d2
;
;.start:                                             ;   // FIRST ITERATION STARTS HERE


;	cmp.w		#ENGINE_Y_MAX, d2					; d2 ? ENGINE_Y_MAX
;	bge			.draw_full
;
;
;    move.l      (a4), a7                            ;   byte *dst = render_buffer + offs[x];
;    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a0  ;   Color filler func
;	moveq.l		#0, d0
;	sub.w		d2, d0								; 0 ? d2
;	bge			.hidden_skip
;	sub.w		d2, d0
;
;    lea.l       .ceil_ret(pc), a6                   ;
;    jmp         (a0,d0.w)                           ;
;.ceil_ret:                                          ;







    move.l      a7, _a7_save                        ;0 d01------ a01------
                                                    ;0 d01------ a01------       Loop initialization
    move.b      line_ceil_col(a5), d5               ;0 d01------ a01------
    swap.w      d5                                  ;0 d01------ a01------
    move.b      line_floor_col(a5), d5              ;0 d01------ a01------
    lea.l       _render_columns, a4                 ;0 d01------ a01------
    add.w       d2, a4                              ;0 d01------ a01------
                                                    ;0 d012----- a01------       ======== init .trace_y0 tracer ========
    move.w      d4, d2                              ;0 d01------ a01------       compute starting Y
    lsr.w       #5, d2                              ;0 d01------ a01------
    move.w      #-64, d0                            ;0 d-1------ a01------
    sub.w       _view_pos_z, d0                     ;0 d-1------ a01------
    muls.w      d0, d2                              ;0 d-1------ a01------
    move.w      _asm_line_s2, d1                    ;0 d-------- a01------       compute Y slope
    lsr.w       #5, d1                              ;0 d-------- a01------
    muls.w      d0, d1                              ;0 d-------- a01------
    sub.l       d2, d1                              ;0 d0------- a01------
    divs.w      _asm_line_xlen, d1                  ;0 d0------- a01------
    move.b      d1, .trace_s+2                      ;0 d0------- a01------       subpixel step
    lsr.w       #8, d1                              ;0 d0------- a01------
    ext.w       d1                                  ;0 d0------- a01------
    move.w      d1, .trace_y0+4                     ;0 d0------- a01------       pixel step
    add.l       #ENGINE_Y_MID*256+128, d2           ;0 d01------ a01------       24.8 fixed
    lsl.l       #8, d2                              ;0 d01------ a01------       16.16 fixed
    swap.w      d4                                  ;0 d01------ a01------
    move.w      d2, d4                              ;0 d01------ a01------
    swap.w      d4                                  ;0 d01------ a01------
    sub.w       d2, d2                              ;0 d01------ a01------
    swap.w      d2                                  ;0 d01------ a01------       current Y value
    move.w      _asm_line_ds(pc), .trace_s+4        ;0 d01------ a01------
    bra         .start                              ;0 d01------ a01------       End of header
                                                    ;0 d01------ a01------
.loop:                                              ;0 d01------ a01------
    addq.w      #8, a4                              ;0 d01------ a01------
.trace_s:                                           ;0 d01------ a01------
    add.l       #$FF00FFFF, d4                      ;0 d01------ a01------       s1 += ds
.trace_y0:                                          ;0 d01------ a01------
    move.l      #$FF000000, d0                      ;0 d-1------ a01------
    addx.l      d0, d2                              ;0 d-1------ a01------
.start:                                             ;0 d01------ a01------       // FIRST ITERATION STARTS HERE
    move.l      (a4), a7                            ;0 d01------ a01------       byte *dst = render_buffer + offs[x];
    move.w      rc_ymin(a4), d0                     ;0 d-1------ a01------       Load ymin
    asr.w       #1, d0                              ;0 d-1------ a01------
    cmp.w       d2, d0                              ;0 d-1------ a01------       ymin>y0
    bgt         .cmp_1                              ;0 d01------ a01------
    move.w      rc_ymax(a4), d1                     ;0 d0------- a01------       Load ymax
    asr.w       #1, d1                              ;0 d0------- a01------
    cmp.w       d2, d1                              ;0 d0------- a01------       ymax>y0
    bgt         .cmp_2                              ;0 d01------ a01------
                                                    ;0 d012----- a01------       DRAW:  [Ceil]
                                                    ;0 d012----- a01------       ======== [Ceil] ========
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a0  ;0 d012----- a-1------       Color filler func
    move.w      rc_ymin(a4), d0                     ;0 d-12----- a-1------       Load ymin
    asr.w       #1, d0                              ;0 d-12----- a-1------
    move.w      rc_ymax(a4), d1                     ;0 d--2----- a-1------       Load ymax
    asr.w       #1, d1                              ;0 d--2----- a-1------
    sub.w       d1, d0                              ;0 d--2----- a-1------
    add.w       d0, d0                              ;0 d-12----- a-1------
    lea.l       .ret_3, a6                          ;0 d-12----- a-1------
    jmp         (a0,d0.w)                           ;0 d-12----- a-1------
.ret_3:                                             ;0 d012----- a01------
    bra         .end_loop                           ;0 d012----- a01------
.cmp_2:                                             ;0 d01------ a01------
                                                    ;0 d01------ a01------       DRAW:  [Ceil Floor]
                                                    ;0 d01------ a01------       ======== [Ceil ========
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a0  ;0 d01------ a-1------       Color filler func
    move.w      rc_ymin(a4), d0                     ;0 d-1------ a-1------       Load ymin
    asr.w       #1, d0                              ;0 d-1------ a-1------
    sub.w       d2, d0                              ;0 d-1------ a-1------
    add.w       d0, d0                              ;0 d-12----- a-1------
    lea.l       .ret_4, a6                          ;0 d-12----- a-1------
    jmp         (a0,d0.w)                           ;0 d-12----- a-1------
.ret_4:                                             ;0 d012----- a01------
    bra         .end_loop                           ;0 d012----- a01------
.cmp_1:                                             ;0 d012----- a01------
                                                    ;0 d012----- a01------       DRAW:  [Floor]
    bra         .end_loop                           ;0 d012----- a01------
                                                    ;0 d012----- a01------













.hidden_skip:                                       ;   
.end_loop:
    dbra.w      d3, .loop                           ;   LOOP END
.end:                                               ;   Cleanup & exit
    move.l      _a7_save(pc), a7                    ;   
    rts                                             ;   


;.draw_full:
;    move.l      (a4), a7                            ;   byte *dst = render_buffer + offs[x];
;    lea.l       .ceil_ret(pc), a6                   ;
;    jmp			_FUNC_COLORFILL						;   Color filler func
