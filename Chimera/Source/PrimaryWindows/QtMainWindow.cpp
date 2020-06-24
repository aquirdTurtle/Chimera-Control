#include "stdafx.h"
#include "QtMainWindow.h"
#include "Agilent/AgilentSettings.h"
#include <ExperimentMonitoringAndStatus/ColorBox.h>
#include <qdesktopwidget.h>
#include <PrimaryWindows/QtScriptWindow.h>
#include <PrimaryWindows/QtAndorWindow.h>
#include <PrimaryWindows/QtAuxiliaryWindow.h>
#include <PrimaryWindows/QtBaslerWindow.h>
#include <PrimaryWindows/QtDeformableMirrorWindow.h>
#include <ExperimentThread/autoCalConfigInfo.h>
#include <GeneralObjects/ChimeraStyleSheets.h>
#include <Plotting/ScopeThreadWorker.h>
#include <QThread.h>
#include <qapplication.h>
#include <qwidget.h>
#include <qwindow.h>
#include <qscreen.h>

QtMainWindow::QtMainWindow () : 
	profile (PROFILES_PATH),
	masterConfig (MASTER_CONFIGURATION_FILE_ADDRESS),
	masterRepumpScope (MASTER_REPUMP_SCOPE_ADDRESS, MASTER_REPUMP_SCOPE_SAFEMODE, 4, "D2 F=1 & Master Lasers Scope"),
	motScope (MOT_SCOPE_ADDRESS, MOT_SCOPE_SAFEMODE, 2, "D2 F=2 Laser Scope"){
	statBox = new ColorBox ();
	//programStartTime = startTime;
	startupTimes.push_back (chronoClock::now ());
	// not done with the script, it will not stay on the NIAWG, so I need to keep track of it so thatI can reload it onto the NIAWG when necessary.	
	/// Initialize Windows
	std::string which = "";
	try	{
		mainWin = this;
		which = "Scripting";
		scriptWin = new QtScriptWindow;
		which = "Camera";
		andorWin = new QtAndorWindow;
		which = "Auxiliary";
		auxWin = new QtAuxiliaryWindow;
		which = "Basler";
		basWin = new QtBaslerWindow;
		which = "DmWin";
		dmWin = new QtDeformableMirrorWindow;
	}
	catch (Error& err) {
		errBox ("FATAL ERROR: " + which + " Window constructor failed! Error: " + err.trace ());
		//forceExit ();
		//return -1;
	}
	scriptWin->loadFriends( this, scriptWin, auxWin, basWin, dmWin, andorWin );
	andorWin->loadFriends (this, scriptWin, auxWin, basWin, dmWin, andorWin);
	auxWin->loadFriends (this, scriptWin, auxWin, basWin, dmWin, andorWin);
	basWin->loadFriends (this, scriptWin, auxWin, basWin, dmWin, andorWin);
	dmWin->loadFriends (this, scriptWin, auxWin, basWin, dmWin, andorWin);
	startupTimes.push_back (chronoClock::now ());
	startupTimes.push_back (chronoClock::now ());


	for (auto* window : winList ()) {
		window->initializeWidgets ();
		window->initializeShortcuts ();
		window->initializeMenu ();
	}
	setStyleSheets ();
	auto numMonitors = qApp->screens ().size ();
	auto screens = qApp->screens ();
	unsigned winCount = 0;
	std::vector<unsigned> monitorNum = { 4,3,5,1,2,0 };
	/*	scriptWin, andorWin, auxWin, basWin, dmWin, mainWin;
		*/
	for (auto* window : winList ()) { 
		auto screen = qApp->screens ()[monitorNum[winCount++] % numMonitors];
		window->setWindowState ((windowState () & ~Qt::WindowMinimized) | Qt::WindowActive);
		window->activateWindow ();
		window->show (); 
		window->move (screen->availableGeometry ().topLeft());
		window->resize (screen->availableGeometry ().width (), screen->availableGeometry().height());

	}
	// hide the splash just before the first window requiring input pops up.
	// appSplash->ShowWindow (SW_HIDE);
	try	{
		masterConfig.load (this, auxWin, andorWin);
	}
	catch (Error& err){
		errBox (err.trace ());
	}
	setWindowTitle ("Main Window");

	QThread* motThread = new QThread;
	QThread* masterRepumpThread = new QThread;
	ScopeThreadWorker* motWorker = new ScopeThreadWorker (&motScope);
	ScopeThreadWorker* masterRepumpWorker = new ScopeThreadWorker (&masterRepumpScope);

	motWorker->moveToThread (motThread);
	masterRepumpWorker->moveToThread (masterRepumpThread);

	connect (motThread, SIGNAL (started ()), motWorker, SLOT (process ()));
	connect (motWorker, &ScopeThreadWorker::newData, &motScope, &ScopeViewer::updateData);
	connect (motThread, SIGNAL (finished ()), motThread, SLOT (deleteLater ()));
	motThread->start ();

	connect (masterRepumpThread, SIGNAL (started ()), masterRepumpWorker, SLOT (process ()));
	connect (masterRepumpWorker, &ScopeThreadWorker::newData, &masterRepumpScope, &ScopeViewer::updateData);
	connect (masterRepumpThread, SIGNAL (finished ()), masterRepumpThread, SLOT (deleteLater ()));
	masterRepumpThread->start ();

	updateConfigurationSavedStatus (true);

	/// summarize system status.
	try {
		// ordering of aux window pieces is a bit funny because I want the devices grouped by type, not by window.
		std::string initializationString;
		initializationString += getSystemStatusString ();
		initializationString += auxWin->getOtherSystemStatusMsg ();
		initializationString += andorWin->getSystemStatusString ();
		initializationString += auxWin->getVisaDeviceStatus ();
		initializationString += scriptWin->getSystemStatusString ();
		initializationString += auxWin->getMicrowaveSystemStatus ();
		infoBox (initializationString);
	}
	catch (Error & err) {
		errBox (err.trace ());
	}
}

