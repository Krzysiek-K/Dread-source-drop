
#include "demo.h"



ScalerInfo FN_SCALERS[NUM_SOFT_SIZES];
word SCALER_BUFFER[85802/2+200];
word FILLER_BUFFER[101*(2+4)];
colorfill_fn_t FN_COLORFILL_START;
colorfill_fn_t FN_COLORFILL;
skycopy_fn_t FN_SKYCOPY;

LineSizeCheat LINE_CHEAT_BUFFER[143];
LineSizeCheat *SIZE_LINE_CHEAT[NUM_SOFT_SIZES];





static word *Dread_Build_Scaler_One(word *dst, int size)
{
	// y1 = 50 + ((size*h1)>>8)				-  y1 is topmost (first) pixel which gets texcoord h1
	//
	//	for each Y>=y1	->	h>=h1

	word *fn = dst;
	for( int y=0; y<100; y++ )
	{
		// (size*h)>>8 = y-50;					// given the <y>, choose largest <h> that satisfies the equation
		int ty;

		if( y>=50 )
		{
			ty = (((y-50)<<8) | 0xFF)/size;
		}
		else
		{
			//	int yv = y-50;
			//	int hibitsneg = (-yv)-1;			// upper bits of multiplication result	(inverted)
			//	int target = hibitsneg<<8;			// obtain this value or higher	(pick lowest <minus_h>)
			//	
			//	// inverted multiplication result:
			//	//	size*minus_h-1 >= target
			//	//	minus_h >= (target+1)/size				// round up
			//	//	minus_h >= (target+1+size-1)/size
			//	//	minus_h >= target/size+1
			//	out[y] = -(target/size+1);


			ty = -1 - ((49-y)<<8)/size;
		}


		//typedef void(*scaler_fn_t)(__a0 byte *dst, __a2 const byte *tex);

		// texture
		*dst++ = 0x116A;		// 0001 000 101 101 010			MOVE.b	xxx(A2), yyy(A0)					MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
								// 0001 0001 0110 1010
		*dst++ = ty<<6;
		*dst++ = 40*(y-50);
	}

	*dst++ = 0x4E75;		// RTS
							
	//// Lowres
	//int mip = 6;
	//for( int y=0; y<99; y++ )
	//{
	//	int step = (fn[1+(y+1)*3]>>mip) - (fn[1+y*3]>>mip);
	//	if( step > 1 )
	//	{
	//		mip++;
	//		y--;
	//		continue;
	//	}
	//}
	////int mask = (-1)<<mip;
	//int mask = (mip==6) ? -1 : 0;
	//for( int y=0; y<100; y++ )
	//	fn[1+y*3] &= mask;




	return dst;
}

void Dread_Build_Scalers(void)
{
	// Scale size map
	// soft_size_map.resize(1201);

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
			FN_SCALERS[start].func_addr = ptr + 50*3;
			FN_SCALERS[start].real_size = size<<4;
			SIZE_LINE_CHEAT[start] = line_cheat;
			start++;
		}

		// Build Line chear
		const int lrc_cam_pos_z = -40;
		{
			int m128 = ((-128 - lrc_cam_pos_z)*size)>>8;
			int m64	 = (( -64 - lrc_cam_pos_z)*size)>>8;
			int z0	 = ((   0 - lrc_cam_pos_z)*size)>>8;
#define CLAMP(y)		if(y<-50) y=-50; if(y>49) y=49;
			CLAMP(m128);
			CLAMP(m64);
			CLAMP(z0);
#undef CLAMP

			line_cheat->ceil_len_m64  = ( m64 - -50)*4;				//	 +2		$0			$3
			line_cheat->ceil_len_m128 = (m128 - -50)*4;				//	 +4			$1	$2
			line_cheat->upper_start_m64  = ptr + (50+m64)*3;		//	 +6		$0			$3
			line_cheat->upper_start_m128 = ptr + (50+m128)*3;		//	+10			$1	$2
			line_cheat->upper_len_m64_0		= ( z0- m64)*6;			//	+14		$0			$3
			line_cheat->upper_len_m128_0	= ( z0-m128)*6;			//	+16			$1
			line_cheat->upper_len_m128_m64	= (m64-m128)*6;			//	+18				$2
			line_cheat->floor_start_0 = (z0 - -50)*4;				//	+20		$0	$1	$2	$3
			//line_cheat->other_ceil_start_m64;						//	+22				$2
		}
		line_cheat++;

		// Build scaler
		ptr = Dread_Build_Scaler_One(ptr, size);
	}

	*ptr++ = 0x4E75;		// RTS
}

void Dread_Build_Fillers(void)
{
	word *ptr = FILLER_BUFFER;

	// Color filler
	FN_COLORFILL_START = (colorfill_fn_t)ptr;
	FN_COLORFILL = (colorfill_fn_t)(ptr + 50*2);
	{
		for(int y=0;y<100;y++)
		{
			*ptr++ = 0x1145;		// 0001 000 101 000 101			MOVE.b	D5, yyy(A0)							MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
									// 0001 0001 0100 0101
			*ptr++ = 40*(y-50);
		}

		*ptr++ = 0x4E75;		// RTS
	}

	// Sky filler
	FN_SKYCOPY = (skycopy_fn_t)(ptr + 50*2);
	{
		for( int y=0; y<100; y++ )
		{
			*ptr++ = 0x1159;		// 0001 000 101 011 001			MOVE.b	(A1)+, yyy(A0)						MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
									// 0001 0001 0101 1001
			*ptr++ = 40*(y-50);
		}

		*ptr++ = 0x4E75;		// RTS
	}
}
