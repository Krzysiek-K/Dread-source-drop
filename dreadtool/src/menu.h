
#ifndef _MENU_H
#define _MENU_H


#include <dxfw.h>

using namespace std;
using namespace base;



#define MF_ACTIVE           (1<<12)


class mh_t {
public:
	mh_t() : ptr(0), id(0) {}
	mh_t(const mh_t &h) : ptr(h.ptr), id(h.id) {}
	mh_t(const void *p) : ptr(p), id(0) {}
	mh_t(const void *p, int i) : ptr(p), id(i) {}
	mh_t(int i) : ptr(0), id(i) {}

	void operator =(const mh_t &h) { ptr=h.ptr; id=h.id; }

	bool operator ==(const mh_t &h) const { return ptr==h.ptr && id==h.id; }
	bool operator !=(const mh_t &h) const { return ptr!=h.ptr || id!=h.id; }


private:
	const void	*ptr;
	int			id;
};


class Spline {
public:
	struct Point {
        float   time;
        float   value;
		float	derivate;

        bool operator <(const Point &p) const { return time<p.time; }

		void Serialize(TreeFileRef tf);
	};

	Spline() { is_loop = false; Prepare(); }

	vector<Point>	pts;
	vector<Point>	pts_work;
	bool			is_loop;

	void  Serialize(TreeFileRef tf);
    void  Prepare();
    float Query(float t);
};

class MenuList {
public:
	vector<string>	elems;
	int				selected;

	MenuList() : selected(-1) {}
};


class MenuService {
public:

    MenuService() { menu_offset = 0; load_done = false; edit_target=NULL; edit_keepalive=false; edit_commit=false; }

    void Begin(int x1,int y1,int x2,int y2);
    void End();

    void Panel();
    void EndPanel();	// calling this is automatic

	void LineSetup(int *sizes);
	void LineSetup(int s1=-1,int s2=0,int s3=0,int s4=0)							{ int sizes[]={s1,s2,s3,s4,0}; LineSetup(sizes); }
	void LineSetup(int s1,int s2,int s3,int s4,int s5,int s6=0,int s7=0,int s8=0)	{ int sizes[]={s1,s2,s3,s4,s5,s6,s7,s8,0}; LineSetup(sizes); }
	void LineHeight(int percent);

    void Label(const char *text,int flags=0);
	bool Button(void *user,const char *text,int flags=0,int style=0);
	bool EnumButton(void *user,const char *text,int &value,int set_value,int flags=0,int style=0)
	{
		if(!Button(user,text,flags | (value==set_value?1:0),style)) return false;
		value = set_value;
		return true;
	}

	bool RockerSwitch(const char *text, int &value, int vmin, int vmax, int flags=0, int style=0)
	{
		int pv = value;
		if( Button(NULL, "<<", flags, style) && value>vmin ) value--;
		Label(text, flags | 1);
		if( Button(NULL, ">>", flags, style) && value<vmax ) value++;
		if( value>vmax ) value = vmax;
		if( value<vmin ) value = vmin;
		return (value!=pv);
	}
	
	bool Slider(const char *hint,int &value,int vmin,int vmax,int flags=0);
    bool SliderF(const char *hint,float &value,float vmin,float vmax,int dec,int flags=0);
	bool SliderRangeF(const char *hint,float &rmin,float &rmax,float vminf,float vmaxf,int dec,int flags=0);

	bool EditBox(string &target, int flags=0);

	bool SplineEditor(const char *hint,Spline &spl,DWORD color);
	bool TextureSelector(const char *hint, DevTexture &tex, string &tex_path);
	bool ColorPicker(const char *hint, vec3 &color);

	void ListStart();
	bool ListItem(const char *text, DWORD color, bool active);
	void ListEnd();


	void NewLine()			{ LineSetup(-1); }
	void Group(int count=1000000);

	void OnFrameStart();
	void OnFrameEnd();

private:
    enum {
        BM_NONE = 0,
        BM_BUTTON
    };

	enum {
		MAX_LAYOUT_STOPS = 32
	};

	struct FramInfo {
		vec2	bmin;	// min texcoords
		vec2	bmax;	// max texcoords
		vec2	split;	// split texcoords
		vec2	dmin;
		vec2	dmax;
		vec2	din1;
		vec2	din2;
	};

	bool					load_done;
	int						menu_x1, menu_y1, menu_x2, menu_y2;
	int						menu_ypos;
	int						menu_offset;
    bool                    is_panel;
	int						panel_start;
    int                     panel_quad;
	int						layout_row_x1;
	int						layout_row_x2;
	int						layout_row_height;
	int						layout_stops[MAX_LAYOUT_STOPS];
	int						num_layout_stops;
	int						cur_layout_stop;
	int						group_size;
	int						group_pos;
	string					*edit_target;
	bool					edit_keepalive;
	bool					edit_commit;
	string					edit_work_copy;

	int                     ctrl_x, ctrl_y;
    int                     ctrl_w, ctrl_h;
	vec2					ctrl_bmin, ctrl_bmax;
	int						list_y1;
	vec2					win_bmin, win_bmax;
	vector<FramInfo>		frames;
	


	void BeginControl();

    void Print(const char *text,int x,int y,DWORD col);
	void PrintActiveControl(const char *text, int x, int y, DWORD col, int align=0x11, float smul_override=-1);
    int  ControlBox(int gfx,int layer,float gp,float gh,DWORD c1,DWORD c2);

	void Quad(int fid,int layer,float x,float y,float w,float h,DWORD col);
	void QuadGrad(int fid,int layer,float x,float y,float w,float h,float gpos,float gw,DWORD c1,DWORD c2,float xmin=-1e8,float xmax=1e8);

	void LoadFrames();

};

extern MenuService ms;


bool get_open_filename(string &path,char *filter,char *ext);
bool get_save_filename(string &path,char *filter,char *ext);



#endif
