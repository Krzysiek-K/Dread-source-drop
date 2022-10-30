
#include "common.h"
#include "app_rendertest.h"




RenderTestApp app_rendertest;

DevEffect fx_render("app_data/render.fx");


short sincos_fix14[256];


void init_sincos()
{
	int cos = 1<<14;
	int sin = 0;
	for( int i=0; i<64; i++ )
	{
		sincos_fix14[i] = (short)cos;
		sincos_fix14[i+64] = (short)-sin;
		sincos_fix14[i+128] = (short)-cos;
		sincos_fix14[i+192] = (short)sin;
		sin += (cos*1608)>>16;
		cos -= (sin*1608)>>16;
	}
}


//byte _color2chunky(DWORD col)
//{
//	byte b = 0;
//#if 0
//	// Amiga
//	if( col & 0x0001 ) b |= 0x80;
//	if( col & 0x0002 ) b |= 0x20;
//	if( col & 0x0004 ) b |= 0x08;
//	if( col & 0x0008 ) b |= 0x02;
//
//	if( col & 0x0100 ) b |= 0x40;
//	if( col & 0x0200 ) b |= 0x10;
//	if( col & 0x0400 ) b |= 0x04;
//	if( col & 0x0800 ) b |= 0x01;
//#else
//	// Atari ST
//	if( col & 0x0001 ) b |= 0x04;
//	if( col & 0x0002 ) b |= 0x08;
//	if( col & 0x0004 ) b |= 0x10;
//	if( col & 0x0008 ) b |= 0x20;
//#endif
//	return b;
//}





// ---------------------------------------------------------------- RenderTestApp ----------------------------------------------------------------


void RenderTestApp::MapMesh::AddWallQuad(float x1, float y1, float x2, float y2, float h1, float h2, float u1, float v1, float u2, float v2)
{
	int vbase = vb.size();
	for( int v=0; v<4; v++ )
	{
		MeshVertex mv;
		mv.pos.x = (v==0 || v==3 ? x1 : x2);
		mv.pos.y = (v==0 || v==3 ? y1 : y2);
		mv.pos.z = (v==0 || v==1 ? h1 : h2);
		mv.uv.x = (v==0 || v==3 ? u1 : u2);
		mv.uv.y = (v==0 || v==1 ? v1 : v2);
		vb.push_back(mv);
	}

	ib.push_back(vbase+0);
	ib.push_back(vbase+1);
	ib.push_back(vbase+2);
	ib.push_back(vbase+0);
	ib.push_back(vbase+2);
	ib.push_back(vbase+3);
}



void RenderTestApp::Init()
{
	tool_mode = cfg.GetInt("rendertest-tool-mode", TOOL_MODS);
	rendermode = cfg.GetInt("rendertest-rendermode", RMODE_GPU);
	refresh_time = cfg.GetInt("rendertest-refresh-time", 35);
	refresh_lag = cfg.GetInt("rendertest-refresh-lag", 0);
	dither_mode = cfg.GetInt("rendertest-dither-mode", DITHER_VERT);
	physics_mode = cfg.GetInt("rendertest-physics-mode", PHYS_NOCLIP_GRAVITY);
	mapview_mode = cfg.GetInt("rendertest-mapview-mode", MAPVIEW_LINE_VIS);
	mapview_test = cfg.GetInt("rendertest-mapview-test", MAPTEST_CLIP_WALK);
	prev_shadow_mode = cfg.GetInt("rendertest-prev-shadow-mode", 0);
	full_screen_mode = cfg.GetInt("rendertest-full-screen-mode", 0);
	preview_zoom = cfg.GetInt("rendertest-preview-zoom", 0);
	preview_yx2 = cfg.GetInt("rendertest-preview-Yx2", 0);

	coltest.Serialize(cfg.GetNode("coltest", 0, false));

	mapgen.export_deflate = (cfg.GetInt("mapgen-export-deflate", mapgen.export_deflate ? 1 : 0) != 0);
	mapgen.export_vis_groups = (cfg.GetInt("mapgen-export-vis-groups", mapgen.export_vis_groups ? 1 : 0) != 0);
	mapgen.map_col_round_corners = (cfg.GetInt("mapgen-col-round-corners", mapgen.map_col_round_corners ? 1 : 0) != 0);
	mapgen.map_optimized_bsp = (cfg.GetInt("mapgen-optimized-bsp", mapgen.map_optimized_bsp ? 1 : 0) != 0);

	modmgr.Init();



	screen.Resize(160, 100);
	init_sincos();

	gfx_converter.Init();
	gfx_converter.sel_asset_name = cfg.GetString("rendertest-selected-asset", "");

	for( int nv = 1; cfg.GetNode(format("rendertest-selected-version-%d", nv).c_str(), 0, false).IsValid(); nv++ )
		gfx_converter.sel_versions[cfg.GetString(format("rendertest-selected-version-%d", nv).c_str(), "")] = true;

	WatchAssets(true);
	CompileScript(false);
	PrepareMap();

	SoftInit();
}

void RenderTestApp::Shutdown()
{
	cfg.SetInt("rendertest-tool-mode", tool_mode);
	cfg.SetInt("rendertest-rendermode", rendermode);
	cfg.SetInt("rendertest-refresh-time", refresh_time);
	cfg.SetInt("rendertest-refresh-lag", refresh_lag);
	cfg.SetInt("rendertest-dither-mode", dither_mode);
	cfg.SetInt("rendertest-physics-mode", physics_mode);
	cfg.SetInt("rendertest-mapview-mode", mapview_mode);
	cfg.SetInt("rendertest-mapview-test", mapview_test);
	cfg.SetInt("rendertest-prev-shadow-mode", prev_shadow_mode);
	cfg.SetInt("rendertest-full-screen-mode", full_screen_mode);
	cfg.SetInt("rendertest-preview-zoom", preview_zoom);
	cfg.SetInt("rendertest-preview-Yx2", preview_yx2);

	cfg.SetInt("mapgen-export-deflate", mapgen.export_deflate ? 1 : 0);
	cfg.SetInt("mapgen-export-vis-groups", mapgen.export_vis_groups ? 1 : 0);
	cfg.SetInt("mapgen-col-round-corners", mapgen.map_col_round_corners ? 1 : 0);
	cfg.SetInt("mapgen-optimized-bsp", mapgen.map_optimized_bsp ? 1 : 0);

	coltest.Serialize(cfg.GetNode("coltest", 0, true));

	 
	modmgr.Shutdown();


	cfg.SetString("rendertest-selected-asset", gfx_converter.sel_asset_name.c_str());

	int nv = 1;
	for( auto &p : gfx_converter.sel_versions )
		if( p.second )
			cfg.SetString(format("rendertest-selected-version-%d", nv++).c_str(), p.first.c_str());
	
	for( ; nv<10; nv++ )
	cfg.SetString(format("rendertest-selected-version-%d", nv++).c_str(), "");

	FreeMapMeshes();
}

void RenderTestApp::Run()
{
	if( reload_suppress_frames > 0 )
		reload_suppress_frames--;
	else
	{
		WatchAssets(false);
		WatchMap();
	}

	// Common setup
	Dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	Dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	Dev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	Dev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	Dev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	if( tool_mode == TOOL_MODS || tool_mode == TOOL_3DVIEW )
		Run3DView();
	else if( tool_mode == TOOL_TEXTURES )
		RunTextures();
	else if( tool_mode == TOOL_MAP )
		RunMapView();
	else if( tool_mode == TOOL_DEV )
	{
		if( proj_cfg.GetInt("dev-mode", 0) )
			coltest.RunView();
	}
	else if( tool_mode == TOOL_ELOG )
		elog.Draw(0, Dev.GetSizeX()-MENU_WIDTH);


	Dev.DPrintF("%s", err_message.c_str());
}

void RenderTestApp::RunMenu()
{
	ms.Panel();

	if( !full_screen_mode )
	{
		int dev_mode = proj_cfg.GetInt("dev-mode", 0);

		if( dev_mode )
		{
			ms.LineSetup(-1, -1, -1, -1, -1, -1);
			ms.Group(6);
		}
		else
		{
			ms.LineSetup(-1, -1, -1, -1, -1);
			ms.Group(5);
		}
		ms.EnumButton(NULL, "File", tool_mode, TOOL_MODS);
		ms.EnumButton(NULL, "3D", tool_mode, TOOL_3DVIEW);
		ms.EnumButton(NULL, "Tex", tool_mode, TOOL_TEXTURES);
		ms.EnumButton(NULL, "Map", tool_mode, TOOL_MAP);
		if( dev_mode )
			ms.EnumButton(NULL, "Dev", tool_mode, TOOL_DEV);

		ErrorLog::error_level_t errorlevel = elog.GetErrorLevel();
		if( errorlevel==ErrorLog::LEVEL_ERROR )	ms.EnumButton(NULL, "Err", tool_mode, TOOL_ELOG, 0, 4);
		else if( errorlevel==ErrorLog::LEVEL_WARNING )	ms.EnumButton(NULL, "Err", tool_mode, TOOL_ELOG, 0, 3);
		else											ms.EnumButton(NULL, "Err", tool_mode, TOOL_ELOG, 0, 0);
	}

	if( tool_mode == TOOL_MODS )
	{
		MenuExportOptions();

		int reload = modmgr.RunFileMenu();

		if( reload )
			elog.Clear();

		if( reload & ModManager::RELOAD_ASSETS )
		{
			gfx_converter.watched_files.clear();
			map_filetime = 0;
			//reload_suppress_frames = 2;
		}

		if( reload & ModManager::RELOAD_MAP )
		{
			map_filetime = 0;
			//reload_suppress_frames = 2;
		}

	}
	else if( tool_mode == TOOL_3DVIEW )
	{
		ms.LineSetup(-1, -1);
		ms.Group(2);
		ms.EnumButton(NULL, "GPU", rendermode, RMODE_GPU);
		//ms.EnumButton(NULL, "Soft", rendermode, RMODE_SOFT);
		ms.EnumButton(NULL, "Dread", rendermode, RMODE_SOFT2);

		MenuDitherSettings();
		gfx_converter.MenuVersionSelector();

		ms.LineSetup(-1, -1, -1);
		ms.Group(3);
		ms.EnumButton(NULL, "Gravity", physics_mode, PHYS_NOCLIP_GRAVITY);
		ms.EnumButton(NULL, "Fly", physics_mode, PHYS_NOCLIP);
		ms.EnumButton(NULL, "Clip", physics_mode, PHYS_BSP_CLIP);

		ms.LineSetup(-1, -1, -1);
		ms.Label("Doors");
		ms.Group(2);
		if( ms.Button(NULL, "Open") ) mapgen.SetDoorsPosition(0), BuildMapMeshes();
		if( ms.Button(NULL, "Closed") ) mapgen.SetDoorsPosition(100), BuildMapMeshes();

		ms.Slider(format("%.1f frames", refresh_time/10.f).c_str(), refresh_time, 10, 100);
		ms.Slider(format("%.1f lag", refresh_lag/10.f).c_str(), refresh_lag, 0, 100);

		MenuExportOptions();
	}
	else if( tool_mode == TOOL_TEXTURES )
	{
		if( !full_screen_mode )
			MenuDitherSettings();

		ms.LineSetup(-1, -1, -1, -1);
		if( ms.Button(NULL, "Shadow", prev_shadow_mode ? 1 : 0) ) prev_shadow_mode = !prev_shadow_mode;
		if( ms.Button(NULL, "Full", full_screen_mode ? 1 : 0) ) full_screen_mode = !full_screen_mode;
		if( ms.Button(NULL, "Dump") ) gfx_converter.SaveAllGfxPreviews("dump");
		if( ms.Button(NULL, "PK3") ) gfx_converter.BuildPK3Structure("pk3");

		if( !full_screen_mode )
		{
			ms.LineSetup(-1, -1, -1, -1, -1, -1);
			ms.Group(5);
			for( int i=0; i<=4; i++ )
				if( ms.Button(NULL, i ? format("x%d", i).c_str() : "Auto", (preview_zoom==i) ? 1 : 0) )
					preview_zoom = i;

			if( ms.Button(NULL, "Yx2", preview_yx2 ? 1 : 0) )
				preview_yx2 = !preview_yx2;

			gfx_converter.MenuVersionSelector();
			ms.Label("");
			gfx_converter.MenuAssetsList();
		}
	}
	else if( tool_mode == TOOL_MAP )
	{
		ms.LineSetup(-1, -1, -1);
		ms.Group(3);
		ms.EnumButton(NULL, "Vis", mapview_mode, MAPVIEW_LINE_VIS);
		ms.EnumButton(NULL, "Vis2", mapview_mode, MAPVIEW_LINE_VIS2);
		ms.EnumButton(NULL, "TVis", mapview_mode, MAPVIEW_THING_VIS);
		//ms.EnumButton(NULL, "RVis", mapview_mode, MAPVIEW_REGION_VIS);
		ms.LineSetup(-1, -1, -1, -1);
		ms.Label("Col");
		ms.Group(3);
		ms.EnumButton(NULL, "Mesh", mapview_mode, MAPVIEW_COL_MESH);
		ms.EnumButton(NULL, "Edges", mapview_mode, MAPVIEW_COL_EDGES);
		ms.EnumButton(NULL, "BSP", mapview_mode, MAPVIEW_COL_BSP);

		ms.LineSetup(-1, -1, -1, -1);
		ms.Label("Clip");
		ms.Group(3);
		ms.EnumButton(NULL, "Walk", mapview_test, MAPTEST_CLIP_WALK);
		ms.EnumButton(NULL, "Hit", mapview_test, MAPTEST_CLIP_HITSCAN);
		ms.EnumButton(NULL, "New", mapview_test, MAPTEST_CLIP_NEW);

		ms.LineSetup(-1, -1, -1, -1);
		ms.Label("Doors");
		ms.Group(3);
		if( ms.Button(NULL, "Open") ) mapgen.SetDoorsPosition(0), BuildMapMeshes();
		if( ms.Button(NULL, "Half") ) mapgen.SetDoorsPosition(50), BuildMapMeshes();
		if( ms.Button(NULL, "Closed") ) mapgen.SetDoorsPosition(100), BuildMapMeshes();


		MenuExportOptions();

		if( mv_sel_line && mapview_mode!=MAPVIEW_THING_VIS )
		{
			ms.EndPanel();
			ms.Panel();

			ms.Label(format("Line %d   [L%d]", mv_sel_line->_number, mv_sel_line->_debug_number).c_str());
			ms.LineSetup(-1, -3);	ms.Label("H1");		ms.Label(format("%d", mv_sel_line->h1).c_str(), 1);
			ms.LineSetup(-1, -3);	ms.Label("H2");		ms.Label(format("%d", mv_sel_line->h2).c_str(), 1);
			ms.LineSetup(-1, -3);	ms.Label("H3");		ms.Label(format("%d", mv_sel_line->h3).c_str(), 1);
			ms.LineSetup(-1, -3);	ms.Label("H4");		ms.Label(format("%d", mv_sel_line->h4).c_str(), 1);

			ms.LineSetup(-1, -3);	ms.Label("Ceil");	ms.Label(gfx_converter.GetTextureName(mv_sel_line->tex_ceil), 1);
			ms.LineSetup(-1, -3);	ms.Label("Upper");	ms.Label(gfx_converter.GetTextureName(mv_sel_line->tex_upper), 1);
			ms.LineSetup(-1, -3);	ms.Label("Lower");	ms.Label(gfx_converter.GetTextureName(mv_sel_line->tex_lower), 1);
			ms.LineSetup(-1, -3);	ms.Label("Floor");	ms.Label(gfx_converter.GetTextureName(mv_sel_line->tex_floor), 1);

			if( mv_vis_region )
			{
				ms.EndPanel();
				ms.Panel();

				if( mv_vis_region->_number >= 0 )
					ms.Label(format("Region ID %d  [%d]", mv_vis_region->_number, mv_vis_region->_vis_line_count).c_str());

				auto p = mv_vis_region->vis.find(mv_sel_line);
				if( p!=mv_vis_region->vis.end() )
				{
					static const char *NAMES_LC[] ={
#include "gen/cheatnames.inc"
					};
					static const char *NAMES_LC2[] ={
#include "gen/render2_cheatnames.inc"
					};

					const char **LC = (mapgen.export_engine_version == 2) ? NAMES_LC2 : NAMES_LC;
					int num_modes = (mapgen.export_engine_version == 2) ? sizeof(NAMES_LC2)/sizeof(NAMES_LC2[0]) : sizeof(NAMES_LC)/sizeof(NAMES_LC[0]);

					DreadMapGen::VisLine *vis = p->second;
					ms.Label(format("T=%d/%d   Visible #%d", vis->draw_order, mv_vis_region->_vis_line_count, vis->cheat_mode).c_str());
					if( vis->cheat_mode>=0 && vis->cheat_mode<num_modes )
						ms.Label(LC[vis->cheat_mode]);
					ms.Label(format("Y1 min = %.1f", vis->range_y1_min).c_str());
					ms.Label(format("Y1 clip = %.1f", vis->range_y1_clip).c_str());
					if( vis->condition>=0 )
					{
						ms.Label(format("Condition %d", vis->condition).c_str());
						ms.Label(mapgen.GetConditionName(vis->condition, false).c_str());
					}
					if( mv_sel_line->_number >= 0 )
						ms.Label(format("Line ID %d", mv_sel_line->_number).c_str());
				}
			}
		}

		if( mv_sel_region && mapview_mode==MAPVIEW_THING_VIS )
		{
			if( mv_vis_region )
			{
				ms.EndPanel();
				ms.Panel();
				ms.Label(format("Region %d", mv_sel_region->_number).c_str());

				auto p = mv_vis_region->svis.find(mv_sel_region->sector);
				if( p!=mv_vis_region->svis.end() )
				{
					DreadMapGen::VisSector *vis = p->second;
					ms.Label(format("T=%d/%d   Visible", vis->draw_order, mv_vis_region->vis.size()).c_str());
					if( vis->condition>=0 )
					{
						ms.Label(format("Condition %d", vis->condition).c_str());
						ms.Label(mapgen.GetConditionName(vis->condition, false).c_str());
					}
				}
			}
		}

	}
	else if( tool_mode == TOOL_DEV )
	{
		if( proj_cfg.GetInt("dev-mode", 0) )
			coltest.RunMenu();
	}


	ms.EndPanel();
}

