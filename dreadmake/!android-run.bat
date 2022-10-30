@echo off
set _oldpath=%cd%
cd /d %~dp0

set PLATFORM=%2
set PLATFORM=%PLATFORM:/=\%$
set PLATFORM=%PLATFORM:"$=$%
set PLATFORM=%PLATFORM:\$=$%
set PLATFORM=%PLATFORM:\$=$%
set PLATFORM=%PLATFORM:\$=$%
set PLATFORM=%PLATFORM:*\=%
set PLATFORM=%PLATFORM:*\=%
set PLATFORM=%PLATFORM:*\=%
set PLATFORM=%PLATFORM:*\=%
set PLATFORM=%PLATFORM:*\=%
set PLATFORM=%PLATFORM:*\=%
set PLATFORM=%PLATFORM:*\=%
set PLATFORM=%PLATFORM:*\=%
set PLATFORM=%PLATFORM:*\=%
set PLATFORM=%PLATFORM:*\=%
set PLATFORM=%PLATFORM:*\=%
set PLATFORM=%PLATFORM:*\=%
set PLATFORM=%PLATFORM:$=%

call !runmake.bat %PLATFORM% rebuild
