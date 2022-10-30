
    public _Dread_LineCore_x021p
_Dread_LineCore_x021p:

                                                    ;0 d01---5-- a01234---       ================================ Sky y1 gap y3 Lower ================================
    move.l      a7, _a7_save                        ;0 d01---5-- a01234---  20   
                                                    ;0 d01---5-- a01234---       
                                                    ;0 d01---5-- a01234---       ======== Loop initialization ========
    lea.l       _render_columns, a4                 ;0 d01---5-- a0123----  12   
    add.w       d2, a4                              ;0 d01---5-- a0123----   8   add xmin*8 to column pointer
    lsr.w       #1, d2                              ;0 d01---5-- a0123----   8   init SKY
    move.l      _e_skyptr, a0                       ;0 d01---5-- a-123----  20   
    add.w       d2, a0                              ;0 d01---5-- a-123----   8   
    move.l      d7, .trace_tex+2                    ;0 d012--5-- a-123----  20   
    move.l      a4, a1                              ;0 d012--5-7 a--23----   4   end = start + count*8
    lsl.w       #3, d3                              ;0 d012--5-7 a--23----  12   
    add.w       d3, a1                              ;0 d012--5-7 a--23----   8   
    move.l      line_tex_lower(a5), a3              ;0 d0123-5-7 a--2-----  16   Lower texture
    move.w      _view_pos_z, d0                     ;0 d-123-5-7 a--2-----  16   
    asr.w       #1, d0                              ;0 d-123-5-7 a--2-----   8   
    add.w       d0, a3                              ;0 d-123-5-7 a--2-----   8   
    move.l      a3, .asm_tex_base2                  ;0 d0123-5-7 a--2-----  20   
                                                    ;0 d0123-5-7 a--23----       
                                                    ;0 d0123-5-7 a--23----       ======== init .trace_y0 tracer ========
    move.w      d4, d0                              ;0 d-123-5-7 a--23----   4   compute starting Y
    lsr.w       #5, d0                              ;0 d-123-5-7 a--23----  16   
    move.w      line_y1(a5), d1                     ;0 d--23-5-7 a--23----  12   
    sub.w       _view_pos_z, d1                     ;0 d--23-5-7 a--23----  16   
    muls.w      d1, d0                              ;0 d--23-5-7 a--23----  70   
    move.w      _asm_line_s2, d5                    ;0 d--23---7 a--23----  16   compute Y slope
    lsr.w       #5, d5                              ;0 d--23---7 a--23----  16   
    muls.w      d1, d5                              ;0 d--23---7 a--23----  70   
    sub.l       d0, d5                              ;0 d-123---7 a--23----   8   
    divs.w      _asm_line_xlen, d5                  ;0 d-123---7 a--23---- 170   
    move.b      d5, .trace_s+2                      ;0 d-123---7 a--23----  16   subpixel step
    lsr.w       #8, d5                              ;0 d-123---7 a--23----  22   
    ext.w       d5                                  ;0 d-123---7 a--23----   4   
    move.w      d5, .trace_y0+4                     ;0 d-123---7 a--23----  16   pixel step
    add.l       #ENGINE_Y_MID*256+128, d0           ;0 d-123-5-7 a--23----  16   24.8 fixed
    lsl.l       #8, d0                              ;0 d-123-5-7 a--23----  24   16.16 fixed
    swap.w      d4                                  ;0 d-123-5-7 a--23----   4   
    move.w      d0, d4                              ;0 d-123-5-7 a--23----   4   
    swap.w      d4                                  ;0 d-123-5-7 a--23----   4   
    sub.w       d0, d0                              ;0 d-123-5-7 a--23----   4   
    swap.w      d0                                  ;0 d-123-5-7 a--23----   4   current Y value
                                                    ;0 d-123-5-7 a--23----       
                                                    ;0 d-123-5-7 a--23----       ======== init .trace_y1 tracer ========
    move.w      d4, d1                              ;0 d--23-5-7 a--23----   4   compute starting Y
    lsr.w       #5, d1                              ;0 d--23-5-7 a--23----  16   
    move.w      line_y3(a5), d2                     ;0 d---3-5-7 a--23----  12   
    sub.w       _view_pos_z, d2                     ;0 d---3-5-7 a--23-5--  16   
    muls.w      d2, d1                              ;0 d---3-5-7 a--23-5--  70   
    move.w      _asm_line_s2, d5                    ;0 d---3---7 a--23-5--  16   compute Y slope
    lsr.w       #5, d5                              ;0 d---3---7 a--23-5--  16   
    muls.w      d2, d5                              ;0 d---3---7 a--23-5--  70   
    sub.l       d1, d5                              ;0 d--23---7 a--23-5--   8   
    divs.w      _asm_line_xlen, d5                  ;0 d--23---7 a--23-5-- 170   
    move.b      d5, .trace_y0+2                     ;0 d--23---7 a--23-5--  16   subpixel step
    lsr.w       #8, d5                              ;0 d--23---7 a--23-5--  22   
    ext.w       d5                                  ;0 d--23---7 a--23-5--   4   
    move.w      d5, .trace_y1+4                     ;0 d--23---7 a--23-5--  16   pixel step
    add.l       #ENGINE_Y_MID*256+128, d1           ;0 d--23-5-7 a--23-5--  16   24.8 fixed
    lsl.l       #8, d1                              ;0 d--23-5-7 a--23-5--  24   16.16 fixed
    swap.w      d0                                  ;0 d--23-5-7 a--23-5--   4   
    move.w      d1, d0                              ;0 d--23-5-7 a--23-5--   4   
    swap.w      d0                                  ;0 d--23-5-7 a--23-5--   4   
    sub.w       d1, d1                              ;0 d--23-5-7 a--23-5--   4   
    swap.w      d1                                  ;0 d--23-5-7 a--23-5--   4   current Y value
    move.w      _asm_line_ds, .trace_s+4            ;0 d--23-5-7 a--23-5--  28   
    bra         .start                              ;0 d--23-5-7 a--23-5--  10   End of header
                                                    ;0 d--23-5-7 a--23-5--       
                                                    ;0 d--23-5-7 a--23-5--       
