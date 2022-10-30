
#include "common.h"
#include "app_rendertest.h"



ModManager modmgr;


void ModManager::Init()
{
	map_path = cfg.GetString("map-path-selected", "");

	core_dir_path = FilePathGetPart(proj_cfg.GetString("assets-definition", "").c_str(), true, false, false);
	if( core_dir_path.size()>0 && (core_dir_path.back()=='/' || core_dir_path.back()=='\\') )
		core_dir_path.pop_back();

	RefreshMapFilesDir();
}

void ModManager::Shutdown()
{
	cfg.SetString("map-path-selected", map_path.c_str());
}

string ModManager::GetMapPath()
{
	if( map_path.size()>0 )
		return map_path;
	return proj_cfg.GetString("map-wad", "map.wad");
}

bool ModManager::ConfigListCheck(const char *key, const string &value)
{
	TreeFileRef tf = cfg.GetNode(key, 0, false);
	for( TreeFileRef::iterator p=tf.begin(); p!=tf.end(); ++p )
		if( value == (*p).GetName() )
			return true;
	return false;
}

void ModManager::ConfigListAdd(const char *key, const string &value)
{
	if( !ConfigListCheck(key, value) )
	{
		int one = 1;
		cfg.GetNode(key, 0, true).SerInt(value.c_str(), one, 0);
	}
}

void ModManager::ConfigListRemove(const char *key, const string &value)
{
	TreeFileRef tf = cfg.GetNode(key, 0, false);
	for( TreeFileRef::iterator p=tf.begin(); p!=tf.end(); ++p )
		if( value == (*p).GetName() )
		{
			tf.DeleteChild(*p);
			break;
		}
}



void ModManager::RefreshMapFilesDir()
{
	vector<string> dir;

	// Load map list
	map_file_list.clear();
	map_filters.clear();
	{
		vector<string> search_dirs;
		proj_cfg.GetStringArray("map-directory", search_dirs);
		proj_cfg.GetStringArray("map-directory-*", search_dirs, true);
		
		for( const string &extdir : search_dirs )
			if( NFS.GetFileList((extdir+"/*.wad").c_str(), dir) && dir.size()>=0 )
			{
				for( int i=0; i<(int)dir.size(); i++ )
				{
					MapFileInfo fi;
					fi.name = dir[i];
					fi.path = extdir + "/" + dir[i];
					fi.color_off = 0xFF40C040;
					fi.color_on  = 0xFF60FF60;
					fi.filter_id = map_filters.size();
					map_file_list.push_back(fi);
				}

				FileFilter mf;
				mf.name = FilePathGetPart(extdir.c_str(), false, true, false)+"/";
				map_filters.push_back(mf);
			}
	}


	// Load mod list
	mod_dir_list.clear();
	mod_filters.clear();
	{
		vector<string> search_dirs;
		proj_cfg.GetStringArray("mod-directory", search_dirs);
		proj_cfg.GetStringArray("mod-directory-*", search_dirs, true);

		for( const string &extdir : search_dirs )
		{
			if( NFS.GetSubdirList((extdir+"/*").c_str(), dir) )
				for( int i=0; i<(int)dir.size(); i++ )
					if( dir[i].size()>0 && dir[i][0]!='.' )
					{
						ModDirInfo md;
						md.name = dir[i];
						md.path = extdir + "/" + dir[i];
						md.enabled = ConfigListCheck("mod-enabled", md.name);
						md.filter_id = mod_filters.size();
						mod_dir_list.push_back(md);

						if( md.enabled )
						{
							vector<string> subdir;
							printf("Querying: %s\n", (md.path+"/*.wad").c_str());
							if( NFS.GetFileList((md.path+"/*.wad").c_str(), subdir) && subdir.size()>0 )
							{
								for( int i=0; i<(int)subdir.size(); i++ )
								{
									MapFileInfo fi;
									fi.name = subdir[i];
									fi.path = md.path + "/" + subdir[i];
									fi.color_off = 0xFFC0FFFF;
									fi.color_on  = 0xFFC0FFFF;
									fi.filter_id = map_filters.size();
									map_file_list.push_back(fi);

									printf("modmap: %s\n", fi.name.c_str());
								}
							}

							FileFilter mf;
							mf.name = md.name;
							map_filters.push_back(mf);
						}
					}

			FileFilter mf;
			mf.name = FilePathGetPart(extdir.c_str(), false, true, false)+"/";
			mod_filters.push_back(mf);
		}
	}

	file_lists_loaded = true;
}


