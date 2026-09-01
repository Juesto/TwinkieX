!include "MUI2.nsh"

Name "TwinkieX for TMCN"
OutFile "Output/TwinkieXTMCN.exe"
InstallDir "$PROGRAMFILES32\TrackmaniaChina\"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section ""
    SetOutPath $INSTDIR
    File "..\..\Builds\x64\TMCN\TwinkieTMCN.dll"
    File "..\..\Builds\x64\TMCN\dinput8.dll"
    File "..\..\Builds\x64\TMCN\TwinkieTMCN.pdb"
    File "..\..\Builds\x64\TMCN\dinput8.pdb"
    File "..\ReleaseDeps\TMCN\discord_game_sdk.dll"
    File "..\ReleaseDeps\TMCN\TrackmaniaCN.exe"
    File "..\ReleaseDeps\TMCN\TrackmaniaUS.exe"
    File "..\ReleaseDeps\TMCN\README.txt"
    SetOutPath "$DOCUMENTS\TwinkieX\"
    File /r "..\ReleaseDeps\DocumentsFolder\Fonts"
SectionEnd