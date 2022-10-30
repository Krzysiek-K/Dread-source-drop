
#include "common.h"



// -------------------------------- TexScalerApp --------------------------------


static const char *DRAW_MODES[] = {
	"Direct",
	"Wall",
	"Cube"
};
#define NUM_DRAW_MODES		(sizeof(DRAW_MODES)/sizeof(DRAW_MODES[0]))


void TexScalerApp::InitBank(TexScalerMethodBank &m, const char *name, const char *ser)
{
	method_banks.push_back(&m);
	m.SetNames(name, ser);
}


void TexScalerApp::Init()
{
	method_banks.clear();

	InitBank(method_basic_clear, "Basic/clear", "basic-clear");
	InitBank(method_basic_avoid, "Basic/avoid", "basic-avoid");
	InitBank(method_basic_masked, "Basic/masked", "basic-masked");
	InitBank(method_shift_clear, "Shift/clear", "shift-clear");
	InitBank(method_shift_avoid, "Shift/avoid", "shift-avoid");
	InitBank(method_shift_masked, "Shift/masked", "shift-masked");

	SetScreenSize(160, 100);
	tex.InitConfig("texscale-tex-path");
	renderer.SerializeSettings(cfg.GetNode("texscale-renderer", 0, false));
	bank_select = cfg.GetInt("texscale-bank-select", 0);
	draw_mode = cfg.GetInt("texscale-draw-mode", 0);
	current_bank = cfg.GetInt("texscale-current-bank", 0);

	TreeFileRef tf = cfg.GetNode("texscale-banks", 0, true);
	for( int i=0; i<(int)method_banks.size(); i++ )
		method_banks[i]->Serialize(tf.SerChild(method_banks[i]->bank_ser_name.c_str()));

	wall_distance = 5;
	wall_angle = 0;
	wall_scroll = 0;
}

void TexScalerApp::Shutdown()
{
	renderer.SerializeSettings(cfg.GetNode("texscale-renderer", 0, true));
	cfg.SetInt("texscale-draw-mode", draw_mode);
	cfg.SetInt("texscale-bank-select", bank_select);

	TreeFileRef tf = cfg.GetNode("texscale-banks", 0, true);
	for(int i=0;i<(int)method_banks.size();i++)
		method_banks[i]->Serialize(tf.SerChild(method_banks[i]->bank_ser_name.c_str()));
}

void TexScalerApp::SetScreenSize(int w, int h)
{
	screen.Resize(w, h);
}

void TexScalerApp::DrawColumn(int x, int tx, TexScalerMethod &m)
{
	renderer.dst_size = screen.height;
	renderer.src_size = tex.height;
	if( !renderer.Render(m) )
	{
		for( int y=0; y<screen.height; y++ )
			screen(x, y) = 0xFF00FF;
		return;
	}

	for( int y=0; y<screen.height; y++ )
	{
		byte val = byte(renderer.target[renderer.margin_dst_pre+y] >> m.config.dst_shift);
		if( val==0xE7 )
			screen(x, y) = 0x808080;
		else if( val<tex.height )
			screen(x, y) = tex((tx%tex.width+tex.width)%tex.width, val);
		else
			screen(x, y) = 0xFF00FF;
	}
}


void TexScalerApp::Run()
{
	screen.Clear(0x404040);

	for(int i=0;i<(int)method_banks.size();i++)
		if( method_banks[i]->BuildTime(renderer,100) )
			break;

	if( draw_mode==0 )
	{
		TexScalerMethod met;
		met.config.run_count = 32;
		met.config.run_length = 1;
		met.config.src_stride = 1;
		met.blit.InitFromConfig(met.config);

		TexScalerMethodBank *method = GetCurentBank();

		static int scroll = 0;
		scroll++;
		if( method )
		{
			for( int x=0; x/3<(int)method->methods.size(); x++ )
			{
				int m = x/3;
				if( m<(int)method->methods.size() && method->methods[m].stats.wall_size==m )
					DrawColumn(x, x+scroll, method->methods[m]);
				else
					DrawColumn(x, x, met);
			}
		}
	}
	else if( draw_mode==1 )
	{
	}

	//	// render
	//	for( int x=0; x<tex.width; x++ )
	//		for( int y=0; y<tex.height; y++ )
	//			screen(x,y) = tex.data[x+y*tex.width];

	// draw screen
	screen.Draw(0, 0, 4);
}

void TexScalerApp::RunMenu()
{
	static float build_progress;

	// building
	{
		ms.Panel();
		TexScalerMethodBank *bank = GetCurentBank();

		ms.LineSetup(32, -1, 32);
		ms.Group(3);
		ms.RockerSwitch((bank ? bank->bank_name.c_str() : "---"), current_bank, 0, method_banks.size()-1);

		if(bank)
		{
			if( bank->IsBuilding() )
			{
				bank->MenuProgress("Building...");
			}
			else
			{
				if( ms.Button(&build_progress, "Rebuild") )
					bank->RequestRebuild(renderer);
			}
		}
	}

	// config
	{
		if( ms.Slider(format("Dst margin: %d", renderer.margin_dst_pre).c_str(), renderer.margin_dst_pre, 0, 100) )
			renderer.margin_dst_post = renderer.margin_dst_pre;
		if( ms.Slider(format("Src margin: %d", renderer.margin_src_pre).c_str(), renderer.margin_src_pre, 0, 100) )
			renderer.margin_src_post = renderer.margin_src_pre;

		TexScalerMethodBank *bank = GetCurentBank();
		if(bank)
			bank->scorecfg.MenuEdit();
	}

	// draw mode
	ms.Panel();
	ms.LineSetup(32, -1, 32);
	ms.Group(3);
	ms.RockerSwitch(DRAW_MODES[draw_mode], draw_mode, 0, NUM_DRAW_MODES-1);

	// texture
	ms.Panel();
	tex.MenuEdit("Texture");
}
