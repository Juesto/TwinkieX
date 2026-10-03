#include "pch.h"
#include <Modules/InputDisplayModule/InputDisplayModule.h>

const char* DashboardStyleNames[] = { "Pad", "Keyboard", "TMViz" };

ImVec2 InputDisplayModule::Lerp(ImVec2 A, ImVec2 B, float T)
{
	Vec2 A2 = Vec2(A.x, A.y);
	Vec2 B2 = Vec2(B.x, B.y);

	return A2 + ((B2 - A2) * T);
}

InputDisplayModule::InputDisplayModule(TwinkTrackmania& TrackmaniaMgr)
{
	using namespace Veridian;

	this->Twinkie = &TrackmaniaMgr;
	this->Name = "Input display";
	this->ID = "InputDisplayModule";
	this->StyleName = DashboardStyleNames[0];

	Veridian::Register("Dashboard", "Style", "Style", VSettingType::VString, &StyleName, false, { .Render = [this](VSetting* Setting) {
		ImGui::SeparatorText("Style");
		StyleIdx = std::string(DashboardStyleNames[0]) == StyleName ? 0 : (std::string(DashboardStyleNames[1]) == StyleName ? 1 : 2);
		StyleName = DashboardStyleNames[StyleIdx];
		ImGui::Combo("Style", &StyleIdx, DashboardStyleNames, IM_ARRAYSIZE(DashboardStyleNames));
		Setting->Set(DashboardStyleNames[StyleIdx]);
		return true;
		} });

	StyleIdx = std::string(DashboardStyleNames[0]) == StyleName ? 0 : (std::string(DashboardStyleNames[1]) == StyleName ? 1 : 2);
	StyleName = DashboardStyleNames[StyleIdx];

	Veridian::Register("Dashboard", "TMVizDim", "Force TMViz Dimensions", VSettingType::VBool, &ForceTMVizDimensions);

	Veridian::Register("Dashboard", "ColorSteer", "Steer", VSettingType::VVec4, &ColorSteer, false, { .BeforeRender = [](VSetting*) { ImGui::SeparatorText("Colors"); return true; } });
	Veridian::Register("Dashboard", "ColorSteerI", "Steer (inactive)", VSettingType::VVec4, &ColorSteerI);
	Veridian::Register("Dashboard", "ColorAccel", "Acceleration", VSettingType::VVec4, &ColorAccel);
	Veridian::Register("Dashboard", "ColorAccelI", "Acceleration (inactive)", VSettingType::VVec4, &ColorAccelI);
	Veridian::Register("Dashboard", "ColorBrake", "Brake", VSettingType::VVec4, &ColorBrake);
	Veridian::Register("Dashboard", "ColorBrakeI", "Brake (inactive)", VSettingType::VVec4, &ColorBrakeI);
	Veridian::Register("Dashboard", "ColorBackground", "Background", VSettingType::VVec4, &ColorBackground);

	Veridian::Register("Dashboard", "ShowInputDisplay", "ShowInputDisplay", VSettingType::VBool, &ShowInputDisplay, true);

	if (StyleName == "") StyleName = DashboardStyleNames[0];
}

InputDisplayModule::~InputDisplayModule() {}

void InputDisplayModule::RenderMenu()
{
	using namespace ImGui;
	if (MenuItem("Input display", nullptr, ShowInputDisplay))
		ShowInputDisplay = not ShowInputDisplay;
}

