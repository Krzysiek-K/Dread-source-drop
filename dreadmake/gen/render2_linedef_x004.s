
    public _Dread_LineCore_x004
_Dread_LineCore_x004:

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
    move.l      line_tex_lower(a5), a1              ;0 d0123-5-7 a--23----  16   Lower texture
    move.w      _view_pos_z, d0                     ;0 d-123-5-7 a--23----  16   
    asr.w       #1, d0                              ;0 d-123-5-7 a--23----   8   
    add.w       d0, a1                              ;0 d-123-5-7 a--23----   8   
    move.l      a1, _asm_tex_base2                  ;0 d0123-5-7 a--23----  20   
                                                    ;0 d0123-5-7 a-123----       
                                                    ;0 d0123-5-7 a-123----       ======== init .trace_y0 tracer ========
    move.w      d4, d0                              ;0 d-123-5-7 a-123----   4   compute starting Y
    lsr.w       #5, d0                              ;0 d-123-5-7 a-123----  16   
    move.w      line_y1(a5), d1                     ;0 d--23-5-7 a-123----  12   
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
    move.w      line_y2(a5), d2                     ;0 d---3-5-7 a-123----  12   
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
                                                    ;0 d--23-5-7 a-123----       
                                                    ;0 d--23-5-7 a-123----       ======== init .trace_y2 tracer ========
    move.w      d4, d2                              ;0 d---3-5-7 a-123----   4   compute starting Y
    lsr.w       #5, d2                              ;0 d---3-5-7 a-123----  16   
    move.w      line_y3(a5), d3                     ;0 d-----5-7 a-123----  12   
    sub.w       _view_pos_z, d3                     ;0 d-----5-7 a-123----  16   
    muls.w      d3, d2                              ;0 d-----5-7 a-123----  70   
    move.w      _asm_line_s2, d5                    ;0 d-------7 a-123----  16   compute Y slope
    lsr.w       #5, d5                              ;0 d-------7 a-123----  16   
    muls.w      d3, d5                              ;0 d-------7 a-123----  70   
    sub.l       d2, d5                              ;0 d---3---7 a-123----   8   
    divs.w      _asm_line_xlen, d5                  ;0 d---3---7 a-123---- 170   
    move.b      d5, .trace_y1+2                     ;0 d---3---7 a-123----  16   subpixel step
    lsr.w       #8, d5                              ;0 d---3---7 a-123----  22   
    ext.w       d5                                  ;0 d---3---7 a-123----   4   
    move.w      d5, .trace_y2+4                     ;0 d---3---7 a-123----  16   pixel step
    add.l       #ENGINE_Y_MID*256+128, d2           ;0 d---3-5-7 a-123----  16   24.8 fixed
    lsl.l       #8, d2                              ;0 d---3-5-7 a-123----  24   16.16 fixed
    swap.w      d1                                  ;0 d---3-5-7 a-123----   4   
    move.w      d2, d1                              ;0 d---3-5-7 a-123----   4   
    swap.w      d1                                  ;0 d---3-5-7 a-123----   4   
    sub.w       d2, d2                              ;0 d---3-5-7 a-123----   4   
    swap.w      d2                                  ;0 d---3-5-7 a-123----   4   current Y value
                                                    ;0 d---3-5-7 a-123----       
                                                    ;0 d---3-5-7 a-123----       ======== init .trace_y3 tracer ========
    move.w      d4, d3                              ;0 d-----5-7 a-123----   4   compute starting Y
    lsr.w       #5, d3                              ;0 d-----5-7 a-123----  16   
    move.w      line_y4(a5), d7                     ;0 d-----5-- a-123----  12   
    sub.w       _view_pos_z, d7                     ;0 d-----5-- a-123----  16   
    muls.w      d7, d3                              ;0 d-----5-- a-123----  70   
    move.w      _asm_line_s2, d5                    ;0 d-------- a-123----  16   compute Y slope
    lsr.w       #5, d5                              ;0 d-------- a-123----  16   
    muls.w      d7, d5                              ;0 d-------- a-123----  70   
    sub.l       d3, d5                              ;0 d-------7 a-123----   8   
    divs.w      _asm_line_xlen, d5                  ;0 d-------7 a-123---- 170   
    move.b      d5, .trace_y2+2                     ;0 d-------7 a-123----  16   subpixel step
    lsr.w       #8, d5                              ;0 d-------7 a-123----  22   
    ext.w       d5                                  ;0 d-------7 a-123----   4   
    move.w      d5, .trace_y3+4                     ;0 d-------7 a-123----  16   pixel step
    add.l       #ENGINE_Y_MID*256+128, d3           ;0 d-----5-7 a-123----  16   24.8 fixed
    lsl.l       #8, d3                              ;0 d-----5-7 a-123----  24   16.16 fixed
    swap.w      d2                                  ;0 d-----5-7 a-123----   4   
    move.w      d3, d2                              ;0 d-----5-7 a-123----   4   
    swap.w      d2                                  ;0 d-----5-7 a-123----   4   
    sub.w       d3, d3                              ;0 d-----5-7 a-123----   4   
    swap.w      d3                                  ;0 d-----5-7 a-123----   4   current Y value
    move.w      _asm_line_ds(pc), .trace_s+4        ;0 d-----5-7 a-123----  28   
    bra         .start                              ;0 d-----5-7 a-123----  10   End of header
                                                    ;0 d-----5-7 a-123----       
                                                    ;0 d-----5-7 a-123----       
.loop:                                              ;1 d-----5-7 a-123----       ======== LOOP START ========
    addq.w      #8, a4                              ;1 d-----5-7 a-123----   8   
.trace_s:                                           ;1 d-----5-7 a-123----       
    add.l       #$FF00FFFF, d4                      ;1 d-----5-7 a-123----  16   s1 += ds
.trace_y0:                                          ;1 d-----5-7 a-123----       
    move.l      #$FF000000, d7                      ;1 d-----5-- a-123----  12   
    addx.l      d7, d0                              ;1 d-----5-- a-123----   8   
.trace_y1:                                          ;1 d-----5-7 a-123----       
    move.l      #$FF000000, d7                      ;1 d-----5-- a-123----  12   
    addx.l      d7, d1                              ;1 d-----5-- a-123----   8   
.trace_y2:                                          ;1 d-----5-7 a-123----       
    move.l      #$FF000000, d7                      ;1 d-----5-- a-123----  12   
    addx.l      d7, d2                              ;1 d-----5-- a-123----   8   
.trace_y3:                                          ;1 d-----5-7 a-123----       
    move.l      #$FF000000, d7                      ;1 d-----5-- a-123----  12   
    addx.l      d7, d3                              ;1 d-----5-- a-123----   8   
.trace_tex:                                         ;1 d-----5-7 a-123----       
    add.l       #$80000000, d6                      ;1 d-----5-7 a-123----  16   u += du
                                                    ;1 d-----5-7 a-123----       
