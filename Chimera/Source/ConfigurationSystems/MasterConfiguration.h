// created by Mark O. Brown
#pragma once
#include "ConfigurationSystems/Version.h"
#include <string>

class QtMainWindow;
class QtAuxiliaryWindow;
class QtAndorWindow;

// This configuration system is different in style from the other configuration file system. This is designed to do more auto-saving and 
// auto-load at the beginning of the experiment. There is only supposed to be one such configuration file
class MasterConfiguration{
	public:
		MasterConfiguration(std::string address);
		void save(QtMainWindow* mainWin, QtAuxiliaryWindow* auxWin, QtAndorWindow* camWin);
		void load(QtMainWindow* mainWin, QtAuxiliaryWindow* auxWin, QtAndorWindow* camWin);
	private:
		const std::string configurationFileAddress;
		// version 2.6: Added servo monitor-only option
		// version 2.7: Added Servo Ao config
		// version 2.8: Added servo avg num setting
		// version 2.9: Added Units Combo and experiment auto servo options to servo manager
		// version 2.10: added CalibrationManager
		// version 2.11: added agilent options to calibration manager.
		// version 2.12: added more fitting options and the calibrations to the calibration manager
		const Version version = Version("2.12");
};

