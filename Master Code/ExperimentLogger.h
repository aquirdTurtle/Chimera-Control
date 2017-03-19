#pragma once

#include <string>
#include <sstream>

class MasterWindow;

class ExperimentLogger
{
	public:
		void generateLog(MasterWindow* master);
		void exportLog();
		std::string getLog();
	private:
		std::stringstream logText;
};