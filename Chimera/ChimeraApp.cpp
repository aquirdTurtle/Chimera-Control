/****************************************
*										*
* Chimera-Control.cpp					*
*										*
*****************************************

/**PROJECT ORGANIZATION****************************************\

\**************************************************************/

// a file visual c++ uses for efficiency in compiling headers.
#include "stdafx.h"
// Contains some user-defined global parameters and options used throughout the code.
#include "constants.h"
#include "externals.h"
// an namespace for agilent functions.

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
#include "ChimeraApp.h"
#include "commonFunctions.h"
// This is used to tell the compiler that this specific library is needed.
#pragma comment(lib, "Ws2_32.lib")


BOOL ChimeraApp::PreTranslateMessage(MSG* pMsg)
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


BOOL ChimeraApp::ProcessMessageFilter(int code, LPMSG lpMsg)
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



BOOL ChimeraApp::InitInstance()
{
	splash->Create(IDD_SPLASH);
	splash->ShowWindow( SW_SHOW );
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
	if (NIAWG_GAIN > MAX_GAIN)
	{
		errBox( "FATAL ERROR: NIAWG_GAIN SET TOO HIGH. Driving too much power into the AOMs could severaly damage the "
				"experiment!\r\n" );
		return -10000;
	}

	// Outputting the code version to the \D: Drive for logging. I've commented this out for now, but could be easily
	// revived if wanted. Git does a great job in versioning though.
	if (false)
	{
		// get time now
		/*
		time_t dateStart = time( 0 );
		struct tm datePointerStart;
		localtime_s( &datePointerStart, &dateStart );
		std::string logFolderNameStart = ("Date " + str( datePointerStart.tm_year + 1900 ) + "-" + str( datePointerStart.tm_mon + 1 ) + "-"
										   + str( datePointerStart.tm_mday ) + " Time " + str( datePointerStart.tm_hour ) + "-"
										   + str( datePointerStart.tm_min ) + "-" + str( datePointerStart.tm_sec ));
		logFolderNameStart += "\\";
		WIN32_FIND_DATA find_cpp_Data;
		HANDLE cpp_Find_Handle;
		WIN32_FIND_DATA find_h_Data;
		HANDLE h_Find_Handle;
		std::string cppFindString = ACTUAL_CODE_FOLDER_PATH + "*.cpp";
		std::string hFindString = ACTUAL_CODE_FOLDER_PATH + "*.h";
		int result = promptBox("Would you like to copy the code files in their current state for logging?", MB_YESNO );
		if (result == IDYES)
		{
			cpp_Find_Handle = FindFirstFile( (LPSTR)cstr( cppFindString ), &find_cpp_Data );
			if (cpp_Find_Handle != INVALID_HANDLE_VALUE)%
			{
				CreateDirectory( cstr( CODE_LOGGING_FILES_PATH + logFolderNameStart ), NULL );
				do
				{
					int result = CopyFile( cstr( ACTUAL_CODE_FOLDER_PATH + find_cpp_Data.cFileName ),
										   cstr( CODE_LOGGING_FILES_PATH + logFolderNameStart + find_cpp_Data.cFileName ), true );
					if (!result)
					{
						int result = promptBox( "Failed to copy cpp file for logging! Error: " + str( GetLastError() ) + " Continue?", MB_YESNO);
						if (result == IDNO)
						{
							break;
						}
					}
				} while (FindNextFile( (LPSTR)cpp_Find_Handle, &find_cpp_Data ));
			}
			else
			{
				errBox( "Failed to find any .cpp files in folder!" );
			}

			h_Find_Handle = FindFirstFile( (LPSTR)cstr( hFindString ), &find_h_Data );
			if (h_Find_Handle != INVALID_HANDLE_VALUE)
			{
				do
				{
					int result = CopyFile( cstr( ACTUAL_CODE_FOLDER_PATH + find_h_Data.cFileName ),
										   cstr( CODE_LOGGING_FILES_PATH + logFolderNameStart + find_h_Data.cFileName ), true );
					if (!result)
					{
						int result = promptBox( "Failed to copy header file for logging! Error: " + str( GetLastError() ) + " Continue?", MB_YESNO );
						if (result == IDNO)
						{
							break;
						}
					}
				} while (FindNextFile( (LPSTR)h_Find_Handle, &find_h_Data ));
			}
			else
			{
				errBox( "Failed to find any .h files in folder!" );
			}
		}
		*/
	}
 	m_haccel = LoadAccelerators( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDR_ACCELERATOR1 ) );
	INT_PTR returnVal = theMainApplicationWindow.DoModal();
	// end of program.
	return int(returnVal);
}


// Upon starting the program, the program finds and uses the following global object based on it being the one object 
// derived from CWinApp. 
// see this page for a nice explanation:
// https://www.codeproject.com/Articles/1672/MFC-under-the-hood
ChimeraApp app;

