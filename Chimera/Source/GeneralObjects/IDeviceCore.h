#pragma once

#include "ParameterSystem/ParameterSystemStructures.h"
#include <string>
#include <vector>
#include "afxwin.h"

template <class DeviceSettings>
class IDeviceCore
{
	public:
		virtual void calculateVariations (std::vector<parameterType> params, DeviceSettings settings) = 0;
		virtual void programNow () = 0;
		virtual void programVariation (UINT variation) = 0;
		virtual void logSettings (DeviceSettings)=0;
		virtual DeviceSettings getSettingsFromConfig (ConfigStream stream, Version ver) = 0;
		std::string delimiter;
	private:

};
