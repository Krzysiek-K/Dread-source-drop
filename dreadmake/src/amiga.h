
#ifndef _AMIGA_H
#define _AMIGA_H


#define COMPUTE_BLTSIZE(width, height)				( (word)((((height) & 0x3FF)<<6) | ((width) & 0x3F)) )





// shut up, Visual...
#ifdef WIN32
  #define __reg(x)
  #define __chip
  #define USE_EMBEDDED_MAP_DATA			1
  #define ENABLE_MAP_FILE_LOADING		1
  #define ENABLE_MAP_AUTOSTART			0
#endif

#define __d0	__reg("d0")
#define __d1	__reg("d1")
#define __d2	__reg("d2")
#define __d3	__reg("d3")
#define __d4	__reg("d4")
#define __d5	__reg("d5")
#define __d6	__reg("d6")
#define __d7	__reg("d7")
#define __a0	__reg("a0")
#define __a1	__reg("a1")
#define __a2	__reg("a2")
#define __a3	__reg("a3")
#define __a4	__reg("a4")
#define __a5	__reg("a5")
#define __a6	__reg("a6")




//
//  &        Register used by DMA channel only.
//  %        Register used by DMA channel usually, processors sometimes.
//
//  +        Address register pair.  Must be an even address pointing to chip memory.
//
//  *        Address not writable by the Copper.
//  ~        Address not writable by the Copper unless the "copper danger bit",  COPCON  is set true.
//
//  A,D,P    A=Agnus chip, D=Denise chip, P=Paula chip.
//
//  W,R      W=write-only// R=read-only,
//
//  ER       Early read. This is a DMA data transfer to RAM, from either the
//           disk or the blitter.  RAM timing requires data to be on the bus
//           earlier than microprocessor read cycles. These transfers are
//           therefore initiated by Agnus timing, rather than a read address
//           on the destination address bus.
//
//  S        Strobe (write address with no register bits).  Writing the
//           register causes the effect.
//
//  PTL,PTH  Chip memory pointer that addresses DMA data.  Must be reloaded
//           by a processor before use (vertical blank for bitplane and
//           sprite pointers, and prior to starting the blitter for blitter
//           pointers).
//
//  LCL,LCH  Chip memory location (starting address) of DMA data.  Used to
//           automatically restart pointers, such as the Copper program
//           counter (during vertical blank) and the audio sample counter
//           (whenever the audio length count is finished).
//
//  MOD      15-bit modulo. A number that is automatically added to the
//           memory address at the end of each line to generate the address
//           for the beginning of the next line. This allows the blitter (or
//           the display window) to operate on (or display) a window of data
//           that is smaller than the actual picture in memory (memory map).
//           Uses 15 bits, plus sign extend.
//
//   About the ECS registers.
//   ------------------------
//   Registers denoted with an "(E)" in the chip column means that
//   those registers have been changed in the Enhanced Chip Set
//   (ECS).  The ECS is found in the A3000, and is installable in the
//   A500 and A2000.  Certain ECS registers are completely new,
//   others have been extended in their functionality. See the
//    register map  in Appendix C for information on which ECS
//   registers are new and which have been modified.
//


typedef struct {
	volatile word	POS;
	volatile word	CTL;
	volatile word	DATA;
	volatile word	DATB;
} sprite_t;


#define BLTDDAT   (*(volatile const word*)0xdff000)		//& *ER  A       Blitter destination early read (dummy address)
#define DMACONR   (*(volatile const word*)0xdff002)		//  *R   AP      DMA control (and blitter status) read
#define VPOSR     (*(volatile const word*)0xdff004)		//  *R   A( E )  Read vert most signif. bit (and frame flop)
#define VHPOSR    (*(volatile const word*)0xdff006)		//  *R   A       Read vert and horiz. position of beam
#define VHPOSR_B  (*(volatile const byte*)0xdff006)
#define DSKDATR   (*(volatile const word*)0xdff008)		//& *ER  P       Disk data early read (dummy address)
#define JOY0DAT   (*(volatile const word*)0xdff00A)		//  *R   D       Joystick-mouse 0 data (vert,horiz)
#define JOY1DAT   (*(volatile const word*)0xdff00C)		//  *R   D       Joystick-mouse 1 data (vert,horiz)
#define CLXDAT    (*(volatile const word*)0xdff00E)		//  *R   D       Collision data register (read and clear)
#define ADKCONR   (*(volatile const word*)0xdff010)		//  *R   P       Audio, disk control register read
#define POT0DAT   (*(volatile const word*)0xdff012)		//  *R   P( E )  Pot counter pair 0 data (vert,horiz)
#define POT1DAT   (*(volatile const word*)0xdff014)		//  *R   P( E )  Pot counter pair 1 data (vert,horiz)
#define POTGOR    (*(volatile const word*)0xdff016)		//  *R   P       Pot port data read (formerly POTINP)
#define SERDATR   (*(volatile const word*)0xdff018)		//  *R   P       Serial port data and status read
#define DSKBYTR   (*(volatile const word*)0xdff01A)		//  *R   P       Disk data byte and status read
#define INTENAR   (*(volatile const word*)0xdff01C)		//  *R   P       Interrupt enable bits read
#define INTREQR   (*(volatile const word*)0xdff01E)		//  *R   P       Interrupt request bits read

