
#include "common.h"
#include "menu.h"


#define PANEL_PADDING       10
#define DEF_LINE_HEIGHT		24
#define BUTTON_SPACING      5
#define BUTTON_WIDTH        64
#define BUTTON_HEIGHT       12
#define NEW_LINE_SPACING    2
#define FONT_HEIGHT         12
#define FONT_START_X        3
#define FONT_START_Y        1

//#define BUTTON_COLOR        0x308030
//#define BUTTON_COLOR_HOVER  0x60C060
//#define BUTTON_COLOR_ACTIVE 0x80FF80
//#define LABEL_BOX_COLOR     0x308030
//#define PANEL_COLOR         0x205820

#define BUTTON_COLOR        0x505080
#define BUTTON_COLOR_HOVER  0x8080C0
#define BUTTON_COLOR_ACTIVE 0xC0C0FF
#define LABEL_BOX_COLOR     0x505080
#define PANEL_COLOR         0x303058


MenuService ms;

enum {
	MENU_LAYER_PANEL,
	MENU_LAYER_BUTTONS,
	MENU_LAYER_DRAWINGS,
	MENU_LAYER_FONTS,
};

static CanvasLayerDesc _cdl[] = {
	{0,"",	RSF_BLEND_ALPHA,TEXF_MIPMAP},		// panel
	{0,"",	RSF_BLEND_ALPHA,TEXF_MIPMAP},		// buttons
	{0,"",	RSF_BLEND_ALPHA,TEXF_MIPMAP},		// drawings
	{0,"",	RSF_BLEND_ALPHA,TEXF_MIPMAP},		// fonts
	{}
};
DevCanvas menu_canvas(_cdl);
DevTxFont menu_font("app_data/Verdana_27");
DevTexture menu_tex("app_data/uiskin");
DevTexture menu_colorcircle("app_data/colorcircle");
DevTexture menu_colordot("app_data/colordot");


static DWORD color_set[][6] = {
	// off1			off2		on1			on2			empty1		empty2
	{ 0xFFC0C0C0, 0xFF606060, 0xFFFFFFFF, 0xFFC0C0C0, 0xFF404040, 0xFF808080 },		// gray	(default)
	{ 0xFF8080FF, 0xFF0000C0, 0xFFC0C0FF, 0xFF4040C0, 0xFF404040, 0xFF808080 },		// blue
	{ 0xFF80FF80, 0xFF008000, 0xFFC0FFC0, 0xFF40C040, 0xFF404040, 0xFF808080 },		// green
	{ 0xFFFFFF80, 0xFF808000, 0xFFFFFFC0, 0xFFC0C040, 0xFF404040, 0xFF808080 },		// yellow
	{ 0xFFFF8080, 0xFF800000, 0xFFFFC0C0, 0xFFC04040, 0xFF404040, 0xFF808080 },		// red
};


bool get_open_filename(string &path,char *filter,char *ext)
{
	string cdir = GetCurrentDir();

	char buffer[MAX_PATH+1];
	strncpy(buffer,path.c_str(),MAX_PATH);
	buffer[MAX_PATH] = 0;

	OPENFILENAME of;
	memset(&of,0,sizeof(of));
	of.lStructSize = sizeof(of);
	of.lpstrFilter = filter;
	of.lpstrFile = buffer;
	of.nMaxFile = MAX_PATH;
	of.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	of.lpstrDefExt = ext;

	bool ok = (GetOpenFileName(&of)!=0);
	if(ok) path = buffer;

	SetCurrentDirectory(cdir.c_str());
	return ok;
}

bool get_save_filename(string &path,char *filter,char *ext)
{
	string cdir = GetCurrentDir();

	char buffer[MAX_PATH+1];
	strncpy(buffer,path.c_str(),MAX_PATH);
	buffer[MAX_PATH] = 0;

	OPENFILENAME of;
	memset(&of,0,sizeof(of));
	of.lStructSize = sizeof(of);
	of.lpstrFilter = filter;
	of.lpstrFile = buffer;
	of.nMaxFile = MAX_PATH;
	of.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	of.lpstrDefExt = ext;

	bool ok = (GetSaveFileName(&of)!=0);
	if(ok) path = buffer;

	SetCurrentDirectory(cdir.c_str());
	return ok;
}


vec3 rgb_to_hsv(const vec3 &rgb)
{
	float cmin = min(rgb.x, min(rgb.y, rgb.z));
	float val = max(rgb.x, max(rgb.y, rgb.z));
	float hue = 0, sat = 0;
	//float cmid = rgb.x + rgb.y + rgb.z - cmin - cmax;
	
	// slice	0	1	2	3	4	5	6
	//	R		1   1 \ 0   0   0 / 1   1
	//	G		0 / 1   1   1 \ 0   0   0
	//	B		0	0	0 / 1	1	1 \ 0
	if( val>cmin )
	{
		float p = (cmin==rgb.x) ? rgb.y-rgb.z : ((cmin==rgb.y) ? rgb.z-rgb.x : rgb.x-rgb.y);
		float s = (cmin==rgb.x) ? 3 : ((cmin==rgb.y) ? 5 : 1);
		hue = (s-p/(val-cmin))/6;
		sat = (val-cmin)/val;
	}
	return vec3(hue,sat,val);
}

