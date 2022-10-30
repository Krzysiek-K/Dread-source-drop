
    public _Dread_LineCore_x012
_Dread_LineCore_x012:

    move.l      a7, _a7_save                        ;0 d01---5-- a01234---  20   
                                                    ;0 d01---5-- a01234---       
                                                    ;0 d01---5-- a01234---       ======== Loop initialization ========
    lea.l       _render_columns, a4                 ;0 d01---5-- a0123----  12   
    add.w       d2, a4                              ;0 d01---5-- a0123----   8   add xmin*8 to column pointer
    move.l      d7, .trace_tex+2                    ;0 d012--5-- a0123----  20   
    move.l      a4, a0                              ;0 d012--5-7 a-123----   4   end = start + count*8
    lsl.w       #3, d3                              ;0 d012--5-7 a-123----  12   
    add.w       d3, a0                              ;0 d012--5-7 a-123----   8   
    move.l      line_tex_upper(a5), a1              ;0 d0123-5-7 a--23----  16   Upper texture
    move.w      _view_pos_z, d0                     ;0 d-123-5-7 a--23----  16   
    asr.w       #1, d0                              ;0 d-123-5-7 a--23----   8   
    add.w       d0, a1                              ;0 d-123-5-7 a--23----   8   
    move.l      a1, _asm_tex_base                   ;0 d0123-5-7 a--23----  20   
                                                    ;0 d0123-5-7 a-123----       
                                                    ;0 d0123-5-7 a-123----       ======== init .trace_y0 tracer ========
    move.w      d4, d0                              ;0 d-123-5-7 a-123----   4   compute starting Y
    lsr.w       #5, d0                              ;0 d-123-5-7 a-123----  16   
    move.w      line_y2(a5), d1                     ;0 d--23-5-7 a-123----  12   
    sub.w       _view_pos_z, d1                     ;0 d--23-5-7 a-123-5--  16   
    muls.w      d1, d0                              ;0 d--23-5-7 a-123-5--  70   
    move.w      _asm_line_s2, d5                    ;0 d--23---7 a-123-5--  16   compute Y slope
    lsr.w       #5, d5                              ;0 d--23---7 a-123-5--  16   
    muls.w      d1, d5                              ;0 d--23---7 a-123-5--  70   
    sub.l       d0, d5                              ;0 d-123---7 a-123-5--   8   
    divs.w      _asm_line_xlen, d5                  ;0 d-123---7 a-123-5-- 170   
    move.b      d5, .trace_s+2                      ;0 d-123---7 a-123-5--  16   subpixel step
    lsr.w       #8, d5                              ;0 d-123---7 a-123-5--  22   
    ext.w       d5                                  ;0 d-123---7 a-123-5--   4   
    move.w      d5, .trace_y0+4                     ;0 d-123---7 a-123-5--  16   pixel step
    add.l       #ENGINE_Y_MID*256+128, d0           ;0 d-123-5-7 a-123-5--  16   24.8 fixed
    lsl.l       #8, d0                              ;0 d-123-5-7 a-123-5--  24   16.16 fixed
    swap.w      d4                                  ;0 d-123-5-7 a-123-5--   4   
    move.w      d0, d4                              ;0 d-123-5-7 a-123-5--   4   
    swap.w      d4                                  ;0 d-123-5-7 a-123-5--   4   
    sub.w       d0, d0                              ;0 d-123-5-7 a-123-5--   4   
    swap.w      d0                                  ;0 d-123-5-7 a-123-5--   4   current Y value
    move.w      _asm_line_ds(pc), .trace_s+4        ;0 d-123-5-7 a-123-5--  28   
    bra         .start                              ;0 d-123-5-7 a-123-5--  10   End of header
                                                    ;0 d-123-5-7 a-123-5--       
                                                    ;0 d-123-5-7 a-123-5--       
.loop:                                              ;1 d-123-5-7 a-123-5--       ======== LOOP START ========
    addq.w      #8, a4                              ;1 d-123-5-7 a-123-5--   8   
.trace_s:                                           ;1 d-123-5-7 a-123-5--       
    add.l       #$FF00FFFF, d4                      ;1 d-123-5-7 a-123-5--  16   s1 += ds
.trace_y0:                                          ;1 d-123-5-7 a-123-5--       
    move.l      #$FF000000, d1                      ;1 d--23-5-7 a-123-5--  12   
    addx.l      d1, d0                              ;1 d--23-5-7 a-123-5--   8   
