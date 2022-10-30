
#include "common.h"
#include "app_rendertest.h"


const char AssetBase::TYPE_NAME[] = "asset";




void AssetBase::Error(const string &msg)
{
	try
	{
		if( src_node.get() )
			src_node->Error(msg);
		else
			elog.Error(this, msg);
	}
	catch( const string &e )
	{
		elog.Error(this, e);
	}
}




void AssetBase::InitLoad(DefNode *node, int name_arg_index, GfxAssetGroup *agroup, GfxConverter *owner)
{
	node->type_name = string(GetAssetClassName()) + " definition";
	src_node = node;
	asset_group = agroup;
	converter_owner = owner;

	if( name_arg_index >= 0 )
	{
		node->RequireArgsMin(name_arg_index+1);
		name = node->args[name_arg_index];
	}
	file_name = name;
	display_name = name;

	full_name = name;
	for( GfxAssetGroup *gr=agroup; gr; gr=gr->parent )
		full_name = gr->name + "/" + full_name;
}
