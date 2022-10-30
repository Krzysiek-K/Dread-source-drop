
    public _Dread_LineCoreCheat_x029p
_Dread_LineCoreCheat_x029p:

    move.l      a7, _a7_save                        ;0 d01------ a0123----  20   
                                                    ;0 d01------ a0123----       Loop initialization
    move.b      line_ceil_col(a5), d5               ;0 d01------ a0123----  12   Prepare ceil
    swap.w      d5                                  ;0 d01------ a0123----   4   
    move.b      line_floor_col(a5), d5              ;0 d01------ a0123----  12   Prepare floor
    lea.l       _render_columns, a5                 ;0 d01------ a0123----  12   Setup column pointer
    add.w       d2, a5                              ;0 d01------ a0123----   8   
    bra         .start                              ;0 d012----- a0123----  10   End of header
.loop:                                              ;1 d012----- a0123----       
    add.w       _asm_line_ds(pc), d4                ;1 d012----- a0123----  12   s1 += ds;
    addq.w      #8, a5                              ;1 d012----- a0123----   8   
.start:                                             ;1 d012----- a0123----       // FIRST ITERATION STARTS HERE
    move.w      rc_ymax(a5), d1                     ;1 d0-2----- a0123----  12   
    beq         .hidden_skip                        ;1 d0-2----- a0123----  10   
    move.l      (a5), a7                            ;1 d0-2----- a0123----  12   byte *dst = render_buffer + offs[x];
    move.w      d4, d0                              ;1 d--2----- a0123----   4   int size = (((word)s1)>>3) & ~3;
    lsr.w       #3, d0                              ;1 d--2----- a0123----  12   
    and.w       #$FFFC, d0                          ;1 d--2----- a0123----   8   
    lea.l       _SIZE_LINE_CHEAT, a4                ;1 d--2----- a0123----  12   const LineCheat *lc = (LineCheat*)((byte*)SIZE_LINE_CHEAT + size);
    move.l      (a4,d0.w), a4                       ;1 d--2----- a0123----  18   
                                                    ;1 d0-2----- a0123----       === [[Ceil
    swap.w      d5                                  ;1 d0-2----- a0123----   4   
    lea.l       _FUNC_COLORFILL+100, a0             ;1 d0-2----- a-123----  12   Color filler func
    move.w      rc_ymin(a5), d0                     ;1 d--2----- a-123----  12   jsr(fn1, d0) LUT.w:	Ceil length(x2)
    add.w       d0, a7                              ;1 d--2----- a-123----   8   skip ymin
    add.w       lc_ceil_len_m128(a4), d0            ;1 d--2----- a-123----  12   shorten by ymin
    bge         .ceil_ret                           ;1 d--2----- a-123----  10   
    lea.l       .ceil_ret(pc), a6                   ;1 d--2----- a-123----   8   
    jmp         100(a0,d0.w)                        ;1 d--2----- a-123----  14   
.ceil_ret:                                          ;1 d0-2----- a0123----       
                                                    ;1 d0-2----- a0123----       === Gap
    move.w      lc_ceil_ymin_m128(a4), rc_ymin(a5)  ;1 d0-2----- a0123----  20   ymin
    add.w       lc_hole_size_m128_0(a4), a7         ;1 d0-2----- a0123----  16   
                                                    ;1 d0-2----- a0123----       === Floor]
    swap.w      d5                                  ;1 d0-2----- a0123----   4   
    lea.l       _FUNC_COLORFILL+100, a0             ;1 d0-2----- a-123----  12   Color filler func
    move.w      lc_floor_start_0(a4), d0            ;1 d--2----- a-123----  12   jsr (fn1,d0)		LUT.w:	Fill func start		(x2)	(fills to screen end)
    move.w      d0, rc_ymax(a5)                     ;1 d--2----- a-123----   8   ymax = Gap end
    sub.w       d1, d0                              ;1 d--2----- a-123----   4   clip ymax
    lea.l       .floor_ret(pc), a6                  ;1 d-12----- a-123----   8   
    jmp         100(a0,d0.w)                        ;1 d-12----- a-123----  14   
.floor_ret:                                         ;1 d012----- a0123----       
.hidden_skip:                                       ;1 d012----- a0123----       
    dbra.w      d3, .loop                           ;1 d012----- a0123----  10   LOOP END
.end:                                               ;0 d012----- a0123----       Cleanup & exit
    move.l      _a7_save(pc), a7                    ;0 d012----- a0123----  16   
    rts                                             ;0 d012----- a0123----  16   
