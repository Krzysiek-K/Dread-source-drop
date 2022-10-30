
    public _Dread_LineCore_x002
_Dread_LineCore_x002:

                                                    ;s d01---5-- a01234---       ================================ Sky y1 Upper y2 Floor ================================
    move.l      a7, _a7_save                        ;s d01---5-- a01234---  20   
                                                    ;s d01---5-- a01234---       
                                                    ;s d01---5-- a01234---       ======== Loop initialization ========
    lea.l       _render_columns, a4                 ;s d01---5-- a0123----  12   
    add.w       d2, a4                              ;s d01---5-- a0123----   8   add xmin*8 to column pointer
    lsr.w       #1, d2                              ;s d01---5-- a0123----   8   init SKY
    move.l      _e_skyptr, a0                       ;s d01---5-- a-123----  20   
    add.w       d2, a0                              ;s d01---5-- a-123----   8   
    move.w      d7, .trace_tex+2                    ;s d012--5-- a-123----  16   
    move.l      line_tex_upper(a5), a1              ;s d012--5-7 a--23----  16   Upper texture
    move.w      _view_pos_z, d0                     ;s d-12--5-7 a--23----  16   
    asr.w       #1, d0                              ;s d-12--5-7 a--23----   8   
    add.w       d0, a1                              ;s d-12--5-7 a--23----   8   
    move.l      a1, .asm_tex_base                   ;s d012--5-7 a--23----  20   
                                                    ;s d012--5-7 a-123----       
                                                    ;s d012--5-7 a-123----       ======== init .trace_y0 tracer ========
    move.w      d4, d0                              ;s d-12--5-7 a-123----   4   compute starting Y
    lsr.w       #5, d0                              ;s d-12--5-7 a-123----  16   
    move.w      line_y1(a5), d1                     ;s d--2--5-7 a-123----  12   
    sub.w       _view_pos_z, d1                     ;s d--2--5-7 a-123----  16   
    muls.w      d1, d0                              ;s d--2--5-7 a-123----  70   
    move.w      _asm_line_s2, d5                    ;s d--2----7 a-123----  16   compute Y slope
    lsr.w       #5, d5                              ;s d--2----7 a-123----  16   
    muls.w      d1, d5                              ;s d--2----7 a-123----  70   
    sub.l       d0, d5                              ;s d-12----7 a-123----   8   
    divs.w      _asm_line_xlen, d5                  ;s d-12----7 a-123---- 170   
    move.b      d5, .trace_s+2                      ;s d-12----7 a-123----  16   subpixel step
    lsr.w       #8, d5                              ;s d-12----7 a-123----  22   
    ext.w       d5                                  ;s d-12----7 a-123----   4   
    move.w      d5, .trace_y0+4                     ;s d-12----7 a-123----  16   pixel step
    add.l       #ENGINE_Y_MID*256+128, d0           ;s d-12--5-7 a-123----  16   24.8 fixed
    lsl.l       #8, d0                              ;s d-12--5-7 a-123----  24   16.16 fixed
    swap.w      d4                                  ;s d-12--5-7 a-123----   4   
    move.w      d0, d4                              ;s d-12--5-7 a-123----   4   
    swap.w      d4                                  ;s d-12--5-7 a-123----   4   
    sub.w       d0, d0                              ;s d-12--5-7 a-123----   4   
    swap.w      d0                                  ;s d-12--5-7 a-123----   4   current Y value
                                                    ;s d-12--5-7 a-123----       
                                                    ;s d-12--5-7 a-123----       ======== init .trace_y1 tracer ========
    move.w      d4, d1                              ;s d--2--5-7 a-123----   4   compute starting Y
    lsr.w       #5, d1                              ;s d--2--5-7 a-123----  16   
    move.w      line_y2(a5), d2                     ;s d-----5-7 a-123----  12   
    sub.w       _view_pos_z, d2                     ;s d-----5-7 a-123----  16   
    muls.w      d2, d1                              ;s d-----5-7 a-123----  70   
    move.w      _asm_line_s2, d5                    ;s d-------7 a-123----  16   compute Y slope
    lsr.w       #5, d5                              ;s d-------7 a-123----  16   
    muls.w      d2, d5                              ;s d-------7 a-123----  70   
    sub.l       d1, d5                              ;s d--2----7 a-123----   8   
    divs.w      _asm_line_xlen, d5                  ;s d--2----7 a-123---- 170   
    move.b      d5, .trace_y0+2                     ;s d--2----7 a-123----  16   subpixel step
    lsr.w       #8, d5                              ;s d--2----7 a-123----  22   
    ext.w       d5                                  ;s d--2----7 a-123----   4   
    move.w      d5, .trace_y1+2                     ;s d--2----7 a-123----  16   pixel step
    add.l       #ENGINE_Y_MID*256+128, d1           ;s d--2--5-7 a-123----  16   24.8 fixed
    lsl.l       #8, d1                              ;s d--2--5-7 a-123----  24   16.16 fixed
    swap.w      d0                                  ;s d--2--5-7 a-123----   4   
    move.w      d1, d0                              ;s d--2--5-7 a-123----   4   
    swap.w      d0                                  ;s d--2--5-7 a-123----   4   
    sub.w       d1, d1                              ;s d--2--5-7 a-123----   4   
    swap.w      d1                                  ;s d--2--5-7 a-123----   4   current Y value
    move.w      _asm_line_ds, .trace_s+4            ;s d--2--5-7 a-123----  28   
    move.b      line_floor_col(a5), d5              ;s d--2----7 a-123----  12   
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a1  ;s d--2--5-7 a--23-5--  12   Color filler func
    lea.l       _FN_SCALERS, a3                     ;s d--2--5-7 a--2--5--  12   
    TRAP_LINEDRAW_SETUP_DONE                        ;s d--2--5-7 a--2--5--       
    bra         .start                              ;s d--2--5-7 a--2--5--  12   End of header
                                                    ;s d--2--5-7 a--2--5--       
                                                    ;s d--2--5-7 a--2--5--       
