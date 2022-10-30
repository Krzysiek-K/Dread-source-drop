
	xref	_GFX_STBAR0
	xref	_statbar_buffer
	xref	_e_status_bar
	xref	_e_status_bar_shadow
	xref	_STAT_DIGITS


	public	_Stat_Set_asm
	public	_Blit_Pass_StatusBar


	; Old precomputed blit perf		= 4.01



_Stat_Set_asm:
	;	a0	- stat
	;	d0	- value
	;	d2	- num_base

	and.l		#$0000FFFF, d0

	cmp.w		#999, d0						;	if( value>999 )
	bls			.max999							;		value = 999;
	move.w		#999, d0						;
.max999:

	cmp.w		(a0), d0						;	if( *stat == value )
	beq.b		.end							;		return;

	move.w		d0, (a0)						;	*stat = value;

	cmp.w		#100, d0						;	if( value >= 100 )
	blo			.under100						;	{
	divu.w		#100, d0						;		stat[1] = num_base + value/100;
	add.w		d2, d0							;
	move.w		d0, 2(a0)						;
	eor.w		d0, d0							;
	swap.w		d0								;		stat[2] = num_base + value%100/10;
	divu.w		#10, d0							;
	add.w		d2, d0							;
	move.w		d0, 4(a0)						;
	swap.w		d0								;		stat[3] = num_base + value%100%10;
	add.w		d2, d0							;
	move.w		d0, 6(a0)						;
.end:
	rts											;	}

.under100:
	cmp.w		#10, d0							;	else if( value>=10 )
	blo			.under10						;	{
	move.w		#0, 2(a0)						;		stat[1] = 0;
	divu.w		#10, d0							;		stat[2] = num_base + value/10;
	add.w		d2, d0							;
	move.w		d0, 4(a0)						;
	swap.w		d0								;		stat[3] = num_base + value%10;
	add.w		d2, d0							;
	move.w		d0, 6(a0)						;
	rts											;	}
												;	else
.under10:										;	{
	moveq.l		#0, d1							;		stat[1] = 0;
	move.l		d1, 2(a0)						;		stat[2] = 0;
	add.w		d2, d0							;		stat[3] = num_base + value;
	move.w		d0, 6(a0)						;
	rts											;	}
