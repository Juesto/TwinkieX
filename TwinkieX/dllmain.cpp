// Precompiled headers.
#include "pch.h"

// Class definition for the Twinkie class used by (Twinkie gTwinkie).
#include <Twinkie/Twinkie.h>

#include "Exports/Exports.h"

#define WaitUntilPause() Sleep(1); if (GetAsyncKeyState(VK_PAUSE) < 0) goto NoCreateTwinkie

// This is a separate thread that runs when the DLL is attached.
// It is used to initialize all hooks for the global Twinkie object (Twinkie gTwinkie).
static DWORD WINAPI InitializerThread([[maybe_unused]] LPVOID lpParameter)
{
    TwinkUiState::UiMgr = &gTwinkie.UiMgr;
    if (GetAsyncKeyState(VK_PAUSE) < 0) return FALSE;

    // While the game is not initialized, wait
    while (!gTwinkie.TrackmaniaMgr.GetApp())
    {
        WaitUntilPause();
    }
    while (!gTwinkie.TrackmaniaMgr.GetViewport())
    {
        WaitUntilPause();
    }
#ifdef MANIAPLANET
    while (!gTwinkie.TrackmaniaMgr.GetDirectXSwapChain())
    {
        WaitUntilPause();
    }
#else
    while (!gTwinkie.TrackmaniaMgr.GetDirectXDevice())
    {
        WaitUntilPause();
    }
#endif
#ifdef GAMEBOX
    // Nothing is set up properly when the intro is running, so wait
    while (!gTwinkie.TrackmaniaMgr.GetIsIntroOver())
    {
        WaitUntilPause();
    }
#endif
    
    if (GetAsyncKeyState(VK_PAUSE) < 0) goto NoCreateTwinkie;

    // After knowing that everything is initialized, we update everything
    gTwinkie.Update();

    // The return value indicates the success/failure of the thread. TRUE is successful.
    return TRUE;

NoCreateTwinkie:
    return FALSE;
}

// DllMain runs when the DLL is attached/detached to a process.
BOOL APIENTRY DllMain(HMODULE hModule, DWORD CallReason, LPVOID lpReserved)
{
    // Disables any subsequent (DLL_THREAD_ATTACH/DETACH) CallReasons for DllMain.
    DisableThreadLibraryCalls(hModule);

    switch (CallReason)
    {
    // Upon attaching to a process...
    case DLL_PROCESS_ATTACH:
        // Create the main thread and start it immediately.
        CreateThread(NULL, 0, InitializerThread, NULL, 0, 0);
		break;
    
    // These are disabled via (DisableThreadLibraryCalls) up above.
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    // Upon detaching from a process...
    case DLL_PROCESS_DETACH:
        // A non-null (lpReserved) means the process is exiting and every other thread is already dead,
        // so the global destructors must not wait on or release anything those threads owned.
        TwinkUiState::ProcessTerminating = lpReserved != nullptr;

        if (Veridian::VastVeridian) Veridian::VastVeridian->Save();
        break;
    }

    return TRUE;
}