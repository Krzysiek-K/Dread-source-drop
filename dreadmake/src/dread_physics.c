
#include "demo.h"


short physics_endx;
short physics_endy;




short bspclip_p1x;
short bspclip_p1y;
short bspclip_p2x;
short bspclip_p2y;
short bspclip_nA;
short bspclip_nB;
int   bspclip_nC;
short bspclip_stuck;





int line_len(int dx, int dy)
{
	//	// Length of(x, y) int vector as 24.8 fixpoint (1.12% max error):
	//	//	x=abs(x);
	//	//	y=abs(y);
	//	//	s=x+y;
	//	//	t=abs(x-y);
	//	//	return s*164 + t*72 + abs(s*15-t*34);
	//	
	//	if( dx<0 ) dx=-dx;
	//	if( dy<0 ) dy=-dy;
	//	int s = dx+dy;
	//	int t = dx-dy;
	//	if( t<0 ) t=-t;
	//	dx = s*15-t*34;
	//	if( dx<0 ) dx=-dx;
	//	int len = s*164 + t*72 + dx;
	//	return (len+128)>>8;
	return lsqrt(dx*dx+dy*dy);
}

int line_len_fix8(int dx, int dy)
{
	if( dx<0 ) dx=-dx;
	if( dy<0 ) dy=-dy;
	int s = dx+dy;
	int t = dx-dy;
	if( t<0 ) t=-t;
	dx = s*15-t*34;
	if( dx<0 ) dx=-dx;
	return s*164 + t*72 + dx;
}










