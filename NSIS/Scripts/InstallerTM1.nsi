!include "MUI2.nsh"

Name "TwinkieX for TM1.0"
OutFile "Output/TwinkieXTM1.exe"
InstallDir "$PROGRAMFILES32\TrackMania\"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section ""
    SetOutPath $INSTDIR
    File "..\..\Builds\Win32\TM1\TwinkieTM1.dll"
    File "..\..\Builds\Win32\TM1\dinput8.dll"
    File "..\..\Builds\Win32\TM1\TwinkieTM1.pdb"
    File "..\..\Builds\Win32\TM1\dinput8.pdb"
    File "..\ReleaseDeps\TM1\discord_game_sdk.dll"
    SetOutPath "$DOCUMENTS\TwinkieX\"
    File /r "..\ReleaseDeps\DocumentsFolder\Fonts"
SectionEnd