// created by Mark O. Brown
#pragma once
#include "ParameterSystem/Expression.h"
#include "Agilent/SegmentEnd.h"
#include <string>


struct rampFormat{
	bool isRamp = false;
	std::string type;
	Expression start;
	Expression end;
};


struct rampData{
	bool isRamp = false;
	std::string type;
	double start = 0;
	double end = 0;
};


// short for modulation
struct modFormat{
	bool modulationIsOn = false;
	// in MHz
	Expression frequency;
	// in radians
	Expression phase;
};


//struct modData{
//	bool modulationIsOn = false;
//	// in MHz
//	double frequency = 0;
//	// in radians
//	double phase = 0;
//};


struct pulseFormat {
	// as of October 6th, can be "sech", "gaussian", or "lorentzian"
	bool isPulse;
	std::string type;
	Expression amplitude;
	Expression width;
	// if pulseLength !>> pulseWidth, there will be a cutoff in the shape of the pulse.
	Expression vOffset;
	Expression tOffset;

};
//
//struct pulseData {
//	bool isPulse;
//	// as of October 6th, can be "sech", "gaussian", or "lorentzian"
//	std::string type;
//	double amplitude;
//	double width;
//	// if time !>> pulseWidth, there will be a cutoff in the shape of the pulse.
//	double vOffset;
//	double tOffset;
//};


struct segmentInfoInput {
	pulseFormat pulse;
	modFormat mod;
	rampFormat ramp;
	Expression holdVal;
	//
	Expression repeatNum;
	// in ms
	Expression time;
	// values such as repeat, repeat until trigger, no repeat, etc.
	SegmentEnd::type continuationType;
};



//struct segmentInfoFinal{
//	pulseData pulse;
//	modData mod;
//	rampData ramp;
//	double holdVal = 0;
//	// 
//	unsigned repeatNum = 0;
//	// in ms
//	double time = 0;
//	// values such as repeat, repeat until trigger, no repeat, etc.
//	SegmentEnd::type continuationType = SegmentEnd::type::once;
//};
