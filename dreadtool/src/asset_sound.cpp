
#include "common.h"
#include "app_rendertest.h"



const char SoundAsset::TYPE_NAME[] = "sound asset";



inline float _sinc(float x)		{ return x ? sinf(x)/x : 1; }
inline float _hamming(float x)	{ return (x>-1 && x<1) ? sqrtf(cosf(x*M_PI/2)) : 0; }




// ================================================================ SoundAsset ================================================================


void SoundAsset::OnClick()
{
	if( is_tracked_sample )
	{
		ami_paula.RemoveAllChannels(false);

		AmigaTrackedVoice *voice = new AmigaTrackedVoice();
		ami_paula.Voices.push_back(voice);

		AudioEnginePlaybackPacket *track = tracker_data.data();
		voice->Trigger(track, track + tracker_data.size());
	}
	else if( byte_data.size()>0 )
	{
		ami_paula.RemoveAllChannels(false);

		AmigaSimpleVoice *voice = new AmigaSimpleVoice();
		ami_paula.Voices.push_back(voice);

		voice->Volume = 64;
		voice->Period = paula_rate;
		voice->Trigger(
			&byte_data[0],		// sample pointer
			byte_data.size(),	// sample length
			1					// loop length
		);
	}
}

bool SoundAsset::ImportAssets()
{
	if( is_tracked_sample )
		return true;


	string path = modmgr.GetResourcePath(asset_group->src_path + "/" + file_name + ".wav");

	src_channels = 0;
	src_bits = 0;
	src_sample_rate = 0;
	wave_data.clear();
	preview.width = 0;

	try {
		vector<uint8_t> file;
		if( !NFS.GetFileBytes(path.c_str(), file) )
			src_node->Error(format("Can't open sound file '%s'", path.c_str()));

		if( file.size()<44 )
			src_node->Error(format("Invalid sound file '%s' - file too short", path.c_str()));

		RiffWaveHeader *header = (RiffWaveHeader*)&file[0];
	
		if( header->chunk_id		!= 0x46464952		)	src_node->Error(format("Invalid sound file '%s' - no RIFF chunk", path.c_str()));
		if( header->chunk_size		!= file.size()-8	)	src_node->Error(format("Invalid sound file '%s' - invalid RIFF chunk size", path.c_str()));
		if( header->format			!= 0x45564157		)	src_node->Error(format("Invalid sound file '%s' - not a WAVE format", path.c_str()));
		if( header->sub1_id			!= 0x20746d66		)	src_node->Error(format("Invalid sound file '%s' - missing 'fmt ' subchunk", path.c_str()));
		if( header->sub1_size		!= 16				)	src_node->Error(format("Invalid sound file '%s' - invalid 'fmt ' subchunk size", path.c_str()));
		if( header->audio_format	!= 1				)	src_node->Error(format("Invalid sound file '%s' - not a PCM file", path.c_str()));
		if( header->channels!=1 && header->channels!=2	)	src_node->Error(format("Invalid sound file '%s' - unsupported channel count (%d)", path.c_str(), header->channels));
		if( header->bits_per_sample!=8 && header->bits_per_sample!=16 )	src_node->Error(format("Invalid sound file '%s' - unsupported bits per sample (%d)", path.c_str(), header->bits_per_sample));
		if( header->byte_rate != header->sample_rate * header->channels * header->bits_per_sample/8 )	src_node->Error(format("Invalid sound file '%s' - invalid byte rate specified", path.c_str()));
		if( header->block_align != header->channels * header->bits_per_sample/8 )	src_node->Error(format("Invalid sound file '%s' - invalid block align value", path.c_str()));

		if( header->sub2_id			!= 0x61746164		)	src_node->Error(format("Invalid sound file '%s' - missing 'data' subchunk", path.c_str()));
		if( header->sub2_size		> file.size()-sizeof(RiffWaveHeader) )	src_node->Error(format("Invalid sound file '%s' - invalid 'data' subchunk size", path.c_str()));
		if( header->sub2_size % header->block_align	!= 0)	src_node->Error(format("Invalid sound file '%s' - unaligned 'data' subchunk size", path.c_str()));

		// load
		int sample_count = header->sub2_size/header->block_align;
		if( sample_count <=0 )	src_node->Error(format("Invalid sound file '%s' - invalid sample count (%d)", path.c_str(), sample_count));
		
		src_channels = header->channels;
		src_bits = header->bits_per_sample;
		src_sample_rate = header->sample_rate;
		sample_rate = header->sample_rate;
		paula_rate = (int)floor(3546895 / sample_rate + 1.5f);

		wave_data.resize(sample_count);
		if( header->bits_per_sample==8 )
		{
			uint8_t *src = (uint8_t*)&file[sizeof(RiffWaveHeader)];
			float gain = asset_group->opt_audio_gain/128.f;
			if( header->channels==1 )
			{
				for( int i=0; i<sample_count; i++ )
					wave_data[i] = vec2(1, 1)*((*src++ - 0x80)*gain);
			}
			else
			{
				for( int i=0; i<sample_count; i++ )
				{
					wave_data[i].x = (*src++ - 0x80)*gain;
					wave_data[i].y = (*src++ - 0x80)*gain;
				}
			}
		}
		else
		{
			int16_t *src = (int16_t*)&file[sizeof(RiffWaveHeader)];
			float gain = asset_group->opt_audio_gain/32786.f;
			if( header->channels==1 )
			{
				for( int i=0; i<sample_count; i++ )
					wave_data[i] = vec2(1, 1)*(*src++*gain);
			}
			else
			{
				for( int i=0; i<sample_count; i++ )
				{
					wave_data[i].x = *src++*gain;
					wave_data[i].y = *src++*gain;
				}
			}
		}

		if( asset_group->opt_audio_resample > 0 )
			ResampleTo(asset_group->opt_audio_resample);
		
		GenerateByteData();


		converter_owner->WatchFile(path.c_str(), this);
	}
	catch( const string &e )
	{
		elog.Error(this, e);
		return false;
	}


	return true;
}