.loop:                                              ;I d--2--5-7 a--2--5--       ======== LOOP START ========
.trace_s:                                           ;I d--2--5-7 a--2--5--       
    add.l       #$FF00FFFF, d4                      ;I d--2--5-7 a--2--5--  16   s1 += ds
.trace_y0:                                          ;I d--2--5-7 a--2--5--       
    move.l      #$FF000000, d2                      ;I d-----5-7 a--2--5--  12   
    addx.l      d2, d0                              ;I d-----5-7 a--2--5--   8   
.trace_y1:                                          ;I d--2--5-7 a--2--5--       
    move.w      #$FF00, d2                          ;I d-----5-7 a--2--5--   8   
    addx.w      d2, d1                              ;I d-----5-7 a--2--5--   4   
.trace_tex:                                         ;I d--2--5-7 a--2--5--       
    add.w       #$8000, d6                          ;I d--2--5-7 a--2--5--   8   u += du
                                                    ;- d--2--5-7 a--2--5--       
.start:                                             ;- d--2--5-7 a--2--5--       ======== FIRST ITERATION STARTS HERE ========
    move.l      (a4)+, a7                           ;- d--2--5-7 a--2--5--  12   byte *dst = render_buffer + offs[x];
                                                    ;C d--2--5-7 a--2--5--       
                                                    ;C d--2--5-7 a--2--5--       -------- ymin/ymax not checked --------
    move.l      (a4)+, d2                           ;C d-----5-7 a--2--5--  12   Load ymin/ymax
    cmp.w       d1, d2                              ;C d-----5-7 a--2--5--   4   ymax>=y1
    bge         .cmp_1                              ;C d-----5-7 a--2--5--  12   
                                                    ;C d-----5-7 a--2--5--       
                                                    ;C d-----5-7 a--2--5--       -------- ymin < y1 && ymax < y1 --------
    cmp.w       d0, d2                              ;C d-----5-7 a--2--5--   4   ymax>=y0
    bge         .cmp_2                              ;C d-----5-7 a--2--5--  12   
                                                    ;C d-----5-7 a--2--5--       
                                                    ;C d-----5-7 a--2--5--       -------- ymin < y0 && ymax < y0 --------
                                                    ;- d-----5-7 a--2--5--       ======== [Sky] ========
                                                    ;S d-----5-7 a--2--5--       -------- [Sky] --------
    lea.l       _FUNC_SKYCOPY+ENGINE_Y_MAX*2, a5    ;S d-----5-7 a--2-----  12   Sky copy func
    move.l      (a0)+, a2                           ;S d-----5-7 a--------  12   
    swap.w      d2                                  ;S d-----5-7 a--------   4   Select ymin
    add.w       d2, a2                              ;S d-----5-7 a--------   8   sky offset by ymin
    move.w      d2, d7                              ;S d-----5-- a--2-----   4   
    swap.w      d2                                  ;S d-----5-- a--2-----   4   Select ymax
    sub.w       d2, d7                              ;S d-----5-- a--2-----   4   
    add.w       d7, d7                              ;S d--2--5-- a--2-----   4   
    lea.l       .ret_3(pc), a6                      ;S d--2--5-- a--2-----   8   
    jmp         (a5,d7.w)                           ;S d--2--5-- a--2-----  14   -> .ret_3
