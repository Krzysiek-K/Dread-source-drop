

	include	"amiga__defs.s"
	include	"amiga__sys_macros.s"



;					struct Node {
;		  0			    Node*	ln_Succ;		/* Pointer to next (successor) */
;		  4			    Node*	ln_Pred;		/* Pointer to previous (predecessor) */
;		  8			    UBYTE   ln_Type;
;		  9			    BYTE    ln_Pri;			/* Priority, for sorting */
;		 10			    char*	ln_Name;		/* ID string, null terminated */
;		 14			};	/* Note: word aligned */
;					
;					struct Message {
;		  0			    Node		mn_Node;
;		 14			    MsgPort*	mn_ReplyPort;	/* message reply port */
;		 18			    UWORD		mn_Length;		/* total message length, in bytes (include the size of the Message structure in the length) */
;		 20			};
;					
;					struct IOStdReq {
;		  0			    Message		io_Message;
;		 20			    Device*		io_Device;			/* device node pointer  */
;		 24			    Unit*		io_Unit;			/* unit (driver private)*/
;		 28			    UWORD		io_Command;			/* device command */
;		 30			    UBYTE		io_Flags;
;		 31			    BYTE		io_Error;		    /* error or warning num */
;		 32			    ULONG		io_Actual;		    /* actual number of bytes transferred */
;		 36			    ULONG		io_Length;		    /* requested number bytes transferred*/
;		 40			    APTR		io_Data;		    /* points to data area */
;		 44			    ULONG		io_Offset;		    /* offset for block structured devices */
;		 48			};

IO_LENGTH		equ     36
IO_DATA			equ     40
IO_OFFSET		equ     44


; ================================================================================================ Bootblock ================================================================================================

        org     0                       ; dont care, all code is PC-relative


        ; AmigaDOS boot blocks (1024 bytes)
boot:
        dc.b    "DOS",0
        dc.l    0                       ; checksum will be inserted here
        dc.l    880                     ; root block does not matter

;---------------------------------------------------------------------------
boot_code:
; a6 = SysBase
; a1 = trackdisk IoStdReq


bootstart:
		move.w		#0, $dff180							; Black screen

        move.l		#ProgEnd - ProgStart, d0			;	d0 = allocation size
		move.l		#$00002, d1							;	d1 = memory flags			MEMF_CLEAR=$10000, MEMF_CHIP=$00002, MEMF_FAST=$00004

		movem.l		a1/a6, -(a7)						; save a1/a6
		move.l		$4.w, a6
		jsr			sys_exec_AllocMem(a6)
		movem.l		(a7)+, a1/a6						; restore a6
														;	result		d0 = reserved memory address (or NULL)	aligned to 4 bytes

		move.l		d0, a5

        move.l		#ProgEnd-ProgStart, IO_LENGTH(a1)
        move.l		a5, IO_DATA(a1)
        move.l		#ProgStart-boot, IO_OFFSET(a1)
        jsr			sys_exec_DoIO(a6)					; load it

        cmp.w		#36, 14+6(a6)						; disable caches if running on kickstart 2.0 or above		(LIB_VERSION)
        blt			.nocache
        moveq.l		#0, d0
        moveq.l		#-1, d1
        jsr			sys_exec_CacheControl(a6)
.nocache:


        jmp			(a5)								; start

        ; fill the rest of the boot blocks with zero
        cnop    0,1024



; ================================================================================================ Loader ================================================================================================

;---------------------------------------------------------------------------
        ; Block 2: the program to load

ProgStart:
		bra			Stage2_Start

Memdef_Region:		dc.b		"MEMDEF--"			;  +0
					dc.l		4096				;  +8	Supervisor stack size
					dc.l		4096				; +12	User stack size
													; +16



		; ================================================================ Extra files ================================================================

	include	"amiga_boot_print.s"
	include	"amiga_boot_copper.s"
	include	"amiga_trackloader.s"
	include	"amiga_boot_memory.s"
	include	"amiga_boot_script.s"


