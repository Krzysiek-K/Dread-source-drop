
    public _Dread_LineCoreCheat_x030p
_Dread_LineCoreCheat_x030p:

    move.l      a7, _a7_save                        ;s d01------ a012-----  20   
                                                    ;s d01------ a012-----       Loop initialization
    move.b      line_floor_col(a5), d5              ;s d01------ a012-----  12   Prepare floor
    lea.l       _render_columns, a5                 ;s d01------ a012-----  12   Setup column pointer
    add.w       d2, a5                              ;s d01------ a012-----   8   
    lsr.w       #1, d2                              ;s d01------ a012-----   8   SKY
    move.l      _e_skyptr, a3                       ;s d01------ a012-----  20   
    add.w       d2, a3                              ;s d01------ a012-----   8   
    bra         .start                              ;s d012----- a012-----  12   End of header
.loop:                                              ;I d012----- a012-----       
    add.w       _asm_line_ds(pc), d4                ;I d012----- a012-----  12   s1 += ds;
    addq.w      #8, a5                              ;I d012----- a012-----   8   
.start:                                             ;- d012----- a012-----       // FIRST ITERATION STARTS HERE
    move.w      rc_ymax(a5), d0                     ;C d-12----- a012-----  12   
    beq         .hidden_skip                        ;C d012----- a012-----  12   
    move.l      (a5), a7                            ;- d012----- a012-----  12   byte *dst = render_buffer + offs[x];
    move.w      d4, d0                              ;- d-12----- a012-----   4   int size = (((word)s1)>>3) & ~3;
    lsr.w       #3, d0                              ;- d-12----- a012-----  12   
    and.w       #$FFFC, d0                          ;- d-12----- a012-----   8   
    lea.l       _SIZE_LINE_CHEAT, a4                ;- d-12----- a012-----  12   const LineCheat *lc = (LineCheat*)((byte*)SIZE_LINE_CHEAT + size);
    move.l      (a4,d0.w), a4                       ;- d-12----- a012-----  18   
                                                    ;S d012----- a012-----       === Sky
    lea.l       _FUNC_SKYCOPY+ENGINE_Y_MAX*2, a0    ;S d012----- a-12-----  12   Fill func
    move.l      (a3)+, a2                           ;S d012----- a-1------  12   
    move.w      lc_ceil_len_m128(a4), d0            ;S d-12----- a-12-----  12   jsr(fn1, d0) LUT.w:	Ceil length(x2)
    lea.l       .sky_ret(pc), a6                    ;S d-12----- a-12-----   8   
    jmp         (a0,d0.w)                           ;S d-12----- a-12-----  14   
.sky_ret:                                           ;S d012----- a012-----       
                                                    ;G d012----- a012-----       === Gap
    move.w      lc_ceil_ymin_m128(a4), rc_ymin(a5)  ;G d012----- a012-----  20   ymin
    add.w       lc_hole_size_m128_0(a4), a7         ;G d012----- a012-----  16   
                                                    ;F d012----- a012-----       === Floor
.floor_start:                                       ;F d012----- a012-----       
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MID*2, a0  ;F d012----- a-12-----  12   Color filler func
    move.w      lc_floor_start_0(a4), d0            ;F d-12----- a-12-----  12   jsr (fn1,d0)		LUT.w:	Fill func start		(x2)	(fills to screen end)
    move.w      d0, rc_ymax(a5)                     ;F d-12----- a-12-----   8   ymax = Gap end
    lea.l       .floor_ret(pc), a6                  ;F d-12----- a-12-----   8   
    jmp         -ENGINE_Y_MID*2(a0,d0.w)            ;F d-12----- a-12-----  14   
.floor_ret:                                         ;F d012----- a012-----       
    dbra.w      d3, .loop                           ;- d012----- a012-----  12   LOOP END
    bra         .end                                ;- d012----- a012-----  12   End of variant
.hidden_skip:                                       ;- d012----- a012-----       
    addq.l      #4, a3                              ;S d012----- a012-----   8   
    dbra.w      d3, .loop                           ;- d012----- a012-----  12   LOOP END
.end:                                               ;o d012----- a012-----       Cleanup & exit
    move.l      _a7_save(pc), a7                    ;o d012----- a012-----  16   
    rts                                             ;o d012----- a012-----  16   