QtMainWindow::~QtMainWindow (){}

void QtMainWindow::setStyleSheets (){
	for (auto* window : winList ()) {
		window->setStyleSheet (chimeraStyleSheets::stdStyleSheet ());
	}
}

void QtMainWindow::initializeWidgets (){
	/// initialize main window controls.
	comm.initialize (this);
	POINT controlLocation = { 0, 25 };
	mainStatus.initialize (controlLocation, this, 870, "EXPERIMENT STATUS", "#6464FF");
	statBox->initialize (controlLocation, this, 960, getDevices ());
	shortStatus.initialize (controlLocation, this);
	controlLocation = { 480, 25 };
	errorStatus.initialize (controlLocation, this, 420, "ERROR STATUS", "#FF0000");
	debugStatus.initialize (controlLocation, this, 425, "DEBUG STATUS", "#0d98ba");
	controlLocation = { 960, 25 };
	profile.initialize (controlLocation, this);
	controlLocation = { 960, 50 };
	notes.initialize (controlLocation, this);
	masterRepumpScope.initialize (controlLocation, 480, 130, this, "Master/Repump");
	motScope.initialize (controlLocation, 480, 130, this, "MOT");
	servos.initialize (controlLocation, this, &auxWin->getAiSys (), &auxWin->getAoSys (),
		auxWin->getTtlSystem (), &auxWin->getGlobals ());
	controlLocation = { 1440, 50 };
	repetitionControl.initialize (controlLocation, this);
	mainOptsCtrl.initialize (controlLocation, this);
	debugger.initialize (controlLocation, this);
	texter.initialize (controlLocation, this);
}

void QtMainWindow::handleThresholdAnalysis (){
	auto grid = andorWin->getMainAtomGrid ();
	auto dateStr = andorWin->getMostRecentDateString ();
	auto fid = andorWin->getMostRecentFid ();
	auto ppr = andorWin->getPicsPerRep ();
	std::string gridString = "[" + str (grid.topLeftCorner.row - 1) + "," + str (grid.topLeftCorner.column - 1) + ","
		+ str (grid.pixelSpacing) + "," + str (grid.width) + "," + str (grid.height) + "]";
	try{
		python.thresholdAnalysis (dateStr, fid, gridString, ppr);
	}
	catch (Error& err){
		comm.sendError ("Threshold Analysis Failed! " + err.trace ());
	}
}


LRESULT QtMainWindow::onFinish (WPARAM wp, LPARAM lp){
	ExperimentType type = static_cast<ExperimentType>(wp);
	switch (type)
	{
	case ExperimentType::Normal:
		onNormalFinish ("");
		break;
	case ExperimentType::LoadMot:
		break;
	case ExperimentType::AutoCal:
		onAutoCalFin ();
		break;
	case ExperimentType::MachineOptimization:
		onMachineOptRoundFin ();
		break;
	}
	return 0;
} 
 