Stage2_Start:


		; ================================================================ Allocate all available memory ================================================================


		bsr			Mem_Detect


		; ================================================================ Kill the system ================================================================
        move.w		#$7fff, INTENA+CHIP_BASE		; disable all interrupts
        move.w		#$7fff, DMACON+CHIP_BASE		; disable all DMA channels
        jsr			sys_exec_SuperState(a6)			; enter supervisor mode
        or.w		#$0700,sr						; set highest priority level (disable interrupts)



		; ================================================================ Estimate real memory ranges ================================================================


		Mem_Check							; macro, because it will most likely destroy the stack in the process


		bsr			Mem_Postprocess


		Mem_AllocateCoreRegions				; initial memory layout





		; ================================================================ Show loading image ================================================================

		lea.l		LocaImg_copper(pc), a0
		lea.l		LoadImg_data(pc), a1
		move.l		a0, COP1LC+CHIP_BASE
		move.l		a1, d0
		
											
		move.w		d0, 2(a0)				; Bitplane 1
		swap.w		d0
		move.w		d0, 6(a0)
		swap.w		d0
		add.l		#40, d0

		move.w		d0, 10(a0)				; Bitplane 2
		swap.w		d0
		move.w		d0, 14(a0)
		swap.w		d0
		add.l		#40, d0

		move.w		d0, 18(a0)				; Bitplane 3
		swap.w		d0
		move.w		d0, 22(a0)
		swap.w		d0
		add.l		#40, d0

		move.w		d0, 26(a0)				; Bitplane 4
		swap.w		d0
		move.w		d0, 30(a0)

		move.w		LoadImg_pal(pc), 34(a0)
		move.w		LoadImg_pal+2(pc), 38(a0)

		move.l		LoadImg_pal(pc), $dff180
		move.l		LoadImg_pal+4(pc), $dff180+4
		move.l		LoadImg_pal+8(pc), $dff180+8
		move.l		LoadImg_pal+12(pc), $dff180+12
		move.l		LoadImg_pal+16(pc), $dff180+16
		move.l		LoadImg_pal+20(pc), $dff180+20
		move.l		LoadImg_pal+24(pc), $dff180+24
		move.l		LoadImg_pal+28(pc), $dff180+28


		bsr			Print_Init

		move.w		#$0200, BPLCON0+CHIP_BASE
		move.w		#0, BPLCON1+CHIP_BASE
		move.w		#$0024, BPLCON2+CHIP_BASE
		move.w		#$0C20, BPLCON3+CHIP_BASE
		move.w		#40*3, BPL1MOD+CHIP_BASE
		move.w		#40*3, BPL2MOD+CHIP_BASE

        move.w		#$8380, DMACON+CHIP_BASE		; enable bitplanes + copper



		bsr			Mem_Print


		; ================================================================ Install trackloader ================================================================


		bsr			Track_Init


		sub.l		a0, a0												; Init interrupt
		lea.l		Irq_CIAA(pc) ,a1
		move.l		a1, IV_LEVEL_2(a0)

		move.b		#0, CIAA_CRA										; Stop the timer
		move.b		#$7F, CIAA_ICR										; Disable interrupts
		move.b		#0, CIAA_CRB
		tst.b		CIAA_ICR

		move.b		#$6C,	CIAA_TBLO
		move.b		#$37,	CIAA_TBHI
		move.b		#CIAICRF_SETCLR|CIAICRF_TB, CIAA_ICR
		move.b		#CIACRBF_START|CIACRBF_LOAD, CIAA_CRB

		move.w		#$C008, CHIP_BASE+INTENA							; Enable CIAA interrupt		TBLOAD = 14188 = $376C	(~50Hz)


        and.w		#$F8FF,sr					; set lowest priority level


		bsr			Boot_Script_Execute
.done:
		bra			.done


		moveq.l		#1, d6
.loop:

		lea.l		scan_timer(pc), a0
		move.w		#25, (a0)
.wait:
		tst.w		(a0)
		bne			.wait

		bsr			Print_Newline
		add.l		#1, d6
		move.l		d6, d0
		bsr			Print_Hex_4
		moveq.l		#' ', d0
		bsr			Print_Char

		move.w		#65, d2
		lea.l		.buffer(pc), a2
		bsr			Track_Load_Sector
		move.l		d0, d7
		

		moveq.l		#' ', d0
		bsr			Print_Char
		move.l		d7, d0
		bsr			Print_Hex_2

		moveq.l		#' ', d0
		bsr			Print_Char
		cmp.w		#1, d7
		bne			.not_ready
		lea.l		.buffer(pc), a0
		bsr			Print_String
.not_ready:

		bra			.loop


.buffer:
		dcb.b		512




		; ================================================================ Interrupts ================================================================

;******************** Irq_CIAA ********************
Irq_CIAA:
		movem.l		d0-d1/a0-a1, -(a7)
		lea.l		CHIP_BASE, a1

		tst.b		CIAA_ICR
		move.w		#$0008,INTREQ(a1)
		move.w		#$0008,INTREQ(a1)

;		lea.l		.counter(pc), a0						; Flash color #0
;		add.w		#$800, (a0)
;		move.w		(a0), $dff180

		lea.l		scan_timer(pc), a0						; Decrement scan_timer to 0
		move.w		(a0), d0
		beq			.timer_done
		subq.l		#1, d0
		move.w		d0, (a0)
.timer_done:


		bsr			Track_ISR_Poll

		movem.l		(a7)+, d0-d1/a0-a1
		rte

;.counter:
;		dc.w		$000


scan_timer		dc.w		0








		; ================================================================ Image definition ================================================================
LoadImg_pal:
		incbin		"data/loader/out/loading.PAL"

LoadImg_data:
		incbin		"data/loader/out/loading.BPL"



		; ================================================================ END ================================================================


        dc.b		">>>TRACKLOADER END<<<"
		even

        cnop		0,512
ProgEnd:
        org			901120

        end
