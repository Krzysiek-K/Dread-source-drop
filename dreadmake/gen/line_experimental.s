
    public _Dread_LineCoreCheat_experimental
_Dread_LineCoreCheat_experimental:

    move.l      a7, _a7_save                        ;0 d01------ a01------  20   
                                                    ;0 d01------ a01------       
                                                    ;0 d01------ a01------       ======== Loop initialization ========
    lea.l       _render_columns, a4                 ;0 d01------ a01------  12   
    add.w       d2, a4                              ;0 d01------ a01------   8   add xmin*8 to column pointer
    move.l      a4, a0                              ;0 d012----- a-1------   4   end = start + count*8
    lsl.w       #3, d3                              ;0 d012----- a-1------  12   
    add.w       d3, a0                              ;0 d012----- a-1------   8   
                                                    ;0 d012----- a-1------       
                                                    ;0 d012----- a-1------       ======== init .trace_y0 tracer ========
    move.w      d4, d0                              ;0 d-12----- a-1------   4   compute starting Y
    lsr.w       #5, d0                              ;0 d-12----- a-1------  16   
    move.w      line_y1(a5), d1                     ;0 d--2----- a-1------  12   
    sub.w       _view_pos_z, d1                     ;0 d--2----- a-1------  16   
    muls.w      d1, d0                              ;0 d--2----- a-1------  70   
    move.w      _asm_line_s2, d5                    ;0 d--2----- a-1------  16   compute Y slope
    lsr.w       #5, d5                              ;0 d--2----- a-1------  16   
    muls.w      d1, d5                              ;0 d--2----- a-1------  70   
    sub.l       d0, d5                              ;0 d-12----- a-1------   8   
    divs.w      _asm_line_xlen, d5                  ;0 d-12----- a-1------ 170   
    move.b      d5, .trace_s+2                      ;0 d-12----- a-1------  16   subpixel step
    lsr.w       #8, d5                              ;0 d-12----- a-1------  22   
    ext.w       d5                                  ;0 d-12----- a-1------   4   
    move.w      d5, .trace_y0+4                     ;0 d-12----- a-1------  16   pixel step
    add.l       #ENGINE_Y_MID*256+128, d0           ;0 d-12----- a-1------  16   24.8 fixed
    lsl.l       #8, d0                              ;0 d-12----- a-1------  24   16.16 fixed
    swap.w      d4                                  ;0 d-12----- a-1------   4   
    move.w      d0, d4                              ;0 d-12----- a-1------   4   
    swap.w      d4                                  ;0 d-12----- a-1------   4   
    sub.w       d0, d0                              ;0 d-12----- a-1------   4   
    swap.w      d0                                  ;0 d-12----- a-1------   4   current Y value
                                                    ;0 d-12----- a-1------       
                                                    ;0 d-12----- a-1------       ======== init .trace_y1 tracer ========
    move.w      d4, d1                              ;0 d--2----- a-1------   4   compute starting Y
    lsr.w       #5, d1                              ;0 d--2----- a-1------  16   
    move.w      line_y2(a5), d2                     ;0 d-------- a-1------  12   
    sub.w       _view_pos_z, d2                     ;0 d-------- a-1------  16   
    muls.w      d2, d1                              ;0 d-------- a-1------  70   
    move.w      _asm_line_s2, d5                    ;0 d-------- a-1------  16   compute Y slope
    lsr.w       #5, d5                              ;0 d-------- a-1------  16   
    muls.w      d2, d5                              ;0 d-------- a-1------  70   
    sub.l       d1, d5                              ;0 d--2----- a-1------   8   
    divs.w      _asm_line_xlen, d5                  ;0 d--2----- a-1------ 170   
    move.b      d5, .trace_y0+2                     ;0 d--2----- a-1------  16   subpixel step
    lsr.w       #8, d5                              ;0 d--2----- a-1------  22   
    ext.w       d5                                  ;0 d--2----- a-1------   4   
    move.w      d5, .trace_y1+4                     ;0 d--2----- a-1------  16   pixel step
    add.l       #ENGINE_Y_MID*256+128, d1           ;0 d--2----- a-1------  16   24.8 fixed
    lsl.l       #8, d1                              ;0 d--2----- a-1------  24   16.16 fixed
    swap.w      d0                                  ;0 d--2----- a-1------   4   
    move.w      d1, d0                              ;0 d--2----- a-1------   4   
    swap.w      d0                                  ;0 d--2----- a-1------   4   
    sub.w       d1, d1                              ;0 d--2----- a-1------   4   
    swap.w      d1                                  ;0 d--2----- a-1------   4   current Y value
                                                    ;0 d--2----- a-1------       
                                                    ;0 d--2----- a-1------       ======== init .trace_y2 tracer ========
    move.w      d4, d2                              ;0 d-------- a-1------   4   compute starting Y
    lsr.w       #5, d2                              ;0 d-------- a-1------  16   
    move.w      line_y3(a5), d3                     ;0 d-------- a-1------  12   
    sub.w       _view_pos_z, d3                     ;0 d-------- a-1------  16   
    muls.w      d3, d2                              ;0 d-------- a-1------  70   
    move.w      _asm_line_s2, d5                    ;0 d-------- a-1------  16   compute Y slope
    lsr.w       #5, d5                              ;0 d-------- a-1------  16   
    muls.w      d3, d5                              ;0 d-------- a-1------  70   
    sub.l       d2, d5                              ;0 d-------- a-1------   8   
    divs.w      _asm_line_xlen, d5                  ;0 d-------- a-1------ 170   
    move.b      d5, .trace_y1+2                     ;0 d-------- a-1------  16   subpixel step
    lsr.w       #8, d5                              ;0 d-------- a-1------  22   
    ext.w       d5                                  ;0 d-------- a-1------   4   
    move.w      d5, .trace_y2+4                     ;0 d-------- a-1------  16   pixel step
    add.l       #ENGINE_Y_MID*256+128, d2           ;0 d-------- a-1------  16   24.8 fixed
    lsl.l       #8, d2                              ;0 d-------- a-1------  24   16.16 fixed
    swap.w      d1                                  ;0 d-------- a-1------   4   
    move.w      d2, d1                              ;0 d-------- a-1------   4   
    swap.w      d1                                  ;0 d-------- a-1------   4   
    sub.w       d2, d2                              ;0 d-------- a-1------   4   
    swap.w      d2                                  ;0 d-------- a-1------   4   current Y value
                                                    ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       ======== init .trace_y3 tracer ========
    move.w      d4, d3                              ;0 d-------- a-1------   4   compute starting Y
    lsr.w       #5, d3                              ;0 d-------- a-1------  16   
    move.w      line_y4(a5), d6                     ;0 d-------- a-1------  12   
    sub.w       _view_pos_z, d6                     ;0 d-------- a-1------  16   
    muls.w      d6, d3                              ;0 d-------- a-1------  70   
    move.w      _asm_line_s2, d5                    ;0 d-------- a-1------  16   compute Y slope
    lsr.w       #5, d5                              ;0 d-------- a-1------  16   
    muls.w      d6, d5                              ;0 d-------- a-1------  70   
    sub.l       d3, d5                              ;0 d-------- a-1------   8   
    divs.w      _asm_line_xlen, d5                  ;0 d-------- a-1------ 170   
    move.b      d5, .trace_y2+2                     ;0 d-------- a-1------  16   subpixel step
    lsr.w       #8, d5                              ;0 d-------- a-1------  22   
    ext.w       d5                                  ;0 d-------- a-1------   4   
    move.w      d5, .trace_y3+4                     ;0 d-------- a-1------  16   pixel step
    add.l       #ENGINE_Y_MID*256+128, d3           ;0 d-------- a-1------  16   24.8 fixed
    lsl.l       #8, d3                              ;0 d-------- a-1------  24   16.16 fixed
    swap.w      d2                                  ;0 d-------- a-1------   4   
    move.w      d3, d2                              ;0 d-------- a-1------   4   
    swap.w      d2                                  ;0 d-------- a-1------   4   
    sub.w       d3, d3                              ;0 d-------- a-1------   4   
    swap.w      d3                                  ;0 d-------- a-1------   4   current Y value
    move.w      _asm_line_ds(pc), .trace_s+4        ;0 d-------- a-1------  28   
    move.b      line_ceil_col(a5), d5               ;0 d-------- a-1------  12   
    swap.w      d5                                  ;0 d-------- a-1------   4   
    move.b      line_floor_col(a5), d5              ;0 d-------- a-1------  12   
    bra         .start                              ;0 d-------- a-1------  10   End of header
                                                    ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       
