#include <stdafx.h>
#include <ConfigurationSystems/profileSettings.h>
#include <ConfigurationSystems/ConfigSystem.h>

std::string profileSettings::configFilePath () {
	if (configLocation.size () == 0) {
		return configuration + "." + ConfigSystem::CONFIG_EXTENSION;
	}
	if (configLocation[configLocation.size () - 1] == '\\' || configLocation[configLocation.size () - 1] == '/') {
		return configLocation + configuration + "." + ConfigSystem::CONFIG_EXTENSION;
	}
	else {
		return configLocation + "\\" + configuration + "." + ConfigSystem::CONFIG_EXTENSION;
	}
}
