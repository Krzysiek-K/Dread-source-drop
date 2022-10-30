
#include "common.h"
#include "app_rendertest.h"



AmigaPaula ami_paula;




const int PAULARATE=3546895;	// 3740000; // approx. pal timing
const int OUTRATE=44100;     // approx. pal timing
const int OUTFPS=50;         // approx. pal timing


//-------------------------------------------------------------------- ----------


const float sFPi = 3.141592f;

template<typename T> T sSqr(T v) { return v*v; }
template<typename T> T sLerp(T a, T b, float f) { return a+f*(b-a); }
template<typename T> T sAbs(T x) { return abs(x); }
template<typename T> T sMin(T a, T b) { return (a<b) ? a : b; }
template<typename T> T sMax(T a, T b) { return (a>b) ? a : b; }
template<typename T> T sClamp(T x, T a, T b) { return (x<a) ? a : (x>b ? b : x); }
inline float sFSqrt(float x) { return sqrtf(x); }
inline float sFSin(float x) { return sinf(x); }
inline float sFCos(float x) { return cosf(x); }
inline float sFSinc(float x) { return x ? sFSin(x)/x : 1; }
inline float sFHamming(float x) { return (x>-1 && x<1) ? sSqr(sFCos(x*sFPi/2)) : 0; }
inline float sFPow(float b, float e) { return powf(b, e); }
inline void sSetMem(void *dest, uint8_t v, int size) { memset(dest, v, size); }
inline void sCopyMem(void *dest, const void *src, int size) { memcpy(dest, src, size); }
inline void sZeroMem(void *dest, int size) { sSetMem(dest, 0, size); }



// ================================================================ AmigaSimpleVoice ================================================================


bool AmigaSimpleVoice::Render(float *buffer, int samples)
{
	if( !Sample || !Volume ) return false;

	uint8_t *smp=(uint8_t*)Sample;
	for( int i=0; i<samples; i++ )
	{
		if( !DivCnt )
		{
			// todo: use a fake d/a table for this
			Cur.U32=((smp[Pos]^0x80)<<15)|0x40000000;
			Cur.F32-=3.0f;
			if( ++Pos==SampleLen ) Pos-=LoopLen;
			DivCnt=Period;
		}
		if( PWMCnt<Volume ) buffer[i]+=Cur.F32;
		PWMCnt=(PWMCnt+1)&0x3f;
		DivCnt--;
	}

	return Pos<(SampleLen-1);
}

void AmigaSimpleVoice::Trigger(int8_t *smp, int sl, int ll, int offs)
{
	Sample=smp;
	SampleLen=sl;
	LoopLen=ll;
	Pos=sMin(offs, SampleLen-1);
}

// ================================================================ AmigaTrackedVoice ================================================================


bool AmigaTrackedVoice::Render(float *buffer, int samples)
{
	bool active = timer_cmd || irq_cmd;

	while( samples>0 )
	{
		int len = samples;
		if( t1_delay>0 && t1_delay < len ) len = t1_delay;
		if( t2_delay>0 && t2_delay < len ) len = t2_delay;

		ChannelRender(buffer, len);
		samples -= len;
		buffer += len;

		if( t1_delay>0 )
		{
			t1_delay -= len;
			if( t1_delay <= 0 )
				Tick_T1();
		}
		
		if( t2_delay>0 )
		{
			t2_delay -= len;
			if( t2_delay <= 0 )
				Tick_T2();
		}
	}

	return active;
}

void AmigaTrackedVoice::Trigger(AudioEnginePlaybackPacket *track, AudioEnginePlaybackPacket *track_end)
{
	timer_cmd = track;
	irq_cmd = NULL;
	cmd_end = track_end;
	t1_entry_count = 0;

	t1_delay = PAULARATE / 50;
	t2_delay = 0;
	DMA_on = false;
	IRQ_on = false;
	AUDxVOL = 64;

	data_pointer = NULL;

	vis_sound_error = 0;

	//AUDxLC = &track->sample->byte_data[0];
	//AUDxLEN = track->sample->byte_data.size();
	//AUDxPER = track->sample->paula_rate;
	//DMA_on = true;
}


