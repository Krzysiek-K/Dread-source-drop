
#include <stdio.h>

#include "common.h"
#include "wad.h"

using namespace std;



static bool compare_names(const char *a,const char *b)
{
	for(int i=0;i<8;i++,a++,b++)
	{
		if(*a!=*b) return false;
		if(!*a) return true;
	}
	return true;
}

static void dump_raw(const char *name,void *data,int size)
{
	FILE *fp = fopen(name,"wb");
	if(fp)
	{
		fwrite(data,1,size,fp);
		fclose(fp);
	}
}


bool WadFile::Load(const char *path,bool merge)
{
	if(!merge)
	{
		file_data.clear();
		lumps.clear();
	}

	int base = file_data.size();

	FILE *fp = fopen(path,"rb");
	if(!fp) return false;

	byte buffer[1024*32];
	int len;
	while( (len=fread(buffer,1,sizeof(buffer),fp))>0 )
		file_data.insert(file_data.end(),buffer,buffer+len);

	fclose(fp);

	//printf("%d bytes read.\n",file_data.size());

	Header *h = (Header*)&file_data[base];
	for(int i=0;i<h->n_lumps;i++)
	{
		Lump lump = *(Lump*)&file_data[h->lump_ofs + sizeof(Lump)*i + base];
		lump.fpos += base;
		lumps.push_back(lump);
	}

	return true;
}

// TBD: throw error string
void WadFile::GetMap(const char *name,MapInfo *m)
{
#define E(msg)	do{ throw format("Map %s load error: %s.\n",name,msg); }while(0)
	if(!FindLump(name)) E("not found");
	if(FindLump("BEHAVIOR",name)) E("BEHAVIOR present");

	Lump *l;

	strcpy(m->name,name);

	if( !(l=FindLump("THINGS", name)) ) E("THINGS not found");
	if( l->fsize % sizeof(Thing) ) E("Invalid THINGS size");
	m->things = (Thing*)&file_data[l->fpos];
	m->n_things = l->fsize/sizeof(Thing);

	if( !(l=FindLump("LINEDEFS",name)) ) E("LINEDEFS not found");
	if( l->fsize % sizeof(LineDef) ) E("Invalid LINEDEFS size");
	m->linedefs = (LineDef*)&file_data[l->fpos];
	m->n_linedefs = l->fsize/sizeof(LineDef);

	if( !(l=FindLump("SIDEDEFS",name)) ) E("SIDEDEFS not found");
	if( l->fsize % sizeof(SideDef) ) E("Invalid SIDEDEFS size");
	m->sidedefs = (SideDef*)&file_data[l->fpos];
	m->n_sidedefs = l->fsize/sizeof(SideDef);

	if( !(l=FindLump("VERTEXES",name)) ) E("VERTEXES not found");
	if( l->fsize % sizeof(Vertex) ) E("Invalid VERTEXES size");
	m->vertexes = (Vertex*)&file_data[l->fpos];
	m->n_vertexes = l->fsize/sizeof(Vertex);

	if( !(l=FindLump("SECTORS",name)) ) E("SECTORS not found");
	if( l->fsize % sizeof(Sector) ) E("Invalid SECTORS size");
	m->sectors = (Sector*)&file_data[l->fpos];
	m->n_sectors = l->fsize/sizeof(Sector);

	if( !(l=FindLump("NODES", name)) ) E("NODES not found");
	if( l->fsize % sizeof(Node) ) E("Invalid NODES size");
	m->nodes = (Node*)&file_data[l->fpos];
	m->n_nodes = l->fsize/sizeof(Node);

	if( !(l=FindLump("SSECTORS", name)) ) E("SSECTORS not found");
	if( l->fsize % sizeof(SubSector) ) E("Invalid SSECTORS size");
	m->subsectors = (SubSector*)&file_data[l->fpos];
	m->n_subsectors = l->fsize/sizeof(SubSector);

	if( !(l=FindLump("SEGS", name)) ) E("SEGS not found");
	if( l->fsize % sizeof(Segment) ) E("Invalid SEGS size");
	m->segments = (Segment*)&file_data[l->fpos];
	m->n_segments = l->fsize/sizeof(Segment);

#undef E
}

bool WadFile::GetZMap(const char *name,ZMapInfo *m)
{
	if(!FindLump(name)) return false;
	if(!FindLump("BEHAVIOR",name)) return false;

	Lump *l;

	strcpy(m->name,name);

	if( !(l=FindLump("THINGS",name)) ) return false;
	m->things = (ZThing*)&file_data[l->fpos];
	m->n_things = l->fsize/sizeof(ZThing);

	if( !(l=FindLump("LINEDEFS",name)) ) return false;
	m->linedefs = (ZLineDef*)&file_data[l->fpos];
	m->n_linedefs = l->fsize/sizeof(ZLineDef);

	if( !(l=FindLump("SIDEDEFS",name)) ) return false;
	m->sidedefs = (SideDef*)&file_data[l->fpos];
	m->n_sidedefs = l->fsize/sizeof(SideDef);

	if( !(l=FindLump("VERTEXES",name)) ) return false;
	m->vertexes = (Vertex*)&file_data[l->fpos];
	m->n_vertexes = l->fsize/sizeof(Vertex);

	if( !(l=FindLump("SECTORS",name)) ) return false;
	m->sectors = (Sector*)&file_data[l->fpos];
	m->n_sectors = l->fsize/sizeof(Sector);

	return true;
}