.loop:                                              ;0 d-------- a-1------       ======== LOOP START ========
    addq.w      #8, a4                              ;0 d-------- a-1------   8   
.trace_s:                                           ;0 d-------- a-1------       
    add.l       #$FF00FFFF, d4                      ;0 d-------- a-1------  16   s1 += ds
.trace_y0:                                          ;0 d-------- a-1------       
    move.l      #$FF000000, d6                      ;0 d-------- a-1------  12   
    addx.l      d6, d0                              ;0 d-------- a-1------   8   
.trace_y1:                                          ;0 d-------- a-1------       
    move.l      #$FF000000, d6                      ;0 d-------- a-1------  12   
    addx.l      d6, d1                              ;0 d-------- a-1------   8   
.trace_y2:                                          ;0 d-------- a-1------       
    move.l      #$FF000000, d6                      ;0 d-------- a-1------  12   
    addx.l      d6, d2                              ;0 d-------- a-1------   8   
.trace_y3:                                          ;0 d-------- a-1------       
    move.l      #$FF000000, d6                      ;0 d-------- a-1------  12   
    addx.l      d6, d3                              ;0 d-------- a-1------   8   
                                                    ;0 d-------- a-1------       
.start:                                             ;0 d-------- a-1------       ======== FIRST ITERATION STARTS HERE ========
    move.l      (a4), a7                            ;0 d-------- a-1------  12   byte *dst = render_buffer + offs[x];
                                                    ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- ymin/ymax not checked --------
    move.w      rc_ymax(a4), d6                     ;0 d-------- a-1------  12   Load ymax
    cmp.w       d2, d6                              ;0 d-------- a-1------   4   ymax>=y2
    bge         .cmp_1                              ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- ymin < y2 && ymax < y2 --------
    cmp.w       d1, d6                              ;0 d-------- a-1------   4   ymax>=y1
    bge         .cmp_2                              ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- ymin < y1 && ymax < y1 --------
    cmp.w       d0, d6                              ;0 d-------- a-1------   4   ymax>=y0
    bge         .cmp_3                              ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- ymin < y0 && ymax < y0 --------
                                                    ;0 d-------- a-1------       ======== [Ceil] ========
                                                    ;0 d-------- a-1------       -------- [Ceil] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    swap.w      d5                                  ;0 d-------- a--------   4   select ceiling color
    move.w      rc_ymin(a4), d7                     ;0 d-------- a--------  12   Load ymin
    sub.w       d6, d7                              ;0 d-------- a--------   4   
    add.w       d7, d7                              ;0 d-------- a--------   4   
    lea.l       .ret_4, a6                          ;0 d-------- a--------  12   
    jmp         (a1,d7.w)                           ;0 d-------- a--------  14   
