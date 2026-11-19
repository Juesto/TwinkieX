#pragma once

#include <Twinkie/TwinkTrackmania/TwinkTrackmania.h>
#include <Twinkie/TwinkLogs/ILoggert.h>
#include <imgui/imgui.h>
#include <Veridian/Veridian.h>

// The module interface base-class.
class IModule
{
public:
	// Members

	// A unique identifier for each module.
	const char* ID = "UnnamedModule";
	// A name that can be displayed to the user as-is.
	const char* Name = "Unnamed module";

	// A reference to the TrackmaniaMgr.
	TwinkTrackmania* Twinkie = nullptr;

	// A reference to the Loggert used for logging.
	// Should be a global loggert for C++ modules (not the C++26 kind) like this one.
	ILoggert* Loggert = nullptr;

	// Controls whether or not the module is enabled. Use sparingly, as disabling an enabled module makes it unable to render anything.
	bool Enabled = true;

	// Behaviors

	// Ctor
	IModule(TwinkTrackmania& Twinkie, ILoggert& Loggert, const char* ID, const char* Name) : ID(ID), Name(Name), Twinkie(&Twinkie), Loggert(&Loggert) {}
	IModule() {}
	// Virtual Dtor
	virtual ~IModule() = default;

	// Methods

	// Called every frame
	virtual void Render() {};

	// Called every frame, but only when the Twinkie interface is visible
	virtual void RenderInterface() {};

	// Called every frame to render the module's menu item (under Modules)
	virtual void RenderMenu() {};

	// Called every frame to render the module's menu item (alongside Twinkie, Modules, Debug)
	virtual void RenderMenuMain() {};

	// Called every frame when the module's settings menu is open
	virtual void RenderSettings() {};
};