.start:                                             ;1 d-----5-7 a-123----       ======== FIRST ITERATION STARTS HERE ========
    move.l      (a4), a7                            ;1 d-----5-7 a-123----  12   byte *dst = render_buffer + offs[x];
                                                    ;1 d-----5-7 a-123----       
                                                    ;1 d-----5-7 a-123----       -------- ymin/ymax not checked --------
    move.w      rc_ymax(a4), d7                     ;1 d-----5-- a-123----  12   Load ymax
    cmp.w       d2, d7                              ;1 d-----5-- a-123----   4   ymax>=y2
    bge         .cmp_1                              ;1 d-----5-- a-123----  10   
                                                    ;1 d-----5-- a-123----       
                                                    ;1 d-----5-- a-123----       -------- ymin < y2 && ymax < y2 --------
    cmp.w       d1, d7                              ;1 d-----5-- a-123----   4   ymax>=y1
    bge         .cmp_2                              ;1 d-----5-- a-123----  10   
                                                    ;1 d-----5-- a-123----       
                                                    ;1 d-----5-- a-123----       -------- ymin < y1 && ymax < y1 --------
    cmp.w       d0, d7                              ;1 d-----5-- a-123----   4   ymax>=y0
    bge         .cmp_3                              ;1 d-----5-- a-123----  10   
                                                    ;1 d-----5-- a-123----       
                                                    ;1 d-----5-- a-123----       -------- ymin < y0 && ymax < y0 --------
                                                    ;1 d-----5-- a-123----       ======== [Ceil] ========
                                                    ;1 d-----5-- a-123----       -------- [Ceil] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;1 d-----5-- a--23----  12   Color filler func
    swap.w      d7                                  ;1 d-----5-- a--23----   4   Select ymin
    move.w      rc_ymin(a4), d7                     ;1 d-----5-- a--23----  12   Load ymin
    move.w      d7, d5                              ;1 d-------- a--23----   4   
    swap.w      d7                                  ;1 d-------- a--23----   4   Select ymax
    sub.w       d7, d5                              ;1 d-------- a--23----   4   
    move.w      d5, d7                              ;1 d-------- a--23----   4   avoid color reg clash
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   
    move.b      line_ceil_col(a5), d5               ;1 d-------- a--23----  12   
    lea.l       .ret_4, a6                          ;1 d-----5-- a--23----  12   
    jmp         (a1,d7.w)                           ;1 d-----5-- a--23----  14   
.ret_4:                                             ;1 d-----5-7 a-123----       
    clr.l       rc_ymin(a4)                         ;1 d-----5-7 a-123----       Disable further rendering - clear ymin/ymax
    move.w      d4, rc_size_limit(a4)               ;1 d-----5-7 a-123----   8   
    bra         .end_loop                           ;1 d-----5-7 a-123----  10   
                                                    ;1 d-----5-- a-123----       
.cmp_3:                                             ;1 d-----5-- a-123----       -------- ymin < y1 && y0 <= ymax < y1 --------
    swap.w      d7                                  ;1 d-----5-- a-123----   4   Select ymin
    move.w      rc_ymin(a4), d7                     ;1 d-----5-- a-123----  12   Load ymin
    cmp.w       d0, d7                              ;1 d-----5-- a-123----   4   ymin>=y0
    bge         .cmp_5                              ;1 d-----5-- a-123----  10   
                                                    ;1 d-----5-- a-123----       
                                                    ;1 d-----5-- a-123----       -------- ymin < y0 && y0 <= ymax < y1 --------
                                                    ;1 d-----5-- a-123----       ======== [Ceil Upper] ========
                                                    ;1 d-----5-- a-123----       -------- [Ceil --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;1 d-----5-- a--23----  12   Color filler func
    sub.w       d0, d7                              ;1 d-----5-- a--23----   4   
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   
    move.b      line_ceil_col(a5), d5               ;1 d-------- a--23----  12   
    lea.l       .ret_6, a6                          ;1 d-----5-- a--23----  12   
    jmp         (a1,d7.w)                           ;1 d-----5-- a--23----  14   
.ret_6:                                             ;1 d-----5-- a-123----       
                                                    ;1 d-----5-- a-123----       -------- Upper] --------
                                                    ;1 d-----5-- a-123----       -- Fill routine --
    move.w      d4, d5                              ;1 d-------- a-123----   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d5                              ;1 d-------- a-123----  12   
    and.w       #$1FFC, d5                          ;1 d-------- a-123----   8   
    lea.l       _FN_SCALERS, a3                     ;1 d-------- a-12-----  12   
    move.l      (a3,d5.w), a3                       ;1 d-------- a-12-----  18   
                                                    ;1 d-----5-- a-12-----       -- Texcoord (no persp) --
    move.l      d6, d5                              ;1 d-------- a-12-----   4   word tx = u1s;
    and.w       #$1F80, d5                          ;1 d-------- a-12-----   8   tx &= $1F80;
    move.l      _asm_tex_base(pc), a2               ;1 d-------- a-1------  16   -- Texture source --
    add.w       d5, a2                              ;1 d-------- a-1------   8   
    swap.w      d7                                  ;1 d-----5-- a-12-----   4   Select ymax
    add.w       d7, d7                              ;1 d-----5-- a-12-----   4   bottom_offset = 4*ymax
    add.w       d7, d7                              ;1 d-----5-- a-12-----   4   
    move.w      #$4ED6, (a3,d7.w)                   ;1 d-----5-- a-12-----  18   #$4ED6 -> (fill_routine, bottom_offset)
    move.w      d0, d5                              ;1 d-------- a-12-----   4   top_offset = 4*ystart
    add.w       d5, d5                              ;1 d-------- a-12-----   4   
    add.w       d5, d5                              ;1 d-------- a-12-----   4   
    lea.l       .ret_7, a6                          ;1 d-------- a-12-----  12   
    jmp         (a3,d5.w)                           ;1 d-------- a-12-----  14   execute at (fill_routine, top_offset)
.ret_7:                                             ;1 d-----5-- a-12-----       
    move.w      #$1EEA, (a3,d7.w)                   ;1 d-----5-- a-12-----  18   #$1EEA -> (fill_routine, bottom_offset)
    clr.l       rc_ymin(a4)                         ;1 d-----5-7 a-123----       Disable further rendering - clear ymin/ymax
    move.w      d4, rc_size_limit(a4)               ;1 d-----5-7 a-123----   8   
    bra         .end_loop                           ;1 d-----5-7 a-123----  10   
                                                    ;1 d-----5-- a-123----       
.cmp_5:                                             ;1 d-----5-- a-123----       -------- y0 <= ymin < y1 && y0 <= ymax < y1 --------
                                                    ;1 d-----5-- a-123----       ======== [Upper] ========
                                                    ;1 d-----5-- a-123----       -------- [Upper] --------
                                                    ;1 d-----5-- a-123----       -- Fill routine --
    move.w      d4, d5                              ;1 d-------- a-123----   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d5                              ;1 d-------- a-123----  12   
    and.w       #$1FFC, d5                          ;1 d-------- a-123----   8   
    lea.l       _FN_SCALERS, a1                     ;1 d-------- a--23----  12   
    move.l      (a1,d5.w), a1                       ;1 d-------- a--23----  18   
                                                    ;1 d-----5-- a--23----       -- Texcoord (no persp) --
    move.l      d6, d5                              ;1 d-------- a--23----   4   word tx = u1s;
    and.w       #$1F80, d5                          ;1 d-------- a--23----   8   tx &= $1F80;
    move.l      _asm_tex_base(pc), a2               ;1 d-------- a---3----  16   -- Texture source --
    add.w       d5, a2                              ;1 d-------- a---3----   8   
    swap.w      d7                                  ;1 d-----5-- a--23----   4   Select ymax
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   bottom_offset = 4*ymax
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   
    move.w      #$4ED6, (a1,d7.w)                   ;1 d-----5-- a--23----  18   #$4ED6 -> (fill_routine, bottom_offset)
    swap.w      d7                                  ;1 d-----5-- a--23----   4   Select ymin
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   top_offset = 4*ymin
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   
    lea.l       .ret_8, a6                          ;1 d-----5-- a--23----  12   
    jmp         (a1,d7.w)                           ;1 d-----5-- a--23----  14   execute at (fill_routine, top_offset)
.ret_8:                                             ;1 d-----5-- a--23----       
    swap.w      d7                                  ;1 d-----5-- a--23----   4   Select ymax
    move.w      #$1EEA, (a1,d7.w)                   ;1 d-----5-- a--23----  18   #$1EEA -> (fill_routine, bottom_offset)
    clr.l       rc_ymin(a4)                         ;1 d-----5-7 a-123----       Disable further rendering - clear ymin/ymax
    move.w      d4, rc_size_limit(a4)               ;1 d-----5-7 a-123----   8   
    bra         .end_loop                           ;1 d-----5-7 a-123----  10   
                                                    ;1 d-----5-- a-123----       
