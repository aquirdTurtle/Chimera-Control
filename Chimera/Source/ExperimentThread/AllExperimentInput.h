#pragma once
#include "ExperimentThreadInput.h"

struct AllExperimentInput
{
	AllExperimentInput::AllExperimentInput () :
		includesAndorRun (false), masterInput (NULL), cruncherInput (NULL) { }
	ExperimentThreadInput* masterInput;
	atomCruncherInput* cruncherInput;
	AndorRunSettings AndorSettings;
	baslerSettings baslerRunSettings;
	bool includesAndorRun;
};

