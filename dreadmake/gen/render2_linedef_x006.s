
    public _Dread_LineCore_x006
_Dread_LineCore_x006:

                                                    ;0 d01---5-- a01234---       ================================ gap y3 Lower y4 Floor ================================
    move.l      a7, _a7_save                        ;0 d01---5-- a01234---  20   
                                                    ;0 d01---5-- a01234---       
                                                    ;0 d01---5-- a01234---       ======== Loop initialization ========
    lea.l       _render_columns, a4                 ;0 d01---5-- a0123----  12   
    add.w       d2, a4                              ;0 d01---5-- a0123----   8   add xmin*8 to column pointer
    move.w      d7, .trace_tex+2                    ;0 d012--5-- a0123----  16   
    move.l      a4, a0                              ;0 d012--5-7 a-123----   4   end = start + count*8
    lsl.w       #3, d3                              ;0 d012--5-7 a-123----  12   
    add.w       d3, a0                              ;0 d012--5-7 a-123----   8   
    move.l      line_tex_lower(a5), a1              ;0 d0123-5-7 a--23----  16   Lower texture
    move.w      _view_pos_z, d0                     ;0 d-123-5-7 a--23----  16   
    asr.w       #1, d0                              ;0 d-123-5-7 a--23----   8   
    add.w       d0, a1                              ;0 d-123-5-7 a--23----   8   
    move.l      a1, .asm_tex_base2                  ;0 d0123-5-7 a--23----  20   
                                                    ;0 d0123-5-7 a-123----       
                                                    ;0 d0123-5-7 a-123----       ======== init .trace_y0 tracer ========
    move.w      d4, d0                              ;0 d-123-5-7 a-123----   4   compute starting Y
    lsr.w       #5, d0                              ;0 d-123-5-7 a-123----  16   
    move.w      line_y3(a5), d1                     ;0 d--23-5-7 a-123----  12   
    sub.w       _view_pos_z, d1                     ;0 d--23-5-7 a-123----  16   
    muls.w      d1, d0                              ;0 d--23-5-7 a-123----  70   
    move.w      _asm_line_s2, d5                    ;0 d--23---7 a-123----  16   compute Y slope
    lsr.w       #5, d5                              ;0 d--23---7 a-123----  16   
    muls.w      d1, d5                              ;0 d--23---7 a-123----  70   
    sub.l       d0, d5                              ;0 d-123---7 a-123----   8   
    divs.w      _asm_line_xlen, d5                  ;0 d-123---7 a-123---- 170   
    move.b      d5, .trace_s+2                      ;0 d-123---7 a-123----  16   subpixel step
    lsr.w       #8, d5                              ;0 d-123---7 a-123----  22   
    ext.w       d5                                  ;0 d-123---7 a-123----   4   
    move.w      d5, .trace_y0+4                     ;0 d-123---7 a-123----  16   pixel step
    add.l       #ENGINE_Y_MID*256+128, d0           ;0 d-123-5-7 a-123----  16   24.8 fixed
    lsl.l       #8, d0                              ;0 d-123-5-7 a-123----  24   16.16 fixed
    swap.w      d4                                  ;0 d-123-5-7 a-123----   4   
    move.w      d0, d4                              ;0 d-123-5-7 a-123----   4   
    swap.w      d4                                  ;0 d-123-5-7 a-123----   4   
    sub.w       d0, d0                              ;0 d-123-5-7 a-123----   4   
    swap.w      d0                                  ;0 d-123-5-7 a-123----   4   current Y value
                                                    ;0 d-123-5-7 a-123----       
                                                    ;0 d-123-5-7 a-123----       ======== init .trace_y1 tracer ========
    move.w      d4, d1                              ;0 d--23-5-7 a-123----   4   compute starting Y
    lsr.w       #5, d1                              ;0 d--23-5-7 a-123----  16   
    move.w      line_y4(a5), d2                     ;0 d---3-5-7 a-123----  12   
    sub.w       _view_pos_z, d2                     ;0 d---3-5-7 a-123----  16   
    muls.w      d2, d1                              ;0 d---3-5-7 a-123----  70   
    move.w      _asm_line_s2, d5                    ;0 d---3---7 a-123----  16   compute Y slope
    lsr.w       #5, d5                              ;0 d---3---7 a-123----  16   
    muls.w      d2, d5                              ;0 d---3---7 a-123----  70   
    sub.l       d1, d5                              ;0 d--23---7 a-123----   8   
    divs.w      _asm_line_xlen, d5                  ;0 d--23---7 a-123---- 170   
    move.b      d5, .trace_y0+2                     ;0 d--23---7 a-123----  16   subpixel step
    lsr.w       #8, d5                              ;0 d--23---7 a-123----  22   
    ext.w       d5                                  ;0 d--23---7 a-123----   4   
    move.w      d5, .trace_y1+4                     ;0 d--23---7 a-123----  16   pixel step
    add.l       #ENGINE_Y_MID*256+128, d1           ;0 d--23-5-7 a-123----  16   24.8 fixed
    lsl.l       #8, d1                              ;0 d--23-5-7 a-123----  24   16.16 fixed
    swap.w      d0                                  ;0 d--23-5-7 a-123----   4   
    move.w      d1, d0                              ;0 d--23-5-7 a-123----   4   
    swap.w      d0                                  ;0 d--23-5-7 a-123----   4   
    sub.w       d1, d1                              ;0 d--23-5-7 a-123----   4   
    swap.w      d1                                  ;0 d--23-5-7 a-123----   4   current Y value
    move.w      _asm_line_ds, .trace_s+4            ;0 d--23-5-7 a-123----  28   
    bra         .start                              ;0 d--23-5-7 a-123----  10   End of header
                                                    ;0 d--23-5-7 a-123----       
                                                    ;0 d--23-5-7 a-123----       
