
#ifndef _AMIGA_AUDIO_H
#define _AMIGA_AUDIO_H

#if AMIGA_AUDIO_ENGINE_VERSION == 1



// amiga_audio_c.c


typedef struct _AudioChannel AudioChannel;





//$asmstruct aud	AMIGA_AUDIO_ENGINE_VERSION=1
struct _AudioChannel {
										// MONO / LEFT hardware channel				(AudioHardwareChannel*)
	word			*reg_base_1;			// CHIP_BASE + $10*index
	word			irq_mask_1;				// INTENA/INTREQ mask
	word			dma_mask_1;				// DMAEN mask
	byte			state_1;				// channel state	AUDSTATE_*
	byte			volume_1;				// spatial volume - 0..64

										// RIGHT hardware channel					(AudioHardwareChannel*)
	word			*reg_base_2;			// CHIP_BASE + $10*index
	word			irq_mask_2;				// INTENA/INTREQ mask
	word			dma_mask_2;				// DMAEN mask
	byte			state_2;				// channel state	AUDSTATE_*
	byte			volume_2;				// spatial volume - 0..64

										// virtual channel
	word			combined_irq_mask;		// INTENA/INTREQ mask			(combined)
	word			combined_dma_mask;		// DMAEN mask					(combined)
	word			*pending_sample;		//
	void			*thing_owner;			//
	word			last_time;				//
	word			attenuation;			// spatial attenuation, squared, shifted	[ (attn*attn)>>8 ]
};


#define AUD_MODE_SFX_MONO_4CH		0
#define AUD_MODE_SFX_STEREO_2CH		1
#define AUD_MODE_MAX				1


extern word aud_mode;
extern word aud_filter_mode;
extern word aud_channel_count;
extern volatile word aud_volume_update_request;



void Aud_Reset(void);
void Aud_Filter(byte state);

#define Aud_VolumeUpdateRequest()				(aud_volume_update_request = 1)


void Sound_PlayThingSound(__a6 EngineThing *thing, __a0 const word *sound, __d7 int attenuation);



#endif

#endif
