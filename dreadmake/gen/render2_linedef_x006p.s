
    public _Dread_LineCore_x006p
_Dread_LineCore_x006p:

                                                    ;s d01---567 a01234---       ================================ Ceil y1 gap ================================
    move.l      a7, _a7_save                        ;s d01---567 a01234---  20   
                                                    ;s d01---567 a01234---       
                                                    ;s d01---567 a01234---       ======== Loop initialization ========
    lea.l       _render_columns, a4                 ;s d01---567 a0123----  12   
    add.w       d2, a4                              ;s d01---567 a0123----   8   add xmin*8 to column pointer
                                                    ;s d012--567 a0123----       
                                                    ;s d012--567 a0123----       ======== init .trace_y0 tracer ========
    move.w      d4, d0                              ;s d-12--567 a0123----   4   compute starting Y
    lsr.w       #5, d0                              ;s d-12--567 a0123----  16   
    move.w      line_y1(a5), d1                     ;s d--2--567 a0123----  12   
    sub.w       _view_pos_z, d1                     ;s d--2--567 a0123----  16   
    muls.w      d1, d0                              ;s d--2--567 a0123----  70   
    move.w      _asm_line_s2, d5                    ;s d--2---67 a0123----  16   compute Y slope
    lsr.w       #5, d5                              ;s d--2---67 a0123----  16   
    muls.w      d1, d5                              ;s d--2---67 a0123----  70   
    sub.l       d0, d5                              ;s d-12---67 a0123----   8   
    divs.w      _asm_line_xlen, d5                  ;s d-12---67 a0123---- 170   
    move.b      d5, .trace_s+2                      ;s d-12---67 a0123----  16   subpixel step
    lsr.w       #8, d5                              ;s d-12---67 a0123----  22   
    ext.w       d5                                  ;s d-12---67 a0123----   4   
    move.w      d5, .trace_y0+2                     ;s d-12---67 a0123----  16   pixel step
    add.l       #ENGINE_Y_MID*256+128, d0           ;s d-12--567 a0123----  16   24.8 fixed
    lsl.l       #8, d0                              ;s d-12--567 a0123----  24   16.16 fixed
    swap.w      d4                                  ;s d-12--567 a0123----   4   
    move.w      d0, d4                              ;s d-12--567 a0123----   4   
    swap.w      d4                                  ;s d-12--567 a0123----   4   
    sub.w       d0, d0                              ;s d-12--567 a0123----   4   
    swap.w      d0                                  ;s d-12--567 a0123----   4   current Y value
    move.w      _asm_line_ds, .trace_s+4            ;s d-12--567 a0123----  28   
    move.b      line_ceil_col(a5), d5               ;s d-12---67 a0123----  12   
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a0  ;s d-12--567 a-123-5--  12   Color filler func
    TRAP_LINEDRAW_SETUP_DONE                        ;s d-12--567 a-123-5--       
    bra         .start                              ;s d-12--567 a-123-5--  12   End of header
                                                    ;s d-12--567 a-123-5--       
                                                    ;s d-12--567 a-123-5--       
.loop:                                              ;I d-12--567 a-123-5--       ======== LOOP START ========
.trace_s:                                           ;I d-12--567 a-123-5--       
    add.l       #$FF00FFFF, d4                      ;I d-12--567 a-123-5--  16   s1 += ds
.trace_y0:                                          ;I d-12--567 a-123-5--       
    move.w      #$FF00, d1                          ;I d--2--567 a-123-5--   8   
    addx.w      d1, d0                              ;I d--2--567 a-123-5--   4   
                                                    ;- d-12--567 a-123-5--       
