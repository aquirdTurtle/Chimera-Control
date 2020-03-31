// created by Mark O. Brown
#pragma once
#include "ExperimentMonitoringAndStatus/ColorBox.h"
#include "Andor/AndorRunSettings.h"
#include "Basler/baslerSettings.h"
class IChimeraWindow;
class MainWindow;
class ScriptingWindow;
class AndorWindow;
class AuxiliaryWindow;
class DeformableMirrorWindow;
class BaslerWindow;

enum class System
{
	Niawg,
	Master,
	Andor,
	Basler
};

enum class ExperimentType;
/*
	* This class was desiged to hold a pointer to the main window that can only be used for communicating messages to said 
	* window. It prevents the need to pass the raw pointer to the main window which a small function shouldn't need.
*/
class Communicator
{
	public:
		void sendFinish ( ExperimentType type ); 
		void initialize (IChimeraWindow* win);

		void sendErrorEx( std::string statusMsg, const char *file, int line );
		void sendFatalErrorEx( std::string statusMsg, const char *file, int line );

		void sendStatus( std::string statusMsg );
		void sendDebug( std::string statusMsg );
		void sendTimer( std::string timerMsg );
		void sendColorBox( System sys, char code ); 
		void sendColorBox( systemInfo<char> colors );

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
	private:
		MainWindow* mainWin;
		ScriptingWindow* scriptWin;
		AndorWindow* andorWin;
		AuxiliaryWindow* auxWin;
		BaslerWindow* basWin;
		DeformableMirrorWindow* dmWin;
		void postMyString( CWnd* window, UINT messageTypeID, std::string message );
};

// macros to include file and line info in error messages. Use these, not the associated functions directly.
#define sendError(arg1) sendErrorEx(arg1, __FILE__, __LINE__)
#define sendFatalError(arg1) sendFatalErrorEx(arg1, __FILE__, __LINE__)

