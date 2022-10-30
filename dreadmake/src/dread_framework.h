
#ifndef _DREAD_FRAMEWORK_H
#define _DREAD_FRAMEWORK_H










// dread_codegen.c

typedef struct {
	word	*func_addr;
	short	real_size;
	short	_pad;
} ScalerInfo;


#define NUM_SOFT_SIZES		2048	//1201

extern word *FN_SCALERS[NUM_SOFT_SIZES];
extern word *FN_COLORFILL_MID;
extern word *FN_SKYCOPY;


void Dread_Build_Scalers(void);
void Dread_Build_Fillers(void);



// dread_soft.c


void Dread_SoftInit(void);
void Dread_DrawSprite(int spx, int spy, const word *frame);
void Dread_WeaponLogic(void);
void Dread_RunLogic(void);
void Dread_Render(void);


// dread_game.c

//void Dread_Init_Objects(void);
//void Dread_Handle_Objects(word ticks);
//void Dread_Draw_Test_Objects(void);
void Dread_Draw_Test_Objects2(void);

void Machine_UpdateConditions(void);
void Machine_InitDoor(EngineMachine *m);
void Machine_UpdateDoorGeometry(EngineThing *th);
void Machine_LineScroll(__a6 EngineThing *th, __d7 short xoffs);
void Machine_ThingUpdate(__a6 EngineThing *th, __d7 short script_tick);
void Machine_Update(EngineMachine *m);
void Machine_Activate(int xp, int yp);
void Player_JumpToLocation(__d7 word id);
void Player_ApplyLocationDelta(__d7 word id1, __d6 word id2);
int Game_TagCheck(__d0 word tag);



// dread_framework.c



extern const byte SKYTEX_DATA[];



void FX_DreadTest(void);


#endif
