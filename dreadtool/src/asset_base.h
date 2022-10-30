
#pragma once

class GfxAsset;
class GfxAnimation;
class GfxAssetGroup;
class GfxConverter;


enum platform_t {
	PLATFORM_AMIGA = 0,
	PLATFORM_ST,
};


struct AssetExportContext {
	FILE			*fp = NULL;
	DataBlockSet	*dbs = NULL;
	int				exported_bytes = 0;
	platform_t		platform = PLATFORM_AMIGA;

	vector<string>	info_table;
};


class AssetBase : public ToolObject {
public:
	static const char TYPE_NAME[];

	weak_ptr<DefNode>			src_node;
	vector<weak_ptr<DefNode> >	extra_options;
	string						name;
	string						file_name;
	string						full_name;
	string						display_name;
	GfxAssetGroup				*asset_group = NULL;
	GfxConverter				*converter_owner = NULL;


	virtual const char *GetAssetClassName()		{ return TYPE_NAME; }
	virtual void Error(const string &msg);
	virtual bool ImportAssets()					{ return true;		}
	virtual bool DrawPreview()					{ return false;		}
	virtual void OnClick() {}

	virtual void ExportGraphics(AssetExportContext &ctx) {}


protected:
	
	void InitLoad(DefNode *node, int name_arg_index, GfxAssetGroup *agroup, GfxConverter *owner);
};
