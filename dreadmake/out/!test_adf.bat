@echo off

set CONFIG=a500_BigWindow
set OPTIONS=
set KICK=KICK13

if "%3" == "turbo" set OPTIONS=%OPTIONS% -s floppy_speed!0


if "%2" == "" goto CONFIG_DONE
if %2 == "" goto CONFIG_DONE
set CONFIG=%2
:CONFIG_DONE

if %2 == a600 set KICK=KICK205
if %2 == a1200 set KICK=KICK31

:RUN

rem -s floppy_speed!0
c:\Progs\WinUAEDemoToolchain\ToolChain\startWinUAE.bat %CONFIG% %KICK% %~dpnx1 "" %OPTIONS%
