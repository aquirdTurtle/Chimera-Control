#pragma once
#include "GeneralImaging/softwareAccumulationOption.h"
#include <array>
#include <vector>

struct andorPicSettingsGroup{
	std::array<int, 4> colors = { 0 };
	//std::vector<float> exposureTimesUnofficial;
	std::array<std::string, 4> thresholdStrs = { "" };
	std::array<std::vector<int>, 4> thresholds = { {} };
	std::array<softwareAccumulationOption, 4> saOpts = {};
	std::string tMode="";
	int picScaleFactor=50;
};

