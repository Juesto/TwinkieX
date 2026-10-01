#include "pch.h"
#include "ScriptManager.h"

#include "Exports/Global/Global.h"
#include "Exports/Trackmania/Trackmania.h"
#include "Exports/imgui/ExportImgui.h"

namespace Gecko
{
	ScriptManager* TheScriptManager = nullptr;

	// Must be synced with CallbackType
	const char* CallbackTypeSignatures[] =
	{
		"void Main()",
		"void Render()",
		"void RenderInterface()",
		"void RenderMenu()",
		"void RenderMenuMain()",
		"void RenderSettings()",
		"void RenderEarly()",
		"void Update(float dt)",
		"void SimulationStep()",
		"void OnDisabled()",
		"void OnEnabled()",
		"void OnDestroyed()",
		"void OnSettingsChanged()",
		"void OnSettingsSave(Settings::Section& section)",
		"void OnSettingsLoad(Settings::Section& section)",
		"void OnKeyPress(bool down, VirtualKey key)",
		"UI::InputBlocking OnKeyPress(bool down, VirtualKey key)",
		"void OnMouseButton(bool down, int button, int x, int y)",
		"UI::InputBlocking OnMouseButton(bool down, int button, int x, int y)",
		"void OnMouseMove(int x, int y)",
		"void OnMouseWheel(int x, int y)",
		"UI::InputBlocking OnMouseWheel(int x, int y)",
		"void OnProtocolUrl(const string &in path)",
		"void OnLoadCallback(CMwNod@ nod)"
	};

	void ScriptCtxLineCallback(asIScriptContext* Ctx, std::chrono::time_point<std::chrono::system_clock>* Timeout)
	{
		if (std::chrono::system_clock::now() >= *Timeout)
		{
			Ctx->Suspend();
			TheScriptManager->Scripts[TheScriptManager->ActiveScriptIdx].Enabled = false;
		}
	}

	void MessageCallback(const asSMessageInfo* msg, void* param)
	{
		const char* type = "ERR ";
		if (msg->type == asMSGTYPE_WARNING)
			type = "WARN";
		else if (msg->type == asMSGTYPE_INFORMATION)
			type = "INFO";
		printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
	}

	static std::u8string FromStr(const std::string& that)
	{
		return (char8_t*)that.c_str();
	}

	ScriptManager::ScriptManager()
	{
		Engine->SetEngineProperty(asEP_PROPERTY_ACCESSOR_MODE, 2);
#ifndef NDEBUG
		Engine->SetMessageCallback(asFUNCTION(MessageCallback), nullptr, asCALL_CDECL);
#endif

		Exports::Global::Registrar(Engine);
		Exports::Trackmania::Registrar(Engine);
		Exports::ImGui::Registrar(Engine);
	}

	ScriptManager::~ScriptManager()
	{
		for (auto& Script : Scripts)
		{
			Script.Ctxs.ReleaseAll();
		}

		Engine->ShutDownAndRelease();

		// Exports::Global::Cleanup();
		Exports::Trackmania::Cleanup();
		// Exports::ImGui::Cleanup();
	}

	void ScriptManager::BuildScriptByCallback(Script& Script, CallbackType CallbackType)
	{
		auto Module = Engine->GetModule((const char*)Script.Name.c_str());
		auto CallbackFn = Module->GetFunctionByDecl(CallbackTypeSignatures[(size_t)CallbackType]);
		if (!CallbackFn) return;

		auto Ctx = Engine->CreateContext();

		if (Script.Ctxs[(size_t)CallbackType])
		{
			Script.Ctxs[(size_t)CallbackType]->Release();
		}

		Script.Ctxs[(size_t)CallbackType] = Ctx;
		Ctx->Prepare(CallbackFn);
	}