.trace_tex:                                         ;1 d-123-5-7 a-123-5--       
    add.l       #$80000000, d6                      ;1 d-123-5-7 a-123-5--  16   u += du
                                                    ;1 d-123-5-7 a-123-5--       
.start:                                             ;1 d-123-5-7 a-123-5--       ======== FIRST ITERATION STARTS HERE ========
    move.l      (a4), a7                            ;1 d-123-5-7 a-123-5--  12   byte *dst = render_buffer + offs[x];
                                                    ;1 d-123-5-7 a-123-5--       
                                                    ;1 d-123-5-7 a-123-5--       -------- ymin/ymax not checked --------
    move.w      rc_ymax(a4), d1                     ;1 d--23-5-7 a-123-5--  12   Load ymax
    cmp.w       d0, d1                              ;1 d--23-5-7 a-123-5--   4   ymax>=y0
    bge         .cmp_1                              ;1 d--23-5-7 a-123-5--  10   
                                                    ;1 d--23-5-7 a-123-5--       
                                                    ;1 d--23-5-7 a-123-5--       -------- ymin < y0 && ymax < y0 --------
                                                    ;1 d--23-5-7 a-123-5--       ======== [Upper] ========
                                                    ;1 d--23-5-7 a-123-5--       -------- [Upper] --------
                                                    ;1 d--23-5-7 a-123-5--       -- Fill routine --
    move.w      d4, d2                              ;1 d---3-5-7 a-123-5--   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d2                              ;1 d---3-5-7 a-123-5--  12   
    and.w       #$1FFC, d2                          ;1 d---3-5-7 a-123-5--   8   
    lea.l       _FN_SCALERS, a1                     ;1 d---3-5-7 a--23-5--  12   
    move.l      (a1,d2.w), a1                       ;1 d---3-5-7 a--23-5--  18   
                                                    ;1 d--23-5-7 a--23-5--       -- Texcoord (no persp) --
    move.l      d6, d2                              ;1 d---3-5-7 a--23-5--   4   word tx = u1s;
    and.w       #$1F80, d2                          ;1 d---3-5-7 a--23-5--   8   tx &= $1F80;
    move.l      _asm_tex_base(pc), a2               ;1 d---3-5-7 a---3-5--  16   -- Texture source --
    add.w       d2, a2                              ;1 d---3-5-7 a---3-5--   8   
    add.w       d1, d1                              ;1 d--23-5-7 a--23-5--   4   bottom_offset = 4*ymax
    add.w       d1, d1                              ;1 d--23-5-7 a--23-5--   4   
    move.w      #$4ED6, (a1,d1.w)                   ;1 d--23-5-7 a--23-5--  18   #$4ED6 -> (fill_routine, bottom_offset)
    swap.w      d1                                  ;1 d--23-5-7 a--23-5--   4   Select ymin
    move.w      rc_ymin(a4), d1                     ;1 d--23-5-7 a--23-5--  12   Load ymin
    add.w       d1, d1                              ;1 d--23-5-7 a--23-5--   4   top_offset = 4*ymin
    add.w       d1, d1                              ;1 d--23-5-7 a--23-5--   4   
    lea.l       .ret_2, a6                          ;1 d--23-5-7 a--23-5--  12   
    jmp         (a1,d1.w)                           ;1 d--23-5-7 a--23-5--  14   execute at (fill_routine, top_offset)
.ret_2:                                             ;1 d--23-5-7 a--23-5--       
    swap.w      d1                                  ;1 d--23-5-7 a--23-5--   4   Select ymax
    move.w      #$1EEA, (a1,d1.w)                   ;1 d--23-5-7 a--23-5--  18   #$1EEA -> (fill_routine, bottom_offset)
    clr.l       rc_ymin(a4)                         ;1 d-123-5-7 a-123-5--       Disable further rendering - clear ymin/ymax
    move.w      d4, rc_size_limit(a4)               ;1 d-123-5-7 a-123-5--   8   
    bra         .end_loop                           ;1 d-123-5-7 a-123-5--  10   
                                                    ;1 d--23-5-7 a-123-5--       
