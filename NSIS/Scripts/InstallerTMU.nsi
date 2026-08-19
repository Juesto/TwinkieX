!include "MUI2.nsh"

Name "TwinkieX for TMU"
OutFile "Output/TwinkieXTMU.exe"
InstallDir "$PROGRAMFILES32\TrackMania United\"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section ""
    SetOutPath $INSTDIR
    File "..\..\Builds\Win32\TMU\TwinkieTMU.dll"
    File "..\..\Builds\Win32\TMU\dinput8.dll"
    File "..\..\Builds\Win32\TMU\TwinkieTMU.pdb"
    File "..\..\Builds\Win32\TMU\dinput8.pdb"
    File "..\ReleaseDeps\TMU\discord_game_sdk.dll"
    SetOutPath "$DOCUMENTS\TwinkieX\"
    File /r "..\ReleaseDeps\DocumentsFolder\Fonts"
SectionEnd