#include "stdafx.h"
#include "myNIAWG.h"
#include "externals.h"
#include "constants.h"
#include "appendText.h"
#include "rmWhite.h"
#include <algorithm>
#include "myMath.h"
#include "myAgilent.h"
#include "postMyString.h"
/****
* Contains all major functions for interacting with the NIAWG. Specifically:
* 	int createXYScript(std::fstream& verticalFileName, std::fstream& horizontalFileName, std::string &scriptHolder, std::string triggerName, int &waveCount, ViSession &vi,
ViConstString channelName, ViStatus error, std::vector<std::string> verticalPredWaveNames, std::vector<std::string> horizontalPredWaveNames,
int &predWaveCount, std::vector<int> predLocs, std::vector<std::string>(&libWaveformArray)[20], bool(&fileStatus)[20],
std::vector<waveData> &allVerticalWaveParameters, std::vector<bool> &verticalWaveformVaried, std::vector<waveData> &allHorizontalWaveParameters,
std::vector<bool> &horizontalWaveformVaried, bool isDefault = false);

int getVariedWaveform(waveData &varWvFmInfo, std::vector<waveData> all_X_Or_Y_WvFmParam, int waveOrderNum, std::vector<std::string>(&libWvFmArray)[20],
bool(&fileStat)[20], ViReal64 *waveformRawData);
int varyParam(std::vector<waveData> &allWvInfo1, std::vector<waveData> &allWvInfo2, int wfCount, int &paramNum, double paramVal);
bool NIAWG_CheckWindowsError(int err);
bool NIAWG_CheckProgrammingError(int err);
bool NIAWG_CheckScriptError(int err, bool isDefaultError);
int myNIAWG_DoubleErrorChecker(int err);
int NIAWG_CheckDefaultError(int err);
namespace handleInput
{
void getInputType(std::string inputType, waveData &wvInfo);
int getWvFmData(std::fstream &fileName, waveData &waveInfo);
int waveformGen(ViReal64 * & tempWaveform, ViReal64 * & readData, waveData & waveInfo, long int size,
std::vector<std::string>(&libWaveformArray)[20], bool &fileOpened);
int logic(std::fstream &verticalFile, std::fstream &horizontalFile, std::string verticalInput, std::string horizontalInput, std::string &scriptString, std::string triggerName);
int special(std::fstream &verticalFile, std::fstream &horizontalFile, std::string verticalInputType, std::string horizontalInputType, std::string &scriptString, std::string triggerName,
int &waveCount, ViSession vi, ViConstString channelName, ViStatus error, std::vector<std::string> xWaveformList, std::vector<std::string> yWaveformList,
int &predWaveCount, std::vector<int> waveListWaveCounts, std::vector<std::string>(&libWaveformArray)[20], bool(&fileStatus)[20], std::vector<waveData> &allXWaveParam,
std::vector<bool> &xWaveVaried, std::vector<waveData> &allYWaveParam, std::vector<bool> &yWaveVaried);
}
namespace script
{
ViReal64* calcWaveData(waveData & inputData, ViReal64 * & waveform, long int waveformSize);
ViReal64* mixWaveforms(ViReal64* waveform1, ViReal64* waveform2, ViReal64* finalWaveform, long int waveformSize);
bool isLogicCommand(std::string command);
bool isGenCommand(std::string c);
bool isSpecialCommand(std::string c);
bool readForVariables(std::fstream &file1, std::fstream &file2, std::vector<std::string> &file1Names, std::vector<std::string> &file2Names,
std::vector<char> &var1Names, std::vector<char> &var2Names, std::vector<std::fstream> &var1Files, bool isDef);
template <typename WAVE_DATA_TYPE>
long int waveformSizeCalc(WAVE_DATA_TYPE inputData)
{
double waveSize = inputData.time * SAMPLE_RATE;
return (long int)(waveSize + 0.5);
}

int getParamCheckVar(double &dataToAssign, std::fstream &fileName, int &vCount, std::vector<std::string> & vNames, std::vector<int> &vParamTypes,
int dataType);
int getParamCheckVar(int &dataToAssign, std::fstream &fileName, int &vCount, std::vector<std::string> & vNames, std::vector<int> &vParamTypes,
int dataType);
int getParamCheckVarConst(double &data1ToAssign, double &data2ToAssign, std::fstream &fileName, int &vCount, std::vector<std::string> &vNames,
std::vector<int> &vParamTypes, int dataType1, int dataType2);
int getParamCheckVarConst(int &data1ToAssign, double &data2ToAssign, std::fstream &fileName, int &vCount, std::vector<std::string> &vNames,
std::vector<int> &vParamTypes, int dataType1, int dataType2);
}
*/
namespace myNIAWG
{
	/**
	 * createXYScript() is essentially a massive subroutine. In retrospect, making this a subroutine was probably mostly unnecessary, and makes the code heierarchy
	 * needlessly more complicated. The purpose of this function is to systematically read the input instructions files, create and read waveforms associated with
	 * them, write the script as it goes, and eventually combine the x and y-waveforms into their final form for being sent to the waveform generator.
	 */
	int createXYScript(std::fstream& verticalFileName, std::fstream& horizontalFileName, std::string& scriptHolder, std::string triggerName, int& waveCount, ViSession& vi,
					   ViConstString channelName, ViStatus error, std::vector<std::string> verticalPredWaveNames, std::vector<std::string> horizontalPredWaveNames,
					   int& predWaveCount, std::vector<int> predLocs, std::vector<std::string>(&libWaveformArray)[20], bool(&fileStatus)[20],
					   std::vector<waveData>& allVerticalWaveParameters, std::vector<bool>& verticalWaveformVaried, std::vector<waveData>& allHorizontalWaveParameters,
					   std::vector<bool>& horizontalWaveformVaried, bool isDefault, bool isThreaded, std::string currentCategoryFolder)
	{
		// Some declarations.
		std::string verticalInputTypeString, horizontalInputTypeString;
		int myError;
		// Variables which, after the first waveform generated, hold the basic information about the previous waveform.
		ViInt32 waveID;
		// This parameter is used to hold the waveform name that actually appears in the final script. I included a couple extra places for good measure.
		ViChar tempWaveformName[11];
		ViChar prevTempWaveName[11];
		// Holds actual waveform data.
		ViReal64 *xWaveform = NULL, *yWaveform = NULL, *mixedWaveform = NULL, *xWaveReadData = NULL, *yWaveReadData = NULL;
		// Holds data in the case that a previous waveform is used to correct for the current waveform's phase.
		ViReal64 *prevXWaveform = NULL, *prevYWaveform = NULL, *prevXWaveformRead = NULL, *prevYWaveformRead = NULL, *prevMixedWaveform = NULL;

		// these are the variables that hold current basic information about the waveform being generated. They are used to create the raw data points. They are 
		// temporary, and pass their information into the larger array of waveData structures.
		waveData xWaveformParameters, yWaveformParameters;

		while (!verticalFileName.eof())
		{
			// Check to see if y-file is at end of file
			if (horizontalFileName.eof())
			{
				std::string errMsg = "User's horizontal script file has ended before the vertical script file. Please make sure that the number of commands in each file matches.\r\n";
				if (isThreaded)
				{
					postMyString(eErrorTextMessageID, "User's horizontal script file has ended before the vertical-script file."
						" Please make sure that the number of commands in each file matches.\r\n");
					PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
				}
				else
				{
					if (isDefault)
					{
						MessageBox(NULL, errMsg.c_str(), 0, MB_OK | MB_ICONERROR);
					}
					else
					{
						appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
					}
				}
				return -3;
			}

			xWaveformParameters = {};
			yWaveformParameters = {};
			sprintf_s(tempWaveformName, 11, "Waveform%i", waveCount);
			sprintf_s(prevTempWaveName, 11, "Waveform%i", waveCount - 1);

			// get rid of white space before instructions line,
			rmWhite(verticalFileName);
			// get instructions line,
			std::getline(verticalFileName, verticalInputTypeString);
			if (verticalInputTypeString.length() != 0)
			{
				if (verticalInputTypeString[verticalInputTypeString.length() - 1] == '\r')
				{
					verticalInputTypeString.erase(verticalInputTypeString.length() - 1);
				}
			}
			rmWhite(horizontalFileName);
			std::getline(horizontalFileName, horizontalInputTypeString);
			if (horizontalInputTypeString.length() != 0)
			{
				if (horizontalInputTypeString[horizontalInputTypeString.length() - 1] == '\r')
				{
					horizontalInputTypeString.erase(horizontalInputTypeString.length() - 1);
				}
			}

			// make the input all lower case to prevent trivial user errors in CaPiTaLiZaTiOn.
			std::transform(verticalInputTypeString.begin(), verticalInputTypeString.end(), verticalInputTypeString.begin(), ::tolower);
			std::transform(horizontalInputTypeString.begin(), horizontalInputTypeString.end(), horizontalInputTypeString.begin(), ::tolower);
			// Send the command to different places depending on what type of command it is.
			if (myNIAWG::script::isLogicCommand(verticalInputTypeString) && myNIAWG::script::isLogicCommand(horizontalInputTypeString))
			{
				if (myNIAWG::handleInput::logic(verticalFileName, horizontalFileName, verticalInputTypeString, horizontalInputTypeString, scriptHolder, triggerName) != 0)
				{
					std::string errMsg = "handleInput::logic() threw an error when handling waveform #" + std::to_string(waveCount - 1) + "!\r\n";
					if (isThreaded)
					{
						postMyString(eErrorTextMessageID, errMsg);

						PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
					}
					else
					{
						if (isDefault)
						{
							MessageBox(NULL, errMsg.c_str(), 0, MB_OK | MB_ICONERROR);
						}
						else
						{
							appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
						}
					}
					return -13954;
				}
			}

			else if (myNIAWG::script::isGenCommand(verticalInputTypeString) && myNIAWG::script::isGenCommand(horizontalInputTypeString))
			{
				//
				if ((waveCount == 1 && isDefault && eCurrentOrientation == "Horizontal") || (waveCount == 2 && isDefault && eCurrentOrientation == "Vertical"))
				{
					// error:
					std::string errMsg = "ERROR: The default waveform files contain sequences of waveforms. Right now, the default waveforms must be a single waveform, "
										 "not a sequence.\r\n";
					if (isThreaded)
					{
						postMyString(eErrorTextMessageID, errMsg);
						PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
					}
					else
					{
						if (isDefault)
						{
							MessageBox(NULL, errMsg.c_str(), 0, MB_OK | MB_ICONERROR);
						}
						else
						{
							appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
						}
					}
					return -8293;
				}
				
				// Create a spot in the verticalWaveformVaried vector for this waveform. The default value is false. If the waveform isn't varied, it gets changed to true.
				verticalWaveformVaried.push_back(false);
				horizontalWaveformVaried.push_back(false);

				// Get a number corresponding directly to the given input type.
				myNIAWG::handleInput::getInputType(verticalInputTypeString, xWaveformParameters);
				myNIAWG::handleInput::getInputType(horizontalInputTypeString, yWaveformParameters);

				if (xWaveformParameters.initType == -1 || yWaveformParameters.initType == -1)
				{
					std::string errMsg = "ERROR: getInputType threw an error!\r\n";
					if (isThreaded)
					{
						postMyString(eErrorTextMessageID, errMsg);
						PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
					}
					else
					{
						if (isDefault)
						{
							MessageBox(NULL, errMsg.c_str(), 0, MB_OK | MB_ICONERROR);
						}
						else{
							appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
						}
					}
					return -1923;
				}
				// this switch statement subdivides the inputs based off of the number of signals the given command was.
				switch (xWaveformParameters.initType)
				{
					case 0:
					case 5:
					case 10:
					case 15:
						xWaveformParameters.signalNum = 1;
						break;
					case 1:
					case 6:
					case 11:
					case 16:
						xWaveformParameters.signalNum = 2;
						break;
					case 2:
					case 7:
					case 12:
					case 17:
						xWaveformParameters.signalNum = 3;
						break;
					case 3:
					case 8:
					case 13:
					case 18:
						xWaveformParameters.signalNum = 4;
						break;
					case 4:
					case 9:
					case 14:
					case 19:
						xWaveformParameters.signalNum = 5;
						break;
				}

				switch (yWaveformParameters.initType)
				{
					case 0:
					case 5:
					case 10:
					case 15:
						yWaveformParameters.signalNum = 1;
						break;
					case 1:
					case 6:
					case 11:
					case 16:
						yWaveformParameters.signalNum = 2;
						break;
					case 2:
					case 7:
					case 12:
					case 17:
						yWaveformParameters.signalNum = 3;
						break;
					case 3:
					case 8:
					case 13:
					case 18:
						yWaveformParameters.signalNum = 4;
						break;
					case 4:
					case 9:
					case 14:
					case 19:
						yWaveformParameters.signalNum = 5;
						break;
				}

				// Gather the parameters the user inputted for the X waveform and sort them into the appropriate data structure.
				if (myNIAWG::handleInput::getWvFmData(verticalFileName, xWaveformParameters) != 0)
				{
					std::string errorMsg;
					errorMsg = "getWvFmData() threw an error when handling x waveform #" + std::to_string(waveCount - 1) + "!";
					if (isThreaded)
					{
						postMyString(eErrorTextMessageID, errorMsg);
						PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
					}
					else
					{
						if (isDefault)
						{
							MessageBox(NULL, errorMsg.c_str(), "ERROR", MB_OK | MB_ICONERROR);
						}
						else
						{
							appendText(errorMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
						}
					}
					return -1634;
				}

				// Gather the parameters the user inputted for the Y waveform and sort them into the appropriate data structure.
				if (myNIAWG::handleInput::getWvFmData(horizontalFileName, yWaveformParameters) != 0)
				{
					std::string errMsg = "getWvFmData() threw an error when handling y waveform #" + std::to_string(waveCount - 1) + "!";
					if (isThreaded)
					{
						postMyString(eErrorTextMessageID, errMsg);
						PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
					}
					else
					{
						if (isDefault)
						{
							MessageBox(NULL, errMsg.c_str(), 0, MB_OK | MB_ICONERROR);
						}
						else
						{
							appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
						}
					}
					return -1634;
				}
				// check the delimeters
				if (xWaveformParameters.delim != "#")
				{
					std::string errMsg = std::string("ERROR: The delimeter is missing in the vertical script file for waveform #") + std::to_string(waveCount - 1) +
						"The value placed in the delimeter location was " + xWaveformParameters.delim + " while it should have been '#'.This";
					" indicates that either the code is not interpreting the user input correctly or that the user has inputted too many parameters for this type"
						" of waveform.";
					if (isThreaded)
					{
						postMyString(eErrorTextMessageID, errMsg);
						PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
					}
					else
					{
						if (isDefault)
						{
							MessageBox(NULL, errMsg.c_str(), 0, MB_OK | MB_ICONERROR);
						}
						else
						{
							appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
						}
					}
					return -1635;
				}
				if (yWaveformParameters.delim != "#")
				{
					std::string errMsg = std::string("ERROR: The delimeter is missing in the Y script file for waveform #") + std::to_string(waveCount - 1) +
						"The value placed in the delimeter location was " + yWaveformParameters.delim + " while it should have been '#'.This";
					" indicates that either the code is not interpreting the user input correctly or that the user has inputted too many parameters for this type"
						" of waveform.";
					if (isThreaded)
					{
						postMyString(eErrorTextMessageID, errMsg);
						PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
					}
					else
					{
						if (isDefault)
						{
							MessageBox(NULL, errMsg.c_str(), 0, MB_OK | MB_ICONERROR);
						}
						else
						{
							appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
						}
					}
					return -1635;
				}
				int currentTimeManageOption;
				// make sure that the waveform management options match (they must, or else the times might not match.
				if (xWaveformParameters.phaseManagementOption != yWaveformParameters.phaseManagementOption)
				{
					std::string errMsg = "ERROR: the x and y waveforms must have the same time management option. They appear to be mismatched for waveform #" + std::to_string(waveCount - 1) + "!";
					if (isThreaded)
					{
						postMyString(eErrorTextMessageID, errMsg);
						PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
					}
					else
					{
						if (isDefault)
						{
							MessageBox(NULL, errMsg.c_str(), 0, MB_OK | MB_ICONERROR);
						}
						else
						{
							appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
						}
					}
					return -3739;
				}
				else
				{
					// set this as a more transparent way of checking the current time management option. X or y should be the same.
					currentTimeManageOption = xWaveformParameters.phaseManagementOption;
				}

				// make sure the times match.
				if (!(fabs(xWaveformParameters.time - yWaveformParameters.time) < 1e-6))
				{
					std::string errMsg = "ERROR: the x and y waveforms must have the same time value. They appear to be mismatched for waveform #" + std::to_string(waveCount - 1) + "!";
					if (isThreaded)
					{
						postMyString(eErrorTextMessageID, errMsg);
						PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
					}
					else
					{
						if (isDefault)
						{
							MessageBox(NULL, errMsg.c_str(), 0, MB_OK | MB_ICONERROR);
						}
						else
						{
							appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
						}
					}
					return -96338;
				}

				if (xWaveformParameters.varNum < 0)
				{
					std::string errMsg = "ERROR: handleInput::waveformData() threw an error while handling X waveform #" + std::to_string(waveCount - 1) + "!";
					if (isThreaded)
					{
						postMyString(eErrorTextMessageID, errMsg);
						PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
					}
					else
					{
						if (isDefault)
						{
							MessageBox(NULL, errMsg.c_str(), 0, MB_OK | MB_ICONERROR);
						}
						else{
							appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
						}
					}
					return -101;
				}

				if (yWaveformParameters.varNum  < 0)
				{
					std::string errMsg = "ERROR: handleInput::waveformData() threw an error while handling horizontal waveform #" + std::to_string(waveCount - 1) + "!";
					if (isThreaded)
					{
						postMyString(eErrorTextMessageID, errMsg);
						PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
					}
					else
					{
						if (isDefault)
						{
							MessageBox(NULL, errMsg.c_str(), 0, MB_OK | MB_ICONERROR);
						}
						else
						{
							appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
						}
					}
					return -101;
				}
				/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				///
				///					Handle -1 Phase (Use the phase that the previous waveform ended with)
				///

				/// Handle phase = -1 for X
				// If the user used a '-1' for the initial phase, this means the user wants to copy the ending phase of the previous waveform.
				for (int i = 0; i < xWaveformParameters.signalNum; i++)
				{
					if (xWaveformParameters.signals[i].initPhase == -1)
					{
						xWaveformParameters.signals[i].phaseOption = -1;
						// if you are trying to copy the phase from a waveform that is being varied, this can only be accomplished if this waveform is also varied.
						// mark this waveform for varying and break.
						if (verticalWaveformVaried[waveCount - 1] == true)
						{
							verticalWaveformVaried[waveCount] = true;
							// Currently, it isn't handled specially when only one of the waves in a waveform is varied.
							horizontalWaveformVaried[waveCount] = true;
							xWaveformParameters.varNum++;
							// the % sign is reserved. Don't use it in a script file. It's just a place-holder here to make sure the number of varied waveforms gets
							// understood properly.
							xWaveformParameters.varNames.push_back("\'");
							xWaveformParameters.varTypes.push_back(-1);
							break;
						}
						if (i + 1 > allVerticalWaveParameters[waveCount - 1].signalNum)
						{
							std::string errMsg = "ERROR: You are trying to copy the phase of signal " + std::to_string(i + 1) + "  of X waveform #" + std::to_string(waveCount - 1)
								+ ", but the previous waveform only " + "had " + std::to_string(allVerticalWaveParameters[waveCount - 1].signalNum) + " signals!\n";
							if (isThreaded)
							{
								postMyString(eErrorTextMessageID, errMsg);
								PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
							}
							else
							{
								if (isDefault)
								{
									MessageBox(NULL, errMsg.c_str(), 0, MB_OK | MB_ICONERROR);
								}
								else
								{
									appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
								}
							}
							return -1304;
						}
						xWaveformParameters.signals[i].initPhase = allVerticalWaveParameters[waveCount - 1].signals[i].finPhase;
					}
				}
				/// Handle phase = -1 for Y
				// If the user used a '-1' for the initial phase, this is code for "copy the ending phase of the previous waveform".
				for (int i = 0; i < yWaveformParameters.signalNum; i++)
				{
					if (yWaveformParameters.signals[i].initPhase == -1)
					{
						yWaveformParameters.signals[i].phaseOption = -1;
						// if you are trying to copy the phase from a waveform that is being varied, this can only be accomplished if this waveform is also varied.
						// mark this waveform for varying and break.
						if (horizontalWaveformVaried[waveCount - 1] == true)
						{
							horizontalWaveformVaried[waveCount] = true;
							verticalWaveformVaried[waveCount] = true;
							yWaveformParameters.varNum++;
							// the ' (single quote) sign is reserved. Don't use it in a script file. It's just a place-holder here to make sure the number of varied waveforms gets
							// understood properly.
							yWaveformParameters.varNames.push_back("\'");
							yWaveformParameters.varTypes.push_back(-1);
							break;
						}
						if (i + 1 > allHorizontalWaveParameters[waveCount - 1].signalNum)
						{
							std::string errMsg = "ERROR: You are trying to copy the phase of signal " + std::to_string(i + 1) + " of Y waveform #" + std::to_string(waveCount - 1)
								+ ", but the previous waveform only " + "had " + std::to_string(allHorizontalWaveParameters[waveCount - 1].signalNum) + " signals!\n";
							if (isThreaded)
							{
								postMyString(eErrorTextMessageID, errMsg);
								PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
							}
							else
							{
								if (isDefault)
								{
									MessageBox(NULL, errMsg.c_str(), 0, MB_OK | MB_ICONERROR);
								}
								else{
									appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
								}
							}
							return -1304;
						}
						yWaveformParameters.signals[i].initPhase = allHorizontalWaveParameters[waveCount - 1].signals[i].finPhase;
					}
				}

				/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				///
				///					Handle -1 Time Management (Bring the frequencies in the previous waveforms to 0 phase.
				///
				///

				/// Handle -1 Time Management for X & Y
				// Check if user used -1 for the time management of this waveform.
				if (currentTimeManageOption == -1)
				{
					std::vector<double> initPhasesForTimeManage;
					// Make sure that both waveforms have the right number of signals.
					if (xWaveformParameters.signalNum != allVerticalWaveParameters[waveCount - 1].signalNum){
						std::string errMsg = "ERROR: Signal Number Mismatch! You appear to be attempting to correcting the phase of waveform number " +
							std::to_string(waveCount - 2) + " with waveform number" + std::to_string(waveCount - 1) + ", but the vertical component of the former has " +
							std::to_string(allVerticalWaveParameters[waveCount - 1].signalNum) + " signals and the latter has " +
							std::to_string(xWaveformParameters.signalNum) +
							" signals. In order for a waveform to correct the time of another waveform, the two must have the same number of signals.";
						if (isThreaded)
						{
							postMyString(eErrorTextMessageID, errMsg);
							PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
						}
						else
						{
							if (isDefault)
							{
								MessageBox(NULL, errMsg.c_str(), 0, MB_OK | MB_ICONERROR);
							}
							else
							{
								appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
							}
						}
						return -10583;
					}
					// check to make sure that the frequencies match
					// For all X signals in a waveform...
					for (int o = 0; o < allVerticalWaveParameters[waveCount - 1].signalNum; o++)
					{
						// Check...
						if (allVerticalWaveParameters[waveCount - 1].signals[o].freqFin != xWaveformParameters.signals[o].freqInit)
						{
							// report error
							std::string errMsg = "ERROR: Frequency Mismatch! You appear to be attempting to correcting the phase of waveform number "
												 + std::to_string(waveCount - 2) + " with waveform number" + std::to_string(waveCount - 1) + ", but signal "
												 + std::to_string(o) + " in the vertical component of the former has final frequency "
												 + std::to_string(allVerticalWaveParameters[waveCount - 1].signals[o].freqFin) + " and signal " + std::to_string(o)
												 + " in the vertical component of the latter has initial frequency of"
												 + std::to_string(xWaveformParameters.signals[o].freqInit)
												 + " signals. In order for a waveform to correct the time of another waveform, these frequencies should match.";
							if (isThreaded)
							{
								postMyString(eErrorTextMessageID, errMsg);
								PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
							}
							else
							{
								if (isDefault)
								{
									MessageBox(NULL, errMsg.c_str(), 0, MB_OK | MB_ICONERROR);
								}
								else
								{
									appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
								}
							}
							return -234923;
						}
						initPhasesForTimeManage.push_back(xWaveformParameters.signals[o].initPhase);
					}
					// for all Y signals in a waveform...
					for (int o = 0; o < allHorizontalWaveParameters[waveCount - 1].signalNum; o++)
					{
						// check to make sure that the frequencies match
						if (allHorizontalWaveParameters[waveCount - 1].signals[o].freqFin != yWaveformParameters.signals[o].freqInit)
						{
							std::string errMsg = "ERROR: Frequency Mismatch! You appear to be attempting to correcting the phase of waveform number " + std::to_string(waveCount - 2) +
								" with waveform number" + std::to_string(waveCount - 1) +
								", but signal " + std::to_string(o) + " in the horizontal component of the former has final frequency " + std::to_string(allHorizontalWaveParameters[waveCount - 1].signals[o].freqFin) +
								" and signal " + std::to_string(o) + " in the horizontal component of the latter has initial frequency of" + std::to_string(yWaveformParameters.signals[o].freqInit) +
								" signals. In order for a waveform to correct the time of another waveform, these frequencies should match.";
							if (isThreaded)
							{
								postMyString(eErrorTextMessageID, errMsg);
								PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
							}
							else
							{
								if (isDefault)
								{
									MessageBox(NULL, errMsg.c_str(), 0, MB_OK | MB_ICONERROR);
								}
								else
								{
									appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
								}
							}
							return -234923;
						}
						initPhasesForTimeManage.push_back(yWaveformParameters.signals[o].initPhase);
					}

					if (verticalWaveformVaried[waveCount - 1] == true)
					{
						// Then mark this waveform as being Varied.
						verticalWaveformVaried[waveCount] = true;
						horizontalWaveformVaried[waveCount] = true;
						// increase the variable number in x.
						xWaveformParameters.varNum++;
						// the ' single quotation mark is reserved. Don't use it in a script file. It's just a place-holder here to make sure the number of 
						// varied waveforms gets understood properly.
						xWaveformParameters.varNames.push_back("\'");
						xWaveformParameters.varTypes.push_back(-2);
						yWaveformParameters.varNum++;
						// the ' sign is reserved. Don't use it in a script file. It's just a place-holder here to make sure the number of varied waveforms 
						// gets understood properly.
						yWaveformParameters.varNames.push_back("\'");
						yWaveformParameters.varTypes.push_back(-2);
					}
					else if (verticalWaveformVaried[waveCount - 1] == false)
					{
						double errVal = myMath::calculateCorrectionTime(xWaveformParameters, yWaveformParameters, initPhasesForTimeManage, "after");
						if (errVal == -1)
						{
							MessageBox(0, "ERROR: Correction waveform was not able to match phases.", 0, 0);
							// throw error
							return -1029;
						}
						else if (errVal != 0)
						{
							std::string errMsg = "WARNING: Correction waveform was not able to match phases very well. The total phase mismatch was "
								+ std::to_string(errVal) + "radians.\r\n";
							appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
							// throw warning
						}

					}
				}
				/// Waveform Creation
				// only create waveform data if neither waveform is being varried and if the time management option is either 0 or -1. The time management  
				// option has already been checked to be the same for x and Y waveforms.
				if (xWaveformParameters.varNum == 0 && yWaveformParameters.varNum == 0 && xWaveformParameters.phaseManagementOption < 1)
				{
					// Initialize the giant waveform arrays.
					xWaveform = new ViReal64[xWaveformParameters.sampleNum];
					xWaveReadData = new ViReal64[xWaveformParameters.sampleNum + xWaveformParameters.signalNum];
					// either calculate or read waveform data into the above arrays. 
					if (myNIAWG::handleInput::waveformGen(xWaveform, xWaveReadData, xWaveformParameters, xWaveformParameters.sampleNum, libWaveformArray,
						fileStatus[xWaveformParameters.initType]) != 0)
					{
						std::string errorMsg;
						errorMsg = "ERROR: handleInput::waveformGen threw an error while handling vertical waveform #" + std::to_string(waveCount - 1) + "!";
						if (isThreaded)
						{
							postMyString(eErrorTextMessageID, errorMsg);
							PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
						}
						else
						{
							if (isDefault)
							{
								MessageBox(NULL, errorMsg.c_str(), 0, MB_OK | MB_ICONERROR);
							}
							else
							{
								appendText(errorMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
							}
						}
						return -101;
					}
					delete[] xWaveReadData;
				}
				else if (xWaveformParameters.varNum > 0)
				{
					// Mark this waveform as being varied.
					verticalWaveformVaried[waveCount] = true;
					horizontalWaveformVaried[waveCount] = true;
				}

				// only create waveform data if neither waveform is being varried.
				if (xWaveformParameters.varNum == 0 && yWaveformParameters.varNum == 0 && xWaveformParameters.phaseManagementOption < 1)
				{
					// Initialize the giant waveform arrays.
					yWaveform = new ViReal64[yWaveformParameters.sampleNum];
					yWaveReadData = new ViReal64[yWaveformParameters.sampleNum + yWaveformParameters.signalNum];
					// either calculate or read waveform data into the above arrays. 
					if (myNIAWG::handleInput::waveformGen(yWaveform, yWaveReadData, yWaveformParameters, yWaveformParameters.sampleNum, libWaveformArray,
						fileStatus[yWaveformParameters.initType]) != 0)
					{
						std::string errorMsg;
						errorMsg = "ERROR: handleInput::waveformGen threw an error while handling horizontal waveform #" + std::to_string(waveCount - 1) + "!";
						MessageBox(NULL, errorMsg.c_str(), "ERROR", MB_OK | MB_ICONERROR);

						return -101;
					}
					delete[] yWaveReadData;
				}
				else if (yWaveformParameters.varNum  > 0)
				{
					// Mark this waveform as being varied.
					horizontalWaveformVaried[waveCount] = true;
					verticalWaveformVaried[waveCount] = true;
				}

				/// Handle +1 Time Management ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
				// if not the first waveform
				if (waveCount != 0)
				{
					/// Handle +1 for X
					// check if user used +1 for the time of the previous waveform.
					if (allVerticalWaveParameters[waveCount - 1].phaseManagementOption == 1)
					{
						// Make sure the waveforms are compatible.
						std::vector<double> initPhasesForTimeManage;
						if (xWaveformParameters.signalNum != allVerticalWaveParameters[waveCount - 1].signalNum){
							std::string errorMsg;
							errorMsg = "ERROR: Signal Number Mismatch! You appear to be attempting to correcting the phase of waveform number " +
								std::to_string(waveCount - 1) + " with waveform number" + std::to_string(waveCount - 2) + ", but the vertical component of the former has " +
								std::to_string(xWaveformParameters.signalNum) + " signals and the latter has " +
								std::to_string(allVerticalWaveParameters[waveCount - 1].signalNum) +
								" signals. In order for a waveform to correct the time of another waveform, the two must have the same number of signals.";
							if (isThreaded)
							{
								postMyString(eErrorTextMessageID, errorMsg);
								PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
							}
							else
							{
								if (isDefault)
								{
									MessageBox(NULL, errorMsg.c_str(), 0, MB_OK | MB_ICONERROR);
								}
								else
								{
									appendText(errorMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
								}
							}
							return -10238;
						}
						// check to make sure that the X frequencies match
						for (int o = 0; o < allVerticalWaveParameters[waveCount - 1].signalNum; o++)
						{
							if (allVerticalWaveParameters[waveCount - 1].signals[o].freqInit != xWaveformParameters.signals[o].freqInit)
							{
								std::string errorMsg;

								errorMsg = "ERROR: Frequency Mismatch! You appear to be attempting to correcting the phase of waveform number " + std::to_string(waveCount - 1) +
									" with waveform number" + std::to_string(waveCount - 2) +
									", but signal " + std::to_string(o) + " in the vertical component of the former has final frequency " + std::to_string(xWaveformParameters.signals[o].freqInit) +
									" and signal " + std::to_string(o) + " in the vertical component of the latter has initial frequency of" + std::to_string(allVerticalWaveParameters[waveCount - 1].signals[o].freqFin) +
									" signals. In order for a waveform to correct the time of another waveform, these frequencies should match.";
								if (isThreaded)
								{
									postMyString(eErrorTextMessageID, errorMsg);
									PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
								}
								else
								{
									if (isDefault)
									{
										MessageBox(NULL, errorMsg.c_str(), 0, MB_OK | MB_ICONERROR);
									}
									else
									{
										appendText(errorMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
									}
								}
								return -234923;
							}
							initPhasesForTimeManage.push_back(xWaveformParameters.signals[o].finPhase + allVerticalWaveParameters[waveCount - 1].signals[o].initPhase);
						}
						// check to make sure that the X frequencies match
						for (int o = 0; o < allHorizontalWaveParameters[waveCount - 1].signalNum; o++)
						{
							// check to make sure that the frequencies match
							if (allHorizontalWaveParameters[waveCount - 1].signals[o].freqInit != yWaveformParameters.signals[o].freqInit){
								std::string errorMsg;
								errorMsg = "ERROR: Frequency Mismatch! You appear to be attempting to correcting the phase of waveform number " + std::to_string(waveCount - 1) +
									" with waveform number" + std::to_string(waveCount - 2) +
									", but signal " + std::to_string(o) + " in the horizontal component of the former has final frequency " + std::to_string(yWaveformParameters.signals[o].freqInit) +
									" and signal " + std::to_string(o) + " in the horizontal component of the latter has initial frequency of" + std::to_string(allHorizontalWaveParameters[waveCount - 1].signals[o].freqFin) +
									" signals. In order for a waveform to correct the time of another waveform, these frequencies should match.";
								if (isThreaded)
								{
									postMyString(eErrorTextMessageID, errorMsg);
									PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
								}
								else
								{
									if (isDefault)
									{
										MessageBox(NULL, errorMsg.c_str(), 0, MB_OK | MB_ICONERROR);
									}
									else
									{
										appendText(errorMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
									}
								}
								return -234923;
							}
							initPhasesForTimeManage.push_back(yWaveformParameters.signals[o].finPhase + allHorizontalWaveParameters[waveCount - 1].signals[o].initPhase);
						}

						if (verticalWaveformVaried[waveCount] == true)
						{
							verticalWaveformVaried[waveCount - 1] = true;
							horizontalWaveformVaried[waveCount - 1] = true;
							allVerticalWaveParameters[waveCount - 1].varNum++;
							// the ' sign is reserved. Don't use it in a script file. It's just a place-holder here to make sure the number of varied waveforms gets
							// understood properly.
							allVerticalWaveParameters[waveCount - 1].varNames.push_back("\'");
							allVerticalWaveParameters[waveCount - 1].varTypes.push_back(-3);
						}
						else if (verticalWaveformVaried[waveCount] == false)
						{
							// calculate a time for the previous waveform.
							double errVal = myMath::calculateCorrectionTime(allVerticalWaveParameters[waveCount - 1], allHorizontalWaveParameters[waveCount - 1], initPhasesForTimeManage, "before");
							if (errVal == -1)
							{
								MessageBox(0, "ERROR: Correction waveform was not able to match phases.", 0, 0);
								// throw error
								return -1029;
							}
							else if (errVal != 0)
							{
								std::string errMsg = "WARNING: Correction waveform was not able to match phases very well. The total phase mismatch was "
									+ std::to_string(errVal) + "radians.\r\n";
								appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
								// throw warning
							}

							// calculate the previous waveform.
							// Initialize the giant waveform arrays.
							prevXWaveform = new ViReal64[allVerticalWaveParameters[waveCount - 1].sampleNum];
							prevXWaveformRead = new ViReal64[allVerticalWaveParameters[waveCount - 1].sampleNum + allVerticalWaveParameters[waveCount - 1].signalNum];
							// either calculate or read waveform data into the above arrays. 
							if (myNIAWG::handleInput::waveformGen(prevXWaveform, prevXWaveformRead, allVerticalWaveParameters[waveCount - 1], allVerticalWaveParameters[waveCount - 1].sampleNum,
								libWaveformArray, fileStatus[allVerticalWaveParameters[waveCount - 1].initType]) != 0){
								std::string errorMsg;
								errorMsg = "ERROR: handleInput::waveformGen threw an error while handling X waveform #" + std::to_string(waveCount - 1) + "!";
								if (isThreaded)
								{
									postMyString(eErrorTextMessageID, errorMsg);
									PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
								}
								else
								{
									if (isDefault)
									{
										MessageBox(NULL, errorMsg.c_str(), 0, MB_OK | MB_ICONERROR);
									}
									else
									{
										appendText(errorMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
									}
								}
								return -1010;
							}
							// modify the phases of the current waveform such that they reach the 0 phase after the waveform.
							for (int v = 0; v < xWaveformParameters.signalNum; v++){
								xWaveformParameters.signals[v].initPhase = 2 * PI - xWaveformParameters.signals[v].finPhase;
							}
						}
					}

					/// Handle +1 for Y
					if (allHorizontalWaveParameters[waveCount - 1].phaseManagementOption == 1)
					{
						std::vector<double> initPhasesForTimeManage;
						if (yWaveformParameters.signalNum != allHorizontalWaveParameters[waveCount - 1].signalNum)
						{
							std::string errorMsg;
							errorMsg = "ERROR: Signal Number Mismatch! You appear to be attempting to correcting the phase of waveform number " +
								std::to_string(waveCount - 1) + " with waveform number" + std::to_string(waveCount - 2) + ", but the horizontal component of the former has " +
								std::to_string(yWaveformParameters.signalNum) + " signals and the horizontal component of the latter has " +
								std::to_string(allHorizontalWaveParameters[waveCount - 1].signalNum) +
								" signals. In order for a waveform to correct the time of another waveform, the two must have the same number of signals.";
							if (isThreaded)
							{
								postMyString(eErrorTextMessageID, errorMsg);
								PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
							}
							else
							{
								if (isDefault)
								{
									MessageBox(NULL, errorMsg.c_str(), 0, MB_OK | MB_ICONERROR);
								}
								else
								{
									appendText(errorMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
								}
							}
							return -10238;
						}
						for (int o = 0; o < allHorizontalWaveParameters[waveCount - 1].signalNum; o++)
						{
							// check to make sure that the frequencies match
							if (allHorizontalWaveParameters[waveCount - 1].signals[o].freqInit != yWaveformParameters.signals[o].freqInit)
							{
								std::string errorMsg;

								errorMsg = "ERROR: Frequency Mismatch! You appear to be attempting to correcting the phase of waveform number " + std::to_string(waveCount - 1) +
									" with waveform number" + std::to_string(waveCount - 2) +
									", but signal " + std::to_string(o) + " in the horizontal component of the former has final frequency " + std::to_string(yWaveformParameters.signals[o].freqInit) +
									" and signal " + std::to_string(o) + " in the horizontal component of the latter has initial frequency of" + std::to_string(allHorizontalWaveParameters[waveCount - 1].signals[o].freqFin) +
									" signals. In order for a waveform to correct the time of another waveform, these frequencies should match.";
								if (isThreaded)
								{
									postMyString(eErrorTextMessageID, errorMsg);
									PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
								}
								else
								{
									if (isDefault)
									{
										MessageBox(NULL, errorMsg.c_str(), 0, MB_OK | MB_ICONERROR);
									}
									else
									{
										appendText(errorMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
									}
								}
								return -234923;
							}
							initPhasesForTimeManage.push_back(yWaveformParameters.signals[o].finPhase + allHorizontalWaveParameters[waveCount - 1].signals[o].initPhase);
						}
						for (int o = 0; o < allVerticalWaveParameters[waveCount - 1].signalNum; o++)
						{
							// check to make sure that the frequencies match
							if (allVerticalWaveParameters[waveCount - 1].signals[o].freqInit != xWaveformParameters.signals[o].freqInit){
								std::string errorMsg;

								errorMsg = "ERROR: Frequency Mismatch! You appear to be attempting to correcting the phase of waveform number " + std::to_string(waveCount - 1) +
									" with waveform number" + std::to_string(waveCount - 2) +
									", but signal " + std::to_string(o) + " in the vertical component of the former has final frequency " + std::to_string(xWaveformParameters.signals[o].freqInit) +
									" and signal " + std::to_string(o) + " in the vertical component of the latter has initial frequency of" + std::to_string(allVerticalWaveParameters[waveCount - 1].signals[o].freqFin) +
									" signals. In order for a waveform to correct the time of another waveform, these frequencies should match.";
								if (isThreaded)
								{
									postMyString(eErrorTextMessageID, errorMsg);
									PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
								}
								else
								{
									if (isDefault)
									{
										MessageBox(NULL, errorMsg.c_str(), 0, MB_OK | MB_ICONERROR);
									}
									else
									{
										appendText(errorMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
									}
								}
								return -234923;
							}
							initPhasesForTimeManage.push_back(xWaveformParameters.signals[o].finPhase + allVerticalWaveParameters[waveCount - 1].signals[o].initPhase);
						}
						if (horizontalWaveformVaried[waveCount] == true)
						{
							horizontalWaveformVaried[waveCount - 1] = true;
							verticalWaveformVaried[waveCount - 1] = true;
							allHorizontalWaveParameters[waveCount - 1].varNum++;
							// the ' sign is reserved. Don't use it in a script file. It's just a place-holder here to make sure the number of varied waveforms gets
							// understood properly.
							allHorizontalWaveParameters[waveCount - 1].varNames.push_back("\'");
							allHorizontalWaveParameters[waveCount - 1].varTypes.push_back(-3);
						}
						else if (horizontalWaveformVaried[waveCount - 1] == false)
						{

							double errVal = myMath::calculateCorrectionTime(allHorizontalWaveParameters[waveCount - 1], allVerticalWaveParameters[waveCount - 1], initPhasesForTimeManage,
								"before");
							if (errVal == -1)
							{
								MessageBox(0, "ERROR: Correction waveform was not able to match phases.", 0, 0);
								// throw error
								return -1029;
							}
							else if (errVal != 0)
							{
								std::string errMsg = "WARNING: Correction waveform was not able to match phases very well. The total phase mismatch was "
									+ std::to_string(errVal) + "radians.\r\n";
								appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
								// throw warning
							}


							// calculate the previous waveform.
							// Initialize the giant waveform arrays.
							prevYWaveform = new ViReal64[allHorizontalWaveParameters[waveCount - 1].sampleNum];
							prevYWaveformRead = new ViReal64[allHorizontalWaveParameters[waveCount - 1].sampleNum + allHorizontalWaveParameters[waveCount - 1].signalNum];
							// either calculate or read waveform data into the above arrays. 
							if (myNIAWG::handleInput::waveformGen(prevYWaveform, prevYWaveformRead, allHorizontalWaveParameters[waveCount - 1], 
																  allHorizontalWaveParameters[waveCount - 1].sampleNum,
								libWaveformArray, fileStatus[allHorizontalWaveParameters[waveCount - 1].initType]) != 0){
								std::string errorMsg;
								errorMsg = "ERROR: handleInput::waveformGen threw an error while handling Y waveform #" + std::to_string(waveCount - 1) + "!";
								if (isThreaded)
								{
									postMyString(eErrorTextMessageID, errorMsg);
									PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
								}
								else
								{
									if (isDefault)
									{
										MessageBox(NULL, errorMsg.c_str(), 0, MB_OK | MB_ICONERROR);
									}
									else
									{
										appendText(errorMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
									}
								}
								return -1010;
							}
							// modify the phases of the current waveform such that they reach the 0 phase after the waveform.
							for (int v = 0; v < yWaveformParameters.signalNum; v++){

								yWaveformParameters.signals[v].initPhase = 2 * PI - yWaveformParameters.signals[v].finPhase;
							}
						}
					}

					// Now That I've calculated the waveforms, I need to actually write them. I should always enter all three of these if statements if any.
					if (allHorizontalWaveParameters[waveCount - 1].phaseManagementOption == 1 && allVerticalWaveParameters[waveCount - 1].phaseManagementOption == 1
						&& horizontalWaveformVaried[waveCount] == false && verticalWaveformVaried[waveCount] == false){
						// Check for bad input
						if (allVerticalWaveParameters[waveCount - 1].sampleNum != allHorizontalWaveParameters[waveCount - 1].sampleNum){
							std::string errorMsg;
							errorMsg = "ERROR: the x and y waveforms must have the same time values option. They appear to be mismatched for waveform #" + std::to_string(waveCount - 1) + "!";
							if (isThreaded)
							{
								postMyString(eErrorTextMessageID, errorMsg);
								PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
							}
							else
							{
								if (isDefault)
								{
									MessageBox(NULL, errorMsg.c_str(), 0, MB_OK | MB_ICONERROR);
								}
								else
								{
									appendText(errorMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
								}
							}
							return -15;
						}

						// Create the mixed waveform holder
						long int mixedSize = 2 * allVerticalWaveParameters[waveCount - 1].sampleNum;
						prevMixedWaveform = new ViReal64[mixedSize];
						// Mix the waveforms
						myNIAWG::script::mixWaveforms(prevXWaveform, prevYWaveform, prevMixedWaveform, allVerticalWaveParameters[waveCount - 1].sampleNum);
						// wvfmToConsole(mixedWaveform, xWaveformParameters.sampleNum * 2);
						// create waveform (necessary?)
						if (!SAFEMODE)
						{
							if (NIAWG_CheckScriptError(niFgen_CreateWaveformF64(vi, SESSION_CHANNELS, mixedSize, prevMixedWaveform, &waveID), isDefault, isThreaded))
							{
								return 0;
							}
							// allocate waveform into the device memory 
							if (NIAWG_CheckScriptError(niFgen_AllocateNamedWaveform(vi, SESSION_CHANNELS, prevTempWaveName, mixedSize / 2), isDefault, isThreaded))
							{
								return 0;
							}
							// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
							if (NIAWG_CheckScriptError(niFgen_WriteNamedWaveformF64(vi, SESSION_CHANNELS, prevTempWaveName, mixedSize, prevMixedWaveform), isDefault, isThreaded))
							{
								return 0;
							}
						}

						// avoid memory leaks.
						delete[] prevXWaveform;
						delete[] prevYWaveform;
						delete[] prevXWaveformRead;
						delete[] prevYWaveformRead;
						delete[] prevMixedWaveform;
						rmWhite(verticalFileName);
					}

					// only create waveform data if neither waveform is being varried, and for this second time, if the previous wave was correcting this waveform such
					// that the waveform needs to be recalculated.
					if (xWaveformParameters.varNum == 0 && yWaveformParameters.varNum == 0 && xWaveformParameters.phaseManagementOption < 1 
						&& allVerticalWaveParameters[waveCount - 1].phaseManagementOption == 1)
					{
						// Initialize the giant waveform arrays.
						xWaveform = new ViReal64[xWaveformParameters.sampleNum];
						xWaveReadData = new ViReal64[xWaveformParameters.sampleNum + xWaveformParameters.signalNum];
						// either calculate or read waveform data into the above arrays. 
						if (myNIAWG::handleInput::waveformGen(xWaveform, xWaveReadData, xWaveformParameters, xWaveformParameters.sampleNum, libWaveformArray,
							fileStatus[xWaveformParameters.initType]) != 0)
						{
							std::string errorMsg;
							errorMsg = "ERROR: handleInput::waveformGen threw an error while handling X waveform #" + std::to_string(waveCount - 1) + "!";
							if (isThreaded)
							{
								postMyString(eErrorTextMessageID, errorMsg);
								PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
							}
							else
							{
								if (isDefault)
								{
									MessageBox(NULL, errorMsg.c_str(), 0, MB_OK | MB_ICONERROR);
								}
								else
								{
									appendText(errorMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
								}
							}
						}
						delete[] xWaveReadData;
					}
					else if (xWaveformParameters.varNum > 0)
					{
						// Mark this waveform as being varied.
						verticalWaveformVaried[waveCount] = true;
						horizontalWaveformVaried[waveCount] = true;
					}

					// only create waveform data if neither waveform is being varried, and for this second time, if the previous wave was correcting this waveform such
					// that the waveform needs to be recalculated.
					if (xWaveformParameters.varNum == 0 && yWaveformParameters.varNum == 0 && xWaveformParameters.phaseManagementOption < 1 
						&& allHorizontalWaveParameters[waveCount - 1].phaseManagementOption == 1)
					{
						// Initialize the giant waveform arrays.
						yWaveform = new ViReal64[yWaveformParameters.sampleNum];
						yWaveReadData = new ViReal64[yWaveformParameters.sampleNum + yWaveformParameters.signalNum];
						// either calculate or read waveform data into the above arrays. 
						if (myNIAWG::handleInput::waveformGen(yWaveform, yWaveReadData, yWaveformParameters, yWaveformParameters.sampleNum, libWaveformArray,
							fileStatus[yWaveformParameters.initType]) != 0)
						{
							std::string errorMsg;
							errorMsg = "ERROR: handleInput::waveformGen threw an error while handling Y waveform #" + std::to_string(waveCount - 1) + "!";
							if (isThreaded)
							{
								postMyString(eErrorTextMessageID, errorMsg);
								PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
							}
							else
							{
								if (isDefault)
								{
									MessageBox(NULL, errorMsg.c_str(), 0, MB_OK | MB_ICONERROR);
								}
								else
								{
									appendText(errorMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
								}
							}
							return -101;
						}
						delete[] yWaveReadData;
					}
					else if (yWaveformParameters.varNum  > 0)
					{
						// Mark this waveform as being varied.
						horizontalWaveformVaried[waveCount] = true;
						verticalWaveformVaried[waveCount] = true;
					}
				}
				if (xWaveformParameters.varNum >= 0)
				{
					allVerticalWaveParameters.push_back(xWaveformParameters);
				}

				if (yWaveformParameters.varNum >= 0)
				{
					allHorizontalWaveParameters.push_back(yWaveformParameters);
				}

				if (yWaveformParameters.varNum == 0 && xWaveformParameters.varNum == 0 && xWaveformParameters.phaseManagementOption < 1) {
					// Check for bad input
					if (xWaveformParameters.sampleNum != yWaveformParameters.sampleNum) {
						std::string errorMsg;
						errorMsg = "ERROR: the x and y waveforms must have the same time values option. They appear to be mismatched for waveform #"
							+ std::to_string(waveCount - 1) + "!";
						if (isThreaded)
						{
							postMyString(eErrorTextMessageID, errorMsg);
							PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
						}
						else
						{
							if (isDefault)
							{
								MessageBox(NULL, errorMsg.c_str(), 0, MB_OK | MB_ICONERROR);
							}
							else
							{
								appendText(errorMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
							}
						}
						return -15;
					}
					// Create the mixed waveform holder
					long int mixedSize = 2 * xWaveformParameters.sampleNum;
					mixedWaveform = new ViReal64[mixedSize];
					// Mix the waveforms
					myNIAWG::script::mixWaveforms(xWaveform, yWaveform, mixedWaveform, xWaveformParameters.sampleNum);
					if (!SAFEMODE)
					{
						// create waveform (necessary?)
						if (NIAWG_CheckScriptError(niFgen_CreateWaveformF64(vi, SESSION_CHANNELS, mixedSize, mixedWaveform, &waveID), isDefault, isThreaded))
						{
							return 0;
						}
						// allocate waveform into the device memory
						if (NIAWG_CheckScriptError(niFgen_AllocateNamedWaveform(vi, SESSION_CHANNELS, tempWaveformName, mixedSize / 2), isDefault, isThreaded))
						{
							return 0;
						}
						// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
						if (NIAWG_CheckScriptError(niFgen_WriteNamedWaveformF64(vi, SESSION_CHANNELS, tempWaveformName, mixedSize, mixedWaveform), isDefault, isThreaded))
						{
							return 0;
						}
					}
					// avoid memory leaks, but only if not default...
					if (isDefault)
					{
						if (eCurrentOrientation == "Horizontal") 
						{
							eDefault_hConfigMixedWaveform = mixedWaveform;
							eDefault_hConfigMixedSize = mixedSize;
							eDefault_hConfigWaveformName = tempWaveformName;
						}
						if (eCurrentOrientation == "Vertical")
						{
							eDefault_vConfigMixedWaveform = mixedWaveform;
							eDefault_vConfigMixedSize = mixedSize;
							eDefault_vConfigWaveformName = tempWaveformName;
						}
					}
					else
					{
						delete[] mixedWaveform;
					}
					delete[] xWaveform;
					delete[] yWaveform;
					rmWhite(verticalFileName);
				}
				// append script with the relevant command. This needs to be done even if variable waveforms are used, because I don't want to have to rewrite the
				// script to insert the new waveform name into it.
				std::string tempWfmNameString(tempWaveformName);
				scriptHolder += "generate " + tempWfmNameString + "\n";

				// increment waveform count.
				waveCount++;
			}
			else if (myNIAWG::script::isSpecialCommand(verticalInputTypeString) && myNIAWG::script::isSpecialCommand(horizontalInputTypeString))
			{
				myError = myNIAWG::handleInput::special(verticalFileName, horizontalFileName, verticalInputTypeString, horizontalInputTypeString, scriptHolder, triggerName, waveCount, vi, SESSION_CHANNELS,
					error, verticalPredWaveNames, horizontalPredWaveNames, predWaveCount, predLocs, libWaveformArray, fileStatus, allVerticalWaveParameters,
					verticalWaveformVaried, allHorizontalWaveParameters, horizontalWaveformVaried, isDefault, isThreaded, currentCategoryFolder);
			}
			// Catch bad input.
			else 
			{
				std::string errorMsg;
				errorMsg = "ERROR: Input types from the two files do not match or are unrecognized!\n Both must be logic commands, both must be generate "
					"commands, or both must be special commands. See documentation on the correct format for these commands.\n\nThe two inputed types are: " + verticalInputTypeString +
					" and " + horizontalInputTypeString + " for waveform #" + std::to_string(waveCount - 1) + "!";
				if (isThreaded)
				{
					postMyString(eErrorTextMessageID, errorMsg);
					PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
				}
				else
				{
					if (isDefault)
					{
						MessageBox(NULL, errorMsg.c_str(), 0, MB_OK | MB_ICONERROR);
					}
					else
					{
						appendText(errorMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
					}
				}
				return -3;
			}
			rmWhite(verticalFileName);
			rmWhite(horizontalFileName);
		}
		return 0;
	}

	/*
	 * this waveform is the first step in getting a varied waveform data written. It calls a couple other functions to finish the work. 
	 * varWvFmInfo: the strucutre containing the updated information for this variation of the waveform.
	 * all_X_Or_Y_WvFmParam: the vector of structures containing information for all of the X or all of the y waveforms
	 * waveOrderNum: the number of the waveform being varied.
	 * libWvFmArray: the array that contains all of the about the previously written waveforms.
	 * fileStat: the array that contains information on whether a waveform has been written before.
	 */
	int getVariedWaveform(waveData &varWvFmInfo, std::vector<waveData> all_X_Or_Y_WvFmParam, int waveOrderNum, std::vector<std::string>(&libWvFmArray)[20],
		bool(&fileStat)[20], ViReal64 * waveformRawData)
	{
		for (int i = 0; i < varWvFmInfo.signalNum; i++)
		{
			// check if this is supposed to be overwritten during variable runs.
			for (int j = 0; j < varWvFmInfo.varTypes.size(); j++)
			{
				if (varWvFmInfo.varTypes[j] == -1)
				{
					if (i + 1 > all_X_Or_Y_WvFmParam[waveOrderNum - 1].signalNum)
					{
						char tempChar[300];
						sprintf_s(tempChar, "ERROR: You are trying to copy the phase of the %ith signal of the previous waveform, but the previous "
							"waveform only had %i signals!\n", i + 1, all_X_Or_Y_WvFmParam[waveOrderNum - 1].signalNum);
						MessageBox(NULL, tempChar, NULL, MB_OK);
						return -2767;
					}
					varWvFmInfo.signals[i].initPhase = all_X_Or_Y_WvFmParam[waveOrderNum - 1].signals[i].finPhase;
				}
			}
		}

		ViReal64* waveReadData;
		waveReadData = new ViReal64[varWvFmInfo.sampleNum + varWvFmInfo.signalNum];
		// either calculate or read waveform data into the above arrays.
		if (myNIAWG::handleInput::waveformGen(waveformRawData, waveReadData, varWvFmInfo, varWvFmInfo.sampleNum, libWvFmArray, fileStat[varWvFmInfo.initType]) != 0)
		{
			MessageBox(NULL, "handleInput::waveformGen threw an error while getting a varied waveform!\n", NULL, MB_OK);
			return -11434;
		}
		delete[] waveReadData;

		return 0;
	}

	/*
	 * This function is called when it is time to vary a parameter of a waveform description file before getting the data corresponding to the new waveform.
	 * It is mostly a large case structure and has different cases for all of the different parameters you could be changing, as well as several cases for 
	 * dealing with correction waveforms and all of their glory.
	 * 
	 * std::vector<waveData>& allWvInfo1: This is the primary vector of data to be changed. For most cases, an entry inside of this vector is changed and 
										  allWvInfo2 is not used.
	 * std::vector<waveData>& allWvInfo2: This is used in the case of correction waveforms as you need both waveform's information in order to calculate the 
										  correction time.
	 * int wfCount: The current waveform being handled. Used to get information from and assign information to the correct waveform in the variation sequence.
	 * int& paramNum: A number which tells the function what to do with the data, i.e. which value where to assign it to. It get's altered during one of the 
					  correction waveform handling places.
	 * double paramVal: the value of the variable which is getting loaded into waveforms.
	 */
	int varyParam(std::vector<waveData>& allWvInfo1, std::vector<waveData>& allWvInfo2, int wfCount, int& paramNum, double paramVal)
	{
		// change parameters, depending on the case. This was set during input reading.
		std::vector<double> initPhases;
		switch (paramNum)
		{
			case -4: 
			{
				// this is the 1 option for time management control when the next waveform is being varied. 
				// calculate a time for the previous waveform.
				for (int o = 0; o < allWvInfo1[wfCount].signalNum; o++) 
				{
					// check to make sure that the frequencies match
					initPhases.push_back(allWvInfo1[wfCount + 1].signals[o].finPhase + allWvInfo1[wfCount].signals[o].initPhase);
				}
				for (int o = 0; o < allWvInfo2[wfCount].signalNum; o++) 
				{
					// check to make sure that the frequencies match
					initPhases.push_back(allWvInfo2[wfCount + 1].signals[o].finPhase + allWvInfo2[wfCount].signals[o].initPhase);
				}
				
				double errVal = myMath::calculateCorrectionTime(allWvInfo1[wfCount], allWvInfo2[wfCount], initPhases, "before");
				if (errVal == -1)
				{
					MessageBox(0, "ERROR: Correction waveform was not able to match phases.", 0, 0);
					// throw error
					return -1029;
				}
				else if (errVal != 0)
				{
					std::string errMsg = "WARNING: Correction waveform was not able to match phases very well. The total phase mismatch was "
						+ std::to_string(errVal) + "radians.\r\n";
					appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
					// throw warning
				}

				// modify the phases of the current waveform such that they reach the 0 phase after the waveform.
				for (int v = 0; v < allWvInfo1[wfCount + 1].signalNum; v++) 
				{
					allWvInfo1[wfCount + 1].signals[v].initPhase = 2 * PI - allWvInfo1[wfCount + 1].signals[v].finPhase;
				}
				// need to reset this so that on future variables things still get calculated in the right order. This doesn't actually lead to case -3 for this variable.
				paramNum = -3;
				break;
			}
			case -3: 
			{
				// Case -3 is the case initially set by a correction wavefunction set to correct the /next/ (as opposed to the previous) waveform. However, at the
				// time that this gets called, the wavefunction can't get the correct time yet because the next waveform hasn't been written. This location should
				// never be reached.
				MessageBox(NULL, "Bad location reached: -3 case of varyParam().", NULL, MB_OK);
				return -1;
				break;
			}

			case -2:
			{
				for (int o = 0; o < allWvInfo1[wfCount - 1].signalNum; o++) 
				{
					initPhases.push_back(allWvInfo1[wfCount].signals[o].initPhase);
				}
				for (int o = 0; o < allWvInfo2[wfCount - 1].signalNum; o++) 
				{
					initPhases.push_back(allWvInfo2[wfCount].signals[o].initPhase);
				}
				// this is the -1 option for time management control when the previous waveform is being varied. 

				double errVal = myMath::calculateCorrectionTime(allWvInfo1[wfCount], allWvInfo2[wfCount], initPhases, "after");
				if (errVal == -1)
				{
					MessageBox(0, "ERROR: Correction waveform was not able to match phases.", 0, 0);
					// throw error
					return -1029;
				}
				else if (errVal != 0)
				{
					std::string errMsg = "WARNING: Correction waveform was not able to match phases very well. The total phase mismatch was "
						+ std::to_string(errVal) + "radians.\r\n";
					appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
					// throw warning
				}
				break;
			}
			case -1:
			{
				// This is the -1 option for phase control when the previous waveform is being varied.
				for (int i = 0; i < allWvInfo1[wfCount].signalNum; i++) 
				{
					// check if this is supposed to be overwritten during variable runs.
					for (int j = 0; j < allWvInfo1[wfCount].varTypes.size(); j++) 
					{
						if (allWvInfo1[wfCount].varTypes[j] == -1) 
						{
							if (i + 1 > allWvInfo1[wfCount - 1].signalNum) 
							{

								char tempChar[300];
								sprintf_s(tempChar, "ERROR: You are trying to copy the phase of the %ith signal of the previous waveform, but the previous "
									"waveform only had %i signals!\n)", i + 1, allWvInfo1[wfCount - 1].signalNum);

								MessageBox(NULL, tempChar, NULL, MB_OK);
								return -2767;
							}
							allWvInfo1[wfCount].signals[i].initPhase = allWvInfo1[wfCount - 1].signals[i].finPhase;
						}
					}
				}
				for (int i = 0; i < allWvInfo2[wfCount].signalNum; i++) 
				{
					// need to check both waveforms at once on this specifically since this information gets used by time-correction waveforms, and the time-correction
					// procedure for x will get called before y changes its initial phase. This is not very efficient currently since this will cause this phase
					// information to get written twice.
					for (int j = 0; j < allWvInfo2[wfCount].varTypes.size(); j++) 
					{
						if (allWvInfo2[wfCount].varTypes[j] == -1) 
						{
							if (i + 1 > allWvInfo2[wfCount - 1].signalNum) 
							{
								char tempChar[300];
								sprintf_s(tempChar, "ERROR: You are trying to copy the phase of the %ith signal of the previous waveform, but the previous "
									"waveform only had %i signals!\n)", i + 1, allWvInfo2[wfCount - 1].signalNum);
								MessageBox(NULL, tempChar, NULL, MB_OK);
								return -2767;
							}
							allWvInfo2[wfCount].signals[i].initPhase = allWvInfo2[wfCount - 1].signals[i].finPhase;
						}
					}
				}

				break;
			}
			case 1:
			{
				if (paramVal < 0)
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				allWvInfo1[wfCount].signals[0].freqInit = paramVal;
				break;
			}
			case 2:
			{
				if (paramVal < 0)
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				allWvInfo1[wfCount].signals[0].freqFin = paramVal;
				break;
			}
			case 3:
			{
				if (paramVal < 0)
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				allWvInfo1[wfCount].signals[0].initPower = paramVal;
				break;
			}
			case 4:
			{
				if (paramVal < 0) 
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				allWvInfo1[wfCount].signals[0].finPower = paramVal;
				break;
			}
			case 5:
			{
				allWvInfo1[wfCount].signals[0].initPhase = paramVal;
				break;
			}
			case 6:
			{
				if (paramVal < 0) 
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				allWvInfo1[wfCount].signals[1].freqInit = paramVal;
				break;
			}
			case 7:
			{
				if (paramVal < 0) 
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				allWvInfo1[wfCount].signals[1].freqFin = paramVal;
				break;
			}
			case 8:
			{
				if (paramVal < 0) 
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				allWvInfo1[wfCount].signals[1].initPower = paramVal;
				break;
			}
			case 9:
			{
				if (paramVal < 0) 
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				allWvInfo1[wfCount].signals[1].finPower = paramVal;
				break;
			}
			case 10:
			{
				allWvInfo1[wfCount].signals[1].initPhase = paramVal;
				break;
			}
			case 11:
			{
				if (paramVal < 0) 
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				allWvInfo1[wfCount].signals[2].freqInit = paramVal;
				break;
			}
			case 12:
			{
				if (paramVal < 0) 
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				allWvInfo1[wfCount].signals[2].freqFin = paramVal;
				break;
			}
			case 13:
			{
				if (paramVal < 0) 
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				allWvInfo1[wfCount].signals[2].initPower = paramVal;
				break;
			}
			case 14:
			{
				if (paramVal < 0) 
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				allWvInfo1[wfCount].signals[2].finPower = paramVal;
				break;
			}
			case 15:
			{
				allWvInfo1[wfCount].signals[2].initPhase = paramVal;
				break;
			}
			case 16:
			{
				if (paramVal < 0) 
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				allWvInfo1[wfCount].signals[3].freqInit = paramVal;
				break;
			}
			case 17:
			{
				if (paramVal < 0) 
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				allWvInfo1[wfCount].signals[3].freqFin = paramVal;
				break;
			}
			case 18:
			{
				if (paramVal < 0) 
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				allWvInfo1[wfCount].signals[3].initPower = paramVal;
				break;
			}
			case 19:
			{
				if (paramVal < 0) 
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				allWvInfo1[wfCount].signals[3].finPower = paramVal;
				break;
			}
			case 20:
			{
				allWvInfo1[wfCount].signals[3].initPhase = paramVal;
				break;
			}
			case 21:
			{
				if (paramVal < 0) 
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				allWvInfo1[wfCount].signals[4].freqInit = paramVal;
				break;
			}
			case 22:
			{
				if (paramVal < 0) 
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				allWvInfo1[wfCount].signals[4].freqFin = paramVal;
				break;
			}
			case 23:
			{
				if (paramVal < 0) 
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				allWvInfo1[wfCount].signals[4].initPower = paramVal;
				break;
			}
			case 24:
			{
				if (paramVal < 0) 
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				allWvInfo1[wfCount].signals[4].finPower = paramVal;
				break;
			}
			case 25:
			{
				allWvInfo1[wfCount].signals[4].initPhase = paramVal;
				break;
			}
			case 26:
			{
				if (paramVal < 0) 
				{
					MessageBox(NULL, "ERROR: Attempted to set negative waveform parameter. Don't do that.", NULL, MB_OK);
					return -1;
				}
				// CONVERT from milliseconds...
				allWvInfo1[wfCount].time = paramVal * 0.001;
				// Changing the time changes the sample number, so recalculate that.
				allWvInfo1[wfCount].sampleNum = myNIAWG::script::waveformSizeCalc(allWvInfo1[wfCount]);
				break;
			}
		}
		return 0;
	}

	/*
	 * This function is the procedure for checking NIAWG errors while in the createXYScript function. It returns true if an error is found, false otherwise. It just
	 * posts a message to the main window if an error is found.
	 */
	bool NIAWG_CheckScriptError(int err, bool isDefaultError, bool isThreaded)
	{
		if (err < 0)
		{
			ViChar errorMsg[256];

			if (!SAFEMODE)
			{
				niFgen_ErrorHandler(eSessionHandle, eError, errorMsg);
			}
			if (isThreaded)
			{
				std::string errMsg = "ERROR! (detected in NIAWG_CheckScriptError())\r\n";
				postMyString(eErrorTextMessageID, errMsg);
				postMyString(eErrorTextMessageID, errorMsg);
				PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
			}
			else
			{
				if (isDefaultError)
				{
					MessageBox(NULL, errorMsg, 0, MB_OK | MB_ICONERROR);
				}
				else
				{
					appendText(errorMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
				}
			}
			return true;
		}
		else
		{
			return false;
		}
	}
	/*
	 * This function looks for NIAWG errors while in either of the windows procedures or in the main function. It returns true if an error is found, false
	 * otherwise. It attempts to get the NIWAG just outputting default waveforms again.
	 */
	bool NIAWG_CheckWindowsError(int err)
	{
		if (err < 0)
		{
			// todo: Handle errors differently here.
			ViChar* errMsg;
			//niFgen_ErrorHandler(eSessionHandle, error, errMsg);
			int errMsgSize = 0;
			if (!SAFEMODE)
			{
				errMsgSize = niFgen_GetError(eSessionHandle, VI_NULL, 0, VI_NULL);
			}
			errMsg = (ViChar *)malloc(sizeof(ViChar) * errMsgSize);
			if (!SAFEMODE)
			{
				niFgen_GetError(eSessionHandle, &eError, errMsgSize, errMsg);
			}
			std::string err(errMsg);
			free(errMsg);
			myNIAWG::myNIAWG_DoubleErrorChecker(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_FALSE));
			// Officially stop trying to generate anything.
			myNIAWG::myNIAWG_DoubleErrorChecker(niFgen_AbortGeneration(eSessionHandle));
			
			appendText("EXITED WITH ERROR! (Seen in function NIAWG_CheckWindowsError)\r\n", IDC_SYSTEM_STATUS_TEXT, eMainWindowHandle);
			appendText("EXITED WITH ERROR! (Seen in function NIAWG_CheckWindowsError)\r\n", IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
			appendText("NIAWG ERROR: (" + err + ")\r\n", IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
			std::string msgStr = "EXITED WITH ERROR! Passively Outputting Default Waveform.";
			SetWindowText(eColoredStatusEdit, msgStr.c_str());
			eGenStatusColor = "R";
			RedrawWindow(eColorBox, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			RedrawWindow(eColoredStatusEdit, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			myAgilent::agilentDefault();
			if (!SAFEMODE)
			{
				// clear the memory
				myNIAWG::myNIAWG_DoubleErrorChecker(niFgen_ClearArbMemory(eSessionHandle));
			}
			ViInt32 waveID;
			if (eCurrentOrientation == "Horizontal")
			{
				if (!SAFEMODE)
				{
					// create waveform (necessary?)
					myNIAWG::myNIAWG_DoubleErrorChecker(niFgen_CreateWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigMixedSize, eDefault_hConfigMixedWaveform, &waveID));
					// allocate waveform into the device memory
					myNIAWG::myNIAWG_DoubleErrorChecker(niFgen_AllocateNamedWaveform(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigWaveformName.c_str(), eDefault_hConfigMixedSize / 2));
					// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
					myNIAWG::myNIAWG_DoubleErrorChecker(niFgen_WriteNamedWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigWaveformName.c_str(), eDefault_hConfigMixedSize, eDefault_hConfigMixedWaveform));
					// rewrite the script. default_hConfigScript should still be valid.
					myNIAWG::myNIAWG_DoubleErrorChecker(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigScript));
					// start generic waveform to maintain power output to AOM.
					myNIAWG::myNIAWG_DoubleErrorChecker(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE));
					myNIAWG::myNIAWG_DoubleErrorChecker(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultHConfigScript"));
				}
				eCurrentScript = "DefaultHConfigScript";

			}
			else if (eCurrentOrientation == "Vertical")
			{
				if (!SAFEMODE)
				{
					// create waveform (necessary?)
					myNIAWG::myNIAWG_DoubleErrorChecker(niFgen_CreateWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigMixedSize, eDefault_vConfigMixedWaveform, &waveID));
					// allocate waveform into the device memory
					myNIAWG::myNIAWG_DoubleErrorChecker(niFgen_AllocateNamedWaveform(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigWaveformName.c_str(), eDefault_vConfigMixedSize / 2));
					// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
					myNIAWG::myNIAWG_DoubleErrorChecker(niFgen_WriteNamedWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigWaveformName.c_str(), eDefault_vConfigMixedSize, eDefault_vConfigMixedWaveform));
					// rewrite the script. default_hConfigScript should still be valid.
					myNIAWG::myNIAWG_DoubleErrorChecker(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigScript));
					// start generic waveform to maintain power output to AOM.
					myNIAWG::myNIAWG_DoubleErrorChecker(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE));
					myNIAWG::myNIAWG_DoubleErrorChecker(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultVConfigScript"));
				}
				eCurrentScript = "DefaultVConfigScript";
			}
			if (!SAFEMODE)
			{
				// Initiate Generation.
				myNIAWG::myNIAWG_DoubleErrorChecker(niFgen_InitiateGeneration(eSessionHandle));
			}
			appendText("Initialized Default Waveform\r\n", IDC_SYSTEM_STATUS_TEXT, eMainWindowHandle);
			return true;
		}

		return false;
	}

	/*
	 * This function looks for NIAWG errors in the programming thread. It returns true if an error is found, false otherwise. If an error is found, it sends
	 * a series of messages to the main window.
	 */
	bool NIAWG_CheckProgrammingError(int err)
	{
		if (err < 0)
		{
			ViChar* errMsg;
			//niFgen_ErrorHandler(eSessionHandle, error, errMsg);
			int errMsgSize = 0;
			if (!SAFEMODE)
			{
				errMsgSize = niFgen_GetError(eSessionHandle, VI_NULL, 0, VI_NULL);
			}
			errMsg = (ViChar *)malloc(sizeof(ViChar) * errMsgSize);
			if (!SAFEMODE)
			{
				niFgen_GetError(eSessionHandle, &eError, errMsgSize, errMsg);
			}
			std::string err(errMsg);
			free(errMsg);
			postMyString(eStatusTextMessageID, "EXITED WITH ERROR! (Seen in function NIAWG_CheckProgrammingError)\r\n");
			postMyString(eErrorTextMessageID, "EXITED WITH ERROR! (Seen in function NIAWG_CheckProgrammingError)\r\n");
			postMyString(eErrorTextMessageID, "NIAWG ERROR : " + err + "\r\n");
			postMyString(eStatusTextMessageID, "Initialized Default Waveform\r\n");
			PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
			return true;
		}

		return false;
	}

	/*
	 * This functino is called while the NIAWG_CheckWindowsError function is called to catch double-errors when the NIAWG_CheckWindowsError function attempts 
	 * to get the NIAWG outputting the default. It always returns 0.
	 */
	int myNIAWG_DoubleErrorChecker(int err)
	{
		if (err != 0)
		{
			std::string errMsg;
			if (err < 0)
			{
				errMsg = "WARNING! The NIAWG encountered an error and was not able to restart smoothly. It is (probably) not outputting anything. You may "
						 "consider restarting the code. Inside the restart area, NIAWG function returned " + std::to_string(err);
			}
			else
			{
				errMsg = "WARNING! The NIAWG threw a warning while trying to restart after an error. Inside the restart area, NIAWG function "
					"returned " + std::to_string(err);
			}
			MessageBox(0, errMsg.c_str(), 0, 0);
		}
		return 0;
	}

	/*
	 * This function is called when creating the default waveform. It returns true if an error and false otherwise. It throws message boxes when errors are 
	 * encountered. It always returns zero.
	 */
	int NIAWG_CheckDefaultError(int err)
	{
		if (err != 0)
		{
			std::string errMsg;
			if (err < 0)
			{
				errMsg = "WARNING! The NIAWG encountered an error while the default waveforms was being reprogrammed. It is (probably) not outputting anything."
						 "It will not output anything until you correct the error and reload the default waveforms. Inside the restart area, NIAWG function "
						 "returned " + std::to_string(err);
				return -1;
			}
			else
			{
				errMsg = "WARNING! The NIAWG threw a warning while the default waveforms was being reprogrammed. Inside the restart area, NIAWG function "
						 "returned " + std::to_string(err);
				MessageBox(0, errMsg.c_str(), 0, 0);
				return 1;
			}
		}
		return 0;
	}

	/*
	* various functions for interpreting the inputted script.
	*/
	namespace handleInput
	{
		/**
		* getInputType takes in a string, the string representation of the input type, and returns a number directly corresponding to that input type.
		* @param inputType is the string to be analyzed
		* @return int is the number corresponding to the input type.
		*/
		void getInputType(std::string inputType, waveData &wvInfo) 
		{
			// Check against every possible generate input type. 
			if (inputType == "gen 1, const <freq> <amp>; <time> <t manage>" || inputType == "gen 1, const") 
			{
				wvInfo.initType = 0;
				return;
			}
			else if (inputType == "gen 2, const <freq1> <amp1>; <sim for 2nd> <phase (rad)>; <time> <t manage>" || inputType == "gen 2, const") 
			{
				wvInfo.initType = 1;
				return;
			}
			else if (inputType == "gen 3, const <freq1> <amp1>; <sim for 2nd, 3rd> <phase (rad)>; <time> <t manage>" || inputType == "gen 3, const") 
			{
				wvInfo.initType = 2;
				return;
			}
			else if (inputType == "gen 4, const" || inputType == "gen4, const <freq1> <amp1> <phase (rad)>; <sim for 2nd, 3rd, 4th>; <time> <t manage>") 
			{
				wvInfo.initType = 3;
				return;
			}
			else if (inputType == "gen 5, const" || inputType == "gen 5, const <freq1> <amp1> <phase (rad)>; <sim for 2nd, 3rd, 4th, 5th>; <time> <t manage>") 
			{
				wvInfo.initType = 4;
				return;
			}
			else if (inputType == "gen 1, amp ramp <freq> <amp ramp type> <initial amp> <final amp> <phase (rad)>; <time> <t manage>" || inputType == "gen 1, amp ramp") 
			{
				wvInfo.initType = 5;
				return;
			}
			else if (inputType == "gen 2, amp ramp <freq1> <amp1 ramp type> <initial amp1> <final amp1> <phase (rad)>; <sim for 2nd>; <time> <t manage>"
				|| inputType == "gen 2, amp ramp") 
			{
				wvInfo.initType = 6;
				return;
			}
			else if (inputType == "gen 3, amp ramp <freq1> <amp1 ramp type> <initial amp1> <final amp1> <phase (rad)>; <sim for 2nd, 3rd>; <time> <t manage>"
				|| inputType == "gen 3, amp ramp") 
			{
				wvInfo.initType = 7;
				return;
			}
			else if (inputType == "gen 4, amp ramp"
				|| inputType == "gen 4, amp ramp <freq1> <amp1 ramp type> <initial amp1> <final amp1> <phase (rad)>; <sim for 2nd, 3rd, 4th>; <time> <t manage>") 
			{
				wvInfo.initType = 8;
				return;
			}
			else if (inputType == "gen 5, amp ramp"
				|| inputType == "gen 5, amp ramp <freq1> <amp1 ramp type> <initial amp1> <final amp1> <phase (rad)>; <sim for 2nd, 3rd, 4th, 5th>; <time> <t manage>") 
			{
				wvInfo.initType = 9;
				return;
			}
			else if (inputType == "gen 1, freq ramp <freq ramp type> <initial freq> <final freq> <amp> <phase (rad)>; <time> <t manage>"
				|| inputType == "gen 1, freq ramp") 
			{
				wvInfo.initType = 10;
				return;
			}
			else if (inputType == "gen 2, freq ramp <freq1 ramp type> <initial freq1> <final freq1> <amp1> <phase (rad)>; <sim for 2nd>; <time> <t manage>"
				|| inputType == "gen 2, freq ramp") 
			{
				wvInfo.initType = 11;
				return;
			}
			else if (inputType == "gen 3, freq ramp <freq1 ramp type> <initial freq1> <final freq1> <amp1> <phase (rad)>; <sim for 2nd, 3rd>; <time> <t manage>"
				|| inputType == "gen 3, freq ramp") 
			{
				wvInfo.initType = 12;
				return;
			}
			else if (inputType == "gen 4, freq ramp <freq1 ramp type> <initial freq1> <final freq1> <amp1> <phase (rad)>; <sim for 2nd, 3rd, 4th>; <time> <t manage>"
				|| inputType == "gen 4, freq ramp") 
			{
				wvInfo.initType = 13;
				return;
			}
			else if (inputType == "gen 5, freq ramp <freq1 ramp type> <initial freq1> <final freq1> <amp1> <phase (rad)>; <sim for 2nd, 3rd, 4th, 5th>; <time> <t manage>"
				|| inputType == "gen 5, freq ramp") 
			{
				wvInfo.initType = 14;
				return;
			}
			else if (inputType == "gen 1, freq & amp ramp <freq ramp type> <initial freq> <final freq> <amp ramp type> <initial amp> <final amp> <phase (rad)>;"
				"<time> <t manage>" || inputType == "gen 1, freq & amp ramp") 
			{
				wvInfo.initType = 15;
				return;
			}
			else if (inputType == "gen 2, freq & amp ramp <freq1 ramp type> <initial freq1> <final freq1> <amp ramp1 type> <initial ramp1> <final ramp1> <phase "
				"(rad)>; <sim for 2nd>; <time> <t manage>" || inputType == "gen 2, freq & amp ramp") 
			{
				wvInfo.initType = 16;
				return;
			}
			else if (inputType == "gen 3, freq & amp ramp <freq1 ramp type> <initial freq1> <final freq1> <amp ramp1 type> <initial ramp1> <final ramp1> <phase "
				"(rad)>; <sim for 2nd, 3rd>; <time> <t manage>" || inputType == "gen 3, freq & amp ramp") 
			{
				wvInfo.initType = 17;
				return;
			}
			else if (inputType == "gen 4, freq & amp ramp <freq1 ramp type> <initial freq1> <final freq1> <amp ramp1 type> <initial ramp1> <final ramp1> <phase "
				"(rad)>; <sim for 2nd, 3rd, 4th>; <time> <t manage>" || inputType == "gen 4, freq & amp ramp") 
			{
				wvInfo.initType = 18;
				return;
			}
			else if (inputType == "gen 5, freq & amp ramp <freq1 ramp type> <initial freq1> <final freq1> <amp ramp1 type> <initial ramp1> <final ramp1> <phase "
				"(rad)>; <sim for 2nd, 3rd, 4th, 5th>; <time> <t manage>" || inputType == "gen 5, freq & amp ramp") 
			{
				wvInfo.initType = 19;
				return;
			}
			else 
			{
				MessageBox(NULL, "ERROR: code not found in the inputToWaveform for this input.\n\n", NULL, MB_OK);
				return;
			}
		}

		/**
		* WaveformData1 gathers input from the file, and based on the input type, sorts that input into the appropriate structure. It also calculates the number of
		* samples that the waveform will have to contain. This function deals with single waveforms.
		* @param inputCase is the number associated with the input type, obtained from getInputType() above.
		* @param fileName is the name of the file being read, which contains all of the waveform data on the next line.
		* @param size is the size of the waveform in question, to be determined by this function.
		* @param waveInfo is the structure which stores the info being read in this function.
		*/
		int getWvFmData(std::fstream &fileName, waveData &waveInfo) 
		{
			rmWhite(fileName);
			// Initialize the variable counter inside the waveData struct to zero:
			waveInfo.varNum = 0;
			waveInfo.signals.resize(waveInfo.signalNum);

			switch (waveInfo.initType) 
			{
				/// the case for "gen ?, const"
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				{
					for (int i = 0; i < waveInfo.signalNum; i++)
					{
						// set the initial and final values to be equal, and to not use a ramp, unless variable present.
						if (myNIAWG::script::getParamCheckVarConst(waveInfo.signals[i].freqInit, waveInfo.signals[i].freqFin, fileName, waveInfo.varNum,
																   waveInfo.varNames, waveInfo.varTypes, (5 * i) + 1, (5 * i) + 2) 
							!= 0)
						{
							MessageBox(NULL, "getParamCheckVarConst() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
							return -1;
						}
						// Scale the frequencies to be in hertz. (input is MHz)
						waveInfo.signals[i].freqInit *= 1000000.;
						waveInfo.signals[i].freqFin *= 1000000.;
						// Can't be varied for this case type
						waveInfo.signals[i].freqRampType = "nr";
						// set the initial and final values to be equal, and to not use a ramp, unless variable present.
						if (myNIAWG::script::getParamCheckVarConst(waveInfo.signals[i].initPower, waveInfo.signals[i].finPower, fileName, waveInfo.varNum,
																   waveInfo.varNames, waveInfo.varTypes, (5 * i) + 3, (5 * i) + 4)
							!= 0)
						{
							MessageBox(NULL, "getParamCheckVarConst() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
							return -1;
						}
						// Can't be varied
						waveInfo.signals[i].powerRampType = "nr";
						// Get phase, unless varied.
						if (myNIAWG::script::getParamCheckVar(waveInfo.signals[i].initPhase, fileName, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
															  (5 * i) + 5) 
							!= 0)
						{
							MessageBox(NULL, "getParamCheckVar() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
							return -1;
						}
					}
					// Get time, unless varied.
					if (myNIAWG::script::getParamCheckVar(waveInfo.time, fileName, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes, 26) != 0)
					{
						MessageBox(NULL, "getParamCheckVar() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
						return -1;
					}
					// Scale the time to be in seconds. (input is ms)
					waveInfo.time *= 0.001;
					// test if waveInfo.time is an integer multiple of 4 samples
					// convert to picoseconds then test the modulo with 10 ns.
					if ((((long long)round(waveInfo.time * 1e12)) % 10000) != 0)
					{
						MessageBox(NULL, "ERROR: time value entered does not correspond to a multiple of 4 samples on the generater, or a multiple of 10 ns.\n",
								   "ERROR", MB_OK | MB_ICONERROR);
						return -2;
					}

					// get the time management option.
					if (myNIAWG::script::getParamCheckVar(waveInfo.phaseManagementOption, fileName, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes, 27) 
						!= 0)
					{
						MessageBox(NULL, "getParamCheckVar() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
						return -1;
					}
					// pick up the delimeter.
					rmWhite(fileName);
					fileName >> waveInfo.delim;
					break;
				}
				/// The case for "gen ?, amp ramp"
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
				{
					for (int i = 0; i < waveInfo.signalNum; i++)
					{
						// set the initial and final values to be equal, and to not use a ramp.
						if (myNIAWG::script::getParamCheckVarConst(waveInfo.signals[i].freqInit, waveInfo.signals[i].freqFin, fileName, waveInfo.varNum, 
																   waveInfo.varNames, waveInfo.varTypes, (5 * i) + 1, (5 * i) + 2)
							!= 0)
						{
							MessageBox(NULL, "getParamCheckVarConst() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
							return -1;
						}
						// Scale the frequencies to be in hertz.(input is MHz)
						waveInfo.signals[i].freqInit *= 1000000.;
						waveInfo.signals[i].freqFin *= 1000000.;
						// can't be varried.
						waveInfo.signals[i].freqRampType = "nr";
						std::string tempStr;
						fileName >> tempStr;
						std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower);
						waveInfo.signals[i].powerRampType = tempStr;
						if (myNIAWG::script::getParamCheckVar(waveInfo.signals[i].initPower, fileName, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
															  (5 * i) + 3) != 0) 
						{
							MessageBox(NULL, "getParamCheckVar() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
							return -1;
						}
						if (myNIAWG::script::getParamCheckVar(waveInfo.signals[i].finPower, fileName, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes, 
															  (5 * i) + 4) 
							!= 0)
						{
							MessageBox(NULL, "getParamCheckVar() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
							return -1;
						}
						if (myNIAWG::script::getParamCheckVar(waveInfo.signals[i].initPhase, fileName, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes, 
															  (5 * i) + 5) != 0) 
						{
							MessageBox(NULL, "getParamCheckVar() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
							return -1;
						}
					}
					if (myNIAWG::script::getParamCheckVar(waveInfo.time, fileName, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes, 26) != 0) 
					{
						MessageBox(NULL, "getParamCheckVar() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
						return -1;
					}
					// Scale the time to be in seconds. (input is ms)
					waveInfo.time *= 0.001;
					// test if waveInfo.time is an integer multiple of 4 samples
					// convert to picoseconds then test the modulo with 10 ns.
					if ((((long long)round(waveInfo.time * 1e12)) % 10000) != 0) 
					{
						MessageBox(NULL, "ERROR: time value entered does not correspond to a multiple of 4 samples on the generater, or a multiple of 10 ns."
										 "\n", "ERROR", MB_OK | MB_ICONERROR);
						return -2;
					}
					// get the time management option.
					if (myNIAWG::script::getParamCheckVar(waveInfo.phaseManagementOption, fileName, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes, 27) != 0) 
					{
						MessageBox(NULL, "getParamCheckVar() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
						return -1;
					}
					// pick up the delimeter.
					rmWhite(fileName);
					fileName >> waveInfo.delim;
					break;
				}
				/// The case for "gen ?, freq ramp"
				case 10:
				case 11:
				case 12:
				case 13:
				case 14:
				{
					for (int i = 0; i < waveInfo.signalNum; i++) 
					{
						// get all parameters from the file
						std::string tempStr;
						fileName >> tempStr;
						std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), tolower);
						waveInfo.signals[i].freqRampType = tempStr;
						if (myNIAWG::script::getParamCheckVar(waveInfo.signals[i].freqInit, fileName, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes, 
															  (5 * i) + 1) 
							!= 0) 
						{
							MessageBox(NULL, "getParamCheckVar() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
							return -1;
						}
						if (myNIAWG::script::getParamCheckVar(waveInfo.signals[i].freqFin, fileName, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes, 
															  (5 * i) + 2)
							!= 0) 
						{
							MessageBox(NULL, "getParamCheckVar() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
							return -1;
						}
						// Scale the frequencies to be in hertz.(input is MHz)
						waveInfo.signals[i].freqInit *= 1000000.;
						waveInfo.signals[i].freqFin *= 1000000.;
						// set the initial and final values to be equal, and to not use a ramp.
						if (myNIAWG::script::getParamCheckVarConst(waveInfo.signals[i].initPower, waveInfo.signals[i].finPower, fileName, waveInfo.varNum,
																   waveInfo.varNames, waveInfo.varTypes, (5 * i) + 3, (5 * i) + 4) 
							!= 0) 
						{
							MessageBox(NULL, "getParamCheckVarConst() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
							return -1;
						}
						waveInfo.signals[i].powerRampType = "nr";
						if (myNIAWG::script::getParamCheckVar(waveInfo.signals[i].initPhase, fileName, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes, 
															  (5 * i) + 5) != 0) 
						{
							MessageBox(NULL, "getParamCheckVar() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
							return -1;
						}
					}
					if (myNIAWG::script::getParamCheckVar(waveInfo.time, fileName, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes, 26) != 0) 
					{
						MessageBox(NULL, "getParamCheckVar() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
						getchar();
						return -1;
					}
					// Scale the time to be in seconds. (input is ms)
					waveInfo.time *= 0.001;
					// test if waveInfo.time is an integer multiple of 4 samples
					// convert to picoseconds then test the modulo.
					if ((((long long)round(waveInfo.time * 1e12)) % 10000) != 0) 
					{
						MessageBox(NULL, "ERROR: time value entered does not correspond to a multiple of 4 samples on the generater, or a multiple of 10 ns.\n", "ERROR", MB_OK | MB_ICONERROR);
						return -2;
					}
					// get the time management option.
					if (myNIAWG::script::getParamCheckVar(waveInfo.phaseManagementOption, fileName, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes, 27) 
						!= 0) 
					{
						MessageBox(NULL, "getParamCheckVar() threw an error!", "ERROR", MB_OK | MB_ICONERROR);					getchar();
						return -1;
					}
					// pick up the delimeter.
					rmWhite(fileName);
					fileName >> waveInfo.delim;
					break;
				}
					/// The case for "gen ?, freq & amp ramp"
				case 15:
				case 16:
				case 17:
				case 18:
				case 19:
				{
					for (int i = 0; i < waveInfo.signalNum; i++) 
					{
						// get all parameters from the file
						std::string tempStr;
						fileName >> tempStr;
						std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower);
						waveInfo.signals[i].freqRampType = tempStr;
						if (myNIAWG::script::getParamCheckVar(waveInfo.signals[i].freqInit, fileName, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes, 
															  (5 * i) + 1) 
							!= 0) 
						{
							MessageBox(NULL, "getParamCheckVar() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
							return -1;
						}
						if (myNIAWG::script::getParamCheckVar(waveInfo.signals[i].freqFin, fileName, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes, 
															  (5 * i) + 2) 
							!= 0) 
						{
							MessageBox(NULL, "getParamCheckVar() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
							return -1;
						}
						// Scale the frequencies to be in hertz.(input is MHz)
						waveInfo.signals[i].freqInit *= 1000000.;
						waveInfo.signals[i].freqFin *= 1000000.;

						// get all parameters from the file
						fileName >> tempStr;
						std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower);
						waveInfo.signals[i].powerRampType = tempStr;
						if (myNIAWG::script::getParamCheckVar(waveInfo.signals[i].initPower, fileName, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes, 
															  (5 * i) + 3) 
							!= 0) 
						{
							MessageBox(NULL, "getParamCheckVar() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
							return -1;
						}
						if (myNIAWG::script::getParamCheckVar(waveInfo.signals[i].finPower, fileName, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes, 
															  (5 * i) + 4) 
							!= 0) 
						{
							MessageBox(NULL, "getParamCheckVar() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
							return -1;
						}
						if (myNIAWG::script::getParamCheckVar(waveInfo.signals[i].initPhase, fileName, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes, 
															  (5 * i) + 5) 
							!= 0) 
						{
							MessageBox(NULL, "getParamCheckVar() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
							return -1;
						}
						//?????????????????????????????????????????????????????????????????????????????
						//waveInfo.signals.push_back(waveInfo.signals[i]); // Why is this here???????????
						//?????????????????????????????????????????????????????????????????????????????
					}
					if (myNIAWG::script::getParamCheckVar(waveInfo.time, fileName, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes, 26) != 0) 
					{
						MessageBox(NULL, "getParamCheckVar() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
						return -1;
					}
					// Scale the time to be in seconds. (input is ms)
					waveInfo.time *= 0.001;
					// test if waveInfo.time is an integer multiple of 4 samples
					// convert to picoseconds then test the modulo.
					if ((((long long)round(waveInfo.time * 1e12)) % 10000) != 0) 
					{
						MessageBox(NULL, "ERROR: time value entered does not correspond to a multiple of 4 samples on the generater, or a multiple of 10 ns."
										 "\n", "ERROR", MB_OK | MB_ICONERROR);
						return -2;
					}
					// get the time management option.
					if (myNIAWG::script::getParamCheckVar(waveInfo.phaseManagementOption, fileName, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes, 27) 
						!= 0) 
					{
						MessageBox(NULL, "getParamCheckVar() threw an error!", "ERROR", MB_OK | MB_ICONERROR);
						return -1;
					}
					// pick up the delimeter.
					rmWhite(fileName);
					fileName >> waveInfo.delim;
					break;
				}
			}
			// Also get the size (all cases)
			waveInfo.sampleNum = myNIAWG::script::waveformSizeCalc(waveInfo);
			return 0;
		}

		/**
		* This function works with input data including only one signal.
		* This important function takes in the input parameters and from those creates the waveform data array. It is inside this function that the waveform
		* "library" functionality is realized. The function checks a list of strings to see if the waveform has previously been generated, and if it has, it reads
		* the waveform data from a binary file. If it hasn't been created before, it writes a file containing all of the waveform data.
		* Appended to the end of the waveform data files is the final phase of each of the signals involved in the file. This must be stripped off of the voltage
		* data that populates the rest of the file as it's being read, and must be appended to the voltage data before it is written to a new file.
		*
		* @return void This function does not return anything currently. This may be changed at some point to include error information in the return value.
		*
		* @param tempWaveform this is the array that will carry the waveform information out of the function into the rest of the program to be written to the AWG.
		*
		* @param readData this is the array that carries the information as it is first written / read from the file, and is slightly longer than waveformData to
		* include the phase data.
		*
		* @param waveInfo this is the parameter that carries the basic information about the waveform, and is used in the writing of the waveforms name (for the
		* library) and for writing the raw data if needed.
		*
		* @param size this is the number of samples for the waveform, calculated previously. This is needed to know how many data points to read into the data
		* arrays.
		*
		* @param libWaveformArray this is an array of vectors which contain strings. Each vector corresponds to each of the input cases (hence size 20), and each
		* string in the vector corresponds to the name of some waveform which has previously been written. This list of previously written waveforms is used to
		* tell if a waveform has been written before.
		*
		* @param fileOpened this carries information that tells if the file correspoinding to the inputCase in question has been opened, and the libWaveformArray
		* strings read, before. If it hasn't the code knows to open the file and read in the strings for the first time.
		*/
		int waveformGen(ViReal64 * & tempWaveform, ViReal64 * & readData, waveData & waveInfo, long int size, std::vector<std::string>(&libWaveformArray)[20], 
						bool& fileOpened) 
		{
			// structures that hold a time stamp
			unsigned long long time1, time2;
			// the number of seconds
			std::string waveformFileSpecs, tempStr, waveformFileName;
			std::ifstream waveformFileRead, libNameFileRead;
			std::ofstream waveformFileWrite, libNameFileWrite;
			std::fstream libNameFile;

			// Construct the name of the raw data file from the parameters for the waveform. This can be a pretty long name, but that's okay because it's just text
			// in a file at the end.
			for (int i = 0; i < waveInfo.signalNum; i++) 
			{
				waveformFileSpecs += (std::to_string(waveInfo.signals[i].freqInit) + " " + std::to_string(waveInfo.signals[i].freqFin) + " " 
									 + waveInfo.signals[i].freqRampType + " " + std::to_string(waveInfo.signals[i].initPower) + " " 
									 + std::to_string(waveInfo.signals[i].finPower) + " " + waveInfo.signals[i].powerRampType + " " 
									 + std::to_string(waveInfo.signals[i].initPhase) + ", ");
			}
			waveformFileSpecs += (std::to_string(waveInfo.time * 1000.0) + "; ");

			// if the file containing the list of waveforms hasn't already been opened...
			if (fileOpened == false) 
			{
				// open the file. It's written in binary. std::ios::app is for writing new waveforms to the end of the file without replacing already existing
				// waveforms.
				std::string libNameFilePath = LIB_PATH + WAVEFORM_TYPE_FOLDERS[waveInfo.initType] + WAVEFORM_NAME_FILES[waveInfo.initType];
				libNameFile.open(libNameFilePath.c_str(), std::ios::binary | std::ios::in);
				// Make sure that the file opened correctly.
			
				if (!libNameFile.is_open())
				{
					MessageBox(NULL, "ERROR: waveform library file did not open correctly.\n", NULL, MB_OK);
					return -1;
				}

				// read all of the waveforms into libWaveformArray
				int j = 0;
				// if not empty, the first line will just have a newline on it, so there is no harm in getting rid of it.
				std::getline(libNameFile, tempStr, '\n');

				while (!libNameFile.eof()) 
				{
					// read waveform names...
					std::getline(libNameFile, tempStr, '\n');
					// put them into the array...
					libWaveformArray[waveInfo.initType].push_back(tempStr);
					// next.
					j++;
				}
				// save the fileOpened value reflecting it's new status.
				fileOpened = true;
				// Close the file.
				libNameFile.close();
			}
			// Start timer
			time1 = GetTickCount64();

			// Loop over all previously recorded files
			for (unsigned int i = 0; i < libWaveformArray[waveInfo.initType].size(); i++) 
			{
				// if you find this waveform to have already been written...
				if (libWaveformArray[waveInfo.initType][i] == waveformFileSpecs) 
				{
					// Construct the file address
					std::string waveFileReadName = LIB_PATH + WAVEFORM_TYPE_FOLDERS[waveInfo.initType] + std::to_string(waveInfo.initType) + "_"
												 + std::to_string(i) + ".txt";
					// open the file
					waveformFileRead.open(waveFileReadName, std::ios::binary | std::ios::in);
					// read the file. size + signalnum because of the phase data on the end.
					waveformFileRead.read((char *)readData, (size + waveInfo.signalNum) * sizeof(ViReal64));
					// grab the phase data off of the end.
					for (int i = 0; i < waveInfo.signalNum; i++) 
					{
						waveInfo.signals[i].finPhase = readData[size + i];
					}
					// put the relevant voltage data into a the new array.
					std::copy_n(readData, size, tempWaveform);
					// close file.
					waveformFileRead.close();
					if (eOutputReadStatus == true) 
					{
						char processTimeMsg[200];
						time2 = GetTickCount64();
						double ellapsedTime = (time2 - time1) / 1000.0;
						sprintf_s(processTimeMsg, "Finished Reading Waveform. Ellapsed Time: %.3f seconds.\r\n", ellapsedTime);
						appendText(processTimeMsg, IDC_SYSTEM_DEBUG_TEXT, eMainWindowHandle);
					}
					// if the file got read, I don't need to do any writing, so go ahead and return.
					return 0;
				}
			}

			// if the code reaches this point, it could not find a file to read, and so will now create the data from scratch and write it. 
			waveformFileName = LIB_PATH + WAVEFORM_TYPE_FOLDERS[waveInfo.initType] + std::to_string(waveInfo.initType) + "_"
							   + std::to_string(libWaveformArray[waveInfo.initType].size()) + ".txt";
			// open file for writing.
			waveformFileWrite.open(waveformFileName, std::ios::binary | std::ios::out);
			// make sure it opened.
			if (!waveformFileWrite.is_open()) 
			{
				// shouldn't ever happen.
				MessageBox(NULL, "ERROR: Data Storage File could not open. File name is probably too long?\n", NULL, MB_OK);
				return -2;
			}
			else
			{
				// start timer.
				unsigned long long time1 = GetTickCount64();
				// calculate all voltage values and final phases and store them in the readData variable.
				myNIAWG::script::calcWaveData(waveInfo, readData, size);
				// Write the data, with phases, to the write file.
				waveformFileWrite.write((const char *)readData, (size + waveInfo.signalNum) * sizeof(ViReal64));
				// close file.
				waveformFileWrite.close();
				// put the relevant data into another string.
				std::copy_n(readData, size, tempWaveform);
				// write the newly written waveform's name to the library file.
				libNameFile.open(LIB_PATH + WAVEFORM_TYPE_FOLDERS[waveInfo.initType] + WAVEFORM_NAME_FILES[waveInfo.initType], 
								 std::ios::binary | std::ios::out | std::ios::app);
				// make sure the library file opened correctly.
				if (!libNameFile.is_open()) 
				{
					MessageBox(NULL, "ERROR! waveform name file not opening correctly.\n", NULL, MB_OK);
					return -3;
				}
				// add the waveform name to the current list of strings. do it BEFORE adding the newline T.T
				libWaveformArray[waveInfo.initType].push_back(waveformFileSpecs.c_str());
				// put a newline in front of the name so that all of the names don't get put on the same line.
				waveformFileSpecs = "\n" + waveformFileSpecs;
				// write the waveform name.
				libNameFile.write((waveformFileSpecs).c_str(), waveformFileSpecs.size());
				// aaaand close.
				libNameFile.close();
				if (eOutputWriteStatus == true) 
				{
					char processTimeMsg[200];
					time2 = GetTickCount64();
					double ellapsedTime = (time2 - time1) / 1000.0;
					sprintf_s(processTimeMsg, "Finished Writing Waveform. Ellapsed Time: %.3f seconds.\r\n", ellapsedTime);
					appendText(processTimeMsg, IDC_SYSTEM_DEBUG_TEXT, eMainWindowHandle);
				}
			}
			return 0;
		}

		/**
		* This function handles logic input. Most of the function of this is to simply figure out which command the user was going for, and append that to the
		* actual script file.
		*
		* @return int this carries error information.
		*
		* @param verticalFile this is the name of the instructions file for the x-waveforms.
		* @param horizontalFile this is the name of the instructions file for the y-waveforms.
		* @param verticalInput this is the actual command name recently grabbed from the verticalFile.
		* @param horizontalInput this is the actual command name recently grabbed from the horizontalFile.
		* @param scriptString this is the string that contains all of the final script. It gets written to.
		* @param triggerName this is the string version of the trigger.
		*/
		int logic(std::fstream& verticalFile, std::fstream& horizontalFile, std::string verticalInput, std::string horizontalInput, std::string& scriptString, 
				  std::string triggerName) 
		{
			// Declare some parameters.
			int xSampleNum, ySampleNum;
			int xRepeatNum, yRepeatNum;

			// In each of the following cases, the x input string is used first identified, and the the y-input command is checked to make sure that it matches
			// the x input command.

			// Wait commands //
			if (verticalInput == "wait until trigger") 
			{
				if (horizontalInput != "wait until trigger") 
				{
					// error message
					MessageBox(NULL, "ERROR: logical input commands must match, and they don't!\n\n", NULL, MB_OK);
					return -4;
				}
				// Append command to script holder
				scriptString += "wait until " + triggerName + "\n";
			}
			else if ((verticalInput == "wait set # <# of samples to wait>") || (verticalInput == "wait set #"))
			{
				// about to read from file so make sure no random extra newlines before read data...
				rmWhite(verticalFile);
				rmWhite(horizontalFile);
				// grab the # of samples the user wants to wait.
				verticalFile >> xSampleNum;
				horizontalFile >> ySampleNum;
				// make sure they are the same.
				if (xSampleNum != ySampleNum) 
				{
					MessageBox(NULL, "ERROR: sample numbers to wait for must match, and they don't!\n\n", NULL, MB_OK);
					return -5;
				}
				// append command to script holder
				scriptString += "wait " + std::to_string((long long)xSampleNum) + "\n";
			}
			// Repeat commands // 
			else if ((verticalInput == "repeat set # <# of times to repeat>") || (verticalInput == "repeat set #")) 
			{
				if ((horizontalInput != "repeat set # <# of times to repeat>") && (horizontalInput != "repeat set #")) 
				{
					// error message
					MessageBox(NULL, "ERROR: logical input commands must match, and they don't!\n\n", NULL, MB_OK);
					return -6;
				}
				// before reading...
				rmWhite(verticalFile);
				rmWhite(horizontalFile);

				// grab the number of times to repeat the user is going for.
				verticalFile >> xRepeatNum;
				horizontalFile >> yRepeatNum;
				// verify that the numbers match
				if (xRepeatNum != yRepeatNum) 
				{
					MessageBox(NULL, "ERROR: number of times to repeat must match, and they don't!\n\n", NULL, MB_OK);
					return -7;
				}

				// append command to script holder
				scriptString += "repeat " + std::to_string((long long)xRepeatNum) + "\n";

			}
			else if (verticalInput == "repeat until trigger") 
			{
				if (horizontalInput != "repeat until trigger") 
				{
					MessageBox(NULL, "ERROR: logical input commands must match, and they don't!\n\n", NULL, MB_OK);
					return -8;
				}
				// append command to script holder
				scriptString += "repeat until " + triggerName + "\n";
			}
			else if (verticalInput == "repeat forever") 
			{
				// append command to script holder
				if (horizontalInput != "repeat forever") 
				{
					MessageBox(NULL, "ERROR: logical input commands must match, and they don't!\n\n", NULL, MB_OK);
					return -9;
				}
				scriptString += "repeat forever\n";
			}
			else if (verticalInput == "end repeat") 
			{
				if (horizontalInput != "end repeat") 
				{
					MessageBox(NULL, "ERROR: logical input commands must match, and they don't!\n\n", NULL, MB_OK);
					return -10;
				}
				// append command to script holder
				scriptString += "end repeat\n";
			}
			// if-else Commands //
			else if (verticalInput == "if trigger") 
			{
				if (horizontalInput != "if trigger") 
				{
					MessageBox(NULL, "ERROR: logical input commands must match, and they don't!\n\n", NULL, MB_OK);
					return -11;
				}
				// append command to script holder
				scriptString += "if " + triggerName + "\n";
			}
			else if (verticalInput == "else") 
			{
				if (horizontalInput != "else") 
				{
					MessageBox(NULL, "ERROR: logical input commands must match, and they don't!\n\n", NULL, MB_OK);
					return -12;
				}
				// append command to script holder
				scriptString += "else\n";
			}
			else if (verticalInput == "end if") 
			{
				if (horizontalInput != "end if") 
				{
					MessageBox(NULL, "ERROR: logical input commands must match, and they don't!\n\n", NULL, MB_OK);
					return -13;
				}
				// append command to script holder
				scriptString += "end if\n";
			}
			return 0;
		}


		/**
		* This function handles the weird case commands, like ones that reference other waveforms or scripts. They are the special cases.
		*
		* @return int this is for error information.
		*
		* @param verticalFile this is the instructiosn file which carries commands for the x waveforms.
		* @param horizontalFile this is the instructions file which carries commands for the y waveforms.
		* @param verticalInputType this contains the command the user inputted for the x waveforms, which was just read from the verticalFile recently.
		* @param horizontalInputType this contains the command the user inputted for the y waveforms, which was just read from the horizontalFile recently.
		* @param scriptString this carries the final script to be outputted to the AWG.
		* @param triggerName this carries the string version of the trigger name.
		* @param waveCount this integer is the number of waveforms that have been written so far.
		* @param vi is the session handle currently in use for communication with the AWG.
		* @param channelName is the channel name ("0,1" normally) that the signal is currently being put through.
		* @param error is the NI fgen error variable which grabs error messages.
		* @param xWaveformList is the list of all of the previously written x-waveforms.
		* @param yWaveformList is the list of all of the previously written y-waveforms.
		* @param predWaveCount is the number of predefined waves (i.e. referring to a file that contains the waveform parameters) that have been used so far.
		* @param waveListWaveCounts is the array that tells which waveforms (i.e. waveform0, waveform1, the names used for the final script) the predefined
		* waveforms refer to.
		* @param libWaveformArray is the array of vectors to strings that contain all of the names of the waveforms currently in the library.
		* @param fileStatus is the array that contains the information as to whether each library file has been opened yet this run or not.
		*/
		int special(std::fstream &verticalFile, std::fstream &horizontalFile, std::string verticalInputType, std::string horizontalInputType, 
					std::string& scriptString, std::string triggerName,	int &waveCount, ViSession vi, ViConstString channelName, ViStatus error, 
					std::vector<std::string> xWaveformList,	std::vector<std::string> yWaveformList, int &predWaveCount, std::vector<int> waveListWaveCounts,
					std::vector<std::string>(&libWaveformArray)[20], bool(&fileStatus)[20], std::vector<waveData> &allXWaveParam, 
					std::vector<bool> &xWaveVaried,	std::vector<waveData> &allYWaveParam, std::vector<bool> &yWaveVaried, bool isDefault, bool isThreaded, std::string currentCategoryFolder)
		{
			// declare some variables
			std::string verticalExternalVerticalScriptName, verticalExternalHorizontalScriptName, horizontalExternalVerticalScriptName, 
						horizontalExternalHorizontalScriptName, verticalSampleWait, horizontalSampleWait, verticalExternalVerticalWaveformName, 
						verticalExternalHorizontalWaveformName,	horizontalExternalVerticalWaveformName, horizontalExternalHorizontalWaveformName, 
						verticalImportType, horizontalImportType;
			std::fstream externalVerticalWaveformFile, externalHorizontalWaveformFile;

			// Interpreting a predefined scripting file.
			if ((verticalInputType == "predefined script \\n <vertical filename.script> \\n <horizontal filename.script>") || (verticalInputType == "predefined script"))
			{
				// make sure that both scripts refer to the same predefined scripts
				if (horizontalInputType != "predefined script \\n <vertical filename.txt> \\n <horizontal filename.txt>" && horizontalInputType != "predefined script") 
				{
					MessageBox(NULL, "ERROR: logical input commands must match, and they don't!\n\n", NULL, MB_OK);
					return -16;
				}
				// before reading...
				rmWhite(verticalFile);
				rmWhite(horizontalFile);
				// grab the script names from each file
				getline(verticalFile, verticalExternalVerticalScriptName, '\r');
				rmWhite(verticalFile);
				getline(verticalFile, verticalExternalHorizontalScriptName, '\r');
				getline(horizontalFile, horizontalExternalVerticalScriptName, '\r');
				rmWhite(horizontalFile);
				getline(horizontalFile, horizontalExternalHorizontalScriptName, '\r');

				// make sure the names match.
				if (verticalExternalVerticalScriptName != horizontalExternalVerticalScriptName || verticalExternalHorizontalScriptName != horizontalExternalHorizontalScriptName) 
				{
					MessageBox(NULL, "ERROR: external script names must match!\n\n", NULL, MB_OK);
					return -17;
				}
				// open the scripts
				std::fstream externalVerticalScriptFile, externalHorizontalScriptFile;
				externalVerticalScriptFile.open(currentCategoryFolder + "\\" + verticalExternalVerticalScriptName);
				if (!externalVerticalScriptFile.is_open())
				{
					MessageBox(0, ("ERROR: external vertical script file: " + verticalExternalVerticalScriptName + " could not be opened! make sure it exists in the current category folder: " + currentCategoryFolder).c_str(), 0, 0);
					return -1;
				}
				externalHorizontalScriptFile.open(currentCategoryFolder + "\\" + verticalExternalHorizontalScriptName);
				if (!externalVerticalScriptFile.is_open())
				{
					MessageBox(0, ("ERROR: external horizontal script file: " + verticalExternalHorizontalScriptName + " could not be opened! make sure it exists in the current category folder: " + currentCategoryFolder).c_str(), 0, 0);
					return -1;
				}
				// recursively call the createXYScript function. This will go through the new script files, write all of their commands to the same script, write
				// waveforms to the AWG, and eventually make it's way back here.
				myNIAWG::createXYScript(externalVerticalScriptFile, externalHorizontalScriptFile, scriptString, triggerName, waveCount, vi, channelName, error, xWaveformList, 
										yWaveformList, predWaveCount, waveListWaveCounts, libWaveformArray, fileStatus, allXWaveParam, xWaveVaried, 
										allYWaveParam, yWaveVaried, isDefault, isThreaded, currentCategoryFolder);

				return 0;
			}
			// work with marker events
			else if (verticalInputType == "create marker event <samples after previous waveform to wait>" || verticalInputType == "create marker event")
			{
				// Make sure the commands match
				if (horizontalInputType != "create marker event <samples after previous waveform to wait>" && horizontalInputType != "create marker event")
				{
					MessageBox(NULL, "ERROR: logical input commands must match, and they don't!\n\n", NULL, MB_OK);
					return -16;
				}
				// before reading...
				rmWhite(verticalFile);
				rmWhite(horizontalFile);

				// get the timing information from the file.
				verticalFile >> verticalSampleWait;
				horizontalFile >> horizontalSampleWait;
				// remove previous newline to put this command on the same line as a generate command, as it needs to be for the final script.
				scriptString.pop_back();
				// append script
				scriptString += " marker0 (" + verticalSampleWait + ")\n";
				return 0;
			}
			// work with predefined waveforms (mini-scripts)
			else if ((verticalInputType == "predefined waveform" )
					|| (verticalInputType == "predefined waveform <x filename.txt> <y filename.txt> <waveform type>"))
			{
				// make sure the commands match
				if ((horizontalInputType != "predefined waveform") && (horizontalInputType != "predefined waveform <x filename.txt> <y filename.txt>"))
				{
					MessageBox(NULL, "ERROR: logical input commands must match, and they don't!\n\n", NULL, MB_OK);
					return -18;
				}
				// before reading...
				rmWhite(verticalFile);
				rmWhite(horizontalFile);
				// grab the waveform names and command types
				verticalFile >> verticalExternalVerticalWaveformName;
				verticalFile >> verticalExternalHorizontalWaveformName;
				horizontalFile >> horizontalExternalVerticalWaveformName;
				horizontalFile >> horizontalExternalHorizontalWaveformName;
				// make sure names match
				if (verticalExternalVerticalWaveformName != horizontalExternalVerticalWaveformName || verticalExternalHorizontalWaveformName != horizontalExternalHorizontalWaveformName) 
				{
					MessageBox(NULL, "ERROR: external waveform names must match!\n\n", NULL, MB_OK);
					return -19;
				}
				if (verticalImportType != horizontalImportType) 
				{
					MessageBox(NULL, "ERROR: import types of the x and y waveforms must match!\n\n", NULL, MB_OK);
					return -20;
				}
				// check if this waveform has already been used. If it has, then just refer to that waveform by name in the script.
				for (int i = 0; i < predWaveCount; i++) 
				{
					if (verticalExternalVerticalWaveformName == xWaveformList[i]) 
					{
						if (verticalExternalHorizontalWaveformName == yWaveformList[i]) 
						{
							scriptString += "generate Waveform" + std::to_string(waveListWaveCounts[i]) + "\n";
							return 0;
						}
					}
				}

				// add the name of the waveform to the list.
				xWaveformList.push_back(verticalExternalVerticalWaveformName);
				yWaveformList.push_back(verticalExternalHorizontalWaveformName);
				// record the waveform number that corresponds to this new waveform.
				waveListWaveCounts.push_back(waveCount);
				// incremend the number of predefined waves that have been used.
				predWaveCount++;

				// open the waveform files
				externalVerticalWaveformFile.open(verticalExternalVerticalWaveformName);
				externalHorizontalWaveformFile.open(verticalExternalHorizontalWaveformName);

				// createXYScript here works in the same way as it does for longer scripts. The effect here is simply to read the one waveform into the same script 
				// file.
				myNIAWG::createXYScript(externalVerticalWaveformFile, externalHorizontalWaveformFile, scriptString, triggerName, waveCount, vi, channelName, error, xWaveformList, 
										yWaveformList, predWaveCount, waveListWaveCounts, libWaveformArray, fileStatus, allXWaveParam, xWaveVaried, 
										allYWaveParam, yWaveVaried, isDefault, isThreaded, currentCategoryFolder);
				return 0;
			}
			else
			{
				MessageBox(NULL, "ERROR: no special command found", NULL, MB_OK);
				return -21;
			}
		}
	}

	/**
	* This namespace includes all of the relevant functions for generating waveforms.
	*/
	namespace script
	{
		/**
		* This function takes in the data for a single waveform and calculates all if the waveform's data points, and returns a pointer to an array containing
		* these data points.
		*
		* @return ViReal64* this is the pointer to the array that contains all of the now calculated data points.
		*
		* @param inputData is the data structure that contains the general parameters of the wavefunction.
		* @param readData is the array which will hold all of the waveform and phase data.
		* @param waveformSize is the size of the waveform to be written in number of samples.
		*/
		ViReal64* calcWaveData(waveData& inputData, ViReal64*& readData, long int waveformSize)
		{
			// Declarations
			std::vector<double> powerPos, freqRampPos, phasePos(inputData.signalNum);
			std::vector<double*> powerRampFileData;
			std::vector<double*> freqRampFileData;
			std::fstream powerRampFile, freqRampFile;
			std::string tempStr;

			/// deal with ramp calibration files. check all signals for files and read if yes.
			for (int j = 0; j < inputData.signalNum; j++)
			{
				// create spots for the ramp positions.
				powerPos.push_back(0);
				freqRampPos.push_back(0);

				// If the ramp type isn't a standard command...
				if (inputData.signals[j].powerRampType != "lin" && inputData.signals[j].powerRampType != "nr" && inputData.signals[j].powerRampType != "tanh")
				{
					powerRampFileData.push_back(new double[waveformSize]);
					// try to open it as a file
					powerRampFile.open(inputData.signals[j].powerRampType);
					// if successful....
					if (powerRampFile.is_open())
					{
						int i = 0;
						// load the data in
						while (!powerRampFile.eof())
						{
							powerRampFile >> powerRampFileData[j][i];
							i++;
						}
						// error message for bad size (powerRampFile.eof() reached too early or too late).
						if (i != waveformSize + 1)
						{
							char tempChar[300];
							sprintf_s(tempChar, "ERROR: file not the correct size?\nSize of upload is %i; size of file is %i ", i, waveformSize);
							MessageBox(NULL, tempChar, NULL, MB_OK);
							return NULL;
						}
						// close the file.
						powerRampFile.close();
					}
					else
					{
						std::string errMsg = "ERROR: ramp type " + std::string(inputData.signals[j].powerRampType) 
											 + " is unrecognized. If this is a file name, make sure the file exists and is in the project folder. ";
						MessageBox(NULL, errMsg.c_str(), NULL, MB_OK);
						return NULL;
					}
				}
				// If the ramp type isn't a standard command...
				if (inputData.signals[j].freqRampType != "lin" && inputData.signals[j].freqRampType != "nr" && inputData.signals[j].freqRampType != "tanh")
				{
					// try to open
					freqRampFileData.push_back(new double[waveformSize]);

					freqRampFile.open(inputData.signals[j].freqRampType, std::ios::in);
					// if open
					if (freqRampFile.is_open())
					{
						int i = 0;
						while (!freqRampFile.eof())
						{
							freqRampFile >> freqRampFileData[j][i];
							i++;
						}
						// error message for bad size (powerRampFile.eof() reached too early or too late).
						if (i != waveformSize + 1)
						{
							char tempChar[300];
							sprintf_s(tempChar, "ERROR: file not the correct size?\nSize of upload is %i; size of file is %i", i, waveformSize);
							MessageBox(NULL, tempChar, NULL, MB_OK);
							return NULL;
						}
						// close file
						freqRampFile.close();
					}
					else
					{
						std::string errMsg = "ERROR: ramp type " + inputData.signals[j].freqRampType +" is unrecognized. If this is a file name, make sure the"
											 " file exists and is in the project folder.";
						MessageBox(NULL, errMsg.c_str(), NULL, MB_OK);
						return NULL;
					}
				}
			}

			/// calculate frequency differences for every signal. This is used for frequency ramps.
			std::vector<double> deltaOmega;
			double deltaTanh = std::tanh(4) - std::tanh(-4);
			for (int j = 0; j < inputData.signalNum; j++)
			{
				deltaOmega.push_back(2 * PI * (inputData.signals[j].freqFin - inputData.signals[j].freqInit));
			}
			/// ////////////////////////////////////////
			///
			///		Get Data Points.
			///
			// initialize i before the loop so that I have access to it afterwards.
			int i = 0;
			/// increment through all samples
			for (; i < waveformSize; i++)
			{
				// calculate the time that this sample number refers to
				double curTime = (double)i / SAMPLE_RATE;
				/// Calculate Phase and Power Positions. For Every signal...
				for (int j = 0; j < inputData.signalNum; j++)
				{
					/// Handle Frequency Ramps
					// Frequency ramps are actually a little complex. we have dPhi/dt = omega(t) and we need phi to calculate data points. So in order to get 
					// the phase you need to integrate the omega(t) you want and modify the integration constant to get your initial phase.
					if (inputData.signals[j].freqRampType == "lin")
					{
						// W{t} = Wi + (DeltaW * t) / (Tfin)
						// Phi{t}   = Wi * t + (DeltaW * t ^ 2) / 2 + phi_i
						phasePos[j] = 2 * PI * inputData.signals[j].freqInit * curTime + deltaOmega[j] * pow(curTime, 2) / (2 * inputData.time) 
									+ inputData.signals[j].initPhase;
					}
					else if (inputData.signals[j].freqRampType == "tanh")
					{
						// We want
						// W{t} = Wi + (DeltaW * (Tanh{-4 + 8 * t' / Tf} - Tanh{-4}) / (Tanh{4} - Tanh{-4})
						// This gives the correct initial value, final value, and symmetry of the ramp for the frequency. -4 was picked somewhat arbitrarily.
						// The integral of this is
						// dw/(2*tanh{4}) * T_f/8 * ln{cosh{-4+8t/T_f}} + (w_i - (dw * tanh{-4})/2*tanh{4}) t + C
						// Evaluating C to give the correct phase gives
						// phi{t} = (w_i+dw/2)t+(dw)/dtanh * T_f/8 * (ln{cosh{-4+8t/T_f}}-ln{cosh{-4}}) + phi_0
						// See onenote for more math.
						phasePos[j] = (2 * PI * inputData.signals[j].freqInit + deltaOmega[j] / 2.0) * curTime
									  + (deltaOmega[j] / deltaTanh ) * (inputData.time / 8.0) * ( std::log(std::cosh(4 - (8 / inputData.time) * curTime))
									  - std::log(std::cosh(4))) + inputData.signals[j].initPhase;
					}
					else if (inputData.signals[j].freqRampType == "nr")
					{
						// omega{t} = omega
						// phi = omega*t
						phasePos[j] = 2 * PI * inputData.signals[j].freqInit * curTime + inputData.signals[j].initPhase;
					}
					else
					{
						// special ramp case. I'm not sure if this is actually useful. The frequency file would have to be designed very carefully.
						freqRampPos[j] = freqRampFileData[j][i] * (inputData.signals[j].freqFin - inputData.signals[j].freqInit);
						phasePos[j] = (ViReal64)i * 2 * PI * (inputData.signals[j].freqInit + freqRampPos[j]) / SAMPLE_RATE
									  + inputData.signals[j].initPhase;
					}

					/// amplitude ramps are much simpler.
					if (inputData.signals[j].powerRampType != "lin" && inputData.signals[j].powerRampType != "nr" && inputData.signals[j].powerRampType 
						!= "tanh")
					{
						// use data from file
						powerPos[j] = powerRampFileData[j][i] * (inputData.signals[j].finPower - inputData.signals[j].initPower);
					}
					else
					{
						// use the ramp calc function to find the current power.
						powerPos[j] = myMath::rampCalc(waveformSize, i, inputData.signals[j].initPower, inputData.signals[j].finPower, 
													   inputData.signals[j].powerRampType);
					}
				}

				/// If option is marked, then normalize the power.
				if (CONST_POWER_OUTPUT == true)
				{
					double currentPower = 0;
					// calculate the total current amplitude.
					for (int j = 0; j < inputData.signalNum; j++)
					{
						currentPower += fabs(inputData.signals[j].initPower + powerPos[j]);
						/// modify here for calibrations!
						/// need current frequency and calibration file.
					}

					// normalize each signal.
					for (int j = 0; j < inputData.signalNum; j++)
					{
						powerPos[j] = (inputData.signals[j].initPower + powerPos[j]) * (TOTAL_POWER / currentPower) - inputData.signals[j].initPower;
					}
				}
				///  Calculate data point.
				readData[i] = 0;
				for (int j = 0; j < inputData.signalNum; j++)
				{
					// get data point. V = Sqrt(Power) * Sin(Phase)
					readData[i] += sqrt(inputData.signals[j].initPower + powerPos[j]) * sin(phasePos[j]);
				}
			}

			/// Calculate one last time for the final phases. I want the final phase to be the phase of the NEXT data point. Then, following waveforms can
			/// START at this phase.
			double curTime = (double)i / SAMPLE_RATE;
			for (int j = 0; j < inputData.signalNum; j++)
			{
				/// Calculate Phase Position. See above for description.
				if (inputData.signals[j].freqRampType == "lin")
				{
					phasePos[j] = 2 * PI * inputData.signals[j].freqInit * curTime + deltaOmega[j] * pow(curTime, 2) * 1 / (2 * inputData.time) 
								  + inputData.signals[j].initPhase;
				}
				else if (inputData.signals[j].freqRampType == "tanh")
				{
					phasePos[j] = (2 * PI * inputData.signals[j].freqInit + deltaOmega[j] / 2.0) * curTime
						+ (deltaOmega[j] / deltaTanh) * (inputData.time / 8.0) * std::log(std::cosh(4 - (8 / inputData.time) * curTime))
						- (deltaOmega[j] / deltaTanh) * (inputData.time / 8.0) * std::log(std::cosh(4))
						+ inputData.signals[j].initPhase;
				}
				else if (inputData.signals[j].freqRampType == "nr")
				{
					phasePos[j] = 2 * PI * inputData.signals[j].freqInit * curTime + inputData.signals[j].initPhase;
				}
				else
				{
					freqRampPos[j] = freqRampFileData[j][i] * (inputData.signals[j].freqFin - inputData.signals[j].freqInit);
					phasePos[j] = (ViReal64)i * 2 * PI * (inputData.signals[j].freqInit + freqRampPos[j]) / (SAMPLE_RATE)
									+inputData.signals[j].initPhase;
				}
				// Don't need amplitude info.
			}

			for (int j = 0; j < inputData.signalNum; j++)
			{
				// get the final phase of this waveform. Note that this is the phase of the /next/ data point (the last time i gets incremented, the for loop 
				// doesn't run) so that if the next waveform starts at this data point, it will avoid repeating the same data point. This is used for the 
				// option where the user uses this phase as the starting phase of the next waveform.
				inputData.signals[j].finPhase = fmod(phasePos[j], 2 * PI);
				// catch the case in which the final phase is virtually identical to 2*PI, which isn't caught in the above line because of bad floating point 
				// arithmetic.
				if (fabs(inputData.signals[j].finPhase - 2 * PI) < 0.00000005)
				{
					inputData.signals[j].finPhase = 0;
				}
				// put the final phase in the last data point.
				readData[i + j] = inputData.signals[j].finPhase;
			}
			return readData;
		}

		/**
		* This function takes two filled waveform arrays, and interweaves them into a new data array. this is required (strangely) by the NI card for outputting to
		* both outputs separately.
		* @param waveform1 the first waveform (usually x)
		* @param waveform2 the second waveform (usually y)
		* @param finalWaveform the larger (2X) array which enters unfilled and exits with the interweaved data
		* @param waveformSize the size in samples of one of the original arrays (they should be the same size).
		*/
		ViReal64* mixWaveforms(ViReal64* waveform1, ViReal64* waveform2, ViReal64* finalWaveform, long int waveformSize)
		{
			for (long int i = 0; i < waveformSize; i++)
			{
				finalWaveform[2 * i] = waveform1[i];
				finalWaveform[2 * i + 1] = waveform2[i];
			}
			return finalWaveform;
		}

		/**
		* this function takes in a command and checks it against all "logic commands", returing true if the inputted command matches a logic command and false
		* otherwise
		* @param command the command being checked for being logic or not.
		* @return bool the truth value to the statement "command is a logic command"
		*/
		bool isLogicCommand(std::string command)
		{
			if (command == "wait until trigger" || command == "wait set # <# of samples to wait>" || command == "wait set #"
				|| command == "repeat set # <# of times to repeat>" || command == "repeat set #")
			{
				return true;
			}
			else if (command == "repeat until trigger" || command == "repeat forever" || command == "end repeat" || command == "if trigger" || command == "else"
				|| command == "end if")
			{
				return true;

			}
			else
			{
				return false;
			}
		}

		/**
		* this function takes in a command and checks it against all "generate commands", returing true if the inputted command matches a generate command and false
		* otherwise
		* @param command the command being checked for being a generate command or not.
		* @return bool the truth value to the statement "command is a generate command"
		*/
		bool isGenCommand(std::string c)
		{
			if (c == "gen 1, const" || c == "gen 1, const <freq> <amp> <phase (rad)>; <time>" || c == "gen 2, const"
				|| c == "gen 2, const <freq1> <amp1> <phase (rad)>; <sim for 2nd>; <time>" || c == "gen 3, const")
			{
				return true;
			}
			else if (c == "gen 3, const <freq1> <amp1> <phase (rad)>; <sim for 2nd, 3rd>; <time>" || c == "gen 4, const"
				|| c == "gen 4, const <freq1> <amp1> <phase (rad)>; <sim for 2nd, 3rd, 4th>; <time>")
			{
				return true;
			}
			else if (c == "gen 5, const" || c == "gen 5, const <freq1> <amp1> <phase (rad)>; <sim for 2nd, 3rd, 4th, 5th>; <time>" || c == "gen 1, amp ramp")
			{
				return true;
			}
			else if (c == "gen 1, amp ramp <freq> <amp ramp type> <initial amp> <final amp> <phase (rad)>; <time>")
			{
				return true;
			}
			else if (c == "gen 2, amp ramp" || c == "gen 2, amp ramp <freq1> <amp1 ramp type> <initial amp1> <final amp1> <phase (rad)>; <sim for 2nd>; <time>"
				|| c == "gen 3, amp ramp")
			{
				return true;
			}
			else if (c == "gen 3, amp ramp <freq1> <amp1 ramp type> <initial amp1> <final amp1> <phase (rad)>; <sim for 2nd, 3rd>; <time>"
				|| c == "gen 4, amp ramp")
			{
				return true;
			}
			else if (c == "gen 4, amp ramp <freq1> <amp1 ramp type> <initial amp1> <final amp1> <phase (rad)>; <sim for 2nd, 3rd, 4th>; <time>"
				|| c == "gen 5, amp ramp")
			{
				return true;
			}
			else if (c == "gen 5, amp ramp <freq1> <amp1 ramp type> <initial amp1> <final amp1> <phase (rad)>; <sim for 2nd, 3rd, 4th, 5th>; <time>")
			{
				return true;
			}
			else if (c == "gen 1, freq ramp" || c == "gen 1, freq ramp <freq ramp type> <initial freq> <final freq> <amp> <phase (rad)>; <time>"
				|| c == "gen 2, freq ramp")
			{
				return true;
			}
			else if (c == "gen 2, freq ramp <freq1 ramp type> <initial freq1> <final freq1> <amp1> <phase (rad)>; <sim for 2nd>; <time>"
				|| c == "gen 3, freq ramp")
			{
				return true;
			}
			else if (c == "gen 3, freq ramp <freq1 ramp type> <initial freq1> <final freq1> <amp1> <phase (rad)>; <sim for 2nd, 3rd>; <time>"
				|| c == "gen 4, freq ramp")
			{
				return true;
			}
			else if (c == "gen 4, freq ramp <freq1 ramp type> <initial freq1> <final freq1> <amp1> <phase (rad)>; <sim for 2nd, 3rd, 4th>; <time>"
				|| c == "gen 5, freq ramp")
			{
				return true;
			}
			else if (c == "gen 2, freq & amp ramp" || c == "gen 2, freq & amp ramp <freq1 ramp type> <initial freq1> <final freq1> <amp ramp1 type> <initial ramp1>"
				" <final ramp1> <phase (rad)>; <sim for 2nd>; <time>")
			{
				return true;
			}
			else if (c == "gen 3, freq & amp ramp" || c == "gen 3, freq & amp ramp <freq1 ramp type> <initial freq1> <final freq1> <amp ramp1 type> <initial ramp1>"
				" <final ramp1> <phase (rad)>; <sim for 2nd, 3rd>; <time>")
			{
				return true;
			}
			else if (c == "gen 4, freq & amp ramp" || c == "gen 4, freq & amp ramp <freq1 ramp type> <initial freq1> <final freq1> <amp ramp1 type> <initial ramp1>"
				" <final ramp1> <phase (rad)>; <sim for 2nd, 3rd, 4th>; <time>")
			{
				return true;
			}
			else if (c == "gen 5, freq & amp ramp" || c == "gen 5, freq & amp ramp <freq1 ramp type> <initial freq1> <final freq1> <amp ramp1 type> <initial ramp1>"
				" <final ramp1> <phase (rad)>; <sim for 2nd, 3rd, 4th, 5th>; <time>")
			{
				return true;
			}
			else 
			{
				return false;
			}
		}

		/**
		 * this function takes in a command and checks it against all "special commands", returing true if the inputted command matches a special command and false
		 * otherwise
		 * @param command the command being checked for being special or not.
		 * @return bool the truth value to the statement "command is a special command"
		 */
		bool isSpecialCommand(std::string c)
		{
			if (c == "predefined script" || c == "predefined script <verticalFileName.txt> <horizontalFileName.txt>" || c == "create marker event"
				|| c == "create marker event <samples after previous waveform to wait>" || c == "predefined waveform")
			{
				return true;
			}
			else if (c == "predefined waveform <x filename.txt> <y filename.txt>")
			{
				return true;
			}
			else if (c == "create marker event <samples after previous waveform to wait>")
			{
				return true;
			}
			else 
			{
				return false;
			}
		}

		/*
		 * - This function checks the beginning of two file for their variable status. It also compares the two files output to check for errors
		 * @param file1 the first file to check
		 * @param file2 the second file to check
		 * @return exit status indicates whether error was encountered during the read & or comparison.
		 */ 
		bool readForVariables(std::fstream &file1, std::fstream &file2, std::vector<std::string> &file1Names, std::vector<std::string> &file2Names,
			std::vector<char> &var1Names, std::vector<char> &var2Names, std::vector<std::fstream> &varFiles, bool isDef)
		{
			// Before reading from a file, clear any extra white-space the user put in it.
			rmWhite(file1);

			std::string varType1;
			//char temp = file1.get();
			// Read in variable information, which is stored at the top of the instructions file.
			std::string tempstr;
			//file1 >> tempstr;
			std::getline(file1, varType1);

			if (varType1.length() != 0)
			{
				if (varType1[varType1.length() - 1] == '\r')
				{
					varType1.erase(varType1.length() - 1);
				}
			}

			if (varType1 == "NO VARIABLES")
			{
				// Don't do anything.
			}
			else if (varType1 == "1 VARIABLE <variable name> <file.txt for variable values>" || varType1 == "1 VARIABLE")
			{
				if (isDef == true)
				{
					MessageBox(NULL, "ERROR: default script should not have any variables in it!", "ERROR", MB_OK | MB_ICONERROR);
					return false;
				}
				// read in variable info
				std::string tempString;
				rmWhite(file1);
				file1 >> tempString;
				// this string should be a single character.
				var1Names.push_back(tempString[0]);
				file1 >> tempString;
				file1Names.push_back(tempString);
			}
			else if (varType1 == "2 VARIABLES <variable #1 name> <file.txt for variable #1 values>; <variable #2 name> <file.txt for variable #2 values>"
				|| varType1 == "2 VARIABLES") 
			{
				if (isDef == true)
				{
					MessageBox(NULL, "ERROR: default script should not have any variables in it!", "ERROR", MB_OK | MB_ICONERROR);
					return false;
				}
				// read in variable info
				std::string tempString;
				rmWhite(file1);
				file1 >> tempString;
				// this string should be a single character.
				var1Names.push_back(tempString[0]);
				file1 >> tempString;
				file1Names.push_back(tempString);

				file1.ignore(1, ';');
				file1 >> tempString;
				// this string should be a single character.
				var1Names.push_back(tempString[0]);
				file1 >> tempString;
				file1Names.push_back(tempString);
			}
			else
			{
				char tempStr[200];
				sprintf_s(tempStr, "ERROR: Variable status not described correctly at top of x instructions file. Relevant bad string is \"%s\"", varType1.c_str());
				MessageBox(NULL, tempStr, "ERROR", MB_OK | MB_ICONERROR);
				return false;
			}

			// clear extra whitespace before reading.
			rmWhite(file2);
			std::string varType2;
			// Similar
			std::getline(file2, varType2);
			if (varType2.length() != 0)
			{
				if (varType2[varType2.length() - 1] == '\r')
				{
					varType2.erase(varType2.length() - 1);
				}
			}


			if (varType2 == "NO VARIABLES")
			{
				// Don't do anything.
			}
			else if (varType2 == "1 VARIABLE <variable name> <file.txt for variable values>" || varType2 == "1 VARIABLE")
			{
				if (isDef == true)
				{
					MessageBox(NULL, "ERROR: default script should not have any variables in it!", "ERROR", MB_OK | MB_ICONERROR);
					return false;
				}
				// read in variable info
				std::string tempString;
				rmWhite(file2);
				file2 >> tempString;
				// this string should be a single character.
				var2Names.push_back(tempString[0]);
				file2 >> tempString;
				file2Names.push_back(tempString);
			}
			else if (varType2 == "2 VARIABLES <variable #1 name> <file.txt for variable #1 values>; <variable #2 name> <file.txt for variable #2 values>"
					 || varType2 == "2 VARIABLES") 
			{
				if (isDef == true)
				{
					MessageBox(NULL, "ERROR: default script should not have any variables in it!", "ERROR", MB_OK | MB_ICONERROR);
					return false;
				}
				// read in variable info
				std::string tempString;
				rmWhite(file2);
				file2 >> tempString;
				// this string should be a single character.
				var2Names.push_back(tempString[0]);
				file2 >> tempString;
				file2Names.push_back(tempString);
				file2.ignore(1, ';');
				file2 >> tempString;
				// this string should be a single character.
				var2Names.push_back(tempString[0]);
				file2 >> tempString;
				file2Names.push_back(tempString);
			}
			else
			{
				char tempStr[200];
				sprintf_s(tempStr, "ERROR: Variable status not described correctly at top of x instructions file. Relevant bad string is \"%s\"", 
						  varType2.c_str());
				MessageBox(NULL, tempStr, "ERROR", MB_OK | MB_ICONERROR);
				return false;
			}

			// Check if the right number of varibles.
			if (file1Names.size() != file2Names.size() || file1Names.size() != var1Names.size() || file1Names.size() != var2Names.size())
			{
				MessageBox(NULL, "ERROR: the x-instructions file and the y-instructions file don't have the same number of variables! They must both have the exact "
					"variables, even if some are only used in one files.\n", NULL, MB_OK);
				return false;
			}
			// Check if the variables are the same
			for (unsigned int i = 0; i < file1Names.size(); i++)
			{
				if (file1Names[i] != file2Names[i])
				{
					MessageBox(NULL, "ERROR: The files referenced for each variable don't match! A variable must reference the same file in the x and y instruction "
									 "files.\n", NULL, MB_OK);
					return false;
				}

				if (var1Names[i] != var2Names[i])
				{
					MessageBox(NULL, "ERROR: The names of the variables listed in each file don't match! The names of the variables in the x-instructions file must "
									 "the names of the variables in the y-instructions file, in order.\n", NULL, MB_OK);
					return false;
				}
				std::fstream tempFile;
				// try to open variable files
				tempFile.open(file1Names[i]);
				varFiles.push_back(std::move(tempFile));
				if (!varFiles[i].is_open())
				{
					char tempStr[200];
					sprintf_s(tempStr, "ERROR: can't open file number %i", i);
					MessageBox(NULL, tempStr, "ERROR", MB_OK | MB_ICONERROR);
					return false;
				}
			}
			return true;
		}


		/**
		 * Overload for double input.
		 * Test if the input is a variable. If it was, store what parameter is getting varied (varParamTypes), store which variable name is associated
		 * with this variable, increment the number of variables in this waveform, and increment the varPresent marker, which gets returned and tells the
		 * function using this function to check the arrays that get returned for the specified number of variables.
		 *
		 * @param dataToAssign is the value of a waveData variable that is being read in. If a variable is found it isn't actually assigned.
		 * @param fName is the instructions file being read for this input.
		 * @param vCount is the number of variables that have been assigned so far.
		 * @param vNames is a vector that holds the names of the variables that are found in the script.
		 * @param vParamTypes is a vector that holds the information as to what type of parameter is being varried for a given variable name.
		 * @param dataType is the number to assign to vParamTypes if a variable is being used.
		 */
		int getParamCheckVar(double& dataToAssign, std::fstream& fName, int& vCount, std::vector<std::string>& vNames, std::vector<int>& vParamTypes, 
							 int dataType)
		{
			std::string tempInput;
			int stringPos;
			rmWhite(fName);
			fName >> tempInput;
			// pull input to lower case to prevent stupid user input errors.
			std::transform(tempInput.begin(), tempInput.end(), tempInput.begin(), tolower);
			if (tempInput[0] == '\'')
			{
				MessageBox(NULL, "ERROR: Don't use \' as a variable in your instructions file, this character is reserved by the program.\n", NULL, MB_OK);
				return -1;
			}
			if (tempInput[0] == '#')
			{
				MessageBox(NULL, "ERROR: the delimeter '#' was detected in a waveform before it was supposed to be. This indicates either that there are too few "
								 "inputs for this waveform type or that the program is not reading the input correctly, e.g. because of extraneous semicolons.",
						   NULL, MB_OK);
				return -2;
			}
			if (tempInput[0] == '%')
			{
				MessageBox(NULL, "ERROR: the character % was detected in the input. This shouldn't be possible. Look for logic errors.", NULL, MB_OK);
				return -3;
			}
			// the following aren't digits, but don't indicate variables.
			if (tempInput[0] == '-' || tempInput[0] == '.')
			{
				stringPos = 1;
				if (tempInput[1] == '-' || tempInput[1] == '.')
				{
					MessageBox(NULL, "ERROR: The first two characters of some input are both either '-' or '.'. This might be because you tried to input a negative"
									 "decimal, which you aren't allowed to do.", NULL, MB_OK);
					return -4;
				}
			}
			else 
			{
				stringPos = 0;
			}
			if (!isdigit(tempInput[stringPos]))
			{
				// load variable name into structure.
				vNames.push_back(tempInput);
				vParamTypes.push_back(dataType);
				vCount++;
				return 0;
			}
			else
			{
				// this should happen most of the time.
				dataToAssign = (std::stod(tempInput));
				return 0;
			}
		}
		/**
		 * overload for integer input.
		 * Test if the input is a variable. If it was, store what parameter is getting varied (varParamTypes), store which variable name is associated
		 * with this variable, increment the number of variables in this waveform, and increment the varPresent marker, which gets returned and tells the
		 * function using this function to check the arrays that get returned for the specified number of variables.
		 *
		 * @param dataToAssign is the value of a waveData variable that is being read in. If a variable is found it isn't actually assigned.
		 * @param fName is the instructions file being read for this input.
		 * @param vCount is the number of variables that have been assigned so far.
		 * @param vNames is a vector that holds the names of the variables that are found in the script.
		 * @param vParamTypes is a vector that holds the information as to what type of parameter is being varried for a given variable name.
		 * @param dataType is the number to assign to vParamTypes if a variable is being used.
		 */
		int getParamCheckVar(int& dataToAssign, std::fstream& fileName, int& vCount, std::vector<std::string>& vNames, std::vector<int>& vParamTypes, int dataType)
		{
			std::string tempInput;
			int stringPos;
			rmWhite(fileName);
			fileName >> tempInput;
			// pull input to lower case to prevent stupid user input errors.
			std::transform(tempInput.begin(), tempInput.end(), tempInput.begin(), ::tolower);

			if (tempInput[0] == '\'') 
			{
				MessageBox(NULL, "ERROR: Don't use \' as a variable in your instructions file, this character is reserved by the program.\n", NULL, MB_OK);
				return -1;
			}
			if (tempInput[0] == '#') 
			{
				MessageBox(NULL, "ERROR: the delimeter '#' was detected in a waveform before it was supposed to be. This indicates either that there are too few "
								 "inputs for this waveform type or that the program is not reading the input correctly, e.g. because of extraneous semicolons.",
						   NULL, MB_OK);
				return -2;
			}
			if (tempInput[0] == '%') 
			{
				MessageBox(NULL, "ERROR: the character % was detected in the input. This shouldn't be possible. Look for logic errors.", NULL, MB_OK);
				return -3;
			}
			// the following aren't digits, but don't indicate variables.
			if (tempInput[0] == '-' || tempInput[0] == '.') 
			{
				stringPos = 1;
				if (tempInput[1] == '-' || tempInput[1] == '.') 
				{
					MessageBox(NULL, "ERROR: The first two characters of some input are both either '-' or '.'. This might be because you tried to input a negative"
									 "decimal, which you aren't allowed to do.", NULL, MB_OK);
					return -4;
				}
			}
			else 
			{
				stringPos = 0;
			}
			if (!isdigit(tempInput[stringPos])) 
			{
				// load variable name into structure.
				vNames.push_back(tempInput);
				vParamTypes.push_back(dataType);
				vCount++;
				return 0;
			}
			else 
			{
				// this should happen most of the time.
				dataToAssign = (std::stoi(tempInput));
				return 0;
			}
		}

		/**
		 * Overload for double input.
		 * Test if the input is a variable. If it was, store what parameter is getting varied (vParamTypes), store which variable name is associated
		 * with this variable, increment the number of variables in this waveform, and increment the varPresent marker, which gets returned and tells the
		 * function using this function to check the arrays that get returned for the specified number of variables. This function deals with constant waveforms,
		 * where either the frequency or the ramp is not being ramped, and so the inputted data needs to be assigned to both the initial and final values of the
		 * parameter type.
		 *
		 * @param data1ToAssign is the value of the first waveData variable that is being read in. If a variable is found it isn't actually assigned.
		 * @param data2ToAssign is the value of the second waveData variable that is being read in. If a variable is found it isn't actually assigned.
		 * @param fName is the instructions file being read for this input.
		 * @param vCount is the number of variables that have been assigned so far.
		 * @param vNames is a vector that holds the names of the variables that are found in the script.
		 * @param vParamTypes is a vector that holds the information as to what type of parameter is being varried for a given variable name.
		 * @param dataType1 is the number to assign to vParamTypes for the first parameter if a variable is being used.
		 * @param dataType2 is the number to assign to vParamTypes for the second parameter if a variable is being used.
		 */
		int getParamCheckVarConst(double& data1ToAssign, double& data2ToAssign, std::fstream& fName, int& vCount, std::vector<std::string>& vNames,
			std::vector<int>& vParamTypes, int dataType1, int dataType2) 
		{
			std::string tempInput;
			rmWhite(fName);
			fName >> tempInput;
			// pull input to lower case to prevent stupid user input errors.
			std::transform(tempInput.begin(), tempInput.end(), tempInput.begin(), ::tolower);

			// the following aren't digits, but don't indicate variables.
			if (tempInput[0] == '\'')
			{
				MessageBox(NULL, "ERROR: Don't use \' as a variable in your instructions file, this character is reserved by the program.\n", NULL, MB_OK);
				return -1;
			}
			if (tempInput[0] == '#')
			{
				MessageBox(NULL, "ERROR: the delimeter '#' was detected in a waveform before it was supposed to be. This indicates either that there are too few "
					"inputs for this waveform type or that the program is not reading the input correctly, e.g. because of extraneous semicolons.",
					NULL, MB_OK);
				return -2;
			}
			if (tempInput[0] == '%')
			{
				MessageBox(NULL, "ERROR: the character % was detected in the input. This shouldn't be possible. Look for logic errors.", NULL, MB_OK);
				return -3;
			}
			if (tempInput[0] == '-')
			{
				MessageBox(NULL, "ERROR: it appears that you entered a negative frequency or amplitude. You can't do that.", NULL, MB_OK);
				return -4;
			}

			// I don't need to check for -1 input because this function should never be used on the phase or the time, only frequency or amplitude for non-ramping waveforms. 
			if (!isdigit(tempInput[0]))
			{
				// add variable name
				vNames.push_back(tempInput);
				vParamTypes.push_back(dataType1);
				vCount++;
				// Do the same for the second data that needs to be assigned.
				vNames.push_back(tempInput);
				vParamTypes.push_back(dataType2);
				vCount++;
				return 0;
			}
			else
			{
				// this should happen most of the time.
				data1ToAssign = std::stod(tempInput);
				data2ToAssign = data1ToAssign;
				return 0;
			}
		}

		/**
		 * Overload for integer input.
		 * Test if the input is a variable. If it was, store what parameter is getting varied (vParamTypes), store which variable name is associated
		 * with this variable, increment the number of variables in this waveform, and increment the varPresent marker, which gets returned and tells the
		 * function using this function to check the arrays that get returned for the specified number of variables. This function deals with constant waveforms,
		 * where either the frequency or the ramp is not being ramped, and so the inputted data needs to be assigned to both the initial and final values of the
		 * parameter type.
		 *
		 * @param data1ToAssign is the value of the first waveData variable that is being read in. If a variable is found it isn't actually assigned.
		 * @param data2ToAssign is the value of the second waveData variable that is being read in. If a variable is found it isn't actually assigned.
		 * @param fName is the instructions file being read for this input.
		 * @param vCount is the number of variables that have been assigned so far.
		 * @param vNames is a vector that holds the names of the variables that are found in the script.
		 * @param vParamTypes is a vector that holds the information as to what type of parameter is being varried for a given variable name.
		 * @param dataType1 is the number to assign to vParamTypes for the first parameter if a variable is being used.
		 * @param dataType2 is the number to assign to vParamTypes for the second parameter if a variable is being used.
		 */
		int getParamCheckVarConst(int& data1ToAssign, double& data2ToAssign, std::fstream& fileName, int& vCount, std::vector<std::string>& vNames, 
								  std::vector<int>& vParamTypes, int dataType1, int dataType2)
		{
			std::string tempInput;
			rmWhite(fileName);
			fileName >> tempInput;
			// pull input to lower case to prevent stupid user input errors.
			std::transform(tempInput.begin(), tempInput.end(), tempInput.begin(), ::tolower);

			// the following aren't digits, but don't indicate variables.
			if (tempInput[0] == '\'')
			{
				MessageBox(NULL, "ERROR: Don't use \' as a variable in your instructions file, this character is reserved by the program.\n", NULL, MB_OK);
				return -1;
			}
			if (tempInput[0] == '#')
			{
				MessageBox(NULL, "ERROR: the delimeter '#' was detected in a waveform before it was supposed to be. This indicates either that there are too few "
					"inputs for this waveform type or that the program is not reading the input correctly, e.g. because of extraneous semicolons.",
					NULL, MB_OK);
				return -2;
			}
			if (tempInput[0] == '%')
			{
				MessageBox(NULL, "ERROR: the character % was detected in the input. This shouldn't be possible. Look for logic errors.", NULL, MB_OK);
				return -3;
			}
			if (tempInput[0] == '-')
			{
				MessageBox(NULL, "ERROR: it appears that you entered a negative frequency or amplitude. You can't do that.", NULL, MB_OK);
				return -4;
			}

			// I don't need to check for -1 input because this function should never be used on the phase or the time, only frequency or amplitude for non-ramping waveforms. 
			if (!isdigit(tempInput[0]))
			{
				// add variable name
				vNames.push_back(tempInput);
				vParamTypes.push_back(dataType1);
				vCount++;
				// Do the same for the second data that needs to be assigned.
				vNames.push_back(tempInput);
				vParamTypes.push_back(dataType2);
				vCount++;
				return 0;
			}
			else
			{
				// this should happen most of the time.
				data1ToAssign = std::stoi(tempInput);
				data2ToAssign = data1ToAssign;
				return 0;
			}
		}
		
	}
}
