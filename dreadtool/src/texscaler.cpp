
#include "common.h"


// -------------------------------- TSBlitterSetup --------------------------------


void TSBlitterSetup::InitDefaults()
{
	BLTCON0 = 0;
	BLTCON1 = 0;
	BLTAFWM = 0xFFFF;
	BLTALWM = 0xFFFF;
	BLTSIZE = 0;
	BLTCMOD = 0;
	BLTBMOD = 0;
	BLTAMOD = 0;
	BLTDMOD = 0;
	BLTCDAT = 0;
	BLTBDAT = 0;
	BLTADAT = 0;

	src_offset = 0;
	dst_offset = 0;
	src_channel = 1;
	dst_channel = 8;
}

bool TSBlitterSetup::InitFromConfig(const TSConfig &sm)
{
	InitDefaults();
	src_offset = sm.src_repos;
	dst_offset = sm.dst_repos;
	BLTAMOD = sm.src_stride<<1;
	BLTBMOD = sm.dst_stride<<1;
	BLTDMOD = sm.dst_stride<<1;
	BLTCON0 = 0x0D00 + MINTERM( (MINT_A & MINT_C) | (MINT_B & ~MINT_C) );
	BLTCON1 = 0x0000;
	BLTSIZE = (sm.run_length & 63) | (sm.run_count<<6);
	src_channel = 1;
	dst_channel = 2+8;
	BLTCDAT = 0x00FF;
	if( sm.run_length>64 || sm.run_count>1024 )
		return false;
	return true;
}


void TSBlitterSetup::SetupBlitter(Blitter &b)
{
	b.InitDefaults();
	b.BLTCON0 = BLTCON0;
	b.BLTCON1 = BLTCON1;
	b.BLTAFWM = BLTAFWM;
	b.BLTALWM = BLTALWM;
	b.BLTSIZE = BLTSIZE;
	b.BLTCMOD = BLTCMOD;
	b.BLTBMOD = BLTBMOD;
	b.BLTAMOD = BLTAMOD;
	b.BLTDMOD = BLTDMOD;
	b.BLTCDAT = BLTCDAT;
	b.BLTBDAT = BLTBDAT;
	b.BLTADAT = BLTADAT;
}

// -------------------------------- TexScalerScoreSettings --------------------------------



void TexScalerScoreSettings::Serialize(TreeFileRef tf)
{
	tf.SerInt("max-bad-order", max_bad_order, 2);
	tf.SerFloat("exponent", exponent, .5f);
}

void TexScalerScoreSettings::MenuEdit()
{
	ms.Slider(format("Max bad order: %d", max_bad_order).c_str(), max_bad_order, 0, 50);
	ms.SliderF(format("Score exp: %.2f", exponent).c_str(), exponent, 0.10f, 3.00f, 2);
}

// -------------------------------- TexScalerTester --------------------------------


bool TexScalerTester::Render(TexScalerMethod &m)
{
	word src_mask = 0x00FF << m.config.src_shift;
	word dst_mask = 0x00FF << m.config.dst_shift;

	source.resize(margin_src_pre + src_size + margin_src_post);

	target.clear();
	target.resize(margin_dst_pre + dst_size + margin_dst_post, (EMPTY & dst_mask) | (DONTTOUCH & ~dst_mask));

	// fill source
	{
		int p = 0;
		for( int i=0; i<margin_src_pre; i++ )
			source[p++] = (EMPTY & src_mask) | (ERRVAL & ~src_mask);;
		for( int i=0; i<src_size; i++ )
			source[p++] = (i*0x0101 & src_mask) | (ERRVAL & ~src_mask);
		for( int i=0; i<margin_src_post; i++ )
			source[p++] = (EMPTY & src_mask) | (ERRVAL & ~src_mask);;
	}

	// setup blitter
	m.blit.SetupBlitter(blit);
	if( m.blit.src_channel & 1 ) { blit.pool_a.SetRef(source);		blit.BLTAPT = margin_src_pre*2 + m.blit.src_offset; }
	if( m.blit.src_channel & 2 ) { blit.pool_b.SetRef(source);		blit.BLTBPT = margin_src_pre*2 + m.blit.src_offset; }
	if( m.blit.src_channel & 4 ) { blit.pool_c.SetRef(source);		blit.BLTCPT = margin_src_pre*2 + m.blit.src_offset; }
	if( m.blit.dst_channel & 1 ) { blit.pool_a.SetRef(target);		blit.BLTAPT = margin_dst_pre*2 + m.blit.dst_offset; }
	if( m.blit.dst_channel & 2 ) { blit.pool_b.SetRef(target);		blit.BLTBPT = margin_dst_pre*2 + m.blit.dst_offset; }
	if( m.blit.dst_channel & 4 ) { blit.pool_c.SetRef(target);		blit.BLTCPT = margin_dst_pre*2 + m.blit.dst_offset; }
	if( m.blit.dst_channel & 8 ) { blit.pool_d.SetRef(target);		blit.BLTDPT = margin_dst_pre*2 + m.blit.dst_offset; }

	// run
	blit.Run();

	// valid?
	if( blit.pool_a.access_error || blit.pool_b.access_error || blit.pool_c.access_error || blit.pool_d.access_error )
		return false;
	return true;
}

bool TexScalerTester::RunTest(TexScalerMethod &m)
{
	m.stats.Clear();
	
	bool ok = Render(m);
	
	m.stats.cycle_count = blit.cycle_counter;

	return ok;
}

