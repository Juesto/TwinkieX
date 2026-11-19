#pragma once

#include <Twinkie/Twinkie.h>
#include <Twinkie/TwinkLogs/CppLoggert/CppLoggert.h>
#include <imgui/imgui.h>

using ModuleCreateFn = IModule * (*)(TwinkTrackmania*, ILoggert*);

extern Twinkie gTwinkie;

extern TWINKIEX_API void AddModuleToQueue(ModuleCreateFn CreateModuleFn);