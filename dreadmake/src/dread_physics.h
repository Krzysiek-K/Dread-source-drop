
#ifndef _DREAD_PHYSICS_H
#define _DREAD_PHYSICS_H


extern short physics_endx;
extern short physics_endy;


// Length of(x, y) int vector rounded to nearest integer.
int line_len(int dx, int dy);

// Length of(x, y) int vector as 24.8 fixpoint (1.12% max error).
int line_len_fix8(int dx, int dy);



// Return value:
//	0	- free movement
//	1	- obstacle hit
short Physics_WalkSlide(short startx, short starty, short endx, short endy);



#endif
