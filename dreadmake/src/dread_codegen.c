
#include "demo.h"



word SCALER_BUFFER[85802/2+200];
word FUNC_COLORFILL[ENGINE_Y_MAX+1];
word FUNC_SKYCOPY[ENGINE_Y_MAX+1];
word *FN_SCALERS[NUM_SOFT_SIZES];
word *FN_COLORFILL_START;
word *FN_COLORFILL_MID;
word *FN_COLORFILL_END;
word *FN_SKYCOPY;

LineSizeCheat LINE_CHEAT_BUFFER[160];
LineSizeCheat *SIZE_LINE_CHEAT[NUM_SOFT_SIZES];





static word *Dread_Build_Scaler_One(word *dst, int size)
{
	// y1 = ENGINE_Y_MID + ((size*h1)>>8)				-  y1 is topmost (first) pixel which gets texcoord h1
	//
	//	for each Y>=y1	->	h>=h1

	for( int y=0; y<ENGINE_Y_MAX; y++ )
	{
		// (size*h)>>8 = y-ENGINE_Y_MID;				// given the <y>, choose largest <h> that satisfies the equation
		int ty;

		if( y>=ENGINE_Y_MID )
		{
			ty = (((y-ENGINE_Y_MID)<<8) | 0xFF)/size;
		}
		else
		{
			//	int yv = y-ENGINE_Y_MID;
			//	int hibitsneg = (-yv)-1;			// upper bits of multiplication result	(inverted)
			//	int target = hibitsneg<<8;			// obtain this value or higher	(pick lowest <minus_h>)
			//	
			//	// inverted multiplication result:
			//	//	size*minus_h-1 >= target
			//	//	minus_h >= (target+1)/size				// round up
			//	//	minus_h >= (target+1+size-1)/size
			//	//	minus_h >= target/size+1
			//	out[y] = -(target/size+1);


			ty = -1 - ((ENGINE_Y_MID-1-y)<<8)/size;
		}
#if STRETCH_SCALING
		ty >>= 1;
#endif


		// Input:
		//	A0		- return address
		//	A2		- texture pointer
		//	A7		- destination write pointer
		//

		// texture
		*dst++ = 0x1EEA;		// 0001 111 011 101 010			MOVE.b	xxx(A2), (A7)+					MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
								// 0001 1110 1110 1010
		*dst++ = ty;
	}

	*dst++ = 0x4ED6;		// 0100 111 01 1 010 110		JMP (A6)			0100 1110 1101 0000

	return dst;
}

