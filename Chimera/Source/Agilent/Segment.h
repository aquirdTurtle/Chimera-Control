// created by Mark O. Brown
#pragma once

#include "segmentStructs.h"
#include <string>
#include "ParameterSystem/ParameterSystemStructures.h"

class Segment {
	public:
		void storeInput( segmentInfo input );
		segmentInfo getInput();
		void calculateSegVariations( unsigned totalNumVariations = 1, std::vector<parameterType>& variables = std::vector<parameterType> ( ) );
		unsigned returnDataSize();
		void assignDataVal( int dataNum, double val );
		double returnDataVal( long dataNum );
		void calcData( unsigned long sampleRate, unsigned varNum );
		static void analyzeRampFile (rampInfo& ramp, long totalSamples);
		double rampCalc(int totalIterations, int iteration, rampInfo ramp, unsigned varnum);
		double pulseCalc( segPulseInfo pulse, int iteration, long size, double pulseLength, double center, unsigned varNum );
		double modCalc( segModInfo mod, int iteration, long size, double pulseLength, unsigned varNum );
	private:
		segmentInfo input;
		std::vector<double> dataArray;
};
