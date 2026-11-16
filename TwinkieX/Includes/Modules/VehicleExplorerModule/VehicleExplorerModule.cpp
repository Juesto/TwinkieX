#include "pch.h"
#ifdef TMCN
#include <Modules/VehicleExplorerModule/VehicleExplorerModule.h>
#include <Modules/ModuleAutoExport.h>
#include <format>

#define CopyButton(Thing) do { if (SmallButton("Copy")) { TempPtrBuffer = PtrToString(Thing); SetClipboardText(TempPtrBuffer.c_str()); MemAddress = (uint64_t)Thing; } } while (false)
#define PreviewPtr(Thing) do { PushID(Thing); Text(#Thing ": %p", Thing); SameLine(); CopyButton(Thing); PopID(); } while (false)

VehicleExplorer::VehicleExplorer(TwinkTrackmania & Twinkie, ILoggert & Loggert) : IModule{ Twinkie, Loggert, "VehicleExplorer", "Vehicle Explorer" } 
{
}

VehicleExplorer::~VehicleExplorer() {}

void VehicleExplorer::Render()
{
	static uint64_t MemAddress = 0;
	static uint64_t MemOffset = 0;

	auto PtrToString = [](void* Ptr) { return std::format("{:08x}", (uint64_t)Ptr); };

	using namespace ImGui;

	auto Playground = Twinkie->ParamGet<CMwNod*>(Twinkie->GetApp(), "CurrentPlayground");
	
	Begin("VehicleExplorer");

	if (not Playground)
	{
		Text("No playground.");
		End();
		return;
	}
	
	auto Arena = Twinkie->ParamGet<CMwNod*>(Playground, "Arena");
	if (not Arena)
	{
		Text("No arena.");
		End();
		return;
	}

	auto Players = Twinkie->ParamGet<CFastBuffer<CMwNod*>>(Arena, "Players");
	if (not Players)
	{
		Text("No players.");
		End();
		return;
	}

	SeparatorText("Pointers");
	{
		std::string TempPtrBuffer;	
		
		for (auto& Player : *Players)
		{
			if (not Player) continue;

			PreviewPtr(Player);
			
			CMwNod* Vehicle = ReadAddr(CMwNod*, ((uintptr_t)Player) + 0x9F0);

			PreviewPtr(Vehicle);
		}
	}

	SeparatorText("Memory explore");
	{
		uint64_t StepSize     =  4;
		uint64_t StepSizeFast = 16;

		InputScalar("Offset", ImGuiDataType_U64, &MemOffset, &StepSize, &StepSizeFast, "%p");

		uint32_t* ResultingAddr = (uint32_t*)(MemAddress + MemOffset);
		uint32_t Result = MemAddress and ResultingAddr ? *ResultingAddr : 0;

		Text("%x\n%u\n%f\n", Result, Result, *(float*)&Result);
	}

	End();
}

#ifdef TMCN
MODULE_EXPORT(VehicleExplorer);
#endif

// NOTES:
// VelocityVec3 = Vehicle + 0x258
// Speed = Vehicle + 0x534
// WheelCount = Vehicle + 0x368
// Wheel1 = Vehicle + 0x368 + 8
// Wheel2 = Vehicle + 0x368 + 16
// ...
// WheelSteerAngle = Vehicle + 0x558
// WheelSurface = Vehicle + 0x55C
// WheelIcyness = Vehicle + 0x56C
// Steer = Vehicle + 0x4C8
// Steer = Vehicle + 0xA0
// Steer = Vehicle + 0x110
// Brake = Vehicle + 0x4CC
// Brake = Vehicle + 0x9C
// Brake = Vehicle + 0x10C
// Accel = Vehicle + 0x4D0
// Accel = Vehicle + 0x98
// Accel = Vehicle + 0x108
// AccelEngine = Vehicle + 0x210
// DigitalBrake = Vehicle + 0x4D8
// GroundContact = Vehicle + 0x544
// DriverProtector = Vehicle + 0x548
#endif