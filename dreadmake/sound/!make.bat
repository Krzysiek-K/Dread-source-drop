@echo off
copy %1 mod.music
del c:\Progs\WinUAEDemoToolchain\DH0\t\p61cache\*.music 
c:\Progs\WinUAEDemoToolchain\ConvertModToP61.bat %~dp0mod.music %~dp0

pause
