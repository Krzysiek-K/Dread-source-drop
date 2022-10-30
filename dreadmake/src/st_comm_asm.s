

		xref _comm_rx_buff		; byte[256]
		xref _comm_rx_put		; word

		xref _comm_tx_buff		; byte[256]
		xref _comm_tx_put		; word
		xref _comm_tx_get		; word



; ================================================================ _ST_Comm_Init_Asm ================================================================

	public _ST_Comm_Init_Asm
_ST_Comm_Init_Asm:

	move.b		#$04, USART_TX_STATUS			;	USART_TX_STATUS = 0x04;		// HL = 10
	move.b		#$00, USART_TX_STATUS

	move.b		#2, TIMER_D_DATA				;	TIMER_D_DATA = 2;
	move.b		TIMER_CD_CONTROL, d0			;	TIMER_CD_CONTROL = (TIMER_CD_CONTROL & 0xF8) | 0x01;
	andi.b		#$F8, d0
	ori.b		#$01, d0
	move.b		d0, TIMER_CD_CONTROL

	move.b		#$98, USART_CONTROL				;	USART_CONTROL = 0x98;		// divide by 16, 8 bits, async 1-stop bit, no parity
	move.b		#$05, USART_TX_STATUS			;	USART_TX_STATUS = 0x05;		// HL = 10, TX enabled
	move.b		#$01, USART_RX_STATUS			;	USART_RX_STATUS = 0x01;

	move.l		#int_usart_rx, $100+12*4		;	RS-232 receive buffer full
	move.l		#int_usart_tx, $100+10*4		;	RS-232 transmit buffer empty


	move.b		INT_ENABLE_A, d0				; disable RS-232 interrupts
	and.b		#$E1, d0
  if ST_COMM_LEVEL == 0
	or.b		#$14, d0						;	enable interrupts
  else
	or.b		#$10, d0						;	enable interrupts
  endif
	move.b		d0, INT_ENABLE_A
	move.b		INT_MASK_A, d0					; mask RS-232 interrupts
	and.b		#$E1, d0
	or.b		#$10, d0						;	unmask interrupts
	move.b		d0, INT_MASK_A
	move.b		#$E1, INT_PENDING_A				; clear pending RS-232 interrupts
	move.b		#$E1, INT_INSERVICE_A			; acknowledge RS-232 interrupts

	rts


int_usart_rx:
	movem.l		d0-d1/a0, -(a7)

	lea.l		_comm_rx_buff, a0
	move.w		_comm_rx_put, d0
	move.b		USART_DATA, (a0,d0.w)
	add.b		#1, d0
	move.w		d0, _comm_rx_put

	movem.l		(a7)+, d0-d1/a0
	bclr		#4, INT_INSERVICE_A
	rte

int_usart_tx:
  if ST_COMM_LEVEL == 0
	movem.l		d0-d1/a0, -(a7)
	move.w		_comm_tx_get, d0
	cmp.w		_comm_tx_put, d0
	beq			.end_stop

	lea.l		_comm_tx_buff, a0
	move.b		(a0,d0.w), USART_DATA
	add.b		#1, d0
	move.w		d0, _comm_tx_get
.end:
	movem.l		(a7)+, d0-d1/a0
  endif
	bclr		#2, INT_INSERVICE_A
	rte

.end_stop:
	move.b		INT_MASK_A, d0					; mask RS-232 interrupts
	andi.b		#$FB, d0						;	mask TX interrupt
	move.b		d0, INT_MASK_A
	movem.l		(a7)+, d0-d1/a0
	bclr		#2, INT_INSERVICE_A
	rte



; ================================================================ _ST_Comm_Commit ================================================================

	public _ST_Comm_Commit
_ST_Comm_Commit:

	move.w		_comm_tx_get, d0
	cmp.w		_comm_tx_put, d0
	beq			.end

	move.b		INT_MASK_A, d0					; mask RS-232 interrupts
	or.b		#$04, d0						;	unmask TX interrupt
	move.b		d0, INT_MASK_A
	nop
	nop

	move.b		USART_TX_STATUS, d0
	and.b		#$80, d0
	beq.b		.end							; TX busy, let interrupt send it

	move.w		_comm_tx_get, d0				; TX ready, and interrupt didn`t kick in - send the first byte
	lea.l		_comm_tx_buff, a0
	move.b		(a0,d0.w), USART_DATA
	add.b		#1, d0
	move.w		d0, _comm_tx_get

.end:
	rts


; ================================================================ _ST_Comm_In ================================================================

	public _ST_Comm_In
_ST_Comm_In:
	move.w		#1, -(a7)					; d0 = Bconstat(1)
	move.w		#bios_bconstat, -(a7)
	trap		#trap_bios
	addq.l		#4, a7
	tst.w		d0
	beq			.end_empty

	move.w		#1, -(a7)					; d0 = Bconin(1)
	move.w		#bios_bconin, -(a7)
	trap		#trap_bios
	addq.l		#4, a7

.end:
	rts

.end_empty:
	move.w		#$FFFF, d0
	rts


; ================================================================ _ST_Comm_Out ================================================================

	public _ST_Comm_Out
_ST_Comm_Out:
	move.w		d0, -(a7)					; Bconout(1, d0)
	move.w		#1, -(a7)
	move.w		#bios_bconout, -(a7)
	trap		#trap_bios
	addq.l		#6, a7
	rts
