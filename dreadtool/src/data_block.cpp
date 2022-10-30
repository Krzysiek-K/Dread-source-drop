
#include "common.h"
#include "app_rendertest.h"

#include "zlib.h"



void _fwrite_word(FILE *fp, word value)
{
	fwrite(((byte*)&value)+1, 1, 1, fp);
	fwrite(((byte*)&value)+0, 1, 1, fp);
}

void _fwrite_dword(FILE *fp, dword value)
{
	fwrite(((byte*)&value)+3, 1, 1, fp);
	fwrite(((byte*)&value)+2, 1, 1, fp);
	fwrite(((byte*)&value)+1, 1, 1, fp);
	fwrite(((byte*)&value)+0, 1, 1, fp);
}

void _push_word(vector<byte> &bin, word value)
{
	bin.push_back(((byte*)&value)[1]);
	bin.push_back(((byte*)&value)[0]);
}

void _push_dword(vector<byte> &bin, dword value)
{
	bin.push_back(((byte*)&value)[3]);
	bin.push_back(((byte*)&value)[2]);
	bin.push_back(((byte*)&value)[1]);
	bin.push_back(((byte*)&value)[0]);
}

byte _read_byte(const byte *&s, const byte *end)
{
	return s<end ? *s++ : 0;
}

word _read_word(const byte *&s, const byte *end)
{
	word w = s<end ? word(*s++)<<8 : 0;
	w |= s<end ? *s++ : 0;
	return w;
}

dword _read_dword(const byte *&s, const byte *end)
{
	dword w = s<end ? dword(*s++)<<24 : 0;
	w |= s<end ? dword(*s++)<<16 : 0;
	w |= s<end ? dword(*s++)<<8 : 0;
	w |= s<end ? *s++ : 0;
	return w;
}




// ================================================================ TextPrinter ================================================================



void TextPrinter::Print(const char *s)
{
	fprintf(fp, "%s", s);

	for( ; *s; s++ )
	{
		/**/ if( *s == '\t' )		charpos = (charpos+TAB_WIDTH)/TAB_WIDTH*TAB_WIDTH;
		else if( *s == '\n' )		charpos = 0;
		else if( byte(*s) >= ' ' )	charpos++;
	}
}

void TextPrinter::PrintF(const char *fmt, ...)
{
	va_list arg;
	string tmp;
	va_start(arg, fmt);
	vsprintf(tmp, fmt, arg);
	va_end(arg);

	Print(tmp.c_str());
}

void TextPrinter::TabTo(int pos, bool force_whitespace)
{
	if( charpos >= pos && force_whitespace )
		Print(" ");

	while( charpos < pos )
		Print("\t");
}




// ================================================================ DataMemTracker ================================================================


void DataMemTracker::AddBank(int size_in_words)
{
	BankInfo bank;
	bank.end = size_in_words;
	banks.push_back(bank);
}

DataMemTracker::Block DataMemTracker::Alloc(int size_in_words)
{
	Block block;

	int best_buff_score = -1000000000;

	// Find smallest bank that fits.
	// If none fits - find the largest bank.
	for( int i=0; i<banks.size(); i++ )
	{
		BankInfo *buff = &banks[i];
		int size = buff->available();
		int score = ( size >= size_in_words ) ? 1000000000 - size : size;

		if( score > best_buff_score )
		{
			block.bank = i;
			best_buff_score = score;
		}
	}

	BankInfo *bank = &banks[block.bank];
	int bank_size = bank->available();
	if( bank_size<0 ) bank_size = 0;

	if( size_in_words > bank_size )
	{
		int missing = size_in_words - bank_size;
		missing_mem += missing;
		new_missing_mem += missing;
	}

	block.start = bank->alloc;
	bank->alloc += size_in_words;
	block.end = bank->alloc;

	int avail = MemAvail();
	if( peak_min_avail<0 || avail<peak_min_avail )
		peak_min_avail = avail;

	return block;
}

void DataMemTracker::Free(Block &block)
{
	if( block.IsValid() )
	{
		if( banks[block.bank].alloc == block.end )
		{
			banks[block.bank].alloc = block.start;
			block = Block();
		}
	}
}

void DataMemTracker::Trim(Block &block, int new_size_in_words)
{
	if( !block.IsValid() || new_size_in_words >= block.end - block.start )
		return;

	if( banks[block.bank].alloc == block.end )
	{
		block.end = block.start + new_size_in_words;
		banks[block.bank].alloc = block.end;
	}
}

DataMemTracker::Block DataMemTracker::AllocAndLoad(int size_in_words, bool map_deflate, int map_deflate_input_size, int map_deflate_output_size, int map_deflate_leeway)
{
	if( !map_deflate )
	{
		file_size += size_in_words;
		return Alloc(size_in_words);
	}

	// all sizes to words
	map_deflate_input_size >>= 1;
	map_deflate_output_size >>= 1;
	map_deflate_leeway >>= 1;

	file_size += 12/2;	// deflate header
	file_size += map_deflate_output_size;


	// check size
	if( map_deflate_input_size > size_in_words )
		throw string("Uncompressed size > block size");

	// allocate main buffer
	Block reg = Alloc(size_in_words + map_deflate_leeway);

	// allocate deflate extra memory
	Block scratch = Alloc(3000/2);

	Free(scratch);
	Trim(reg, size_in_words);

	return reg;
}