.start:                                             ;- d-12--567 a-123-5--       ======== FIRST ITERATION STARTS HERE ========
    move.l      (a4)+, a7                           ;- d-12--567 a-123-5--  12   byte *dst = render_buffer + offs[x];
                                                    ;C d-12--567 a-123-5--       
                                                    ;C d-12--567 a-123-5--       -------- ymin/ymax not checked --------
    move.l      (a4)+, d1                           ;C d--2--567 a-123-5--  12   Load ymin/ymax
    cmp.w       d0, d1                              ;C d--2--567 a-123-5--   4   ymax>=y0
    bge         .cmp_1                              ;C d--2--567 a-123-5--  12   
                                                    ;C d--2--567 a-123-5--       
                                                    ;C d--2--567 a-123-5--       -------- ymin < y0 && ymax < y0 --------
                                                    ;- d--2--567 a-123-5--       ======== [Ceil] ========
                                                    ;F d--2--567 a-123-5--       -------- [Ceil] --------
    swap.w      d1                                  ;F d--2--567 a-123-5--   4   Select ymin
    move.w      d1, d2                              ;F d-----567 a-123-5--   4   
    swap.w      d1                                  ;F d-----567 a-123-5--   4   Select ymax
    sub.w       d1, d2                              ;F d-----567 a-123-5--   4   
    add.w       d2, d2                              ;F d-1---567 a-123-5--   4   
    lea.l       .ret_2(pc), a6                      ;F d-1---567 a-123-5--   8   
    jmp         (a0,d2.w)                           ;F d-1---567 a-123-5--  14   -> .ret_2
.ret_2:                                             ;F d-12--567 a-123-5--       
    moveq.l     #0, d1                              ;G d--2--567 a-123-5--   4   Disable further rendering - clear ymin/ymax
    move.l      d1, rc_ymin-8(a4)                   ;G d--2--567 a-123-5--  16   
    move.w      d4, rc_size_limit-8(a4)             ;G d-12--567 a-123-5--  12   
    bra         .end_loop                           ;- d-12--567 a-123-5--  12   
                                                    ;C d--2--567 a-123-5--       
.cmp_1:                                             ;C d--2--567 a-123-5--       -------- y0 <= ymax --------
    swap.w      d1                                  ;C d--2--567 a-123-5--   4   Select ymin
    cmp.w       d0, d1                              ;C d--2--567 a-123-5--   4   ymin>=y0
    bge         .cmp_3                              ;C d--2--567 a-123-5--  12   
                                                    ;C d--2--567 a-123-5--       
                                                    ;C d--2--567 a-123-5--       -------- ymin < y0 && y0 <= ymax --------
                                                    ;- d--2--567 a-123-5--       ======== [Ceil gap] ========
                                                    ;F d--2--567 a-123-5--       -------- [Ceil --------
    sub.w       d0, d1                              ;F d--2--567 a-123-5--   4   
    add.w       d1, d1                              ;F d--2--567 a-123-5--   4   
    lea.l       .ret_4(pc), a6                      ;F d--2--567 a-123-5--   8   
    jmp         (a0,d1.w)                           ;F d--2--567 a-123-5--  14   -> .ret_4
.ret_4:                                             ;F d--2--567 a-123-5--       
                                                    ;G d--2--567 a-123-5--       -------- gap] --------
    swap.w      d1                                  ;G d--2--567 a-123-5--   4   Select ymax
    move.w      d0, rc_ymin-8(a4)                   ;G d-12--567 a-123-5--  12   Update rc_ymin with ystart
    move.l      a7, 0-8(a4)                         ;G d-12--567 a-123-5--  16   Set future drawing start pointer to current pos
    bra         .end_loop                           ;- d-12--567 a-123-5--  12   
                                                    ;C d-12--567 a-123-5--       
.cmp_3:                                             ;C d-12--567 a-123-5--       -------- y0 <= ymin && y0 <= ymax --------
                                                    ;- d-12--567 a-123-5--       ======== [gap] ========
                                                    ;G d-12--567 a-123-5--       -------- [gap] --------
                                                    ;G d-12--567 a-123-5--       nothing to be done
    bra         .end_loop                           ;- d-12--567 a-123-5--  12   
                                                    ;- d-12--567 a-123-5--       
                                                    ;- d-12--567 a-123-5--       
.end_loop:                                          ;- d-12--567 a-123-5--       ======== LOOP END ========
    dbra.w      d3, .loop                           ;- d-12--567 a-123-5--  12   
.end:                                               ;o d01234567 a012345--       Cleanup & exit
    move.l      _a7_save, a7                        ;o d01234567 a012345--  20   
    rts                                             ;o d01234567 a012345--  16   
