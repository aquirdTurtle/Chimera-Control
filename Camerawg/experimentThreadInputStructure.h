#pragma once

#include <string>
#include "VariableSystem.h"
#include "DebuggingOptionsControl.h"
#include "NiawgController.h"

struct experimentThreadInputStructure
{
	std::string currentScript;
	unsigned int repetitions;
	bool dontActuallyGenerate;
	std::string currentFolderLocation;
	std::vector<std::string> sequenceFileNames;
	debugInfo debugInfo;
	mainOptions settings;
	profileSettings profile;
	int variableNumber;
	Communicator* comm;
	NiawgController* niawg;
};