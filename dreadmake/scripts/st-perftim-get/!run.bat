@echo off
:LOOP
cls
gawk -f _perftim.awk _perftim.awk
pause
goto LOOP
