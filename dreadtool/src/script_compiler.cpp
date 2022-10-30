
#include "common.h"
#include "script_compiler.h"



struct ScriptFnInfo {
	const char	*name;
	int			arg_count;
};

static const ScriptFnInfo SCRIPT_FNS[] = {
	// { "stop_logic",					0	},		// TBD: macro
	{ "collision_set",					1	},		// <size>				Enable collisions and set specified radius.
	{ "collision_off",					0	},		//						Disable collisions.
	{ "hitbox_on",						0	},		//						Enable hitbox - shooting the object on screen will accumulate damage and call OnDamage.
	{ "hitbox_off",						0	},		//						Disable hitbox.
	{ "pickup_on",						0	},		//						Call OnPulse when player touches the item.
	{ "pickup_off",						0	},		//						Disable "pickup_on".
	{ "player_hitscan",					2	},		//	<offs>, <dmg>		Fire a bullet from player's perspective. <offs> is offset from the center in pixels. <dmg> is the damage applied.
	{ "player_damaged",					1	},		//	<dmg>				Apply <dmg> damage to the player - calls OnDamage on the player object.
	{ "player_pulse",					1	},		//	<pulse>				Send <pulse> to the player - calls OnPulse on the player object.
	{ "player_here",					0	},		//						Set object position to current player position. Used to update weapon thing position before firing.
	{ "player_set_location",			1	},		//	<location_id>		Teleport player to specified location.
	{ "player_apply_location_delta",	2	},		//	<loc1>, <loc2>		Teleport player by the offset between <loc1> and <loc2>.
	{ "vector_aim",						0	},		//						Set <vector_x/y> fields to point to the player.
	{ "vector_camera",					0	},		//						Set <vector_x/y> to current camera viewing direction.
	{ "vector_align",					2	},		//	<front>, <side>		Apply scaling/rotation matrix to the <vector_x/y>. Values specified as fixed.8 (256 means 1.0).
	{ "vector_distance",				0	},		//						Compute current length of <vector_x/y> and save it in <distance> field.
	{ "vector_fastdist",				0	},		//						Compute approximage length of <vector_x/y> (using "max(abs(x),abs(y))" ) and save it in <distance> field.
	{ "vector_set_length",				1	},		//						Set length of <vector_x/y>.
	{ "vector_reaim",					0	},		//						Reaims the vector to the player. <distance> contains distance player moved away from the previous aim.
	{ "vector_set_timer",				1	},		//	<time>				Prepares the timer to trigger after X ticks. Uses <vector_x/y> internally.
	{ "move_walk",						2	},		//	<speed>, <time>		Initiate enemy movement to location pointed by <vector_x/y> with speed <speed>. Stop movement after <time> ticks, if vector is too long.
	{ "move_missile",					2	},		//	<speed>, <time>		Like "move_walk", but always moves in straight path without any extra logic.
	{ "move_stop",						0	},		//						Immediately stop movement.
	{ "explosion_damage",				1	},		//	<dmg>				Damage everything with hitbox around this thing (not including player).
	{ "machine_line_scroll",			1	},		//	<xoffs>				Scroll texcoords of the line associated with the machine.
	{ "mp_player_damage",				1	},		//	<dmg>				Multiplayer: damage player on the other side
	{ "mp_pulse_avatar",				1	},		//	<pulse>				Multiplayer: send pulse code to avatar on the other side
	{ "mp_pulse_remote_copy",			1	},		//	<pulse>				Multiplayer: send pulse code an item with exactly the same index on the other side



	// Macros:
	//	frame							//	<#frame>			Set object graphics to specified frame.
	//	frame_none						//						Set object graphics to not displayed.
	//	weapon_frame					//	<#frame>			Set object graphics to specified frame. The data will be exported as a weapon sprite.
	//	delay							//	<time>				Stop logic for <time> ticks. Only allowed within a state and required there (otherwise logic will loop infinitely).
	//	destroyed						//						Immediately stop logic and remove the object.
	//	become							//	<actor>				Immediately change the object into specified actor.
	//	spawn_missile					//	<actor>				Spawn <actor> object at current location, passing a copy of <vector_x/y> to it.
	//	done							//						Stop logic indefinitely and exit (callbacks can still wake up the object).
	//

	// Functions:
	//	rand()						Returns pseudo-random number 0..255.
	//	vector_hitscan()			Checks if <vector_x/y> collides with wall. Returns 0 on collision, and 1 if no collision found.
	//	vector_get_timer()			Returns remaining time of the  <vector_x/y> timer.
	//	check_blockers_radius()		Checks if any blocker entity has its center within range of "collision box + <distance>".
	//



	// unimplemented:	{ "shoot_target",		1	},
	{ NULL, 0 }
};






