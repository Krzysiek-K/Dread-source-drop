
#ifndef _AMIGA_STATBAR_H
#define _AMIGA_STATBAR_H





//$asmstruct statblit
typedef struct {
	word		bltcon0;			// BLTCON0 + BLTCON1 can be copied as one dword
	word		bltcon1;			//
	word		bltafwm;			// BLTAFWM + BLTALWM can be copied as one dword
	word		bltalwm;			//
	word		bltcdmod;			// BLTCMOD + BLTBMOD can be copied as one dword
	word		bltabmod;			// BLTAMOD + BLTDMOD can be copied as one dword (after word-swapping the above)
	const word	*source;			// runtime
	word		srcmask_offs;
	word		target_offs;
	word		bltsize;
	void		*next_blit;			// runtime
} AmigaStatbarBlitDef;


extern AmigaStatbarBlitDef statbar_blit_commands[ENGINE_MAX_STATBAR_BLITS];
extern word statbar_data_shadow[ENGINE_MAX_STATBAR_CACHE];



void Statbar_Init(word *screenbase, const dword *info);
void Statbar_Update(word *screenbase, const dword *info);




#endif
