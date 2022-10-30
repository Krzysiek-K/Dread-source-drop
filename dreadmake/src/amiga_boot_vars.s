

; ================================================================ Globals ================================================================


Bootloader_Vars:
Print_Vars:
Track_Vars:

.prn_target_pointer			dc.l		0
.prn_target_subbyte			dc.w		8

.trk_buffer_address			dc.l		0			;	Track buffer address - unused by the routine itself
.trk_decoded_address		dc.l		0			;	Decoded track buffer address - unused by the routine itself
.trk_req_trackhead			dc.w		$FFFF		;	Track/head number to load 
.trk_req_destination		dc.l		0			;	Load destination address
.trk_current_track			dc.w		0			;	The track head currently is over
.trk_current_head			dc.w		2			;	Currently selected head (0 or 1)
.trk_dma_trackhead			dc.w		$FFFF		;	Track/head number DMA was activated for, or $FFFF if DMA not active
.trk_loaded_trackhead		dc.w		$FFFF		;	Recently loaded track/head number

.load_index					dc.w		0			; Arguments
.load_offset				dc.l		0
.load_flags					dc.w		0
.load_trackhead				dc.w		0
.load_startsec				dc.w		0
.load_numsec				dc.w		0
.load_crc					dc.l		0
.load_final_dst:			dc.l		0			; Locals
.load_progress_pixels:		dc.w		$FFFF
.load_total_sectors:		dc.w		1
.load_loaded_sectors:		dc.w		0
.load_decoded_track:		dc.l		0



prn_target_pointer			= .prn_target_pointer		- Print_Vars
prn_target_subbyte			= .prn_target_subbyte		- Print_Vars

trk_buffer_address			= .trk_buffer_address		- Track_Vars
trk_decoded_address			= .trk_decoded_address		- Track_Vars
trk_req_trackhead			= .trk_req_trackhead		- Track_Vars
trk_req_destination			= .trk_req_destination		- Track_Vars
trk_current_track			= .trk_current_track		- Track_Vars
trk_current_head			= .trk_current_head			- Track_Vars
trk_dma_trackhead			= .trk_dma_trackhead		- Track_Vars
trk_loaded_trackhead		= .trk_loaded_trackhead		- Track_Vars

load_index					= .load_index				- Bootloader_Vars
load_offset					= .load_offset				- Bootloader_Vars
load_flags					= .load_flags				- Bootloader_Vars
load_trackhead				= .load_trackhead			- Bootloader_Vars
load_startsec				= .load_startsec			- Bootloader_Vars
load_numsec					= .load_numsec				- Bootloader_Vars
load_crc					= .load_crc					- Bootloader_Vars
load_final_dst				= .load_final_dst			- Bootloader_Vars
load_progress_pixels		= .load_progress_pixels		- Bootloader_Vars
load_total_sectors			= .load_total_sectors		- Bootloader_Vars
load_loaded_sectors			= .load_loaded_sectors		- Bootloader_Vars
load_decoded_track			= .load_decoded_track		- Bootloader_Vars
