
#ifndef _TRACKLOADER_H
#define _TRACKLOADER_H

#if TRACKLOADER


#define TRACK_BUFFER_SIZE			12980
#define SECTOR_SIZE					512
#define SECTORS_PER_TRACK			11
#define DECODED_TRACK_SIZE			(SECTORS_PER_TRACK*SECTOR_SIZE)
#define TRACKHEADS_PER_DISK			(80*2)


//$asmstruct trk
typedef struct {
	//word	*buffer_address;				// Track buffer address - unused by the routine itself
	//word	*decoded_address;				// Decoded track buffer address - unused by the routine itself
	word	req_trackhead;			// Track/head number to load
	word	*req_destination;		// Load destination address
	word	current_track;			// The track head currently is over
	word	current_head;			// Currently selected head(0 or 1)
	word	dma_trackhead;			// Track/head number DMA was activated for, or $FFFF if DMA not active
	word	loaded_trackhead;		// Recently loaded track/head number
} TrackloaderVars;





void Track_Init(void);
//void Track_ISR_Poll(void);



// Fake system routines
//void Sys_Restore(void);
//void Sys_Flush(void);
//void Sys_WaitTOF(void);
void *Sys_Open(const char *filename);
void Sys_Close(void *file);
int  Sys_Read(void *file, void *buffer, int length);
void *Sys_Lock(const char *filename);
void Sys_UnLock(void *lock);
int Sys_Examine(void *lock, dos_FileInfoBlock *fileInfoBlock);
int Sys_ExNext(void *lock, dos_FileInfoBlock *fileInfoBlock);






#endif

#endif
