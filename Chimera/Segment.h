#pragma once

#include "segmentStructs.h"
#include "windows.h"
#include <string>
#include "VariableStructures.h"

class Segment
{
	public:
		void storeInput( segmentInfoInput input );
		segmentInfoInput getInput();
		segmentInfoFinal getFinalSettings();
		void convertInputToFinal();
		void convertInputToFinal( UINT variation, std::vector<parameterType>& vars);
		UINT returnDataSize();
		void assignDataVal( int dataNum, double val );
		double returnDataVal( long dataNum );
		void calcData( ULONG sampleRate );
		double rampCalc( int size, int iteration, double initPos, double finPos, std::string rampType );
		double pulseCalc( pulseData pulse, int iteration, long size, double pulseLength );
		double modCalc( modData mod, int iteration, long size, double pulseLength );
	private:
		segmentInfoInput input;
		segmentInfoFinal finalSettings;
		std::vector<double> dataArray;
};
