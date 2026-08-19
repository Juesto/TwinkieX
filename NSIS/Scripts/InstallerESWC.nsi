!include "MUI2.nsh"

Name "TwinkieX for ESWC"
OutFile "Output/TwinkieXESWC.exe"
InstallDir "$PROGRAMFILES32\Nadeo\Trackmania Nations ESWC\"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section ""
    SetOutPath $INSTDIR
    File "..\..\Builds\Win32\ESWC\TwinkieESWC.dll"
    File "..\..\Builds\Win32\ESWC\dinput8.dll"
    File "..\..\Builds\Win32\ESWC\TwinkieESWC.pdb"
    File "..\..\Builds\Win32\ESWC\dinput8.pdb"
    File "..\ReleaseDeps\ESWC\discord_game_sdk.dll"
    SetOutPath "$DOCUMENTS\TwinkieX\"
    File /r "..\ReleaseDeps\DocumentsFolder\Fonts"
SectionEnd