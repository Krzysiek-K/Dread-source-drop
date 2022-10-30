
#include "demo.h"


#if TRACKLOADER




static __chip word track_buffer[TRACK_BUFFER_SIZE/2];
static word track_decoded[DECODED_TRACK_SIZE/2];
static byte *track_read_pointer;
static word track_read_trackhead;
static word track_read_startsector;


volatile TrackloaderVars trk;
static const char *dir_scan;



// internam asm functions
void Track_Init_asm(void);
__d0 word Track_Decode(__a1 void *track_data, __d0 word start_sec, __d1 word num_sec, __a0 void *target_buffer);




void Track_Init(void)
{
	trk.req_trackhead = 0xFFFF;
	trk.req_destination = track_buffer;
	trk.current_track = 0;
	trk.current_head = 2;
	trk.dma_trackhead = 0xFFFF;
	trk.loaded_trackhead = 0xFFFF;

	Track_Init_asm();

	track_read_pointer = NULL;
}




// ================================================================ Fake system routines ================================================================


void *Sys_Open(const char *filename)
{
	track_read_pointer = NULL;

	while( *filename && *filename!='.' )
		filename++;

	word sector = 0;
	while( *filename )
	{
		if( *filename>='0' && *filename<='9' )
			sector = sector*10 + (*filename-'0');
		filename++;
	}

	word track = sector/11;
	track_read_startsector = sector%11;

	while( (short)trk.req_trackhead >= 0 ){}
	trk.req_trackhead = track;
	track_read_trackhead = track;
	track_read_pointer = NULL;

	return (void*)1;
}

void Sys_Close(void *file)
{
	track_read_pointer = NULL;
}

int Sys_Read(void *file, void *_buffer, int length)
{
	const byte *track_end = (byte*)track_decoded + DECODED_TRACK_SIZE;
	byte *buffer = (byte*)_buffer;
	int required_len = length;
	
	while( length>0 )
	{
		if( track_read_pointer && track_read_pointer < track_end )
		{
			dword copylen = track_end - track_read_pointer;
			if( copylen > length )
				copylen = length;

			if( buffer )
			{
				asm_memcpy(buffer, track_read_pointer, copylen);
				buffer += copylen;
			}
			track_read_pointer += copylen;
			length -= (int)copylen;

			//if( length>0 )
			//	Menu_Print(".");	// TRACKDEBUG: reading bytes, not finished
			//else
			//	Menu_Print(":");	// TRACKDEBUG: reading bytes, finished
		}

		if( !track_read_pointer || track_read_pointer >= track_end )
		{
			if( track_read_trackhead >= TRACKHEADS_PER_DISK )
				break;	// end of the disk


			//word rqt = trk.req_trackhead;
			//if( (short)rqt >= 0 )
			//{
			//	if( length<=0 )
			//	{
			//		Menu_Print("N");	// TRACKDEBUG: trackloader busy, not not required yet
			//		continue;
			//	}
			//	else if( rqt == track_read_trackhead  )
			//		Menu_Print("B");	// TRACKDEBUG: trackloader busy loading correct track
			//	else
			//		Menu_Print("X");	// TRACKDEBUG: trackloader busy loading wrong track
			//}
			//else if( trk.loaded_trackhead==track_read_trackhead )
			//	Menu_Print("L");	// TRACKDEBUG: track already preloaded
			//else
			//	Menu_Print("F");	// TRACKDEBUG: trackloader free

			// No valid track - assure one
			while(1)
			{
				// Assure valid track is loaded
				while( 1 )
				{
					while( (short)trk.req_trackhead >= 0 ) {}

					if( trk.loaded_trackhead==track_read_trackhead )
						break;

					trk.req_trackhead = track_read_trackhead;
				}


				word error = Track_Decode(trk.req_destination, 0, SECTORS_PER_TRACK, track_decoded);
				if( !error )
					break;

				trk.loaded_trackhead = 0xFFFF;	// invalidate and repeat track loading
			}

			// Track decoded fine
			track_read_pointer = (byte*)track_decoded + track_read_startsector*SECTOR_SIZE;

			// Initialize loading next track
			track_read_trackhead++;
			track_read_startsector = 0;

			if( track_read_trackhead < TRACKHEADS_PER_DISK )
			{
				trk.req_trackhead = track_read_trackhead;
				//Menu_PrintDec(track_read_trackhead%10);		// TRACKDEBUG: requesting next track
			}
		}
	}

	return required_len - length;
}

void *Sys_Lock(const char *filename)
{
	//dir_scan = "TestDiskMap.891 Foo.100 Bar.120 AnotherFakeMapEntry.140";
	//return dir_scan;
	
	void *fp = Sys_Open(".880");
	if( !fp ) return NULL;
	if( !Sys_Read(fp, track_decoded, 1) ) return NULL;		// assure track is in the buffer

	dir_scan = (const char*)track_decoded;

	return fp;
}

void Sys_UnLock(void *lock)
{
	Sys_Close(lock);
}

int Sys_Examine(void *lock, dos_FileInfoBlock *fileInfoBlock)
{
	return (lock != NULL);
}

int Sys_ExNext(void *lock, dos_FileInfoBlock *fileInfoBlock)
{
	fileInfoBlock->DirEntryType = -1;
	
	if( !lock || !*dir_scan )
		return 0;

	char *dst = fileInfoBlock->FileName;
	while( *dir_scan && *dir_scan!=' ')
		*dst++ = *dir_scan++;
	*dst = 0;

	if( *dir_scan )
		dir_scan++;

	return 1;
}





#endif
