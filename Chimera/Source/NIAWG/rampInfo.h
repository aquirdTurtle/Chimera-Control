#pragma once
#include <string>

struct rampInfo {
	std::string rampType;
	double initVal;
	double finVal;
	// only for agilent custom_tanh ramps
	double initShapeConst = -4;
	double finShapeConst = 4;
};