void ScriptCompiler::Clear()
{
	for( int i=0; i<(int)actors.size(); i++ )
		delete actors[i];
	actors.clear();


	src.clear();
	src_pos = "";
	src_filename.clear();
	src_line = 1;
	was_error = false;

	as.Clear();

	idents.clear();
	ident_map.clear();
	strings.clear();
	string_map.clear();
	actor_name.clear();
	eas.clear();
	glob_alloc = 0;
	thing_alloc = 0;
	xr_alloc = 0;
	label_alloc = 0;
	enum_value_counter = 0;

	actor = NULL;
	fn = NULL;
	reqs.clear();
	fn_args.clear();
	jump_list.clear();
}


void ScriptCompiler::Compile(const char *code)
{
	src = code;
	src_pos = src.c_str();
	src_line = 1;
	was_error = false;
	Parse();

	if( was_error )
		return;

	//DumpResult(stdout);

	FILE *fp = fopen("../dreadmake/data/scripts.i", "wt");
	if( fp )
	{
		DumpResult(fp);
		fclose(fp);
	}
}

void ScriptCompiler::CompileFile(const char *path)
{
	if( was_error )
		return;

	src_filename = FilePathGetPart(path, false, true, true);
	src_line = 1;

	string code;
	if( !NFS.GetFileText(path, code) )
	{
		Error("Can't open file");
		return;
	}

	src = code;
	src_pos = src.c_str();

	try {
		Parse();
	}
	catch(string err)
	{
		Error(err.c_str());
	}
}

void ScriptCompiler::ExportScript(const char *path)
{
	FILE *fp = fopen(path, "wt");
	if( fp )
	{
		if( !was_error )
			DumpResult(fp);
		fclose(fp);
	}
}


void ScriptCompiler::DumpResult(FILE *fp)
{
	// Declarations
	for( int i=0; i<(int)idents.size(); i++ )
	{
		Ident &id = idents[i];
		if( id.is_frame )
			fprintf(fp, "\txref \t_Object_%s\n", id.name.c_str());

		if( id.is_weaponframe )
			fprintf(fp, "\txref \t_WEAPONFRAME_%s\n", id.name.c_str());

		if( id.is_weaponsprite )
			fprintf(fp, "\txref \t_WEAPONSPRITEANIM_%s\n", id.name.c_str());

		if( id.is_sound )
		{
			fprintf(fp, "\txref \t_SOUND_%s\n", id.name.c_str());
			fprintf(fp, "\txref \t_TRKSOUND_%s\n", id.name.c_str());
		}
	}

	// Actors
	for( int i=0; i<(int)actors.size(); i++ )
	{
		Actor *act = actors[i];
		if( !act->is_used )
			continue;

		fprintf(fp, "\tpublic\t_Actor_%s:\n", idents[act->name_id].name.c_str());
		fprintf(fp, "_Actor_%s:\n", idents[act->name_id].name.c_str());
		fprintf(fp, "\tdc.w\t%d\n", act->thing_id);

		static const char *CALLBACKS[] ={ "OnCreate", "OnDamage", "OnPulse", NULL };
		for( int j=0; CALLBACKS[j]; j++ )
		{
			Function *fn = act->FindFunction(FindIdent(CALLBACKS[j]));
			if( fn && fn->type==FNT_CALLBACK )
				fprintf(fp, "\tdc.l\t%s_%s\n", idents[act->name_id].name.c_str(), idents[fn->name_id].name.c_str());
			else
				fprintf(fp, "\tdc.l\t0\n");
		}
	}

	// Code
	for( int i=0; i<(int)actors.size(); i++ )
	{
		Actor *act = actors[i];
		if( !act->is_used )
			continue;

		fprintf(fp, "; Actor %s %d\n", idents[act->name_id].name.c_str(), act->thing_id);
		for( int j=0; j<(int)act->fns.size(); j++ )
		{
			Function *fn = act->fns[j];
			static const char *FTYPE[] ={ "","Callback","Function","State" };
			fprintf(fp, ";   %s %s\n", FTYPE[fn->type], idents[fn->name_id].name.c_str());
			for( int k=0; k<(int)fn->ins.size(); k++ )
				fprintf(fp, "%s\n", fn->ins[k]->ToString().c_str());
		}
	}
}

