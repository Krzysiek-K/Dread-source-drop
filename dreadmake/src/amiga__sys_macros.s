



; ================================================================ Sys defines ================================================================


; Interrupt table
IV_LEVEL_1					=	$64		; transmitter buffer empty, disk block complete, software interrupt
IV_LEVEL_2					=	$68		; external INT2 & CIAA
IV_LEVEL_3					=	$6c		; graphics coprocessor, vertical blank interval, blitter finished
IV_LEVEL_4					=	$70		; audio channel 0..3
IV_LEVEL_5					=	$74		; receiver buffer full, disk sync pattern found
IV_LEVEL_6					=	$78		; external INT6 & CIAB
IV_LEVEL_7					=	$7c		; non-maskable interrupt

IV_TRAP_0					=	$80
IV_TRAP_1					=	$84



; -------------------------------- exec.library --------------------------------

sys_exec_Supervisor			=	-$1e 
sys_exec_CacheClearU		=	-$27c 
sys_exec_OpenResource		=	-$1f2 
sys_exec_OldOpenLibrary		=	-$198 
sys_exec_CloseLibrary		=	-$19e
sys_exec_PutMsg				=	-$16e
sys_exec_GetMsg				=	-$174
sys_exec_WaitPort			=	-$180
sys_exec_CacheControl		=	-648
sys_exec_DoIO				=	-456
sys_exec_AllocMem			=	-198
sys_exec_AllocAbs			=	-204
sys_exec_FreeMem			=	-210
sys_exec_SuperState			=	-150
sys_exec_AvailMem			=	-216 

sys_exec_lib_Version		=	 $14
sys_exec_eb_AttnFlags		=	 $128
;sys_exec_graphics_library	=	 156
sys_exec_ThisTask			=	 276

sys_task_pr_MsgPort			=	 92

sys_message_ln_Name			=	 10
sys_message_mn_ReplyPort	=	 14

; -------------------------------- graphics.library --------------------------------

sys_graphics_LoadView		=	-222
sys_graphics_WaitTOF		=	-270

sys_graphics_gb_ActiView	=	 $22
sys_graphics_gb_copinit		=	 $26


; -------------------------------- cia.resource --------------------------------

sys_cia_AbleICR				=	-18


; -------------------------------- dos.library --------------------------------

sys_dos_Open				=	-$1e
sys_dos_Close				=	-$24
sys_dos_Read				=	-$2a
sys_dos_Lock				=	-$54
sys_dos_UnLock				=	-$5a 
sys_dos_Examine				=	-$66 
sys_dos_ExNext				=	-$6c 
sys_dos_DeviceProc			=	-$ae 


dos_ACCESS_READ				=	-2
dos_ACCESS_WRITE			=	-1

dos_MODE_OLDFILE			=	1005	; Open existing file read/write  positioned at beginning of file.
dos_MODE_NEWFILE			=	1006	; Open freshly created file (delete old file) read/write, exclusive lock.
dos_MODE_READWRITE			=	1004	; Open old file w/shared lock, creates file if doesn't exist.

dos_ACTION_FLUSH			=	27
