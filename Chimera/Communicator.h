#pragma once
#include "ColorBox.h"

class MainWindow;
class ScriptingWindow;
class AndorWindow;
class AuxiliaryWindow;

enum class System
{
	Niawg,
	Master,
	Camera,
	Basler
};

/*
	* This class was desiged to hold a pointer to the main window that can only be used for communicating messages to said 
	* window. It prevents the need to pass the raw pointer to the main window which a small function shouldn't need.
*/
class Communicator
{
	public:
		void initialize( MainWindow* mainWinParent, ScriptingWindow* scriptingWin, AndorWindow* cameraWin,
						 AuxiliaryWindow* masterWindow);
		void sendErrorEx( std::string statusMsg, const char *file, int line );
		void sendFatalErrorEx( std::string statusMsg, const char *file, int line );
		void sendStatus( std::string statusMsg );
		void sendDebug( std::string statusMsg );
		void sendTimer( std::string timerMsg );
		void sendColorBox( System sys, char code ); 
		void sendColorBox( systemInfo<char> colors );
		void sendCameraCalFin( );
		void sendCameraFin();
		void sendCameraProgress( long prog );
		void sendCameraCalProgress( long progress );
		void sendBaslerFin ( );
		void sendRepProgress(ULONG rep);
		void sendNoAtomsAlert( );

		void sendMachineOptimizationRoundFinish ( );
		void sendMotCalFinish ( );
		void sendNormalFinish ( );
		void sendMotFinish( );
	private:
		MainWindow* mainWin;
		ScriptingWindow* scriptWin;
		AndorWindow* camWin;
		AuxiliaryWindow* auxWin;
		void postMyString( CWnd* window, UINT messageTypeID, std::string message );
};

// macros to include file and line info in error messages. Use these, not the associated functions directly.
#define sendError(arg1) sendErrorEx(arg1, __FILE__, __LINE__)
#define sendFatalError(arg1) sendFatalErrorEx(arg1, __FILE__, __LINE__)

