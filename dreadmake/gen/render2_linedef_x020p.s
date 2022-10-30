
    public _Dread_LineCore_x020p
_Dread_LineCore_x020p:

                                                    ;0 d01---567 a01234---       ================================ Sky y1 gap y3 Floor ================================
    move.l      a7, _a7_save                        ;0 d01---567 a01234---  20   
                                                    ;0 d01---567 a01234---       
                                                    ;0 d01---567 a01234---       ======== Loop initialization ========
    lea.l       _render_columns, a4                 ;0 d01---567 a0123----  12   
    add.w       d2, a4                              ;0 d01---567 a0123----   8   add xmin*8 to column pointer
    lsr.w       #1, d2                              ;0 d01---567 a0123----   8   init SKY
    move.l      _e_skyptr, a0                       ;0 d01---567 a-123----  20   
    add.w       d2, a0                              ;0 d01---567 a-123----   8   
    move.l      a4, a1                              ;0 d012--567 a--23----   4   end = start + count*8
    lsl.w       #3, d3                              ;0 d012--567 a--23----  12   
    add.w       d3, a1                              ;0 d012--567 a--23----   8   
                                                    ;0 d0123-567 a--23----       
                                                    ;0 d0123-567 a--23----       ======== init .trace_y0 tracer ========
    move.w      d4, d0                              ;0 d-123-567 a--23----   4   compute starting Y
    lsr.w       #5, d0                              ;0 d-123-567 a--23----  16   
    move.w      line_y1(a5), d1                     ;0 d--23-567 a--23----  12   
    sub.w       _view_pos_z, d1                     ;0 d--23-567 a--23----  16   
    muls.w      d1, d0                              ;0 d--23-567 a--23----  70   
    move.w      _asm_line_s2, d5                    ;0 d--23--67 a--23----  16   compute Y slope
    lsr.w       #5, d5                              ;0 d--23--67 a--23----  16   
    muls.w      d1, d5                              ;0 d--23--67 a--23----  70   
    sub.l       d0, d5                              ;0 d-123--67 a--23----   8   
    divs.w      _asm_line_xlen, d5                  ;0 d-123--67 a--23---- 170   
    move.b      d5, .trace_s+2                      ;0 d-123--67 a--23----  16   subpixel step
    lsr.w       #8, d5                              ;0 d-123--67 a--23----  22   
    ext.w       d5                                  ;0 d-123--67 a--23----   4   
    move.w      d5, .trace_y0+4                     ;0 d-123--67 a--23----  16   pixel step
    add.l       #ENGINE_Y_MID*256+128, d0           ;0 d-123-567 a--23----  16   24.8 fixed
    lsl.l       #8, d0                              ;0 d-123-567 a--23----  24   16.16 fixed
    swap.w      d4                                  ;0 d-123-567 a--23----   4   
    move.w      d0, d4                              ;0 d-123-567 a--23----   4   
    swap.w      d4                                  ;0 d-123-567 a--23----   4   
    sub.w       d0, d0                              ;0 d-123-567 a--23----   4   
    swap.w      d0                                  ;0 d-123-567 a--23----   4   current Y value
                                                    ;0 d-123-567 a--23----       
                                                    ;0 d-123-567 a--23----       ======== init .trace_y1 tracer ========
    move.w      d4, d1                              ;0 d--23-567 a--23----   4   compute starting Y
    lsr.w       #5, d1                              ;0 d--23-567 a--23----  16   
    move.w      line_y3(a5), d2                     ;0 d---3-567 a--23----  12   
    sub.w       _view_pos_z, d2                     ;0 d---3-567 a--23----  16   
    muls.w      d2, d1                              ;0 d---3-567 a--23----  70   
    move.w      _asm_line_s2, d5                    ;0 d---3--67 a--23----  16   compute Y slope
    lsr.w       #5, d5                              ;0 d---3--67 a--23----  16   
    muls.w      d2, d5                              ;0 d---3--67 a--23----  70   
    sub.l       d1, d5                              ;0 d--23--67 a--23----   8   
    divs.w      _asm_line_xlen, d5                  ;0 d--23--67 a--23---- 170   
    move.b      d5, .trace_y0+2                     ;0 d--23--67 a--23----  16   subpixel step
    lsr.w       #8, d5                              ;0 d--23--67 a--23----  22   
    ext.w       d5                                  ;0 d--23--67 a--23----   4   
    move.w      d5, .trace_y1+4                     ;0 d--23--67 a--23----  16   pixel step
    add.l       #ENGINE_Y_MID*256+128, d1           ;0 d--23-567 a--23----  16   24.8 fixed
    lsl.l       #8, d1                              ;0 d--23-567 a--23----  24   16.16 fixed
    swap.w      d0                                  ;0 d--23-567 a--23----   4   
    move.w      d1, d0                              ;0 d--23-567 a--23----   4   
    swap.w      d0                                  ;0 d--23-567 a--23----   4   
    sub.w       d1, d1                              ;0 d--23-567 a--23----   4   
    swap.w      d1                                  ;0 d--23-567 a--23----   4   current Y value
    move.w      _asm_line_ds, .trace_s+4            ;0 d--23-567 a--23----  28   
    bra         .start                              ;0 d--23-567 a--23----  10   End of header
                                                    ;0 d--23-567 a--23----       
                                                    ;0 d--23-567 a--23----       
