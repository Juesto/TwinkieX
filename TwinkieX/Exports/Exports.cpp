#include "pch.h"
#include "Exports.h"

__declspec(dllexport) void AddModuleToQueue(ModuleCreateFn CreateModuleFn)
{
	gTwinkie.UiMgr->ModuleQueue.push_back(CreateModuleFn(&gTwinkie.TrackmaniaMgr, ImGui::GetCurrentContext()));
}