UINT QtMainWindow::getAutoCalNumber () { return autoCalNum; }

void QtMainWindow::onAutoCalFin (){
	try	{
		scriptWin->restartNiawgDefaults ();
	}
	catch (Error& except)	{
		comm.sendError ("The niawg finished normally, but upon restarting the default waveform, threw the "
			"following error: " + except.trace ());
		comm.sendStatus ("ERROR!\r\n");
	}
	scriptWin->setNiawgRunningState (false);
	andorWin->cleanUpAfterExp ();
	autoCalNum++;
	if (autoCalNum >= AUTO_CAL_LIST.size ())	{
		// then just finished the calibrations.
		autoCalNum = 0;
		infoBox ("Finished Automatic Calibrations.");
	}
	else{
		commonFunctions::handleCommonMessage (ID_ACCELERATOR_F11, this);
	}
}

void QtMainWindow::onMachineOptRoundFin (){
	// do normal finish
	onNormalFinish ("");
	Sleep (1000);
	// then restart.
	commonFunctions::handleCommonMessage (ID_MACHINE_OPTIMIZATION, this);
}


void QtMainWindow::OnTimer (UINT_PTR id) {
	if (id == 10){
		motScope.refreshData ();
		masterRepumpScope.refreshData ();
	}
}


void QtMainWindow::loadCameraCalSettings (ExperimentThreadInput* input)
{
	input->skipNext = NULL;
	input->expType = ExperimentType::CameraCal;
}

LRESULT QtMainWindow::onNoMotAlertMessage (WPARAM wp, LPARAM lp){
	try	{
		if (andorWin->wantsAutoPause ()){
			expThreadManager.pause ();
			//checkAllMenus (ID_RUNMENU_PAUSE, MF_CHECKED);
		}
		//beepFuture
		//	= std::async ( std::launch::async, [] { Beep ( 1000, 100 ); } );
		time_t t = time (0);
		struct tm now;
		localtime_s (&now, &t);
		std::string message = "Experiment Stopped loading the MOT at ";
		if (now.tm_hour < 10){
			message += "0";
		}
		message += str (now.tm_hour) + ":";
		if (now.tm_min < 10){
			message += "0";
		}
		message += str (now.tm_min) + ":";
		if (now.tm_sec < 10){
			message += "0";
		}
		message += str (now.tm_sec);
		texter.sendMessage (message, &python, "Mot");
	}
	catch (Error& err){
		comm.sendError (err.what ());
	}
	return 0;
}


LRESULT QtMainWindow::onNoAtomsAlertMessage (WPARAM wp, LPARAM lp)
{
	try	{
		if (andorWin->wantsAutoPause ()){
			expThreadManager.pause ();
			//checkAllMenus (ID_RUNMENU_PAUSE, MF_CHECKED);
		}
		//beepFuture = std::async( std::launch::async, [] { Beep( 1000, 100 ); } );
		time_t t = time (0);
		struct tm now;
		localtime_s (&now, &t);
		std::string message = "Experiment Stopped loading atoms at ";
		if (now.tm_hour < 10){
			message += "0";
		}
		message += str (now.tm_hour) + ":";
		if (now.tm_min < 10){
			message += "0";
		}
		message += str (now.tm_min) + ":";
		if (now.tm_sec < 10){
			message += "0";
		}
		message += str (now.tm_sec);
		texter.sendMessage (message, &python, "Loading");
	}
	catch (Error& err){
		comm.sendError (err.trace ());
	}
	return 0;
}


void QtMainWindow::showHardwareStatus (){
	try	{
		// ordering of aux window pieces is a bit funny because I want the devices grouped by type, not by window.
		std::string initializationString;
		initializationString += getSystemStatusString ();
		initializationString += auxWin->getOtherSystemStatusMsg ();
		initializationString += andorWin->getSystemStatusString ();
		initializationString += auxWin->getVisaDeviceStatus ();
		initializationString += scriptWin->getSystemStatusString ( );
		initializationString += auxWin->getMicrowaveSystemStatus ();
		infoBox (initializationString);
	}
	catch (Error& err)	{
		errBox (err.trace ());
	}
}


