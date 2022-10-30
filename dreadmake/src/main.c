
#include "demo.h"



int debug_vars[16];
word debug_target_index;



volatile byte io_keystate[128];
volatile byte io_key_last;
volatile byte io_mouse_state;
volatile short io_mouse_xdelta;
volatile short io_mouse_ydelta;


byte *blockmem1;		// MEM:CHIP 64k aligned
byte *blockmem2;		// MEM:CHIP 64k aligned
byte *blockmem3;		// MEM:CHIP 64k aligned
word all_ok;
volatile word intnum;
volatile word rawintnum;
dword _rnd;



__chip word NULL_SPRITE[2];

const void *NULL_SPR_POINTER = &NULL_SPRITE;

__chip word TICK_SHOW [] = {
	// Word 0:	Bits 15-8 contain the low 8 bits of VSTART
	//			Bits 7-0 contain the high 8 bits of HSTART

	//	Word 1:	Bits 15-8       The low eight bits of VSTOP
	//			Bit 7           (Used in attachment)
	//			Bits 6-3        Unused(make zero)
	//			Bit 2           The VSTART high bit
	//			Bit 1           The VSTOP high bit
	//			Bit 0           The HSTART low bit
	0x2C40, 0x3200,	
#ifdef SHOW_TICKS
	0x0000, 0xFFFF,	
	0x0000, 0xFFFF,
	0x0000, 0xFFFF,
	0x0000, 0xFFFF,
	0x0000, 0xFFFF,
	0x0000, 0xFFFF,
#else
	0x0000, 0x0000,
	0x0000, 0x0000,
	0x0000, 0x0000,
	0x0000, 0x0000,
	0x0000, 0x0000,
	0x0000, 0x0000,
#endif
	0x0000, 0x0000,

//	//0x3C40, 0x4200,
//	0x61E0, 0xFFFF,
//	0x9100, 0xFFFF,
//	0x21C0, 0xFFFF,
//	0x1020, 0xFFFF,
//	0x9120, 0xFFFF,
//	0x64C0, 0xFFFF,
//	0x0000, 0x0000,

//	0x61E0, 0x0000,		//	-##- ---# ###- ----
//	0x9100, 0x0000,		//	#--# ---# ---- ----
//	0x21C0, 0x0000,		//	--#- ---# ##-- ----
//	0x1020, 0x0000,		//	---# ---- --#- ----
//	0x9120, 0x0000,		//	#--# ---# --#- ----
//	0x64C0, 0x0000,		//	-##- -#-- ##-- ----
//	0x0000, 0x0000,		//
};
void *TICK_SPR_POINTER = &TICK_SHOW;






// API
void Precalc(void)
{
	all_ok = 1;

	blockmem1 = Sys_Alloc64();
	blockmem2 = Sys_Alloc64();
//	blockmem3 = Sys_Alloc64();

#if SYSTEM_PRESENT
	// allocate as much memory as possible
	dword size = 256*1024;
	for( int i=0; i<NUM_MAP_MEMORY_BUFFERS; i++ )
	{
		memory_buffer_t *buff = map_memory_buffers + i;

		while( size )
		{
			buff->ptr = (word*)Sys_AllocMem(size, 0);
			if( buff->ptr )
			{
				buff->size = size;
				buff->alloc = buff->ptr;
				buff->end = buff->ptr + buff->size/sizeof(word);
				break;
			}

			size -= 1024;
		}
	}
#else
	// Allocate memory buffers
	for( int i=0; i<NUM_MAP_MEMORY_BUFFERS; i++ )
	{
		memory_buffer_t *buff = map_memory_buffers + i;

		buff->ptr = (word*)Sys_AllocMax(&buff->size);
		buff->alloc = buff->ptr;
		buff->end = buff->ptr + buff->size/sizeof(word);
	}

	if( map_memory_buffers[0].size <= 0 )
		all_ok = 0;

#endif

//	if( !all_ok )
//	{
//		while( 1 )
//			COLOR00 = VHPOSR;
//	}

}

