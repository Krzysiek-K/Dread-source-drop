
#ifndef _AMIGA_MINIMAP_H
#define _AMIGA_MINIMAP_H



typedef struct _MinimapFrame{
	word	**screen;
	word	*copper;
	short	xpos;
	short	ypos;
} MinimapFrame;


typedef struct _MinimapInfo {
	MinimapFrame	frame[2];
	word			frameflip;
	word			copper_offs_addr_low_p0;
	word			copper_offs_addr_low_p1;
	MinimapFrame	*drawframe;
	MinimapFrame	*showframe;
	short			cam_xpos;
	short			cam_ypos;
} MinimapInfo;




void Minimap_Run(void);



#endif
