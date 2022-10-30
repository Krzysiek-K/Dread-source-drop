
#ifndef _AMIGA_FILES_H
#define _AMIGA_FILES_H


#if ENABLE_MAP_FILE_LOADING


#define FILE_MAX_PATH			32		// including terminating zero
#define MAX_FILES_DIR			32


typedef struct {
	char path[FILE_MAX_PATH];
} FileInfo;


extern FileInfo files_dir[MAX_FILES_DIR];
extern word n_files_dir;

extern char temp_text_buffer[256];

void File_ReadDirFiles(const char *dirpath);
const char *File_CreatePath(const char *dir, const char *name);
const char *File_CreateDisplayName(const char *path);
const char *File_ReadTextFile(const char *path);



#endif



#endif
