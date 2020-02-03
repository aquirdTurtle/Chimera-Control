// created by Mark O. Brown
#pragma once

#include <string>
#include <array>

struct AndorRunModes
{
	enum class mode
	{
		Video = 5,
		Kinetic = 3,
		Accumulate = 2
	};
	static const std::array<mode, 3> allModes;
	static std::string toStr ( mode m );
	static mode fromStr ( std::string txt );
};

