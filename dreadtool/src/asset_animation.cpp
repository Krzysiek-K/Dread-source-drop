
#include "common.h"
#include "app_rendertest.h"



const char GfxAnimation::TYPE_NAME[] = "animation";






// ================================================================ GfxAnimation ================================================================

void GfxAnimation::Load(DefNode *node, GfxAssetGroup *aowner, GfxConverter *owner)
{
	try
	{
		InitLoad(node, 0, aowner, owner);
		node->RequireArgs(1);
		node->RequireSubnodes();

		name = ".anim:" + full_name;
	}
	catch( const string &e )
	{
		elog.Error(this, e);
		return;
	}

	bool walkswing = false;
	for( DefNode *n : node->subnodes )
	{
		n->type_name = "animation element";

		try
		{
			if( n->name=="frame" )
			{
				Frame fr;
				n->RequireArgsMin(2);
				n->ForbidSubnodes();

				if( frames.size()>0 )
				{
					if( n->args[0]=="-" || n->args[0]=="*" )
						fr.delay = frames.back().delay;
					else
						fr.delay = n->GetIntegerArg(0)/300.f;

					if( n->args[1]=="-" || n->args[1]=="*" )
						fr.asset = frames.back().asset;
					else
						fr.asset = owner->FindGfxAsset(n->args[1].c_str(), true);
				}
				else
				{
					fr.delay = n->GetIntegerArg(0)/300.f;
					fr.asset = owner->FindGfxAsset(n->args[1].c_str(), true);
				}


				if( !fr.asset )
					n->Error(format("Missing asset '%s'", n->args[1].c_str()));

				int argpos = 2;
				bool has_offset = false;
				while( argpos < n->args.size() )
				{
					if( argpos+1 < n->args.size() && n->IsArgInteger(argpos) && n->IsArgInteger(argpos+1) && !has_offset )
					{
						fr.add_off_x = n->GetIntegerArg(argpos++);
						fr.add_off_y = n->GetIntegerArg(argpos++);
						has_offset = true;
					}
					else if( n->args[argpos].size()>=2 && n->args[argpos][0]=='$' )		// TBD: document
					{
						fr.play_sound = n->args[argpos++].c_str() + 1;
					}
					else
						n->InvalidArg(argpos);
				}

				fr.walkswing = walkswing;
				frames.push_back(fr);
			}
			else if( n->name=="-name" )
			{
				n->RequireArgs(1);
				n->ForbidSubnodes();
				name = n->args[0];
			}
			else if( n->name=="-walkswing" )
			{
				n->RequireArgsOr(0, 1);
				n->ForbidSubnodes();
				walkswing = (n->GetIntegerArgDef(0, 1)!=0);
			}
			else if( n->name=="-loop" )
			{
				n->ForbidArgs();
				n->ForbidSubnodes();
				loop_pos = (int)frames.size();
			}
			else
				n->Invalid();
		}
		catch( const string &e )
		{
			elog.Error(this, e);
		}
	}


	owner->AddAsset(this);
}


bool GfxAnimation::SavePreviewEmulation(FILE *fp)
{
	string aname;
	for( const char *s = full_name.c_str(); *s; s++ )
		if( *s=='/' )
			aname.push_back('_');
		else if( *s=='%' )
			aname.push_back('p');
		else if( *s!=' ' )
			aname.push_back(*s);

	fprintf(fp, "\n\tfunction anim_%s(a)\n", aname.c_str());
	fprintf(fp, "\t{\n");
	for( int i=0; i<frames.size(); i++ )
	{
		const Frame &f = frames[i];
		fprintf(fp, "\t\ta.AddFrame(\"%s\",%5d,%3d,%3d,%3d,%3d)\n", f.asset->name.c_str(), int(f.delay*300+0.5f), f.add_off_x, f.add_off_y, f.asset->xoffs, f.asset->yoffs);
	}
	fprintf(fp, "\t}\n");

	return true;
}