void ScriptCompiler::DependencyReset()
{
	for( int i=0; i<(int)idents.size(); i++ )
	{
		Ident &id = idents[i];
		id.is_frame = false;
		id.is_weaponframe = false;
		id.is_weaponsprite = false;
		id.is_sound = false;
	}

	for( int i=0; i<(int)actors.size(); i++ )
		actors[i]->is_used = false;

	for( int i=0; i<(int)actors.size(); i++ )
		if( actors[i]->is_system_component )
			DependencyUseActor(actors[i]);
}

void ScriptCompiler::DependencyUseActor(Actor *actor)
{
	if( actor->is_used )
		return;

	actor->is_used = true;

	for( int i=0; i<(int)actor->fns.size(); i++ )
	{
		Function *fn = actor->fns[i];
		for( int j=0; j<(int)fn->deps.size(); j++ )
		{
			const Dependency &dep = fn->deps[j];
			switch( dep.type )
			{
			case Dependency::DT_ACTOR:			
				for( int k=0; k<(int)actors.size(); k++ )
					if( actors[k]->name_id == dep.index )
						DependencyUseActor(actors[k]);
				break;
			case Dependency::DT_FRAME:			idents[dep.index].is_frame = true;				break;
			case Dependency::DT_SOUND:			idents[dep.index].is_sound = true;				break;
			case Dependency::DT_WEAPON_FRAME:	idents[dep.index].is_weaponframe = true;		break;
			case Dependency::DT_WEAPON_SPRITE:	idents[dep.index].is_weaponsprite = true;		break;
			}
		}
	}
}

void ScriptCompiler::DependencyUseThing(int thing_id)
{
	for( int i=0; i<(int)actors.size(); i++ )
		if( actors[i]->thing_id == thing_id )
			DependencyUseActor(actors[i]);
}




int	ScriptCompiler::GetChar()
{
	if( !*src_pos )
		return 0;
	if( *src_pos=='\n' ) src_line++;
	return *src_pos++;
}

void ScriptCompiler::Error(const string &message)
{
	printf("%s(%d) : ERROR : %s.\n", src_filename.c_str(), src_line, message.c_str());
	was_error = true;
}

void ScriptCompiler::Error(int line, const string &message)
{
	printf("%s(%d) : ERROR : %s.\n", src_filename.c_str(), line, message.c_str());
	was_error = true;
}



ScriptCompiler::Ident *ScriptCompiler::GetIdent(const char *idname)
{
	auto p = ident_map.find(idname);
	if( p!=ident_map.end() )
		return &idents[p->second];
	return NULL;
}

int ScriptCompiler::FindIdent(const char *idname)
{
	auto p = ident_map.find(idname);
	if( p!=ident_map.end() )
		return p->second;
	Ident id;
	id.name = idname;
	idents.push_back(id);
	return (ident_map[id.name] = idents.size()-1);
}

int	ScriptCompiler::FindString(const char *s)
{
	auto p = string_map.find(s);
	if (p != string_map.end())
		return p->second;
	strings.push_back(s);
	return (string_map[s] = strings.size() - 1);
}


int ScriptCompiler::IdentToEA(int idnum)
{
	using namespace instructions;

	Ident &id = idents[idnum];
	if( id.mode==IDM_THING_VAR )
	{
		eas.push_back(EAInfo(mem("thing_"+id.name, a6),id.size));
	}
	else if( id.mode==IDM_GLOBAL_VAR )
	{
		eas.push_back(EAInfo(mem("gv_"+id.name, a5), id.size));
	}
	else if( id.mode==IDM_GLOBAL_CONST )
	{
		eas.push_back(EAInfo(imm(id.value), id.size));
	}
	else
	{
		Error("Undeclared variable '"+id.name+"'");
		return 0;
	}
	return eas.size()-1;
}