.ret_4:                                             ;0 d-------- a-1------       
    swap.w      d5                                  ;0 d-------- a-1------   4   deselect ceiling color
    clr.l       rc_ymin(a4)                         ;0 d-------- a-1------       Disable further rendering - clear ymin/ymax
    bra         .end_loop                           ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
.cmp_3:                                             ;0 d-------- a-1------       -------- ymin < y1 && y0 <= ymax < y1 --------
    move.w      rc_ymin(a4), d7                     ;0 d-------- a-1------  12   Load ymin
    cmp.w       d0, d7                              ;0 d-------- a-1------   4   ymin>=y0
    bge         .cmp_5                              ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- ymin < y0 && y0 <= ymax < y1 --------
                                                    ;0 d-------- a-1------       ======== [Ceil Floor] ========
                                                    ;0 d-------- a-1------       -------- [Ceil --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    swap.w      d5                                  ;0 d-------- a--------   4   select ceiling color
    sub.w       d0, d7                              ;0 d-------- a--------   4   
    add.w       d7, d7                              ;0 d-------- a--------   4   
    lea.l       .ret_6, a6                          ;0 d-------- a--------  12   
    jmp         (a1,d7.w)                           ;0 d-------- a--------  14   
.ret_6:                                             ;0 d-------- a-1------       
    swap.w      d5                                  ;0 d-------- a-1------   4   deselect ceiling color
                                                    ;0 d-------- a-1------       -------- Floor] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    sub.w       d0, d6                              ;0 d-------- a--------   4   
    neg.w       d6                                  ;0 d-------- a--------   4   
    add.w       d6, d6                              ;0 d-------- a--------   4   
    lea.l       .ret_7, a6                          ;0 d-------- a--------  12   
    jmp         (a1,d6.w)                           ;0 d-------- a--------  14   
