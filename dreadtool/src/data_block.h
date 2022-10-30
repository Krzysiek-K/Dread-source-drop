
#pragma once



#define	MAPBLOCK_ID_TEXTURES			 0
#define	MAPBLOCK_ID_VERTEXES			 1
#define	MAPBLOCK_ID_LINES				 2
#define	MAPBLOCK_ID_VIS					 3
#define	MAPBLOCK_ID_BSP					 4
#define	MAPBLOCK_ID_SUBSECTORS			 5
#define	MAPBLOCK_ID_LOCATIONS			 6
#define	MAPBLOCK_ID_THINGS				 7
#define	MAPBLOCK_ID_MACHINES			 8
#define	MAPBLOCK_ID_CONDITIONS			 9
#define	MAPBLOCK_ID_HEADER				10
#define	MAPBLOCK_ID_PALETTE				11
#define	MAPBLOCK_ID_TEXT				12
#define	NUM_MAPBLOCK_ID					13



// ================================================================ DiskSplitTracker ================================================================


class DiskSplitTracker {
public:
	dword	disk_offset = 0;
	dword	track_length = 11*512;

	dword	GetAvailableTrackSpace()		{ return track_length - disk_offset%track_length;	}
	void	BytesAdded(dword count)			{ disk_offset += count;	}
};





// ================================================================ TextPrinter ================================================================



class TextPrinter {
public:
	enum {
		TAB_WIDTH = 4,
	};

	FILE	*fp;
	int		charpos = 0;		// position in current line


	TextPrinter(FILE *_fp) : fp(_fp) {}

	void Print(const char *s);
	void PrintF(const char *fmt, ...);
	void Print(const string &s)						{ Print(s.c_str()); }

	void TabTo(int pos, bool force_whitespace);			// tabulate to given position; <force_whitespace> inserts single space if past the position
};




// ================================================================ DataMemTracker ================================================================

struct DataMemTracker {
public:
	struct Block {
		int		bank	= -1;
		int		start	=  0;		// in words
		int		end		=  0;		// in words

		bool IsValid()	{ return bank>=0 && start<end; }
		int  size()		{ return end - start; }
	};

	struct BankInfo {
		int		alloc	= 0;		// in words
		int		end		= 0;		// in words

		int available() { return end - alloc; }
	};

	vector<BankInfo>	banks;
	int					missing_mem			= 0;		// in words
	int					new_missing_mem		= 0;		// in words
	int					peak_min_avail		= -1;		// in words
	int					file_size			= 0;		// in words


	void	AddBank(int size_in_words);
	Block	Alloc(int size_in_words);
	void	Free(Block &block);
	void	Trim(Block &block, int new_size_in_words);

	Block	AllocAndLoad(int size_in_words, bool map_deflate, int map_deflate_input_size, int map_deflate_output_size, int map_deflate_leeway);

	int		MemAvail();
};


// ================================================================ DataBlock ================================================================

class DataBlock;
class DataBlockSet;


struct DataBlockStats {
	int current_alloc_size = 0;
	int max_alloc_size = 0;
	int size_total = 0;
	int file_offset = -1;
};


struct DataPointer {
	DataBlock	*block	= 0;
	int			data_element_index = 0;
	int			label_text_id = 0;
};

struct DataElement {
	enum type_t {
		TYPE_NONE	= 0,
		TYPE_LABEL,
		TYPE_PUBLIC_LABEL,
		TYPE_WORD,				// data in words		(up to 16)
		TYPE_DWORD,				// data in dwords		(up to  8)
		TYPE_POINTER,			// pointer to data in this or another block
	};

	enum {
		MAX_WORDS	= 16,
		MAX_DWORDS	= MAX_WORDS/2,
	};

	// !!!! PODs ONLY !!!!
	type_t	type = TYPE_NONE;
	int		count = 0;				// number of data elements
	int		comment_id = -1;		// >=0:  followed by comment	(implies newline)
	bool	newline = false;		// true: followed by newline

