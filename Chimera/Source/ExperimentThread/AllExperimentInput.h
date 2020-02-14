#pragma once
#include "ExperimentThreadInput.h"

struct AllExperimentInput
{
	AllExperimentInput::AllExperimentInput () : masterInput (NULL), cruncherInput (NULL) { }
	ExperimentThreadInput* masterInput;
	atomCruncherInput* cruncherInput;
};

