#include "pch.h"
#include "Global.h"

namespace Gecko::Exports::Global
{
	void yield()
	{
		auto Main = TheScriptManager.Scripts[TheScriptManager.ActiveScriptIdx].Ctxs.CtxMain;
		if (Main && TheScriptManager.ActiveCallbackType == CallbackType::Main) Main->Suspend();
		else if (TheScriptManager.ActiveCallbackType != CallbackType::Main) (void)0; // TODO: raise an error
	}

	void yieldFor(uint32_t For)
	{
		TheScriptManager.Scripts[TheScriptManager.ActiveScriptIdx].YieldFor = For ? For - 1 : 0;
		if (For) yield();
	}

	void print(std::string& Str)
	{
		std::cout << "[AS] " << Str << '\n';
	}

	void Registrar(asIScriptEngine* Engine)
	{
		RegisterStdString(Engine);

		Engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(print), asCALL_CDECL);
		Engine->RegisterGlobalFunction("void yield()", asFUNCTION(yield), asCALL_CDECL);
		Engine->RegisterGlobalFunction("void yield(uint)", asFUNCTION(yieldFor), asCALL_CDECL);
	}
}