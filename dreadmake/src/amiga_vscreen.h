
#ifndef _AMIGA_VSCREEN_H
#define _AMIGA_VSCREEN_H



extern word *screen_A[5];		// 5th bitplane used only in menu
extern word *screen_B[5];		// 5th bitplane used only in menu
extern byte *render_A;
extern byte *render_B;
extern word *temp_X;
extern word *temp_Y;
extern word *statbar_buffer;
extern word *copper_A;
extern word *copper_B;
extern word *copper_common;
#if WEAPONSPRITES
extern word *copper_sprite_update;
extern word *copper_sprite_update2;
#endif
extern byte *render_buffer;
extern word *render_copper;
extern word palette_index;




void VScreen_Init(void);
void VScreen_Shutdown(void);
void VScreen_BeginFrame(void);
void VScreen_EndFrame(void);



#endif
