
#pragma once


class DefParser;



class DefNode : public Collectable {
public:
	string				name;
	string				type_name;		// used in error reporting
	vector<string>		args;
	vector<DefNode*>	subnodes;

	int					line = 0;
	int					file_id = -1;
	DefParser			*parser = NULL;


	~DefNode();

	void Error(const string &msg, int _line = -1);

	const char *GetTypeName()		{ return type_name.size()>0 ? type_name.c_str() : "node"; }
	string GetTypeNameUppercase()	{ string type = GetTypeName();	type[0]=toupper(type[0]);	return type;	}

	void Unexpected();
	void Unknown();
	void Invalid();
	void InvalidArg(int index, const char *msg=NULL);
	void ForbidArgs();
	void RequireArgs(int count);
	void RequireArgsRange(int mincount, int maxcount);
	void RequireArgsMin(int mincount);
	void RequireArgsOr(int count1, int count2, int count3=-1);
	void RequireSubnodes();
	void ForbidSubnodes();
	bool HasSubnodes()			{ return subnodes.size()>0; }
	bool IsArgInteger(int index);
	bool IsArgFloat(int index);
	int GetIntegerArg(int index);
	int GetIntegerArgRange(int index, int min, int max);
	int GetIntegerArgDef(int index, int def);
	float GetFloatArg(int index);
	float GetFloatArgDef(int index, float def);

	void ParseSubnodes(vector<string> &lines, int &linenum, bool is_root);
	void Parse(vector<string> &lines, int &linenum, DefParser *_parser);

	void GatherVersions(map<string, bool> &versions);
	void FilterVersions(map<string, bool> &versions);
};

class DefParser {
public:
	DefNode			*root = NULL;
	vector<string>	filenames;
	int				current_file_id = 0;


	~DefParser() {
		if( root ) delete root;
	}

	void Reset();
	void LoadFile(const char *path);
	void AppendModFile(const char *path);
};