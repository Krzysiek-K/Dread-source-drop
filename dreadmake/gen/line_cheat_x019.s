
    public _Dread_LineCoreCheat_x019
_Dread_LineCoreCheat_x019:

    move.l      a7, _a7_save                        ;s d01------ a01-3----  20   
                                                    ;s d01------ a01-3----       Loop initialization
    move.b      line_ceil_col(a5), d5               ;s d01------ a01-3----  12   Prepare ceil
    swap.w      d5                                  ;s d01------ a01-3----   4   
    move.b      line_floor_col(a5), d5              ;s d01------ a01-3----  12   Prepare floor
    lea.l       _render_columns, a5                 ;s d01------ a01-3----  12   Setup column pointer
    add.w       d2, a5                              ;s d01------ a01-3----   8   
    bra         .start                              ;s d012----- a01-3----  12   End of header
.loop:                                              ;I d012----- a01-3----       
    add.w       _asm_line_ds(pc), d4                ;I d012----- a01-3----  12   s1 += ds;
    add.l       d7, d6                              ;I d012----- a01-3----   8   u1s += du;
    addq.w      #8, a5                              ;I d012----- a01-3----   8   
.start:                                             ;- d012----- a01-3----       // FIRST ITERATION STARTS HERE
    move.w      rc_ymax(a5), d1                     ;C d0-2----- a01-3----  12   
    beq         .hidden_skip                        ;C d0-2----- a01-3----  12   
    move.l      (a5), a7                            ;- d0-2----- a01-3----  12   byte *dst = render_buffer + offs[x];
    move.w      d4, d0                              ;- d--2----- a01-3----   4   int size = (((word)s1)>>3) & ~3;
    lsr.w       #3, d0                              ;- d--2----- a01-3----  12   
    and.w       #$FFFC, d0                          ;- d--2----- a01-3----   8   
    lea.l       _SIZE_LINE_CHEAT, a4                ;- d--2----- a01-3----  12   const LineCheat *lc = (LineCheat*)((byte*)SIZE_LINE_CHEAT + size);
    move.l      (a4,d0.w), a4                       ;- d--2----- a01-3----  18   
                                                    ;F d0-2----- a01-3----       === [[Ceil
    swap.w      d5                                  ;F d0-2----- a01-3----   4   
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MAX*2, a0  ;F d0-2----- a-1-3----  12   Color filler func
    move.w      rc_ymin(a5), d0                     ;F d--2----- a-1-3----  12   jsr(fn1, d0) LUT.w:	Ceil length(x2)
    add.w       d0, a7                              ;F d--2----- a-1-3----   8   skip ymin
    add.w       lc_ceil_len_m96(a4), d0             ;F d--2----- a-1-3----  12   shorten to end at texture edge
    bge         .skyceil_culled                     ;C d--2----- a-1-3----  12   
    lea.l       .ceil_ret(pc), a6                   ;F d--2----- a-1-3----   8   
    jmp         (a0,d0.w)                           ;F d--2----- a-1-3----  14   
.ceil_ret:                                          ;F d0-2----- a01-3----       
                                                    ;T d0-2----- a01-3----       === Texcoord
    move.l      d6, d0                              ;T d--2----- a01-3----   4   word tx = u1s & $1F80;
    and.w       #$1F80, d0                          ;T d--2----- a01-3----   8   tx &= $1F80;
    move.l      _asm_tex_base(pc), a2               ;T d--2----- a01-3----  16   
    add.w       d0, a2                              ;T d--2----- a01-3----   8   void *tex @ a2 = _asm_tex_base + tx;
                                                    ;R d0-2----- a01-3----       === Upper
    move.l      lc_upper_start_m96(a4), a0          ;R d0-2----- a-1-3----  16   void *fn = line_cheat.upper_start_m64;
    move.w      lc_upper_len_m96_m64(a4), d0        ;R d--2----- a-1-3----  12   $ fn[line_cheat.upper_len_m64_0] = $4ED6;  (L = -64..0 / -128..0)
    move.w      #$4ED6, (a0,d0.w)                   ;R d--2----- a-1-3----  18   
    lea.l       .upper_ret(pc), a6                  ;R d--2----- a-1-3----   8   
    jmp         (a0)                                ;R d--2----- a-1-3----   8   $ call_a6(fn)	(Z = -64 / -128)
.upper_ret:                                         ;R d--2----- a-1-3----       
    move.w      #$1EEA, (a0,d0.w)                   ;R d--2----- a-1-3----  18   $ fn[line_cheat.upper_len_m64_0] = $1EEA;
                                                    ;G d0-2----- a01-3----       === Gap
    move.w      lc_ceil_ymin_m64(a4), rc_ymin(a5)   ;G d0-2----- a01-3----  20   ymin
    add.w       lc_hole_size_m64_0(a4), a7          ;G d0-2----- a01-3----  16   
                                                    ;F d0-2----- a01-3----       === Floor]
.floor_start:                                       ;F d0-2----- a01-3----       
    swap.w      d5                                  ;F d0-2----- a01-3----   4   
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MID*2, a0  ;F d0-2----- a-1-3----  12   Color filler func
    move.w      lc_floor_start_0(a4), d0            ;F d--2----- a-1-3----  12   jsr (fn1,d0)		LUT.w:	Fill func start		(x2)	(fills to screen end)
    move.w      d0, rc_ymax(a5)                     ;F d--2----- a-1-3----   8   ymax = Gap end
    sub.w       d1, d0                              ;F d--2----- a-1-3----   4   clip ymax
    lea.l       .floor_ret(pc), a6                  ;F d-12----- a-1-3----   8   
    jmp         (ENGINE_Y_MAX-ENGINE_Y_MID)*2(a0,d0.w);F d-12----- a-1-3----  14   
.floor_ret:                                         ;F d012----- a01-3----       
.hidden_skip:                                       ;- d012----- a01-3----       
    dbra.w      d3, .loop                           ;- d012----- a01-3----  12   LOOP END
    bra         .end                                ;- d012----- a01-3----  12   End of variant
.skyceil_culled:                                    ;- d0-2----- a01-3----       Variant 1
                                                    ;T d0-2----- a01-3----       === Texcoord
    move.l      d6, d0                              ;T d--2----- a01-3----   4   word tx = u1s & $1F80;
    and.w       #$1F80, d0                          ;T d--2----- a01-3----   8   tx &= $1F80;
    move.l      _asm_tex_base(pc), a2               ;T d--2----- a01-3----  16   
    add.w       d0, a2                              ;T d--2----- a01-3----   8   void *tex @ a2 = _asm_tex_base + tx;
                                                    ;R d0-2----- a01-3----       === ![Upper
    move.w      rc_ymin(a5), d2                     ;R d0------- a01-3----  12   ymin
    add.w       #-ENGINE_Y_MID*2, d2                ;R d0------- a01-3----   8   
    add.w       d2, d2                              ;R d0------- a01-3----   4   
    move.w      lc_upper_endpos_m64(a4), d0         ;R d-------- a01-3----  12   $ fn[line_cheat.upper_len_m64_0] = $4ED6;  (L = -64..0 / -128..0)
    cmp.w       d0, d2                              ;C d-------- a01-3----   4   
    bge         .upper_skip1                        ;C d-------- a01-3----  12   
    move.l      lc_scaler_fn(a4), a0                ;R d-------- a-1-3----  16   void *fn = line_cheat.upper_start_m64;
    move.w      #$4ED6, (a0,d0.w)                   ;R d-------- a-1-3----  18   
    lea.l       .upper_ret1(pc), a6                 ;R d-------- a-1-3----   8   
    jmp         (a0,d2.w)                           ;R d-------- a-1-3----  14   $ call_a6(fn)	(Z = -64 / -128)
.upper_ret1:                                        ;R d--2----- a-1-3----       
    move.w      #$1EEA, (a0,d0.w)                   ;R d--2----- a-1-3----  18   $ fn[line_cheat.upper_len_m64_0] = $1EEA;
.upper_skip1:                                       ;R d0-2----- a01-3----       
                                                    ;G d0-2----- a01-3----       === Gap
    move.w      lc_ceil_ymin_m64(a4), rc_ymin(a5)   ;G d0-2----- a01-3----  20   ymin
    add.w       lc_hole_size_m64_0(a4), a7          ;G d0-2----- a01-3----  16   
                                                    ;F d0-2----- a01-3----       === Floor]
.floor_start1:                                      ;F d0-2----- a01-3----       
    swap.w      d5                                  ;F d0-2----- a01-3----   4   
    lea.l       _FUNC_COLORFILL+ENGINE_Y_MID*2, a0  ;F d0-2----- a-1-3----  12   Color filler func
    move.w      lc_floor_start_0(a4), d0            ;F d--2----- a-1-3----  12   jsr (fn1,d0)		LUT.w:	Fill func start		(x2)	(fills to screen end)
    move.w      d0, rc_ymax(a5)                     ;F d--2----- a-1-3----   8   ymax = Gap end
    sub.w       d1, d0                              ;F d--2----- a-1-3----   4   clip ymax
    lea.l       .floor_ret1(pc), a6                 ;F d-12----- a-1-3----   8   
    jmp         (ENGINE_Y_MAX-ENGINE_Y_MID)*2(a0,d0.w);F d-12----- a-1-3----  14   
.floor_ret1:                                        ;F d012----- a01-3----       
    dbra.w      d3, .loop                           ;- d012----- a01-3----  12   LOOP END
.end:                                               ;o d012----- a01-3----       Cleanup & exit
    move.l      _a7_save(pc), a7                    ;o d012----- a01-3----  16   
    rts                                             ;o d012----- a01-3----  16   