.ret_3:                                             ;S d--2--5-7 a--2--5--       
    moveq.l     #0, d2                              ;G d-----5-7 a--2--5--   4   Disable further rendering - clear ymin/ymax
    move.l      d2, rc_ymin-8(a4)                   ;G d-----5-7 a--2--5--  16   
    move.w      d4, rc_size_limit-8(a4)             ;G d--2--5-7 a--2--5--  12   
    bra         .end_loop                           ;- d--2--5-7 a--2--5--  12   
                                                    ;C d-----5-7 a--2--5--       
.cmp_2:                                             ;C d-----5-7 a--2--5--       -------- ymin < y1 && y0 <= ymax < y1 --------
    swap.w      d2                                  ;C d-----5-7 a--2--5--   4   Select ymin
    cmp.w       d0, d2                              ;C d-----5-7 a--2--5--   4   ymin>=y0
    bge         .cmp_4                              ;C d-----5-7 a--2--5--  12   
                                                    ;C d-----5-7 a--2--5--       
                                                    ;C d-----5-7 a--2--5--       -------- ymin < y0 && y0 <= ymax < y1 --------
                                                    ;- d-----5-7 a--2--5--       ======== [Sky Upper] ========
                                                    ;S d-----5-7 a--2--5--       -------- [Sky --------
    lea.l       _FUNC_SKYCOPY+ENGINE_Y_MAX*2, a5    ;S d-----5-7 a--2-----  12   Sky copy func
    move.l      (a0)+, a2                           ;S d-----5-7 a--------  12   
    add.w       d2, a2                              ;S d-----5-7 a--------   8   sky offset by ymin
    sub.w       d0, d2                              ;S d-----5-7 a--2-----   4   
    add.w       d2, d2                              ;S d-----5-7 a--2-----   4   
    lea.l       .ret_5(pc), a6                      ;S d-----5-7 a--2-----   8   
    jmp         (a5,d2.w)                           ;S d-----5-7 a--2-----  14   -> .ret_5
.ret_5:                                             ;S d-----5-7 a--2--5--       
                                                    ;R d-----5-7 a--2--5--       -------- Upper] --------
                                                    ;R d-----5-7 a--2--5--       -- Fill routine --
    move.w      d4, d7                              ;R d-----5-- a--2--5--   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d7                              ;R d-----5-- a--2--5--  12   
    and.w       #$1FFC, d7                          ;R d-----5-- a--2--5--   8   
    move.l      (a3,d7.w), a5                       ;R d-----5-- a--2-----  18   
                                                    ;T d-----5-7 a--2-----       -- Texcoord (no persp) --
    move.w      d6, d7                              ;T d-----5-- a--2-----   4   word tx = u1s;
    and.w       #$1F80, d7                          ;T d-----5-- a--2-----   8   tx &= $1F80;
    move.l      .asm_tex_base(pc), a2               ;T d-----5-- a--------  16   -- Texture source --
    add.w       d7, a2                              ;T d-----5-- a--------   8   
    swap.w      d2                                  ;R d-----5-7 a--2-----   4   Select ymax
    add.w       d2, d2                              ;R d-----5-7 a--2-----   4   bottom_offset = 4*ymax
    add.w       d2, d2                              ;R d-----5-7 a--2-----   4   
    move.w      #$4ED6, (a5,d2.w)                   ;R d-----5-7 a--2-----  18   #$4ED6 -> (fill_routine, bottom_offset)
    move.w      d0, d7                              ;R d-----5-- a--2-----   4   top_offset = 4*ystart
    add.w       d7, d7                              ;R d-----5-- a--2-----   4   
    add.w       d7, d7                              ;R d-----5-- a--2-----   4   
    lea.l       .ret_6(pc), a6                      ;R d-----5-- a--2-----   8   
    jmp         (a5,d7.w)                           ;R d-----5-- a--2-----  14   -> .ret_6