void AmigaTrackedVoice::ChannelRender(float *buffer, int samples)
{
	if( !DMA_on )
	{
		data_pointer = NULL;
		return;
	}

	if( !data_pointer && AUDxLC )
	{
		data_pointer = AUDxLC;
		data_length = AUDxLEN;
		if( IRQ_on ) AudioIrq();
	}

	if( !data_pointer || AUDxVOL<=0 ) return;

	uint8_t *smp=(uint8_t*)AUDxLC;
	for( int i=0; i<samples; i++ )
	{
		if( !div_cnt )
		{
			div_cnt = AUDxPER;

			cur.F32 = *data_pointer++ / 128.f;

			if( --data_length <= 0 )
			{
				if( DMA_on && AUDxLC )
				{
					data_pointer = AUDxLC;
					data_length = AUDxLEN;
					if( IRQ_on ) AudioIrq();
				}
				else
				{
					data_pointer = NULL;
					data_length = 1;
					return;
				}
			}
		}
		if( pwm_cnt<AUDxVOL ) buffer[i]+=cur.F32;
		pwm_cnt=(pwm_cnt+1)&0x3f;
		div_cnt--;
	}
}

void AmigaTrackedVoice::Tick_T1()
{
	// Keep CIA timing
	t1_delay = PAULARATE / 50;					// T1 at 50Hz
	t2_delay = PAULARATE * 15 / 50 / 312;		// T2 after 15 scanlines

	// overrun check
	if( timer_cmd >= cmd_end )
	{
		vis_sound_error |= 0x001;		// T1 overrun
		return;
	}

	if( timer_cmd && !timer_cmd->delay )
	{
		vis_sound_error |= 0x002;		// T1 bad packet
		return;
	}

	// ---------------- TICK ROUTINE ----------------

	if( !timer_cmd )
		return;

	// Delay counter
	t1_entry_count++;
	if( t1_entry_count < timer_cmd->delay )
		return;

	// Execute Stop/End commands
	ExecutePacket_Stop(timer_cmd);

	// ---------------- End ----------------

	vis_active_timer_cmd = timer_cmd;
	vis_active_timer_timestamp = Dev.GetElapsedTime();
}

void AmigaTrackedVoice::Tick_T2()
{
	// overrun check
	if( timer_cmd >= cmd_end )
	{
		vis_sound_error |= 0x010;		// T2 overrun
		return;
	}

	if( timer_cmd && !timer_cmd->delay )
	{
		vis_sound_error |= 0x020;		// T2 bad packet
		return;
	}

	// ---------------- TOCK ROUTINE ----------------

	if( !timer_cmd )
		return;

	// Delay counter
	if( t1_entry_count < timer_cmd->delay )
		return;
	t1_entry_count = 0;


	// Execute play commands
	ExecutePacket_Play(timer_cmd);


	// Schedule IRQ program
	if( timer_cmd->flags & AudioEnginePlaybackPacket::F_IRQ_ENABLE )
	{
		irq_cmd = timer_cmd + 1;
		IRQ_on = true;
	}

	if( timer_cmd->flags & AudioEnginePlaybackPacket::F_DONE )
	{
		timer_cmd = NULL;
	}
	else
	{
		do {
			timer_cmd++;
			if( timer_cmd >= cmd_end )
			{
				vis_sound_error |= 0x040;		// T2 overrun while skipping IRQ tracks
				return;
			}
		} while( timer_cmd->delay==0 );
	}

	// ---------------- End ----------------
}

void AmigaTrackedVoice::AudioIrq()
{
	if( irq_cmd >= cmd_end )
	{
		vis_sound_error |= 0x100;			// TRQ overrun
		return;
	}
	
	if( irq_cmd && irq_cmd->delay )
	{
		vis_sound_error |= 0x200;			// IRQ bad packet
		return;
	}

	if( !irq_cmd )
	{
		vis_sound_error |= 0x400;			// IRQ no command
		IRQ_on = false;
		return;
	}

	vis_active_irq_cmd = irq_cmd;
	vis_active_irq_timestamp = Dev.GetElapsedTime();

	// ---------------- IRQ ROUTINE ----------------

	AudioEnginePlaybackPacket *cmd = irq_cmd++;		// shadow, because F_STOP might NULL it
	ExecutePacket_Stop(cmd);
	ExecutePacket_Play(cmd);

	if( cmd->flags & AudioEnginePlaybackPacket::F_DONE )
	{
		irq_cmd = NULL;
		IRQ_on = false;
	}

	// ---------------- End ----------------
}

bool AmigaTrackedVoice::ExecutePacket_Stop(AudioEnginePlaybackPacket *pk)
{
	// F_STOP
	if( pk->flags & AudioEnginePlaybackPacket::F_STOP )
	{
		DMA_on = false;
		IRQ_on = false;
	}

	// F_END
	if( pk->flags & AudioEnginePlaybackPacket::F_END )
	{
		timer_cmd = NULL;
		irq_cmd = NULL;
		return false;
	}

	return true;
}

