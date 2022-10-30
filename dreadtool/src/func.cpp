

#include "common.h"


int BinaryWordData::WriteTable(FILE *fp, const char *name, bool chipmem)
{
	if( fp )
	{
		fprintf(fp, "\n%s word %s[] = {", chipmem ? "__chip" : "const", name);
		int len = 0;
		for( int i=0; i<(int)data.size(); i++ )
		{
			auto pn = newline.find(i);
			auto pc = comments.find(i);

			if( len>=maxlen ) len = 0;
			if( pn!=newline.end() && pn->second ) len = 0;
			if( pc!=comments.end() )
			{
				fprintf(fp, "%s", pc->second.c_str());
				len = 0;
			}

			if( len==0 ) fprintf(fp, "\n/*$%03X*/\t", i*2);

			fprintf(fp, "0x%04X,", data[i]);
			len++;
		}
		fprintf(fp, "\n};\n");
	}

	return data.size()*2;
}
