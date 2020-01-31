// created by Mark O. Brown
#pragma once
#include <string>
#include <array>

struct SegmentEnd
{
	enum class type
	{
		repeat,
		repeatTilTrig,
		once,
		repeatInf,
		onceWaitTrig,
	};
	static const std::array<type, 5> allTypes;
	static std::string toStr ( type t );
	static type fromStr ( std::string txt );
	static bool invovlesTrig ( type t );
};
