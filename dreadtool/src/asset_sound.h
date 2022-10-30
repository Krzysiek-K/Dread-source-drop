
#pragma once


class GfxAssetGroup;
class GfxConverter;



// ================================================================ SoundAsset ================================================================


struct RiffWaveHeader {
	uint32_t	chunk_id;			// "RIFF"
	uint32_t	chunk_size;			// filesize - 8
	uint32_t	format;				// "WAVE"
	uint32_t	sub1_id;			// "fmt "
	uint32_t	sub1_size;			// 16
	uint16_t	audio_format;		// 1 = PCM
	uint16_t	channels;			// 1 or 2
	uint32_t	sample_rate;		// sample rate
	uint32_t	byte_rate;			// sample_rate * channels * bits_per_sample/8
	uint16_t	block_align;		// channels * bits_per_sample/8
	uint16_t	bits_per_sample;	// 8 or 16
	uint32_t	sub2_id;			// "data"
	uint32_t	sub2_size;			// num_samples * block_align
};


class SoundAsset : public AssetBase {
public:
	static const char TYPE_NAME[];


	int					src_channels = 0;
	int					src_bits = 0;
	int					src_sample_rate = 0;
	int					paula_rate = 0;
	float				sample_rate = 0;
	vector<vec2>		wave_data;
	vector<int8_t>		byte_data;
	DataTexture			preview;

	
	vector<AudioEnginePlaybackPacket>	tracker_data;
	bool								is_tracked_sample = false;



	virtual const char *GetAssetClassName() { return TYPE_NAME; }
	virtual void OnClick();
	virtual bool ImportAssets();

	void ResampleTo(float rate);
	bool GeneratePreview(int width);
	void GenerateByteData();

	void Load(DefNode *node, GfxAssetGroup *aowner, GfxConverter *owner);
	void LoadTracked(DefNode *node, GfxAssetGroup *aowner, GfxConverter *owner);

	virtual bool DrawPreview();
	void DrawTrackedPreview();

	int ExportSound(FILE *fp, platform_t platform);

	void SavePreview(const char *path);
};
