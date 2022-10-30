
#ifndef _DREAD_ASM_H
#define _DREAD_ASM_H


// cheat structs

typedef void(*line_cheat_fn)(__d2 int xmin, __d3 int xcount, __d4 dword s1, __d6 dword u1s, __d7 int du, __a5 EngineLine *line);

typedef struct {
	line_cheat_fn	fn_persp;
	line_cheat_fn	fn_nopersp;
	short			clip;
	short			_pad0;
	short			_pad1;
	short			_pad2;		// 16 bytes total
} LineCheatMode;

//$asmstruct lc
typedef struct {
	word	_pad;						//	 +0
	short	ceil_len_m64;				//	 +2
	short	ceil_len_m128;				//	 +4
	word	*upper_start_m64;			//	 +6
	word	*upper_start_m128;			//	+10
	short	upper_len_m64_0;			//	+14
	short	upper_len_m128_0;			//	+16
	short	upper_len_m128_m64;			//	+18
	short	floor_start_0;				//	+20
	short	hole_size_m64_0;			//	+22
	short	ceil_ymin_m64;				//	+24
	short	floor_length_0;				//	+26
	short	real_size;					//	+28
	word	*scaler_fn;					//	+30
	short	upper_endpos_0;				//	+34
	short	upper_endpos_m64;			//	+36
	short	upper_len_m96_0;			//	+38
	word	*upper_start_m96;			//	+40
	short	ceil_len_m96;				//	+44
	short	upper_len_m96_m64;			//	+46
	short	upper_endpos_m96;			//	+48
	short	hole_size_m96_0;			//	+50
	short	ceil_ymin_m96;				//	+52
	short	upper_len_m128_m96;			//	+54
	short	hole_size_m128_0;			//	+56
	short	ceil_ymin_m128;				//	+58
	short	hole_end_0;					//	+60
										// 62 bytes
} LineSizeCheat;


extern LineCheatMode CHEAT_MODES[];


// asm globals

extern word cpu_type;

extern short asm_line_ds;
extern const byte *asm_tex_base;
extern word *asm_fn_skycopy;
extern byte *asm_render_buffer;


// asm functons

void asm_memclear(__a0 void *addr, __d0 dword byte_count);
void asm_memcpy(__a0 void *dst, __a1 const void *src, __d0 dword byte_count);
int lsqrt(__d0 int x);

void IO_CheckMouse(void);

void Dread_FrameReset(__a2 byte *render_buffer);
EngineSubSector *Dread_FindSubSector(__d2 short xp, __d3 short yp);
void Dread_LineCore_Sprite(__d2 int xmin, __d3 int xcount, __d4 dword s0, __d6 dword u1s, __d7 int du, __a3 const word *frame);

int Dread_LineWall_Fix2_asm(__a5 EngineLine *line, __d7 word mode);


void Dread_DrawSprites(void);


void Dread_DrawVisible(__a2 const word *vis);
void Dread_DrawLinesTable(void);


void Script_TickObjects(__d7 word ticks);
void Script_ResolveDamage(void);
void Script_OnCreate(__a6 EngineThing *th);
void Script_OnDamage(__a6 EngineThing *th);
void Script_OnPulse(__a6 EngineThing *th);
void Script_PlayerDamaged(__d7 short damage);



// inflate.s
//void inflate(__a5 const void *input, __a4 void *output);
void inflate(__a5 const void *input, __a4 void *output, __a6 word *scratch_end);

#endif
