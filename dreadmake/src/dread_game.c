
#include "demo.h"



const byte RAND_TAB[256] = {
#include "gen/gen_rand.inc"
};




void Machine_UpdateConditions(void)
{
	const DataCondition *mc = e_map_conditions;
	byte *mc_value = e_condition_states + 1;
	while( mc->op!=0xFFFF )
	{
		switch( mc->op )
		{
		//case 0:	*mc_value = (e_machines[mc->c1].position < 0x4000);								break;
		case 0:	*mc_value = (byte)(e_machines[mc->c1].blocker->state);							break;
		case 1:	*mc_value = (e_condition_states+1)[mc->c1] & (e_condition_states+1)[mc->c2];	break;
		case 2:	*mc_value = (e_condition_states+1)[mc->c1] | (e_condition_states+1)[mc->c2];	break;
		}
		mc++;
		mc_value++;
	}

}

void Machine_InitDoor(EngineMachine *m)
{
	EngineLine *line0 = e_lines + (int)m->line_id;
	m->line_len = line0[1].xcoord2 - line0[1].xcoord1;
}

void Machine_UpdateDoorGeometry(EngineThing *th)
{
	EngineMachine *machine = e_machines + (int)th->machine_id;
	EngineLine *line0 = e_lines + (int)machine->line_id;
	EngineVertex *v = line0->v1;
	v[0].xp = v[2].xp + (((v[4].xp-v[2].xp)*(int)th->machine_position) >> 14);
	v[0].yp = v[2].yp + (((v[4].yp-v[2].yp)*(int)th->machine_position) >> 14);
	v[1].xp = v[3].xp + (((v[5].xp-v[3].xp)*(int)th->machine_position) >> 14);
	v[1].yp = v[3].yp + (((v[5].yp-v[3].yp)*(int)th->machine_position) >> 14);
//	word texlen = th->machine_position >> 8;
//	line0[1].xcoord1 = 64 - texlen;
//	line0[2].xcoord1 = 49;
//	line0[2].xcoord2 = 49 + texlen;

	word texlen = (word)(mulu(machine->line_len,th->machine_position) >> 14);
	line0[1].xcoord1 = line0[1].xcoord2 - texlen;
	line0[2].xcoord2 = line0[2].xcoord1 + texlen;

	short xmin=v[0].xp, ymin=v[0].yp;
	short xmax=xmin, ymax=ymin;
	for( int i=1; i<=3; i++ )
	{
		short x = v[i].xp;
		if( x<xmin ) xmin = x;
		if( x>xmax ) xmax = x;
		short y = v[i].yp;
		if( y<ymin ) ymin = y;
		if( y>ymax ) ymax = y;
	}
	th->block_x1 = (xmin - ENGINE_PLAYER_SIZE) << ENGINE_SUBVERTEX_PRECISION;
	th->block_y1 = (ymin - ENGINE_PLAYER_SIZE) << ENGINE_SUBVERTEX_PRECISION;
	th->block_x2 = (xmax + ENGINE_PLAYER_SIZE) << ENGINE_SUBVERTEX_PRECISION;
	th->block_y2 = (ymax + ENGINE_PLAYER_SIZE) << ENGINE_SUBVERTEX_PRECISION;
}


void Machine_LineScroll(__a6 EngineThing *th, __d7 short xoffs)
{
	EngineMachine *machine = e_machines + (int)th->machine_id;
	EngineLine *line = e_lines + (int)machine->line_id;
	word newx1 = (line->xcoord1 + xoffs) & 63;
	short delta = (short)(newx1 - line->xcoord1);
	line->xcoord1 += delta;
	line->xcoord2 += delta;
}


void Machine_ThingUpdate(__a6 EngineThing *th, __d7 short script_tick)
{
	th->machine_position += th->machine_speed * script_tick;
	if( th->machine_position <= 0 )
	{
		th->machine_position = 0;
		th->machine_speed = 0;
		if( !th->state )
		{
			th->state = 1;
			e_globals.update_conditions = 1;
		}
	}
	else if( th->machine_position >= 0x4000 )
	{
		th->machine_position = 0x4000;
		th->machine_speed = 0;
		if( th->state )
		{
			th->state = 0;
			e_globals.update_conditions = 1;
		}
	}
	else
	{
		if( !th->state )
		{
			th->state = 1;
			e_globals.update_conditions = 1;
		}
	}

	Machine_UpdateDoorGeometry(th);
}