int DataMemTracker::MemAvail()
{
	int avail = 0;
	for( int i=0; i<banks.size(); i++ )
		avail += banks[i].end - banks[i].alloc;
	return avail;
}




// ================================================================ DataBlock ================================================================


void DataBlock::Clear()
{
	name.clear();
	data.clear();
	size_in_words = 0;
	
	newline_max_items = 16;
	newline_item_count = 0;
	
	map_block_id = -1;
	map_block_export = MAP_BLOCK_EXPORT_NONE;
	map_block_engine_element_size = 0;
	map_block_element_size = 0;
	map_block_element_count = 0;

	map_deflate = false;
	map_deflate_segments.clear();
	map_deflate_input_size = 0;
	map_deflate_output_size = 0;
	map_deflate_leeway = 0;
}

void DataBlock::ConfigRawBlock(int block_id)
{
	map_block_id = block_id;
	map_block_export = MAP_BLOCK_EXPORT_RAW;
}

void DataBlock::ConfigInitBlock(int block_id, int element_size, int element_count)
{
	map_block_id = block_id;
	map_block_export = MAP_BLOCK_EXPORT_INIT;
	map_block_element_size = element_size;
	map_block_element_count = element_count;
}

void DataBlock::ConfigSetupBlock(int block_id, int element_size, int element_count)
{
	map_block_id = block_id;
	map_block_export = MAP_BLOCK_EXPORT_SETUP;
	map_block_element_size = element_size;
	map_block_element_count = element_count;
}

DataPointer DataBlock::GetCurrentPointer(const string &label)
{
	AddLineBreak();

	DataPointer ptr;
	ptr.block = this;
	ptr.data_element_index = data.size();
	ptr.label_text_id = owner->AssureString(("__data_"+name+"_"+label).c_str());

	DataElement &el = AddElement();
	el.type = DataElement::TYPE_LABEL;
	el.data_label_text_id = ptr.label_text_id;

	el.newline = true;
	newline_item_count = 0;

	return ptr;
}

void DataBlock::AddPublicLabel(const string &label)
{
	AddLineBreak();

	DataElement &el = AddElement();
	el.type = DataElement::TYPE_PUBLIC_LABEL;
	el.data_label_text_id = owner->AssureString(("__data_"+name+"_"+label).c_str()); 

	el.newline = true;
	newline_item_count = 0;
}


DataBlock::location_t DataBlock::AddWord(word value)
{
	location_t loc;
	DataElement *el;

	if( data.size()>0 && data.back().type==DataElement::TYPE_WORD && data.back().count < DataElement::MAX_WORDS && !data.back().newline && data.back().comment_id<0 )
		el = &data[data.size()-1];
	else
	{
		el = &AddElement();
		el->type = DataElement::TYPE_WORD;
	}

	loc.type = DataElement::TYPE_WORD;
	loc.data_index = el - &data[0];
	loc.array_index = el->count;

	el->data_word[el->count++] = value;
	size_in_words++;

	newline_item_count = (newline_item_count+1) % newline_max_items;
	el->newline = (newline_item_count==0);

	return loc;
}

void DataBlock::UpdateWord(const location_t &loc, word value)
{
	data[loc.data_index].data_word[loc.array_index] = value;
}


void DataBlock::AddDWord(dword value)
{
	DataElement *el;
	if( data.size()>0 && data.back().type==DataElement::TYPE_DWORD && data.back().count < DataElement::MAX_DWORDS && !data.back().newline && data.back().comment_id<0 )
		el = &data[data.size()-1];
	else
	{
		el = &AddElement();
		el->type = DataElement::TYPE_DWORD;
	}

	el->data_dword[el->count++] = value;
	size_in_words += 2;

	newline_item_count = (newline_item_count+1) % newline_max_items;
	el->newline = (newline_item_count==0);
}

void DataBlock::AddPointer(DataPointer ptr)
{
	AddLineBreak();

	DataElement &el = AddElement();
	el.type = DataElement::TYPE_POINTER;
	el.data_pointer = ptr;
	size_in_words += 2;

	el.newline = true;
	newline_item_count = 0;
}

void DataBlock::AddString(const char *s, bool nullterm)
{
	const char *s0 = s;

	while( *s )
	{
		if( s[1] )
			AddWord((uint8_t(*s)<<8) | uint8_t(s[1])), s += 2;
		else
		{
			AddWord(uint8_t(*s++)<<8);
			nullterm = false;			// already terminated
		}
	}

	if( nullterm )
		AddWord(0);

	AddCommentF("\"%s\"", s0);
}

void DataBlock::AddLineComment(const char *text)
{
	AddLineBreak();

	DataElement &el = AddElement();
	el.comment_id = owner->AssureString(text);
	newline_item_count = 0;
}

