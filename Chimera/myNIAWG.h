#pragma once

#include "waveformData.h"
#include "niFgen.h"
#include <string>
#include <vector>
#include <fstream>


namespace myNIAWG
{
	/*
	* @param xFileName:  the fstream object that holds the vertical script file.
	*/
	int analyzeNIAWGScripts(std::fstream& verticalFileName, std::fstream& horizontalFileName, std::string &scriptHolder, std::string triggerName, int &waveCount, ViSession &vi,
					   ViConstString channelName, ViStatus error, std::vector<std::string> verticalPredWaveNames, std::vector<std::string> horizontalPredWaveNames, 
					   int &predWaveCount, std::vector<int> predLocs, std::vector<std::string>(&libWaveformArray)[20], bool(&fileStatus)[20], 
					   std::vector<waveData> &allVerticalWaveParameters, std::vector<bool> &verticalWaveformVaried, std::vector<waveData> &allHorizontalWaveParameters, 
					   std::vector<bool> &horizontalWaveformVaried, bool isDefault, std::string currentCategoryFolder, std::vector<variable> singletons, 
					   std::string orientation, debugOptions options, std::string& warnings, std::string& debugMessages);

	int getVariedWaveform(waveData &varWvFmInfo, std::vector<waveData> all_X_Or_Y_WvFmParam, int waveOrderNum, std::vector<std::string>(&libWvFmArray)[20],
						  bool(&fileStat)[20], ViReal64 *waveformRawData, debugOptions options, std::string& debugMsg);

	int varyParam(std::vector<waveData> &allWvInfo1, std::vector<waveData> &allWvInfo2, int wfCount, int &paramNum, double paramVal, std::string& warnings);
	
	// NIAWG error checkers...
	bool NIAWG_CheckWindowsError(int err, std::string orientation);
	void NIAWG_CheckProgrammingError();
	void NIAWG_CheckProgrammingError(int err);
	void NIAWG_CheckScriptError(int err);
	int myNIAWG_DoubleErrorChecker(int err);
	int NIAWG_CheckDefaultError(int err);

	namespace handleInput
	{
		void getInputType(std::string inputType, waveData &wvInfo);

		int getWvFmData(std::fstream &scriptName, waveData &waveInfo, std::vector<variable> singletons);

		int waveformGen(ViReal64 * & tempWaveform, ViReal64 * & readData, waveData & waveInfo, long int size,
			std::vector<std::string>(&libWaveformArray)[20], bool &fileOpened, debugOptions options, std::string& debugMsg);

		int logic(std::fstream &xFile, std::fstream &yFile, std::string xInput, std::string yInput, std::string &scriptString, std::string triggerName);

		int special(std::fstream &xFile, std::fstream &yFile, std::string xInputType, std::string yInputType, std::string &scriptString, std::string triggerName,
			int &waveCount, ViSession vi, ViConstString channelName, ViStatus error, std::vector<std::string> xWaveformList, std::vector<std::string> yWaveformList,
			int &predWaveCount, std::vector<int> waveListWaveCounts, std::vector<std::string>(&libWaveformArray)[20], bool(&fileStatus)[20], std::vector<waveData> &allXWaveParam,
			std::vector<bool> &xWaveVaried, std::vector<waveData> &allYWaveParam, std::vector<bool> &yWaveVaried, bool isDefault, std::string currentCategoryFolder, 
				std::vector<variable> singletons, std::string orientation, debugOptions options, std::string& warnings, std::string& debugMessages);
	}

	/**
	* This namespace includes all of the relevant functions for generating waveforms.
	*/
	namespace script
	{
		ViReal64* calcWaveData(waveData & inputData, ViReal64 * & waveform, long int waveformSize);

		ViReal64* mixWaveforms(ViReal64* waveform1, ViReal64* waveform2, ViReal64* finalWaveform, long int waveformSize);

		bool isLogicCommand(std::string command);
		bool isGenCommand(std::string c);
		bool isSpecialCommand(std::string c);

		bool readForVariables(std::fstream &file1, std::fstream &file2, std::vector<std::string> &file1Names, std::vector<std::string> &file2Names,
							  std::vector<char> &var1Names, std::vector<char> &var2Names, std::vector<std::fstream> &var1Files, bool isDef);

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
		template <typename WAVE_DATA_TYPE>
		long int waveformSizeCalc(WAVE_DATA_TYPE inputData)
		{
			double waveSize = inputData.time * SAMPLE_RATE;
			return (long int)(waveSize + 0.5);
		}
		// overload for just the time input which is used to check the sample number of times that the master computer sends.
		long int waveformSizeCalc(double time);

		int getParamCheckVar(double &dataToAssign, std::fstream &scriptName, int &vCount, std::vector<std::string> & vNames, std::vector<int> &vParamTypes, 
							 int dataType, std::vector<variable> singletons);
		int getParamCheckVar(int &dataToAssign, std::fstream &scriptName, int &vCount, std::vector<std::string> & vNames, std::vector<int> &vParamTypes,
							 int dataType, std::vector<variable> singletons);
		int getParamCheckVarConst(double &data1ToAssign, double &data2ToAssign, std::fstream &scriptName, int &vCount, std::vector<std::string> &vNames,
								  std::vector<int> &vParamTypes, int dataType1, int dataType2, std::vector<variable> singletons);
		int getParamCheckVarConst(int &data1ToAssign, double &data2ToAssign, std::fstream &scriptName, int &vCount, std::vector<std::string> &vNames,
								  std::vector<int> &vParamTypes, int dataType1, int dataType2, std::vector<variable> singletons);
	}
}