void Machine_Activate(int xp, int yp)
{
	EngineMachine *best_m = NULL;
	int best_len = 0x7FFFFFFF;
	
	xp <<= 1;
	yp <<= 1;

	int forward_x = view_rotate_dy>>4;
	int forward_y = view_rotate_dx>>4;

	for( int i=0; i<e_n_machines; i++ )
	{
		EngineMachine *m = e_machines + i;
		EngineLine *line0 = e_lines + (int)m->line_id;
		EngineVertex *v = line0->v1;
		int dx = v[0].xp + v[1].xp - xp;
		int dy = v[0].yp + v[1].yp - yp;
		int len = dx;
		if( len < 0 ) len = -len;
		if( dy > len ) len = dy;
		if( -dy > len ) len = -dy;

		if( len < best_len )
		{
			if( dx*forward_x + dy*forward_y > 0 )
			{
				best_len = len;
				best_m = m;
			}
		}
	}

	if( best_len < 128*2 )
	{
		e_globals.pulse = 1;
		Script_OnPulse(best_m->blocker);
	}
}

void Player_JumpToLocation(__d7 word id)
{
	const DataLocation *loc = Map_GetLocation(id);
	if( !loc ) return;

	view_fine_pos_x = ((int)loc->xp) << 16;
	view_fine_pos_y = ((int)loc->yp) << 16;
	view_angle = loc->angle & 0xFF;
	view_angle_fine = (view_angle<<8) + 128;
	view_subsector = NULL;
}

void Player_ApplyLocationDelta(__d7 word id1, __d6 word id2)
{
	const DataLocation *loc1 = Map_GetLocation(id1);
	const DataLocation *loc2 = Map_GetLocation(id2);
	if( !loc1 || !loc2 ) return;

	int delta_x = view_fine_pos_x - ((int)loc1->xp << 16);
	int delta_y = view_fine_pos_y - ((int)loc1->yp << 16);
	word delta_a = (loc2->angle - loc1->angle) & 0xFF;

	int tmp;
	switch( delta_a )
	{
	case 0x40:
		tmp = delta_x;			delta_x = delta_y;					delta_y = -tmp;	
		tmp = view_delta_x;		view_delta_x = view_delta_y;		view_delta_y = -tmp;
		break;
	case 0x80:
		delta_x = -delta_x;				delta_y = -delta_y;
		view_delta_x = -view_delta_x;	view_delta_y = -view_delta_y;
		break;
	case 0xC0:
		tmp = delta_x;			delta_x = -delta_y;				delta_y = tmp;
		tmp = view_delta_x;		view_delta_x = -view_delta_y;	view_delta_y = tmp;
		break;
	}

	view_fine_pos_x = delta_x + ((int)loc2->xp << 16);
	view_fine_pos_y = delta_y + ((int)loc2->yp << 16);
	view_angle_fine += delta_a << 8;
	view_angle = view_angle_fine >> 8;		// "& 0xFF" not needed after the shift
	view_subsector = NULL;
}

int Game_TagCheck(__d0 word tag)
{
	if( !tag ) return 1;
	word mode = tag/100;
	tag %= 100;
	tag--;

	if( mode <= 9 )
	{
		// Mode:
		//	 0	- normal
		//	 1	- invert
		//	 2	- dual - all
		//	 3	- dual - not all
		//	 4	- dual - none
		//	 5	- dual - any
		//	 6	- triple - all
		//	 7	- triple - not all
		//	 8	- triple - none
		//	 9	- triple - any
		static byte MODE_MASK[] = { 1, 3, 3, 7, 7 };
		static byte MODE_REF[]  = { 1, 3, 0, 7, 0 };
		if( mode & 1 )
		{
			mode >>= 1;
			return ((e_globals.switch_flags >> tag) & MODE_MASK[mode]) != MODE_REF[mode];
		}
		else
		{
			mode >>= 1;
			return ((e_globals.switch_flags >> tag) & MODE_MASK[mode]) == MODE_REF[mode];
		}
	}
	return 0;	//(e_globals.switch_flags >> tag) & 1;
}
