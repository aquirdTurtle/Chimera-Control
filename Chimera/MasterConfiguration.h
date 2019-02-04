#pragma once
#include "DioSystem.h"
#include "Version.h"
#include <string>
#include <array>


class MainWindow;
class AuxiliaryWindow;
class AndorWindow;


// This configuration system is different in style from the other configuration file system. This is designed to do more auto-saving and 
// auto-load at the beginning of the experiment. There is only supposed to be one such configuration file
class MasterConfiguration
{
	public:
		MasterConfiguration(std::string address);
		void save(MainWindow* mainWin, AuxiliaryWindow* auxWin, AndorWindow* camWin);
		void load(MainWindow* mainWin, AuxiliaryWindow* auxWin, AndorWindow* camWin);
	private:
		const std::string configurationFileAddress;
		const Version version = Version("2.5");
};

