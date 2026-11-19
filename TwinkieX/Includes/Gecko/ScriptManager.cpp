#include "pch.h"
#include "ScriptManager.h"

#define CCPP_IMPL
#include <ccpp.h>

#include "Exports/Global/Global.h"
#include "Exports/Trackmania/Trackmania.h"
#include "Exports/imgui/ExportImgui.h"
#include "Exports/Reflection/Reflection.h"
#include "Exports/Net/Net.h"

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
			TheScriptManager->GetActiveScript()->Enabled = false;
		}
	}

	static std::u8string FromStr(const std::string& that)
	{
		return (char8_t*)that.c_str();
	}

	void MessageCallback(const asSMessageInfo* msg, void* param)
	{
		const char* type = "ERR ";
		if (msg->type == asMSGTYPE_WARNING)
			type = "WARN";
		else if (msg->type == asMSGTYPE_INFORMATION)
			type = "INFO";
#ifdef _DEBUG
		printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
#endif
		auto message = FromStr(std::format("{} ({}, {}) : {} : {}", msg->section, msg->row, msg->col, type, msg->message));
		if (msg->type == asMSGTYPE_WARNING)
			LoggertAngelscript.warn (message);
		else if (msg->type == asMSGTYPE_INFORMATION)
			LoggertAngelscript.print(message);
		else
			LoggertAngelscript.error(message);
	}

	ScriptManager::ScriptManager()
	{
		Engine->SetEngineProperty(asEP_PROPERTY_ACCESSOR_MODE, 2);
		Engine->SetMessageCallback(asFUNCTION(MessageCallback), nullptr, asCALL_CDECL);

		Exports::Trackmania::Registrar(Engine);
		Exports::Global::Registrar(Engine);
		Exports::ImGui::Registrar(Engine);
		Exports::Net::Registrar(Engine);
		Exports::Reflection::Registrar(Engine);
	}

	ScriptManager::~ScriptManager()
	{
		for (auto& Script : Scripts)
		{
			Script.Ctxs.ReleaseAll();
		}

		Engine->ShutDownAndRelease();

		Exports::Trackmania::Cleanup();
		// Exports::Global::Cleanup();
		// Exports::ImGui::Cleanup();
		// Exports::Reflection::Cleanup();
	}

	Script* ScriptManager::GetActiveScript()
	{
		return &(Scripts[ActiveScriptIdx]);
	}

	asIScriptContext* ScriptManager::GetActiveContext()
	{
		return Scripts[ActiveScriptIdx].Ctxs[(size_t)ActiveCallbackType];
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

	bool ScriptManager::IsExecutingRenderCallback()
	{
		switch (ActiveCallbackType)
		{
		case CallbackType::Render:
		case CallbackType::RenderInterface:
		case CallbackType::RenderMenu:
		case CallbackType::RenderMenuMain:
		case CallbackType::RenderSettings:
		case CallbackType::RenderEarly:
			return true;
		default:
			return false;
		}
	}

	void ScriptManager::BuildScript(Script& Script)
	{
		LoggertAngelscript.trace(FromStr(std::format("Compiling {}", (const char*)Script.Name.c_str())));

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

#ifndef _DEBUG
			static std::chrono::time_point<std::chrono::system_clock> When;
			When = std::chrono::system_clock::now() + TimeoutDurationMs;

			CtxFromCallback->SetLineCallback(asFUNCTION(ScriptCtxLineCallback), &When, asCALL_CDECL);
#endif

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
			std::ostringstream SourceCodeStream;

			std::ifstream ScriptFile(FilePath);

			SourceCodeStream << ScriptFile.rdbuf();

			// TODO: Error checking
			AddFromSource(FromStr(SourceCodeStream.str()), FilePath.filename().u8string(), FilePath);
		}
		catch (...)
		{
			return false;
		}

		return true;
	}

	bool ScriptManager::AddFromSource(std::u8string Source, std::u8string ScriptName, std::filesystem::path FilePath)
	{
		try
		{
			Script NewScript;
			NewScript.ScriptPath = FilePath.empty() ? ScriptName : FilePath;
			NewScript.Name = ScriptName;

			ccpp::processor Preprocessor;

			{
				Preprocessor.add_define("TWINKIEX");
				Preprocessor.add_define(TWINKIEX_GAME);
#ifdef TMCN
				Preprocessor.add_define("TMNEXT");
#endif
#ifdef _DEBUG
				Preprocessor.add_define("DEVELOPER");
#endif
#ifdef X86
				Preprocessor.add_define("MANIA32");
#else
				Preprocessor.add_define("MANIA64");
#endif
				// TODO: Maybe add a signature toggle?
				Preprocessor.add_define("SIG_DEVELOPER");

				auto NtDll = LoadLibrary(L"ntdll.dll");
				if (NtDll && GetProcAddress(NtDll, "wine_get_version"))
					Preprocessor.add_define("WINDOWS_WINE");

				Preprocessor.add_define("WINDOWS");
			}

			// ...ugh
			char* Buffer = new char[Source.size() + 1];
			strcpy_s(Buffer, Source.size() + 1, (const char*)Source.c_str());

			Preprocessor.process(Buffer, Source.size());
			NewScript.SourceCode = (char8_t*)Buffer;

			delete[] Buffer;

			Scripts.push_back(std::move(NewScript));
		}
		catch (...)
		{
			return false;
		}

		return true;
	}

	bool ScriptManager::ReloadScript(Script& Script)
	{
		try
		{
			std::ostringstream SourceCodeStream;
			std::ifstream ScriptFile(Script.ScriptPath);

			SourceCodeStream << ScriptFile.rdbuf();

			ccpp::processor Preprocessor;

			{
				Preprocessor.add_define("TWINKIEX");
				Preprocessor.add_define(TWINKIEX_GAME);
#ifdef TMCN
				Preprocessor.add_define("TMNEXT");
#endif
#ifdef _DEBUG
				Preprocessor.add_define("DEVELOPER");
#endif
#ifdef X86
				Preprocessor.add_define("MANIA32");
#else
				Preprocessor.add_define("MANIA64");
#endif
				// TODO: Maybe add a signature toggle?
				Preprocessor.add_define("SIG_DEVELOPER");

				auto NtDll = LoadLibrary(L"ntdll.dll");
				if (NtDll && GetProcAddress(NtDll, "wine_get_version"))
					Preprocessor.add_define("WINDOWS_WINE");

				Preprocessor.add_define("WINDOWS");
			}

			// ...ugh
			char* Buffer = new char[SourceCodeStream.str().size() + 1];
			strcpy_s(Buffer, SourceCodeStream.str().size() + 1, (const char*)SourceCodeStream.str().c_str());

			Preprocessor.process(Buffer, SourceCodeStream.str().size());
			Script.SourceCode = (char8_t*)Buffer;

			delete[] Buffer;
		}
		catch (...)
		{
			return false;
		}

		return true;
	}

	void ScriptManager::RenderDevMenu()
	{
		using namespace ImGui;

		if (BeginMenu("Developer"))
		{
			if (BeginMenu("Reload module"))
			{
				for (auto& Module : Scripts)
				{
					if (MenuItem((const char*)Module.Name.c_str()))
					{
						ReloadScript(Module);
						BuildScript(Module);
					}
				}

				ImGui::EndMenu();
			}
			// TODO: Rest

			ImGui::EndMenu();
		}
	}
}