.loop:                                              ;1 d--23-567 a--23----       ======== LOOP START ========
    addq.w      #8, a4                              ;1 d--23-567 a--23----   8   
.trace_s:                                           ;1 d--23-567 a--23----       
    add.l       #$FF00FFFF, d4                      ;1 d--23-567 a--23----  16   s1 += ds
.trace_y0:                                          ;1 d--23-567 a--23----       
    move.l      #$FF000000, d2                      ;1 d---3-567 a--23----  12   
    addx.l      d2, d0                              ;1 d---3-567 a--23----   8   
.trace_y1:                                          ;1 d--23-567 a--23----       
    move.l      #$FF000000, d2                      ;1 d---3-567 a--23----  12   
    addx.l      d2, d1                              ;1 d---3-567 a--23----   8   
                                                    ;1 d--23-567 a--23----       
.start:                                             ;1 d--23-567 a--23----       ======== FIRST ITERATION STARTS HERE ========
    move.l      (a4), a7                            ;1 d--23-567 a--23----  12   byte *dst = render_buffer + offs[x];
                                                    ;1 d--23-567 a--23----       
                                                    ;1 d--23-567 a--23----       -------- ymin/ymax not checked --------
    move.w      rc_ymax(a4), d2                     ;1 d---3-567 a--23----  12   Load ymax
    cmp.w       d1, d2                              ;1 d---3-567 a--23----   4   ymax>=y1
    bge         .cmp_1                              ;1 d---3-567 a--23----  10   
                                                    ;1 d---3-567 a--23----       
                                                    ;1 d---3-567 a--23----       -------- ymin < y1 && ymax < y1 --------
    cmp.w       d0, d2                              ;1 d---3-567 a--23----   4   ymax>=y0
    bge         .cmp_2                              ;1 d---3-567 a--23----  10   
                                                    ;1 d---3-567 a--23----       
                                                    ;1 d---3-567 a--23----       -------- ymin < y0 && ymax < y0 --------
                                                    ;1 d---3-567 a--23----       ======== [Sky] ========
                                                    ;1 d---3-567 a--23----       -------- [Sky] --------
    lea.l       _FUNC_SKYCOPY+ENGINE_Y_MAX*2, a3    ;1 d---3-567 a--2-----  12   Sky copy func
    move.l      (a0)+, a2                           ;1 d---3-567 a--------  12   
    swap.w      d2                                  ;1 d---3-567 a--------   4   Select ymin
    move.w      rc_ymin(a4), d2                     ;1 d---3-567 a--------  12   Load ymin
    add.w       d2, a2                              ;1 d---3-567 a--------   8   sky offset by ymin
    move.w      d2, d3                              ;1 d-----567 a--2-----   4   
    swap.w      d2                                  ;1 d-----567 a--2-----   4   Select ymax
    sub.w       d2, d3                              ;1 d-----567 a--2-----   4   
    add.w       d3, d3                              ;1 d--2--567 a--2-----   4   
    lea.l       .ret_3, a6                          ;1 d--2--567 a--2-----  12   
    jmp         (a3,d3.w)                           ;1 d--2--567 a--2-----  14   
.ret_3:                                             ;1 d--23-567 a--23----       
    clr.l       rc_ymin(a4)                         ;1 d--23-567 a--23----       Disable further rendering - clear ymin/ymax
    move.w      d4, rc_size_limit(a4)               ;1 d--23-567 a--23----   8   
    bra         .end_loop                           ;1 d--23-567 a--23----  10   
                                                    ;1 d---3-567 a--23----       
