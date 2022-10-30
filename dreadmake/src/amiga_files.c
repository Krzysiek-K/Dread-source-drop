
#include "demo.h"


#if ENABLE_MAP_FILE_LOADING



FileInfo files_dir[MAX_FILES_DIR];
word n_files_dir;

char temp_text_buffer[256];

void File_ReadDirFiles(const char *dirpath)
{
	// Read "data" directory contents
	n_files_dir = 0;

	void *dir = Sys_Lock(dirpath);
	if( dir )
	{
		if( Sys_Examine(dir, &file_info_block) )
		{
			while( Sys_ExNext(dir, &file_info_block) && n_files_dir<MAX_FILES_DIR )
			{
				if( file_info_block.DirEntryType < 0 )
				{
					const char *s = file_info_block.FileName;
					const char *start = s;
					
					// check if directory name is present and remove it
					while( *s )
					{
						if( *s=='/' || *s=='\\' )
							start = s+1;
						s++;
					}

					// copy file name
					char *d = files_dir[n_files_dir].path;
					char *d_end = d + FILE_MAX_PATH - 1;
					s = start;

					while( *s && d<d_end )
						*d++ = *s++;
					*d = 0;
					n_files_dir++;
				}
			}
		}
		Sys_UnLock(dir);
	}
}

const char *File_CreatePath(const char *dir, const char *name)
{
	char *d = temp_text_buffer;

	while( *dir ) *d++ = *dir++;
	while( *name ) *d++ = *name++;
	*d = 0;

	return temp_text_buffer;
}

const char *File_CreateDisplayName(const char *path)
{
	char *d = temp_text_buffer;
	byte prev_lowcase = 0;

	while( *path )
	{
		/**/ if( *path>='a' && *path<='z' )					*d++ = *path++ - 'a' + 'A',			prev_lowcase = 1;
		else if( *path>='A' && *path<='Z' && prev_lowcase )	*d++ = ' ', *d++ = *path++,			prev_lowcase = 0;
		else if( *path=='.' )																	break;
		else if( *path<=' ' )								*d++ = ' ', path++,					prev_lowcase = 0;
		else if( *path=='_' )								*d++ = ' ', path++,					prev_lowcase = 0;
		else												*d++ = *path++,						prev_lowcase = 0;
	}
	*d = 0;

	return temp_text_buffer;
}

const char *File_ReadTextFile(const char *path)
{
	void *file = Sys_Open(path);
	if(!file) return "";

	int len = Sys_Read(file, temp_text_buffer, sizeof(temp_text_buffer)-1);

	if( len>=0 )	temp_text_buffer[len] = 0;
	else			temp_text_buffer[0] = 0;

	Sys_Close(file);

	return temp_text_buffer;
}


#endif