void SoundAsset::ResampleTo(float rate)
{
	const int FIR_WIDTH = 32;
	vector<float> FIRTable(2*FIR_WIDTH+1);

	const float yscale=rate/sample_rate;
	const float xscale=float(M_PI*yscale);
	float wsum = 0;
	for( int i=-FIR_WIDTH; i<=FIR_WIDTH; i++ )
		wsum += FIRTable[i+FIR_WIDTH] = yscale*_sinc(i*xscale)*_hamming(i/float(FIR_WIDTH-1));

	for( int i=-FIR_WIDTH; i<=FIR_WIDTH; i++ )
		FIRTable[i+FIR_WIDTH] /= wsum;

	vector<vec2> new_data(int(floor(wave_data.size()*yscale+0.5f)));
	for( int i=0; i<new_data.size(); i++ )
	{
		float spos = (i+0.5f)/yscale;
		int ipos = int(floor(spos));
		spos -= ipos;

		int d1 = ipos - FIR_WIDTH;
		int d2 = ipos + FIR_WIDTH + 1;
		if( d1<0 ) d1 = 0;
		if( d2>wave_data.size() ) d2 = wave_data.size();
		if( d1>=d2 ) continue;


		vec2 v = wave_data[d1++];
		vec2 out0 = vec2(0, 0);
		vec2 out1 = vec2(0, 0);
		float *fir_src = &FIRTable[d1-ipos+FIR_WIDTH];
		while( d1 < d2 )
		{
			float w = *fir_src++;
			out0 += v*w;
			v = wave_data[d1++];
			out1 += v*w;
		}

		v = out0 + (out1-out0)*spos;
		if( v.x < -1 ) v.x = -1;
		if( v.x >  1 ) v.x =  1;
		if( v.y < -1 ) v.y = -1;
		if( v.y >  1 ) v.y =  1;
		new_data[i] = v;
	}

	wave_data.swap(new_data);
	sample_rate = rate;
	paula_rate = (int)floor(3546895 / sample_rate + 1.5f);
}

bool SoundAsset::GeneratePreview(int width)
{
	if( wave_data.size()<=0 )
		return false;

	if( preview.width == width )
		return true;

	preview.Resize(width, 1);

	for( int x=0; x<width; x++ )
	{
		float vmin = 1;
		float vmax = -1;
		float vsum = 0;

		int w1 = x*wave_data.size()/width;
		int w2 = ((x+1)*wave_data.size()+width-1)/width;
		for( int w=w1; w<w2; w++ )
		{
			float v = (wave_data[w].x + wave_data[w].y)/2;
			if( v<vmin ) vmin = v;
			if( v>vmax ) vmax = v;
			vsum += v;
		}
		if( w2>w1 ) vsum /= w2-w1;

		preview.data[x] = (vec3(vmin,vmax,vsum)*0.5f+0.5f).make_rgb();
	}

	preview.Upload();

	return true;
}