int ModManager::RunFileMenu()
{
	int reload = 0;

	if( !file_lists_loaded )
		RefreshMapFilesDir();

	ms.Label("Map selection");
	ms.ListStart();
	for( int i=0; i<(int)map_filters.size(); i++ )
		map_filters[i].done = false;

	for( int i=0; i<(int)map_file_list.size(); i++ )
	{
		MapFileInfo &fi = map_file_list[i];
		FileFilter *mf = (fi.filter_id>=0) ? &map_filters[fi.filter_id] : NULL;

		if( mf && !mf->done )
		{
			mf->open = ConfigListCheck("map-directory-open", mf->name);
			mf->done = true;

			if( ms.ListItem(format("%c %s", mf->open ? '-' : '+', mf->name.c_str()).c_str(), 0xFFC0C0C0, false) )
			{
				mf->open = !mf->open;

				if( mf->open )
					ConfigListAdd("map-directory-open", mf->name);
				else
					ConfigListRemove("map-directory-open", mf->name);
			}
		}

		if( mf && !mf->open )
			continue;


		bool active = (map_path == fi.path);
		if( ms.ListItem((mf ? ("    "+fi.name).c_str() : fi.name.c_str()), active ? fi.color_on : fi.color_off, active) )
		{
			map_path = fi.path;
			reload |= RELOAD_MAP;
		}
	}
	ms.ListEnd();


	ms.Label("Mod selection");
	ms.ListStart();
	for( int i=0; i<(int)mod_filters.size(); i++ )
		mod_filters[i].done = false;

	for( int i=0; i<(int)mod_dir_list.size(); i++ )
	{
		ModDirInfo &mi = mod_dir_list[i];
		FileFilter *mf = (mi.filter_id>=0) ? &mod_filters[mi.filter_id] : NULL;

		if( mf && !mf->done )
		{
			mf->open = ConfigListCheck("mod-directory-open", mf->name);
			mf->done = true;

			if( ms.ListItem(format("%c %s", mf->open ? '-' : '+', mf->name.c_str()).c_str(), 0xFFC0C0C0, false) )
			{
				mf->open = !mf->open;

				if( mf->open )
					ConfigListAdd("mod-directory-open", mf->name);
				else
					ConfigListRemove("mod-directory-open", mf->name);
			}
		}

		if( mf && !mf->open )
			continue;

		if( ms.ListItem((mf ? ("    "+mi.name).c_str() : mi.name.c_str()), mi.enabled ? 0xFFFFFFFF : 0xFF606060, false) )
		{
			mi.enabled = !mi.enabled;

			if( mi.enabled )
				ConfigListAdd("mod-enabled", mi.name);
			else
				ConfigListRemove("mod-enabled", mi.name);

			RefreshMapFilesDir();

			reload |= RELOAD_MAP | RELOAD_ASSETS;
		}
	}
	ms.ListEnd();

	return reload;
}


string ModManager::FindAvailableFile(const string &base_path, const char *extension_list)
{
	if( GetFileTime(base_path.c_str()) != 0 )
		return base_path;

	if( extension_list )
		while( *extension_list )
		{
			const char *s = extension_list;
			while( *extension_list && *extension_list!=';' )
				extension_list++;

			string path = base_path + string(s, extension_list);
			if( GetFileTime(path.c_str()) != 0 )
				return path;

			if( *extension_list==';' )
				extension_list++;
		}

	return "";
}

bool ModManager::PathStartsWith(const string &path, const string &start)
{
	if( path.size() < start.size() + 1 )
		return false;

	return strncmp(path.c_str(), start.c_str(), start.size())==0 && (path[start.size()]=='/' || path[start.size()]=='\\');
}

string ModManager::GetResourcePath(const string &path, const char *extension_list)
{
	int mod_index = -2;		// -2: not found		-1: core		0+: mod_dir_list[index]
	const char *internal_path = NULL;

	if( PathStartsWith(path, core_dir_path) )
	{
		mod_index = -1;
		internal_path = path.c_str() + core_dir_path.size() + 1;
	}
	else
		for( int i=0; i<(int)mod_dir_list.size(); i++ )
			if( mod_dir_list[i].enabled )
				if( PathStartsWith(path, mod_dir_list[i].path) )
				{
					mod_index = i;
					internal_path = path.c_str() + mod_dir_list[i].path.size() + 1;
					break;
				}

	if( mod_index>=-1 && internal_path )
		for( int i=(int)mod_dir_list.size()-1; i>mod_index; i-- )
			if( mod_dir_list[i].enabled )
			{
				const string &base_dir = *((i>=0) ? &mod_dir_list[i].path : &core_dir_path);
				string file = FindAvailableFile(base_dir+"/"+internal_path, extension_list);
				if( file.size()>0 )
					return file;
			}

	return FindAvailableFile(path, extension_list);
}
