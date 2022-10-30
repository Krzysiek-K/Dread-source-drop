
	if MULTIPLAYER_MODE = 1

		xref _comm_rx_buff		; byte[256]
		xref _comm_rx_put		; word

		xref _comm_tx_buff		; byte[256]
		xref _comm_tx_put		; word
		xref _comm_tx_get		; word


; ================================================================ _Comm_Commit ================================================================
	public _Comm_Commit
_Comm_Commit:

	move.w		_comm_tx_get, d0
	cmp.w		_comm_tx_put, d0
	beq			.end

	lea.l		CHIP_BASE, a1

	move.w		INTENAR(a1), d0
	and.w		#$0001, d0						; check TBE
	bne			.end

	move.w		#$0001, INTREQ(a1)
	move.w		#$8001, INTENA(a1)				; unmask TBE


	lea.l		_comm_tx_buff, a0				; SERDAT = 0x0100 | comm_tx_buff[d0];
	move.w		_comm_tx_get, d0				; d0 = comm_tx_get
	move.w		#$0100, d1
	move.b		(a0,d0.w), d1
	move.w		d1, SERDAT(a1)
	add.b		#1, d0							; comm_tx_get = byte(d0 + 1);
	move.w		d0, _comm_tx_get

.end:
	rts


; ================================================================ Irq_TBE ================================================================

Irq_TBE:
	movem.l		d0-d7/a0-a6, -(a7)
	lea.l		CHIP_BASE, a1

;.loop:
;	addq.l		#1, d0
;	move.w		d0, COLOR00(a1)
;	bra			.loop
	move.w		#$0F0F, COLOR00(a1)

	move.w		_comm_tx_get, d0			; d0 = comm_tx_get
	cmp.w		_comm_tx_put, d0			; if( d0 == comm_tx_put ) -> .end_stop
	beq			.end_stop

	lea.l		_comm_tx_buff, a0			; SERDAT = 0x0100 | comm_tx_buff[d0];
.send_next:
	move.w		#$0100, d1
	move.b		(a0,d0.w), d1
	move.w		d1, SERDAT(a1)
	move.w		#$0001, INTREQ(a1)

	add.b		#1, d0						; comm_tx_get = byte(d0 + 1);
	move.w		d0, _comm_tx_get
	cmp.w		_comm_tx_put, d0			; if( d0 != comm_tx_put ) -> .end
	beq			.end_stop

;	move.w		SERDATR(a1), d1
;	and.w		#$2000, d1
;	bne			.send_next

	bra			.end

.end_stop:
	move.w		#$0001, INTENA(a1)				; disable TBE

.end:
	movem.l		(a7)+, d0-d7/a0-a6
	rte


; ================================================================ Irq_RBF ================================================================
Irq_RBF:
	movem.l		d0-d7/a0-a6, -(a7)
	lea.l		CHIP_BASE, a1

	move.w		SERDATR(a1), d1
	move.w		#$0800, INTREQ(a1)

	lea.l		_comm_rx_buff, a0
	move.w		_comm_rx_put, d0
	move.b		d1, (a0,d0.w)
	add.b		#1, d0
	move.w		d0, _comm_rx_put

	; end
	movem.l		(a7)+, d0-d7/a0-a6
	rte


	endif		; MULTIPLAYER_MODE = 1
