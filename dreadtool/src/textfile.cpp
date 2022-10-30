
#include "common.h"

bool TextFile::Load(const char *path)
{
	file_path = string(path);
	timestamp = GetFileTime(file_path.c_str());
	modified = false;
	return NFS.GetFileLines(file_path.c_str(), lines);
}

bool TextFile::Save()
{
	return SaveAs(file_path.c_str());
}

bool TextFile::SaveAs(const char *path)
{
	while(lines.size()>0 && lines.back().size()==0)
		lines.pop_back();

	file_path = string(path);
	if(!NFS.WriteFileLines(file_path.c_str(), lines))
		return false;
	timestamp = GetFileTime(file_path.c_str());
	modified = false;
	return true;
}

void TextFile::SaveIfModified()
{
	if(modified)
		Save();
}

bool TextFile::ReloadIfChanged()
{
	unsigned long long tf = GetFileTime(file_path.c_str());
	if(tf>timestamp)
		return Load(file_path.c_str());
	return false;
}