vec3 hsv_to_rgb(const vec3 &hsv)
{
	if(hsv.z==0)
		return vec3(0, 0, 0);

	float h = fmod(fmod(hsv.x, 1)+1, 1)*6;
	int i = floor(h);
	float f = h - i;
	float p = hsv.z*(1-hsv.y);
	float q = hsv.z*(1-hsv.y*f);
	float t = hsv.z*(1-hsv.y*(1-f));
	switch(i)
	{
	case 0: return vec3(hsv.z, t, p);
	case 1: return vec3(q, hsv.z, p);
	case 2: return vec3(p, hsv.z, t);
	case 3: return vec3(p, q, hsv.z);
	case 4: return vec3(t, p, hsv.z);
	case 5: return vec3(hsv.z, p, q);
	}
	return vec3(0, 0, 0);
}




// -------------------------------- Spline --------------------------------


void Spline::Point::Serialize(TreeFileRef tf)
{
	tf.SerFloat("time",time,0);
	tf.SerFloat("value",value,0);
}

void Spline::Serialize(TreeFileRef tf)
{
	tf.SerVector("point",pts);
	tf.SerBool("is_loop",is_loop,false);

	if(tf.IsReading())
		Prepare();
}

void Spline::Prepare()
{
	pts_work = pts;

	if(pts_work.size()<1)
	{
		Point p;
		p.time = .5f;
		p.value = 1;
		pts_work.push_back(p);
	}

	if(pts_work.size()<2)
	{
		pts_work[0].time = 0;
		pts_work.push_back(pts_work[0]);
		pts_work[1].time = .5f;
	}

	sort(pts_work.begin(),pts_work.end());

	if(is_loop)
	{
		Point p = pts_work.front();
		p.time += 1;
		pts_work.push_back(p);

		p = pts_work.back();
		p.time -= 1;
		pts_work.insert(pts_work.begin(),p);
	}

	for(int i=0;i<pts_work.size();i++)
	{
		int ia = i-1, ib = i+1;
		if(ia<0) ia = 0;
		if(ib>=pts_work.size()) ib = pts_work.size()-1;
		Point &a = pts_work[ia];
		Point &b = pts_work[i];
		Point &c = pts_work[ib];
		if(c.time==a.time)
			b.derivate = 0;
		else
			b.derivate = (c.value - a.value)/(c.time - a.time);
	}

	if(!is_loop)
	{
		if(pts_work.front().time>0) pts_work.front().derivate = 0;
		if(pts_work.back().time<1) pts_work.back().derivate = 0;

		Point p = pts_work.back();
		p.time += 1;
		pts_work.push_back(p);

		p = pts_work.front();
		p.time -= 1;
		pts_work.insert(pts_work.begin(),p);
	}
}

float Spline::Query(float t)
{
	if(pts_work.size()<2)
		return 0;

	if(t<0) t = 0;
	if(t>0.9999f) t = 0.9999f;

    // binary search
    int a=-1, b=pts_work.size()-1, m;
	while(a<b)
	{
		m=(a+b+1)/2;
		if(t<pts_work[m].time)	b = m-1;
		else					a = m;
	}
	b = a+1;

	float dt = pts_work[b].time - pts_work[a].time;
	if(dt<=0) dt = 1;
	t = (t-pts_work[a].time)/dt;

    float v0 = pts_work[a].value;
	float v1 = pts_work[b].value;
	float d0 = pts_work[a].derivate*dt;
	float d1 = pts_work[b].derivate*dt;
	float pa = v0*2 - v1*2 + d0 + d1;
	float pb = v0*-3 + v1*3 - d0*2 - d1;

	// tangent = (pa*(3*t)+pb*2)*t+d0;
	float v = ((pa*t+pb)*t+d0)*t+v0;
	if(v<0) v = 0;
	if(v>1) v = 1;
	return v;
}



// -------------------------------- MenuService --------------------------------


void MenuService::Begin(int x1,int y1,int x2,int y2)
{
	LoadFrames();

	menu_x1 = x1;
	menu_y1 = y1;
	menu_x2 = x2;
	menu_y2 = y2;
	menu_ypos = y1 + menu_offset;

	vec2 ss = Dev.GetScreenSizeV();
	menu_canvas.SetView(ss*.5f,ss.y);

    is_panel = false;
}

void MenuService::End()
{
    EndPanel();

    mt.SenseScavenge(this,vec2(menu_x1,menu_y1),vec2(menu_x2,menu_y2));
	menu_offset += mt.SenseDelta(this,vec2(menu_x1,menu_y1),vec2(menu_x2,menu_y2)).y;

	menu_canvas.Flush();
	Dev.SetRState(0);
}

void MenuService::Panel()
{
    EndPanel();

    is_panel = true;
	panel_start = menu_ypos;

	layout_row_x1 = menu_x1 + PANEL_PADDING;
	layout_row_x2 = menu_x2 - PANEL_PADDING;
	layout_row_height = DEF_LINE_HEIGHT;
	menu_ypos += PANEL_PADDING;

	NewLine();
}

void MenuService::EndPanel()
{
    if(!is_panel) return;
	NewLine();

	menu_ypos += PANEL_PADDING;
	Quad(0,MENU_LAYER_PANEL,menu_x1,panel_start,menu_x2-menu_x1,menu_ypos-panel_start,0xFF303038);
	Quad(1,MENU_LAYER_PANEL,menu_x1,panel_start,menu_x2-menu_x1,menu_ypos-panel_start,-1);

    is_panel = false;
	menu_ypos += PANEL_PADDING/2;
}

