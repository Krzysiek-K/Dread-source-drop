
# ---------------- Build parameters ----------------

DEMO_NAME		= Dread
BOOT_ADF_NAME	= BootTest

DEBUG			= 0



#MAP_FILES = c:/Progs/WinUAEDemoToolchain/DH0/maps/start.map
#MAP_FILES = out/maps-extra/MonsterRampage.map

#ADF_LOAD_MAP_FILES = --raw-data \
#	A,out/maps/BigMap.map \
#	B,out/maps/DemoMapAlternative.map \
#	C,out/maps-extra/DemoMap.map \
#	D,out/maps-extra/FeatureTest.map \
#	E,out/maps-extra/MonsterRampage.map \
#	F,out/maps-extra/MonsterTest.map \
#	G,out/maps-extra/Wildcat3D.map \
#	H,out/maps-raw/start.map 


ADF_LOAD_MAP_FILES = \
	--dread-dir-alloc 4 \
	--dread-map A,data/start.map ADF \
	--dread-dir-update

#ADF_LOAD_MAP_FILES = \
#	--dread-dir-alloc 4 \
#	--dread-map "A,data/maps/Bartekmil - GammaKnife.map" ADF \
#	--dread-map "B,data/maps/Zero Senki - The Library.map" ADF \
#	--dread-map "C,data/maps/Zero Senki - The Library.map" ADF \
#	--dread-dir-update

#ADF_LOAD_MAP_FILES = \
#	--dread-dir-alloc 4 \
#	--dread-map A,out/maps-raw/start.map ADF \
#	--dread-map B,out/maps-raw/BigMap.map ADF \
#	--dread-dir-update
##	--dread-map C,out/maps-raw/FeatureTest.map ADF


#MAP_FILES = \
#	A,out/maps-raw/BigMap.map \
#	B,out/maps-raw/DemoMapAlternative.map \
#	C,out/maps-raw/FeatureTest.map


# ---------------- Build options ----------------

DEFS	+= -DDebug=$(DEBUG)
DEFS	+= -DWEAPONSPRITES=1
DEFS	+= -DAMIGA=1
DEFS	+= -DATARI_ST=0
DEFS	+= -DSTRETCH_SCALING=1
DEFS	+= -DENGINE_Y_MID=50
DEFS	+= -DENGINE_Y_MAX=100
DEFS	+= -DAMIGA_AUDIO_ENGINE_VERSION=2

####### General engine configuration ########

### Embedded map data
DEFS	+= -DTRACKLOADER=0							# Enable trackloader
DEFS	+= -DUSE_EMBEDDED_MAP_DATA=1
DEFS	+= -DENABLE_MAP_FILE_LOADING=0
DEFS	+= -DENABLE_MAP_AUTOSTART=0					# use 1xx to immediately select skill
DEFS	+= -DSYSTEM_PRESENT=0						# 0: system already killed, bootloader detected memory regions		1: system present and useable

#### Trackloader
#DEFS	+= -DTRACKLOADER=1							# Enable trackloader
#DEFS	+= -DUSE_EMBEDDED_MAP_DATA=0
#DEFS	+= -DENABLE_MAP_FILE_LOADING=1
#DEFS	+= -DENABLE_MAP_AUTOSTART=0					# use 1xx to immediately select skill
#DEFS	+= -DSYSTEM_PRESENT=0						# 0: system already killed, bootloader detected memory regions		1: system present and useable

#### File loader
#DEFS	+= -DTRACKLOADER=0							# Enable trackloader
#DEFS	+= -DUSE_EMBEDDED_MAP_DATA=0
#DEFS	+= -DENABLE_MAP_FILE_LOADING=1
#DEFS	+= -DENABLE_MAP_AUTOSTART=0					# use 1xx to immediately select skill
#DEFS	+= -DSYSTEM_PRESENT=1						# 0: system already killed, bootloader detected memory regions		1: system present and useable


####### Engine flags ########
DEFS	+= -DWALL_RENDERER_VERSION=1
DEFS	+= -DUSE_VIS_GROUPS=1
DEFS	+= -DVIS_ALIGN=2
#DEFS	+= -DKICKSTART_CHECKSUM=0x0D1B6127


## Debugging
DEFS	+= -DWALL_ORDER_DEBUG=0
DEFS	+= -DSHOW_MEM_STATS=0
DEFS	+= -DPERSPECTIVE_CORRECTION=1			#	0: always OFF,    1: dynamic,    2: always ON
DEFS	+= -DDISABLE_ALL_THINGS=0
DEFS	+= -DAUDIO_ENABLED=1
DEFS	+= -DMULTIPLAYER_MODE=1
DEFS	+= -DAMIGA_COMM_LEVEL=1					#	0: interrupts,	  1: polling
DEFS	+= -DDEBUG_CONSOLE=0					#	debug console line count



# ---------------- Build directories ----------------

SRCDIR = ./src
OBJDIR = ./obj
OUTDIR = ./out

SOUNDDIR	= ./sound
GENDIR		= ./gen
DATADIR		= ./data


