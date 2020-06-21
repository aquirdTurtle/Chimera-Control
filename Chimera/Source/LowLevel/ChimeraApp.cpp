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
// Contains functions and types used by the NIAWG.
#include "niFgen.h"
// Contains information the API uses for dialogues.
#include "resource.h"
#include "externals.h"
// Some headers used for communication protocols.
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
#include <PrimaryWindows/QtMainWindow.h>
#include "ChimeraApp.h"
#include "GeneralUtilityFunctions/commonFunctions.h"
#include "qmfcapp.h"

// This is used to tell the compiler that this specific library is needed.
// I don't remember why this specific library is needed though. 
#pragma comment(lib, "Ws2_32.lib")

BOOL ChimeraApp::InitInstance(){
	chronoTime initTime = chronoClock::now();
	splash->Create(IDD_SPLASH);
	splash->ShowWindow( SW_SHOW );
	/// initialize some stuff
	initMyColors ( );
	Gdiplus::GdiplusStartupInput input;
	Gdiplus::GdiplusStartup( &gdip_token, &input, NULL );
	// Check to make sure that the gain hasn't been defined to be too high.
	if (NIAWG_GAIN > MAX_GAIN){
		errBox( "FATAL ERROR: NIAWG_GAIN SET TOO HIGH. Driving too much power into the AOMs could severaly damage the "
				"experiment!\r\n" );
		return -10000;
	}
	
	auto inst = QMfcApp::instance (this);
	qRegisterMetaType<QVector<double>> ();
	qRegisterMetaType<std::vector<double>> ();
	qRegisterMetaType<std::vector<std::vector<plotDataVec>>> ();
	qRegisterMetaType<NormalImage> ();
	qRegisterMetaType<atomQueue> ();
	qRegisterMetaType<std::vector<std::vector<dataPoint> >> ();
	qRegisterMetaType<PlottingInfo> ();
	qRegisterMetaType<PixListQueue> ();
	QtMainWindow* mainWinQt = new QtMainWindow((CDialog*)splash, &initTime);
	mainWinQt->show ();
	inst->exec ();
	
	// end of program.
	std::chrono::high_resolution_clock::now();
	return int(0);
}

Q_DECLARE_METATYPE (dataPoint)
Q_DECLARE_METATYPE (std::vector<dataPoint>)
Q_DECLARE_METATYPE (std::vector<std::vector<dataPoint>>)
Q_DECLARE_METATYPE (std::vector<std::vector<plotDataVec>>)
Q_DECLARE_METATYPE (PlottingInfo)


BOOL ChimeraApp::ExitInstance( ){
	Gdiplus::GdiplusShutdown( gdip_token );
	return CWinAppEx::ExitInstance( );
}

// Upon starting the program, the program finds and uses the following global object based on it being the one object 
// derived from CWinApp. 
// see this page for a nice explanation:
// https://www.codeproject.com/Articles/1672/MFC-under-the-hood
ChimeraApp app;

BOOL ChimeraApp::Run ()
{
	return QMfcApp::run (this);
}