.ret_6:                                             ;R d-----5-7 a--2-----       
    move.w      #$1EEA, (a5,d2.w)                   ;R d-----5-7 a--2-----  18   #$1EEA -> (fill_routine, bottom_offset)
    moveq.l     #0, d2                              ;G d-----5-7 a--2--5--   4   Disable further rendering - clear ymin/ymax
    move.l      d2, rc_ymin-8(a4)                   ;G d-----5-7 a--2--5--  16   
    move.w      d4, rc_size_limit-8(a4)             ;G d--2--5-7 a--2--5--  12   
    bra         .end_loop                           ;- d--2--5-7 a--2--5--  12   
                                                    ;C d-----5-7 a--2--5--       
.cmp_4:                                             ;C d-----5-7 a--2--5--       -------- y0 <= ymin < y1 && y0 <= ymax < y1 --------
                                                    ;- d-----5-7 a--2--5--       ======== [Upper] ========
                                                    ;R d-----5-7 a--2--5--       -------- [Upper] --------
                                                    ;R d-----5-7 a--2--5--       -- Fill routine --
    move.w      d4, d7                              ;R d-----5-- a--2--5--   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d7                              ;R d-----5-- a--2--5--  12   
    and.w       #$1FFC, d7                          ;R d-----5-- a--2--5--   8   
    move.l      (a3,d7.w), a5                       ;R d-----5-- a--2-----  18   
                                                    ;T d-----5-7 a--2-----       -- Texcoord (no persp) --
    move.w      d6, d7                              ;T d-----5-- a--2-----   4   word tx = u1s;
    and.w       #$1F80, d7                          ;T d-----5-- a--2-----   8   tx &= $1F80;
    move.l      .asm_tex_base(pc), a2               ;T d-----5-- a--------  16   -- Texture source --
    add.w       d7, a2                              ;T d-----5-- a--------   8   
    swap.w      d2                                  ;R d-----5-7 a--2-----   4   Select ymax
    add.w       d2, d2                              ;R d-----5-7 a--2-----   4   bottom_offset = 4*ymax
    add.w       d2, d2                              ;R d-----5-7 a--2-----   4   
    move.w      #$4ED6, (a5,d2.w)                   ;R d-----5-7 a--2-----  18   #$4ED6 -> (fill_routine, bottom_offset)
    swap.w      d2                                  ;R d-----5-7 a--2-----   4   Select ymin
    add.w       d2, d2                              ;R d-----5-7 a--2-----   4   top_offset = 4*ymin
    add.w       d2, d2                              ;R d-----5-7 a--2-----   4   
    lea.l       .ret_7(pc), a6                      ;R d-----5-7 a--2-----   8   
    jmp         (a5,d2.w)                           ;R d-----5-7 a--2-----  14   -> .ret_7
