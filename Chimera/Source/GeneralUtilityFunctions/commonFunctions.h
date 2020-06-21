// created by Mark O. Brown
#pragma once
#include "afxwin.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "ExperimentThread/AllExperimentInput.h"
#include "AnalogInput/servoInfo.h"

class MainWindow;
class AndorWindow;
class AuxiliaryWindow;
class DeformableMirrorWindow;
class ScriptingWindow;
class BaslerWindow;
class IChimeraWindow;
class DataLogger;

namespace commonFunctions
{
	/// Call to direct message to appropriate function in this namespace
	void handleCommonMessage( int msgID, IChimeraWindowWidget* win);
	/// Run Menu
	void calibrateCameraBackground (IChimeraWindowWidget* win);
	void prepareMasterThread( int msgID, IChimeraWindowWidget* win, AllExperimentInput& input, bool runNiawg, bool runTtls,
							  bool runAndor, bool runBasler, bool startPlotThread );
	void startExperimentThread (IChimeraWindowWidget* win, AllExperimentInput& input);
	void logStandard( AllExperimentInput input, DataLogger& logger, std::vector<servoInfo> servos, 
					  std::string specialName="", bool needsCal=false );
	void abortNiawg(IChimeraWindowWidget* win);
	void abortCamera(IChimeraWindowWidget* win);
	void abortMaster(IChimeraWindowWidget* win);
	void forceExit (IChimeraWindowWidget* win);
	void exitProgram(IChimeraWindowWidget* win);
	bool getPermissionToStart(IChimeraWindowWidget* win, bool runNiawg, bool runMaster, AllExperimentInput& input );
	/// Scripting Menu
	void reloadNIAWGDefaults( QtMainWindow* mainWin, QtScriptWindow* scriptWin);
}


