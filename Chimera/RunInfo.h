#pragma once
#include <vector>
#include <string>

struct RunInfo
{
	bool running;
	std::vector<std::string> currentlyRunningScripts;
};