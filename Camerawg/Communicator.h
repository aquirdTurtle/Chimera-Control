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
		void sendErrorEx( std::string statusMsg, std::string shortMsg, const char *file, int line, colorBoxes<char> colors = { '-', '-', '-' } );
		void sendFatalErrorEx(std::string statusMsg, std::string shortMsg, const char *file, int line, colorBoxes<char> colors = { '-','-','-' });
		void sendStatus( std::string statusMsg, std::string shortMsg, colorBoxes<char> colors = { '-','-','-' } );
		void sendDebug(std::string statusMsg, std::string shortMsg, colorBoxes<char> colors = { '-','-','-' } );
		void sendTimer( std::string timerMsg, std::string timerColor );
		void sendCameraFin();
		void sendCameraProgress( long prog );
	private:
		MainWindow* mainWin;
		ScriptingWindow* scriptWin;
		CameraWindow* camWin;
};

// macros to include file and line info in error messages, as well as overloads for default cases.
#define sendErrorDef(arg1, arg2) sendErrorEx(arg1, arg2, __FILE__, __LINE__)
#define sendError(arg1, arg2, arg3) sendErrorEx(arg1, arg2, __FILE__, __LINE__, arg3)
#define sendFatalErrorDef(arg1, arg2) sendFatalErrorEx(arg1, arg2,  __FILE__, __LINE__)
#define sendFatalError(arg1, arg2, arg3) sendFatalErrorEx(arg1, arg2,  __FILE__, __LINE__, arg3)
