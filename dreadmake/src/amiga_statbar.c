
#include "demo.h"


AmigaStatbarBlitDef statbar_blit_commands[ENGINE_MAX_STATBAR_BLITS];
word statbar_data_shadow[ENGINE_MAX_STATBAR_CACHE];
volatile AmigaStatbarBlitDef *statbar_blit_scheduled_cmd;




static void _stat_init_clear(AmigaStatbarBlitDef *cmd, const word *background, word x0, word y0, word w, word h)
{
	//
	//	BLTCON0 + BLTCON1		$07CA0000
	//	BLTAFWM + BLTALWM							[compute]
	//	BLTADAT					$FFFF
	//	BLTCMOD + BLTBMOD		40-2ww | 40-2ww		[compute]
	//			  BLTDMOD				 40-2ww		[compute]
	//	
	//	BLTBPT					[source]
	//	BLTCPT					[target]
	//	BLTDPT					[target]
	//	BLTSIZE					[compute]
	//
	word x2 = x0 + w;
	word ww = ((x2+15)>>4) - (x0>>4);
	word off = 2*((x0>>4) + 20*4*y0);

	memset(cmd, 0, sizeof(*cmd));

	cmd->bltcon0 = 0x07CA;
	cmd->bltcon1 = 0x0000;
	cmd->bltafwm = 0xFFFF >> (x0&15);
	cmd->bltalwm = (x2 & 15) ? (0xFFFF << (16-(x2&15))) & 0xFFFF : 0xFFFF;
	cmd->bltcdmod = cmd->bltabmod = 40-2*ww;
	cmd->source = background + (off>>1);
	cmd->target_offs = off;
	cmd->bltsize = COMPUTE_BLTSIZE(ww, h*4);
}

static void _stat_init_draw_masked(AmigaStatbarBlitDef *cmd, word x0, word y0, word w, word h)
{
	//
	//	BLTCON0 + BLTCON1		$_FCA_000		[compute '_']
	//	BLTAFWM + BLTALWM		$FFFF____		[compute '_', $FFFF or $0000]
	//
	//	BLTCMOD + BLTBMOD		40-2ww | smod	[compute]
	//	BLTAMOD + BLTDMOD		smod | 40-2ww	[compute]
	//	BLTAPT									[source + mask offset]
	//	BLTBPT									[source]
	//	BLTCPT									[target]
	//	BLTDPT									[target]
	//	BLTSIZE									[compute]
	//
	word x2 = x0 + w;
	word ww = ((x2+15)>>4) - (x0>>4);
	word smod = -2*(ww-((w+15)>>4));
	word off = 2*((x0>>4) + 20*4*y0);

	memset(cmd, 0, sizeof(*cmd));

	cmd->bltcon0 = 0x0FCA + (x0&15)*0x1000;
	cmd->bltcon1 = 0x0000 + (x0&15)*0x1000;
	cmd->bltafwm = 0xFFFF;
	cmd->bltalwm = ww>((w+15)>>4) ? 0x0000 : 0xFFFF;
	cmd->bltcdmod = 40-2*ww;
	cmd->bltabmod = smod;
	cmd->srcmask_offs = 2*4*((w+15)>>4)*h;
	cmd->target_offs = off;
	cmd->bltsize = COMPUTE_BLTSIZE(ww, h*4);
}

static void _stat_init_draw_opaque(AmigaStatbarBlitDef *cmd, word x0, word y0, word w, word h)
{
	//
	//	BLTCON0 + BLTCON1		$_FCA_000		[compute '_']
	//	BLTAFWM + BLTALWM		$FFFF____		[compute '_', $FFFF or $0000]
	//
	//	BLTCMOD + BLTBMOD		40-2ww | smod	[compute]
	//	BLTAMOD + BLTDMOD		smod | 40-2ww	[compute]
	//	BLTAPT									[source + mask offset]
	//	BLTBPT									[source]
	//	BLTCPT									[target]
	//	BLTDPT									[target]
	//	BLTSIZE									[compute]
	//
	word x2 = x0 + w;
	word ww = ((x2+15)>>4) - (x0>>4);
	word smod = -2*(ww-((w+15)>>4));
	word off = 2*((x0>>4) + 20*4*y0);

	memset(cmd, 0, sizeof(*cmd));

	cmd->bltcon0 = 0x07CA;
	cmd->bltcon1 = 0x0000 + (x0&15)*0x1000;
	cmd->bltafwm = 0xFFFF >> (x0&15);
	cmd->bltalwm = (x2 & 15) ? (0xFFFF << (16-(x2&15))) & 0xFFFF : 0xFFFF;
	cmd->bltcdmod = 40-2*ww;
	cmd->bltabmod = smod;

	cmd->target_offs = off;
	cmd->bltsize = COMPUTE_BLTSIZE(ww, h*4);
}