#define DSKPT	  (*(volatile ptr_t*)0xdff020)			//+ *W   A       Disk pointer (low 15 bits)
#define DSKPTH    (*(volatile word*)0xdff020)			//+ *W   A( E )  Disk pointer (high 3 bits, 5 bits if ECS)
#define DSKPTL    (*(volatile word*)0xdff022)			//+ *W   A       Disk pointer (low 15 bits)
#define DSKLEN    (*(volatile word*)0xdff024)			//  *W   P       Disk length
#define DSKDAT    (*(volatile word*)0xdff026)			//& *W   P       Disk DMA data write
#define REFPTR    (*(volatile word*)0xdff028)			//& *W   A       Refresh pointer
#define VPOSW     (*(volatile word*)0xdff02A)			//  *W   A       Write vert most signif. bit (and frame flop)
#define VHPOSW    (*(volatile word*)0xdff02C)			//  *W   A       Write vert and horiz position of beam
#define COPCON    (*(volatile word*)0xdff02E)			//  *W   A( E )  Coprocessor control register (CDANG)
#define SERDAT    (*(volatile word*)0xdff030)			//  *W   P       Serial port data and stop bits write
#define SERPER    (*(volatile word*)0xdff032)			//  *W   P       Serial port period and control
#define POTGO     (*(volatile word*)0xdff034)			//  *W   P       Pot port data write and start
#define JOYTEST   (*(volatile word*)0xdff036)			//  *W   D       Write to all four joystick-mouse counters at once
#define STREQU    (*(volatile word*)0xdff038)			//& *S   D       Strobe for horiz sync with VB and EQU
#define STRVBL    (*(volatile word*)0xdff03A)			//& *S   D       Strobe for horiz sync with VB (vert. blank)
#define STRHOR    (*(volatile word*)0xdff03C)			//& *S   DP      Strobe for horiz sync
#define STRLONG   (*(volatile word*)0xdff03E)			//& *S   D( E )  Strobe for identification of long horiz. line.
#define BLTCON0   (*(volatile word*)0xdff040)			//  ~W   A       Blitter control register 0
#define BLTCON1   (*(volatile word*)0xdff042)			//  ~W   A( E )  Blitter control register 1
#define BLTAWM    (*(volatile dword*)0xdff044)			//  ~W   A       Blitter first word mask for source A
#define BLTAFWM   (*(volatile word*)0xdff044)			//  ~W   A       Blitter first word mask for source A
#define BLTALWM   (*(volatile word*)0xdff046)			//  ~W   A       Blitter last word mask for source A
#define BLTCPT    (*(volatile ptr_t*)0xdff048)			//+ ~W   A       Blitter pointer to source C (high 3 bits)
#define BLTCPTH   (*(volatile word*)0xdff048)			//+ ~W   A       Blitter pointer to source C (high 3 bits)
#define BLTCPTL   (*(volatile word*)0xdff04A)			//+ ~W   A       Blitter pointer to source C (low 15 bits)
#define BLTBPT    (*(volatile ptr_t*)0xdff04C)			//+ ~W   A       Blitter pointer to source B (high 3 bits)
#define BLTBPTH   (*(volatile word*)0xdff04C)			//+ ~W   A       Blitter pointer to source B (high 3 bits)
#define BLTBPTL   (*(volatile word*)0xdff04E)			//+ ~W   A       Blitter pointer to source B (low 15 bits)
#define BLTAPT    (*(volatile ptr_t*)0xdff050)			//+ ~W   A( E )  Blitter pointer to source A (high 3 bits)
#define BLTAPTH   (*(volatile word*)0xdff050)			//+ ~W   A( E )  Blitter pointer to source A (high 3 bits)
#define BLTAPTL   (*(volatile word*)0xdff052)			//+ ~W   A       Blitter pointer to source A (low 15 bits)
#define BLTDPT    (*(volatile ptr_t*)0xdff054)			//+ ~W   A       Blitter pointer to destination D (high 3 bits)
#define BLTDPTH   (*(volatile word*)0xdff054)			//+ ~W   A       Blitter pointer to destination D (high 3 bits)
#define BLTDPTL   (*(volatile word*)0xdff056)			//+ ~W   A       Blitter pointer to destination D (low 15 bits)
#define BLTSIZE   (*(volatile word*)0xdff058)			//  ~W   A       Blitter start and size (window width,height)
#define BLTCON0L  (*(volatile word*)0xdff05A)			//  ~W   A( E )  Blitter control 0, lower 8 bits (minterms)
#define BLTSIZV   (*(volatile word*)0xdff05C)			//  ~W   A( E )  Blitter V size (for 15 bit vertical size)
#define BLTSIZH   (*(volatile word*)0xdff05E)			//  ~W   A( E )  Blitter H size and start (for 11 bit H size)
#define BLTCMOD   (*(volatile word*)0xdff060)			//  ~W   A       Blitter modulo for source C
#define BLTBMOD   (*(volatile word*)0xdff062)			//  ~W   A       Blitter modulo for source B
#define BLTAMOD   (*(volatile word*)0xdff064)			//  ~W   A       Blitter modulo for source A
#define BLTDMOD   (*(volatile word*)0xdff066)			//  ~W   A       Blitter modulo for destination D

