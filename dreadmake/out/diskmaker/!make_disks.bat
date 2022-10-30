@echo off


set TOOL=c:\Progs\WinUAEDemoToolchain\ToolChain\CopyToAdf.exe
set BOOT=c:\Progs\WinUAEDemoToolchain\ToolChain\CopyToAdf\Boot\stdboot3.bbk


echo|gawk "BEGIN{BINMODE=3;printf(\"Dread\n\")>\"Startup-Sequence\"}"


set ADF=Dread_1.adf

%TOOL% Dread %ADF% -new
%TOOL% %BOOT% %ADF% -bootblock
%TOOL% Dread %ADF%
%TOOL% Startup-Sequence %ADF% s

%TOOL% BigMap.map %ADF% maps
%TOOL% DemoMapAlternative.map %ADF% maps
%TOOL% FeatureTest.map %ADF% maps

set ADF=Dread_2.adf

%TOOL% Dread2 %ADF% -new
%TOOL% "Jammer - Big Concrete.map" %ADF% maps
%TOOL% "LaBodilsen - DS_Portland.map" %ADF% maps
%TOOL% "m3_mapper - Uberkoph.map" %ADF% maps
%TOOL% "Tsak - BigMapRemake.map" %ADF% maps
%TOOL% "Wuerfel21 - Chasm.map" %ADF% maps


pause
