
#include "demo.h"

#if AMIGA_AUDIO_ENGINE_VERSION == 1


word aud_mode = AUD_MODE_SFX_STEREO_2CH;
word aud_filter_mode;
word aud_channel_count;
volatile word aud_volume_update_request;



AudioChannel Aud_Channels[4];
//={
//	{
//		(word*)0xdff000, 0x0080, 0x0001, 0, 0,		
//		(word*)0xdff010, 0x0100, 0x0002, 0, 0,		
//		0x0180, 0x0003, 0, 0, 0, 0
//	},
//	{
//		(word*)0xdff030, 0x0400, 0x0008, 0, 0,		// 3: left
//		(word*)0xdff020, 0x0200, 0x0004, 0, 0,		// 2: right
//		0x0600, 0x000C, 0, 0, 0, 0
//	}
//};

const void *Aud_Hardware_Channels[4];
//={
//	&Aud_Channels[0].reg_base_1,	// 0: left
//	&Aud_Channels[0].reg_base_2,	// 1: right
//	&Aud_Channels[1].reg_base_2,	// 2: right
//	&Aud_Channels[1].reg_base_1,	// 3: left
//};



// Assign one or two hardware channels to an audio channel.
// (ch2 = -1 to disable)
void Aud_CreateChannel(int ch1, int ch2)
{
	AudioChannel *ch = &Aud_Channels[aud_channel_count++];

	ch->reg_base_1 = (word*)(0xdff000+ch1*0x10);
	ch->irq_mask_1 = 0x0080 << ch1;
	ch->dma_mask_1 = 0x0001 << ch1;
	Aud_Hardware_Channels[ch1] = &ch->reg_base_1;


	if( ch2 >= 0 )
	{
		ch->reg_base_2 = (word*)(0xdff000+ch2*0x10);
		ch->irq_mask_2 = 0x0080 << ch2;
		ch->dma_mask_2 = 0x0001 << ch2;
		Aud_Hardware_Channels[ch2] = &ch->reg_base_2;
	}

	ch->combined_irq_mask = ch->irq_mask_1 | ch->irq_mask_2;
	ch->combined_dma_mask = ch->dma_mask_1 | ch->dma_mask_2;
}

//void Aud_CreateChannel(int ch1, int ch2)
//{
//	_Aud_CreateChannel(ch1, ch2);
//}




// Resets the sound system - mutes everything, clears all internal state and sets the system up according to current configuration.
void Aud_Reset(void)
{
	word sr = Critical_Begin();		// Make sure nothing triggers inbetween.


	AUD0VOL = 0;			// Mute audio
	AUD1VOL = 0;
	AUD2VOL = 0;
	AUD3VOL = 0;

	INTENA = 0x0780;		// Disable all audio interrupts
	INTREQ = 0x0780;		// ACK all audio interrupts
	INTREQ = 0x0780;		// (compatbility)
	DMACON = 0x000F;		// Disable all audio DMA

	// Clear all internals.
	aud_channel_count = 0;
	aud_volume_update_request = 0;
	memset(Aud_Channels, 0, sizeof(Aud_Channels));
	memset(Aud_Hardware_Channels, 0, sizeof(Aud_Hardware_Channels));


	// Create channels.
	switch( aud_mode )
	{
	default:
	case AUD_MODE_SFX_MONO_4CH:
		Aud_CreateChannel(0, -1);
		Aud_CreateChannel(1, -1);
		Aud_CreateChannel(2, -1);
		Aud_CreateChannel(3, -1);
		break;

	case AUD_MODE_SFX_STEREO_2CH:
		Aud_CreateChannel(0, 1);
		Aud_CreateChannel(3, 2);
		break;
	}

	Aud_Filter(aud_filter_mode);

	Critical_End(sr);		// Done, resume interrupts.
}


void Aud_Filter(byte state)
{
	CIAA_PRA = (CIAA_PRA & ~2) | (((~state)&1)<<1);
}


#endif