// API
int Exit(void)
{
#if SYSTEM_PRESENT
	if( blockmem1 ) Sys_FreeMem(blockmem1, 0x10000);
	if( blockmem2 ) Sys_FreeMem(blockmem2, 0x10000);
	if( blockmem3 ) Sys_FreeMem(blockmem3, 0x10000);

	for( int i=0; i<NUM_MAP_MEMORY_BUFFERS; i++ )
		if( map_memory_buffers[i].ptr )
			Sys_FreeMem(map_memory_buffers[i].ptr, map_memory_buffers[i].size);
#endif

	return 0;
}

void Main_Init(void)
{
	DMACON = 0x0000 | (1<< 7);	// disable copper
	//DMACON = 0x8000 | (1<<10);	// blitter nasty
	DMACON = 0x0000 | (1<< 5);	// disable sprites

	BPLCON0 = 0x0200;
	BPLCON1 = 0;
	BPLCON2 = 0x0024;
	BPLCON3 = 0x0C20;
	COLOR00 = 0;
	COLOR01 = 0;

	Init_Copper();

	// init null sprite
	NULL_SPRITE[0] = 0x0000;
	NULL_SPRITE[1] = 0x0000;

	// XSTRT	=	129
	// XSTOP	=	129+320
	// YSTRT	=	44
	// YSTOP	=	44+256
	// HSTRT	=	129
	// WIDTH	=	320		; actual bpl width (excluding modulos)
	// RES		=	8		; 8=lores, 4=hires
	// DIWSTRT = XSTRT+(YSTRT*256);
	// DIWSTOP = (XSTOP-256)+(YSTOP-256)*256;
	// DDFSTRT = (HSTRT/2-RES);
	// DDFSTOP = (HSTRT/2-RES)+(8*((WIDTH/16)-1));

	if( opt_ntsc )
	{
		// NTSC
		DIWSTRT = 0x1181; //0x2C81;
		DIWSTOP = 0x05C1; //0xF4C1;
		DDFSTRT = 56;
		DDFSTOP = 208;
	}
	else
	{
		// PAL
		DIWSTRT = 0x2C81;
		DIWSTOP = 0x2CC1;
		DDFSTRT = 56;
		DDFSTOP = 208;
	}

	DMACON = 0x8000 | (1<< 8);	// enable bitplanes
	DMACON = 0x8000 | (1<< 7);	// enable copper
	DMACON = 0x8000 | (1<< 6);	// enable blitter

	SetCopper(BLANK_COPPER);

	CIAA_CRA = 0;
	CIAA_TALO = 0xFF;
	CIAA_TAHI = 0xFF;
	CIAA_CRA = 0x13;		// start timer A, force reload, count CNT pulses


#if MULTIPLAYER_MODE
	Comm_Init();
#endif
}





// API
void Main(void)
{
	BPLCON0 = 0x0200;

	if( !all_ok )
	{
		COLOR00 = 0xF00;
		return;
	}

	Main_Init();

	if( !all_ok )
	{
		COLOR00 = 0xFF0;
		return;
	}

	init_sincos();

	SetCopper(BLANK_COPPER);

	FX_DreadTest();


	//while( CIAA_PRA & (1<<6) ){}
}

// API
#if WEAPONSPRITES
extern const WeaponSpriteAnimFrame WEAPONSPRITEANIM_SP_PISTOL_FIRE[];
const WeaponSpriteAnimFrame *weaponsprite_anim;
short weaponsprite_delay;
#endif

