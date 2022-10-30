
#ifndef _DREAD_MAPLOAD_H
#define _DREAD_MAPLOAD_H



#define ALL_OK												 0
#define ERROR_INVALID_MAPBLOCK_ID							 1
#define ERROR_MAP_ELEMENT_SIZE_MISMATCH						 2
#define ERROR_MAP_MEMORY_ALLOCATION_ERROR					 3
#define ERROR_INVALID_MAP_FUNCTION							 4
#define ERROR_MAP_HEADER_READ_ERROR							 5
#define ERROR_MAP_RAW_BLOCK_ALLOCATION_ERROR				 6
															 
#define ERROR_DEFLATE_HEADER_READ_ERROR						 7
#define ERROR_DEFLATE_DECOMPRESSION_TOO_LARGE				 8
#define ERROR_DEFLATE_ALLOCATION_ERROR						 9
#define ERROR_DEFLATE_SCRATCH_ALLOCATION_ERROR				10
#define ERROR_DEFLATE_DATA_READ_ERROR						11
#define ERROR_DEFLATE_SEGHEADER_READ_ERROR					12
#define ERROR_DEFLATE_SEG_DECOMPRESSION_TOO_LARGE			13
#define ERROR_DEFLATE_SEGMENT_READ_ERROR					14
#define ERROR_DEFLATE_INVALID_OPTION						15

#define ERROR_MAP_LOAD_ALLOCATION_ERROR						16
#define ERROR_MAP_LOAD_ERROR								17
#define ERROR_MAP_BLOCK_INIT_FAILED							18
#define ERROR_MAP_UNKNOWN_ERROR								19
#define ERROR_MAP_NOT_LOADED								20
#define ERROR_MAP_NOT_FOUND									21


#define ERROR_MAP_VERIFY_NO_DATA							101
#define ERROR_MAP_VERIFY_NO_INIT							102
#define ERROR_MAP_VERIFY_NO_RAW_DATA						103



typedef struct {
	word		*ptr;
	word		*alloc;
	word		*end;
	dword		size;
} memory_buffer_t;




#if ENABLE_MAP_FILE_LOADING
#define MAP_BUFFER_EXTRA_SPACE			150000
#else
#define MAP_BUFFER_EXTRA_SPACE			0
#endif


#define MAP_MEMORY_BUFFER_SIZE		(	\
				ENGINE_MAX_VERTEXES * sizeof(EngineVertex) +		\
				ENGINE_MAX_LINES * sizeof(EngineLine) +				\
				ENGINE_MAX_SUBSECTORS * sizeof(EngineSubSector) +	\
				MAP_BUFFER_EXTRA_SPACE								\
			)

#ifdef _WIN32
const int _map_memory_buffer_size = MAP_MEMORY_BUFFER_SIZE;
#endif

#define NUM_MAP_MEMORY_BUFFERS			3

extern memory_buffer_t map_memory_buffers[NUM_MAP_MEMORY_BUFFERS];





const DataLocation *Map_GetLocation(int id);

#if USE_EMBEDDED_MAP_DATA
int Map_LoadAll(word skill);
#endif

#if ENABLE_MAP_FILE_LOADING
int Map_FileLoadAll_FP(word skill, void *file);
int Map_FileLoadAll(word skill, const char *path);
#endif




#endif