void SoundAsset::GenerateByteData()
{
	byte_data.resize(wave_data.size());

	for( int i=0; i<byte_data.size(); i++ )
	{
		float v = floor( (wave_data[i].x + wave_data[i].y)*(0.5f*127) + 0.5f );
		if( v<-128 ) v = -128;
		if( v> 127 ) v = -127;
		byte_data[i] = int8_t(v);
	}
}


void SoundAsset::Load(DefNode *node, GfxAssetGroup *aowner, GfxConverter *owner)
{
	try
	{
		InitLoad(node, 0, aowner, owner);

		int argpos = 1;
		bool has_coords = false;
		while( argpos < node->args.size() )
		{
			if( node->args[argpos] == "-file" && argpos+1<node->args.size() )			// TBD: document
			{
				argpos++;
				file_name = node->args[argpos++];
			}
			else
				node->InvalidArg(argpos);
		}

		owner->AddAsset(this);
	}
	catch( const string &e )
	{
		elog.Error(this, e);
		return;
	}
}

void SoundAsset::LoadTracked(DefNode *node, GfxAssetGroup *aowner, GfxConverter *owner)
{
	try
	{
		is_tracked_sample = true;
		node->RequireArgs(1);
		InitLoad(node, 0, aowner, owner);
		owner->AddAsset(this);


		// Load tracker steps
		for( int i=0; i<(int)node->subnodes.size(); i++ )
		{
			//    1   S   DSPISTOL    64  +0    // wait 1, 'S'top audio, program sample, volume 64, pitch +0c
			//    +   -   XXX         --  --    // on interrupt (+), queue zero sample (XXX)
			//    +   E   ---          0  --    // on next interrupt (+), 'E'nd sound completely, volume 0
			DefNode *s = node->subnodes[i];
			s->ForbidSubnodes();
			s->RequireArgs(4);

			// Row timing
			AudioEnginePlaybackPacket pk = {};
			if( s->name!="+" )
			{
				const char *sname = s->name.c_str();
				pk.delay = ParseInt(sname);
				if( pk.delay<=0 || *sname )
					s->Error(format("Tracked sound '%s': invalid delay time '%s' in row entry", name.c_str(), s->name.c_str()));
			}

			// Stop/end command
			if( s->args[0]=="S" )
			{
				pk.flags |= AudioEnginePlaybackPacket::F_STOP;
				if( pk.delay==0 )
					s->Error(format("Tracked sound '%s': 'S' stop command can't be used in IRQ row", name.c_str()));
			}
			else if( s->args[0]=="E" )
			{
				pk.flags |= AudioEnginePlaybackPacket::F_STOP | AudioEnginePlaybackPacket::F_END;
			}
			else if( s->args[0]!="-" )
				s->Error(format("Tracked sound '%s': '%s' is not a valid stop/end command", name.c_str(), s->args[0].c_str()));

			// Sample trigger
			if( s->args[1]=="XXX" )
			{
				pk.flags |= AudioEnginePlaybackPacket::F_SET_SAMPLE;
				pk.sample = NULL;
			}
			else if( s->args[1]!="---" )
			{
				SoundAsset *sample = owner->FindSound(s->args[1].c_str(), true);
				if( !sample || sample->is_tracked_sample )
					s->Error(format("Tracked sound '%s': no loaded sample '%s'", name.c_str(), s->args[1].c_str()));

				pk.flags |= AudioEnginePlaybackPacket::F_SET_SAMPLE;
				pk.sample = sample;
			}

			// Volume
			if( s->args[2]!="--" )
			{
				pk.flags |= AudioEnginePlaybackPacket::F_UPDATE_VOLUME;
				pk.volume = s->GetIntegerArg(2);

				if( pk.volume<0 || pk.volume>64 )
					s->Error(format("Tracked sound '%s': invalid volume %d (0..64 range)", name.c_str(), pk.volume));
			}

			// Frequency
			if( s->args[3]!="--" )
			{
				pk.flags |= AudioEnginePlaybackPacket::F_UPDATE_FREQUENCY;
				pk._freq_shift = s->GetFloatArg(3);
				if( pk._freq_shift<=-50 || pk._freq_shift>=50 )
					pk._freq_shift /= 100.f;

				float pitch_mul = pow(2.f, -pk._freq_shift/12.f);
				uint32_t pitch_mul_i = (uint32_t)floor(pitch_mul*0x10000 + 0.5);

				pk.period_mul = uint16_t(pitch_mul_i>>16);
				pk.period_mul_fract = uint16_t(pitch_mul_i);
			}

			tracker_data.push_back(pk);
		}

		//	AudioEnginePlaybackPacket pk = {};
		//	pk.flags = AudioEnginePlaybackPacket::F_DONE;

		// Link/check commands
		AudioEnginePlaybackPacket *last_timer_packet = NULL;
		for( int i=0; i<(int)tracker_data.size(); i++ )
		{
			AudioEnginePlaybackPacket &pk = tracker_data[i];
			AudioEnginePlaybackPacket *next = (i+1<(int)tracker_data.size()) ? &tracker_data[i+1] : NULL;

			if( pk.delay )
			{
				// timer packet
				if( last_timer_packet )
					last_timer_packet->flags &= ~AudioEnginePlaybackPacket::F_DONE;

				if( next && next->delay==0 )
					pk.flags |= AudioEnginePlaybackPacket::F_IRQ_ENABLE;

				pk.flags |= AudioEnginePlaybackPacket::F_DONE;
				last_timer_packet = &pk;
			}
			else
			{
				// IRQ packet
				if( !next || next->delay!=0 )
					pk.flags |= AudioEnginePlaybackPacket::F_DONE;
			}
		}
	}
	catch( const string &e )
	{
		elog.Error(this, e);
		return;
	}
}


