Patch Info from TwinkieX for TrackmaniaChina

TwinkieX for TMCN includes extra .exes that are patched versions from the normal Trackmania.exe included with TMCN.
Includes:
- TrackmaniaCN.exe: the normal Trackmania.exe, but the timelimit is completely removed.
- TrackmaniaUS.exe: same as TrackmaniaCN.exe, but the game locale is forced to be English (en-us).

Deleting these extra files does not change Twinkie's behavior, and are just simple byte-patches of the original .exe.


If you are skeptical, below are instructions on how to patch YOUR Trackmania.exe to the same TrackmaniaCN.exe/TrackmaniaUS.exe files shipped with Twinkie:
- Open Trackmania.exe in a hex editor
For the language check patch:
- Go to Trackmania.exe+0xfb594, and change the byte there from 0x74 to 0x75 (this changes a JZ instruction to JNZ, which flips the valid language file check on its head, so invalid files become valid)
- Go to Trackmania.exe+0x1688a18, and change the next few bytes from "7A 00 68 00 2d 00 43 00 4e 00" to "65 00 6e 00 2d 00 55 00 53 00" (this changes which language file the game loads from zh-CH to en-US, it's a Unicode utf-16 string, that's why half of it is zeros)
- Go to Trackmania.exe+0x29da36, and change the 2 bytes there from "ff d0" to "90 90" (this is an extra failsafe that needs to be disabled, otherwise the game closes randomly)
For the timelimit patch:
- Go to Trackmania.exe+0x848b1c, and change the next few bytes from "41 bb c0 65 52 00" to "41 bb ff ff ff ff"
- Go to Trackmania.exe+0x848b39, and change the 2 bytes there from "74 05" to "75 40"
- Go to Trackmania.exe+0x848b6c, and change the next few bytes from "e8 3f 31 28 00" to "f2 48 90 48 90"

Have fun driving!