.loop:                                              ;1 d--23-5-7 a--23-5--       ======== LOOP START ========
    addq.w      #8, a4                              ;1 d--23-5-7 a--23-5--   8   
.trace_s:                                           ;1 d--23-5-7 a--23-5--       
    add.l       #$FF00FFFF, d4                      ;1 d--23-5-7 a--23-5--  16   s1 += ds
.trace_y0:                                          ;1 d--23-5-7 a--23-5--       
    move.l      #$FF000000, d2                      ;1 d---3-5-7 a--23-5--  12   
    addx.l      d2, d0                              ;1 d---3-5-7 a--23-5--   8   
.trace_y1:                                          ;1 d--23-5-7 a--23-5--       
    move.l      #$FF000000, d2                      ;1 d---3-5-7 a--23-5--  12   
    addx.l      d2, d1                              ;1 d---3-5-7 a--23-5--   8   
.trace_tex:                                         ;1 d--23-5-7 a--23-5--       
    add.l       #$80000000, d6                      ;1 d--23-5-7 a--23-5--  16   u += du
                                                    ;1 d--23-5-7 a--23-5--       
.start:                                             ;1 d--23-5-7 a--23-5--       ======== FIRST ITERATION STARTS HERE ========
    move.l      (a4), a7                            ;1 d--23-5-7 a--23-5--  12   byte *dst = render_buffer + offs[x];
                                                    ;1 d--23-5-7 a--23-5--       
                                                    ;1 d--23-5-7 a--23-5--       -------- ymin/ymax not checked --------
    move.w      rc_ymax(a4), d2                     ;1 d---3-5-7 a--23-5--  12   Load ymax
    cmp.w       d1, d2                              ;1 d---3-5-7 a--23-5--   4   ymax>=y1
    bge         .cmp_1                              ;1 d---3-5-7 a--23-5--  10   
                                                    ;1 d---3-5-7 a--23-5--       
                                                    ;1 d---3-5-7 a--23-5--       -------- ymin < y1 && ymax < y1 --------
    cmp.w       d0, d2                              ;1 d---3-5-7 a--23-5--   4   ymax>=y0
    bge         .cmp_2                              ;1 d---3-5-7 a--23-5--  10   
                                                    ;1 d---3-5-7 a--23-5--       
                                                    ;1 d---3-5-7 a--23-5--       -------- ymin < y0 && ymax < y0 --------
                                                    ;1 d---3-5-7 a--23-5--       ======== [Sky] ========
                                                    ;1 d---3-5-7 a--23-5--       -------- [Sky] --------
    lea.l       _FUNC_SKYCOPY+ENGINE_Y_MAX*2, a3    ;1 d---3-5-7 a--2--5--  12   Sky copy func
    move.l      (a0)+, a2                           ;1 d---3-5-7 a-----5--  12   
    swap.w      d2                                  ;1 d---3-5-7 a-----5--   4   Select ymin
    move.w      rc_ymin(a4), d2                     ;1 d---3-5-7 a-----5--  12   Load ymin
    add.w       d2, a2                              ;1 d---3-5-7 a-----5--   8   sky offset by ymin
    move.w      d2, d3                              ;1 d-----5-7 a--2--5--   4   
    swap.w      d2                                  ;1 d-----5-7 a--2--5--   4   Select ymax
    sub.w       d2, d3                              ;1 d-----5-7 a--2--5--   4   
    add.w       d3, d3                              ;1 d--2--5-7 a--2--5--   4   
    lea.l       .ret_3, a6                          ;1 d--2--5-7 a--2--5--  12   
    jmp         (a3,d3.w)                           ;1 d--2--5-7 a--2--5--  14   
