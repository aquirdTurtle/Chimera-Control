// created by Mark O. Brown
#pragma once
#include "ExperimentThread/ExperimentThreadInput.h"
#include "ExperimentThread/AllExperimentInput.h"
#include "AnalogInput/servoInfo.h"
#include <Piezo/piezoChan.h>

class MainWindow;
class ScriptingWindow;
class BaslerWindow;
class IChimeraQtWindow;
class DataLogger;

namespace commonFunctions{
	/// Call to direct message to appropriate function in this namespace
	void handleCommonMessage( int msgID, IChimeraQtWindow* win);
	/// Run Menu
	void calibrateCameraBackground (IChimeraQtWindow* win);
	void prepareMasterThread( int msgID, IChimeraQtWindow* win, AllExperimentInput& input, bool runNiawg, bool runTtls,
							  bool runAndor, bool runBasler, bool startPlotThread );
	void startExperimentThread (IChimeraQtWindow* win, AllExperimentInput& input);
	void logStandard( AllExperimentInput input, DataLogger& logger, piezoChan<double> cameraPiezoVals,
					  std::string specialName="", bool needsCal=false );
	void abortNiawg(IChimeraQtWindow* win);
	void abortCamera(IChimeraQtWindow* win);
	void abortMaster(IChimeraQtWindow* win);
	void forceExit (IChimeraQtWindow* win);
	void exitProgram(IChimeraQtWindow* win);
	bool getPermissionToStart(IChimeraQtWindow* win, bool runNiawg, bool runMaster, AllExperimentInput& input );
	/// Scripting Menu
	void reloadNIAWGDefaults( QtMainWindow* mainWin, QtScriptWindow* scriptWin);
}


