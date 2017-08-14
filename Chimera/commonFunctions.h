#pragma once
#include "Windows.h"

class MainWindow;
class CameraWindow;
class DeviceWindow;

namespace commonFunctions
{
	/// Call to direct message to appropriate function in this namespace
	void handleCommonMessage( int msgID, CWnd* parent, MainWindow* comm, ScriptingWindow* scriptWin, 
							 CameraWindow* camWin, DeviceWindow* deviceWin );
	/// Run Menu
	void startCamera( ScriptingWindow* scriptWindow, MainWindow* mainWin, CameraWindow* camWin );
	void startMasterOnly( MainWindow* mainWin, DeviceWindow* deviceWin, ScriptingWindow* scriptWin );
	void startNiawgOnly( int msgID, ScriptingWindow* scriptWin, MainWindow* mainWin, CameraWindow* camWin,
						 DeviceWindow* deviceWin );
	void startFullMasterThread( MainWindow* mainWin, DeviceWindow* deviceWin, ScriptingWindow* scriptWin );
	void setMot(MainWindow* mainWin);
	void abortNiawg( ScriptingWindow* scriptWin, MainWindow* mainWin );
	void abortCamera( CameraWindow* camWin, MainWindow* mainWin );
	void abortMaster(MainWindow* mainWin, DeviceWindow* deviceWin);
	void exitProgram( ScriptingWindow* scriptWindow, MainWindow* mainWin, CameraWindow* camWin );
	/// Scripting Menu
	int saveProfile( ScriptingWindow* scriptWindow, MainWindow* mainWin );
	void helpWindow();
	void reloadNIAWGDefaults( MainWindow* mainWin );
}