.loop:                                              ;1 d--23-5-7 a-123----       ======== LOOP START ========
    addq.w      #8, a4                              ;1 d--23-5-7 a-123----   8   
.trace_s:                                           ;1 d--23-5-7 a-123----       
    add.l       #$FF00FFFF, d4                      ;1 d--23-5-7 a-123----  16   s1 += ds
.trace_y0:                                          ;1 d--23-5-7 a-123----       
    move.l      #$FF000000, d2                      ;1 d---3-5-7 a-123----  12   
    addx.l      d2, d0                              ;1 d---3-5-7 a-123----   8   
.trace_y1:                                          ;1 d--23-5-7 a-123----       
    move.l      #$FF000000, d2                      ;1 d---3-5-7 a-123----  12   
    addx.l      d2, d1                              ;1 d---3-5-7 a-123----   8   
.trace_tex:                                         ;1 d--23-5-7 a-123----       
    add.w       #$8000, d6                          ;1 d--23-5-7 a-123----   8   u += du
                                                    ;1 d--23-5-7 a-123----       
.start:                                             ;1 d--23-5-7 a-123----       ======== FIRST ITERATION STARTS HERE ========
    move.l      (a4), a7                            ;1 d--23-5-7 a-123----  12   byte *dst = render_buffer + offs[x];
                                                    ;1 d--23-5-7 a-123----       
                                                    ;1 d--23-5-7 a-123----       -------- ymin/ymax not checked --------
    move.w      rc_ymax(a4), d2                     ;1 d---3-5-7 a-123----  12   Load ymax
    cmp.w       d1, d2                              ;1 d---3-5-7 a-123----   4   ymax>=y1
    bge         .cmp_1                              ;1 d---3-5-7 a-123----  10   
                                                    ;1 d---3-5-7 a-123----       
                                                    ;1 d---3-5-7 a-123----       -------- ymin < y1 && ymax < y1 --------
    cmp.w       d0, d2                              ;1 d---3-5-7 a-123----   4   ymax>=y0
    bge         .cmp_2                              ;1 d---3-5-7 a-123----  10   
                                                    ;1 d--23-5-7 a-123----       
                                                    ;1 d--23-5-7 a-123----       -------- ymin < y0 && ymax < y0 --------
                                                    ;1 d--23-5-7 a-123----       ======== [gap] ========
                                                    ;1 d--23-5-7 a-123----       -------- [gap] --------
                                                    ;1 d--23-5-7 a-123----       nothing to be done
    bra         .end_loop                           ;1 d--23-5-7 a-123----  10   
                                                    ;1 d---3-5-7 a-123----       
