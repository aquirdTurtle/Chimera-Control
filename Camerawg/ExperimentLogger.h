#pragma once

#include <string>
#include <sstream>
#include "ExperimentManager.h"

struct experimentThreadInput;
class MasterWindow;

class ExperimentLogger
{
	public:
		//void generateLog(MasterWindow* master);
	static void generateNiawgLog( experimentThreadInput* input, niawgPair<std::vector<std::fstream>>& niawgScripts,
								  std::vector<std::fstream > &intensityScripts, unsigned int repetitions );
		void exportLog();
		std::string getLog();
	private:
		std::stringstream logText;
};