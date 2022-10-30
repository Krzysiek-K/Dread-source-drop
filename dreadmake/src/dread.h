
#ifndef _DREAD_H
#define _DREAD_H



// ================================================================ Rendering definitions ================================================================



// limits

#define ENGINE_MAX_VERTEXES			1024					// [estimate]	Maximum number of vertexes
#define ENGINE_MAX_LINES			1536					// [estimate]	Maximum number of lines
#define ENGINE_MAX_SUBSECTORS		 512					// [estimate]	Maximum number of subsectors
#define ENGINE_MAX_THINGS			 512					//				Maximum number of things		(including run-time generated)
#define ENGINE_MAX_MACHINES			  64					//				Maximum number of machines	(doors, etc...)
#define ENGINE_MAX_CONDITIONS		 128					//				Maximum number of conditions (through-doors visibility combinations)
#define ENGINE_MAX_VISLINES			 540					//				Maximum number of vislines	(lines rendered in one frame)
#define ENGINE_MAX_VISTHINGS		  64					//				Maximum number of visthings	(things currently rendered, including door-generated things)
#define ENGINE_MAX_DMGTHINGS		  16					//				Maximum number of simultaneously damaged things

#define ENGINE_SUBVERTEX_PRECISION	3			// Redefined in asm and tools
#define ENGINE_THING_Z_PRECISION	6			// Redefined in asm

#define ENGINE_PLAYER_SIZE			15			// Redefined in asm and tools

#define ENGINE_MAX_STATBAR_BLITS	80			// Maxumum number of precomputed blit commands for player status bar	(Amiga only)
#define ENGINE_MAX_STATBAR_CACHE	60			// Size of status bar shadow data cache



// game definitions

#define GAMESTATE_NONE			0
#define GAMESTATE_IN_GAME		1
#define GAMESTATE_LEVEL_DONE	2


#include "dread_data.h"
#include "dread_estructs.h"
#include "dread_engine.h"
#include "dread_mapload.h"
#include "dread_asm.h"
#include "dread_framework.h"
#include "dread_physics.h"


#endif
