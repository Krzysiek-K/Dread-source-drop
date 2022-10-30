
	xref	_e_globals
	xref	_RAND_TAB
	xref	_view_weapon
	xref	_view_player
	xref	_e_delthings
	xref	_e_delthings_end
	xref	_e_dmgthings
	xref	_e_dmgthings_end
	xref	_e_dmgthings_max
	xref	_e_things
	xref	_e_n_things
	xref	_bspclip_p1x
	xref	_bspclip_p1y
	xref	_bspclip_p2x
	xref	_bspclip_p2y

  if WEAPONSPRITES
	xref	_weaponsprite_anim
	xref	_weaponsprite_delay
  endif

	xref	_Physics_MoveCollide
	xref	_Physics_EnemyWalkSlide
	xref	_Physics_MissileMoveClip
	xref	_Physics_EnemyWalkInit
	xref	_Physics_Hitscan
	xref	_Engine_ThingSetSubsector
	xref	_Engine_SpawnWeaponPuff
	xref	_Engine_SpawnMissile
	xref	_Engine_ExplosionDamage
	xref	_Engine_CheckRadiusBlocked
	xref	_Machine_LineScroll
	xref	_Machine_ThingUpdate
	xref	_Player_JumpToLocation
	xref	_Player_ApplyLocationDelta
	xref	_Game_TagCheck
	xref	_Comm_Write_Byte
	xref	_Comm_Write_Word

	public	_Script_TickObjects
	public	_Script_ResolveDamage
	public	_Script_OnCreate
	public	_Script_OnDamage
	public	_Script_OnPulse
	public	_Script_PlayerDamaged


	include	"data/export_scripts.i"
	


;	VM regs:
;	
;		d0	- (temp)
;		d1	- (temp)
;		d2	-
;		d3	-
;		d4	-
;		d5	-
;		d6	- reserved
;		d7	- reserved			ticks
;	
;		a0	-
;		a1	-
;		a2	-
;		a3	-					script fn
;		a4	-					e_visthings iterator
;		a5	- Globals ptr
;		a6	- Thing ptr
;		a7	- STACK


; ================ static storage ================

script_tick:			dc.w		0
script_stack_reset:		dc.l		0




; ================================================================ helper function ================================================================

; Register usage:
;	
;	d7.w	- script_tick
;
;	a3		- th->script_fn
;	a6		- EngineThing*		(input)
;

Tick_One_Object:

	; Function
	move.l		thing_script_fn(a6), d0						; th->script_fn == NULL		-> skip completely
	bne.b		.fn_not_null
	rts


.fn_not_null:
	move.l		d0, a3										; [a3] = th->script_fn
	move.w		script_tick, d7								; [d7] = script_tick

	sub.w		d7, thing_timer_attack(a6)					; TIMER:	timer_attack
	bcc.b		.timer_attack_ok
	move.w		#0, thing_timer_attack(a6)
.timer_attack_ok:

	sub.w		d7, thing_timer_walk(a6)					; TIMER:	timer_walk
	bcc.b		.timer_walk_ok
	move.w		#0, thing_timer_walk(a6)
.timer_walk_ok:

	sub.w		d7, thing_timer_delay(a6)					; TIMER:	timer_delay		(special, do not clamp here)
	bgt.b		.timer_delay_wait

	move.l		a7, script_stack_reset						; RUN	(only when timer_delay expired)
	jmp			(a3)

.timer_delay_wait:
	rts





; ================================================================ _Script_TickObjects ================================================================

_Script_TickObjects:
	movem.l		d2-d7/a2-a6,-(a7)

	move.w		d7, script_tick
	lea.l		_e_globals, a5


													; ============ Pass: tick weapon & player
	move.l		_view_weapon, a6
	bsr			Tick_One_Object

	move.l		_view_player, a6
	bsr			Tick_One_Object


	jsr			_Script_ResolveDamage_asm			;	call any pending OnDamage



													; ============ Pass: tick all things
	move.l		_e_visthings_end, a4
	cmp.l		#_e_visthings, a4
	ble			.done
