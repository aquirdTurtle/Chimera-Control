// created by Mark O. Brown
#pragma once

#include "segmentStructs.h"
#include <string>
#include "ParameterSystem/ParameterSystemStructures.h"

class Segment {
	public:
		void storeInput( segmentInfoInput input );
		segmentInfoInput getInput();
		// segmentInfoFinal getFinalSettings();
		void calculateSegVariations( unsigned totalNumVariations = 1, std::vector<parameterType>& variables = std::vector<parameterType> ( ) );
		unsigned returnDataSize();
		void assignDataVal( int dataNum, double val );
		double returnDataVal( long dataNum );
		void calcData( unsigned long sampleRate, unsigned varNum );
		double rampCalc( int size, int iteration, double initPos, double finPos, std::string rampType );
		double pulseCalc( pulseFormat pulse, int iteration, long size, double pulseLength, double center, unsigned varNum );
		double modCalc( modFormat mod, int iteration, long size, double pulseLength, unsigned varNum );
	private:
		segmentInfoInput input;
		//segmentInfoFinal finalSettings;
		std::vector<double> dataArray;
};
