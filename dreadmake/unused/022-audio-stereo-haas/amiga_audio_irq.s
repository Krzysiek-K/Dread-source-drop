



;******************** Irq_AudioBufferStarted ********************

Irq_AudioBufferStarted:					; IV_LEVEL_4	$70		; audio channel 0..3
	movem.l		d0-d1/a0-a2, -(a7)
	lea.l		CHIP_BASE, a1

	move.w		INTREQR(a1), d0						; INTREQR
	
	lea.l		_Aud_Channels, a2
	btst.l		#7, d0
	beq.b		.noaud0
	jsr			AudChan_Buffer_Started
.noaud0:

	adda.l		#aud_sizeof, a2
	btst.l		#8, d0
	beq.b		.noaud1
	jsr			AudChan_Buffer_Started
.noaud1:

	adda.l		#aud_sizeof, a2
	btst.l		#9, d0
	beq.b		.noaud2
	jsr			AudChan_Buffer_Started
.noaud2:

	adda.l		#aud_sizeof, a2
	btst.l		#10, d0
	beq.b		.noaud3
	jsr			AudChan_Buffer_Started
.noaud3:

	; end
	movem.l		(a7)+, d0-d1/a0-a2
	rte




;******************** Irq_CIAA ********************
Irq_CIAA:
	movem.l		d0-d1/a0-a2, -(a7)
	lea.l		CHIP_BASE, a1

	add.w		#1, aud_tick_count

	tst.b		CIAA_ICR
	move.w		#$0008,INTREQ(a1)
	move.w		#$0008,INTREQ(a1)

	lea.l		_Aud_Channels, a2
	jsr			AudChan_Tick

	adda.l		#aud_sizeof, a2
	jsr			AudChan_Tick

	adda.l		#aud_sizeof, a2
	jsr			AudChan_Tick

	adda.l		#aud_sizeof, a2
	jsr			AudChan_Tick

	move.w		#0, _aud_volume_update_request
	
	move.b		#CIACRBF_START|CIACRBF_ONESHOT|CIACRBF_LOAD, CIAB_CRB 

	movem.l		(a7)+, d0-d1/a0-a2
	rte



;******************** Irq_CIAB ********************

Irq_CIAB:
	movem.l		d0-d1/a0-a3, -(a7)
	lea.l		CHIP_BASE, a1
	
	tst.b		CIAB_ICR
	move.w		#$2000,INTREQ(a1)
	move.w		#$2000,INTREQ(a1)

	lea.l		_Aud_Channels, a2
	jsr			AudChan_Tock

	adda.l		#aud_sizeof, a2
	jsr			AudChan_Tock

	adda.l		#aud_sizeof, a2
	jsr			AudChan_Tock

	adda.l		#aud_sizeof, a2
	jsr			AudChan_Tock

	movem.l		(a7)+, d0-d1/a0-a3
	rte