.cmp_2:                                             ;1 d---3-567 a--23----       -------- ymin < y1 && y0 <= ymax < y1 --------
    swap.w      d2                                  ;1 d---3-567 a--23----   4   Select ymin
    move.w      rc_ymin(a4), d2                     ;1 d---3-567 a--23----  12   Load ymin
    cmp.w       d0, d2                              ;1 d---3-567 a--23----   4   ymin>=y0
    bge         .cmp_4                              ;1 d---3-567 a--23----  10   
                                                    ;1 d---3-567 a--23----       
                                                    ;1 d---3-567 a--23----       -------- ymin < y0 && y0 <= ymax < y1 --------
                                                    ;1 d---3-567 a--23----       ======== [Sky gap] ========
                                                    ;1 d---3-567 a--23----       -------- [Sky --------
    lea.l       _FUNC_SKYCOPY+ENGINE_Y_MAX*2, a3    ;1 d---3-567 a--2-----  12   Sky copy func
    move.l      (a0)+, a2                           ;1 d---3-567 a--------  12   
    add.w       d2, a2                              ;1 d---3-567 a--------   8   sky offset by ymin
    sub.w       d0, d2                              ;1 d---3-567 a--2-----   4   
    add.w       d2, d2                              ;1 d---3-567 a--2-----   4   
    lea.l       .ret_5, a6                          ;1 d---3-567 a--2-----  12   
    jmp         (a3,d2.w)                           ;1 d---3-567 a--2-----  14   
.ret_5:                                             ;1 d---3-567 a--23----       
                                                    ;1 d---3-567 a--23----       -------- gap] --------
    swap.w      d2                                  ;1 d---3-567 a--23----   4   Select ymax
    move.w      d0, rc_ymin(a4)                     ;1 d--23-567 a--23----   8   Update rc_ymin with ystart
    move.l      a7, (a4)                            ;1 d--23-567 a--23----  12   Set future drawing start pointer to current pos
    bra         .end_loop                           ;1 d--23-567 a--23----  10   
                                                    ;1 d--23-567 a--23----       
.cmp_4:                                             ;1 d--23-567 a--23----       -------- y0 <= ymin < y1 && y0 <= ymax < y1 --------
                                                    ;1 d--23-567 a--23----       ======== [gap] ========
                                                    ;1 d--23-567 a--23----       -------- [gap] --------
                                                    ;1 d--23-567 a--23----       nothing to be done
    addq.l      #4, a0                              ;1 d--23-567 a--23----   8   SKY was skipped
    bra         .end_loop                           ;1 d--23-567 a--23----  10   
                                                    ;1 d---3-567 a--23----       
.cmp_1:                                             ;1 d---3-567 a--23----       -------- y1 <= ymax --------
    swap.w      d2                                  ;1 d---3-567 a--23----   4   Select ymin
    move.w      rc_ymin(a4), d2                     ;1 d---3-567 a--23----  12   Load ymin
    cmp.w       d0, d2                              ;1 d---3-567 a--23----   4   ymin>=y0
    bge         .cmp_6                              ;1 d---3-567 a--23----  10   
                                                    ;1 d---3-567 a--23----       
                                                    ;1 d---3-567 a--23----       -------- ymin < y0 && y1 <= ymax --------
                                                    ;1 d---3-567 a--23----       ======== [Sky gap Floor] ========
                                                    ;1 d---3-567 a--23----       -------- [Sky --------
    lea.l       _FUNC_SKYCOPY+ENGINE_Y_MAX*2, a3    ;1 d---3-567 a--2-----  12   Sky copy func
    move.l      (a0)+, a2                           ;1 d---3-567 a--------  12   
    add.w       d2, a2                              ;1 d---3-567 a--------   8   sky offset by ymin
    sub.w       d0, d2                              ;1 d---3-567 a--2-----   4   
    add.w       d2, d2                              ;1 d---3-567 a--2-----   4   
    lea.l       .ret_7, a6                          ;1 d---3-567 a--2-----  12   
    jmp         (a3,d2.w)                           ;1 d---3-567 a--2-----  14   
.ret_7:                                             ;1 d---3-567 a--23----       
                                                    ;1 d---3-567 a--23----       -------- gap --------
    move.w      d0, rc_ymin(a4)                     ;1 d---3-567 a--23----   8   Update rc_ymin with ystart
    move.l      a7, (a4)                            ;1 d---3-567 a--23----  12   Set future drawing start pointer to current pos
    move.w      d1, rc_ymax(a4)                     ;1 d---3-567 a--23----   8   Update rc_ymax with yend
    move.w      d1, d3                              ;1 d-----567 a--23----   4   Add 2*(yend - ystart) to A7
    sub.w       d0, d3                              ;1 d-----567 a--23----   4   
    add.w       d3, d3                              ;1 d-----567 a--23----   4   
    add.w       d3, a7                              ;1 d-----567 a--23----   8   
                                                    ;1 d---3-567 a--23----       -------- Floor] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a3  ;1 d---3-567 a--2-----  12   Color filler func
    swap.w      d2                                  ;1 d---3-567 a--2-----   4   Select ymax
    sub.w       d1, d2                              ;1 d---3-567 a--2-----   4   
    neg.w       d2                                  ;1 d---3-567 a--2-----   4   
    add.w       d2, d2                              ;1 d---3-567 a--2-----   4   
    move.b      line_floor_col(a5), d5              ;1 d---3--67 a--2-----  12   
    lea.l       .ret_8, a6                          ;1 d---3-567 a--2-----  12   
    jmp         (a3,d2.w)                           ;1 d---3-567 a--2-----  14   