bool SoundAsset::DrawPreview()
{
	if( is_tracked_sample )
	{
		DrawTrackedPreview();
		return true;
	}

	const int xpos1 = 10;
	const int xpos2 = 170;
	const int ystep = 30;
	int ypos = 10;
	Dev.PrintF(&font, xpos1, ypos, 0xFFFFFFFF, "Sound:");
	Dev.PrintF(&font, xpos2, ypos, 0xFFFFFFFF, "%s", name.c_str());
	ypos += ystep;

	Dev.PrintF(&font, xpos1, ypos, 0xFFFFFFFF, "Channels:");
	Dev.PrintF(&font, xpos2, ypos, 0xFFFFFFFF, "%d", src_channels);
	ypos += ystep;

	Dev.PrintF(&font, xpos1, ypos, 0xFFFFFFFF, "Bits:");
	Dev.PrintF(&font, xpos2, ypos, 0xFFFFFFFF, "%d", src_bits);
	ypos += ystep;

	Dev.PrintF(&font, xpos1, ypos, 0xFFFFFFFF, "Source rate:");
	Dev.PrintF(&font, xpos2, ypos, 0xFFFFFFFF, "%d Hz", src_sample_rate);
	ypos += ystep;

	Dev.PrintF(&font, xpos1, ypos, 0xFFFFFFFF, "Playback:");
	Dev.PrintF(&font, xpos2, ypos, 0xFFFFFFFF, "%.1f Hz  (period %d)", sample_rate, paula_rate);
	ypos += ystep;

	Dev.PrintF(&font, xpos1, ypos, 0xFFFFFFFF, "Length:");
	Dev.PrintF(&font, xpos2, ypos, 0xFFFFFFFF, "%d", wave_data.size());
	ypos += ystep;

	Dev.PrintF(&font, xpos1, ypos, 0xFFFFFFFF, "Time:");
	Dev.PrintF(&font, xpos2, ypos, 0xFFFFFFFF, "%.3f s", wave_data.size()/sample_rate);
	ypos += ystep;

	ypos += ystep;


	// Draw preview
	int px1 = 0;
	int px2 = Dev.GetScreenSizeV().x-MENU_WIDTH;
	int py1 = ypos;
	int py2 = ypos + 128;
	if( GeneratePreview(px2-px1) )
	{
		Dev->SetTexture(0, preview.tex);

		fx_render.StartTechnique("sound_preview");
		while( fx_render.StartPass() )
			Dev.DrawScreenQuad(px1, py1, px2, py2);
	}

	return true;
}


