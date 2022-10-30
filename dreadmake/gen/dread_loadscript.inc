

; ================================================================ HEADER ================================================================
;
		public _memfile_START
_memfile_START:
																								; offset $0000

; ================================================================ DreadLoadScript ================================================================
;
		public _data_block_DreadLoadScript
_data_block_DreadLoadScript:
		dc.w	$0017, $01DF																	; BCMD_MACRO_TOTAL_SECTORS
		; 
		; ================================ Allocate segments ================================
		; 
		dc.w	$0003, $0000																	; BCMD_ALLOC_FAST
		dc.l	$0000C200
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$5345, $4354, $494F, $4E20, $3020, $434F, $4445, $2041, $5420, $2400			; "SECTION 0 CODE AT $"
		dc.w	$0010, $0000																	; BCMD_PRINT_ADDR
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$2053, $495A, $4520, $2443, $3230, $3000										; " SIZE $C200"
		dc.w	$0003, $0001																	; BCMD_ALLOC_FAST
		dc.l	$00039C00
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$5345, $4354, $494F, $4E20, $3120, $434F, $4445, $2041, $5420, $2400			; "SECTION 1 CODE AT $"
		dc.w	$0010, $0001																	; BCMD_PRINT_ADDR
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$2053, $495A, $4520, $2433, $3943, $3030, $0000									; " SIZE $39C00"
		dc.w	$0003, $0002																	; BCMD_ALLOC_FAST
		dc.l	$0000C600
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$5345, $4354, $494F, $4E20, $3220, $4154, $2024, $0000							; "SECTION 2 AT $"
		dc.w	$0010, $0002																	; BCMD_PRINT_ADDR
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$2053, $495A, $4520, $2443, $3630, $3000										; " SIZE $C600"
		dc.w	$0001, $0003																	; BCMD_ALLOC_CHIP
		dc.l	$00000200
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$5345, $4354, $494F, $4E20, $3320, $4348, $4950, $2041, $5420, $2400			; "SECTION 3 CHIP AT $"
		dc.w	$0010, $0003																	; BCMD_PRINT_ADDR
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$2053, $495A, $4520, $2432, $3030, $0000										; " SIZE $200"
		dc.w	$0003, $0004																	; BCMD_ALLOC_FAST
		dc.l	$00000600
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$5345, $4354, $494F, $4E20, $3420, $4154, $2024, $0000							; "SECTION 4 AT $"
		dc.w	$0010, $0004																	; BCMD_PRINT_ADDR
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$2053, $495A, $4520, $2436, $3030, $0000										; " SIZE $600"
		dc.w	$0001, $0005																	; BCMD_ALLOC_CHIP
		dc.l	$0001A200
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$5345, $4354, $494F, $4E20, $3520, $4348, $4950, $2041, $5420, $2400			; "SECTION 5 CHIP AT $"
		dc.w	$0010, $0005																	; BCMD_PRINT_ADDR
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$2053, $495A, $4520, $2431, $4132, $3030, $0000									; " SIZE $1A200"
		dc.w	$0003, $0006																	; BCMD_ALLOC_FAST
		dc.l	$00000200
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$5345, $4354, $494F, $4E20, $3620, $4253, $5320, $4154, $2024, $0000			; "SECTION 6 BSS AT $"
		dc.w	$0010, $0006																	; BCMD_PRINT_ADDR
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$2053, $495A, $4520, $2432, $3030, $0000										; " SIZE $200"
		dc.w	$0003, $0007																	; BCMD_ALLOC_FAST
		dc.l	$0001F800
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$5345, $4354, $494F, $4E20, $3720, $4253, $5320, $4154, $2024, $0000			; "SECTION 7 BSS AT $"
		dc.w	$0010, $0007																	; BCMD_PRINT_ADDR
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$2053, $495A, $4520, $2431, $4638, $3030, $0000									; " SIZE $1F800"
		dc.w	$0001, $0008																	; BCMD_ALLOC_CHIP
		dc.l	$0000D200
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$5345, $4354, $494F, $4E20, $3820, $4348, $4950, $2042, $5353, $2041, $5420, $2400 ; "SECTION 8 CHIP BSS AT $"
		dc.w	$0010, $0008																	; BCMD_PRINT_ADDR
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$2053, $495A, $4520, $2444, $3230, $3000										; " SIZE $D200"
		; 
		; ================================ Load segment 0 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5345, $4720, $3000									; "LOADING SEG 0"
		dc.w	$0018, $0000																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0007, $0006, $0009, $0002
		dc.l	$282194B0
		dc.w	$0018, $0000																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$000005F0
		dc.w	$0007, $0007, $0000, $000B
		dc.l	$64495FAE
		dc.w	$0018, $0000																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00004EC6
		dc.w	$0007, $0008, $0000, $000B
		dc.l	$9EE98288
		dc.w	$0018, $0000																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00009BE8
		dc.w	$0003, $0009, $0000, $0009
		dc.l	$71718E72
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0000																	; BCMD_PRINT_SECTION_CRC
		dc.l	$0000C138
		; 
		; ================================ Relocations 0 -> 0 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5245, $4C4F, $4320, $302D, $3E30, $0000			; "LOADING RELOC 0->0"
		dc.w	$0018, $0007																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0007, $0009, $0009, $0002
		dc.l	$347B414D
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0007																	; BCMD_PRINT_SECTION_CRC
		dc.l	$000004A8
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2052, $454C, $4F43, $4154, $494E, $4700									; " - RELOCATING"
		dc.w	$000B, $0000, $0000, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$012A
		; 
		; ================================ Relocations 0 -> 1 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5245, $4C4F, $4320, $302D, $3E31, $0000			; "LOADING RELOC 0->1"
		dc.w	$0018, $0007																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0003, $000A, $0000, $0002
		dc.l	$81D20CF3
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0007																	; BCMD_PRINT_SECTION_CRC
		dc.l	$000005C8
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2052, $454C, $4F43, $4154, $494E, $4700									; " - RELOCATING"
		dc.w	$000B, $0000, $0001, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0172
		; 
		; ================================ Relocations 0 -> 4 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5245, $4C4F, $4320, $302D, $3E34, $0000			; "LOADING RELOC 0->4"
		dc.w	$0018, $0007																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0003, $000A, $0002, $0001
		dc.l	$417E6F28
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0007																	; BCMD_PRINT_SECTION_CRC
		dc.l	$00000030
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2052, $454C, $4F43, $4154, $494E, $4700									; " - RELOCATING"
		dc.w	$000B, $0000, $0004, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$000C
		; 
		; ================================ Relocations 0 -> 6 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5245, $4C4F, $4320, $302D, $3E36, $0000			; "LOADING RELOC 0->6"
		dc.w	$0018, $0007																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0003, $000A, $0003, $0001
		dc.l	$0CF5018E
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0007																	; BCMD_PRINT_SECTION_CRC
		dc.l	$00000028
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2052, $454C, $4F43, $4154, $494E, $4700									; " - RELOCATING"
		dc.w	$000B, $0000, $0006, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$000A
		; 
		; ================================ Relocations 0 -> 7 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5245, $4C4F, $4320, $302D, $3E37, $0000			; "LOADING RELOC 0->7"
		dc.w	$0018, $0007																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0003, $000A, $0004, $0003
		dc.l	$9F69D396
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0007																	; BCMD_PRINT_SECTION_CRC
		dc.l	$00000920
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2052, $454C, $4F43, $4154, $494E, $4700									; " - RELOCATING"
		dc.w	$000B, $0000, $0007, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0248
		; 
		; ================================ Load segment 1 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5345, $4720, $3100									; "LOADING SEG 1"
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0007, $000A, $0007, $0004
		dc.l	$A28B55D0
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00001058
		dc.w	$0007, $000B, $0000, $000B
		dc.l	$0451864F
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00004215
		dc.w	$0007, $000C, $0000, $000B
		dc.l	$F0C5C20A
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$0000725E
		dc.w	$0007, $000D, $0000, $000B
		dc.l	$62E0B40A
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$0000A200
		dc.w	$0007, $000E, $0000, $000B
		dc.l	$9D5ACBFA
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$0000E00C
		dc.w	$0007, $000F, $0000, $000B
		dc.l	$8A3DC545
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$0001124F
		dc.w	$0007, $0010, $0000, $000B
		dc.l	$84ADF829
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00013F48
		dc.w	$0007, $0011, $0000, $000B
		dc.l	$D50CE244
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00017171
		dc.w	$0007, $0012, $0000, $000B
		dc.l	$E1F2B5C4
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$0001A0B5
		dc.w	$0007, $0013, $0000, $000B
		dc.l	$1CE614DF
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$0001CFD4
		dc.w	$0007, $0014, $0000, $000B
		dc.l	$EA7CAD03
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$0001F989
		dc.w	$0007, $0015, $0000, $000B
		dc.l	$2BFED570
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00021FB4
		dc.w	$0007, $0016, $0000, $000B
		dc.l	$6CDA7AA2
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00024B1B
		dc.w	$0007, $0017, $0000, $000B
		dc.l	$1C80EDE1
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$000279AF
		dc.w	$0007, $0018, $0000, $000B
		dc.l	$4B747A34
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00029D7A
		dc.w	$0007, $0019, $0000, $000B
		dc.l	$7029C577
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$0002D253
		dc.w	$0007, $001A, $0000, $000B
		dc.l	$1D9D6B26
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00030666
		dc.w	$0007, $001B, $0000, $000B
		dc.l	$C1FA73CB
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00032FB4
		dc.w	$0007, $001C, $0000, $000B
		dc.l	$2940936A
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$000350E5
		dc.w	$0007, $001D, $0000, $000B
		dc.l	$FE35857E
		dc.w	$0018, $0001																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$000374F3
		dc.w	$0003, $001E, $0000, $000A
		dc.l	$46F2CF91
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0001																	; BCMD_PRINT_SECTION_CRC
		dc.l	$00039BEC
		; 
		; ================================ Relocations 1 -> 0 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5245, $4C4F, $4320, $312D, $3E30, $0000			; "LOADING RELOC 1->0"
		dc.w	$0018, $0007																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0007, $001E, $000A, $0001
		dc.l	$93EF8F2A
		dc.w	$0018, $0007																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$000002FF
		dc.w	$0003, $001F, $0000, $0001
		dc.l	$261CECD1
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0007																	; BCMD_PRINT_SECTION_CRC
		dc.l	$00000434
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2052, $454C, $4F43, $4154, $494E, $4700									; " - RELOCATING"
		dc.w	$000B, $0001, $0000, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$010D
		; 
		; ================================ Relocations 1 -> 1 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5245, $4C4F, $4320, $312D, $3E31, $0000			; "LOADING RELOC 1->1"
		dc.w	$0018, $0007																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0003, $001F, $0001, $0004
		dc.l	$66BBDC2B
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0007																	; BCMD_PRINT_SECTION_CRC
		dc.l	$00000BA8
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2052, $454C, $4F43, $4154, $494E, $4700									; " - RELOCATING"
		dc.w	$000B, $0001, $0001, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$02EA
		; 
		; ================================ Relocations 1 -> 2 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5245, $4C4F, $4320, $312D, $3E32, $0000			; "LOADING RELOC 1->2"
		dc.w	$0018, $0007																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0003, $001F, $0005, $0001
		dc.l	$83A02050
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0007																	; BCMD_PRINT_SECTION_CRC
		dc.l	$00000008
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2052, $454C, $4F43, $4154, $494E, $4700									; " - RELOCATING"
		dc.w	$000B, $0001, $0002, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0002
		; 
		; ================================ Relocations 1 -> 3 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5245, $4C4F, $4320, $312D, $3E33, $0000			; "LOADING RELOC 1->3"
		dc.w	$0018, $0007																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0003, $001F, $0006, $0001
		dc.l	$B50DA505
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0007																	; BCMD_PRINT_SECTION_CRC
		dc.l	$00000100
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2052, $454C, $4F43, $4154, $494E, $4700									; " - RELOCATING"
		dc.w	$000B, $0001, $0003, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0040
		; 
		; ================================ Relocations 1 -> 4 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5245, $4C4F, $4320, $312D, $3E34, $0000			; "LOADING RELOC 1->4"
		dc.w	$0018, $0007																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0003, $001F, $0007, $0002
		dc.l	$18089559
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0007																	; BCMD_PRINT_SECTION_CRC
		dc.l	$00000480
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2052, $454C, $4F43, $4154, $494E, $4700									; " - RELOCATING"
		dc.w	$000B, $0001, $0004, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0120
		; 
		; ================================ Relocations 1 -> 5 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5245, $4C4F, $4320, $312D, $3E35, $0000			; "LOADING RELOC 1->5"
		dc.w	$0018, $0007																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0007, $001F, $0009, $0002
		dc.l	$F1E370E1
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0007																	; BCMD_PRINT_SECTION_CRC
		dc.l	$000004F0
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2052, $454C, $4F43, $4154, $494E, $4700									; " - RELOCATING"
		dc.w	$000B, $0001, $0005, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$013C
		; 
		; ================================ Relocations 1 -> 7 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5245, $4C4F, $4320, $312D, $3E37, $0000			; "LOADING RELOC 1->7"
		dc.w	$0018, $0007																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0007, $0020, $0000, $000B
		dc.l	$A6E02CFE
		dc.w	$0018, $0007																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00002302
		dc.w	$0003, $0021, $0000, $0004
		dc.l	$13CAF4D0
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0007																	; BCMD_PRINT_SECTION_CRC
		dc.l	$00002F0C
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2052, $454C, $4F43, $4154, $494E, $4700									; " - RELOCATING"
		dc.w	$000B, $0001, $0007, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0BC3
		; 
		; ================================ Relocations 1 -> 8 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5245, $4C4F, $4320, $312D, $3E38, $0000			; "LOADING RELOC 1->8"
		dc.w	$0018, $0007																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0003, $0021, $0004, $0001
		dc.l	$D3AB02EC
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0007																	; BCMD_PRINT_SECTION_CRC
		dc.l	$00000124
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2052, $454C, $4F43, $4154, $494E, $4700									; " - RELOCATING"
		dc.w	$000B, $0001, $0008, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0049
		; 
		; ================================ Load segment 2 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5345, $4720, $3200									; "LOADING SEG 2"
		dc.w	$0018, $0002																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0007, $0021, $0005, $0006
		dc.l	$5A02FC07
		dc.w	$0018, $0002																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00001826
		dc.w	$0007, $0022, $0000, $000B
		dc.l	$8A950D2D
		dc.w	$0018, $0002																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00004D39
		dc.w	$0007, $0023, $0000, $000B
		dc.l	$A239D718
		dc.w	$0018, $0002																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$000080D7
		dc.w	$0007, $0024, $0000, $000B
		dc.l	$39AC22C9
		dc.w	$0018, $0002																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$0000A8DC
		dc.w	$0003, $0025, $0000, $0009
		dc.l	$EB4F998D
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0002																	; BCMD_PRINT_SECTION_CRC
		dc.l	$0000C4F8
		; 
		; ================================ Load segment 3 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5345, $4720, $3300									; "LOADING SEG 3"
		dc.w	$0018, $0003																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0003, $0025, $0009, $0001
		dc.l	$31B03230
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0003																	; BCMD_PRINT_SECTION_CRC
		dc.l	$00000004
		; 
		; ================================ Load segment 4 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5345, $4720, $3400									; "LOADING SEG 4"
		dc.w	$0018, $0004																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0007, $0025, $000A, $0001
		dc.l	$CBEA6DF7
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0004																	; BCMD_PRINT_SECTION_CRC
		dc.l	$00000458
		; 
		; ================================ Relocations 4 -> 0 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5245, $4C4F, $4320, $342D, $3E30, $0000			; "LOADING RELOC 4->0"
		dc.w	$0018, $0007																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0003, $0026, $0000, $0001
		dc.l	$5A1D7678
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0007																	; BCMD_PRINT_SECTION_CRC
		dc.l	$0000006C
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2052, $454C, $4F43, $4154, $494E, $4700									; " - RELOCATING"
		dc.w	$000B, $0004, $0000, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$001B
		; 
		; ================================ Relocations 4 -> 1 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5245, $4C4F, $4320, $342D, $3E31, $0000			; "LOADING RELOC 4->1"
		dc.w	$0018, $0007																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0003, $0026, $0001, $0001
		dc.l	$1EC277E8
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0007																	; BCMD_PRINT_SECTION_CRC
		dc.l	$0000004C
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2052, $454C, $4F43, $4154, $494E, $4700									; " - RELOCATING"
		dc.w	$000B, $0004, $0001, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0013
		; 
		; ================================ Relocations 4 -> 5 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5245, $4C4F, $4320, $342D, $3E35, $0000			; "LOADING RELOC 4->5"
		dc.w	$0018, $0007																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0003, $0026, $0002, $0001
		dc.l	$09BCB8E3
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0007																	; BCMD_PRINT_SECTION_CRC
		dc.l	$00000194
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2052, $454C, $4F43, $4154, $494E, $4700									; " - RELOCATING"
		dc.w	$000B, $0004, $0005, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0065
		; 
		; ================================ Relocations 4 -> 8 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5245, $4C4F, $4320, $342D, $3E38, $0000			; "LOADING RELOC 4->8"
		dc.w	$0018, $0007																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0003, $0026, $0003, $0001
		dc.l	$69C8481C
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0007																	; BCMD_PRINT_SECTION_CRC
		dc.l	$00000008
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2052, $454C, $4F43, $4154, $494E, $4700									; " - RELOCATING"
		dc.w	$000B, $0004, $0008, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0002
		; 
		; ================================ Load segment 5 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$4C4F, $4144, $494E, $4720, $5345, $4720, $3500									; "LOADING SEG 5"
		dc.w	$0018, $0005																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00000000
		dc.w	$0007, $0026, $0004, $0007
		dc.l	$4F029E2E
		dc.w	$0018, $0005																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00002CCA
		dc.w	$0007, $0027, $0000, $000B
		dc.l	$9F3F034B
		dc.w	$0018, $0005																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$000054A5
		dc.w	$0007, $0028, $0000, $000B
		dc.l	$8F359DA5
		dc.w	$0018, $0005																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00007A07
		dc.w	$0007, $0029, $0000, $000B
		dc.l	$6A004E95
		dc.w	$0018, $0005																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$000098AD
		dc.w	$0007, $002A, $0000, $000B
		dc.l	$6AF4B9F3
		dc.w	$0018, $0005																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$0000D8DE
		dc.w	$0007, $002B, $0000, $000B
		dc.l	$933888E5
		dc.w	$0018, $0005																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$0000FEA6
		dc.w	$0007, $002C, $0000, $000B
		dc.l	$24592CAD
		dc.w	$0018, $0005																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$000117BB
		dc.w	$0007, $002D, $0000, $000B
		dc.l	$35FEEA2D
		dc.w	$0018, $0005																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$0001301E
		dc.w	$0007, $002E, $0000, $000B
		dc.l	$865A7B7F
		dc.w	$0018, $0005																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00014BBC
		dc.w	$0007, $002F, $0000, $000B
		dc.l	$3410EC63
		dc.w	$0018, $0005																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$000163EF
		dc.w	$0007, $0030, $0000, $000B
		dc.l	$2CEDE80B
		dc.w	$0018, $0005																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$00017F24
		dc.w	$0007, $0031, $0000, $000B
		dc.l	$EE4DCDF2
		dc.w	$0018, $0005																	; BCMD_MACRO_TRACK_LOAD
		dc.l	$000197CA
		dc.w	$0003, $0032, $0000, $0004
		dc.l	$0782E43F
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$202D, $2043, $5243, $2024, $0000												; " - CRC $"
		dc.w	$0014, $0005																	; BCMD_PRINT_SECTION_CRC
		dc.l	$0001A09C
		; 
		; ================================ Load segment 6 ================================
		; 
		; 
		; ================================ Load segment 7 ================================
		; 
		; 
		; ================================ Load segment 8 ================================
		; 
		; 
		; ================================ Clear BSS 6 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$434C, $4541, $5249, $4E47, $2042, $5353, $2036, $0000							; "CLEARING BSS 6"
		dc.w	$000A, $0006																	; BCMD_FILL
		dc.l	$00000000, $00000184
		dc.w	$0000
		; 
		; ================================ Clear BSS 7 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$434C, $4541, $5249, $4E47, $2042, $5353, $2037, $0000							; "CLEARING BSS 7"
		dc.w	$000A, $0007																	; BCMD_FILL
		dc.l	$00000000, $0001F634
		dc.w	$0000
		; 
		; ================================ Clear BSS 8 ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$434C, $4541, $5249, $4E47, $2042, $5353, $2038, $0000							; "CLEARING BSS 8"
		dc.w	$000A, $0008																	; BCMD_FILL
		dc.l	$00000000, $0000D078
		dc.w	$0000
		; 
		; ================================ Print final info ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$5345, $4354, $494F, $4E20, $3020, $434F, $4445, $2041, $5420, $2400			; "SECTION 0 CODE AT $"
		dc.w	$0010, $0000																	; BCMD_PRINT_ADDR
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$2043, $5243, $2024, $0000														; " CRC $"
		dc.w	$0014, $0000																	; BCMD_PRINT_SECTION_CRC
		dc.l	$0000C138
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$5345, $4354, $494F, $4E20, $3120, $434F, $4445, $2041, $5420, $2400			; "SECTION 1 CODE AT $"
		dc.w	$0010, $0001																	; BCMD_PRINT_ADDR
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$2043, $5243, $2024, $0000														; " CRC $"
		dc.w	$0014, $0001																	; BCMD_PRINT_SECTION_CRC
		dc.l	$00039BEC
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$5345, $4354, $494F, $4E20, $3220, $4154, $2024, $0000							; "SECTION 2 AT $"
		dc.w	$0010, $0002																	; BCMD_PRINT_ADDR
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$2043, $5243, $2024, $0000														; " CRC $"
		dc.w	$0014, $0002																	; BCMD_PRINT_SECTION_CRC
		dc.l	$0000C4F8
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$5345, $4354, $494F, $4E20, $3320, $4348, $4950, $2041, $5420, $2400			; "SECTION 3 CHIP AT $"
		dc.w	$0010, $0003																	; BCMD_PRINT_ADDR
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$2043, $5243, $2024, $0000														; " CRC $"
		dc.w	$0014, $0003																	; BCMD_PRINT_SECTION_CRC
		dc.l	$00000004
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$5345, $4354, $494F, $4E20, $3420, $4154, $2024, $0000							; "SECTION 4 AT $"
		dc.w	$0010, $0004																	; BCMD_PRINT_ADDR
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$2043, $5243, $2024, $0000														; " CRC $"
		dc.w	$0014, $0004																	; BCMD_PRINT_SECTION_CRC
		dc.l	$00000458
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$5345, $4354, $494F, $4E20, $3520, $4348, $4950, $2041, $5420, $2400			; "SECTION 5 CHIP AT $"
		dc.w	$0010, $0005																	; BCMD_PRINT_ADDR
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$2043, $5243, $2024, $0000														; " CRC $"
		dc.w	$0014, $0005																	; BCMD_PRINT_SECTION_CRC
		dc.l	$0001A09C
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$5345, $4354, $494F, $4E20, $3620, $4253, $5320, $4154, $2024, $0000			; "SECTION 6 BSS AT $"
		dc.w	$0010, $0006																	; BCMD_PRINT_ADDR
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$2043, $5243, $2024, $0000														; " CRC $"
		dc.w	$0014, $0006																	; BCMD_PRINT_SECTION_CRC
		dc.l	$00000184
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$5345, $4354, $494F, $4E20, $3720, $4253, $5320, $4154, $2024, $0000			; "SECTION 7 BSS AT $"
		dc.w	$0010, $0007																	; BCMD_PRINT_ADDR
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$2043, $5243, $2024, $0000														; " CRC $"
		dc.w	$0014, $0007																	; BCMD_PRINT_SECTION_CRC
		dc.l	$0001F634
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$5345, $4354, $494F, $4E20, $3820, $4348, $4950, $2042, $5353, $2041, $5420, $2400 ; "SECTION 8 CHIP BSS AT $"
		dc.w	$0010, $0008																	; BCMD_PRINT_ADDR
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$2043, $5243, $2024, $0000														; " CRC $"
		dc.w	$0014, $0008																	; BCMD_PRINT_SECTION_CRC
		dc.l	$0000D078
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$5345, $4720, $3134, $2041, $5420, $2400										; "SEG 14 AT $"
		dc.w	$0010, $000E																	; BCMD_PRINT_ADDR
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$5345, $4720, $3135, $2041, $5420, $2400										; "SEG 15 AT $"
		dc.w	$0010, $000F																	; BCMD_PRINT_ADDR
		dc.w	$0013																			; BCMD_PRINT_MEM_STATS
		; 
		; ================================ End ================================
		; 
		dc.w	$000E																			; BCMD_PRINT_NEWLINE
		dc.w	$000F																			; BCMD_PRINT_STRING
		dc.w	$444F, $4E45, $0000																; "DONE"
		dc.w	$000C																			; BCMD_FINALIZE
		dc.w	$000D																			; BCMD_START
		dc.w	$0000
		dc.l	$00000000
		dc.w	$0013																			; BCMD_PRINT_MEM_STATS
		dc.w	$0000																			; BCMD_END

		dc.w	$0000
