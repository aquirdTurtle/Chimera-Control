#pragma once

#include "waveformData.h"
#include "niFgen.h"
#include <string>
#include <vector>
#include <fstream>

class NiawgController
{
	public:
		NiawgController::NiawgController()
		{
			this->defaultOrientation = HORIZONTAL_ORIENTATION;
		}
		void setDefaultWaveforms(MainWindow* mainWin, bool isFirstLoad);
		
		int analyzeNIAWGScripts(std::fstream& verticalFileName, std::fstream& horizontalFileName, std::string &scriptHolder, int &waveCount, 
							std::vector<std::string> verticalPredWaveNames, std::vector<std::string> horizontalPredWaveNames, 
						   int &predWaveCount, std::vector<int> predLocs, std::vector<std::string>(&libWaveformArray)[20], bool(&fileStatus)[20], 
						   std::vector<waveData> &allVerticalWaveParameters, std::vector<bool> &verticalWaveformVaried, std::vector<waveData> &allHorizontalWaveParameters, 
						   std::vector<bool> &horizontalWaveformVaried, bool isDefault, std::string currentCategoryFolder, std::vector<variable> singletons, 
						   std::string orientation, debugOptions options, std::string& warnings, std::string& debugMessages);

		int getVariedWaveform(waveData &varWvFmInfo, std::vector<waveData> all_X_Or_Y_WvFmParam, int waveOrderNum, std::vector<std::string>(&libWvFmArray)[20],
							  bool(&fileStat)[20], ViReal64 *waveformRawData, debugOptions options, std::string& debugMsg);

		int varyParam(std::vector<waveData> &allWvInfo1, std::vector<waveData> &allWvInfo2, int wfCount, int &paramNum, double paramVal, std::string& warnings);
		
		// wrappers around niFgen functions.
		void initialize();
		void loadDefaults();
		signed short isDone();
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

		// might need to add more handlers for other objects.
		void setViStringAttribute(ViAttr atributeID, ViConstString attributeValue);
		void setViBooleanAttribute(ViAttr attribute, bool state);

		void setDefaultOrientation(std::string orientation);
		void restartDefault();

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

		static int getParamCheckVar(double &dataToAssign, std::fstream &scriptName, int &vCount, std::vector<std::string> & vNames, std::vector<int> &vParamTypes,
			int dataType, std::vector<variable> singletons);
		static int getParamCheckVar(int &dataToAssign, std::fstream &scriptName, int &vCount, std::vector<std::string> & vNames, std::vector<int> &vParamTypes,
			int dataType, std::vector<variable> singletons);
		static int getParamCheckVarConst(double &data1ToAssign, double &data2ToAssign, std::fstream &scriptName, int &vCount, std::vector<std::string> &vNames,
			std::vector<int> &vParamTypes, int dataType1, int dataType2, std::vector<variable> singletons);
		static int getParamCheckVarConst(int &data1ToAssign, double &data2ToAssign, std::fstream &scriptName, int &vCount, std::vector<std::string> &vNames,
			std::vector<int> &vParamTypes, int dataType1, int dataType2, std::vector<variable> singletons);
		ViReal64* mixWaveforms(ViReal64* waveform1, ViReal64* waveform2, ViReal64* finalWaveform, long int waveformSize);

	private:
		// NIAWG error checkers...
		void errChecker(int err);
		void getInputType(std::string inputType, waveData &wvInfo);

		int getWvFmData(std::fstream &scriptName, waveData &waveInfo, std::vector<variable> singletons);

		int waveformGen(ViReal64 * & tempWaveform, ViReal64 * & readData, waveData & waveInfo, long int size,
			std::vector<std::string>(&libWaveformArray)[20], bool &fileOpened, debugOptions options, std::string& debugMsg);

		int logic(std::fstream &xFile, std::fstream &yFile, std::string xInput, std::string yInput, std::string &scriptString);

		int special(std::fstream &verticalFile, std::fstream &horizontalFile, std::string verticalInputType, std::string horizontalInputType, std::string &scriptString,
			int &waveCount, std::vector<std::string> xWaveformList, std::vector<std::string> yWaveformList,
			int &predWaveCount, std::vector<int> waveListWaveCounts, std::vector<std::string>(&libWaveformArray)[20], bool(&fileStatus)[20], std::vector<waveData> &allXWaveParam,
			std::vector<bool> &xWaveVaried, std::vector<waveData> &allYWaveParam, std::vector<bool> &yWaveVaried, bool isDefault, std::string currentCategoryFolder, 
				std::vector<variable> singletons, std::string orientation, debugOptions options, std::string& warnings, std::string& debugMessages);

		ViReal64* calcWaveData(waveData & inputData, ViReal64 * & waveform, long int waveformSize);

		

		bool isLogicCommand(std::string command);
		bool isGenCommand(std::string c);
		bool isSpecialCommand(std::string c);

		bool readForVariables(std::fstream &file1, std::fstream &file2, std::vector<std::string> &file1Names, std::vector<std::string> &file2Names,
								std::vector<char> &var1Names, std::vector<char> &var2Names, std::vector<std::fstream> &var1Files, bool isDef);

		// overload for just the time input which is used to check the sample number of times that the master computer sends.
		

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

		// Session Parameters
		const ViInt32 OUTPUT_MODE = NIFGEN_VAL_OUTPUT_SCRIPT;
		// 
		const ViRsrc NI_5451_LOCATION = "PXI1Slot2";
		//

		// Channel parameters
		//const ViConstString SESSION_CHANNELS = "0,1";
		const ViConstString X_CHANNEL = "0";
		const ViConstString Y_CHANNEL = "1";

		// Minimum waveform size that the waveform generator can produce:
		const int MIN_WAVE_SAMPLE_SIZE = 100;
		const double MAX_CAP_TIME = 1e-3;

		// Trigger Parameters:
		const ViConstString TRIGGER_NAME = "ScriptTrigger0";
		const ViConstString TRIGGER_SOURCE = "PFI0";
		const ViInt32 TRIGGER_EDGE_TYPE = NIFGEN_VAL_RISING_EDGE;
};

template <typename WAVE_DATA_TYPE> long int NiawgController::waveformSizeCalc(WAVE_DATA_TYPE inputData)
{
	double waveSize = inputData.time * SAMPLE_RATE;
	return (long int)(waveSize + 0.5);
}