void AmigaTrackedVoice::ExecutePacket_Play(AudioEnginePlaybackPacket *pk)
{
	// Set sample
	if( pk->flags & AudioEnginePlaybackPacket::F_SET_SAMPLE )
	{
		if( pk->sample && pk->sample->byte_data.size()>0 )
		{
			AUDxLC = &pk->sample->byte_data[0];
			AUDxLEN = pk->sample->byte_data.size();
			base_period = pk->sample->paula_rate;
		}
		else
		{
			static int8_t ZERO = 0;
			AUDxLC = &ZERO;
			AUDxLEN = 1;
		}
		DMA_on = true;
	}

	// Set volume
	if( pk->flags & AudioEnginePlaybackPacket::F_UPDATE_VOLUME )
	{
		AUDxVOL = pk->volume;
	}

	// Set frequency
	if( pk->flags & AudioEnginePlaybackPacket::F_UPDATE_FREQUENCY )
	{
		uint16_t period = uint16_t(mulu(base_period, pk->period_mul));
		period += uint16_t(mulu(base_period, pk->period_mul_fract) >> 16);

		if( period < 124 ) period = 124;
		//if( period > 65535 ) period = 65535;

		AUDxPER = period;
	}
}


// ================================================================ AmigaPaula ================================================================


void AmigaPaula::CalcFrag(float *out, int samples)
{
	sZeroMem(out, sizeof(float)*samples);
	sZeroMem(out+RBSIZE, sizeof(float)*samples);
	for( int i=0; i<Voices.size(); i++ )
	{
		if( Voices[i]->right_channel )
		{
			if( Voices[i]->Render(out+RBSIZE, samples) )
				voices_active = true;
		}
		else
		{
			if( Voices[i]->Render(out, samples) )
				voices_active = true;
		}
	}
}

void AmigaPaula::Calc() // todo: stereo
{
	int RealReadPos=ReadPos-FIR_WIDTH-1;
	int samples=(RealReadPos-WritePos)&(RBSIZE-1);

	voices_active = false;

	int todo=sMin(samples, RBSIZE-WritePos);
	CalcFrag(RingBuf+WritePos, todo);
	if( todo<samples )
	{
		WritePos=0;
		todo=samples-todo;
		CalcFrag(RingBuf, todo);
	}
	WritePos+=todo;
}

// rendering in output freq
void AmigaPaula::Render(float *outbuf, int samples)
{
	const float step=float(PAULARATE)/float(OUTRATE);
	const float pan=0.5f+0.5f*MasterSeparation;
	const float vm0=MasterVolume*sFSqrt(pan);
	const float vm1=MasterVolume*sFSqrt(1-pan);

	for( int s=0; s<samples; s++ )
	{
		int ReadEnd=ReadPos+FIR_WIDTH+1;
		if( WritePos<ReadPos ) ReadEnd-=RBSIZE;
		if( ReadEnd>WritePos ) Calc();
		float outl0=0, outl1=0;
		float outr0=0, outr1=0;

		// this needs optimization. SSE would come to mind.
		int offs=(ReadPos-FIR_WIDTH-1)&(RBSIZE-1);
		float vl=RingBuf[offs];
		float vr=RingBuf[offs+RBSIZE];
		for( int i=1; i<2*FIR_WIDTH-1; i++ )
		{
			float w=FIRMem[i];
			outl0+=vl*w;
			outr0+=vr*w;
			offs=(offs+1)&(RBSIZE-1);
			vl=RingBuf[offs];
			vr=RingBuf[offs+RBSIZE];
			outl1+=vl*w;
			outr1+=vr*w;
		}
		float outl=sLerp(outl0, outl1, ReadFrac);
		float outr=sLerp(outr0, outr1, ReadFrac);
		*outbuf++=vm0*outl+vm1*outr;
		*outbuf++=vm1*outl+vm0*outr;

		ReadFrac+=step;
		int rfi=int(ReadFrac);
		ReadPos=(ReadPos+rfi)&(RBSIZE-1);
		ReadFrac-=rfi;
	}
}

AmigaPaula::AmigaPaula()
{
	// make FIR table
	float *FIRTable=FIRMem+FIR_WIDTH;
	float yscale=float(OUTRATE)/float(PAULARATE);
	float xscale=sFPi*yscale;
	float fsum = 0;
	for( int i=-FIR_WIDTH; i<=FIR_WIDTH; i++ )
		fsum += FIRTable[i] = sFSinc(float(i)*xscale)*sFHamming(float(i)/float(FIR_WIDTH-1));

	for( int i=-FIR_WIDTH; i<=FIR_WIDTH; i++ )
		FIRTable[i] /= fsum;


	sZeroMem(RingBuf, sizeof(RingBuf));
	ReadPos=0;
	ReadFrac=0;
	WritePos=FIR_WIDTH;

	MasterVolume=0.9f;
	MasterSeparation=0.0f;
	//FltBuf=0;

	voices_active = false;
}