#define BLTCDAT   (*(volatile word*)0xdff070)			//% ~W   A       Blitter source C data register
#define BLTBDAT   (*(volatile word*)0xdff072)			//% ~W   A       Blitter source B data register

#define BLTADAT   (*(volatile word*)0xdff074)			//% ~W   A       Blitter source A data register

#define SPRHDAT   (*(volatile word*)0xdff078)			//  ~W   A( E )  Ext. logic UHRES sprite pointer and data id

#define DENISEID  (*(volatile const word*)0xdff07C)		//  ~R   D( E )  Chip revision level for Denise (video out chip)
#define DSKSYNC   (*(volatile word*)0xdff07E)			//  ~W   P       Disk sync pattern register for disk read
#define COP1LC    (*(volatile ptr_t*)0xdff080)			//+  W   A( E )  Coprocessor first location register (high 3 bits, high 5 bits if ECS)
#define COP1LCH   (*(volatile word*)0xdff080)			//+  W   A( E )  Coprocessor first location register (high 3 bits, high 5 bits if ECS)
#define COP1LCL   (*(volatile word*)0xdff082)			//+  W   A       Coprocessor first location register (low 15 bits)
#define COP2LC    (*(volatile ptr_t*)0xdff084)			//+  W   A( E )  Coprocessor second location register (high 3 bits, high 5 bits if ECS)
#define COP2LCH   (*(volatile word*)0xdff084)			//+  W   A( E )  Coprocessor second location register (high 3 bits, high 5 bits if ECS)
#define COP2LCL   (*(volatile word*)0xdff086)			//+  W   A       Coprocessor second location register

#define COPJMP1   (*(volatile word*)0xdff088)			//   S   A       Coprocessor restart at first location
#define COPJMP2   (*(volatile word*)0xdff08A)			//   S   A       Coprocessor restart at second location
#define COPINS    (*(volatile word*)0xdff08C)			//   W   A       Coprocessor instruction fetch identify
#define DIWSTRT   (*(volatile word*)0xdff08E)			//   W   A       Display window start (upper left vert-horiz position)
#define DIWSTOP   (*(volatile word*)0xdff090)			//   W   A       Display window stop (lower right vert.-horiz. position)
#define DDFSTRT   (*(volatile word*)0xdff092)			//   W   A       Display bitplane data fetch start (horiz. position)
#define DDFSTOP   (*(volatile word*)0xdff094)			//   W   A       Display bitplane data fetch stop (horiz. position)
#define DMACON    (*(volatile word*)0xdff096)			//   W   ADP     DMA control write (clear or set)
#define CLXCON    (*(volatile word*)0xdff098)			//   W   D       Collision control
#define INTENA    (*(volatile word*)0xdff09A)			//   W   P       Interrupt enable bits (clear or set bits)
#define INTREQ    (*(volatile word*)0xdff09C)			//   W   P       Interrupt request bits (clear or set bits)
#define ADKCON    (*(volatile word*)0xdff09E)			//   W   P       Audio, disk, UART control
#define AUD0LC    (*(volatile ptr_t*)0xdff0A0)			//+  W   A( E )  Audio channel 0 location (high 3 bits, 5 if ECS)
#define AUD0LCH   (*(volatile word*)0xdff0A0)			//+  W   A( E )  Audio channel 0 location (high 3 bits, 5 if ECS)
#define AUD0LCL   (*(volatile word*)0xdff0A2)			//+  W   A       Audio channel 0 location (low 15 bits)
#define AUD0LEN   (*(volatile word*)0xdff0A4)			//   W   P       Audio channel 0 length
#define AUD0PER   (*(volatile word*)0xdff0A6)			//   W   P( E )  Audio channel 0 period
#define AUD0VOL   (*(volatile word*)0xdff0A8)			//   W   P       Audio channel 0 volume
#define AUD0DAT   (*(volatile word*)0xdff0AA)			//&  W   P       Audio channel 0 data

#define AUD1LC    (*(volatile ptr_t*)0xdff0B0)			//+  W   A       Audio channel 1 location (high 3 bits)
#define AUD1LCH   (*(volatile word*)0xdff0B0)			//+  W   A       Audio channel 1 location (high 3 bits)
#define AUD1LCL   (*(volatile word*)0xdff0B2)			//+  W   A       Audio channel 1 location (low 15 bits)
#define AUD1LEN   (*(volatile word*)0xdff0B4)			//   W   P       Audio channel 1 length
#define AUD1PER   (*(volatile word*)0xdff0B6)			//   W   P       Audio channel 1 period
#define AUD1VOL   (*(volatile word*)0xdff0B8)			//   W   P       Audio channel 1 volume
#define AUD1DAT   (*(volatile word*)0xdff0BA)			//&  W   P       Audio channel 1 data

#define AUD2LC    (*(volatile ptr_t*)0xdff0C0)			//+  W   A       Audio channel 2 location (high 3 bits)
#define AUD2LCH   (*(volatile word*)0xdff0C0)			//+  W   A       Audio channel 2 location (high 3 bits)
#define AUD2LCL   (*(volatile word*)0xdff0C2)			//+  W   A       Audio channel 2 location (low 15 bits)
#define AUD2LEN   (*(volatile word*)0xdff0C4)			//   W   P       Audio channel 2 length
#define AUD2PER   (*(volatile word*)0xdff0C6)			//   W   P       Audio channel 2 period
#define AUD2VOL   (*(volatile word*)0xdff0C8)			//   W   P       Audio channel 2 volume
#define AUD2DAT   (*(volatile word*)0xdff0CA)			//&  W   P       Audio channel 2 data