.cmp_2:                                             ;1 d---3-5-7 a-123----       -------- ymin < y1 && y0 <= ymax < y1 --------
    swap.w      d2                                  ;1 d---3-5-7 a-123----   4   Select ymin
    move.w      rc_ymin(a4), d2                     ;1 d---3-5-7 a-123----  12   Load ymin
    cmp.w       d0, d2                              ;1 d---3-5-7 a-123----   4   ymin>=y0
    bge         .cmp_3                              ;1 d---3-5-7 a-123----  10   
                                                    ;1 d---3-5-7 a-123----       
                                                    ;1 d---3-5-7 a-123----       -------- ymin < y0 && y0 <= ymax < y1 --------
                                                    ;1 d---3-5-7 a-123----       ======== [gap Lower] ========
                                                    ;1 d---3-5-7 a-123----       -------- [gap --------
    move.w      d0, rc_ymax(a4)                     ;1 d---3-5-7 a-123----   8   Update rc_ymax with yend
    sub.w       d0, d2                              ;1 d---3-5-7 a-123----   4   Add 2*(yend - rc_ymin) to A7
    neg.w       d2                                  ;1 d---3-5-7 a-123----   4   
    add.w       d2, d2                              ;1 d---3-5-7 a-123----   4   
    add.w       d2, a7                              ;1 d---3-5-7 a-123----   8   
                                                    ;1 d---3-5-7 a-123----       -------- Lower] --------
                                                    ;1 d---3-5-7 a-123----       -- Fill routine --
    move.w      d4, d3                              ;1 d-----5-7 a-123----   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d3                              ;1 d-----5-7 a-123----  12   
    and.w       #$1FFC, d3                          ;1 d-----5-7 a-123----   8   
    lea.l       _FN_SCALERS, a1                     ;1 d-----5-7 a--23----  12   
    move.l      (a1,d3.w), a1                       ;1 d-----5-7 a--23----  18   
                                                    ;1 d---3-5-7 a--23----       -- Texcoord (no persp) --
    move.w      d6, d3                              ;1 d-----5-7 a--23----   4   word tx = u1s;
    and.w       #$1F80, d3                          ;1 d-----5-7 a--23----   8   tx &= $1F80;
    move.l      .asm_tex_base2(pc), a2              ;1 d-----5-7 a---3----  16   -- Texture source --
    add.w       d3, a2                              ;1 d-----5-7 a---3----   8   
    swap.w      d2                                  ;1 d---3-5-7 a--23----   4   Select ymax
    add.w       d2, d2                              ;1 d---3-5-7 a--23----   4   bottom_offset = 4*ymax
    add.w       d2, d2                              ;1 d---3-5-7 a--23----   4   
    move.w      #$4ED6, (a1,d2.w)                   ;1 d---3-5-7 a--23----  18   #$4ED6 -> (fill_routine, bottom_offset)
    move.w      d0, d3                              ;1 d-----5-7 a--23----   4   top_offset = 4*ystart
    add.w       d3, d3                              ;1 d-----5-7 a--23----   4   
    add.w       d3, d3                              ;1 d-----5-7 a--23----   4   
    lea.l       .ret_4, a6                          ;1 d-----5-7 a--23----  12   
    jmp         (a1,d3.w)                           ;1 d-----5-7 a--23----  14   execute at (fill_routine, top_offset)
.ret_4:                                             ;1 d---3-5-7 a--23----       
    move.w      #$1EEA, (a1,d2.w)                   ;1 d---3-5-7 a--23----  18   #$1EEA -> (fill_routine, bottom_offset)
    bra         .end_loop                           ;1 d--23-5-7 a-123----  10   
                                                    ;1 d---3-5-7 a-123----       
