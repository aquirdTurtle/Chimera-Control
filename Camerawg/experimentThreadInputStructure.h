#pragma once

#include <string>
#include "VariableSystem.h"
#include "DebuggingOptionsControl.h"

struct experimentThreadInputStructure
{
	std::string threadCurrentScript;
	unsigned int threadRepetitions;
	bool threadDontActuallyGenerate;
	std::string currentFolderLocation;
	std::vector<std::string> threadSequenceFileNames;
	debugOptions debugOptions;
	mainOptions settings;
	profileSettings profileInfo;
	int numberOfVariables;
	Communicator* comm;
};