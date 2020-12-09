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
#include <ExperimentThread/ExpThreadWorker.h>
#include <QThread.h>
#include <qapplication.h>
#include <qwidget.h>
#include <qwindow.h>
#include <qscreen.h>

QtMainWindow::QtMainWindow () : 
	profile (PROFILES_PATH, this),
	masterConfig (MASTER_CONFIGURATION_FILE_ADDRESS),
	masterRepumpScope (MASTER_REPUMP_SCOPE_ADDRESS, MASTER_REPUMP_SCOPE_SAFEMODE, 4, "D2 F=1 & Master Lasers Scope"),
	motScope (MOT_SCOPE_ADDRESS, MOT_SCOPE_SAFEMODE, 2, "D2 F=2 Laser Scope"),
	expScope(EXPERIMENT_SCOPE_ADDRESS, EXPERIMENT_SCOPE_SAFEMODE, 4, "Experiment Scope"),
	calManager(this){
	statBox = new ColorBox ();
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
	catch (ChimeraError& err) {
		errBox ("FATAL ERROR: " + which + " Window constructor failed! Error: " + err.trace ());
		return;
	}
	scriptWin->loadFriends( this, scriptWin, auxWin, basWin, dmWin, andorWin );
	andorWin->loadFriends (this, scriptWin, auxWin, basWin, dmWin, andorWin);
	auxWin->loadFriends (this, scriptWin, auxWin, basWin, dmWin, andorWin);
	basWin->loadFriends (this, scriptWin, auxWin, basWin, dmWin, andorWin);
	dmWin->loadFriends (this, scriptWin, auxWin, basWin, dmWin, andorWin);
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
	/*	scriptWin, andorWin, auxWin, basWin, dmWin, mainWin; */
	for (auto* window : winList ()) { 
		auto screen = qApp->screens ()[monitorNum[winCount++] % numMonitors];
		window->setWindowState ((windowState () & ~Qt::WindowMinimized) | Qt::WindowActive);
		window->activateWindow ();
		window->show (); 
		window->move (screen->availableGeometry ().topLeft());
		window->resize (screen->availableGeometry ().width (), screen->availableGeometry().height());
	}
	// hide the splash just before the first window requiring input pops up.
	try	{
		masterConfig.load (this, auxWin, andorWin);
	}
	catch (ChimeraError& err){
		errBox (err.trace ());
	}
	setWindowTitle ("Main Window");
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
		reportStatus (qstr(initializationString));
	}
	catch (ChimeraError & err) {
		errBox (err.trace ());
	}
	QTimer* timer = new QTimer (this);
	connect (timer, &QTimer::timeout, [this]() {
		// should auto quit in the handling here if calibration has already been completed for the day. 
		commonFunctions::handleCommonMessage (ID_ACCELERATOR_F11, this);
		});
	// 6 minutes
	timer->start (360000);
}

bool QtMainWindow::expIsRunning () {
	return experimentIsRunning;
}

void QtMainWindow::setStyleSheets (){
	for (auto* window : winList ()) {
		window->setStyleSheet (chimeraStyleSheets::stdStyleSheet ());
	}
}

void QtMainWindow::pauseExperiment () {
	if (expWorker != NULL) {
		expWorker->pause ();
	}
}

void QtMainWindow::initializeWidgets (){
	/// initialize main window controls.
	QPoint controlLocation = { 0, 25 };
	mainStatus.initialize (controlLocation, this, 870, "EXPERIMENT STATUS", { "#7474FF","#4848FF","#2222EF" });
	statBox->initialize (controlLocation, this, 960, getDevices ());
	shortStatus.initialize (controlLocation, this);
	controlLocation = { 480, 25 };
	errorStatus.initialize (controlLocation, this, 870, "ERROR STATUS", { "#FF0000", "#800000"});
	controlLocation = { 960, 25 };
	profile.initialize (controlLocation, this);
	controlLocation = { 960, 50 };
	notes.initialize (controlLocation, this);
	masterRepumpScope.initialize (controlLocation, 480, 130, this, "Master/Repump");
	motScope.initialize (controlLocation, 480, 130, this, "MOT");
	expScope.initialize (controlLocation, 480, 130, this, "Experiment");
	calManager.initialize (controlLocation, this, &auxWin->getAiSys (), &auxWin->getAoSys (), auxWin->getTtlSystem (),
						   auxWin->getAgilents(), andorWin->getPython()); 
	controlLocation = { 1440, 50 };
	repetitionControl.initialize (controlLocation, this);
	mainOptsCtrl.initialize (controlLocation, this);
	debugger.initialize (controlLocation, this);
	texter.initialize (controlLocation, this);

}