#define AUD3LC    (*(volatile ptr_t*)0xdff0D0)			//+  W   A       Audio channel 3 location (high 3 bits)
#define AUD3LCH   (*(volatile word*)0xdff0D0)			//+  W   A       Audio channel 3 location (high 3 bits)
#define AUD3LCL   (*(volatile word*)0xdff0D2)			//+  W   A       Audio channel 3 location (low 15 bits)
#define AUD3LEN   (*(volatile word*)0xdff0D4)			//   W   P       Audio channel 3 length
#define AUD3PER   (*(volatile word*)0xdff0D6)			//   W   P       Audio channel 3 period
#define AUD3VOL   (*(volatile word*)0xdff0D8)			//   W   P       Audio channel 3 volume
#define AUD3DAT   (*(volatile word*)0xdff0DA)			//&  W   P       Audio channel 3 data

#define BPL1PT    (*(volatile ptr_t*)0xdff0E0)			//+  W   A       Bitplane 1 pointer (high 3 bits)
#define BPL1PTH   (*(volatile word*)0xdff0E0)			//+  W   A       Bitplane 1 pointer (high 3 bits)
#define BPL1PTL   (*(volatile word*)0xdff0E2)			//+  W   A       Bitplane 1 pointer (low 15 bits)
#define BPL2PT    (*(volatile ptr_t*)0xdff0E4)			//+  W   A       Bitplane 2 pointer (high 3 bits)
#define BPL2PTH   (*(volatile word*)0xdff0E4)			//+  W   A       Bitplane 2 pointer (high 3 bits)
#define BPL2PTL   (*(volatile word*)0xdff0E6)			//+  W   A       Bitplane 2 pointer (low 15 bits)
#define BPL3PT    (*(volatile ptr_t*)0xdff0E8)			//+  W   A       Bitplane 3 pointer (high 3 bits)
#define BPL3PTH   (*(volatile word*)0xdff0E8)			//+  W   A       Bitplane 3 pointer (high 3 bits)
#define BPL3PTL   (*(volatile word*)0xdff0EA)			//+  W   A       Bitplane 3 pointer (low 15 bits)
#define BPL4PT    (*(volatile ptr_t*)0xdff0EC)			//+  W   A       Bitplane 4 pointer (high 3 bits)
#define BPL4PTH   (*(volatile word*)0xdff0EC)			//+  W   A       Bitplane 4 pointer (high 3 bits)
#define BPL4PTL   (*(volatile word*)0xdff0EE)			//+  W   A       Bitplane 4 pointer (low 15 bits)
#define BPL5PT    (*(volatile ptr_t*)0xdff0F0)			//+  W   A       Bitplane 5 pointer (high 3 bits)
#define BPL5PTH   (*(volatile word*)0xdff0F0)			//+  W   A       Bitplane 5 pointer (high 3 bits)
#define BPL5PTL   (*(volatile word*)0xdff0F2)			//+  W   A       Bitplane 5 pointer (low 15 bits)
#define BPL6PT    (*(volatile ptr_t*)0xdff0F4)			//+  W   A       Bitplane 6 pointer (high 3 bits)
#define BPL6PTH   (*(volatile word*)0xdff0F4)			//+  W   A       Bitplane 6 pointer (high 3 bits)
#define BPL6PTL   (*(volatile word*)0xdff0F6)			//+  W   A       Bitplane 6 pointer (low 15 bits)

#define BPLCON0   (*(volatile word*)0xdff100)			//   W   AD( E ) Bitplane control register (misc. control bits)
#define BPLCON1   (*(volatile word*)0xdff102)			//   W   D       Bitplane control reg. (scroll value PF1, PF2)
#define BPLCON2   (*(volatile word*)0xdff104)			//   W   D( E )  Bitplane control reg. (priority control)
#define BPLCON3   (*(volatile word*)0xdff106)			//   W   D( E )  Bitplane control (enhanced features)
#define BPL1MOD   (*(volatile word*)0xdff108)			//   W   A       Bitplane modulo (odd planes)
#define BPL2MOD   (*(volatile word*)0xdff10A)			//   W   A       Bitplane modulo (even planes)

#define BPL1DAT   (*(volatile word*)0xdff110)			//&  W   D       Bitplane 1 data (parallel-to-serial convert)
#define BPL2DAT   (*(volatile word*)0xdff112)			//&  W   D       Bitplane 2 data (parallel-to-serial convert)
#define BPL3DAT   (*(volatile word*)0xdff114)			//&  W   D       Bitplane 3 data (parallel-to-serial convert)
#define BPL4DAT   (*(volatile word*)0xdff116)			//&  W   D       Bitplane 4 data (parallel-to-serial convert)
#define BPL5DAT   (*(volatile word*)0xdff118)			//&  W   D       Bitplane 5 data (parallel-to-serial convert)
#define BPL6DAT   (*(volatile word*)0xdff11A)			//&  W   D       Bitplane 6 data (parallel-to-serial convert)

