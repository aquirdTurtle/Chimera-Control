#pragma once

#include "GeneralObjects/IDeviceCore.h"
#include <type_traits>
#include <vector>
#include <functional>

/*
 * A small wrapper around a std::vector<std::reference_wrapper<IDeviceCore>>, mostly so that I can get the devices by
 * their parent type. Most of the devices should not need to be gotten by their parent type, all of their functionality
 * be handled in standard calls, but there are some exceptions, partially because of backwards compatibility with the 
 * older design. 
 */
class DeviceList
{
	public:
		std::vector<std::reference_wrapper<IDeviceCore>> list;
		template <class deviceType>
		deviceType& getSingleDevice ()
		{
			auto res = getDevicesByClass<deviceType> ();
			if (res.size () == 0)
			{
				thrower ("ERROR: no devices of the requested type were found in the Device List!");
			}
			if (res.size () != 1)
			{
				thrower ("ERROR: There is more than one class of given type requested by the "
						 "getSingleDeviceByClass function in the DeviceList! This function should"
						 " only be used to get a unique object out of the list.");
			}
			return res[0].get ();
		}

		template <class deviceType>
		std::vector<std::reference_wrapper<deviceType>> getDevicesByClass ()
		{
			std::vector<std::reference_wrapper<deviceType>> classList;
			for (auto& device : list)
			{
				// check if device can be converted to requested type. if so, it's a match. 
				if (dynamic_cast<deviceType*>(&device.get()) != nullptr)
				{
					classList.push_back (*(dynamic_cast<deviceType*>(&device.get())));
				}
			}
			return classList;
		}
	private:

};