int ScriptCompiler::FuncToEA(int idnum, int arg)
{
	using namespace instructions;

	Ident &id = idents[idnum];
	if( id.name == "rand" )
	{
		if( arg>=0 ) Error("Function '"+id.name+"' does not expect arguments");

		EAddr dr = xr(xr_alloc++);

		as << mov.l(imm(0), dr) << "rand()";
		as << mov.w(mem("gv_randpos", a5), dr);
		as << add.b(imm(1), dr);
		as << mov.w(dr, mem("gv_randpos", a5));
		as << lea.l(label("_RAND_TAB"), a0);
		as << mov.b(mem_idx_w(a0, dr), dr);

		eas.push_back(EAInfo(dr, 0));
	}
	else if( id.name == "vector_hitscan" )
	{
		if( arg>=0 ) Error("Function '"+id.name+"' does not expect arguments");

		EAddr dr = xr(xr_alloc++);

		as << jsr(label("script_fn_vector_hitscan_0")) << "vector_hitscan()";
		as << mov.l(d7, dr);

		eas.push_back(EAInfo(dr, 0));
	}
	else if( id.name == "vector_get_timer" )
	{
		if( arg>=0 ) Error("Function '"+id.name+"' does not expect arguments");

		EAddr dr = xr(xr_alloc++);
		as << mov.l(mem("thing_vector_x", a6), dr);
		as << sub.l(mem("gv_levstat_time", a5), dr);
		eas.push_back(EAInfo(dr, 0));
	}
	else if( id.name == "check_blockers_radius" )
	{
		if( arg>=0 ) Error("Function '"+id.name+"' does not expect arguments");

		EAddr dr = xr(xr_alloc++);

		as << jsr(label("script_fn_check_blockers_radius_0")) << "check_blockers_radius()";
		as << mov.l(d7, dr);

		eas.push_back(EAInfo(dr, 0));
	}
	else if( id.name == "tag_check" )
	{
		if( arg<0 ) Error("Function '"+id.name+"' expects exactly one argument");

		if( !IsValidSource(arg) )
			arg = LoadToTemp(arg);

		EAInfo &earg = eas[arg];
		EAddr dr = xr(xr_alloc++);

		as << mov.l(earg.ea, mem_predec(a7));
		as << jsr(label("script_fn_tag_check_1")) << "tag_check(...)";
		as << mov.l(mem_postinc(a7), dr);

		eas.push_back(EAInfo(dr, 0));
	}
	else
	{
		Error("Unknown function '"+id.name+"'");
		return 0;
	}
	return eas.size()-1;
}


void ScriptCompiler::DeclareVar(int idnum, int size, idmode_t mode)
{
	Ident &id = idents[idnum];
	if( id.mode!=IDM_UNKNOWN )
	{
		Error("Redeclaration of identifier '"+id.name+"'");
		return;
	}
	id.mode = mode;
	id.size = size;
	if( id.mode==IDM_THING_VAR )
	{
		if( abs(size)>=2 && (thing_alloc&1) ) thing_alloc++;
		id.offset = thing_alloc;
		thing_alloc += abs(size);
	}
	else if( id.mode==IDM_GLOBAL_VAR )
	{
		if( abs(size)>=2 && (glob_alloc&1) ) glob_alloc++;
		id.offset = glob_alloc;
		glob_alloc += abs(size);
	}
}

void ScriptCompiler::DeclareConst(int idnum, int expr)
{
	using namespace instructions;

	Ident &id = idents[idnum];
	if( id.mode!=IDM_UNKNOWN )
	{
		Error("Redeclaration of identifier '"+id.name+"'");
		return;
	}
	id.mode = IDM_GLOBAL_CONST;
	id.size = 0;

	EAInfo &ea = eas[expr];
	if( ea.ea.amode!=AMODE_IMM )
	{
		Error("Const definition of '"+id.name+"' requires constant value");
		return;
	}

	id.value = ea.ea.value;
}

void ScriptCompiler::DeclareConstValue(int idnum, int value)
{
	using namespace instructions;

	Ident &id = idents[idnum];
	if( id.mode!=IDM_UNKNOWN )
	{
		Error("Redeclaration of identifier '"+id.name+"'");
		return;
	}
	id.mode = IDM_GLOBAL_CONST;
	id.size = 0;
	id.value = value;
}

int ScriptCompiler::EAImmValue(int expr)
{
	EAInfo &ea = eas[expr];
	if( ea.ea.amode!=AMODE_IMM )
	{
		Error("Immediate constant expected");
		return 0;
	}

	return ea.ea.value;
}

int	ScriptCompiler::TempEA()
{
	using namespace instructions;
	eas.push_back(xr(xr_alloc++));
	return eas.size()-1;
}

// Make sure source is loaded to another register
void ScriptCompiler::LoadToEA(int _src, int _dst)
{
	using namespace instructions;

	if( was_error ) return;

	EAInfo &src = eas[_src];
	EAInfo &dst = eas[_dst];

	if( src.size>=0 )
	{
		/**/ if( src.size==1 )	as << mov.l(imm(0), dst.ea), as << mov.b(src.ea, dst.ea);
		else if( src.size==2 )	as << mov.l(imm(0), dst.ea), as << mov.w(src.ea, dst.ea);
		else					as << mov.l(src.ea, dst.ea);
	}
	else
	{
		if( src.size==-1 )
		{
			as << mov.b(src.ea, dst.ea);
			as << extb.l(dst.ea);
		}
		if( src.size==-2 )
		{
			as << mov.w(src.ea, dst.ea);
			as << ext.l(dst.ea);
		}
		if( src.size==-4 ) as << mov.l(src.ea, dst.ea);
	}
}

