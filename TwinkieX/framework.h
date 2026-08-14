#pragma once

#include <Version.h>

#pragma warning(disable : 4738 5045 4710)

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN  

// This warning always fires when including Windows.h for some reason
#pragma warning(disable : 5039 4514 4820 4668)
#pragma warning(push)

// Windows Header Files
#include <windows.h>
#include <shlobj_core.h>
#include <commdlg.h>

#pragma comment(lib, "Comdlg32.lib")

#pragma warning(pop)

// Architecture detection header file
#include <Utils/Arch.h>

// Disable all warnings emmitted by DirectX11
#pragma warning(disable : 4061 4365 4820 4365)
#pragma warning(push)

// DirectX (11 for TMCN, 9 otherwise)
#ifdef MANIAPLANET

#include <d3d11.h>
#include <dxgi.h>

#else

#include <d3d9.h>

#endif

#pragma warning(pop)

// dear ImGui
// No idea why this define is needed
#define IMGUI_DEFINE_MATH_OPERATORS

// Disable all warnings emmitted by dear ImGui
#pragma warning(disable : 4191 4582 4774 5219)
#pragma warning(push)

#include <imgui/imgui.h>

#ifdef GAMEBOX
#include <imgui/imgui_impl_dx9.h>
#else
#include <imgui/imgui_impl_dx11.h>
#endif

#include <imgui/imgui_impl_win32.h>


#pragma warning(pop)

#include <TMText/ImGuiTMText.h>

#include <GlyphTable/IconsForkAwesome.h>
#include <GlyphTable/IconsKenney.h>

#pragma warning(disable : 4244)
#pragma warning(push)
#include <algorithm>
#pragma warning(pop)

#include <vector>

#include <cstdlib>

#include <Veridian/Veridian.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#define _CRT_SECURE_NO_WARNINGS
#include <DiscordGameSDK/discord.h>
#undef _CRT_SECURE_NO_WARNINGS