.cmp_2:                                             ;1 d-----5-- a-123----       -------- ymin < y2 && y1 <= ymax < y2 --------
    swap.w      d7                                  ;1 d-----5-- a-123----   4   Select ymin
    move.w      rc_ymin(a4), d7                     ;1 d-----5-- a-123----  12   Load ymin
    cmp.w       d0, d7                              ;1 d-----5-- a-123----   4   ymin>=y0
    bge         .cmp_9                              ;1 d-----5-- a-123----  10   
                                                    ;1 d-----5-- a-123----       
                                                    ;1 d-----5-- a-123----       -------- ymin < y0 && y1 <= ymax < y2 --------
                                                    ;1 d-----5-- a-123----       ======== [Ceil Upper gap] ========
                                                    ;1 d-----5-- a-123----       -------- [Ceil --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;1 d-----5-- a--23----  12   Color filler func
    sub.w       d0, d7                              ;1 d-----5-- a--23----   4   
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   
    move.b      line_ceil_col(a5), d5               ;1 d-------- a--23----  12   
    lea.l       .ret_10, a6                         ;1 d-----5-- a--23----  12   
    jmp         (a1,d7.w)                           ;1 d-----5-- a--23----  14   
.ret_10:                                            ;1 d-----5-7 a-123----       
                                                    ;1 d-----5-7 a-123----       -------- Upper --------
                                                    ;1 d-----5-7 a-123----       -- Fill routine --
    move.w      d4, d5                              ;1 d-------7 a-123----   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d5                              ;1 d-------7 a-123----  12   
    and.w       #$1FFC, d5                          ;1 d-------7 a-123----   8   
    lea.l       _FN_SCALERS, a3                     ;1 d-------7 a-12-----  12   
    move.l      (a3,d5.w), a3                       ;1 d-------7 a-12-----  18   
                                                    ;1 d-----5-7 a-12-----       -- Texcoord (no persp) --
    move.l      d6, d5                              ;1 d-------7 a-12-----   4   word tx = u1s;
    and.w       #$1F80, d5                          ;1 d-------7 a-12-----   8   tx &= $1F80;
    move.l      _asm_tex_base(pc), a2               ;1 d-------7 a-1------  16   -- Texture source --
    add.w       d5, a2                              ;1 d-------7 a-1------   8   
    move.w      d1, d5                              ;1 d-------7 a-12-----   4   bottom_offset = 4*yend
    add.w       d5, d5                              ;1 d-------7 a-12-----   4   
    add.w       d5, d5                              ;1 d-------7 a-12-----   4   
    move.w      #$4ED6, (a3,d5.w)                   ;1 d-------7 a-12-----  18   #$4ED6 -> (fill_routine, bottom_offset)
    move.w      d0, d7                              ;1 d-------- a-12-----   4   top_offset = 4*ystart
    add.w       d7, d7                              ;1 d-------- a-12-----   4   
    add.w       d7, d7                              ;1 d-------- a-12-----   4   
    lea.l       .ret_11, a6                         ;1 d-------- a-12-----  12   
    jmp         (a3,d7.w)                           ;1 d-------- a-12-----  14   execute at (fill_routine, top_offset)
.ret_11:                                            ;1 d-------- a-12-----       
    move.w      #$1EEA, (a3,d5.w)                   ;1 d-------- a-12-----  18   #$1EEA -> (fill_routine, bottom_offset)
                                                    ;1 d-----5-- a-123----       -------- gap] --------
    swap.w      d7                                  ;1 d-----5-- a-123----   4   Select ymax
    move.w      d1, rc_ymin(a4)                     ;1 d-----5-7 a-123----   8   Update rc_ymin with ystart
    move.l      a7, (a4)                            ;1 d-----5-7 a-123----  12   Set future drawing start pointer to current pos
    bra         .end_loop                           ;1 d-----5-7 a-123----  10   
                                                    ;1 d-----5-- a-123----       
.cmp_9:                                             ;1 d-----5-- a-123----       -------- y0 <= ymin < y2 && y1 <= ymax < y2 --------
    cmp.w       d1, d7                              ;1 d-----5-- a-123----   4   ymin>=y1
    bge         .cmp_12                             ;1 d-----5-- a-123----  10   
                                                    ;1 d-----5-- a-123----       
                                                    ;1 d-----5-- a-123----       -------- y0 <= ymin < y1 && y1 <= ymax < y2 --------
                                                    ;1 d-----5-- a-123----       ======== [Upper gap] ========
                                                    ;1 d-----5-- a-123----       -------- [Upper --------
                                                    ;1 d-----5-- a-123----       -- Fill routine --
    move.w      d4, d5                              ;1 d-------- a-123----   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d5                              ;1 d-------- a-123----  12   
    and.w       #$1FFC, d5                          ;1 d-------- a-123----   8   
    lea.l       _FN_SCALERS, a1                     ;1 d-------- a--23----  12   
    move.l      (a1,d5.w), a1                       ;1 d-------- a--23----  18   
                                                    ;1 d-----5-- a--23----       -- Texcoord (no persp) --
    move.l      d6, d5                              ;1 d-------- a--23----   4   word tx = u1s;
    and.w       #$1F80, d5                          ;1 d-------- a--23----   8   tx &= $1F80;
    move.l      _asm_tex_base(pc), a2               ;1 d-------- a---3----  16   -- Texture source --
    add.w       d5, a2                              ;1 d-------- a---3----   8   
    move.w      d1, d5                              ;1 d-------- a--23----   4   bottom_offset = 4*yend
    add.w       d5, d5                              ;1 d-------- a--23----   4   
    add.w       d5, d5                              ;1 d-------- a--23----   4   
    move.w      #$4ED6, (a1,d5.w)                   ;1 d-------- a--23----  18   #$4ED6 -> (fill_routine, bottom_offset)
    add.w       d7, d7                              ;1 d-------- a--23----   4   top_offset = 4*ymin
    add.w       d7, d7                              ;1 d-------- a--23----   4   
    lea.l       .ret_13, a6                         ;1 d-------- a--23----  12   
    jmp         (a1,d7.w)                           ;1 d-------- a--23----  14   execute at (fill_routine, top_offset)
.ret_13:                                            ;1 d-------- a--23----       
    move.w      #$1EEA, (a1,d5.w)                   ;1 d-------- a--23----  18   #$1EEA -> (fill_routine, bottom_offset)
                                                    ;1 d-----5-- a-123----       -------- gap] --------
    swap.w      d7                                  ;1 d-----5-- a-123----   4   Select ymax
    move.w      d1, rc_ymin(a4)                     ;1 d-----5-7 a-123----   8   Update rc_ymin with ystart
    move.l      a7, (a4)                            ;1 d-----5-7 a-123----  12   Set future drawing start pointer to current pos
    bra         .end_loop                           ;1 d-----5-7 a-123----  10   
                                                    ;1 d-----5-7 a-123----       
.cmp_12:                                            ;1 d-----5-7 a-123----       -------- y1 <= ymin < y2 && y1 <= ymax < y2 --------
                                                    ;1 d-----5-7 a-123----       ======== [gap] ========
                                                    ;1 d-----5-7 a-123----       -------- [gap] --------
                                                    ;1 d-----5-7 a-123----       nothing to be done
    bra         .end_loop                           ;1 d-----5-7 a-123----  10   
                                                    ;1 d-----5-- a-123----       