// just notifies the profile object that the configuration is no longer saved.
void QtMainWindow::notifyConfigUpdate (){
	profile.updateConfigurationSavedStatus (false);
}

BOOL CALLBACK QtMainWindow::monitorHandlingProc (_In_ HMONITOR hMonitor, _In_ HDC hdcMonitor,
	_In_ LPRECT lprcMonitor, _In_ LPARAM dwData){
	static UINT count = 0;
	std::vector<CDialog*>* windows = reinterpret_cast<std::vector<CDialog*>*>(dwData);
	if (count == 1)	{
		// skip the tall monitor.
		count++;
		return TRUE;
	}
	if (count < 6){
		if (windows->at (count) != NULL){
			windows->at (count)->MoveWindow (lprcMonitor);
		}
		else{
			errBox ("Error in monitorHandlingProc! Tried to move \"NULL\" Window to monitor.");
		}
	}
	count++;
	return TRUE;
}


void QtMainWindow::handlePause (){
	if (expThreadManager.runningStatus ()){
		if (expThreadManager.getIsPaused ()){
			// then it's currently paused, so unpause it.
			//checkAllMenus (ID_RUNMENU_PAUSE, MF_UNCHECKED);
			expThreadManager.unPause ();
		}
		else{
			// then not paused so pause it.
			//checkAllMenus (ID_RUNMENU_PAUSE, MF_CHECKED);
			expThreadManager.pause ();
		}
	}
	else{
		comm.sendStatus ("Can't pause, experiment was not running.\r\n");
	}
}


void QtMainWindow::onRepProgress (unsigned int repNum){
	repetitionControl.updateNumber (repNum);
}

void QtMainWindow::windowSaveConfig (ConfigStream& saveFile){
	notes.handleSaveConfig (saveFile);
	mainOptsCtrl.handleSaveConfig (saveFile);
	debugger.handleSaveConfig (saveFile);
	repetitionControl.handleSaveConfig (saveFile);
}

void QtMainWindow::windowOpenConfig (ConfigStream& configStream){
	try	{
		ProfileSystem::standardOpenConfig (configStream, "CONFIGURATION_NOTES", &notes);
		mainOptsCtrl.setOptions (ProfileSystem::stdConfigGetter (configStream, "MAIN_OPTIONS",
			MainOptionsControl::getSettingsFromConfig));
		ProfileSystem::standardOpenConfig (configStream, "DEBUGGING_OPTIONS", &debugger);
		repetitionControl.setRepetitions (ProfileSystem::stdConfigGetter (configStream, "REPETITIONS",
			Repetitions::getSettingsFromConfig));

	}
	catch (Error&){
		throwNested ("Main Window failed to read parameters from the configuration file.");
	}
}


fontMap QtMainWindow::getFonts () { return mainFonts; }


UINT QtMainWindow::getRepNumber () { return repetitionControl.getRepetitionNumber (); }

std::string QtMainWindow::getSystemStatusString (){
	std::string status;
	status += "\nMOT Scope:\n";
	if (!MOT_SCOPE_SAFEMODE){
		status += "\tCode System is Active!\n";
		try{
			status += "\t" + motScope.getScopeInfo ();
		}
		catch (Error& err){
			status += "\tFailed to get device info! Error: " + err.trace ();
		}
	}
	else{
		status += "\tCode System is disabled! Enable in \"constants.h\"\r\n";
	}
	status += "Master/Repump Scope:\n";
	if (!MASTER_REPUMP_SCOPE_SAFEMODE){
		status += "\tCode System is Active!\n";
		try	{
			status += "\t" + masterRepumpScope.getScopeInfo ();
		}
		catch (Error& err){
			status += "\tFailed to get device info! Error: " + err.trace ();
		}
	}
	else{
		status += "\tCode System is disabled! Enable in \"constants.h\"\r\n";
	}
	return status;
}


void QtMainWindow::startExperimentThread (ExperimentThreadInput* input){
	expThreadManager.startExperimentThread (input, this);
}


void QtMainWindow::fillMotInput (ExperimentThreadInput* input){
	input->profile.configuration = "Set MOT Settings";
	input->profile.configLocation = MOT_ROUTINES_ADDRESS;
	input->profile.parentFolderName = "MOT";
	// the mot procedure doesn't need the NIAWG at all.
	input->skipNext = NULL;
	input->runList.andor = false;
}

