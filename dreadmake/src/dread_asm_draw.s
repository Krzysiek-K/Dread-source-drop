

	xref	_e_vislines
	xref	_e_map_vislist

	xref	_e_visthings
	xref	_e_visthings_end
	xref	_e_visthings_max

	xref	_view_frame_number

	public _Dread_DrawVisible
	public _Dread_DrawLinesTable




  if WALL_ORDER_DEBUG == 1
draw_line_limit:	dc.w		0
  endif


  if USE_VIS_GROUPS == 0

; ================================================================ _Dread_DrawVisible  (USE_VIS_GROUPS=0) ================================================================
;	d0		-		(temp)
;	d1
;	d2		-		view_frame_number
;	d3		-		last_frame

;	a0		-		(temp)
;	a1		-		(temp)
;	a2		- IN	visibility list
;	a3		-		_e_vislines write pointer
;	a4		-		_e_visthings write pointer
;	a5		-		_e_lines base address
;	a6		-		_e_visthings_max

_Dread_DrawVisible:
	movem.l		d2-d7/a2-a6,-(a7)

	move.w		_view_frame_number, d2					;	word last_frame = (word)(view_frame_number-1);						[d2: view_frame_number]		[d3: last_frame=view_frame_number-1]
	move.w		d2, d3
	subq.w		#1, d3

	lea.l		_e_vislines, a3
	move.l		_e_visthings_end, a4
	move.l		_e_lines, a5
	move.l		_e_visthings_max, a6
	
  if WALL_ORDER_DEBUG == 1
	move.w		_debug_vars+10, draw_line_limit		; TBD: remove debug
  endif

	bra.b		.core_loop


.process_line:
  if WALL_ORDER_DEBUG == 1
	sub.w		#1, draw_line_limit
	bmi			.core_loop
  endif
	move.w		d0, (a3)+							;		vislines->mode = cmd;
	lsr.w		#1, d0								;		vislines->line = e_lines + (cmd>>1);
	mulu.w		#line_sizeof, d0
	lea.l		(a5,d0.l), a0
	move.l		a0, (a3)+							;		vislines++;

.core_loop:
	move.w		(a2)+, d0							;		word cmd = MAP_VIS[vis++];
	bpl.b		.process_line						;	[ $0000..$7FFF - line ]
	
	cmp.w		#$C000, d0
	bhs.b		.not_a_thingsector

.process_thingsector:								;	[ $8000..$BFFF - thing sector ]
	add.w		d0, d0								; [ remove $8000 offset ]
	mulu.w		#subsector_sizeof/2, d0				;		EngineSubSector *ss = e_subsectors + (*thingvis++);			[a0]
	move.l		_e_subsectors, a0
	lea.l		(a0,d0), a0
	
	cmp.w		subsector_visframe(a0), d3			;		if( ss->visframe!=last_frame )
	beq.b		.subsector_done						;		{
												
	move.l		subsector_first_thing(a0), d0		;			EngineThing *th = ss->first_thing;						[a1 (via d0)]
	beq.b		.subsector_done						;			while( th )
.thing_loop:										;			{
	move.l		d0, a1
	move.b		thing_visible(a1), d0				;				if( !th->visible && e_n_visthings<ENGINE_MAX_VISTHINGS )
	bne.b		.thing_next							;				{
	cmp.l		a6, a4								; [a4:visthings ? a6:max]
	bge.b		.end
	move.l		a1, (a4)+							;					e_visthings[e_n_visthings++] = th;
	move.b		#1, thing_visible(a1)				;					th->visible = 1;
.thing_next:										;				}
	move.l		thing_ss_next(a1), d0				;				th = th->ss_next;
	bne.b		.thing_loop							;			}
.subsector_done:									;		}
	move.w		d2, subsector_visframe(a0)			;		ss->visframe = view_frame_number;

	bra.b		.core_loop


.not_a_thingsector:
;	cmp.w		#$E000, d0
;	bhi.b		.core_loop							;	[ $E000..$FFFF - group, not implemented ]