.ret_7:                                             ;R d-----5-7 a--2-----       
    swap.w      d2                                  ;R d-----5-7 a--2-----   4   Select ymax
    move.w      #$1EEA, (a5,d2.w)                   ;R d-----5-7 a--2-----  18   #$1EEA -> (fill_routine, bottom_offset)
    addq.l      #4, a0                              ;S d--2--5-7 a--2--5--   8   SKY was skipped
    moveq.l     #0, d2                              ;G d-----5-7 a--2--5--   4   Disable further rendering - clear ymin/ymax
    move.l      d2, rc_ymin-8(a4)                   ;G d-----5-7 a--2--5--  16   
    move.w      d4, rc_size_limit-8(a4)             ;G d--2--5-7 a--2--5--  12   
    bra         .end_loop                           ;- d--2--5-7 a--2--5--  12   
                                                    ;C d-----5-7 a--2--5--       
.cmp_1:                                             ;C d-----5-7 a--2--5--       -------- y1 <= ymax --------
    swap.w      d2                                  ;C d-----5-7 a--2--5--   4   Select ymin
    cmp.w       d0, d2                              ;C d-----5-7 a--2--5--   4   ymin>=y0
    bge         .cmp_8                              ;C d-----5-7 a--2--5--  12   
                                                    ;C d-----5-7 a--2--5--       
                                                    ;C d-----5-7 a--2--5--       -------- ymin < y0 && y1 <= ymax --------
                                                    ;- d-----5-7 a--2--5--       ======== [Sky Upper Floor] ========
                                                    ;S d-----5-7 a--2--5--       -------- [Sky --------
    lea.l       _FUNC_SKYCOPY+ENGINE_Y_MAX*2, a5    ;S d-----5-7 a--2-----  12   Sky copy func
    move.l      (a0)+, a2                           ;S d-----5-7 a--------  12   
    add.w       d2, a2                              ;S d-----5-7 a--------   8   sky offset by ymin
    sub.w       d0, d2                              ;S d-----5-7 a--2-----   4   
    add.w       d2, d2                              ;S d-----5-7 a--2-----   4   
    lea.l       .ret_9(pc), a6                      ;S d-----5-7 a--2-----   8   
    jmp         (a5,d2.w)                           ;S d-----5-7 a--2-----  14   -> .ret_9
.ret_9:                                             ;S d-----5-7 a--2--5--       
                                                    ;R d-----5-7 a--2--5--       -------- Upper --------
                                                    ;R d-----5-7 a--2--5--       -- Fill routine --
    move.w      d4, d7                              ;R d-----5-- a--2--5--   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d7                              ;R d-----5-- a--2--5--  12   
    and.w       #$1FFC, d7                          ;R d-----5-- a--2--5--   8   
    move.l      (a3,d7.w), a5                       ;R d-----5-- a--2-----  18   
                                                    ;T d-----5-7 a--2-----       -- Texcoord (no persp) --
    move.w      d6, d7                              ;T d-----5-- a--2-----   4   word tx = u1s;
    and.w       #$1F80, d7                          ;T d-----5-- a--2-----   8   tx &= $1F80;
    move.l      .asm_tex_base(pc), a2               ;T d-----5-- a--------  16   -- Texture source --
    add.w       d7, a2                              ;T d-----5-- a--------   8   
    move.w      d1, d7                              ;R d-----5-- a--2-----   4   bottom_offset = 4*yend
    add.w       d7, d7                              ;R d-----5-- a--2-----   4   
    add.w       d7, d7                              ;R d-----5-- a--2-----   4   
    add.w       d7, a5                              ;R d-----5-- a--2-----   8   
    move.w      #$4ED6, (a5)                        ;R d-----5-7 a--2-----  12   #$4ED6 -> (fill_routine, bottom_offset)
    move.w      d0, d7                              ;R d-----5-- a--2-----   4   top_offset = 4*ystart
    sub.w       d1, d7                              ;R d-----5-- a--2-----   4   
    add.w       d7, d7                              ;R d-----5-- a--2-----   4   
    add.w       d7, d7                              ;R d-----5-- a--2-----   4   
    lea.l       .ret_10(pc), a6                     ;R d-----5-- a--2-----   8   
    jmp         (a5,d7.w)                           ;R d-----5-- a--2-----  14   -> .ret_10
.ret_10:                                            ;R d-----5-7 a--2-----       
    move.w      #$1EEA, (a5)                        ;R d-----5-7 a--2-----  12   #$1EEA -> (fill_routine, bottom_offset)
                                                    ;F d-----5-7 a--2--5--       -------- Floor] --------
    swap.w      d2                                  ;F d-----5-7 a--2--5--   4   Select ymax
    sub.w       d1, d2                              ;F d-----5-7 a--2--5--   4   
    neg.w       d2                                  ;F d-----5-7 a--2--5--   4   
    add.w       d2, d2                              ;F d-----5-7 a--2--5--   4   
    moveq.l     #0, d7                              ;G d-----5-- a--2--5--   4   Disable further rendering - clear ymin/ymax
    move.l      d7, rc_ymin-8(a4)                   ;G d-----5-- a--2--5--  16   
    move.w      d4, rc_size_limit-8(a4)             ;G d-----5-7 a--2--5--  12   
    lea.l       .end_loop(pc), a6                   ;F d-----5-7 a--2--5--   8   
    jmp         (a1,d2.w)                           ;F d-----5-7 a--2--5--  14   -> .end_loop
                                                    ;C d-----5-7 a--2--5--       