#define SPR0PT    (*(volatile ptr_t*)0xdff120)			//+  W   A       Sprite 0 pointer (high 3 bits)
#define SPR0PTH   (*(volatile word*)0xdff120)			//+  W   A       Sprite 0 pointer (high 3 bits)
#define SPR0PTL   (*(volatile word*)0xdff122)			//+  W   A       Sprite 0 pointer (low 15 bits)
#define SPR1PT    (*(volatile ptr_t*)0xdff124)			//+  W   A       Sprite 1 pointer (high 3 bits)
#define SPR1PTH   (*(volatile word*)0xdff124)			//+  W   A       Sprite 1 pointer (high 3 bits)
#define SPR1PTL   (*(volatile word*)0xdff126)			//+  W   A       Sprite 1 pointer (low 15 bits)
#define SPR2PT    (*(volatile ptr_t*)0xdff128)			//+  W   A       Sprite 2 pointer (high 3 bits)
#define SPR2PTH   (*(volatile word*)0xdff128)			//+  W   A       Sprite 2 pointer (high 3 bits)
#define SPR2PTL   (*(volatile word*)0xdff12A)			//+  W   A       Sprite 2 pointer (low 15 bits)
#define SPR3PT    (*(volatile ptr_t*)0xdff12C)			//+  W   A       Sprite 3 pointer (high 3 bits)
#define SPR3PTH   (*(volatile word*)0xdff12C)			//+  W   A       Sprite 3 pointer (high 3 bits)
#define SPR3PTL   (*(volatile word*)0xdff12E)			//+  W   A       Sprite 3 pointer (low 15 bits)
#define SPR4PT    (*(volatile ptr_t*)0xdff130)			//+  W   A       Sprite 4 pointer (high 3 bits)
#define SPR4PTH   (*(volatile word*)0xdff130)			//+  W   A       Sprite 4 pointer (high 3 bits)
#define SPR4PTL   (*(volatile word*)0xdff132)			//+  W   A       Sprite 4 pointer (low 15 bits)
#define SPR5PT    (*(volatile ptr_t*)0xdff134)			//+  W   A       Sprite 5 pointer (high 3 bits)
#define SPR5PTH   (*(volatile word*)0xdff134)			//+  W   A       Sprite 5 pointer (high 3 bits)
#define SPR5PTL   (*(volatile word*)0xdff136)			//+  W   A       Sprite 5 pointer (low 15 bits)
#define SPR6PT    (*(volatile ptr_t*)0xdff138)			//+  W   A       Sprite 6 pointer (high 3 bits)
#define SPR6PTH   (*(volatile word*)0xdff138)			//+  W   A       Sprite 6 pointer (high 3 bits)
#define SPR6PTL   (*(volatile word*)0xdff13A)			//+  W   A       Sprite 6 pointer (low 15 bits)
#define SPR7PT    (*(volatile ptr_t*)0xdff13C)			//+  W   A       Sprite 7 pointer (high 3 bits)
#define SPR7PTH   (*(volatile word*)0xdff13C)			//+  W   A       Sprite 7 pointer (high 3 bits)
#define SPR7PTL   (*(volatile word*)0xdff13E)			//+  W   A       Sprite 7 pointer (low 15 bits)