	union {
		// union
		int			data_label_text_id;
		word		data_word[MAX_WORDS];
		dword		data_dword[MAX_DWORDS];
		DataPointer data_pointer;
	};

	DataElement() {}
	DataElement(const DataElement &el)
	{
		memcpy(this, &el, sizeof(DataElement));
	}
};

class DataBlock {
public:
	enum {
		CODE_INDENT			=  8,
		DATA_INDENT			= 16,
		COMMENT_INDENT		= 96
	};

	enum map_block_export_mode_t {
		MAP_BLOCK_EXPORT_NONE	= 0,
		MAP_BLOCK_EXPORT_RAW,
		MAP_BLOCK_EXPORT_INIT,
		MAP_BLOCK_EXPORT_SETUP,
	};

	struct location_t {
		DataElement::type_t		type			= DataElement::TYPE_NONE;
		int						data_index		= 0;
		int						array_index		= 0;
	};

	string					name;
	vector<DataElement>		data;
	int						size_in_words = 0;
	DataBlockSet			*owner = NULL;					// not cleared by Clear()

	int						newline_max_items = 16;
	int						newline_item_count = 0;


	// block system info
	int						map_block_id = -1;
	map_block_export_mode_t	map_block_export = MAP_BLOCK_EXPORT_NONE;
	int						map_block_engine_element_size = 0;
	int						map_block_element_size = 0;
	int						map_block_element_count = 0;
	bool					map_deflate = false;
	vector<dword>			map_deflate_segments;					// data offset to segment end		(from after the block header)
	int						map_deflate_input_size = 0;
	int						map_deflate_output_size = 0;
	int						map_deflate_leeway = 0;

	DataBlock() { Clear(); }
	void Clear();

	void ConfigRawBlock(int block_id);
	void ConfigInitBlock(int block_id, int element_size, int element_count);
	void ConfigSetupBlock(int block_id, int element_size, int element_count);


	DataPointer GetCurrentPointer(const string &label);
	DataPointer GetCurrentPointer(const char *prefix, int index)		{ return GetCurrentPointer(format("%s_%d",prefix,index)); }
	void		AddPublicLabel(const string &label);

	void SetItemsPerLine(int count)				{ newline_max_items = count; }
	location_t AddWord(word value);
	void UpdateWord(const location_t &loc, word value);
	void AddDWord(dword value);
	void AddPointer(DataPointer ptr);
	void AddString(const char *s, bool nullterm=false);
	void AddLineComment(const char *text);
	void AddLineCommentF(const char *fmt, ...);
	void AddComment(const char *text);
	void AddCommentF(const char *fmt, ...);
	void AddLineBreak();
	void AddEmptyLine();

	void DumpBlockStats(DataBlockStats &stats);

	void ExportPublicLabelsAsAsm(FILE *fp);
	int  ExportAsAsm(FILE *fp);
	void ExportAsBytes(vector<byte> &out_bin, bool append=false);
	void ExportAsBinary(FILE *fp);

	int ImportBytes(const byte *&src, const byte *end);

	bool Deflate(DiskSplitTracker *disk=NULL);

	
	int VerifyLoad(DataMemTracker &mem);

protected:
	DataElement &AddElement() {
		data.resize(data.size()+1);
		return data[data.size()-1];
	}

};



// ================================================================ DataBlockSet ================================================================

class DataBlockSet {
public:
	vector<DataBlock*>	datablocks;
	vector<string>		text_pool;
	map<string, int>	text_pool_dict;

	~DataBlockSet() { Clear(); }

	void Clear();

	int AssureString(const char *text);
	const char *GetString(int text_id)		{ return text_pool[text_id].c_str(); }

	DataBlock *NewBlock(const char *name);
	
	void DumpBlockStats();
	void DeflateAll(DiskSplitTracker *disk=NULL);
	void ExportAllAsAsm(FILE *fp);
	void ExportAllAsBytes(vector<byte> &out_bin, bool verbose=false, int file_offset=0);
	void ExportAllAsFile(const char *path, bool verbose=false);

	bool ImportFile(const char *path);
	
	void VeryfyLoad(DataMemTracker &mem);
};