void RenderTestApp::MenuDitherSettings()
{
	ms.LineSetup(-1, -1, -1, -1, -1);
	ms.Group(5);
	ms.EnumButton(NULL, "Off", dither_mode, DITHER_OFF);
	ms.EnumButton(NULL, "Vert", dither_mode, DITHER_VERT);
	ms.EnumButton(NULL, "Shake", dither_mode, DITHER_SHAKE);
	ms.EnumButton(NULL, "Scan", dither_mode, DITHER_SCAN);
	ms.EnumButton(NULL, "HQ", dither_mode, DITHER_HQ);
}

bool RenderTestApp::MenuListOptionBool(const char *name, bool &value)
{
//	if( ms.ListItem(format("%s: %s", name, value ? "yes" : "no").c_str(), 0xFFFF00, false) )
	if( ms.ListItem(format("%s %s", value ? "[X]" : "[  ]", name).c_str(), 0xFFFF00, false) )
	{
		value = !value;
		return true;
	}
	return false;
}

void RenderTestApp::MenuExportOptions()
{
//	ms.LineSetup(-1, -1, -1);
//	ms.Label("Map opt");
//	if( ms.Button(NULL, "Deflate", mapgen.export_deflate ? 1 : 0) ) mapgen.export_deflate = !mapgen.export_deflate;
//	if( ms.Button(NULL, "VisGroup", mapgen.export_vis_groups ? 1 : 0) ) mapgen.export_vis_groups = !mapgen.export_vis_groups;
	ms.ListStart();
	ms.ListItem("--- Map options ---", 0xFFFF00, false);
	MenuListOptionBool("Deflate", mapgen.export_deflate);
	MenuListOptionBool("VisGroup", mapgen.export_vis_groups);
	MenuListOptionBool("ColRoundCorners", mapgen.map_col_round_corners);
	MenuListOptionBool("OptBSP", mapgen.map_optimized_bsp);
	ms.ListEnd();

	if( ms.Button(NULL, "Reload map") )
		PrepareMap();
	
	ms.LineSetup(-1, -1, -1);
	ms.Label("Export");
	if( ms.Button(NULL, "Amiga") ) Export(PLATFORM_AMIGA);
	if( ms.Button(NULL, "ST") ) Export(PLATFORM_ST);

	ms.LineSetup(-1, -1, -1);
	ms.Label("Exp & Run");
	if( ms.Button(NULL, "A500") ) Export(PLATFORM_AMIGA), RunWinUAE(proj_cfg.GetString("run-a500-config", "A500_HD.uae").c_str());
	if( ms.Button(NULL, "A1200") ) Export(PLATFORM_AMIGA), RunWinUAE(proj_cfg.GetString("run-a1200-config", "A1200_HD.uae").c_str());

	ms.LineSetup(-4, -1);
	if( ms.Button(NULL, "Compile script") ) script_gc_mode = false, CompileScript(script_gc_mode);
	if( ms.Button(NULL, "GC") ) script_gc_mode = true, CompileScript(script_gc_mode);
}


void RenderTestApp::Run3DView()
{
	float frame_time = refresh_time/10.f/50;
	frame_timer += Dev.GetTimeDelta();
	if( frame_timer >= frame_time )
	{
		frame_timer -= frame_time;
		if( frame_timer >= frame_time )
			frame_timer = frame_time;

		// Walk
		vec3 old_cam_pos = real_cam_pos;
		float speed = 256;
		float turnspeed = 200;
		if( Dev.GetKeyState(VK_SHIFT) ) speed = 1024;
		if( Dev.GetKeyState(VK_CONTROL) ) speed = 4096;
		if( Dev.GetKeyState(VK_RBUTTON) )
			Dev.TickFPSCamera(real_cam_pos, real_cam_ypr, speed*frame_time/Dev.GetTimeDelta(), 0.1f);
		else
			Dev.SetMouseCapture(false);
		if( Dev.GetKeyState(VK_LEFT) ) real_cam_ypr.x -= frame_time*turnspeed;
		if( Dev.GetKeyState(VK_RIGHT) ) real_cam_ypr.x += frame_time*turnspeed;

		// gravity
		if( physics_mode == PHYS_NOCLIP_GRAVITY )
		{
			static float yvel = 0;
			MapFileSector *mapsec = BspFindSector(real_cam_pos.x, -real_cam_pos.y);
			if( mapsec )
			{
				float h = mapsec->floor_h + 40;

				yvel -= 8000 * Dev.GetTimeDelta();
				real_cam_pos.z += yvel * Dev.GetTimeDelta();

				if( h >= real_cam_pos.z )
				{
					real_cam_pos.z = h;
					yvel = 0;
				}
			}
			else
			{
				real_cam_pos.z = 40;
				yvel = 0;
			}
		}
		if( physics_mode == PHYS_BSP_CLIP )
		{
			//vec2 p1 = vec2(old_cam_pos.x, -old_cam_pos.y);
			//vec2 p2 = vec2(real_cam_pos.x, -real_cam_pos.y);
			//vec2 hit, normal(0,0);
			//if( BspClip(mapgen.col_bsp_root, p1, p2, hit, normal) )
			//{
			//	real_cam_pos.x = hit.x;
			//	real_cam_pos.y = -hit.y;
			//}

			BspClipArgs state;
			short startx = state.p1x =  (int)floor(old_cam_pos.x*16+0.5f);
			short starty = state.p1y = -(int)floor(old_cam_pos.y*16+0.5f);
			short endx = state.p2x =  (int)floor(real_cam_pos.x*16+0.5f);
			short endy = state.p2y = -(int)floor(real_cam_pos.y*16+0.5f);
			if( BspClipFix(mapgen.col_bsp_root_psize, state) )
			{
				// Slide
				int es = muls(endx, state.nA) + muls(endy, state.nB) + state.nC;
				int ln = muls(state.nA, state.nA) + muls(state.nB, state.nB);
				endx -= state.nA*es/ln;
				endy -= state.nB*es/ln;

				if( state.nA<0 ) endx++;
				if( state.nA>0 ) endx--;
				if( state.nB<0 ) endy++;
				if( state.nB>0 ) endy--;

				state.p1x = startx;
				state.p1y = starty;
				state.p2x = endx;
				state.p2y = endy;

				if( BspClipFix(mapgen.col_bsp_root_psize, state) )
				{
					state.p2x = state.p1x;
					state.p2y = state.p1y;
					if( BspClipCheckFix(mapgen.col_bsp_root_psize, state.p1x, state.p1y) )
					{
						real_cam_pos.x = startx/16.f;
						real_cam_pos.y = -starty/16.f;
					}
					else
					{
						real_cam_pos.x = state.p1x/16.f;
						real_cam_pos.y = -state.p1y/16.f;
					}
				}
				else
				{
					real_cam_pos.x = endx/16.f;
					real_cam_pos.y = -endy/16.f;
				}

				//if( state.hitnode )
				//	printf("A = %d  B = %d  C = %d  %s\n", state.hitnode->A, state.hitnode->B, state.hitnode->C, state.hit_side ? "back" : "front");
			}
			else
			{
				real_cam_pos.x = endx/16.f;
				real_cam_pos.y = -endy/16.f;
			}
		}
		real_cam_ypr.y = 0;

		CamPos cp;
		cp.delay = refresh_lag/10.f/50 - frame_timer + Dev.GetTimeDelta();
		cp.pos.x = floor(real_cam_pos.x*16+0.5f)/16;
		cp.pos.y = floor(real_cam_pos.y*16+0.5f)/16;
		cp.pos.z = floor(real_cam_pos.z+0.5f);
		cp.ypr.x = floor(real_cam_ypr.x/360*256+0.5)/256*360;
		cp.ypr.y = 0;
		cp.ypr.z = 0;
		cam_lag.push_back(cp);
	}
	Dev.DPrintF("X = %.2f", real_cam_pos.x);
	Dev.DPrintF("Y = %.2f", -real_cam_pos.y);

	for( int i=0; i<cam_lag.size(); i++ )
	{
		cam_lag[i].delay -= Dev.GetTimeDelta();
		if( cam_lag[i].delay <= 0 )
		{
			cam_pos = cam_lag[i].pos;
			cam_ypr = cam_lag[i].ypr;
			cam_lag.erase(cam_lag.begin()+i);
			i--;
		}
	}

	// Set camera
	D3DXMATRIX vp;
	Dev.BuildCameraViewProjMatrix(&vp, cam_pos, cam_ypr, atan(100.f/160)/M_PI*180*2, 160.f/100.f, 0.1, 0);
	Dev->SetTransform(D3DTS_PROJECTION, &vp);

	// Init render
	off_rt.SetParameters(D3DFMT_A8R8G8B8, 320/2, 200/2, 0);
	main_rt.SetParameters(D3DFMT_A8R8G8B8, 320, 200, 0);

	if( rendermode==RMODE_GPU )
	{
		// Render sky
		//GfxTexture *sky_tex = gfx_converter.FindTexture("SKY", 255);
		Dev.SetRenderTarget(0, &off_rt);
		//Dev->SetTexture(0, sky_tex ? sky_tex->texture.tex.GetTexture() : NULL);
		//Dev.SetRState(RSF_NO_ZENABLE);
		//int dx = int(floor(cam_ypr.x/360*128*4))&127;
		//Dev.DrawScreenQuad(-dx, 0, 256/2-dx, 128/2);
		//dx -= 128;
		//Dev.DrawScreenQuad(-dx, 0, 256/2-dx, 128/2);
		//dx -= 128;
		//Dev.DrawScreenQuad(-dx, 0, 256/2-dx, 128/2);
		Dev->Clear(0, NULL, D3DCLEAR_TARGET, 0x00008000, 1.f, 0);

		// Render world
		fx_render.SetMatrix("ViewProj", vp);

		// Draw map
		RenderMapMeshes();

		// Draw objects
		vec3 cam_front, cam_right, cam_up;
		Dev.BuildCameraVectors(cam_ypr, &cam_front, &cam_right, &cam_up);
		fx_render.StartTechnique("sprites");
		fx_render.SetFloat3("cam_right", cam_right);
		while( fx_render.StartPass() )
		{
			for( int i=0; i<(int)mapgen.things.size(); i++ )
			{
				MapFileThing &t = mapgen.things[i];

				for( int j=0; j<(int)script_compiler.actors.size(); j++ )
					if( script_compiler.actors[j]->thing_id && script_compiler.actors[j]->thing_id == t.type && script_compiler.actors[j]->editor_frame_id )
					{
						GfxTexture *tex = gfx_converter.FindTexture(script_compiler.idents[script_compiler.actors[j]->editor_frame_id].name.c_str(), -1);
						if( tex )
						{
							Dev->SetTexture(0, tex->texture.tex);
							fx_render.SetFloat2("tex_size", vec2(tex->orig_w, tex->orig_h));
							fx_render.SetFloat2("tex_offs",vec2(tex->orig_ox, tex->orig_oy));
							fx_render.CommitChanges();

							vec3 pos;
							pos.x = t.xp;
							pos.y = t.yp;
							pos.z = 0;
							Dev.DrawScreenQuadVS(pos.x, pos.y, pos.x, pos.y, pos.z);
						}
					}
			}
		}

		// Resolve
		Dev.SetRenderTarget(0, &main_rt);
		Dev->SetTexture(0, off_rt);
		Dev->SetTexture(1, tex_palette.tex);
		fx_render.SetFloat("pal_size", tex_palette.width);
		fx_render.SetFloat2("ssize", main_rt.GetCurrentSizeV()*0.5f);
		fx_render.StartTechnique(format("resolve%d", dither_mode).c_str());
		while( fx_render.StartPass() )
			Dev.DrawScreenQuad(0, 0, 320, 200);
	}
	else if( rendermode==RMODE_SOFT )
	{
		screen.Resize(160, 100);
		SoftTest();
		screen.Upload();

		// Resolve
		Dev.SetRenderTarget(0, &main_rt);
		Dev->SetTexture(0, screen.tex);
		Dev->SetTexture(1, tex_palette.tex);
		fx_render.SetFloat("pal_size", tex_palette.width);
		fx_render.SetFloat2("ssize", main_rt.GetCurrentSizeV()*0.5f);
		fx_render.StartTechnique(format("resolve%d", dither_mode).c_str());
		while( fx_render.StartPass() )
			Dev.DrawScreenQuad(0, 0, 320, 200);
	}
	else if( rendermode==RMODE_SOFT2 )
	{
		mapgen.AssureVisibility();
		renderer.Draw(screen, cam_pos, cam_ypr.x);
		screen.Upload();

		// Resolve
		Dev.SetRenderTarget(0, &main_rt);
		Dev->SetTexture(0, screen.tex);
		Dev->SetTexture(1, tex_palette.tex);
		fx_render.SetFloat("pal_size", tex_palette.width);
		fx_render.SetFloat2("ssize", main_rt.GetCurrentSizeV()*0.5f);
		fx_render.StartTechnique(format("resolve%d", dither_mode).c_str());
		while( fx_render.StartPass() )
			Dev.DrawScreenQuad(0, 0, 320, 200);
	}

	// Flush
	{
		vec2 ss = Dev.GetScreenSizeV();
		int scale = (int)min((ss.x-MENU_WIDTH)/320, ss.y/200);
		int x0 = (ss.x-MENU_WIDTH - 320*scale)/2;
		int y0 = (ss.y - 24 - 200*scale)/2;
		if( y0<0 ) y0 = 0;

		Dev.SetRenderTarget(0, NULL);
		Dev->SetTexture(0, main_rt);
		Dev.SetRState(RSF_NO_ZWRITE);
		Dev.DrawScreenQuad(x0, y0, x0+320*scale, y0+200*scale);
	}

	// Draw palette
	if( gfx_converter.main_palette )
	{
		const int ps = 24;
		PreparePalette(*gfx_converter.main_palette);
		Dev->SetTexture(0, tex_palette.tex);
		Dev.DrawScreenQuad(0, Dev.GetScreenSizeV().y-tex_palette.height*ps, tex_palette.width*ps, Dev.GetScreenSizeV().y);
	}
}