// Make sure source is loaded to temp register
int ScriptCompiler::LoadToTemp(int _src)
{
	using namespace instructions;

	if( was_error ) return _src;

	int _x = TempEA();
	EAInfo &src = eas[_src];
	if( src.temp ) return _src;

	EAInfo &x = eas[_x];
	x.temp = true;

	LoadToEA(_src, _x);

	return _x;
}

int ScriptCompiler::MoveAccumulateOp(int _dst, int _src, opcode_t op)
{
	if( was_error ) return 0;

	if( (eas[_src].size && eas[_dst].size && abs(eas[_src].size)<abs(eas[_dst].size)) ||
		op!=OP_MOVE )
		_src = LoadToTemp(_src);

	EAInfo &src = eas[_src];
	EAInfo &dst = eas[_dst];

	if( !dst.size )
	{
		Error("Destination is not a l-value");
		return 0;
	}

	Instruction *ins = new Instruction();
	ins->opcode = op;
	ins->dst = dst.ea;
	ins->src = src.ea;
	if( abs(dst.size)==1 ) ins->width = SIZE_BYTE;
	if( abs(dst.size)==2 ) ins->width = SIZE_WORD;
	if( abs(dst.size)==4 ) ins->width = SIZE_LONG;
	as.Assemble(ins);

	return (op==OP_MOVE) ? _src : _dst;
}

bool ScriptCompiler::IsValidSource(int _ea)
{
	EAInfo &ea = eas[_ea];
	if( !ea.size || abs(ea.size)==4 )
		return true;
	return false;
}

bool ScriptCompiler::IsValidDestination(int _ea)
{
	EAInfo &ea = eas[_ea];
	return ea.temp;
}

int	ScriptCompiler::BinaryOp(int _dst, int _src, opcode_t op, bool symmetric)
{
	using namespace instructions;

	if( was_error ) return 0;

	if( symmetric )
	{
		int ns = 0;
		int rs = 0;
		if( IsValidSource(_src) ) ns++;
		if( IsValidDestination(_dst) ) ns++;
		if( IsValidSource(_dst) ) rs++;
		if( IsValidDestination(_src) ) rs++;
		if( rs>ns )
			swap(_src, _dst);
	}

	if( !IsValidSource(_src) )
		_src = LoadToTemp(_src);
	if( !IsValidDestination(_dst) )
		_dst = LoadToTemp(_dst);

	EAInfo &src = eas[_src];
	EAInfo &dst = eas[_dst];

	Instruction *ins = new Instruction();
	ins->opcode = opcode_t(op & 0x3FFF);
	ins->dst = dst.ea;
	ins->src = src.ea;
	ins->width = (ins->opcode==OP_MULS || ins->opcode==OP_DIVS) ? SIZE_WORD : SIZE_LONG;
	as.Assemble(ins);
	
	if( op == OP_DIVS )
		as << ext.l(dst.ea);

	if( op == OP_DIVS+0x4000 )
	{
		as << swp.w(dst.ea);
		as << ext.l(dst.ea);
	}

	return _dst;
}

int ScriptCompiler::CreateJump(opcode_t op)
{
	using namespace instructions;

	Instruction *ins = new Instruction();
	ins->opcode = op;
	ins->width = SIZE_WORD;
	as.Assemble(ins);

	JumpInfo ji;
	ji.bxx_pos = as.instr.size()-1;
	jump_list.push_back(ji);

	return jump_list.size()-1;
}

void ScriptCompiler::UpdateJump(int jumplist_id)
{
	using namespace instructions;

	string lab = format("__script_label_%d", label_alloc++);
	as.Assemble(lab+":");

	const JumpInfo &ji = jump_list[jumplist_id];
	as.instr[ji.bxx_pos]->dst = label(lab);
}

int ScriptCompiler::CompareAndJump(int _e1, int _e2, opcode_t bsigned_true, opcode_t bunsigned_true, opcode_t bsigned_false, opcode_t bunsigned_false)
{
	using namespace instructions;

	if( _e1>=0 )
	{
		if( _e2<0 )
		{
			if( eas[_e1].temp )
				as << mov.l(eas[_e1].ea, eas[_e1].ea);
			else
				LoadToTemp(_e1);
		}
		else
		{
			if( !IsValidSource(_e1) )
				_e1 = LoadToTemp(_e1);
			if( !IsValidSource(_e2) )
				_e2 = LoadToTemp(_e2);

			as << cmp.l(eas[_e2].ea, eas[_e1].ea);
		}
	}

	Instruction *ins = new Instruction();
	ins->opcode = bsigned_true;
	ins->dst = label("__TBD__");
	as.Assemble(ins);

	string lab = format("__script_label_%d", label_alloc++);

	JumpInfo ji;
	ji.bxx_pos = as.instr.size()-1;
	ji.inv_opcode = bsigned_false;
	ji.label_after = lab;
	jump_list.push_back(ji);

	as.Assemble(lab+":");

	return jump_list.size()-1;
}

