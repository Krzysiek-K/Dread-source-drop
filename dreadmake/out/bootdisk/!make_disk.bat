@echo off


set TOOL=c:\Progs\WinUAEDemoToolchain\ToolChain\CopyToAdf.exe
set BOOT=..\..\obj\amiga_bootsector.bin


set ADF=BootTest.adf

%TOOL% BootTest %ADF% -new
%TOOL% %BOOT% %ADF% -bootblock


pause