void RenderTestApp::RunTextures()
{
	if( !gfx_converter.sel_asset && gfx_converter.sel_asset_name.size()>0 )
	{
		gfx_converter.sel_asset = gfx_converter.FindAsset<AssetBase>(gfx_converter.sel_asset_name.c_str(), true);

		if( gfx_converter.sel_asset )
		{
			GfxAssetGroup *group = gfx_converter.sel_asset->asset_group;
			while( group )
			{
				gfx_converter.open_asset_groups[group->full_name] = true;
				group = group->parent;
			}
		}
	}

	AssetBase *asset = gfx_converter.sel_asset;
	GfxAnimation *anim = dynamic_cast<GfxAnimation*>(asset);
	int add_off_x = 0;
	int add_off_y = 0;
	if( anim && anim->frames.size()>0 )
	{
		gfx_converter.sel_anim_delay += Dev.GetTimeDelta();

		while( 1 )
		{
			if( gfx_converter.sel_anim_frame >= anim->frames.size() )
			{
				// start from first frame
				gfx_converter.sel_anim_frame = 0;
				gfx_converter.sel_anim_delay = 0;
			}
			else
			{
				if( gfx_converter.sel_anim_delay < anim->frames[gfx_converter.sel_anim_frame].delay ||
					anim->frames[gfx_converter.sel_anim_frame].delay <= 0 )
					break;
				gfx_converter.sel_anim_delay -= anim->frames[gfx_converter.sel_anim_frame].delay;
				gfx_converter.sel_anim_frame++;

				if( gfx_converter.sel_anim_frame >= anim->frames.size() )
					gfx_converter.sel_anim_frame = 0;
			}

			if( anim->frames[gfx_converter.sel_anim_frame].play_sound.size()>0 )
			{
				SoundAsset *sound = gfx_converter.FindAsset<SoundAsset>(anim->frames[gfx_converter.sel_anim_frame].play_sound.c_str(), true);
				if( sound )
					sound->OnClick();
			}
		}

		asset = anim->frames[gfx_converter.sel_anim_frame].asset;
		add_off_x = anim->frames[gfx_converter.sel_anim_frame].add_off_x;
		add_off_y = anim->frames[gfx_converter.sel_anim_frame].add_off_y;
	}

	if( !asset )
		return;

	if( asset->DrawPreview() )
		return;


	GfxAsset *gfx = dynamic_cast<GfxAsset*>(asset);
	if(!gfx) return;

	asset_usage_t usage = asset->asset_group->opt_usage;


	int zoom = 2;
	int screen_w = 160;
	int screen_h = 100;
	int origin_x = 0;
	int origin_y = 0;
	bool crosshair = false;
	int step_width = 0;
	int y2x = preview_yx2 ? 2 : 1;
	switch( asset->asset_group->opt_usage )
	{
	case ASSET_USAGE_TEXTURE:			zoom = 2;	screen_w = 160;		screen_h = 128/y2x;																									break;
	case ASSET_USAGE_FLAT:				zoom = 2;	screen_w = 160;		screen_h = 128;																										break;
	case ASSET_USAGE_OBJECT:			zoom = 2;	screen_w = 160;		screen_h = 128/y2x;	origin_x = screen_w/2;	origin_y = screen_h*2/3;	crosshair = true;	step_width = 64;		break;
	case ASSET_USAGE_WEAPON:			zoom = 2;	screen_w = 160;		screen_h = 100;																										break;
	case ASSET_USAGE_WEAPON_SPRITE:		zoom = 1;	screen_w = 320;		screen_h = 200;																										break;
	case ASSET_USAGE_GRAPHICS:			zoom = 1;	screen_w = 320;		screen_h = 256;																										break;
	case ASSET_USAGE_SKY:				zoom = 2;	screen_w = 160;		screen_h = 128;																										break;
	}
	if( preview_zoom )
		zoom = preview_zoom;

	screen.Resize(screen_w, screen_h);
	screen.Clear(0xFFFF);

	if( crosshair )
	{
		for( int i=0; i<100; i++ )
			screen(origin_x, i) = 0x0000;
		for( int i=0; i<160; i++ )
			screen(i, origin_y) = 0x0000;
	}


	// show assets
	for( int i=0; i<gfx->textures.size(); i++ )
	{
		GfxTexture *mtex = gfx->textures[i];
		DataTexture &tex = mtex->texture;
		int xpos = origin_x - tex.off_x + add_off_x;
		int ypos = origin_y - tex.off_y + add_off_y;

		for( int y=0; y<tex.height; y++ )
			for( int x=0; x<tex.width; x++ )
			{
				if( DWORD(xpos+x) < DWORD(screen.width) && DWORD(ypos+y) < DWORD(screen.height) )
				{
					DWORD col = tex.data[x+y*tex.width];
					//if( col!=0x000F )	// draw preview if not transparent
						screen.data[(xpos+x)+(ypos+y)*screen.width] = col;
				}
			}

		add_off_x += (step_width ? step_width : tex.width);
	}

	// show overlay
	static float anim_pos = 1.f;
	if( Dev.GetKeyState(VK_LBUTTON) )
	{
		vec2 ssize = Dev.GetScreenSizeV();
		vec2 mpos = Dev.GetMousePosV();
		ssize.x -= MENU_WIDTH;
		if( mpos.x>=0 && mpos.x<ssize.x )
			anim_pos = mpos.x / ssize.x;
	}
	for( int i=0; i<gfx->overlays.size(); i++ )
	{

		GfxAsset::Overlay &ov = gfx->overlays[i];
		if( ov.mode == GfxAsset::Overlay::M_ICON && ov.gfx.get()!=NULL && ov.gfx->textures.size()>0 )
			screen.IndexOverlay(ov.gfx->textures[0]->texture, ov.xoffs, ov.yoffs);

		if( ov.mode == GfxAsset::Overlay::M_PART && ov.gfx.get()!=NULL && ov.gfx->textures.size()>0 )
		{
			DataTexture &tex = ov.gfx->textures[0]->texture;
			int xpos = origin_x - tex.off_x + add_off_x + ov.xoffs;
			int ypos = origin_y - tex.off_y + add_off_y + ov.yoffs;
			screen.IndexOverlay(ov.gfx->textures[0]->texture, xpos, ypos);
		}

		if( ov.mode==GfxAsset::Overlay::M_FRAME && ov.font.get()!=NULL && ov.font->assets.size()>0 )
		{
			int index = 0;
			if( gfx->asset_group->opt_masked )
				index = int(anim_pos * (ov.font->assets.size()+1));
			else
				index = int(anim_pos * ov.font->assets.size()) + 1;

			if( index>0 )
			{
				index--;
				if( index >= ov.font->assets.size() )
					index = ov.font->assets.size()-1;

				GfxAsset *digit = dynamic_cast<GfxAsset*>(ov.font->assets[index]);
				if( digit && digit->textures.size()>0 )
				{
					for( int d=0; d<ov.num_digits; d++ )
						screen.IndexOverlay(digit->textures[0]->texture, ov.xoffs + ov.font_step*d, ov.yoffs);
				}
			}
		}

		if( ov.mode==GfxAsset::Overlay::M_NUMBER && ov.font.get()!=NULL && ov.font->assets.size()>=11 )
		{
			int value = int(anim_pos * (ov.num_max+1));
			if( value > ov.num_max )
				value = ov.num_max;

			for( int d=ov.num_digits-1; d>=0; d-- )
			{
				int di = value%10 + 1;
				if( value==0 && d<ov.num_digits-1 )
					di = 0;
				value /= 10;
				GfxAsset *digit = dynamic_cast<GfxAsset*>(ov.font->assets[di]);

				if( digit && digit->textures.size()>0 )
					screen.IndexOverlay(digit->textures[0]->texture, ov.xoffs + ov.font_step*d, ov.yoffs);
			}
		}
	}

	screen.Upload();
	
	main_rt.SetParameters(D3DFMT_A8R8G8B8, screen.width*2, screen.height*2, 0);

	// Resolve
	PreparePalette(*asset->asset_group->opt_palette);

	Dev.SetRenderTarget(0, &main_rt);
	Dev->SetTexture(0, screen.tex);
	Dev->SetTexture(1, tex_palette.tex);
	Dev->SetTexture(2, prev_screen.tex);
	fx_render.SetFloat("pal_size", tex_palette.width);
	fx_render.SetFloat2("ssize", main_rt.GetCurrentSizeV()*0.5f);
	fx_render.SetFloat3("bg_color", vec3(0.4f, 0.4f, 0.4f));
	if( prev_shadow_mode )
	{
		if( prev_timer_show>0 ) prev_timer_show += Dev.GetTimeDelta()*4.f;
		if( prev_timer_fade>0 ) prev_timer_fade += Dev.GetTimeDelta()*4.f;
		if( prev_timer_show>2.f ) prev_timer_show = 2.f;
		if( prev_timer_fade>1 ) prev_timer_fade = 1;

		float pts = max(0,prev_timer_show-1.f);
		float xo = floor(pts*48.f+0.5f);
		float alpha = (1.f-prev_timer_fade);
		fx_render.SetFloat4("shadow_params", vec4(xo, 0, 0, alpha));
	}
	else
		fx_render.SetFloat4("shadow_params", vec4(0, 0, 0, 0));

	fx_render.StartTechnique(format("resolve%d", dither_mode).c_str());
	while( fx_render.StartPass() )
		Dev.DrawScreenQuad(0, 0, screen.width*2, screen.height*2);

	// Flush
	if( full_screen_mode )
	{
		int sx=0, sy=0;
		Dev.GetScreenSize(sx, sy);
		zoom = min(sx/(screen.width*2), sy/(screen.height*2));

		Dev.SetRenderTarget(0, NULL);
		Dev->SetTexture(0, main_rt);
		Dev.SetRState(RSF_NO_ZWRITE);
		Dev.DrawScreenQuad(0, 0, screen.width*2*zoom, screen.height*2*zoom);
	}
	else
	{
		vec2 ss = Dev.GetScreenSizeV();
		int w = screen.width*2;
		int h = screen.height*2;
		int scale = (int)min((ss.x-MENU_WIDTH)/w, ss.y/h);

		if( preview_zoom )
			scale = preview_zoom;

		int yscale = (preview_yx2 ? scale*2 : scale);

		int x0 = (ss.x-MENU_WIDTH - w*scale)/2;
		int y0 = (ss.y - 24 - h*yscale)/2;
		if( y0<0 ) y0 = 0;

		Dev.SetRenderTarget(0, NULL);
		Dev->SetTexture(0, main_rt);
		Dev.SetRState(RSF_NO_ZWRITE);
		Dev.DrawScreenQuad(x0, y0, x0+w*scale, y0+h*yscale);
	}

	// Draw palette
	const int ps = 24;
	Dev->SetTexture(0, tex_palette.tex);
	Dev.DrawScreenQuad(0, Dev.GetScreenSizeV().y-tex_palette.height*ps, tex_palette.width*ps, Dev.GetScreenSizeV().y);
}

void RenderTestApp::MV_DrawLines(int flags, DWORD color)
{
	vec2 vmin(0, 0), vmax(0, 0);
	for( int i=0; i<(int)mapgen.lines.size(); i++ )
	{
		DreadMapGen::Line *line = mapgen.lines[i];

		DreadMapGen::Vertex &v1 = *line->v1;
		DreadMapGen::Vertex &v2 = *line->v2;
		vec2 p1(v1.xp, v1.yp);
		vec2 p2(v2.xp, v2.yp);

		p1.update_min_max(vmin, vmax);
		p2.update_min_max(vmin, vmax);

		if( (flags & MVDL_SELECTED_ONLY) && line!=mv_sel_line )
			continue;

		if( !(flags & (line->other_sector ? MVDL_TWO_SIDED : MVDL_ONE_SIDED)) )
			continue;

		if( (flags & MVDL_DEBUG_MARK_ONLY) && !line->_debug_mark )
			continue;

		if( flags & MVDL_VISIBLE_ONLY )
		{ 
			if( !mv_vis_region ) continue;

			if( mv_vis_region->vis.find(line)==mv_vis_region->vis.end() )
				continue;
		}

		if( flags & MVDL_HIDE_NOT_REQUIRED )
			if( !line->IsRequired() )
				continue;

		mv.DrawLine(p1, p2, color, true);
	}

	if( flags & MVDL_CENTER_ZOOM )
		mv.CenterRegion(vmin, vmax);
}

void RenderTestApp::MV_DrawPolygon(Polygon2D &p, int flags, DWORD color)
{
	if( (flags & MVDR_NULL_USER_ONLY) && p._user )
		return;

	mv.DrawPolygon(p, color, (flags & MVDR_DRAW_VERTEXES) != 0, (flags & MVDR_DRAW_LINE_MARKERS) != 0);
}

