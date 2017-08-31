#pragma once

#include <string>
#include "windows.h"
#include "KeyHandler.h"

struct segmentInfoInput
{
	int segmentType;
	std::string rampType;
	std::string repeatNum;
	std::string initValue;
	std::string finValue;
	// in ms
	std::string time;
	// values such as repeat, repeat until trigger, no repeat, etc.
	int continuationType;
};



struct segmentInfoFinal
{
	int segmentType;
	std::string rampType;
	UINT repeatNum;
	double initValue;
	double finValue;
	// in ms
	double time;
	// values such as repeat, repeat until trigger, no repeat, etc.
	int continuationType;
};


class Segment
{
	public:
	Segment();
	void storeInput( segmentInfoInput input );
	segmentInfoInput getInput();
	segmentInfoFinal getFinalSettings();
	void convertInputToFinal();
	void convertInputToFinal( key variableKey, UINT variation, std::vector<variable>& vars);
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
