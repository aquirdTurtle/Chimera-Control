#pragma once
#include "miscellaneousCommonFunctions.h"
#include "multiDimensionalKey.h"
#include <string>
#include "windows.h"
#include <vector>

struct variationRangeInfo
{
	double initialValue;
	double finalValue;
	unsigned int variations;
	bool leftInclusive;
	bool rightInclusive;
};


struct variableType
{
	std::string name;
	// whether this variable is constant or varies.
	bool constant;
	bool active = false;
	bool overwritten = false;
	// records which scan dimension the variable is in.
	USHORT scanDimension=1;
	std::vector<variationRangeInfo> ranges;
	/// this stuff used to be in the keyHandler system.
	std::vector<double> keyValues;
	// this might just be redundant with constant above.
	bool valuesVary;
};


