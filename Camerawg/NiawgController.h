#pragma once
#include <algorithm>
#include "NiawgController.h"
#include "externals.h"
#include "constants.h"
#include "myMath.h"
#include "myAgilent.h"
#include "miscellaneousCommonFunctions.h"
#include "ScriptStream.h"
#include "SocketWrapper.h"
#include "ConfigurationFileSystem.h"
#include "DebuggingOptionsControl.h"
#include "Communicator.h"
#include <memory>
#include <cmath>

// order here matches the literal channel number on the 5451. Vertical is actually channel0 and Horizontal is actually channel1.
enum AXES { Vertical = 0, Horizontal = 1 };
// used to pair together info for each channel of the niawg in an easy, iterable way.
template<typename type> using niawgPair = std::array<type, 2>;


/* * * * 
 * Niawg Data structure objects, in increasing order of complexity. I.e. waveSignals make up channelWaves which make up...
 * */

/* * * * *
 * A "Signal" structure contains all of the information for a single signal. Vectors of these are included in a "waveInfo" structure.
 * */
struct waveSignal
{
	double freqInit;
	double freqFin;
	std::string freqRampType;

	double initPower;
	double finPower;
	std::string powerRampType;

	double initPhase;
	// Asssigned only after a waveform is calculated or read.
	double finPhase;
};


/* * * * * * *
 * info for a single channel's output.
 * note that this structure does not contain any information about the time of a waveform because these always come with pairs in a waveInfo
 * struct, and I think it's better to just have a single copy of the time in the waveInfo struct rather than two (potentially conflicting)
 * copies in each channel.
 * */
struct channelWave
{
	std::vector<waveSignal> signals;
	// should be 0 until this option is re-implemented!
	int phaseOption;
	int initType;
	// variables for dealing with varied waveforms. These only get set when a varied waveform is used, and they serve the purpose of 
	// carrying relevant info to the end of the program, when these varried waveforms are compiled.
	int varNum;
	std::vector<std::string> varNames;
	std::vector<int> varTypes;
	// This should usually just be a single char, but if it's wrong I can use the whole string to debug.
	std::string delim;
	// the actual waveform data.
	std::vector<ViReal64> wave;
};


// declare it so that flashInfo knows what it is.
struct waveInfo;

struct flashInfo
{
	niawgPair<std::string> flashCycleFreqInput;
	niawgPair<std::string> totalTimeInput;
	std::vector<waveInfo> flashWaves;
	double flashCycleFreq;
	unsigned int flashNumber;
};


// contains all info for a waveform on the niawg; i.e. info for both channels, special options, time, and waveform data.
struct waveInfo
{
	niawgPair<channelWave> chan;
	// may or may not be filled... check the "is flashing variable". Don't have a better way of doing this atm...
	flashInfo flash;
	double time;
	long int sampleNum;
	bool varies;
	bool isStreamed;
	bool isFlashing;
	std::string name;
	std::vector<ViReal64> waveVals;
};


/* * * * * 
 * The largest output structure, contains all info for a script to be outputted. Because this contains a lot of info, it gets passed around
 * a lot between functions under a name "output". 
 * */
struct outputInfo
{
	// wave <-> waveform
	int waveCount;
	int predefinedWaveCount;
	std::vector<int> predefinedWaveLocs;
	bool isDefault;
	std::string niawgLanguageScript;
	// output channel
	std::vector<waveInfo> waves;
	niawgPair<std::vector<std::string>> predefinedWaveNames;
};


class NiawgController
{
	public:
		NiawgController::NiawgController()
		{
			defaultOrientation = HORIZONTAL_ORIENTATION;
		}
		void setDefaultWaveforms( MainWindow* mainWin );
		void analyzeNiawgScripts( niawgPair<ScriptStream>& scripts, outputInfo& output, profileSettings profile,
								  std::vector<variable> singletons, debugInfo& options, std::string& warnings );
		void handleVariations( outputInfo& output, std::vector<variable>& variables, std::vector<std::vector<double>> varValues,
							   size_t variation, std::vector<long> mixedWaveSizes, std::string& warnings, debugInfo& debugOptions );
		void getVariables( SocketWrapper& socket, std::vector<std::vector<double>>& varValues, std::vector<variable> variables );
		void loadWaveformParameters( outputInfo& output, profileSettings profile, niawgPair<std::string> command, debugInfo& debug, 
							  niawgPair<ScriptStream>& scripts, std::vector<variable> singletons );
		void varyParam( waveInfo& wave, waveInfo previousWave, int axis, int &paramNum, double paramVal, std::string& warnings );
		void finalizeScript( unsigned long long repetitions, std::string name, std::vector<std::string> workingUserScripts,
							 std::vector<ViChar>& userScriptSubmit );

