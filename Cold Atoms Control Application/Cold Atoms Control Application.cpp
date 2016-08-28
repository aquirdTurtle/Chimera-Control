/****************************************
*										*
* NI-PXIe-5451 AWG Control.cpp			*
*										*
*****************************************

* This program takes as input the x and y instructions files, which are to be written by the user to define the sequence of waveforms and triggers
* for the arbitrary waveform generator to generate, being dependent on which particular experiment is being run. And so much more.
*/
/**PROJECT ORGANIZATION****************************************\
3 Threads:
- Main Window Message Handling thread
- Calculations Thread for the experiment
- NIAWG wait thread
\*************************************************************/

// a file visual c++ uses for efficiency in compiling headers.
#include "stdafx.h"
// Contains some user-defined global parameters and options used throughout the code.
#include "constants.h"
#include "externals.h"
// a function used to clear any leading whitespace from the input files.
#include "rmWhite.h"
// Used to send text to main window debug or status sections
#include "appendText.h"
// cleans up parameters related to a socket
#include "cleanSocket.h"
// an namespace for agilent functions.
#include "myAgilent.h"
//
#include "myErrorHandler.h"
// 
#include "myNIAWG.h"

// Contains functions and types used by the NIAWG.
#include "niFgen.h"

// Contians the declaration of the main window procedure control function
#include "winProcScripts.h"
#include "winProcMain.h"

// Contains information the API uses for dialogues.
#include "resource.h"

// Used to get a precise (milisecond-level) time from the computer. Also for windows API functions & declarations
#include <Windows.h>

// Some headers used for communication protocols.
#include <winsock2.h>
#include <ws2tcpip.h>

// Some general use headers.
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <stdlib.h>
// contains the boost function for finding a common factor.
#include "boost/math/common_factor.hpp"
// contains stuff I use for file IO.
#include <boost/filesystem.hpp>

// This is used to tell the compiler that this specific library is needed.
#pragma comment(lib, "Ws2_32.lib")
//#include <vld.h> 

