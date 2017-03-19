#pragma once

#include <string>
#include <sstream>
#include "experimentProgrammingThread.h"

class MasterWindow;

class ExperimentLogger
{
	public:
		//void generateLog(MasterWindow* master);
	static void generateNiawgLog( experimentThreadInputStructure* input, niawgPair<std::vector<std::fstream>>& niawgScripts,
								  std::vector<std::fstream > &intensityScripts );
		void exportLog();
		std::string getLog();
	private:
		std::stringstream logText;
};