#pragma once
#include "afxwin.h"
#include "MasterThreadInput.h"

class MainWindow;
class AndorWindow;
class AuxiliaryWindow;
class ScriptingWindow;
class BaslerWindow;

namespace commonFunctions
{
	/// Call to direct message to appropriate function in this namespace
	void handleCommonMessage( int msgID, CWnd* parent, MainWindow* comm, ScriptingWindow* scriptWin, 
							 AndorWindow* camWin, AuxiliaryWindow* auxWin, BaslerWindow* basWin );
	/// Run Menu
	void calibrateCameraBackground( ScriptingWindow* scriptWin, MainWindow* mainWin, AndorWindow* camWin,
									AuxiliaryWindow* auxWin );
	void prepareBaslerCamera ( BaslerWindow* basWin, ExperimentInput& input );
	void prepareMasterThread( int msgID, ScriptingWindow* scriptWin, MainWindow* mainWin, AndorWindow* camWin,
							  AuxiliaryWindow* auxWin, ExperimentInput& input, bool runNiawg, bool runTtls );
	void startExperimentThread(MainWindow* mainWin, ExperimentInput& input);

	void logParameters( ExperimentInput& input, AndorWindow* camWin, BaslerWindow* basWin, bool takeAndorPictures, bool takeBaslerPictures );
	void setMot ( MainWindow* mainWin, AuxiliaryWindow* auxWin, AndorWindow* camWin = NULL, BaslerWindow* basWin = NULL );
	void abortNiawg( ScriptingWindow* scriptWin, MainWindow* mainWin );
	void abortCamera( AndorWindow* camWin, MainWindow* mainWin );
	void abortMaster(MainWindow* mainWin, AuxiliaryWindow* auxWin);
	void exitProgram( ScriptingWindow* scriptWindow, MainWindow* mainWin, AndorWindow* camWin, AuxiliaryWindow* auxWin );
	bool getPermissionToStart( AndorWindow* camWin, MainWindow* mainWin, ScriptingWindow* scriptWin,
							   AuxiliaryWindow* auxWin, bool runNiawg, bool runMaster, ExperimentInput& input );
	/// Scripting Menu
	void reloadNIAWGDefaults( MainWindow* mainWin );

}