		void setDefaultOrientation( std::string orientation );
		void restartDefault();
		bool isRunning();

		long int waveformSizeCalc( double time );
		template <typename WAVE_DATA_TYPE> long int waveformSizeCalc( WAVE_DATA_TYPE inputData );
		template <typename type> static void loadParam( type& dataToAssign, ScriptStream& scriptName, int& varCount,
														std::vector<std::string>& varNames, std::vector<int> &varParamTypes,
														std::vector<int> dataTypes, std::vector<variable> singletons );

		void mixWaveforms( waveInfo& waveInfo );
		void setRunningState( bool newRunningState );
		void checkThatWaveformsAreSensible( Communicator* comm, outputInfo& output );
				
		void abortGeneration();
		void initiateGeneration();
		void configureOutputEnabled( int state );
		void allocateNamedWaveform( ViConstString waveformName, ViInt32 unmixedSampleNumber );
		void writeScript(std::vector<ViChar> script);
		void setViStringAttribute( ViAttr atributeID, ViConstString attributeValue );
		void deleteWaveform( ViConstString waveformName );
		void writeNamedWaveform( ViConstString waveformName, ViInt32 mixedSampleNumber, ViReal64* wave );
		void deleteScript( ViConstString scriptName );
		void sendSoftwareTrigger();
		signed short isDone();
		void initialize();
		void streamWaveform();
		std::string getErrorMsg();
	private:
		void errChecker( int err );
		void calcWaveData( channelWave& inputData, std::vector<ViReal64>& readData, long int sampleNum, double time );
		void getStandardInputType( std::string inputType, channelWave &wvInfo );
		void openWaveformFiles( );
		void generateWaveform( channelWave & waveInfo, debugInfo& options, long int sampleNum, double time );
		void finalizeStandardWave( waveInfo& wave, debugInfo options );
		void createFlashingWave( waveInfo& wave, debugInfo options );
		void handleLogic( niawgPair<ScriptStream>& script, niawgPair<std::string> inputs, std::string &scriptString );
		void handleSpecial( niawgPair<ScriptStream>& script, outputInfo& output, niawgPair<std::string> inputTypes,
							profileSettings profile, std::vector<variable> singletons, debugInfo& options, std::string& warnings );
		void handleStandardWaveform( outputInfo& output, profileSettings profile, niawgPair<std::string> command,
									 niawgPair<ScriptStream>& scripts, std::vector<variable> singletons, debugInfo& options );
		void handleSpecialWaveform( outputInfo& output, profileSettings profile, niawgPair<std::string> command,
									niawgPair<ScriptStream>& scripts, std::vector<variable> singletons, debugInfo& options );
		bool isLogic( std::string command );
		bool isStandardWaveform( std::string command );
		bool isSpecialWaveform( std::string command );
		bool isSpecialCommand( std::string command );
		

		/// wrappers around niFgen functions.
		
		ViInt32 getInt32Attribute( ViAttr attribute );
		ViInt64 getInt64Attribute( ViAttr attribute );
		ViReal64 getReal64Attribute( ViAttr attribute );
		std::string getViStringAttribute( ViAttr attribute );
		ViBoolean getViBoolAttribute( ViAttr attribute );
		ViSession getViSessionAttribute( ViAttr attribute );
		void setAttributeViString( ViAttr attribute, ViString string );
		void createWaveform( long size, ViReal64* wave );
		void writeUnNamedWaveform( ViInt32 waveID, ViInt32 mixedSampleNumber, ViReal64* wave );
		

