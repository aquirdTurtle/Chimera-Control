// created by Mark O. Brown
#pragma once

#include "Andor/AndorRunSettings.h"
#include "Basler/baslerSettings.h"
#include "afxwin.h"
//#include <ExperimentThread/ExpThreadWorker.h>
//#include <PrimaryWindows/IChimeraWindowWidget.h>

class IChimeraWindowWidget;
class QtMainWindow;
class QtScriptWindow;
class QtAndorWindow;
class QtAuxiliaryWindow;
class QtDeformableMirrorWindow;
class QtBaslerWindow;
class ExpThreadWorker;

enum class ExperimentType;
/*
	* This class was desiged to hold a pointer to the main window that can only be used for communicating messages to said 
	* window. It prevents the need to pass the raw pointer to the main window which a small function shouldn't need.
*/
class Communicator
{
	public:
		void sendFinish ( ExperimentType type ); 
		void initialize ( IChimeraWindowWidget* win);

		void sendErrorEx( std::string statusMsg, const char *file, int line );
		void sendFatalErrorEx( std::string statusMsg, const char *file, int line );

		void sendStatus( std::string statusMsg );
		void sendDebug( std::string statusMsg );
		void sendTimer( std::string timerMsg );
		//void sendColorBox( std::string sysDelim, std::string color);
		void sendColorBox (std::string sysDelim, std::string color, ExpThreadWorker* threadManager);

		void sendCameraProgress ( long prog );
		void sendCameraCalProgress ( long progress );
		void sendRepProgress ( ULONG rep );
		void sendLogVoltsMessage ( UINT variation );
		
		void sendPrepareAndor (AndorRunSettings& settingsToPrepare);
		void sendPrepareBasler (baslerSettings& settingsToPrepare);
		
		void sendCameraCalFin( );
		void sendCameraFin();
		void sendBaslerFin ( );
		void sendNoAtomsAlert( );
		void sendNoMotAlert ( );

		void sendExperimentProcedureFinish ( );

		void sendAutoServo( );
		bool expQuiet=false;

		void expUpdate (std::string updateTxt);
		std::string warnings;
		std::string debugStr;
		void postDoubleString (CWnd* window, UINT messageTypeID, std::string message, std::string msg2);
	private:
		QtMainWindow* mainWin;
		QtScriptWindow* scriptWin;
		QtAndorWindow* andorWin;
		QtAuxiliaryWindow* auxWin;
		QtBaslerWindow* basWin;
		QtDeformableMirrorWindow* dmWin;
		void postMyString( CWnd* window, UINT messageTypeID, std::string message );
};

// macros to include file and line info in error messages. Use these, not the associated functions directly.
#define sendError(arg1) sendErrorEx(arg1, __FILE__, __LINE__)
#define sendFatalError(arg1) sendFatalErrorEx(arg1, __FILE__, __LINE__)