.ret_3:                                             ;1 d--23-5-7 a--23-5--       
    clr.l       rc_ymin(a4)                         ;1 d--23-5-7 a--23-5--       Disable further rendering - clear ymin/ymax
    move.w      d4, rc_size_limit(a4)               ;1 d--23-5-7 a--23-5--   8   
    bra         .end_loop                           ;1 d--23-5-7 a--23-5--  10   
                                                    ;1 d---3-5-7 a--23-5--       
.cmp_2:                                             ;1 d---3-5-7 a--23-5--       -------- ymin < y1 && y0 <= ymax < y1 --------
    swap.w      d2                                  ;1 d---3-5-7 a--23-5--   4   Select ymin
    move.w      rc_ymin(a4), d2                     ;1 d---3-5-7 a--23-5--  12   Load ymin
    cmp.w       d0, d2                              ;1 d---3-5-7 a--23-5--   4   ymin>=y0
    bge         .cmp_4                              ;1 d---3-5-7 a--23-5--  10   
                                                    ;1 d---3-5-7 a--23-5--       
                                                    ;1 d---3-5-7 a--23-5--       -------- ymin < y0 && y0 <= ymax < y1 --------
                                                    ;1 d---3-5-7 a--23-5--       ======== [Sky gap] ========
                                                    ;1 d---3-5-7 a--23-5--       -------- [Sky --------
    lea.l       _FUNC_SKYCOPY+ENGINE_Y_MAX*2, a3    ;1 d---3-5-7 a--2--5--  12   Sky copy func
    move.l      (a0)+, a2                           ;1 d---3-5-7 a-----5--  12   
    add.w       d2, a2                              ;1 d---3-5-7 a-----5--   8   sky offset by ymin
    sub.w       d0, d2                              ;1 d---3-5-7 a--2--5--   4   
    add.w       d2, d2                              ;1 d---3-5-7 a--2--5--   4   
    lea.l       .ret_5, a6                          ;1 d---3-5-7 a--2--5--  12   
    jmp         (a3,d2.w)                           ;1 d---3-5-7 a--2--5--  14   
.ret_5:                                             ;1 d---3-5-7 a--23-5--       
                                                    ;1 d---3-5-7 a--23-5--       -------- gap] --------
    swap.w      d2                                  ;1 d---3-5-7 a--23-5--   4   Select ymax
    move.w      d0, rc_ymin(a4)                     ;1 d--23-5-7 a--23-5--   8   Update rc_ymin with ystart
    move.l      a7, (a4)                            ;1 d--23-5-7 a--23-5--  12   Set future drawing start pointer to current pos
    bra         .end_loop                           ;1 d--23-5-7 a--23-5--  10   
                                                    ;1 d--23-5-7 a--23-5--       
.cmp_4:                                             ;1 d--23-5-7 a--23-5--       -------- y0 <= ymin < y1 && y0 <= ymax < y1 --------
                                                    ;1 d--23-5-7 a--23-5--       ======== [gap] ========
                                                    ;1 d--23-5-7 a--23-5--       -------- [gap] --------
                                                    ;1 d--23-5-7 a--23-5--       nothing to be done
    addq.l      #4, a0                              ;1 d--23-5-7 a--23-5--   8   SKY was skipped
    bra         .end_loop                           ;1 d--23-5-7 a--23-5--  10   
                                                    ;1 d---3-5-7 a--23-5--       
