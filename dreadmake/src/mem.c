

#include "demo.h"


#if SYSTEM_PRESENT
// ================================================================ System present ================================================================


void *Sys_Alloc64(void)
{
	void *ptr = 0;

	for( int i=0x10000; !ptr && i<0x1FFFFF; i+=0x10000 )
		ptr = Sys_AllocAbs((void*)i, 0x10000);
	if( !ptr ) all_ok = 0;
	return ptr;
}


#else

// ================================================================ Custom bootloader ================================================================

#define NUM_RAM_ENTRIES			8		// should match bootloader


typedef struct {
	word	type;
	dword	start;
	dword	end;		// $0 = missing
} MemRegion;


#define mem_regions			((MemRegion*)0x100)			// fixed address of memory info block



// allocate 64k-aligned block of chip memory
void *Sys_Alloc64(void)
{
	for( word i=0; i<NUM_RAM_ENTRIES; i++ )
	{
		MemRegion *mem = &mem_regions[i];
		if( !mem->end ) continue;				// missing
		if( mem->type != 0x0002 ) continue;		// not Chip
		
		// try to allocate aligned 64k from the top
		dword addr = (mem->end & 0xFFFF0000);
		if( addr < 0x10000 ) continue;			// address too low to allocate
		addr -= 0x10000;

		if( addr < mem->start ) continue;		// out of memory

		if( addr + 0x10000 < mem->end )
		{
			// Create new region with the remaining chunk of memory
			for( word j=0; j<NUM_RAM_ENTRIES; j++ )
			{
				MemRegion *mem2 = &mem_regions[j];
				if( mem2->end ) continue;			// present
				mem2->type = mem->type;
				mem2->start = addr + 0x10000;
				mem2->end = mem->end;
				break;
			}
		}

		mem->end = addr;		// allocate
		return (void*)addr;		// done
	}

	all_ok = 0;
	return 0;
}


// allocate biggest chunk of available memory
void *Sys_AllocMax(dword *out_size)
{
	MemRegion *max_region = NULL;
	dword max_size = 0;
	for( word i=0; i<NUM_RAM_ENTRIES; i++ )
	{
		MemRegion *mem = &mem_regions[i];
		if( !mem->end ) continue;				// missing
		
		dword size = mem->end - mem->start;
		if( size > max_size )
		{
			max_region = mem;
			max_size = size;
		}
	}

	*out_size = max_size;

	if( !max_region )
		return NULL;

	max_region->end = max_region->start;		// allocate all
	return (void*)max_region->start;			// done
}

#endif
