
#include "common.h"
#include "app_rendertest.h"




// Clips segment specified by bspclip_p[1-2][xy] by offset collision BSP tree.
//	Input:
//		bspclip_p1[xy]		- start point
//		bspclip_p2[xy]		- end point
//
//	Results:
//		return value		- 0: no collision			(no meaningful data in bspclip_*)
//							  1: there was a collision	(below data applies)
//		bspclip_p1[xy]		- collision point
//		bspclip_n[ABC]		- collision line	(flipped, if neccessary)
//
short DreadSim::BspClipFix(short node_ref)
{
	while( node_ref>=0 )
	{
		//const DataBSPNode *node = (const DataBSPNode*)(((const byte*)e_map_bspnodes) + node_ref);
		const DataBSPNode *node = access_array_byte_offset(e_map_bspnodes, node_ref);

		int s1 = muls(bspclip_p1x, node->A) + muls(bspclip_p1y, node->B) + node->C;
		int s2 = muls(bspclip_p2x, node->A) + muls(bspclip_p2y, node->B) + node->C;
		if( s1<=0 && s2<=0 )
		{
			node_ref = node->left;
			continue;
		}
		if( s1>=0 && s2>=0 )
		{
			node_ref = node->right;
			continue;
		}

		// Line on both sides - must clip
		short tx = bspclip_p2x;
		short ty = bspclip_p2y;
		bspclip_p2x = bspclip_p1x + (bspclip_p2x-bspclip_p1x)*s1/(s1-s2);
		bspclip_p2y = bspclip_p1y + (bspclip_p2y-bspclip_p1y)*s1/(s1-s2);

		if( s1<=0 && s2>=0 )
		{
			if( node->A<0 ) bspclip_p2x++;
			if( node->A>0 ) bspclip_p2x--;
			if( node->B<0 ) bspclip_p2y++;
			if( node->B>0 ) bspclip_p2y--;

			if( BspClipFix(node->left) )
				return 1;
			bspclip_stuck = 0;
			bspclip_p1x = bspclip_p2x;
			bspclip_p1y = bspclip_p2y;
			bspclip_p2x = tx;
			bspclip_p2y = ty;
			bspclip_nA = node->A;
			bspclip_nB = node->B;
			bspclip_nC = node->C;
			node_ref = node->right;
		}
		else
		{
			if( node->A<0 ) bspclip_p2x--;
			if( node->A>0 ) bspclip_p2x++;
			if( node->B<0 ) bspclip_p2y--;
			if( node->B>0 ) bspclip_p2y++;

			if( BspClipFix(node->right) )
				return 1;
			bspclip_stuck = 0;
			bspclip_p1x = bspclip_p2x;
			bspclip_p1y = bspclip_p2y;
			bspclip_p2x = tx;
			bspclip_p2y = ty;
			bspclip_nA = -node->A;
			bspclip_nB = -node->B;
			bspclip_nC = -node->C;
			node_ref = node->left;
		}
	}

	return ~node_ref;		// 0: empty, 1: solid
}

