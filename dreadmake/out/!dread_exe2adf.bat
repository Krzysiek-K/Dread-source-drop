@echo off

rem Usage:  <exe-path> <demo&diskname>


set TOOL=c:\Progs\WinUAEDemoToolchain\ToolChain\CopyToAdf.exe
set BOOT=c:\Progs\WinUAEDemoToolchain\ToolChain\CopyToAdf\Boot\stdboot3.bbk


set ADF=%2.adf
set FILE=%~nx1

echo|gawk "BEGIN{BINMODE=3;printf(\"%2\n\")>\"Startup-Sequence\"}"

copy %1 %2

%TOOL% %2 %ADF% -new
%TOOL% %BOOT% %ADF% -bootblock
%TOOL% %2 %ADF%
%TOOL% Startup-Sequence %ADF% s
rem for %%f in (*.map) do %TOOL% %%~nxf %ADF% maps


pause
