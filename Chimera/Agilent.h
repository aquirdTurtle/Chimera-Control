#pragma once
#include <vector>
#include <array>
#include "Windows.h"
#include "ConfigurationFileSystem.h"
#include "KeyHandler.h"
#include "ScriptStream.h"
#include "visa.h"
#include "commonTypes.h"

class Agilent;

struct minMaxDoublet
{
	double min;
	double max;
};


struct generalAgilentOutputInfo
{
	std::string load;
	double sampleRate;
	bool synced;
};


struct dcInfo : public generalAgilentOutputInfo
{
	std::string dcLevelInput;
	double dcLevel;
};


struct scriptedArbInfo : public generalAgilentOutputInfo
{
		// ??
};


struct squareInfo : public generalAgilentOutputInfo
{
	std::string frequencyInput;
	double frequency;
	std::string amplitudeInput;
	double amplitude;
	std::string offsetInput;
	double offset;
};


struct sineInfo : public generalAgilentOutputInfo
{
	std::string frequencyInput;
	double frequency;
	std::string amplitudeInput;
	double amplitude;
};


struct preloadedArbInfo : public generalAgilentOutputInfo
{
	std::string address;
	// could add burst settings options, impedance options, etc.
};


struct channelInfo
{
	int option;
	dcInfo dc;
	sineInfo sine;
	squareInfo square;
	preloadedArbInfo preloadedArb;
	scriptedArbInfo scriptedArb;
};


struct deviceOutputInfo
{
	// first ([0]) is channel 1, second ([1]) is channel 2.
	std::array<channelInfo, 2> channel;
	bool synced;
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
	* the following functions and variables
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
	* The class ScriptedAgilentWaveform contains all of the information and handling relevant for the entire intensity waveform that gets programmed to the Andor.
	* This includes a vector of segments which contain segment-specific information. The functions and variabels relevant for this class are:
	*/
class ScriptedAgilentWaveform
{
	public:
		ScriptedAgilentWaveform();
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
		void initialize( POINT& loc, std::vector<CToolTipCtrl*>& toolTips, DeviceWindow* master, int& id, 
						 std::string address, std::string header );		
		void handleChannelPress( int channel );
		void handleCombo();
		void setDC( int channel, dcInfo info );
		void setExistingWaveform( int channel, preloadedArbInfo info );
		void setSquare( int channel, squareInfo info );
		void setSingleFreq( int channel, sineInfo info );
		void outputOff(int channel);
		void handleInput();
		void handleInput(int chan);
		void agilentDefault( int channel );
		void zeroSettings();
		bool connected();
		void analyzeAgilentScript( ScriptStream& intensityFile, ScriptedAgilentWaveform* intensityWaveformData, 
								   int& currentSegmentNumber, profileSettings profileInfo );
		HBRUSH handleColorMessage(CWnd* window, brushMap brushes, rgbMap rGBs, CDC* cDC);
		std::string getConfigurationString();
		std::string getDeviceIdentity();
		void readConfigurationFile( std::ifstream& file );
		void programScript( int varNum, key variableKey, std::vector<ScriptStream>& intensityFiles, profileSettings profileInfo );
		void selectIntensityProfile( int varNum );
		void convertInputToFinalSettings( key variableKey, unsigned int variation );
		void selectScriptProfile( int channel, int varNum );
		void updateEdit(int chan);
		deviceOutputInfo getOutputInfo();
		void rearrange(UINT width, UINT height, fontMap fonts);
	private:
		// usb address...
		std::string usbAddress;
		std::string deviceName;
		minMaxDoublet chan1Range;
		minMaxDoublet chan2Range;
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
		void visaPrintf( std::string msg );
		void visaErrQuery(std::string& errMsg, long& errCode);
		std::string visaIdentityQuery();
		bool isVaried;
		bool isConnected;
		int currentChannel;
		std::string deviceInfo;
		std::vector<minMaxDoublet> ranges;		
		deviceOutputInfo settings;

		// GUI ELEMENTS
		Control<CStatic> header;
		Control<CStatic> deviceInfoDisplay;
		Control<CButton> channel1Button;
		Control<CButton> channel2Button;
		Control<CButton> syncedButton;
		Control<CComboBox> settingCombo;
		Control<CStatic> optionsFormat;
		Control<CEdit> optionsEdit;
};
