#include "pch.h"
#include <Twinkie/TwinkIo/TwinkIo.h>
#include <Utils/Offsets.h>
#include <atomic>
#include <imgui/imgui.h>

namespace TwinkIoState
{
    DInput8GetDeviceStateFn oGetDeviceState = nullptr;
    DInput8GetDeviceDataFn  oGetDeviceData = nullptr;

    std::atomic<bool> BlockKeyboard{ false };
    std::atomic<bool> BlockMouse{ false };
};

HRESULT WINAPI hkGetDeviceState(IDirectInputDevice8* pDevice, DWORD cbData, LPVOID lpvData);
HRESULT WINAPI hkGetDeviceData(IDirectInputDevice8* pDevice, DWORD cbObjectSize, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags);

void TwinkIo::Sync()
{
    if (ImGui::GetCurrentContext() != nullptr && TwinkUiState::RenderUi)
    {
        ImGuiIO& io = ImGui::GetIO();
        TwinkIoState::BlockKeyboard.store(io.WantCaptureKeyboard, std::memory_order_relaxed);
        TwinkIoState::BlockMouse.store(io.WantCaptureMouse, std::memory_order_relaxed);
    }
    else
    {
        TwinkIoState::BlockKeyboard.store(false, std::memory_order_relaxed);
        TwinkIoState::BlockMouse.store(false, std::memory_order_relaxed);
    }
}

void TwinkIo::Update()
{
    IDirectInput8* pDInput8 = nullptr;
    HRESULT hr = DirectInput8Create(
        GetModuleHandle(NULL),
        DIRECTINPUT_VERSION,
        IID_IDirectInput8A,
        (void**)&pDInput8,
        NULL
    );

    if (FAILED(hr)) return;

    IDirectInputDevice8* pDummyDevice = nullptr;
    hr = pDInput8->CreateDevice(GUID_SysKeyboard, &pDummyDevice, NULL);
    if (FAILED(hr))
    {
        pDInput8->Release();
        return;
    }

    Unprotect(VirtualPtr(9, (uintptr_t)pDummyDevice), sizeof(uintptr_t));
    Unprotect(VirtualPtr(10, (uintptr_t)pDummyDevice), sizeof(uintptr_t));

    TwinkIoState::oGetDeviceState = (DInput8GetDeviceStateFn)VirtualWrite(9, (uintptr_t)pDummyDevice, (uintptr_t)&hkGetDeviceState);
    TwinkIoState::oGetDeviceData = (DInput8GetDeviceDataFn)VirtualWrite(10, (uintptr_t)pDummyDevice, (uintptr_t)&hkGetDeviceData);

    pDummyDevice->Release();
    pDInput8->Release();
}

HRESULT WINAPI hkGetDeviceState(IDirectInputDevice8* pDevice, DWORD cbData, LPVOID lpvData)
{
    HRESULT hr = TwinkIoState::oGetDeviceState(pDevice, cbData, lpvData);

    if (SUCCEEDED(hr) && TwinkUiState::RenderUi)
    {
        if (ImGui::GetIO().WantCaptureKeyboard)
        {
            ZeroMemory(lpvData, cbData);
        }
    }
    return hr;
}

HRESULT WINAPI hkGetDeviceData(IDirectInputDevice8* pDevice, DWORD cbObjectSize, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
    HRESULT hr = TwinkIoState::oGetDeviceData(pDevice, cbObjectSize, rgdod, pdwInOut, dwFlags);

    if (SUCCEEDED(hr) && pdwInOut && *pdwInOut > 0)
    {
        if (TwinkIoState::BlockKeyboard.load(std::memory_order_relaxed))
        {
            if ((dwFlags & DIGDD_PEEK) == 0)
            {
                *pdwInOut = 0;
            }
        }
    }

    return hr;
}