.cmp_1:                                             ;1 d--23-5-7 a-123-5--       -------- y0 <= ymax --------
    swap.w      d1                                  ;1 d--23-5-7 a-123-5--   4   Select ymin
    move.w      rc_ymin(a4), d1                     ;1 d--23-5-7 a-123-5--  12   Load ymin
    cmp.w       d0, d1                              ;1 d--23-5-7 a-123-5--   4   ymin>=y0
    bge         .cmp_3                              ;1 d--23-5-7 a-123-5--  10   
                                                    ;1 d--23-5-7 a-123-5--       
                                                    ;1 d--23-5-7 a-123-5--       -------- ymin < y0 && y0 <= ymax --------
                                                    ;1 d--23-5-7 a-123-5--       ======== [Upper gap] ========
                                                    ;1 d--23-5-7 a-123-5--       -------- [Upper --------
                                                    ;1 d--23-5-7 a-123-5--       -- Fill routine --
    move.w      d4, d2                              ;1 d---3-5-7 a-123-5--   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d2                              ;1 d---3-5-7 a-123-5--  12   
    and.w       #$1FFC, d2                          ;1 d---3-5-7 a-123-5--   8   
    lea.l       _FN_SCALERS, a1                     ;1 d---3-5-7 a--23-5--  12   
    move.l      (a1,d2.w), a1                       ;1 d---3-5-7 a--23-5--  18   
                                                    ;1 d--23-5-7 a--23-5--       -- Texcoord (no persp) --
    move.l      d6, d2                              ;1 d---3-5-7 a--23-5--   4   word tx = u1s;
    and.w       #$1F80, d2                          ;1 d---3-5-7 a--23-5--   8   tx &= $1F80;
    move.l      _asm_tex_base(pc), a2               ;1 d---3-5-7 a---3-5--  16   -- Texture source --
    add.w       d2, a2                              ;1 d---3-5-7 a---3-5--   8   
    move.w      d0, d2                              ;1 d---3-5-7 a--23-5--   4   bottom_offset = 4*yend
    add.w       d2, d2                              ;1 d---3-5-7 a--23-5--   4   
    add.w       d2, d2                              ;1 d---3-5-7 a--23-5--   4   
    move.w      #$4ED6, (a1,d2.w)                   ;1 d---3-5-7 a--23-5--  18   #$4ED6 -> (fill_routine, bottom_offset)
    add.w       d1, d1                              ;1 d---3-5-7 a--23-5--   4   top_offset = 4*ymin
    add.w       d1, d1                              ;1 d---3-5-7 a--23-5--   4   
    lea.l       .ret_4, a6                          ;1 d---3-5-7 a--23-5--  12   
    jmp         (a1,d1.w)                           ;1 d---3-5-7 a--23-5--  14   execute at (fill_routine, top_offset)
.ret_4:                                             ;1 d---3-5-7 a--23-5--       
    move.w      #$1EEA, (a1,d2.w)                   ;1 d---3-5-7 a--23-5--  18   #$1EEA -> (fill_routine, bottom_offset)
                                                    ;1 d--23-5-7 a-123-5--       -------- gap] --------
    swap.w      d1                                  ;1 d--23-5-7 a-123-5--   4   Select ymax
    move.w      d0, rc_ymin(a4)                     ;1 d-123-5-7 a-123-5--   8   Update rc_ymin with ystart
    move.l      a7, (a4)                            ;1 d-123-5-7 a-123-5--  12   Set future drawing start pointer to current pos
    bra         .end_loop                           ;1 d-123-5-7 a-123-5--  10   
                                                    ;1 d-123-5-7 a-123-5--       
.cmp_3:                                             ;1 d-123-5-7 a-123-5--       -------- y0 <= ymin && y0 <= ymax --------
                                                    ;1 d-123-5-7 a-123-5--       ======== [gap] ========
                                                    ;1 d-123-5-7 a-123-5--       -------- [gap] --------
                                                    ;1 d-123-5-7 a-123-5--       nothing to be done
    bra         .end_loop                           ;1 d-123-5-7 a-123-5--  10   
                                                    ;1 d-123-5-7 a-123-5--       
                                                    ;1 d-123-5-7 a-123-5--       
.end_loop:                                          ;1 d-123-5-7 a-123-5--       ======== LOOP END ========
    cmp.l       a0, a4                              ;1 d-123-5-7 a-123-5--   6   render_column ? render_end
    blt         .loop                               ;1 d-123-5-7 a-123-5--  10   continue if <
.end:                                               ;1 d01234567 a012345--       Cleanup & exit
    move.l      _a7_save(pc), a7                    ;1 d01234567 a012345--  16   
    rts                                             ;1 d01234567 a012345--  16   
