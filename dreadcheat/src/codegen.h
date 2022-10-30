
#pragma once

#define SAVE_REGS		0



enum genmode_t {
	GEN_OFF					= 0,
	GEN_NOCLIP				= 0x100,		// (at least one bit must be set)
	GEN_Y1					= 0x004,		// possibly clipped by ymin		(may be removed entirely, but ymin<=y2)
	GEN_Y1_CULL				= 0x005,		// possibly culled by ymin		( ymin > y2 possible )
	GEN_Y1_MUST_CLIP		= 0x006,		// guaranteed  ymin >= y1
	GEN_Y1_MUST_CLIP_CULL	= 0x007,		// must clip, may cull
	GEN_Y1_MASK				= 0x007,
	GEN_Y2					= 0x040,
	GEN_Y2_CULL				= 0x050,
	GEN_Y2_MUST_CLIP		= 0x060,
	GEN_Y2_MUST_CLIP_CULL	= 0x070,
	GEN_Y2_MASK				= 0x070,
};

#define ISGEN_Y1_CULL(gen)			(((gen) & GEN_Y1_CULL)==GEN_Y1_CULL)
#define ISGEN_Y1_MUST_CLIP(gen)		(((gen) & GEN_Y1_MUST_CLIP)==GEN_Y1_MUST_CLIP)


enum texmode_t {
	TEXM_OFF			= 0,
	TEXM_PERSPECTIVE	= 0x0001,
	TEXM_LINEAR			= 0x0002,
};

struct CheatInfo {
	int			__name_index;		// used outside the generator
	genmode_t	sky_mode;			//	= GEN_OFF;
	genmode_t	ceil_mode;			//	= GEN_OFF;
	int			upper_y1;			//	= 0;			// ceil/sky y2
	genmode_t	upper_mode;			//	= GEN_OFF;
	int			upper_y2;			//	= 0;			// gap y1
	genmode_t	gap_mode;			//	= GEN_OFF;
	int			lower_y1;			//	= 0;			// gap y2
	genmode_t	lower_mode;			//	= GEN_OFF;
	int			lower_y2;			//	= 0;			// floor y1
	genmode_t	floor_mode;			//	= GEN_OFF;
	texmode_t	tex_mode;
};

struct CheatState : public CheatInfo {
	enum genstage_t {
		STAGE_CEIL_SKY = 0,
		STAGE_TEXCOORD,
		STAGE_UPPER,
		STAGE_GAP_SKIP,
		STAGE_FLOOR,
		STAGE_GAP_LIMITS,
		STAGE_DONE,
	};

	int			variant;
	genstage_t	stage;
};

class CodeGen : public CheatState {
public:
	Assembler	as;

	void Generate();

protected:

	EAddr xr_size_temp			= instructions::xr(0);
	EAddr xr_ceil_fill_addr		= instructions::xr(1);
	EAddr xr_ceil_temp			= instructions::xr(2);
	EAddr xr_texcoord_temp		= instructions::xr(3);
	EAddr xr_texcoord_tex_base	= instructions::xr(4);
	EAddr xr_upper_fn			= instructions::xr(5);
	EAddr xr_upper_temp			= instructions::xr(6);
	EAddr xr_floor_fill_addr	= instructions::xr(7);
	EAddr xr_floor_temp			= instructions::xr(8);
	EAddr xr_sky_fill_addr		= instructions::xr(9);
	EAddr xr_sky_temp			= instructions::xr(10);
	EAddr xr_sky_temp2			= instructions::xr(11);		// TBD: reorder not to use it
	EAddr xr_ymin				= instructions::xr(12);
	EAddr xr_ymax				= instructions::xr(13);
	EAddr xr_upper_fn2			= instructions::xr(14);
	EAddr xr_upper_temp2		= instructions::xr(15);
	EAddr xr_gap_temp			= instructions::xr(16);
	EAddr column_ymin			= instructions::mem("rc_ymin", instructions::a5);
	EAddr column_ymax			= instructions::mem("rc_ymax", instructions::a5);
	EAddr column_size_limit		= instructions::mem("rc_size_limit", instructions::a5);

	int variant_counter;


	void GenCeil(const char *cull_label=NULL);
	void GenSky(const char *cull_label=NULL);
	void GenTexcoord();
	void GenUpper(const char *cull_label=NULL);
	void GenGap();
	void GenFloor();
	void StartVariant(const string &lb);
	void GenDrawingCode();
};