void Statbar_Init(word *screenbase, const dword *info)
{
	const word *statgfx = (const word*)(*info++);
	AmigaStatbarBlitDef *blitcmd = statbar_blit_commands;
	word *shadow = statbar_data_shadow;

	memcpy(screenbase, statgfx+3, (statgfx[0]>>4)*statgfx[1]*8);

	while( 1 )
	{
		dword code = *info++;
		byte cmd = (byte)(code>>24);
		if( cmd>=1 && cmd<=3 )		// number 1..3
		{
			word xstep = (word)(code>>17) & 0x007F;
			word xpos = (word)(code>>8) & 0x01FF;
			word ypos = (byte)code;
			const word **font = (const word**)(*info++);
			word font_w = font[0][0];
			word font_h = font[0][1];
			info++;		// skip value

			// Prepare clear commands - longest clear first
			for( int i=0; i<cmd; i++ )
				_stat_init_clear(blitcmd++, statgfx+3, xpos + xstep*i, ypos, xstep*(cmd-1-i)+font_w, font_h);

			// Prepare draw commands - most significant digit first
			for( int i=0; i<cmd; i++ )
				_stat_init_draw_masked(blitcmd++, xpos + xstep*i, ypos, font_w, font_h);

			// Initialize shadow
			*shadow++ = 0xFFFF;
			for( int i=0; i<cmd; i++ )
				*shadow++ = 0xFFFF;
		}
		else if( (cmd&0xFE)==0x10 )		// frame	(masked or not)
		{
			word xpos = (word)(code>>8) & 0x01FF;
			word ypos = (byte)code;
			const word **frames = (const word**)(*info++);
			word frame_w = frames[0][0];
			word frame_h = frames[0][1];
			info++;		// skip value


			// Prepare commands
			if( cmd==16 )
			{
				_stat_init_clear(blitcmd++, statgfx+3, xpos, ypos, frame_w, frame_h);
				_stat_init_draw_masked(blitcmd++, xpos, ypos, frame_w, frame_h);
			}
			else
				_stat_init_draw_opaque(blitcmd++, xpos, ypos, frame_w, frame_h);

			// Initialize shadow
			*shadow++ = 0xFFFF;
		}
		else
			break;
	}
}


void Statbar_Update(word *screenbase, const dword *info)
{
	if( statbar_blit_scheduled_cmd )
		return;		// currently still drawing

	AmigaStatbarBlitDef *blitcmd = statbar_blit_commands;
	AmigaStatbarBlitDef *blit_start = NULL;
	AmigaStatbarBlitDef **next_pointer = &blit_start;
	word *shadow = statbar_data_shadow;

	info++;		// ignore background info

#define BLIT_QUEUE(b)			( *next_pointer=&(b), next_pointer=(AmigaStatbarBlitDef**)&((b).next_blit) )
	while( 1 )
	{
		dword code = *info++;
		byte cmd = (byte)(code>>24);
		if( cmd>=1 && cmd<=3 )
		{
			// draw value
			const word **font = (const word**)(*info++);
			word value = *(word*)(*info++);

			if( value == *shadow )
			{
				shadow++;
				shadow += cmd;
				blitcmd += cmd<<1;
				continue;
			}
			*shadow++ = value;

			if( cmd==3 )
			{
				word vv = value;
				word d3 = vv % 10;		vv /= 10;
				word d2 = vv % 10;		vv /= 10;
				word d1 = vv;

				if( shadow[0] != d1 )
				{
					// Repaint all 3 digits
					blitcmd[3].source = font[shadow[0]=d1] + 3;
					blitcmd[4].source = font[shadow[1]=d2] + 3;
					blitcmd[5].source = font[shadow[2]=d3] + 3;
					BLIT_QUEUE(blitcmd[0]);
					if( value>=100 ) BLIT_QUEUE(blitcmd[3]);
					if( value>=10 ) BLIT_QUEUE(blitcmd[4]);
					BLIT_QUEUE(blitcmd[5]);
				}
				else if( shadow[1] != d2 )
				{
					// Repaint 2 last digits
					blitcmd[4].source = font[shadow[1]=d2] + 3;
					blitcmd[5].source = font[shadow[2]=d3] + 3;
					BLIT_QUEUE(blitcmd[1]);
					if( value>=10 ) BLIT_QUEUE(blitcmd[4]);
					BLIT_QUEUE(blitcmd[5]);
				}
				else if( shadow[2] != d3 )
				{
					// Repaint last digit
					blitcmd[5].source = font[shadow[2]=d3] + 3;
					BLIT_QUEUE(blitcmd[2]);
					BLIT_QUEUE(blitcmd[5]);
				}

				shadow += 3;
				blitcmd += 6;
			}
		}
		else if( cmd==0x10 )
		{
			// draw frame
			const word **frames = (const word**)(*info++);
			word value = *(word*)(*info++);

			if( value == *shadow )
			{
				shadow++;
				blitcmd += 2;
				continue;
			}
			*shadow++ = value;

			// Draw
			BLIT_QUEUE(blitcmd[0]);
			if( value )
			{
				blitcmd[1].source = frames[value-1] + 3;
				BLIT_QUEUE(blitcmd[1]);
			}
			blitcmd += 2;
		}
		else if( cmd==0x11 )
		{
			// draw masked frame
			const word **frames = (const word**)(*info++);
			word value = *(word*)(*info++);

			if( value != *shadow )
			{
				*shadow = value;

				blitcmd[0].source = frames[value] + 3;
				BLIT_QUEUE(blitcmd[0]);
			}
			shadow++;
			blitcmd++;
		}
		else
			break;
	}

	if( blit_start )
	{
		*next_pointer = NULL;
		Atomic_WritePointer(blit_start, (void**)&statbar_blit_scheduled_cmd);
	}
}
