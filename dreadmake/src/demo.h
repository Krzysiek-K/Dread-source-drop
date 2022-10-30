
#ifndef _DEMO_H
#define _DEMO_H




typedef unsigned char	byte;
typedef unsigned short	word;
typedef unsigned int	dword;
typedef void			*ptr_t;


//#define muls(a,b)			(((int)(short)(a))*((short)(b)))
//#define divs(a,b)			((short)(((int)(a))/((short)(b))))

#define muls(a,b)		( (int)((short)(a)) * (short)(b) )
#define mulu(a,b)		( (dword)((word)(a)) * (word)(b) )
#define divs(a,b)		( (short)( (int)(a) / (short)(b) ) )
#define divu(a,b)		( (word)( (dword)(a) / (word)(b) ) )



// Amiga stuff
#if AMIGA
	#include "amiga.h"
	#include "amiga_system.h"
	#include "amiga_debug.h"
	#include "amiga_blitter.h"
	#include "amiga_vscreen.h"
	#include "amiga_menu.h"
	#include "amiga_minimap.h"
	#include "amiga_asm.h"
	#include "amiga_audio.h"
	#include "amiga_audio2.h"
	#include "amiga_files.h"
	#include "amiga_comm.h"
	#include "trackloader.h"
#endif

// ST stuff
#if ATARI_ST
	#include "st.h"
	#include "st_framework.h"
	#include "st_menu.h"
#endif





#define SHOW_TICKS


#include "xcode.h"
#if AMIGA
	#include "trackloader.h"
#endif

//#include "diskmap.inc"


void memset(void *, int, int);
void memcpy(void *, const void *, int);


// generic typedefs
#ifndef NULL
  #define NULL		((void*)0)
#endif


#define TABLE_LENGTH(tab)			( sizeof(tab)/sizeof((tab)[0]) )




// functions done in ASM

#define MEMF_CLEAR		0x10000
#define MEMF_CHIP		0x00002
#define MEMF_FAST		0x00004



void Foo(__a0 void *addr);
void BlitWait(void);
void SetSR(__d0 word value);
word GetSR(void);
word GetCIAA_TimerA(void);
void ScreenWait(void);

__d0 word Track_Decode(__a1 void *track_data, __d0 word start_sec, __d1 word num_sec, __a0 void *target_buffer);




// variables declared in ASM
extern word P61Module[];
extern word P61Samples[];




// mem.c

void *Sys_Alloc64(void);
#if !SYSTEM_PRESENT
void *Sys_AllocMax(dword *out_size);
#endif


// fn.c

#define HIWORD(x)			(word)(((dword)(x)) >> 16 )
#define LOWORD(x)			(word)((dword)(x))



static inline int Min(int a,int b)	{ return a<b ? a : b; }
static inline int Max(int a,int b)	{ return a>b ? a : b; }





// m_copper.c


extern __chip word BLANK_COPPER[];
extern __chip word WHITE_COPPER[];

extern word *cop_write;
extern word cop_write_y;




#define Cop_WriteAddr(addr,value)	do{ *cop_write++=(addr);						*cop_write++=(word)(value); }while(0)
#define Cop_Write(reg,value)		do{ *cop_write++=((word)(int)(&(reg)))&0x01FE;	*cop_write++=(word)(value); }while(0)



void SetCopper(__a0 word *coplist);
void Init_Copper(void);



// m_timing.c

extern dword demo_frame;		// demo frame counter
extern word frame_ticks;		// how many ticks previous frame has taken




// main.c


extern int debug_vars[16];
extern word debug_target_index;


extern volatile byte io_keystate[128];
extern volatile byte io_key_last;
extern volatile byte io_mouse_state;
extern volatile short io_mouse_xdelta;
extern volatile short io_mouse_ydelta;

extern byte *blockmem1;			// aligned 64k memory block
extern byte *blockmem2;			// aligned 64k memory block
extern byte *blockmem3;			// aligned 64k memory block

extern volatile word intnum;	// vblank counter
extern volatile word rawintnum;	// vblank counter
extern dword _rnd;				// random counter






extern word all_ok;

extern short sincos_fix14[256];		// fn.c

#define fixsin(x)		(sincos_fix14[((x)+192)&0xFF])
#define fixcos(x)		(sincos_fix14[(x)&0xFF])
#define rnd()			(_rnd = _rnd*214013 + 2531011)
#define krand()			((int)(rnd()>>17))

void init_sincos(void);							// fn.c
const char *_itoa(int value);					// fn.c
const char *_itoa_hex(dword value);				// fn.c
char *_append_s(char *s, const char *text);		// fn.c
const char *_time_100s_a(int time);				// fn.c




extern __chip word NULL_SPRITE[];
extern __chip word TICK_SHOW[];



void Main_Init(void);



void MarkFrame(void);


#include "dread.h"


// Amiga stuff
#if AMIGA
	#include "amiga_statbar.h"			// late, because uses dread limits
#endif


#endif
