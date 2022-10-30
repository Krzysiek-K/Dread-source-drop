
#ifndef _DREAD_TEST_H
#define _DREAD_TEST_H


#include "dread_engine.h"



// globals declared in ASM
extern word BlockAddr_Work;
extern word BlockAddr_Temp;
extern word C2P_Scene_Queue[2];

// functions done in ASM
byte *C2P_BeginScene(void);
void C2P_EndScene(void);



// dread_codegen.c

typedef void(*scaler_fn_t)(__a0 byte *dst, __a2 const byte *tex);
typedef void(*colorfill_fn_t)(__a0 byte *dst, __d1 byte color);
typedef void(*skycopy_fn_t)(__a0 byte *dst, __a1 const byte *sky);

typedef struct {
	word	*func_addr;
	short	real_size;
	short	_pad;
} ScalerInfo;


#define NUM_SOFT_SIZES		1201

extern ScalerInfo FN_SCALERS[NUM_SOFT_SIZES];
extern colorfill_fn_t FN_COLORFILL;
extern skycopy_fn_t FN_SKYCOPY;


void Dread_Build_Scalers(void);
void Dread_Build_Fillers(void);



// dread_soft.c

extern word cam_weapon_swing;
extern word weapon_frame;
extern word weapon_reload;

extern short lrc_cam_pos_x;							// camera
extern short lrc_cam_pos_y;
extern short lrc_cam_pos_z;
extern short lrc_cam_angle;

void Dread_SoftInit(void);
void Dread_SoftTest(void);



// dread_test.c



extern byte *render_A;

extern const byte TEXTURE_DATA[];
extern const byte SKYTEX_DATA[];

extern byte *render_buffer;


void FX_DreadTest(void);


#endif
