
#ifndef _DREAD_TEST_H
#define _DREAD_TEST_H



// dread_soft.c

extern short lrc_cam_pos_x;							// camera
extern short lrc_cam_pos_y;
extern short lrc_cam_pos_z;
extern short lrc_cam_angle;

void Dread_SoftInit(void);
void Dread_SoftTest(void);



// dread_test.c

//typedef void(*colblit_fn_t)(__a0 byte *dst, __a2 const byte *tex, __d0 byte ceil, __d1 byte floor);
typedef void(*colblit_fn_t)(__a0 byte *dst, __a2 const byte *tex, __a1 byte *sky, __d1 byte floor);

extern const byte TEXTURE_DATA[];
extern const byte SKYTEX_DATA[];

extern colblit_fn_t COLBLITS[101];
extern byte *render_buffer;


void FX_DreadTest(void);


#endif
