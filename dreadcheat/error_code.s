
    public ERROR
ERROR:

                                                    ;s d01---5-- a01234---       ================================ Ceil y1 Upper y2 Floor ================================
    move.l      a7, _a7_save                        ;s d01---5-- a01234---  20   
                                                    ;s d01---5-- a01234---       
                                                    ;s d01---5-- a01234---       ======== Loop initialization ========
    lea.l       _render_columns, a4                 ;s d01---5-- a0123----  12   
    add.w       d2, a4                              ;s d01---5-- a0123----   8   add xmin*8 to column pointer
    move.l      d7, .trace_tex+2                    ;s d012--5-- a0123----  20   
    move.l      a4, a0                              ;s d012--5-7 a-123----   4   end = start + count*8
    lsl.w       #3, d3                              ;s d012--5-7 a-123----  12   
    addq.l      #1, d3                              ;s d012--5-7 a-123----   8   
    add.w       d3, a0                              ;s d012--5-7 a-123----   8   
    move.l      line_tex_upper(a5), a1              ;s d0123-5-7 a--23----  16   Upper texture
    move.w      _view_pos_z, d0                     ;s d-123-5-7 a--23----  16   
    asr.w       #1, d0                              ;s d-123-5-7 a--23----   8   
    add.w       d0, a1                              ;s d-123-5-7 a--23----   8   
    move.l      a1, .asm_tex_base                   ;s d0123-5-7 a--23----  20   
                                                    ;s d0123-5-7 a-123----       
                                                    ;s d0123-5-7 a-123----       ======== init .trace_y0 tracer ========
    move.w      d4, d0                              ;s d-123-5-7 a-123----   4   compute starting Y
    lsr.w       #5, d0                              ;s d-123-5-7 a-123----  16   
    move.w      line_y1(a5), d1                     ;s d--23-5-7 a-123----  12   
    sub.w       _view_pos_z, d1                     ;s d--23-5-7 a-123----  16   
    muls.w      d1, d0                              ;s d--23-5-7 a-123----  70   
    move.w      _asm_line_s2, d5                    ;s d--23---7 a-123----  16   compute Y slope
    lsr.w       #5, d5                              ;s d--23---7 a-123----  16   
    muls.w      d1, d5                              ;s d--23---7 a-123----  70   
    sub.l       d0, d5                              ;s d-123---7 a-123----   8   
    divs.w      _asm_line_xlen, d5                  ;s d-123---7 a-123---- 170   
    move.b      d5, .trace_s+2                      ;s d-123---7 a-123----  16   subpixel step
    lsr.w       #8, d5                              ;s d-123---7 a-123----  22   
    ext.w       d5                                  ;s d-123---7 a-123----   4   
    move.w      d5, .trace_y0+4                     ;s d-123---7 a-123----  16   pixel step
    add.l       #ENGINE_Y_MID*256+128, d0           ;s d-123-5-7 a-123----  16   24.8 fixed
    lsl.l       #8, d0                              ;s d-123-5-7 a-123----  24   16.16 fixed
    swap.w      d4                                  ;s d-123-5-7 a-123----   4   
    move.w      d0, d4                              ;s d-123-5-7 a-123----   4   
    swap.w      d4                                  ;s d-123-5-7 a-123----   4   
    sub.w       d0, d0                              ;s d-123-5-7 a-123----   4   
    swap.w      d0                                  ;s d-123-5-7 a-123----   4   current Y value
                                                    ;s d-123-5-7 a-123----       
                                                    ;s d-123-5-7 a-123----       ======== init .trace_y1 tracer ========
    move.w      d4, d1                              ;s d--23-5-7 a-123----   4   compute starting Y
    lsr.w       #5, d1                              ;s d--23-5-7 a-123----  16   
    move.w      line_y2(a5), d2                     ;s d---3-5-7 a-123----  12   
    sub.w       _view_pos_z, d2                     ;s d---3-5-7 a-123----  16   
    muls.w      d2, d1                              ;s d---3-5-7 a-123----  70   
    move.w      _asm_line_s2, d5                    ;s d---3---7 a-123----  16   compute Y slope
    lsr.w       #5, d5                              ;s d---3---7 a-123----  16   
    muls.w      d2, d5                              ;s d---3---7 a-123----  70   
    sub.l       d1, d5                              ;s d--23---7 a-123----   8   
    divs.w      _asm_line_xlen, d5                  ;s d--23---7 a-123---- 170   
    move.b      d5, .trace_y0+2                     ;s d--23---7 a-123----  16   subpixel step
    lsr.w       #8, d5                              ;s d--23---7 a-123----  22   
    ext.w       d5                                  ;s d--23---7 a-123----   4   
    move.w      d5, .trace_y1+2                     ;s d--23---7 a-123----  16   pixel step
    add.l       #ENGINE_Y_MID*256+128, d1           ;s d--23-5-7 a-123----  16   24.8 fixed
    lsl.l       #8, d1                              ;s d--23-5-7 a-123----  24   16.16 fixed
    swap.w      d0                                  ;s d--23-5-7 a-123----   4   
    move.w      d1, d0                              ;s d--23-5-7 a-123----   4   
    swap.w      d0                                  ;s d--23-5-7 a-123----   4   
    sub.w       d1, d1                              ;s d--23-5-7 a-123----   4   
    swap.w      d1                                  ;s d--23-5-7 a-123----   4   current Y value
    move.w      _asm_line_ds, .trace_s+4            ;s d--23-5-7 a-123----  28   
    TRAP_LINEDRAW_SETUP_DONE                        ;s d--23-5-7 a-123----       
    bra         .start                              ;s d--23-5-7 a-123----       End of header
                                                    ;s d--23-5-7 a-123----       
                                                    ;s d--23-5-7 a-123----       
