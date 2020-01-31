#pragma once

#include "ParameterSystem/Expression.h"
#include <vector>

enum class microwaveDevice
{
	RohdeSchwarzGenerator,
	WindFreak,
	NONE // I.e. total safemode.
};


struct microwaveListEntry
{
	Expression frequency;
	Expression power;
};

struct microwaveSettings
{
	std::vector<microwaveListEntry> list;
	bool control = false;
};