#define SPR		  ((sprite_t*)0xdff140)
#define SPR0POS   (*(volatile word*)0xdff140)			//%  W   AD      Sprite 0 vert-horiz start position data
#define SPR0CTL   (*(volatile word*)0xdff142)			//%  W   AD( E ) Sprite 0 vert stop position and control data
#define SPR0DAT   (*(volatile ptr_t*)0xdff144)			//%  W   D       Sprite 0 image data register A
#define SPR0DATA  (*(volatile word*)0xdff144)			//%  W   D       Sprite 0 image data register A
#define SPR0DATB  (*(volatile word*)0xdff146)			//%  W   D       Sprite 0 image data register B
#define SPR1POS   (*(volatile word*)0xdff148)			//%  W   AD      Sprite 1 vert-horiz start position data
#define SPR1CTL   (*(volatile word*)0xdff14A)			//%  W   AD      Sprite 1 vert stop position and control data
#define SPR1DAT   (*(volatile ptr_t*)0xdff14C)			//%  W   D       Sprite 1 image data register A
#define SPR1DATA  (*(volatile word*)0xdff14C)			//%  W   D       Sprite 1 image data register A
#define SPR1DATB  (*(volatile word*)0xdff14E)			//%  W   D       Sprite 1 image data register B
#define SPR2POS   (*(volatile word*)0xdff150)			//%  W   AD      Sprite 2 vert-horiz start position data
#define SPR2CTL   (*(volatile word*)0xdff152)			//%  W   AD      Sprite 2 vert stop position and control data
#define SPR2DAT   (*(volatile ptr_t*)0xdff154)			//%  W   D       Sprite 2 image data register A
#define SPR2DATA  (*(volatile word*)0xdff154)			//%  W   D       Sprite 2 image data register A
#define SPR2DATB  (*(volatile word*)0xdff156)			//%  W   D       Sprite 2 image data register B
#define SPR3POS   (*(volatile word*)0xdff158)			//%  W   AD      Sprite 3 vert-horiz start position data
#define SPR3CTL   (*(volatile word*)0xdff15A)			//%  W   AD      Sprite 3 vert stop position and control data
#define SPR3DAT   (*(volatile ptr_t*)0xdff15C)			//%  W   D       Sprite 3 image data register A
#define SPR3DATA  (*(volatile word*)0xdff15C)			//%  W   D       Sprite 3 image data register A
#define SPR3DATB  (*(volatile word*)0xdff15E)			//%  W   D       Sprite 3 image data register B
#define SPR4POS   (*(volatile word*)0xdff160)			//%  W   AD      Sprite 4 vert-horiz start position data
#define SPR4CTL   (*(volatile word*)0xdff162)			//%  W   AD      Sprite 4 vert stop position and control data
#define SPR4DAT   (*(volatile ptr_t*)0xdff164)			//%  W   D       Sprite 4 image data register A
#define SPR4DATA  (*(volatile word*)0xdff164)			//%  W   D       Sprite 4 image data register A
#define SPR4DATB  (*(volatile word*)0xdff166)			//%  W   D       Sprite 4 image data register B
#define SPR5POS   (*(volatile word*)0xdff168)			//%  W   AD      Sprite 5 vert-horiz start position data
#define SPR5CTL   (*(volatile word*)0xdff16A)			//%  W   AD      Sprite 5 vert stop position and control data
#define SPR5DAT   (*(volatile ptr_t*)0xdff16C)			//%  W   D       Sprite 5 image data register A
#define SPR5DATA  (*(volatile word*)0xdff16C)			//%  W   D       Sprite 5 image data register A
#define SPR5DATB  (*(volatile word*)0xdff16E)			//%  W   D       Sprite 5 image data register B
#define SPR6POS   (*(volatile word*)0xdff170)			//%  W   AD      Sprite 6 vert-horiz start position data
#define SPR6CTL   (*(volatile word*)0xdff172)			//%  W   AD      Sprite 6 vert stop position and control data
#define SPR6DAT   (*(volatile ptr_t*)0xdff174)			//%  W   D       Sprite 6 image data register A
#define SPR6DATA  (*(volatile word*)0xdff174)			//%  W   D       Sprite 6 image data register A
#define SPR6DATB  (*(volatile word*)0xdff176)			//%  W   D       Sprite 6 image data register B
#define SPR7POS   (*(volatile word*)0xdff178)			//%  W   AD      Sprite 7 vert-horiz start position data
#define SPR7CTL   (*(volatile word*)0xdff17A)			//%  W   AD      Sprite 7 vert stop position and control data
#define SPR7DAT   (*(volatile ptr_t*)0xdff17C)			//%  W   D       Sprite 7 image data register A
#define SPR7DATA  (*(volatile word*)0xdff17C)			//%  W   D       Sprite 7 image data register A
#define SPR7DATB  (*(volatile word*)0xdff17E)			//%  W   D       Sprite 7 image data register B

#define COLOR	  ((volatile word*)0xdff180)
#define COLOR00   (*(volatile word*)0xdff180)			//   W   D       Color table 00
#define COLOR01   (*(volatile word*)0xdff182)			//   W   D       Color table 01
#define COLOR02   (*(volatile word*)0xdff184)			//   W   D       Color table 02
#define COLOR03   (*(volatile word*)0xdff186)			//   W   D       Color table 03
#define COLOR04   (*(volatile word*)0xdff188)			//   W   D       Color table 04
#define COLOR05   (*(volatile word*)0xdff18A)			//   W   D       Color table 05
#define COLOR06   (*(volatile word*)0xdff18C)			//   W   D       Color table 06
#define COLOR07   (*(volatile word*)0xdff18E)			//   W   D       Color table 07
#define COLOR08   (*(volatile word*)0xdff190)			//   W   D       Color table 08
#define COLOR09   (*(volatile word*)0xdff192)			//   W   D       Color table 09
#define COLOR10   (*(volatile word*)0xdff194)			//   W   D       Color table 10
#define COLOR11   (*(volatile word*)0xdff196)			//   W   D       Color table 11
#define COLOR12   (*(volatile word*)0xdff198)			//   W   D       Color table 12
#define COLOR13   (*(volatile word*)0xdff19A)			//   W   D       Color table 13
#define COLOR14   (*(volatile word*)0xdff19C)			//   W   D       Color table 14
#define COLOR15   (*(volatile word*)0xdff19E)			//   W   D       Color table 15
#define COLOR16   (*(volatile word*)0xdff1A0)			//   W   D       Color table 16
#define COLOR17   (*(volatile word*)0xdff1A2)			//   W   D       Color table 17
#define COLOR18   (*(volatile word*)0xdff1A4)			//   W   D       Color table 18
#define COLOR19   (*(volatile word*)0xdff1A6)			//   W   D       Color table 19
#define COLOR20   (*(volatile word*)0xdff1A8)			//   W   D       Color table 20
#define COLOR21   (*(volatile word*)0xdff1AA)			//   W   D       Color table 21
#define COLOR22   (*(volatile word*)0xdff1AC)			//   W   D       Color table 22
#define COLOR23   (*(volatile word*)0xdff1AE)			//   W   D       Color table 23
#define COLOR24   (*(volatile word*)0xdff1B0)			//   W   D       Color table 24
#define COLOR25   (*(volatile word*)0xdff1B2)			//   W   D       Color table 25
#define COLOR26   (*(volatile word*)0xdff1B4)			//   W   D       Color table 26
#define COLOR27   (*(volatile word*)0xdff1B6)			//   W   D       Color table 27
#define COLOR28   (*(volatile word*)0xdff1B8)			//   W   D       Color table 28
#define COLOR29   (*(volatile word*)0xdff1BA)			//   W   D       Color table 29
#define COLOR30   (*(volatile word*)0xdff1BC)			//   W   D       Color table 30
#define COLOR31   (*(volatile word*)0xdff1BE)			//   W   D       Color table 31