bool QtMainWindow::masterIsRunning () { return expThreadManager.runningStatus (); }
RunInfo QtMainWindow::getRunInfo () { return systemRunningInfo; }
Communicator& QtMainWindow::getCommRef () { return comm; }
EmbeddedPythonHandler& QtMainWindow::getPython () { return python; }
profileSettings QtMainWindow::getProfileSettings () { return profile.getProfileSettings (); }
std::string QtMainWindow::getNotes () { return notes.getConfigurationNotes (); }
void QtMainWindow::setNotes (std::string newNotes) { notes.setConfigurationNotes (newNotes); }
debugInfo QtMainWindow::getDebuggingOptions () { return debugger.getOptions (); }
void QtMainWindow::setDebuggingOptions (debugInfo options) { debugger.setOptions (options); }
mainOptions QtMainWindow::getMainOptions () { return mainOptsCtrl.getOptions (); }
void QtMainWindow::setShortStatus (std::string text) { shortStatus.setText (text); }
void QtMainWindow::changeShortStatusColor (std::string color) { shortStatus.setColor (color); }
bool QtMainWindow::experimentIsPaused () { return expThreadManager.getIsPaused (); }
Communicator* QtMainWindow::getComm () { return &comm; }


void QtMainWindow::fillMasterThreadInput (ExperimentThreadInput* input){
	input->debugOptions = debugger.getOptions ();
	input->profile = profile.getProfileSettings ();
}


void QtMainWindow::logParams (DataLogger* logger, ExperimentThreadInput* input){
	logger->logMasterInput (input);
	logger->logServoInfo (getServoinfo ());
}


void QtMainWindow::checkProfileReady (){
	//profile.allSettingsReadyCheck( this );
}


void QtMainWindow::checkProfileSave (){
	//profile.checkSaveEntireProfile( this );
}


void QtMainWindow::updateConfigurationSavedStatus (bool status){
	profile.updateConfigurationSavedStatus (status);
}


void QtMainWindow::updateStatusText (std::string whichStatus, std::string text){
	std::transform (whichStatus.begin (), whichStatus.end (), whichStatus.begin (), ::tolower);
	if (whichStatus == "error")	{
		errorStatus.addStatusText (text);
	}
	else if (whichStatus == "debug")	{
		debugStatus.addStatusText (text);
	}
	else if (whichStatus == "main")	{
		mainStatus.addStatusText (text);
	}
	else{
		thrower ("Main Window's updateStatusText function recieved a bad argument for which status"
			" control to update. Options are \"error\", \"debug\", and \"main\", but recieved " + whichStatus);
	}
}


void QtMainWindow::addTimebar (std::string whichStatus)
{
	std::transform (whichStatus.begin (), whichStatus.end (), whichStatus.begin (), ::tolower);
	if (whichStatus == "error")	{
		errorStatus.appendTimebar ();
	}
	else if (whichStatus == "debug"){
		debugStatus.appendTimebar ();
	}
	else if (whichStatus == "main")	{
		mainStatus.appendTimebar ();
	}
	else{
		thrower ("Main Window's addTimebar function recieved a bad argument for which status"
			" control to update. Options are \"error\", \"debug\", and \"main\", but recieved " + whichStatus + ". This"
			"exception can be safely ignored.");
	}
}

void QtMainWindow::changeBoxColor (std::string sysDelim, std::string color){
	IChimeraWindowWidget::changeBoxColor (sysDelim, color);
	changeShortStatusColor (color);
}

void QtMainWindow::abortMasterThread (){
	if (expThreadManager.runningStatus ()){
		expThreadManager.abort ();
		autoF5_AfterFinish = false;
	}
	else { thrower ("Can't abort, experiment was not running.\r\n"); }
}


void QtMainWindow::onErrorMessage (QString errMessage){
	if (str(errMessage) != ""){
		errorStatus.addStatusText (str(errMessage));
	}
}


