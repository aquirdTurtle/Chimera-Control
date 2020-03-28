#pragma once

#include "IDeviceCore.h"
#include <type_traits>


template <typename DeviceSettings, typename DeviceCore,
	// this last line asserts that the template argument DeviceCore inherits from IDeviceCore<DeviceSettingsStructure>. 
	std::enable_if_t<std::is_base_of<IDeviceCore<DeviceSettings>, DeviceCore>::value> * = nullptr>
class DeviceInterface
{
	public:
		virtual void handleSave (ConfigStream stream)=0;
		void rearrange ()=0;
		DeviceCore core;
};