short DreadSim::BspClipFix_Coarse(short node_ref)
{
	while( node_ref>=0 )
	{
		//const DataBSPNode *node = (const DataBSPNode*)(((const byte*)e_map_bspnodes) + node_ref);
		const DataBSPNode *node = access_array_byte_offset(e_map_bspnodes, node_ref);

		int s1 = muls(bspclip_p1x, node->A) + muls(bspclip_p1y, node->B) + node->C;
		int s2 = muls(bspclip_p2x, node->A) + muls(bspclip_p2y, node->B) + node->C;
		if( s1<=0 && s2<=0 )
		{
			node_ref = node->left;
			continue;
		}
		if( s1>=0 && s2>=0 )
		{
			node_ref = node->right;
			continue;
		}

		// Line on both sides - must clip
		short tx = bspclip_p2x;
		short ty = bspclip_p2y;
		bspclip_p2x = bspclip_p1x + (bspclip_p2x-bspclip_p1x)*s1/(s1-s2);
		bspclip_p2y = bspclip_p1y + (bspclip_p2y-bspclip_p1y)*s1/(s1-s2);

		if( s1<=0 && s2>=0 )
		{
			if( BspClipFix(node->left) )
				return 1;
			bspclip_stuck = 0;
			bspclip_p1x = bspclip_p2x;
			bspclip_p1y = bspclip_p2y;
			bspclip_p2x = tx;
			bspclip_p2y = ty;
			bspclip_nA = node->A;
			bspclip_nB = node->B;
			bspclip_nC = node->C;
			node_ref = node->right;
		}
		else
		{
			if( BspClipFix(node->right) )
				return 1;
			bspclip_stuck = 0;
			bspclip_p1x = bspclip_p2x;
			bspclip_p1y = bspclip_p2y;
			bspclip_p2x = tx;
			bspclip_p2y = ty;
			bspclip_nA = -node->A;
			bspclip_nB = -node->B;
			bspclip_nC = -node->C;
			node_ref = node->left;
		}
	}

	return ~node_ref;		// 0: empty, 1: solid
}

// Check if the <bspclip_p1[xy]> is placed in a wall.
//
int DreadSim::BspClipCheckFix(short node_ref)
{
	while( node_ref>=0 )
	{
		//const DataBSPNode *node = (const DataBSPNode*)(((const byte*)e_map_bspnodes) + node_ref);
		const DataBSPNode *node = access_array_byte_offset(e_map_bspnodes, node_ref);

		int s1 = muls(bspclip_p1x, node->A) + muls(bspclip_p1y, node->B) + node->C;
		if( s1==0 && BspClipCheckFix(node->right) ) return 1;
		node_ref = (s1<=0) ? node->left : node->right;
	}

	return ~node_ref;		// 0: empty, 1: solid
}


short DreadSim::Physics_WalkSlide(short startx, short starty, short endx, short endy)
{
	bspclip_p1x = startx;
	bspclip_p1y = starty;
	bspclip_p2x = endx;
	bspclip_p2y = endy;
	bspclip_stuck = 1;

	short th_collision = 0;	//BspClipThings(1);
	endx = bspclip_p2x;
	endy = bspclip_p2y;

	if( !BspClipFix(e_map_header->map_collision_bsp_root) || bspclip_stuck )
	{
		bspclip_p1x = endx;
		bspclip_p1y = endy;
		if( BspClipCheckFix(e_map_header->map_collision_bsp_root) )
		{
			midpoint_endx = physics_endx = startx;
			midpoint_endy = physics_endy = starty;
		}
		else
		{
			midpoint_endx = physics_endx = endx;
			midpoint_endy = physics_endy = endy;
		}
		return th_collision ? 1 : 0;
	}
	midpoint_endx = bspclip_p1x;
	midpoint_endy = bspclip_p1y;

	// Wall slide
	short clipx = bspclip_p1x;
	short clipy = bspclip_p1y;
	int es = muls(endx, bspclip_nA) + muls(endy, bspclip_nB) + bspclip_nC;
	int ln = muls(bspclip_nA, bspclip_nA) + muls(bspclip_nB, bspclip_nB);
	endx -= bspclip_nA*es/ln;
	endy -= bspclip_nB*es/ln;
	if( bspclip_nA<0 ) endx++;
	if( bspclip_nA>0 ) endx--;
	if( bspclip_nB<0 ) endy++;
	if( bspclip_nB>0 ) endy--;

	bspclip_p1x = startx;
	bspclip_p1y = starty;
	bspclip_p2x = endx;
	bspclip_p2y = endy;

	//BspClipThings(0);
	endx = bspclip_p2x;
	endy = bspclip_p2y;

	if( BspClipFix(e_map_header->map_collision_bsp_root) )
	{
		if( BspClipCheckFix(e_map_header->map_collision_bsp_root) )
		{
			// Slide ended in stuck position
			bspclip_p1x = clipx;
			bspclip_p1y = clipy;
			if( BspClipCheckFix(e_map_header->map_collision_bsp_root) )
			{
				physics_endx = startx;
				physics_endy = starty;
			}
			else
			{
				physics_endx = clipx;
				physics_endy = clipy;
			}
		}
		else
		{
			// Slide clipped, but still fine
			physics_endx = bspclip_p1x;
			physics_endy = bspclip_p1y;
		}
	}
	else
	{
		// Slide did not clip
		if( BspClipCheckFix(e_map_header->map_collision_bsp_root) )
		{
			physics_endx = startx;
			physics_endy = starty;
		}
		else
		{
			physics_endx = endx;
			physics_endy = endy;
		}
	}

	return 1;
}


