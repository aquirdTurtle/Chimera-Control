#pragma once

#include <string>
#include "VariableSystem.h"

struct experimentThreadInputStructure
{
	std::string threadCurrentScript;
	unsigned int threadAccumulations;
	bool threadConnectToMaster;
	bool threadGetVarFilesFromMaster;
	bool threadLogScriptAndParams;
	bool threadXScriptSaved;
	bool threadDontActuallyGenerate;
	bool threadProgramIntensityOption;
	std::string currentFolderLocation;
	std::vector<std::string> threadSequenceFileNames;
};