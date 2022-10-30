
#include "common.h"
#include "app_rendertest.h"



AmigaPaula ami_paula;




const int PAULARATE=3546895;	// 3740000; // approx. pal timing
const int OUTRATE=44100;     // approx. pal timing
const int OUTFPS=50;         // approx. pal timing


//-------------------------------------------------------------------- ----------


const float sFPi=3.141592f;	// 4*atanf(1);

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




// ================================================================ AmigaPaula::Voice ================================================================


void AmigaPaula::Voice::Render(float *buffer, int samples)
{
	if( !Sample || !Volume ) return;

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
}

void AmigaPaula::Voice::Trigger(int8_t *smp, int sl, int ll, int offs)
{
	Sample=smp;
	SampleLen=sl;
	LoopLen=ll;
	Pos=sMin(offs, SampleLen-1);
}



// ================================================================ AmigaPaula ================================================================


void AmigaPaula::CalcFrag(float *out, int samples)
{
	sZeroMem(out, sizeof(float)*samples);
	sZeroMem(out+RBSIZE, sizeof(float)*samples);
	for( int i=0; i<4; i++ )
	{
		if( i==1 || i==2 )
			V[i].Render(out+RBSIZE, samples);
		else
			V[i].Render(out, samples);
	}
}

void AmigaPaula::Calc() // todo: stereo
{
	int RealReadPos=ReadPos-FIR_WIDTH-1;
	int samples=(RealReadPos-WritePos)&(RBSIZE-1);

	int todo=sMin(samples, RBSIZE-WritePos);
	CalcFrag(RingBuf+WritePos, todo);
	if( todo<samples )
	{
		WritePos=0;
		todo=samples-todo;
		CalcFrag(RingBuf, todo);
	}
	WritePos+=todo;
};

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
	MasterSeparation=0.5f;
	//FltBuf=0;
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
