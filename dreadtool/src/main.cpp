
#include "common.h"
#include "app_rendertest.h"



DevFont font("Verdana", 27, 0, false, false);

Config cfg;
AppConfig proj_cfg;


//TexScalerApp app_texscaler;
SubApplication * const app = &app_rendertest;
SubApplication * const menu_app = &app_rendertest;


//SubApplication *APPS[] = {
//	&app_texscaler,
//	&app_rendertest,
//	NULL
//};



bool AppConfig::LoadFile(const char *path)
{
	vector<string> lines;
	if( !NFS.GetFileLines(path, lines) )
	{
		printf("ERROR: Can't open config: %s\n", path);
		return false;
	}

	for( const string &line : lines )
	{
		const char *s = line.c_str();
		ParseWhitespace(s);
		if( !*s || *s=='#' || *s=='/' ) continue;

		Entry e;
		ParseStringT(s, e.key, ":");
		ParseWhitespace(s);
		if( *s++!=':' ) printf("ERROR: Bad line in config '%s': %s\n", path, line.c_str());
		e.value = ParseString(s);
		ParseWhitespace(s);
		if( *s==';' ) s++;
		ParseWhitespace(s);
		if( *s ) printf("ERROR: Bad line in config '%s': %s\n", path, line.c_str());

		data.push_back(e);
	}

	return true;
}

int AppConfig::GetInt(const string &name, int def)
{
	string str = GetString(name, "");
	const char *s = str.c_str();
	ParseWhitespace(s);
	const char *b = s;
	int value = ParseInt(s);
	return (s==b) ? def : value;
}

float AppConfig::GetFloat(const string &name, float def)
{
	string str = GetString(name, "");
	const char *s = str.c_str();
	ParseWhitespace(s);
	const char *b = s;
	float value = ParseFloat(s);
	return (s==b) ? def : value;
}

string AppConfig::GetString(const string &name, const string &def)
{
	for( int i=(int)data.size()-1; i>=0; i-- )
		if( data[i].key == name )
			return data[i].value;
	return def;
}

bool AppConfig::GetStringArray(const string &name, vector<string> &arr, bool append)
{
	if( !append )
		arr.clear();

	int len = 1000000;
	if( name.size()>0 && name.back()=='*' )
		len = name.size() - 1;

	for( int i=0; i<(int)data.size()-1; i++ )
		if( strncmp(data[i].key.c_str(), name.c_str(), len) == 0 )
			arr.push_back(data[i].value);
	return arr.size() > 0;
}








void SetResolution(int w,int h)
{
	int pw, ph;
	Dev.GetScreenSize(pw, ph);
	if(pw!=w || ph!=h)
		Dev.SetResolution(w, h, false);
}

//void AppSelectMenu()
//{
//	ms.Panel();
//	ms.Label("Select App");
//	for(int i=0;APPS[i];i++)
//		if( ms.Button(APPS[i], APPS[i]->GetName(), app==APPS[i] ? MF_ACTIVE : 0) )
//		{
//			app = APPS[i];
//			menu_app = app;
//			cfg.SetInt("active-app", i);
//		}
//}

void KeepWindowPosition(HWND hwnd, const string &name, bool save)
{
	if( save )
	{
		RECT rect;
		if( !GetWindowRect(hwnd, &rect) )
			return;
		cfg.SetInt((name+"-x").c_str(), rect.left);
		cfg.SetInt((name+"-y").c_str(), rect.top);
		cfg.SetInt((name+"-w").c_str(), rect.right - rect.left);
		cfg.SetInt((name+"-h").c_str(), rect.bottom - rect.top);
	}
	else
	{
		int xp = cfg.GetInt((name+"-x").c_str(), -1000000);
		int yp = cfg.GetInt((name+"-y").c_str(), -1000000);
		int w = cfg.GetInt((name+"-w").c_str(), -1000000);
		int h = cfg.GetInt((name+"-h").c_str(), -1000000);
		if( xp<=-32000 || yp<=-32000 || w<=-32000 || h<=-32000 )
			return;
		SetWindowPos(hwnd, 0, xp, yp, w, h, /*SWP_NOSIZE |*/ SWP_NOZORDER);
	}
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, int nShowCmd)
{
	cfg.Load("config.cfg", true);
	proj_cfg.LoadFile("project.cfg");
	proj_cfg.LoadFile("user.cfg");

	//Dev.SetResolution(cfg.GetInt("width", 640), cfg.GetInt("height", 480), false);
	//Dev.SetResolution(640+MENU_WIDTH, 480+32, false);
	Dev.SetResolution(640+MENU_WIDTH, 512+32, false);
	Dev.Init();
	Dev.MainLoop();
	Dev.MainLoop();
	//Dev.SetTopmost(true);
	Dev.SetReloadTimer(500);

	KeepWindowPosition(Dev.GetHWnd(), "appwindow", false);
	KeepWindowPosition(GetConsoleWindow(), "appconsole", false);


	// init
//	{
//		for( int i=0; APPS[i]; i++ )
//			APPS[i]->Init();
//		int aid = cfg.GetInt("active-app", -1);
//		if(aid>=0 && aid<sizeof(APPS)/sizeof(APPS[0]))
//		{
//			app = APPS[aid];
//			menu_app = APPS[aid];
//		}
//	}
	app_rendertest.Init();



	while( Dev.MainLoop() )
	{
		Dev->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 0x303030, 1.f, 0);
		Dev->SetRenderState(D3DRS_ZENABLE, FALSE);
		
		// multitouch
		{
			static bool altstate = false;
			int mx=-1, my=-1;
			Dev.GetMousePos(mx, my);
			mt.TouchUpdate(0, 0, mx, my, Dev.GetKeyState(VK_LBUTTON));

			bool alt = Dev.GetKeyState(VK_LMENU);
			if( alt!=altstate )
				if( !alt )	mt.TouchUpdate(1, 0, mx, my, true);
				else		mt.TouchUpdate(1, 0, -1, -1, false);
				altstate = alt;
		}

		// menu
		ms.OnFrameStart();

		// run
		if(app)
			app->Run();

		// menu
		{
			//if( Dev.GetKeyStroke(VK_ESCAPE) )
			//	menu_app = NULL;
			
			int sx, sy;
			Dev.GetScreenSize(sx, sy);
			ms.Begin(sx-MENU_WIDTH, 0, sx, sy);
			if( menu_app )
					menu_app->RunMenu();
			//	else
			//		AppSelectMenu();
			ms.End();
		}

		// end
		ms.OnFrameEnd();
		mt.OnAfterFrame();
		KeepWindowPosition(Dev.GetHWnd(), "appwindow", true);

		ami_paula.PlaybackTick();
	}


	KeepWindowPosition(GetConsoleWindow(), "appconsole", true);

	// shutdown
	//for( int i=0; APPS[i]; i++ )
	//	APPS[i]->Shutdown();
	app_rendertest.Shutdown();

	return 0;
}

int main()
{
	return WinMain(0, 0, "", 0);
}