#define SAMPLE_RATE     44100
#define BUFF_SAMPLES    10000


int synth_time;
int synth_fill_block = 0;
bool is_playing = false;

HWAVEOUT	hWaveOut;
short       sbuff[BUFF_SAMPLES][2];
float		fbuff[BUFF_SAMPLES][2];
int			sbuff_pos = 1;

WAVEFORMATEX WaveFMT =
{
	WAVE_FORMAT_PCM,
	2, // channels
	SAMPLE_RATE, // samples per sec
	SAMPLE_RATE*sizeof(short)*2, // bytes per sec
	4, // block alignment;
	16, // bits per sample
	0 // extension not needed
};

WAVEHDR WaveHDR =
{
	(LPSTR)sbuff,
	BUFF_SAMPLES*sizeof(short)*2,
	0,
	0,
	WHDR_BEGINLOOP | WHDR_ENDLOOP,
	-1,
	0,
	0
};


void AmigaPaula::RemoveAllChannels(bool mute)
{
	for( int i=0; i<Voices.size(); i++ )
		delete Voices[i];
	Voices.clear();

	// mute voice
	if( mute )
		memset(sbuff, 0, sizeof(sbuff));
}


void AmigaPaula::PlaybackTick()
{
	static bool open = false;
	static MMTIME MMTime ={ TIME_SAMPLES, 0 };

	int reqlen = (MMTime.u.sample-sbuff_pos+BUFF_SAMPLES) % BUFF_SAMPLES;

	// render
	Render(&fbuff[0][0], reqlen);

	int fpos = 0;
	while( sbuff_pos!=MMTime.u.sample )
	{
		sbuff[sbuff_pos][0] = short(sClamp(fbuff[fpos][0], -1.f, 1.f)*32000);
		sbuff[sbuff_pos][1] = short(sClamp(fbuff[fpos][1], -1.f, 1.f)*32000);
		fpos++;
		sbuff_pos++;
		sbuff_pos %= BUFF_SAMPLES;
	}
	assert(fpos==reqlen);

	if( !open )
	{
		waveOutOpen(&hWaveOut, WAVE_MAPPER, &WaveFMT, NULL, 0, CALLBACK_NULL);
		waveOutPrepareHeader(hWaveOut, &WaveHDR, sizeof(WaveHDR));
		waveOutWrite(hWaveOut, &WaveHDR, sizeof(WaveHDR));
		open = true;
	}

	waveOutGetPosition(hWaveOut, &MMTime, sizeof(MMTIME));
	MMTime.u.sample %= BUFF_SAMPLES;
}

void AmigaPaula::RenderToFile(const char *path)
{
#define MAKEFOURCC(ch0, ch1, ch2, ch3)  ((DWORD)(byte)(ch0) | ((DWORD)(byte)(ch1) << 8) |  ((DWORD)(byte)(ch2) << 16) | ((DWORD)(byte)(ch3) << 24 ))

	DWORD wavHeader[11] ={
		MAKEFOURCC('R','I','F','F'),
		36, // +SONG_SAMPLES*2
		MAKEFOURCC('W','A','V','E'),
		MAKEFOURCC('f','m','t',' '),
		0x10,
		0x00010001,
		44100,
		2*44100,
		0x00100002,
		MAKEFOURCC('d','a','t','a'),
		0  // +SONG_SAMPLES*2
	};

	sZeroMem(RingBuf, sizeof(RingBuf));
	ReadPos=0;
	ReadFrac=0;
	WritePos=FIR_WIDTH;

	// Render
	const int CHUNK_SIZE = 256;
	vector<int16_t> audio_data;
	float buff[CHUNK_SIZE][2];
	
	int chunks_to_do = 1;
	bool enabled = false;
	while( chunks_to_do-->0 )
	{
		Render(&buff[0][0], CHUNK_SIZE);

		for( int i=0; i<CHUNK_SIZE; i++ )
		{
			short s = short(sClamp(buff[i][0], -1.f, 1.f)*32000);
			if( s ) enabled = true;
			if( enabled )
				audio_data.push_back(s);
		}

		if( voices_active )
			chunks_to_do = 3;
	}

	// Trim
	while( audio_data.size()>=2 && audio_data[audio_data.size()-2]==audio_data[audio_data.size()-1] )
		audio_data.pop_back();

	// Save
	wavHeader[1] += audio_data.size()*sizeof(int16_t);
	wavHeader[10] += audio_data.size()*sizeof(int16_t);

	FILE *fp = fopen(path, "wb");
	if( fp )
	{
		fwrite(wavHeader, 1, sizeof(wavHeader), fp);
		fwrite(&audio_data[0], 1, audio_data.size()*sizeof(int16_t), fp);
		fclose(fp);
	}

}
