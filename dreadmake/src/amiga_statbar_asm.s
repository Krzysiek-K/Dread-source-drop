
		
		xref	_statbar_buffer
		xref	_statbar_blit_scheduled_cmd



; ================================================================ _Blit_Pass_StatusBar - new version ================================================================


_Blit_Pass_StatusBar:
	;
	; only d0-d1/a0-a1 are saved
	;	a1 = CHIP_BASE
	;	
	;

	move.l		_statbar_blit_scheduled_cmd, d0
	beq			.end_stop
	
	move.l		d0, a0										; a0 = AmigaStatbarBlitDef*

	move.l		statblit_bltcon0(a0), BLTCON0(a1)			;	word		bltcon0;			// BLTCON0 + BLTCON1 can be copied as one dword
															;	word		bltcon1;			//
	move.l		statblit_bltafwm(a0), BLTAFWM(a1)			;	word		bltafwm;			// BLTAFWM + BLTALWM can be copied as one dword
															;	word		bltalwm;			//

	move.w		#$FFFF, BLTADAT(a1)							;		BLTADAT = $FFFF

	move.l		statblit_bltcdmod(a0), d0					;	word		bltcdmod;			// BLTCMOD + BLTBMOD can be copied as one dword
	move.l		d0, BLTCMOD(a1)								;	word		bltabmod;			// BLTAMOD + BLTDMOD can be copied as one dword (after word-swapping the above)
	swap.w		d0											;
	move.l		d0, BLTAMOD(a1)								;

	move.l		statblit_source(a0), d0						;	const word	*source;			// runtime
	move.l		d0, BLTBPT(a1)								;
	add.w		statblit_srcmask_offs(a0), d0				;	word		srcmask_offs;
	move.l		d0, BLTAPT(a1)								;

	moveq.l		#0, d0										;	word		target_offs;
	move.w		statblit_target_offs(a0), d0				;
	add.l		_statbar_buffer, d0							;
	move.l		d0, BLTCPT(a1)								;
	move.l		d0, BLTDPT(a1)								;

	move.w		statblit_bltsize(a0), BLTSIZE(a1)			;	word		bltsize;

	move.l		statblit_next_blit(a0), d0					;	void		*next_blit;			// runtime
	move.l		d0, _statbar_blit_scheduled_cmd
	bne			.end_continue


.end_stop:
	moveq.l		#0, d0
	move.l		d0, _Blit_IRQ_fn
	rts

.end_continue:
	move.l		#_Blit_Pass_StatusBar, _Blit_IRQ_fn
	rts
