#pragma once
#include <vector>
//#include "minMaxDoublet.h"
//#include "myMath.h"
#include "Windows.h"

struct minMaxDoublet
{
	double min;
	double max;
};

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
		double rampCalc( int size, int iteration, double initPos, double finPos, std::string rampType );
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
		bool readIntoSegment( int segNum, ScriptStream intensityScript, std::vector<variable> singletons, profileSettings profileInfo );
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
		void analyzeIntensityScript( ScriptStream intensityFile, IntensityWaveform* intensityWaveformData, int& currentSegmentNumber, 
									 std::vector<variable> singletons, profileSettings profileInfo );
		
		/*
		 * The programIntensity function reads in the intensity script file, interprets it, creates the segments and sequences, and outputs them to the andor to be
		 * ready for usage. 
		 * 
		 */
		void programIntensity(int varNum, std::vector<variable> varNames, std::vector<std::vector<double> > varValues, bool& intensityVaried,
							 std::vector<minMaxDoublet>& minsAndMaxes, std::vector<std::vector<POINT>>& pointsToDraw, 
							 std::vector<std::fstream>& intensityFiles, std::vector<variable> singletons, profileSettings profileInfo);

		void selectIntensityProfile( int varNum, bool intensityIsVaried, std::vector<minMaxDoublet> intensityMinMax );

	private:
		// usb address...
		// name...?
		ViSession session;
		unsigned long instrument;
		unsigned long defaultResourceManager;
		double currentAgilentHigh;
		double currentAgilentLow;
		// since currently all visa communication is done to communicate with agilent machines, my visa wrappers exist in this class.
		void visaWrite( std::string message );
		void visaClose();

		// open the default resource manager resource. From ni.com:
		// " The viOpenDefaultRM() function must be called before any VISA operations can be invoked.The first call to this function 
		// initializes the VISA system, including the Default Resource Manager resource, and also returns a session to that resource. 
		// Subsequent calls to this function return unique sessions to the same Default Resource Manager resource.
		// When a Resource Manager session is passed to viClose(), not only is that session closed, but also all find lists and device 
		// sessions( which that Resource Manager session was used to create ) are closed.
		void visaOpenDefaultRM();
		void visaOpen( std::string address );
		void errCheck( long status );
		void visaSetAttribute( ViAttr attributeName, ViAttrState value );
}
