
#include "common.h"
#include "app_rendertest.h"


static bool ParseEndOfLine(const char *&s)
{
	ParseWhitespace(s);
	if( !*s ) return true;
	if( *s=='/' && s[1]=='/' ) return true;
	if( *s=='#' ) return true;
	return false;
}


DefNode::~DefNode()
{
	for( int i=0; i<(int)subnodes.size(); i++ )
		delete subnodes[i];
	subnodes.clear();
}



// throws string
void DefNode::Error(const string &msg, int _line)
{
	throw format("%s(%d): %s", parser->filenames[file_id].c_str(), _line>=0 ? _line+1 : line+1, msg.c_str());
}


// throws string
void DefNode::Unexpected()
{
	Error(format("Unexpected %s '%s'", GetTypeName(), name.c_str()));
}

// throws string
void DefNode::Unknown()
{
	Error(format("Unknown %s '%s'", GetTypeName(), name.c_str()));
}

// throws string
void DefNode::Invalid()
{
	Error(format("Invalid %s '%s'", GetTypeName(), name.c_str()));
}

// throws string
void DefNode::InvalidArg(int index, const char *msg)
{
	if( msg )
		Error(format("Invalid argument %d of %s '%s': %s", index+1, GetTypeName(), name.c_str(), msg));
	else
		Error(format("Invalid argument %d of %s '%s'", index+1, GetTypeName(), name.c_str()));
}

// throws string on error
void DefNode::ForbidArgs()
{
	if( args.size()>0 )
		Error(format("%s '%s' can't have any arguments (%d given)", GetTypeNameUppercase().c_str(), name.c_str(), args.size()));
}

// throws string on error
void DefNode::RequireArgs(int count)
{
	if( args.size()!=count )
		Error(format("%s '%s' expects exactly %d arguments (%d given)", GetTypeNameUppercase().c_str(), name.c_str(), count, args.size()));
}

// throws string on error
void DefNode::RequireArgsRange(int mincount, int maxcount)
{
	if( (int)args.size()<mincount || (int)args.size()>maxcount )
		Error(format("%s '%s' expects %d to %d arguments (%d given)", GetTypeNameUppercase().c_str(), name.c_str(), mincount, maxcount, args.size()));
}

// throws string on error
void DefNode::RequireArgsMin(int mincount)
{
	if( (int)args.size()<mincount )
		Error(format("%s '%s' expects at least %d arguments (%d given)", GetTypeNameUppercase().c_str(), name.c_str(), mincount, args.size()));
}

// throws string on error
void DefNode::RequireArgsOr(int count1, int count2, int count3)
{
	if( (int)args.size()!=count1 && (int)args.size()!=count2 && (int)args.size()!=count3 )
	{
		if( count3>=0 )
			Error(format("%s '%s' expects exactly %d or %d arguments (%d given)", GetTypeNameUppercase().c_str(), name.c_str(), count1, count2, args.size()));
		else
			Error(format("%s '%s' expects exactly %d, %d or %d arguments (%d given)", GetTypeNameUppercase().c_str(), name.c_str(), count1, count2, count3, args.size()));
	}
}

// throws string on error
void DefNode::RequireSubnodes()
{
	if( subnodes.size()<=0 )
		Error(format("%s '%s' must start a paremeter block", GetTypeNameUppercase().c_str(), name.c_str()));
}

// throws string on error
void DefNode::ForbidSubnodes()
{
	if( subnodes.size()>0 )
		Error(format("%s '%s' can't start a paremeter block", GetTypeNameUppercase().c_str(), name.c_str()));
}

// throws string on error
bool DefNode::IsArgInteger(int index)
{
	if( index<0 || index>=(int)args.size() )
		Error(format("[internal] argument index %d requested", index));

	const char *s = args[index].c_str();
	ParseWhitespace(s);
	ParseInt(s);
	ParseWhitespace(s);

	return !*s;
}

// throws string on error
bool DefNode::IsArgFloat(int index)
{
	if( index<0 || index>=(int)args.size() )
		Error(format("[internal] argument index %d requested", index));

	const char *s = args[index].c_str();
	ParseWhitespace(s);
	ParseFloat(s);
	ParseWhitespace(s);

	return !*s;
}

// throws string on error
int DefNode::GetIntegerArg(int index)
{
	if( index<0 || index>=(int)args.size() )
		Error(format("[internal] argument index %d requested", index));

	const char *s = args[index].c_str();
	ParseWhitespace(s);
	int value = ParseInt(s);
	ParseWhitespace(s);
	if( *s )
		Error(format("Argument %d must be an integer", index+1));

	return value;
}


// throws string on error
int DefNode::GetIntegerArgRange(int index, int min, int max)
{
	int value = GetIntegerArg(index);
	if( value < min || value > max )
		Error(format("Argument %d must be an integer in the %d..%d range", index+1, min, max));
	return value;
}

// throws string on error
int DefNode::GetIntegerArgDef(int index, int def)
{
	if( index<0 )
		Error(format("[internal] argument index %d requested", index));

	if( index>=(int)args.size() )
		return def;

	const char *s = args[index].c_str();
	ParseWhitespace(s);
	int value = ParseInt(s);
	ParseWhitespace(s);
	if( *s )
		Error(format("Argument %d must be an integer", index+1));

	return value;
}

