#pragma once
#ifdef TMCN 
#include <Modules/IModule.h>

class VehicleExplorer : public IModule
{
public:
	VehicleExplorer(TwinkTrackmania& Twinkie, ILoggert& Loggert);

	virtual ~VehicleExplorer() override;

	virtual void Render() override;
};
#endif