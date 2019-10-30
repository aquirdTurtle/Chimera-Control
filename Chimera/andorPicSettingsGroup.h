#pragma once
#include "softwareAccumulationOption.h"
#include <array>
#include <vector>
#include "afxwin.h"

struct andorPicSettingsGroup
{
	std::array<int, 4> colors;
	//std::vector<float> exposureTimesUnofficial;
	std::array<std::string, 4> thresholdStrs;
	std::array<std::vector<int>, 4> thresholds;
	std::array<softwareAccumulationOption, 4> saOpts;
	// This variable is used by this control and communicated to the andor object, but is not directly accessed
	// while the main camera control needs to figure out how many pictures per repetition there are.
	//UINT picsPerRepetitionUnofficial;
};
