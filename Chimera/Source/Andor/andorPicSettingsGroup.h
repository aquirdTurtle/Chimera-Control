#pragma once
#include "GeneralImaging/softwareAccumulationOption.h"
#include <array>
#include <vector>
#include "afxwin.h"

struct andorPicSettingsGroup{
	std::array<int, 4> colors;
	//std::vector<float> exposureTimesUnofficial;
	std::array<std::string, 4> thresholdStrs;
	std::array<std::vector<int>, 4> thresholds;
	std::array<softwareAccumulationOption, 4> saOpts;
};

