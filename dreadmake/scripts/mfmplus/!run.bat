@echo off
:LOOP
cls
gawk -f _mfmp.awk _mfmp.awk
pause
goto LOOP
