#pragma once

#include "commonTypes.h"
#include <array>

struct DacCommandForm
{
	// can either be "dac", "dacarange", or "daclinspace"
	std::string commandName;

	unsigned short line;
	timeType time;
	std::string initVal;
	std::string finalVal;
	std::string rampTime;
	std::string rampInc;
	std::string numSteps;
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