.cmp_3:                                             ;1 d---3-5-7 a-123----       -------- y0 <= ymin < y1 && y0 <= ymax < y1 --------
                                                    ;1 d---3-5-7 a-123----       ======== [Lower] ========
                                                    ;1 d---3-5-7 a-123----       -------- [Lower] --------
                                                    ;1 d---3-5-7 a-123----       -- Fill routine --
    move.w      d4, d3                              ;1 d-----5-7 a-123----   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d3                              ;1 d-----5-7 a-123----  12   
    and.w       #$1FFC, d3                          ;1 d-----5-7 a-123----   8   
    lea.l       _FN_SCALERS, a1                     ;1 d-----5-7 a--23----  12   
    move.l      (a1,d3.w), a1                       ;1 d-----5-7 a--23----  18   
                                                    ;1 d---3-5-7 a--23----       -- Texcoord (no persp) --
    move.w      d6, d3                              ;1 d-----5-7 a--23----   4   word tx = u1s;
    and.w       #$1F80, d3                          ;1 d-----5-7 a--23----   8   tx &= $1F80;
    move.l      .asm_tex_base2(pc), a2              ;1 d-----5-7 a---3----  16   -- Texture source --
    add.w       d3, a2                              ;1 d-----5-7 a---3----   8   
    swap.w      d2                                  ;1 d---3-5-7 a--23----   4   Select ymax
    add.w       d2, d2                              ;1 d---3-5-7 a--23----   4   bottom_offset = 4*ymax
    add.w       d2, d2                              ;1 d---3-5-7 a--23----   4   
    move.w      #$4ED6, (a1,d2.w)                   ;1 d---3-5-7 a--23----  18   #$4ED6 -> (fill_routine, bottom_offset)
    swap.w      d2                                  ;1 d---3-5-7 a--23----   4   Select ymin
    add.w       d2, d2                              ;1 d---3-5-7 a--23----   4   top_offset = 4*ymin
    add.w       d2, d2                              ;1 d---3-5-7 a--23----   4   
    lea.l       .ret_5, a6                          ;1 d---3-5-7 a--23----  12   
    jmp         (a1,d2.w)                           ;1 d---3-5-7 a--23----  14   execute at (fill_routine, top_offset)
.ret_5:                                             ;1 d---3-5-7 a--23----       
    swap.w      d2                                  ;1 d---3-5-7 a--23----   4   Select ymax
    move.w      #$1EEA, (a1,d2.w)                   ;1 d---3-5-7 a--23----  18   #$1EEA -> (fill_routine, bottom_offset)
    clr.l       rc_ymin(a4)                         ;1 d--23-5-7 a-123----       Disable further rendering - clear ymin/ymax
    move.w      d4, rc_size_limit(a4)               ;1 d--23-5-7 a-123----   8   
    bra         .end_loop                           ;1 d--23-5-7 a-123----  10   
                                                    ;1 d---3-5-7 a-123----       