void DataBlock::AddLineCommentF(const char *fmt, ...)
{
	va_list arg;
	string tmp;
	va_start(arg, fmt);
	vsprintf(tmp, fmt, arg);
	va_end(arg);

	AddLineComment(tmp.c_str());
}

void DataBlock::AddComment(const char *text)
{
	if( data.size()>0 && data[data.size()-1].comment_id<0 )
	{
		data[data.size()-1].comment_id = owner->AssureString(text);
		data[data.size()-1].newline = true;
	}
	else
		AddLineComment(text);
}

void DataBlock::AddCommentF(const char *fmt, ...)
{
	va_list arg;
	string tmp;
	va_start(arg, fmt);
	vsprintf(tmp, fmt, arg);
	va_end(arg);

	AddComment(tmp.c_str());
}

void DataBlock::AddLineBreak()
{
	if( data.size()>0 )
		data[data.size()-1].newline = true;
	newline_item_count = 0;
}

void DataBlock::AddEmptyLine()
{
	DataElement &el = AddElement();
	el.newline = true;
}


void DataBlock::DumpBlockStats(DataBlockStats &stats)
{
	// print block stats
	if( stats.file_offset>=0 )
		printf("%6X: ", stats.file_offset);

	if( map_deflate )
	{
		printf("    %-16s %6d bytes  (%6d compressed, %5.1f%% ratio)", name.c_str(), size_in_words*2, map_deflate_output_size, map_deflate_output_size*100.f/map_deflate_input_size);

		if( map_deflate_segments.size()>0 )
		{
			printf("   ");
			for( int i=0; i<map_deflate_segments.size(); i++ )
				printf(" %6X", stats.file_offset+8+12+map_deflate_segments[i]);
		}

		printf("\n");
	}
	else
		printf("    %-16s %6d bytes\n", name.c_str(), size_in_words*2);
	stats.size_total += size_in_words*2;

	// track loadint memory pressure
	if( map_block_export == DataBlock::MAP_BLOCK_EXPORT_RAW )
	{
		stats.current_alloc_size += size_in_words * 2;
	}
	else if( map_block_export == DataBlock::MAP_BLOCK_EXPORT_INIT )
	{
		stats.current_alloc_size += map_block_element_count * map_block_engine_element_size;
		stats.current_alloc_size += map_block_element_count * map_block_element_size;

		if( stats.current_alloc_size > stats.max_alloc_size )
			stats.max_alloc_size = stats.current_alloc_size;

		stats.current_alloc_size -= map_block_element_count * map_block_element_size;
	}
	else if( map_block_export == DataBlock::MAP_BLOCK_EXPORT_SETUP )
	{
		stats.current_alloc_size += map_block_element_count * map_block_element_size;
	}

	if( stats.current_alloc_size > stats.max_alloc_size )
		stats.max_alloc_size = stats.current_alloc_size;
}

void DataBlock::ExportPublicLabelsAsAsm(FILE *fp)
{
	const DataElement *el = data.size()>0 ? &data[0] : NULL;
	const DataElement *data_end = el + data.size();

	while( el < data_end )
	{
		DataElement::type_t datatype = el->type;

		if( datatype == DataElement::TYPE_PUBLIC_LABEL )
			fprintf(fp, "\t\tpublic %s", owner->GetString(el->data_label_text_id));

		el++;
	}
}

