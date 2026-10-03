#include "pch.h"
#include "Global.h"

namespace Gecko::Exports::Global
{
	void yield()
	{
		auto Main = TheScriptManager->GetActiveScript()->Ctxs.CtxMain;
		if (Main && TheScriptManager->ActiveCallbackType == CallbackType::Main) Main->Suspend();
		else if (TheScriptManager->ActiveCallbackType != CallbackType::Main) (void)0; // TODO: raise an error
	}

	void yieldFor(uint32_t For)
	{
		TheScriptManager->GetActiveScript()->YieldFor = For ? For - 1 : 0;
		if (For) yield();
	}

	void print([[maybe_unused]] std::string& Str)
	{
#ifdef _DEBUG
		std::cout << "[AS] " << Str << '\n';
#endif
	}

	void Registrar(asIScriptEngine* Engine)
	{
		RegisterStdString(Engine);
		RegisterScriptArray(Engine, true);

		Engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(print), asCALL_CDECL);
		Engine->RegisterGlobalFunction("void yield()", asFUNCTION(yield), asCALL_CDECL);
		Engine->RegisterGlobalFunction("void yield(uint)", asFUNCTION(yieldFor), asCALL_CDECL);
	}
}