#define HTOTAL    (*(volatile word*)0xdff1C0)			//   W   A( E )  Highest number count, horiz line (VARBEAMEN=1)
#define HSSTOP    (*(volatile word*)0xdff1C2)			//   W   A( E )  Horizontal line position for HSYNC stop
#define HBSTRT    (*(volatile word*)0xdff1C4)			//   W   A( E )  Horizontal line position for HBLANK start
#define HBSTOP    (*(volatile word*)0xdff1C6)			//   W   A( E )  Horizontal line position for HBLANK stop
#define VTOTAL    (*(volatile word*)0xdff1C8)			//   W   A( E )  Highest numbered vertical line (VARBEAMEN=1)
#define VSSTOP    (*(volatile word*)0xdff1CA)			//   W   A( E )  Vertical line position for VSYNC stop
#define VBSTRT    (*(volatile word*)0xdff1CC)			//   W   A( E )  Vertical line for VBLANK start
#define VBSTOP    (*(volatile word*)0xdff1CE)			//   W   A( E )  Vertical line for VBLANK stop

#define BEAMCON0  (*(volatile word*)0xdff1DC)			//   W   A( E )  Beam counter control register (SHRES,PAL)
#define HSSTRT    (*(volatile word*)0xdff1DE)			//   W   A( E )  Horizontal sync start (VARHSY)
#define VSSTRT    (*(volatile word*)0xdff1E0)			//   W   A( E )  Vertical sync start   (VARVSY)
#define HCENTER   (*(volatile word*)0xdff1E2)			//   W   A( E )  Horizontal position for Vsync on interlace
#define DIWHIGH   (*(volatile word*)0xdff1E4)			//   W   AD( E ) Display window -  upper bits for start, stop


									//    CIAA Address Map
									//    ---------------------------------------------------------------------------
									//    Byte    Register                  Data bits
									//    Address    Name     7     6     5     4     3     2     1    0
									//    ---------------------------------------------------------------------------
#define CIAA_PRA     (*(volatile byte*)0xBFE001)		//	/FIR1 /FIR0  /RDY /TK0  /WPRO /CHNG /LED  OVL
#define CIAA_PRB     (*(volatile byte*)0xBFE101)		//	Parallel port
#define CIAA_DDRA    (*(volatile byte*)0xBFE201)		//	Direction for port A(BFE001); 1=output(set to 0x03)
#define CIAA_DDRB    (*(volatile byte*)0xBFE301)		//	Direction for port B(BFE101); 1=output(can be in or out)
#define CIAA_TALO    (*(volatile byte*)0xBFE401)		//	CIAA timer A low byte(.715909 Mhz NTSC; .709379 Mhz PAL)
#define CIAA_TAHI    (*(volatile byte*)0xBFE501)		//	CIAA timer A high byte
#define CIAA_TBLO    (*(volatile byte*)0xBFE601)		//	CIAA timer B low byte(.715909 Mhz NTSC; .709379 Mhz PAL)
#define CIAA_TBHI    (*(volatile byte*)0xBFE701)		//	CIAA timer B high byte
#define CIAA_TODLO   (*(volatile byte*)0xBFE801)		//	50/60 Hz event counter bits 7-0 (VSync or line tick)
#define CIAA_TODMID  (*(volatile byte*)0xBFE901)		//	50/60 Hz event counter bits 15-8
#define CIAA_TODHI   (*(volatile byte*)0xBFEA01)		//	50/60 Hz event counter bits 23-16
//					 (*(volatile byte*)0xBFEB01)		//	not used
#define CIAA_SDR     (*(volatile byte*)0xBFEC01)		//	CIAA serial data register (connected to keyboard)
#define CIAA_ICR     (*(volatile byte*)0xBFED01)		//	CIAA interrupt control register
#define CIAA_CRA     (*(volatile byte*)0xBFEE01)		//	CIAA control register A
#define CIAA_CRB     (*(volatile byte*)0xBFEF01)		//	CIAA control register B


									//   CIAB Address Map
									//   ---------------------------------------------------------------------------
									//   Byte     Register                   Data bits
									//   Address     Name     7     6     5     4     3     2     1     0
									//   ---------------------------------------------------------------------------
