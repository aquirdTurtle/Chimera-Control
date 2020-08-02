// created by Mark O. Brown
#pragma once
#include <string>

struct debugInfo
{
	bool showReadProgress = false;
	bool showWriteProgress = false;
	bool showCorrectionTimes = false;
	bool outputNiawgMachineScript = false;
	bool outputNiawgHumanScript = false;
	bool outputAgilentScript = false;
	bool outputExcessInfo = false;
	std::string message = "";
	bool showTtls = false;
	bool showDacs = false;
	bool outputNiawgWavesToText = false;
	unsigned long sleepTime = 0;
};