/// /////////////////////////////////////////////////////////////////////////////////////////
/// The main function for win32 programs. The main programming sequence for this application.
/// /////////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	if (TWEEZER_COMPUTER_SAFEMODE)
	{
		MessageBox(0, "Starting in Safe Mode. The program will not actually communicate with any of the devices", 0, MB_OK);
	}
	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		errBox("Fatal Error: MFC initialization failed");
		return false;
	}
	eGlobalInstance = hInstance;
	/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	///					Initializations
	///
	// create the MSG variable which contains messages sent by the OS to the function based on user input
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	// Socket object for communicating with the other computer.
	SOCKET ConnectSocket = INVALID_SOCKET;
	// An array of variable files. Only used if not receiving variable information from the master computer.
	std::vector<std::fstream> xVarFiles;
	// Will contain the entire script of the default waveform.
	std::string default_hConfigScriptString;
	std::string default_vConfigScriptString;
	// Vectors of structures that each contain all the basic information about a single waveform. Most of this (pardon the default waveforms) gets erased after 
	// an experiment.
	std::vector<waveData> allXWaveformParameters, allYWaveformParameters;
	// Vectors of flags that signify whether a given waveform is being varied or not.
	std::vector<bool> xWaveformIsVaried, yWaveformIsVaried, intensityIsVaried;
	// A vector of vectors that hold variables. Each sub-vector contains all of the values that an individual variable will take on. The main vector then 
	// contains all of the variable value data.
	std::vector<std::vector<double> > variableValues;
	// A vector which stores the number of values that a given variable will take through an experiment.
	std::vector<std::size_t> length;
	// The eSessionHandle variable is used as an input to many of the arbitrary waveform generator functions so that the functions know which NI device (we only
	// have one, but we could have more) the given function is meant for.
	// An array of vectors holding strings. Each vector within the array is for a specific type of waveform output (e.g. gen 1, const). The each string within a
	// vector contains unique descriptions of waveforms. I check this list to see if a waveform has been written already.
	std::vector<std::string> libWaveformArray[20];
	// an array of boolian values with one-to-one correspondence to the array above. A value of 1 corresponds to the file holding the strings having already
	// been opened and read for the first time.
	bool fileOpenedStatus[20] = { 0 };
	// a file that contains the final version of the script sent to the waveform generator. Not actually used by the program, just for debugging purposes.
	std::ofstream scriptOutput("Most Recent 5451 Script Output.txt");
	// contains the sizes of mixed varied waveforms.
	std::vector<long int> variedMixedSize;
	// Counts the number of waveforms that have been handled so far.
	int waveformCount = 0;
	// counts the number of predefined waveforms that have been handled or defined.
	int predWaveformCount = 0;
	// counts the number of varied waveform pairs for x or y waveforms.
	int yVariedWaveformPairsCount = 0, xVariedWaveformPairsCount = 0;
	// A variable that records whether a given experiment has completed or not.
	ViBoolean isDoneTest = VI_FALSE;

	// Contains the names of predefined x waveforms
	std::vector<std::string> defXPredWaveformNames;
	// Contains the names of predefined y waveforms
	std::vector<std::string> defYPredWaveformNames;
	// Contains a bool that tells whether the user script has been written or not. This is used to tell whether I need to delete it or not.
	bool userScriptIsWritten = false;

	// This array stores the waveform #s of the predefined waveforms.
	std::vector<int> defPredWaveLocs;

	/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	///				Initializing the main application window
	///
	// structure that contains all the fundamental information for the main window.	
	WNDCLASSEX wClassNIAWG, wClassOther;
	ZeroMemory(&wClassNIAWG, sizeof(WNDCLASSEX));
	// some strange parameters I don't understand.
	wClassNIAWG.cbClsExtra = NULL;
	wClassNIAWG.cbSize = sizeof(WNDCLASSEX);
	wClassNIAWG.cbWndExtra = NULL;
	wClassNIAWG.lpszClassName = "Scripting Control Window";
	// change the background color ot the window to be a dark grey.0
	wClassNIAWG.hbrBackground = CreateSolidBrush(RGB(20, 0, 0));
	// cursor type.
	wClassNIAWG.hCursor = LoadCursor(NULL, IDC_ARROW);
	// load the NIAWG icon used for the application
	wClassNIAWG.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPLICATION_ICON));
	wClassNIAWG.hIconSm = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPLICATION_ICON), IMAGE_ICON, 32, 32, 0);
	// handle for addressing this object
	wClassNIAWG.hInstance = hInstance;
	// The procedure function this window follows
	wClassNIAWG.lpfnWndProc = (WNDPROC)winProcScripts;
	// parameters about how the window looks
	wClassNIAWG.style = CS_HREDRAW | CS_VREDRAW;
	// the identifier for the menu I created.
	wClassNIAWG.lpszMenuName = MAKEINTRESOURCE(IDR_MAIN_MENU);
	// register the window and check for errors
	if (!RegisterClassEx(&wClassNIAWG))
	{
		int errVal = GetLastError();
		MessageBox(0, ("Window class creation failed: " + std::to_string(errVal) + "\r\n").c_str(), 0, 0);
		return -1;
	}

	ZeroMemory(&wClassOther, sizeof(WNDCLASSEX));
	// some strange parameters I don't understand.
	wClassOther.cbClsExtra = NULL;
	wClassOther.cbSize = sizeof(WNDCLASSEX);
	wClassOther.cbWndExtra = NULL;
	wClassOther.lpszClassName = "Main Control Window";
	// change the background color ot the window to be a dark grey.
	wClassOther.hbrBackground = CreateSolidBrush(RGB(54, 62, 70));
	// cursor type.
	wClassOther.hCursor = LoadCursor(NULL, IDC_ARROW);
	// load the NIAWG icon used for the application
	wClassOther.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPLICATION_ICON));
	wClassOther.hIconSm = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPLICATION_ICON), IMAGE_ICON, 32, 32, 0);
	// handle for addressing this object
	wClassOther.hInstance = hInstance;
	// The procedure function this window follows
	wClassOther.lpfnWndProc = (WNDPROC)winProcMain;
	// parameters about how the window looks
	wClassOther.style = CS_HREDRAW | CS_VREDRAW;
	// the identifier for the menu I created.
	wClassOther.lpszMenuName = MAKEINTRESOURCE(IDR_MAIN_MENU);
	// register the window and check for errors
	if (!RegisterClassEx(&wClassOther))
	{
		int errVal = GetLastError();
		MessageBox(0, ("Window class creation failed: " + std::to_string(errVal) + "\r\n").c_str(), 0, 0);
		return -1;
	}

	// create handle for window.
	eScriptingWindowHandle = CreateWindowEx(NULL, "Scripting Control Window", "Cold Atoms Control: Scripting Window",
										   WS_MAXIMIZE | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
										   0, 0, 1920, 1000, NULL, NULL, hInstance, NULL);
	eMainWindowHandle = CreateWindowEx(NULL, "Main Control Window", "Cold Atoms Control: Main Control Window",
									  WS_MAXIMIZE | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
									  0, 0, 1920, 1000, NULL, NULL, hInstance, NULL);

	// check for errors.
	if (!eScriptingWindowHandle)
	{
		int errVal = GetLastError();
		MessageBox(0, ("FATAL ERROR: Create Window (CreateWindowEx) Failed: " + std::to_string(errVal) +"\r\n").c_str(), 0, MB_OK);
		return -1;
	}
	if (!eMainWindowHandle)
	{
		int errVal = GetLastError();
		MessageBox(0, ("FATAL ERROR: Create Window (CreateWindowEx) Failed: " + std::to_string(errVal) + "\r\n").c_str(), 0, MB_OK);
		return -1;
	}
	// make the windows appear.
	ShowWindow(eScriptingWindowHandle, SW_MAXIMIZE);
	ShowWindow(eMainWindowHandle, SW_MAXIMIZE);

	/// Other General Initializations
	// Check to make sure that the gain hasn't been defined to be too high.
	if (GAIN > MAX_GAIN)
	{
		MessageBox(0, "FATAL ERROR: GAIN SET TOO HIGH. Driving too much power into the AOMs could severaly damage the experiment!\r\n", 0, MB_OK);
		return -10000;
	}

	// initialize default file names and open the files.
	std::vector<std::fstream> default_hConfigVerticalScriptFile, default_hConfigHorizontalScriptFile, default_vConfigVerticalScriptFile,
		default_vConfigHorizontalScriptFile;
	default_hConfigVerticalScriptFile.push_back(std::fstream(DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_HCONFIG_VERTICAL_SCRIPT.nScript"));
	default_hConfigHorizontalScriptFile.push_back(std::fstream(DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_HCONFIG_HORIZONTAL_SCRIPT.nScript"));
	default_vConfigVerticalScriptFile.push_back(std::fstream(DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_VCONFIG_VERTICAL_SCRIPT.nScript"));
	default_vConfigHorizontalScriptFile.push_back(std::fstream((DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_VCONFIG_HORIZONTAL_SCRIPT.nScript")));

	// check errors
	if (!default_hConfigVerticalScriptFile[0].is_open())
	{
		errBox("FATAL ERROR: Couldn't open default file. Was looking for file " + DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_HCONFIG_VERTICAL_SCRIPT.nScript");
		return -1;
	}
	if (!default_hConfigHorizontalScriptFile[0].is_open())
	{
		errBox("FATAL ERROR: Couldn't open default file. Was looking for file " + DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_HCONFIG_HORIZONTAL_SCRIPT.nScript");
		return -1;
	}
	if (!default_vConfigVerticalScriptFile[0].is_open())
	{
		errBox("FATAL ERROR: Couldn't open default file. Was looking for file " + DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_VCONFIG_VERTICAL_SCRIPT.nScript");
		return -1;
	}
	if (!default_vConfigHorizontalScriptFile[0].is_open())
	{
		errBox("FATAL ERROR: Couldn't open default file. Was looking for file " + DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_VCONFIG_HORIZONTAL_SCRIPT.nScript");
		return -1;
	}

	/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	///				Outputting the code version to the Andor Computer 
	///

	// get time now
	time_t dateStart = time(0);
	struct tm datePointerStart;
	localtime_s(&datePointerStart, &dateStart);
	std::string logFolderNameStart = "Date " + std::to_string(datePointerStart.tm_year + 1900) + "-" + std::to_string(datePointerStart.tm_mon + 1) + "-"
		+ std::to_string(datePointerStart.tm_mday) + " Time " + std::to_string(datePointerStart.tm_hour) + "-" + std::to_string(datePointerStart.tm_min) + "-"
		+ std::to_string(datePointerStart.tm_sec);
	bool andorConnectedForFolder = false;
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		boost::filesystem::path dir(CODE_LOGGING_FILES_PATH + logFolderNameStart);

		do
		{
			int andorTest = boost::filesystem::create_directory(dir);
			if (andorTest == false)
			{
				// For some reason this doesn't seem to get called when the connection breaks.
				int andorDisconnectedOption = MessageBox(NULL, "This computer can't currently open logging files on the andor.\nAbort will quit the program"
					" (no output has started).\nRetry will re-attempt to connect to the Andor.\nIgnore will continue "
					"without saving the current file.", "Andor Disconnected", MB_ABORTRETRYIGNORE);
				switch (andorDisconnectedOption)
				{
				case IDABORT:
				{
					return -19192;
					break;
				}
				case IDRETRY:
				{
					break;
				}
				case IDIGNORE:
				{
					// break out without writing file.
					andorConnectedForFolder = true;
					break;
				}
				}
			}
			else
			{
				andorConnectedForFolder = true;
			}
		} while (andorConnectedForFolder == false);
	}
	logFolderNameStart += "\\";

	WIN32_FIND_DATA find_cpp_Data;
	HANDLE cpp_Find_Handle;
	WIN32_FIND_DATA find_h_Data;
	HANDLE h_Find_Handle;
	std::string cppFindString = ACTUAL_CODE_FOLDER_PATH + "*.cpp";
	std::string hFindString = ACTUAL_CODE_FOLDER_PATH + "*.h";
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		cpp_Find_Handle = FindFirstFile((LPSTR)cppFindString.c_str(), &find_cpp_Data);
		if (cpp_Find_Handle != INVALID_HANDLE_VALUE)
		{
			do
			{
				std::ifstream originalCodeFile(ACTUAL_CODE_FOLDER_PATH + find_cpp_Data.cFileName);
				if (originalCodeFile.is_open() == false)
				{
					MessageBox(NULL, "Error encountered when trying to use FindFirstFile or FindNextFile. The functions returned names to files that don't exist.",
						NULL, NULL);
				}
				bool andorConnected = false;
				do
				{
					std::ofstream andorCodeFileCopy(CODE_LOGGING_FILES_PATH + logFolderNameStart + find_cpp_Data.cFileName);
					if (andorCodeFileCopy.is_open() == false)
					{
						int andorDisconnectedOption = MessageBox(NULL, "This computer can't currently open logging files on the andor.\nAbort will quit the program"
							" (no output has started).\nRetry will re-attempt to connect to the Andor.\nIgnore will continue "
							"without saving the current file.", "Andor Disconnected", MB_ABORTRETRYIGNORE);
						switch (andorDisconnectedOption)
						{
						case IDABORT:
						{
							return -19191;
							break;
						}
						case IDRETRY:
						{
							break;
						}
						case IDIGNORE:
						{
							// break out without writing file.
							andorConnected = true;
							break;
						}
						}
					}
					else
					{
						andorConnected = true;
						andorCodeFileCopy << originalCodeFile.rdbuf();
					}
				} while (andorConnected == false);
			} while (FindNextFile((LPSTR)cpp_Find_Handle, &find_cpp_Data));
		}
		else
		{
			MessageBox(NULL, "Failed to find any .cpp files in folder!", NULL, NULL);
		}
	}
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		h_Find_Handle = FindFirstFile((LPSTR)hFindString.c_str(), &find_cpp_Data);
		if (h_Find_Handle != INVALID_HANDLE_VALUE)
		{
			do
			{
				std::ifstream originalCodeFile(ACTUAL_CODE_FOLDER_PATH + find_h_Data.cFileName);
				std::ofstream andorCodeFileCopy(CODE_LOGGING_FILES_PATH + logFolderNameStart + find_h_Data.cFileName);
				andorCodeFileCopy << originalCodeFile.rdbuf();
			} while (FindNextFile((LPSTR)h_Find_Handle, &find_h_Data));
		}
		else
		{
			MessageBox(NULL, "Failed to find any .h files in folder!", NULL, NULL);
		}
	}

	/// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	///				Initialize NIAWG
	///

	// parameters for variables used by the default file. (there shouldn't be any, these are essentially just placeholders so that I can use the same functions.
	std::vector<char> defXVarNames, defYVarNames;
	// parameters for variables used by the default file. (there shouldn't be any, these are essentially just placeholders so that I can use the same functions.
	std::vector<std::string> defXVarFileNames, defYVarFileNames;
	// parameters for variables used by the default file. (there shouldn't be any, these are essentially just placeholders so that I can use the same functions.
	std::vector<std::fstream> defXVarFiles;
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		std::ofstream hConfigVerticalDefaultScriptLog(EXPERIMENT_LOGGING_FILES_PATH + logFolderNameStart + "\\Default hConfig Vertical Script.script");
		std::ofstream hConfigHorizontalDefaultScriptLog(EXPERIMENT_LOGGING_FILES_PATH + logFolderNameStart + "\\Default hConfig Horizontal Script.script");
		std::ofstream vConfigVerticalDefaultScriptLog(EXPERIMENT_LOGGING_FILES_PATH + logFolderNameStart + "\\Default vConfig Vertical Script.script");
		std::ofstream vConfigHorizontalDefaultScriptLog(EXPERIMENT_LOGGING_FILES_PATH + logFolderNameStart + "\\Default vConfig Horizontal Script.script");
		hConfigVerticalDefaultScriptLog << default_hConfigVerticalScriptFile[0].rdbuf();
		hConfigHorizontalDefaultScriptLog << default_hConfigHorizontalScriptFile[0].rdbuf();
		vConfigVerticalDefaultScriptLog << default_vConfigVerticalScriptFile[0].rdbuf();
		vConfigHorizontalDefaultScriptLog << default_vConfigHorizontalScriptFile[0].rdbuf();
	}
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		/// Initialize the waveform generator. Currently this is set to reset the initialization parameters from the last run.
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_init(NI_5451_LOCATION, VI_TRUE, VI_TRUE, &eSessionHandle)))
		{
			return -1;
		}
		// Configure channels
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureChannels(eSessionHandle, SESSION_CHANNELS)))
		{
			return -1;
		}
		// Set output mode of the device to scripting mode (defined in constants.h)
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputMode(eSessionHandle, OUTPUT_MODE)))
		{
			return -1;
		}
		// configure marker event. This is set to output on PFI1, a port on the front of the card.
		// if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ExportSignal(eSessionHandle, NIFGEN_VAL_MARKER_EVENT, "Marker0", "PFI1")))
		//{
		//	return -1;
		//}
		// enable flatness correction. This allows there to be a bit less frequency dependence on the power outputted by the waveform generator.
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_SetAttributeViBoolean(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_FLATNESS_CORRECTION_ENABLED, VI_TRUE)))
		{
			return -1;
		}
		// configure the trigger. Trigger mode doesn't need to be set because I'm using scripting mode.
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureDigitalEdgeScriptTrigger(eSessionHandle, TRIGGER_NAME, TRIGGER_SOURCE, TRIGGER_EDGE_TYPE)))
		{
			return -1;
		}
		// Configure the gain of the signal amplification.
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureGain(eSessionHandle, SESSION_CHANNELS, GAIN)))
		{
			return -1;
		}
		// Configure Sample Rate. The maximum value of this is 400 mega-samples per second, but it is quite buggy, so we've been using 350 MS/s
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureSampleRate(eSessionHandle, SAMPLE_RATE)))
		{
			return -1;
		}
		// Configure the analogue filter. This is important for high frequency signals, as it smooths out the discrete steps that the waveform generator outputs.
		// This is a low-pass filter.
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_EnableAnalogFilter(eSessionHandle, SESSION_CHANNELS, NIAWG_FILTER_FREQENCY)))
		{
			return -1;
		}

		/// Configure Clock input
		// uncomment for high resolution mode
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureClockMode(eSessionHandle, NIFGEN_VAL_HIGH_RESOLUTION)))
		{
			return -1;
		}
		// uncomment for default onboard clock
		// if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureSampleClockSource(eSessionHandle, "OnboardClock"));
		// Unccoment for using an external clock as a "sample clock"
		// if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureSampleClockSource(eSessionHandle, "ClkIn"));

		// Uncomment for using an external clock as a reference clock
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureReferenceClock(eSessionHandle, "ClkIn", 10000000)))
		{
			return -1;
		}
	}
	// Use this section of code to output some characteristics of the 5451. If you want.
	/*
	ViInt32 maximumNumberofWaveforms, waveformQuantum, minimumWaveformSize, maximumWaveformSize;

	if (myNIAWG::NIAWG_CheckWindowsError(niFgen_QueryArbWfmCapabilities(eSessionHandle, &maximumNumberofWaveforms, &waveformQuantum, &minimumWaveformSize, &maximumWaveformSize)))
	{
		return -1;
	}

	MessageBox(NULL, (LPCSTR)std::to_string(maximumNumberofWaveforms).c_str(), NULL, MB_OK);
	MessageBox(NULL, (LPCSTR)std::to_string(waveformQuantum).c_str(), NULL, MB_OK);
	MessageBox(NULL, (LPCSTR)std::to_string(minimumWaveformSize).c_str(), NULL, MB_OK);
	MessageBox(NULL, (LPCSTR)std::to_string(maximumWaveformSize).c_str(), NULL, MB_OK);
	*/

	// first line of every script is script "name".
	default_hConfigScriptString = "script DefaultHConfigScript\n";
	default_vConfigScriptString = "script DefaultVConfigScript\n";
	/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	///					Load Default Waveforms
	///

	int defPredWaveformCount = 0;
	// analyze the input files and create the xy-script. Originally, I thought I'd write the script in two parts, the x and y parts, but it turns out not to 
	// work like I thought it did. If  I'd known this from the start, I probably wouldn't have created this subroutine, except perhaps for the fact that it get 
	// called recursively by predefined scripts in the instructions file.
	/// Create Horizontal Configuration
	eProfile.setOrientation(HORIZONTAL_ORIENTATION);
	std::vector<variable> noSingletons;
	if (myErrorHandler(myNIAWG::analyzeNIAWGScripts(default_hConfigVerticalScriptFile[0], default_hConfigHorizontalScriptFile[0], default_hConfigScriptString, TRIGGER_NAME, waveformCount, eSessionHandle, SESSION_CHANNELS,
											   eError, defXPredWaveformNames, defYPredWaveformNames, defPredWaveformCount, defPredWaveLocs, libWaveformArray,
											   fileOpenedStatus, allXWaveformParameters, xWaveformIsVaried, allYWaveformParameters, yWaveformIsVaried, true, false, "", noSingletons),
		"", ConnectSocket, default_hConfigVerticalScriptFile, default_hConfigHorizontalScriptFile, false, eError, eSessionHandle, userScriptIsWritten, "", false, false)
		== true)
	{
		MessageBox(0, "FATAL ERROR: Creation of Default Waveforms and Default Script Has Failed!", 0, MB_OK);
		return -1;
	}
	// the script file must end with "end script".
	default_hConfigScriptString += "end Script";
	// Convert script string to ViConstString. +1 for a null character on the end.
	eDefault_hConfigScript = new ViChar[default_hConfigScriptString.length() + 1];
	sprintf_s(eDefault_hConfigScript, default_hConfigScriptString.length() + 1, "%s", default_hConfigScriptString.c_str());
	strcpy_s(eDefault_hConfigScript, default_hConfigScriptString.length() + 1, default_hConfigScriptString.c_str());
	
	eProfile.setOrientation(VERTICAL_ORIENTATION);

	if (myErrorHandler(myNIAWG::analyzeNIAWGScripts(default_vConfigVerticalScriptFile[0], default_vConfigHorizontalScriptFile[0], default_vConfigScriptString, TRIGGER_NAME, waveformCount, eSessionHandle, SESSION_CHANNELS,
		eError, defXPredWaveformNames, defYPredWaveformNames, defPredWaveformCount, defPredWaveLocs, libWaveformArray,
		fileOpenedStatus, allXWaveformParameters, xWaveformIsVaried, allYWaveformParameters, yWaveformIsVaried, true, false, "", noSingletons),
		"", ConnectSocket, default_vConfigVerticalScriptFile, default_vConfigHorizontalScriptFile, false, eError, eSessionHandle, userScriptIsWritten, "", false, false)
		== true)
	{
		MessageBox(0, "FATAL ERROR: Creation of Default Waveforms and Default Script Has Failed!", 0, MB_OK);
		return -1;
	}
	// the script file must end with "end script".
	default_vConfigScriptString += "end Script";
	// Convert script string to ViConstString. +1 for a null character on the end.
	eDefault_vConfigScript = new ViChar[default_vConfigScriptString.length() + 1];
	sprintf_s(eDefault_vConfigScript, default_vConfigScriptString.length() + 1, "%s", default_vConfigScriptString.c_str());
	strcpy_s(eDefault_vConfigScript, default_vConfigScriptString.length() + 1, default_vConfigScriptString.c_str());

	// but the default starts in the horizontal configuration, so switch back and start in this config.
	eProfile.setOrientation(HORIZONTAL_ORIENTATION);
	// default value of this variable.
	eDontActuallyGenerate = false;
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		// write script to NIAWG
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigScript)))
		{
			return -1;
		}
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE)))
		{
			return -1;
		}
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultHConfigScript")))
		{
			return -1;
		}
	}
	eCurrentScript = "DefaultScript";
	// Initiate Generation.
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_InitiateGeneration(eSessionHandle)))
		{
			return -1;
		}
	}
	// not done with the script, it will not stay on the NIAWG, so I need to keep track of it so thatI can reload it onto the NIAWG when necessary.



	/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	///			MAIN LOOP
	///

	/// Initialization
	// Communication object used to open up the windows socket applications (WSA) DLL. 
	WSADATA wsaData;
	// object that contains error information.
	int iResult;
	// the socket object used to connect to the other computer. Starts invalid because it isn't active yet.

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	// check for errors initializing winsock
	if (iResult != 0) {
		char tempChar[300];
		sprintf_s(tempChar, "WSAStartup failed: %d\r\n", iResult);
		MessageBox(NULL, tempChar, NULL, MB_OK);
		return 1;
	}

	// catch for the first time you run through this when you will be running the default script going through here. 
	cleanSocket(ConnectSocket, false);
	myAgilent::agilentDefault();
	std::string msgString = "Passively Outputting Default Waveform";
	SetWindowText(eColoredStatusEdit, msgString.c_str());
	eDontActuallyGenerate = false;
	eGenStatusColor = "B";
	RedrawWindow(eColoredStatusEdit, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	RedrawWindow(eColorBox, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);

	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

	MainWindow MainWin;
	ScriptingWindow ScriptWin;
	BOOL ret = MainWin.Create(IDD_LARGE_TEMPLATE, 0);
	if (!ret)   //Create failed.
	{
		errBox("Error creating Dialog");
	}
	MainWin.ShowWindow(SW_SHOW);
	ScriptWin.Create(IDD_LARGE_TEMPLATE, 0);

	/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	///				User Interaction Loop
	///
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAcceleratorA(eScriptingWindowHandle, hAccel, &msg) && !TranslateAccelerator(eMainWindowHandle, hAccel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	/// /////////////////////////////////!@#$!@#$@!#$!@#$!@#$@!#$@!#$@!#$@!#$!@#$!@#$@#!$@!#$!@#$@!#$@!#$@!#$!@#$@!#$
	// Check if the user wants to exit
	if (msg.wParam == 1)
	{
		delete[] eDefault_hConfigScript;
		delete[] eDefault_vConfigScript;
		if (!TWEEZER_COMPUTER_SAFEMODE)
		{
			if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_FALSE)))
			{
				return -1;
			}
			// Officially stop trying to generate anything.
			if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AbortGeneration(eSessionHandle)))
			{
				return -1;
			}
		}
		delete eDefault_hConfigMixedWaveform;
		delete eDefault_vConfigMixedWaveform;
		return 0;
	}
	return 0;
}
