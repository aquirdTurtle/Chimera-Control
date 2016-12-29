#pragma once
#include "ColorBox.h"


class MainWindow;
class ScriptingWindow;


/*
	* This class was desiged to hold a pointer to the main window that can only be used for communicating messages to said 
	* window. It prevents the need to pass the raw pointer to the main window which a small function shouldn't need.
*/
class Communicator
{
	public:
		void initialize(MainWindow* parent, ScriptingWindow* scriptingWin, CameraWindow* cameraWin);
		void sendErrorEx( std::string statusMsg, const char *file, int line );
		void sendFatalErrorEx( std::string statusMsg, const char *file, int line );
		void sendStatus( std::string statusMsg );
		void sendDebug( std::string statusMsg );
		void sendTimer( std::string timerMsg );
		void sendColorBox( colorBoxes<char> colors );
		void sendCameraFin();
		void sendCameraProgress( long prog );
	private:
		MainWindow* mainWin;
		ScriptingWindow* scriptWin;
		CameraWindow* camWin;
};

// macros to include file and line info in error messages. Use these, not the associated functions directly.
#define sendError(arg1) sendErrorEx(arg1, __FILE__, __LINE__)
#define sendFatalError(arg1) sendFatalErrorEx(arg1, __FILE__, __LINE__)