void MenuService::LineSetup(int *sizes)
{
	static int SINGLE[2]={-1,0};
	if(!sizes)
		sizes = SINGLE;

	if(cur_layout_stop>0)
	{
		menu_ypos += layout_row_height;
		layout_row_height = DEF_LINE_HEIGHT;
	}

	int msum=0;
	int psum=0;
	for(int i=0;i<MAX_LAYOUT_STOPS-1 && sizes[i];i++)
		if(sizes[i]>0)	psum +=  sizes[i];
		else			msum += -sizes[i];

	int ww = layout_row_x2 - layout_row_x1;
	ww -= psum;

	num_layout_stops = 1;
	layout_stops[0] = layout_row_x1;
	for(int i=0;num_layout_stops<MAX_LAYOUT_STOPS && sizes[i];i++)
	{
		int w = 0;
		if(sizes[i]>0) w = sizes[i];
		else
		{
			w = (ww*-sizes[i]+msum/2)/msum;
            ww -= w;
			msum -= -sizes[i];
		}
		layout_stops[num_layout_stops] = layout_stops[num_layout_stops-1] + w;
		num_layout_stops++;
	}

	cur_layout_stop = 0;
	Group(1);
}

void MenuService::LineHeight(int percent)
{
	int dlh = (DEF_LINE_HEIGHT*percent+50)/100 - layout_row_height;
	//if(dlh<=0) return;

	layout_row_height += dlh;
}

int MenuService::ControlBox(int gfx,int layer,float gp,float gh,DWORD c1,DWORD c2)
{
	bool g_first = (group_pos==0);
	bool g_last = (group_pos+1>=group_size);
	group_pos++;
	if(g_last) Group(1);

	int x1 = ctrl_x+3;
	int x2 = ctrl_x+ctrl_w-6;
	int xx = 0;
	int dxp = 0;
	if(!g_first) x1 = -10000;			else dxp += 1;
	if(!g_last ) x2 =  10000, xx = 1;	else dxp -= 2;

	QuadGrad(gfx,layer,x1, ctrl_y+3, x2-x1-xx, ctrl_h-6, gp, gh, c1, c2,ctrl_bmin.x,ctrl_bmax.x-xx);
	if(xx) QuadGrad(gfx,layer,x1, ctrl_y+3, x2-x1, ctrl_h-6, gp, gh, 0xFF000000, 0xFF000000,ctrl_bmax.x-1,ctrl_bmax.x);
	QuadGrad(gfx+1,layer,x1, ctrl_y+3, x2-x1, ctrl_h-6, gp, gh, -1, -1,ctrl_bmin.x,ctrl_bmax.x-xx);

    return dxp;
}

void MenuService::Label(const char *text,int flags)
{
    BeginControl();

    int dxp = 0;
    if(flags&1) dxp = ControlBox(2,MENU_LAYER_BUTTONS,.5f,1,0x303035,0x303035);

	Print(text,ctrl_x+ctrl_w/2+dxp,ctrl_y+ctrl_h/2,0xFFFFFFFF);
}

bool MenuService::Button(void *user,const char *text,int flags,int style)
{
	static int _def_button;
	if(!user) user = &_def_button;
    BeginControl();
	DWORD *cs = color_set[style];
	DWORD c1=cs[0], c2=cs[1];
	bool hold  = mt.SenseHalfButton(user,ctrl_bmin,ctrl_bmax);
	bool press = mt.SenseFullButton(user,ctrl_bmin,ctrl_bmax);
	float gp=.5f, gh=.9f;
	if(hold)  c1=cs[2], c2=cs[3];
	if(press) c1=cs[2], c2=cs[3];
	if(flags&1) gp=.7f, gh=-.7f, c1=(c1>>1)&0x7F7F7F, c2=(c2>>1)&0x7F7F7F;

    int dxp = ControlBox(0,MENU_LAYER_BUTTONS,gp,gh,c1,c2);
    PrintActiveControl(text,ctrl_x+ctrl_w/2+dxp,ctrl_y+ctrl_h/2,0xFFFFFFFF);
    return press;
}

bool MenuService::Slider(const char *hint,int &value,int vmin,int vmax,int flags)
{
    BeginControl();
	int _pv = value;
	int minx = ctrl_x+3;
	int maxx = ctrl_x+ctrl_w-3;
	int mrg = 2;
	int xx = minx+mrg + (maxx-minx-mrg-mrg)*(value-vmin)/(vmax-vmin);

	QuadGrad(0,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFF80FF80, 0xFF008000, -1e8, xx);
	QuadGrad(0,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFF000000, 0xFF000000, xx, xx+1);
	QuadGrad(2,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFF202020, 0xFF404040, xx+1, 1e8);
//	Quad(1,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, -1);
	QuadGrad(1,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFFFFFFFF, 0xFFFFFFFF, -1e8, xx+1);
	QuadGrad(3,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFFFFFFFF, 0xFFFFFFFF, xx+1, 1e8);
    if(hint) PrintActiveControl(hint,ctrl_x+ctrl_w/2,ctrl_y+ctrl_h/2,0xFFFFFFFF);
	
	int _value = value;
	float _p = float(value-vmin)/(vmax-vmin);
	float p = mt.SenseSliderX(&value,vec2(minx+mrg,ctrl_bmin.y),vec2(maxx-mrg,ctrl_bmax.y),_p);
	if(p!=_p)
	{
		value = floor( p*(vmax-vmin) + vmin + .5f );
		if(value>vmax) value = vmax;
		if(value<vmin) value = vmin;
	}
	return (value!=_value);
}

