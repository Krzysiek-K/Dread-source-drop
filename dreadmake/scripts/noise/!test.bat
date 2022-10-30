@echo off
:LOOP
gawk -f _rand.awk _rand.awk
pause
goto LOOP
