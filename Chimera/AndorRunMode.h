#pragma once

#include <string>

enum class AndorRunModes
{
	Video = 5,
	Kinetic = 3,
	Accumulate = 2,
	None=-1
};

std::string AndorRunModeText ( AndorRunModes mode );

