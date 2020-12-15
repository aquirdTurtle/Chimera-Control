// created by Mark O. Brown
#pragma once
#include "ExperimentThread/ExperimentThreadInput.h"
#include "ExperimentThread/AllExperimentInput.h"
#include "AnalogInput/servoInfo.h"

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
	void prepareMasterThread( int msgID, IChimeraQtWindow* win, AllExperimentInput& input, bool runTtls,
							  bool runAndor, bool runBasler, bool startPlotThread );
	void startExperimentThread (IChimeraQtWindow* win, AllExperimentInput& input);
	void logStandard( AllExperimentInput input, DataLogger& logger, std::string specialName="", bool needsCal=false );
	void abortCamera(IChimeraQtWindow* win);
	void abortMaster(IChimeraQtWindow* win);
	void forceExit (IChimeraQtWindow* win);
	void exitProgram(IChimeraQtWindow* win);
	bool getPermissionToStart(IChimeraQtWindow* win, bool runMaster, AllExperimentInput& input );
	/// Scripting Menu
}