.ret_7:                                             ;0 d-------- a-1------       
    clr.l       rc_ymin(a4)                         ;0 d-------- a-1------       Disable further rendering - clear ymin/ymax
    bra         .end_loop                           ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
.cmp_5:                                             ;0 d-------- a-1------       -------- y0 <= ymin < y1 && y0 <= ymax < y1 --------
                                                    ;0 d-------- a-1------       ======== [Floor] ========
                                                    ;0 d-------- a-1------       -------- [Floor] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    sub.w       d6, d7                              ;0 d-------- a--------   4   
    add.w       d7, d7                              ;0 d-------- a--------   4   
    lea.l       .ret_8, a6                          ;0 d-------- a--------  12   
    jmp         (a1,d7.w)                           ;0 d-------- a--------  14   
.ret_8:                                             ;0 d-------- a-1------       
    clr.l       rc_ymin(a4)                         ;0 d-------- a-1------       Disable further rendering - clear ymin/ymax
    bra         .end_loop                           ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
.cmp_2:                                             ;0 d-------- a-1------       -------- ymin < y2 && y1 <= ymax < y2 --------
    move.w      rc_ymin(a4), d7                     ;0 d-------- a-1------  12   Load ymin
    cmp.w       d0, d7                              ;0 d-------- a-1------   4   ymin>=y0
    bge         .cmp_9                              ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- ymin < y0 && y1 <= ymax < y2 --------
                                                    ;0 d-------- a-1------       ======== [Ceil Floor gap] ========
                                                    ;0 d-------- a-1------       -------- [Ceil --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    swap.w      d5                                  ;0 d-------- a--------   4   select ceiling color
    sub.w       d0, d7                              ;0 d-------- a--------   4   
    add.w       d7, d7                              ;0 d-------- a--------   4   
    lea.l       .ret_10, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d7.w)                           ;0 d-------- a--------  14   
.ret_10:                                            ;0 d-------- a-1------       
    swap.w      d5                                  ;0 d-------- a-1------   4   deselect ceiling color
                                                    ;0 d-------- a-1------       -------- Floor --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    move.w      d0, d7                              ;0 d-------- a--------   4   
    sub.w       d1, d7                              ;0 d-------- a--------   4   
    add.w       d7, d7                              ;0 d-------- a--------   4   
    lea.l       .ret_11, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d7.w)                           ;0 d-------- a--------  14   
.ret_11:                                            ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- gap] --------
    move.w      d1, rc_ymin(a4)                     ;0 d-------- a-1------   8   Update rc_ymin with ystart
    move.l      a7, (a4)                            ;0 d-------- a-1------  12   Set future drawing start pointer to current pos
    sub.w       d1, d6                              ;0 d-------- a-1------   4   Add 2*(rc_ymax - ystart) to A7
    add.w       d6, d6                              ;0 d-------- a-1------   4   
    add.w       d6, a7                              ;0 d-------- a-1------   8   
    bra         .end_loop                           ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
.cmp_9:                                             ;0 d-------- a-1------       -------- y0 <= ymin < y2 && y1 <= ymax < y2 --------
    cmp.w       d1, d7                              ;0 d-------- a-1------   4   ymin>=y1
    bge         .cmp_12                             ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- y0 <= ymin < y1 && y1 <= ymax < y2 --------
                                                    ;0 d-------- a-1------       ======== [Floor gap] ========
                                                    ;0 d-------- a-1------       -------- [Floor --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    sub.w       d1, d7                              ;0 d-------- a--------   4   
    add.w       d7, d7                              ;0 d-------- a--------   4   
    lea.l       .ret_13, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d7.w)                           ;0 d-------- a--------  14   
.ret_13:                                            ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- gap] --------
    move.w      d1, rc_ymin(a4)                     ;0 d-------- a-1------   8   Update rc_ymin with ystart
    move.l      a7, (a4)                            ;0 d-------- a-1------  12   Set future drawing start pointer to current pos
    sub.w       d1, d6                              ;0 d-------- a-1------   4   Add 2*(rc_ymax - ystart) to A7
    add.w       d6, d6                              ;0 d-------- a-1------   4   
    add.w       d6, a7                              ;0 d-------- a-1------   8   
    bra         .end_loop                           ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