int ScriptCompiler::CombineConditions(int index1, int index2, bool mode_or)
{
	JumpInfo j;
	if( mode_or )	j.logic_or  = true;
	else			j.logic_and = true;
	j.subjump1 = index1;
	j.subjump2 = index2;
	j.label_after = jump_list[index2].label_after;
	jump_list.push_back(j);

	return jump_list.size()-1;
}

void ScriptCompiler::UpdateConditionalJump(int jumplist_id, bool jump_if_true, const char *jump_target)
{
	using namespace instructions;
	string lab;

	// target not specified - jump here
	if( !jump_target )
	{
		lab = format("__script_label_%d", label_alloc++);
		as.Assemble(lab+":");
		jump_target = lab.c_str();
	}

	JumpInfo &j = jump_list[jumplist_id];
	if( j.logic_or )
	{
		if( jump_if_true )
		{
			// jump if any subcondition IS met
			UpdateConditionalJump(j.subjump1, true, jump_target);
			UpdateConditionalJump(j.subjump2, true, jump_target);
		}
		else // jump if false
		{
			UpdateConditionalJump(j.subjump1, true, jump_list[j.subjump2].label_after.c_str());		// if first condition IS met, skip the second check
			UpdateConditionalJump(j.subjump2, false, jump_target);									// jump to target if second condition is also NOT met
		}
	}
	else if( j.logic_and )
	{
		if( jump_if_true )
		{
			UpdateConditionalJump(j.subjump1, false, jump_list[j.subjump2].label_after.c_str());	// if first condition is NOT met, skip the second check
			UpdateConditionalJump(j.subjump2, true, jump_target);									// jump to target if second condition IS also met
		}
		else // jump if false
		{
			// jump if any subcondition is NOT met
			UpdateConditionalJump(j.subjump1, false, jump_target);
			UpdateConditionalJump(j.subjump2, false, jump_target);
		}
	}
	else
	{
		if( !jump_if_true )
			swap(as.instr[j.bxx_pos]->opcode, j.inv_opcode);
		if( strcmp(jump_target, "")==0 )
			Error("[internal] empty jump target");
		as.instr[j.bxx_pos]->dst = label(jump_target);
	}
}


void ScriptCompiler::SetFrame(int idnum)
{
	using namespace instructions;

	if( idnum >= 0 )
	{
		Ident &id = idents[idnum];
		id.is_frame = true;
		as << lea.l(label("_Object_"+id.name), a0);
		as << mov.l(a0, mem("thing_sprite", a6));

		fn->deps.push_back(Dependency(Dependency::DT_FRAME, idnum));
	}
	else
		as << mov.l(imm(0), mem("thing_sprite", a6));
}

void ScriptCompiler::SetWeaponFrame(int idnum)
{
	using namespace instructions;

	Ident &id = idents[idnum];
	id.is_weaponframe = true;
	as.Assemble(" if \"WEAPONSPRITES=0\"");
	as << lea.l(label("_WEAPONFRAME_"+id.name), a0);
	as << mov.l(a0, mem("thing_sprite", a6));
	as.Assemble(" endif");

	fn->deps.push_back(Dependency(Dependency::DT_WEAPON_FRAME, idnum));
}

void ScriptCompiler::SetWeaponSprite(int idnum)
{
	using namespace instructions;

	Ident &id = idents[idnum];
	id.is_weaponsprite = true;
	as.Assemble(" if \"WEAPONSPRITES=1\"");
	as << lea.l(label("_WEAPONSPRITEANIM_"+id.name), a0);
	as << mov.l(a0, label("_weaponsprite_anim"));
	as << mov.w(imm(0), label("_weaponsprite_delay"));
	as.Assemble(" endif");

	fn->deps.push_back(Dependency(Dependency::DT_WEAPON_SPRITE, idnum));
}