void QtMainWindow::onFatalError (QString finMsg){
	onErrorMessage (finMsg);
	autoF5_AfterFinish = false;
	// resetting things.
	scriptWin->setIntensityDefault ();
	std::string msgText = "Exited with Error!\nPassively Outputting Default Waveform.";
	changeShortStatusColor ("R");
	try{
		scriptWin->restartNiawgDefaults ();
		comm.sendError ("EXITED WITH ERROR!\n");
		comm.sendStatus ("EXITED WITH ERROR!\nInitialized Default Waveform\r\n");
	}
	catch (Error& except){
		comm.sendError ("EXITED WITH ERROR! " + except.trace ());
		comm.sendStatus ("EXITED WITH ERROR!\nNIAWG RESTART FAILED!\r\n");
	}
	scriptWin->setNiawgRunningState (false);
}


void QtMainWindow::onNormalFinish (QString finMsg) { 
	handleExpNotification (finMsg);
	scriptWin->setIntensityDefault ();
	setShortStatus ("Passively Outputting Default Waveform");
	changeShortStatusColor ("B");
	scriptWin->stopRearranger ();
	andorWin->wakeRearranger ();
	andorWin->cleanUpAfterExp ();
	handleFinish ();
	try { scriptWin->restartNiawgDefaults (); }
	catch (Error& except){
		comm.sendError ("The niawg finished normally, but upon restarting the default waveform, threw the "
			"following error: " + except.trace ());
		comm.sendStatus ("ERROR!\r\n");
	}
	scriptWin->setNiawgRunningState (false);
	try { scriptWin->waitForRearranger (); }
	catch (Error& err) { comm.sendError (err.trace ()); }
	if (andorWin->wantsThresholdAnalysis ()) { handleThresholdAnalysis (); }
	if (autoF5_AfterFinish)	{
		commonFunctions::handleCommonMessage (ID_ACCELERATOR_F5, this);
		autoF5_AfterFinish = false;
	}
}


void QtMainWindow::handleFinish (){
	time_t t = time (0);
	struct tm now;
	localtime_s (&now, &t);
	std::string message = "Experiment Completed at ";
	if (now.tm_hour < 10){
		message += "0";
	}
	message += str (now.tm_hour) + ":";
	if (now.tm_min < 10){
		message += "0";
	}
	message += str (now.tm_min) + ":";
	if (now.tm_sec < 10){
		message += "0";
	}
	message += str (now.tm_sec);
	try{
		texter.sendMessage (message, &python, "Finished");
	}
	catch (Error& err){
		comm.sendError (err.trace ());
	}
}


void QtMainWindow::onDebugMessage (std::string msg){
	debugStatus.addStatusText (msg);
}

// MESSAGE MAP FUNCTION
LRESULT QtMainWindow::autoServo (WPARAM w, LPARAM l){
	try	{
		updateConfigurationSavedStatus (false);
		if (servos.wantsCalAutoServo ()){
			runServos ();
		}
	}
	catch (Error& err){
		comm.sendError ("Auto-Servo Failed.\n" + err.trace ());
	}
	return TRUE;
}

// MESSAGE MAP FUNCTION
void QtMainWindow::runServos (){
	try{
		updateConfigurationSavedStatus (false);
		comm.sendStatus ("Running Servos...\r\n");
		servos.runAll (comm);
	}
	catch (Error& err){
		comm.sendError ("Running Servos failed.\n" + err.trace ());
	}
}

std::vector<servoInfo> QtMainWindow::getServoinfo (){
	return servos.getServoInfo ();
}

void QtMainWindow::handleMasterConfigOpen (ConfigStream& configStream){
	servos.handleOpenMasterConfig (configStream);
}

void QtMainWindow::handleMasterConfigSave (std::stringstream& configStream){
	servos.handleSaveMasterConfig (configStream);
}


void QtMainWindow::fillExpDeviceList (DeviceList& list) {}

DeviceList QtMainWindow::getDevices (){
	DeviceList list;
	for (auto win_ : winList ()) {
		win_->fillExpDeviceList (list);
	}
	return list;
}

void QtMainWindow::handleColorboxUpdate (QString color, QString systemDelim){
	auto colorstr = str (color);
	auto delimStr = str (systemDelim);
	mainWin->changeBoxColor (delimStr, colorstr);
	scriptWin->changeBoxColor (delimStr, colorstr);
	andorWin->changeBoxColor (delimStr, colorstr);
	auxWin->changeBoxColor (delimStr, colorstr);
	basWin->changeBoxColor (delimStr, colorstr);
	dmWin->changeBoxColor (delimStr, colorstr);
 }

void QtMainWindow::handleExpNotification (QString txt){
	mainStatus.addStatusText (str(txt));
}