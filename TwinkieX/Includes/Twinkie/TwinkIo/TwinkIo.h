#include <dinput.h>
#include <imgui/imgui.h>

typedef HRESULT(WINAPI* DInput8GetDeviceStateFn)(IDirectInputDevice8* pDevice, DWORD cbData, LPVOID lpvData);
typedef HRESULT(WINAPI* DInput8GetDeviceDataFn)(IDirectInputDevice8* pDevice, DWORD cbObjectSize, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags);

extern HRESULT WINAPI hkGetDeviceState(IDirectInputDevice8* pDevice, DWORD cbData, LPVOID lpvData);
extern HRESULT WINAPI hkGetDeviceData(IDirectInputDevice8* pDevice, DWORD cbObjectSize, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags);

namespace TwinkIoState
{
	extern DInput8GetDeviceStateFn oGetDeviceState;
	extern DInput8GetDeviceDataFn  oGetDeviceData;

	extern std::atomic<bool> BlockKeyboard;
	extern std::atomic<bool> BlockMouse;
};

namespace TwinkUiState
{
	extern bool RenderUi;
};

class TwinkIo
{
public:
	TwinkIo() = default;

	void Update();
	void Sync();
};