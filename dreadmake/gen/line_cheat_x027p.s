
    public _Dread_LineCoreCheat_x027p
_Dread_LineCoreCheat_x027p:

    move.l      a7, _a7_save                        ;s d01------ a0123----  20   
                                                    ;s d01------ a0123----       Loop initialization
    move.b      line_ceil_col(a5), d5               ;s d01------ a0123----  12   Prepare ceil
    swap.w      d5                                  ;s d01------ a0123----   4   
    move.b      line_floor_col(a5), d5              ;s d01------ a0123----  12   Prepare floor
    lea.l       _render_columns, a5                 ;s d01------ a0123----  12   Setup column pointer
    add.w       d2, a5                              ;s d01------ a0123----   8   
    bra         .start                              ;s d012----- a0123----  12   End of header
.loop:                                              ;I d012----- a0123----       
    add.w       _asm_line_ds(pc), d4                ;I d012----- a0123----  12   s1 += ds;
    addq.w      #8, a5                              ;I d012----- a0123----   8   
.start:                                             ;- d012----- a0123----       // FIRST ITERATION STARTS HERE
    move.w      rc_ymax(a5), d1                     ;C d0-2----- a0123----  12   
    beq         .hidden_skip                        ;C d0-2----- a0123----  12   
    move.l      (a5), a7                            ;- d0-2----- a0123----  12   byte *dst = render_buffer + offs[x];
    move.w      d4, d0                              ;- d--2----- a0123----   4   int size = (((word)s1)>>3) & ~3;
    lsr.w       #3, d0                              ;- d--2----- a0123----  12   
    and.w       #$FFFC, d0                          ;- d--2----- a0123----   8   
    lea.l       _SIZE_LINE_CHEAT, a4                ;- d--2----- a0123----  12   const LineCheat *lc = (LineCheat*)((byte*)SIZE_LINE_CHEAT + size);
    move.l      (a4,d0.w), a4                       ;- d--2----- a0123----  18   
                                                    ;F d0-2----- a0123----       === [[Ceil
    swap.w      d5                                  ;F d0-2----- a0123----   4   
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a0  ;F d0-2----- a-123----  12   Color filler func
    move.w      rc_ymin(a5), d0                     ;F d--2----- a-123----  12   jsr(fn1, d0) LUT.w:	Ceil length(x2)
    add.w       d0, a7                              ;F d--2----- a-123----   8   skip ymin
    add.w       lc_ceil_len_m96(a4), d0             ;F d--2----- a-123----  12   shorten to end at texture edge
    bge         .skyceil_culled                     ;C d--2----- a-123----  12   
    lea.l       .ceil_ret(pc), a6                   ;F d--2----- a-123----   8   
    jmp         (a0,d0.w)                           ;F d--2----- a-123----  14   
.ceil_ret:                                          ;F d0-2----- a0123----       
                                                    ;G d0-2----- a0123----       === Gap
    move.w      lc_ceil_ymin_m96(a4), rc_ymin(a5)   ;G d0-2----- a0123----  20   ymin
    add.w       lc_hole_size_m96_0(a4), a7          ;G d0-2----- a0123----  16   
                                                    ;F d0-2----- a0123----       === Floor]
.floor_start:                                       ;F d0-2----- a0123----       
    swap.w      d5                                  ;F d0-2----- a0123----   4   
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MID*2, a0  ;F d0-2----- a-123----  12   Color filler func
    move.w      lc_floor_start_0(a4), d0            ;F d--2----- a-123----  12   jsr (fn1,d0)		LUT.w:	Fill func start		(x2)	(fills to screen end)
    move.w      d0, rc_ymax(a5)                     ;F d--2----- a-123----   8   ymax = Gap end
    sub.w       d1, d0                              ;F d--2----- a-123----   4   clip ymax
    lea.l       .floor_ret(pc), a6                  ;F d-12----- a-123----   8   
    jmp         (ENGINE_Y_MAX-ENGINE_Y_MID)*2(a0,d0.w);F d-12----- a-123----  14   
.floor_ret:                                         ;F d012----- a0123----       
.hidden_skip:                                       ;- d012----- a0123----       
    dbra.w      d3, .loop                           ;- d012----- a0123----  12   LOOP END
    bra         .end                                ;- d012----- a0123----  12   End of variant
.skyceil_culled:                                    ;- d0-2----- a0123----       Variant 1
                                                    ;G d0-2----- a0123----       === !Gap
    move.l      (a5), a7                            ;G d0-2----- a0123----  12   byte *dst = render_buffer + offs[x];
    add.w       lc_hole_end_0(a4), a7               ;G d0-2----- a0123----  16   
                                                    ;F d0-2----- a0123----       === Floor]
.floor_start1:                                      ;F d0-2----- a0123----       
    swap.w      d5                                  ;F d0-2----- a0123----   4   
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MID*2, a0  ;F d0-2----- a-123----  12   Color filler func
    move.w      lc_floor_start_0(a4), d0            ;F d--2----- a-123----  12   jsr (fn1,d0)		LUT.w:	Fill func start		(x2)	(fills to screen end)
    move.w      d0, rc_ymax(a5)                     ;F d--2----- a-123----   8   ymax = Gap end
    sub.w       d1, d0                              ;F d--2----- a-123----   4   clip ymax
    lea.l       .floor_ret1(pc), a6                 ;F d-12----- a-123----   8   
    jmp         (ENGINE_Y_MAX-ENGINE_Y_MID)*2(a0,d0.w);F d-12----- a-123----  14   
.floor_ret1:                                        ;F d012----- a0123----       
    dbra.w      d3, .loop                           ;- d012----- a0123----  12   LOOP END
.end:                                               ;o d012----- a0123----       Cleanup & exit
    move.l      _a7_save(pc), a7                    ;o d012----- a0123----  16   
    rts                                             ;o d012----- a0123----  16   
