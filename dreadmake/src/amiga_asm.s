


;	include "defs.s"


; Set demo compatibility
; 0: 68000 only, Kickstart 1.3 only, PAL only
; 1: All CPUs, Kickstarts and display modes
; 2: 68010+, Kickstart 3.0+, all display modes
COMPATIBILITY=				1

; Set to 1 to require fast memory
FASTMEM=					0


COMPILING_BOOTLOADER		=		0



	if AMIGA_AUDIO_ENGINE_VERSION == 2
AMIGA_AUDIO_ENGINE_VERSION_2	equ		1
	else
AMIGA_AUDIO_ENGINE_VERSION_2	equ		0
	endif



	include "amiga__macros.s"
	include	"amiga__sys_macros.s"
	include	"amiga__startup.s"
	include	"amiga__system.s"
	include "amiga__defs.s"
	include	"amiga__traps.s"
	include	"amiga_asmblit.s"
	include	"amiga_framework.s"
	include	"amiga_audio.s"
	include	"amiga_audio_irq.s"
	include	"amiga_audio2.s"
	include	"amiga_audio2_irq.s"
	include	"amiga_statbar_asm.s"
	include	"amiga_comm_asm.s"
	include	"dread_asm.s"
	include	"inflate.asm"
  if TRACKLOADER
	include	"amiga_trackloader.s"
  endif


	xref	_Precalc
	xref	_Exit
	xref	_Main
	xref	_InterruptMain

	include xcode.asm





******************** ScreenWait ********************
	public _ScreenWait
_ScreenWait:
.waitforscreen1:
	cmp.b	#$2c,$dff006
	blt		.waitforscreen1
	move.w	#$f00,$dff180
	rts


******************** BlitWait ********************

	public _BlitWait
_BlitWait:
; DMACONR   =	$dff002			;  *R   AP      DMA control (and blitter status) read
	tst		$dff002				;for compatibility
.waitblit:
	btst	#6, $dff002
	bne.s	.waitblit
	rts



******************** GetCIAA_TimerA ********************

	public _GetCIAA_TimerA
_GetCIAA_TimerA:
	moveq.l	#0, d0
	move.b	CIAA_TALO, d0		;	CIAA timer A low byte(.715909 Mhz NTSC; .709379 Mhz PAL)
	move.b	CIAA_TAHI, d1		;	CIAA timer A high byte
	lsl.w	#8, d1
	or.w	d1, d0
	rts


******************** Foo ********************
	public _Foo
_Foo:
	WinUAEBreakpoint
	rts





************************


	SECTION_DATA_CHIP

	public		_SOUND_NONE
_SOUND_TKPISTOL:					; TBD: remove
_SOUND_TKPISTOL_SUP:
_SOUND_TKROCKT:
_SOUND_TKCLIP:
_SOUND_DSNOCARD:
_SOUND_DSCHGN3:
_SOUND_DSCHGN3S:
_SOUND_DSCHGN3E:
_SOUND_DSPOPAIN2:
_SOUND_DSHEADATK:
_SOUND_DSHEADPAIN1:
_SOUND_DSHEADPAIN2:
_SOUND_DSHEADDTH:
_SOUND_NONE:
	dc.w		1, 0
ZeroSample:
	dc.w		0


	SECTION_DATA

  if USE_EMBEDDED_MAP_DATA = 1
	include	"data/export_map_asm.i"
  endif