bool MenuService::SliderF(const char *hint,float &value,float vminf,float vmaxf,int dec,int flags)
{
    BeginControl();
    int prec = 1;
    for(int i=0;i<dec;i++) prec*=10;
	int _pv = int(floor(value*prec+.5f));
    int vmin = int(ceil(vminf*prec));
    int vmax = int(floor(vmaxf*prec));
	int minx = ctrl_x+3;
	int maxx = ctrl_x+ctrl_w-3;
	int mrg = 2;
	int xx = minx+mrg + (maxx-minx-mrg-mrg)*(_pv-vmin)/(vmax-vmin);

	if( (flags&2) == 0 )
	{
		QuadGrad(0,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFF80FF80, 0xFF004000, -1e8, xx);
		QuadGrad(0,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFF000000, 0xFF000000, xx, xx+1);
		QuadGrad(2,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFF202020, 0xFF404040, xx+1, 1e8);
	//	Quad(1,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, -1);
		QuadGrad(1,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFFFFFFFF, 0xFFFFFFFF, -1e8, xx+1);
		QuadGrad(3,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFFFFFFFF, 0xFFFFFFFF, xx+1, 1e8);
	}
	else
	{
		QuadGrad(2,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFF202020, 0xFF404040, -1e8, xx);
		QuadGrad(0,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFF000000, 0xFF000000, xx, xx+1);
		QuadGrad(0,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFF80FF80, 0xFF004000, xx+1, 1e8);
		QuadGrad(3,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFFFFFFFF, 0xFFFFFFFF, -1e8, xx);
		QuadGrad(1,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFFFFFFFF, 0xFFFFFFFF, xx, 1e8);
	}
    if(hint) PrintActiveControl(hint,ctrl_x+ctrl_w/2,ctrl_y+ctrl_h/2,0xFFFFFFFF);
	
	float _value = value;
	float _p = float(value-vminf)/(vmaxf-vminf);
	float p = mt.SenseSliderX(&value,vec2(minx+mrg,ctrl_bmin.y),vec2(maxx-mrg,ctrl_bmax.y),_p);
	if(p!=_p)
	{
		value = floor( (p*(vmaxf-vminf) + vminf)*prec + .5f )/prec;
		if(value>vmaxf) value = vmaxf;
		if(value<vminf) value = vminf;
	}
	return (value!=_value);
}

bool MenuService::SliderRangeF(const char *hint,float &rmin,float &rmax,float vminf,float vmaxf,int dec,int flags)
{
    BeginControl();
    int prec = 1;
    for(int i=0;i<dec;i++) prec*=10;
	int _pv1 = int(floor(rmin*prec+.5f));
	int _pv2 = int(floor(rmax*prec+.5f));
    int vmin = int(ceil(vminf*prec));
    int vmax = int(floor(vmaxf*prec));
	int minx = ctrl_x+3;
	int maxx = ctrl_x+ctrl_w-3;
	int mrg = 2;
	int xx1 = minx+mrg + (maxx-minx-mrg-mrg)*(_pv1-vmin)/(vmax-vmin);
	int xx2 = minx+mrg + (maxx-minx-mrg-mrg)*(_pv2-vmin)/(vmax-vmin);

	QuadGrad(2,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFF202020, 0xFF404040, -1e8, xx1);
	QuadGrad(0,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFF000000, 0xFF000000, xx1, xx1+1);
	QuadGrad(0,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFF80FF80, 0xFF004000, xx1+1, xx2);
	QuadGrad(0,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFF000000, 0xFF000000, xx2, xx2+1);
	QuadGrad(2,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFF202020, 0xFF404040, xx2+1, 1e8);
	QuadGrad(3,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFFFFFFFF, 0xFFFFFFFF, -1e8, xx1);
	QuadGrad(1,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFFFFFFFF, 0xFFFFFFFF, xx1, xx2+1);
	QuadGrad(3,MENU_LAYER_BUTTONS,ctrl_x+3, ctrl_y+3, ctrl_w-6, ctrl_h-6, .5f, .9f, 0xFFFFFFFF, 0xFFFFFFFF, xx2+1, 1e8);

    if(hint) PrintActiveControl(hint,ctrl_x+ctrl_w/2,ctrl_y+ctrl_h/2,0xFFFFFFFF);
	
	float _value1 = rmin;
	float _value2 = rmax;
	float _p1 = float(rmin-vminf)/(vmaxf-vminf);
	float _p2 = float(rmax-vminf)/(vmaxf-vminf);
	float p = mt.SenseSliderX(&rmin,vec2(minx+mrg,ctrl_bmin.y),vec2(maxx-mrg,ctrl_bmax.y),-1);
	if(p>=0 && (p!=_p1 || p!=_p2))
	{
		float &value = *(p<=_p1 || (p<_p2 && fabs(p-_p1)<=fabs(p-_p2)) ? &rmin : &rmax);
		value = floor( (p*(vmaxf-vminf) + vminf)*prec + .5f )/prec;
		if(value>vmaxf) value = vmaxf;
		if(value<vminf) value = vminf;
		if(&value==&rmin && rmin>rmax) rmax = rmin;
		if(&value==&rmax && rmin>rmax) rmin = rmax;
	}
	return (rmin!=_value1 || rmax!=_value2);
}

bool MenuService::EditBox(string &target, int flags)
{
	BeginControl();

	// window
	int dxp = ControlBox(2, MENU_LAYER_BUTTONS, .5f, 1, 0x08080C, 0x18181C);
	if( &target==edit_target )
		PrintActiveControl((edit_work_copy+"|").c_str(), ctrl_x+ctrl_w/2+dxp, ctrl_y+ctrl_h/2, 0xFF00FF00);
	else
		PrintActiveControl(target.c_str(), ctrl_x+ctrl_w/2+dxp, ctrl_y+ctrl_h/2, 0xFFFFFFFF);

	if( mt.SenseFullButton(&target, ctrl_bmin, ctrl_bmax) )
	{
		edit_target = &target;
		edit_keepalive = false;
		edit_work_copy = target;
	}

	if( &target == edit_target )
	{
		edit_keepalive = true;

		if( edit_commit )
		{
			target = edit_work_copy;

			edit_target = NULL;
			edit_keepalive = false;
			edit_commit = false;
			edit_work_copy.clear();
			return true;
		}
	}

	return false;
}


