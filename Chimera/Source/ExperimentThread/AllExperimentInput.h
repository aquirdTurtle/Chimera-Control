#pragma once
#include "ExperimentThreadInput.h"

struct AllExperimentInput
{
	AllExperimentInput::AllExperimentInput () : masterInput (nullptr), cruncherInput (nullptr) { }
	ExperimentThreadInput* masterInput;
	atomCruncherInput* cruncherInput;
};

