#pragma once
#include "MasterThreadInput.h"
#include "afxwin.h"

class MainWindow;
class CameraWindow;
class AuxiliaryWindow;
class ScriptingWindow;
class BaslerWindow;

namespace commonFunctions
{
	/// Call to direct message to appropriate function in this namespace
	void handleCommonMessage( int msgID, CWnd* parent, MainWindow* comm, ScriptingWindow* scriptWin, 
							 CameraWindow* camWin, AuxiliaryWindow* auxWin, BaslerWindow* basWin );
	/// Run Menu
	void calibrateCameraBackground( ScriptingWindow* scriptWin, MainWindow* mainWin, CameraWindow* camWin,
									AuxiliaryWindow* auxWin );
	void prepareCamera( MainWindow* mainWin, CameraWindow* camWin, ExperimentInput& input );
	void prepareMasterThread( int msgID, ScriptingWindow* scriptWin, MainWindow* mainWin, CameraWindow* camWin,
							  AuxiliaryWindow* auxWin, ExperimentInput& input, bool runNiawg, bool runTtls );
	void startMaster(MainWindow* mainWin, ExperimentInput& input);

	void logParameters( ExperimentInput& input, CameraWindow* camWin, bool takeAndorPictures );
	void setMot(MainWindow* mainWin);
	void abortNiawg( ScriptingWindow* scriptWin, MainWindow* mainWin );
	void abortCamera( CameraWindow* camWin, MainWindow* mainWin );
	void abortMaster(MainWindow* mainWin, AuxiliaryWindow* auxWin);
	void exitProgram( ScriptingWindow* scriptWindow, MainWindow* mainWin, CameraWindow* camWin, AuxiliaryWindow* auxWin );
	bool getPermissionToStart( CameraWindow* camWin, MainWindow* mainWin, ScriptingWindow* scriptWin,
							   AuxiliaryWindow* auxWin, bool runNiawg, bool runMaster, ExperimentInput& input );
	/// Scripting Menu
	void reloadNIAWGDefaults( MainWindow* mainWin );

}