// Clips segment specified by bspclip_p[1-2][xy] by offset collision BSP tree.
//	Input:
//		bspclip_p1[xy]		- start point
//		bspclip_p2[xy]		- end point
//
//	Results:
//		return value		- 0: no collision			(no meaningful data in bspclip_*)
//							  1: there was a collision	(below data applies)
//		bspclip_p1[xy]		- collision point
//		bspclip_n[ABC]		- collision line	(flipped, if neccessary)
//
short DreadSim::New_BspClipFix(short node_ref)
{
	while( node_ref>=0 )
	{
		//const DataBSPNode *node = (const DataBSPNode*)(((const byte*)e_map_bspnodes) + node_ref);
		const DataBSPNode *node = access_array_byte_offset(e_map_bspnodes, node_ref);

		int s1 = muls(bspclip_p1x, node->A) + muls(bspclip_p1y, node->B) + node->C;
		int s2 = muls(bspclip_p2x, node->A) + muls(bspclip_p2y, node->B) + node->C;
		if( s1<=0 && s2<=0 )
		{
			node_ref = node->left;
			continue;
		}
		if( s1>=0 && s2>=0 )
		{
			node_ref = node->right;
			continue;
		}

		// Line on both sides - must clip
		short tx = bspclip_p2x;
		short ty = bspclip_p2y;
		bspclip_p2x = bspclip_p1x + (bspclip_p2x-bspclip_p1x)*s1/(s1-s2);
		bspclip_p2y = bspclip_p1y + (bspclip_p2y-bspclip_p1y)*s1/(s1-s2);

		if( s1<=0 && s2>=0 )
		{
			if( node->A<0 ) bspclip_p2x++;
			if( node->A>0 ) bspclip_p2x--;
			if( node->B<0 ) bspclip_p2y++;
			if( node->B>0 ) bspclip_p2y--;

			if( BspClipFix(node->left) )
				return 1;
			bspclip_stuck = 0;
			bspclip_p1x = bspclip_p2x;
			bspclip_p1y = bspclip_p2y;
			bspclip_p2x = tx;
			bspclip_p2y = ty;
			bspclip_nA = node->A;
			bspclip_nB = node->B;
			bspclip_nC = node->C;
			node_ref = node->right;
		}
		else
		{
			if( node->A<0 ) bspclip_p2x--;
			if( node->A>0 ) bspclip_p2x++;
			if( node->B<0 ) bspclip_p2y--;
			if( node->B>0 ) bspclip_p2y++;

			if( BspClipFix(node->right) )
				return 1;
			bspclip_stuck = 0;
			bspclip_p1x = bspclip_p2x;
			bspclip_p1y = bspclip_p2y;
			bspclip_p2x = tx;
			bspclip_p2y = ty;
			bspclip_nA = -node->A;
			bspclip_nB = -node->B;
			bspclip_nC = -node->C;
			node_ref = node->left;
		}
	}

	return ~node_ref;		// 0: empty, 1: solid
}

short DreadSim::New_Physics_WalkSlide(short startx, short starty, short endx, short endy)
{
	return 0;
}