void ScriptCompiler::StartSound(int idnum, int arg_count)
{
	using namespace instructions;

	if( arg_count > 1 )
	{
		Error("Sound function requires sound name and up to one optional argument");
		return;
	}

	Ident &id = idents[idnum];
	int *args = (arg_count>0) ? &fn_args[fn_args.size()-arg_count] : NULL;

	id.is_sound = true;
	fn->deps.push_back(Dependency(Dependency::DT_SOUND, idnum));

	as.Assemble(" if AMIGA_AUDIO_ENGINE_VERSION_2");
	as << lea.l(label("_TRKSOUND_"+id.name), a0);
	as.Assemble(" else");
	as << lea.l(label("_SOUND_"+id.name), a0);
	as.Assemble(" endif");

	if( arg_count>=1 )	as << mov.l(eas[args[0]].ea, d7);
	else				as << mov.l(imm(0), d7);
	as << jsr(label("_Sound_PlayThingSound"));

	fn_args.clear();
}

void ScriptCompiler::BecomeActor(int idnum)
{
	using namespace instructions;

	Ident &id = idents[idnum];

	as << lea.l(label("_Actor_"+id.name), a0);
	as << mov.l(a0, mem("thing_actor", a6));

	if( fn->type==FNT_STATE )
	{
		as << jsr(label(id.name+"_OnCreate"));
		as << mov.l(mem("thing_script_fn", a6), a0);
		as << jmp(mem(a0));
	}
	else if( fn->type==FNT_FUNCTION )
	{
		as << mov.l(label("script_stack_reset"), a7);
		as << jsr(label(id.name+"_OnCreate"));
		as << mov.l(mem("thing_script_fn", a6), a0);
		as << jmp(mem(a0));
	}
	else if( fn->type==FNT_CALLBACK )
	{
		as << jmp(label(id.name+"_OnCreate"));
	}

	reqs.push_back(Requirement(REQ_ACTOR_NAME, src_line, idnum));
	fn->deps.push_back(Dependency(Dependency::DT_ACTOR, idnum));
}

void ScriptCompiler::SpawnActor(int idnum)
{
	using namespace instructions;

	Ident &id = idents[idnum];

	as << lea.l(label("_Actor_"+id.name), a0);
	as << jsr(label("script_fn_spawn_missile_0"));

	reqs.push_back(Requirement(REQ_ACTOR_NAME, src_line, idnum));
	fn->deps.push_back(Dependency(Dependency::DT_ACTOR, idnum));
}


void ScriptCompiler::SwitchToState(int idnum)
{
	using namespace instructions;

	if( fn->type==FNT_STATE )
	{
		as << bra(label(actor_name+"_"+idents[idnum].name));
	}
	else if( fn->type==FNT_FUNCTION )
	{
		as << mov.l(label("script_stack_reset"), a7);
		as << bra(label(actor_name+"_"+idents[idnum].name));
	}
	else if( fn->type==FNT_CALLBACK )
	{
		as << lea.l(label(actor_name+"_"+idents[idnum].name), a0);
		as << mov.l(a0, mem("thing_script_fn", a6));
		as << mov.w(imm(0), mem("thing_timer_delay", a6));
		as.Assemble(" rts");
	}

	reqs.push_back(Requirement(REQ_ACTOR_STATE, src_line, idnum));
}

void ScriptCompiler::FunctionCall(int idnum, int num_args)
{
	using namespace instructions;

	string &fname = idents[idnum].name;
	if( (int)fn_args.size() < num_args )
		num_args = (int)fn_args.size();
	int *args = (num_args>0) ? &fn_args[fn_args.size()-num_args] : NULL;

	// macros
	if( fname=="delay" )
	{
		if( num_args!=1 )
		{
			Error("'delay' function expects 1 argument");
			return;
		}
		if( fn->type!=FNT_STATE )
		{
			Error("'delay' can be called only within a state");
			return;
		}

		string lab = format("__script_continue_%d", label_alloc++);
		string lab2 = format("__script_delay_check_%d", label_alloc++);
		as << add.w(eas[LoadToTemp(args[0])].ea, mem("thing_timer_delay", a6));
		as << bpl(label(lab2));
		as << mov.w(imm(0), mem("thing_timer_delay", a6));
		as.Assemble(lab2+":");
		as << lea.l(label(lab), a0);
		as << mov.l(a0, mem("thing_script_fn", a6));
		as.Assemble(" rts");
		as.Assemble(lab+":");
	
		fn_args.clear();
		return;
	}

	if( fname=="done" )
	{
		if( num_args!=0 )
		{
			Error("'done' function expects no arguments");
			return;
		}
		if( fn->type!=FNT_STATE )
		{
			Error("'done' can be called only within a state");
			return;
		}

		as << mov.l(imm(0), mem("thing_script_fn", a6));
		as.Assemble(" rts");

		fn_args.clear();
		return;
	}

	if( fname=="destroyed" )
	{
		if( num_args!=0 )
		{
			Error("'destroyed' function expects 0 arguments");
			return;
		}

		if( fn->type==FNT_FUNCTION )
			as << mov.l(label("script_stack_reset"), a7);
		as << jmp(label("script_fn_destroyed_0"));

		fn_args.clear();
		return;
	}

	// internal functions
	for( int i=0; SCRIPT_FNS[i].name; i++ )
		if( SCRIPT_FNS[i].name==fname )
		{
			if( num_args!=SCRIPT_FNS[i].arg_count )
			{
				Error(format("'%s' function expects %d argument(s)", fname.c_str(), SCRIPT_FNS[i].arg_count).c_str());
				return;
			}

			if( num_args>=1 )
			{
				//as << mov.l(eas[args[0]].ea, d7);
				eas.push_back(EAInfo(d7, 4));
				LoadToEA(args[0], eas.size()-1);
			}
			if( num_args>=2 )
			{
				//as << mov.l(eas[args[1]].ea, d6);
				eas.push_back(EAInfo(d6, 4));
				LoadToEA(args[1], eas.size()-1);
			}
			as << bsr(label(format("script_fn_%s_%d", fname.c_str(), num_args)));

			fn_args.clear();
			return;
		}

	// actor function
	if( fn->type==FNT_CALLBACK )
	{
		Error("Actor function '"+idents[idnum].name+"' can't be called from a callback");
		return;
	}

	if( num_args!=0 )
	{
		Error("Actor function '"+idents[idnum].name+"' can't have arguments");
		return;
	}

	as << bsr(label(actor_name+"_"+idents[idnum].name));
	reqs.push_back(Requirement(REQ_ACTOR_FUNCTION, src_line, idnum));

	fn_args.clear();
}


