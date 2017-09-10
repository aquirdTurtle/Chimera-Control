#pragma once

#include "Expression.h"
#include "commonTypes.h"
#include <array>

struct DacCommandForm
{
	// can either be "dac", "dacarange", or "daclinspace"
	std::string commandName;

	unsigned short line;
	timeType time;

	Expression initVal;
	Expression finalVal;
	Expression rampTime;
	Expression rampInc;
	Expression numSteps;
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
