@echo off
call amipaths.bat

kingcon @assetlist.txt
if errorlevel 1 goto ERR


goto END

:ERR
echo.
echo.
echo.
echo ERROR!

:END
echo.
echo.
echo.
pause
