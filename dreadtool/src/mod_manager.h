
#pragma once


class ModManager {
public:
	enum {
		RELOAD_ASSETS	= (1<<0),
		RELOAD_MAP		= (1<<1),
	};

	struct FileFilter {
		string	name;
		bool	open = false;
		bool	done = false;
	};

	struct MapFileInfo {
		string	name;
		string	path;
		DWORD	color_off = 0xFFC0C0C0;
		DWORD	color_on  = 0xFFFFFFFF;
		int		filter_id = -1;
	};

	struct ModDirInfo {
		string	name;
		string	path;
		bool	enabled = false;
		int		filter_id = -1;
	};

	string						core_dir_path;
	string						map_path;
	vector<MapFileInfo>			map_file_list;
	vector<FileFilter>			map_filters;
	vector<ModDirInfo>			mod_dir_list;
	vector<FileFilter>			mod_filters;
	bool						file_lists_loaded = false;

	void Init();
	void Shutdown();
	string GetMapPath();

	bool ConfigListCheck(const char *key, const string &value);
	void ConfigListAdd(const char *key, const string &value);
	void ConfigListRemove(const char *key, const string &value);

	void RefreshMapFilesDir();
	int	 RunFileMenu();

	string	FindAvailableFile(const string &base_path, const char *extension_list);
	bool	PathStartsWith(const string &path, const string &start);
	string	GetResourcePath(const string &path, const char *extension_list=NULL);
};


extern ModManager modmgr;