.loop:                                              ;I d--23-5-7 a-123----       ======== LOOP START ========
.trace_s:                                           ;I d--23-5-7 a-123----       
    add.l       #$FF00FFFF, d4                      ;I d--23-5-7 a-123----       s1 += ds
.trace_y0:                                          ;I d--23-5-7 a-123----       
    move.l      #$FF000000, d2                      ;I d---3-5-7 a-123----       
    addx.l      d2, d0                              ;I d---3-5-7 a-123----       
.trace_y1:                                          ;I d--23-5-7 a-123----       
    move.w      #$FF00, d2                          ;I d---3-5-7 a-123----       
    addx.w      d2, d1                              ;I d---3-5-7 a-123----       
.trace_tex:                                         ;I d--23-5-7 a-123----       
    add.l       #$80000000, d6                      ;I d--23-5-7 a-123----       u += du
                                                    ;- d--23-5-7 a-123----       
.start:                                             ;- d--23-5-7 a-123----       ======== FIRST ITERATION STARTS HERE ========
    move.l      (a4)+, a7                           ;- d--23-5-7 a-123----       byte *dst = render_buffer + offs[x];
                                                    ;C d--23-5-7 a-123----       
                                                    ;C d--23-5-7 a-123----       -------- ymin/ymax not checked --------
    move.l      (a4)+, d2                           ;C d---3-5-7 a-123----       Load ymin/ymax
    cmp.w       d1, d2                              ;C d---3-5-7 a-123----       ymax>=y1
    bge         .cmp_1                              ;C d---3-5-7 a-123----       
                                                    ;C d---3-5-7 a-123----       
                                                    ;C d---3-5-7 a-123----       -------- ymin < y1 && ymax < y1 --------
    cmp.w       d0, d2                              ;C d---3-5-7 a-123----       ymax>=y0
    bge         .cmp_2                              ;C d---3-5-7 a-123----       
                                                    ;C d---3-5-7 a-123----       
                                                    ;C d---3-5-7 a-123----       -------- ymin < y0 && ymax < y0 --------
                                                    ;- d---3-5-7 a-123----       ======== [Ceil] ========
                                                    ;F d---3-5-7 a-123----       -------- [Ceil] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;F d---3-5-7 a--23----       Color filler func
    swap.w      d2                                  ;F d---3-5-7 a--23----       Select ymin
    move.w      d2, d3                              ;F d-----5-7 a--23----       
    swap.w      d2                                  ;F d-----5-7 a--23----       Select ymax
    sub.w       d2, d3                              ;F d-----5-7 a--23----       
    add.w       d3, d3                              ;F d--2--5-7 a--23----       
    move.b      line_ceil_col(a5), d5               ;F d--2----7 a--23----       
    lea.l       .ret_3(pc), a6                      ;F d--2--5-7 a--23----       
    jmp         (a1,d3.w)                           ;F d--2--5-7 a--23----       -> .ret_3
