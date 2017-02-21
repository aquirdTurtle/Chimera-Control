#pragma once
#include <vector>
//#include "minMaxDoublet.h"
//#include "myMath.h"
#include "Windows.h"

/*
	* The Segment class contains all of the information and handling for a single segment of the waveform to be programmed to the Agilent. The class includes
	* the following functions and variables:
	*/
class Segment
{
	public:
		Segment();
		~Segment();
		void assignSegType(int segTypeIn);
		void assignRampType(std::string rampTypeIn);
		void assignInitValue(double initValIn);
		void assignFinValue(double finValIn);
		void assignContinuationType(int contTypeIn);
		void assignTime(double timeIn);
		void assignRepeatNum(int repeatNumIn);
		int returnSegmentType();
		long returnDataSize();
		int returnRepeatNum();
		int returnContinuationType();
		void assignVarNum(int vNumIn);
		void assignSegVarNames(std::vector<std::string> vNamesIn);
		void assignVarLocations(std::vector<int> varLocationsIn);
		int returnSegVarNamesSize();
		std::string returnVarName(int varNameIdentifier);
		int returnVarLocation(int varNameIdentifier);
		void assignDataVal(int dataNum, double val);
		double returnDataVal(long dataNum);
		void calcData();
		double returnTime();
	private:
		int segmentType;
		std::string rampType;
		int repeatNum;
		double initValue;
		double finValue;
		double time;
		// values such as repeat, repeat until trigger, no repeat, etc.
		int continuationType;
		std::vector<double> dataArray;
		int varNum;
		std::vector<std::string> segVarNames;
		std::vector<int> varLocations;
};

	/*
	 * The class IntensityWaveform contains all of the information and handling relevant for the entire intensity waveform that gets programmed to the Andor.
	 * This includes a vector of segments which contain segment-specific information. The functions and variabels relevant for this class are:
	 */
class IntensityWaveform
{
	public:
		IntensityWaveform();
		void readIntoSegment(int segNum, std::fstream& scriptName, std::vector<variable> singletons, profileSettings profileInfo);
		void writeData(int SegNum);
		std::string compileAndReturnDataSendString(int segNum, int varNum, int totalSegNum);
		void compileSequenceString(int totalSegNum, int sequenceNum);
		std::string returnSequenceString();
		bool returnIsVaried();
		void replaceVarValues(std::string varName, double varValue);
		void convertPowersToVoltages();
		void normalizeVoltages();
		void calcMinMax();
		double returnMaxVolt();
		double returnMinVolt();
	private:
		std::vector<Segment> waveformSegments;
		double maxVolt;
		double minVolt;
		int segmentNum;
		std::string totalSequence;
		bool isVaried;
};

class Agilent
{
	public:
		/*
		 * The agilentDefalut function restores the status of the Agilent to be outputting the default DC level (full depth traps). It returns an error code.
		 */
		void agilentDefault();
	
		/*
		]---- This function is used to analyze a given intensity file. It's used to analyze all of the basic intensity files listed in the sequence of 
		]- configurations, but also recursively to analyze nested intensity scripts.
		*/
		void analyzeIntensityScript(std::fstream& intensityFile, IntensityWaveform* intensityWaveformData, int& currentSegmentNumber, 
									std::vector<variable> singletons, profileSettings profileInfo);
		/*
		 * The programIntensity function reads in the intensity script file, interprets it, creates the segments and sequences, and outputs them to the andor to be
		 * ready for usage. 
		 * 
		 */
		void programIntensity(int varNum, std::vector<variable> varNames, std::vector<std::vector<double> > varValues, bool& intensityVaried,
							 std::vector<myMath::minMaxDoublet>& minsAndMaxes, std::vector<std::vector<POINT>>& pointsToDraw, 
							 std::vector<std::fstream>& intensityFiles, std::vector<variable> singletons, profileSettings profileInfo);

		void agilentErrorCheck(long status, unsigned long vi);
	
		void selectIntensityProfile(int varNum, bool intensityIsVaried, std::vector<myMath::minMaxDoublet> intensityMinMax);
	private:
		// usb address...
		// name...?
		ViSession session;
		ViSession instr;
		// since currently all visa communication is done to communicate with agilent machines, my visa wrappers exist in this class.
		void visaWrite( std::string message, ViPUInt32 retCnt );
		void visaClose(ViObject obj);
		void visaOpenDefaultRM();
		void visaOpen( ViSession sesn, ViRsrc name, ViAccessMode mode, ViUInt32 timeout);
		void visaQueryf( ViString writeFmt, ViString readFmt, ... );
}
