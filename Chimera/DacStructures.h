#pragma once

#include "commonTypes.h"
#include <array>

struct DacCommandForm
{
	unsigned short line;
	timeType time;
	std::string initVal;
	std::string finalVal;
	std::string rampTime;
	std::string rampInc;
};

struct DacCommand
{
	unsigned short line;
	double time;
	double value;
};


struct DacSnapshot
{
	double time;
	std::array<double, 24> dacValues;
};