void SoundAsset::DrawTrackedPreview()
{
	const int xp0 = 10;
	const int xp1 = xp0+40;
	const int xp2 = xp1+40;
	const int xp3 = xp2+50;
	const int xp4 = xp3+40;
	const int xp5 = xp4+180;
	const int xp6 = xp5+80;
	const int xpend = xp6+60;
	const int ystep = 30;
	int ypos = 10;

	DWORD col_text = 0xFFFFFFFF;
	DWORD col_inactive = 0x80808080;
	DWORD col_timer_cmd = 0xFF40FF40;
	DWORD col_irq_cmd = 0xFFFF4040;
	string tmps;

	AmigaTrackedVoice *voice = (ami_paula.Voices.size()>0) ? dynamic_cast<AmigaTrackedVoice*>(ami_paula.Voices[0]) : NULL;


	// <time>	<step>	<func>	<sample>	<volume>	<pitch>
	int timepos = 0;
	for( int i=0; i<tracker_data.size(); i++ )
	{
		const AudioEnginePlaybackPacket &row = tracker_data[i];
		int ytext = ypos + ystep/2 - 2;

		// Background
		DWORD rowcol = 0xFF484860;
		if( !row.delay ) rowcol = 0xFF303040;
		if( voice && voice->vis_active_timer_cmd == &row )
		{
			vec3 c1 = vec3::from_rgb(rowcol);
			vec3 c2 = vec3::from_rgb(col_timer_cmd);
			float amount = pow(0.0001f, Dev.GetElapsedTime() - voice->vis_active_timer_timestamp);
			if( amount < 0.4f ) amount = 0.4f;
			rowcol = (c1+(c2-c1)*amount).make_rgb() | 0xFF000000;
		}
		if( voice && voice->vis_active_irq_cmd == &row )
		{
			vec3 c1 = vec3::from_rgb(rowcol);
			vec3 c2 = vec3::from_rgb(col_irq_cmd);
			float amount = pow(0.0001f, Dev.GetElapsedTime() - voice->vis_active_irq_timestamp);
			if( amount < 0.4f ) amount = 0.4f;
			rowcol = (c1+(c2-c1)*amount).make_rgb() | 0xFF000000;
		}


		Dev.DrawScreenQuadCol(xp0, ypos, xpend, ypos+ystep, 0, rowcol);

		// Time / delay / irq
		if( row.delay )
		{
			timepos += row.delay;
			Dev.AlignPrintF(&font, xp1, ytext, 0x21, col_text, "%02d", timepos);
			Dev.AlignPrintF(&font, xp2, ytext, 0x11, col_text, "%+d", row.delay);
		}
		else
		{
			Dev.AlignPrintF(&font, xp2, ytext, 0x11, col_inactive, "+");
		}

		// Action
		tmps.clear();
		if( row.flags & AudioEnginePlaybackPacket::F_STOP ) tmps += "S";
		if( row.flags & AudioEnginePlaybackPacket::F_END ) tmps += "E";
		if( row.flags & AudioEnginePlaybackPacket::F_DONE ) tmps += "D";
		if( row.flags & AudioEnginePlaybackPacket::F_IRQ_ENABLE ) tmps += "I";
		if( tmps.size()>0 )
			Dev.AlignPrintF(&font, xp3, ytext, 0x11, col_text, tmps.c_str());
		else
			Dev.AlignPrintF(&font, xp3, ytext, 0x11, col_inactive, "-");

		// Sample
		if( row.flags & AudioEnginePlaybackPacket::F_SET_SAMPLE )
		{
			if( row.sample )
				Dev.AlignPrintF(&font, xp4, ytext, 0x01, col_text, "%s", row.sample->name.c_str());
			else
				Dev.AlignPrintF(&font, xp4, ytext, 0x01, col_text, "XXX");
		}
		else
			Dev.AlignPrintF(&font, xp4, ytext, 0x01, col_inactive, "---");

		// Volume
		if( row.flags & AudioEnginePlaybackPacket::F_UPDATE_VOLUME )
			Dev.AlignPrintF(&font, xp5, ytext, 0x01, col_text, "%2d", row.volume);
		else
			Dev.AlignPrintF(&font, xp5, ytext, 0x01, col_inactive, "--");

		// Frequency
		if( row.flags & AudioEnginePlaybackPacket::F_UPDATE_FREQUENCY )
			Dev.AlignPrintF(&font, xp6, ytext, 0x11, col_text, "%+.2f", row._freq_shift);
		else
			Dev.AlignPrintF(&font, xp6, ytext, 0x11, col_inactive, "--");

		ypos += ystep;
	}

	if( voice && voice->vis_sound_error )
	{
		int ytext = ypos + ystep/2 - 2;
		Dev.AlignPrintF(&font, xp0, ytext, 0x01, 0xFFFF0000, "Error $%03X", voice->vis_sound_error);
	}
}


