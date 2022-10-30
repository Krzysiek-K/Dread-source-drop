
#ifndef _AMIGA_DEBUG_H
#define _AMIGA_DEBUG_H

// amiga_debug.c

#if DEBUG_CONSOLE
extern __chip word DEBUG_CONSOLE_SCREEN[20*8*DEBUG_CONSOLE];
#endif


extern const byte HEXFONT[];

void debug_write_hexbuff(word *screenpos, int len, byte *data);
void debug_write_dec(word *screenpos, byte value);
void debug_write_perf(word *screenpos, int value);


#endif
