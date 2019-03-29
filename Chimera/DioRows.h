// created by Mark O. Brown
#pragma once
#include <string>
// as of now not yet used extensively in the actual dio system

struct DioRows
{
	enum class which
	{
		A, B, C, D
	};
	static std::string toStr ( which m );
	static which fromStr ( std::string txt );
};