#define CIAB_PRA     (*(volatile byte*)0xBFD000)		//  /DTR  /RTS  /CD   /CTS  /DSR   SEL   POUT  BUSY
#define CIAB_PRB     (*(volatile byte*)0xBFD100)		//  /MTR  /SEL3 /SEL2 /SEL1 /SEL0 /SIDE  DIR  /STEP
#define CIAB_DDRA    (*(volatile byte*)0xBFD200)		//  Direction for Port A(BFD000); 1 = output(set to 0xFF)
#define CIAB_DDRB    (*(volatile byte*)0xBFD300)		//  Direction for Port B(BFD100); 1 = output(set to 0xFF)
#define CIAB_TALO    (*(volatile byte*)0xBFD400)		//  CIAB timer A low byte(.715909 Mhz NTSC; .709379 Mhz PAL)
#define CIAB_TAHI    (*(volatile byte*)0xBFD500)		//  CIAB timer A high byte
#define CIAB_TBLO    (*(volatile byte*)0xBFD600)		//  CIAB timer B low byte(.715909 Mhz NTSC; .709379 Mhz PAL)
#define CIAB_TBHI    (*(volatile byte*)0xBFD700)		//  CIAB timer B high byte
#define CIAB_TODLO   (*(volatile byte*)0xBFD800)		//  Horizontal sync event counter bits 7-0
#define CIAB_TODMID  (*(volatile byte*)0xBFD900)		//  Horizontal sync event counter bits 15-8
#define CIAB_TODHI   (*(volatile byte*)0xBFDA00)		//  Horizontal sync event counter bits 23-16
//					 (*(volatile byte*)0xBFDB00)		//  not used
#define CIAB_SDR     (*(volatile byte*)0xBFDC00)		//  CIAB serial data register (unused)
#define CIAB_ICR     (*(volatile byte*)0xBFDD00)		//  CIAB interrupt control register
#define CIAB_CRA     (*(volatile byte*)0xBFDE00)		//  CIAB Control register A
#define CIAB_CRB     (*(volatile byte*)0xBFDF00)		//  CIAB Control register B




//#define KEYSCAN_Q		0x10
//#define KEYSCAN_W		0x11
//#define KEYSCAN_E		0x12
//#define KEYSCAN_A		0x20
//#define KEYSCAN_S		0x21
//#define KEYSCAN_D		0x22


#define	KEYSCAN_Q		0x10
#define	KEYSCAN_W		0x11
#define	KEYSCAN_E		0x12
#define	KEYSCAN_R		0x13
#define	KEYSCAN_T		0x14
#define	KEYSCAN_Y		0x15
#define	KEYSCAN_U		0x16
#define	KEYSCAN_I		0x17
#define	KEYSCAN_O		0x18
#define	KEYSCAN_P		0x19
//#define	KEYSCAN_[		0x1A
//#define	KEYSCAN_]		0x1B
//#define	KEYSCAN_		0x1C
//#define	KEYSCAN_1		0x1D
//#define	KEYSCAN_2		0x1E
//#define	KEYSCAN_3		0x1F
#define	KEYSCAN_A		0x20
#define	KEYSCAN_S		0x21
#define	KEYSCAN_D		0x22
#define	KEYSCAN_F		0x23
#define	KEYSCAN_G		0x24
#define	KEYSCAN_H		0x25
#define	KEYSCAN_J		0x26
#define	KEYSCAN_K		0x27
#define	KEYSCAN_L		0x28
//#define	KEYSCAN_;		0x29
//#define	KEYSCAN_'		0x2A
//#define	KEYSCAN_		0x2B
//#define	KEYSCAN_		0x2C
//#define	KEYSCAN_4		0x2D
//#define	KEYSCAN_5		0x2E
//#define	KEYSCAN_6		0x2F
//#define	KEYSCAN_		0x30
#define	KEYSCAN_Z		0x31
#define	KEYSCAN_X		0x32
#define	KEYSCAN_C		0x33
#define	KEYSCAN_V		0x34
#define	KEYSCAN_B		0x35
#define	KEYSCAN_N		0x36
#define	KEYSCAN_M		0x37
//#define	KEYSCAN_,		0x38
//#define	KEYSCAN_.		0x39






#define KEYSCAN_ESC		0x45
#define KEYSCAN_TAB		0x42

#define KEYSCAN_F1		0x50		// this are expected to be sequential
#define KEYSCAN_F2		0x51
#define KEYSCAN_F3		0x52
#define KEYSCAN_F4		0x53
#define KEYSCAN_F5		0x54
#define KEYSCAN_F6		0x55
#define KEYSCAN_F7		0x56
#define KEYSCAN_F8		0x57
#define KEYSCAN_F9		0x58
#define KEYSCAN_F10		0x59

#define KEYSCAN_1		0x01		// this are expected to be sequential
#define KEYSCAN_2		0x02
#define KEYSCAN_3		0x03
#define KEYSCAN_4		0x04
#define KEYSCAN_5		0x05
#define KEYSCAN_6		0x06
#define KEYSCAN_7		0x07
#define KEYSCAN_8		0x08
#define KEYSCAN_9		0x09
#define KEYSCAN_0		0x0A

#define KEYSCAN_SPACE	0x40

#define KEYSCAN_LBRACE	0x1A
#define KEYSCAN_RBRACE	0x1B

#define KEYSCAN_UP		0x4C
#define KEYSCAN_DOWN	0x4D
#define KEYSCAN_LEFT	0x4F
#define KEYSCAN_RIGHT	0x4E
#define KEYSCAN_ENTER	0x44


#endif