// throws string on error
float DefNode::GetFloatArg(int index)
{
	if( index<0 || index>=(int)args.size() )
		Error(format("[internal] argument index %d requested", index));

	const char *s = args[index].c_str();
	ParseWhitespace(s);
	float value = ParseFloat(s);
	ParseWhitespace(s);
	if( *s )
		Error(format("Argument %d must be a decimal number", index+1));

	return value;
}

// throws string on error
float DefNode::GetFloatArgDef(int index, float def)
{
	if( index<0 )
		Error(format("[internal] argument index %d requested", index));

	if( index>=(int)args.size() )
		return def;

	const char *s = args[index].c_str();
	ParseWhitespace(s);
	float value = ParseFloat(s);
	ParseWhitespace(s);
	if( *s )
		Error(format("Argument %d must be a decimal number", index+1));

	return value;
}





void DefNode::ParseSubnodes(vector<string> &lines, int &linenum, bool is_root)
{
	while(1)
	{
		if( linenum>=(int)lines.size() )
		{
			if( is_root )
				break;
			else
				Error("Unexpected end of file while parsing subnodes of this node");
		}

		const char *s = lines[linenum].c_str();
		if( ParseEndOfLine(s) )
		{
			linenum++;
			continue;
		}

		if( *s=='}' )
		{
			if( is_root )
				Error("Unexpected '}' without matching '{'", linenum);

			s++;
			if( !ParseEndOfLine(s) )
				Error("Unexpected characters after closing '}'", linenum);
			linenum++;
			break;
		}

		DefNode *subnode = new DefNode();
		subnodes.push_back(subnode);
		subnode->Parse(lines, linenum, parser);

		if( subnode->name=="include" )
		{
			subnode->RequireArgs(1);
			subnode->ForbidSubnodes();

			// parse included file
			vector<string> inclines;
			string path = FilePathGetPart(parser->filenames[file_id].c_str(), true, false, false) + subnode->args[0];
			subnodes.pop_back();
			delete subnode;

			if( !NFS.GetFileLines(modmgr.GetResourcePath(path).c_str(), inclines) )
				Error(format("Can't open definition file: %s", path.c_str()));

			int inc_line_num = 0;
			int old_file_id = parser->current_file_id;
			parser->current_file_id = parser->filenames.size();
			parser->filenames.push_back(path);

			ParseSubnodes(inclines, inc_line_num, true);

			parser->current_file_id = old_file_id;
		}
	}
}

void DefNode::Parse(vector<string> &lines, int &linenum, DefParser *_parser)
{
	parser = _parser;
	line = linenum;
	file_id = parser->current_file_id;

	const char *s = lines[linenum++].c_str();
	if( ParseEndOfLine(s) )
		Error("[internal] Missing node definition");

	name = ParseString(s);
	while( !ParseEndOfLine(s) )
	{
		if( *s=='{' )
		{
			s++;
			if( !ParseEndOfLine(s) )
				Error("Unexpected characters after '{'");
			ParseSubnodes(lines, linenum, false);
			break;
		}
		args.push_back(ParseString(s));
	}
}


void DefNode::GatherVersions(map<string, bool> &versions)
{
	if( name=="version" )
	{
		for( int i=0; i<(int)args.size(); i++ )
		{
			const char *v = args[i].c_str();
			if( *v=='!' ) v++;
			if( !*v ) continue;
			versions[v] = true;
		}
	}

	for( int i=0; i<(int)subnodes.size(); i++ )
		subnodes[i]->GatherVersions(versions);
}

void DefNode::FilterVersions(map<string, bool> &versions)
{
	if( name=="version" )
	{
		bool allow = true;
		for( int i=0; i<(int)args.size(); i++ )
		{
			const char *v = args[i].c_str();
			if( *v=='!' )
			{
				v++;
				if( !*v ) continue;
				if( versions[v] ) allow = false;
			}
			else
			{
				if( i==0 ) allow = false;
				if( versions[v] ) allow = true;
			}
		}

		if( !allow )
		{
			for( int i=0; i<(int)subnodes.size(); i++ )
				delete subnodes[i];
			subnodes.clear();
		}
	}

	for( auto p : subnodes )
		p->FilterVersions(versions);

	static vector<DefNode*> new_nodes;
	new_nodes.clear();
	for( int i=0; i<(int)subnodes.size(); i++ )
	{
		DefNode *sn = subnodes[i];
		
		if( sn->name == "version" )
		{
			new_nodes.insert(new_nodes.end(),sn->subnodes.begin(),sn->subnodes.end());
			sn->subnodes.clear();
			delete sn;
		}
		else
			new_nodes.push_back(sn);
	}
	subnodes.swap(new_nodes);
}


void DefParser::Reset()
{
	delete root;
	root = new DefNode();

	filenames.clear();
}

void DefParser::LoadFile(const char *path)
{
	if( !root )
		Reset();

	filenames.push_back(path);
	current_file_id = filenames.size()-1;

	vector<string> lines;
	if( !NFS.GetFileLines(modmgr.GetResourcePath(path).c_str(), lines) )
		throw format("Can't open definition file: %s", path);

	int line_num = 0;
	root->parser = this;
	root->line = line_num;
	root->file_id = current_file_id;
	root->ParseSubnodes(lines, line_num, true);
}

void DefParser::AppendModFile(const char *path)
{
	if( !root )
		Reset();

	filenames.push_back(path);
	current_file_id = filenames.size()-1;

	vector<string> lines;
	if( !NFS.GetFileLines(path, lines) )
		return;

	int line_num = 0;
	root->parser = this;
	root->line = line_num;
	root->file_id = current_file_id;
	root->ParseSubnodes(lines, line_num, true);
}