unsigned QtMainWindow::getAutoCalNumber () { return autoCalNum; }

void QtMainWindow::onAutoCalFin (QString msg, profileSettings finishedConfig){
	try	{
		scriptWin->restartNiawgDefaults ();
	}
	catch (ChimeraError& except)	{
		reportErr ("The niawg finished normally, but upon restarting the default waveform, threw the "
						"following error: " + except.qtrace ());
		reportStatus ("ERROR!\r\n");
	}
	scriptWin->setNiawgRunningState (false);
	andorWin->handleNormalFinish (finishedConfig);
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
	onNormalFinish ("", {});
	Sleep (1000);
	// then restart.
	//commonFunctions::handleCommonMessage (ID_MACHINE_OPTIMIZATION, this);
}

void QtMainWindow::loadCameraCalSettings (ExperimentThreadInput* input){
	input->skipNext = NULL;
	input->expType = ExperimentType::CameraCal;
}

LRESULT QtMainWindow::onNoMotAlertMessage (WPARAM wp, LPARAM lp){
	try	{
		if (andorWin->wantsAutoPause ()){
			reportErr ("No MOT and andor win wants auto pause!");
			expWorker->pause ();
		}
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
		//texter.sendMessage (message, &python, "Mot");
	}
	catch (ChimeraError& err){
		reportErr (err.qtrace ());
	}
	return 0;
}

