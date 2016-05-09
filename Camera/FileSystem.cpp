#include "FileSystem.h"
#include "Windows.h"
#include <fstream>
#include "externals.h"

FileSystem::FileSystem(std::string fileSystemPath)
{
	FILE_SYSTEM_PATH = fileSystemPath;
}
FileSystem::~FileSystem()
{
	// nothing for destructor right now
}

int FileSystem::newConfiguration(std::string newConfigurationName)
{

	return 0;
}
int FileSystem::openConfiguration(std::string configurationNameToOpen)
{

	return 0;
}
int FileSystem::saveConfiguration()
{
	// check if file exists
	struct stat buffer;
	if (stat((FILE_SYSTEM_PATH + configurationName).c_str(), &buffer) != 0)
	{
		// File doesn't exist. Save as.
	}
	// else open it.
	std::ofstream configurationSaveFile(FILE_SYSTEM_PATH + configurationName);
	/// Start Outputting information
	// # of exposure times
	configurationSaveFile << std::to_string(eExposureTimes.size()) + "\n";
	// each expousure time
	for (int exposureTimesInc = 0; exposureTimesInc < eExposureTimes.size(); exposureTimesInc++)
	{
		configurationSaveFile << std::to_string(eExposureTimes[exposureTimesInc]) + "\n";
	}
	// Image Parameters
	configurationSaveFile << eLeftImageBorder + "\n";
	configurationSaveFile << eRightImageBorder + "\n";
	configurationSaveFile << eHorizontalBinning + "\n";
	configurationSaveFile << eBottomImageBorder + "\n";
	configurationSaveFile << eTopImageBorder + "\n";
	configurationSaveFile << eVerticalBinning + "\n";
	// gain settings
	configurationSaveFile << eEMGainMode + "\n";
	configurationSaveFile << eEMGainLevel + "\n";
	// pictures per experiment
	configurationSaveFile << ePicturesPerExperiment + "\n";
	// experiment # per stack
	configurationSaveFile << eExperimentsPerStack + "\n";
	// stack #
	configurationSaveFile << eCurrentAccumulationStackNumber + "\n;";
	
	/*
	- Pictures per experiment
	- experiment #
	- Camera Mode
	- Kinetic Cycle Time
	- Accumulation Cycle Time
	- Accumulation #
	*/
	return 0;
}
int FileSystem::saveConfigurationAs(std::string newConfigurationName)
{

	return 0;
}
int FileSystem::renameConfiguration(std::string newConfigurationNAme)
{

	return 0;
}
int FileSystem::deleteConfiguration()
{

	return 0;
}
int FileSystem::checkSave()
{

	return 0;
}