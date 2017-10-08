#pragma once
#include "Expression.h"
#include <string>
#include "windows.h"

struct rampFormat
{
	bool isRamp = false;
	std::string type;
	Expression start;
	Expression end;
};;


struct rampData
{
	bool isRamp = false;
	std::string type;
	double start = 0;
	double end = 0;
};;


// short for modulation
struct modFormat
{
	bool modulationIsOn = false;
	// in MHz
	Expression frequency;
	// in radians
	Expression phase;
};;


struct modData
{
	bool modulationIsOn = false;
	// in MHz
	double frequency = 0;
	// in radians
	double phase = 0;
};;


struct pulseFormat
{
	// as of October 6th, can be "sech", "gaussian", or "lorentzian"
	bool isPulse;
	std::string type;
	Expression amplitude;
	Expression width;
	// if pulseLength !>> pulseWidth, there will be a cutoff in the shape of the pulse. The peak of the pulse is always
	// centered.
	Expression offset;
};

struct pulseData
{
	bool isPulse;
	// as of October 6th, can be "sech", "gaussian", or "lorentzian"
	std::string type;
	double amplitude;
	double width;
	// if time !>> pulseWidth, there will be a cutoff in the shape of the pulse. The peak of the pulse is always
	// centered.
	double offset;
};


struct segmentInfoInput
{
	pulseFormat pulse;
	modFormat mod;
	rampFormat ramp;
	Expression holdVal;
	//
	Expression repeatNum;
	// in ms
	Expression time;
	// values such as repeat, repeat until trigger, no repeat, etc.
	int continuationType;
};



struct segmentInfoFinal
{
	pulseData pulse;
	modData mod;
	rampData ramp;
	double holdVal = 0;
	// 
	UINT repeatNum = 0;
	// in ms
	double time = 0;
	// values such as repeat, repeat until trigger, no repeat, etc.
	int continuationType = 0;
};
