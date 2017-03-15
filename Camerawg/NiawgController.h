#pragma once

#include "waveformData.h"
#include "niFgen.h"
#include <string>
#include <vector>
#include <fstream>
#include "ScriptStream.h"
#include <map>

// this structure has been designed to easily loop through horizontal and vertical data simultaneously. e.g.
// niawgPair waveform;
// for (auto axis : waveform.axis)
//{
//	try
//	{
//		genData(waveform.data[axis]);
//	}
//  catch()
//	{
//		 thrower(waveform.name[axis] + "threw an error!");
//	}
//}

enum AXES { Vertical = 0, Horizontal = 1 };

template<typename type> using niawgPair = std::array<type, 2>;

// some globals
std::array<int, 2> AXES = { Vertical, Horizontal };
// the following is used to receive the index of whatever axis is not your current axis.
std::array<int, 2> ALT_AXES = { Horizontal, Vertical };
std::array<std::string, 2> AXES_NAMES = { "Vertical", "Horizontal" };

/*
// for niawg types, element 0 always corresponds to the vertical one and element 1 always corresponds to the horizontal one.
//template <typename type> using doublet = std::array<type, 2>;

template<typename type> struct niawgData
{
	public:
		// info is the only element you're supposed to access.
		niawgData( int ind, std::string nameInput)
		{
			channelName = nameInput;
			channelIndex = ind;
		}
		int index()
		{
			return channelIndex;
		}
		std::string name()
		{
			return channelName;
		}
		type elem;
	private:
		std::string channelName;
		int channelIndex;
};


template<typename type> class niawgPair
{
	public:
		niawgPair::niawgPair() : AXES[0]( 0, "Vertical" ), AXES[1]( 1, "Horizontal" ){}
		// I would use pair but then I couldn't easily iterate through it.
		std::array<niawgData<type>, 2> AXES;
		bool match()
		{
			if (AXES[0].elem == AXES[1].elem)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
};
*/

// contains a slew of information about waveforms being outputted to a given channel (vertical, horizontal)
struct channelInfo
{
	// wave <-> waveform
	std::vector<std::string> predefinedWaveNames;
	std::vector<bool> waveIsVaried;
	std::vector<waveData> waveData;
};


struct outputInfo
{
	// wave <-> waveform
	int waveCount;
	int predefinedWaveCount;
	std::vector<int> predefinedWaveLocs;
	bool isDefault;
	std::string niawgLanguageScript;
	niawgPair<channelInfo> channel;
};


class NiawgController
{
	public:
		NiawgController::NiawgController()
		{
			defaultOrientation = HORIZONTAL_ORIENTATION;
		}
		void setDefaultWaveforms(MainWindow* mainWin, bool isFirstLoad);
		

		void analyzeNiawgScripts( niawgPair<ScriptStream>& scripts, outputInfo& output, std::vector<std::string>( &libWaveformArray )[MAX_NIAWG_SIGNALS * 4], profileSettings profile,
								  std::vector<variable> singletons, debugInfo options, std::string& warnings, std::string& debugMessages );

		void getVariedWaveform( waveData &varWvFmInfo, std::vector<waveData> all_X_Or_Y_WvFmParam, int waveOrderNum, 
								std::vector<std::string>( &libWvFmArray )[MAX_NIAWG_SIGNALS * 4], ViReal64 *waveformRawData, 
								debugInfo options, std::string& debugMsg );