bool WadFile::ReadPatch(const char *name,int &width,int &height,int &offs_x,int &offs_y,vector<int> &data)
{
	Lump *l = FindLump(name);
	if(!l)
	{
		width = 0;
		height = 0;
		offs_x = 0;
		offs_y = 0;
		data.clear();
		return false;
	}

	LoadPalette();

	unsigned char *base = &file_data[l->fpos];
	width  = *(short*)(base+0);
	height = *(short*)(base+2);
	offs_x = *(short*)(base+4);
	offs_y = *(short*)(base+8);
	data.clear();
	data.resize(width*height);

//	dump_raw("_patch",&file_data[l->fpos],l->fsize);

	for(int x=0;x<width;x++)
	{
		unsigned char *p = base + *(int*)(base+(8+4*x));
		int y = 0;
		while(*p!=0xFF && y<height)
		{
			y = *p++;
			int cnt = *p++;
			p++;
			while(y<height && cnt-->0)
				data[x+(y++)*width] = palette[*p++];
			p++;
		}
	}

	return true;
}

bool WadFile::ReadTexture(const char *name,int &width,int &height,std::vector<int> &data)
{
	struct GfxTexture {
		char	name[8];	//  8B
		int		masked;		//  4B
		word	width;		//  2B
		word	height;		//  2B
		int		coldir;		//  4B
		word	patchcount;	//  2B
							// 22B total
	};

	static const char *LUMPS[] = { "TEXTURE1", "TEXTURE2", NULL };
	GfxTexture *texture = NULL;

	for(const char **ln = LUMPS;*ln;ln++)
	{
		Lump *l = FindLump(*ln);
		if(!l) continue;

		unsigned char *base = &file_data[l->fpos];
		int cnt = *(int*)base;

		for(int i=0;i<cnt;i++)
		{
			GfxTexture *t = (GfxTexture*)(base + ((int*)base)[i+1]);
			if(compare_names(t->name,name))
			{
				texture = t;
				break;
			}
		}
		if(texture)
			break;
	}

	Lump *pnl = FindLump("PNAMES");

	if(!texture || !pnl)
		return ReadFlat(name,width,height,data);
	
//	dump_raw("_pnames",&file_data[pnl->fpos],pnl->fsize);
//	dump_raw("_texture",texture,22+10*texture->patchcount);

	width = texture->width;
	height = texture->height;
	data.clear();
	data.resize(width*height);

	short *pd = (short*)(((char*)texture)+22);
	char *pnames = (char*)&file_data[pnl->fpos];
	vector<int> pdata;
	int pwidth,pheight,poffsx,poffsy;
	for(int i=0;i<texture->patchcount;i++,pd+=5)
	{
		char buff[9];
		memcpy(buff,pnames+(pd[2]*8+4),8);
		buff[8] = 0;
		if(!ReadPatch(buff,pwidth,pheight,poffsx,poffsy,pdata))
		{
			printf("Texture %s: patch %s not found.\n",name,buff);
			continue;
		}
		for(int y=0;y<pheight;y++)
			for(int x=0;x<pwidth;x++)
			{
				int nx = x + pd[0];// - poffsx;
				int ny = y + pd[1];// - poffsy;
				if(nx<0 || nx>=width || ny<0 || ny>=height)
					continue;
				if(pdata[x+pwidth*y])
					data[nx+width*ny] = pdata[x+pwidth*y];
			}
	}

	return true;
}

bool WadFile::ReadFlat(const char *name,int &width,int &height,std::vector<int> &data)
{
	Lump *l = FindLump(name,"F_START","F_END");
	if(!l)
	{
		width = 0;
		height = 0;
		data.clear();
		return false;
	}

	width = 64;
	height = 64;
	data.resize(width*height);

	LoadPalette();
	for(int i=0;i<64*64;i++)
		data[i] = palette[file_data[l->fpos+i]];

	return true;
}



WadFile::Lump *WadFile::FindLump(const char *name,const char *dir,const char *dir_end)
{
	bool in_dir = (dir==NULL);
	for(int i=0;i<lumps.size();i++)
	{
		if(in_dir)
		{
			if(compare_names(name,lumps[i].name))
				return &lumps[i];
			if(lumps[i].fsize==0 && dir!=NULL && (dir_end==NULL || compare_names(lumps[i].name,dir_end)))
				return NULL;
		}
		else
			if(lumps[i].fsize==0 && dir && compare_names(lumps[i].name,dir))
				in_dir = true;
	}
	return NULL;
}

void WadFile::LoadPalette()
{
	if(palette.size()==256)
		return;
	palette.resize(256);

	Lump *l = FindLump("PLAYPAL");
	if(!l) return;

	unsigned char *d = &file_data[l->fpos];
	for(int i=0;i<256;i++)
	{
		int c = 0xFF000000;
		c |= (*d++) << 16;
		c |= (*d++) <<  8;
		c |= (*d++);
		palette[i] = c;
	}

}