void RenderTestApp::MV_DrawMesh(PolygonMesh2D &mesh, int flags, DWORD color)
{
	for( int i=0; i<(int)mesh.polys.size(); i++ )
		MV_DrawPolygon(mesh.polys[i], flags, color);
}


void RenderTestApp::MV_DrawRegion(DreadMapGen::RegionPoly &p, int flags, DWORD color)
{
	if( (flags & MVDR_MARKED_ONLY) && !p.marked )
		return;

	if( (flags & MVDR_SELECTED_ONLY) && &p!=mv_sel_region )
		return;

	if( (flags & MVDR_VISREGION_ONLY) && &p!=mv_vis_region )
		return;

	MV_DrawPolygon(p.poly, flags, color);
}

void RenderTestApp::MV_DrawRegions(int flags, DWORD color)
{
	for( auto pp=mapgen.regions.poly_set.begin(); pp!=mapgen.regions.poly_set.end(); ++pp )
		MV_DrawRegion(**pp, flags, color);
}

void RenderTestApp::MV_DrawVisibleRegions(int flags, DWORD color)
{
	if( !mv_vis_region )
		return;

	for( auto pp=mv_vis_region->svis.begin(); pp!=mv_vis_region->svis.end(); ++pp )
	{
		DreadMapGen::Sector *sec = pp->second->sector;
		for( int j=0; j<sec->regions.size(); j++ )
			MV_DrawRegion(*sec->regions[j], flags, color);
	}
}

void RenderTestApp::MV_DrawColEdges(vector<DreadMapGen::ColEdge> &edges, int flags, DWORD color)
{
	for( int i=0; i<(int)edges.size(); i++ )
	{
		DreadMapGen::ColEdge &e = edges[i];

		vec2 _v1 = e.p1.GetVec2();
		vec2 _v2 = e.p2.GetVec2();
		vec2 p1(_v1.x, _v1.y);
		vec2 p2(_v2.x, _v2.y);

		mv.DrawEdge(p1, p2, color);
	}
}


void RenderTestApp::RunMapView()
{
	static bool show_all = true;
	mv.Run(DreadMapGen::REGION_SIZE);

	if( mv.mouse_lmb )
	{
		if( mapview_mode==MAPVIEW_THING_VIS || mapview_mode==MAPVIEW_LINE_VIS2 )
		{
			mv_sel_region = mapgen.FindCameraRegion(mv.world_pos, Dev.GetKeyState(VK_SHIFT));
		}
		else
		{
			float ls = 0.5f/mv.mv_zoom;	// line size
			mv_sel_line = mapgen.FindClosestLine(mv.world_pos, ls*32);
			if( mv_sel_line && Dev.GetKeyState(VK_SHIFT) )
			{
				mv_vis_region = NULL;
				mapgen.ComputeVisibility(mv_sel_line, true);
			}
		}
	}

	if( mv.mouse_rmb )
	{
		mv_eyepos = mv.world_pos;
		mapgen.AssureVisibility();
		mv_vis_region = mapgen.FindCameraRegion(mv.world_pos, Dev.GetKeyState(VK_SHIFT));
	}

	if( Dev.GetKeyStroke('A') )			show_all = true;


	// Draw lines - underlay
	switch( mapview_mode )
	{
	case MAPVIEW_LINE_VIS:
		MV_DrawRegions(0, 0x008080);															// dark cyan		- regions
		// !!! Fall through !!!

	case MAPVIEW_THING_VIS:
	//case MAPVIEW_REGION_VIS:
	case MAPVIEW_COL_MESH:
	case MAPVIEW_COL_EDGES:
	case MAPVIEW_COL_BSP:
		MV_DrawLines(MVDL_ONE_SIDED, 0xC0C0C0);													// light gray		- 1-sided lines			(MAP CORE)
		MV_DrawLines(MVDL_TWO_SIDED, 0x606060);													// dark gray		- 2-sided lines			(MAP CORE)
		break;

	case MAPVIEW_LINE_VIS2:
		MV_DrawRegions(0, 0x008080);															// dark cyan		- regions
		MV_DrawLines(MVDL_ONE_SIDED | MVDL_HIDE_NOT_REQUIRED, 0xC0C0C0);						// light gray		- 1-sided lines			(MAP CORE)
		MV_DrawLines(MVDL_TWO_SIDED | MVDL_HIDE_NOT_REQUIRED, 0x606060);						// dark gray		- 2-sided lines			(MAP CORE)
		break;
	}

	// Draw lines - overlay
	switch( mapview_mode )
	{
	case MAPVIEW_LINE_VIS:
		MV_DrawLines(MVDL_TWO_SIDED | MVDL_VISIBLE_ONLY, 0xC00000);								// red				- visible 2-sided lines
		MV_DrawLines(MVDL_ONE_SIDED | MVDL_VISIBLE_ONLY, 0xFF0000);								// light red		- visible 1-sided lines
		break;

	case MAPVIEW_LINE_VIS2:
		MV_DrawLines(MVDL_TWO_SIDED | MVDL_VISIBLE_ONLY, 0xC00000);								// red				- visible 2-sided lines
		MV_DrawLines(MVDL_ONE_SIDED | MVDL_VISIBLE_ONLY, 0xFF0000);								// light red		- visible 1-sided lines
		MV_DrawRegions(MVDR_SELECTED_ONLY, 0xFFFF00);											// yellow			- selected region
		break;

	case MAPVIEW_THING_VIS:
		MV_DrawVisibleRegions(MVDR_DRAW_VERTEXES | MVDR_DRAW_LINE_MARKERS, 0x00FF00);			// light green		- thing vis regions
		MV_DrawRegions(MVDR_SELECTED_ONLY, 0xFFFF00);											// yellow			- selected region
		break;
	//case MAPVIEW_REGION_VIS:
	//	MV_DrawMesh(mapgen.work_mesh, MVDR_DRAW_LINE_MARKERS, 0xFF8000);						// orange			- work mesh
	//	MV_DrawLines(MVDL_TWO_SIDED | MVDL_DEBUG_MARK_ONLY, 0x00C000);							// green			- debug mark 2-sided lines
	//	MV_DrawLines(MVDL_ONE_SIDED | MVDL_DEBUG_MARK_ONLY, 0x00FF00);							// light green		- debug mark 1-sided lines
	//	break;
	case MAPVIEW_COL_MESH:
		MV_DrawMesh(mapgen.collision_mesh, 0, 0x004000);										// dark green		- collision mesh
		MV_DrawMesh(mapgen.collision_mesh, MVDR_NULL_USER_ONLY, 0x80FF80);						// lime				- collision mesh		(NULL user)
		break;
	case MAPVIEW_COL_EDGES:
		MV_DrawColEdges(mapgen.col_edges, 0, 0x80FF80);
		break;
	case MAPVIEW_COL_BSP:
		MV_DrawMesh(mapgen.collision_bsp_mesh, 0, 0x004000);									// dark green		- collision mesh
		MV_DrawMesh(mapgen.collision_bsp_mesh, MVDR_NULL_USER_ONLY, 0x80FF80);					// lime				- collision mesh		(NULL user)
		break;
	}

	if( mapview_mode!=MAPVIEW_THING_VIS )
	{
		// Draw lines - line selection
		MV_DrawLines(MVDL_ONE_SIDED | MVDL_SELECTED_ONLY, 0xFFFF00);							// light yellow			- selected 1-sided lines
		MV_DrawLines(MVDL_TWO_SIDED | MVDL_SELECTED_ONLY, 0xC0C000);							// yellow				- selected 2-sided lines
	}
	else
	{
		MV_DrawVisibleRegions(MVDR_SELECTED_ONLY, 0xFFFF00);									// light yellow			- selected region
	}

	if( show_all )
		MV_DrawLines(MVDL_CENTER_ZOOM, 0xC0C000);

	show_all = false;

	//if( mapview_mode == MAPVIEW_COL_EDGES || mapview_mode == MAPVIEW_COL_BSP )
	//{
	//	vec2 p1 = mv_eyepos;
	//	vec2 p2 = mv.world_pos;	//p1 + vec2(100, 100);
	//	vec2 hit, normal(0,0);
	//	bool is_hit = BspClip(mapgen.col_bsp_root_0, p1, p2, hit, normal);
	//	can.Draw("")()(is_hit ? 0x008080 : 0x00FFFF).line(p1*vec2(1, -1), p2*vec2(1, -1), ls);
	//	if( is_hit )
	//	{
	//		can.Draw("")()(0xFF00FF).line(p1*vec2(1, -1), hit*vec2(1, -1), ls);
	//		can.Draw("")()(0xFFFF00).line(hit*vec2(1, -1), (hit+normal*16)*vec2(1, -1), ls);
	//	}
	//}

	// error markers
	for( int i=0; i<elog.log.size(); i++ )
	{
		ErrorLog::LogEntry *e = elog.log[i];
		if( e->object==&mapgen && e->has_coord )
			if( mv.DrawWarning(vec2(e->coord.x, e->coord.y), e->color) )
			{
				for( ErrorLog::MapDebugLine &d : e->map_debug )
					mv.DrawLine(d.v1, d.v2, d.color, true);
			}
	}

	// eye
	mv.DrawEye(mv_eyepos, 0x0000FF);


	if( (mapview_mode == MAPVIEW_COL_EDGES || mapview_mode == MAPVIEW_COL_BSP) && dread_sim.e_map_bspnodes.size()>0 )
	{
		vec2 p1 = mv_eyepos;
		vec2 p2 = mv.world_pos;	//p1 + vec2(100, 100);

#if 1
		short startx = (short)_float_to_dread(p1.x);
		short starty = (short)_float_to_dread(p1.y);
		short endx = _float_to_dread(p2.x);
		short endy = _float_to_dread(p2.y);
		p1.x = _dread_to_float(startx);
		p1.y = _dread_to_float(starty);
		p2.x = _dread_to_float(endx);
		p2.y = _dread_to_float(endy);
		
		if( mapview_test == MAPTEST_CLIP_HITSCAN )
		{
			// Hitscan
			dread_sim.bspclip_p1x = startx;
			dread_sim.bspclip_p1y = starty;
			dread_sim.bspclip_p2x = endx;
			dread_sim.bspclip_p2y = endy;
			dread_sim.array_error = false;

			short is_hit = dread_sim.BspClipFix(dread_sim.e_map_header->map_hitscan_bsp_root);

			DWORD color = is_hit ? 0x008080 : 0x00FFFF;
			if( dread_sim.array_error ) color = 0xFF0000;
			mv.DrawLine(p1, p2, color);
			if( is_hit )
			{
				vec2 hit;
				hit.x = _dread_to_float(dread_sim.bspclip_p1x);
				hit.y = _dread_to_float(dread_sim.bspclip_p1y);
				vec2 normal = vec2(dread_sim.bspclip_nA, dread_sim.bspclip_nB).get_normalized();
				mv.DrawLine(p1, hit, 0xFF00FF);
				mv.DrawLine(hit, hit+normal*16, 0xFFFF00);
			}
		}
		else if( mapview_test == MAPTEST_CLIP_WALK )
		{
			// Walk
			short is_hit = dread_sim.Physics_WalkSlide(startx, starty, endx, endy);

			DWORD color = is_hit ? 0x008080 : 0x00FFFF;
			if( dread_sim.array_error ) color = 0xFF0000;
			mv.DrawLine(p1, p2, color);

			if( is_hit )
			{
				vec2 mid, hit;
				mid.x = _dread_to_float(dread_sim.midpoint_endx);
				mid.y = _dread_to_float(dread_sim.midpoint_endy);
				hit.x = _dread_to_float(dread_sim.physics_endx);
				hit.y = _dread_to_float(dread_sim.physics_endy);
				mv.DrawLine(p1, mid, 0xFFFF00);
				mv.DrawLine(mid, hit, 0xFF00FF);
			}

			if( Dev.GetKeyStroke(VK_SPACE) && (is_hit || Dev.GetKeyState(VK_SHIFT)) )
			{
				mv_eyepos.x = _dread_to_float(dread_sim.physics_endx);
				mv_eyepos.y = _dread_to_float(dread_sim.physics_endy);
			}
		}
		else if( mapview_test == MAPTEST_CLIP_NEW )
		{
			// Hitscan
			dread_sim.bspclip_p1x = startx;
			dread_sim.bspclip_p1y = starty;
			dread_sim.bspclip_p2x = endx;
			dread_sim.bspclip_p2y = endy;
			dread_sim.array_error = false;

			short is_hit = dread_sim.New_BspClipFix(dread_sim.e_map_header->map_collision_bsp_root);

			DWORD color = is_hit ? 0x008080 : 0x00FFFF;
			if( dread_sim.array_error ) color = 0xFF0000;
			mv.DrawLine(p1, p2, color);
			if( is_hit )
			{
				vec2 hit;
				hit.x = _dread_to_float(dread_sim.bspclip_p1x);
				hit.y = _dread_to_float(dread_sim.bspclip_p1y);
				vec2 normal = vec2(dread_sim.bspclip_nA, dread_sim.bspclip_nB).get_normalized();
				mv.DrawLine(p1, hit, 0xFF00FF);
				mv.DrawLine(hit, hit+normal*16, 0xFFFF00);
			}
		}

		// Draw collision probe
		dread_sim.bspclip_p1x = endx;
		dread_sim.bspclip_p1y = endy;
		if( dread_sim.BspClipCheckFix(dread_sim.e_map_header->map_collision_bsp_root) )
			mv.DrawVertex(p2, 0xFF0000);
		else
			mv.DrawVertex(p2, 0x00FF00);

#else
		BspClipArgs clip;
		clip.p1x = int(floor(p1.x*(1<<DreadMapGen::ENGINE_SUBVERTEX_PRECISION)+0.5f));
		clip.p1y = int(floor(p1.y*(1<<DreadMapGen::ENGINE_SUBVERTEX_PRECISION)+0.5f));
		clip.p2x = int(floor(p2.x*(1<<DreadMapGen::ENGINE_SUBVERTEX_PRECISION)+0.5f));
		clip.p2y = int(floor(p2.y*(1<<DreadMapGen::ENGINE_SUBVERTEX_PRECISION)+0.5f));

		bool is_hit = BspClipFix(mapgen.col_bsp_root_0, clip);
		can.Draw("")()(is_hit ? 0x008080 : 0x00FFFF).line(p1*vec2(1, -1), p2*vec2(1, -1), ls);
		if( is_hit )
		{
			vec2 hit;
			hit.x = clip.p1x/float(1<<DreadMapGen::ENGINE_SUBVERTEX_PRECISION);
			hit.y = clip.p1y/float(1<<DreadMapGen::ENGINE_SUBVERTEX_PRECISION);
			vec2 normal = vec2(clip.nA, clip.nB).get_normalized();
			can.Draw("")()(0xFF00FF).line(p1*vec2(1, -1), hit*vec2(1, -1), ls);
			can.Draw("")()(0xFFFF00).line(hit*vec2(1, -1), (hit+normal*16)*vec2(1, -1), ls);
		}
#endif
	}

	mv.Flush();
}

