	public _Dread_DrawLines
_Dread_DrawLines:
	movem.l		d2-d7/a2-a6,-(a7)
	move.l		a0, .vis_orig_save
	move.w		(a0)+, d0							;		word line = MAP_VIS[vis++];
	cmp.w		#$FFFF, d0							;		if( line==0xFFFF ) break;
	beq.b		.end								; [nothing at all to draw - just exit]
.loop:												;		while(1)
													;		{
	move.w		(a0)+, d7							;			word mode = MAP_VIS[vis++];
	mulu.w		#line_sizeof, d0					;			Dread_LineWall_Fix2_asm(e_lines + line, mode);
	lea.l		_e_lines, a5						;
	lea.l		(a5,d0), a5							;
	move.l		a0, .vis_save						;
	bsr.b		_Dread_LineWall_Fix2_asm			;
	move.l		.vis_save, a0						;
	move.w		(a0)+, d0							;			word line = MAP_VIS[vis++];
	cmp.w		#$FFFF, d0							;			if( line==0xFFFF ) break;
	bne.b		.loop								;		}


													;		// cleanup verts
	lea.l		_e_lines, a5						; a5: e_lines
	moveq.l		#0, d1								; d1: 0
	move.l		.vis_orig_save, a0					;		vis = lrc_cam_bspvis;
	move.w		(a0)+, d0							;		word line = MAP_VIS[vis++];
.clean_loop:										;		while(1)
													;		{
	mulu.w		#line_sizeof, d0					;
	lea.l		(a5,d0), a1							;
	move.l		line_v1(a1), a2						;			e_lines[line].v1->tr_x = 0;
	move.l		d1, vtx_tr_x(a2)					;
	move.l		line_v2(a1), a2						;			e_lines[line].v2->tr_x = 0;
	move.l		d1, vtx_tr_x(a2)					;
	addq.l		#2, a0								;			vis++;
	move.w		(a0)+, d0							;			word line = MAP_VIS[vis++];
	cmp.w		#$FFFF, d0							;			if( line==0xFFFF ) break;
	bne.b		.clean_loop							;		}

.end:
	movem.l		(a7)+,d2-d7/a2-a6
	rts

.vis_save:			dc.l	0
.vis_orig_save:		dc.l	0
