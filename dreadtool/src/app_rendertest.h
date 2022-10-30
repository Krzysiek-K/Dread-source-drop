
#pragma once

#include "wad.h"

extern "C" {
#define _E_SKIP_TYPEDEFS
#include "e_common.h"
}

#include "deflate.h"
#include "data_block.h"
#include "mod_manager.h"
#include "audio_engine.h"
#include "asset_base.h"
#include "gfx_colorspaces.h"
#include "gfx_palette.h"
#include "gfx_converter.h"
#include "asset_sound.h"
#include "asset_gfx.h"
#include "asset_flat.h"
#include "asset_animation.h"
#include "asset_group.h"
#include "wad_converter.h"
#include "script_compiler.h"
#include "dread_mapgen.h"
#include "dread_render.h"
#include "dread_sim.h"

#include "mapdraw.h"
#include "dev_coltest.h"



// ---------------------------------------------------------------- RenderTestApp ----------------------------------------------------------------


class RenderTestApp : public SubApplication {
public:
	enum {
		TOOL_MODS = 0,
		TOOL_3DVIEW,
		TOOL_TEXTURES,
		TOOL_MAP,
		TOOL_DEV,
		TOOL_ELOG,

		RMODE_GPU = 0,
		RMODE_SOFT,
		RMODE_SOFT2,

		DITHER_OFF = 0,
		DITHER_VERT,
		DITHER_SHAKE,
		DITHER_SCAN,
		DITHER_HQ,

		PHYS_NOCLIP_GRAVITY = 0,
		PHYS_NOCLIP,
		PHYS_BSP_CLIP,

		MAPVIEW_LINE_VIS = 0,
		MAPVIEW_THING_VIS,
		//MAPVIEW_REGION_VIS,
		MAPVIEW_COL_MESH,
		MAPVIEW_COL_EDGES,
		MAPVIEW_COL_BSP,
		MAPVIEW_LINE_VIS2,

		MAPTEST_CLIP_WALK = 0,
		MAPTEST_CLIP_HITSCAN,
		MAPTEST_CLIP_NEW,
	};

	struct MeshVertex {
		enum { FVF = D3DFVF_XYZ | D3DFVF_TEX1 };
		vec3	pos;
		vec2	uv;
	};

	class MapMesh {
	public:
		DevMesh				mesh;
		vector<MeshVertex>	vb;
		vector<int>			ib;

		void AddWallQuad(float x1, float y1, float x2, float y2, float h1, float h2, float u1, float v1, float u2, float v2);
	};

	struct CamPos {
		float	delay;
		vec3	pos;
		vec3	ypr;
	};

	struct MapFileInfo {
		string	name;
		string	path;
		DWORD	color_off = 0xFFC0C0C0;
		DWORD	color_on  = 0xFFFFFFFF;
	};

	struct ModDirInfo {
		string	name;
		string	path;
		bool	enabled = false;
	};

	DataTexture					screen;
	WadFile						map_wad;
	WadFile::MapInfo			wad_mapinfo;
	WadConverter				wad_converter;
	ScriptCompiler				script_compiler;
	MapFileHeader				*converted_map;
	unsigned long long			map_filetime = 0;
	int							reload_suppress_frames = 0;
	string						err_message;
	map<texture_id_t, MapMesh*>	map_meshes;
	vector<CamPos>				cam_lag;
	vec3						real_cam_pos = vec3(0, 0, 40);
	vec3						real_cam_ypr = vec3(0, 0, 0);
	vec3						cam_pos = vec3(0, 0, 40);
	vec3						cam_ypr = vec3(0, 0, 0);
	DevRenderTarget				off_rt;
	DevRenderTarget				main_rt;
	DataTexture					tex_palette;
	float						frame_timer = 0;

	// tex shadow
	DataTexture					prev_screen;
	float						prev_timer_show = 0;
	float						prev_timer_fade = 0;

	// settings
	int							tool_mode = TOOL_3DVIEW;
	int							rendermode = RMODE_GPU;
	int							refresh_time = 35;
	int							refresh_lag = 0;
	int							dither_mode = DITHER_HQ;
	int							physics_mode = PHYS_NOCLIP_GRAVITY;
	int							mapview_mode = MAPVIEW_LINE_VIS;
	int							mapview_test = MAPTEST_CLIP_WALK;
	int							prev_shadow_mode = 0;
	int							full_screen_mode = 0;
	int							preview_zoom = 0;
	int							preview_yx2 = 0;


	~RenderTestApp()
	{
	//	Shutdown();
	}

	virtual const char *GetName() { return "Render Test"; }

	virtual void Init();
	virtual void Shutdown();

	virtual void Run();
	virtual void RunMenu();

protected:

	// soft
	enum {
		SOFT_MAX_VERT	= 10,
		SOFT_MAX_LINES	= 10
	};
	vector<int>				soft_size_map;
	vector<int>				soft_size_map_step;
	vector<vector<int> >	soft_scaler;
	int soft_step_debug = -1;
	short lrc_cam_pos_x;							// camera
	short lrc_cam_pos_y;
	short lrc_cam_pos_z;
	short lrc_cam_angle;
	word  lrc_n_vert = 0;							// data
	short lrc_vert_x[SOFT_MAX_VERT];
	short lrc_vert_y[SOFT_MAX_VERT];
	word  lrc_n_lines = 0;
	word  lrc_lines_v1[SOFT_MAX_LINES];
	word  lrc_lines_v2[SOFT_MAX_LINES];
	word  lrc_lines_tex[SOFT_MAX_LINES];
	word  lrc_lines_tx2[SOFT_MAX_LINES];
	short lrc_vert_final_x[SOFT_MAX_VERT];			// internal
	short lrc_vert_final_y[SOFT_MAX_VERT];

	// map view
	MapDraw					mv;
	vec2					mv_eyepos = vec2(0, 0);
	DreadMapGen::RegionPoly	*mv_vis_region = NULL;
	DreadMapGen::Line		*mv_sel_line = NULL;
	DreadMapGen::RegionPoly	*mv_sel_region = NULL;
	DreadMapGen				mapgen;
	DreadSoftRenderer		renderer;
	DreadSim				dread_sim;

	// export
	ChunkyColorPacker		chunky_packer;
	bool					script_gc_mode = false;

	// developer tests
	CollisionTestTool		coltest;


	void MenuDitherSettings();
	bool MenuListOptionBool(const char *name, bool &value);
	void MenuExportOptions();

	void Run3DView();
	void RunTextures();
	void RunMapView();

	enum {
		MVDL_ONE_SIDED			= (1<<0),
		MVDL_TWO_SIDED			= (1<<1),
		MVDL_SELECTED_ONLY		= (1<<2),
		MVDL_CENTER_ZOOM		= (1<<3),
		MVDL_VISIBLE_ONLY		= (1<<4),
		MVDL_DEBUG_MARK_ONLY	= (1<<5),
		MVDL_HIDE_NOT_REQUIRED	= (1<<6),
	
		MVDR_MARKED_ONLY		= (1<<0),
		MVDR_SELECTED_ONLY		= (1<<1),
		MVDR_DRAW_VERTEXES		= (1<<2),
		MVDR_DRAW_LINE_MARKERS	= (1<<3),
		MVDR_NULL_USER_ONLY		= (1<<4),
		MVDR_VISREGION_ONLY		= (1<<5),
	};
	void MV_DrawLines(int flags, DWORD color);
	void MV_DrawPolygon(Polygon2D &p, int flags, DWORD color);
	void MV_DrawMesh(PolygonMesh2D &mesh, int flags, DWORD color);
	void MV_DrawRegion(DreadMapGen::RegionPoly &p, int flags, DWORD color);
	void MV_DrawRegions(int flags, DWORD color);
	void MV_DrawVisibleRegions(int flags, DWORD color);
	void MV_DrawColEdges(vector<DreadMapGen::ColEdge> &edges, int flags, DWORD color);


	void CompileScript(bool gc_actors);

	void PrepareMap();
	void WatchMap();
	void WatchAssets(bool force_reload);
	void FreeMapMeshes();
	MapMesh *AssureMapMesh(const texture_id_t &key);
	void BuildMapMeshes();
	void BuildMapMeshes2();
	void PreparePalette(GfxPalette &pal);
	void RenderMapMeshes();
	MapFileSector *BspFindSector(int xp, int yp);
	bool BspClip(DreadMapGen::ColBSPNode *node, const vec2 &p1, const vec2 &p2, vec2 &hit, vec2 &normal);
	
	struct BspClipArgs {
		short	p1x;	// hit point is found
		short	p1y;
		short	p2x;
		short	p2y;
		short	nA;
		short	nB;
		int		nC;
	};
	bool BspClipFix(DreadMapGen::ColBSPNode *node, BspClipArgs &state);
	bool BspClipCheckFix(DreadMapGen::ColBSPNode *node, int px, int py);

	// soft functions
	void SoftGenScaler(vector<int> &out, int size);
	void SoftInit();
	void Wolf_LineWall_Fix(int p1x, int p1y, int p2x, int p2y, int tx1, int tx2, texture_id_t tex);
	void SoftTest();

	// export
	//void ExportFrame(FILE *fp);
	void ExportSimpleObject(FILE *fp, GfxTexture *tex, const char *name, int &total);
	void ExportObjects(FILE *fp);
	void ExportSky(FILE *fp);
	int  ExportWeaponSprite(FILE *fp, GfxAsset *asset);
	int  ExportWeaponAnimation(FILE *fp, GfxAnimation *anim);
	void ExportWeaponSpritesAll(FILE *fp);
	void ExportWeapon(FILE *fp);
	void ExportWeaponST(FILE *fp);
	void Export(platform_t platform);
	
	void RunWinUAE(const char *config);
};


extern DevEffect fx_render;
extern RenderTestApp app_rendertest;