void RenderTestApp::CompileScript(bool gc_actors)
{
	printf("================ Compiling ================\n");
	//script_compiler.Clear();
	//string code;
	//if( NFS.GetFileText("code/mon_guards.code", code) )
	//	script_compiler.Compile(code.c_str());

	script_compiler.Clear();

//	for( int i=0;; i++ )
//	{
//		string path = proj_cfg.GetString(format("script-file-%d", i).c_str(), "");
//		if( path.size()<=0 )
//			break;
//		script_compiler.CompileFile(path.c_str());
//	}

	for( const string &s : gfx_converter.script_files )
		script_compiler.CompileFile(s.c_str());

	if( gc_actors )
	{
		script_compiler.DependencyReset();
		for( int i=0; i<(int)mapgen.things.size(); i++ )
			script_compiler.DependencyUseThing(mapgen.things[i].type);
	}


	string export_path = proj_cfg.GetString("export-directory", "export") + "/export_scripts.i";
	script_compiler.ExportScript(export_path.c_str());
}


void RenderTestApp::PrepareMap()
{
	elog.RemoveMessagesOf(&mapgen);
	converted_map = NULL;
	err_message = "";

	try {
		string path = modmgr.GetMapPath();
		map_filetime = GetFileTime(path.c_str());
		if( !map_wad.Load(path.c_str(), false) )
			throw string("Can't load WAD");

		if( map_wad.HasLump("DREADMAP") )
			map_wad.GetMap("DREADMAP", &wad_mapinfo);
		else
			map_wad.GetMap("MAP01", &wad_mapinfo);

		FreeMapMeshes();
		converted_map = wad_converter.Convert(&wad_mapinfo);

		mv_sel_line = NULL;
		mv_sel_region = NULL;
		mv_vis_region = NULL;
		mapgen.export_engine_version = proj_cfg.GetInt("export-engine-version", 1);
		printf("Engine version: %d\n", mapgen.export_engine_version);
		mapgen.LoadMap(converted_map, &wad_converter);

		renderer.LoadMap(mapgen);
	}
	catch( const string &e )
	{
		err_message = e;
	}

	BuildMapMeshes();
}

void RenderTestApp::WatchMap()
{
	string path = modmgr.GetMapPath();
	unsigned long long time = GetFileTime(path.c_str());
	if( time>map_filetime )
		PrepareMap();
}

void RenderTestApp::WatchAssets(bool force_reload)
{
	if( gfx_converter.WatchAssetFiles(force_reload) )
	{
		prev_screen.CopyFrom(screen);
		prev_timer_show = prev_shadow_mode ? 0.0001f : 0;
		prev_timer_fade = 0;

		CompileScript(script_gc_mode);
		//PrepareMap();
		BuildMapMeshes();
	}

	if( prev_timer_fade<=0 && GetAsyncKeyState(VK_ESCAPE)<0 )
		prev_timer_fade = 0.0001f;
}

void RenderTestApp::FreeMapMeshes()
{
	for( auto p=map_meshes.begin(); p!=map_meshes.end(); ++p )
		delete p->second;
	map_meshes.clear();
}

RenderTestApp::MapMesh *RenderTestApp::AssureMapMesh(const texture_id_t &key)
{
	MapMesh *mesh = map_meshes[key];
	if( !mesh )
	{
		mesh = new MapMesh();
		map_meshes[key] = mesh;
	}
	return mesh;
}

void RenderTestApp::BuildMapMeshes()
{
	BuildMapMeshes2();
	return;

	// Clear
	FreeMapMeshes();

	if( !converted_map )
		return;

	// Build
	vector<int> sec_vert;
	for( int i=0; i<converted_map->n_sectors; i++ )
	{
		MapFileSector &sec = converted_map->sectors[i];
		sec_vert.clear();

		for( int j=0; j<sec.num_lines; j++ )
		{
			MapFileLine &line = converted_map->lines[sec.first_line + j];
			int v2 = converted_map->lines[sec.first_line + (j+1)%sec.num_lines].vertex;
			int up_y1 = 0;
			int up_y2 = 0;
			int low_y1 = 0;
			int low_y2 = 0;


			sec_vert.push_back(line.vertex);

			if( line.other_sector!=0xFFFF )
			{
				MapFileSector &sec2 = converted_map->sectors[line.other_sector];
				up_y1 = sec.ceil_h;
				up_y2 = sec2.ceil_h;
				low_y1 = sec2.floor_h;
				low_y2 = sec.floor_h;
			}
			else
			{
				up_y1 = sec.ceil_h;
				up_y2 = sec.floor_h;
			}

			for( int ul=0; ul<2; ul++ )
			{
				int y1 = ul ? low_y1 : up_y1;
				int y2 = ul ? low_y2 : up_y2;
				if( y1<=y2 ) continue;

				texture_id_t mesh_key = gfx_converter.GetTextureId(ul ? line.lower_tex.c_str() : line.upper_tex.c_str(), line.light_level);
				MapMesh *mesh = AssureMapMesh(mesh_key);
				GfxTexture *tex = gfx_converter.FindTexture(mesh_key);
				if( !tex ) continue;

#if 1
				int tex_offs_y = line.ycoord;
				int _yoffs_u = 0;
				int h1 = -up_y1;
				int h2 = -up_y2;
				{
					bool tex_unpegged_upper = ((line.line_flags & WadFile::LineDef::UPPER_UNPEGGED) != 0);
					bool tex_unpegged_lower = ((line.line_flags & WadFile::LineDef::LOWER_UNPEGGED) != 0);
					if( line.other_sector!=0xFFFF )
					{
						if( tex_unpegged_upper )	_yoffs_u = h1 - tex_offs_y;
						else						_yoffs_u = h2 - tex_offs_y;
					}
					else
					{
						if( tex_unpegged_lower )	_yoffs_u = h2 - tex_offs_y;
						else						_yoffs_u = h1 - tex_offs_y;
					}
					int uh = (tex ? tex->orig_h : 128);
					_yoffs_u = adjust_y_offset(_yoffs_u, uh, GfxConverter::FUDGE_BORDER, h1, h2);
				}
				_yoffs_u -= h1;
#endif

				//int vbase = mesh->vb.size();
				//for( int v=0; v<4; v++ )
				//{
				//	MapFileVertex &fv = converted_map->vertexes[v==0 || v==3 ? line.vertex : v2];
				//	MeshVertex mv;
				//	mv.pos.x = fv.xp;
				//	mv.pos.y = -fv.yp;
				//	mv.pos.z = (v==0 || v==1 ? y1 : y2);
				//	mv.uv.x = (v==0 || v==3 ? line.xcoord1 : line.xcoord2)/float(tex->orig_w);
				//	mv.uv.y = (-_yoffs_u + (v==0 || v==1 ? 0 : y1-y2))/float(tex->orig_h);
				//	mesh->vb.push_back(mv);
				//}

				//mesh->ib.push_back(vbase+0);
				//mesh->ib.push_back(vbase+1);
				//mesh->ib.push_back(vbase+2);
				//mesh->ib.push_back(vbase+0);
				//mesh->ib.push_back(vbase+2);
				//mesh->ib.push_back(vbase+3);
				
				MapFileVertex &vx1 = converted_map->vertexes[line.vertex];
				MapFileVertex &vx2 = converted_map->vertexes[v2];
				mesh->AddWallQuad(
					vx1.xp, -vx1.yp,
					vx2.xp, -vx2.yp,
					y1, y2,
					line.xcoord1/float(tex->orig_w),
					 (-_yoffs_u)/float(tex->orig_h),
					line.xcoord2/float(tex->orig_w),
					(-_yoffs_u + (y1-y2))/float(tex->orig_h)
				);
			}
		}

		// Flats
		for( int fc=0; fc<2; fc++ )
		{
			texture_id_t mesh_key = gfx_converter.GetTextureId(fc ? sec.ceil_tex.c_str() : sec.floor_tex.c_str(), sec.light_level);
			MapMesh *mesh = AssureMapMesh(mesh_key);
			int v0=-1, pv=-1;
			
			GfxTexture *tex = gfx_converter.FindFlatTexture(mesh_key);
			if( !tex ) continue;

			for( int v=0; v<sec_vert.size(); v++ )
			{
				MapFileVertex &fv = converted_map->vertexes[sec_vert[v]];
				MeshVertex mv;
				mv.pos.x = fv.xp;
				mv.pos.y = -fv.yp;
				mv.pos.z = (fc ? sec.ceil_h : sec.floor_h);
				mv.uv.x = fv.xp/float(tex->texture.width);
				mv.uv.y = fv.yp/float(tex->texture.height);
				mesh->vb.push_back(mv);

				int nv = mesh->vb.size()-1;
				if( v0<0 )
					v0 = nv;
				else
				{
					if( pv>=0 )
					{
						mesh->ib.push_back(v0);
						mesh->ib.push_back(fc ? nv : pv);
						mesh->ib.push_back(fc ? pv : nv);
					}
					pv = nv;
				}
			}
		}
	}

	// Upload
	for( auto p=map_meshes.begin(); p!=map_meshes.end(); ++p )
	{
		MapMesh &mesh = *p->second;
		if( mesh.vb.size()>0 && mesh.ib.size()>0 )
			mesh.mesh.LoadVBIB(&mesh.vb[0], mesh.vb.size(), sizeof(MeshVertex), MeshVertex::FVF, &mesh.ib[0], mesh.ib.size(), false);
	}
}

void RenderTestApp::BuildMapMeshes2()
{
	// Clear
	FreeMapMeshes();

	if( !converted_map )
		return;

	// Build
	for( DreadMapGen::Line *line : mapgen.lines )
	{
		if( !line->IsDrawn() )
			continue;

		// upper / lower
		for( int ul=0; ul<2; ul++ )
		{
			int y1 = ul ? line->h3 : line->h1;
			int y2 = ul ? line->h4 : line->h2;
			if( y1>=y2 ) continue;

			texture_id_t mesh_key = ul ? line->tex_lower : line->tex_upper;
			MapMesh *mesh = AssureMapMesh(mesh_key);
			GfxTexture *tex = gfx_converter.FindTexture(mesh_key);
			if( !tex ) continue;

			int yoffs = ul ? line->_yoffs_l : line->_yoffs_u;
			int dx = line->v2->xp - line->v1->xp;
			int dy = line->v2->yp - line->v1->yp;
			int len = (int)sqrt(dx*dx+dy*dy);

			mesh->AddWallQuad(
				line->v1->xp, -line->v1->yp,
				line->v2->xp, -line->v2->yp,
				-y1, -y2,
				line->tex_offs_x/float(tex->orig_w),
				(y1-yoffs)/float(tex->orig_h),
				(line->tex_offs_x+len)/float(tex->orig_w),
				(y2-yoffs)/float(tex->orig_h)
			);
		}

		// floor / ceiling
		for( int cf=0; cf<2; cf++ )
		{
			int y1 = cf ? -16000 : line->h4;
			int y2 = cf ? line->h1 : 16000;
			if( y1>=y2 ) continue;

			texture_id_t mesh_key = cf ? line->tex_ceil : line->tex_floor;
			MapMesh *mesh = AssureMapMesh(mesh_key);
			GfxTexture *tex = gfx_converter.FindFlatTexture(mesh_key);
			//if( !tex ) continue;

			mesh->AddWallQuad(
				line->v1->xp, -line->v1->yp,
				line->v2->xp, -line->v2->yp,
				-y1, -y2,
				0, 0, 0, 0
			);
		}
	}

	// Upload
	for( auto p=map_meshes.begin(); p!=map_meshes.end(); ++p )
	{
		MapMesh &mesh = *p->second;
		if( mesh.vb.size()>0 && mesh.ib.size()>0 )
			mesh.mesh.LoadVBIB(&mesh.vb[0], mesh.vb.size(), sizeof(MeshVertex), MeshVertex::FVF, &mesh.ib[0], mesh.ib.size(), false);
	}
}

//void RenderTestApp::BuildMapMeshes2()
//{
//	// Clear
//	FreeMapMeshes();
//
//	if( !converted_map )
//		return;
//
//	// Build
//	for( DreadMapGen::Line *line : mapgen.lines )
//	{
//		for( int ul=0; ul<2; ul++ )
//		{
//			int y1 = ul ? low_y1 : up_y1;
//			int y2 = ul ? low_y2 : up_y2;
//			if( y1<y2 ) continue;
//
//			texture_id_t mesh_key = gfx_converter.GetTextureId(ul ? line.lower_tex.c_str() : line.upper_tex.c_str(), line.light_level);
//			MapMesh *mesh = AssureMapMesh(mesh_key);
//			GfxTexture *tex = gfx_converter.FindTexture(mesh_key);
//			if( !tex ) continue;
//
//			int vbase = mesh->vb.size();
//			for( int v=0; v<4; v++ )
//			{
//				MapFileVertex &fv = converted_map->vertexes[v==0 || v==3 ? line.vertex : v2];
//				MeshVertex mv;
//				mv.pos.x = fv.xp;
//				mv.pos.y = -fv.yp;
//				mv.pos.z = (v==0 || v==1 ? y1 : y2);
//				mv.uv.x = (v==0 || v==3 ? line.xcoord1 : line.xcoord2)/float(tex->orig_w);
//				mv.uv.y = (v==0 || v==1 ? 0 : y1-y2)/float(tex->orig_h);
//				//#if 1
//				//					int tex_offs_y = line.ycoord;
//				//					bool tex_unpegged_upper = ((line.line_flags & WadFile::LineDef::UPPER_UNPEGGED) != 0);
//				//					bool tex_unpegged_lower = ((line.line_flags & WadFile::LineDef::LOWER_UNPEGGED) != 0);
//				//					int _yoffs_u = 0;
//				//					if( line.other_sector!=0xFFFF )
//				//					{
//				//						if( tex_unpegged_upper )	_yoffs_u = y1 - tex_offs_y;
//				//						else						_yoffs_u = y2 - tex_offs_y;
//				//
//				//					}
//				//					else
//				//					{
//				//						if( tex_unpegged_lower )	_yoffs_u = y2 - tex_offs_y;
//				//						else						_yoffs_u = y1 - tex_offs_y;
//				//					}
//				//					GfxTexture *utex = gfx_converter.FindTexture(tex_upper);
//				//					int uh = (utex ? utex->orig_h : 128);
//				//					_yoffs_u = adjust_y_offset(_yoffs_u, uh, GfxConverter::FUDGE_BORDER, y1, y2);
//				//#endif
//				mesh->vb.push_back(mv);
//			}
//
//			mesh->ib.push_back(vbase+0);
//			mesh->ib.push_back(vbase+1);
//			mesh->ib.push_back(vbase+2);
//			mesh->ib.push_back(vbase+0);
//			mesh->ib.push_back(vbase+2);
//			mesh->ib.push_back(vbase+3);
//		}
//	}
//
//	// Upload
//	for( auto p=map_meshes.begin(); p!=map_meshes.end(); ++p )
//	{
//		MapMesh &mesh = *p->second;
//		if( mesh.vb.size()>0 && mesh.ib.size()>0 )
//			mesh.mesh.LoadVBIB(&mesh.vb[0], mesh.vb.size(), sizeof(MeshVertex), MeshVertex::FVF, &mesh.ib[0], mesh.ib.size(), false);
//	}
//}


