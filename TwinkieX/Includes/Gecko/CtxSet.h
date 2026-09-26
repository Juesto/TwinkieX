#pragma once
#include "pch.h"

namespace Gecko
{
	struct CtxSet
	{
		// void Main()
		asIScriptContext* CtxMain = nullptr;
		// void Render()
		asIScriptContext* CtxRender = nullptr;
		// void RenderInterface()
		asIScriptContext* CtxRenderInterface = nullptr;
		// void RenderMenu()
		asIScriptContext* CtxRenderMenu = nullptr;
		// void RenderMenuMain()
		asIScriptContext* CtxRenderMenuMain = nullptr;
		// void RenderSettings()
		asIScriptContext* CtxRenderSettings = nullptr;
		// void RenderEarly()
		asIScriptContext* CtxRenderEarly = nullptr;
		// void Update(float dt)
		asIScriptContext* CtxUpdate = nullptr;
		// void SimulationStep()
		asIScriptContext* CtxSimulationStep = nullptr;
		// void OnDisabled()
		asIScriptContext* CtxOnDisabled = nullptr;
		// void OnEnabled()
		asIScriptContext* CtxOnEnabled = nullptr;
		// void OnDestroyed()
		asIScriptContext* CtxOnDestroyed = nullptr;
		// void OnSettingsChanged()
		asIScriptContext* CtxOnSettingsChanged = nullptr;
		// void OnSettingsSave(Settings::Section& section)
		asIScriptContext* CtxOnSettingsSave = nullptr;
		// void OnSettingsLoad(Settings::Section& section)
		asIScriptContext* CtxOnSettingsLoad = nullptr;
		// void OnKeyPress(bool down, VirtualKey key)
		asIScriptContext* CtxOnKeyPress_VoidReturn = nullptr;
		// UI::InputBlocking OnKeyPress(bool down, VirtualKey key)
		asIScriptContext* CtxOnKeyPress_InputBlockingReturn = nullptr;
		// void OnMouseButton(bool down, int button, int x, int y)
		asIScriptContext* CtxOnMouseButton_VoidReturn = nullptr;
		// UI::InputBlocking OnMouseButton(bool down, int button, int x, int y)
		asIScriptContext* CtxOnMouseButton_InputBlockingReturn = nullptr;
		// void OnMouseMove(int x, int y)
		asIScriptContext* CtxOnMouseMove = nullptr;
		// void OnMouseWheel(int x, int y)
		asIScriptContext* CtxOnMouseWheel_VoidReturn = nullptr;
		// UI::InputBlocking OnMouseWheel(int x, int y)
		asIScriptContext* CtxOnMouseWheel_InputBlockingReturn = nullptr;
		// void OnProtocolUrl(const string &in path)
		asIScriptContext* CtxOnProtocolUrl = nullptr;
		// void OnLoadCallback(CMwNod@ nod)
		asIScriptContext* CtxOnLoadCallback = nullptr;

		const asIScriptContext*& operator[](const size_t Idx) const;
		asIScriptContext*& operator[](size_t Idx);

		void ReleaseAll();
	};
}