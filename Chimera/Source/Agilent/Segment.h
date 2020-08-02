// created by Mark O. Brown
#pragma once

#include "segmentStructs.h"
#include <string>
#include "ParameterSystem/ParameterSystemStructures.h"

class Segment
{
	public:
		void storeInput( segmentInfoInput input );
		segmentInfoInput getInput();
		segmentInfoFinal getFinalSettings();
		void convertInputToFinal( unsigned variation = -1, std::vector<parameterType>& variables = std::vector<parameterType> ( ) );
		unsigned returnDataSize();
		void assignDataVal( int dataNum, double val );
		double returnDataVal( long dataNum );
		void calcData( ULONG sampleRate );
		double rampCalc( int size, int iteration, double initPos, double finPos, std::string rampType );
		double pulseCalc( pulseData pulse, int iteration, long size, double pulseLength, double center );
		double modCalc( modData mod, int iteration, long size, double pulseLength );
	private:
		segmentInfoInput input;
		segmentInfoFinal finalSettings;
		std::vector<double> dataArray;
};
