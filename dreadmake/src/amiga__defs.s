
; All registers relative to $dff000

CHIP_BASE	equ	$dff000


DMACONR:	equ $002				;	word      *R   AP      DMA control (and blitter status) read

VHPOSR		equ	$006				;	word	  *R   A       Read vert and horiz. position of beam


JOY0DAT		equ $00A				;	word	  *R   D       Joystick-mouse 0 data (vert,horiz)

SERDATR		equ $018				;	word	  *R   P       Serial port data and status read
INTENAR		equ $01C				;	word	   R   P       Interrupt enable bits read
INTREQR		equ $01E				;	word	   R   P       Interrupt request bits read

DSKPT		equ $020				;	ptr_t	+ *W   A       Disk pointer (low 15 bits)
DSKPTH		equ $020				;			+ *W   A( E )  Disk pointer (high 3 bits, 5 bits if ECS)
DSKPTL		equ $022				;			+ *W   A       Disk pointer (low 15 bits)
DSKLEN		equ $024				;			  *W   P       Disk length
DSKDAT		equ $026				;			& *W   P       Disk DMA data write
REFPTR		equ $028				;			& *W   A       Refresh pointer
VPOSW		equ $02A				;			  *W   A       Write vert most signif. bit (and frame flop)
VHPOSW		equ $02C				;			  *W   A       Write vert and horiz position of beam
COPCON		equ $02E				;			  *W   A( E )  Coprocessor control register (CDANG)
SERDAT		equ $030				;			  *W   P       Serial port data and stop bits write
SERPER		equ $032				;			  *W   P       Serial port period and control
POTGO		equ $034				;			  *W   P       Pot port data write and start
JOYTEST		equ $036				;			  *W   D       Write to all four joystick-mouse counters at once
STREQU		equ $038				;			& *S   D       Strobe for horiz sync with VB and EQU
STRVBL		equ $03A				;			& *S   D       Strobe for horiz sync with VB (vert. blank)
STRHOR		equ $03C				;			& *S   DP      Strobe for horiz sync
STRLONG		equ $03E				;			& *S   D( E )  Strobe for identification of long horiz. line.

BLTCON0:	equ $040				;	word	  ~W   A       Blitter control register 0
BLTCON1:	equ $042				;	word	  ~W   A( E )  Blitter control register 1
BLTAWM:		equ $044				;	dword	  ~W   A       Blitter first word mask for source A
BLTAFWM:	equ $044				;	word	  ~W   A       Blitter first word mask for source A
BLTALWM:	equ $046				;	word	  ~W   A       Blitter last word mask for source A
BLTCPT:		equ $048				;	ptr_t	+ ~W   A       Blitter pointer to source C (high 3 bits)
BLTCPTH:	equ $048				;	word	+ ~W   A       Blitter pointer to source C (high 3 bits)
BLTCPTL:	equ $04A				;	word	+ ~W   A       Blitter pointer to source C (low 15 bits)
BLTBPT:		equ $04C				;	ptr_t	+ ~W   A       Blitter pointer to source B (high 3 bits)
BLTBPTH:	equ $04C				;	word	+ ~W   A       Blitter pointer to source B (high 3 bits)
BLTBPTL:	equ $04E				;	word	+ ~W   A       Blitter pointer to source B (low 15 bits)
BLTAPT:		equ $050				;	ptr_t	+ ~W   A( E )  Blitter pointer to source A (high 3 bits)
BLTAPTH:	equ $050				;	word	+ ~W   A( E )  Blitter pointer to source A (high 3 bits)
BLTAPTL:	equ $052				;	word	+ ~W   A       Blitter pointer to source A (low 15 bits)
BLTDPT:		equ $054				;	ptr_t	+ ~W   A       Blitter pointer to destination D (high 3 bits)
BLTDPTH:	equ $054				;	word	+ ~W   A       Blitter pointer to destination D (high 3 bits)
BLTDPTL:	equ $056				;	word	+ ~W   A       Blitter pointer to destination D (low 15 bits)
BLTSIZE:	equ $058				;	word	  ~W   A       Blitter start and size (window width,height)
BLTCON0L:	equ $05A				;	word	  ~W   A( E )  Blitter control 0, lower 8 bits (minterms)
BLTSIZV:	equ $05C				;	word	  ~W   A( E )  Blitter V size (for 15 bit vertical size)
BLTSIZH:	equ $05E				;	word	  ~W   A( E )  Blitter H size and start (for 11 bit H size)
BLTCMOD:	equ $060				;	word	  ~W   A       Blitter modulo for source C
BLTBMOD:	equ $062				;	word	  ~W   A       Blitter modulo for source B
BLTAMOD:	equ $064				;	word	  ~W   A       Blitter modulo for source A
BLTDMOD:	equ $066				;	word	  ~W   A       Blitter modulo for destination D
BLTCDAT:	equ $070				;	word	% ~W   A       Blitter source C data register
BLTBDAT:	equ $072				;	word	% ~W   A       Blitter source B data register
BLTADAT:	equ $074				;	word	% ~W   A       Blitter source A data register

