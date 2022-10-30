


	public _Blit_DrawImage

; void Blit_DrawImage(__a2 word *screen[], __a3 const word *image, __d2 word xp, __d3 word yp);
;
;	d2	- xp
;	d3	- yp
;	d4	* 0
;	d5	* width
;	d6	* planestride
;	d7	* h / BLTSIZE
;
;	a2	- *screen[]
;	a3	- image

_Blit_DrawImage:
	movem.l	d2-d7/a2-a6,-(a7)

	lea.l	$dff000, a6
	moveq.l	#0, d4

	move.w	4(a3), d5											;	word width = image[2]>>1;			// in words
	lsr.w	#1, d5												;
	move.w	2(a3), d7											;	word h = image[1];										// [d7]
	move.w	d7, d6												;	word planestride = width*h;			// in words			// [d6]
	mulu.w	d5, d6												;
																;
	tst.w	DMACONR(a6)											;	BlitWait();
.waitblit_1:													;
	btst	#6, DMACONR(a6)										;
	bne.b	.waitblit_1											;
	
	
	move.w	#$0FCA, d1											;	word func = 0x0FCA;										// [d1]
	move.l	#$FFFFFFFF, BLTAWM(a6)								;	BLTAFWM = 0xFFFF;
																;	BLTALWM = 0xFFFF;
	move.w	d4, BLTAMOD(a6)										;	BLTAMOD = 0;				// mask
	move.w	d4, BLTBMOD(a6)										;	BLTBMOD = 0;				// data
	
	move.w	d2, d0												;	if( xp & 0x0F )											// [d0]
	andi.w	#$000F, d0											;
	beq		.noshift											;	{
	lsl.w	#8, d0												;		func |= (xp & 0x0F)<<12;							// [d1]
	lsl.w	#4, d0												;
	or.w	d0, d1												;
	addq.w	#1, d5												;		width++;
	move.w	d4, BLTALWM(a6)										;		BLTALWM = 0;
	move.w	#-2, BLTAMOD(a6)									;		BLTAMOD = -2;
	move.w	#-2, BLTBMOD(a6)									;		BLTBMOD = -2;
.noshift:														;	}
																;
	move.w	d1, BLTCON0(a6)										;	BLTCON0 = func;
	and.w	#$F000, d1											;	BLTCON1 = func & 0xF000;
	move.w	d1, BLTCON1(a6)										;

	moveq	#40, d1												;	BLTCMOD = 40 - (width<<1);	// screen
	sub.w	d5, d1												;	BLTDMOD = 40 - (width<<1);	// screen
	sub.w	d5, d1												;
	move.w	d1, BLTCMOD(a6)										;
	move.w	d1, BLTDMOD(a6)										;
																;
	
	
	lsr.w	#3, d2												;	word dstoff = ((xp>>3)&~1) + yp*40;		// in bytes		// [d2]
	and.w	#$FFFE, d2											;
	mulu.w	#40, d3												;
	add.w	d3, d2												;
	ext.l	d2													; [clear upper word]


	adda.l	#6, a3 												;	word *src = image + 3;									// [a3]
	move.l	a3, BLTBPT(a6)										;	BLTBPT = src;
	mulu.w	#10, d6												;															// [a3] = src + planestride*5	// in bytes
	add.l	d6, a3												;

	lsl.w	#6, d7												;															// [d7] = (h<<6) | width;
	or.w	d5, d7												;


	moveq.l	#4, d0												;	for( int plane=0; plane<5; plane++ )
.plane_loop:													;	{
	move.l	a3, BLTAPT(a6)										;		BLTAPT = src + planestride*5;
	move.l	(a2)+, d3											;		BLTCPT = ((byte*)screen_A[plane]) + dstoff;
	add.l	d2, d3												;		BLTDPT = ((byte*)screen_A[plane]) + dstoff;
	move.l	d3, BLTCPT(a6)										;
	move.l	d3, BLTDPT(a6)										;

																;
	move.w	d7, BLTSIZE(a6)										;		BLTSIZE = (h<<6) | width;

	tst.w	DMACONR(a6)											;	BlitWait();
.waitblit_2:													;
	btst	#6, DMACONR(a6)										;
	bne.b	.waitblit_2											;
	dbra.w	d0, .plane_loop										;	}


	movem.l	(a7)+,d2-d7/a2-a6,
	rts
