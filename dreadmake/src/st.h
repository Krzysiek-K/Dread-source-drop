
#ifndef _ST_H
#define _ST_H


// shut up, Visual...
#ifdef WIN32
	#define __reg(x)
	#define __chip
	#define USE_EMBEDDED_MAP_DATA			1
	#define ENABLE_MAP_FILE_LOADING			0
#endif

#define __d0	__reg("d0")
#define __d1	__reg("d1")
#define __d2	__reg("d2")
#define __d3	__reg("d3")
#define __d4	__reg("d4")
#define __d5	__reg("d5")
#define __d6	__reg("d6")
#define __d7	__reg("d7")
#define __a0	__reg("a0")
#define __a1	__reg("a1")
#define __a2	__reg("a2")
#define __a3	__reg("a3")
#define __a4	__reg("a4")
#define __a5	__reg("a5")
#define __a6	__reg("a6")


#define KEYSCAN_Q		0x10
#define KEYSCAN_W		0x11
#define KEYSCAN_E		0x12
#define KEYSCAN_R		0x13
#define KEYSCAN_T		0x14
#define KEYSCAN_Y		0x15
#define KEYSCAN_U		0x16
#define KEYSCAN_I		0x17
#define KEYSCAN_O		0x18
#define KEYSCAN_P		0x19

#define KEYSCAN_A		0x1E
#define KEYSCAN_S		0x1F
#define KEYSCAN_D		0x20
#define KEYSCAN_F		0x21
#define KEYSCAN_G		0x22
#define KEYSCAN_H		0x23
#define KEYSCAN_J		0x24
#define KEYSCAN_K		0x25
#define KEYSCAN_L		0x26

#define KEYSCAN_Z		0x2C
#define KEYSCAN_X		0x2D
#define KEYSCAN_C		0x2E
#define KEYSCAN_V		0x2F
#define KEYSCAN_B		0x30
#define KEYSCAN_N		0x31
#define KEYSCAN_M		0x32

#define KEYSCAN_ESC		0x01
#define KEYSCAN_TAB		0x0F

#define KEYSCAN_F1		0x3B		// this are expected to be sequential
#define KEYSCAN_F2		0x3C
#define KEYSCAN_F3		0x3D
#define KEYSCAN_F4		0x3E
#define KEYSCAN_F5		0x3F
#define KEYSCAN_F6		0x40
#define KEYSCAN_F7		0x41
#define KEYSCAN_F8		0x42
#define KEYSCAN_F9		0x43
#define KEYSCAN_F10		0x44

#define KEYSCAN_1		0x02		// this are expected to be sequential
#define KEYSCAN_2		0x03
#define KEYSCAN_3		0x04
#define KEYSCAN_4		0x05
#define KEYSCAN_5		0x06
#define KEYSCAN_6		0x07
#define KEYSCAN_7		0x08
#define KEYSCAN_8		0x09
#define KEYSCAN_9		0x0A
#define KEYSCAN_0		0x0B

#define KEYSCAN_SPACE	0x39

#define KEYSCAN_LBRACE	0x1A
#define KEYSCAN_RBRACE	0x1B

#define KEYSCAN_UP		0x48
#define KEYSCAN_DOWN	0x50
#define KEYSCAN_LEFT	0x4B
#define KEYSCAN_RIGHT	0x4D
#define KEYSCAN_ENTER	0x1C




#define VID_SCREEN_HIGH		(*(volatile byte*)0xFFFF8201)
#define VID_SCREEN_MED		(*(volatile byte*)0xFFFF8203)
#define VID_PAL				( (volatile word*)0xFFFF8240)
#define VID_RESOLUTION		(*(volatile byte*)0xFFFF8260)
#define VID_SYNC_MODE		(*(volatile byte*)0xFFFF820A)

#define TIMER_CD_CONTROL	(*(volatile byte*)0xFFFFFA1D)
#define TIMER_D_DATA		(*(volatile byte*)0xFFFFFA25)

#define USART_CONTROL		(*(volatile byte*)0xFFFFFA29)
#define USART_RX_STATUS		(*(volatile byte*)0xFFFFFA2B)
#define USART_TX_STATUS		(*(volatile byte*)0xFFFFFA2D)
#define USART_DATA			(*(volatile byte*)0xFFFFFA2F)



#define VID_SET_SCREEN(s)		( VID_SCREEN_HIGH = (byte)(((int)(s))>>16), VID_SCREEN_MED = (byte)(((int)(s))>>8) )




#endif