.ret_3:                                             ;F d--23-5-7 a-123----       
    moveq.l     #0, d2                              ;G d---3-5-7 a-123----       Disable further rendering - clear ymin/ymax
    move.l      d2, rc_ymin-8(a4)                   ;G d---3-5-7 a-123----       
    move.w      d4, rc_size_limit-8(a4)             ;G d--23-5-7 a-123----       
    bra         .end_loop                           ;- d--23-5-7 a-123----       
                                                    ;C d---3-5-7 a-123----       
.cmp_2:                                             ;C d---3-5-7 a-123----       -------- ymin < y1 && y0 <= ymax < y1 --------
    swap.w      d2                                  ;C d---3-5-7 a-123----       Select ymin
    cmp.w       d0, d2                              ;C d---3-5-7 a-123----       ymin>=y0
    bge         .cmp_4                              ;C d---3-5-7 a-123----       
                                                    ;C d---3-5-7 a-123----       
                                                    ;C d---3-5-7 a-123----       -------- ymin < y0 && y0 <= ymax < y1 --------
                                                    ;- d---3-5-7 a-123----       ======== [Ceil Upper] ========
                                                    ;F d---3-5-7 a-123----       -------- [Ceil --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;F d---3-5-7 a--23----       Color filler func
    sub.w       d0, d2                              ;F d---3-5-7 a--23----       
    add.w       d2, d2                              ;F d---3-5-7 a--23----       
    move.b      line_ceil_col(a5), d5               ;F d---3---7 a--23----       
    lea.l       .ret_5(pc), a6                      ;F d---3-5-7 a--23----       
    jmp         (a1,d2.w)                           ;F d---3-5-7 a--23----       -> .ret_5
.ret_5:                                             ;F d---3-5-7 a-123----       
                                                    ;R d---3-5-7 a-123----       -------- Upper] --------
                                                    ;R d---3-5-7 a-123----       -- Fill routine --
    move.w      d4, d3                              ;R d-----5-7 a-123----       int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d3                              ;R d-----5-7 a-123----       
    and.w       #$1FFC, d3                          ;R d-----5-7 a-123----       
    lea.l       _FN_SCALERS, a3                     ;R d-----5-7 a-12-----       
    move.l      (a3,d3.w), a3                       ;R d-----5-7 a-12-----       
                                                    ;T d---3-5-7 a-12-----       -- Texcoord (persp) --
    move.l      d6, d3                              ;T d-----5-7 a-12-----       word tx = u1s /divu/ s1;
    divu.w      d4, d3                              ;T d-----5-7 a-12-----       
    and.w       #$1F80, d3                          ;T d-----5-7 a-12-----       tx &= $1F80;
    move.l      .asm_tex_base(pc), a2               ;T d-----5-7 a-1------       -- Texture source --
    add.w       d3, a2                              ;T d-----5-7 a-1------       
    swap.w      d2                                  ;R d---3-5-7 a-12-----       Select ymax
    add.w       d2, d2                              ;R d---3-5-7 a-12-----       bottom_offset = 4*ymax
    add.w       d2, d2                              ;R d---3-5-7 a-12-----       
    move.w      #$4ED6, (a3,d2.w)                   ;R d---3-5-7 a-12-----       #$4ED6 -> (fill_routine, bottom_offset)
    move.w      d0, d3                              ;R d-----5-7 a-12-----       top_offset = 4*ystart
    add.w       d3, d3                              ;R d-----5-7 a-12-----       
    add.w       d3, d3                              ;R d-----5-7 a-12-----       
    lea.l       .ret_6(pc), a6                      ;R d-----5-7 a-12-----       
    jmp         (a3,d3.w)                           ;R d-----5-7 a-12-----       -> .ret_6
.ret_6:                                             ;R d---3-5-7 a-12-----       
    move.w      #$1EEA, (a3,d2.w)                   ;R d---3-5-7 a-12-----       #$1EEA -> (fill_routine, bottom_offset)
    moveq.l     #0, d2                              ;G d---3-5-7 a-123----       Disable further rendering - clear ymin/ymax
    move.l      d2, rc_ymin-8(a4)                   ;G d---3-5-7 a-123----       
    move.w      d4, rc_size_limit-8(a4)             ;G d--23-5-7 a-123----       
    bra         .end_loop                           ;- d--23-5-7 a-123----       
                                                    ;C d---3-5-7 a-123----       
