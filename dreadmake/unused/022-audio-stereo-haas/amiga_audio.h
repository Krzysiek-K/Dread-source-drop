
#ifndef _AMIGA_AUDIO_H
#define _AMIGA_AUDIO_H



// amiga_audio.s
extern word aud_volume_update_request;


#define Aud_VolumeUpdateRequest()				(aud_volume_update_request = 1)



// amiga_audio_c.c


typedef struct _AudioChannel AudioChannel;



//$asmstruct aud
struct _AudioChannel {
	word			*custom_base;			// CHIP_BASE + $10*index
	word			irq_mask;				// INTENA/INTREQ mask
	word			dma_mask;				// DMAEN mask
	word			*pending_sample;		//
	void			*thing_owner;			//
	word			last_time;				//
	byte			state;					// channel state	AUDSTATE_*
	byte			_pad;					//
	word			attenuation;			// spatial attenuation, squared, shifted	[ (attn*attn)>>8 ]
	byte			volume;					// spatial volume - 0..64		(mono / right)
	byte			volume2;				// spatial volume - 0..64		(left)
};



//struct _AudioHardwareChannel {
//	word			*custom_base;			// CHIP_BASE + $10*index
//	word			irq_mask;				// INTENA/INTREQ mask
//	word			dma_mask;				// DMAEN mask
//	byte			state;					// channel state	AUDSTATE_*
//	byte			volume;					// spatial volume - 0..64		(mono / right)
//};
//
//struct _AudioVirtualChannel {
//	AudioHardwareChannel	*chan;				//
//	AudioHardwareChannel	*chan2;				//
//	word					irq_mask;			// INTENA/INTREQ mask
//	word					dma_mask;			// DMAEN mask
//	word					*pending_sample;	//
//	void					*thing_owner;		//
//	word					last_time;			//
//	byte					state;				// channel state	AUDSTATE_*
//	byte					_pad;				//
//	word					attenuation;		// spatial attenuation, squared, shifted	[ (attn*attn)>>8 ]
//};


void Aud_Cleanup(void);




#endif