int DataBlock::ExportAsAsm(FILE *fp)
{
	TextPrinter tp(fp);
	const DataElement *el = data.size()>0 ? &data[0] : NULL;
	const DataElement *data_end = el + data.size();
	int word_address = 0;

	word header_flags = 0;

	if( map_deflate )
		header_flags |= 0x8000;


	tp.PrintF("\n; ================================================================ %s ================================================================\n", name.c_str());
	tp.Print(";\n");
	tp.TabTo(CODE_INDENT, false);
	tp.PrintF("public _data_block_%s\n", name.c_str());

	if( map_block_export == MAP_BLOCK_EXPORT_RAW )
	{
		tp.TabTo(CODE_INDENT, false);
		tp.Print("dc.w");
		tp.TabTo(DATA_INDENT, true);
		tp.PrintF("$%04X", header_flags | 0x0100 | map_block_id);
		tp.TabTo(COMMENT_INDENT, true);
		tp.Print("; MAP_BLOCK_EXPORT_RAW\n");

		tp.TabTo(CODE_INDENT, false);
		tp.Print("dc.l");
		tp.TabTo(DATA_INDENT, true);
		tp.PrintF("%d\n", size_in_words);

		tp.TabTo(CODE_INDENT, false);
		tp.Print("dc.w");
		tp.TabTo(DATA_INDENT, true);
		tp.PrintF("$%04X\n", 0);

		word_address += 4;
	}
	else if( map_block_export == MAP_BLOCK_EXPORT_INIT )
	{
		tp.TabTo(CODE_INDENT, false);
		tp.Print("dc.w");
		tp.TabTo(DATA_INDENT, true);
		tp.PrintF("$%04X, %d, %d, 0", header_flags | 0x0200 | map_block_id, map_block_element_size, map_block_element_count);
		tp.TabTo(COMMENT_INDENT, true);
		tp.Print("; MAP_BLOCK_EXPORT_INIT\n");
	
		word_address += 4;
	}
	else if( map_block_export == MAP_BLOCK_EXPORT_SETUP )
	{
		tp.TabTo(CODE_INDENT, false);
		tp.Print("dc.w");
		tp.TabTo(DATA_INDENT, true);
		tp.PrintF("$%04X, %d, %d, 0", header_flags | 0x0300 | map_block_id, map_block_element_size, map_block_element_count);
		tp.TabTo(COMMENT_INDENT, true);
		tp.Print("; MAP_BLOCK_EXPORT_SETUP\n");

		word_address += 4;
	}

	if( map_deflate )
	{
		tp.TabTo(CODE_INDENT, false);
		tp.Print("dc.l");
		tp.TabTo(DATA_INDENT, true);
		tp.PrintF("%d, %d, %d", map_deflate_input_size | (map_deflate_segments.size() ? 0x01000000 : 0), map_deflate_output_size, map_deflate_leeway);
		tp.TabTo(COMMENT_INDENT, true);
		tp.Print("; deflate in/out size\n");
		word_address += 12;
	}


	tp.PrintF("_data_block_%s:\n", name.c_str());


	bool line_dirty = false;
	while( el < data_end )
	{
		// print one line
		DataElement::type_t datatype = el->type;
		bool allow_run = true;
		bool first = true;

		while( el < data_end && el->type == datatype && allow_run )
		{
			allow_run = false;

			if( line_dirty )
			{
				tp.PrintF("\n");
				line_dirty = false;
			}

			if( datatype == DataElement::TYPE_LABEL || datatype == DataElement::TYPE_PUBLIC_LABEL )
			{
				tp.PrintF("%s:", owner->GetString(el->data_label_text_id));
				line_dirty = true;
			}
			else if( datatype == DataElement::TYPE_WORD )
			{
				for( int i=0; i<el->count; i++ )
				{
					tp.TabTo(CODE_INDENT, false);
					if( first )
					{
						tp.Print("dc.w");
						tp.TabTo(DATA_INDENT, true);
						first = false;
					}
					else
						tp.Print(", ");

					tp.PrintF("$%04X", el->data_word[i]);
					word_address++;
					line_dirty = true;
				}
				allow_run = true;
			}
			else if( datatype == DataElement::TYPE_DWORD )
			{
				for( int i=0; i<el->count; i++ )
				{
					tp.TabTo(CODE_INDENT, false);
					if( first )
					{
						tp.Print("dc.l");
						tp.TabTo(DATA_INDENT, true);
						first = false;
					}
					else
						tp.Print(", ");

					tp.PrintF("$%08X", el->data_dword[i]);
					word_address += 2;
					line_dirty = true;
				}
				allow_run = true;
			}
			else if( datatype == DataElement::TYPE_POINTER )
			{
				tp.TabTo(CODE_INDENT, false);
				if( first )
				{
					tp.Print("dc.l");
					tp.TabTo(DATA_INDENT, true);
					first = false;
				}
				else
					tp.Print(", ");

				const DataPointer &p = el->data_pointer;
				tp.Print(owner->GetString(p.label_text_id));
				word_address += 2;
				line_dirty = true;
			}

			if( el->comment_id >= 0 )
			{
				if( el->type )
					tp.TabTo(COMMENT_INDENT, true);
				else
					tp.TabTo(CODE_INDENT, false);

				tp.PrintF("; %s\n", owner->GetString(el->comment_id));
				allow_run = false;
				line_dirty = false;
			}
			else if( el->newline )
			{
				tp.PrintF("\n");
				allow_run = false;
				line_dirty = false;
			}

			first = false;
			el++;
		}
	}

	if( line_dirty )
		tp.PrintF("\n");

	return word_address;
}

void DataBlock::ExportAsBytes(vector<byte> &out_bin, bool append)
{
	const DataElement *el = data.size()>0 ? &data[0] : NULL;
	const DataElement *data_end = el + data.size();
	word header_flags = 0;

	if( !append )
		out_bin.clear();

	if( map_deflate )
		header_flags |= 0x8000;

	if( map_block_export == MAP_BLOCK_EXPORT_RAW )
	{
		_push_word(out_bin, header_flags | 0x0100 | map_block_id);
		_push_dword(out_bin, size_in_words);
		_push_word(out_bin, 0);
	}
	else if( map_block_export == MAP_BLOCK_EXPORT_INIT )
	{
		_push_word(out_bin, header_flags | 0x0200 | map_block_id);
		_push_word(out_bin, map_block_element_size);
		_push_word(out_bin, map_block_element_count);
		_push_word(out_bin, map_block_element_count);		// TBD: count of elements to keep
	}
	else if( map_block_export == MAP_BLOCK_EXPORT_SETUP )
	{
		_push_word(out_bin, header_flags | 0x0300 | map_block_id);
		_push_word(out_bin, map_block_element_size);
		_push_word(out_bin, map_block_element_count);
		_push_word(out_bin, 0);
	}

	if( map_deflate )
	{
		_push_dword(out_bin, map_deflate_input_size | (map_deflate_segments.size() ? 0x01000000 : 0));
		_push_dword(out_bin, map_deflate_output_size);
		_push_dword(out_bin, map_deflate_leeway);
	}

	while( el < data_end )
	{
		DataElement::type_t datatype = el->type;

		if( datatype == DataElement::TYPE_WORD )
		{
			for( int i=0; i<el->count; i++ )
				_push_word(out_bin, el->data_word[i]);
		}
		else if( datatype == DataElement::TYPE_DWORD )
		{
			for( int i=0; i<el->count; i++ )
				_push_dword(out_bin, el->data_dword[i]);
		}
		else if( datatype == DataElement::TYPE_POINTER )
		{
			// ERROR
			printf("DataBlock::ExportAsBinary: TYPE_POINTER not supported\n");
		}

		el++;
	}
}

