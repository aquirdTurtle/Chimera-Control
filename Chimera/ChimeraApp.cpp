// created by Mark O. Brown
/****************************************
*										*
* Chimera-Control.cpp					*
*										*
******************************************/

// a file visual c++ uses for efficiency in compiling headers.
#include "stdafx.h"
// Contains some user-defined global parameters and options used throughout the code.
#include "constants.h"
// an namespace for agilent functions.

// Contains functions and types used by the NIAWG.
#include "niFgen.h"

// Contains information the API uses for dialogues.
#include "resource.h"
#include "externals.h"

// Used to get a precise (milisecond-level) time from the computer. Also for windows API functions & declarations
#include <Windows.h>

// Some headers used for communication protocols.
#include <winsock2.h>
#include <ws2tcpip.h>

// Some general use headers.
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <afxsock.h>
#include "gdiplus.h"
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
		if ( theMainApplicationWindow.handleAccelerators( m_haccel, lpMsg ) )
		{
			return (TRUE);
		}
		if (::TranslateAcceleratorA( this->theMainApplicationWindow.m_hWnd, m_haccel, lpMsg ))
		{
			return(TRUE);
		}
	}
	return CWinApp::ProcessMessageFilter(code, lpMsg);
}


BOOL ChimeraApp::InitInstance()
{
	initTime = chronoClock::now();
	splash->Create(IDD_SPLASH);
	splash->ShowWindow( SW_SHOW );
	/// initialize some stuff
	initMyColors ( );
	Gdiplus::GdiplusStartupInput input;
	Gdiplus::GdiplusStartup( &gdip_token, &input, NULL );
	// Check to make sure that the gain hasn't been defined to be too high.
	if (NIAWG_GAIN > MAX_GAIN)
	{
		errBox( "FATAL ERROR: NIAWG_GAIN SET TOO HIGH. Driving too much power into the AOMs could severaly damage the "
				"experiment!\r\n" );
		return -10000;
	}
 	m_haccel = LoadAccelerators( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDR_ACCELERATOR1 ) );
	
	INT_PTR returnVal = theMainApplicationWindow.DoModal();
	// end of program.
	std::chrono::high_resolution_clock::now();
	return int(returnVal);
}


BOOL ChimeraApp::ExitInstance( )
{
	Gdiplus::GdiplusShutdown( gdip_token );
	return CWinAppEx::ExitInstance( );
}

// Upon starting the program, the program finds and uses the following global object based on it being the one object 
// derived from CWinApp. 
// see this page for a nice explanation:
// https://www.codeproject.com/Articles/1672/MFC-under-the-hood
ChimeraApp app;

