// created by Mark O. Brown
#pragma once
#include <array>
#include "GeneralObjects/commonTypes.h"

// this struct keeps variable names.
struct DoCommandForm
{
	// the hardware location of this line
	std::pair<unsigned short, unsigned short> line;
	// the time to make the change
	timeType time;
	// the evaluated values of the time for each varation.
	std::vector<double> timeVals;
	// the value to set it to. 
	bool value;
};

// no variables in this version. It's calculated each variation based on corresponding ComandForm structs.
struct DoCommand
{
	// the hardware location of this line
	std::pair<unsigned short, unsigned short> line;
	// the time to make the change
	double time;
	// the value to set it to. 
	bool value;
};

// an object constructed for having all info the ttls for a single time
struct DoSnapshot
{
	// the time of the snapshot
	double time;
	// all values at this time.
	std::array< std::array<bool, 16>, 4 > ttlStatus;
};