.cmp_1:                                             ;1 d-----5-- a-123----       -------- y2 <= ymax --------
    swap.w      d7                                  ;1 d-----5-- a-123----   4   Select ymin
    move.w      rc_ymin(a4), d7                     ;1 d-----5-- a-123----  12   Load ymin
    cmp.w       d2, d7                              ;1 d-----5-- a-123----   4   ymin>=y2
    bge         .cmp_14                             ;1 d-----5-- a-123----  10   
                                                    ;1 d-----5-- a-123----       
                                                    ;1 d-----5-- a-123----       -------- ymin < y2 && y2 <= ymax --------
    cmp.w       d0, d7                              ;1 d-----5-- a-123----   4   ymin>=y0
    bge         .cmp_15                             ;1 d-----5-- a-123----  10   
                                                    ;1 d-----5-- a-123----       
                                                    ;1 d-----5-- a-123----       -------- ymin < y0 && y2 <= ymax --------
    swap.w      d7                                  ;1 d-----5-- a-123----   4   Select ymax
    cmp.w       d3, d7                              ;1 d-----5-- a-123----   4   ymax>=y3
    bge         .cmp_16                             ;1 d-----5-- a-123----  10   
                                                    ;1 d-----5-- a-123----       
                                                    ;1 d-----5-- a-123----       -------- ymin < y0 && y2 <= ymax < y3 --------
                                                    ;1 d-----5-- a-123----       ======== [Ceil Upper gap Lower] ========
                                                    ;1 d-----5-- a-123----       -------- [Ceil --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;1 d-----5-- a--23----  12   Color filler func
    swap.w      d7                                  ;1 d-----5-- a--23----   4   Select ymin
    sub.w       d0, d7                              ;1 d-----5-- a--23----   4   
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   
    move.b      line_ceil_col(a5), d5               ;1 d-------- a--23----  12   
    lea.l       .ret_17, a6                         ;1 d-----5-- a--23----  12   
    jmp         (a1,d7.w)                           ;1 d-----5-- a--23----  14   
.ret_17:                                            ;1 d-----5-7 a-123----       
                                                    ;1 d-----5-7 a-123----       -------- Upper --------
                                                    ;1 d-----5-7 a-123----       -- Fill routine --
    move.w      d4, d5                              ;1 d-------7 a-123----   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d5                              ;1 d-------7 a-123----  12   
    and.w       #$1FFC, d5                          ;1 d-------7 a-123----   8   
    lea.l       _FN_SCALERS, a3                     ;1 d-------7 a-12-----  12   
    move.l      (a3,d5.w), a3                       ;1 d-------7 a-12-----  18   
                                                    ;1 d-----5-7 a-12-----       -- Texcoord (no persp) --
    move.l      d6, d5                              ;1 d-------7 a-12-----   4   word tx = u1s;
    and.w       #$1F80, d5                          ;1 d-------7 a-12-----   8   tx &= $1F80;
    move.l      _asm_tex_base(pc), a2               ;1 d-------7 a-1------  16   -- Texture source --
    add.w       d5, a2                              ;1 d-------7 a-1------   8   
    move.w      d1, d5                              ;1 d-------7 a-12-----   4   bottom_offset = 4*yend
    add.w       d5, d5                              ;1 d-------7 a-12-----   4   
    add.w       d5, d5                              ;1 d-------7 a-12-----   4   
    move.w      #$4ED6, (a3,d5.w)                   ;1 d-------7 a-12-----  18   #$4ED6 -> (fill_routine, bottom_offset)
    move.w      d0, d7                              ;1 d-------- a-12-----   4   top_offset = 4*ystart
    add.w       d7, d7                              ;1 d-------- a-12-----   4   
    add.w       d7, d7                              ;1 d-------- a-12-----   4   
    lea.l       .ret_18, a6                         ;1 d-------- a-12-----  12   
    jmp         (a3,d7.w)                           ;1 d-------- a-12-----  14   execute at (fill_routine, top_offset)
.ret_18:                                            ;1 d-------- a-12-----       
    move.w      #$1EEA, (a3,d5.w)                   ;1 d-------- a-12-----  18   #$1EEA -> (fill_routine, bottom_offset)
                                                    ;1 d-----5-- a-123----       -------- gap --------
    move.w      d1, rc_ymin(a4)                     ;1 d-----5-- a-123----   8   Update rc_ymin with ystart
    move.l      a7, (a4)                            ;1 d-----5-- a-123----  12   Set future drawing start pointer to current pos
    move.w      d2, rc_ymax(a4)                     ;1 d-----5-- a-123----   8   Update rc_ymax with yend
    move.w      d2, d5                              ;1 d-------- a-123----   4   Add 2*(yend - ystart) to A7
    sub.w       d1, d5                              ;1 d-------- a-123----   4   
    add.w       d5, d5                              ;1 d-------- a-123----   4   
    add.w       d5, a7                              ;1 d-------- a-123----   8   
                                                    ;1 d-----5-- a-123----       -------- Lower] --------
                                                    ;1 d-----5-- a-123----       -- Fill routine --
    move.w      d4, d5                              ;1 d-------- a-123----   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d5                              ;1 d-------- a-123----  12   
    and.w       #$1FFC, d5                          ;1 d-------- a-123----   8   
    lea.l       _FN_SCALERS, a3                     ;1 d-------- a-12-----  12   
    move.l      (a3,d5.w), a3                       ;1 d-------- a-12-----  18   
                                                    ;1 d-----5-- a-12-----       -- Texcoord (no persp) --
    move.l      d6, d5                              ;1 d-------- a-12-----   4   word tx = u1s;
    and.w       #$1F80, d5                          ;1 d-------- a-12-----   8   tx &= $1F80;
    move.l      _asm_tex_base2(pc), a2              ;1 d-------- a-1------  16   -- Texture source --
    add.w       d5, a2                              ;1 d-------- a-1------   8   
    swap.w      d7                                  ;1 d-----5-- a-12-----   4   Select ymax
    add.w       d7, d7                              ;1 d-----5-- a-12-----   4   bottom_offset = 4*ymax
    add.w       d7, d7                              ;1 d-----5-- a-12-----   4   
    move.w      #$4ED6, (a3,d7.w)                   ;1 d-----5-- a-12-----  18   #$4ED6 -> (fill_routine, bottom_offset)
    move.w      d2, d5                              ;1 d-------- a-12-----   4   top_offset = 4*ystart
    add.w       d5, d5                              ;1 d-------- a-12-----   4   
    add.w       d5, d5                              ;1 d-------- a-12-----   4   
    lea.l       .ret_19, a6                         ;1 d-------- a-12-----  12   
    jmp         (a3,d5.w)                           ;1 d-------- a-12-----  14   execute at (fill_routine, top_offset)
.ret_19:                                            ;1 d-----5-- a-12-----       
    move.w      #$1EEA, (a3,d7.w)                   ;1 d-----5-- a-12-----  18   #$1EEA -> (fill_routine, bottom_offset)
    bra         .end_loop                           ;1 d-----5-7 a-123----  10   
                                                    ;1 d-----5-- a-123----       