.ret_8:                                             ;1 d--23-567 a--23----       
    bra         .end_loop                           ;1 d--23-567 a--23----  10   
                                                    ;1 d---3-567 a--23----       
.cmp_6:                                             ;1 d---3-567 a--23----       -------- y0 <= ymin && y1 <= ymax --------
    cmp.w       d1, d2                              ;1 d---3-567 a--23----   4   ymin>=y1
    bge         .cmp_9                              ;1 d---3-567 a--23----  10   
                                                    ;1 d---3-567 a--23----       
                                                    ;1 d---3-567 a--23----       -------- y0 <= ymin < y1 && y1 <= ymax --------
                                                    ;1 d---3-567 a--23----       ======== [gap Floor] ========
                                                    ;1 d---3-567 a--23----       -------- [gap --------
    move.w      d1, rc_ymax(a4)                     ;1 d---3-567 a--23----   8   Update rc_ymax with yend
    sub.w       d1, d2                              ;1 d---3-567 a--23----   4   Add 2*(yend - rc_ymin) to A7
    neg.w       d2                                  ;1 d---3-567 a--23----   4   
    add.w       d2, d2                              ;1 d---3-567 a--23----   4   
    add.w       d2, a7                              ;1 d---3-567 a--23----   8   
                                                    ;1 d---3-567 a--23----       -------- Floor] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a3  ;1 d---3-567 a--2-----  12   Color filler func
    swap.w      d2                                  ;1 d---3-567 a--2-----   4   Select ymax
    sub.w       d1, d2                              ;1 d---3-567 a--2-----   4   
    neg.w       d2                                  ;1 d---3-567 a--2-----   4   
    add.w       d2, d2                              ;1 d---3-567 a--2-----   4   
    move.b      line_floor_col(a5), d5              ;1 d---3--67 a--2-----  12   
    lea.l       .ret_10, a6                         ;1 d---3-567 a--2-----  12   
    jmp         (a3,d2.w)                           ;1 d---3-567 a--2-----  14   
.ret_10:                                            ;1 d--23-567 a--23----       
    addq.l      #4, a0                              ;1 d--23-567 a--23----   8   SKY was skipped
    bra         .end_loop                           ;1 d--23-567 a--23----  10   
                                                    ;1 d---3-567 a--23----       
.cmp_9:                                             ;1 d---3-567 a--23----       -------- y1 <= ymin && y1 <= ymax --------
                                                    ;1 d---3-567 a--23----       ======== [Floor] ========
                                                    ;1 d---3-567 a--23----       -------- [Floor] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a3  ;1 d---3-567 a--2-----  12   Color filler func
    move.w      d2, d3                              ;1 d-----567 a--2-----   4   
    swap.w      d2                                  ;1 d-----567 a--2-----   4   Select ymax
    sub.w       d2, d3                              ;1 d-----567 a--2-----   4   
    add.w       d3, d3                              ;1 d--2--567 a--2-----   4   
    move.b      line_floor_col(a5), d5              ;1 d--2---67 a--2-----  12   
    lea.l       .ret_11, a6                         ;1 d--2--567 a--2-----  12   
    jmp         (a3,d3.w)                           ;1 d--2--567 a--2-----  14   
.ret_11:                                            ;1 d--23-567 a--23----       
    addq.l      #4, a0                              ;1 d--23-567 a--23----   8   SKY was skipped
    clr.l       rc_ymin(a4)                         ;1 d--23-567 a--23----       Disable further rendering - clear ymin/ymax
    move.w      d4, rc_size_limit(a4)               ;1 d--23-567 a--23----   8   
    bra         .end_loop                           ;1 d--23-567 a--23----  10   
                                                    ;1 d--23-567 a--23----       
                                                    ;1 d--23-567 a--23----       
.end_loop:                                          ;1 d--23-567 a--23----       ======== LOOP END ========
    cmp.l       a1, a4                              ;1 d--23-567 a--23----   6   render_column ? render_end
    blt         .loop                               ;1 d--23-567 a--23----  10   continue if <
.end:                                               ;1 d01234567 a012345--       Cleanup & exit
    move.l      _a7_save, a7                        ;1 d01234567 a012345--  20   
    rts                                             ;1 d01234567 a012345--  16   
