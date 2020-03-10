#pragma once
#include <array>
#include <vector>

struct PhotoDetectorCalibrationInfo
{
	std::vector<double> atPdCalCoeff;
	std::vector<double> atAtomsCalCoeff;
};

const std::array<PhotoDetectorCalibrationInfo, 8> AI_SYSTEM_CAL = { {
	{{0,2},{0,10}},
	{{0,2},{0,10}},
	{{0,2},{0,10}},
	{{0,2},{0,10}},
	{{0,2},{0,10}},
	{{0,2},{0,10}},
	{{0,2},{0,10}},
	{{0,2},{0,10}}} };

