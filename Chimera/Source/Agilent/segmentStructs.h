// created by Mark O. Brown
#pragma once
#include "ParameterSystem/Expression.h"
#include "Agilent/SegmentEnd.h"
#include <string>

struct rampInfo{
	bool isRamp = false;
	std::string type;
	Expression start;
	Expression end;

	std::string rampFileName="";
	std::vector<double> rampFileVals = std::vector<double> ();
	bool isFileRamp = false;
};

// short for modulation
struct segModInfo{
	bool modulationIsOn = false;
	// in MHz
	Expression frequency;
	// in radians
	Expression phase;
};

struct segPulseInfo {
	// as of October 6th, can be "sech", "gaussian", or "lorentzian"
	bool isPulse;
	std::string type;
	Expression amplitude;
	Expression width;
	// if pulseLength !>> pulseWidth, there will be a cutoff in the shape of the pulse.
	Expression vOffset;
	Expression tOffset;
};

struct segmentInfo {
	segPulseInfo pulse;
	segModInfo mod;
	rampInfo ramp;
	Expression holdVal;
	//
	Expression repeatNum;
	// in ms
	Expression time;
	// values such as repeat, repeat until trigger, no repeat, etc.
	SegmentEnd::type continuationType;
};

