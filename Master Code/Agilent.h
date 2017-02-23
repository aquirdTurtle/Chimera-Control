#pragma once
#include <vector>
//#include "minMaxDoublet.h"
//#include "myMath.h"
#include "Windows.h"
#include "ConfigurationFileSystem.h"
#include "KeyHandler.h"
#include "ScriptStream.h"
#include "visa.h"

class Agilent;

struct minMaxDoublet
{
	double min;
	double max;
};


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
	int repeatNum;
	double initValue;
	double finValue;
	// in ms
	double time;
	// values such as repeat, repeat until trigger, no repeat, etc.
	int continuationType;
};

/*
	* The Segment class contains all of the information and handling for a single segment of the waveform to be programmed to the Agilent. The class includes
	* the following functions and variables:
	*/
class Segment
{
	public:
		Segment();
		void storeInput( segmentInfoInput input );
		segmentInfoInput getInput();
		segmentInfoFinal getFinalSettings();
		void convertInputToFinal( key variableKey, unsigned int variation );
		long returnDataSize();
		void assignDataVal( int dataNum, double val );
		double returnDataVal( long dataNum );
		void calcData();
		double rampCalc( int size, int iteration, double initPos, double finPos, std::string rampType );
	private:
		segmentInfoInput input;
		segmentInfoFinal finalSettings;
		std::vector<double> dataArray;
};

	/*
	 * The class IntensityWaveform contains all of the information and handling relevant for the entire intensity waveform that gets programmed to the Andor.
	 * This includes a vector of segments which contain segment-specific information. The functions and variabels relevant for this class are:
	 */
class IntensityWaveform
{
	public:
		IntensityWaveform();
		bool readIntoSegment( int segNum, ScriptStream& script, profileSettings profileInfo, Agilent* parent );
		void writeData( int SegNum );
		std::string compileAndReturnDataSendString( int segNum, int varNum, int totalSegNum );
		void compileSequenceString( int totalSegNum, int sequenceNum );
		std::string returnSequenceString();
		bool returnIsVaried();
		void replaceVarValues( key variableKey, unsigned int variation );
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
		void initialize(std::string address);
		void setDC( std::string level );
		void setExistingWaveform();
		void agilentDefault();
		void analyzeIntensityScript( ScriptStream& intensityFile, IntensityWaveform* intensityWaveformData, int& currentSegmentNumber, profileSettings profileInfo );
		void programIntensity( int varNum, key variableKey, bool& intensityVaried, std::vector<minMaxDoublet>& minsAndMaxes,
							   std::vector<std::vector<POINT>>& pointsToDraw, std::vector<ScriptStream>& intensityFiles, profileSettings profileInfo );
		void selectIntensityProfile( int varNum, bool intensityIsVaried, std::vector<minMaxDoublet> intensityMinMax );

	private:
		// usb address...
		std::string usbAddress;
		std::string deviceName;
		ViSession session;
		unsigned long instrument;
		unsigned long defaultResourceManager;
		double currentAgilentHigh;
		double currentAgilentLow;
		// since currently all visa communication is done to communicate with agilent machines, my visa wrappers exist in this class.
		void visaWrite( std::string message );
		void visaClose();
		void visaOpenDefaultRM();
		void visaOpen( std::string address );
		void errCheck( long status );
		void visaSetAttribute( ViAttr attributeName, ViAttrState value );
};
