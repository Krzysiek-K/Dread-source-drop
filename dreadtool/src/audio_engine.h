
#pragma once


class SoundAsset;


struct AudioEnginePlaybackPacket {
	enum {
		F_STOP					= 0x0001,
		F_END					= 0x0002,
		F_SET_SAMPLE			= 0x0004,
		F_UPDATE_VOLUME			= 0x0008,
		F_UPDATE_FREQUENCY		= 0x0010,
		F_DONE					= 0x0040,		// IRQ or main timer is done with this command (no next command in chain)
		F_IRQ_ENABLE			= 0x0080,		// set next packet as IRQ, clear pending IRQ, enable IRQ
	};

	uint16_t	flags				= 0;
	uint16_t	delay				= 0;		// 0: IRQ packet
	SoundAsset	*sample				= NULL;
	int16_t		volume				= 0;
	float		_freq_shift			= 0;

	uint16_t	period_mul			= 0;
	uint16_t	period_mul_fract	= 0;


};





// ================================================================ AmigaVoiceBase ================================================================


class AmigaVoiceBase {
public:
	bool	right_channel = false;


	virtual ~AmigaVoiceBase() {}
	virtual bool Render(float *buffer, int samples) = 0;
};


// ================================================================ AmigaSimpleVoice ================================================================


class AmigaSimpleVoice : public AmigaVoiceBase {
private:
	union sIntFlt { uint32_t U32; float F32; };

	int Pos;
	int PWMCnt, DivCnt;
	sIntFlt Cur;

public:
	int8_t* Sample;
	int SampleLen;
	int LoopLen;
	int Period;			// 124 .. 65535
	int Volume;			// 0 .. 64


	AmigaSimpleVoice() : Period(65535), Volume(0), Sample(0), Pos(0), PWMCnt(0), DivCnt(0), LoopLen(1) { Cur.F32=0; }
	virtual bool Render(float *buffer, int samples);
	void Trigger(int8_t *smp, int sl, int ll, int offs=0);
};


// ================================================================ AmigaTrackedVoice ================================================================


class AmigaTrackedVoice : public AmigaVoiceBase {
public:
	AudioEnginePlaybackPacket	*vis_active_timer_cmd = NULL;
	AudioEnginePlaybackPacket	*vis_active_irq_cmd = NULL;
	double						vis_active_timer_timestamp = 0.f;
	double						vis_active_irq_timestamp = 0.f;
	int							vis_sound_error = 0;


	virtual bool Render(float *buffer, int samples);
	void Trigger(AudioEnginePlaybackPacket *track, AudioEnginePlaybackPacket *track_end);


private:
	union sIntFlt { uint32_t U32; float F32; };

	// Audio engine simulation
	AudioEnginePlaybackPacket	*timer_cmd = NULL;
	AudioEnginePlaybackPacket	*irq_cmd = NULL;
	AudioEnginePlaybackPacket	*cmd_end = NULL;
	int							base_period = 65535;
	int							t1_entry_count = 0;
	int							t1_delay = 0;
	int							t2_delay = 0;

	// Paula channel registers
	int8_t						*AUDxLC = NULL;
	int							AUDxLEN = 1;			// in samples, not words
	int							AUDxPER = 65535;
	int							AUDxVOL = 64;
	bool						DMA_on = false;
	bool						IRQ_on = false;
	
	// Paula channel internals
	int8_t						*data_pointer = NULL;
	int							data_length = 0;
	int							pwm_cnt = 0;
	int							div_cnt = 0;
	sIntFlt						cur;


	void ChannelRender(float *buffer, int samples);
	void Tick_T1();
	void Tick_T2();
	void AudioIrq();
	bool ExecutePacket_Stop(AudioEnginePlaybackPacket *pk);
	void ExecutePacket_Play(AudioEnginePlaybackPacket *pk);
};




// ================================================================ AmigaPaula ================================================================


class AmigaPaula
{
public:

	static const int FIR_WIDTH=512;
	float FIRMem[2*FIR_WIDTH+1];

	vector<AmigaVoiceBase*>		Voices;

	// settings
	float MasterVolume;
	float MasterSeparation;


	// rendering in paula freq
	static const int RBSIZE = 4096;
	float RingBuf[2*RBSIZE];
	int WritePos;
	int ReadPos;
	float ReadFrac;
	//float FltFreq;
	//float FltBuf;
	bool voices_active;

	void CalcFrag(float *out, int samples);
	void Calc(); // todo: stereo

	// rendering in output freq
	void Render(float *outbuf, int samples);

	AmigaPaula();


	void RemoveAllChannels(bool mute);
	void PlaybackTick();
	void RenderToFile(const char *path);
};



extern AmigaPaula ami_paula;
