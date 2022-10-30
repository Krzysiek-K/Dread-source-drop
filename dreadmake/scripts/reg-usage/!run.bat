@echo off
:LOOP
cls
gawk -f _reg_usage.awk	../../src/amiga_audio2.s ../../src/amiga_audio2_irq.s

echo.
echo.
echo.
pause
goto LOOP
