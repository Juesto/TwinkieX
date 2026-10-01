#pragma once
#include <Gecko/ScriptManager.h>

namespace Gecko::Exports::Global
{
	// void yield();
	void yield();
	// void yield(uint frames);
	void yieldFor(uint32_t);
	// void print(const string&in);
	void print(std::string&);

	// Registers all globals with an engine.
	void Registrar(asIScriptEngine*);
}