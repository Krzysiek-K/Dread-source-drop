@echo off


rem set TOOL=c:\Progs\WinUAEDemoToolchain\ToolChain\CopyToAdf_d.exe
set TOOL=c:\Progs\WinUAEDemoToolchain\ToolChain\CopyToAdf\VisualStudio\Out\CopyToAdf\Debug\CopyToAdf_d.exe
set BOOT=c:\Progs\WinUAEDemoToolchain\ToolChain\CopyToAdf\Boot\stdboot3.bbk

set ADFTOOL=c:\LocalProjects\_amiga\adftool\Debug\adftool.exe


echo|gawk "BEGIN{BINMODE=3;printf(\"Dread\n\")>\"Startup-Sequence\"}"


echo.
echo.
echo.
set ADF=Dread_1.adf
echo ================================ %ADF% ================================

%TOOL% Dread %ADF% -new
%TOOL% %BOOT% %ADF% -bootblock
%TOOL% Dread %ADF%
%TOOL% Startup-Sequence %ADF% s

%TOOL% BigMap.map %ADF% maps
%TOOL% DemoMap.map %ADF% maps
%TOOL% DemoMapAlternative.map %ADF% maps
%TOOL% FeatureTest.map %ADF% maps
%TOOL% MonsterTest.map %ADF% maps
%TOOL% MonsterRampage.map %ADF% maps
%TOOL% Wildcat3D.map %ADF% maps

%ADFTOOL% %ADF%


echo.
echo.
echo.
set ADF=Dread_2.adf
echo ================================ %ADF% ================================

%TOOL% Dread %ADF% -new
%TOOL% %BOOT% %ADF% -bootblock
%TOOL% Dread %ADF%
%TOOL% Startup-Sequence %ADF% s

%TOOL% "Jammer - Big Concrete.map" %ADF% maps
%TOOL% "LaBodilsen - DS_Portland.map" %ADF% maps
%TOOL% "m3_mapper - Uberkoph.map" %ADF% maps
%TOOL% "Wuerfel21 - Chasm.map" %ADF% maps
%TOOL% "Wuerfel21 - TechBase.map" %ADF% maps
%TOOL% "Tsak - BigMapRemake.map" %ADF% maps

%ADFTOOL% %ADF%




echo.
echo.
echo.
pause
