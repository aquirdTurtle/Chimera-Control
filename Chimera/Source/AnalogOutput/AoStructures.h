// created by Mark O. Brown
#pragma once

#include "ParameterSystem/Expression.h"
#include "GeneralObjects/commonTypes.h"
#include <array>


struct AoInfo{
	double currVal=0;
	std::string name="";
	double minVal=-10;
	double maxVal=10;
	// This is the value taken upon opening the master configuration. I don't think there's actually a way to set this 
	// currently. 
	double defaultVal=0;
	// notes are purely cosmetic. Can be used e.g. to store calibration curves for VCOs or detailed reminders about 
	// using a given dac.
	std::string note="";
};


struct AoCommandForm{
	// can either be "dac", "dacarange", or "daclinspace"
	std::string commandName;
	unsigned short line=0;
	timeType time;
	Expression initVal;
	Expression finalVal;
	Expression rampTime;
	Expression rampInc;
	Expression numSteps;
};


struct AoCommand{
	unsigned short line;
	double time;
	double value;
};


struct AoSnapshot
{
	double time;
	std::array<double, 24> dacValues;
};
