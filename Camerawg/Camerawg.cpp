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
#include <afxsock.h>

// contains the boost function for finding a common factor.
#include "boost/math/common_factor.hpp"
// contains stuff I use for file IO.
#include <boost/filesystem.hpp>
#include "myApplicationApp.h"
#include "commonFunctions.h"
// This is used to tell the compiler that this specific library is needed.
#pragma comment(lib, "Ws2_32.lib")


BOOL myApplicationApp::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			theMainApplicationWindow.passCommonCommand(ID_ACCELERATOR_ESC);
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
		if (::TranslateAcceleratorA( this->theMainApplicationWindow.m_hWnd, m_haccel, lpMsg ))
		{
			return(TRUE);
		}
	}
	return CWinApp::ProcessMessageFilter(code, lpMsg);
}


BOOL myApplicationApp::InitInstance()
{
	if (NIAWG_SAFEMODE)
	{
		errBox("Starting in Safe Mode. The program will not actually communicate with any of the devices");
	}
	// Contains all of of the names of the files that hold actual data file names.
	for (auto number : range( MAX_NIAWG_SIGNALS ))
	{
		WAVEFORM_NAME_FILES[number] = "gen " + str( number + 1 ) + ", const waveform file names.txt";
		WAVEFORM_NAME_FILES[number + MAX_NIAWG_SIGNALS] = "gen " + str( number + 1 ) + ", amp ramp waveform file names.txt";
		WAVEFORM_NAME_FILES[number + 2 * MAX_NIAWG_SIGNALS] = "gen " + str( number + 1 ) + ", freq ramp waveform file names.txt";
		WAVEFORM_NAME_FILES[number + 3 * MAX_NIAWG_SIGNALS] = "gen " + str( number + 1 ) + ", freq & amp ramp waveform file names.txt";

		WAVEFORM_TYPE_FOLDERS[number] = "gen" + str( number + 1 ) + "const\\";
		WAVEFORM_TYPE_FOLDERS[number + MAX_NIAWG_SIGNALS] = "gen" + str( number + 1 ) + "ampramp\\";
		WAVEFORM_TYPE_FOLDERS[number + 2 * MAX_NIAWG_SIGNALS] = "gen" + str( number + 1 ) + "freqramp\\";
		WAVEFORM_TYPE_FOLDERS[number + 3 * MAX_NIAWG_SIGNALS] = "gen" + str( number + 1 ) + "ampfreqramp\\";
	}

	/// Other General Initializations
	// Check to make sure that the gain hasn't been defined to be too high.
	if (GAIN > MAX_GAIN)
	{
		errBox( "FATAL ERROR: GAIN SET TOO HIGH. Driving too much power into the AOMs could severaly damage the experiment!\r\n" );
		return -10000;
	}

	///				Outputting the code version to the Andor Computer 
	// get time now
	time_t dateStart = time( 0 );
	struct tm datePointerStart;
	localtime_s( &datePointerStart, &dateStart );
	std::string logFolderNameStart = ("Date " + str( datePointerStart.tm_year + 1900 ) + "-" + str( datePointerStart.tm_mon + 1 ) + "-"
									   + str( datePointerStart.tm_mday ) + " Time " + str( datePointerStart.tm_hour ) + "-" 
									   + str( datePointerStart.tm_min ) + "-" + str( datePointerStart.tm_sec ));
	bool andorConnectedForFolder = false;

	if (!CONNECT_TO_ANDOR_SAFEMODE)
	{
		boost::filesystem::path dir( CODE_LOGGING_FILES_PATH + logFolderNameStart );
		do
		{
			try
			{
				boost::filesystem::create_directory(dir);
				andorConnectedForFolder = true;
			}
			catch (boost::filesystem::filesystem_error& err)
			{
				// For some reason this doesn't seem to get called when the connection breaks.
				int andorDisconnectedOption = MessageBox( NULL, "This computer can't currently open logging files on the andor.\nAbort "
														  "will quit the program (no output has started).\nRetry will re-attempt to "
														  "connect to the Andor.\nIgnore will continue without saving the current file.",  
														  "Andor Disconnected", MB_ABORTRETRYIGNORE );
				switch (andorDisconnectedOption)
				{
					case IDABORT:
					{
						return -2;
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
		} while (andorConnectedForFolder == false);
	}
	logFolderNameStart += "\\";

	WIN32_FIND_DATA find_cpp_Data;
	HANDLE cpp_Find_Handle;
	WIN32_FIND_DATA find_h_Data;
	HANDLE h_Find_Handle;
	std::string cppFindString = ACTUAL_CODE_FOLDER_PATH + "*.cpp";
	std::string hFindString = ACTUAL_CODE_FOLDER_PATH + "*.h";
	if (!NIAWG_SAFEMODE)
	{
		cpp_Find_Handle = FindFirstFile( (LPSTR)cppFindString.c_str(), &find_cpp_Data );
		if (cpp_Find_Handle != INVALID_HANDLE_VALUE)
		{
			do
			{
				std::ifstream originalCodeFile( ACTUAL_CODE_FOLDER_PATH + find_cpp_Data.cFileName );
				if (originalCodeFile.is_open() == false)
				{
					thrower( "Error encountered when trying to use FindFirstFile or FindNextFile. The functions returned names to files "
							 "that don't exist." );
				}
				bool andorConnected = false;
				do
				{
					std::ofstream andorCodeFileCopy( CODE_LOGGING_FILES_PATH + logFolderNameStart + find_cpp_Data.cFileName );
					if (andorCodeFileCopy.is_open() == false)
					{
						int andorDisconnectedOption = MessageBox( NULL, "This computer can't currently open logging files on the andor."
																  "\nAbort will quit the program (no output has started).\nRetry will "
																  "re-attempt to connect to the Andor.\nIgnore will continue "
																  "without saving the current file.", "Andor Disconnected",
																  MB_ABORTRETRYIGNORE );
						switch (andorDisconnectedOption)
						{
							case IDABORT:
							{
								return -2;
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
			} while (FindNextFile( (LPSTR)cpp_Find_Handle, &find_cpp_Data ));
		}
		else
		{
			errBox( "Failed to find any .cpp files in folder!" );
		}

		h_Find_Handle = FindFirstFile( (LPSTR)hFindString.c_str(), &find_cpp_Data );
		if (h_Find_Handle != INVALID_HANDLE_VALUE)
		{
			do
			{
				std::ifstream originalCodeFile( ACTUAL_CODE_FOLDER_PATH + find_h_Data.cFileName );
				std::ofstream andorCodeFileCopy( CODE_LOGGING_FILES_PATH + logFolderNameStart + find_h_Data.cFileName );
				andorCodeFileCopy << originalCodeFile.rdbuf();
			} while (FindNextFile( (LPSTR)h_Find_Handle, &find_h_Data ));
		}
		else
		{
			errBox( "Failed to find any .h files in folder!" );
		}
	}

	m_haccel = LoadAccelerators( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDR_ACCELERATOR1 ) );

	/// Initialize Socket stuffs
	// Communication object used to open up the windows socket applications (WSA) DLL. 
	//WSADATA wsaData;
	// object that contains error information.
	int iResult = 0;
	// the socket object used to connect to the other computer. Starts invalid because it isn't active yet.

	// Initialize Winsock
	//iResult = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
	// check for errors initializing winsock
	if (iResult != 0)
	{
		errBox( "WSAStartup failed: " + str( iResult ) );
		return 1;
	}
	myAgilent::agilentDefault();
	INT_PTR returnVal = theMainApplicationWindow.DoModal();
	// end of program.
	return returnVal;
}

myApplicationApp app;