void RenderTestApp::PreparePalette(GfxPalette &pal)
{
	// Build palette
	tex_palette.data.clear();
	for( int i=0; i<pal.size(); i++ )
		tex_palette.data.push_back(pal[i].color_out);

	tex_palette.width = tex_palette.data.size();
	tex_palette.height = 1;
	tex_palette.Upload();
}

void RenderTestApp::RenderMapMeshes()
{
	Dev.SetRState(RSF_CULL_CCW);

	fx_render.StartTechnique("tech");
	while( fx_render.StartPass() )
	{
		for( auto p=map_meshes.begin(); p!=map_meshes.end(); ++p )
			if( p->first.is_set() )	// Skip sky
			{
				MapMesh &mesh = *p->second;
				GfxTexture *tex = gfx_converter.FindMapTexture(p->first);
				if( tex )
				{
					Dev->SetTexture(0, tex->texture.tex);
					mesh.mesh.DrawSection(0);
				}
			}
	}

	GfxTexture *sky_tex = gfx_converter.FindTexture("SKY", 255);
	Dev->SetTexture(0, sky_tex ? sky_tex->texture.tex.GetTexture() : NULL);
	int dx = int(floor(cam_ypr.x/360*128*4))&127;

	fx_render.SetFloat2("tex_offs", vec2(dx, 0));
	fx_render.StartTechnique("tech_sky");
	while( fx_render.StartPass() )
	{
		for( auto p=map_meshes.begin(); p!=map_meshes.end(); ++p )
			if( !p->first.is_set() )	// Draw sky
			{
				MapMesh &mesh = *p->second;
				mesh.mesh.DrawSection(0);
			}
	}
}


MapFileSector *RenderTestApp::BspFindSector(int xp, int yp)
{
	if( !converted_map || !converted_map->n_nodes )
		return NULL;

	int node = converted_map->n_nodes-1;
	while( node>=0 )
	{
		MapFileNode &n = converted_map->nodes[node];
		int res = xp*n.A + yp*n.B - n.C;
		node = (res<0 ? n.right : n.left);
	}
	//Dev.DPrintF("Sector %d", node&0x7FFF);

	return converted_map->sectors + (node&0x7FFF);
}

bool RenderTestApp::BspClip(DreadMapGen::ColBSPNode *node, const vec2 &p1, const vec2 &p2, vec2 &hit, vec2 &normal)
{
	if( node->is_leaf )
	{
		if( !node->is_solid )
			return false;
		hit = p1;
		return true;
	}

	float s1 = node->line.CheckSideVec2(p1);
	float s2 = node->line.CheckSideVec2(p2);
	if( s1<=0 && s2<=0 ) return BspClip(node->left, p1, p2, hit, normal);
	if( s1>=0 && s2>=0 ) return BspClip(node->right, p1, p2, hit, normal);
	vec2 mid = p1 + (p2-p1)*(s1/(s1-s2));
	if( s1<=0 && s2>=0 )
	{
		if( BspClip(node->left, p1, mid, hit, normal) )
			return true;
		normal.x = (float)node->line.normal.x;
		normal.y = (float)node->line.normal.y;
		normal.normalize();
		return BspClip(node->right, mid, p2, hit, normal);
	}

	if( BspClip(node->right, p1, mid, hit, normal) )
		return true;
	normal.x = -(float)node->line.normal.x;
	normal.y = -(float)node->line.normal.y;
	normal.normalize();
	return BspClip(node->left, mid, p2, hit, normal);
}

bool RenderTestApp::BspClipFix(DreadMapGen::ColBSPNode *node, BspClipArgs &state)
{
	if( node->is_leaf )
	{
		if( !node->is_solid )
			return false;
		//	hit = p1;	- already the same variable
		return true;
	}

	int s1 = muls(state.p1x, node->A) + muls(state.p1y, node->B) + node->C;
	int s2 = muls(state.p2x, node->A) + muls(state.p2y, node->B) + node->C;
	if( s1<=0 && s2<=0 ) return BspClipFix(node->left, state);
	if( s1>=0 && s2>=0 ) return BspClipFix(node->right, state);

	//vec2 mid = p1 + (p2-p1)*(s1/(s1-s2));
	short tx = state.p2x;
	short ty = state.p2y;
	state.p2x = state.p1x + (state.p2x-state.p1x)*s1/(s1-s2);	// final division is rounded towards zero - midpoint falls closer to p1
	state.p2y = state.p1y + (state.p2y-state.p1y)*s1/(s1-s2);

	//int pf = 0x100*s1/(s1-s2);
	//state.p2x = state.p1x + (((state.p2x-state.p1x)*pf)>>8);
	//state.p2y = state.p1y + (((state.p2y-state.p1y)*pf)>>8);

	if( s1<=0 && s2>=0 )
	{
		if( node->A<0 ) state.p2x++;
		if( node->A>0 ) state.p2x--;
		if( node->B<0 ) state.p2y++;
		if( node->B>0 ) state.p2y--;

		if( BspClipFix(node->left, state) )
			return true;
		state.p1x = state.p2x;
		state.p1y = state.p2y;
		state.p2x = tx;
		state.p2y = ty;
		state.nA = node->A;
		state.nB = node->B;
		state.nC = node->C;
		return BspClipFix(node->right, state);
	}

	if( node->A<0 ) state.p2x--;
	if( node->A>0 ) state.p2x++;
	if( node->B<0 ) state.p2y--;
	if( node->B>0 ) state.p2y++;

	if( BspClipFix(node->right, state) )
		return true;
	////	36/9705    -103: 906->906    311: 2036->2037	!!!
	//printf("%d/%d    %d: %d->%d    %d: %d->%d\n",
	//	s1, s1-s2,
	//	tx-state.p1x, state.p1x, state.p2x,
	//	ty-state.p1y, state.p1y, state.p2y
	//);
	state.p1x = state.p2x;
	state.p1y = state.p2y;
	state.p2x = tx;
	state.p2y = ty;
	state.nA = -node->A;
	state.nB = -node->B;
	state.nC = -node->C;
	return BspClipFix(node->left, state);
}

bool RenderTestApp::BspClipCheckFix(DreadMapGen::ColBSPNode *node, int px, int py)
{
	while( !node->is_leaf )
	{
		int s = muls(px, node->A) + muls(py, node->B) + node->C;
		node = (s<=0) ? node->left : node->right;
	}

	return node->is_solid;
}

void RenderTestApp::SoftGenScaler(vector<int> &out, int size)
{
	// y1 = 50 + ((size*h1)>>8)				-  y1 is topmost (first) pixel which gets texcoord h1
	//
	//	for each Y>=y1	->	h>=h1


	out.clear();

	bool done = true;
	for( int y=0; y<100 || !done; y++ )
	{
		// (size*h)>>8 = y-50;					// given the <y>, choose largest <h> that satisfies the equation

		if( y>=50 )
		{
			int yv = ((y-50)<<8) | 0xFF;
			out.push_back(yv/size);
			if( out.back()>=64 ) done = true;
		}
		else
		{
			//	int yv = y-50;
			//	int hibitsneg = (-yv)-1;			// upper bits of multiplication result	(inverted)
			//	int target = hibitsneg<<8;			// obtain this value or higher	(pick lowest <minus_h>)
			//	
			//	// inverted multiplication result:
			//	//	size*minus_h-1 >= target
			//	//	minus_h >= (target+1)/size				// round up
			//	//	minus_h >= (target+1+size-1)/size
			//	//	minus_h >= target/size+1
			//	out[y] = -(target/size+1);


			out.push_back(-1 - ((49-y)<<8)/size);
		}
	}

	// Verify
	int nerr = 0;
	for( int h=-2048; h<=2048; h++ )
	{
		int y1 = 50 + ((size*h)>>8);		//				-  y1 is topmost (first) pixel which gets texcoord h or greater
		if( y1<0 ) continue;
		if( y1>=out.size() ) continue;

		if( out[y1] < h ) nerr++;
		
		if( y1<=0 ) continue;
		if( out[y1-1] >= h ) nerr++;
	}

	if( nerr>0 )
		printf("Size %4d : %3d errors\n", size, nerr);

	//SoftGenScalerRec(out, size, 0, 100,
	//	(0-50)*256/size-1,
	//	(100-50)*256/size+1);
}

void RenderTestApp::SoftInit()
{
	// test data
	lrc_n_vert = 0;
	lrc_n_lines = 0;

	//lrc_vert_x[lrc_n_vert] = -32;		lrc_vert_y[lrc_n_vert++] =  32;
	//lrc_vert_x[lrc_n_vert] =  32;		lrc_vert_y[lrc_n_vert++] =  32;
	//lrc_vert_x[lrc_n_vert] =  32;		lrc_vert_y[lrc_n_vert++] = -32;
	//lrc_vert_x[lrc_n_vert] = -32;		lrc_vert_y[lrc_n_vert++] = -32;
	
	//lrc_vert_x[lrc_n_vert] = -128;		lrc_vert_y[lrc_n_vert++] = 192;
	//lrc_vert_x[lrc_n_vert] = 128;		lrc_vert_y[lrc_n_vert++] = 192;
	//lrc_vert_x[lrc_n_vert] = 128;		lrc_vert_y[lrc_n_vert++] = 128;
	//lrc_vert_x[lrc_n_vert] = -128;		lrc_vert_y[lrc_n_vert++] = 128;

	//lrc_vert_x[lrc_n_vert] = -128;		lrc_vert_y[lrc_n_vert++] =  128;
	//lrc_vert_x[lrc_n_vert] = -128;		lrc_vert_y[lrc_n_vert++] = -128;
	//lrc_vert_x[lrc_n_vert] =  128;		lrc_vert_y[lrc_n_vert++] = -128;
	//lrc_vert_x[lrc_n_vert] =  128;		lrc_vert_y[lrc_n_vert++] =  128;
	//
	//lrc_lines_v1[lrc_n_lines] = 0;		lrc_lines_v2[lrc_n_lines++] = 3;
	//lrc_lines_v1[lrc_n_lines] = 3;		lrc_lines_v2[lrc_n_lines++] = 2;
	//lrc_lines_v1[lrc_n_lines] = 2;		lrc_lines_v2[lrc_n_lines++] = 1;
	//lrc_lines_v1[lrc_n_lines] = 1;		lrc_lines_v2[lrc_n_lines++] = 0;

	lrc_vert_x[lrc_n_vert] = -192;		lrc_vert_y[lrc_n_vert++] =  192;
	lrc_vert_x[lrc_n_vert] =  192;		lrc_vert_y[lrc_n_vert++] =  192;
	lrc_vert_x[lrc_n_vert] =  192;		lrc_vert_y[lrc_n_vert++] =   32;
	lrc_vert_x[lrc_n_vert] =  192;		lrc_vert_y[lrc_n_vert++] =  -32;
	lrc_vert_x[lrc_n_vert] =  192;		lrc_vert_y[lrc_n_vert++] = -192;
	lrc_vert_x[lrc_n_vert] = -192;		lrc_vert_y[lrc_n_vert++] = -192;

	lrc_lines_v1[lrc_n_lines] = 0;		lrc_lines_v2[lrc_n_lines] = 1;		lrc_lines_tex[lrc_n_lines++] = 2;
	lrc_lines_v1[lrc_n_lines] = 1;		lrc_lines_v2[lrc_n_lines] = 2;		lrc_lines_tex[lrc_n_lines++] = 0;
	lrc_lines_v1[lrc_n_lines] = 2;		lrc_lines_v2[lrc_n_lines] = 3;		lrc_lines_tex[lrc_n_lines++] = 1;
	lrc_lines_v1[lrc_n_lines] = 3;		lrc_lines_v2[lrc_n_lines] = 4;		lrc_lines_tex[lrc_n_lines++] = 0;
	lrc_lines_v1[lrc_n_lines] = 4;		lrc_lines_v2[lrc_n_lines] = 5;		lrc_lines_tex[lrc_n_lines++] = 2;
	lrc_lines_v1[lrc_n_lines] = 5;		lrc_lines_v2[lrc_n_lines] = 0;		lrc_lines_tex[lrc_n_lines++] = 0;

	for( int i=0; i<lrc_n_lines; i++ )
	{
		// Length of(x, y) int vector as 24.8 fixpoint (1.12% max error):
		//	x=abs(x);
		//	y=abs(y);
		//	s=x+y;
		//	t=abs(x-y);
		//	return s*164 + t*72 + abs(s*15-t*34);

		int v1 = lrc_lines_v1[i];
		int v2 = lrc_lines_v2[i];
		int x = lrc_vert_x[v2] - lrc_vert_x[v1];
		int y = lrc_vert_y[v2] - lrc_vert_y[v1];
		if( x<0 ) x=-x;
		if( y<0 ) y=-y;
		int s = x+y;
		int t = x-y;
		if( t<0 ) t=-t;
		x = s*15-t*34;
		if( x<0 ) x=-x;
		int len = s*164 + t*72 + x;
		len = (len+128)>>8;
		if( len<1 ) len = 1;

		lrc_lines_tx2[i] = (word)len;
	}


	// Scale size map
	soft_size_map.clear();
	//soft_size_map.resize(1201);
	soft_size_map.resize(2048);
	soft_size_map_step = soft_size_map;
	soft_scaler.clear();

	//for( int i=1; i<soft_size_map.size(); i++ )
	//{
	//	// Size = screen size of wall with 256 height
	//
	//	int step = 2;
	//	int stepnum = 9;
	//		 if( i>400 ) step = 24,		stepnum = 7;
	//	else if( i>400 ) step = 24,		stepnum = 7;
	//	else if( i>300 ) step = 12,		stepnum = 6;
	//	else if( i>150 ) step = 6, 		stepnum = 4;
	//	else if( i>100 ) step = 4, 		stepnum = 3;
	//	else if( i> 75 ) step = 3, 		stepnum = 2;
	//	else			 step = 2, 		stepnum = 1;
	//	step = max(2, i/25);
	//
	//	//if( step )	s = (i+step/2)/step*step;
	//	//else		s = 1;
	//
	//	float fs = i;
	//	float fstep = fmax(2.f, fs/25);
	//	fs = floor(fs/fstep+0.5f)*fstep;
	//	int s = int(fs);
	//
	//
	//	soft_size_map[i] = s;
	//	soft_size_map_step[i] = stepnum;
	//}

	//{
	//	int start = 1;
	//	float srem = 0;
	//	while( start<soft_size_map.size() )
	//	{
	//		float regsize = start/30.f + srem;
	//		if( regsize<1 ) regsize = 1;
	//		int ireg = int(regsize);
	//		srem = regsize - ireg;
	//		if(srem<0) srem = 0;
	//
	//		int end = start + ireg;
	//		if( end>=soft_size_map.size() )
	//			end = soft_size_map.size();
	//
	//		int mid = (start+(end-1))/2;
	//		for( int i=start; i<end; i++ )
	//			soft_size_map[i] = mid;
	//		if( start==1 )
	//			printf("Smallest scaler size = %d\n", mid);
	//
	//		start = end;
	//	}
	//}

	int total_bytes = 0;
	int ns = 0;
	{
		int start = 1;
		int srem = 0;		// fixed.8
		while( start<soft_size_map.size() )
		{
			int regsize = (start<<8)/30 + srem;		// fixed.8
			if( regsize<256 ) regsize = 256;
			int ireg = regsize >> 8;
			srem = regsize - (ireg<<8);
			if( srem<0 ) srem = 0;

			int end = start + ireg;
			if( end > soft_size_map.size() )
				end = soft_size_map.size();

			int mid = (start+(end-1))/2;
			while( start<end )
				soft_size_map[start++] = mid;

			{
				ns++;

				soft_scaler.resize(mid+1);
				SoftGenScaler(soft_scaler[mid], mid);

				total_bytes += soft_scaler[mid].size()*6;
			}
		}
		soft_size_map[0] = soft_size_map[1];
	}

	printf("SoftInit: %d unique sizes, %d scaler bytes.\n", ns, total_bytes);
}