.cmp_16:                                            ;1 d-----5-- a-123----       -------- ymin < y0 && y3 <= ymax --------
                                                    ;1 d-----5-- a-123----       ======== [Ceil Upper gap Lower Floor] ========
                                                    ;1 d-----5-- a-123----       -------- [Ceil --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;1 d-----5-- a--23----  12   Color filler func
    swap.w      d7                                  ;1 d-----5-- a--23----   4   Select ymin
    sub.w       d0, d7                              ;1 d-----5-- a--23----   4   
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   
    move.b      line_ceil_col(a5), d5               ;1 d-------- a--23----  12   
    lea.l       .ret_20, a6                         ;1 d-----5-- a--23----  12   
    jmp         (a1,d7.w)                           ;1 d-----5-- a--23----  14   
.ret_20:                                            ;1 d-----5-7 a--23----       
                                                    ;1 d-----5-7 a--23----       -------- Upper --------
                                                    ;1 d-----5-7 a--23----       -- Fill routine --
    move.w      d4, d5                              ;1 d-------7 a--23----   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d5                              ;1 d-------7 a--23----  12   
    and.w       #$1FFC, d5                          ;1 d-------7 a--23----   8   
    lea.l       _FN_SCALERS, a3                     ;1 d-------7 a--2-----  12   
    move.l      (a3,d5.w), a3                       ;1 d-------7 a--2-----  18   
                                                    ;1 d-----5-7 a--2-----       -- Texcoord (no persp) --
    move.l      d6, d5                              ;1 d-------7 a--2-----   4   word tx = u1s;
    and.w       #$1F80, d5                          ;1 d-------7 a--2-----   8   tx &= $1F80;
    move.l      _asm_tex_base(pc), a2               ;1 d-------7 a--------  16   -- Texture source --
    add.w       d5, a2                              ;1 d-------7 a--------   8   
    move.w      d1, d5                              ;1 d-------7 a--2-----   4   bottom_offset = 4*yend
    add.w       d5, d5                              ;1 d-------7 a--2-----   4   
    add.w       d5, d5                              ;1 d-------7 a--2-----   4   
    move.w      #$4ED6, (a3,d5.w)                   ;1 d-------7 a--2-----  18   #$4ED6 -> (fill_routine, bottom_offset)
    move.w      d0, d7                              ;1 d-------- a--2-----   4   top_offset = 4*ystart
    add.w       d7, d7                              ;1 d-------- a--2-----   4   
    add.w       d7, d7                              ;1 d-------- a--2-----   4   
    lea.l       .ret_21, a6                         ;1 d-------- a--2-----  12   
    jmp         (a3,d7.w)                           ;1 d-------- a--2-----  14   execute at (fill_routine, top_offset)
.ret_21:                                            ;1 d-------7 a--2-----       
    move.w      #$1EEA, (a3,d5.w)                   ;1 d-------7 a--2-----  18   #$1EEA -> (fill_routine, bottom_offset)
                                                    ;1 d-----5-7 a--23----       -------- gap --------
    move.w      d1, rc_ymin(a4)                     ;1 d-----5-7 a--23----   8   Update rc_ymin with ystart
    move.l      a7, (a4)                            ;1 d-----5-7 a--23----  12   Set future drawing start pointer to current pos
    move.w      d2, rc_ymax(a4)                     ;1 d-----5-7 a--23----   8   Update rc_ymax with yend
    move.w      d2, d5                              ;1 d-------7 a--23----   4   Add 2*(yend - ystart) to A7
    sub.w       d1, d5                              ;1 d-------7 a--23----   4   
    add.w       d5, d5                              ;1 d-------7 a--23----   4   
    add.w       d5, a7                              ;1 d-------7 a--23----   8   
                                                    ;1 d-----5-7 a--23----       -------- Lower --------
                                                    ;1 d-----5-7 a--23----       -- Fill routine --
    move.w      d4, d5                              ;1 d-------7 a--23----   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d5                              ;1 d-------7 a--23----  12   
    and.w       #$1FFC, d5                          ;1 d-------7 a--23----   8   
    lea.l       _FN_SCALERS, a3                     ;1 d-------7 a--2-----  12   
    move.l      (a3,d5.w), a3                       ;1 d-------7 a--2-----  18   
                                                    ;1 d-----5-7 a--2-----       -- Texcoord (no persp) --
    move.l      d6, d5                              ;1 d-------7 a--2-----   4   word tx = u1s;
    and.w       #$1F80, d5                          ;1 d-------7 a--2-----   8   tx &= $1F80;
    move.l      _asm_tex_base2(pc), a2              ;1 d-------7 a--------  16   -- Texture source --
    add.w       d5, a2                              ;1 d-------7 a--------   8   
    move.w      d3, d5                              ;1 d-------7 a--2-----   4   bottom_offset = 4*yend
    add.w       d5, d5                              ;1 d-------7 a--2-----   4   
    add.w       d5, d5                              ;1 d-------7 a--2-----   4   
    move.w      #$4ED6, (a3,d5.w)                   ;1 d-------7 a--2-----  18   #$4ED6 -> (fill_routine, bottom_offset)
    move.w      d2, d7                              ;1 d-------- a--2-----   4   top_offset = 4*ystart
    add.w       d7, d7                              ;1 d-------- a--2-----   4   
    add.w       d7, d7                              ;1 d-------- a--2-----   4   
    lea.l       .ret_22, a6                         ;1 d-------- a--2-----  12   
    jmp         (a3,d7.w)                           ;1 d-------- a--2-----  14   execute at (fill_routine, top_offset)
.ret_22:                                            ;1 d-------- a--2-----       
    move.w      #$1EEA, (a3,d5.w)                   ;1 d-------- a--2-----  18   #$1EEA -> (fill_routine, bottom_offset)
                                                    ;1 d-----5-- a--23----       -------- Floor] --------
    swap.w      d7                                  ;1 d-----5-- a--23----   4   Select ymax
    sub.w       d3, d7                              ;1 d-----5-- a--23----   4   
    neg.w       d7                                  ;1 d-----5-- a--23----   4   
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   
    move.b      line_floor_col(a5), d5              ;1 d-------- a--23----  12   
    lea.l       .ret_23, a6                         ;1 d-----5-- a--23----  12   
    jmp         (a1,d7.w)                           ;1 d-----5-- a--23----  14   
.ret_23:                                            ;1 d-----5-7 a-123----       
    bra         .end_loop                           ;1 d-----5-7 a-123----  10   
                                                    ;1 d-----5-- a-123----       
.cmp_15:                                            ;1 d-----5-- a-123----       -------- y0 <= ymin < y2 && y2 <= ymax --------
    swap.w      d7                                  ;1 d-----5-- a-123----   4   Select ymax
    cmp.w       d3, d7                              ;1 d-----5-- a-123----   4   ymax>=y3
    bge         .cmp_24                             ;1 d-----5-- a-123----  10   
                                                    ;1 d-----5-- a-123----       
                                                    ;1 d-----5-- a-123----       -------- y0 <= ymin < y2 && y2 <= ymax < y3 --------
    swap.w      d7                                  ;1 d-----5-- a-123----   4   Select ymin
    cmp.w       d1, d7                              ;1 d-----5-- a-123----   4   ymin>=y1
    bge         .cmp_25                             ;1 d-----5-- a-123----  10   
                                                    ;1 d-----5-- a-123----       
                                                    ;1 d-----5-- a-123----       -------- y0 <= ymin < y1 && y2 <= ymax < y3 --------
                                                    ;1 d-----5-- a-123----       ======== [Upper gap Lower] ========
                                                    ;1 d-----5-- a-123----       -------- [Upper --------
                                                    ;1 d-----5-- a-123----       -- Fill routine --
    move.w      d4, d5                              ;1 d-------- a-123----   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d5                              ;1 d-------- a-123----  12   
    and.w       #$1FFC, d5                          ;1 d-------- a-123----   8   
    lea.l       _FN_SCALERS, a1                     ;1 d-------- a--23----  12   
    move.l      (a1,d5.w), a1                       ;1 d-------- a--23----  18   
                                                    ;1 d-----5-- a--23----       -- Texcoord (no persp) --
    move.l      d6, d5                              ;1 d-------- a--23----   4   word tx = u1s;
    and.w       #$1F80, d5                          ;1 d-------- a--23----   8   tx &= $1F80;
    move.l      _asm_tex_base(pc), a2               ;1 d-------- a---3----  16   -- Texture source --
    add.w       d5, a2                              ;1 d-------- a---3----   8   
    move.w      d1, d5                              ;1 d-------- a--23----   4   bottom_offset = 4*yend
    add.w       d5, d5                              ;1 d-------- a--23----   4   
    add.w       d5, d5                              ;1 d-------- a--23----   4   
    move.w      #$4ED6, (a1,d5.w)                   ;1 d-------- a--23----  18   #$4ED6 -> (fill_routine, bottom_offset)
    add.w       d7, d7                              ;1 d-------- a--23----   4   top_offset = 4*ymin
    add.w       d7, d7                              ;1 d-------- a--23----   4   
    lea.l       .ret_26, a6                         ;1 d-------- a--23----  12   
    jmp         (a1,d7.w)                           ;1 d-------- a--23----  14   execute at (fill_routine, top_offset)
.ret_26:                                            ;1 d-------- a--23----       
    move.w      #$1EEA, (a1,d5.w)                   ;1 d-------- a--23----  18   #$1EEA -> (fill_routine, bottom_offset)
                                                    ;1 d-----5-- a-123----       -------- gap --------
    move.w      d1, rc_ymin(a4)                     ;1 d-----5-- a-123----   8   Update rc_ymin with ystart
    move.l      a7, (a4)                            ;1 d-----5-- a-123----  12   Set future drawing start pointer to current pos
    move.w      d2, rc_ymax(a4)                     ;1 d-----5-- a-123----   8   Update rc_ymax with yend
    move.w      d2, d5                              ;1 d-------- a-123----   4   Add 2*(yend - ystart) to A7
    sub.w       d1, d5                              ;1 d-------- a-123----   4   
    add.w       d5, d5                              ;1 d-------- a-123----   4   
    add.w       d5, a7                              ;1 d-------- a-123----   8   
                                                    ;1 d-----5-- a-123----       -------- Lower] --------
                                                    ;1 d-----5-- a-123----       -- Fill routine --
    move.w      d4, d5                              ;1 d-------- a-123----   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d5                              ;1 d-------- a-123----  12   
    and.w       #$1FFC, d5                          ;1 d-------- a-123----   8   
    lea.l       _FN_SCALERS, a1                     ;1 d-------- a--23----  12   
    move.l      (a1,d5.w), a1                       ;1 d-------- a--23----  18   
                                                    ;1 d-----5-- a--23----       -- Texcoord (no persp) --
    move.l      d6, d5                              ;1 d-------- a--23----   4   word tx = u1s;
    and.w       #$1F80, d5                          ;1 d-------- a--23----   8   tx &= $1F80;
    move.l      _asm_tex_base2(pc), a2              ;1 d-------- a---3----  16   -- Texture source --
    add.w       d5, a2                              ;1 d-------- a---3----   8   
    swap.w      d7                                  ;1 d-----5-- a--23----   4   Select ymax
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   bottom_offset = 4*ymax
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   
    move.w      #$4ED6, (a1,d7.w)                   ;1 d-----5-- a--23----  18   #$4ED6 -> (fill_routine, bottom_offset)
    move.w      d2, d5                              ;1 d-------- a--23----   4   top_offset = 4*ystart
    add.w       d5, d5                              ;1 d-------- a--23----   4   
    add.w       d5, d5                              ;1 d-------- a--23----   4   
    lea.l       .ret_27, a6                         ;1 d-------- a--23----  12   
    jmp         (a1,d5.w)                           ;1 d-------- a--23----  14   execute at (fill_routine, top_offset)
