#pragma once

#include <string>
#include <vector>
#include "ScriptStream.h"
#include "Segment.h"

/**
* The class ScriptedAgilentWaveform contains all of the information and handling relevant for the entire intensity waveform that gets programmed to the Andor.
* This includes a vector of segments which contain segment-specific information. The functions and variabels relevant for this class are:
*/
class ScriptedAgilentWaveform
{
	public:
		ScriptedAgilentWaveform();
		bool readIntoSegment( int segNum, ScriptStream& script );
		void writeData( int SegNum );
		std::string compileAndReturnDataSendString( int segNum, int varNum, int totalSegNum );
		void compileSequenceString( int totalSegNum, int sequenceNum );
		std::string returnSequenceString();
		bool isVaried();
		void replaceVarValues();
		void replaceVarValues( key variableKey, UINT variation, std::vector<variableType>& variables);
		void convertPowersToVoltages();
		void normalizeVoltages();
		void calcMinMax();
		double getMaxVolt();
		double getMinVolt();
		ULONG getSegmentNumber();
		std::vector<std::pair<double, double>> minsAndMaxes;
	private:
		std::vector<Segment> waveformSegments;
		double maxVolt;
		double minVolt;
		int segmentNum;
		std::string totalSequence;
		bool varies;
};
