

			xref	_vblank_pal
			xref	_vblank_hudpal
			xref	_vblank_playsound
			xref	_vblank_playwait
			;xref	_vblank_screen
			xref	_music_active



BASEPAGE_SIZE 			equ $100
STACK_SIZE				equ	$10000		
		

stvar_nvbls				equ		$454		; word
stvar__vblqueue			equ		$456		; *dword
stvar__hz_200			equ		$4BA		; dword

INT_ENABLE_A			equ		$FFFFFA07	; byte
INT_PENDING_A			equ		$FFFFFA0B	; byte
INT_INSERVICE_A			equ		$FFFFFA0F	; byte
INT_MASK_A				equ		$FFFFFA13	; byte
INT_ENABLE_B			equ		$FFFFFA09	; byte
INT_PENDING_B			equ		$FFFFFA0D	; byte
INT_INSERVICE_B			equ		$FFFFFA11	; byte
INT_MASK_B				equ		$FFFFFA15	; byte
TIMER_B_CONTROL			equ		$FFFFFA1B	; byte
TIMER_B_DATA			equ		$FFFFFA21	; byte
TIMER_CD_CONTROL		equ		$FFFFFA1D	; byte
TIMER_C_DATA			equ		$FFFFFA23	; byte
TIMER_D_DATA			equ		$FFFFFA25	; byte
COLOR00					equ		$FFFF8240	; word
VID_SCREEN_HIGH			equ		$FFFF8201	; byte
VID_SCREEN_MED			equ		$FFFF8203	; byte

USART_CONTROL			equ		$FFFFFA29	; byte
USART_RX_STATUS			equ		$FFFFFA2B	; byte
USART_TX_STATUS			equ		$FFFFFA2D	; byte
USART_DATA				equ		$FFFFFA2F	; byte


BLIT_HALFTONE_0			equ		$FFFF8A00	;	|word |Halftone-RAM, Word 0                                 |R/W (Blit)
						;						|  :  |    :     :     :  :                                 | :     :
						;		$FFFF8A1E		|word |Halftone-RAM, Word 15                                |R/W (Blit)
BLIT_SOURCE_INC_X		equ		$FFFF8A20	;	|word |Source X Increment                      (signed,even)|R/W (Blit)
BLIT_SOURCE_INC_Y		equ		$FFFF8A22	;	|word |Source Y Increment                      (signed,even)|R/W (Blit)
BLIT_SOURCE_ADDR		equ		$FFFF8A24	;	|long |Source Address Register                 (24 bit,even)|R/W (Blit)
BLIT_ENDMASK_1			equ		$FFFF8A28	;	|word |Endmask 1                     (First write of a line)|R/W (Blit)
BLIT_ENDMASK_2			equ		$FFFF8A2A	;	|word |Endmask 2                     (All other line writes)|R/W (Blit)
BLIT_ENDMASK_3			equ		$FFFF8A2C	;	|word |Endmask 3                      (Last write of a line)|R/W (Blit)
BLIT_DEST_INC_X			equ		$FFFF8A2E	;	|word |Destination X Increment                 (signed,even)|R/W (Blit)
BLIT_DEST_INC_Y			equ		$FFFF8A30	;	|word |Destination Y Increment                 (signed,even)|R/W (Blit)
BLIT_DEST_ADDR			equ		$FFFF8A32	;	|long |Destination Address Register            (24 bit,even)|R/W (Blit)
BLIT_WORDS_PER_LINE		equ		$FFFF8A36	;	|word |Words per Line in Bit-Block                 (0=65536)|R/W (Blit)
BLIT_LINES_PER_BLOCK	equ		$FFFF8A38	;	|word |Lines per Bit-Block                         (0=65536)|R/W (Blit)
BLIT_HALFTONE_OP		equ		$FFFF8A3A	;	|byte |Halftone Operation Register                   BIT 1 0|R/W (Blit)
											;   |     |00 - All ones ------------------------------------+-+|
											;   |     |01 - Halftone ------------------------------------+-+|
											;   |     |10 - Source --------------------------------------+-+|
											;   |     |11 - Source AND Halftone -------------------------+-'|