bool MenuService::SplineEditor(const char *hint,Spline &spl,DWORD color)
{
    BeginControl();

	// window
    int dxp = ControlBox(2,MENU_LAYER_BUTTONS,.5f,1,0x08080C,0x18181C);
	if(hint) Print(hint,ctrl_x+ctrl_w/2+dxp,ctrl_y+DEF_LINE_HEIGHT/3,0xFFFFFFFF);

	vec2 bmin = ctrl_bmin+vec2(10,10);
	vec2 bmax = ctrl_bmax-vec2(10,10);
	if(hint) bmin.y += DEF_LINE_HEIGHT/3;
	win_bmin = bmin;
	win_bmax = bmax;

	vec2 bsize = bmax-bmin;


	// -------- draw spline --------
	{
		menu_canvas.SelectActive(MENU_LAYER_DRAWINGS,"");
		vector<DevCanvas::Vertex> &vb = *menu_canvas.GetActiveBuffer();

		DWORD col = color | 0xFF000000;

		DevCanvas::Vertex v;
		v.color = col;
		v.tc = vec2(0,0);
		v.z = 0;

		vec2 _p;
		for(int i=0;i<=200;i++)
		{
			vec2 p;
			p.x = float(i)/200;
			p.y = 1-spl.Query(p.x);
			p.scale_xy(bsize);
			p += bmin;

			if(i==0)
			{
				_p = p;
				continue;
			}

			float dd = p.x>_p.x ? (p.y-_p.y)/(p.x-_p.x) : 0;
			dd = sqrt(1+dd*dd);

			v.pos.x = _p.x;		v.pos.y = _p.y - dd;	v.color=col&0xFFFFFF;		vb.push_back(v);
			v.pos.x =  p.x;		v.pos.y =  p.y - dd;	v.color=col&0xFFFFFF;		vb.push_back(v);
			v.pos.x =  p.x;		v.pos.y =  p.y;			v.color=col;				vb.push_back(v);
			v.pos.x = _p.x;		v.pos.y = _p.y;			v.color=col;				vb.push_back(v);

			v.pos.x = _p.x;		v.pos.y = _p.y;			v.color=col;				vb.push_back(v);
			v.pos.x =  p.x;		v.pos.y =  p.y;			v.color=col;				vb.push_back(v);
			v.pos.x =  p.x;		v.pos.y =  p.y + dd;	v.color=col&0xFFFFFF;		vb.push_back(v);
			v.pos.x = _p.x;		v.pos.y = _p.y + dd;	v.color=col&0xFFFFFF;		vb.push_back(v);

			_p = p;
		}
	}


	// -------- edit spline --------
	vec2 size(3.5f,3.5f);
	float click_fudge = 2;
	bool mod = false;
	bool active = false;

	for(int i=0;i<spl.pts.size();i++)
	{
		Spline::Point &p = spl.pts[i];

		vec2 s(p.time,1-p.value);
		s.scale_xy(bsize);
		s += bmin;

		mt.SenseClick(&p,s-size*click_fudge,s+size*click_fudge);
		if(mt.SenseActive(&p,s,s))
		{
			Spline::Point _p = p;
			vec2 ss = mt.SenseDragPos(&p,bmin,bmax);
			p.time = ss.x;
			p.value = 1-ss.y;

			if(p.time<0) p.time = 0;
			if(p.time>1) p.time = 1;
			if(p.value<0) p.value = 0;
			if(p.value>1) p.value = 1;

			if(p.time!=_p.time || p.value!=_p.value)
				mod = true;

			menu_canvas.Draw(MENU_LAYER_DRAWINGS,"")()(color|0x00FFFFFF)(vec2(s.x,s.y),size.x,.125f);
			active = true;
		}
		else
			menu_canvas.Draw(MENU_LAYER_DRAWINGS,"")()(color)(vec2(s.x,s.y),size.x,.125f);
	}

	if(!active)
	{
		float ex = size.x/bsize.x;
		float ey = size.y/bsize.y;

		for(int i=0;i<spl.pts.size();i++)
			for(int j=i+1;j<spl.pts.size();j++)
			{
				if(fabs(spl.pts[i].time-spl.pts[j].time)<ex && fabs(spl.pts[i].value-spl.pts[j].value)<ey)
				{
					spl.pts[i].time = (spl.pts[i].time+spl.pts[j].time)*.5f;
					spl.pts[i].value = (spl.pts[i].value+spl.pts[j].value)*.5f;
					spl.pts.erase(spl.pts.begin()+j);
					j = i+1;
					mod = true;
				}
			}
	}

	if(mt.SenseClick(this,bmin,bmax))
	{
		vec2 pp = mt.SenseLocalPos(this,bmin,bmax);
		Spline::Point p;
		p.time = pp.x;
		p.value = 1-pp.y;
		spl.pts.push_back(p);
		
		mt.TrasnferOwnership(this,&spl.pts[spl.pts.size()-1]);
		mod = true;
	}

	if(mod)
		spl.Prepare();

	return mod;
}

