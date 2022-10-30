        public _inflate
static_huffman_prefix:
        dc.b $ff, $5b, $00, $6c, $03, $36, $db
        dc.b $b6, $6d, $db, $b6, $6d, $db, $b6
        dc.b $cd, $db, $b6, $6d, $db, $b6, $6d
        dc.b $db, $a8, $6d, $ce, $8b, $6d, $3b
build_code:
        movem.l d0-d7,-(a6)


        moveq #(16 +1)/2,d1
        moveq #0,d2
_inflate_1_1: move.l d2,-(a6)
        dbf d1,_inflate_1_1


        subq.w #1,d0
        move.w d0,d1
        move.l a0,a2
_inflate_1_2: move.b (a2)+,d2



        add.b d2,d2
        addq.w #1,(a6,d2.w)

        dbf d1,_inflate_1_2


        move.l a6,a2
        moveq #16 -1,d1
        moveq #0,d2
        move.w d2,(a6)
_inflate_1_3: add.w (a2),d2
        add.w d2,d2
        move.w d2,(a2)+
        dbf d1,_inflate_1_3


        move.w d0,d1
        moveq #127,d4
        move.l a0,a2
_inflate_1_4: moveq #0,d5
        move.b (a2)+,d5
        beq _inflate_4_1
        subq.w #1,d5
        move.w d5,d6




        add.w d6,d6
        move.w (a6,d6.w),d3
        addq.w #1,(a6,d6.w)
        move.w d5,d6


        moveq #0,d2
_inflate_9_1: lsr.w #1,d3
        roxl.w #1,d2
        dbf d6,_inflate_9_1
        move.b d2,d3
        add.w d3,d3
        move.w d0,d6
        sub.w d1,d6
        cmp.w (((16 +1)/2)+1)*4+6(a6),d6
        bls _inflate_9_2
        lsl.w #2,d6
_inflate_9_2: cmp.b #9-1,d5
        bcc codelen_gt_8

codelen_le_8:
        lsl.w #3,d6
        or.b d5,d6
        moveq #0,d2
        addq.b #2,d5
        bset d5,d2
        move.w d2,d7
        neg.w d7
        and.w #511,d7
        or.w d7,d3
_inflate_9_3: move.w d6,(a1,d3.w)
        sub.w d2,d3
        bcc _inflate_9_3
        bra _inflate_4_1

codelen_gt_8:
        lsr.w #8,d2
        subq.b #8,d5
        lea (a1,d3.w),a3

_inflate_2_1:
        move.w (a3),d7
        bne _inflate_3_1

        addq.w #1,d4
        move.w d4,d7
        bset #15,d7
        move.w d7,(a3)
_inflate_3_1:
        lsr.b #1,d2
        addx.w d7,d7



        add.w d7,d7
        lea (a1,d7.w),a3

_inflate_3_2: dbf d5,_inflate_2_1


        move.w d6,(a3)
_inflate_4_1: dbf d1,_inflate_1_4

        lea (((16 +1)/2)+1)*4(a6),a6
        movem.l (a6)+,d0-d7
        rts



STREAM_NEXT_SYMBOL:macro
        moveq #0,d0
        moveq #7,d1
        cmp.b d1,d6
        bhi _inflate_99_1\@

        move.b (a5)+,d0
        lsl.w d6,d0
        or.w d0,d5
        addq.b #8,d6
        moveq #0,d0
_inflate_99_1\@:
        move.b d5,d0



        add.w d0,d0
        move.w (a0,d0.w),d0

        bpl _inflate_99_2\@

        lsr.w #8,d5
        subq.b #8,d6
_inflate_98_1\@:
        subq.b #1,d6
        bcc _inflate_97_1\@
        move.b (a5)+,d5
        moveq #7,d6
_inflate_97_1\@: lsr.w #1,d5
        addx.w d0,d0



        add.w d0,d0
        move.w (a0,d0.w),d0

        bmi _inflate_98_1\@
        bra _inflate_98_2\@
_inflate_99_2\@:
        and.b d0,d1
        addq.b #1,d1
        lsr.w d1,d5
        sub.b d1,d6
        lsr.w #3,d0
_inflate_98_2\@:
	endm
STREAM_NEXT_BITS:macro
_inflate_99_3\@: moveq #0,d0
        cmp.b d1,d6
        bcc _inflate_99_4\@
        move.b (a5)+,d0
        lsl.l d6,d0
        or.l d0,d5
        addq.b #8,d6
        bra _inflate_99_3\@
_inflate_99_4\@: bset d1,d0
        subq.w #1,d0
        and.w d5,d0
        lsr.l d1,d5
        sub.b d1,d6
	endm
stream_next_bits:
        STREAM_NEXT_BITS
        rts



uncompressed_block:


        lsr.w #3,d6
        sub.w d6,a5





        moveq #0,d5
        moveq #0,d6

        moveq #16,d1
        bsr stream_next_bits
        addq.w #2,a5
        subq.w #1,d0
_inflate_1_5: move.b (a5)+,(a4)+
        dbf d0,_inflate_1_5
        rts
static_huffman:
        movem.l d5-d6/a5,-(a6)
        moveq #0,d5
        moveq #0,d6
        lea static_huffman_prefix(pc),a5
        move.w #((((2 +2)+288 +32)+(256*2+((288)-9)*4))+(256*2+((32)-9)*4))/4-2,d0
        bra _inflate_1_6



dynamic_huffman:

        move.w #(((((2 +2)+288 +32)+(256*2+((288)-9)*4))+(256*2+((32)-9)*4))+3*4)/4-2,d0