		ViInt32 allocateUnNamedWaveform( ViInt32 unmixedSampleNumber );
		void clearMemory();
		void configureSoftwareTrigger();
		void configureDigtalEdgeScriptTrigger();
		void configureOutputMode();
		void configureClockMode( ViInt32 clockMode );
		void configureMarker( ViConstString markerName, ViConstString outputLocation );
		void configureGain( ViReal64 gain );
		void configureChannels( ViConstString channelName );
		void configureSampleRate( ViReal64 sampleRate );
		void enableAnalogFilter( ViReal64 filterFrequency );
		void init( ViRsrc location, ViBoolean idQuery, ViBoolean resetDevice );
			
		void setViInt32Attribute( ViAttr attributeID, ViInt32 value );
		void setViBooleanAttribute( ViAttr attribute, bool state );

		/// member variables
		std::string defaultOrientation;
		niawgPair<std::string> currentScripts;
		bool runningState;
		// don't take the word "library" too seriously... it's just a listing of all of the waveforms that have been already created.
		std::array<std::vector<std::string>, MAX_NIAWG_SIGNALS * 4> waveLibrary;
		ViInt32 streamWaveHandle;
		std::string streamWaveformName;
		std::vector<double> streamWaveformVals;
		// pair is of horizontal and vertical configurations.
		niawgPair<std::vector<ViReal64>> defaultMixedWaveforms;
		niawgPair<std::string> defaultWaveNames;
		niawgPair<std::vector<ViChar>> defaultScripts;
		ViSession sessionHandle;

		ViConstString outputChannels = "0,1";
		// Session Parameters
		const ViInt32 OUTPUT_MODE = NIFGEN_VAL_OUTPUT_SCRIPT;
		const ViRsrc NI_5451_LOCATION = "PXI1Slot2";
		// Minimum waveform size that the waveform generator can produce:
		const int MIN_WAVE_SAMPLE_SIZE = 100;
		const double MAX_CAP_TIME = 1e-3;
		// Trigger Parameters:
		const ViConstString EXTERNAL_TRIGGER_NAME = "ScriptTrigger0";
		const ViConstString SOFTWARE_TRIGGER_NAME = "ScriptTrigger1";
		const ViConstString TRIGGER_SOURCE = "PFI0";
		const ViInt32 TRIGGER_EDGE_TYPE = NIFGEN_VAL_RISING_EDGE;
};

/* * * *
 * This function calculates the size in samples of the waveform to be generated.
 * This function works the same for all waveInfo types, so I use a template definition.
 * You can't include the template in the header file and in the source file, as when the function gets called from the header file,
 * the reference will be to a specific instantiation of the template function, which won't explicitly exist in the source file.
 * Instead, I put the entire definition here. There is probably a moer elegant way of doing this here.
 *
 * @param inputData this is the data which contains the time for which the waveform will be running, which along with the sample rate, 
 * determines the waveform size.
 * */
template <typename WAVE_DATA_TYPE> long NiawgController::waveformSizeCalc(WAVE_DATA_TYPE inputData)
{
	double waveSize = inputData.time * SAMPLE_RATE;
	// round. just (long) would floor.
	return long(waveSize + 0.5);
}



template <typename type> static void NiawgController::loadParam( type& dataToAssign, ScriptStream& file, int& varCount,
																 std::vector<std::string>& varNames, std::vector<int> &varParamTypes,
																 std::vector<int> dataTypes, std::vector<variable> singletons )
{
	std::string tempInput;
	file >> tempInput;
	if (tempInput.size() > 0)
	{
		if (tempInput[0] == '\'' || tempInput[0] == '#' || tempInput[0] == '}' || tempInput[0] == '{')
		{
			thrower( "ERROR: Detected reserved character out of place inside niawg script string \"" + tempInput + "\"" );
		}
	}
	type val;
	std::stringstream stream;
	stream << tempInput;
	stream >> val;
	// check if it was able to convert text to type correctly.
	if (stream.fail())
	{
		// check if this is a singleton variable. If so, immediately assign relevant data point to singleton value.
		for (int singletonInc = 0; singletonInc < singletons.size(); singletonInc++)
		{
			if (tempInput == singletons[singletonInc].name)
			{
				dataToAssign = singletons[singletonInc].value;
				return;
			}
		}
		// load variable name into structure.
		for (auto type : dataTypes)
		{
			varNames.push_back( tempInput );
			varParamTypes.push_back( type );
			varCount++;
		}
	}
	else
	{
		// this should happen most of the time.
		dataToAssign = val;
	}
}