	void ScriptManager::BuildScript(Script& Script)
	{
		if (Engine->GetModule((const char*)Script.Name.c_str()))
		{
			Engine->DiscardModule((const char*)Script.Name.c_str());
		}

		ScriptBuilder.StartNewModule(Engine, (const char*)Script.Name.c_str());
		ScriptBuilder.AddSectionFromMemory((const char*)Script.Name.c_str(), (const char*)Script.SourceCode.c_str());
		ScriptBuilder.BuildModule();

		CallbackType CallbackType = CallbackType::Main;
		for ([[maybe_unused]] auto& _ : CallbackTypeSignatures)
		{
			BuildScriptByCallback(Script, CallbackType);
			(*(uint32_t*)&CallbackType)++;
		}
	}

	void ScriptManager::BuildAll()
	{
		for (auto& Script : Scripts)
		{
			BuildScript(Script);
		}
	}

	void ScriptManager::RunAll(CallbackType CallbackType)
	{
		ActiveScriptIdx = 0;
		ActiveCallbackType = CallbackType;
		for (auto& Script : Scripts)
		{
			if (!Script.Enabled)
			{
				ActiveScriptIdx++;
				continue;
			}

			asIScriptContext*& CtxFromCallback = Script.Ctxs[(size_t)CallbackType];
			if (!CtxFromCallback)
			{
				ActiveScriptIdx++;
				continue;
			}

			if (CallbackType == CallbackType::Main && Script.YieldFor > 0)
			{
				Script.YieldFor--;
				ActiveScriptIdx++;
				continue;
			}

			static std::chrono::time_point<std::chrono::system_clock> When;
			When = std::chrono::system_clock::now() + TimeoutDurationMs;

			CtxFromCallback->SetLineCallback(asFUNCTION(ScriptCtxLineCallback), &When, asCALL_CDECL);

			int r = CtxFromCallback->Execute();
			if (r != asEXECUTION_FINISHED)
			{
				if (r == asEXECUTION_EXCEPTION)
				{
					printf("An exception '%s' occurred. Please correct the code and try again.\n", CtxFromCallback->GetExceptionString());
				}
			}

			if (r == asEXECUTION_SUSPENDED)
			{
				goto Continue;
			}

			// Main is only run once. A rebuild is required to re-run it.
			if (CallbackType == CallbackType::Main)
			{
				CtxFromCallback->Release();
				CtxFromCallback = nullptr;
			}
			else
			{
				CtxFromCallback->Release();
				CtxFromCallback = nullptr;

				CtxFromCallback = Engine->CreateContext();
				auto Module = Engine->GetModule((const char*)Script.Name.c_str());
				auto CallbackFn = Module->GetFunctionByDecl(CallbackTypeSignatures[(size_t)CallbackType]);
				CtxFromCallback->Prepare(CallbackFn);
			}
		Continue:
			ActiveScriptIdx++;
		}
		ActiveCallbackType = CallbackType::Main;
		ActiveScriptIdx = -1;
	}

	Script* ScriptManager::GetScriptFromCtx(asIScriptContext* Ctx)
	{
		for (auto& Script : Scripts)
		{
			size_t Idx = 0;
			for ([[maybe_unused]] auto& _ : CallbackTypeSignatures)
			{
				if (Script.Ctxs[Idx] == Ctx)
				{
					return &Script;
				}
				Idx++;
			}
		}
		return nullptr;
	}

	bool ScriptManager::AddFromFile(const std::filesystem::path& FilePath)
	{
		try
		{
			Script NewScript;
			NewScript.ScriptPath = FilePath;
			std::ostringstream SourceCodeStream;

			std::ifstream ScriptFile(FilePath);

			SourceCodeStream << ScriptFile.rdbuf();

			NewScript.SourceCode = FromStr(SourceCodeStream.str());
			NewScript.Name = FilePath.filename().u8string();

			Scripts.push_back(NewScript);
		}
		catch (...)
		{
			return false;
		}

		return true;
	}

	bool ScriptManager::AddFromSource(std::u8string Source, std::u8string ScriptName)
	{
		try
		{
			Script NewScript;
			NewScript.ScriptPath = ScriptName;
			NewScript.SourceCode = Source;
			NewScript.Name = ScriptName;

			Scripts.push_back(NewScript);
		}
		catch (...)
		{
			return false;
		}

		return true;
	}
}