.cmp_1:                                             ;1 d---3-5-7 a-123----       -------- y1 <= ymax --------
    swap.w      d2                                  ;1 d---3-5-7 a-123----   4   Select ymin
    move.w      rc_ymin(a4), d2                     ;1 d---3-5-7 a-123----  12   Load ymin
    cmp.w       d0, d2                              ;1 d---3-5-7 a-123----   4   ymin>=y0
    bge         .cmp_6                              ;1 d---3-5-7 a-123----  10   
                                                    ;1 d---3-5-7 a-123----       
                                                    ;1 d---3-5-7 a-123----       -------- ymin < y0 && y1 <= ymax --------
                                                    ;1 d---3-5-7 a-123----       ======== [gap Lower Floor] ========
                                                    ;1 d---3-5-7 a-123----       -------- [gap --------
    move.w      d0, rc_ymax(a4)                     ;1 d---3-5-7 a-123----   8   Update rc_ymax with yend
    sub.w       d0, d2                              ;1 d---3-5-7 a-123----   4   Add 2*(yend - rc_ymin) to A7
    neg.w       d2                                  ;1 d---3-5-7 a-123----   4   
    add.w       d2, d2                              ;1 d---3-5-7 a-123----   4   
    add.w       d2, a7                              ;1 d---3-5-7 a-123----   8   
                                                    ;1 d---3-5-7 a-123----       -------- Lower --------
                                                    ;1 d---3-5-7 a-123----       -- Fill routine --
    move.w      d4, d3                              ;1 d-----5-7 a-123----   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d3                              ;1 d-----5-7 a-123----  12   
    and.w       #$1FFC, d3                          ;1 d-----5-7 a-123----   8   
    lea.l       _FN_SCALERS, a1                     ;1 d-----5-7 a--23----  12   
    move.l      (a1,d3.w), a1                       ;1 d-----5-7 a--23----  18   
                                                    ;1 d---3-5-7 a--23----       -- Texcoord (no persp) --
    move.w      d6, d3                              ;1 d-----5-7 a--23----   4   word tx = u1s;
    and.w       #$1F80, d3                          ;1 d-----5-7 a--23----   8   tx &= $1F80;
    move.l      .asm_tex_base2(pc), a2              ;1 d-----5-7 a---3----  16   -- Texture source --
    add.w       d3, a2                              ;1 d-----5-7 a---3----   8   
    move.w      d1, d3                              ;1 d-----5-7 a--23----   4   bottom_offset = 4*yend
    add.w       d3, d3                              ;1 d-----5-7 a--23----   4   
    add.w       d3, d3                              ;1 d-----5-7 a--23----   4   
    move.w      #$4ED6, (a1,d3.w)                   ;1 d-----5-7 a--23----  18   #$4ED6 -> (fill_routine, bottom_offset)
    move.w      d0, d7                              ;1 d-----5-- a--23----   4   top_offset = 4*ystart
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   
    lea.l       .ret_7, a6                          ;1 d-----5-- a--23----  12   
    jmp         (a1,d7.w)                           ;1 d-----5-- a--23----  14   execute at (fill_routine, top_offset)
.ret_7:                                             ;1 d-----5-7 a--23----       
    move.w      #$1EEA, (a1,d3.w)                   ;1 d-----5-7 a--23----  18   #$1EEA -> (fill_routine, bottom_offset)
                                                    ;1 d---3-5-7 a-123----       -------- Floor] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;1 d---3-5-7 a--23----  12   Color filler func
    swap.w      d2                                  ;1 d---3-5-7 a--23----   4   Select ymax
    sub.w       d1, d2                              ;1 d---3-5-7 a--23----   4   
    neg.w       d2                                  ;1 d---3-5-7 a--23----   4   
    add.w       d2, d2                              ;1 d---3-5-7 a--23----   4   
    move.b      line_floor_col(a5), d5              ;1 d---3---7 a--23----  12   
    lea.l       .ret_8, a6                          ;1 d---3-5-7 a--23----  12   
    jmp         (a1,d2.w)                           ;1 d---3-5-7 a--23----  14   
.ret_8:                                             ;1 d--23-5-7 a-123----       
    bra         .end_loop                           ;1 d--23-5-7 a-123----  10   
                                                    ;1 d---3-5-7 a-123----       
.cmp_6:                                             ;1 d---3-5-7 a-123----       -------- y0 <= ymin && y1 <= ymax --------
    cmp.w       d1, d2                              ;1 d---3-5-7 a-123----   4   ymin>=y1
    bge         .cmp_9                              ;1 d---3-5-7 a-123----  10   
                                                    ;1 d---3-5-7 a-123----       
                                                    ;1 d---3-5-7 a-123----       -------- y0 <= ymin < y1 && y1 <= ymax --------
                                                    ;1 d---3-5-7 a-123----       ======== [Lower Floor] ========
                                                    ;1 d---3-5-7 a-123----       -------- [Lower --------
                                                    ;1 d---3-5-7 a-123----       -- Fill routine --
    move.w      d4, d3                              ;1 d-----5-7 a-123----   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d3                              ;1 d-----5-7 a-123----  12   
    and.w       #$1FFC, d3                          ;1 d-----5-7 a-123----   8   
    lea.l       _FN_SCALERS, a1                     ;1 d-----5-7 a--23----  12   
    move.l      (a1,d3.w), a1                       ;1 d-----5-7 a--23----  18   
                                                    ;1 d---3-5-7 a--23----       -- Texcoord (no persp) --
    move.w      d6, d3                              ;1 d-----5-7 a--23----   4   word tx = u1s;
    and.w       #$1F80, d3                          ;1 d-----5-7 a--23----   8   tx &= $1F80;
    move.l      .asm_tex_base2(pc), a2              ;1 d-----5-7 a---3----  16   -- Texture source --
    add.w       d3, a2                              ;1 d-----5-7 a---3----   8   
    move.w      d1, d3                              ;1 d-----5-7 a--23----   4   bottom_offset = 4*yend
    add.w       d3, d3                              ;1 d-----5-7 a--23----   4   
    add.w       d3, d3                              ;1 d-----5-7 a--23----   4   
    move.w      #$4ED6, (a1,d3.w)                   ;1 d-----5-7 a--23----  18   #$4ED6 -> (fill_routine, bottom_offset)
    add.w       d2, d2                              ;1 d-----5-7 a--23----   4   top_offset = 4*ymin
    add.w       d2, d2                              ;1 d-----5-7 a--23----   4   
    lea.l       .ret_10, a6                         ;1 d-----5-7 a--23----  12   
    jmp         (a1,d2.w)                           ;1 d-----5-7 a--23----  14   execute at (fill_routine, top_offset)
.ret_10:                                            ;1 d-----5-7 a--23----       
    move.w      #$1EEA, (a1,d3.w)                   ;1 d-----5-7 a--23----  18   #$1EEA -> (fill_routine, bottom_offset)
                                                    ;1 d---3-5-7 a-123----       -------- Floor] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;1 d---3-5-7 a--23----  12   Color filler func
    swap.w      d2                                  ;1 d---3-5-7 a--23----   4   Select ymax
    sub.w       d1, d2                              ;1 d---3-5-7 a--23----   4   
    neg.w       d2                                  ;1 d---3-5-7 a--23----   4   
    add.w       d2, d2                              ;1 d---3-5-7 a--23----   4   
    move.b      line_floor_col(a5), d5              ;1 d---3---7 a--23----  12   
    lea.l       .ret_11, a6                         ;1 d---3-5-7 a--23----  12   
    jmp         (a1,d2.w)                           ;1 d---3-5-7 a--23----  14   