BLIT_LOGIC_OP			equ		$FFFF8A3B	;	|byte |Logical Operation Register                BIT 3 2 1 0|R/W (Blit)
											;	|     |0000 All zeros -------------------------------+-+-+-+|
											;	|     |0001 Source AND destination ------------------+-+-+-+|
											;	|     |0010 Source AND NOT destination --------------+-+-+-+|
											;	|     |0011 Source ----------------------------------+-+-+-+|
											;	|     |0100 NOT source AND destination --------------+-+-+-+|
											;	|     |0101 Destination -----------------------------+-+-+-+|
											;	|     |0110 Source XOR destination ------------------+-+-+-+|
											;	|     |0111 Source OR destination -------------------+-+-+-+|
											;	|     |1000 NOT source AND NOT destination ----------+-+-+-+|
											;	|     |1001 NOT source XOR destination --------------+-+-+-+|
											;	|     |1010 NOT destination -------------------------+-+-+-+|
											;	|     |1011 Source OR NOT destination ---------------+-+-+-+|
											;	|     |1100 NOT source ------------------------------+-+-+-+|
											;	|     |1101 NOT source OR destination ---------------+-+-+-+|
											;	|     |1110 NOT source OR NOT destination -----------+-+-+-+|
											;	|     |1111 All ones --------------------------------+-+-+-'|
BLIT_LINE_NUM			equ		$FFFF8A3C	;	|byte |Line Number Register              BIT 7 6 5 . 3 2 1 0|R/W (Blit)
											;	|     |BUSY ---------------------------------' | |   | | | ||
											;	|     |0 - Share bus, 1 - Hog bus -------------' |   | | | ||
											;	|     |SMUDGE mode ------------------------------'   | | | ||
											;	|     |Halftone line number -------------------------+-+-+-'|
BLIT_SKEW				equ		$FFFF8A3D	;	|byte |SKEW Register                     BIT 7 6 . . 3 2 1 0|R/W (Blit)
											;	|     |Force eXtra Source Read --------------' |     | | | ||
											;	|     |No Final Source Read -------------------'     | | | ||
											;	|     |Source skew ----------------------------------+-+-+-'|


DMA_CONTROL				equ		$FFFF8901	;	|byte |DMA Control Register              BIT 7 . 5 4 . . 1 0|R/W
											;	|     |Loop replay buffer -------------------------------' ||     (STe)
											;	|     |DMA Replay on --------------------------------------'|     (STe)
DMA_START_HI			equ		$FFFF8903	;	|byte |Frame start address (high byte)                      |R/W  (STe)
DMA_START_MID			equ		$FFFF8905	;	|byte |Frame start address (mid byte)                       |R/W  (STe)
DMA_START_LOW			equ		$FFFF8907	;	|byte |Frame start address (low byte)                       |R/W  (STe)
DMA_COUNT_HI			equ		$FFFF8909	;	|byte |Frame address counter (high byte)                    |R    (STe)
DMA_COUNT_MID			equ		$FFFF890B	;	|byte |Frame address counter (mid byte)                     |R    (STe)
DMA_COUNT_LOW			equ		$FFFF890D	;	|byte |Frame address counter (low byte)                     |R    (STe)
DMA_END_HI				equ		$FFFF890F	;	|byte |Frame end address (high byte)                        |R/W  (STe)
DMA_END_MID				equ		$FFFF8911	;	|byte |Frame end address (mid byte)                         |R/W  (STe)
DMA_END_LOW				equ		$FFFF8913	;	|byte |Frame end address (low byte)                         |R/W  (STe)
DMA_SOUND_MODE			equ		$FFFF8921	;	|byte |Sound mode control                BIT 7 6 . . . . 1 0|R/W  (STe)
											;	|     |0 - Stereo, 1 - Mono -----------------' |         | ||
											;	|     |0 - 8bit -------------------------------+         | ||
											;	|     |Frequency control bits                            | ||
											;	|     |00 - 6258hz frequency (STe only) -----------------+-+|
											;	|     |01 - 12517hz frequency ---------------------------+-+|
											;	|     |10 - 25033hz frequency ---------------------------+-+|
											;	|     |11 - 50066hz frequency ---------------------------+-'|
											;	|     |Samples are always signed. In stereo mode, data is   |
											;	|     |arranged in pairs with high pair the left channel,low|
											;	|     |pair right channel. Sample length MUST be even in    |
											;	|     |either mono or stereo mode.                          |
											;	|     |Example: 8 bit Stereo : LRLRLRLRLRLRLRLR             |
											;	|     |        16 bit Stereo : LLRRLLRRLLRRLLRR (F030)      |
											;	|     |2 track 16 bit stereo : LLRRllrrLLRRllrr (F030)      |


trap_gemdos				equ		  1
gemdos_pterm0			equ		$00
gemdos_cconin			equ		$01
gemdos_super			equ		$20
gemdos_fforce			equ		$46
gemdos_mshrink			equ		$4a

