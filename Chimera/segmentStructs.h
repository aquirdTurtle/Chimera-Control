#pragma once
#include "Expression.h"
#include <string>
#include "windows.h"

// short for modulation
struct modFormat
{
	bool modulationIsOn;
	// in MHz
	Expression frequency;
	// in radians
	Expression phase;
};


struct modData
{
	bool modulationIsOn;
	// in MHz
	double frequency;
	// in radians
	double phase;
};;


struct pulseFormat
{
	// as of October 6th, can be "sech", "gaussian", or "lorentzian"
	std::string type;
	Expression amplitude;
	Expression width;
	// if pulseLength !>> pulseWidth, there will be a cutoff in the shape of the pulse. The peak of the pulse is always
	// centered.
	Expression length;
	Expression offset;
};

struct pulseData
{
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
	std::string rampType;
	Expression repeatNum;
	Expression initValue;
	Expression finValue;
	// in ms
	Expression time;
	// values such as repeat, repeat until trigger, no repeat, etc.
	int continuationType;
};



struct segmentInfoFinal
{
	int segmentType = 0;
	std::string rampType = "";
	UINT repeatNum = 0;
	double initValue = 0;
	double finValue = 0;
	// in ms
	double time = 0;
	// values such as repeat, repeat until trigger, no repeat, etc.
	int continuationType = 0;
	pulseData pulse;
	modData mod;
};
