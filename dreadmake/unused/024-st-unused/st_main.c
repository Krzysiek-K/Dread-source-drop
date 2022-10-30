
#include "demo.h"
#include "st_framework.h"




word SOUND_DSPLPAIN;
word SOUND_DSPSTART;
word SOUND_DSBAREXP;
word SOUND_DSRLAUNC;
word SOUND_DSWPNUP;
word SOUND_DSDBLOAD;
word SOUND_DSITEMUP;



word _screen[320*200/16*4*2+256];
word *screen;
word *screen_active;
short opt_mouse_sens;

word render_screen[160*100/2];


word SOUND_DSPISTOL[1];
word SOUND_DSSHOTGN[1];
word SOUND_DSPOPAIN[1];
word SOUND_DSPODTH1[1];


#define VID_SCREEN_HIGH		(*(byte*)0xFFFF8201)
#define VID_SCREEN_MED		(*(byte*)0xFFFF8203)
#define VID_PAL				( (word*)0xFFFF8240)
#define VID_RESOLUTION		(*(byte*)0xFFFF8260)



/*const*/ word PAL[16] ={
	0x000,		//	black
	0x222,		//	gray ramp
	0x444,		//	|
	0x666,		//	|
	0x888,		//	|
	0xCCC,		//	|
	0xEEE,		//	white
	0x242,		//	green ramp
	0x462,		//	|
	0x664,		//	|
	0x004,		//	blue
	0x420,		//	brown
	0x442,		//	brown
	0xCA4,		//	yellow
	0xC00,		//	red
	0x600,		//	|

	//0x000,	// black			0000	OK
	//0x222,	// gray ramp		0001	OK
	//0x333,	// |
	//0x555,	// |
	//0x777,	// |
	//0xBBB,	// |
	//0xFFF,	// white
	//0x343,	// green ramp
	//0x573,	// |
	//0x774,	// |
	//0x004,	// blue				1010	OK
	//0x320,	// brown
	//0x431,	// brown
	//0xCA5,	// yellow
	//0xC00,	// 
	//0x600,	// 
};




void set_pixel(word x,word y,word c)
{
	word *p = screen + ((y*(320/16)+(x>>4))<<2);
	if(c&1) p[0] |= 0x8000>>(x&15);
	if(c&2) p[1] |= 0x8000>>(x&15);
	if(c&4) p[2] |= 0x8000>>(x&15);
	if(c&8) p[3] |= 0x8000>>(x&15);
}

void render_pixel(word x,word y,word c)
{
	byte *dst= (byte*)render_screen;
	dst += (x&1);
	dst += (x>>1)*(200);

	dst[y<<1] = c;
}


int main(int argc, char **argv)
{
	// system setup
	set_supervisor();
	//while(1){}

	// screen setup
	screen = (word*)( (((int)_screen)|0xFF)+1 );
	screen_active = screen + (320*200*4/16);
	VID_SCREEN_HIGH = (byte)( ((int)screen_active)>>16 );
	VID_SCREEN_MED = (byte)( ((int)screen_active)>>8 );
	VID_RESOLUTION = 0;

	for(word i=0;i<16;i++)
		VID_PAL[i] = (PAL[i]>>1)&0x777;


	init_sincos();
	init_c2p_lut();

	Dread_Build_Fillers();
	Dread_Build_Scalers();
	Dread_SoftInit();
	Map_LoadAll(2);

#if 0
	// draw test
	for(word x=0;x<100;x++)
		render_pixel(99-x, x, (x&15)<<2);
	//set_pixel(99-x,x,x&15);

	//// test render
	//for( int i=0; i<3*128; i++ )
	//	e_skymap[i] = SKYTEX_DATA + ((i&127)<<6);


	for(int y=0;y<10;y++)
	{
		render_screen[100+y] = (y+4)*0x0404;
		render_screen[200+y] = (y+4)*0x0404;
		render_screen[300+y] = (y+4)*0x0404;
		render_screen[400+y] = (y+4)*0x0404;
		render_screen[500+y] = (y+4)*0x0404;
		render_screen[600+y] = (y+4)*0x0404;
	}

	//render_screen[0] = 0x3438;
	render_screen[0] = 0x0038;
	render_screen[3] = 0x0038;
	render_screen[100] = 0x0038;
	render_screen[200] = 0x0038;
	render_screen[300] = 0x0038;
	render_screen[400] = 0x0038;
	render_screen[500] = 0x0038;
	render_screen[600] = 0x0038;
	render_screen[700] = 0x0038;
	render_screen[101] = 0x0034;
	render_screen[202] = 0x0034;
	render_screen[303] = 0x0034;
	render_screen[404] = 0x0034;
	render_screen[505] = 0x0034;
	render_screen[606] = 0x0034;
	render_screen[707] = 0x0034;

	render_pixel(0, 5, 14<<2);
	render_pixel(2, 6, 14<<2);
	render_pixel(4, 7, 14<<2);
	render_pixel(6, 8, 14<<2);
	render_pixel(8, 9, 14<<2);

	screen = screen_active;
	while(1)
	{
		perform_c2p();
		render_screen[0] ^= 0x3838;
		render_screen[1] = (render_screen[1]+0x0404)&0x3C3C;
		if( !render_screen[1] )
		{
			render_screen[2] = (render_screen[2]+0x0404)&0x3C3C;
			render_screen[3] ^= 0x3838;
		}
	}
#endif

	st_install_vblank();
	set_usermode();

	ikbd_install_handler();

	debug_vars[1] = 0;
	debug_vars[2] = 0;
	opt_mouse_sens = 16;

	//byte comm[3] ={ 0x08 };
	//ikbd_write(comm, 1);
	ikbd_qwrite(0x08000000, 1);			// mouse in relative mode
	ikbd_qwrite(0x0B040400, 3);			// set threshold to 4 pulses

	// dread
	while( 1 )
	{
		render_buffer = (byte*)render_screen;

		Dread_WeaponLogic();
		Dread_SoftTest();

		perform_c2p();

		//debug_write_hexbuff(screen, 8, ikbd_rxbuff);
		//debug_write_hexbuff(screen+80*6, 4, (byte*)&debug_vars);
		//debug_write_hexbuff(screen+80*6*2, 4, (byte*)&debug_vars[1]);
		//debug_write_hexbuff(screen+80*6*3, 1, (byte*)&io_mouse_state);


		word *tmp = screen;
		screen = screen_active;
		screen_active = tmp;

		set_supervisor();
		VID_SCREEN_HIGH = (byte)(((int)screen_active)>>16);
		VID_SCREEN_MED = (byte)(((int)screen_active)>>8);
		set_usermode();

		debug_vars[1] += 1<<4;
	}

	set_supervisor();
	st_remove_vblank();
	set_usermode();

	ikbd_remove_handler();


	return 0;
}

