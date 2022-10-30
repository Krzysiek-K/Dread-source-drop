
#ifndef _AMIGA_SYSTEM_H
#define _AMIGA_SYSTEM_H



//$asmstruct dos_fib
typedef struct {
	dword	DiskKey;
	int		DirEntryType;
	char	FileName[108];
	dword	Protection;
	dword	EntryType;
	dword	Size;
	dword	NumBlocks;
	dword	Days;
	dword	Minute;
	dword	Tick;
	char	Comment[80];
	word	OwnerUID;
	word	OwnerGID;
	char	Reserved[32];
} dos_FileInfoBlock;




// amiga__system.s

extern dos_FileInfoBlock file_info_block;

void Sys_Install(void);

#if SYSTEM_PRESENT
void Sys_Restore(void);
void Sys_Flush(void);
void Sys_WaitTOF(void);
void *Sys_Open(__d1 const char *filename);
void Sys_Close(__d1 void *file);
int  Sys_Read(__d1 void *file, __d2 void *buffer, __d3 int length);
void *Sys_Lock(__d1 const char *filename);
void Sys_UnLock(__d1 void *lock);
int Sys_Examine(__d1 void *lock, __d2 dos_FileInfoBlock *fileInfoBlock);
int Sys_ExNext(__d1 void *lock, __d2 dos_FileInfoBlock *fileInfoBlock);
__reg("d0") void *Sys_AllocAbs(__a1 void *addr, __d0 int size);
__reg("d0") void *Sys_AllocMem(__d0 int size, __d1 int memf_flags);
void Sys_FreeMem(__a1 void *addr, __d0 int size);
#endif

dword Sys_KSChecksum(void);





// amiga__traps.s

word Critical_Begin(void);
void Critical_End(__d0 word sr);
void Atomic_WritePointer(__d0 void *value, __a1 void **target);




#endif
