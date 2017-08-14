#pragma once

#include <string>
#include <sstream>

struct niawgIntensityThreadInput;
class DeviceWindow;

class ExperimentLogger
{
	public:
		//void generateLog(DeviceWindow* master);
	static void generateNiawgLog( MasterThreadInput* input, niawgPair<std::vector<std::fstream>>& niawgScripts,
								  std::vector<std::fstream > &intensityScripts, UINT repetitions );
		void exportLog();
		std::string getLog();
	private:
		std::stringstream logText;
};