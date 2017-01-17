#pragma once
#include <string>
#include <array>
#include "TTL_System.h"
#include "DAC_System.h"

// This configuration system is different in style from the other configuration file system. This is designed to do more auto-saving and 
// auto-load at the beginning of the experiment. There is only supposed to be one such configuration file
class MasterConfiguration
{
	public:
		MasterConfiguration(std::string address);
		bool save(TtlSystem* ttls, DacSystem* dacs);
		bool load(TtlSystem* ttls, DacSystem& dacs, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master);
		bool updateDefaultTTLs(TtlSystem ttls);
		bool updateDefaultDacs(DacSystem dacs);
	private:
		std::array<std::array<bool, 16>, 4> defaultTTLs;
		std::array<int, 24> defaultDACs;
		const std::string configurationFileAddress;
		const std::string version;
};

