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
	void handleCommonMessage( int msgID, IChimeraWindow* win);
	/// Run Menu
	void calibrateCameraBackground (IChimeraWindow* win);
	void prepareMasterThread( int msgID, IChimeraWindow* win, AllExperimentInput& input, bool runNiawg, bool runTtls,
							  bool runAndor, bool runBasler, bool startPlotThread );
	void startExperimentThread (IChimeraWindow* win, AllExperimentInput& input);
	void logStandard( AllExperimentInput input, DataLogger& logger, std::vector<servoInfo> servos, 
					  std::string specialName="", bool needsCal=false );
	void abortNiawg(IChimeraWindow* win);
	void abortCamera(IChimeraWindow* win);
	void abortMaster(IChimeraWindow* win);
	void forceExit (IChimeraWindow* win);
	void exitProgram(IChimeraWindow* win);
	bool getPermissionToStart(IChimeraWindow* win, bool runNiawg, bool runMaster, AllExperimentInput& input );
	/// Scripting Menu
	void reloadNIAWGDefaults( MainWindow* mainWin, ScriptingWindow* scriptWin);
}


