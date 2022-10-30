
//#include "e_common.h"


//	EngineMapData e_mapdata;
//	
//	
//	void E_LoadMapFile(MapFileHeader *mapfile)
//	{
//		e_mapdata.n_bsp_nodes = mapfile->n_nodes;
//		e_mapdata.n_vertexes = mapfile->n_vertexes;
//		e_mapdata.n_lines = mapfile->n_lines;
//		e_mapdata.n_sectors = mapfile->n_sectors;
//	
//		// Load nodes
//		for( int i=0; i<e_mapdata.n_bsp_nodes; i++ )
//		{
//			MapFileNode *mn = mapfile->nodes + i;
//			EngineBSPNode *en = e_mapdata.bsp_nodes + i;
//			en->A = mn->A;
//			en->B = mn->B;
//			en->C = mn->C;
//			en->right = (mn->right>=0 ? e_mapdata.bsp_nodes + mn->right : e_mapdata.sectors + (mn->right & 0x7FFF));
//			en->left = (mn->left>=0 ? e_mapdata.bsp_nodes + mn->left : e_mapdata.sectors + (mn->left & 0x7FFF));
//		}
//	
//		// Load vertexes
//		for( int i=0; i<e_mapdata.n_vertexes; i++ )
//		{
//			MapFileVertex *mv = mapfile->vertexes + i;
//			EngineVertex *ev = e_mapdata.vertexes + i;
//			ev->xp = mv->xp;
//			ev->yp = mv->yp;
//			ev->tr_x = 0;
//		}
//	}
