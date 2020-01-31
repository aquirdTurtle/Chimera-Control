// created by Mark O. Brown
#pragma once
#include <string>
// as of now not yet used extensively in the actual dio system

struct AgilentChannelMode
{
	enum class which
	{
		No_Control, Output_Off, DC, Sine, Square, Preloaded, Script
	};
	static const std::array<which, 7> allModes;
	static std::string toStr ( which m );
	static which fromStr ( std::string txt );
};