void RenderTestApp::Wolf_LineWall_Fix(int p1x, int p1y, int p2x, int p2y, int tx1, int tx2, texture_id_t tex)
{
	GfxTexture *gtex = gfx_converter.FindTexture(tex);
	if( !gtex ) return;

	const int WOLFSCREEN_W = 160;
	const int wolf_xoffs = 80;
	const int zoom_i = 80;
	const int clip = 321;	// fix.4

	if( p1y<clip && p2y<clip )
		return;

	// Clip:	Y+X > 0
	{
		int d1 = p1y+p1x;
		int d2 = p2y+p2x;
		if( d1<0 )
		{
			if( d2<0 ) return;
			d2 -= d1;
			p1x -= (p2x-p1x)*d1/d2;
			p1y -= (p2y-p1y)*d1/d2;
			tx1 -= (tx2-tx1)*d1/d2;
		}
	}
	// Clip:	Y-X > 0		(required only for non-perspective)
	{
		int d1 = p1y-p1x;
		int d2 = p2y-p2x;
		if( d2<0 )
		{
			if( d1<0 ) return;
			d1 -= d2;
			p2x -= (p1x-p2x)*d2/d1;
			p2y -= (p1y-p2y)*d2/d1;
			tx2 -= (tx1-tx2)*d2/d1;
		}
	}

	
	p1y -= clip;
	p2y -= clip;
	if( p1y<0 )
	{
		if( p2y<0 ) return;
		tx1 -= divs(muls(p1y, tx1-tx2), p1y-p2y);
		p1x -= divs(muls(p1y, p1x-p2x), p1y-p2y);
		p1y = 0;
	}
	else if( p2y<0 )
	{
		tx2 -= divs(muls(p2y, tx1-tx2), p1y-p2y);
		p2x -= divs(muls(p2y, p1x-p2x), p1y-p2y);
		p2y = 0;
	}
	p1y += clip;
	p2y += clip;
	//tx2 -= tx1&~63;
	//tx1 &= 63;
	tx1 <<= 7;
	tx2 <<= 7;


	// xp = px*zoom / py
	int xmin = divs(muls(p1x, zoom_i), p1y);
	int xmax = divs(muls(p2x, zoom_i), p2y);
	if( xmin>=xmax ) return;

	short s1 = divs((zoom_i*32)<<12, p1y);		// won't overflow because of clipping p#y>=300
	short s2 = divs((zoom_i*32)<<12, p2y);
	short ds = divs(s2-s1, xmax-xmin);			// s# is signed word
	int u1s = tx1*s1;
	int u2s = tx2*s2;
	int du = (u2s-u1s)/(xmax-xmin);			// result can be 32-bit!!

	if( xmin+wolf_xoffs<0 )
	{
		//int d = -wolf_xoffs - xmin;
		//s1+=(s2-s1)*d/(xmax-xmin);
		//u1s+=(u2s-u1s)*d/(xmax-xmin);
		//xmin += (-wolf_xoffs-xmin);

		xmin = -wolf_xoffs;
	}
	if( xmax+wolf_xoffs>WOLFSCREEN_W ) xmax = WOLFSCREEN_W-wolf_xoffs;
	if( xmin>=xmax ) return;

	//tex <<= 6;

	short line_h1 = -64 - lrc_cam_pos_z;
	short line_h2 = 0 - lrc_cam_pos_z;
	short tex_shift = (64 + lrc_cam_pos_z)<<6;
	for(int x=xmin;x<xmax;x++)
	{
		int size = s1>>5;
		if( size<=1 ) size = 1;
		if( size>=soft_size_map.size() ) size = soft_size_map.size()-1;
		int stepnum = soft_size_map_step[size];
		size = soft_size_map[size];

		int y1 = 50 + ((size*line_h1)>>8);
		int y2 = 50 + ((size*line_h2)>>8);
		if( y1<=0 ) y1 = 0;
		if( y1>=100 ) y1 = 100;
		if( y2<=0 ) y2 = 0;
		if( y2>=100 ) y2 = 100;

		for( int y=0; y<y1; y++ )
			screen.data[(x+wolf_xoffs)+y*160] = 0x0001;

		for( int y=y1; y<y2; y++ )
		{
			int tx = ((u1s / s1)>>7)&63;
			int ty = soft_scaler[size][y]*64 + tex_shift;		// texture is shifted up by 64 units to cover -64..0 world Z coords

			if( (tx&~63) | (ty&~(63*64)) )
				screen.data[(x+wolf_xoffs)+y*160] = 0x0F00;
			else
				screen.data[(x+wolf_xoffs)+y*160] = gtex->texture.data[tx+ty];

			//if( stepnum == soft_step_debug )
			//if( ty&~63 )
			//if( (u1s/s1) & ~0xFFFF )
			//tx = u1s / s1;
			//if( tx<0x2000 || tx>0x2200 )
			//	screen.data[(x+wolf_xoffs)+y*160] = (screen.data[(x+wolf_xoffs)+y*160] & 0x00FF) | 0x0E00;
		}
		
		for( int y=y2; y<100; y++ )
			screen.data[(x+wolf_xoffs)+y*160] = 0x0001;
		
		s1 += ds;
		u1s += du;
	}
}

void RenderTestApp::SoftTest()
{
	lrc_cam_pos_x = (int)floor(cam_pos.x+0.5f);
	lrc_cam_pos_y = (int)floor(-cam_pos.y+0.5f);
	lrc_cam_pos_z = -(int)floor(cam_pos.z+0.5f);		// Z goes DOWN drom now on
	lrc_cam_angle = int(floor(cam_ypr.x/360*256+0.5f)) & 0xFF;
	//Dev.DPrintF("X %d", lrc_cam_pos_x);
	//Dev.DPrintF("Y %d", lrc_cam_pos_y);
	//Dev.DPrintF("Angle %d", lrc_cam_angle);

	// debug
	//soft_step_debug = -1;
	for( int i=0; i<=9; i++ )
		if( Dev.GetKeyStroke('0'+i) )
			soft_step_debug = i;


	// precompute vertexes
	{
		short dx = sincos_fix14[(lrc_cam_angle+64)&0xFF];
		short dy = sincos_fix14[(lrc_cam_angle)&0xFF];
		for( int id=0; id<lrc_n_vert; id++ )
		{
			short xp = lrc_vert_x[id] - lrc_cam_pos_x;
			short yp = lrc_vert_y[id] - lrc_cam_pos_y;

			lrc_vert_final_x[id] = (((int)xp)*dx - ((int)yp)*dy)>>10;
			lrc_vert_final_y[id] = (((int)xp)*dy + ((int)yp)*dx)>>10;
		}
	}

	// draw lines
	{
		texture_id_t tex[] = {
			gfx_converter.GetTextureId("RW23_4", 255),
			gfx_converter.GetTextureId("DOOR3_6", 255),
			gfx_converter.GetTextureId("RW11_3", 255),
		};
		for( int id=0; id<lrc_n_lines; id++ )
		{
			word v1 = lrc_lines_v1[id];
			word v2 = lrc_lines_v2[id];

			Wolf_LineWall_Fix(
				lrc_vert_final_x[v1],
				lrc_vert_final_y[v1],
				lrc_vert_final_x[v2],
				lrc_vert_final_y[v2],
				0,						// tx1
				lrc_lines_tx2[id],		// tx2	lrc_line_chunk[id+3]
				tex[lrc_lines_tex[id]]);
		}
	}
}


//void RenderTestApp::ExportFrame(FILE *fp)
//{
//	vector<DWORD> pix;
//	int w=0, h=0;
//	if( off_rt.GetPixels(w, h, pix) )
//	{
//		// Frame
//		fprintf(fp, "\n#define TEST_FRAME_W\t%d\n", w);
//		fprintf(fp, "#define TEST_FRAME_H\t%d\n", h);
//		fprintf(fp, "const byte TEST_FRAME[] = {\n");
//		for( int x=0; x<w; x++ )
//		{
//			fprintf(fp, "\t");
//			for( int y=0; y<h; y++ )
//				fprintf(fp, "0x%02X,", _color2chunky(pix[x+y*w]));
//			fprintf(fp, "\n");
//		}
//		fprintf(fp, "};\n");
//	}
//}



void RenderTestApp::ExportSimpleObject(FILE *fp, GfxTexture *tex, const char *name, int &total)
{
	if( !tex )
		return;

	// Dump single frame
	BinaryWordData bin;
	tex->texture.PrepareObjectData(bin, chunky_packer);
	if( fp )
		total += bin.WriteTable(fp, format("Object_%s", name).c_str(), false);
}

void RenderTestApp::ExportObjects(FILE *fp)
{
	// Forward actors
	if( fp )
		for( int j=0; j<(int)script_compiler.actors.size(); j++ )
			fprintf(fp, "extern DataActor Actor_%s;\n", script_compiler.idents[script_compiler.actors[j]->name_id].name.c_str());



	map<string, bool> done;
	int total = 0;
	for( int j=0; j<(int)script_compiler.idents.size(); j++ )
		if( script_compiler.idents[j].is_frame )
		{
			const char *name = script_compiler.idents[j].name.c_str();
			if( done[name] )
				continue;
			done[name] = true;

			GfxAsset *asset = gfx_converter.FindGfxAsset(name, true);
			if( asset && asset->is_multipart_gfx )
			{
				// Dump multipart object
//				printf(" - multipart gfx found\n");
				if( fp )
				{
					// Assure part objects
					for( int i=0; i<asset->overlays.size(); i++ )
					{
						GfxAsset::Overlay &ovr = asset->overlays[i];
						if( ovr.mode == GfxAsset::Overlay::M_PART && ovr.gfx != NULL && !done[ovr.gfx->name] && ovr.gfx->textures.size()>=0 )
						{
							GfxTexture *tex = ovr.gfx->textures[0];
							ExportSimpleObject(fp, tex, ovr.gfx->name.c_str(), total);
							done[ovr.gfx->name] = true;
						}
					}

					// Export definition
					fprintf(fp, "\nconst dword Object_%s[] = {\n", name);
					for( int i=0; i<asset->overlays.size(); i++ )
					{
						GfxAsset::Overlay &ovr = asset->overlays[i];
						if( ovr.mode == GfxAsset::Overlay::M_PART && ovr.gfx != NULL )
						{
							dword offs = 0;
							offs |= dword(word(-ovr.xoffs)) << 16;
							offs |= dword(word(-ovr.yoffs));
							fprintf(fp, "\t0x%08X, (dword)&Object_%s,\n", offs, ovr.gfx->name.c_str());
						}
						else
							fprintf(fp, "\tERROR: invalid multipart overlay\n");
					}
					fprintf(fp, "\t0, 0\n");
					fprintf(fp, "};\n");
				}
				continue;
			}

			GfxTexture *tex = gfx_converter.FindTexture(name, -1);
			ExportSimpleObject(fp, tex, name, total);
		}

	if( fp ) 
		printf("Export: %d object bytes\n", total);
}

void RenderTestApp::ExportSky(FILE *fp)
{
	GfxTexture *gtex = gfx_converter.FindTexture("SKY", 255);
	if( !gtex )
		return;

	DataTexture &tex = gtex->texture;

	if( fp ) fprintf(fp, "\nconst byte SKYTEX_DATA[] = {\n");
	if( fp ) fprintf(fp, "\t// %s  %dx%d\n", "SKY", tex.width, tex.height);
	for( int x=0; x<tex.width; x++ )
	{
		if( fp ) fprintf(fp, "\t");
		for( int y=0; y<tex.height; y++ )
		{
			byte chunky = chunky_packer.Process(tex.data[x+y*tex.width]);
			if( fp )
				fprintf(fp, "0x%02X,", chunky);
		}
		if( fp ) fprintf(fp, "\n");
	}
	if( fp ) fprintf(fp, "};\n");
}