.cmp_1:                                             ;1 d---3-5-7 a--23-5--       -------- y1 <= ymax --------
    swap.w      d2                                  ;1 d---3-5-7 a--23-5--   4   Select ymin
    move.w      rc_ymin(a4), d2                     ;1 d---3-5-7 a--23-5--  12   Load ymin
    cmp.w       d0, d2                              ;1 d---3-5-7 a--23-5--   4   ymin>=y0
    bge         .cmp_6                              ;1 d---3-5-7 a--23-5--  10   
                                                    ;1 d---3-5-7 a--23-5--       
                                                    ;1 d---3-5-7 a--23-5--       -------- ymin < y0 && y1 <= ymax --------
                                                    ;1 d---3-5-7 a--23-5--       ======== [Sky gap Lower] ========
                                                    ;1 d---3-5-7 a--23-5--       -------- [Sky --------
    lea.l       _FUNC_SKYCOPY+ENGINE_Y_MAX*2, a3    ;1 d---3-5-7 a--2--5--  12   Sky copy func
    move.l      (a0)+, a2                           ;1 d---3-5-7 a-----5--  12   
    add.w       d2, a2                              ;1 d---3-5-7 a-----5--   8   sky offset by ymin
    sub.w       d0, d2                              ;1 d---3-5-7 a--2--5--   4   
    add.w       d2, d2                              ;1 d---3-5-7 a--2--5--   4   
    lea.l       .ret_7, a6                          ;1 d---3-5-7 a--2--5--  12   
    jmp         (a3,d2.w)                           ;1 d---3-5-7 a--2--5--  14   
.ret_7:                                             ;1 d---3-5-7 a--23-5--       
                                                    ;1 d---3-5-7 a--23-5--       -------- gap --------
    move.w      d0, rc_ymin(a4)                     ;1 d---3-5-7 a--23-5--   8   Update rc_ymin with ystart
    move.l      a7, (a4)                            ;1 d---3-5-7 a--23-5--  12   Set future drawing start pointer to current pos
    move.w      d1, rc_ymax(a4)                     ;1 d---3-5-7 a--23-5--   8   Update rc_ymax with yend
    move.w      d1, d3                              ;1 d-----5-7 a--23-5--   4   Add 2*(yend - ystart) to A7
    sub.w       d0, d3                              ;1 d-----5-7 a--23-5--   4   
    add.w       d3, d3                              ;1 d-----5-7 a--23-5--   4   
    add.w       d3, a7                              ;1 d-----5-7 a--23-5--   8   
                                                    ;1 d---3-5-7 a--23-5--       -------- Lower] --------
                                                    ;1 d---3-5-7 a--23-5--       -- Fill routine --
    move.w      d4, d3                              ;1 d-----5-7 a--23-5--   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d3                              ;1 d-----5-7 a--23-5--  12   
    and.w       #$1FFC, d3                          ;1 d-----5-7 a--23-5--   8   
    lea.l       _FN_SCALERS, a3                     ;1 d-----5-7 a--2--5--  12   
    move.l      (a3,d3.w), a3                       ;1 d-----5-7 a--2--5--  18   
                                                    ;1 d---3-5-7 a--2--5--       -- Texcoord (persp) --
    move.l      d6, d3                              ;1 d-----5-7 a--2--5--   4   word tx = u1s /divu/ s1;
    divu.w      d4, d3                              ;1 d-----5-7 a--2--5-- 140   
    and.w       #$1F80, d3                          ;1 d-----5-7 a--2--5--   8   tx &= $1F80;
    move.l      .asm_tex_base2(pc), a2              ;1 d-----5-7 a-----5--  16   -- Texture source --
    add.w       d3, a2                              ;1 d-----5-7 a-----5--   8   
    swap.w      d2                                  ;1 d---3-5-7 a--2--5--   4   Select ymax
    add.w       d2, d2                              ;1 d---3-5-7 a--2--5--   4   bottom_offset = 4*ymax
    add.w       d2, d2                              ;1 d---3-5-7 a--2--5--   4   
    move.w      #$4ED6, (a3,d2.w)                   ;1 d---3-5-7 a--2--5--  18   #$4ED6 -> (fill_routine, bottom_offset)
    move.w      d1, d3                              ;1 d-----5-7 a--2--5--   4   top_offset = 4*ystart
    add.w       d3, d3                              ;1 d-----5-7 a--2--5--   4   
    add.w       d3, d3                              ;1 d-----5-7 a--2--5--   4   
    lea.l       .ret_8, a6                          ;1 d-----5-7 a--2--5--  12   
    jmp         (a3,d3.w)                           ;1 d-----5-7 a--2--5--  14   execute at (fill_routine, top_offset)
