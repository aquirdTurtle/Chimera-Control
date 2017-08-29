#include "stdafx.h"
#include "ExperimentLogger.h"
#include <iomanip>
#include <ctime>
#include <iostream>
#include <fstream>
#include "VariableSystem.h"
#include <boost/algorithm/string/replace.hpp>

void ExperimentLogger::exportLog()
{
	// put this on the andor.
	std::fstream exportFile;
	exportFile.open( EXPERIMENT_LOGGING_FILES_PATH, std::ios::out);
	if (!exportFile.is_open())
	{
		thrower( "ERROR: logging file failed to open!" );
		return;
	}
	// export...
	exportFile << logText.str();
	exportFile.close();
}

std::string ExperimentLogger::getLog()
{
	return logText.str();
}
