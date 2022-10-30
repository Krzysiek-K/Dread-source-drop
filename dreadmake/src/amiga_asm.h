

#ifndef _AMIGA_ASM_H
#define _AMIGA_ASM_H





// globals declared in ASM
extern word BlockAddr_Work;
extern word BlockAddr_Temp;
extern word C2P_Scene_Queue[2];

// functions done in ASM
byte *C2P_BeginScene(void);
void C2P_EndScene(void);
void C2P_WaitAll(void);



// amiga_asmblit.s

void Blit_DrawImage(__a2 word *screen[], __a3 const word *image, __d2 word xp, __d3 short yp);




#endif
