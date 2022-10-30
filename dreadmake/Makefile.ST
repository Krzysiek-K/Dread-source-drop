
# ---------------- Build parameters ----------------

DEMO_NAME		= DreadST





# ---------------- Build directories ----------------

SRCDIR = ./src
OBJDIR = ./objst
OUTDIR = ./out

SOUNDDIR	= ./sound
GENDIR		= ./gen
DATADIR		= ./data


MAPFILE = $(OUTDIR)/auto/$(DEMO_NAME).map

FINALEXE = $(realpath $(OUTDIR)/auto/$(DEMO_NAME).prg)
FINALHD = $(realpath $(OUTDIR))



OBJECTS = $(patsubst %,$(OBJDIR)/%,$(_OBJECTS))
HEADERS = $(patsubst %,$(SRCDIR)/%,$(_HEADERS))



# ---------------- Build sources ----------------


CSRC += $(SRCDIR)/st_main.c
CSRC += $(SRCDIR)/st_comm.c
CSRC += $(SRCDIR)/st_menu.c
CSRC += $(SRCDIR)/st_framework.c
CSRC += $(SRCDIR)/st_debug.c
CSRC += $(SRCDIR)/dread_mapload.c
CSRC += $(SRCDIR)/dread_estructs.c
CSRC += $(SRCDIR)/dread_physics.c
CSRC += $(SRCDIR)/dread_codegen.c
CSRC += $(SRCDIR)/dread_soft.c
CSRC += $(SRCDIR)/dread_game.c
CSRC += $(SRCDIR)/fn.c

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
ASMSRC	+= $(GENDIR)/gen_asmstruct_st.i

HEADERS	+= $(wildcard $(DATADIR)/*.inc)
ASMSRC	+= $(wildcard $(DATADIR)/*.i)


LISTINGS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.lst,$(CSRC))



# ---------------- Build objects ----------------

# st_asm.o must be first
OBJECTS += $(OBJDIR)/st_asm.o
OBJECTS += $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(CSRC))

LIBS = lib/vc.lib





# ---------------- Build tools ----------------

K68		= c:/LocalProjects/_amiga/k68/Release/k68.exe
VASM	= vasmm68k_mot_win32.exe
VC		= vc
LINK	= vlink

HATARI		= c:/Progs/Hatari/hatari.exe 
HATARI_OPT	+= --tos c:/Progs/Hatari/tos.img
HATARI_OPT	+= --confirm-quit 0
HATARI_OPT	+= --borders 0
HATARI_OPT	+= --fast-boot 1
HATARI_OPT	+= --memsize 2
HATARI_OPT	+= --harddrive c:/LocalProjects/_amiga/dreadmake/out
HATARI_OPT	+= --png-level 6 
#HATARI_OPT	+= --rs232-out /./COM6


# ---------------- Build options ----------------


DEFS	+= -DAMIGA=0
DEFS	+= -DATARI_ST=1
DEFS	+= -DWEAPONSPRITES=0
DEFS	+= -DSTRETCH_SCALING=1
DEFS	+= -DAMIGA_AUDIO_ENGINE_VERSION_2=0
#DEFS	+= -DENGINE_Y_MID=50		# 42
#DEFS	+= -DENGINE_Y_MAX=100		# 84
DEFS	+= -DENGINE_Y_MID=42
DEFS	+= -DENGINE_Y_MAX=84
DEFS	+= -DST_PLAY_MUSIC=1
DEFS	+= -DWALL_RENDERER_VERSION=1
DEFS	+= -DST_COMM_LEVEL=0					#	0: interrupt-driven,	1: polled TX,	2: BIOS

## Map loading
DEFS	+= -DUSE_EMBEDDED_MAP_DATA=1
DEFS	+= -DENABLE_MAP_FILE_LOADING=0
DEFS	+= -DENABLE_MAP_AUTOSTART=0				# use 1xx to immediately select skill


## Debugging
DEFS	+= -DWALL_ORDER_DEBUG=0
DEFS	+= -DPERSPECTIVE_CORRECTION=1			#	0: always OFF,    1: dynamic,    2: always ON
DEFS	+= -DDISABLE_ALL_THINGS=0
DEFS	+= -DMULTIPLAYER_MODE=1





CFLAGS	+= $(DEFS)

#CFLAGS	+= -O4 -notmpfile
# #CFLAGS	+= -O=23999 -maxoptpasses=32
#CFLAGS	+= -O3 -maxoptpasses=32 -notmpfile
CFLAGS  += -nostdlib
CFLAGS	+= -I. -Isrc

# Disable nasty warnings:
#	 62 - imported symbol <...> was not referenced

ASMFLAGS	+= -m68000 -spaces -Fhunk $(BUILD_PARM) -x
ASMFLAGS	+= -nowarn=62



# Disable nasty warnings:
#	 81	- casting number to address				//	(word*)0xdff001
#	170	- dead code elliminated
#	 65	- unused variable
#	152 - loop eliminated						//	do{...}while(0);
#	172	- optimizer passes						//	Dread_Build_Fillers()
#	 79	- offset equals size of object			//	e_visthings_max = e_visthings + ENGINE_MAX_VISTHINGS;

CFLAGS += -dontwarn=81 -dontwarn=170 -dontwarn=65 -dontwarn=152 -dontwarn=172 -dontwarn=79

LINKFLAGS += -b ataritos -e _start -gc-all -gc-empty
# -M$(MAPFILE) -gc-all 



# ---------------- Build generated sources ----------------


gen/gen_asmstruct_st.i : src/dread_data.h src/dread_estructs.h src/dread_asm.h scripts/asmstruct/_gen_asmstruct.awk Makefile.ST
	gawk -f scripts/asmstruct/_gen_asmstruct.awk $^ >$@



# ---------------- Build commands ----------------


#src/xcode.asm : $(XSRC) Makefile
#	$(K68) $(XSRC) -o $(SRCDIR)/xcode.asm -oh $(SRCDIR)/xcode.h

$(OBJDIR)/st_asm.o : $(ASMSRC) Makefile
	$(VASM) $(ASMFLAGS) -o $(OBJDIR)/st_asm.o $(SRCDIR)/st_asm.s $(DEFS)

$(OBJDIR)/%.o : $(SRCDIR)/%.c $(HEADERS) Makefile
	$(VC) $(CFLAGS) -c $(subst /,\,$<) -o $@

$(OBJDIR)/%.lst : $(SRCDIR)/%.c $(HEADERS) Makefile
	$(VC) $(CFLAGS) -S $(subst /,\,$<) -o $@


$(FINALEXE) : $(OBJECTS) Makefile
	$(LINK) $(LINKFLAGS) -o $(FINALEXE) $(subst /,\,$(OBJECTS)) $(LIBS)


#$(FINALEXE) : $(CSRC) $(OBJDIR)/st_asm.o Makefile
#	$(VC) +vc_st.cfg $(CFLAGS) -o $(FINALEXE) $(subst /,\,$(CSRC)) $(OBJDIR)/st_asm.o $(LIBS)




# ---------------- Build targets ----------------


all: $(FINALEXE)

rebuild: clean all

clean:
	-rm -rf ./$(OBJDIR)/*.o
	-rm -rf $(FINALEXE)

run_st: $(FINALEXE)
	$(HATARI) -W --machine st $(HATARI_OPT)

run_ste: $(FINALEXE)
	#$(HATARI) -W --machine ste $(HATARI_OPT)
	cmd /C run.bat

asm: $(LISTINGS)

.PHONY: all rebuild clean run_st run_ste asm