void DataBlock::ExportAsBinary(FILE *fp)
{
	static vector<byte> tmp_bin;
	
	ExportAsBytes(tmp_bin);

	if( tmp_bin.size()>0 )
		fwrite(&tmp_bin[0], 1, tmp_bin.size(), fp);
}

int DataBlock::ImportBytes(const byte *&src, const byte *end)
{
	Clear();
	
	const byte *header_start = src;

	dword expected_size_in_words = 0;
	word header_flags = _read_word(src, end);
	if( header_flags==0 ) return -1;			// End
	if( header_flags & 0x8000 )
	{
		printf("Error 1: Can't load deflated map\n");
		return 1;
	}

	if( (header_flags & 0x7F00) == 0x0100 )
	{
		map_block_export = MAP_BLOCK_EXPORT_RAW;
		map_block_id = header_flags & 0x00FF;
		expected_size_in_words = _read_dword(src, end);
		if( _read_word(src, end) != 0 )
		{
			printf("Error 2: Invalid value in header\n");
			return 2; 
		}

		printf("Importing RAW block %d: %d words\n", map_block_id, expected_size_in_words);
	}
	else if( (header_flags & 0x7F00) == 0x0200 )
	{
		map_block_export = MAP_BLOCK_EXPORT_INIT;
		map_block_id = header_flags & 0x00FF;
		map_block_element_size = _read_word(src, end);
		map_block_element_count = _read_word(src, end);
		if( _read_word(src, end) != map_block_element_count )
		{
			printf("Error 3: Unsupported header argument combination\n");
			return 3;
		}
		expected_size_in_words = (map_block_element_count * map_block_element_size)/2;

		printf("Importing INIT block %d: %d elements of size %d  (%d words)\n", map_block_id, map_block_element_count, map_block_element_size, expected_size_in_words);
	}
	else if( (header_flags & 0x7F00) == 0x0300 )
	{
		map_block_export = MAP_BLOCK_EXPORT_SETUP;
		map_block_id = header_flags & 0x00FF;
		map_block_element_size = _read_word(src, end);
		map_block_element_count = _read_word(src, end);
		if( _read_word(src, end) != 0 )
		{
			printf("Error 4: Unsupported header argument combination\n");
			return 4;
		}
		expected_size_in_words = (map_block_element_count * map_block_element_size)/2;

		printf("Importing SETUP block %d: %d elements of size %d  (%d words)\n", map_block_id, map_block_element_count, map_block_element_size, expected_size_in_words);
	}
	else
	{
		printf("Error 5: Unsupported header\n");
		return 5;
	}

	if( src - header_start != 8 )
	{
		printf("Error 6: Read error when parsing header\n");
		return 6;
	}

	while( size_in_words < expected_size_in_words )
	{
		if( src+2 > end )
		{
			printf("Error 7: Read error when loading data\n");
			return 7;
		}
		AddWord(_read_word(src, end));
	}

	return 0;	// All OK
}


