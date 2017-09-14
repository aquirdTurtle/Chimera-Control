#pragma once

#include <string>
#include "windows.h"
#include "KeyHandler.h"

struct segmentInfoInput
{
	int segmentType;
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
};


class Segment
{
	public:
	Segment();
	void storeInput( segmentInfoInput input );
	segmentInfoInput getInput();
	segmentInfoFinal getFinalSettings();
	void convertInputToFinal();
	void convertInputToFinal( key variableKey, UINT variation, std::vector<variableType>& vars);
	UINT returnDataSize();
	void assignDataVal( int dataNum, double val );
	double returnDataVal( long dataNum );
	void calcData();
	double rampCalc( int size, int iteration, double initPos, double finPos, std::string rampType );

	private:
	segmentInfoInput input;
	segmentInfoFinal finalSettings;
	std::vector<double> dataArray;
};
