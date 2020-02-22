// created by Mark O. Brown
#pragma once
#include <string>
#include <array>
// as of now not yet used extensively in the actual dio system

struct DoRows
{
	enum class which
	{
		A, B, C, D
	};
	static const std::array<which, 4> allRows;
	static std::string toStr ( which m );
	static which fromStr ( std::string txt );
};