FINALEXE = $(realpath $(OUTDIR)/$(DEMO_NAME).exe)
FINALADF = $(realpath $(OUTDIR)/$(BOOT_ADF_NAME).adf)

FINALEXE_WIN = $(subst /,\\,$(FINALEXE))
FINALADF_WIN = $(subst /,\\,$(FINALADF))


#WINUAE_OPTIONS += -s gfx_autoresolution_vga!false
#WINUAE_OPTIONS += -s gfx_lores!true
#WINUAE_OPTIONS += -s gfx_resolution!lores
#WINUAE_OPTIONS += -s gfx_linemode!none
#WINUAE_OPTIONS += -s gfx_filter_autoscale!none
#WINUAE_OPTIONS += -s gfx_filter_aspect_ratio!0:0


# ---------------- Build sources ----------------


CSRC += $(SRCDIR)/amiga_debug.c
CSRC += $(SRCDIR)/amiga_comm.c
CSRC += $(SRCDIR)/amiga_audio_c.c
CSRC += $(SRCDIR)/amiga_audio2_c.c
CSRC += $(SRCDIR)/amiga_files.c
CSRC += $(SRCDIR)/amiga_vscreen.c
CSRC += $(SRCDIR)/amiga_menu.c
CSRC += $(SRCDIR)/amiga_minimap.c
CSRC += $(SRCDIR)/amiga_blitter.c
CSRC += $(SRCDIR)/amiga_statbar.c
CSRC += $(SRCDIR)/dread_estructs.c
CSRC += $(SRCDIR)/dread_statbar.c
CSRC += $(SRCDIR)/dread_game.c
CSRC += $(SRCDIR)/dread_physics.c
CSRC += $(SRCDIR)/dread_framework.c
CSRC += $(SRCDIR)/dread_soft.c
CSRC += $(SRCDIR)/dread_codegen.c
CSRC += $(SRCDIR)/dread_mapload.c
CSRC += $(SRCDIR)/trackloader.c
CSRC += $(SRCDIR)/main.c
CSRC += $(SRCDIR)/m_copper.c
CSRC += $(SRCDIR)/m_timing.c
CSRC += $(SRCDIR)/mem.c
CSRC += $(SRCDIR)/fn.c

XSRC += $(SRCDIR)/xcode.k68
XSRC += $(SRCDIR)/lineasm.k68
XSRC += $(SRCDIR)/copper.k68