.condition_check:									;	[ $C001..$DFFF - condition code ]
	sub.w		#$C001, d0
	blo.b		.end								;	[ $C000 - end ]
	lea.l		_e_condition_states, a1
	move.b		(a1,d0.w), d0
	bne.b		.core_loop							;	visibility enabled - continue

.invis_loop:
	move.w		(a2)+, d0							;	skip items until next visibility token
	cmp.w		#$C000, d0
	blo.b		.invis_loop
	bne.b		.condition_check
	;bra.b		.end

.end:
	move.w		#$FFFF, (a3)
	move.l		a4, _e_visthings_end
	movem.l		(a7)+,d2-d7/a2-a6
	rts


  else

; ================================================================ _Dread_DrawVisible  (USE_VIS_GROUPS=1) ================================================================
;	d0		-		(temp)
;	d1
;	d2		-		view_frame_number
;	d3		-		last_frame

;	a0		-		(temp)
;	a1		-		visibility end	/  (temp)
;	a2		- IN	visibility list
;	a3		-		_e_vislines write pointer
;	a4		-		_e_visthings write pointer
;	a5		-		_e_lines base address
;	a6		-		_e_visthings_max

_Dread_DrawVisible:
	movem.l		d2-d7/a2-a6,-(a7)

	move.w		_view_frame_number, d2					;	word last_frame = (word)(view_frame_number-1);						[d2: view_frame_number]		[d3: last_frame=view_frame_number-1]
	move.w		d2, d3
	subq.w		#1, d3

	move.l		#$FFFFFFFF, a1							; endvis
	lea.l		_e_vislines, a3
	move.l		_e_visthings_end, a4
	move.l		_e_lines, a5
	move.l		_e_visthings_max, a6
	move.w		#1, .vismode
	
  if WALL_ORDER_DEBUG == 1
	move.w		_debug_vars+10, draw_line_limit
  endif


.core_loop:
	cmpa.l		a1, a2								;	if( vis == endvis )
	beq			.group_return						;		goto group_return;
	moveq.l		#0, d0
	move.w		(a2)+, d0							;		word cmd = *vis++;
	beq			.end								;	[ $0000 - END ]
	bpl			.group_call							;	[ $0001..$7FFF - group ]

	cmp.w		#$C000, d0							;	[ $8000..$BFFF - line ]
	bhs			.not_a_line
.process_line:
  if WALL_ORDER_DEBUG == 1
	sub.w		#1, draw_line_limit
	bmi			.core_loop
  endif
	move.w		.vismode, (a3)+						;		vislines->mode = cmd;
	sub.w		#$8000, d0							;		vislines->line = e_lines + (cmd-0x8000);
	mulu.w		#line_sizeof, d0
	lea.l		(a5,d0.l), a0
	move.l		a0, (a3)+							;		vislines++;
	bra.b		.core_loop


.not_a_line:
	cmp.w		#$F000, d0							;	[ $C000..$EFFF - thing sector ]
	bhs			.not_a_thingsector
.process_thingsector:				
	move.l		a1, -(a7)							;	save <a1>
	sub.w		#$C000, d0							; [ remove $C000 offset ]
	mulu.w		#subsector_sizeof, d0				;		EngineSubSector *ss = e_subsectors + (*thingvis++);			[a0]
	move.l		_e_subsectors, a0
	lea.l		(a0,d0), a0
	
	cmp.w		subsector_visframe(a0), d3			;		if( ss->visframe!=last_frame )
	beq.b		.subsector_done						;		{
												
	move.l		subsector_first_thing(a0), d0		;			EngineThing *th = ss->first_thing;						[a1 (via d0)]
	beq.b		.subsector_done						;			while( th )
.thing_loop:										;			{
	move.l		d0, a1
	move.b		thing_visible(a1), d0				;				if( !th->visible && e_n_visthings<ENGINE_MAX_VISTHINGS )
	bne.b		.thing_next							;				{
	cmp.l		a6, a4								; [a4:visthings ? a6:max]
	bge.b		.end
	move.l		a1, (a4)+							;					e_visthings[e_n_visthings++] = th;
	move.b		#1, thing_visible(a1)				;					th->visible = 1;
.thing_next:										;				}
	move.l		thing_ss_next(a1), d0				;				th = th->ss_next;
	bne.b		.thing_loop							;			}
.subsector_done:									;		}
	move.w		d2, subsector_visframe(a0)			;		ss->visframe = view_frame_number;
	move.l		(a7)+, a1							;	restore <a1>
	bra.b		.core_loop


