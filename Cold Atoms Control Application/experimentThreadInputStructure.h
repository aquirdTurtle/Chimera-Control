#pragma once

#include <string>

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
	bool threadUseDummyVariables;
	int threadDummyNum;
	std::string currentFolderLocation;
	std::vector<std::string> threadVariableNames;
	std::vector<std::string> threadSequenceFileNames;
};