bool DataBlock::Deflate(DiskSplitTracker *disk)
{
	const DataElement *el = data.size()>0 ? &data[0] : NULL;
	const DataElement *data_end = el + data.size();
	static vector<byte> tmpbuff;


	// ================================ Build input stream ================================
	vector<byte> indata;

	while( el < data_end )
	{
		DataElement::type_t datatype = el->type;

		if( datatype == DataElement::TYPE_WORD )
		{
			for( int i=0; i<el->count; i++ )
			{
				indata.push_back(byte(el->data_word[i]>>8));
				indata.push_back(byte(el->data_word[i]));
			}
		}
		else if( datatype == DataElement::TYPE_DWORD )
		{
			for( int i=0; i<el->count; i++ )
			{
				indata.push_back(byte(el->data_dword[i]>>24));
				indata.push_back(byte(el->data_dword[i]>>16));
				indata.push_back(byte(el->data_dword[i]>> 8));
				indata.push_back(byte(el->data_dword[i]));
			}
		}
		else if( datatype == DataElement::TYPE_POINTER )
		{
			// ERROR
			printf("%s Deflate: TYPE_POINTER not supported\n", name.c_str());		// TBD: switch to exceptions
			return false;
		}
		else if( datatype == DataElement::TYPE_LABEL || datatype == DataElement::TYPE_PUBLIC_LABEL )
		{
			// ERROR
			printf("%s Deflate: labels not supported\n", name.c_str());
			return false;
		}
		else
		{
			// ERROR
			printf("%s Deflate: unknown type\n", name.c_str());
			return false;
		}
		el++;
	}

	if( indata.size()<=0 )
		return false;


	// ================================ Deflate ================================

	vector<byte> outdata;
	if( !disk )
	{
		// Pack entire block
		map_deflate_leeway = 0;
		if( run_deflate(&indata[0], indata.size(), outdata, &map_deflate_leeway) != Z_OK || outdata.size()<=0 )
			return false;

		if( outdata.size() % 2 )
			outdata.push_back(0);

		if( outdata.size()+12 >= indata.size() )
		{
			printf("%s Deflate: pointless compression (%d -> %d)\n", name.c_str(), indata.size(), outdata.size()+12);
			return false;
		}


		map_deflate = true;
		map_deflate_segments.clear();
		map_deflate_input_size = indata.size();
		map_deflate_output_size = outdata.size();
		if( map_deflate_leeway % 2 )
			map_deflate_leeway++;
		map_deflate_leeway += 4;
	}
	else
	{
		map_deflate_segments.clear();
		map_deflate_leeway = 0;

		uint32_t start_offset = disk->disk_offset;
		disk->BytesAdded(8+12);		// block & compression headers


		// Split block into tracks
		int srcpos = 0;
		while( srcpos < indata.size() )
		{
			// Estimate much data can we put in one segment
			int max_compressed_size = disk->GetAvailableTrackSpace() - 8;		// subtract subheader size
			if( max_compressed_size < 64 )
				max_compressed_size += disk->track_length;

			// Find how much data can we compress

			// Compress the track
			int remaining_data = indata.size() - srcpos;
			int lower_bound = 1;											// minimum number of uncompressed bytes to compress
			int upper_bound = remaining_data;								// maximum number of uncompressed bytes we still hope to compress
			int next_guess = min(remaining_data, max_compressed_size*2);	// first guess is compression ration 50%

			if( next_guess <= lower_bound || next_guess >= upper_bound )
				next_guess = (lower_bound + upper_bound + 1) / 2;

			int steps = 0;
			while( lower_bound < upper_bound )
			{
				steps++;
				int ret = run_deflate(&indata[srcpos], next_guess, tmpbuff);
				if( ret != 0 )
				{
					// ERROR
					printf("%s Deflate: deflate returned error code %d", name.c_str(), ret);
					return false;
				}

				// next_guess -> compressed.size()		- does it fit?
				if( tmpbuff.size() <= max_compressed_size )
				{
					lower_bound = next_guess;
				}
				else
				{
					upper_bound = next_guess - 1;
				}

				next_guess = (lower_bound + upper_bound + 1) / 2;
			}

			if( lower_bound != upper_bound )
			{
				// ERROR
				printf("%s Deflate: bisection error (%d..%d)", name.c_str(), lower_bound, upper_bound);
				return false;
			}

			int seg_leeway = 0;
			int ret = run_deflate(&indata[srcpos], lower_bound, tmpbuff, &seg_leeway);
			if( ret != 0 || tmpbuff.size()<=0 || tmpbuff.size() > max_compressed_size )
			{
				// ERROR
				printf("%s Deflate: deflate returned error code %d (2)", name.c_str(), ret);
				return false;
			}

			srcpos += lower_bound;


			// Enlarge tmpbuff to max_compressed_size, include skip in leeway
			int skip = srcpos<indata.size() ? max_compressed_size - tmpbuff.size() : 0;
			tmpbuff.resize(tmpbuff.size()+skip);
			if( tmpbuff.size()%2 )
			{
				tmpbuff.push_back(0);
				skip++;
			}

			// Include <seg_leeway> plus the above enlargement in the general leeway
			seg_leeway += skip;
			if( skip > map_deflate_leeway )
				map_deflate_leeway = skip;

			// Attach to output buffer
			_push_dword(outdata, lower_bound);		// uncompressed
			_push_dword(outdata, tmpbuff.size());	// compressed

			outdata.insert(outdata.end(), tmpbuff.begin(), tmpbuff.end());

			disk->BytesAdded(8+tmpbuff.size());
			map_deflate_segments.push_back(outdata.size());
		}

		// End the stream
		_push_dword(outdata, 0);
		_push_dword(outdata, 0);
		disk->BytesAdded(8);

		//if( outdata.size() < indata.size() )
		{
			map_deflate = true;
			map_deflate_input_size = indata.size();
			map_deflate_output_size = outdata.size();
		}
		//else
		//{
		//	// Skip compression
		//	map_deflate = false;
		//	map_deflate_segments.clear();
		//	map_deflate_input_size = 0;
		//	map_deflate_output_size = 0;
		//	map_deflate_leeway = 0;
		//
		//	disk->disk_offset = start_offset;
		//	disk->BytesAdded(8 + size_in_words*2);		// block headers + uncompressed data
		//
		//	return true;
		//}
	}


	// ================================ Rewrite ================================
	int _size = size_in_words;

	data.clear();
	int pos = 0;
	for( ; pos+1<outdata.size(); pos+=2 )
		AddWord((outdata[pos]<<8) | outdata[pos+1]);
	if( pos < outdata.size() )
		AddWord(outdata[pos]<<8);

	size_in_words = _size;

	return true;
}