void ScriptCompiler::StartActor(int nameid, int thing_num, int editor_frame_id, int base_class_id)
{
	Actor *actor = new Actor();
	actor->name_id = nameid;
	actor->thing_id = (thing_num>=0) ? thing_num : 0;
	actor->is_system_component = (thing_num<0);
	actor->editor_frame_id = editor_frame_id;
	actors.push_back(actor);

	Ident &id = idents[nameid];
	id.is_actor = true;
	actor_name = id.name;

	if( editor_frame_id ) idents[editor_frame_id].is_editor_frame = true;

	if( base_class_id )
	{
		for( int i=0; i<(int)actors.size()-1; i++ )
			if( actors[i]->name_id == base_class_id )
			{
				actor->base_class = actors[i];
				break;
			}
		if( !actor->base_class )
			Error("Can't find actor base class '"+idents[base_class_id].name+"'");
	}

	this->actor = actor;
}

void ScriptCompiler::AddActorAnnotation_Int(int id, int value)
{
	AnnotationValue v;
	v.int_value = value;
	v.is_integer = true;
	actor->annotations[idents[id].name] = v;
}

void ScriptCompiler::AddActorAnnotation_String(int id, int string_id)
{
	AnnotationValue v;
	v.text = strings[string_id];
	actor->annotations[idents[id].name] = v;
}


void ScriptCompiler::EndActor()
{
	for( int i=0; i<(int)reqs.size(); i++ )
	{
		Requirement &req = reqs[i];
		if( req.type==REQ_ACTOR_STATE )
		{
			Function *fn = actor->FindFunction(req.arg);
			if( !fn ) Error(req.line, "No state named '"+idents[req.arg].name+"'");
			else if( fn->type!=FNT_STATE ) Error(req.line, "Function '"+idents[req.arg].name+"' is not a state");
			reqs.erase(reqs.begin()+i--);
			continue;
		}
		if( req.type==REQ_ACTOR_FUNCTION )
		{
			Function *fn = actor->FindFunction(req.arg);
			if( !fn ) Error(req.line, "No function named '"+idents[req.arg].name+"'");
			else if( fn->type!=FNT_FUNCTION ) Error(req.line, "Function '"+idents[req.arg].name+"' is not a callable function");
			reqs.erase(reqs.begin()+i--);
			continue;
		}
	}

	actor = NULL;
}

void ScriptCompiler::StartFunction(int nameid, functype_t type)
{
	Function *fn = new Function();
	fn->name_id = nameid;
	fn->type = type;
	actor->fns.push_back(fn);
	this->fn = fn;

	as.instr.clear();
	as.Assemble(actor_name+"_"+idents[nameid].name+":");
}

void ScriptCompiler::EndFunction()
{
	using namespace instructions;

	if( fn->type==FNT_STATE )
	{
		as << bra(label(actor_name+"_"+idents[fn->name_id].name));
	}
	else
	{
		as.Assemble(" rts");
	}

	as.AllocateRegs(0x0F3F);
	fn->ins.swap(as.instr);
	fn = NULL;
}
