@echo off

rem
rem ---- Usage: !runmake.bat <platform> <command> ----
rem
rem Platforms:
rem
rem Commands:
rem		build		- build and run
rem		rebuild		- rebuild and run
rem		runonly		- only run existing executable
rem		buildonly	- build without running
rem		rebuildonly	- rebuild without running
rem

call amipaths.bat

set PLATFORM=%1
set COMMAND=%2

echo PLATFORM = %PLATFORM%
echo COMMAND  = %COMMAND%


goto %PLATFORM%

:Bootloader
set MAKETARGETS=bootloader
goto RUN

:Bootloader-A500
set MAKETARGETS=bootloader_a500
goto RUN

:Bootloader-A1200
set MAKETARGETS=bootloader_a1200
goto RUN

:A500
set RUNTARGET=run_a500
goto MAKE

:A1200
set RUNTARGET=run_a1200
goto MAKE

:A4000-60
set RUNTARGET=run_a4000_60
goto MAKE

:ST
set MAKEFLAGS= --file=Makefile.ST
set RUNTARGET=run_st
goto MAKE

:STe
set MAKEFLAGS= --file=Makefile.ST
set RUNTARGET=run_ste
goto MAKE

:MAKE

set MAKETARGETS=
if "%COMMAND%" == "build"		set MAKETARGETS=all %RUNTARGET%
if "%COMMAND%" == "bootloader"	set MAKETARGETS=bootloader
if "%COMMAND%" == "rebuild"		set MAKETARGETS=rebuild %RUNTARGET%
if "%COMMAND%" == "runonly"		set MAKETARGETS=%RUNTARGET%
if "%COMMAND%" == "buildonly"	set MAKETARGETS=all
if "%COMMAND%" == "rebuildonly"	set MAKETARGETS=rebuild

:RUN

echo --- make %MAKEFLAGS% %MAKETARGETS% ---
set CURDIR=%cd%
set TMPERR=_tmp_errors
del %TMPERR% 2>nul
make %MAKEFLAGS% %MAKETARGETS% 2>>%TMPERR%
set ERR=%errorlevel%
gawk -f c:\!DLL\_vbccerrors.awk %TMPERR%
del %TMPERR% 2>nul


echo End.
