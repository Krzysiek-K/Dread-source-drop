
		xref	_music_active
		xref	_music_track
		xref	_st_machine_type
		xref	_opt_sfx_enable


	;incbin	"data/st_music.snd"
	;incbin	"data/TRANS_Y.SND"
	;incbin	"data/SEQNOTIM.SND"



	SECTION_CODE


; ================================================================ _init_music ================================================================

	public	_init_music
_init_music:
  if ST_PLAY_MUSIC
		cmp.w		_music_track, d0			; already playing that track?
		beq			.end
		move.w		d0, _music_track


		move.l		_music_active, d0
		beq			.no_previous_msx
		clr.l		_music_active				; (disable VBL playback)
		move.l		d0, a0
		adda.w		#4, a0
		jsr			(a0)						; stop music
.no_previous_msx:					

		move.w		_music_track, d0			; restore <d0>
		lsl.w		#2, d0

		lea.l		.tracklist, a0
		move.l		(a0,d0.w), d0
		beq			.end
		move.l		d0, a0

		move.l		a0, .temp					; initialize muisic in <a0> and set it as active music
		moveq.l		#0, d0						; subsong #
		jsr			(a0)						; init music
		move.l		.temp, _music_active		; (enable VBL playback)
  endif
.end:
		rts

.temp:	dc.l		0
.tracklist:
		dc.l		0
		dc.l		_ST_Music_Menu
		dc.l		_ST_Music_Ingame
	


; ================================================================ _ST_Music_Ingame / _ST_Music_Menu ================================================================

  if ST_PLAY_MUSIC
_ST_Music_Ingame:
	incbin	"sound-st/DRDEIG0M.SND"

_ST_Music_Menu:
	incbin	"sound-st/DRDEMN04.SND"
  endif






; ================================================================ _Sound_PlayThingSound ================================================================
;
;	d0		- temp						(NOT SAVED)
;	d1		-							(NOT SAVED)
;	d2		-							(saved)
;	d7	IN	- sound attenuation			(saved)
;
;	a0	IN	- sound pointer				(NOT SAVED)
;	a1		- &.temp					(NOT SAVED)
;	a2		-							(saved)
;	a6	IN	- thing pointer				(saved)
;
	public _Sound_PlayThingSound
_Sound_PlayThingSound:
	move.w		_st_machine_type, d0
	beq			.end
	move.w		_opt_sfx_enable, d0
	beq			.end
	move.w		(a0), d0
	beq			.end

	tst.l		_vblank_playsound
	bne			.slot2
	move.l		a0, _vblank_playsound
	bra			.end

.slot2:
	tst.l		_vblank_playsound+4
	bne			.slot3
	move.l		a0, _vblank_playsound+4
	bra			.end

.slot3:
	move.l		a0, _vblank_playsound+8

.end:
	rts