.cmp_12:                                            ;0 d-------- a-1------       -------- y1 <= ymin < y2 && y1 <= ymax < y2 --------
                                                    ;0 d-------- a-1------       ======== [gap] ========
                                                    ;0 d-------- a-1------       -------- [gap] --------
                                                    ;0 d-------- a-1------       nothing to be done
    bra         .end_loop                           ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
.cmp_1:                                             ;0 d-------- a-1------       -------- y2 <= ymax --------
    move.w      rc_ymin(a4), d7                     ;0 d-------- a-1------  12   Load ymin
    cmp.w       d2, d7                              ;0 d-------- a-1------   4   ymin>=y2
    bge         .cmp_14                             ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- ymin < y2 && y2 <= ymax --------
    cmp.w       d0, d7                              ;0 d-------- a-1------   4   ymin>=y0
    bge         .cmp_15                             ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- ymin < y0 && y2 <= ymax --------
    cmp.w       d3, d6                              ;0 d-------- a-1------   4   ymax>=y3
    bge         .cmp_16                             ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- ymin < y0 && y2 <= ymax < y3 --------
                                                    ;0 d-------- a-1------       ======== [Ceil Floor gap Floor] ========
                                                    ;0 d-------- a-1------       -------- [Ceil --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    swap.w      d5                                  ;0 d-------- a--------   4   select ceiling color
    sub.w       d0, d7                              ;0 d-------- a--------   4   
    add.w       d7, d7                              ;0 d-------- a--------   4   
    lea.l       .ret_17, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d7.w)                           ;0 d-------- a--------  14   
.ret_17:                                            ;0 d-------- a-1------       
    swap.w      d5                                  ;0 d-------- a-1------   4   deselect ceiling color
                                                    ;0 d-------- a-1------       -------- Floor --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    move.w      d0, d7                              ;0 d-------- a--------   4   
    sub.w       d1, d7                              ;0 d-------- a--------   4   
    add.w       d7, d7                              ;0 d-------- a--------   4   
    lea.l       .ret_18, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d7.w)                           ;0 d-------- a--------  14   
.ret_18:                                            ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- gap --------
    move.w      d1, rc_ymin(a4)                     ;0 d-------- a-1------   8   Update rc_ymin with ystart
    move.l      a7, (a4)                            ;0 d-------- a-1------  12   Set future drawing start pointer to current pos
    move.w      d2, rc_ymax(a4)                     ;0 d-------- a-1------   8   Update rc_ymax with yend
    move.w      d2, d7                              ;0 d-------- a-1------   4   Add 2*(yend - ystart) to A7
    sub.w       d1, d7                              ;0 d-------- a-1------   4   
    add.w       d7, d7                              ;0 d-------- a-1------   4   
    add.w       d7, a7                              ;0 d-------- a-1------   8   
                                                    ;0 d-------- a-1------       -------- Floor] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    sub.w       d2, d6                              ;0 d-------- a--------   4   
    neg.w       d6                                  ;0 d-------- a--------   4   
    add.w       d6, d6                              ;0 d-------- a--------   4   
    lea.l       .ret_19, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d6.w)                           ;0 d-------- a--------  14   
.ret_19:                                            ;0 d-------- a-1------       
    bra         .end_loop                           ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
.cmp_16:                                            ;0 d-------- a-1------       -------- ymin < y0 && y3 <= ymax --------
                                                    ;0 d-------- a-1------       ======== [Ceil Floor gap Floor Ceil] ========
                                                    ;0 d-------- a-1------       -------- [Ceil --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    swap.w      d5                                  ;0 d-------- a--------   4   select ceiling color
    sub.w       d0, d7                              ;0 d-------- a--------   4   
    add.w       d7, d7                              ;0 d-------- a--------   4   
    lea.l       .ret_20, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d7.w)                           ;0 d-------- a--------  14   
.ret_20:                                            ;0 d-------- a-1------       
    swap.w      d5                                  ;0 d-------- a-1------   4   deselect ceiling color
                                                    ;0 d-------- a-1------       -------- Floor --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    move.w      d0, d7                              ;0 d-------- a--------   4   
    sub.w       d1, d7                              ;0 d-------- a--------   4   
    add.w       d7, d7                              ;0 d-------- a--------   4   
    lea.l       .ret_21, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d7.w)                           ;0 d-------- a--------  14   
