#pragma once
#include "pch.h"
#include "Script.h"

namespace Gecko
{
	void ScriptCtxLineCallback(asIScriptContext* Ctx, std::chrono::time_point<std::chrono::system_clock>* Timeout);

	// Must be synced with CallbackType
	extern const char* CallbackTypeSignatures[24];

	// Must be synced with CallbackTypeSignatures
	enum class CallbackType : uint32_t
	{
		// void Main()
		Main = 0,
		// void Render()
		Render,
		// void RenderInterface()
		RenderInterface,
		// void RenderMenu()
		RenderMenu,
		// void RenderMenuMain()
		RenderMenuMain,
		// void RenderSettings()
		RenderSettings,
		// void RenderEarly()
		RenderEarly,
		// void Update(float dt)
		Update,
		// void SimulationStep()
		SimulationStep,
		// void OnDisabled()
		OnDisabled,
		// void OnEnabled()
		OnEnabled,
		// void OnDestroyed()
		OnDestroyed,
		// void OnSettingsChanged()
		OnSettingsChanged,
		// void OnSettingsSave(Settings::Section& section)
		OnSettingsSave,
		// void OnSettingsLoad(Settings::Section& section)
		OnSettingsLoad,
		// void OnKeyPress(bool down, VirtualKey key)
		OnKeyPress_VoidReturn,
		// UI::InputBlocking OnKeyPress(bool down, VirtualKey key)
		OnKeyPress_InputBlockingReturn,
		// void OnMouseButton(bool down, int button, int x, int y)
		OnMouseButton_VoidReturn,
		// UI::InputBlocking OnMouseButton(bool down, int button, int x, int y)
		OnMouseButton_InputBlockingReturn,
		// void OnMouseMove(int x, int y)
		OnMouseMove,
		// void OnMouseWheel(int x, int y)
		OnMouseWheel_VoidReturn,
		// UI::InputBlocking OnMouseWheel(int x, int y)
		OnMouseWheel_InputBlockingReturn,
		// void OnProtocolUrl(const string &in path)
		OnProtocolUrl,
		// void OnLoadCallback(CMwNod@ nod)
		OnLoadCallback
	};

	class ScriptManager
	{
	public:
		std::vector<Script> Scripts;
		asIScriptEngine* Engine = asCreateScriptEngine();
		CScriptBuilder ScriptBuilder;
		intmax_t ActiveScriptIdx = -1;
		CallbackType ActiveCallbackType = CallbackType::Main;
		std::chrono::milliseconds TimeoutDurationMs = 1000ms;

		ScriptManager();
		~ScriptManager();

		void BuildAll();
		void BuildScript(Script&);
		void BuildScriptByCallback(Script&, CallbackType);
		void RunAll(CallbackType CallbackType);
		Script* GetScriptFromCtx(asIScriptContext* Ctx);
		bool AddFromFile(const std::filesystem::path& FilePath);
		bool AddFromSource(std::u8string Source, std::u8string ScriptName);
	};

	extern ScriptManager TheScriptManager;
}