.logic_loop:										;		for( EngineThing **ptr = e_visthings_end; ptr>e_visthings; )
													;		{
	move.l		-(a4), a6							;			EngineThing *th = *--ptr;

	move.l		a4, -(a7)							;	======== RUN LOGIC ========
	bsr			Tick_One_Object						;

	move.w		thing_machine_speed(a6), d0			;	======== MACHINE MOVE ========
	beq			.no_machine_move					;
	move.w		script_tick, d7						;
	jsr			_Machine_ThingUpdate				;
	beq			.no_machine_move					;

	bra			.zmove_skip							;	skip other forms of movement

.no_machine_move:

	move.w		thing_step_count(a6), d0			;	======== MOVE ========
	beq			.move_skip

	move.w		script_tick, d7						;		count steps
	move.b		thing_move_collide(a6), d0
	beq			.move_normal
	jsr			_Physics_MoveCollide
	move.w		d0, d0
	beq			.move_done
	move.w		d0, gv_pulse(a5)
	move.l		thing_actor(a6), a0
	move.l		actor_cb_pulse(a0), d0
	beq			.move_done
	move.l		d0, a0
	jsr			(a0)
	bra			.move_done

.move_normal:
	sub.w		d7, thing_step_count(a6)			;		expired - end movement in final position
	bhi			.move_step							;
	move.w		thing_step_endx(a6), thing_xp(a6)	;
	move.w		thing_step_endy(a6), thing_yp(a6)	;
	move.w		#0, thing_step_count(a6)			;
	bra			.move_done							;

.move_step:
	move.w		thing_step_xp(a6), d0				;		step X
	muls.w		d7, d0								;	
	asr.l		#3, d0								;	
	add.w		d0, thing_xp(a6)					;	
	move.w		thing_step_yp(a6), d0				;		step Y
	muls.w		d7, d0								;	
	asr.l		#3, d0								;	
	add.w		d0, thing_yp(a6)					;	

.move_done:
	move.w		thing_xp(a6), d2					;		update subsector
	move.w		thing_yp(a6), d3					;
	jsr			_Dread_FindSubSector				;
	move.l		d0, a0								;
	jsr			_Engine_ThingSetSubsector			;

	move.l		#0, d0								;		blocker?
	move.b		thing_blocker_size(a6), d0			;
	beq			.not_a_blocker						;

													;		update blocking region
	add.w		#ENGINE_PLAYER_SIZE, d0				;			short block_offs = (th->blocker_size + ENGINE_PLAYER_SIZE) << ENGINE_SUBVERTEX_PRECISION;
	lsl.w		#ENGINE_SUBVERTEX_PRECISION, d0		;

	move.w		thing_xp(a6), d1					;			th->block_x1 = th->xp - block_offs;
	sub.w		d0, d1								;
	move.w		d1, thing_block_x1(a6)				;
	add.w		d0, d1								;			th->block_x2 = th->xp + block_offs;
	add.w		d0, d1								;
	move.w		d1, thing_block_x2(a6)				;

	move.w		thing_yp(a6), d1					;			th->block_y1 = th->yp - block_offs;
	sub.w		d0, d1								;
	move.w		d1, thing_block_y1(a6)				;
	add.w		d0, d1								;			th->block_y2 = th->yp + block_offs;
	add.w		d0, d1								;
	move.w		d1, thing_block_y2(a6)				;
.not_a_blocker:

.move_skip:											; ======== Gravity ========
	move.b		thing_gravity(a6), d0				;
	beq			.zmove_skip							;
	ext.w		d0									; th->vel_z += th->gravity*ticks;
	muls.w		d7, d0								;
	
	move.w		thing_vel_z(a6), d1					;
	add.w		d1, d0								;
	move.w		d0, thing_vel_z(a6)					;
	;add.w		d1, d0								;
	;asr.w		#1, d0								; average old & new value
	move.w		d1, d0								;

	muls.w		d7, d0								; th->zp += th->vel_z*ticks;
	asr.l		#6, d0								;	extra Z velocity/gravity precision
	add.w		thing_zp(a6), d0					;
	bmi			.zmove_ok
	clr.w		thing_vel_z(a6)						;
	moveq.l		#0, d0
.zmove_ok:
	move.w		d0, thing_zp(a6)					;
.zmove_skip:

	move.l		(a7)+, a4							;


	cmp.l		#_e_visthings, a4				;		}
	bgt			.logic_loop

.done:

.end:
	movem.l		(a7)+,d2-d7/a2-a6
	rts



; ================================================================ _Script_ResolveDamage_asm ================================================================

_Script_ResolveDamage_asm:
	;movem.l		d2-d7/a2-a6,-(a7)

	;lea.l		_e_globals, a5

	move.l		_e_dmgthings_end, a0
	cmp.l		#_e_dmgthings, a0
	bls			.end

.loop:
	move.l		-(a0), a6
	move.l		thing_actor(a6), a4
	move.l		actor_cb_damage(a4), a3
	move.l		a0, -(a7)
	jsr			(a3)
	move.l		(a7)+, a0
	cmp.l		#_e_dmgthings, a0
	bhi			.loop

	move.l		a0, _e_dmgthings_end

.end:
	;movem.l		(a7)+,d2-d7/a2-a6
	rts




; ================================================================ _Script_OnCreate ================================================================

_Script_OnCreate:
	movem.l		d2-d7/a2-a6,-(a7)

	lea.l		_e_globals, a5

	move.l		thing_actor(a6), a4
	move.l		actor_cb_create(a4), a3
	cmp.l		#0, a3
	beq			.end
	jsr			(a3)

.end:
	movem.l		(a7)+,d2-d7/a2-a6
	rts



; ================================================================ _Script_OnDamage ================================================================

_Script_OnDamage:
	movem.l		d2-d7/a2-a6,-(a7)

	lea.l		_e_globals, a5

	move.l		thing_actor(a6), a4
	move.l		actor_cb_damage(a4), a3
	jsr			(a3)

.end:
	movem.l		(a7)+,d2-d7/a2-a6
	rts


; ================================================================ _Script_OnPulse ================================================================

_Script_OnPulse:
	movem.l		d2-d7/a2-a6,-(a7)

	lea.l		_e_globals, a5

	move.l		thing_actor(a6), a4
	move.l		actor_cb_pulse(a4), d0
	beq			.end
	move.l		d0, a3
	jsr			(a3)

.end:
	movem.l		(a7)+,d2-d7/a2-a6
	rts


; ================================================================ script functions ================================================================



; ================ Fn: collision_set ================

script_fn_collision_set_1:
	move.b		d7, thing_blocker_size(a6)			;	th->blocker_size = 10;

	add.w		#ENGINE_PLAYER_SIZE, d7				;	short block_offs = (th->blocker_size + ENGINE_PLAYER_SIZE) << ENGINE_SUBVERTEX_PRECISION;
	lsl.w		#ENGINE_SUBVERTEX_PRECISION, d7		;

	move.w		thing_xp(a6), d6					;	th->block_x1 = th->xp - block_offs;
	sub.w		d7, d6
	move.w		d6, thing_block_x1(a6)
	add.w		d7, d6								;	th->block_x2 = th->xp + block_offs;
	add.w		d7, d6
	move.w		d6, thing_block_x2(a6)

	move.w		thing_yp(a6), d6					;	th->block_y1 = th->yp - block_offs;
	sub.w		d7, d6
	move.w		d6, thing_block_y1(a6)
	add.w		d7, d6								;	th->block_y2 = th->yp + block_offs;
	add.w		d7, d6
	move.w		d6, thing_block_y2(a6)
	rts


; ================ Fn: collision_off ================

script_fn_collision_off_0:
	move.b		#0, thing_blocker_size(a6)			;	th->blocker_size = 0;
	rts


; ================ Fn: hitbox_on ================

script_fn_hitbox_on_0:
	move.w		thing_index(a6), d7
	add.w		#1, d7
	move.w		d7, thing_hitnum(a6)
	rts


; ================ Fn: hitbox_off ================

script_fn_hitbox_off_0:
	move.w		#0, thing_hitnum(a6)
	rts


; ================ Fn: pickup_on ================

script_fn_pickup_on_0:
	move.b		#1, thing_pickup(a6)
	rts


; ================ Fn: pickup_off ================

script_fn_pickup_off_0:
	move.b		#0, thing_pickup(a6)
	rts


; ================ Fn: player_hitscan ================

script_fn_player_hitscan_2:
	move.l		d0, -(a7)
	move.l		d1, -(a7)

	move.w		d6, -(a7)						;	save damage for later, we need free [d6]
	add.w		#80, d7
	cmp.w		#1, d7							;	clamp to 1..158
	bhs			.min_ok
	moveq.l		#1, d7
.min_ok:
	cmp.w		#158, d7
	bls			.max_ok
	move.w		#158, d7
.max_ok:
	lsl.w		#3, d7							;	dword index = render_columns.chunk2[pos+80].thing_num;
	lea.l		_render_columns+rc_thing_num, a0
	move.w		(a0, d7.w), d6
	bne			.hit_found
	move.w		-8(a0, d7.w), d6	;	try one pixel to the left
	bne			.hit_found_minus_1
	move.w		+8(a0, d7.w), d6	;	try one pixel to the right
	bne			.hit_found_plus_1
	
	move.w		d7, d0
	bra			.end_miss


.hit_found_minus_1:
	sub.w		#8, d7
	bra			.hit_found

.hit_found_plus_1:
	add.w		#8, d7
	bra			.hit_found


.hit_found:										;	if( index )
												;	{

	move.w		d7, d0

	cmp.w		_e_n_things, d6					;		if( index <= e_n_things )
	bhi			.end_miss						;		{
	move.w		d6, d7							;			EngineThing *th = e_things + (index-1);
	mulu.w		#thing_sizeof, d6				;
	move.l		_e_things, a0					;
	lea.l		-thing_sizeof(a0,d6.l), a0		;
	cmp.w		thing_hitnum(a0), d7			;			if( th->hitnum == (word)(index) )
	bne			.end_miss						;			{
	move.w		(a7)+, d6						;				th->damage += damage;
	add.w		d6, thing_damage(a0)			;

	move.l		_e_dmgthings_end, a1			;				[deferred] -> Script_OnDamage(th);
	cmp.l		_e_dmgthings_max, a1			;
	bhs			.end_hit						;	OnDamage can`t be scheduled, but the damage will roll over
	move.l		a0, (a1)+						;
	move.l		a1, _e_dmgthings_end			;
												;			}
												;		}
												;	}
	; --- fall through ---
.end_hit:										;	==== HIT ====
	jsr			_Engine_SpawnWeaponPuff
	move.l		(a7)+, d1
	move.l		(a7)+, d0
	rts

.end_miss:										;	==== MISS ====
	addq.l		#2, a7							;	pop unused [d6]
	move.l		#0, a0
	jsr			_Engine_SpawnWeaponPuff
	move.l		(a7)+, d1
	move.l		(a7)+, d0
	rts

; ================ Fn: player_damaged ================
script_fn_player_damaged_1:
	movem.l		d2-d7/a2-a6, -(a7)

	move.w		thing_xp(a6), gv_player_damage_source_x(a5)
	move.w		thing_yp(a6), gv_player_damage_source_y(a5)

	move.l		_view_player, a6
	move.l		thing_actor(a6), a4
	move.l		actor_cb_damage(a4), a3
	move.w		d7, thing_damage(a6)
	jsr			(a3)

	movem.l		(a7)+, d2-d7/a2-a6

	rts

_Script_PlayerDamaged:
	movem.l		d6-d7/a5,-(a7)

	lea.l		_e_globals, a5
	jsr			script_fn_player_damaged_1

	movem.l		(a7)+,d6-d7/a5
	rts


; ================ Fn: player_pulse ================
script_fn_player_pulse_1:
	movem.l		d2-d7/a2-a6, -(a7)

	move.l		_view_player, a6
	move.l		thing_actor(a6), a4
	move.l		actor_cb_pulse(a4), a3
	move.w		d7, gv_pulse(a5)
	jsr			(a3)

	movem.l		(a7)+, d2-d7/a2-a6

	rts


; ================ Fn: player_here ================
script_fn_player_here_0:
	move.w		_view_pos_x, thing_xp(a6)
	move.w		_view_pos_y, thing_yp(a6)
	move.w		#0, thing_zp(a6)				; TBD: player_here should update ZP as well
	rts

; ================ Fn: player_set_location ================
script_fn_player_set_location_1:
	move.l		d0, -(a7)
	move.l		d1, -(a7)
	jsr			_Player_JumpToLocation
	move.l		(a7)+, d1
	move.l		(a7)+, d0
	rts

; ================ Fn: player_apply_location_delta ================
script_fn_player_apply_location_delta_2:
	move.l		d0, -(a7)
	move.l		d1, -(a7)
	jsr			_Player_ApplyLocationDelta
	move.l		(a7)+, d1
	move.l		(a7)+, d0
	rts
	

; ================ Fn: vector_aim ================
script_fn_vector_aim_0:
	move.w		_view_pos_x, d7
	sub.w		thing_xp(a6), d7
	move.w		d7, thing_vector_x(a6)
	move.w		_view_pos_y, d7
	sub.w		thing_yp(a6), d7
	move.w		d7, thing_vector_y(a6)
	rts


; ================ Fn: vector_camera ================
script_fn_vector_camera_0:
	move.w		_view_rotate_dy, d0
	asr.w		#4, d0
	move.w		d0, thing_vector_x(a6)
	move.w		_view_rotate_dx, d0,
	asr.w		#4, d0
	move.w		d0, thing_vector_y(a6)
	rts
	

; ================ Fn: vector_align ================
;
;	vector_x` = ( vector_x*mwalk + vector_y*mside )>>8;
;	vector_y` = ( vector_y*mwalk - vector_x*mside )>>8;
;
script_fn_vector_align_2:
	move.l		d5, -(a7)
	move.w		d6, -(a7)
	move.w		d7, -(a7)

	muls.w		thing_vector_x(a6), d7
	muls.w		thing_vector_y(a6), d6
	add.l		d6, d7
	asr.l		#8, d7

	move.w		(a7)+, d6
	move.w		(a7)+, d5
	muls.w		thing_vector_y(a6), d6
	muls.w		thing_vector_x(a6), d5
	sub.l		d5, d6
	asr.l		#8, d6

	move.w		d7, thing_vector_x(a6)
	move.w		d6, thing_vector_y(a6)

	move.l		(a7)+, d5
	rts


; ================ Fn: vector_distance ================
script_fn_vector_distance_0:
	movem.l		d0-d1, -(a7)
	
	move.w		thing_vector_x(a6), d0
	muls.w		d0, d0
	move.w		thing_vector_y(a6), d1
	muls.w		d1, d1
	add.l		d1, d0
	jsr			lsqrt
	lsr.w		#ENGINE_SUBVERTEX_PRECISION, d0
	move.w		d0, thing_distance(a6)
	movem.l		(a7)+, d0-d1
	rts

; ================ Fn: vector_fastdist ================
script_fn_vector_fastdist_0:
	move.w		thing_vector_x(a6), d6
	bpl			.xpos
	neg.w		d6
.xpos:

	move.w		thing_vector_y(a6), d7
	bpl			.ypos
	neg.w		d7
.ypos:

	cmp.w		d7, d6
	bhi			.xgreater
	move.w		d7, d6
.xgreater:
	lsr.w		#ENGINE_SUBVERTEX_PRECISION, d6
	move.w		d6, thing_distance(a6)
	rts


; ================ Fn: vector_set_length ================
;	d0	- (temp)
;	d1	- (temp)
;	d5	- vector_x
;	d6	- vector_y
;	d7	- desired length (input)
;
script_fn_vector_set_length_1:
	movem.l		d0-d1/d5, -(a7)
	
	move.w		thing_vector_x(a6), d5					;	d5 = vector_x
	move.w		d5, d0									;	d0 = vector_x*vector_x
	muls.w		d0, d0									;
	move.w		thing_vector_y(a6), d6					;	d6 = vector_y
	move.w		d6, d1									;	d1 = vector_y*vector_y
	muls.w		d1, d1									;
	add.l		d1, d0									;	d0 = vector_x*vector_x + vector_y*vector_y
	jsr			lsqrt									;	d0 = length
	tst.w		d0
	beq			.end
	lsl.w		#ENGINE_SUBVERTEX_PRECISION, d7			;	d7 = new_length << 3		(make fixed point)
	muls.w		d7, d5									;
	divs.w		d0, d5									;
	move.w		d5, thing_vector_x(a6)					;	vector_x = d5 * d7 / d0
	muls.w		d7, d6									;
	divs.w		d0, d6									;
	move.w		d6, thing_vector_y(a6)					;	vector_y = d6 * d7 / d0

.end:
	movem.l		(a7)+, d0-d1/d5
	rts


; ================ Fn: vector_reaim ================
script_fn_vector_reaim_0:
	movem.l		d0-d1/d6-d7, -(a7)
	
	move.w		#256>>ENGINE_SUBVERTEX_PRECISION, d7	; normalize current vector to 256
	jsr			script_fn_vector_set_length_1

	move.w		_view_pos_x, d6							; aim to new position	(d6,d7)
	sub.w		thing_xp(a6), d6
	move.w		_view_pos_y, d7
	sub.w		thing_yp(a6), d7
	
	move.w		thing_vector_y(a6), d0					; compute distance
	muls.w		d6, d0
	move.w		thing_vector_x(a6), d1
	muls.w		d7, d1
	sub.l		d1, d0									; d0 = d6*vector_y - d7*vector_x
	bpl			.positive
	neg.l		d0
.positive:
	lsr.l		#8, d0
	lsr.l		#ENGINE_SUBVERTEX_PRECISION, d0
	move.w		d0, thing_distance(a6)					; save distance

	move.w		d6, thing_vector_x(a6)					; save new aim
	move.w		d7, thing_vector_y(a6)
	
	movem.l		(a7)+, d0-d1/d6-d7
	rts


; ================ Fn: vector_hitscan ================
script_fn_vector_hitscan_0:
	movem.l		d0-d1, -(a7)

	;	move.w		thing_xp(a6), d0			;	bspclip_p1x = startx;
	;	move.w		d0, _bspclip_p1x			;
	;	add.w		thing_vector_x(a6), d0		;	bspclip_p2x = endx;
	;	move.w		d0, _bspclip_p2x			;
	;	
	;	move.w		thing_yp(a6), d0			;	bspclip_p1y = starty;
	;	move.w		d0, _bspclip_p1y			;
	;	add.w		thing_vector_y(a6), d0		;	bspclip_p2y = endy;
	;	move.w		d0, _bspclip_p2y			;

	jsr			_Physics_Hitscan
	move.l		d0, d7

	movem.l		(a7)+, d0-d1
	rts


; ================ Fn: vector_set_timer ================
;	d7	- time delay (input)
;
script_fn_vector_set_timer_1:
	add.l		gv_levstat_time(a5), d7
	move.l		d7, thing_vector_x(a6)
	rts


; ================ Fn: move_walk ================
;	d7 - speed (units/s)
;	d6 - walk time
script_fn_move_walk_2:
	move.l		d0, -(a7)
	move.l		d1, -(a7)

	move.w		thing_vector_x(a6), thing_step_xp(a6)
	move.w		thing_vector_y(a6), thing_step_yp(a6)

	jsr			_Physics_EnemyWalkSlide					; clip step_xy
	
	jsr			_Physics_EnemyWalkInit

	move.l		(a7)+, d1
	move.l		(a7)+, d0
	rts

; ================ Fn: move_missile ================
;	d7 - speed (units/s)
;	d6 - walk time
script_fn_move_missile_2:
	move.l		d0, -(a7)
	move.l		d1, -(a7)

	move.w		thing_vector_x(a6), thing_step_xp(a6)
	move.w		thing_vector_y(a6), thing_step_yp(a6)

	jsr			_Physics_MissileMoveClip				; clip step_xy
	
	jsr			_Physics_EnemyWalkInit

	move.l		(a7)+, d1
	move.l		(a7)+, d0
	rts

; ================ Fn: move_stop ================
script_fn_move_stop_0:
	move.w		#0, thing_step_count(a6)
	rts


; ================ Fn: destroyed ================
script_fn_destroyed_0:
	move.l		#0, d0
	move.l		d0, thing_script_fn(a6)
	move.l		d0, thing_sprite(a6)
	move.w		d0, thing_hitnum(a6)
	move.b		d0, thing_blocker_size(a6)
	move.w		d0, thing_step_count(a6)
	move.l		d0, a0
	jsr			_Engine_ThingSetSubsector
	
	move.l		_e_delthings_end, a0					; store thing pointer for later reuse
	move.l		a6, (a0)+
	move.l		a0, _e_delthings_end

	rts


; ================ Fn: spawn_missile ================
script_fn_spawn_missile_0:
	move.l		d0, -(a7)
	move.l		d1, -(a7)
	jsr			_Engine_SpawnMissile
	move.l		(a7)+, d1
	move.l		(a7)+, d0
	rts


; ================ Fn: explosion_damage ================
script_fn_explosion_damage_1:
	move.l		d0, -(a7)
	move.l		d1, -(a7)
	jsr			_Engine_ExplosionDamage
	move.l		(a7)+, d1
	move.l		(a7)+, d0
	rts


; ================ Fn: machine_line_scroll ================
script_fn_machine_line_scroll_1:
	move.l		d0, -(a7)
	move.l		d1, -(a7)
	jsr			_Machine_LineScroll
	move.l		(a7)+, d1
	move.l		(a7)+, d0
	rts


; ================ Fn: script_fn_mp_player_damage_1 ================
script_fn_mp_player_damage_1:
  if MULTIPLAYER_MODE
	move.l		d0, -(a7)
	move.l		d1, -(a7)
	moveq.l		#$02, d0
	jsr			_Comm_Write_Byte
	move.w		d7, d0
	jsr			_Comm_Write_Word
	move.l		(a7)+, d1
	move.l		(a7)+, d0
  endif
	rts


; ================ Fn: script_fn_mp_pulse_avatar_1 ================
script_fn_mp_pulse_avatar_1:
  if MULTIPLAYER_MODE
	move.l		d0, -(a7)
	move.l		d1, -(a7)
	moveq.l		#$04, d0
	jsr			_Comm_Write_Byte
	move.w		d7, d0
	jsr			_Comm_Write_Word
	move.l		(a7)+, d1
	move.l		(a7)+, d0
  endif
	rts


; ================ Fn: script_fn_mp_pulse_remote_copy_1 ================
script_fn_mp_pulse_remote_copy_1:
  if MULTIPLAYER_MODE
	move.l		d0, -(a7)
	move.l		d1, -(a7)
	moveq.l		#$03, d0
	jsr			_Comm_Write_Byte
	move.w		thing_index(a6), d0
	jsr			_Comm_Write_Word
	move.w		d7, d0
	jsr			_Comm_Write_Word
	move.l		(a7)+, d1
	move.l		(a7)+, d0
  endif
	rts


; ================ Fn: check_blockers_radius ================
script_fn_check_blockers_radius_0:
	move.l		d0, -(a7)
	move.l		d1, -(a7)
	move.w		thing_distance(a6), d7
	jsr			_Engine_CheckRadiusBlocked
	move.w		d0, d7
	move.l		(a7)+, d1
	move.l		(a7)+, d0
	rts


	; ================ Fn: tag_check ================
script_fn_tag_check_1:
	move.l		d0, -(a7)
	move.l		d1, -(a7)
	move.l		12(a7), d0
	jsr			_Game_TagCheck
	move.l		d0, 12(a7)
	move.l		(a7)+, d1
	move.l		(a7)+, d0
	rts