.ret_21:                                            ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- gap --------
    move.w      d1, rc_ymin(a4)                     ;0 d-------- a-1------   8   Update rc_ymin with ystart
    move.l      a7, (a4)                            ;0 d-------- a-1------  12   Set future drawing start pointer to current pos
    move.w      d2, rc_ymax(a4)                     ;0 d-------- a-1------   8   Update rc_ymax with yend
    move.w      d2, d7                              ;0 d-------- a-1------   4   Add 2*(yend - ystart) to A7
    sub.w       d1, d7                              ;0 d-------- a-1------   4   
    add.w       d7, d7                              ;0 d-------- a-1------   4   
    add.w       d7, a7                              ;0 d-------- a-1------   8   
                                                    ;0 d-------- a-1------       -------- Floor --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    move.w      d2, d7                              ;0 d-------- a--------   4   
    sub.w       d3, d7                              ;0 d-------- a--------   4   
    add.w       d7, d7                              ;0 d-------- a--------   4   
    lea.l       .ret_22, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d7.w)                           ;0 d-------- a--------  14   
.ret_22:                                            ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- Ceil] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    swap.w      d5                                  ;0 d-------- a--------   4   select ceiling color
    sub.w       d3, d6                              ;0 d-------- a--------   4   
    neg.w       d6                                  ;0 d-------- a--------   4   
    add.w       d6, d6                              ;0 d-------- a--------   4   
    lea.l       .ret_23, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d6.w)                           ;0 d-------- a--------  14   
.ret_23:                                            ;0 d-------- a-1------       
    swap.w      d5                                  ;0 d-------- a-1------   4   deselect ceiling color
    bra         .end_loop                           ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
.cmp_15:                                            ;0 d-------- a-1------       -------- y0 <= ymin < y2 && y2 <= ymax --------
    cmp.w       d3, d6                              ;0 d-------- a-1------   4   ymax>=y3
    bge         .cmp_24                             ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- y0 <= ymin < y2 && y2 <= ymax < y3 --------
    cmp.w       d1, d7                              ;0 d-------- a-1------   4   ymin>=y1
    bge         .cmp_25                             ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- y0 <= ymin < y1 && y2 <= ymax < y3 --------
                                                    ;0 d-------- a-1------       ======== [Floor gap Floor] ========
                                                    ;0 d-------- a-1------       -------- [Floor --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    sub.w       d1, d7                              ;0 d-------- a--------   4   
    add.w       d7, d7                              ;0 d-------- a--------   4   
    lea.l       .ret_26, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d7.w)                           ;0 d-------- a--------  14   
.ret_26:                                            ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- gap --------
    move.w      d1, rc_ymin(a4)                     ;0 d-------- a-1------   8   Update rc_ymin with ystart
    move.l      a7, (a4)                            ;0 d-------- a-1------  12   Set future drawing start pointer to current pos
    move.w      d2, rc_ymax(a4)                     ;0 d-------- a-1------   8   Update rc_ymax with yend
    move.w      d2, d7                              ;0 d-------- a-1------   4   Add 2*(yend - ystart) to A7
    sub.w       d1, d7                              ;0 d-------- a-1------   4   
    add.w       d7, d7                              ;0 d-------- a-1------   4   
    add.w       d7, a7                              ;0 d-------- a-1------   8   
                                                    ;0 d-------- a-1------       -------- Floor] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    sub.w       d2, d6                              ;0 d-------- a--------   4   
    neg.w       d6                                  ;0 d-------- a--------   4   
    add.w       d6, d6                              ;0 d-------- a--------   4   
    lea.l       .ret_27, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d6.w)                           ;0 d-------- a--------  14   
.ret_27:                                            ;0 d-------- a-1------       
    bra         .end_loop                           ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