bool TexScalerTester::CheckValidity(TexScalerMethod &m)
{
	word src_mask = 0x00FF << m.config.src_shift;
	word dst_mask = 0x00FF << m.config.dst_shift;

	for(int i=0;i<dst_size;i++)
	{
		word dst = target[i+margin_dst_pre];
		if( (dst & dst_mask) == (ERRVAL & dst_mask) )
			return false;
		if( (dst & ~dst_mask) != (DONTTOUCH & ~dst_mask) )
			return false;
	}
	return true;
}

bool TexScalerTester::MeasureQuality(TexScalerMethod &m)
{
	word src_mask = 0x00FF << m.config.src_shift;
	word dst_mask = 0x00FF << m.config.dst_shift;

	int pix_first = -1;
	int pix_last = -1;
	word masked_size = (src_size*0x0101) & dst_mask;
	for( int i=0; i<target.size(); i++ )
		if( (target[i]&dst_mask) < masked_size )
		{
			if( pix_first<0 ) pix_first = i;
			pix_last = i;
		}
	if( pix_first<0 ) return false;
	pix_last++;

	int pix_count = pix_last - pix_first;
	if( pix_count%2 ) return false;
	if( pix_count<=0 || pix_count>dst_size ) return false;

	int ideal_ypos = dst_size/2 - pix_count/2;
	m.blit.dst_offset += (ideal_ypos - pix_first + margin_dst_pre)*2;
	if( m.blit.dst_offset<0 ) return false;


	float error = 0;
	float prev_pos = -1;
	int monotonic = 0;
	for( int i=0; i<pix_count; i++ )
	{
		word val = target[pix_first+i] & dst_mask;
		float real_pos = ((val + (val>>8))&0xFF) + .5f;
		float ideal_pos = (i+.5f)/pix_count*64;
		float err = fabs(real_pos - ideal_pos);
		error += pow(err+0.000001f, scorecfg.exponent);

		if( real_pos<prev_pos )
			monotonic++;
		prev_pos = real_pos;
	}
	if( monotonic>scorecfg.max_bad_order ) return false;

	m.stats.total_error = error;
	m.stats.wall_size = pix_count/2;
	return true;
}

void TexScalerTester::SerializeSettings(TreeFileRef tf)
{
	tf.SerInt("src-size", src_size, 64);
	tf.SerInt("dst-size", dst_size, 100);
	tf.SerInt("margin-src", margin_src_pre, 0);
	if(tf.IsReading())
		margin_src_post	= margin_src_pre;
	tf.SerInt("margin-dst", margin_dst_pre, 0);
	if( tf.IsReading() )
		margin_dst_post	= margin_dst_pre;
}




// -------------------------------- TexScalerMethodBank --------------------------------

void TexScalerMethodBank::Serialize(TreeFileRef tf)
{
	scorecfg.Serialize(tf.SerChild("score-config"));
}

void TexScalerMethodBank::RequestRebuild(TexScalerTester &tester)
{
	build_progress = 0;
	build_max = 0;
	counter_run_length = 1;
	counter_run_count = 1;
	draw_max = tester.dst_size*2;
	tester.scorecfg = scorecfg;

	for( int run_length=1; run_length<=64; run_length++ )
		for( int run_count=1; run_count*run_length<=draw_max; run_count++ )
		build_max++;
}

void TexScalerMethodBank::MenuProgress(const char *label)
{
	int value = build_progress;
	ms.Slider(label, build_progress, 0, max(1, build_max));
	build_progress = value;
}


bool TexScalerMethodBank::BuildStep(TexScalerTester &tester)
{
	if( build_progress < 0 )
		return false;

	if(build_progress==0)
	{
		methods.clear();
		methods.resize(51);
	}

	TexScalerMethod method;
	TSConfig cfg;

	cfg.run_length = counter_run_length;
	cfg.run_count = counter_run_count;

	if( cfg.run_length>64 )
	{
		build_progress = -1;
		return false;
	}

	for( cfg.src_stride=-5; cfg.src_stride<=5; cfg.src_stride++ )
		for( cfg.src_repos=-4; cfg.src_repos<=4; cfg.src_repos++ )
			for( cfg.dst_stride=-2; cfg.dst_stride<=0; cfg.dst_stride++ )
			{
				method.config = cfg;
				method.blit.InitFromConfig(method.config);

				if( !tester.RunTest(method) )
					continue;

				if( !tester.CheckValidity(method) )
					continue;

				if( !tester.MeasureQuality(method) )
					continue;

				if( method.stats.wall_size<=0 || method.stats.wall_size>=methods.size() )
					continue;

				if( method.stats.total_error < methods[method.stats.wall_size].stats.total_error )
					methods[method.stats.wall_size] = method;
			}

	// next
	build_progress++;
	counter_run_count++;
	if( counter_run_count*counter_run_length > draw_max )
	{
		counter_run_count = 1;
		counter_run_length++;
		if( counter_run_length > 64 )
		{
			// done
			build_progress = -1;
			return false;
		}
	}

	return true;
}

bool TexScalerMethodBank::BuildTime(TexScalerTester &tester, DWORD max_ms)
{
	DWORD t0 = timeGetTime();
	while( BuildStep(tester) )
	{
		if( timeGetTime() - t0 >= max_ms )
			return true;
	}
	return false;
}
