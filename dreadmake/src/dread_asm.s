	
	xref	_e_skyptr
	xref	_render_buffer
	xref	_render_columns
	xref	_FN_COLORFILL_START
	xref	_FN_COLORFILL_MID
	xref	_FN_COLORFILL_END
	xref	_FN_SKYCOPY
	xref	_SIZE_LINE_CHEAT
	xref	_FUNC_COLORFILL
	xref	_FUNC_SKYCOPY
	xref	_render_layout_delta
	xref	_e_subsectors

	xref	_e_map_bspnodes			; DataBSPNode*



	public _asm_line_ds
	public _asm_tex_base
	public _asm_tex_base2
	public _asm_fn_skycopy
	public _asm_render_buffer
	public _asm_line_s2
	public _asm_line_xlen
	public _asm_line_du
_asm_line_ds:			dc.w	0
_asm_tex_base:			dc.l	0
_asm_tex_base2:			dc.l	0
_a7_save:				dc.l	0
_asm_fn_skycopy:		dc.l	0
_asm_render_buffer:		dc.l	0
_asm_line_s2:			dc.w	0
_asm_line_xlen:			dc.w	0
_asm_line_du:			dc.l	0


ENGINE_SUBVERTEX_PRECISION	equ	  3												; Do not change. Not fully used yet. Also redefined in C. And the map converted. And the cheat generator.
ENGINE_THING_Z_PRECISION	equ	  6												; Also redefined in C.
ENGINE_WIDTH				equ	160
ENGINE_ZOOM					equ	 80
ENGINE_ZOOM_CONSTANT		equ  (ENGINE_ZOOM*32)<<(8+ENGINE_SUBVERTEX_PRECISION)		; #(80*32)<<12
ENGINE_MIN_ZNEAR			equ  (ENGINE_ZOOM_CONSTANT/65536)+1
ENGINE_PICKUP_DISTANCE		equ	(32<<ENGINE_SUBVERTEX_PRECISION)

ENGINE_PLAYER_SIZE			equ	 15			; Redefined in C and tools.



cheatmode_fn_persp			equ	  0
cheatmode_fn_nopersp		equ	  4
cheatmode_clip				equ	  8



rc_ymin						equ   4
rc_ymax						equ   6
rc_size_limit				equ  (160*8+0)
rc_thing_num				equ  (160*8+2)
rc_sprite_render_addr		equ  (160*8+4)





  if AMIGA
	include	"gen/gen_asmstruct.i"
  endc
  if ATARI_ST
	include	"gen/gen_asmstruct_st.i"
  endc
	include	"dread_asm_sprites.s"
	include	"dread_asm_experimental.s"
  if WALL_RENDERER_VERSION == 1
	include	"gen/line_cheats.s"
  else
	include	"gen/render2_linedefs.s"
  endif
	include	"dread_asm_fn.s"
	include	"dread_asm_lines.s"
	include	"dread_asm_draw.s"
	include	"dread_asm_scripts.s"
	include	"dread_asm_weaponsprites.s"


	public _Dread_FrameReset
	public _Dread_FindSubSector



; ================================================================ _Dread_FrameReset ================================================================



_Dread_FrameReset:
	movem.l		d2/a2, -(a7)

	move.w		#160-1, d1
	lea.l		_render_columns, a0
	lea.l		_render_layout_delta, a1
  if WALL_RENDERER_VERSION == 1
	move.l		#ENGINE_Y_MAX*2, d0				;	ymin = 0;				ymax =  100*2;
  else
	move.l		#ENGINE_Y_MAX, d0				;	ymin = 0;				ymax =  100;
  endif
	move.l		#0, d2							;	<d2> is zero

.loop:
	move.w		d2, rc_thing_num(a0)
	add.w		(a1)+, a2						; compute column addr
	move.l		a2, rc_sprite_render_addr(a0)
	move.l		a2, (a0)+
	move.l		d0, (a0)+
	dbra.w		d1, .loop

	movem.l		(a7)+, d2/a2
	rts



; ================================================================ _Dread_FindSubSector ================================================================

_Dread_FindSubSector:
	;	d2.w:	xp
	;	d3.w:	yp
	;
	movem.l		d4, -(a7)

	move.l		_e_map_bspnodes, a0
	moveq.l		#0, d0						;		short bsp = 0;
											;		do
.loop:										;		{
	lea.l		(a0,d0.w), a1				;			const DataBSPNode *node = (const DataBSPNode*)(((const byte*)e_map_bspnodes) + bsp);
	move.w		node_A(a1), d1				;			int side = muls(view_pos_x, node->A) + muls(view_pos_y, node->B) + node->C;
	muls.w		d2, d1						;
	move.w		node_B(a1), d4				;
	muls.w		d3, d4						;
	add.l		d4, d1						;
	add.l		node_C(a1), d1				;
	bmi.b		.left_side					;			bsp = side>=0 ? node->right : node->left;
	addq.l		#2, a1						;
.left_side:									;
	move.w		node_left(a1), d0			;
	bpl.b		.loop						;		} while( bsp>=0 );
	
	not.w		d0							;		view_subsector = e_subsectors + ~bsp;
	mulu.w		#subsector_sizeof, d0		;
	add.l		_e_subsectors, d0			;
	
	movem.l		(a7)+, d4
	rts