int DataBlock::VerifyLoad(DataMemTracker &mem)
{
	int info_offset = mem.file_size*2;		// offset in file in bytes
	const char *info_typename = "???";
	string info_error;
	int info_payload_size = 0;				// in words
	int info_prev_avail = mem.MemAvail();
	int info_prev_file_size = mem.file_size;
	int buff_size =  0;
	int init_size = -1;

	try {
		mem.new_missing_mem = 0;
		mem.file_size += 4;						// standard header size

												// compute payload size
		{
			const DataElement *el = data.size()>0 ? &data[0] : NULL;
			const DataElement *data_end = el + data.size();
			while( el < data_end )
			{
				switch( el->type )
				{
				case DataElement::TYPE_LABEL:																					break;
				case DataElement::TYPE_PUBLIC_LABEL:																			break;
				case DataElement::TYPE_WORD:			info_payload_size += el->count;											break;
				case DataElement::TYPE_DWORD:			info_payload_size += el->count * 2;										break;
				case DataElement::TYPE_POINTER:			printf("DataBlock::VerifyLoad: TYPE_POINTER not supported\n");			break;
				}
				el++;
			}
		}

		if( map_deflate )
		{
			if( info_payload_size*2 != map_deflate_output_size )
				info_error += format(" deflate payload size mismatch %d!=%d", info_payload_size*2, map_deflate_output_size);
			
			info_payload_size = map_deflate_input_size / 2;
		}

		if( map_block_export == MAP_BLOCK_EXPORT_RAW )
		{
			info_typename = "RAW";		// header:		$x1xx.w		size_in_words.l		0.w
			if( info_payload_size != size_in_words )
				info_error += format(" payload size mismatch %d!=%d", info_payload_size, size_in_words);

			DataMemTracker::Block buff = mem.AllocAndLoad(size_in_words, map_deflate, map_deflate_input_size, map_deflate_output_size, map_deflate_leeway);
			buff_size = buff.size()*2;
		}
		else if( map_block_export == MAP_BLOCK_EXPORT_INIT )
		{
			info_typename = "INIT";		// header:		$x2xx.w		element_size.w		element_count.w		0.w
			if( info_payload_size != map_block_element_size*map_block_element_count/2 )
				info_error += format(" payload size mismatch %d!=%d", info_payload_size, map_block_element_size*map_block_element_count/2);

			DataMemTracker::Block buff = mem.Alloc(map_block_engine_element_size*map_block_element_count/2);
			DataMemTracker::Block init = mem.AllocAndLoad(map_block_element_size*map_block_element_count/2, map_deflate, map_deflate_input_size, map_deflate_output_size, map_deflate_leeway);
			buff_size = buff.size()*2;
			init_size = init.size()*2;
			mem.Free(init);
		}
		else if( map_block_export == MAP_BLOCK_EXPORT_SETUP )
		{
			info_typename = "SETUP";	// header:		$x2xx.w		element_size.w		element_count.w		0.w
			if( info_payload_size != map_block_element_size*map_block_element_count/2 )
				info_error += format(" payload size mismatch %d!=%d", info_payload_size, map_block_element_size*map_block_element_count/2);

			DataMemTracker::Block buff = mem.AllocAndLoad(map_block_element_size*map_block_element_count/2, map_deflate, map_deflate_input_size, map_deflate_output_size, map_deflate_leeway);
			buff_size = buff.size()*2;
		}
	}
	catch(const string &err)
	{
		info_error += " "+err;
	}

	if( mem.new_missing_mem > 0 )
	{
		info_error += format(" out of memory (%d bytes)", mem.new_missing_mem*2);
		//for( int i=0; i<mem.banks.size(); i++ )
		//	info_error += format(" %d", mem.banks[i].available()*2);
	}

	printf("    %6s", format("$%02X", info_offset).c_str());
	printf(" %-16s", name.c_str());
	if( mem.new_missing_mem )	printf(" %6d", buff_size);
	else						printf(" %6d", (info_prev_avail - mem.MemAvail())*2);
	if( init_size >= 0 ) printf(" %6d", init_size);
	else				 printf("       ");
	printf(" %6d", (mem.file_size - info_prev_file_size)*2);
	if( map_deflate )	printf(" %3.0f%%", map_deflate_output_size*100.f/map_deflate_input_size);
	else				printf("     ");

	for( int i=0; i<mem.banks.size(); i++ )
		printf(" %6d", mem.banks[i].available()*2);

	if( info_error.size()>0 )
		printf(" ERROR:%s", info_error.c_str());
	
	printf("\n");

	return 0;
}


// ================================================================ DataBlockSet ================================================================


void DataBlockSet::Clear()
{
	for( int i=0; i<datablocks.size(); i++ )
		delete datablocks[i];
	datablocks.clear();

	text_pool.clear();
	text_pool_dict.clear();
}