.cmp_4:                                             ;C d---3-5-7 a-123----       -------- y0 <= ymin < y1 && y0 <= ymax < y1 --------
                                                    ;- d---3-5-7 a-123----       ======== [Upper] ========
                                                    ;R d---3-5-7 a-123----       -------- [Upper] --------
                                                    ;R d---3-5-7 a-123----       -- Fill routine --
    move.w      d4, d3                              ;R d-----5-7 a-123----       int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d3                              ;R d-----5-7 a-123----       
    and.w       #$1FFC, d3                          ;R d-----5-7 a-123----       
    lea.l       _FN_SCALERS, a1                     ;R d-----5-7 a--23----       
    move.l      (a1,d3.w), a1                       ;R d-----5-7 a--23----       
                                                    ;T d---3-5-7 a--23----       -- Texcoord (persp) --
    move.l      d6, d3                              ;T d-----5-7 a--23----       word tx = u1s /divu/ s1;
    divu.w      d4, d3                              ;T d-----5-7 a--23----       
    and.w       #$1F80, d3                          ;T d-----5-7 a--23----       tx &= $1F80;
    move.l      .asm_tex_base(pc), a2               ;T d-----5-7 a---3----       -- Texture source --
    add.w       d3, a2                              ;T d-----5-7 a---3----       
    swap.w      d2                                  ;R d---3-5-7 a--23----       Select ymax
    add.w       d2, d2                              ;R d---3-5-7 a--23----       bottom_offset = 4*ymax
    add.w       d2, d2                              ;R d---3-5-7 a--23----       
    move.w      #$4ED6, (a1,d2.w)                   ;R d---3-5-7 a--23----       #$4ED6 -> (fill_routine, bottom_offset)
    swap.w      d2                                  ;R d---3-5-7 a--23----       Select ymin
    add.w       d2, d2                              ;R d---3-5-7 a--23----       top_offset = 4*ymin
    add.w       d2, d2                              ;R d---3-5-7 a--23----       
    lea.l       .ret_7(pc), a6                      ;R d---3-5-7 a--23----       
    jmp         (a1,d2.w)                           ;R d---3-5-7 a--23----       -> .ret_7
.ret_7:                                             ;R d---3-5-7 a--23----       
    swap.w      d2                                  ;R d---3-5-7 a--23----       Select ymax
    move.w      #$1EEA, (a1,d2.w)                   ;R d---3-5-7 a--23----       #$1EEA -> (fill_routine, bottom_offset)
    moveq.l     #0, d2                              ;G d---3-5-7 a-123----       Disable further rendering - clear ymin/ymax
    move.l      d2, rc_ymin-8(a4)                   ;G d---3-5-7 a-123----       
    move.w      d4, rc_size_limit-8(a4)             ;G d--23-5-7 a-123----       
    bra         .end_loop                           ;- d--23-5-7 a-123----       
                                                    ;C d---3-5-7 a-123----       
