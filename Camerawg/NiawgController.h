#pragma once
//#include "stdafx.h"
#include "miscellaneousCommonFunctions.h"
#include "waveformData.h"
#include "niFgen.h"
#include <string>
#include <vector>
#include <fstream>
#include "ScriptStream.h"
#include <map>

// order here matches the literal channel number on the 5451.
enum AXES { Vertical = 0, Horizontal = 1 };

template<typename type> using niawgPair = std::array<type, 2>;
// don't take "library" too seriously... it's just a listing of all of the waveforms that have been already created.
typedef std::array<std::vector<std::string>, MAX_NIAWG_SIGNALS * 4> library;

// contains a slew of information about waveforms being outputted to a given chan (vertical, horizontal)
struct channelInfo
{
	// wave <-> waveform
	std::vector<std::string> predefinedWaveNames;
	std::vector<waveInfo> waveInfo;
};


struct outputInfo
{
	// wave <-> waveform
	int waveCount;
	int predefinedWaveCount;
	std::vector<int> predefinedWaveLocs;
	bool isDefault;
	std::string niawgLanguageScript;
	// output channel
	niawgPair<channelInfo> chan;
};


class NiawgController
{
	public:
		NiawgController::NiawgController()
		{
			defaultOrientation = HORIZONTAL_ORIENTATION;
		}
		void setDefaultWaveforms(MainWindow* mainWin, bool isFirstLoad);
		void analyzeNiawgScripts( niawgPair<ScriptStream>& scripts, outputInfo& output, library &waveLibrary, profileSettings profile,
								  std::vector<variable> singletons, debugInfo options, std::string& warnings, std::string& debugMessages );
		void handleVariations( outputInfo& output, std::vector<variable>& variables, std::vector<std::vector<double>> variableValues, 
							   size_t& variation, std::vector<long int> mixedWaveSizes, std::vector<ViReal64 *> mixedWaveforms, 
							   std::string& warnings, std::string& debugMsgs, library& waveLibrary, debugInfo debugOptions );
		
		void getVariedWaveform( waveInfo &varWvFmInfo, std::vector<waveInfo> all_X_Or_Y_WvFmParam, int waveOrderNum, library &waveLibrary, 
								ViReal64 *waveformRawData, debugInfo options, std::string& debugMsg );
		void varyParam(std::vector<waveInfo> &allWvInfo1, std::vector<waveInfo> &allWvInfo2, int wfCount, int &paramNum, 
						double paramVal, std::string& warnings);
		
		// wrappers around niFgen functions.
		void initialize();
		signed short isDone();
		ViInt32 getInt32Attribute( ViAttr attribute );
		ViInt64 getInt64Attribute( ViAttr attribute );
		ViReal64 getReal64Attribute( ViAttr attribute );
		std::string getViStringAttribute( ViAttr attribute );
		ViBoolean getViBoolAttribute( ViAttr attribute );
		ViSession getViSessionAttribute( ViAttr attribute );
		void setAttributeViString( ViAttr attribute, ViString string );
		void createWaveform(long size, ViReal64* mixedWaveform);
		void writeWaveform(ViConstString waveformName, ViInt32 mixedSampleNumber, ViReal64* mixedWaveform);
		void writeScript(ViConstString script);
		void deleteScript(ViConstString scriptName);
		void allocateWaveform(ViConstString waveformName, ViInt32 unmixedSampleNumber);
		void clearMemory();
		void configureOutputEnabled( int state );
		void configureSoftwareTrigger();
		void configureDigtalEdgeScriptTrigger();
		void configureOutputMode();
		void configureClockMode( ViInt32 clockMode );
		void configureMarker( ViConstString markerName, ViConstString outputLocation );
		void configureGain( ViReal64 gain );
		void configureChannels( ViConstString channelName );
		void configureSampleRate( ViReal64 sampleRate );
		void enableAnalogFilter(ViReal64 filterFrequency);
		void init(ViRsrc location, ViBoolean idQuery, ViBoolean resetDevice);
		void initiateGeneration();
		void abortGeneration();
		void deleteWaveform(ViConstString waveformName);
		std::string getErrorMsg();
		//

		// might need to add more handlers for other objects.
		void setViStringAttribute(ViAttr atributeID, ViConstString attributeValue);
		void setViBooleanAttribute(ViAttr attribute, bool state);

		void setDefaultOrientation(std::string orientation);
		void restartDefault();
		bool isRunning();
		