SPRHDAT		equ	$078				;			  ~W   A( E )  Ext. logic UHRES sprite pointer and data id
DENISEID	equ	$07C				;			  ~R   D( E )  Chip revision level for Denise (video out chip)
DSKSYNC		equ	$07E				;			  ~W   P       Disk sync pattern register for disk read

COP1LC		equ $080				;			+  W   A( E )  Coprocessor first location register (high 3 bits, high 5 bits if ECS)
COP1LCH		equ $080				;			+  W   A( E )  Coprocessor first location register (high 3 bits, high 5 bits if ECS)
COP1LCL		equ $082				;			+  W   A       Coprocessor first location register (low 15 bits)
COP2LC		equ $084				;			+  W   A( E )  Coprocessor second location register (high 3 bits, high 5 bits if ECS)
COP2LCH		equ $084				;			+  W   A( E )  Coprocessor second location register (high 3 bits, high 5 bits if ECS)
COP2LCL		equ $086				;			+  W   A       Coprocessor second location register
COPJMP1		equ $088				;			   S   A       Coprocessor restart at first location
COPJMP2		equ $08A				;			   S   A       Coprocessor restart at second location


DMACON		equ	$096				;	word	   W   ADP     DMA control write (clear or set)
INTENA		equ	$09A				;	word	   W   P       Interrupt enable bits (clear or set bits)
INTREQ		equ	$09C				;	word	   W   P       Interrupt request bits (clear or set bits)
ADKCON		equ	$09E				;			   W   P       Audio, disk, UART control

AUD0LC:		equ $0A0				;	ptr_t	+  W   A( E )  Audio channel 0 location (high 3 bits, 5 if ECS)
AUD0LCH:	equ $0A0				;	word	+  W   A( E )  Audio channel 0 location (high 3 bits, 5 if ECS)
AUD0LCL:	equ $0A2				;	word	+  W   A       Audio channel 0 location (low 15 bits)
AUD0LEN:	equ $0A4				;	word	   W   P       Audio channel 0 length
AUD0PER:	equ $0A6				;	word	   W   P( E )  Audio channel 0 period
AUD0VOL:	equ $0A8				;	word	   W   P       Audio channel 0 volume
AUD0DAT:	equ $0AA				;	word	&  W   P       Audio channel 0 data

; AUD1LC    (*(volatile ptr_t*)0xdff0B0)			//+  W   A       Audio channel 1 location (high 3 bits)
; AUD1LCH   (*(volatile word*)0xdff0B0)			//+  W   A       Audio channel 1 location (high 3 bits)
; AUD1LCL   (*(volatile word*)0xdff0B2)			//+  W   A       Audio channel 1 location (low 15 bits)
; AUD1LEN   (*(volatile word*)0xdff0B4)			//   W   P       Audio channel 1 length
; AUD1PER   (*(volatile word*)0xdff0B6)			//   W   P       Audio channel 1 period
; AUD1VOL   (*(volatile word*)0xdff0B8)			//   W   P       Audio channel 1 volume
; AUD1DAT   (*(volatile word*)0xdff0BA)			//&  W   P       Audio channel 1 data
; 
; AUD2LC    (*(volatile ptr_t*)0xdff0C0)			//+  W   A       Audio channel 2 location (high 3 bits)
; AUD2LCH   (*(volatile word*)0xdff0C0)			//+  W   A       Audio channel 2 location (high 3 bits)
; AUD2LCL   (*(volatile word*)0xdff0C2)			//+  W   A       Audio channel 2 location (low 15 bits)
; AUD2LEN   (*(volatile word*)0xdff0C4)			//   W   P       Audio channel 2 length
; AUD2PER   (*(volatile word*)0xdff0C6)			//   W   P       Audio channel 2 period
; AUD2VOL   (*(volatile word*)0xdff0C8)			//   W   P       Audio channel 2 volume
; AUD2DAT   (*(volatile word*)0xdff0CA)			//&  W   P       Audio channel 2 data
; 
; AUD3LC    (*(volatile ptr_t*)0xdff0D0)			//+  W   A       Audio channel 3 location (high 3 bits)
; AUD3LCH   (*(volatile word*)0xdff0D0)			//+  W   A       Audio channel 3 location (high 3 bits)
; AUD3LCL   (*(volatile word*)0xdff0D2)			//+  W   A       Audio channel 3 location (low 15 bits)
; AUD3LEN   (*(volatile word*)0xdff0D4)			//   W   P       Audio channel 3 length
; AUD3PER   (*(volatile word*)0xdff0D6)			//   W   P       Audio channel 3 period
; AUD3VOL   (*(volatile word*)0xdff0D8)			//   W   P       Audio channel 3 volume
; AUD3DAT   (*(volatile word*)0xdff0DA)			//&  W   P       Audio channel 3 data