.cmp_25:                                            ;0 d-------- a-1------       -------- y1 <= ymin < y2 && y2 <= ymax < y3 --------
                                                    ;0 d-------- a-1------       ======== [gap Floor] ========
                                                    ;0 d-------- a-1------       -------- [gap --------
    move.w      d2, rc_ymax(a4)                     ;0 d-------- a-1------   8   Update rc_ymax with yend
    sub.w       d2, d7                              ;0 d-------- a-1------   4   Add 2*(yend - rc_ymin) to A7
    neg.w       d7                                  ;0 d-------- a-1------   4   
    add.w       d7, d7                              ;0 d-------- a-1------   4   
    add.w       d7, a7                              ;0 d-------- a-1------   8   
                                                    ;0 d-------- a-1------       -------- Floor] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    sub.w       d2, d6                              ;0 d-------- a--------   4   
    neg.w       d6                                  ;0 d-------- a--------   4   
    add.w       d6, d6                              ;0 d-------- a--------   4   
    lea.l       .ret_28, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d6.w)                           ;0 d-------- a--------  14   
.ret_28:                                            ;0 d-------- a-1------       
    bra         .end_loop                           ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
.cmp_24:                                            ;0 d-------- a-1------       -------- y0 <= ymin < y2 && y3 <= ymax --------
    cmp.w       d1, d7                              ;0 d-------- a-1------   4   ymin>=y1
    bge         .cmp_29                             ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- y0 <= ymin < y1 && y3 <= ymax --------
                                                    ;0 d-------- a-1------       ======== [Floor gap Floor Ceil] ========
                                                    ;0 d-------- a-1------       -------- [Floor --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    sub.w       d1, d7                              ;0 d-------- a--------   4   
    add.w       d7, d7                              ;0 d-------- a--------   4   
    lea.l       .ret_30, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d7.w)                           ;0 d-------- a--------  14   
.ret_30:                                            ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- gap --------
    move.w      d1, rc_ymin(a4)                     ;0 d-------- a-1------   8   Update rc_ymin with ystart
    move.l      a7, (a4)                            ;0 d-------- a-1------  12   Set future drawing start pointer to current pos
    move.w      d2, rc_ymax(a4)                     ;0 d-------- a-1------   8   Update rc_ymax with yend
    move.w      d2, d7                              ;0 d-------- a-1------   4   Add 2*(yend - ystart) to A7
    sub.w       d1, d7                              ;0 d-------- a-1------   4   
    add.w       d7, d7                              ;0 d-------- a-1------   4   
    add.w       d7, a7                              ;0 d-------- a-1------   8   
                                                    ;0 d-------- a-1------       -------- Floor --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    move.w      d2, d7                              ;0 d-------- a--------   4   
    sub.w       d3, d7                              ;0 d-------- a--------   4   
    add.w       d7, d7                              ;0 d-------- a--------   4   
    lea.l       .ret_31, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d7.w)                           ;0 d-------- a--------  14   
.ret_31:                                            ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- Ceil] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    swap.w      d5                                  ;0 d-------- a--------   4   select ceiling color
    sub.w       d3, d6                              ;0 d-------- a--------   4   
    neg.w       d6                                  ;0 d-------- a--------   4   
    add.w       d6, d6                              ;0 d-------- a--------   4   
    lea.l       .ret_32, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d6.w)                           ;0 d-------- a--------  14   
.ret_32:                                            ;0 d-------- a-1------       
    swap.w      d5                                  ;0 d-------- a-1------   4   deselect ceiling color
    bra         .end_loop                           ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
.cmp_29:                                            ;0 d-------- a-1------       -------- y1 <= ymin < y2 && y3 <= ymax --------
                                                    ;0 d-------- a-1------       ======== [gap Floor Ceil] ========
                                                    ;0 d-------- a-1------       -------- [gap --------
    move.w      d2, rc_ymax(a4)                     ;0 d-------- a-1------   8   Update rc_ymax with yend
    sub.w       d2, d7                              ;0 d-------- a-1------   4   Add 2*(yend - rc_ymin) to A7
    neg.w       d7                                  ;0 d-------- a-1------   4   
    add.w       d7, d7                              ;0 d-------- a-1------   4   
    add.w       d7, a7                              ;0 d-------- a-1------   8   
                                                    ;0 d-------- a-1------       -------- Floor --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    move.w      d2, d7                              ;0 d-------- a--------   4   
    sub.w       d3, d7                              ;0 d-------- a--------   4   
    add.w       d7, d7                              ;0 d-------- a--------   4   
    lea.l       .ret_33, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d7.w)                           ;0 d-------- a--------  14   