// Clips segment specified by bspclip_p[1-2][xy] by offset collision BSP tree.
//	Results:
//		return value		- 0: no collision			(no meaningful data in bspclip_*)
//							  1: there was a collision	(below data applies)
//		bspclip_p1[xy]		- collision point
//		bspclip_n[ABC]		- collision line	(flipped, if neccessary)
//
short BspClipFix(short node_ref)
{
	while( node_ref>=0 )
	{
		const DataBSPNode *node = (const DataBSPNode*)(((const byte*)e_map_bspnodes) + node_ref);
		
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

short BspClipFix_Coarse(short node_ref)
{
	while( node_ref>=0 )
	{
		const DataBSPNode *node = (const DataBSPNode*)(((const byte*)e_map_bspnodes) + node_ref);

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

int BspClipCheckFix(short node_ref)
{
	while( node_ref>=0 )
	{
		const DataBSPNode *node = (const DataBSPNode*)(((const byte*)e_map_bspnodes) + node_ref);

		int s1 = muls(bspclip_p1x, node->A) + muls(bspclip_p1y, node->B) + node->C;
		if( s1==0 && BspClipCheckFix(node->right) ) return 1;
		node_ref = (s1<=0) ? node->left : node->right;
	}

	return ~node_ref;		// 0: empty, 1: solid
}

// result		- 0: no collision
//				  1: clipped against X
//				  2: clipped against Y
//
short BspClipThings(int slide)
{
	short result = 0;
	short dx = bspclip_p2x - bspclip_p1x;
	short dy = bspclip_p2y - bspclip_p1y;


	EngineThing **ptr = e_visthings;
	while( ptr < e_visthings_end )
	{
		EngineThing *th = *ptr++;
		if( !th->blocker_size ) continue;

		if( dx > 0 )
		{
			short tdx = th->block_x1 - bspclip_p1x;
			if( tdx>=0 && tdx<dx )
			{
				short yclip = bspclip_p1y + divs(muls(dy, tdx), dx);
				if( yclip>=th->block_y1 && yclip<=th->block_y2 )
				{
					dx = tdx;
					if( slide )	ptr = e_visthings;
					else		dy = yclip - bspclip_p1y;
					result = 1;
				}
			}
		}
		else
		{
			short tdx = th->block_x2 - bspclip_p1x;
			if( tdx<=0 && tdx>dx )
			{
				short yclip = bspclip_p1y + divs(muls(dy, tdx), dx);
				if( yclip>=th->block_y1 && yclip<=th->block_y2 )
				{
					dx = tdx;
					if( slide )	ptr = e_visthings;
					else		dy = yclip - bspclip_p1y;
					result = 1;
				}
			}
		}

		if( dy > 0 )
		{
			short tdy = th->block_y1 - bspclip_p1y;
			if( tdy>=0 && tdy<dy )
			{
				short xclip = bspclip_p1x + divs(muls(dx, tdy), dy);
				if( xclip>=th->block_x1 && xclip<=th->block_x2 )
				{
					dy = tdy;
					if( slide )	ptr = e_visthings;
					else		dx = xclip - bspclip_p1x;
					result = 1;
				}
			}
		}
		else
		{
			short tdy = th->block_y2 - bspclip_p1y;
			if( tdy<=0 && tdy>dy )
			{
				short xclip = bspclip_p1x + divs(muls(dx, tdy), dy);
				if( xclip>=th->block_x1 && xclip<=th->block_x2 )
				{
					dy = tdy;
					if( slide )	ptr = e_visthings;
					else		dx = xclip - bspclip_p1x;
					result = 1;
				}
			}
		}
	}
	//if( result )
	//{
	//	if( slide )
	//	{
	//		if( result==1 ) bspclip_p2x = bspclip_p1x + dx;
	//		else		    bspclip_p2y = bspclip_p1y + dy;
	//	}
	//	else
	//	{
	//		bspclip_p2x = bspclip_p1x + dx;
	//		bspclip_p2y = bspclip_p1y + dy;
	//	}
	//}
	bspclip_p2x = bspclip_p1x + dx;
	bspclip_p2y = bspclip_p1y + dy;

	return result;
}


short Physics_MoveCollide(__a6 EngineThing *self, __d7 short script_ticks)
{
	short p1x = self->xp;
	short p1y = self->yp;
	short dx;
	short dy;
	if( (self->step_count -= script_ticks) <= 0 )
	{
		dx = self->step_endx - p1x;
		dy = self->step_endy - p1y;
		self->step_count = 0;
	}
	else
	{
		dx = (short)(muls(self->step_xp, script_ticks)>>3);
		dy = (short)(muls(self->step_yp, script_ticks)>>3);
	}


	EngineThing **ptr = e_visthings;
	short result = 0;	// non-zero result is passed to OnPulse
	while( ptr < e_visthings_end )
	{
		EngineThing *th = *ptr++;
		if( !th->blocker_size || th==self || th==self->owner ) continue;

		if( dx > 0 )
		{
			short tdx = th->block_x1 - p1x;
			if( tdx>=0 && tdx<dx )
			{
				short yclip = p1y + divs(muls(dy, tdx), dx);
				if( yclip>=th->block_y1 && yclip<=th->block_y2 )
				{
					dx = tdx;
					dy = yclip - p1y;
					result = 1;
				}
			}
		}
		else
		{
			short tdx = th->block_x2 - p1x;
			if( tdx<=0 && tdx>dx )
			{
				short yclip = p1y + divs(muls(dy, tdx), dx);
				if( yclip>=th->block_y1 && yclip<=th->block_y2 )
				{
					dx = tdx;
					dy = yclip - p1y;
					result = 1;
				}
			}
		}

		if( dy > 0 )
		{
			short tdy = th->block_y1 - p1y;
			if( tdy>=0 && tdy<dy )
			{
				short xclip = p1x + divs(muls(dx, tdy), dy);
				if( xclip>=th->block_x1 && xclip<=th->block_x2 )
				{
					dy = tdy;
					dx = xclip - p1x;
					result = 1;
				}
			}
		}
		else
		{
			short tdy = th->block_y2 - p1y;
			if( tdy<=0 && tdy>dy )
			{
				short xclip = p1x + divs(muls(dx, tdy), dy);
				if( xclip>=th->block_x1 && xclip<=th->block_x2 )
				{
					dy = tdy;
					dx = xclip - p1x;
					result = 1;
				}
			}
		}
	}

	if (!result)
	{
		if (self->move_collide == 2 )
		{
#define LIMIT	((ENGINE_PLAYER_SIZE*3/2)<<ENGINE_SUBVERTEX_PRECISION)
			// Collide with player
			short tmp = view_pos_x - self->xp;
			if (tmp >= -LIMIT && tmp <= LIMIT)
			{
				tmp = view_pos_y - self->yp;
				if (tmp >= -LIMIT && tmp <= LIMIT)
					result = 2;
			}
#undef LIMIT
		}
		else if (self->move_collide == 3)
		{
			short block_offs = (self->blocker_size + ENGINE_PLAYER_SIZE) << ENGINE_SUBVERTEX_PRECISION;

			if (dx > 0)
			{
				short tdx = (view_pos_x- block_offs) - p1x;
				if (tdx >= 0 && tdx<dx)
				{
					short yclip = p1y + divs(muls(dy, tdx), dx);
					if (yclip >= (view_pos_y - block_offs) && yclip <= (view_pos_y + block_offs))
					{
						dx = tdx;
						dy = yclip - p1y;
						result = 2;
					}
				}
			}
			else
			{
				short tdx = (view_pos_x + block_offs) - p1x;
				if (tdx <= 0 && tdx>dx)
				{
					short yclip = p1y + divs(muls(dy, tdx), dx);
					if (yclip >= (view_pos_y - block_offs) && yclip <= (view_pos_y + block_offs))
					{
						dx = tdx;
						dy = yclip - p1y;
						result = 2;
					}
				}
			}

			if (dy > 0)
			{
				short tdy = (view_pos_y - block_offs) - p1y;
				if (tdy >= 0 && tdy<dy)
				{
					short xclip = p1x + divs(muls(dx, tdy), dy);
					if (xclip >= (view_pos_x - block_offs) && xclip <= (view_pos_x + block_offs))
					{
						dy = tdy;
						dx = xclip - p1x;
						result = 2;
					}
				}
			}
			else
			{
				short tdy = (view_pos_y + block_offs) - p1y;
				if (tdy <= 0 && tdy>dy)
				{
					short xclip = p1x + divs(muls(dx, tdy), dy);
					if (xclip >= (view_pos_x - block_offs) && xclip <= (view_pos_x + block_offs))
					{
						dy = tdy;
						dx = xclip - p1x;
						result = 2;
					}
				}
			}
		}
	}

	self->xp += dx;
	self->yp += dy;

	return result;
}

short Physics_WalkSlide(short startx, short starty, short endx, short endy)
{
	bspclip_p1x = startx;
	bspclip_p1y = starty;
	bspclip_p2x = endx;
	bspclip_p2y = endy;
	bspclip_stuck = 1;

	short th_collision = BspClipThings(1);
	endx = bspclip_p2x;
	endy = bspclip_p2y;

	if( !BspClipFix(e_map_header->map_collision_bsp_root) || bspclip_stuck )
	{
		//physics_endx = endx;
		//physics_endy = endy;
		bspclip_p1x = endx;
		bspclip_p1y = endy;
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
		return th_collision ? 1 : 0;
	}

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

	BspClipThings(0);
	endx = bspclip_p2x;
	endy = bspclip_p2y;

	if( BspClipFix(e_map_header->map_collision_bsp_root) )
	{
		if( BspClipCheckFix(e_map_header->map_collision_bsp_root) )
		{
			// Slide ended in stuck position
			//physics_endx = clipx;
			//physics_endy = clipy;
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
		//physics_endx = endx;
		//physics_endy = endy;

		// Slide did not clip
		bspclip_p1x = endx;
		bspclip_p1y = endy;
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


void Physics_EnemyWalkSlide(__a6 EngineThing *th)
{
	short startx = th->xp;
	short starty = th->yp;
	short endx = th->xp + th->step_xp;
	short endy = th->yp + th->step_yp;

	bspclip_p1x = startx;
	bspclip_p1y = starty;
	bspclip_p2x = endx;
	bspclip_p2y = endy;
	bspclip_stuck = 1;

	if( !BspClipFix(e_map_header->map_collision_bsp_root) || bspclip_stuck )
	{
		bspclip_p1x = endx;
		bspclip_p1y = endy;
		if( BspClipCheckFix(e_map_header->map_collision_bsp_root) )
		{
			// Stuck - move to unstuck pos
			th->step_xp = th->unstuck_x - th->xp;
			th->step_yp = th->unstuck_y - th->yp;
		}
		else
		{
			// All OK - save as safe coord
			th->unstuck_x = endx;
			th->unstuck_y = endy;
		}
		return;
	}

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

	if( BspClipFix(e_map_header->map_collision_bsp_root) )
	{
		// Slide clipped - result in <bspclip_p1[xy]>

		if( BspClipCheckFix(e_map_header->map_collision_bsp_root) )
		{
			// Slide ended in stuck position - consider first clip instead
			bspclip_p1x = clipx;
			bspclip_p1y = clipy;
			if( BspClipCheckFix(e_map_header->map_collision_bsp_root) )
			{
				// Clip is also stuck - move to unstuck pos
				th->step_xp = th->unstuck_x - th->xp;
				th->step_yp = th->unstuck_y - th->yp;
			}
			else
			{
				// First clip is fine
				th->unstuck_x = clipx;
				th->unstuck_y = clipy;
				th->step_xp = clipx - th->xp;
				th->step_yp = clipy - th->yp;
			}
		}
		else
		{
			// Slide clipped, but still fine
			th->unstuck_x = bspclip_p1x;
			th->unstuck_y = bspclip_p1y;
			th->step_xp = bspclip_p1x - th->xp;
			th->step_yp = bspclip_p1y - th->yp;
		}
	}
	else
	{
		// Slide did not clip
		//th->step_xp = endx - th->xp;
		//th->step_yp = endy - th->yp;
		bspclip_p1x = endx;
		bspclip_p1y = endy;
		if( BspClipCheckFix(e_map_header->map_collision_bsp_root) )
		{
			// Move to unstuck pos
			th->step_xp = th->unstuck_x - th->xp;
			th->step_yp = th->unstuck_y - th->yp;
		}
		else
		{
			th->unstuck_x = endx;
			th->unstuck_y = endy;
			th->step_xp = endx - th->xp;
			th->step_yp = endy - th->yp;
		}
	}
}


void Physics_MissileMoveClip(__a6 EngineThing *th)
{
	short startx = th->xp;
	short starty = th->yp;
	short endx = th->xp + th->step_xp;
	short endy = th->yp + th->step_yp;

	bspclip_p1x = startx;
	bspclip_p1y = starty;
	bspclip_p2x = endx;
	bspclip_p2y = endy;
	bspclip_stuck = 1;

	if( !BspClipFix(e_map_header->map_hitscan_bsp_root) )		// map_collision_bsp_root
	{
		th->unstuck_x = th->xp;
		th->unstuck_y = th->yp;
	}
	else if( bspclip_stuck )
	{
		//th->step_xp = th->unstuck_x - th->xp;
		//th->step_yp = th->unstuck_y - th->yp;
		th->step_xp = 0;
		th->step_yp = 0;
	}
	else
	{
		th->step_xp = bspclip_p1x - th->xp;
		th->step_yp = bspclip_p1y - th->yp;
		th->unstuck_x = th->xp;
		th->unstuck_y = th->yp;
	}
}

void Physics_EnemyWalkInit(__a6 EngineThing *th, __d7 int move_speed, __d6 int move_duration)
{
	int numstep = line_len(th->step_xp, th->step_yp)/move_speed;
	th->step_endx = th->xp + th->step_xp;
	th->step_endy = th->yp + th->step_yp;

	if( numstep<=1 )
	{
		th->step_count = 8;
		th->timer_walk = 8;
		return;
	}

	th->step_xp /= numstep;
	th->step_yp /= numstep;
	th->step_count = numstep<<3;
	
	th->timer_walk = (th->step_count < move_duration) ? th->step_count : move_duration;
}


int Physics_Hitscan(__a6 EngineThing *th)
{
	bspclip_p1x = th->xp;
	bspclip_p1y = th->yp;
	bspclip_p2x = view_pos_x;
	bspclip_p2y = view_pos_y;
	bspclip_stuck = 1;
					
	if( BspClipFix(e_map_header->map_hitscan_bsp_root) )
		return 0;
	
	return 1;
}
