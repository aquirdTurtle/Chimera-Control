#pragma once
#include <string>

struct AndorTemperatureStatus
{
	int temperature;
	int temperatureSetting;
	std::string andorRawMsg;
	std::string msg;
};