.ret_27:                                            ;1 d-----5-- a--23----       
    move.w      #$1EEA, (a1,d7.w)                   ;1 d-----5-- a--23----  18   #$1EEA -> (fill_routine, bottom_offset)
    bra         .end_loop                           ;1 d-----5-7 a-123----  10   
                                                    ;1 d-----5-- a-123----       
.cmp_25:                                            ;1 d-----5-- a-123----       -------- y1 <= ymin < y2 && y2 <= ymax < y3 --------
                                                    ;1 d-----5-- a-123----       ======== [gap Lower] ========
                                                    ;1 d-----5-- a-123----       -------- [gap --------
    move.w      d2, rc_ymax(a4)                     ;1 d-----5-- a-123----   8   Update rc_ymax with yend
    sub.w       d2, d7                              ;1 d-----5-- a-123----   4   Add 2*(yend - rc_ymin) to A7
    neg.w       d7                                  ;1 d-----5-- a-123----   4   
    add.w       d7, d7                              ;1 d-----5-- a-123----   4   
    add.w       d7, a7                              ;1 d-----5-- a-123----   8   
                                                    ;1 d-----5-- a-123----       -------- Lower] --------
                                                    ;1 d-----5-- a-123----       -- Fill routine --
    move.w      d4, d5                              ;1 d-------- a-123----   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d5                              ;1 d-------- a-123----  12   
    and.w       #$1FFC, d5                          ;1 d-------- a-123----   8   
    lea.l       _FN_SCALERS, a1                     ;1 d-------- a--23----  12   
    move.l      (a1,d5.w), a1                       ;1 d-------- a--23----  18   
                                                    ;1 d-----5-- a--23----       -- Texcoord (no persp) --
    move.l      d6, d5                              ;1 d-------- a--23----   4   word tx = u1s;
    and.w       #$1F80, d5                          ;1 d-------- a--23----   8   tx &= $1F80;
    move.l      _asm_tex_base2(pc), a2              ;1 d-------- a---3----  16   -- Texture source --
    add.w       d5, a2                              ;1 d-------- a---3----   8   
    swap.w      d7                                  ;1 d-----5-- a--23----   4   Select ymax
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   bottom_offset = 4*ymax
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   
    move.w      #$4ED6, (a1,d7.w)                   ;1 d-----5-- a--23----  18   #$4ED6 -> (fill_routine, bottom_offset)
    move.w      d2, d5                              ;1 d-------- a--23----   4   top_offset = 4*ystart
    add.w       d5, d5                              ;1 d-------- a--23----   4   
    add.w       d5, d5                              ;1 d-------- a--23----   4   
    lea.l       .ret_28, a6                         ;1 d-------- a--23----  12   
    jmp         (a1,d5.w)                           ;1 d-------- a--23----  14   execute at (fill_routine, top_offset)
.ret_28:                                            ;1 d-----5-- a--23----       
    move.w      #$1EEA, (a1,d7.w)                   ;1 d-----5-- a--23----  18   #$1EEA -> (fill_routine, bottom_offset)
    bra         .end_loop                           ;1 d-----5-7 a-123----  10   
                                                    ;1 d-----5-- a-123----       