.cmp_8:                                             ;C d-----5-7 a--2--5--       -------- y0 <= ymin && y1 <= ymax --------
    cmp.w       d1, d2                              ;C d-----5-7 a--2--5--   4   ymin>=y1
    bge         .cmp_11                             ;C d-----5-7 a--2--5--  12   
                                                    ;C d-----5-7 a--2--5--       
                                                    ;C d-----5-7 a--2--5--       -------- y0 <= ymin < y1 && y1 <= ymax --------
                                                    ;- d-----5-7 a--2--5--       ======== [Upper Floor] ========
                                                    ;R d-----5-7 a--2--5--       -------- [Upper --------
                                                    ;R d-----5-7 a--2--5--       -- Fill routine --
    move.w      d4, d7                              ;R d-----5-- a--2--5--   4   int lut_offset = (((word)s1)>>3) & ~3;
    lsr.w       #3, d7                              ;R d-----5-- a--2--5--  12   
    and.w       #$1FFC, d7                          ;R d-----5-- a--2--5--   8   
    move.l      (a3,d7.w), a5                       ;R d-----5-- a--2-----  18   
                                                    ;T d-----5-7 a--2-----       -- Texcoord (no persp) --
    move.w      d6, d7                              ;T d-----5-- a--2-----   4   word tx = u1s;
    and.w       #$1F80, d7                          ;T d-----5-- a--2-----   8   tx &= $1F80;
    move.l      .asm_tex_base(pc), a2               ;T d-----5-- a--------  16   -- Texture source --
    add.w       d7, a2                              ;T d-----5-- a--------   8   
    move.w      d1, d7                              ;R d-----5-- a--2-----   4   bottom_offset = 4*yend
    add.w       d7, d7                              ;R d-----5-- a--2-----   4   
    add.w       d7, d7                              ;R d-----5-- a--2-----   4   
    add.w       d7, a5                              ;R d-----5-- a--2-----   8   
    move.w      #$4ED6, (a5)                        ;R d-----5-7 a--2-----  12   #$4ED6 -> (fill_routine, bottom_offset)
    sub.w       d1, d2                              ;R d-----5-7 a--2-----   4   
    add.w       d2, d2                              ;R d-----5-7 a--2-----   4   top_offset = 4*ymin
    add.w       d2, d2                              ;R d-----5-7 a--2-----   4   
    lea.l       .ret_12(pc), a6                     ;R d-----5-7 a--2-----   8   
    jmp         (a5,d2.w)                           ;R d-----5-7 a--2-----  14   -> .ret_12
.ret_12:                                            ;R d-----5-7 a--2-----       
    move.w      #$1EEA, (a5)                        ;R d-----5-7 a--2-----  12   #$1EEA -> (fill_routine, bottom_offset)
                                                    ;F d-----5-7 a--2--5--       -------- Floor] --------
    swap.w      d2                                  ;F d-----5-7 a--2--5--   4   Select ymax
    sub.w       d1, d2                              ;F d-----5-7 a--2--5--   4   
    neg.w       d2                                  ;F d-----5-7 a--2--5--   4   
    add.w       d2, d2                              ;F d-----5-7 a--2--5--   4   
    addq.l      #4, a0                              ;S d-----5-7 a--2--5--   8   SKY was skipped
    moveq.l     #0, d7                              ;G d-----5-- a--2--5--   4   Disable further rendering - clear ymin/ymax
    move.l      d7, rc_ymin-8(a4)                   ;G d-----5-- a--2--5--  16   
    move.w      d4, rc_size_limit-8(a4)             ;G d-----5-7 a--2--5--  12   
    lea.l       .end_loop(pc), a6                   ;F d-----5-7 a--2--5--   8   
    jmp         (a1,d2.w)                           ;F d-----5-7 a--2--5--  14   -> .end_loop
                                                    ;C d-----5-7 a--2--5--       