.cmp_1:                                             ;C d---3-5-7 a-123----       -------- y1 <= ymax --------
    swap.w      d2                                  ;C d---3-5-7 a-123----       Select ymin
    cmp.w       d0, d2                              ;C d---3-5-7 a-123----       ymin>=y0
    bge         .cmp_8                              ;C d---3-5-7 a-123----       
                                                    ;C d---3-5-7 a-123----       
                                                    ;C d---3-5-7 a-123----       -------- ymin < y0 && y1 <= ymax --------
                                                    ;- d---3-5-7 a-123----       ======== [Ceil Upper Floor] ========
                                                    ;F d---3-5-7 a-123----       -------- [Ceil --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;F d---3-5-7 a--23----       Color filler func
    sub.w       d0, d2                              ;F d---3-5-7 a--23----       
    add.w       d2, d2                              ;F d---3-5-7 a--23----       
    move.b      line_ceil_col(a5), d5               ;F d---3---7 a--23----       
    lea.l       .ret_9(pc), a6                      ;F d---3-5-7 a--23----       
    jmp         (a1,d2.w)                           ;F d---3-5-7 a--23----       -> .ret_9
.ret_9:                                             ;F d---3-5-7 a--23----       
                                                    ;R d---3-5-7 a--23----       -------- Upper --------
                                                    ;R d---3-5-7 a--23----       -- Fill routine --
    move.w      d4, d3                              ;R d-----5-7 a--23----       int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d3                              ;R d-----5-7 a--23----       
    and.w       #$1FFC, d3                          ;R d-----5-7 a--23----       
    lea.l       _FN_SCALERS, a3                     ;R d-----5-7 a--2-----       
    move.l      (a3,d3.w), a3                       ;R d-----5-7 a--2-----       
                                                    ;T d---3-5-7 a--2-----       -- Texcoord (persp) --
    move.l      d6, d3                              ;T d-----5-7 a--2-----       word tx = u1s /divu/ s1;
    divu.w      d4, d3                              ;T d-----5-7 a--2-----       
    and.w       #$1F80, d3                          ;T d-----5-7 a--2-----       tx &= $1F80;
    move.l      .asm_tex_base(pc), a2               ;T d-----5-7 a--------       -- Texture source --
    add.w       d3, a2                              ;T d-----5-7 a--------       
    move.w      d1, d3                              ;R d-----5-7 a--2-----       bottom_offset = 4*yend
    add.w       d3, d3                              ;R d-----5-7 a--2-----       
    add.w       d3, d3                              ;R d-----5-7 a--2-----       
    add.w       d3, a3                              ;R d-----5-7 a--2-----       
    move.w      #$4ED6, (a3)                        ;R d---3-5-7 a--2-----       #$4ED6 -> (fill_routine, bottom_offset)
    move.w      d0, d3                              ;R d-----5-7 a--2-----       top_offset = 4*ystart
    sub.w       d1, d3                              ;R d-----5-7 a--2-----       
    add.w       d3, d3                              ;R d-----5-7 a--2-----       
    add.w       d3, d3                              ;R d-----5-7 a--2-----       
    lea.l       .ret_10(pc), a6                     ;R d-----5-7 a--2-----       
    jmp         (a3,d3.w)                           ;R d-----5-7 a--2-----       -> .ret_10
.ret_10:                                            ;R d---3-5-7 a--2-----       
    move.w      #$1EEA, (a3)                        ;R d---3-5-7 a--2-----       #$1EEA -> (fill_routine, bottom_offset)
                                                    ;F d---3-5-7 a--23----       -------- Floor] --------
    swap.w      d2                                  ;F d---3-5-7 a--23----       Select ymax
    sub.w       d1, d2                              ;F d---3-5-7 a--23----       
    neg.w       d2                                  ;F d---3-5-7 a--23----       
    add.w       d2, d2                              ;F d---3-5-7 a--23----       
    move.b      line_floor_col(a5), d5              ;F d---3---7 a--23----       
    moveq.l     #0, d3                              ;G d-----5-7 a--23----       Disable further rendering - clear ymin/ymax
    move.l      d3, rc_ymin-8(a4)                   ;G d-----5-7 a--23----       
    move.w      d4, rc_size_limit-8(a4)             ;G d---3-5-7 a--23----       
    lea.l       .end_loop(pc), a6                   ;F d---3-5-7 a--23----       
    jmp         (a1,d2.w)                           ;F d---3-5-7 a--23----       -> .end_loop
                                                    ;C d---3-5-7 a-123----       