int DataBlockSet::AssureString(const char *text)
{
	auto p = text_pool_dict.find(text);
	if( p != text_pool_dict.end() )
		return p->second;

	int index = text_pool.size();
	text_pool.push_back(text);
	text_pool_dict[text] = index;

	return index;
}

DataBlock *DataBlockSet::NewBlock(const char *name)
{
	DataBlock *block = new DataBlock();
	datablocks.push_back(block);
	block->name = name;
	block->owner = this;
	return block;
}


void DataBlockSet::DumpBlockStats()
{
	DataBlockStats stats;

	printf("Map file structure:\n");

	for( int i=0; i<(int)datablocks.size(); i++ )
		datablocks[i]->DumpBlockStats(stats);

	printf("    %16s %6d bytes)\n", "(total", stats.size_total);
	printf("  Required buffer size = %d bytes\n", stats.max_alloc_size);
}

void DataBlockSet::DeflateAll(DiskSplitTracker *disk)
{
	for( int i=0; i<(int)datablocks.size(); i++ )
		datablocks[i]->Deflate(disk);
}

void DataBlockSet::ExportAllAsAsm(FILE *fp)
{
	TextPrinter tp(fp);

	for( int i=0; i<(int)datablocks.size(); i++ )
		datablocks[i]->ExportPublicLabelsAsAsm(fp);

	tp.Print("\n\n; ================================================================ HEADER ================================================================\n");
	tp.Print(";\n");
	tp.TabTo(DataBlock::CODE_INDENT, false);
	tp.Print("public _memfile_START\n");
	tp.Print("_memfile_START:\n");

	int word_address = 0;
	for( int i=0; i<(int)datablocks.size(); i++ )
	{
		tp.TabTo(DataBlock::COMMENT_INDENT, false);
		tp.PrintF("; offset $%04X\n", word_address*2);

		word_address += datablocks[i]->ExportAsAsm(fp);
	}


	tp.Print("\n");
	tp.TabTo(DataBlock::CODE_INDENT, false);
	tp.Print("dc.w");
	tp.TabTo(DataBlock::DATA_INDENT, true);
	tp.PrintF("$0000\n");
}

void DataBlockSet::ExportAllAsBytes(vector<byte> &out_bin, bool verbose, int file_offset)
{
	DataBlockStats stats;

	out_bin.clear();

	for( int i=0; i<(int)datablocks.size(); i++ )
	{
		if( verbose )
		{
			stats.file_offset = out_bin.size() + file_offset;
			datablocks[i]->DumpBlockStats(stats);
		}

		datablocks[i]->ExportAsBytes(out_bin, true);
	}

	_push_word(out_bin, 0);
	_push_word(out_bin, 0);
	_push_word(out_bin, 0);
	_push_word(out_bin, 0);

	if( verbose )
		printf("%6X: data end  (%d bytes)\n", out_bin.size() + file_offset, out_bin.size() + file_offset);
}

void DataBlockSet::ExportAllAsFile(const char *path, bool verbose)
{
	vector<byte> out_bin;

	ExportAllAsBytes(out_bin, verbose);

	NFS.DumpRawVector(path, out_bin);
}

bool DataBlockSet::ImportFile(const char *path)
{
	vector<byte> data;

	Clear();

	if( !NFS.GetFileBytes(path, data) || data.size()<=0 )
		return false;

	const byte *src = &data[0];
	const byte *end = src + data.size();
	while( 1 )
	{
		DataBlock *block = NewBlock("-");
		int offs = src - &data[0];
		int ret = block->ImportBytes(src, end);
		if( ret==-1 )
		{
			delete datablocks.back();
			datablocks.pop_back();
			break;
		}
		if( ret!=0 )
		{
			printf("DataBlockSet::ImportFile: block import error code %d starting at offset +$%02X\n", ret, offs);
			return false;
		}

		block->name = format("BLOCK %d", block->map_block_id);
	}

	return true;
}

void DataBlockSet::VeryfyLoad(DataMemTracker &mem)
{
	int init_avail = mem.MemAvail();

	printf("Map file structure:\n");
	printf("    [OFFS] [---NAME-------] [RAM-] [INIT] [FILE] [%%-]\n");
	for( int i=0; i<(int)datablocks.size(); i++ )
		datablocks[i]->VerifyLoad(mem);
	mem.file_size += 4;

	printf("  RAM used:        %6d bytes\n", (init_avail - mem.MemAvail())*2);
	printf("  peak RAM used:   %6d bytes\n", (init_avail - mem.peak_min_avail)*2);
	printf("  file size:       %6d bytes\n", mem.file_size*2);
	if( mem.missing_mem )
		printf("  MISSING RAM:     %6d bytes\n", mem.missing_mem*2);
	for( int i=0; i<mem.banks.size(); i++ )
	{
		if( i==0 ) printf("  RAM left:        %6d bytes\n", (mem.banks[i].end - mem.banks[i].alloc)*2);
		else	   printf("                   %6d bytes\n", (mem.banks[i].end - mem.banks[i].alloc)*2);
	}
}