bool MenuService::TextureSelector(const char *hint,DevTexture &tex,string &tex_path)
{
    BeginControl();

	// window
    int dxp = ControlBox(2,MENU_LAYER_BUTTONS,.5f,1,0x08080C,0x18181C);
	if(hint) Print(hint,ctrl_x+ctrl_w/2+dxp,ctrl_y+DEF_LINE_HEIGHT/3,0xFFFFFFFF);

	vec2 bmin = ctrl_bmin+vec2(10,10);
	vec2 bmax = ctrl_bmax-vec2(10,10);
	if(hint) bmin.y += DEF_LINE_HEIGHT/3;
	win_bmin = bmin;
	win_bmax = bmax;

	vec2 bsize = bmax-bmin;

	// draw texture
	vec2 tsize = *(const vec2*)&tex.GetSize2D();
	if(tsize.x<=0 || tsize.y<=0)
		tsize = vec2(1,1);

	vec2 bmid = (bmin+bmax)*.5f;
	float xscale = (bmax.x-bmid.x)/tsize.x;
	float yscale = (bmax.y-bmid.y)/tsize.y;
	float scale = min(xscale,yscale);

	menu_canvas.Draw(MENU_LAYER_DRAWINGS,tex)()()(*(const vec2*)&bmid,vec2(tsize.x*scale,tsize.y*scale));

	// file select dialog
	bool mod = false;
	if(mt.SenseClick(this,bmin,bmax))
		if(get_open_filename(tex_path,"Texture file\0*.bmp;*.dds;*.dib;*.hdr;*.jpg;*.pfm,;*png;*.ppm;*.tga\0","png"))
		{
			tex.Load(tex_path.c_str());
			mod = true;
		}

	return mod;
}

bool MenuService::ColorPicker(const char *hint, vec3 &color)
{
	bool mod = false;
	
	BeginControl();

	// window
	int dxp = ControlBox(2, MENU_LAYER_BUTTONS, .5f, 1, 0x08080C, 0x18181C);
	if( hint ) Print(hint, ctrl_x+ctrl_w/2+dxp, ctrl_y+DEF_LINE_HEIGHT/3, 0xFFFFFFFF);

	vec2 bmin = ctrl_bmin+vec2(5, 5);
	vec2 bmax = ctrl_bmax-vec2(5, 5);
	if( hint ) bmin.y += DEF_LINE_HEIGHT/2.5f;
	win_bmin = bmin;
	win_bmax = bmax;

	// sizes
	const float bar_margin = 5;
	const float region_inset = 5;
	float wheel_size = min(bmax.x-bmin.x, bmax.y-bmin.y);
	float bars_split = ((bmin.x + (wheel_size+bar_margin)) + bmax.x)*.5f;

	// the color
	vec3 hsv = rgb_to_hsv(color);
	vec2 dotpos = vec2(sin(hsv.x*(2*M_PI)), -cos(hsv.x*(2*M_PI)))*hsv.y;

	// color wheel
	{
		vec2 tmin = bmin;
		vec2 tmax = bmin + vec2(1,1)*wheel_size;
		vec2 tmid = (tmin+tmax)*.5f;
		menu_canvas.Draw(MENU_LAYER_DRAWINGS, menu_colorcircle)()()(tmid, wheel_size*.5f);
		menu_canvas.Draw(MENU_LAYER_FONTS, menu_colordot)()()(tmid+dotpos*(wheel_size*(1-6/80.f)*.5f), 5);

		vec2 lpos = mt.SenseLocalPos(&color.x, tmin, tmax, vec2(-20000, -20000));
		if( lpos.x>-20000 )
		{
			lpos = (lpos-.5f)*2;
			hsv.y = lpos.length();
			if( hsv.y>1 ) hsv.y = 1;
			if( hsv.y>0 )
				hsv.x = (atan2(-lpos.x, lpos.y)/3.141593+1)/2;
			else
				hsv.x = 0;
			mod = true;
		}
	}

	// color value bar
	{
		vec2 tmin = vec2(bmin.x+wheel_size+bar_margin, bmin.y);
		vec2 tmax = vec2((tmin.x+bmax.x)*.5f, bmax.y);
		float yy = (tmax.y-1) + (tmin.y-(tmax.y-1))*hsv.z;
		menu_canvas.SelectActive(MENU_LAYER_DRAWINGS, "");
		DWORD c = hsv_to_rgb(vec3(hsv.x, hsv.y, 1)).make_rgb() | 0xFF000000;
		menu_canvas.PushGradientQuad(tmin+vec2(5, 0), tmax-vec2(5, 0), vec2(0, 0), vec2(1, 1), c, c, 0xFF000000, 0xFF000000);
		menu_canvas.PushGradientQuad(vec2(tmin.x, yy), vec2(tmax.x, yy+1), vec2(0, 0), vec2(1, 1), -1, -1, -1, -1);

		float lpos = mt.SenseLocalPos(&color.y, tmin, tmax, vec2(-20000, -20000)).y;
		if( lpos>-20000 )
		{
			hsv.z = 1-lpos;
			if( hsv.z<0 ) hsv.z = 0;
			if( hsv.z>1 ) hsv.z = 1;
			mod = true;
		}
	}

	// color region
	{
		vec2 tmin = vec2(bars_split, bmin.y);
		vec2 tmax = bmax;
		tmin += region_inset;
		tmax -= region_inset;
		menu_canvas.SelectActive(MENU_LAYER_DRAWINGS, "");
		DWORD c = color.make_rgb() | 0xFF000000;
		menu_canvas.PushGradientQuad(tmin, tmax, vec2(0, 0), vec2(1, 1), c, c, c, c);
	}

	if( mod )
		color = hsv_to_rgb(hsv);

	return mod;
}


void MenuService::ListStart()
{
	list_y1 = -1;
}