LRESULT QtMainWindow::onNoAtomsAlertMessage (WPARAM wp, LPARAM lp){
	try	{
		if (andorWin->wantsAutoPause ()){
			reportErr ("No Atoms and andor win wants auto pause!");
			expWorker->pause ();
		}
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
		//texter.sendMessage (message, &python, "Loading");
	}
	catch (ChimeraError& err){
		reportErr (err.qtrace ());
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
	catch (ChimeraError& err)	{
		reportErr (err.qtrace ());
	}
}

// just notifies the profile object that the configuration is no longer saved.
void QtMainWindow::notifyConfigUpdate (){
	profile.updateConfigurationSavedStatus (false);
}

void QtMainWindow::handlePauseToggle (){
	if (expWorker->runningStatus ()){
		reportErr ("Pause Toggle!");
		if (expWorker->getIsPaused ()){
			expWorker->unPause ();
		}
		else{
			expWorker->pause ();
		}
	}
	else{
		reportStatus ("Can't pause, experiment was not running.\r\n");
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
		ConfigSystem::standardOpenConfig (configStream, "CONFIGURATION_NOTES", &notes);
		mainOptsCtrl.setOptions (ConfigSystem::stdConfigGetter (configStream, "MAIN_OPTIONS",
			MainOptionsControl::getSettingsFromConfig));
		ConfigSystem::standardOpenConfig (configStream, "DEBUGGING_OPTIONS", &debugger);
		repetitionControl.setRepetitions (ConfigSystem::stdConfigGetter (configStream, "REPETITIONS",
			Repetitions::getSettingsFromConfig));

	}
	catch (ChimeraError&){
		throwNested ("Main Window failed to read parameters from the configuration file.");
	}
}

unsigned QtMainWindow::getRepNumber () { return repetitionControl.getRepetitionNumber (); }

std::string QtMainWindow::getSystemStatusString (){
	std::string status;
	status += "\nMOT Scope:\n";
	if (!MOT_SCOPE_SAFEMODE){
		status += "\tCode System is Active!\n";
		try{
			status += "\t" + motScope.getScopeInfo ();
		}
		catch (ChimeraError& err){
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
		catch (ChimeraError& err){
			status += "\tFailed to get device info! Error: " + err.trace ();
		}
	}
	else{
		status += "\tCode System is disabled! Enable in \"constants.h\"\r\n";
	}
	return status;
}

void QtMainWindow::startExperimentThread (ExperimentThreadInput* input){
	//expThreadManager.startExperimentThread (input, this);
	if (!input) {
		thrower ("Input to start experiment thread was null?!?!? (a Low level bug, this shouldn't happen).");
	}
	if (experimentIsRunning) {
		delete input;
		thrower ("Experiment is already Running! You can only run one experiment at a time! Please abort before "
			"running again.");
	}
	//input->thisObj = this;
	expWorker = new ExpThreadWorker (input, experimentIsRunning);
	expThread = new QThread;
	expWorker->moveToThread (expThread);
	connect (expWorker, &ExpThreadWorker::updateBoxColor, this, &QtMainWindow::handleColorboxUpdate);
	connect (expWorker, &ExpThreadWorker::prepareAndor, andorWin, &QtAndorWindow::handlePrepareForAcq);
	connect (expWorker, &ExpThreadWorker::prepareBasler, basWin, &QtBaslerWindow::prepareWinForAcq);
	connect (expWorker, &ExpThreadWorker::notification, this, &QtMainWindow::handleNotification);
	connect (expWorker, &ExpThreadWorker::warn, this, &QtMainWindow::onErrorMessage);
	connect (expWorker, &ExpThreadWorker::doAoData, auxWin, &QtAuxiliaryWindow::handleDoAoPlotData);
	connect (expWorker, &ExpThreadWorker::repUpdate, this, &QtMainWindow::onRepProgress);
	connect (expWorker, &ExpThreadWorker::mainProcessFinish, expThread, &QThread::quit);
	connect (expWorker, &ExpThreadWorker::normalExperimentFinish, this, &QtMainWindow::onNormalFinish);
	connect (expWorker, &ExpThreadWorker::calibrationFinish, this, &QtMainWindow::onAutoCalFin);
	connect (expWorker, &ExpThreadWorker::errorExperimentFinish, this, &QtMainWindow::onFatalError);
	connect (expWorker, &ExpThreadWorker::expParamsSet, this->auxWin, &QtAuxiliaryWindow::updateExpActiveInfo);

	connect (expThread, &QThread::started, expWorker, &ExpThreadWorker::process);
	connect (expThread, &QThread::finished, expThread, &QObject::deleteLater);
	connect (expThread, &QThread::finished, expWorker, &QObject::deleteLater);
	expThread->start (QThread::TimeCriticalPriority);
}

void QtMainWindow::fillMotInput (ExperimentThreadInput* input){
	input->profile.configuration = "Set MOT Settings";
	input->profile.configLocation = MOT_ROUTINES_ADDRESS;
	input->profile.parentFolderName = "MOT";
	input->calibrations = calManager.getCalibrationInfo ();
	// the mot procedure doesn't need the NIAWG at all.
	input->skipNext = NULL;
}

bool QtMainWindow::masterIsRunning () { return experimentIsRunning; }
RunInfo QtMainWindow::getRunInfo () { return systemRunningInfo; }
profileSettings QtMainWindow::getProfileSettings () { return profile.getProfileSettings (); }
std::string QtMainWindow::getNotes () { return notes.getConfigurationNotes (); }
void QtMainWindow::setNotes (std::string newNotes) { notes.setConfigurationNotes (newNotes); }
debugInfo QtMainWindow::getDebuggingOptions () { return debugger.getOptions (); }
void QtMainWindow::setDebuggingOptions (debugInfo options) { debugger.setOptions (options); }
mainOptions QtMainWindow::getMainOptions () { return mainOptsCtrl.getOptions (); }
void QtMainWindow::setShortStatus (std::string text) { shortStatus.setText (text); }
void QtMainWindow::changeShortStatusColor (std::string color) { shortStatus.setColor (color); }
bool QtMainWindow::experimentIsPaused () { return expWorker->getIsPaused (); }
std::vector<calResult> QtMainWindow::getCalInfo () {
	return calManager.getCalibrationInfo ();
}

void QtMainWindow::fillMasterThreadInput (ExperimentThreadInput* input){
	input->sleepTime = debugger.getOptions ().sleepTime;
	input->profile = profile.getProfileSettings ();
	input->calibrations = calManager.getCalibrationInfo ();
}

void QtMainWindow::logParams (DataLogger* logger, ExperimentThreadInput* input){
	logger->logMasterInput (input);
}

void QtMainWindow::checkProfileSave (){
	profile.checkSaveEntireProfile( this );
}

void QtMainWindow::updateConfigurationSavedStatus (bool status){
	profile.updateConfigurationSavedStatus (status);
}

void QtMainWindow::addTimebar (std::string whichStatus){
	std::transform (whichStatus.begin (), whichStatus.end (), whichStatus.begin (), ::tolower);
	if (whichStatus == "error")	{
		errorStatus.appendTimebar ();
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
	IChimeraQtWindow::changeBoxColor (sysDelim, color);
	changeShortStatusColor (color);
}

void QtMainWindow::abortMasterThread (){
	if (expWorker->runningStatus ()){
		expWorker->abort ();
		autoF5_AfterFinish = false;
	}
	else { thrower ("Can't abort, experiment was not running.\r\n"); }
}

void QtMainWindow::onErrorMessage (QString errMessage, unsigned level){
	if (str(errMessage) != ""){
		QApplication::beep ();
		errorStatus.addStatusText (str(errMessage), level);
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
		reportErr ("EXITED WITH ERROR!\n");
		reportStatus ("EXITED WITH ERROR!\nInitialized Default Waveform\r\n");
	}
	catch (ChimeraError& except){
		reportErr ("EXITED WITH ERROR! " + except.qtrace ());
		reportStatus ("EXITED WITH ERROR!\nNIAWG RESTART FAILED!\r\n");
	}
	scriptWin->setNiawgRunningState (false);
}

void QtMainWindow::onNormalFinish (QString finMsg, profileSettings finishedProfile) {
	handleNotification (finMsg);
	scriptWin->setIntensityDefault ();
	setShortStatus ("Passively Outputting Default Waveform");
	changeShortStatusColor ("B");
	scriptWin->stopRearranger ();
	andorWin->handleNormalFinish (finishedProfile);
	handleFinishText ();
	auxWin->handleNormalFin ();
	try { scriptWin->restartNiawgDefaults (); }
	catch (ChimeraError& except){
		reportErr ("The niawg finished normally, but upon restarting the default waveform, threw the "
			"following error: " + except.qtrace ());
		reportStatus ("ERROR!\r\n");
	}
	scriptWin->setNiawgRunningState (false);
	try { scriptWin->waitForRearranger (); }
	catch (ChimeraError& err) { reportErr (err.qtrace ()); }

	if (autoF5_AfterFinish)	{
		commonFunctions::handleCommonMessage (ID_ACCELERATOR_F5, this);
		autoF5_AfterFinish = false;
	}
}

void QtMainWindow::handleFinishText (){
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
		//texter.sendMessage (message, &python, "Finished");
	}
	catch (ChimeraError& err){
		reportErr (err.qtrace ());
	}
}

void QtMainWindow::handleMasterConfigOpen (ConfigStream& configStream){
	calManager.handleOpenMasterConfig (configStream);
}

void QtMainWindow::handleMasterConfigSave (std::stringstream& configStream){
	calManager.handleSaveMasterConfig (configStream);
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

void QtMainWindow::handleNotification (QString txt, unsigned level){
	mainStatus.addStatusText (str(txt), level);
}

QThread* QtMainWindow::getExpThread () {
	return expThread;
}

ExpThreadWorker* QtMainWindow::getExpThreadWorker () {
	return expWorker;
}