void Dread_Build_Scalers(void)
{
	// Scale size map

	LineSizeCheat *line_cheat = LINE_CHEAT_BUFFER;
	word *ptr = SCALER_BUFFER;
	int start = 1;
	int srem = 0;		// fixed.8
	while( start<NUM_SOFT_SIZES )
	{
		int regsize = (start<<8)/30 + srem;		// fixed.8
		if( regsize<256 ) regsize = 256;
		int ireg = regsize >> 8;
		srem = regsize - (ireg<<8);
		if( srem<0 ) srem = 0;

		int end = start + ireg;
		if( end > NUM_SOFT_SIZES )
			end = NUM_SOFT_SIZES;

		int size = (start+(end-1))/2;
		while( start<end )
		{
			FN_SCALERS[start] = ptr;
			SIZE_LINE_CHEAT[start] = line_cheat;
			start++;
		}

		// Build Line cheat
		const int view_pos_z = -40;
		{
			const int top = -ENGINE_Y_MID;
			const int end = ENGINE_Y_MAX-ENGINE_Y_MID;
			int m128 = ((-128 - view_pos_z)*size)>>8;
			int m96	 = (( -96 - view_pos_z)*size)>>8;
			int m64	 = (( -64 - view_pos_z)*size)>>8;
			int z0	 = ((   0 - view_pos_z)*size)>>8;
#define CLAMP(y)		if(y<-ENGINE_Y_MID) y=-ENGINE_Y_MID; if(y>ENGINE_Y_MAX-ENGINE_Y_MID) y=ENGINE_Y_MAX-ENGINE_Y_MID;
			CLAMP(m128);
			CLAMP(m96);
			CLAMP(m64);
			CLAMP(z0);
#undef CLAMP

			// Cheat types:
			//	short ceil_len_<X>			= -( <X> - top)*2
			//	word *upper_start_<X>		= ptr + ( ENGINE_Y_MID + <X> )*2
			//	short upper_len_<X>_<Y>		= ( <Y> - <X> )*4
			//	short floor_start_<X>		= ( <X> - top )*2		// floor_start = -ceil_len
			//	short hole_size_<X>_<Y>		= ( <Y> - <X> )*2		// hole_size*2 = upper_len
			//	short ceil_ymin_<X>			= ( <X> - top )*2		// ceil_ymin = floor_start						OPTIMIZE
			//	short floor_length_<X>		= ( end - <X> )*2		// floor_length = ceil_len + (end-top)*2		RECHECK??
			//

			line_cheat->ceil_len_m64		= -( m64 - top)*2;					//	 +2
			line_cheat->ceil_len_m128		= -(m128 - top)*2;					//	 +4
			line_cheat->upper_start_m64		= ptr + (ENGINE_Y_MID+m64)*2;		//	 +6
			line_cheat->upper_start_m128	= ptr + (ENGINE_Y_MID+m128)*2;		//	+10
			line_cheat->upper_len_m64_0		= ( z0- m64)*4;						//	+14
			line_cheat->upper_len_m128_0	= ( z0-m128)*4;						//	+16
			line_cheat->upper_len_m128_m64	= (m64-m128)*4;						//	+18
			line_cheat->floor_start_0		= (z0 - top)*2;						//	+20
			line_cheat->hole_size_m64_0		= (z0 - m64)*2;						//	+22
			line_cheat->ceil_ymin_m64		= (m64 - top)*2;					//	+24
			line_cheat->floor_length_0		= (end - z0)*2;						//	+26
			line_cheat->real_size			= size<<(4+STRETCH_SCALING);		//	+28
			line_cheat->scaler_fn			= ptr + ENGINE_Y_MID*2;				//	+30
			line_cheat->upper_endpos_0		= z0*4;								//	+34
			line_cheat->upper_endpos_m64	= m64*4;							//	+36
			line_cheat->upper_len_m96_0 	= (z0- m96)*4;						//	+38
			line_cheat->upper_start_m96		= ptr + (ENGINE_Y_MID+m96)*2;		//	+40
			line_cheat->ceil_len_m96		= -(m96 - top)*2;					//	+44
			line_cheat->upper_len_m96_m64	= (m64-m96)*4;						//	+46
			line_cheat->upper_endpos_m96	= m96*4;							//	+48
			line_cheat->hole_size_m96_0		= (z0- m96)*2;						//	+50
			line_cheat->ceil_ymin_m96		= (m96 - top)*2;					//	+52
			line_cheat->upper_len_m128_m96	= (m96-m128)*4;						//	+54
			line_cheat->hole_size_m128_0	= (z0- m128)*2;						//	+56
			line_cheat->ceil_ymin_m128		= (m128 - top)*2;					//	+58
			line_cheat->hole_end_0			= (z0 - top)*2;						//	+60
																				// 62 bytes
		}
		line_cheat++;

		// Build scaler
		ptr = Dread_Build_Scaler_One(ptr, size);
	}

	//*ptr++ = 0x4E75;		// RTS
	*ptr++ = 0x4ED6;		// 0100 111 01 1 010 110		JMP (A6)			0100 1110 1101 0000
}

void Dread_Build_Fillers(void)
{
	// Color filler
	word *ptr = FUNC_COLORFILL;
	FN_COLORFILL_START = ptr;
	FN_COLORFILL_MID = ptr + ENGINE_Y_MID;
	FN_COLORFILL_END = ptr + ENGINE_Y_MAX;
	{
		// Input:
		//	D5		- fill color
		//	A0		- return address
		//	A7		- destination write pointer
		//

		for(int y=0;y<ENGINE_Y_MAX;y++)
		{
			*ptr++ = 0x1EC5;		// 0001 111 011 000 101			MOVE.b	D5, (A7)+							MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
									// 0001 1110 1100 0101
		}

		*ptr++ = 0x4ED6;		// 0100 111 01 1 010 000		JMP (A6)			0100 1110 1101 0000
	}

	// Sky filler
	ptr = FUNC_SKYCOPY;
	FN_SKYCOPY = ptr + ENGINE_Y_MID;
	{
		for( int y=0; y<ENGINE_Y_MAX; y++ )
		{
			*ptr++ = 0x1EDA;		// 0001 111 011 011 010			MOVE.b	(A2)+, (A7)+						MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
									// 0001 1110 1101 1010
		}

		*ptr++ = 0x4ED6;		// 0100 111 01 1 010 110		JMP (A6)			0100 1110 1101 0000
	}
}
