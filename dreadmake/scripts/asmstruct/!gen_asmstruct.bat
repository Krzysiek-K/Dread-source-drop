@echo off
gawk -f _gen_asmstruct.awk ..\..\src\dread_data.h ..\..\src\dread_engine.h >gen_asmstruct.inc
pause