bool MenuService::ListItem(const char *text, DWORD color, bool active)
{
	static int _def_button;
	void *user = &_def_button;
	LineSetup(-1);
	LineHeight(70);
	BeginControl();

	if( list_y1<0 )
		list_y1 = ctrl_y;

	DWORD c1=color;
	bool hold  = mt.SenseHalfButton(user, ctrl_bmin, ctrl_bmax);
	bool press = mt.SenseFullButton(user, ctrl_bmin, ctrl_bmax);
	float gp=.5f, gh=.9f;
	//if( hold )  c1=cs[2], c2=cs[3];
	//if( press ) c1=cs[2], c2=cs[3];
	//if( flags&1 ) gp=.7f, gh=-.7f, c1=(c1>>1)&0x7F7F7F, c2=(c2>>1)&0x7F7F7F;

	//int dxp = ControlBox(0, MENU_LAYER_BUTTONS, gp, gh, c1, c2);
	if( active )
		Quad(-1, MENU_LAYER_DRAWINGS, ctrl_x+5, ctrl_y, ctrl_w-12, ctrl_h, 0x40000000 | (c1&0x00FFFFFF) );
	PrintActiveControl(text, ctrl_x+15, ctrl_y+ctrl_h/2+1, 0xFF000000 | c1, 0x01, 1.f);
	return press;
}

void MenuService::ListEnd()
{
	if( list_y1<0 )
		return;

	ctrl_y = list_y1-5;
	ctrl_bmin.y = float(list_y1);
	ctrl_h = int(ctrl_bmax.y-ctrl_bmin.y)+10;

	ControlBox(2, MENU_LAYER_BUTTONS, .5f, 1, 0x08080C, 0x18181C);

	list_y1 = -1;
}


void MenuService::Group(int count)
{
	if(count>num_layout_stops-cur_layout_stop-1)
		count = num_layout_stops-cur_layout_stop-1;
	if(count<1) count = 1;

	group_size = count;
	group_pos = 0;

	for(int i=1;i<count;i++)
		layout_stops[cur_layout_stop+i] += 3 - (6*i + count/2)/count;
}

void MenuService::OnFrameStart()
{
	edit_commit = false;
	if( edit_target )
	{
		// run text editor
		int key;
		while( (key = Dev.ReadKey()) )
		{
			if(key<0)
			{
				key = ~key;
				if( key=='\b' && edit_work_copy.size()>0 )
				{
					edit_work_copy.pop_back();
				}
				else if( key>=' ' && key<128 )
					edit_work_copy.push_back(key);
			}
			else
			{
				if( key==VK_RETURN )
				{
					edit_commit = true;
				}
				else if( key==VK_ESCAPE || key==VK_LBUTTON || key==VK_RBUTTON || key==VK_MBUTTON )
				{
					edit_target = NULL;
				}
			}
		}
	}
	
	edit_keepalive = false;
}

void MenuService::OnFrameEnd()
{
	if( !edit_keepalive )
		edit_target = NULL;
}



void MenuService::BeginControl()
{
	if(!is_panel) return;

	if(cur_layout_stop+1>=num_layout_stops)
		NewLine();

	ctrl_x = layout_stops[cur_layout_stop];
	ctrl_y = menu_ypos;
    ctrl_w = layout_stops[cur_layout_stop+1] - layout_stops[cur_layout_stop];
    ctrl_h = layout_row_height;
	ctrl_bmin.x = ctrl_x;
	ctrl_bmin.y = ctrl_y;
	ctrl_bmax.x = ctrl_bmin.x + ctrl_w;
	ctrl_bmax.y = ctrl_bmin.y + ctrl_h;

	cur_layout_stop++;
}

void MenuService::Print(const char *text,int x,int y,DWORD col)
{
	menu_font.DrawText(menu_canvas,MENU_LAYER_FONTS,x,y+1,0x11,.45f,col|0xFF000000,text);
}

void MenuService::PrintActiveControl(const char *text,int x,int y,DWORD col, int align, float smul_override)
{
	vec2 bmin = ctrl_bmin + vec2(16,16);
	vec2 bmax = ctrl_bmax - vec2(16,16);
	if(bmin.x>bmax.x) bmin.x = bmax.x = (bmin.x+bmax.x)*.5f;
	if(bmin.y>bmax.y) bmin.y = bmax.y = (bmin.y+bmax.y)*.5f;

	vec2 mpos = *(const vec2*)&Dev.GetMousePosV();
	vec2 hit = mpos;
	hit.x = max(bmin.x,min(bmax.x,hit.x));
	hit.y = max(bmin.y,min(bmax.y,hit.y));

	float dist = (hit - mpos).length2();
	float smul = 1 + 15/(dist+1);
	if( smul>1.25f ) smul = 1.25f;
	if( smul_override >=0 ) smul = smul_override;

	menu_font.DrawText(menu_canvas, MENU_LAYER_FONTS, x, y+1, align, .45f*smul, col|0xFF000000, text);
}

void MenuService::Quad(int fid, int layer, float x, float y, float w, float h, DWORD col)
{
	if( fid<0 || fid>=frames.size() )
	{
		menu_canvas.SelectActive(layer, "");
		menu_canvas.PushGradientQuad(
			vec2(x, y),
			vec2(x+w, y+h),
			vec2(0, 0),
			vec2(1, 1),
			col, col, col, col, -1);
		return;
	}

	FramInfo &f = frames[fid];

	float xp[4]  = {x+f.dmin.x,x+f.din1.x,x+w+f.din2.x,x+w+f.dmax.x};
	float yp[4]  = {y+f.dmin.y,y+f.din1.y,y+h+f.din2.y,y+h+f.dmax.y};
	float xtc[4] = {f.bmin.x,f.split.x,f.split.x,f.bmax.x};
	float ytc[4] = {f.bmin.y,f.split.y,f.split.y,f.bmax.y};
	
	menu_canvas.SelectActive(layer, menu_tex);

	for(int i=0;i<9;i++)
	{
		int xx = i%3;
		int yy = i/3;
		menu_canvas.PushGradientQuad(
			vec2(xp[xx  ],yp[yy  ]),
			vec2(xp[xx+1],yp[yy+1]),
			vec2(xtc[xx  ],ytc[yy  ]),
			vec2(xtc[xx+1],ytc[yy+1]),
			col, col, col, col, -1 );
	}
}