_inflate_1_6: moveq #0,d1
_inflate_1_7: move.l d1,-(a6)
        dbf d0,_inflate_1_7

        moveq #5,d1
        bsr stream_next_bits
        add.w #257,d0
        move.w d0,-(a6)

        moveq #5,d1
        bsr stream_next_bits
        addq.w #1,d0
        move.w d0,-(a6)

        moveq #4,d1
        bsr stream_next_bits
        addq.w #4-1,d0

        lea codelen_order(pc),a1
        lea (2 +2)(a6),a0
        moveq #0,d2
        move.w d0,d3
_inflate_1_8: moveq #3,d1
        bsr stream_next_bits
        move.b (a1)+,d2
        move.b d0,(a0,d2.w)
        dbf d3,_inflate_1_8

        lea ((2 +2)+288 +32)(a6),a1
        moveq #19,d0

        moveq #127,d1

        bsr build_code

        move.w 2(a6),d2
        add.w (a6),d2
        subq.w #1,d2
        move.l a0,a2
        move.l a1,a0
_inflate_1_9: STREAM_NEXT_SYMBOL
        cmp.b #16,d0
        bcs c_lit
        beq c_16
        cmp.b #17,d0
        beq c_17
c_18:
        moveq #7,d1
        bsr stream_next_bits
        addq.w #11-3,d0
        bra _inflate_2_2
c_17:
        moveq #3,d1
        bsr stream_next_bits
_inflate_2_2: moveq #0,d1
        bra _inflate_3_3
c_16:
        moveq #2,d1
        bsr stream_next_bits
        move.b -1(a2),d1
_inflate_3_3: addq.w #3-1,d0
        sub.w d0,d2
_inflate_4_2: move.b d1,(a2)+
        dbf d0,_inflate_4_2
        bra _inflate_5_1
c_lit:
        move.b d0,(a2)+
_inflate_5_1: dbf d2,_inflate_1_9




        moveq #0,d0
        move.w #(256*2+((19)-9)*4)/4-1,d1
_inflate_1_10: move.l d0,(a0)+
        dbf d1,_inflate_1_10




        lea (2 +2)(a6),a0
        move.w 2(a6),d0

        move.w #256,d1
        move.w d1,d4

        bsr build_code
        add.w d0,a0
        lea (((2 +2)+288 +32)+(256*2+((288)-9)*4))(a6),a1
        move.w (a6),d0

        moveq #0,d1

        bsr build_code

        tst.l ((((2 +2)+288 +32)+(256*2+((288)-9)*4))+(256*2+((32)-9)*4))+8(a6)
        beq decode_loop
        movem.l ((((2 +2)+288 +32)+(256*2+((288)-9)*4))+(256*2+((32)-9)*4))(a6),d5-d6/a5

decode_loop:
        lea ((2 +2)+288 +32)(a6),a0

_inflate_2_3: STREAM_NEXT_SYMBOL

        cmp.w d4,d0



        bcc _inflate_2_4

        move.b d0,(a4)+
        bra _inflate_2_3

_inflate_9_4:
        lea (((((2 +2)+288 +32)+(256*2+((288)-9)*4))+(256*2+((32)-9)*4))+3*4)(a6),a6
        rts
_inflate_2_4: beq _inflate_9_4




        lea ((((((((2 +2)+288 +32)+(256*2+((288)-9)*4))+(256*2+((32)-9)*4))+3*4))+4)+30*4)-257*4(a6),a2
        add.w d0,a2
        move.w (a2)+,d1
        STREAM_NEXT_BITS
        add.w (a2),d0
        move.w d0,d3
        lea (((2 +2)+288 +32)+(256*2+((288)-9)*4))(a6),a0
        STREAM_NEXT_SYMBOL



        lea (((((((2 +2)+288 +32)+(256*2+((288)-9)*4))+(256*2+((32)-9)*4))+3*4))+4)(a6),a2
        add.w d0,a2
        move.w (a2)+,d1
        STREAM_NEXT_BITS
        add.w (a2),d0
        move.l a4,a0
        sub.w d0,a0

        lsr.w #1,d3
        bcs _inflate_4_3
        subq.w #1,d3
_inflate_3_4: move.b (a0)+,(a4)+
_inflate_4_3: move.b (a0)+,(a4)+




        dbf d3,_inflate_3_4
        bra decode_loop
build_base_extrabits:



_inflate_1_11: move.w d0,d3
        lsr.w d4,d3
        subq.w #1,d3
        bcc _inflate_2_5
        moveq #0,d3
_inflate_2_5: moveq #0,d1
        bset d3,d1
        sub.w d1,d2
        move.w d2,-(a6)
        move.w d3,-(a6)
        dbf d0,_inflate_1_11



        rts


dispatch:
        dc.b uncompressed_block - uncompressed_block
        dc.b static_huffman - uncompressed_block
        dc.b dynamic_huffman - uncompressed_block

codelen_order:
        dc.b 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15



_inflate:
        movem.l d0-d6/a0-a5,-(a6)


        move.l #258,d2
        move.l d2,-(a6)
        addq.w #1,d2
        moveq #27,d0
        moveq #2,d4
        bsr build_base_extrabits


        move.w #32769,d2
        moveq #29,d0
        moveq #1,d4
        bsr build_base_extrabits


        moveq #0,d5
        moveq #0,d6

_inflate_1_12:
        moveq #3,d1
        bsr stream_next_bits
        move.l d0,-(a6)

        lsr.b #1,d0
        move.b dispatch(pc,d0.w),d0
        lea uncompressed_block(pc),a0
        jsr (a0,d0.w)

        move.l (a6)+,d0
        lsr.b #1,d0
        bcc _inflate_1_12


        lea (30+29)*4(a6),a6

        movem.l (a6)+,d0-d6/a0-a5
        rts