HEADERS += $(wildcard $(SRCDIR)/*.h)
HEADERS += $(wildcard $(SRCDIR)/*.inc)
HEADERS += $(wildcard $(SRCDIR)/*.inl)

ASMSRC	+= $(wildcard $(SRCDIR)/*.S)
ASMSRC	+= $(wildcard $(SRCDIR)/*.s)
ASMSRC	+= $(wildcard $(SRCDIR)/*.i)
ASMSRC	+= $(wildcard $(SRCDIR)/*.asm)

HEADERS += $(wildcard $(GENDIR)/*.inl)
ASMSRC	+= $(wildcard $(GENDIR)/*.s)
ASMSRC	+= $(wildcard $(GENDIR)/*.i)
ASMSRC	+= $(GENDIR)/gen_asmstruct.i

HEADERS	+= $(wildcard $(DATADIR)/*.inc)
ASMSRC	+= $(wildcard $(DATADIR)/*.i)



# ---------------- Build objects ----------------

OBJECTS += $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(CSRC))
OBJECTS += $(OBJDIR)/amiga_asm.o

LISTINGS += $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.lst,$(CSRC))


LIBS = lib/vc.lib


# ---------------- Build tools ----------------

K68		= c:/LocalProjects/_amiga/k68/Release/k68.exe
VASM	= vasmm68k_mot_win32.exe
VC		= vc
LINK	= vlink
EXAMINE	= c:\LocalProjects\_tools\objectexaminer\Debug\objectexaminer.exe
WINUAE	= c:/Progs/WinUAEDemoToolchain/ToolChain/startWinUAE.bat
#ADFTOOL	= c:/LocalProjects/_amiga/adftool/Debug/adftool.exe 
ADFTOOL	= c:/LocalProjects/_amiga/adftool/Release/adftool.exe 



CFLAGS	+= $(DEFS)
CFLAGS	+= -O4 -notmpfile
#CFLAGS	+= -O=23999 -maxoptpasses=32
#CFLAGS	+= -O3 -maxoptpasses=32 -notmpfile
CFLAGS	+= -nostdlib
CFLAGS	+= -I. -Isrc

ASMFLAGS	+= -m68000 -spaces -Fhunk -x -esc
ASMFLAGS	+= -nowarn=62									# warning 62: imported symbol <...> was not referenced


# Disable nasty warnings:
#	 81	- casting number to address				//	(word*)0xdff001
#	170	- dead code elliminated
#	 65	- unused variable
#	152 - loop eliminated						//	do{...}while(0);
#	172	- optimizer passes						//	Dread_Build_Fillers()
#	 79	- offset equals size of object			//	e_visthings_max = e_visthings + ENGINE_MAX_VISTHINGS;

CFLAGS += -dontwarn=81 -dontwarn=170 -dontwarn=65 -dontwarn=152 -dontwarn=172 -dontwarn=79


# ---------------- Build generated sources ----------------


gen/gen_asmstruct.i : src/dread_data.h src/dread_estructs.h src/dread_asm.h src/amiga_asm.h src/amiga_system.h src/amiga_audio.h src/amiga_audio2.h src/amiga_statbar.h src/trackloader.h scripts/asmstruct/_gen_asmstruct.awk Makefile
	gawk -f scripts/asmstruct/_gen_asmstruct.awk $^ >$@


# ---------------- Build commands ----------------


src/xcode.asm : $(XSRC) Makefile
	$(K68) $(XSRC) -o $(SRCDIR)/xcode.asm -oh $(SRCDIR)/xcode.h


obj/amiga_asm.o : $(ASMSRC) Makefile
	$(VASM) $(ASMFLAGS) -o obj/amiga_asm.o src/amiga_asm.s $(DEFS)

obj/amiga_bootsector.bin : src/amiga_bootsector.s $(ASMSRC) Makefile	FORCE
	$(VASM) $(ASMFLAGS) -o obj/amiga_bootsector.bin src/amiga_bootsector.s -Fbin

obj/%.o : src/%.c $(HEADERS) Makefile
	$(VC) $(CFLAGS) -c $(subst /,\,$<) -o $@

obj/%.lst : src/%.c $(HEADERS) Makefile
	$(VC) $(CFLAGS) -S $(subst /,\,$<) -o $@


# vasmm68k_mot_win32.exe -m68000 -spaces -Fhunk  -x -o out\DreadTestUnpacked.o amiga_asm.s -DDebug=0 -DUseMiniPacker=0 -DTRACKLOADER=0
# vc -O4 -notmpfile -nostdlib -DDebug=0 -DUseMiniPacker=0 -dontwarn=81 -dontwarn=170 -dontwarn=65 -dontwarn=152 -DTRACKLOADER=0 -o out\DreadTestUnpacked.exe out\DreadTestUnpacked.o dread_statbar.c dread_game.c dread_physics.c dread_framework.c dread_soft.c dread_codegen.c dread_mapload.c trackloader.c blit.c main.c m_copper.c m_timing.c mem.c fn.c vc.lib

#$(FINALEXE) : $(OBJECTS) Makefile
#	$(VC) $(CFLAGS) -o $(FINALEXE) $(subst /,\,$(OBJECTS)) $(LIBS)
#	$(EXAMINE) $(FINALEXE)

$(FINALEXE) : $(CSRC) obj/amiga_asm.o Makefile
	$(VC) $(CFLAGS) -o $(FINALEXE) $(subst /,\,$(CSRC)) obj/amiga_asm.o $(LIBS)
#	$(EXAMINE) $(FINALEXE)


$(FINALADF) : obj/amiga_bootsector.bin $(ADFTOOL) $(FINALEXE)	FORCE
	$(ADFTOOL) -w \
		--adf-load $< \
		--boot-checksum \
		--dread-trackloader \
		--exe-load $(FINALEXE) \
		--exe-dreadload gen/dread_loadscript.inc \
		$(ADF_LOAD_MAP_FILES) \
		--sector-bitmap \
		--adf-save $@

#	$(ADFTOOL) --adf-load $< --boot-checksum --dread-trackloader --raw-data HELLO=data/hello.txt --raw-bitmap 440#data/test/testimg.bmp --sector-bitmap --adf-save $@
	

FORCE: ;



# ---------------- Build targets ----------------


all: $(FINALEXE)

bootloader: $(FINALADF)

bootloader_a500: $(FINALADF)
	$(WINUAE) a500_BigWindow KICK13 $(FINALADF_WIN) "" $(WINUAE_OPTIONS)

bootloader_a1200: $(FINALADF)
	$(WINUAE) a1200 KICK31 $(FINALADF_WIN) "" $(WINUAE_OPTIONS)



rebuild: clean all asm

clean:
	-rm -rf ./$(OBJDIR)/*.o
	-rm -rf ./$(OBJDIR)/*.bin
	-rm -rf $(FINALEXE)

run_a500: $(FINALEXE)
	start $(WINUAE) a500_master KICK13 $(FINALEXE_WIN) "" $(WINUAE_OPTIONS)
	$(WINUAE) a500_slave KICK13 $(FINALEXE_WIN) "" $(WINUAE_OPTIONS)

# $(WINUAE) a500_BigWindow KICK13 $(FINALEXE_WIN) "" $(WINUAE_OPTIONS)

run_a600: $(FINALEXE)
	$(WINUAE) a600 KICK205 $(FINALEXE_WIN)

run_a1200: $(FINALEXE)
	$(WINUAE) a1200 KICK31 $(FINALEXE_WIN)

run_a4000_40: $(FINALEXE)
	$(WINUAE) a4000_40 KICK31 $(FINALEXE_WIN)

run_a4000_60: $(FINALEXE)
	$(WINUAE) a4000_60 KICK31 $(FINALEXE_WIN)

asm: $(LISTINGS)

.PHONY: all asm rebuild clean run_a500 run_a600 run_a1200 run_a4000_40 run_a4000_60
