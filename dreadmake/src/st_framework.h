
#ifndef _ST_FRAMEWORK_H
#define _ST_FRAMEWORK_H


extern word palette_index;
extern byte *render_buffer;



extern word *screen;
extern word *screen_active;
extern word render_screen[160*100/2];
extern word *music_active;
extern word music_track;


extern volatile const word *vblank_pal;
extern volatile const word *vblank_hudpal;
extern volatile const word *vblank_playsound[3];
extern volatile word vblank_playwait;


void init_c2p_lut(void);
void perform_c2p(void);



// st_debug.c
extern const byte NUMFONT[];

void debug_write_hexbuff(word *screenpos, int len, byte *data);
void debug_write_dec(word *screenpos, byte value);
void debug_write_perf(word *screenpos, int value);


// st_audio.s
void init_music(__d0 word track);



// st_asm.s
void set_supervisor(void);
void set_usermode(void);

void st_install_vblank(void);
void st_remove_vblank(void);
dword st_get_perftimer(void);


void ikbd_install_handler(void);
void ikbd_remove_handler(void);
void ikbd_write(__a0 byte *data, __d0 int len);
void ikbd_qwrite(__d1 dword command, __d0 int len);

void c2p_asm(__a0 word *render, __a1 byte* dst, __a0 word *render2, __a5 const dword *lut0, __a6 const dword *lut1);
void c2p_fix(__a0 word *screen);
void c2p_fix_blit(__a0 word *screen);


extern byte ikbd_rxbuff[8];


// st_system.s

dword ST_GetPerfTimer(void);
word ST_CriticalBegin(void);
void ST_CriticalEnd(__d0 word saved_sr);



// st_draw.s


void Debug_WritePerf(__a0 word *screenpos, __d0 int value);
void ST_DrawWeaponAsm(__a0 word *screen, __a1 const void *info);		// word*, const WeaponFrameInfoST*
void ST_DrawClearMaskedNarrow(__d2 word xpos, __d3 word ypos, __a0 word *screen, __a1 const word *glyph, __a2 const word *background);
void ST_DrawOpaqueAligned(__d2 word xpos, __d3 word ypos, __a0 word *screen, __a1 const word *bitmap);
void ST_DrawResetBlockAsm(__a0 word *screen, __a1 const word *background, __d0 word x1, __d1 word y1, __d2 word x2, __d3 word y2);
void ST_DrawMasked(__a0 word *screen, __a1 const word *bitmap, __d0 word xpos, __d2 word ypos);
void ST_DrawDreadGlyph(__a0 word *screen, __a1 const word *glyph, __d0 word xpos, __d2 word ypos, __d1 word height, __d3 word color_set);
void ST_DrawDreadGlyphShadow(__a0 word *screen, __a1 const word *glyph, __d0 word xpos, __d2 word ypos, __d1 word height);


// st_audio.s

typedef struct _EngineThing		EngineThing;
void Sound_PlayThingSound(__a6 EngineThing *thing, __a0 const word *sound, __d7 int attenuation);



// st_comm.c

//#define MAX_COMM_PACKET_LENGTH		32
//#define NUM_COMM_PACKETS			5
//
//typedef struct {
//	byte	status;
//	byte	_pad;
//	word	len;
//	byte	data[MAX_COMM_PACKET_LENGTH];
//} CommPacket;
//

#define COMM_MAX_RXSCAN_BUFF	256

extern byte comm_rx_buff[256];
extern word comm_rx_put;
extern word comm_rx_get;

extern byte comm_debug_buff[32];
extern word comm_debug1;
extern word comm_debug2;

extern byte comm_scan_buff[COMM_MAX_RXSCAN_BUFF];
extern word comm_scan_len;				// Packet length	(0: no packet)
extern byte comm_scan_escape;			// 0xD5 was received


void Comm_Init(void);
void Comm_Packet_End(void);
void Comm_Write_Byte(__d0 byte data);
void Comm_Write_Word(__d0 word data);
void Comm_Write(byte *data, int len);

void Parse_MP_Packets(void);
void MP_Respawn(void);
byte MP_ChooseLocation(void);


void ST_Comm_Init_Asm(void);		// asm
void ST_Comm_Commit(void);			// asm
word ST_Comm_In(void);				// asm
void ST_Comm_Out(__d0 word data);	// asm


#endif