DIWSTRT		equ $08E				;			//   W   A       Display window start (upper left vert-horiz position)
DIWSTOP		equ $090				;			//   W   A       Display window stop (lower right vert.-horiz. position)
DDFSTRT		equ $092				;			//   W   A       Display bitplane data fetch start (horiz. position)
DDFSTOP		equ $094				;			//   W   A       Display bitplane data fetch stop (horiz. position)

BPLCON0		equ $100				;			   W   AD( E ) Bitplane control register (misc. control bits)
BPLCON1		equ $102				;			   W   D       Bitplane control reg. (scroll value PF1, PF2)
BPLCON2		equ $104				;			   W   D( E )  Bitplane control reg. (priority control)
BPLCON3		equ $106				;			   W   D( E )  Bitplane control (enhanced features)
BPL1MOD		equ $108				;			   W   A       Bitplane modulo (odd planes)
BPL2MOD		equ $10A				;			   W   A       Bitplane modulo (even planes)


COLOR00:	equ	$180				;	word	   W   D       Color table 00
COLOR01:	equ	$182				;	word	   W   D       Color table 01
COLOR02:	equ	$184				;	word	   W   D       Color table 02
COLOR03:	equ	$186				;	word	   W   D       Color table 03
COLOR04:	equ	$188				;	word	   W   D       Color table 04
COLOR05:	equ	$18A				;	word	   W   D       Color table 05
COLOR06:	equ	$18C				;	word	   W   D       Color table 06
COLOR07:	equ	$18E				;	word	   W   D       Color table 07
COLOR08:	equ	$190				;	word	   W   D       Color table 08
COLOR09:	equ	$192				;	word	   W   D       Color table 09
COLOR10:	equ	$194				;	word	   W   D       Color table 10
COLOR11:	equ	$196				;	word	   W   D       Color table 11
COLOR12:	equ	$198				;	word	   W   D       Color table 12
COLOR13:	equ	$19A				;	word	   W   D       Color table 13
COLOR14:	equ	$19C				;	word	   W   D       Color table 14
COLOR15:	equ	$19E				;	word	   W   D       Color table 15
COLOR16:	equ	$1A0				;	word	   W   D       Color table 16
COLOR17:	equ	$1A2				;	word	   W   D       Color table 17
COLOR18:	equ	$1A4				;	word	   W   D       Color table 18
COLOR19:	equ	$1A6				;	word	   W   D       Color table 19
COLOR20:	equ	$1A8				;	word	   W   D       Color table 20
COLOR21:	equ	$1AA				;	word	   W   D       Color table 21
COLOR22:	equ	$1AC				;	word	   W   D       Color table 22
COLOR23:	equ	$1AE				;	word	   W   D       Color table 23
COLOR24:	equ	$1B0				;	word	   W   D       Color table 24
COLOR25:	equ	$1B2				;	word	   W   D       Color table 25
COLOR26:	equ	$1B4				;	word	   W   D       Color table 26
COLOR27:	equ	$1B6				;	word	   W   D       Color table 27
COLOR28:	equ	$1B8				;	word	   W   D       Color table 28
COLOR29:	equ	$1BA				;	word	   W   D       Color table 29
COLOR30:	equ	$1BC				;	word	   W   D       Color table 30
COLOR31:	equ	$1BE				;	word	   W   D       Color table 31