		void varyParam(std::vector<waveData> &allWvInfo1, std::vector<waveData> &allWvInfo2, int wfCount, int &paramNum, 
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
		void createWaveform(long size, ViReal64* mixedWaveform);
		void writeWaveform(ViConstString waveformName, ViInt32 mixedSampleNumber, ViReal64* mixedWaveform);
		void writeScript(ViConstString script);
		void deleteScript(ViConstString scriptName);
		void allocateWaveform(ViConstString waveformName, ViInt32 unmixedSampleNumber);
		void configureOutputEnabled(int state);
		void clearMemory();
		void configureDigtalEdgeScriptTrigger();
		void enableAnalogFilter(ViReal64 filterFrequency);
		void configureClockMode(ViInt32 clockMode);
		void init(ViRsrc location, ViBoolean idQuery, ViBoolean resetDevice);
		void initiateGeneration();
		void abortGeneration();
		void configureMarker(ViConstString markerName, ViConstString outputLocation);
		void configureGain(ViReal64 gain);
		void configureChannels(ViConstString channelName);
		void configureSampleRate(ViReal64 sampleRate);
		void deleteWaveform(ViConstString waveformName);
		void configureOutputMode();
		std::string getErrorMsg();
		//

		// might need to add more handlers for other objects.
		void setViStringAttribute(ViAttr atributeID, ViConstString attributeValue);
		void setViBooleanAttribute(ViAttr attribute, bool state);

		void setDefaultOrientation(std::string orientation);
		void restartDefault();
		bool isRunning();

		/**
		* This function calculates the size in samples of the waveform to be generated.
		* This function works the same for all waveData types, so I use a template definition.
		* You can't include the template in the header file and in the source file, as when the function gets called from the header file, the reference will be to
		* a specific instantiation of the template function, which won't explicitly exist in the source file. Instead, I put the entire definition here. There is
		* probably a moer elegant way of doing this here.
		*
		* @param inputData this is the data which contains the time for which the waveform will be running, which along with the sample rate, determines the
		* waveform size.
		*/
		long int waveformSizeCalc(double time);
		template <typename WAVE_DATA_TYPE> long int waveformSizeCalc(WAVE_DATA_TYPE inputData);
		static void getParamCheckVar(double &dataToAssign, ScriptStream &scriptName, int &vCount, std::vector<std::string> & vNames, 
									  std::vector<int> &vParamTypes, int dataType, std::vector<variable> singletons);
		static void getParamCheckVar(int &dataToAssign, ScriptStream& scriptName, int &vCount, std::vector<std::string> & vNames,
									  std::vector<int> &vParamTypes, int dataType, std::vector<variable> singletons);
		static void getParamCheckVarConst(double &data1ToAssign, double &data2ToAssign, ScriptStream& scriptName, int &vCount,
										   std::vector<std::string> &vNames, std::vector<int> &vParamTypes, int dataType1, int dataType2, 
										   std::vector<variable> singletons);
		static void getParamCheckVarConst(int &data1ToAssign, double &data2ToAssign, ScriptStream& scriptName, int &vCount, 
										   std::vector<std::string> &vNames, std::vector<int> &vParamTypes, int dataType1, int dataType2, 
										   std::vector<variable> singletons);
		ViReal64* mixWaveforms( niawgPair<ViReal64*> waveforms, ViReal64* finalWaveform, long int waveformSize );
		void setRunningState( bool newRunningState );

	private:

		void errChecker(int err);
		void getInputType(std::string inputType, waveData &wvInfo);
		void getWaveData( ScriptStream& scriptName, waveData &waveInfo, std::vector<variable> singletons);
		void openWaveformFiles( std::vector<std::string>( &libWaveformArray )[MAX_NIAWG_SIGNALS * 4] );
		void waveformGen( ViReal64 * & tempWaveform, ViReal64 * & readData, waveData & waveInfo, 
						  std::vector<std::string>( &libWaveformArray )[MAX_NIAWG_SIGNALS * 4], debugInfo options, std::string& debugMsg ); 
		void logic( niawgPair<ScriptStream>& script, niawgPair<std::string> inputs,	std::string &scriptString);
		void special( niawgPair<ScriptStream>& script, outputInfo& output, niawgPair<std::string> inputTypes, 
					  std::vector<std::string>( &libWaveformArray )[MAX_NIAWG_SIGNALS * 4],
					  profileSettings profile, std::vector<variable> singletons, debugInfo options, std::string& warnings, 
					  std::string& debugMessages );
		ViReal64* calcWaveData(waveData & inputData, ViReal64 * & waveform);
		bool isLogicCommand(std::string command);
		bool isGenCommand(std::string command );
		bool isSpecialCommand(std::string command );

		/// member variables
		std::string defaultOrientation;		
		ViReal64* default_hConfigMixedWaveform;
		std::string default_hConfigWaveformName;
		long default_hConfigMixedSize;
		ViChar* default_hConfigScript;
		// 
		ViReal64* default_vConfigMixedWaveform;
		std::string default_vConfigWaveformName;
		long default_vConfigMixedSize;
		ViChar* default_vConfigScript;

		ViStatus error;
		ViSession sessionHandle;
		ViConstString outputChannels;

		niawgPair<std::string> currentScripts;

		// Session Parameters
		const ViInt32 OUTPUT_MODE = NIFGEN_VAL_OUTPUT_SCRIPT;
		// 
		const ViRsrc NI_5451_LOCATION = "PXI1Slot2";
		//

		// Minimum waveform size that the waveform generator can produce:
		const int MIN_WAVE_SAMPLE_SIZE = 100;
		const double MAX_CAP_TIME = 1e-3;

		// Trigger Parameters:
		const ViConstString TRIGGER_NAME = "ScriptTrigger0";
		const ViConstString TRIGGER_SOURCE = "PFI0";
		const ViInt32 TRIGGER_EDGE_TYPE = NIFGEN_VAL_RISING_EDGE;
		bool runningState;
};

template <typename WAVE_DATA_TYPE> long int NiawgController::waveformSizeCalc(WAVE_DATA_TYPE inputData)
{
	double waveSize = inputData.time * SAMPLE_RATE;
	return (long int)(waveSize + 0.5);
}

