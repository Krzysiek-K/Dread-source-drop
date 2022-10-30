

; ================================================================ HEADER ================================================================
;
		public _memfile_START
_memfile_START:
																								; offset $0000

; ================================================================ DreadLoadScript ================================================================
;
		public _data_block_DreadLoadScript
_data_block_DreadLoadScript:
		; 
		; ================================ Allocate segments ================================
		; 
		dc.w	$0003, $0000																	; BCMD_ALLOC_FAST
		dc.l	$0000D200
		dc.w	$0003, $0001																	; BCMD_ALLOC_FAST
		dc.l	$00048A00
		dc.w	$0003, $0002																	; BCMD_ALLOC_FAST
		dc.l	$00004A00
		dc.w	$0001, $0003																	; BCMD_ALLOC_CHIP
		dc.l	$00000200
		dc.w	$0003, $0004																	; BCMD_ALLOC_FAST
		dc.l	$00001000
		dc.w	$0001, $0005																	; BCMD_ALLOC_CHIP
		dc.l	$0001A200
		dc.w	$0003, $0006																	; BCMD_ALLOC_FAST
		dc.l	$00000200
		dc.w	$0003, $0007																	; BCMD_ALLOC_FAST
		dc.l	$00022E00
		dc.w	$0001, $0008																	; BCMD_ALLOC_CHIP
		dc.l	$00003400
		; 
		; ================================ Load segment 0 ================================
		; 
		dc.w	$0005, $0006																	; BCMD_TRACK_LOAD      sectors 66 - 76      track 3:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0000																	; BCMD_TRACK_DECODE    sectors 71 - 76
		dc.l	$00000000
		dc.w	$0005, $0006
		dc.w	$0005, $0007																	; BCMD_TRACK_LOAD      sectors 77 - 87      track 3:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0000																	; BCMD_TRACK_DECODE    sectors 77 - 87
		dc.l	$00000C00
		dc.w	$0000, $000B
		dc.w	$0014, $0001																	; BCMD_PROGRESS
		dc.w	$0005, $0008																	; BCMD_TRACK_LOAD      sectors 88 - 98      track 4:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0000																	; BCMD_TRACK_DECODE    sectors 88 - 98
		dc.l	$00002200
		dc.w	$0000, $000B
		dc.w	$0014, $0003																	; BCMD_PROGRESS
		dc.w	$0005, $0009																	; BCMD_TRACK_LOAD      sectors 99 - 109      track 4:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0000																	; BCMD_TRACK_DECODE    sectors 99 - 109
		dc.l	$00003800
		dc.w	$0000, $000B
		dc.w	$0014, $0004																	; BCMD_PROGRESS
		dc.w	$0005, $000A																	; BCMD_TRACK_LOAD      sectors 110 - 120      track 5:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0000																	; BCMD_TRACK_DECODE    sectors 110 - 120
		dc.l	$00004E00
		dc.w	$0000, $000B
		dc.w	$0014, $0005																	; BCMD_PROGRESS
		dc.w	$0005, $000B																	; BCMD_TRACK_LOAD      sectors 121 - 131      track 5:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0000																	; BCMD_TRACK_DECODE    sectors 121 - 131
		dc.l	$00006400
		dc.w	$0000, $000B
		dc.w	$0014, $0006																	; BCMD_PROGRESS
		dc.w	$0005, $000C																	; BCMD_TRACK_LOAD      sectors 132 - 142      track 6:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0000																	; BCMD_TRACK_DECODE    sectors 132 - 142
		dc.l	$00007A00
		dc.w	$0000, $000B
		dc.w	$0014, $0007																	; BCMD_PROGRESS
		dc.w	$0005, $000D																	; BCMD_TRACK_LOAD      sectors 143 - 153      track 6:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0000																	; BCMD_TRACK_DECODE    sectors 143 - 153
		dc.l	$00009000
		dc.w	$0000, $000B
		dc.w	$0014, $0009																	; BCMD_PROGRESS
		dc.w	$0005, $000E																	; BCMD_TRACK_LOAD      sectors 154 - 164      track 7:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0000																	; BCMD_TRACK_DECODE    sectors 154 - 164
		dc.l	$0000A600
		dc.w	$0000, $000B
		dc.w	$0014, $000A																	; BCMD_PROGRESS
		dc.w	$0005, $000F																	; BCMD_TRACK_LOAD      sectors 165 - 175      track 7:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0000																	; BCMD_TRACK_DECODE    sectors 165 - 175
		dc.l	$0000BC00
		dc.w	$0000, $000B
		dc.w	$0014, $000B																	; BCMD_PROGRESS
		; 
		; ================================ Relocations 0 -> 0 ================================
		; 
		dc.w	$0005, $0010																	; BCMD_TRACK_LOAD      sectors 176 - 186      track 8:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 176 - 179
		dc.l	$00000000
		dc.w	$0000, $0004
		dc.w	$000B, $0000, $0000, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$01AE
		; 
		; ================================ Relocations 0 -> 1 ================================
		; 
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 180 - 184
		dc.l	$00000000
		dc.w	$0004, $0005
		dc.w	$0014, $000C																	; BCMD_PROGRESS
		dc.w	$000B, $0000, $0001, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0208
		; 
		; ================================ Relocations 0 -> 4 ================================
		; 
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 185 - 185
		dc.l	$00000000
		dc.w	$0009, $0001
		dc.w	$000B, $0000, $0004, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$000B
		; 
		; ================================ Relocations 0 -> 6 ================================
		; 
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 186 - 186
		dc.l	$00000000
		dc.w	$000A, $0001
		dc.w	$000B, $0000, $0006, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0008
		; 
		; ================================ Relocations 0 -> 7 ================================
		; 
		dc.w	$0005, $0011																	; BCMD_TRACK_LOAD      sectors 187 - 197      track 8:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 187 - 191
		dc.l	$00000000
		dc.w	$0000, $0005
		dc.w	$0014, $000D																	; BCMD_PROGRESS
		dc.w	$000B, $0000, $0007, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0239
		; 
		; ================================ Load segment 1 ================================
		; 
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 192 - 197
		dc.l	$00000000
		dc.w	$0005, $0006
		dc.w	$0005, $0012																	; BCMD_TRACK_LOAD      sectors 198 - 208      track 9:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 198 - 208
		dc.l	$00000C00
		dc.w	$0000, $000B
		dc.w	$0014, $000F																	; BCMD_PROGRESS
		dc.w	$0005, $0013																	; BCMD_TRACK_LOAD      sectors 209 - 219      track 9:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 209 - 219
		dc.l	$00002200
		dc.w	$0000, $000B
		dc.w	$0014, $0010																	; BCMD_PROGRESS
		dc.w	$0005, $0014																	; BCMD_TRACK_LOAD      sectors 220 - 230      track 10:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 220 - 230
		dc.l	$00003800
		dc.w	$0000, $000B
		dc.w	$0014, $0011																	; BCMD_PROGRESS
		dc.w	$0005, $0015																	; BCMD_TRACK_LOAD      sectors 231 - 241      track 10:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 231 - 241
		dc.l	$00004E00
		dc.w	$0000, $000B
		dc.w	$0014, $0012																	; BCMD_PROGRESS
		dc.w	$0005, $0016																	; BCMD_TRACK_LOAD      sectors 242 - 252      track 11:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 242 - 252
		dc.l	$00006400
		dc.w	$0000, $000B
		dc.w	$0014, $0014																	; BCMD_PROGRESS
		dc.w	$0005, $0017																	; BCMD_TRACK_LOAD      sectors 253 - 263      track 11:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 253 - 263
		dc.l	$00007A00
		dc.w	$0000, $000B
		dc.w	$0014, $0015																	; BCMD_PROGRESS
		dc.w	$0005, $0018																	; BCMD_TRACK_LOAD      sectors 264 - 274      track 12:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 264 - 274
		dc.l	$00009000
		dc.w	$0000, $000B
		dc.w	$0014, $0016																	; BCMD_PROGRESS
		dc.w	$0005, $0019																	; BCMD_TRACK_LOAD      sectors 275 - 285      track 12:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 275 - 285
		dc.l	$0000A600
		dc.w	$0000, $000B
		dc.w	$0014, $0017																	; BCMD_PROGRESS
		dc.w	$0005, $001A																	; BCMD_TRACK_LOAD      sectors 286 - 296      track 13:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 286 - 296
		dc.l	$0000BC00
		dc.w	$0000, $000B
		dc.w	$0014, $0018																	; BCMD_PROGRESS
		dc.w	$0005, $001B																	; BCMD_TRACK_LOAD      sectors 297 - 307      track 13:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 297 - 307
		dc.l	$0000D200
		dc.w	$0000, $000B
		dc.w	$0014, $001A																	; BCMD_PROGRESS
		dc.w	$0005, $001C																	; BCMD_TRACK_LOAD      sectors 308 - 318      track 14:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 308 - 318
		dc.l	$0000E800
		dc.w	$0000, $000B
		dc.w	$0014, $001B																	; BCMD_PROGRESS
		dc.w	$0005, $001D																	; BCMD_TRACK_LOAD      sectors 319 - 329      track 14:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 319 - 329
		dc.l	$0000FE00
		dc.w	$0000, $000B
		dc.w	$0014, $001C																	; BCMD_PROGRESS
		dc.w	$0005, $001E																	; BCMD_TRACK_LOAD      sectors 330 - 340      track 15:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 330 - 340
		dc.l	$00011400
		dc.w	$0000, $000B
		dc.w	$0014, $001D																	; BCMD_PROGRESS
		dc.w	$0005, $001F																	; BCMD_TRACK_LOAD      sectors 341 - 351      track 15:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 341 - 351
		dc.l	$00012A00
		dc.w	$0000, $000B
		dc.w	$0014, $001E																	; BCMD_PROGRESS
		dc.w	$0005, $0020																	; BCMD_TRACK_LOAD      sectors 352 - 362      track 16:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 352 - 362
		dc.l	$00014000
		dc.w	$0000, $000B
		dc.w	$0014, $0020																	; BCMD_PROGRESS
		dc.w	$0005, $0021																	; BCMD_TRACK_LOAD      sectors 363 - 373      track 16:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 363 - 373
		dc.l	$00015600
		dc.w	$0000, $000B
		dc.w	$0014, $0021																	; BCMD_PROGRESS
		dc.w	$0005, $0022																	; BCMD_TRACK_LOAD      sectors 374 - 384      track 17:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 374 - 384
		dc.l	$00016C00
		dc.w	$0000, $000B
		dc.w	$0014, $0022																	; BCMD_PROGRESS
		dc.w	$0005, $0023																	; BCMD_TRACK_LOAD      sectors 385 - 395      track 17:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 385 - 395
		dc.l	$00018200
		dc.w	$0000, $000B
		dc.w	$0014, $0023																	; BCMD_PROGRESS
		dc.w	$0005, $0024																	; BCMD_TRACK_LOAD      sectors 396 - 406      track 18:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 396 - 406
		dc.l	$00019800
		dc.w	$0000, $000B
		dc.w	$0014, $0024																	; BCMD_PROGRESS
		dc.w	$0005, $0025																	; BCMD_TRACK_LOAD      sectors 407 - 417      track 18:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 407 - 417
		dc.l	$0001AE00
		dc.w	$0000, $000B
		dc.w	$0014, $0026																	; BCMD_PROGRESS
		dc.w	$0005, $0026																	; BCMD_TRACK_LOAD      sectors 418 - 428      track 19:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 418 - 428
		dc.l	$0001C400
		dc.w	$0000, $000B
		dc.w	$0014, $0027																	; BCMD_PROGRESS
		dc.w	$0005, $0027																	; BCMD_TRACK_LOAD      sectors 429 - 439      track 19:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 429 - 439
		dc.l	$0001DA00
		dc.w	$0000, $000B
		dc.w	$0014, $0028																	; BCMD_PROGRESS
		dc.w	$0005, $0028																	; BCMD_TRACK_LOAD      sectors 440 - 450      track 20:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 440 - 450
		dc.l	$0001F000
		dc.w	$0000, $000B
		dc.w	$0014, $0029																	; BCMD_PROGRESS
		dc.w	$0005, $0029																	; BCMD_TRACK_LOAD      sectors 451 - 461      track 20:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 451 - 461
		dc.l	$00020600
		dc.w	$0000, $000B
		dc.w	$0014, $002A																	; BCMD_PROGRESS
		dc.w	$0005, $002A																	; BCMD_TRACK_LOAD      sectors 462 - 472      track 21:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 462 - 472
		dc.l	$00021C00
		dc.w	$0000, $000B
		dc.w	$0014, $002C																	; BCMD_PROGRESS
		dc.w	$0005, $002B																	; BCMD_TRACK_LOAD      sectors 473 - 483      track 21:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 473 - 483
		dc.l	$00023200
		dc.w	$0000, $000B
		dc.w	$0014, $002D																	; BCMD_PROGRESS
		dc.w	$0005, $002C																	; BCMD_TRACK_LOAD      sectors 484 - 494      track 22:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 484 - 494
		dc.l	$00024800
		dc.w	$0000, $000B
		dc.w	$0014, $002E																	; BCMD_PROGRESS
		dc.w	$0005, $002D																	; BCMD_TRACK_LOAD      sectors 495 - 505      track 22:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 495 - 505
		dc.l	$00025E00
		dc.w	$0000, $000B
		dc.w	$0014, $002F																	; BCMD_PROGRESS
		dc.w	$0005, $002E																	; BCMD_TRACK_LOAD      sectors 506 - 516      track 23:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 506 - 516
		dc.l	$00027400
		dc.w	$0000, $000B
		dc.w	$0014, $0031																	; BCMD_PROGRESS
		dc.w	$0005, $002F																	; BCMD_TRACK_LOAD      sectors 517 - 527      track 23:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 517 - 527
		dc.l	$00028A00
		dc.w	$0000, $000B
		dc.w	$0014, $0032																	; BCMD_PROGRESS
		dc.w	$0005, $0030																	; BCMD_TRACK_LOAD      sectors 528 - 538      track 24:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 528 - 538
		dc.l	$0002A000
		dc.w	$0000, $000B
		dc.w	$0014, $0033																	; BCMD_PROGRESS
		dc.w	$0005, $0031																	; BCMD_TRACK_LOAD      sectors 539 - 549      track 24:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 539 - 549
		dc.l	$0002B600
		dc.w	$0000, $000B
		dc.w	$0014, $0034																	; BCMD_PROGRESS
		dc.w	$0005, $0032																	; BCMD_TRACK_LOAD      sectors 550 - 560      track 25:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 550 - 560
		dc.l	$0002CC00
		dc.w	$0000, $000B
		dc.w	$0014, $0035																	; BCMD_PROGRESS
		dc.w	$0005, $0033																	; BCMD_TRACK_LOAD      sectors 561 - 571      track 25:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 561 - 571
		dc.l	$0002E200
		dc.w	$0000, $000B
		dc.w	$0014, $0037																	; BCMD_PROGRESS
		dc.w	$0005, $0034																	; BCMD_TRACK_LOAD      sectors 572 - 582      track 26:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 572 - 582
		dc.l	$0002F800
		dc.w	$0000, $000B
		dc.w	$0014, $0038																	; BCMD_PROGRESS
		dc.w	$0005, $0035																	; BCMD_TRACK_LOAD      sectors 583 - 593      track 26:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 583 - 593
		dc.l	$00030E00
		dc.w	$0000, $000B
		dc.w	$0014, $0039																	; BCMD_PROGRESS
		dc.w	$0005, $0036																	; BCMD_TRACK_LOAD      sectors 594 - 604      track 27:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 594 - 604
		dc.l	$00032400
		dc.w	$0000, $000B
		dc.w	$0014, $003A																	; BCMD_PROGRESS
		dc.w	$0005, $0037																	; BCMD_TRACK_LOAD      sectors 605 - 615      track 27:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 605 - 615
		dc.l	$00033A00
		dc.w	$0000, $000B
		dc.w	$0014, $003B																	; BCMD_PROGRESS
		dc.w	$0005, $0038																	; BCMD_TRACK_LOAD      sectors 616 - 626      track 28:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 616 - 626
		dc.l	$00035000
		dc.w	$0000, $000B
		dc.w	$0014, $003D																	; BCMD_PROGRESS
		dc.w	$0005, $0039																	; BCMD_TRACK_LOAD      sectors 627 - 637      track 28:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 627 - 637
		dc.l	$00036600
		dc.w	$0000, $000B
		dc.w	$0014, $003E																	; BCMD_PROGRESS
		dc.w	$0005, $003A																	; BCMD_TRACK_LOAD      sectors 638 - 648      track 29:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 638 - 648
		dc.l	$00037C00
		dc.w	$0000, $000B
		dc.w	$0014, $003F																	; BCMD_PROGRESS
		dc.w	$0005, $003B																	; BCMD_TRACK_LOAD      sectors 649 - 659      track 29:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 649 - 659
		dc.l	$00039200
		dc.w	$0000, $000B
		dc.w	$0014, $0040																	; BCMD_PROGRESS
		dc.w	$0005, $003C																	; BCMD_TRACK_LOAD      sectors 660 - 670      track 30:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 660 - 670
		dc.l	$0003A800
		dc.w	$0000, $000B
		dc.w	$0014, $0041																	; BCMD_PROGRESS
		dc.w	$0005, $003D																	; BCMD_TRACK_LOAD      sectors 671 - 681      track 30:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 671 - 681
		dc.l	$0003BE00
		dc.w	$0000, $000B
		dc.w	$0014, $0043																	; BCMD_PROGRESS
		dc.w	$0005, $003E																	; BCMD_TRACK_LOAD      sectors 682 - 692      track 31:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 682 - 692
		dc.l	$0003D400
		dc.w	$0000, $000B
		dc.w	$0014, $0044																	; BCMD_PROGRESS
		dc.w	$0005, $003F																	; BCMD_TRACK_LOAD      sectors 693 - 703      track 31:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 693 - 703
		dc.l	$0003EA00
		dc.w	$0000, $000B
		dc.w	$0014, $0045																	; BCMD_PROGRESS
		dc.w	$0005, $0040																	; BCMD_TRACK_LOAD      sectors 704 - 714      track 32:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 704 - 714
		dc.l	$00040000
		dc.w	$0000, $000B
		dc.w	$0014, $0046																	; BCMD_PROGRESS
		dc.w	$0005, $0041																	; BCMD_TRACK_LOAD      sectors 715 - 725      track 32:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 715 - 725
		dc.l	$00041600
		dc.w	$0000, $000B
		dc.w	$0014, $0047																	; BCMD_PROGRESS
		dc.w	$0005, $0042																	; BCMD_TRACK_LOAD      sectors 726 - 736      track 33:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 726 - 736
		dc.l	$00042C00
		dc.w	$0000, $000B
		dc.w	$0014, $0049																	; BCMD_PROGRESS
		dc.w	$0005, $0043																	; BCMD_TRACK_LOAD      sectors 737 - 747      track 33:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 737 - 747
		dc.l	$00044200
		dc.w	$0000, $000B
		dc.w	$0014, $004A																	; BCMD_PROGRESS
		dc.w	$0005, $0044																	; BCMD_TRACK_LOAD      sectors 748 - 758      track 34:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 748 - 758
		dc.l	$00045800
		dc.w	$0000, $000B
		dc.w	$0014, $004B																	; BCMD_PROGRESS
		dc.w	$0005, $0045																	; BCMD_TRACK_LOAD      sectors 759 - 769      track 34:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 759 - 769
		dc.l	$00046E00
		dc.w	$0000, $000B
		dc.w	$0014, $004C																	; BCMD_PROGRESS
		dc.w	$0005, $0046																	; BCMD_TRACK_LOAD      sectors 770 - 780      track 35:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0001																	; BCMD_TRACK_DECODE    sectors 770 - 772
		dc.l	$00048400
		dc.w	$0000, $0003
		dc.w	$0014, $004D																	; BCMD_PROGRESS
		; 
		; ================================ Relocations 1 -> 0 ================================
		; 
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 773 - 775
		dc.l	$00000000
		dc.w	$0003, $0003
		dc.w	$000B, $0001, $0000, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$010A
		; 
		; ================================ Relocations 1 -> 1 ================================
		; 
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 776 - 780
		dc.l	$00000000
		dc.w	$0006, $0005
		dc.w	$0014, $004E																	; BCMD_PROGRESS
		dc.w	$000B, $0001, $0001, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0269
		; 
		; ================================ Relocations 1 -> 2 ================================
		; 
		dc.w	$0005, $0047																	; BCMD_TRACK_LOAD      sectors 781 - 791      track 35:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 781 - 781
		dc.l	$00000000
		dc.w	$0000, $0001
		dc.w	$000B, $0001, $0002, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0002
		; 
		; ================================ Relocations 1 -> 3 ================================
		; 
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 782 - 782
		dc.l	$00000000
		dc.w	$0001, $0001
		dc.w	$000B, $0001, $0003, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$003E
		; 
		; ================================ Relocations 1 -> 4 ================================
		; 
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 783 - 785
		dc.l	$00000000
		dc.w	$0002, $0003
		dc.w	$000B, $0001, $0004, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0126
		; 
		; ================================ Relocations 1 -> 5 ================================
		; 
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 786 - 788
		dc.l	$00000000
		dc.w	$0005, $0003
		dc.w	$000B, $0001, $0005, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0136
		; 
		; ================================ Relocations 1 -> 7 ================================
		; 
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 789 - 791
		dc.l	$00000000
		dc.w	$0008, $0003
		dc.w	$0014, $004F																	; BCMD_PROGRESS
		dc.w	$0005, $0048																	; BCMD_TRACK_LOAD      sectors 792 - 802      track 36:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 792 - 802
		dc.l	$00000600
		dc.w	$0000, $000B
		dc.w	$0014, $0050																	; BCMD_PROGRESS
		dc.w	$0005, $0049																	; BCMD_TRACK_LOAD      sectors 803 - 813      track 36:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 803 - 809
		dc.l	$00001C00
		dc.w	$0000, $0007
		dc.w	$0014, $0051																	; BCMD_PROGRESS
		dc.w	$000B, $0001, $0007, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0A43
		; 
		; ================================ Relocations 1 -> 8 ================================
		; 
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 810 - 810
		dc.l	$00000000
		dc.w	$0007, $0001
		dc.w	$000B, $0001, $0008, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0052
		; 
		; ================================ Load segment 2 ================================
		; 
		dc.w	$0007, $0002																	; BCMD_TRACK_DECODE    sectors 811 - 813
		dc.l	$00000000
		dc.w	$0008, $0003
		dc.w	$0005, $004A																	; BCMD_TRACK_LOAD      sectors 814 - 824      track 37:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0002																	; BCMD_TRACK_DECODE    sectors 814 - 824
		dc.l	$00000600
		dc.w	$0000, $000B
		dc.w	$0014, $0052																	; BCMD_PROGRESS
		dc.w	$0005, $004B																	; BCMD_TRACK_LOAD      sectors 825 - 835      track 37:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0002																	; BCMD_TRACK_DECODE    sectors 825 - 835
		dc.l	$00001C00
		dc.w	$0000, $000B
		dc.w	$0014, $0054																	; BCMD_PROGRESS
		dc.w	$0005, $004C																	; BCMD_TRACK_LOAD      sectors 836 - 846      track 38:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0002																	; BCMD_TRACK_DECODE    sectors 836 - 846
		dc.l	$00003200
		dc.w	$0000, $000B
		dc.w	$0014, $0055																	; BCMD_PROGRESS
		dc.w	$0005, $004D																	; BCMD_TRACK_LOAD      sectors 847 - 857      track 38:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0002																	; BCMD_TRACK_DECODE    sectors 847 - 847
		dc.l	$00004800
		dc.w	$0000, $0001
		; 
		; ================================ Load segment 3 ================================
		; 
		dc.w	$0007, $0003																	; BCMD_TRACK_DECODE    sectors 848 - 848
		dc.l	$00000000
		dc.w	$0001, $0001
		; 
		; ================================ Load segment 4 ================================
		; 
		dc.w	$0007, $0004																	; BCMD_TRACK_DECODE    sectors 849 - 856
		dc.l	$00000000
		dc.w	$0002, $0008
		dc.w	$0014, $0056																	; BCMD_PROGRESS
		; 
		; ================================ Relocations 4 -> 0 ================================
		; 
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 857 - 857
		dc.l	$00000000
		dc.w	$000A, $0001
		dc.w	$0005, $004E																	; BCMD_TRACK_LOAD      sectors 858 - 868      track 39:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 858 - 858
		dc.l	$00000200
		dc.w	$0000, $0001
		dc.w	$000B, $0004, $0000, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$00A5
		; 
		; ================================ Relocations 4 -> 1 ================================
		; 
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 859 - 859
		dc.l	$00000000
		dc.w	$0001, $0001
		dc.w	$000B, $0004, $0001, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0013
		; 
		; ================================ Relocations 4 -> 5 ================================
		; 
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 860 - 860
		dc.l	$00000000
		dc.w	$0002, $0001
		dc.w	$000B, $0004, $0005, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0065
		; 
		; ================================ Relocations 4 -> 7 ================================
		; 
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 861 - 861
		dc.l	$00000000
		dc.w	$0003, $0001
		dc.w	$000B, $0004, $0007, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0001
		; 
		; ================================ Relocations 4 -> 8 ================================
		; 
		dc.w	$0007, $0007																	; BCMD_TRACK_DECODE    sectors 862 - 862
		dc.l	$00000000
		dc.w	$0004, $0001
		dc.w	$0014, $0057																	; BCMD_PROGRESS
		dc.w	$000B, $0004, $0008, $0007														; BCMD_RELOC32
		dc.l	$00000000
		dc.w	$0001
		; 
		; ================================ Load segment 5 ================================
		; 
		dc.w	$0007, $0005																	; BCMD_TRACK_DECODE    sectors 863 - 868
		dc.l	$00000000
		dc.w	$0005, $0006
		dc.w	$0005, $004F																	; BCMD_TRACK_LOAD      sectors 869 - 879      track 39:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0005																	; BCMD_TRACK_DECODE    sectors 869 - 879
		dc.l	$00000C00
		dc.w	$0000, $000B
		dc.w	$0014, $0058																	; BCMD_PROGRESS
		dc.w	$0005, $0050																	; BCMD_TRACK_LOAD      sectors 880 - 890      track 40:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0005																	; BCMD_TRACK_DECODE    sectors 880 - 890
		dc.l	$00002200
		dc.w	$0000, $000B
		dc.w	$0014, $005A																	; BCMD_PROGRESS
		dc.w	$0005, $0051																	; BCMD_TRACK_LOAD      sectors 891 - 901      track 40:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0005																	; BCMD_TRACK_DECODE    sectors 891 - 901
		dc.l	$00003800
		dc.w	$0000, $000B
		dc.w	$0014, $005B																	; BCMD_PROGRESS
		dc.w	$0005, $0052																	; BCMD_TRACK_LOAD      sectors 902 - 912      track 41:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0005																	; BCMD_TRACK_DECODE    sectors 902 - 912
		dc.l	$00004E00
		dc.w	$0000, $000B
		dc.w	$0014, $005C																	; BCMD_PROGRESS
		dc.w	$0005, $0053																	; BCMD_TRACK_LOAD      sectors 913 - 923      track 41:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0005																	; BCMD_TRACK_DECODE    sectors 913 - 923
		dc.l	$00006400
		dc.w	$0000, $000B
		dc.w	$0014, $005D																	; BCMD_PROGRESS
		dc.w	$0005, $0054																	; BCMD_TRACK_LOAD      sectors 924 - 934      track 42:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0005																	; BCMD_TRACK_DECODE    sectors 924 - 934
		dc.l	$00007A00
		dc.w	$0000, $000B
		dc.w	$0014, $005E																	; BCMD_PROGRESS
		dc.w	$0005, $0055																	; BCMD_TRACK_LOAD      sectors 935 - 945      track 42:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0005																	; BCMD_TRACK_DECODE    sectors 935 - 945
		dc.l	$00009000
		dc.w	$0000, $000B
		dc.w	$0014, $0060																	; BCMD_PROGRESS
		dc.w	$0005, $0056																	; BCMD_TRACK_LOAD      sectors 946 - 956      track 43:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0005																	; BCMD_TRACK_DECODE    sectors 946 - 956
		dc.l	$0000A600
		dc.w	$0000, $000B
		dc.w	$0014, $0061																	; BCMD_PROGRESS
		dc.w	$0005, $0057																	; BCMD_TRACK_LOAD      sectors 957 - 967      track 43:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0005																	; BCMD_TRACK_DECODE    sectors 957 - 967
		dc.l	$0000BC00
		dc.w	$0000, $000B
		dc.w	$0014, $0062																	; BCMD_PROGRESS
		dc.w	$0005, $0058																	; BCMD_TRACK_LOAD      sectors 968 - 978      track 44:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0005																	; BCMD_TRACK_DECODE    sectors 968 - 978
		dc.l	$0000D200
		dc.w	$0000, $000B
		dc.w	$0014, $0063																	; BCMD_PROGRESS
		dc.w	$0005, $0059																	; BCMD_TRACK_LOAD      sectors 979 - 989      track 44:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0005																	; BCMD_TRACK_DECODE    sectors 979 - 989
		dc.l	$0000E800
		dc.w	$0000, $000B
		dc.w	$0014, $0064																	; BCMD_PROGRESS
		dc.w	$0005, $005A																	; BCMD_TRACK_LOAD      sectors 990 - 1000      track 45:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0005																	; BCMD_TRACK_DECODE    sectors 990 - 1000
		dc.l	$0000FE00
		dc.w	$0000, $000B
		dc.w	$0014, $0066																	; BCMD_PROGRESS
		dc.w	$0005, $005B																	; BCMD_TRACK_LOAD      sectors 1001 - 1011      track 45:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0005																	; BCMD_TRACK_DECODE    sectors 1001 - 1011
		dc.l	$00011400
		dc.w	$0000, $000B
		dc.w	$0014, $0067																	; BCMD_PROGRESS
		dc.w	$0005, $005C																	; BCMD_TRACK_LOAD      sectors 1012 - 1022      track 46:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0005																	; BCMD_TRACK_DECODE    sectors 1012 - 1022
		dc.l	$00012A00
		dc.w	$0000, $000B
		dc.w	$0014, $0068																	; BCMD_PROGRESS
		dc.w	$0005, $005D																	; BCMD_TRACK_LOAD      sectors 1023 - 1033      track 46:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0005																	; BCMD_TRACK_DECODE    sectors 1023 - 1033
		dc.l	$00014000
		dc.w	$0000, $000B
		dc.w	$0014, $0069																	; BCMD_PROGRESS
		dc.w	$0005, $005E																	; BCMD_TRACK_LOAD      sectors 1034 - 1044      track 47:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0005																	; BCMD_TRACK_DECODE    sectors 1034 - 1044
		dc.l	$00015600
		dc.w	$0000, $000B
		dc.w	$0014, $006B																	; BCMD_PROGRESS
		dc.w	$0005, $005F																	; BCMD_TRACK_LOAD      sectors 1045 - 1055      track 47:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0005																	; BCMD_TRACK_DECODE    sectors 1045 - 1055
		dc.l	$00016C00
		dc.w	$0000, $000B
		dc.w	$0014, $006C																	; BCMD_PROGRESS
		dc.w	$0005, $0060																	; BCMD_TRACK_LOAD      sectors 1056 - 1066      track 48:0
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0005																	; BCMD_TRACK_DECODE    sectors 1056 - 1066
		dc.l	$00018200
		dc.w	$0000, $000B
		dc.w	$0014, $006D																	; BCMD_PROGRESS
		dc.w	$0005, $0061																	; BCMD_TRACK_LOAD      sectors 1067 - 1077      track 48:1
		dc.w	$0006																			; BCMD_TRACK_WAIT
		dc.w	$0007, $0005																	; BCMD_TRACK_DECODE    sectors 1067 - 1071
		dc.l	$00019800
		dc.w	$0000, $0005
		dc.w	$0014, $006E																	; BCMD_PROGRESS
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
		dc.w	$000A, $0006																	; BCMD_FILL
		dc.l	$00000000, $0000017C
		dc.w	$0000
		; 
		; ================================ Clear BSS 7 ================================
		; 
		dc.w	$000A, $0007																	; BCMD_FILL
		dc.l	$00000000, $00022DA0
		dc.w	$0000
		; 
		; ================================ Clear BSS 8 ================================
		; 
		dc.w	$000A, $0008																	; BCMD_FILL
		dc.l	$00000000, $0000332C
		dc.w	$0000
		; 
		; ================================ End ================================
		; 
		dc.w	$000C																			; BCMD_FINALIZE
		dc.w	$000D																			; BCMD_START
		dc.w	$0000
		dc.l	$00000000
		dc.w	$0000																			; BCMD_END

		dc.w	$0000
