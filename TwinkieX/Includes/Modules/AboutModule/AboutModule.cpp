#include "pch.h"
#include "AboutModule.h"
#include <Modules/ModuleAutoExport.h>

AboutModule::AboutModule(TwinkTrackmania& Twinkie, ILoggert& Loggert) : IModule{ Twinkie, Loggert, "AboutModule", "About" }
{
	Loader.Load(IDB_PNG1, L"PNG");
	TwinkieXBanner.From(Loader.Data.data(), Loader.Data.size());
}

AboutModule::~AboutModule() {}

void AboutModule::RenderInterface()
{
	using namespace ImGui;

	if (not AboutWindowVisible) return;

	if (Begin("About", &AboutWindowVisible, ImGuiWindowFlags_AlwaysAutoResize))
	{
		Image(TwinkieXBanner.Get(), { 480, 180 });

		Text("TwinkieX for " TWINKIEX_GAMEFULL);
		Text(TWINKIEX_VERSION);

		Separator();

		Text("This version of TwinkieX is still \\$f00work-in-progress\\$z!");
	}

	End();
}

void AboutModule::RenderMenuMain()
{
	using namespace ImGui;

	if (MenuItem("About", nullptr, AboutWindowVisible))
		AboutWindowVisible = not AboutWindowVisible;
}

MODULE_EXPORT(AboutModule);