int RenderTestApp::ExportWeaponSprite(FILE *fp, GfxAsset *asset)
{
	if( !asset || !asset->asset_group || !asset->asset_group->opt_palette || asset->textures.size()<=0 )
		return 0;

	const char *name = asset->name.c_str();
	GfxTexture *gtex = asset->textures[0];
	DataTexture &tex = gtex->texture;
	int size = 0;

	// Data format
	//	- word[15]	palette
	//	- word[4]	control
	//	- dword[8]	sprite data pointers
	//	- ...		data

	fprintf(fp, "\n#if WEAPONSPRITES\n");

	word sp_xpos[8] ={};
	word sp_ypos[8] ={};
	for( int sprite=0; sprite<8; sprite++ )
	{
		fprintf(fp, "\n__chip word WEAPONSPRITEDATA_%s_s%d[] = {\n", name, sprite);

		const int x0 = sprite/2*16;
		int y1 = tex.height;
		int y2 = tex.height;
		for( int y=0; y<tex.height; y++ )
			for( int x=0; x<16 && x0+x<tex.width; x++ )
			{
				DWORD col = tex.data[(x0+x)+y*tex.width];
				if( !COLOR_IS_TRANSPARENT(col) )	// trim empty weapon sprite columns
				{
					if( y<y1 ) y1 = y;
					//y2 = y+1;
					break;
				}
			}

		//	0x2C40, 0x3200,			// Word 0:	Bits 15-8 contain the low 8 bits of VSTART
		word ystart = 0x2C + y1;// - tex.off_y;
		word yend   = 0x2C + y2;// - tex.off_y;
		word xstart = /*0x80 + */sprite/2*16;// - tex.off_x;
		//sprpos[sprite] = ((ystart&0xFF)<<8) | ((xstart&0x1FE)>>1);
		//sprctl[sprite] = ((yend  &0xFF)<<8) | (sprite&1 ? 0x80 : 0x00) | (xstart&1);
		sp_xpos[sprite] = xstart;
		sp_ypos[sprite] = ystart;

		for( int y=y1; y<y2; y++ )
		{
			word data[4] ={};
			for( int x=0; x<16 && x0+x<tex.width; x++ )
			{
				DWORD col = tex.data[(x0+x)+y*tex.width];
				if( COLOR_IS_TRANSPARENT(col) )	// handle weapon sprite data transparency
					col = 0;
				else
					col = (col&0xFF)+1;

				if( col&1 ) data[0] |= 0x8000>>x;
				if( col&2 ) data[1] |= 0x8000>>x;
				if( col&4 ) data[2] |= 0x8000>>x;
				if( col&8 ) data[3] |= 0x8000>>x;
			}
			fprintf(fp, "\t0x%04X, 0x%04X,\n", data[sprite%2*2+0], data[sprite%2*2+1]);
			size += 4;
		}
		fprintf(fp, "\t0x0000, 0x0000\n");
		size += 4;
		fprintf(fp, "};\n");
	}


	// info
	vector<GfxColor> &colors = asset->asset_group->opt_palette->colors;
	fprintf(fp, "const WeaponSpriteInfo WEAPONSPRITE_%s = {\n\t", name);
	for( int i=0; i<15; i++ )
	{
		DWORD col = (i<colors.size() ? colors[i].color_out : 0x000000);
		col = ((col&0xF00000)>>12) | ((col&0x00F000)>>8) | ((col&0x0000F0)>>4);
		fprintf(fp, "0x%03X,", col);
	}
	//fprintf(fp, "\n\t");
	//for( int i=0; i<8; i++ ) fprintf(fp, "0x%04X,", sp_xpos[i]);
	//fprintf(fp, "\n\t");
	//for( int i=0; i<8; i++ ) fprintf(fp, "0x%04X,", sp_ypos[i]);
	//fprintf(fp, "\n");
	//for( int i=0; i<8; i++ ) fprintf(fp, "\tWEAPONSPRITEDATA_%s_s%d,\n", name, i);
	fprintf(fp, "\n");
	int yprev = 0;
	for( int i=0; i<8; i+=2 )
	{
		fprintf(fp, "\t%4d, WEAPONSPRITEDATA_%s_s%d, WEAPONSPRITEDATA_%s_s%d,\n", sp_ypos[i] - yprev, name, i, name, i+1 );
		yprev = sp_ypos[i];
	}
	fprintf(fp, "};\n");

	fprintf(fp, "#endif\n");

	return size;
}

int RenderTestApp::ExportWeaponAnimation(FILE *fp, GfxAnimation *anim)
{
	// info
	fprintf(fp, "\n#if WEAPONSPRITES\n");
	fprintf(fp, "const WeaponSpriteAnimFrame WEAPONSPRITEANIM_%s[] = {\n", anim->name.c_str());
	for( int i=0; i<anim->frames.size(); i++ )
	{
		GfxAnimation::Frame &frame = anim->frames[i];
		fprintf(fp, "\t{ %5d, &WEAPONSPRITE_%-16s, %4d, %4d, %d },\n",
			int(frame.delay*300+0.5f),
			frame.asset->name.c_str(),
			frame.add_off_x - frame.asset->textures[0]->texture.off_x + 0x80,
			frame.add_off_y - frame.asset->textures[0]->texture.off_y,
			frame.walkswing ? 1 : 0 );
	}

	int loopback = anim->frames.size();
	if( anim->loop_pos>=0 && anim->loop_pos<anim->frames.size() )
		loopback = anim->frames.size() - anim->loop_pos;

	fprintf(fp, "\t{ %5d }\n", -loopback*12);			// 12 = sizeof(WeaponSpriteAnimFrame)
	fprintf(fp, "};\n");
	fprintf(fp, "#endif\n");

	return 0;
}

void RenderTestApp::ExportWeaponSpritesAll(FILE *fp)
{
	// Export weapon sprites
	map<GfxAsset*, bool> asset_done;
	int size = 0;

	for( int j=0; j<(int)script_compiler.idents.size(); j++ )
		if( script_compiler.idents[j].is_weaponsprite )
		{
			const char *name = script_compiler.idents[j].name.c_str();

			GfxAnimation *anim = gfx_converter.FindAnimation(name, false);
			if( anim && anim->name.size()>0 && anim->name[0]!='.' )
			{
				for( int k=0; k<(int)anim->frames.size(); k++ )
				{
					GfxAsset *asset = anim->frames[k].asset;
					if( !asset_done[asset] )
					{
						size += ExportWeaponSprite(fp, asset);
						asset_done[asset] = true;
					}
				}
				size += ExportWeaponAnimation(fp, anim);
			}
		}

	printf("Export: %d weapon sprite bytes.\n", size);
}


void RenderTestApp::ExportWeapon(FILE *fp)
{
	// Export weapon frames
	int size = 0;

	for( int j=0; j<(int)script_compiler.idents.size(); j++ )
		if( script_compiler.idents[j].is_weaponframe )
		{
			const char *name = script_compiler.idents[j].name.c_str();

			int plane_size = 0;
			GfxTexture *gtex = gfx_converter.FindTexture(name, -1);
			if( !gtex ) continue;
			DataTexture &tex = gtex->texture;

			fprintf(fp, "\n#if !WEAPONSPRITES\n");

			fprintf(fp, "__chip word WEAPON_%s[] = {\n", name);
			for( int pass=0; pass<=4; pass++ )
			{
				if( pass )
					fprintf(fp, "\t// Bitplane %d\n", pass-1);
				else
					fprintf(fp, "\t// Mask\n");

				plane_size = 0;
				for( int y=0; y<tex.height; y++ )
				{
					fprintf(fp, "\t");
					for( int x0=0; x0<tex.width; x0+=8 )
					{
						word w = 0;
						for( int x=0; x<8 && x0+x<tex.width; x++ )
						{
							DWORD col = tex.data[(x0+x)+y*tex.width];
							word bits = 0;
							if( pass==0 )
							{
								if( !COLOR_IS_TRANSPARENT_LOW(col)  ) bits |= 1;
								if( !COLOR_IS_TRANSPARENT_HIGH(col) ) bits |= 2;
							}
							else
							{
								if( !COLOR_IS_TRANSPARENT_LOW(col)  && (col&(0x0001<<(pass-1))) ) bits |= 1;
								if( !COLOR_IS_TRANSPARENT_HIGH(col) && (col&(0x0100<<(pass-1))) ) bits |= 2;
							}
							w |= bits<<(14-x*2);
						}
						fprintf(fp, "0x%04X,", w);
						size += 2;
						plane_size += 2;
					}
					fprintf(fp, "\n");
				}
			}

			fprintf(fp, "};\n");

			// info
			fprintf(fp, "const WeaponFrameInfoX WEAPONFRAME_%s = {\n", name);
			fprintf(fp, "\tWEAPON_%s,\n", name);
			fprintf(fp, "\tWEAPON_%s + %d,\n", name, plane_size/2*1);
			fprintf(fp, "\tWEAPON_%s + %d,\n", name, plane_size/2*2);
			fprintf(fp, "\tWEAPON_%s + %d,\n", name, plane_size/2*3);
			fprintf(fp, "\tWEAPON_%s + %d,\n", name, plane_size/2*4);
			fprintf(fp, "\t%d, %d,\n", tex.width, tex.height);
			fprintf(fp, "\t%d, %d\n", -tex.off_x, -tex.off_y);
			fprintf(fp, "};\n");
			fprintf(fp, "#endif\n");
		}

	printf("Export: %d weapon bytes.\n", size);
}

void RenderTestApp::ExportWeaponST(FILE *fp)
{
	// Export weapon frames
	int size = 0;
	string colstring;

	for( int j=0; j<(int)script_compiler.idents.size(); j++ )
		if( script_compiler.idents[j].is_weaponframe )
		{
			const char *name = script_compiler.idents[j].name.c_str();

			int plane_size = 0;
			GfxTexture *gtex = gfx_converter.FindTexture(name, -1);
			if( !gtex ) continue;
			DataTexture &tex = gtex->texture;

			int x1 = -tex.off_x;
			int x2 = x1 + tex.width;
			int y1 = 20-tex.off_y;
			int y2 = y1 + tex.height;
			x1 = x1 & ~7;
			x2 = (x2+7) & ~7;
			if( x1<0 ) x1 = 0;
			if( x2>160 ) x2 = 160;
			if( y1<0 ) y1 = 0;
			if( y2>100 ) y2 = 100;
			if( x1>=x2 ) continue;
			if( y1>=y2 ) continue;


			fprintf(fp, "word WEAPON_%s[] = {\n", name);
			for( int ys=y1; ys<y2; ys++ )
				for( int xs=x1; xs<x2; xs+=8 )
				{
					word mask = 0, p1 = 0, p2 = 0, p3 = 0, p4 = 0;
					colstring.clear();
					for( int b=0; b<8; b++ )
					{
						int xt = xs + b + tex.off_x;
						int yt = ys + tex.off_y-20;
						DWORD col = COLOR_TRANSPARENT;
						if( uint32_t(xt)<uint32_t(tex.width) && uint32_t(yt)<uint32_t(tex.height) )
							col = tex.data[xt+yt*tex.width];

						if( !COLOR_IS_TRANSPARENT_LOW(col) )
						{
							word bit = word(1<<(15-b*2));
							mask |= bit;
							if( col & 0x0001 ) p1 |= bit;
							if( col & 0x0002 ) p2 |= bit;
							if( col & 0x0004 ) p3 |= bit;
							if( col & 0x0008 ) p4 |= bit;
							colstring.push_back("0123456789ABCDEF"[col&0x0F]);
						}
						else
							colstring.push_back('-');
						if( !COLOR_IS_TRANSPARENT_HIGH(col) )
						{
							word bit = word(1<<(14-b*2));
							mask |= bit;
							if( col & 0x0100 ) p1 |= bit;
							if( col & 0x0200 ) p2 |= bit;
							if( col & 0x0400 ) p3 |= bit;
							if( col & 0x0800 ) p4 |= bit;
							colstring.push_back("0123456789ABCDEF"[(col>>8)&0x0F]);
						}
						else
							colstring.push_back('-');
					}
					if( mask )
						fprintf(fp, "\t0x%04X, 0x%04X,0x%04X,0x%04X,0x%04X,\t// %3d..%3d, %3d  %s\n", mask, p1, p2, p3, p4, xs, xs+7, ys, colstring.c_str());
					else
						fprintf(fp, "\t0x%04X,\t\t\t\t\t\t\t\t\t// %3d..%3d, %3d\n", mask, xs, xs+7, ys);
					size += 10;
				}
			fprintf(fp, "};\n");

			// info
			fprintf(fp, "const WeaponFrameInfoST WEAPONFRAME_%s = {\n", name);
			fprintf(fp, "\tWEAPON_%s,\n", name);
			fprintf(fp, "\t%d,\n", x1/8*8 + y1*(40*8));		// screen offset in bytes
			fprintf(fp, "\t%d, %d\n", (x2-x1)/8, y2-y1);	// size
			fprintf(fp, "};\n");
		}

	printf("Export: %d weapon bytes.\n", size);
}


void RenderTestApp::Export(platform_t platform)
{
	// Setup platform & directories
	string platform_name = "amiga";
	string platform_ext = "";

	if( platform == PLATFORM_ST )
	{
		platform_name = "st";
		platform_ext = "_st";
	}

	string export_dir_amiga = proj_cfg.GetString("export-directory-"+platform_name, "export");
	string export_path =  export_dir_amiga + "/export_data" + platform_ext + ".inc";
	string asm_export_path = export_dir_amiga + "/export_map_asm" + platform_ext + ".i";
	vector<string> bin_export_paths;		// "export/export_map" + platform_ext + ".bin"
	proj_cfg.GetStringArray("export-binary-amiga", bin_export_paths);


	printf("======== Export ========\n");

	vector<texture_id_t> tex_list;
	map<texture_id_t, int> tex_offsets;
	DataBlockSet bset;
	mapgen.AssureVisibility();
	mapgen.ExportGather(tex_list);

	do {
		//string export_dir_amiga = proj_cfg.GetString("export-directory-amiga", "export");
		//string export_path =  export_dir_amiga + "/frame.inc";
		//string asm_export_path = export_dir_amiga + "/frame_asm.i";
		//string bin_export_path = proj_cfg.GetString("export-binary-amiga", "export/frame_asm.bin");
		//const char *binpath = bin_export_path.c_str();

		FILE *fp = fopen(export_path.c_str(), "wt");
		if( !fp ) break;

		FILE *asm_fp = fopen(asm_export_path.c_str(), "wt");
		if( !asm_fp )
		{
			fclose(fp);
			break;
		}

		if( platform == PLATFORM_AMIGA )
		{
			// Amiga
			chunky_packer.Amiga_Init();
			ExportWeapon(fp);
			ExportWeaponSpritesAll(fp);
		}
		else if( platform == PLATFORM_ST )
		{
			// Atari ST
			chunky_packer.ST_StartGather();
			ExportObjects(NULL);																	// chunky
			mapgen.ExportTextures(NULL, tex_list, tex_offsets, chunky_packer);						// chunky
			mapgen.ExportWrite(NULL, NULL, bset, script_compiler, chunky_packer, tex_offsets);		// chunky
			ExportSky(NULL);																		// chunky
		
			chunky_packer.ST_StartApply();
			ExportWeaponST(fp);

			bset.Clear();
		}

		ExportObjects(fp);																						// chunky
		mapgen.ExportTextures(NULL, tex_list, tex_offsets, chunky_packer);										// chunky
		mapgen.ExportWrite(fp, asm_fp, bset, script_compiler, chunky_packer, tex_offsets);						// chunky
		ExportSky(fp);																							// chunky
		gfx_converter.ExportGraphics(fp, platform);
		gfx_converter.ExportSounds(fp, script_compiler, platform);


		if( platform == PLATFORM_ST )
			chunky_packer.st_color_table.WriteTable(fp, "ST_CHUNKY_COLOR_PAIRS", false);


		fclose(fp);
		if( asm_fp ) fclose(asm_fp);

		for( string path : bin_export_paths )
		{
			for( int i=0; i<path.size(); i++ )
				if( path[i]=='*' )
				{
					string name = FilePathGetPart(modmgr.GetMapPath().c_str(), false, true, false);
					path.erase(path.begin() + i);
					path.insert(path.begin() + i, name.begin(), name.end());
					break;
				}
			bset.ExportAllAsFile(path.c_str());
		}

	} while( 0 );


	mapgen.ExportToSim(dread_sim);

}


void RenderTestApp::RunWinUAE(const char *config)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));


	CreateProcess(
		proj_cfg.GetString("run-winuae-application", "winuae64.exe").c_str(),									// Application name
		(char*)format(proj_cfg.GetString("run-winuae-command-line", "winuae64.exe").c_str(), config).c_str(),	// Command line
		NULL,																									// Process attributes
		NULL,																									// Thread attributes
		FALSE,																									// Inherit handles
		CREATE_NEW_PROCESS_GROUP | DETACHED_PROCESS,															// Creation flags
		NULL,																									// Environment
		proj_cfg.GetString("run-winuae-working-dir", "").c_str(),												// Current directory
		&si,																									// Startup info
		&pi																										// Process information
	);
}
