
#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <dxfw.h>

using namespace std;
using namespace base;


#include "mtouch.h"
#include "menu.h"
#include "defparser.h"
#include "errlog.h"

#include "blitter.h"


#define MENU_WIDTH		256


#define muls(a,b)		( (int)((short)(a)) * (short)(b) )
#define mulu(a,b)		( (dword)((word)(a)) * (word)(b) )
#define divs(a,b)		( (short)( (int)(a) / (short)(b) ) )
#define divu(a,b)		( (word)( (dword)(a) / (word)(b) ) )



// app_rendertest.cpp
//byte _color2chunky(DWORD col);			// TBD: move somewhere where it makes sense

class ChunkyColorPacker;




class AppConfig {
public:

	struct Entry {
		string	key;
		string	value;
	};

	vector<Entry>	data;

	bool LoadFile(const char *path);

	int			GetInt(const string &name, int def);
	float		GetFloat(const string &name, float def);
	std::string	GetString(const string &name, const string &def);
	bool		GetStringArray(const string &name, vector<string> &arr, bool append=false);
};





// ---------------- textfile.cpp ----------------

class TextFile {
public:
	vector<string>		lines;
	string				file_path;
	bool				modified = false;
	unsigned long long	timestamp = 0;

	bool Load(const char *path);
	bool Save();
	bool SaveAs(const char *path);

	void Modified() { modified = true; }

	void SaveIfModified();
	bool ReloadIfChanged();
};


// ---------------- func.cpp ----------------

struct BinaryWordData {
	vector<word>		data;
	int					maxlen = 16;	// maximum number of elements in one line
	map<int, bool>		newline;		// newline[i] == true	- start <i>th element from new line
	map<int, string>	comments;		// adds comment after <i>th element and move to new line

	void Clear() {
		data.clear();
		newline.clear();
		comments.clear();
	}

	inline void AddWord(word value)					{ data.push_back(value); }
	inline void AddComment(const string &comment)	{ comments[data.size()] += "\t// " + comment; }
	inline void LineComment(const string &comment)	{ comments[data.size()] += "\n\t\t\t// " + comment; }
	inline void NewLine()							{ newline[data.size()] = true; }

	// returns bytes written
	int WriteTable(FILE *fp,const char *name, bool chipmem);
};


// ---------------- tex.cpp ----------------

class DataTexture {
public:
	DevTexture			tex;
	vector<DWORD>		data;
	int					width;
	int					height;
	string				path;
	string				config_id;
	int					off_x = 0;		// pivot point
	int					off_y = 0;


	void Clear(DWORD col) { data.clear(); data.resize(width*height, col); }
	void Resize(int w, int h) { width=w; height=h; data.clear(); data.resize(w*h,0xFF00FF); }
	void Upload();
	bool Load(string path);
	void CopyFrom(const DataTexture &src);
	void InitConfig(const char *id);
	void Draw(int x0, int y0, int scale);
	bool MenuEdit(const char *desc, int height=450);

	DWORD &operator()(int x, int y) { return data[x+width*y]; }

	// processing
	void Crop(int x1, int y1, int x2, int y2);
	bool IsColumnTransparent(int x);
	bool IsRowTransparent(int y);
	void Trim();
	void Collapse();
	void Downscale();
	void DownscaleY();
	void ColorKey(int aref, DWORD key);
	void OverlayColorKey(DataTexture &tex1, DataTexture &tex2, int x0, int y0, DWORD key);
	void ColorMask(DWORD color, DWORD mask);
	void Scroll(int dx, int dy);

	// dreadtool encoding processing
	void ColorIndexRemap(vector<int> remap);
	bool IsIndexColumnTransparent(int x);
	bool IsIndexRowTransparent(int y);
	void IndexTrim();
	void IndexHorizPack();
	void IndexHorizScroll(bool wrap);

	// Fix modes:
	//	0	- make solid
	//	1	- make transparent
	//	2	- use left side for entire pixel
	//	3	- use right side for entire pixel
	//	4	- pad with black (index 0)
	void FixTransparency(int mode);
	void IndexOverlay(DataTexture &tex, int x0, int y0);



	// export
	void PrepareObjectData(BinaryWordData &bindata, ChunkyColorPacker &chunky_packer);
	word GatherBitplaneWord(int xp, int yp, int bitplane);		// bitplane=-1 - transparency mask
};

// ---------------- main.cpp ----------------

class SubApplication {
public:
	virtual ~SubApplication() {}
	
	virtual const char *GetName() = 0;
	virtual void Init() {}
	virtual void Shutdown() = 0;
	virtual void Run() = 0;
	virtual void RunMenu() = 0;
};

extern DevFont font;

extern Config cfg;
extern AppConfig proj_cfg;


void SetResolution(int w, int h);




// ---- other ----

#include "texscaler.h"
#include "app_texscaler.h"
