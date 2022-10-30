
#include "demo.h"


AudioChannel Aud_Channels[4] = {
	{ (word*)0xdff000, 0x0080, 0x0001, 0, 0, 0, 0, 0 },
	{ (word*)0xdff010, 0x0100, 0x0002, 0, 0, 0, 0, 0 },
	{ (word*)0xdff020, 0x0200, 0x0004, 0, 0, 0, 0, 0 },
	{ (word*)0xdff030, 0x0400, 0x0008, 0, 0, 0, 0, 0 },
};




void Aud_Cleanup(void)
{
	word sr = Critical_Begin();

	for( int i=0; i<4; i++ )
		Aud_Channels[i].thing_owner = NULL;

	Critical_End(sr);
}