BEAMCON0	equ	$1DC				;			//   W   A( E )  Beam counter control register (SHRES,PAL)



; All registers ABSOLUTE

CIAA_PRA     equ	$BFE001			;	byte	   /FIR1 /FIR0  /RDY /TK0  /WPRO /CHNG /LED  OVL
CIAA_PRB     equ	$BFE101			;	byte	   Parallel port
CIAA_DDRA    equ	$BFE201			;	byte	   Direction for port A(BFE001); 1=output(set to 0x03)
CIAA_DDRB    equ	$BFE301			;	byte	   Direction for port B(BFE101); 1=output(can be in or out)
CIAA_TALO    equ	$BFE401			;	byte	   CIAA timer A low byte(.715909 Mhz NTSC; .709379 Mhz PAL)
CIAA_TAHI    equ	$BFE501			;	byte	   CIAA timer A high byte
CIAA_TBLO    equ	$BFE601			;	byte	   CIAA timer B low byte(.715909 Mhz NTSC; .709379 Mhz PAL)
CIAA_TBHI    equ	$BFE701			;	byte	   CIAA timer B high byte
CIAA_TODLO   equ	$BFE801			;	byte	   50/60 Hz event counter bits 7-0 (VSync or line tick)
CIAA_TODMID  equ	$BFE901			;	byte	   50/60 Hz event counter bits 15-8
CIAA_TODHI   equ	$BFEA01			;	byte	   50/60 Hz event counter bits 23-16
;					$BFEB01			;			   not used
CIAA_SDR     equ	$BFEC01			;	byte	   CIAA serial data register (connected to keyboard)
CIAA_ICR     equ	$BFED01			;	byte	   CIAA interrupt control register
CIAA_CRA     equ	$BFEE01			;	byte	   CIAA control register A
CIAA_CRB     equ	$BFEF01			;	byte	   CIAA control register B


CIAB_PRA:		equ	$BFD000			;	byte	   /DTR  /RTS  /CD   /CTS  /DSR   SEL   POUT  BUSY
CIAB_PRB:		equ	$BFD100			;	byte	   /MTR  /SEL3 /SEL2 /SEL1 /SEL0 /SIDE  DIR  /STEP
CIAB_DDRA:		equ	$BFD200			;	byte	   Direction for Port A(BFD000); 1 = output(set to 0xFF)
CIAB_DDRB:		equ	$BFD300			;	byte	   Direction for Port B(BFD100); 1 = output(set to 0xFF)
CIAB_TALO:		equ	$BFD400			;	byte	   CIAB timer A low byte(.715909 Mhz NTSC; .709379 Mhz PAL)
CIAB_TAHI:		equ	$BFD500			;	byte	   CIAB timer A high byte
CIAB_TBLO:		equ	$BFD600			;	byte	   CIAB timer B low byte(.715909 Mhz NTSC; .709379 Mhz PAL)
CIAB_TBHI:		equ	$BFD700			;	byte	   CIAB timer B high byte
CIAB_TODLO:		equ	$BFD800			;	byte	   Horizontal sync event counter bits 7-0
CIAB_TODMID:	equ	$BFD900			;	byte	   Horizontal sync event counter bits 15-8
CIAB_TODHI:		equ	$BFDA00			;	byte	   Horizontal sync event counter bits 23-16
;					$BFDB00			;			   not used
CIAB_SDR:		equ	$BFDC00			;	byte	   CIAB serial data register (unused)
CIAB_ICR:		equ	$BFDD00			;	byte	   CIAB interrupt control register
CIAB_CRA:		equ	$BFDE00			;	byte	   CIAB Control register A
CIAB_CRB:		equ	$BFDF00			;	byte	   CIAB Control register B




CIAICRF_TA			equ	$01
CIAICRF_TB			equ	$02
CIAICRF_SETCLR		equ	$80

CIACRAF_START		equ	$01
CIACRAF_LOAD		equ	$10

CIACRBF_START		equ	$01
CIACRBF_ONESHOT		equ	$08
CIACRBF_LOAD		equ	$10




; Blitter register load order:
;	BLTxDAT
;	BLTCONx
;	BLTAxWM
;	BLTxMOD
;	BLTxPT
;	BLTSIZE
