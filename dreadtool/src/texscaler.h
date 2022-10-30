
#pragma once


struct TSConfig {
	int	dst_repos;		// reposition destination pointer before blit (counted from start of the whole column)
	int	dst_stride;		// reposition destination pointer after each run
	int	src_repos;		// reposition source pointer before blit
	int	src_stride;		// how to reposition source pointer after each run
	int	run_length;		// how many source pixels to copy
	int	run_count;		// how many runs
	int src_shift;
	int dst_shift;

	TSConfig() { InitDefaults(); }

	void InitDefaults() {
		dst_repos = 0;
		dst_stride = 0;
		src_repos = 0;
		src_stride = 0;
		run_length = 0;
		run_count = 0;
		src_shift = 0;
		dst_shift = 0;
	}
};

struct TSBlitterSetup {
	word	BLTCON0;
	word	BLTCON1;
	word	BLTAFWM;
	word	BLTALWM;
	word	BLTSIZE;
	short	BLTCMOD;
	short	BLTBMOD;
	short	BLTAMOD;
	short	BLTDMOD;
	word	BLTCDAT;
	word	BLTBDAT;
	word	BLTADAT;

	// non-blitter settings
	short	src_offset;
	short	dst_offset;
	byte	src_channel;	// bitmask: 1=A, 2=B, 4=C
	byte	dst_channel;	// bitmask: 1=A, 2=B, 4=C, 8=D

	TSBlitterSetup() { InitDefaults(); }

	void InitDefaults();
	bool InitFromConfig(const TSConfig &sm);
	void SetupBlitter(Blitter &b);
};

struct TSMethodStats {
	float	total_error;
	int		cycle_count;
	int		wall_size;

	TSMethodStats() { Clear(); }

	void Clear() {
		total_error = 1e18f;
		cycle_count = 0;
		wall_size = -1;
	}
};

class TexScalerMethod {
public:
	TSConfig		config;
	TSBlitterSetup	blit;
	TSMethodStats	stats;
};

struct TexScalerScoreSettings {
	int		max_bad_order;
	float	exponent;

	TexScalerScoreSettings() {
		max_bad_order = 2;
		exponent = .5f;
	}
	void Serialize(TreeFileRef tf);
	void MenuEdit();
};

class TexScalerTester {
public:
	enum {
		EMPTY		= 0xE7E7,
		DONTTOUCH	= 0xEDED,
		ERRVAL		= 0xEEEE
	};

	Blitter					blit;
	vector<word>			source;
	vector<word>			target;
	int						src_size;
	int						dst_size;
	int						margin_src_pre;
	int						margin_src_post;
	int						margin_dst_pre;
	int						margin_dst_post;
	TexScalerScoreSettings	scorecfg;

	TexScalerTester() {
		src_size = 64;
		dst_size = 100;
		margin_src_pre	= 0;
		margin_src_post	= 0;
		margin_dst_pre	= 0;
		margin_dst_post	= 0;
	}

	bool Render(TexScalerMethod &m);
	bool RunTest(TexScalerMethod &m);
	bool CheckValidity(TexScalerMethod &m);
	bool MeasureQuality(TexScalerMethod &m);

	void SerializeSettings(TreeFileRef tf);
};


class TexScalerMethodBank {
public:
	string						bank_name;
	string						bank_ser_name;
	vector<TexScalerMethod>		methods;
	int							build_progress;
	int							build_max;
	int							counter_run_length;
	int							counter_run_count;
	int							draw_max;
	TexScalerScoreSettings		scorecfg;

	TexScalerMethodBank() : build_progress(-1), build_max(1) {}

	void SetNames(const char *name, const char *ser) { bank_name=name; bank_ser_name=ser; }

	void Serialize(TreeFileRef tf);

	void  RequestRebuild(TexScalerTester &tester);
	float GetProgress()		{ return float(build_progress)/build_max; }
	bool  IsBuilding()		{ return build_progress >= 0; }
	void  MenuProgress(const char *label);

	bool BuildStep(TexScalerTester &tester);
	bool BuildTime(TexScalerTester &tester,DWORD max_ms);
};
