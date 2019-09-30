// created by Mark O. Brown
#pragma once
#include "afxwin.h"
#include "MasterThreadInput.h"

class MainWindow;
class AndorWindow;
class AuxiliaryWindow;
class ScriptingWindow;
class BaslerWindow;
class DataLogger;

namespace commonFunctions
{
	/// Call to direct message to appropriate function in this namespace
	void handleCommonMessage( int msgID, CWnd* parent, MainWindow* mainWin, ScriptingWindow* scriptWin, 
							 AndorWindow* camWin, AuxiliaryWindow* auxWin, BaslerWindow* basWin );
	/// Run Menu
	void calibrateCameraBackground( ScriptingWindow* scriptWin, MainWindow* mainWin, AndorWindow* camWin,
									AuxiliaryWindow* auxWin );
	void prepareMasterThread( int msgID, ScriptingWindow* scriptWin, MainWindow* mainWin, AndorWindow* camWin,
							  AuxiliaryWindow* auxWin, AllExperimentInput& input, bool runNiawg, bool runTtls, 
							  bool runAndor, bool runBasler, bool startPlotThread );
	void startExperimentThread(MainWindow* mainWin, AllExperimentInput& input);
	void logParameters( AllExperimentInput input, DataLogger& logger, std::string specialName="", bool needsCal=true );
	void setMot ( MainWindow* mainWin, AuxiliaryWindow* auxWin );
	void abortNiawg( ScriptingWindow* scriptWin, MainWindow* mainWin );
	void abortCamera( AndorWindow* camWin, MainWindow* mainWin );
	void abortMaster(MainWindow* mainWin, AuxiliaryWindow* auxWin);
	void forceExit ( ScriptingWindow* scriptWindow, MainWindow* mainWin, AndorWindow* camWin, AuxiliaryWindow* auxWin );
	void exitProgram( ScriptingWindow* scriptWindow, MainWindow* mainWin, AndorWindow* camWin, AuxiliaryWindow* auxWin );
	bool getPermissionToStart( AndorWindow* camWin, MainWindow* mainWin, ScriptingWindow* scriptWin,
							   AuxiliaryWindow* auxWin, bool runNiawg, bool runMaster, AllExperimentInput& input );
	/// Scripting Menu
	void reloadNIAWGDefaults( MainWindow* mainWin );

}