.ret_33:                                            ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- Ceil] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    swap.w      d5                                  ;0 d-------- a--------   4   select ceiling color
    sub.w       d3, d6                              ;0 d-------- a--------   4   
    neg.w       d6                                  ;0 d-------- a--------   4   
    add.w       d6, d6                              ;0 d-------- a--------   4   
    lea.l       .ret_34, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d6.w)                           ;0 d-------- a--------  14   
.ret_34:                                            ;0 d-------- a-1------       
    swap.w      d5                                  ;0 d-------- a-1------   4   deselect ceiling color
    bra         .end_loop                           ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
.cmp_14:                                            ;0 d-------- a-1------       -------- y2 <= ymin && y2 <= ymax --------
    cmp.w       d3, d6                              ;0 d-------- a-1------   4   ymax>=y3
    bge         .cmp_35                             ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- y2 <= ymin < y3 && y2 <= ymax < y3 --------
                                                    ;0 d-------- a-1------       ======== [Floor] ========
                                                    ;0 d-------- a-1------       -------- [Floor] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    sub.w       d6, d7                              ;0 d-------- a--------   4   
    add.w       d7, d7                              ;0 d-------- a--------   4   
    lea.l       .ret_36, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d7.w)                           ;0 d-------- a--------  14   
.ret_36:                                            ;0 d-------- a-1------       
    clr.l       rc_ymin(a4)                         ;0 d-------- a-1------       Disable further rendering - clear ymin/ymax
    bra         .end_loop                           ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
.cmp_35:                                            ;0 d-------- a-1------       -------- y2 <= ymin && y3 <= ymax --------
    cmp.w       d3, d7                              ;0 d-------- a-1------   4   ymin>=y3
    bge         .cmp_37                             ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- y2 <= ymin < y3 && y3 <= ymax --------
                                                    ;0 d-------- a-1------       ======== [Floor Ceil] ========
                                                    ;0 d-------- a-1------       -------- [Floor --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    sub.w       d3, d7                              ;0 d-------- a--------   4   
    add.w       d7, d7                              ;0 d-------- a--------   4   
    lea.l       .ret_38, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d7.w)                           ;0 d-------- a--------  14   
.ret_38:                                            ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       -------- Ceil] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    swap.w      d5                                  ;0 d-------- a--------   4   select ceiling color
    sub.w       d3, d6                              ;0 d-------- a--------   4   
    neg.w       d6                                  ;0 d-------- a--------   4   
    add.w       d6, d6                              ;0 d-------- a--------   4   
    lea.l       .ret_39, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d6.w)                           ;0 d-------- a--------  14   
.ret_39:                                            ;0 d-------- a-1------       
    swap.w      d5                                  ;0 d-------- a-1------   4   deselect ceiling color
    clr.l       rc_ymin(a4)                         ;0 d-------- a-1------       Disable further rendering - clear ymin/ymax
    bra         .end_loop                           ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
.cmp_37:                                            ;0 d-------- a-1------       -------- y3 <= ymin && y3 <= ymax --------
                                                    ;0 d-------- a-1------       ======== [Ceil] ========
                                                    ;0 d-------- a-1------       -------- [Ceil] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;0 d-------- a--------  12   Color filler func
    swap.w      d5                                  ;0 d-------- a--------   4   select ceiling color
    sub.w       d6, d7                              ;0 d-------- a--------   4   
    add.w       d7, d7                              ;0 d-------- a--------   4   
    lea.l       .ret_40, a6                         ;0 d-------- a--------  12   
    jmp         (a1,d7.w)                           ;0 d-------- a--------  14   
.ret_40:                                            ;0 d-------- a-1------       
    swap.w      d5                                  ;0 d-------- a-1------   4   deselect ceiling color
    clr.l       rc_ymin(a4)                         ;0 d-------- a-1------       Disable further rendering - clear ymin/ymax
    bra         .end_loop                           ;0 d-------- a-1------  10   
                                                    ;0 d-------- a-1------       
                                                    ;0 d-------- a-1------       
.end_loop:                                          ;0 d-------- a-1------       ======== LOOP END ========
    cmp.l       a0, a4                              ;0 d-------- a-1------   6   render_column ? render_end
    blt         .loop                               ;0 d-------- a-1------  10   continue if <
.end:                                               ;0 d012----- a01------       Cleanup & exit
    move.l      _a7_save(pc), a7                    ;0 d012----- a01------  16   
    rts                                             ;0 d012----- a01------  16   