.cmp_8:                                             ;C d---3-5-7 a-123----       -------- y0 <= ymin && y1 <= ymax --------
    cmp.w       d1, d2                              ;C d---3-5-7 a-123----       ymin>=y1
    bge         .cmp_11                             ;C d---3-5-7 a-123----       
                                                    ;C d---3-5-7 a-123----       
                                                    ;C d---3-5-7 a-123----       -------- y0 <= ymin < y1 && y1 <= ymax --------
                                                    ;- d---3-5-7 a-123----       ======== [Upper Floor] ========
                                                    ;R d---3-5-7 a-123----       -------- [Upper --------
                                                    ;R d---3-5-7 a-123----       -- Fill routine --
    move.w      d4, d3                              ;R d-----5-7 a-123----       int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d3                              ;R d-----5-7 a-123----       
    and.w       #$1FFC, d3                          ;R d-----5-7 a-123----       
    lea.l       _FN_SCALERS, a1                     ;R d-----5-7 a--23----       
    move.l      (a1,d3.w), a1                       ;R d-----5-7 a--23----       
                                                    ;T d---3-5-7 a--23----       -- Texcoord (persp) --
    move.l      d6, d3                              ;T d-----5-7 a--23----       word tx = u1s /divu/ s1;
    divu.w      d4, d3                              ;T d-----5-7 a--23----       
    and.w       #$1F80, d3                          ;T d-----5-7 a--23----       tx &= $1F80;
    move.l      .asm_tex_base(pc), a2               ;T d-----5-7 a---3----       -- Texture source --
    add.w       d3, a2                              ;T d-----5-7 a---3----       
    move.w      d1, d3                              ;R d-----5-7 a--23----       bottom_offset = 4*yend
    add.w       d3, d3                              ;R d-----5-7 a--23----       
    add.w       d3, d3                              ;R d-----5-7 a--23----       
    add.w       d3, a1                              ;R d-----5-7 a--23----       
    move.w      #$4ED6, (a1)                        ;R d---3-5-7 a--23----       #$4ED6 -> (fill_routine, bottom_offset)
    sub.w       d1, d2                              ;R d---3-5-7 a--23----       
    add.w       d2, d2                              ;R d---3-5-7 a--23----       top_offset = 4*ymin
    add.w       d2, d2                              ;R d---3-5-7 a--23----       
    lea.l       .ret_12(pc), a6                     ;R d---3-5-7 a--23----       
    jmp         (a1,d2.w)                           ;R d---3-5-7 a--23----       -> .ret_12
.ret_12:                                            ;R d---3-5-7 a--23----       
    move.w      #$1EEA, (a1)                        ;R d---3-5-7 a--23----       #$1EEA -> (fill_routine, bottom_offset)
                                                    ;F d---3-5-7 a-123----       -------- Floor] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;F d---3-5-7 a--23----       Color filler func
    swap.w      d2                                  ;F d---3-5-7 a--23----       Select ymax
    sub.w       d1, d2                              ;F d---3-5-7 a--23----       
    neg.w       d2                                  ;F d---3-5-7 a--23----       
    add.w       d2, d2                              ;F d---3-5-7 a--23----       
    move.b      line_floor_col(a5), d5              ;F d---3---7 a--23----       
    moveq.l     #0, d3                              ;G d-----5-7 a--23----       Disable further rendering - clear ymin/ymax
    move.l      d3, rc_ymin-8(a4)                   ;G d-----5-7 a--23----       
    move.w      d4, rc_size_limit-8(a4)             ;G d---3-5-7 a--23----       
    lea.l       .end_loop(pc), a6                   ;F d---3-5-7 a--23----       
    jmp         (a1,d2.w)                           ;F d---3-5-7 a--23----       -> .end_loop
                                                    ;C d---3-5-7 a-123----       
.cmp_11:                                            ;C d---3-5-7 a-123----       -------- y1 <= ymin && y1 <= ymax --------
                                                    ;- d---3-5-7 a-123----       ======== [Floor] ========
                                                    ;F d---3-5-7 a-123----       -------- [Floor] --------
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;F d---3-5-7 a--23----       Color filler func
    move.w      d2, d3                              ;F d-----5-7 a--23----       
    swap.w      d2                                  ;F d-----5-7 a--23----       Select ymax
    sub.w       d2, d3                              ;F d-----5-7 a--23----       
    add.w       d3, d3                              ;F d--2--5-7 a--23----       
    move.b      line_floor_col(a5), d5              ;F d--2----7 a--23----       
    moveq.l     #0, d2                              ;G d-----5-7 a--23----       Disable further rendering - clear ymin/ymax
    move.l      d2, rc_ymin-8(a4)                   ;G d-----5-7 a--23----       
    move.w      d4, rc_size_limit-8(a4)             ;G d--2--5-7 a--23----       
    lea.l       .end_loop(pc), a6                   ;F d--2--5-7 a--23----       
    jmp         (a1,d3.w)                           ;F d--2--5-7 a--23----       -> .end_loop
                                                    ;- d--23-5-7 a-123----       
                                                    ;- d--23-5-7 a-123----       
.end_loop:                                          ;- d--23-5-7 a-123----       ======== LOOP END ========
    cmp.l       a0, a4                              ;- d--23-5-7 a-123----       render_column ? render_end
    blt         .loop                               ;- d--23-5-7 a-123----       continue if <
.end:                                               ;o d01234567 a012345--       Cleanup & exit
    move.l      _a7_save, a7                        ;o d01234567 a012345--       
    rts                                             ;o d01234567 a012345--       
.asm_tex_base:                                      ;o d01234567 a012345--       
    move.w      #0, d0                              ;o d-1234567 a012345--       reserve 32-bit