.ret_11:                                            ;1 d--23-5-7 a-123----       
    clr.l       rc_ymin(a4)                         ;1 d--23-5-7 a-123----       Disable further rendering - clear ymin/ymax
    move.w      d4, rc_size_limit(a4)               ;1 d--23-5-7 a-123----   8   
    bra         .end_loop                           ;1 d--23-5-7 a-123----  10   
                                                    ;1 d---3-5-7 a-123----       
.cmp_9:                                             ;1 d---3-5-7 a-123----       -------- y1 <= ymin && y1 <= ymax --------
                                                    ;1 d---3-5-7 a-123----       ======== [Floor] ========
                                                    ;1 d---3-5-7 a-123----       -------- [Floor] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;1 d---3-5-7 a--23----  12   Color filler func
    move.w      d2, d3                              ;1 d-----5-7 a--23----   4   
    swap.w      d2                                  ;1 d-----5-7 a--23----   4   Select ymax
    sub.w       d2, d3                              ;1 d-----5-7 a--23----   4   
    add.w       d3, d3                              ;1 d--2--5-7 a--23----   4   
    move.b      line_floor_col(a5), d5              ;1 d--2----7 a--23----  12   
    lea.l       .ret_12, a6                         ;1 d--2--5-7 a--23----  12   
    jmp         (a1,d3.w)                           ;1 d--2--5-7 a--23----  14   
.ret_12:                                            ;1 d--23-5-7 a-123----       
    clr.l       rc_ymin(a4)                         ;1 d--23-5-7 a-123----       Disable further rendering - clear ymin/ymax
    move.w      d4, rc_size_limit(a4)               ;1 d--23-5-7 a-123----   8   
    bra         .end_loop                           ;1 d--23-5-7 a-123----  10   
                                                    ;1 d--23-5-7 a-123----       
                                                    ;1 d--23-5-7 a-123----       
.end_loop:                                          ;1 d--23-5-7 a-123----       ======== LOOP END ========
    cmp.l       a0, a4                              ;1 d--23-5-7 a-123----   6   render_column ? render_end
    blt         .loop                               ;1 d--23-5-7 a-123----  10   continue if <
.end:                                               ;1 d01234567 a012345--       Cleanup & exit
    move.l      _a7_save, a7                        ;1 d01234567 a012345--  20   
    rts                                             ;1 d01234567 a012345--  16   
.asm_tex_base2:                                     ;1 d01234567 a012345--       
    move.w      #0, d0                              ;1 d-1234567 a012345--   8   reserve 32-bit