.ret_8:                                             ;1 d---3-5-7 a--2--5--       
    move.w      #$1EEA, (a3,d2.w)                   ;1 d---3-5-7 a--2--5--  18   #$1EEA -> (fill_routine, bottom_offset)
    bra         .end_loop                           ;1 d--23-5-7 a--23-5--  10   
                                                    ;1 d---3-5-7 a--23-5--       
.cmp_6:                                             ;1 d---3-5-7 a--23-5--       -------- y0 <= ymin && y1 <= ymax --------
    cmp.w       d1, d2                              ;1 d---3-5-7 a--23-5--   4   ymin>=y1
    bge         .cmp_9                              ;1 d---3-5-7 a--23-5--  10   
                                                    ;1 d---3-5-7 a--23-5--       
                                                    ;1 d---3-5-7 a--23-5--       -------- y0 <= ymin < y1 && y1 <= ymax --------
                                                    ;1 d---3-5-7 a--23-5--       ======== [gap Lower] ========
                                                    ;1 d---3-5-7 a--23-5--       -------- [gap --------
    move.w      d1, rc_ymax(a4)                     ;1 d---3-5-7 a--23-5--   8   Update rc_ymax with yend
    sub.w       d1, d2                              ;1 d---3-5-7 a--23-5--   4   Add 2*(yend - rc_ymin) to A7
    neg.w       d2                                  ;1 d---3-5-7 a--23-5--   4   
    add.w       d2, d2                              ;1 d---3-5-7 a--23-5--   4   
    add.w       d2, a7                              ;1 d---3-5-7 a--23-5--   8   
                                                    ;1 d---3-5-7 a--23-5--       -------- Lower] --------
                                                    ;1 d---3-5-7 a--23-5--       -- Fill routine --
    move.w      d4, d3                              ;1 d-----5-7 a--23-5--   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d3                              ;1 d-----5-7 a--23-5--  12   
    and.w       #$1FFC, d3                          ;1 d-----5-7 a--23-5--   8   
    lea.l       _FN_SCALERS, a3                     ;1 d-----5-7 a--2--5--  12   
    move.l      (a3,d3.w), a3                       ;1 d-----5-7 a--2--5--  18   
                                                    ;1 d---3-5-7 a--2--5--       -- Texcoord (persp) --
    move.l      d6, d3                              ;1 d-----5-7 a--2--5--   4   word tx = u1s /divu/ s1;
    divu.w      d4, d3                              ;1 d-----5-7 a--2--5-- 140   
    and.w       #$1F80, d3                          ;1 d-----5-7 a--2--5--   8   tx &= $1F80;
    move.l      .asm_tex_base2(pc), a2              ;1 d-----5-7 a-----5--  16   -- Texture source --
    add.w       d3, a2                              ;1 d-----5-7 a-----5--   8   
    swap.w      d2                                  ;1 d---3-5-7 a--2--5--   4   Select ymax
    add.w       d2, d2                              ;1 d---3-5-7 a--2--5--   4   bottom_offset = 4*ymax
    add.w       d2, d2                              ;1 d---3-5-7 a--2--5--   4   
    move.w      #$4ED6, (a3,d2.w)                   ;1 d---3-5-7 a--2--5--  18   #$4ED6 -> (fill_routine, bottom_offset)
    move.w      d1, d3                              ;1 d-----5-7 a--2--5--   4   top_offset = 4*ystart
    add.w       d3, d3                              ;1 d-----5-7 a--2--5--   4   
    add.w       d3, d3                              ;1 d-----5-7 a--2--5--   4   
    lea.l       .ret_10, a6                         ;1 d-----5-7 a--2--5--  12   
    jmp         (a3,d3.w)                           ;1 d-----5-7 a--2--5--  14   execute at (fill_routine, top_offset)
