#pragma once
#include <Modules/IModule.h>
#include <Version.h>
#include <Gecko/Exports/imgui/Texture/Texture.h>
#include <Resources/ResourceLoader.h>

class AboutModule : public IModule
{
	bool AboutWindowVisible = false;
	ResourceLoader Loader;
	Texture TwinkieXBanner;
public:
	AboutModule() = delete;
	AboutModule(TwinkTrackmania& Twinkie, ILoggert& Loggert);

	virtual ~AboutModule();
	virtual void RenderInterface() override;
	virtual void RenderMenuMain() override;
};