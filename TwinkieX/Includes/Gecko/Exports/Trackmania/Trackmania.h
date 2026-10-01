#pragma once
#include "Trackmania/TMTypes.h"

namespace Gecko::Exports::Trackmania
{
	extern std::unordered_map<uint32_t, CMwClassInfo*> ClassIDToInfo;
	extern std::unordered_map<std::string, CMwClassInfo*> ClassNameToInfo;

	void Registrar(asIScriptEngine* Engine);
	void Cleanup();
}