void InterruptMain(void)
{
#if WEAPONSPRITES && 0

	// interrupt weapon logic
	//IO_CheckMouse();			//|| !(CIAA_PRA & (1<<6)) )
	//if( io_mouse_state )
	//{
	//	e_globals.pulse = io_mouse_state;
	//	io_mouse_state = 0;
	//	Script_OnPulse(view_weapon);
	//}

//	if( view_weapon->timer_attack )
//		view_weapon_swing = 0;
//	else if( view_movement )
//		view_weapon_swing += 6>>1;		// weapon swing		(ticks)
//	else if( view_weapon_swing & 0x7F )
//	{
//		word pswing = view_weapon_swing;
//		if( view_weapon_swing & 0x40 )	view_weapon_swing += 6;	//ticks;
//		else							view_weapon_swing -= 6;	//ticks;
//		if( (pswing^view_weapon_swing) & 0x80 )
//			view_weapon_swing = 0;
//	}

	// update weapon sprite
	if( copper_sprite_update && weaponsprite_anim )
	{
		//weaponsprite_delay += 6;
		//while( weaponsprite_delay >= weaponsprite_anim->delay )
		//{
		//	weaponsprite_delay -= weaponsprite_anim->delay;
		//	weaponsprite_anim++;
		//	if( weaponsprite_anim->delay < 0 )
		//		weaponsprite_anim += weaponsprite_anim->delay/(int)sizeof(WeaponSpriteAnimFrame);
		//}

		// update sprite
		const WeaponSpriteInfo *ws = weaponsprite_anim->gfx;
		word *dst = copper_sprite_update;
		//for( int i=0; i<15; i++ )
		//{
		//	*dst = ws->palette[i];
		//	dst += 2;
		//}
		dst += 2*15;

		word xp = weaponsprite_anim->xpos + (word)(sincos_fix14[(view_weapon_swing+64)&0xFF]>>11);				// cos(x+1/8)		x=32
		word yp = weaponsprite_anim->ypos + (word)(sincos_fix14[(byte)((view_weapon_swing<<1)+64)]>>11);		// cos(2x)			x=32	x=96	x=160	x=224


		for( int i=0; i<4; i++ )
		{
			yp += ws->sprites[i].ydelta;
		
			const word *ptr = ws->sprites[i].data0;
			//dst[0] = (dword)ptr;
			//dst[2] = ((dword)ptr)>>16;
			//dst[4] = (yp<<8) | (xp>>1);
			//dst[6] = 0xFF00 | (xp&1);
			dst += 8;
		
			ptr = ws->sprites[i].data1;
			//dst[0] = (dword)ptr;
			//dst[2] = ((dword)ptr)>>16;
			//dst[4] = (yp<<8) | (xp>>1);
			//dst[6] = 0xFF80 | (xp&1);
			dst += 8;
		
			xp += 16;
		}
	}
#endif

	//intnum++;
	//rawintnum++;
	
	//// Handle mouse
	//static word prev_joydat;
	//word joydat = JOY0DAT;
	//signed char xdelta = (signed char)joydat - (signed char)prev_joydat;
	//signed char ydelta = (signed char)(joydat>>8) - (signed char)(prev_joydat>>8);
	//
	//debug_vars[debug_target_index] += xdelta;
	//debug_vars[debug_target_index+1] += ydelta;
	//
	//prev_joydat = joydat;

	//io_mouse_state |= !(CIAA_PRA & (1<<6));



	//// Handle keyboard
	//static byte _data = 0xFF;
	//byte data = CIAA_SDR;
	//if( data!=_data )
	//{
	//	_data = data;
	//
	//	// acknowledge
	//	CIAA_CRA |= (1<<6);
	//	for( word n=0; n<3; n++ )
	//	{
	//		word pos = VHPOSR_B;
	//		while( VHPOSR_B == pos ) {}
	//	}
	//	CIAA_CRA &= ~(1<<6);
	//
	//	// process keycode
	//	data = ~((data>>1) | (data<<7));
	//	io_key_last = data;
	//
	//	if( data&0x80 ) io_keystate[data&0x7F] = 0;
	//	else			io_keystate[data] = 1;
	//}

	//Track_ISR_Poll();



}
