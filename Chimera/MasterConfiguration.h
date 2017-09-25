#pragma once
#include <string>
#include <array>
#include "DioSystem.h"
#include "DacSystem.h"

// This configuration system is different in style from the other configuration file system. This is designed to do more auto-saving and 
// auto-load at the beginning of the experiment. There is only supposed to be one such configuration file
class MasterConfiguration
{
	public:
		MasterConfiguration(std::string address);
		void save(MainWindow* mainWin, AuxiliaryWindow* auxWin, CameraWindow* camWin);
		void load(MainWindow* mainWin, AuxiliaryWindow* auxWin, CameraWindow* camWin);
	private:
		const std::string configurationFileAddress;
		const std::string version;
};

