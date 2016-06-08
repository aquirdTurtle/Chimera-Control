#pragma once
#include <vector>
//#include "minMaxDoublet.h"
#include "myMath.h"
#include "Windows.h"

/*
 * This Namespace includes all of my function handling for interacting withe agilent waveform generator. It includes:
 * The Segment Class
 * The IntensityWaveform Class
 * The agilentDefault function
 * The agilentErrorCheck function
 * The selectIntensityProfile function
 */
namespace myAgilent
{
	/*
	 * The Segment class contains all of the information and handling for a single segment of the waveform to be programmed to the Agilent. The class includes
	 * the following functions and variables:
	 *
	 *	public:
	 *		Segment();
	 *		~Segment();
	 *		int assignSegType(int segTypeIn);
	 *		int assignRampType(std::string rampTypeIn);
	 *		int assignInitValue(double initValIn);
	 *		int assignFinValue(double finValIn);
	 *		int assignContinuationType(int contTypeIn);
	 *		int assignTime(double timeIn);
	 *		int	assignRepeatNum(int repeatNumIn);
	 *		int returnSegmentType();
	 *		long returnDataSize();
	 *		int returnRepeatNum();
	 *		int returnContinuationType();
	 *		int assignVarNum(int vNumIn);
	 *		int assignSegVarNames(std::vector<std::string> vNamesIn);
	 *		int assignVarLocations(std::vector<int> varLocationsIn);
	 *		int returnSegVarNamesSize();
	 *		std::string returnVarName(int varNameIdentifier);
	 *		int returnVarLocation(int varNameIdentifier);
	 *		int assignDataVal(int dataNum, double val);
	 *		double returnDataVal(long dataNum);
	 *		int calcData();
	 *		double returnTime();
	 *	private:
	 *		int segmentType;
	 *		std::string rampType;
	 *		int repeatNum;
	 *		double initValue;
	 *		double finValue;
	 *		double time;
	 *		// values such as repeat, repeat until trigger, no repeat, etc.
	 *		int continuationType;
	 *		std::vector<double> dataArray;
	 *		int varNum;
	 *		std::vector<std::string> segVarNames;
	 *		std::vector<int> varLocations;
	 */
	class Segment
	{
		public:
			Segment();
			~Segment();
			int assignSegType(int segTypeIn);
			int assignRampType(std::string rampTypeIn);
			int assignInitValue(double initValIn);
			int assignFinValue(double finValIn);
			int assignContinuationType(int contTypeIn);
			int assignTime(double timeIn);
			int	assignRepeatNum(int repeatNumIn);
			int returnSegmentType();
			long returnDataSize();
			int returnRepeatNum();
			int returnContinuationType();
			int assignVarNum(int vNumIn);
			int assignSegVarNames(std::vector<std::string> vNamesIn);
			int assignVarLocations(std::vector<int> varLocationsIn);
			int returnSegVarNamesSize();
			std::string returnVarName(int varNameIdentifier);
			int returnVarLocation(int varNameIdentifier);
			int assignDataVal(int dataNum, double val);
			double returnDataVal(long dataNum);
			int calcData();
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
	 *	 public:
	 *		 IntensityWaveform();
	 *		 ~IntensityWaveform();
	 *		 int readIntoSegment(int segNum, std::fstream& fileName);
	 *		 int writeData(int SegNum);
	 *		 std::string compileAndReturnDataSendString(int segNum, int varNum, int totalSegNum);
	 *		 int compileSequenceString(int totalSegNum, int sequenceNum);
	 *		 std::string returnSequenceString();
	 *		 bool returnIsVaried();
	 *		 int returnGivenSegVarNamesSize(int segNum);
	 *		 int replaceVarValues(std::string varName, double varValue);
	 *		 int convertPowersToVoltages();
	 *		 int normalizeVoltages();
	 *		 int calcMinMax();
	 *		 double returnMaxVolt();
	 *		 double returnMinVolt();
	 *		 std::vector<POINT> returnIntensityPointsForDraw();
	 *	 private:
	 *		 std::vector<myAgilent::Segment> waveformSegments;
	 *		 double maxVolt;
	 *		 double minVolt;
	 *		 int segmentNum;
	 *		 std::string totalSequence;
	 *		 bool isVaried;
	 */
	class IntensityWaveform
	{
		public:
			IntensityWaveform();
			~IntensityWaveform();
			int readIntoSegment(int segNum, std::fstream& fileName, std::vector<variable> singletons);
			int writeData(int SegNum);
			std::string compileAndReturnDataSendString(int segNum, int varNum, int totalSegNum);
			int compileSequenceString(int totalSegNum, int sequenceNum);
			std::string returnSequenceString();
			bool returnIsVaried();
			int replaceVarValues(std::string varName, double varValue);
			int convertPowersToVoltages();
			int normalizeVoltages();
			int calcMinMax();
			double returnMaxVolt();
			double returnMinVolt();
		private:
			std::vector<myAgilent::Segment> waveformSegments;
			double maxVolt;
			double minVolt;
			int segmentNum;
			std::string totalSequence;
			bool isVaried;
	};

	/*
	 * The agilentDefalut function restores the status of the Agilent to be outputting the default DC level (full depth traps). It returns an error code.
	 */
	int agilentDefault();
	
	/*
	]---- This function is used to analyze a given intensity file. It's used to analyze all of the basic intensity files listed in the sequence of 
	]- configurations, but also recursively to analyze nested intensity scripts.
	*/
	bool analyzeIntensityScript(std::fstream& intensityFile, myAgilent::IntensityWaveform* intensityWaveformData, int& currentSegmentNumber, 
								std::vector<variable> singletons);
	/*
	 * The programIntensity function reads in the intensity script file, interprets it, creates the segments and sequences, and outputs them to the andor to be
	 * ready for usage. 
	 * 
	 */
	int programIntensity(int varNum, std::vector<variable> varNames, std::vector<std::vector<double> > varValues, bool& intensityVaried,
						 std::vector<myMath::minMaxDoublet>& minsAndMaxes, std::vector<std::vector<POINT>>& pointsToDraw, 
						 std::vector<std::fstream>& intensityFiles, std::vector<variable> singletons);

	int agilentErrorCheck(long status, unsigned long vi);
	
	int selectIntensityProfile(int varNum, bool intensityIsVaried, std::vector<myMath::minMaxDoublet> intensityMinMax);

}
