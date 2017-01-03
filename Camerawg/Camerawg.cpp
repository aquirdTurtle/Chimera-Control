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
// cleans up parameters related to a socket
#include "cleanSocket.h"
// an namespace for agilent functions.
#include "myAgilent.h"
//
#include "myErrorHandler.h"

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
#include "myApplicationApp.h"
#include "commonFunctions.h"
// This is used to tell the compiler that this specific library is needed.
#pragma comment(lib, "Ws2_32.lib")
//#include <vld.h> 

BOOL myApplicationApp::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			this->theMainApplicationWindow.passCommonCommand(ID_ACCELERATOR_ESC);
			// Do not process further
			return TRUE;
		}
	}
	return CWinApp::PreTranslateMessage(pMsg);
}

BOOL myApplicationApp::ProcessMessageFilter(int code, LPMSG lpMsg)
{
	if (code >= 0 && theMainApplicationWindow && m_haccel)
	{
		if (::TranslateAcceleratorA(this->theMainApplicationWindow.m_hWnd, m_haccel, lpMsg))
			return(TRUE);
	}

	return CWinApp::ProcessMessageFilter(code, lpMsg);
}


/// /////////////////////////////////////////////////////////////////////////////////////////
/// The main function for win32 programs. The main programming sequence for this application.
/// /////////////////////////////////////////////////////////////////////////////////////////
BOOL myApplicationApp::InitInstance()
{
	if (TWEEZER_COMPUTER_SAFEMODE)
	{
		MessageBox(0, "Starting in Safe Mode. The program will not actually communicate with any of the devices", 0, MB_OK);
	}
	// Socket object for communicating with the other computer.
	SOCKET ConnectSocket = INVALID_SOCKET;
	// An array of variable files. Only used if not receiving variable information from the master computer.
	std::vector<std::fstream> xVarFiles;
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
	// A variable that records whether a given experiment has completed or not.
	ViBoolean isDoneTest = VI_FALSE;
	// This array stores the waveform #s of the predefined waveforms.
	std::vector<int> defPredWaveLocs;
	
	/// Other General Initializations
	// Check to make sure that the gain hasn't been defined to be too high.
	if (GAIN > MAX_GAIN)
	{
		MessageBox(0, "FATAL ERROR: GAIN SET TOO HIGH. Driving too much power into the AOMs could severaly damage the experiment!\r\n", 0, MB_OK);
		return -10000;
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

	m_haccel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR1));

	/// Initialize Socket stuffs
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
	myAgilent::agilentDefault();
	eDontActuallyGenerate = false;
	INT_PTR returnVal = theMainApplicationWindow.DoModal();
	// end of program.
	return 0;
}

myApplicationApp app;
