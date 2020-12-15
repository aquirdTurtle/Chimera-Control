#pragma once
#include "ConfigurationSystems/profileSettings.h"
#include "expSystemRunList.h"
#include <string>

struct autoCalConfigInfo { 
	profileSettings prof; 
	std::string fileName;
	std::string infoStr;  
}; 

const std::vector<autoCalConfigInfo> AUTO_CAL_LIST;
