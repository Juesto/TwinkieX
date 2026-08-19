#pragma once

#include <Twinkie/Twinkie.h>
#include <imgui/imgui.h>

using ModuleCreateFn = IModule * (*)(TwinkTrackmania*, ImGuiContext*);

extern Twinkie gTwinkie;

extern __declspec(dllexport) void AddModuleToQueue(ModuleCreateFn CreateModuleFn);