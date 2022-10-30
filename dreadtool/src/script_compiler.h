
#pragma once

#include "assembler.h"
#include "instructions.h"



class ScriptCompiler {
public:
	typedef int symb_t;

	enum idmode_t {
		IDM_UNKNOWN = 0,
		IDM_GLOBAL_CONST,
		IDM_GLOBAL_VAR,
		IDM_THING_VAR,
	};

	enum functype_t {
		FNT_CALLBACK = 1,
		FNT_FUNCTION = 2,
		FNT_STATE	 = 3,
	};

	struct Actor;

	struct Ident {
		idmode_t	mode = IDM_UNKNOWN;
		string		name;
		int			size = 0;
		int			offset = 0;
		int			value = 0;
		bool		is_actor = false;
		bool		is_editor_frame = false;
		bool		is_frame = false;
		bool		is_sound = false;
		bool		is_weaponframe = false;
		bool		is_weaponsprite = false;
	};

	struct Dependency {
		enum deptype_t {
			DT_NONE = 0,
			DT_ACTOR,				// idents[index]
			DT_FRAME,				// idents[index]
			DT_SOUND,				// idents[index]
			DT_WEAPON_FRAME,		// idents[index]
			DT_WEAPON_SPRITE,		// idents[index]
		};
		deptype_t	type;
		int			index = -1;

		Dependency(deptype_t _type, int _index) : type(_type), index(_index) {}
	};

	struct Function {
		functype_t	type;
		int			name_id;

		vector<Instruction*>	ins;
		vector<Dependency>		deps;

		~Function() {
			for( int i=0; i<(int)ins.size(); i++ )
				delete ins[i];
			ins.clear();
		}
	};

	struct AnnotationValue {
		int		int_value = 0;
		bool	is_integer = false;
		string	text;
	};

	struct Actor {
		int								name_id = -1;
		int								thing_id = 0;
		int								editor_frame_id = -1;
		Actor							*base_class = NULL;
		vector<Function*>				fns;
		map<string, AnnotationValue>	annotations;
		bool							is_used = true;
		bool							is_system_component = false;

		~Actor() {
			for( int i=0; i<(int)fns.size(); i++ )
				delete fns[i];
			fns.clear();
		}

		Function *FindFunction(int idnum) {
			for( int i=0; i<(int)fns.size(); i++ )
				if( fns[i]->name_id==idnum )
					return fns[i];
			return NULL;
		}
	};

	vector<Actor*>	actors;
	vector<Ident>	idents;


	ScriptCompiler() { Clear(); }
	~ScriptCompiler() { Clear(); }

	void	Clear();
	void	Compile(const char *code);
	void	CompileFile(const char *path);
	void	ExportScript(const char *path);
	Ident	*GetIdent(const char *idname);

	void	DumpResult(FILE *fp);

	void	DependencyReset();
	void	DependencyUseActor(Actor *actor);
	void	DependencyUseThing(int thing_id);

protected:

	enum reqtype_t {
		REQ_ACTOR_NAME,
		REQ_ACTOR_STATE,
		REQ_ACTOR_FUNCTION,
	};

	struct EAInfo {
		EAddr	ea;
		int		size = 0;
		bool	temp = false;

		EAInfo(const EAddr &_ea, int _size=0) : ea(_ea), size(_size) {}
	};

	struct Requirement {
		reqtype_t	type;
		int			line;
		int			arg;

		Requirement(reqtype_t _type, int _line, int _arg) : type(_type), line(_line), arg(_arg) {}
	};

	struct JumpInfo {
		bool		logic_or	= false;
		bool		logic_and	= false;
		int			bxx_pos		= -1;
		opcode_t	inv_opcode	= OP_INVALID;
		string		label_after;
		int			subjump1	= -1;
		int			subjump2	= -1;
	};


	string					src;
	const char				*src_pos;
	string					src_filename;
	int						src_line;
	bool					was_error;

	Assembler				as;

	map<string, int>		ident_map;
	map<string, int>		string_map;
	vector<string>			strings;
	string					actor_name;
	vector<EAInfo>			eas;
	int						glob_alloc;
	int						thing_alloc;
	int						xr_alloc;
	int						label_alloc;
	int						enum_value_counter;
	Actor					*actor;
	Function				*fn;			// currently compiled function
	vector<Requirement>		reqs;
	vector<int>				fn_args;
	vector<JumpInfo>		jump_list;


	int		GetChar();
	void	Error(const string &message);
	void	Error(int line, const string &message);
	symb_t	Parse();

	int		FindIdent(const char *idname);
	int		FindString(const char *s);
	int		IdentToEA(int idnum);
	int		FuncToEA(int idnum, int arg=-1);
	void	DeclareVar(int idnum, int size, idmode_t mode);
	void	DeclareConst(int idnum, int expr);
	void	DeclareConstValue(int idnum, int value);
	int		EAImmValue(int expr);
	int		IntegerEA(int value)	{ eas.push_back(instructions::imm(value)); return eas.size()-1; }
	int		TempEA();
	void	LoadToEA(int src, int dst);
	int		LoadToTemp(int src);
	int		MoveAccumulateOp(int dst, int src, opcode_t op);
	bool	IsValidSource(int _ea);
	bool	IsValidDestination(int _ea);
	int		BinaryOp(int dst, int src, opcode_t op, bool symmetric=false);
	int		CreateJump(opcode_t op);
	void	UpdateJump(int jumplist_id);
	int		CompareAndJump(int e1, int e2, opcode_t bsigned_true, opcode_t bunsigned_true, opcode_t bsigned_false, opcode_t bunsigned_false);
	int		CombineConditions(int index1, int index2, bool mode_or);
	void	UpdateConditionalJump(int jumplist_id, bool jump_if_true, const char *jump_target = NULL);
	void	SetFrame(int idnum);
	void	SetWeaponFrame(int idnum);
	void	SetWeaponSprite(int idnum);
	void	StartSound(int idnum, int arg_count);
	void	BecomeActor(int idnum);
	void	SpawnActor(int idnum);
	void	SwitchToState(int idnum);
	void	FunctionCall(int idnum, int num_args);

	void	StartActor(int nameid, int thing_num, int editor_frame_id=0, int base_class_id=0);
	void	AddActorAnnotation_Int(int id, int value);
	void	AddActorAnnotation_String(int id, int string_id);
	void	EndActor();
	void	StartFunction(int nameid, functype_t type);
	void	EndFunction();

};