		long int waveformSizeCalc( double time );
		template <typename WAVE_DATA_TYPE> long int waveformSizeCalc(WAVE_DATA_TYPE inputData);
		template <typename type> static void loadParam( type& dataToAssign, ScriptStream& scriptName, int& varCount,
												 std::vector<std::string>& varNames, std::vector<int> &varParamTypes,
												 std::vector<int> dataTypes, std::vector<variable> singletons );

		ViReal64* mixWaveforms( ViReal64* verticalWaveform, ViReal64* HorizontalWaveform, ViReal64* finalWaveform, long int waveformSize );
		void setRunningState( bool newRunningState );
		void checkThatWaveformsAreSensible( Communicator* comm, outputInfo& output );

		void calculateFlashingWaveform();
		void streamWaveformData();

	private:

		void errChecker(int err);
		void getInputType(std::string inputType, waveInfo &wvInfo);
		void getWaveData( ScriptStream& scriptName, waveInfo &waveInfo, std::vector<variable> singletons);
		void openWaveformFiles( library &waveLibrary );
		void waveformGen( ViReal64 * & tempWaveform, waveInfo & waveInfo, 
						  library &waveLibrary, debugInfo options, std::string& debugMsg );
		void logic( niawgPair<ScriptStream>& script, niawgPair<std::string> inputs,	std::string &scriptString);
		void special( niawgPair<ScriptStream>& script, outputInfo& output, niawgPair<std::string> inputTypes,  library &waveLibrary,
					  profileSettings profile, std::vector<variable> singletons, debugInfo options, std::string& warnings, 
					  std::string& debugMessages );
		ViReal64* calcWaveData(waveInfo & inputData, ViReal64 * & waveform);
		bool isLogicCommand(std::string command);
		bool isGenCommand(std::string command );
		bool isSpecialCommand(std::string command );

		/// member variables
		std::string defaultOrientation;
		niawgPair<std::string> currentScripts;
		bool runningState;

		// pair is of horizontal and vertical configurations.
		niawgPair<ViReal64*> defaultMixedWaveforms;
		niawgPair<std::string> defaultWaveformNames;
		niawgPair<long> defaultMixedSizes;
		niawgPair<ViChar*> defaultScripts;
		ViStatus error;
		ViSession sessionHandle;
		ViConstString outputChannels;

		// Session Parameters
		const ViInt32 OUTPUT_MODE = NIFGEN_VAL_OUTPUT_SCRIPT;
		const ViRsrc NI_5451_LOCATION = "PXI1Slot2";
		// Minimum waveform size that the waveform generator can produce:
		const int MIN_WAVE_SAMPLE_SIZE = 100;
		const double MAX_CAP_TIME = 1e-3;
		// Trigger Parameters:
		const ViConstString EXTERNAL_TRIGGER_NAME = "ScriptTrigger0";
		const ViConstString SOFTWARE_TRIGGER_NAME = "SoftwareTrigger0";
		const ViConstString TRIGGER_SOURCE = "PFI0";
		const ViInt32 TRIGGER_EDGE_TYPE = NIFGEN_VAL_RISING_EDGE;

};

/**
* This function calculates the size in samples of the waveform to be generated.
* This function works the same for all waveInfo types, so I use a template definition.
* You can't include the template in the header file and in the source file, as when the function gets called from the header file,
* the reference will be to a specific instantiation of the template function, which won't explicitly exist in the source file.
* Instead, I put the entire definition here. There is probably a moer elegant way of doing this here.
*
* @param inputData this is the data which contains the time for which the waveform will be running, which along with the sample rate, determines the
* waveform size.
*/
template <typename WAVE_DATA_TYPE> long int NiawgController::waveformSizeCalc(WAVE_DATA_TYPE inputData)
{
	double waveSize = inputData.time * SAMPLE_RATE;
	// +0.5 ??
	return (long int)(waveSize + 0.5);
}


template <typename type> static void NiawgController::loadParam( type& dataToAssign, ScriptStream& file, int& varCount,
																 std::vector<std::string>& varNames, std::vector<int> &varParamTypes,
																 std::vector<int> dataTypes, std::vector<variable> singletons )
{
	std::string tempInput;
	int stringPos;
	file >> tempInput;
	if (tempInput.size() > 0)
	{
		if (tempInput[0] == '\'' || tempInput[0] == '#')
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
