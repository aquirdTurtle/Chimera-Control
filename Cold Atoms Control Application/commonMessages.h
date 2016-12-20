#pragma once
#include "Windows.h"

class MainWindow;

namespace commonMessages 
{
	/// Call to direct message to appropriate function in this namespace
	bool handleCommonMessage(int msgID, CWnd* parent, MainWindow* comm, ScriptingWindow* scriptWin);
	/// Run Menu
	int startSystem(HWND parentWindow, int msgID, ScriptingWindow* scriptWindow, MainWindow* comm);
	int abortSystem(HWND parentWindow, ScriptingWindow* scriptWin, MainWindow* comm);
	/// File Menu
	int saveAll(HWND parentWindow);
	int exitProgram(HWND parentWindow, ScriptingWindow* scriptWindow, MainWindow* comm);
	/// Scripting Menu
	int saveProfile(HWND parentWindow, ScriptingWindow* scriptWindow, MainWindow* comm);
	
	int helpWindow(HWND parentWindow);

	int reloadNIAWGDefaults(HWND parentWindow, profileSettings profileInfo, MainWindow* comm);
};
