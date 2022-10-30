

SDK=Dread-SDK
MAIN=..\Dread-assets\dread-main
DB2=c:\Games\Doom Builder 2


clean:
	rm -rf $(SDK)


all: clean
	mkdir $(SDK)

	mkdir $(SDK)\app_data
	xcopy /E /I $(MAIN)\app_data $(SDK)\app_data
	copy ..\dreadtool\Release\DreadTool.exe $(SDK)
	rename $(SDK)\dreadtool.exe DreadTool.exe
	copy SDK-data\_LICENSE_.txt $(SDK)
	copy SDK-data\_README_.txt $(SDK)
	copy SDK-data\project.cfg $(SDK)
	copy SDK-data\user.cfg $(SDK)

	mkdir $(SDK)\WinUAE
	xcopy /E /I SDK-data\WinUAE $(SDK)\WinUAE
	copy ..\dreadmake\out\Dread.exe $(SDK)\WinUAE\DH0\Dread

	mkdir $(SDK)\export
	echo.>$(SDK)\export\_keep_this_dir_

	mkdir $(SDK)\pk3
	echo.>$(SDK)\pk3\_keep_this_dir_

	mkdir $(SDK)\dread
	xcopy /E /I $(MAIN)\dread $(SDK)\dread
	rmdir /S /Q $(SDK)\dread\.vs

	mkdir $(SDK)\maps
	xcopy /E /I $(MAIN)\maps-sdk-public $(SDK)\maps

	mkdir $(SDK)\maps-contrib
	copy $(MAIN)\maps-contrib $(SDK)\maps-contrib

	mkdir $(SDK)\mods
	echo.>$(SDK)\MODS\_keep_this_dir_

	mkdir $(SDK)\mods-contrib
	xcopy /E /I $(MAIN)\mods-contrib $(SDK)\mods-contrib

	mkdir $(SDK)\Configurations 
	mkdir $(SDK)\Configurations\Includes
	copy "$(DB2)\Configurations\KK_Dread.cfg" $(SDK)\Configurations
	copy "$(DB2)\Configurations\Includes\KK_Dread_*.cfg" $(SDK)\Configurations\Includes

sweep:
	rm -f $(SDK)\config.cfg
	rm -f $(SDK)\WinUAE\DH0\maps\start.map 
	rm -f $(SDK)\WinUAE\winuaebootlog.txt
