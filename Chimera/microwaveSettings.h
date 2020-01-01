#pragma once

#include "Expression.h"
#include <vector>

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

