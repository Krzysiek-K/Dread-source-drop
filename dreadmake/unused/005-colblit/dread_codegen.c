
#include "demo.h"




word COLBLIT_BUFFER[32000];	// overestimated
colblit_fn_t FN_COLBLITS[101];


ScalerInfo FN_SCALERS[NUM_SOFT_SIZES];
word SCALER_BUFFER[85802/2+200];
word FILLER_BUFFER[101*(2+4)];
colorfill_fn_t FN_COLORFILL;
skycopy_fn_t FN_SKYCOPY;






static word *Dread_Build_ColBlit_One(word *ptr, int size)
{
	const short lrc_cam_pos_z = 40;
	short line_h1 = 64 - lrc_cam_pos_z;
	short line_h2 = 0 - lrc_cam_pos_z;

	int s1 = size << 8;
	int y1 = 50 - ((s1*line_h1)>>13);
	int y2 = 50 - ((s1*line_h2)>>13);
	if( y1<=0 ) y1 = 0;
	if( y2>=100 ) y2 = 100;

	//typedef void(*colblit_fn_t)(__a0 byte *dst, __a2 const byte *tex, __a1 byte *sky, __d1 byte floor);
	for( int y=0; y<100; y++ )
	{
		int ty;
		//if( size>=12 ) ty = ((y-50)*32/size - (lrc_cam_pos_z-64)) + 32, tyy = (ty-32)>>1;
		//else if( size>=1 ) tyy = ty = ((y-50)*16/size - (lrc_cam_pos_z-64)/2);
		//else tyy = (y<50) ? -1 : 32;
		
		if( size>=1 ) ty = ((y-50)*32/size - (lrc_cam_pos_z-64));
		else ty = (y<50) ? -1 : 64;

#if 1
		// Row order (interleaved)
		if( ty<0 )
		{
			//// ceil
			//*ptr++ = 0x1140;		// 0001 000 101 000 000			MOVE.b	D0, yyy(A0)							MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
			//						// 0001 0001 0100 0000
			//*ptr++ = 40*y;

			// sky
			*ptr++ = 0x1159;		// 0001 000 101 011 001			MOVE.b	(A1)+, yyy(A0)						MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
									// 0001 0001 0101 1001
			*ptr++ = 40*y;
		}
		else if( ty>=64 )
		{
			// floor
			*ptr++ = 0x1141;		// 0001 000 101 000 001			MOVE.b	D1, yyy(A0)							MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
									// 0001 0001 0100 0001
			*ptr++ = 40*y;
		}
		else
		{
			// texture
			*ptr++ = 0x116A;		// 0001 000 101 101 010			MOVE.b	xxx(A2), yyy(A0)					MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
									// 0001 0001 0110 1010
			*ptr++ = ty<<6;
			*ptr++ = 40*y;
		}
#else
		// Column order
		if( tyy<0 )
		{
			// sky
			*ptr++ = 0x10D9;		// 0001 000 011 011 001			MOVE.b	(A1)+, (A0)+						MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
									// 0001 0000 1101 1001
		}
		else if( tyy>=32 )
		{
			// floor
			*ptr++ = 0x10C1;		// 0001 000 011 000 001			MOVE.b	D1, (A0)+							MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
									// 0001 0000 1100 0001
		}
		else
		{
			// texture
			*ptr++ = 0x10EA;		// 0001 000 011 101 010			MOVE.b	xxx(A2), (A0)+						MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
									// 0001 0000 1110 1010
			*ptr++ = ty<<6;
		}
#endif
	}

	*ptr++ = 0x4E75;		// RTS

	return ptr;
}

void Dread_Build_ColBlits(void)
{
	word *ptr = COLBLIT_BUFFER;
	for( int size=0; size<=100; size++ )
	{
		if( size<=50 || (size&1)==0 )
		{
			FN_COLBLITS[size] = (colblit_fn_t)ptr;
			ptr = Dread_Build_ColBlit_One(ptr, size);
		}
		else
			FN_COLBLITS[size] = FN_COLBLITS[size-1];
	}
}





static word *Dread_Build_Scaler_One(word *dst, int size)
{
	// y1 = 50 + ((size*h1)>>8)				-  y1 is topmost (first) pixel which gets texcoord h1
	//
	//	for each Y>=y1	->	h>=h1

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
		*dst++ = 40*y;
	}

	*dst++ = 0x4E75;		// RTS


	return dst;
}

void Dread_Build_Scalers(void)
{
	// Scale size map
	// soft_size_map.resize(1201);

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

		int mid = (start+(end-1))/2;
		while( start<end )
		{
			FN_SCALERS[start].func_addr = ptr;
			FN_SCALERS[start].real_size = mid;
			start++;
		}

		ptr = Dread_Build_Scaler_One(ptr, mid);
	}
}

void Dread_Build_Fillers(void)
{
	word *ptr = FILLER_BUFFER;

	// Color filler
	FN_COLORFILL = ptr;
	{
		for(int y=0;y<100;y++)
		{
			*ptr++ = 0x1141;		// 0001 000 101 000 001			MOVE.b	D1, yyy(A0)							MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
									// 0001 0001 0100 0001
			*ptr++ = 40*y;
		}

		*ptr++ = 0x4E75;		// RTS
	}

	// Sky filler
	FN_SKYCOPY = ptr;
	{
		for( int y=0; y<100; y++ )
		{
			*ptr++ = 0x1159;		// 0001 000 101 011 001			MOVE.b	(A1)+, yyy(A0)						MOVE =	00 SS	<Xdst> <Mdst> <Msrc> <Xsrc>
									// 0001 0001 0101 1001
			*ptr++ = 40*y;
		}

		*ptr++ = 0x4E75;		// RTS
	}
}