.cmp_11:                                            ;C d-----5-7 a--2--5--       -------- y1 <= ymin && y1 <= ymax --------
                                                    ;- d-----5-7 a--2--5--       ======== [Floor] ========
                                                    ;F d-----5-7 a--2--5--       -------- [Floor] --------
    move.w      d2, d7                              ;F d-----5-- a--2--5--   4   
    swap.w      d2                                  ;F d-----5-- a--2--5--   4   Select ymax
    sub.w       d2, d7                              ;F d-----5-- a--2--5--   4   
    add.w       d7, d7                              ;F d--2--5-- a--2--5--   4   
    addq.l      #4, a0                              ;S d--2--5-- a--2--5--   8   SKY was skipped
    moveq.l     #0, d2                              ;G d-----5-- a--2--5--   4   Disable further rendering - clear ymin/ymax
    move.l      d2, rc_ymin-8(a4)                   ;G d-----5-- a--2--5--  16   
    move.w      d4, rc_size_limit-8(a4)             ;G d--2--5-- a--2--5--  12   
    lea.l       .end_loop(pc), a6                   ;F d--2--5-- a--2--5--   8   
    jmp         (a1,d7.w)                           ;F d--2--5-- a--2--5--  14   -> .end_loop
                                                    ;- d--2--5-7 a--2--5--       
                                                    ;- d--2--5-7 a--2--5--       
.end_loop:                                          ;- d--2--5-7 a--2--5--       ======== LOOP END ========
    dbra.w      d3, .loop                           ;- d--2--5-7 a--2--5--  12   
.end:                                               ;o d01234567 a012345--       Cleanup & exit
    move.l      _a7_save, a7                        ;o d01234567 a012345--  20   
    rts                                             ;o d01234567 a012345--  16   
.asm_tex_base:                                      ;o d01234567 a012345--       
    move.w      #0, d0                              ;o d-1234567 a012345--   8   reserve 32-bit
