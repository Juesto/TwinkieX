#include "pch.h"
#include "ExportImgui.h"

namespace Gecko::Exports::ImGui
{
	void BeginAs(std::string& s)
	{
		::ImGui::Begin(s.c_str());
	}

	void TextAs(std::string& s)
	{
		::ImGui::Text(s.c_str());
	}

	void Registrar(asIScriptEngine* Engine)
	{
		Engine->SetDefaultNamespace("UI");

		Engine->RegisterGlobalFunction("void Begin(string&in)", asFUNCTION(BeginAs), asCALL_CDECL);
		Engine->RegisterGlobalFunction("void Text(string&in)", asFUNCTION(TextAs), asCALL_CDECL);
		Engine->RegisterGlobalFunction("void End()", asFUNCTION(::ImGui::End), asCALL_CDECL);

		Engine->SetDefaultNamespace("");
	}
}