
; struct node
node_A                  equ   0
node_B                  equ   2
node_C                  equ   4
node_left               equ   8
node_right              equ  10
node_sizeof             equ  12

; struct actor
actor_num               equ   0
actor_cb_create         equ   2
actor_cb_damage         equ   6
actor_cb_pulse          equ  10
actor_sizeof            equ  14

; struct st_wpinfo
st_wpinfo_data          equ   0
st_wpinfo_screen_start  equ   4
st_wpinfo_width         equ   6
st_wpinfo_height        equ   8
st_wpinfo_sizeof        equ  10

; struct weaponanim
weaponanim_delay        equ   0
weaponanim_gfx          equ   2
weaponanim_xpos         equ   6
weaponanim_ypos         equ   8
weaponanim_flags        equ  10
weaponanim_sizeof       equ  12

; struct vtx
vtx_xp                  equ   0
vtx_yp                  equ   2
vtx_tr_x                equ   4
vtx_tr_y                equ   6
vtx_sizeof              equ   8

; struct line
line_v1                 equ   0
line_v2                 equ   4
line_tex_upper          equ   8
line_xcoord1            equ  12
line_xcoord2            equ  14
line_ceil_col           equ  16
line_floor_col          equ  17
line_modes              equ  18
line_tex_lower          equ  20
line_y1                 equ  24
line_y2                 equ  26
line_y3                 equ  28
line_y4                 equ  30
line_sizeof             equ  32

; struct subsector
subsector_vis           equ   0
subsector_first_thing   equ   4
subsector_visframe      equ   8
subsector_type          equ  10
subsector_height        equ  12
subsector_sizeof        equ  14

; struct thing
thing_ss_next           equ   0
thing_ss_prev           equ   4
thing_subsector         equ   8
thing_visible           equ  12
thing_blocker_size      equ  13
thing_pickup            equ  14
thing_gravity           equ  15
thing_move_collide      equ  16
thing__pad              equ  17
thing_block_x1          equ  18
thing_block_y1          equ  20
thing_block_x2          equ  22
thing_block_y2          equ  24
thing_actor             equ  26
thing_sprite            equ  30
thing_xp                equ  34
thing_yp                equ  36
thing_zp                equ  38
thing_tr_x              equ  40
thing_tr_y              equ  42
thing_vel_z             equ  44
thing_unstuck_x         equ  46
thing_unstuck_y         equ  48
thing_hitnum            equ  50
thing_index             equ  52
thing_machine_id        equ  54
thing_machine_position  equ  56
thing_machine_speed     equ  58
thing_owner             equ  60
thing_script_fn         equ  64
thing_timer_delay       equ  68
thing_timer_attack      equ  70
thing_timer_walk        equ  72
thing_health            equ  74
thing_damage            equ  76
thing_vector_x          equ  78
thing_vector_y          equ  80
thing_distance          equ  82
thing_step_xp           equ  84
thing_step_yp           equ  86
thing_step_count        equ  88
thing_step_endx         equ  90
thing_step_endy         equ  92
thing_state             equ  94
thing_value             equ  96
thing_value2            equ  98
thing_value3            equ 100
thing_value4            equ 102
thing_sizeof            equ 104

; struct gv
gv_randpos              equ   0
gv_io_mouse_state       equ   2
gv_skill                equ   4
gv_pulse                equ   6
gv_player_ammo          equ   8
gv_player_health        equ  10
gv_player_armor         equ  12
gv_player_bullets       equ  14
gv_player_shells        equ  16
gv_player_rockets       equ  18
gv_player_cells         equ  20
gv_player_bullets_cap   equ  22
gv_player_shells_cap    equ  24
gv_player_rockets_cap   equ  26
gv_player_cells_cap     equ  28
gv_player_red_glow      equ  30
gv_player_pal_flash     equ  32
gv_player_face          equ  34
gv_weapon_send_pulse    equ  36
gv_statbar_key_0        equ  38
gv_statbar_key_1        equ  40
gv_statbar_key_2        equ  42
gv_statbar_weapon_0     equ  44
gv_statbar_weapon_1     equ  46
gv_statbar_weapon_2     equ  48
gv_statbar_weapon_3     equ  50
gv_statbar_weapon_4     equ  52
gv_statbar_weapon_5     equ  54
gv_update_conditions    equ  56
gv_temp1                equ  58
gv_temp2                equ  60
gv_temp3                equ  62
gv_levstat_monsters     equ  64
gv_levstat_monsters_max equ  66
gv_levstat_items        equ  68
gv_levstat_items_max    equ  70
gv_levstat_secrets      equ  72
gv_levstat_secrets_max  equ  74
gv_levstat_time         equ  76
gv_levstat_secrets_done equ  80
gv_game_state           equ  84
gv_cheat_code_pos       equ  86
gv_cheat_flags          equ  87
gv_debug_var            equ  88
gv_debug_var2           equ  90
gv_debug_var3           equ  92
gv_debug_var4           equ  94
gv_player_damage_source_x equ  96
gv_player_damage_source_y equ  98
gv_player_floor_timer   equ 100
gv_player_sector_type   equ 102
gv_player_sector_tag    equ 104
gv_player_boots_timer   equ 106
gv_switch_flags         equ 108
gv_special_monsters_blue_key equ 112
gv_special_monsters_yellow_key equ 113
gv_special_monsters_red_key equ 114
gv_mp_mode              equ 115
gv_mp_commstate         equ 116
gv_mp_spawnresponse     equ 117
gv_mp_deaths            equ 118
gv_mp_frags             equ 119
gv_sizeof               equ 120

; struct lc
lc__pad                 equ   0
lc_ceil_len_m64         equ   2
lc_ceil_len_m128        equ   4
lc_upper_start_m64      equ   6
lc_upper_start_m128     equ  10
lc_upper_len_m64_0      equ  14
lc_upper_len_m128_0     equ  16
lc_upper_len_m128_m64   equ  18
lc_floor_start_0        equ  20
lc_hole_size_m64_0      equ  22
lc_ceil_ymin_m64        equ  24
lc_floor_length_0       equ  26
lc_real_size            equ  28
lc_scaler_fn            equ  30
lc_upper_endpos_0       equ  34
lc_upper_endpos_m64     equ  36
lc_upper_len_m96_0      equ  38
lc_upper_start_m96      equ  40
lc_ceil_len_m96         equ  44
lc_upper_len_m96_m64    equ  46
lc_upper_endpos_m96     equ  48
lc_hole_size_m96_0      equ  50
lc_ceil_ymin_m96        equ  52
lc_upper_len_m128_m96   equ  54
lc_hole_size_m128_0     equ  56
lc_ceil_ymin_m128       equ  58
lc_hole_end_0           equ  60
lc_sizeof               equ  62