void MenuService::QuadGrad(int fid,int layer,float x,float y,float w,float h,float gpos,float gw,DWORD c1,DWORD c2,float xmin,float xmax)
{
	if(fid<0 || fid>=frames.size()) return;
	FramInfo &f = frames[fid];
	
	float xp[4]  = {x+f.dmin.x,x+f.din1.x,x+w+f.din2.x,x+w+f.dmax.x};
	float xtc[4] = {f.bmin.x,f.split.x,f.split.x,f.bmax.x};
	float yp[6]  = {y+f.dmin.y,y+f.din1.y,y+h+f.din2.y,y+h+f.dmax.y};
	float ytc[6] = {f.bmin.y,f.split.y,f.split.y,f.bmax.y};
	int ny = 4;

	float c1y = (gpos-gw*.5f)*h+y;
	float c2y = (gpos+gw*.5f)*h+y;

	float yx = c1y;
	for(int i=0;i<ny-1;i++)
		if(yp[i]<yx && yx<yp[i+1])
		{
			for(int j=ny-1;j>i;j--)
				yp[j+1]=yp[j], ytc[j+1]=ytc[j];
			yp[i+1] = yx;
			ytc[i+1] = ytc[i] + (ytc[i+2]-ytc[i])*( (yx-yp[i])/(yp[i+2]-yp[i]) );
			ny++;
			break;
		}

	yx = c2y;
	for(int i=0;i<ny-1;i++)
		if(yp[i]<yx && yx<yp[i+1])
		{
			for(int j=ny-1;j>i;j--)
				yp[j+1]=yp[j], ytc[j+1]=ytc[j];
			yp[i+1] = yx;
			ytc[i+1] = ytc[i] + (ytc[i+2]-ytc[i])*( (yx-yp[i])/(yp[i+2]-yp[i]) );
			ny++;
			break;
		}

	DWORD cc[6];
	vec3 c1v = vec3::from_rgb(c1);
	vec3 cdv = vec3::from_rgb(c2) - c1v;
	for(int i=0;i<ny;i++)
	{
		float p = (yp[i]-c1y)/(c2y-c1y);
		if(p<0) p = 0;
		if(p>1) p = 1;
		cc[i] = (c1v + cdv*p).make_rgba(1);
	}

	menu_canvas.SelectActive(layer,menu_tex);

	for(int i=0;i<3*(ny-1);i++)
	{
		int xx = i%3;
		int yy = i/3;
		float x1 = xp[xx];
		float x2 = xp[xx+1];
		if(x1<xmin) x1=xmin;
		if(x2>xmax) x2=xmax;
		if(x1>=x2) continue;

		float tx1 = xtc[xx]+(xtc[xx+1]-xtc[xx])*(x1-xp[xx])/(xp[xx+1]-xp[xx]);
		float tx2 = xtc[xx]+(xtc[xx+1]-xtc[xx])*(x2-xp[xx])/(xp[xx+1]-xp[xx]);
		
		menu_canvas.PushGradientQuad(
						vec2(x1,yp[yy  ]),
						vec2(x2,yp[yy+1]),
						vec2(tx1,ytc[yy  ]),
						vec2(tx2,ytc[yy+1]),
						cc[yy], cc[yy], cc[yy+1], cc[yy+1], -1 );
	}
}


void MenuService::LoadFrames()
{
	if(load_done) return;
	load_done = true;

	vector<string> lines;
	if(!NFS.GetFileLines("app_data/uiskin.info",lines))
		return;

	vec2 tex_size = menu_tex.GetSize2D();
	if(tex_size.x<=0 || tex_size.y<=0) return;

	for(int i=0;i<lines.size();i++)
	{
		int     w=0, h=0;					// gfx size
		int     atx=0, aty=0;				// gfx atlas pos
		int     bx1=0, by1=0, bx2=0, by2=0; // frame position within gfx
		int     cutx=0, cuty=0;				// pixel row/col within gfx to stretch
		const char *s = lines[i].c_str();
		string cmd = ParseString(s);
		if(cmd=="f")
		{
			w = ParseInt(s);
			h = ParseInt(s);
			atx = ParseInt(s);
			aty = ParseInt(s);
			bx1 = ParseInt(s);
			by1 = ParseInt(s);
			bx2 = ParseInt(s);
			by2 = ParseInt(s);
			cutx = ParseInt(s);
			cuty = ParseInt(s);

			FramInfo f;
			f.bmin.x = float(atx)/tex_size.x;
			f.bmin.y = float(aty)/tex_size.y;
			f.bmax.x = float(atx+w)/tex_size.x;
			f.bmax.y = float(aty+h)/tex_size.y;
			f.split.x = float(atx+cutx)/tex_size.x;
			f.split.y = float(aty+cuty)/tex_size.y;

			f.dmin.x = -bx1;
			f.dmin.y = -by1;
			f.dmax.x = w-bx2;
			f.dmax.y = h-by2;
			f.din1.x = cutx-bx1;
			f.din1.y = cuty-by1;
			f.din2.x = cutx-bx2;
			f.din2.y = cuty-by2;

			frames.push_back(f);
		}
	}
}
