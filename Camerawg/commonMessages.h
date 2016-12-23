#pragma once
#include "Windows.h"

class MainWindow;
class CameraWindow;

namespace commonMessages 
{
	/// Call to direct message to appropriate function in this namespace
	bool handleCommonMessage(int msgID, CWnd* parent, MainWindow* comm, ScriptingWindow* scriptWin, CameraWindow* camWin);
	/// Run Menu
	int startSystem(HWND parentWindow, int msgID, ScriptingWindow* scriptWindow, MainWindow* mainWin, CameraWindow* camWin);
	int abortSystem(HWND parentWindow, ScriptingWindow* scriptWin, MainWindow* mainWin);
	/// File Menu
	int saveAll(HWND parentWindow);
	int exitProgram(HWND parentWindow, ScriptingWindow* scriptWindow, MainWindow* mainWin);
	/// Scripting Menu
	int saveProfile(HWND parentWindow, ScriptingWindow* scriptWindow, MainWindow* mainWin);
	
	int helpWindow(HWND parentWindow);

	int reloadNIAWGDefaults(HWND parentWindow, profileSettings profileInfo, MainWindow* mainWin);
};