trap_bios				equ		13
bios_bconstat			equ		 1		; test if input character is ready
bios_bconin				equ		 2		; read input characher
bios_bconout			equ		 3		; write output character
bios_bcostat			equ		 8		; test if output device is ready to receive a character


trap_xbios				equ		14
xbios_ikbdws			equ		25
xbios_kbdvbase			equ		34
xbios_rsconf			equ		15

kbdvbase_kb_statvec		equ		12
kbdvbase_kb_mouevec		equ		16
kbdvbase_kb_kbdsys		equ		32


; Timers:
;	- all timers are clocked by 2457600 Hz clock
;	- timer C is system 200Hz timer		- DATA=192, div=64



		SECTION_DATA


save_stack:				dc.l	0
vblank_irq_vloc:		dc.l	0
frame_time_queue:		dc.l	0, 0, 0, 0, 0, 0, 0, 0
frame_time_queue_pos:	dc.w	0
frame_time_last:		dc.l	0


		SECTION_CODE


; ================================================================ _start ================================================================
	public	_start
_start:
		move.l	4(sp),a5				;address to basepage
		move.l	$0c(a5),d0				;length of text segment
		add.l	$14(a5),d0				;length of data segment
		add.l	$1c(a5),d0				;length of bss segment
		add.l	#STACK_SIZE+BASEPAGE_SIZE,d0		;length of stackpointer+basepage
		move.l	a5,d1					;address to basepage
		add.l	d0,d1					;end of program
		and.l	#$fffffff0,d1				;align stack
		move.l	d1,sp					;new stackspace

		move.l	d0,-(sp)				; mshrink()
		move.l	a5,-(sp)				;
		clr.w	-(sp)					;
		move.w	#gemdos_mshrink, -(sp)	;
		trap	#trap_gemdos			;
		lea.l	12(sp),sp				;
				
;########################## redirect output to serial		
	if (REDIRECT_OUTPUT_TO_SERIAL==1)  
		; redirect to serial
		move.w #2,-(sp)
		move.w #1,-(sp)
		move.w #gemdos_fforce,-(sp)
		trap #trap_gemdos
		addq.l #6,sp
	endif


		jsr	_main


exit:	
;		move.w #gemdos_cconin,-(sp)
;		trap #trap_gemdos
;		addq.l #2,sp
		
		move.w #gemdos_pterm0,-(sp)
		trap #trap_gemdos
		
		
_basepage:	ds.l	1
_len:		ds.l	1





; ================================================================ _set_supervisor ================================================================
    public	_set_supervisor
_set_supervisor:
	clr.l	-(sp)
	move.w	#gemdos_super,-(sp)
    trap	#trap_gemdos
	addq	#6,sp
	move.l	d0, save_stack

	;move.b	#$00, $FFFF8001
	;move.l	sp, d0
	;and     #$0fff, SR
	;move.l	d0, sp
    rts



; ================================================================ _set_usermode ================================================================
	public	_set_usermode
_set_usermode:
	move.l	save_stack,-(sp)
	move.w	#gemdos_super,-(sp)
	trap	#trap_gemdos
	addq.l	#6,sp
    rts

    xdef	_reset
_reset:
	reset
    rts






; ================================================================ _st_install_vblank ================================================================

	public		_st_install_vblank
_st_install_vblank:
	move.l		#int_timerb, $120				;	Timer B interrupt vector
	bset		#0, INT_ENABLE_A
	bset		#0, INT_MASK_A

	move.l		stvar__vblqueue, a0
	move.w		stvar_nvbls, d0
	subq.l		#1, d0
.loop:
	move.l		(a0)+, d1
	cmp.l		#0, d1
	beq			.found
	dbra.w		d0, .loop
	rts

.found:
	move.l		#_int_vblank, -(a0)
	move.l		a0, vblank_irq_vloc
	rts



; ================================================================ _st_remove_vblank ================================================================
	public		_st_remove_vblank
_st_remove_vblank:
	move.l		vblank_irq_vloc, d0
	beq			.end
	move.l		d0, a0
	move.l		#0, (a0)
.end:
	rts
	


