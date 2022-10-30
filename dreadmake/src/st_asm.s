; custom, minimal startup for C code
;(c)2011 Mariusz Buras (all), Pawel Goralski (modifications for C)

REDIRECT_OUTPUT_TO_SERIAL	equ	0	;0-output to console,1-output to serial port


    xref	_main

	xref	_intnum
	xref	_rawintnum
	xref	_debug_vars
	xref	_io_mouse_xdelta
	xref	_io_keystate
	xref	_io_key_last
	xref	_io_mouse_state






	include	"st__macros.s"
	include	"st_system.s"
	include	"st_fn.s"
	include	"st_ikbd.s"
	include	"st_c2p.s"
	include	"st_draw.s"
	include	"st_audio.s"
	include	"st_comm_asm.s"

	SECTION_DATA
	include	"data/frame_asm_st.i"