int SoundAsset::ExportSound(FILE *fp, platform_t platform)
{
	int size = 0;

	if( platform == PLATFORM_AMIGA )
	{
		// Amiga
		if( byte_data.size() > 0 )
		{
			fprintf(fp, "__chip word SOUND_%s[] = {\n", name.c_str());
			fprintf(fp, "\t%d, %d,", (byte_data.size()+1)/2, paula_rate);

			for( int s=0; s<byte_data.size(); s+=2 )
			{
				int8_t s1 = byte_data[s];
				int8_t s2 = (s+1 < byte_data.size()) ? byte_data[s+1] : 0;

				if( s%32==0 ) fprintf(fp, "\n\t");

				fprintf(fp, "0x%02X%02X,", uint8_t(s1), uint8_t(s2));
				size += 2;
			}
			fprintf(fp, "\n};\n");
		}

		fprintf(fp, "#if AMIGA_AUDIO_ENGINE_VERSION == 2\n");
		if( tracker_data.size() > 0 )
		{
			fprintf(fp, "const AudioTrackRow TRKSOUND_%s[] = {\n", name.c_str());

			for( int i=0; i<tracker_data.size(); i++ )
			{
				AudioEnginePlaybackPacket &tr = tracker_data[i];

				fprintf(fp, "\t{ %3d, 0x%02X, %2d, ", tr.delay, tr.flags, tr.volume);
				if( tr.sample )
					fprintf(fp, "SOUND_%-16s, ", tr.sample->name.c_str());
				else if( tr.flags & AudioEnginePlaybackPacket::F_SET_SAMPLE )
					fprintf(fp, "SOUND_%-16s, ", "NONE");
				else
					fprintf(fp, "%-22s, ", "NULL");
				fprintf(fp, "0x%04X, 0x%04X },\n", tr.period_mul, tr.period_mul_fract);
				size += 12;
			}
			fprintf(fp, "\n};\n");
		}
		else
		{
			// Export default oneshot playback track
			fprintf(fp, "const AudioTrackRow TRKSOUND_%s[] ={\n", name.c_str());
			fprintf(fp, "\t{ 1, 0xDD, 64, SOUND_%-16s, 0x0001, 0x0000 },\n", name.c_str());
			fprintf(fp, "\t{ 0, 0x04,  0, SOUND_NONE            , 0x0000, 0x0000 },\n");
			fprintf(fp, "\t{ 0, 0x4B,  0, NULL                  , 0x0000, 0x0000 },\n");
			fprintf(fp, "};\n");
		}
		fprintf(fp, "#endif\n");
	}
	else if( platform == PLATFORM_ST )
	{
		// STe
		if( byte_data.size() > 0 )
		{
			fprintf(fp, "__chip word SOUND_%s[] = {\n", name.c_str());
			fprintf(fp, "\t%d, ", (byte_data.size()+1)/2);

			for( int s=0; s<byte_data.size(); s+=2 )
			{
				int8_t s1 = byte_data[s];
				int8_t s2 = (s+1 < byte_data.size()) ? byte_data[s+1] : 0;

				if( s%32==0 ) fprintf(fp, "\n\t");

				fprintf(fp, "0x%02X%02X,", uint8_t(s1), uint8_t(s2));
				size += 2;
			}
			fprintf(fp, "\n};\n");
		}
	}

	return size;
}


void SoundAsset::SavePreview(const char *path)
{
	if( is_tracked_sample )
	{
		ami_paula.RemoveAllChannels(false);

		AmigaTrackedVoice *voice = new AmigaTrackedVoice();
		ami_paula.Voices.push_back(voice);

		AudioEnginePlaybackPacket *track = tracker_data.data();
		voice->Trigger(track, track + tracker_data.size());

		ami_paula.RenderToFile(format(path, name.c_str()).c_str());
	}
}