.not_a_thingsector:
.condition_check:									;	[ $F000..$FFFF - condition/vismode code ]
	sub.w		#$F000, d0
	move.w		d0, .vismode
	lsr.w		d0
	lea.l		_e_condition_states, a1
	move.b		(a1,d0.w), d0
	bne			.core_loop							;	visibility enabled - continue

.invis_loop:
	move.w		(a2)+, d0							;	skip items until next visibility token
	beq			.end								;	[ $0000 - END ]
	cmp.w		#$F000, d0
	blo			.invis_loop
	bra			.condition_check


	; Group call
	;
.group_call:
	move.l		a2, .return_vis_ptr					; save sector vis pointer
	lsl.l		d0									;		vis = e_map_vislist + (cmd<<1);
	move.l		_e_map_vislist, a2
	lea.l		(a2,d0.l), a2
	move.l		a2, a1								;		endvis = vis;						[a1]
.group_seek_start:									;		while( *--vis != 0) {}
	move.w		-(a2), d0							;
	bne			.group_seek_start					;
	adda.l		#2, a2								;		vis++;
	bra			.core_loop


	; Group return
	;
.group_return:
	move.l		.return_vis_ptr, a2					; restore sector vis pointer
	move.l		#$FFFFFFFF, a1						; disable endvis
	bra			.core_loop



.end:
	move.w		#$FFFF, (a3)
	move.l		a4, _e_visthings_end
	movem.l		(a7)+,d2-d7/a2-a6
	rts


.return_vis_ptr:	dc.l		0
.vismode:			dc.w		1



  endif




; ================================================================ _Dread_DrawLinesTable ================================================================

_Dread_DrawLinesTable:
	movem.l		d2-d7/a2-a6,-(a7)

;	TRAP_ICACHE_DISABLE

	lea.l		_e_vislines, a0

	move.w		(a0)+, d7
	bmi.b		.end								; [nothing at all to draw]
.loop:												;		for( ; vis->mode < 0x8000 ; vis++ )
													;		{
	and.w		#1, d7								;			word mode = vis->mode & 1;

	move.l		(a0)+, a5							;			Dread_LineWall_Fix2_asm(vis->line, mode);
	move.b		line_modes(a5,d7.w), d7				; [upper byte already cleared by AND]
	move.l		a0, .vis_save						;
	bsr.w		_Dread_LineWall_Fix2_asm			;
	move.l		.vis_save, a0						;
	
	move.w		(a0)+, d7							;		}
	bpl.b		.loop								;



.draw_done:
													; ================================ cleanup verts ================================
	lea.l		_e_vislines, a0
	moveq.l		#0, d1

	move.w		(a0)+, d7							; [list already checked to be non-empty]
.clean_loop:										;		for( ; vis->mode < 0x8000 ; vis++ )
													;		{
	move.l		(a0)+, a5							;			EngineLine *line = vis->line;
	move.l		line_v1(a5), a1						;			line->v1->tr_x = 0;
	move.l		d1, vtx_tr_x(a1)					;
	move.l		line_v2(a5), a1						;			line->v2->tr_x = 0;
	move.l		d1, vtx_tr_x(a1)					;
	move.w		(a0)+, d7							;		}
	bpl.b		.clean_loop							;

.end:
;	TRAP_ICACHE_ENABLE

	movem.l		(a7)+,d2-d7/a2-a6
	rts

.vis_save:			dc.l	0