void InputDisplayModule::Render()
{
	using namespace ImGui;

	if (!ShowInputDisplay) return;

	struct
	{
		float Steer = 0.f;
		float Gas = 0.f;
		float Brake = 0.f;
	} InputInfo;

#ifdef GAMEBOX
#if !(defined(TM1) || defined(TMO))
	auto Race = Twinkie->ParamGet<CMwNod*>(Twinkie->GetApp(), "CurrentRace");
	if (not Race) return;

	auto Car = Twinkie->ParamGet<CMwNod*>(Race, "LocalPlayerMobil");
	if (not Car) return;
#elif defined(TM1)
	auto Races = Twinkie->ParamGet<CFastBuffer<CMwNod*>>(Twinkie->ParamGet<CMwNod*>(Twinkie->GetApp(), "Races"), "Nods");

	CMwNod* FinalRaceNod = nullptr;
	for (auto& RaceNod : *Races)
	{
		auto Players = Twinkie->ParamGet<CFastBuffer<CMwNod*>>(RaceNod, "Players");
		if (Players->Size != 0)
		{
			FinalRaceNod = RaceNod;
			break;
		}
	}
	if (not FinalRaceNod) return;

	auto pPlayersBuffer = Twinkie->ParamGet<CFastBuffer<CMwNod*>>(FinalRaceNod, "Players");
	if (not pPlayersBuffer) return;
	auto PlayersBuffer = *pPlayersBuffer;

	auto Car = Twinkie->ParamGet<CMwNod*>(*PlayersBuffer[0], "Mobil");
	if (not Car) return;
#elif defined(TMO)
	auto Race = Twinkie->ParamGet<CMwNod*>(Twinkie->GetApp(), "CurrentRace");
	if (not Race) return;

	auto pPlayersBuffer = Twinkie->ParamGet<CFastBuffer<CMwNod*>>(Race, "Players");
	if (not pPlayersBuffer) return;
	auto PlayersBuffer = *pPlayersBuffer;

	auto Car = Twinkie->ParamGet<CMwNod*>(*PlayersBuffer[0], "Mobil");
	if (not Car) return;
#endif
#ifndef TMU
	InputInfo.Steer = *Twinkie->ParamGet<float>(Car, "InputSteer");
	InputInfo.Gas = *Twinkie->ParamGet<float>(Car, "InputGas");
	InputInfo.Brake = *Twinkie->ParamGet<float>(Car, "InputBrake");
#else
	InputInfo.Steer = ReadAddr(float, (uintptr_t)Car + 0x58);
	InputInfo.Gas = ReadAddr(float, (uintptr_t)Car + 0x50);
	InputInfo.Brake = ReadAddr(float, (uintptr_t)Car + 0x54);
#endif
#elif defined(TMCN)
	auto Playground = Twinkie->ParamGet<CMwNod*>(Twinkie->GetApp(), "CurrentPlayground");

	if (not Playground)
	{
		return;
	}

	auto Arena = Twinkie->ParamGet<CMwNod*>(Playground, "Arena");
	if (not Arena)
	{
		return;
	}

	auto Players = Twinkie->ParamGet<CFastBuffer<CMwNod*>>(Arena, "Players");
	if (not Players)
	{
		return;
	}

	CMwNod* FinalPlayer = nullptr;
	for (auto& Player : *Players)
	{
		if (not Player) continue;
		FinalPlayer = Player;
		break;
	}
	if (not FinalPlayer) return;

	auto Vehicle = ReadAddr(CMwNod*, ((uintptr_t)FinalPlayer) + 0x9F0);
	if (not Vehicle) return;

	InputInfo.Steer = ReadAddr(float, ((uintptr_t)Vehicle) + 0xA0);
	InputInfo.Gas = ReadAddr(float, ((uintptr_t)Vehicle) + 0x98);
	InputInfo.Brake = ReadAddr(float, ((uintptr_t)Vehicle) + 0x9C);
#endif

	int DashboardWindowFlags = ImGuiWindowFlags_NoTitleBar;

	if (StyleName == "TMViz" and ForceTMVizDimensions)
	{
		DashboardWindowFlags |= ImGuiWindowFlags_NoResize;
		SetNextWindowSize(ImVec2(256, 140));
	}

	PushStyleColor(ImGuiCol_WindowBg, ColorBackground);
	PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	Begin("Dashboard##Inputs", nullptr, DashboardWindowFlags);

	PopStyleColor();
	PopStyleVar();

	if (StyleName == "Pad")
	{
		auto UIDrawList = GetWindowDrawList();
		auto CursorPos = GetCursorScreenPos();

		float WindowWidth = GetWindowWidth() / 3.f;
		float WindowHeight = GetWindowHeight();

		// float Width = WindowWidth * -InputInfo.Steer;

		// float OffsettedWidth = abs(WindowWidth - Width);

		auto UpperL = ImVec2(CursorPos.x + WindowWidth, CursorPos.y);
		auto LowerL = ImVec2(CursorPos.x + WindowWidth, CursorPos.y + WindowHeight);
		auto TipL = ImVec2(CursorPos.x, CursorPos.y + WindowHeight / 2.f);

		auto UpperLSteer = Lerp(UpperL, TipL, -InputInfo.Steer);
		auto LowerLSteer = Lerp(LowerL, TipL, -InputInfo.Steer);

		auto UpperR = ImVec2(CursorPos.x + WindowWidth * 2, CursorPos.y);
		auto LowerR = ImVec2(CursorPos.x + WindowWidth * 2, CursorPos.y + WindowHeight);
		auto TipR = ImVec2(CursorPos.x + WindowWidth * 3, CursorPos.y + WindowHeight / 2.f);

		auto UpperRSteer = Lerp(UpperR, TipR, InputInfo.Steer);
		auto LowerRSteer = Lerp(LowerR, TipR, InputInfo.Steer);

		UIDrawList->AddTriangleFilled(UpperL, LowerL, TipL, ColorConvertFloat4ToU32(ColorSteerI));
		UIDrawList->AddTriangleFilled(TipR, LowerR, UpperR, ColorConvertFloat4ToU32(ColorSteerI));

		if (InputInfo.Steer > 0)
		{
			UIDrawList->AddQuadFilled(UpperRSteer, LowerRSteer, LowerR, UpperR, ColorConvertFloat4ToU32(ColorSteer));
		}
		else if (InputInfo.Steer < 0)
		{
			UIDrawList->AddQuadFilled(UpperL, LowerL, LowerLSteer, UpperLSteer, ColorConvertFloat4ToU32(ColorSteer));
		}

		auto BottomCornerGas = ImVec2(CursorPos.x + WindowWidth * 2, CursorPos.y + WindowHeight / 2.f);
		auto TopCornerBrake = ImVec2(CursorPos.x + WindowWidth, CursorPos.y + WindowHeight / 2.f);

		UIDrawList->AddRectFilled(ImVec2(UpperL.x + 6.f, UpperL.y), ImVec2(BottomCornerGas.x - 6.f, BottomCornerGas.y - 3.f), InputInfo.Gas ? ColorConvertFloat4ToU32(ColorAccel) : ColorConvertFloat4ToU32(ColorAccelI));
		UIDrawList->AddRectFilled(ImVec2(TopCornerBrake.x + 6.f, TopCornerBrake.y + 3.f), ImVec2(LowerR.x - 6.f, LowerR.y), InputInfo.Brake ? ColorConvertFloat4ToU32(ColorBrake) : ColorConvertFloat4ToU32(ColorBrakeI));
	}
	else if (StyleName == "Keyboard")
	{
		auto UIDrawList = GetWindowDrawList();
		auto CursorPos = GetCursorScreenPos();

		float WindowWidth = GetWindowWidth() / 3.f;
		float WindowHeight = GetWindowHeight();

		// float Width = WindowWidth * -InputInfo.Steer;

		// float OffsettedWidth = abs(WindowWidth - Width);

		auto UpperL = ImVec2(CursorPos.x + WindowWidth, CursorPos.y);
		auto LowerL = ImVec2(CursorPos.x + WindowWidth, CursorPos.y + WindowHeight);
		auto UpperTipL = ImVec2(CursorPos.x + 13.f, CursorPos.y + 3.f + WindowHeight / 2.f);
		auto LowerTipL = ImVec2(CursorPos.x + 6.f, CursorPos.y + WindowHeight / 2.f);

		auto UpperR = ImVec2(CursorPos.x + WindowWidth * 2, CursorPos.y);
		auto LowerR = ImVec2(CursorPos.x + WindowWidth * 2, CursorPos.y + WindowHeight);
		auto UpperTipR = ImVec2(CursorPos.x + WindowWidth * 3 - 13.f, CursorPos.y + 3.f + WindowHeight / 2.f);
		auto LowerTipR = ImVec2(CursorPos.x + WindowWidth * 3 - 6.f, CursorPos.y + WindowHeight / 2.f);

		UIDrawList->AddRectFilled(LowerL, UpperTipL, ColorConvertFloat4ToU32(ColorSteerI));
		UIDrawList->AddRectFilled(LowerR, UpperTipR, ColorConvertFloat4ToU32(ColorSteerI));

		if (InputInfo.Steer > 0)
		{
			auto LerpedUpperR = Lerp(ImVec2(UpperR.x, UpperTipR.y), UpperTipR, InputInfo.Steer);
			auto LerpedLowerR = Lerp(ImVec2(LowerR.x, LowerTipR.y), LowerTipR, InputInfo.Steer);

			UIDrawList->AddRectFilled(LowerR, LerpedUpperR, ColorConvertFloat4ToU32(ColorSteer));
		}
		else if (InputInfo.Steer < 0)
		{
			auto LerpedUpperL = Lerp(ImVec2(UpperL.x, UpperTipL.y), UpperTipL, abs(InputInfo.Steer));
			auto LerpedLowerL = Lerp(ImVec2(LowerL.x, LowerTipL.y), LowerTipL, abs(InputInfo.Steer));

			UIDrawList->AddRectFilled(LowerL, LerpedUpperL, ColorConvertFloat4ToU32(ColorSteer));
		}

		auto BottomCornerGas = ImVec2(CursorPos.x + WindowWidth * 2, CursorPos.y + WindowHeight / 2.f);
		auto TopCornerBrake = ImVec2(CursorPos.x + WindowWidth, CursorPos.y + WindowHeight / 2.f);

		UIDrawList->AddRectFilled(ImVec2(UpperL.x + 6.f, UpperL.y), ImVec2(BottomCornerGas.x - 6.f, BottomCornerGas.y - 3.f), InputInfo.Gas ? ColorConvertFloat4ToU32(ColorAccel) : ColorConvertFloat4ToU32(ColorAccelI));
		UIDrawList->AddRectFilled(ImVec2(TopCornerBrake.x + 6.f, TopCornerBrake.y + 3.f), ImVec2(LowerR.x - 6.f, LowerR.y), InputInfo.Brake ? ColorConvertFloat4ToU32(ColorBrake) : ColorConvertFloat4ToU32(ColorBrakeI));
	}
	else if (StyleName == "TMViz")
	{
		// I hope I don't forget how this code works when I have to debug it

		auto UIDrawList = GetWindowDrawList();
		auto CursorPos = GetCursorScreenPos();

		float WindowWidth = GetWindowWidth();
		float WindowHeight = GetWindowHeight();

		auto CornerTop = ImVec2(CursorPos.x + WindowWidth / 2.f, CursorPos.y);
		auto CornerBottom = ImVec2(CursorPos.x + WindowWidth / 2.f, CursorPos.y + WindowHeight);
		auto CornerLeft = ImVec2(CursorPos.x, CursorPos.y + WindowHeight / 2.f);
		auto CornerRight = ImVec2(CursorPos.x + WindowWidth, CursorPos.y + WindowHeight / 2.f);

		auto MidTopLeft1 = Lerp(CornerTop, CornerLeft, 3.0 / 16.0);
		auto MidTopLeft2 = Lerp(CornerTop, CornerLeft, 4.0 / 16.0);

		auto MidBottomLeft1 = Lerp(CornerBottom, CornerLeft, 3.0 / 16.0);
		auto MidBottomLeft2 = Lerp(CornerBottom, CornerLeft, 4.0 / 16.0);

		auto MidTopRight1 = Lerp(CornerTop, CornerRight, 3.0 / 16.0);
		auto MidTopRight2 = Lerp(CornerTop, CornerRight, 4.0 / 16.0);

		auto MidBottomRight1 = Lerp(CornerBottom, CornerRight, 3.0 / 16.0);
		auto MidBottomRight2 = Lerp(CornerBottom, CornerRight, 4.0 / 16.0);

		auto ExactMid = ImVec2(CursorPos.x + WindowWidth / 2.f, CursorPos.y + WindowHeight / 2.f);

		auto MidTopLeft = ImVec2(ExactMid.x - (WindowWidth * 0.09375f), ExactMid.y - (WindowHeight / 35.f));
		auto MidBottomLeft = ImVec2(ExactMid.x - (WindowWidth * 0.09375f), ExactMid.y + (WindowHeight / 35.f));
		auto MidTopRight = ImVec2(ExactMid.x + (WindowWidth * 0.09375f), ExactMid.y - (WindowHeight / 35.f));
		auto MidBottomRight = ImVec2(ExactMid.x + (WindowWidth * 0.09375f), ExactMid.y + (WindowHeight / 35.f));

		UIDrawList->AddTriangleFilled(CornerLeft, MidTopLeft2, MidBottomLeft2, ColorConvertFloat4ToU32(ColorSteerI));
		UIDrawList->AddTriangleFilled(MidTopRight2, CornerRight, MidBottomRight2, ColorConvertFloat4ToU32(ColorSteerI));

#ifdef TMCN
#pragma warning(push)
#pragma warning(disable : 4267)
#endif
		std::vector<ImVec2> PointsAccel = { CornerTop, MidTopRight1, MidTopRight, MidTopLeft, MidTopLeft1 };
		UIDrawList->AddConvexPolyFilled(PointsAccel.data(), PointsAccel.size(), ColorConvertFloat4ToU32(InputInfo.Gas ? ColorAccel : ColorAccelI));

		std::vector<ImVec2> PointsBrake = { CornerBottom, MidBottomLeft1, MidBottomLeft, MidBottomRight, MidBottomRight1 };
		UIDrawList->AddConvexPolyFilled(PointsBrake.data(), PointsBrake.size(), ColorConvertFloat4ToU32(InputInfo.Brake ? ColorBrake : ColorBrakeI));
#ifdef TMCN
#pragma warning(pop)
#endif

		if (InputInfo.Steer < 0)
		{
			auto SteerTop = Lerp(MidTopLeft2, CornerLeft, abs(InputInfo.Steer));
			auto SteerBottom = Lerp(MidBottomLeft2, CornerLeft, abs(InputInfo.Steer));

			UIDrawList->AddQuadFilled(SteerBottom, SteerTop, MidTopLeft2, MidBottomLeft2, ColorConvertFloat4ToU32(ColorSteer));
		}
		else if (InputInfo.Steer > 0)
		{
			auto SteerTop = Lerp(MidTopRight2, CornerRight, InputInfo.Steer);
			auto SteerBottom = Lerp(MidBottomRight2, CornerRight, InputInfo.Steer);

			UIDrawList->AddQuadFilled(MidBottomRight2, MidTopRight2, SteerTop, SteerBottom, ColorConvertFloat4ToU32(ColorSteer));
		}
	}
	End();
}