.cmp_24:                                            ;1 d-----5-- a-123----       -------- y0 <= ymin < y2 && y3 <= ymax --------
    swap.w      d7                                  ;1 d-----5-- a-123----   4   Select ymin
    cmp.w       d1, d7                              ;1 d-----5-- a-123----   4   ymin>=y1
    bge         .cmp_29                             ;1 d-----5-- a-123----  10   
                                                    ;1 d-----5-- a-123----       
                                                    ;1 d-----5-- a-123----       -------- y0 <= ymin < y1 && y3 <= ymax --------
                                                    ;1 d-----5-- a-123----       ======== [Upper gap Lower Floor] ========
                                                    ;1 d-----5-- a-123----       -------- [Upper --------
                                                    ;1 d-----5-- a-123----       -- Fill routine --
    move.w      d4, d5                              ;1 d-------- a-123----   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d5                              ;1 d-------- a-123----  12   
    and.w       #$1FFC, d5                          ;1 d-------- a-123----   8   
    lea.l       _FN_SCALERS, a1                     ;1 d-------- a--23----  12   
    move.l      (a1,d5.w), a1                       ;1 d-------- a--23----  18   
                                                    ;1 d-----5-- a--23----       -- Texcoord (no persp) --
    move.l      d6, d5                              ;1 d-------- a--23----   4   word tx = u1s;
    and.w       #$1F80, d5                          ;1 d-------- a--23----   8   tx &= $1F80;
    move.l      _asm_tex_base(pc), a2               ;1 d-------- a---3----  16   -- Texture source --
    add.w       d5, a2                              ;1 d-------- a---3----   8   
    move.w      d1, d5                              ;1 d-------- a--23----   4   bottom_offset = 4*yend
    add.w       d5, d5                              ;1 d-------- a--23----   4   
    add.w       d5, d5                              ;1 d-------- a--23----   4   
    move.w      #$4ED6, (a1,d5.w)                   ;1 d-------- a--23----  18   #$4ED6 -> (fill_routine, bottom_offset)
    add.w       d7, d7                              ;1 d-------- a--23----   4   top_offset = 4*ymin
    add.w       d7, d7                              ;1 d-------- a--23----   4   
    lea.l       .ret_30, a6                         ;1 d-------- a--23----  12   
    jmp         (a1,d7.w)                           ;1 d-------- a--23----  14   execute at (fill_routine, top_offset)
.ret_30:                                            ;1 d-------7 a--23----       
    move.w      #$1EEA, (a1,d5.w)                   ;1 d-------7 a--23----  18   #$1EEA -> (fill_routine, bottom_offset)
                                                    ;1 d-----5-7 a-123----       -------- gap --------
    move.w      d1, rc_ymin(a4)                     ;1 d-----5-7 a-123----   8   Update rc_ymin with ystart
    move.l      a7, (a4)                            ;1 d-----5-7 a-123----  12   Set future drawing start pointer to current pos
    move.w      d2, rc_ymax(a4)                     ;1 d-----5-7 a-123----   8   Update rc_ymax with yend
    move.w      d2, d5                              ;1 d-------7 a-123----   4   Add 2*(yend - ystart) to A7
    sub.w       d1, d5                              ;1 d-------7 a-123----   4   
    add.w       d5, d5                              ;1 d-------7 a-123----   4   
    add.w       d5, a7                              ;1 d-------7 a-123----   8   
                                                    ;1 d-----5-7 a-123----       -------- Lower --------
                                                    ;1 d-----5-7 a-123----       -- Fill routine --
    move.w      d4, d5                              ;1 d-------7 a-123----   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d5                              ;1 d-------7 a-123----  12   
    and.w       #$1FFC, d5                          ;1 d-------7 a-123----   8   
    lea.l       _FN_SCALERS, a1                     ;1 d-------7 a--23----  12   
    move.l      (a1,d5.w), a1                       ;1 d-------7 a--23----  18   
                                                    ;1 d-----5-7 a--23----       -- Texcoord (no persp) --
    move.l      d6, d5                              ;1 d-------7 a--23----   4   word tx = u1s;
    and.w       #$1F80, d5                          ;1 d-------7 a--23----   8   tx &= $1F80;
    move.l      _asm_tex_base2(pc), a2              ;1 d-------7 a---3----  16   -- Texture source --
    add.w       d5, a2                              ;1 d-------7 a---3----   8   
    move.w      d3, d5                              ;1 d-------7 a--23----   4   bottom_offset = 4*yend
    add.w       d5, d5                              ;1 d-------7 a--23----   4   
    add.w       d5, d5                              ;1 d-------7 a--23----   4   
    move.w      #$4ED6, (a1,d5.w)                   ;1 d-------7 a--23----  18   #$4ED6 -> (fill_routine, bottom_offset)
    move.w      d2, d7                              ;1 d-------- a--23----   4   top_offset = 4*ystart
    add.w       d7, d7                              ;1 d-------- a--23----   4   
    add.w       d7, d7                              ;1 d-------- a--23----   4   
    lea.l       .ret_31, a6                         ;1 d-------- a--23----  12   
    jmp         (a1,d7.w)                           ;1 d-------- a--23----  14   execute at (fill_routine, top_offset)
.ret_31:                                            ;1 d-------- a--23----       
    move.w      #$1EEA, (a1,d5.w)                   ;1 d-------- a--23----  18   #$1EEA -> (fill_routine, bottom_offset)
                                                    ;1 d-----5-- a-123----       -------- Floor] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;1 d-----5-- a--23----  12   Color filler func
    swap.w      d7                                  ;1 d-----5-- a--23----   4   Select ymax
    sub.w       d3, d7                              ;1 d-----5-- a--23----   4   
    neg.w       d7                                  ;1 d-----5-- a--23----   4   
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   
    move.b      line_floor_col(a5), d5              ;1 d-------- a--23----  12   
    lea.l       .ret_32, a6                         ;1 d-----5-- a--23----  12   
    jmp         (a1,d7.w)                           ;1 d-----5-- a--23----  14   
.ret_32:                                            ;1 d-----5-7 a-123----       
    bra         .end_loop                           ;1 d-----5-7 a-123----  10   
                                                    ;1 d-----5-- a-123----       
.cmp_29:                                            ;1 d-----5-- a-123----       -------- y1 <= ymin < y2 && y3 <= ymax --------
                                                    ;1 d-----5-- a-123----       ======== [gap Lower Floor] ========
                                                    ;1 d-----5-- a-123----       -------- [gap --------
    move.w      d2, rc_ymax(a4)                     ;1 d-----5-- a-123----   8   Update rc_ymax with yend
    sub.w       d2, d7                              ;1 d-----5-- a-123----   4   Add 2*(yend - rc_ymin) to A7
    neg.w       d7                                  ;1 d-----5-- a-123----   4   
    add.w       d7, d7                              ;1 d-----5-- a-123----   4   
    add.w       d7, a7                              ;1 d-----5-- a-123----   8   
                                                    ;1 d-----5-7 a-123----       -------- Lower --------
                                                    ;1 d-----5-7 a-123----       -- Fill routine --
    move.w      d4, d5                              ;1 d-------7 a-123----   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d5                              ;1 d-------7 a-123----  12   
    and.w       #$1FFC, d5                          ;1 d-------7 a-123----   8   
    lea.l       _FN_SCALERS, a1                     ;1 d-------7 a--23----  12   
    move.l      (a1,d5.w), a1                       ;1 d-------7 a--23----  18   
                                                    ;1 d-----5-7 a--23----       -- Texcoord (no persp) --
    move.l      d6, d5                              ;1 d-------7 a--23----   4   word tx = u1s;
    and.w       #$1F80, d5                          ;1 d-------7 a--23----   8   tx &= $1F80;
    move.l      _asm_tex_base2(pc), a2              ;1 d-------7 a---3----  16   -- Texture source --
    add.w       d5, a2                              ;1 d-------7 a---3----   8   
    move.w      d3, d5                              ;1 d-------7 a--23----   4   bottom_offset = 4*yend
    add.w       d5, d5                              ;1 d-------7 a--23----   4   
    add.w       d5, d5                              ;1 d-------7 a--23----   4   
    move.w      #$4ED6, (a1,d5.w)                   ;1 d-------7 a--23----  18   #$4ED6 -> (fill_routine, bottom_offset)
    move.w      d2, d7                              ;1 d-------- a--23----   4   top_offset = 4*ystart
    add.w       d7, d7                              ;1 d-------- a--23----   4   
    add.w       d7, d7                              ;1 d-------- a--23----   4   
    lea.l       .ret_33, a6                         ;1 d-------- a--23----  12   
    jmp         (a1,d7.w)                           ;1 d-------- a--23----  14   execute at (fill_routine, top_offset)
.ret_33:                                            ;1 d-------- a--23----       
    move.w      #$1EEA, (a1,d5.w)                   ;1 d-------- a--23----  18   #$1EEA -> (fill_routine, bottom_offset)
                                                    ;1 d-----5-- a-123----       -------- Floor] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;1 d-----5-- a--23----  12   Color filler func
    swap.w      d7                                  ;1 d-----5-- a--23----   4   Select ymax
    sub.w       d3, d7                              ;1 d-----5-- a--23----   4   
    neg.w       d7                                  ;1 d-----5-- a--23----   4   
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   
    move.b      line_floor_col(a5), d5              ;1 d-------- a--23----  12   
    lea.l       .ret_34, a6                         ;1 d-----5-- a--23----  12   
    jmp         (a1,d7.w)                           ;1 d-----5-- a--23----  14   
.ret_34:                                            ;1 d-----5-7 a-123----       
    bra         .end_loop                           ;1 d-----5-7 a-123----  10   
                                                    ;1 d-----5-- a-123----       
