
#ifndef _AMIGA_AUDIO2_H
#define _AMIGA_AUDIO2_H

#if AMIGA_AUDIO_ENGINE_VERSION == 2



// amiga_audio2_c.c


typedef struct _AudioTrackRow AudioTrackRow;
typedef struct _AudioChannel AudioChannel;
//typedef struct _EngineThing		EngineThing;




#define AUD_ROW_FLAG_STOP				0x01		// #0
#define AUD_ROW_FLAG_END				0x02		// #1
#define AUD_ROW_FLAG_SET_SAMPLE			0x04		// #2		
#define AUD_ROW_FLAG_UPDATE_VOLUME		0x08		// #3		
#define AUD_ROW_FLAG_UPDATE_FREQUENCY	0x10		// #4		

#define AUD_ROW_FLAG_DONE				0x40		// #6		IRQ or main timer is done with this command (no next command in chain)
#define AUD_ROW_FLAG_IRQ_ENABLE			0x80		// #7		set next packet as IRQ, clear pending IRQ, enable IRQ


//$asmstruct  audtrak	AMIGA_AUDIO_ENGINE_VERSION=2
struct _AudioTrackRow {
	word		delay;
	byte		flags;
	byte		volume;
	word		*sample;
	word		period_mul;
	word		period_mul_fract;
};


//$asmstruct aud	AMIGA_AUDIO_ENGINE_VERSION=2
struct _AudioChannel {
	
	word			*reg_base_1;			//		CHIP_BASE + $10*index			[ MONO / LEFT ]
	word			*reg_base_2;			//		CHIP_BASE + $10*index			[ RIGHT ]
	word			irq_mask;				//		INTENA/INTREQ mask				(only second channel)
	word			dma_mask;				//		DMAEN mask						(combined)

	word			base_period;			//		Sample base period
	byte			volume;					//
	byte			stereo_mode;			//

										// virtual channel
	void			*thing_owner;			//
	word			last_time;				//
	word			attenuation;			//		spatial attenuation, squared, shifted	[ (attn*attn)>>8 ]

										// tracked samples
	AudioTrackRow	*timer_cmd;				//		Timer command to execute
	AudioTrackRow	*irq_cmd;				//		IRQ command to execute
	word			t1_entry_count;			//		T1 delay

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


// asm
void Sound_PlayThingSound(__a6 void /*EngineThing*/ *thing, __a0 const AudioTrackRow *sound, __d7 int attenuation);



#endif

#endif
