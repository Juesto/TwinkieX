#pragma once

#include <Twinkie/TwinkDiscordRP/TwinkDiscordRP.h>
	
// Modules
#include <Modules/AppExplorerModule/AppExplorerModule.h>
#include <Modules/AboutModule/AboutModule.h>
#include <Modules/InputDisplayModule/InputDisplayModule.h>
#ifdef TMCN
#include <Modules/AddictionUnlimiterModule/AddictionUnlimiter.h>
#include <Modules/VehicleExplorerModule/VehicleExplorerModule.h>
#endif

#include <Twinkie/TwinkFilePicker/TwinkFilePickerExports.h>

// Veridian settings manager
#include <Veridian/Veridian.h>

#include <vector>

class TwinkUi;

#ifdef MANIAPLANET
using DirectXDevice = ID3D11Device;
using DirectXContext = ID3D11DeviceContext;

using PresentFn = HRESULT(__stdcall*)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
using ResizeBuffersFn = HRESULT(__stdcall*)(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
HRESULT hkResizeBuffers(IDXGISwapChain* pThis, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

#else
using DirectXDevice = IDirect3DDevice9;

using PresentFn = long(__stdcall*)(LPDIRECT3DDEVICE9 pDevice, LPVOID, LPVOID, HWND, LPVOID);
using ResetFn = long(__stdcall*)(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pParams);

static long __stdcall hkPresent(LPDIRECT3DDEVICE9 pDevice, LPVOID A, LPVOID B, HWND C, LPVOID D);
#ifdef TMU
static long __stdcall hkReset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pParams);
#endif

static LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

// The namespace that holds all globals for the TwinkUiState
namespace TwinkUiState
{
	// The original present function definition for the device.
	extern PresentFn oPresent;

#ifdef TMU
	// The original reset function definition for the device.
	// Only for TMU, the rest don't use reset.
	extern ResetFn oReset;
#endif

	// The original window process definition for the window.
	extern WNDPROC oWndProc;

	// Handle to the game window.
	extern HWND Window;

	// Check if dear ImGui has been initialized yet
	extern bool ImGuiInit;

	// Draw the UI
	extern bool RenderUi;

	// The device, since it's not given by any function
	extern DirectXDevice* Device;

	// The UiMgr
	extern TwinkUi* UiMgr;

#ifdef MANIAPLANET
	// The DirectX11 context.
	extern DirectXContext* Context;

	// The main render target view.
	extern ID3D11RenderTargetView* MainRenderTargetView;

	// The original resize buffers function for this DX11 device;
	extern ResizeBuffersFn oResizeBuffers;
#elif defined(GAMEBOX)
	// The new window size, when the window is resized.
	extern unsigned int WindowWidth;
	// The new window size, when the window is resized.
	extern unsigned int WindowHeight;
#endif
}

// The UI manager type, used by (TwinkUi gTwinkie.UiMgr).
class TwinkUi
{
public:
	// The Trackmania manager, set by (class Twinkie) to (TrackmaniaMgr Twinkie::TrackmaniaMgr)
	TwinkTrackmania* TrackmaniaMgr = nullptr;

	// The Discord Rich Presence manager, managed by this class
	TwinkDiscordRP* DiscordMgr = nullptr;

	std::vector<IModule*> Modules = {};

	// Used when another thread wants to add a module
	std::vector<IModule*> ModuleQueue = {};

	// The current font path.
	std::u8string FontPath = u8"";

	// The Documents folder path.
	std::filesystem::path DocumentsFolderPath;

	// The current font itself.
	ImFont* Font = nullptr;

	// The file picker.
	TwinkFilePicker* FilePicker = nullptr;

	// The size of the font.
	float UiScale = 1.f;

	// Settings window
	bool ShowSettings = false;

	// True if the current font is set to be destroyed/removed.
	bool DestroyCurrentFont = false;

	// Behaviors

	// Ctor
	TwinkUi(TwinkTrackmania& TrackmaniaMgr);
	// Dtor
	~TwinkUi();

	// Methods
	// imgui render calls are allowed here
	void Render();

	// Updates the internal hooks to be up-to-date
	void Update(std::filesystem::path DocumentsFolderPath);
};

#ifdef GAMEBOX
// Initialize dear ImGui
void InitImGui(DirectXDevice* Device);
#else
// Initialize dear ImGui
void InitImGui(DirectXContext* Context, DirectXDevice* Device);
#endif