.ret_10:                                            ;1 d---3-5-7 a--2--5--       
    move.w      #$1EEA, (a3,d2.w)                   ;1 d---3-5-7 a--2--5--  18   #$1EEA -> (fill_routine, bottom_offset)
    addq.l      #4, a0                              ;1 d--23-5-7 a--23-5--   8   SKY was skipped
    bra         .end_loop                           ;1 d--23-5-7 a--23-5--  10   
                                                    ;1 d---3-5-7 a--23-5--       
.cmp_9:                                             ;1 d---3-5-7 a--23-5--       -------- y1 <= ymin && y1 <= ymax --------
                                                    ;1 d---3-5-7 a--23-5--       ======== [Lower] ========
                                                    ;1 d---3-5-7 a--23-5--       -------- [Lower] --------
                                                    ;1 d---3-5-7 a--23-5--       -- Fill routine --
    move.w      d4, d3                              ;1 d-----5-7 a--23-5--   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d3                              ;1 d-----5-7 a--23-5--  12   
    and.w       #$1FFC, d3                          ;1 d-----5-7 a--23-5--   8   
    lea.l       _FN_SCALERS, a3                     ;1 d-----5-7 a--2--5--  12   
    move.l      (a3,d3.w), a3                       ;1 d-----5-7 a--2--5--  18   
                                                    ;1 d---3-5-7 a--2--5--       -- Texcoord (persp) --
    move.l      d6, d3                              ;1 d-----5-7 a--2--5--   4   word tx = u1s /divu/ s1;
    divu.w      d4, d3                              ;1 d-----5-7 a--2--5-- 140   
    and.w       #$1F80, d3                          ;1 d-----5-7 a--2--5--   8   tx &= $1F80;
    move.l      .asm_tex_base2(pc), a2              ;1 d-----5-7 a-----5--  16   -- Texture source --
    add.w       d3, a2                              ;1 d-----5-7 a-----5--   8   
    swap.w      d2                                  ;1 d---3-5-7 a--2--5--   4   Select ymax
    add.w       d2, d2                              ;1 d---3-5-7 a--2--5--   4   bottom_offset = 4*ymax
    add.w       d2, d2                              ;1 d---3-5-7 a--2--5--   4   
    move.w      #$4ED6, (a3,d2.w)                   ;1 d---3-5-7 a--2--5--  18   #$4ED6 -> (fill_routine, bottom_offset)
    swap.w      d2                                  ;1 d---3-5-7 a--2--5--   4   Select ymin
    add.w       d2, d2                              ;1 d---3-5-7 a--2--5--   4   top_offset = 4*ymin
    add.w       d2, d2                              ;1 d---3-5-7 a--2--5--   4   
    lea.l       .ret_11, a6                         ;1 d---3-5-7 a--2--5--  12   
    jmp         (a3,d2.w)                           ;1 d---3-5-7 a--2--5--  14   execute at (fill_routine, top_offset)
.ret_11:                                            ;1 d---3-5-7 a--2--5--       
    swap.w      d2                                  ;1 d---3-5-7 a--2--5--   4   Select ymax
    move.w      #$1EEA, (a3,d2.w)                   ;1 d---3-5-7 a--2--5--  18   #$1EEA -> (fill_routine, bottom_offset)
    addq.l      #4, a0                              ;1 d--23-5-7 a--23-5--   8   SKY was skipped
    clr.l       rc_ymin(a4)                         ;1 d--23-5-7 a--23-5--       Disable further rendering - clear ymin/ymax
    move.w      d4, rc_size_limit(a4)               ;1 d--23-5-7 a--23-5--   8   
    bra         .end_loop                           ;1 d--23-5-7 a--23-5--  10   
                                                    ;1 d--23-5-7 a--23-5--       
                                                    ;1 d--23-5-7 a--23-5--       
.end_loop:                                          ;1 d--23-5-7 a--23-5--       ======== LOOP END ========
    cmp.l       a1, a4                              ;1 d--23-5-7 a--23-5--   6   render_column ? render_end
    blt         .loop                               ;1 d--23-5-7 a--23-5--  10   continue if <
.end:                                               ;1 d01234567 a012345--       Cleanup & exit
    move.l      _a7_save, a7                        ;1 d01234567 a012345--  20   
    rts                                             ;1 d01234567 a012345--  16   
.asm_tex_base2:                                     ;1 d01234567 a012345--       
    move.w      #0, d0                              ;1 d-1234567 a012345--   8   reserve 32-bit
