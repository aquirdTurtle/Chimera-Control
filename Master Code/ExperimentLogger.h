#pragma once

#include <string>
#include <sstream>

class MasterWindow;

class ExperimentLogger
{
	public:
		bool generateLog(MasterWindow* master);
		bool exportLog();
		std::string getLog();
	private:
		std::stringstream logText;
};