; ================================================================ _int_vblank ================================================================
_int_vblank:
	add.w		#1, _intnum						;	intnum++;
	add.w		#1, _rawintnum					;	rawintnum++;
	;move.w		#$000, COLOR00

	;move.l		_vblank_screen, d0				;	VID_SCREEN_MED = (byte)(((int)vblank_screen)>>8);
	;lsr.l		#8, d0
	;move.b		d0, VID_SCREEN_MED
	;lsr.l		#8, d0							;	VID_SCREEN_HIGH = (byte)(((int)vblank_screen)>>16);
	;move.b		d0, VID_SCREEN_HIGH
	
	move.l		_vblank_pal, a0
	movem.l		(a0), d0-d7
	movem.l		d0-d7, COLOR00

	move.b		#0, TIMER_B_CONTROL
	move.b		#167, TIMER_B_DATA		; #168
	move.b		#8, TIMER_B_CONTROL

	; play DMA sound
	tst.w		_vblank_playwait
	beq			.testplay
	sub.w		#1, _vblank_playwait
	bra			.nosound

.testplay:
	move.l		_vblank_playsound, d0
	beq			.nosound
	
	move.l		_vblank_playsound+4, _vblank_playsound
	move.l		_vblank_playsound+8, _vblank_playsound+4
	clr.l		_vblank_playsound+8

	move.l		d0, a0
	moveq.l		#0, d0
	move.w		(a0)+, d0
	lsl.l		#1, d0
	
	move.b		#0, DMA_CONTROL			; stop DMA

	move.l		a0, .temp
	move.b		.temp+1, DMA_START_HI
	move.b		.temp+2, DMA_START_MID
	move.b		.temp+3, DMA_START_LOW

	add.l		d0, a0
	move.l		a0, .temp
	move.b		.temp+1, DMA_END_HI
	move.b		.temp+2, DMA_END_MID
	move.b		.temp+3, DMA_END_LOW

	move.b		#$80, DMA_SOUND_MODE	; mono + 6k
	move.b		#1, DMA_CONTROL			; start DMA

	divu.w		#125*3, d0				; 6258 / 50 ~= 125		- dont let delay be longer than 1/3 of the sample
	cmp.w		#12, d0					; d0 ? 12
	bls			.noclamp
	move.w		#12, d0
.noclamp:
	move.w		d0, _vblank_playwait


.nosound:

	if ST_PLAY_MUSIC
	move.l		_music_active, d0
	beq			.nomusic
	move.l		d0, a0
	adda.w		#8, a0
	jsr			(a0)					; play music
.nomusic:
	endif

	rts

.temp:	ds.l	1			; .temp+0 | .temp+1 | .temp+2 | .temp+3



; ================================================================ int_timerb ================================================================
int_timerb:
	;move.w		#$010, COLOR00
	movem.l		d0-d7/a0, -(a7)
	move.l		_vblank_hudpal, d0
	beq			.nopal
	move.l		d0, a0
	movem.l		(a0), d0-d7
	movem.l		d0-d7, COLOR00
.nopal:
	movem.l		(a7)+, d0-d7/a0
	bclr		#0, INT_INSERVICE_A
	rte



; ================================================================ ST_GetPerfTimer ================================================================
;
; call in SUPERVISOR MODE !!!
;
	public		_ST_GetPerfTimer
_ST_GetPerfTimer:
	movem.l		d2,-(a7)

	moveq.l		#0, d1
	move.l		stvar__hz_200, d0			; d0 = stvar__hz_200
	move.b		TIMER_C_DATA, d1			; d1 = TIMER_C_DATA
	move.l		d0, d2						; d2 = d0

	mulu.w		#192, d0					; d0 = loword(stvar__hz_200) * 192

	swap.w		d2
	mulu.w		#192, d2					; d2 = (hiword(stvar__hz_200) * 192) << 16
	swap.w		d2							;
	eor.w		d2, d2						;

	add.l		d2, d0
	sub.l		d1, d0

	cmp.l		frame_time_last, d0			; d0 ? frame_time_last
	bpl			.result_ok
	move.l		frame_time_last, d0			; fix up
.result_ok:
	move.l		d0, frame_time_last

	; dword perf = hz*192 - t1;
	; if( last_perf && (int)(perf-last_perf)<0 )
	; 	perf = last_perf;
	; last_perf = perf;

	movem.l		(a7)+,d2
	rts


; ================================================================ ST_CriticalBegin ================================================================
;
; call in SUPERVISOR MODE !!!
;
	public		_ST_CriticalBegin
_ST_CriticalBegin:
	move.w		sr, d0				; save old SR
	move.w		d0, d1				; disable interrupts
	or.w		#$0700, d1
	move.w		d1, sr
	rts


; ================================================================ ST_CriticalEnd ================================================================
;
; call in SUPERVISOR MODE !!!
;
	public		_ST_CriticalEnd
_ST_CriticalEnd:
	move.w		d0, sr				; restore old SR
	rts