.cmp_14:                                            ;1 d-----5-- a-123----       -------- y2 <= ymin && y2 <= ymax --------
    swap.w      d7                                  ;1 d-----5-- a-123----   4   Select ymax
    cmp.w       d3, d7                              ;1 d-----5-- a-123----   4   ymax>=y3
    bge         .cmp_35                             ;1 d-----5-- a-123----  10   
                                                    ;1 d-----5-- a-123----       
                                                    ;1 d-----5-- a-123----       -------- y2 <= ymin < y3 && y2 <= ymax < y3 --------
                                                    ;1 d-----5-- a-123----       ======== [Lower] ========
                                                    ;1 d-----5-- a-123----       -------- [Lower] --------
                                                    ;1 d-----5-- a-123----       -- Fill routine --
    move.w      d4, d5                              ;1 d-------- a-123----   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d5                              ;1 d-------- a-123----  12   
    and.w       #$1FFC, d5                          ;1 d-------- a-123----   8   
    lea.l       _FN_SCALERS, a1                     ;1 d-------- a--23----  12   
    move.l      (a1,d5.w), a1                       ;1 d-------- a--23----  18   
                                                    ;1 d-----5-- a--23----       -- Texcoord (no persp) --
    move.l      d6, d5                              ;1 d-------- a--23----   4   word tx = u1s;
    and.w       #$1F80, d5                          ;1 d-------- a--23----   8   tx &= $1F80;
    move.l      _asm_tex_base2(pc), a2              ;1 d-------- a---3----  16   -- Texture source --
    add.w       d5, a2                              ;1 d-------- a---3----   8   
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   bottom_offset = 4*ymax
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   
    move.w      #$4ED6, (a1,d7.w)                   ;1 d-----5-- a--23----  18   #$4ED6 -> (fill_routine, bottom_offset)
    swap.w      d7                                  ;1 d-----5-- a--23----   4   Select ymin
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   top_offset = 4*ymin
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   
    lea.l       .ret_36, a6                         ;1 d-----5-- a--23----  12   
    jmp         (a1,d7.w)                           ;1 d-----5-- a--23----  14   execute at (fill_routine, top_offset)
.ret_36:                                            ;1 d-----5-- a--23----       
    swap.w      d7                                  ;1 d-----5-- a--23----   4   Select ymax
    move.w      #$1EEA, (a1,d7.w)                   ;1 d-----5-- a--23----  18   #$1EEA -> (fill_routine, bottom_offset)
    clr.l       rc_ymin(a4)                         ;1 d-----5-7 a-123----       Disable further rendering - clear ymin/ymax
    move.w      d4, rc_size_limit(a4)               ;1 d-----5-7 a-123----   8   
    bra         .end_loop                           ;1 d-----5-7 a-123----  10   
                                                    ;1 d-----5-- a-123----       
.cmp_35:                                            ;1 d-----5-- a-123----       -------- y2 <= ymin && y3 <= ymax --------
    swap.w      d7                                  ;1 d-----5-- a-123----   4   Select ymin
    cmp.w       d3, d7                              ;1 d-----5-- a-123----   4   ymin>=y3
    bge         .cmp_37                             ;1 d-----5-- a-123----  10   
                                                    ;1 d-----5-- a-123----       
                                                    ;1 d-----5-- a-123----       -------- y2 <= ymin < y3 && y3 <= ymax --------
                                                    ;1 d-----5-- a-123----       ======== [Lower Floor] ========
                                                    ;1 d-----5-- a-123----       -------- [Lower --------
                                                    ;1 d-----5-- a-123----       -- Fill routine --
    move.w      d4, d5                              ;1 d-------- a-123----   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d5                              ;1 d-------- a-123----  12   
    and.w       #$1FFC, d5                          ;1 d-------- a-123----   8   
    lea.l       _FN_SCALERS, a1                     ;1 d-------- a--23----  12   
    move.l      (a1,d5.w), a1                       ;1 d-------- a--23----  18   
                                                    ;1 d-----5-- a--23----       -- Texcoord (no persp) --
    move.l      d6, d5                              ;1 d-------- a--23----   4   word tx = u1s;
    and.w       #$1F80, d5                          ;1 d-------- a--23----   8   tx &= $1F80;
    move.l      _asm_tex_base2(pc), a2              ;1 d-------- a---3----  16   -- Texture source --
    add.w       d5, a2                              ;1 d-------- a---3----   8   
    move.w      d3, d5                              ;1 d-------- a--23----   4   bottom_offset = 4*yend
    add.w       d5, d5                              ;1 d-------- a--23----   4   
    add.w       d5, d5                              ;1 d-------- a--23----   4   
    move.w      #$4ED6, (a1,d5.w)                   ;1 d-------- a--23----  18   #$4ED6 -> (fill_routine, bottom_offset)
    add.w       d7, d7                              ;1 d-------- a--23----   4   top_offset = 4*ymin
    add.w       d7, d7                              ;1 d-------- a--23----   4   
    lea.l       .ret_38, a6                         ;1 d-------- a--23----  12   
    jmp         (a1,d7.w)                           ;1 d-------- a--23----  14   execute at (fill_routine, top_offset)
.ret_38:                                            ;1 d-------- a--23----       
    move.w      #$1EEA, (a1,d5.w)                   ;1 d-------- a--23----  18   #$1EEA -> (fill_routine, bottom_offset)
                                                    ;1 d-----5-- a-123----       -------- Floor] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;1 d-----5-- a--23----  12   Color filler func
    swap.w      d7                                  ;1 d-----5-- a--23----   4   Select ymax
    sub.w       d3, d7                              ;1 d-----5-- a--23----   4   
    neg.w       d7                                  ;1 d-----5-- a--23----   4   
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   
    move.b      line_floor_col(a5), d5              ;1 d-------- a--23----  12   
    lea.l       .ret_39, a6                         ;1 d-----5-- a--23----  12   
    jmp         (a1,d7.w)                           ;1 d-----5-- a--23----  14   
.ret_39:                                            ;1 d-----5-7 a-123----       
    clr.l       rc_ymin(a4)                         ;1 d-----5-7 a-123----       Disable further rendering - clear ymin/ymax
    move.w      d4, rc_size_limit(a4)               ;1 d-----5-7 a-123----   8   
    bra         .end_loop                           ;1 d-----5-7 a-123----  10   
                                                    ;1 d-----5-- a-123----       
.cmp_37:                                            ;1 d-----5-- a-123----       -------- y3 <= ymin && y3 <= ymax --------
                                                    ;1 d-----5-- a-123----       ======== [Floor] ========
                                                    ;1 d-----5-- a-123----       -------- [Floor] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;1 d-----5-- a--23----  12   Color filler func
    move.w      d7, d5                              ;1 d-------- a--23----   4   
    swap.w      d7                                  ;1 d-------- a--23----   4   Select ymax
    sub.w       d7, d5                              ;1 d-------- a--23----   4   
    move.w      d5, d7                              ;1 d-------- a--23----   4   avoid color reg clash
    add.w       d7, d7                              ;1 d-----5-- a--23----   4   
    move.b      line_floor_col(a5), d5              ;1 d-------- a--23----  12   
    lea.l       .ret_40, a6                         ;1 d-----5-- a--23----  12   
    jmp         (a1,d7.w)                           ;1 d-----5-- a--23----  14   
.ret_40:                                            ;1 d-----5-7 a-123----       
    clr.l       rc_ymin(a4)                         ;1 d-----5-7 a-123----       Disable further rendering - clear ymin/ymax
    move.w      d4, rc_size_limit(a4)               ;1 d-----5-7 a-123----   8   
    bra         .end_loop                           ;1 d-----5-7 a-123----  10   
                                                    ;1 d-----5-7 a-123----       
                                                    ;1 d-----5-7 a-123----       
.end_loop:                                          ;1 d-----5-7 a-123----       ======== LOOP END ========
    cmp.l       a0, a4                              ;1 d-----5-7 a-123----   6   render_column ? render_end
    blt         .loop                               ;1 d-----5-7 a-123----  10   continue if <
.end:                                               ;1 d01234567 a012345--       Cleanup & exit
    move.l      _a7_save(pc), a7                    ;1 d01234567 a012345--  16   
    rts                                             ;1 d01234567 a012345--  16   
