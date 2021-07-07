#include "stdafx.h"
#include "QtScriptWindow.h"
#include "Agilent/AgilentSettings.h"
#include <qdesktopwidget.h>
#include <PrimaryWindows/QtScriptWindow.h>
#include <PrimaryWindows/QtAndorWindow.h>
#include <PrimaryWindows/QtAuxiliaryWindow.h>
#include <PrimaryWindows/QtMainWindow.h>
#include <PrimaryWindows/QtBaslerWindow.h>
#include <PrimaryWindows/QtDeformableMirrorWindow.h>
#include <ExcessDialogs/saveWithExplorer.h>
#include <ExcessDialogs/openWithExplorer.h>
#include "GeneralObjects/CodeTimer.h"


QtScriptWindow::QtScriptWindow (QWidget* parent) : IChimeraQtWindow (parent),
		intensityAgilent (INTENSITY_AGILENT_SETTINGS, this),
		niawg (DoRows::which::B, 14, NIAWG_SAFEMODE, this), 
		masterScript(this) {
	statBox = new ColorBox ();
	setWindowTitle ("Script Window");
}

QtScriptWindow::~QtScriptWindow (){
}

void QtScriptWindow::initializeWidgets (){
	int id = 2000;
	try {
		niawg.core.initialize ();
	}
	catch (ChimeraError& except) {
		errBox ("NIAWG failed to Initialize! Error: " + except.trace ());
	}
	try {
		niawg.core.setDefaultWaveforms ();
		// but the default starts in the horizontal configuration, so switch back and start in this config.
		// restartNiawgDefaults ();
	}
	catch (ChimeraError& exception) {
		errBox ("Failed to start niawg default waveforms! Niawg gave the following error message: "
				+ exception.trace ());
	}
	QPoint startLocation = { 0, 50 };
	niawg.initialize (startLocation, this);
	niawg.niawgScript.setEnabled (true, false);
	startLocation = { 640, 50 };

	intensityAgilent.initialize (startLocation, "Tweezer Intensity Agilent", 865, this, 640);
	startLocation = { 2 * 640, 50 };
	masterScript.initialize (640, 900, startLocation, this, "Master", "Master Script");
	startLocation = { 1000, 25 };
	statBox->initialize (startLocation, this, 900, mainWin->getDevices ());
	profileDisplay.initialize ({ 0,25 }, this);
	
	try{
		// I only do this for the intensity agilent at the moment.
		intensityAgilent.setDefault (1);
	}
	catch (ChimeraError& err){
		errBox ("ERROR: Failed to initialize intensity agilent: " + err.trace ());
	}
	updateDoAoNames ();
	updateVarNames ();
	
}

void QtScriptWindow::updateVarNames() {
	auto params = auxWin->getAllParams ();
	masterScript.highlighter->setOtherParams (params);
	masterScript.highlighter->setLocalParams (masterScript.getLocalParams ());
	masterScript.highlighter->rehighlight ();
	intensityAgilent.agilentScript.highlighter->setOtherParams (params);
	intensityAgilent.agilentScript.highlighter->setLocalParams (intensityAgilent.agilentScript.getLocalParams ());
	intensityAgilent.agilentScript.highlighter->rehighlight ();
	niawg.niawgScript.highlighter->setOtherParams (params);
	niawg.niawgScript.highlighter->setLocalParams (niawg.niawgScript.getLocalParams ());
	niawg.niawgScript.highlighter->rehighlight ();
}

void QtScriptWindow::updateDoAoNames () {
	auto doNames = auxWin->getTtlNames ();
	auto aoInfo = auxWin->getDacInfo ();
	std::vector<std::string> aoNames;
	for (auto dacInfo : aoInfo) {
		aoNames.push_back (dacInfo.name);
	}
	masterScript.highlighter->setTtlNames (doNames);
	masterScript.highlighter->setDacNames (aoNames);
	intensityAgilent.agilentScript.highlighter->setTtlNames (doNames);
	intensityAgilent.agilentScript.highlighter->setDacNames (aoNames);
	niawg.niawgScript.highlighter->setTtlNames (doNames);
	niawg.niawgScript.highlighter->setDacNames (aoNames);
}

void QtScriptWindow::handleControlNiawgCheck (){
	niawg.updateWindowEnabled ();
}

void QtScriptWindow::handleMasterFunctionChange (){
	try{
		masterScript.functionChangeHandler (mainWin->getProfileSettings ().configLocation);
		masterScript.updateSavedStatus (true);
	}
	catch (ChimeraError& err){
		errBox (err.trace ());
	}
}


void QtScriptWindow::handleIntensityCombo (){
	intensityAgilent.checkSave (mainWin->getProfileSettings ().configLocation, mainWin->getRunInfo ());
	intensityAgilent.readGuiSettings ();
	intensityAgilent.handleModeCombo ();
	intensityAgilent.updateSettingsDisplay (mainWin->getProfileSettings ().configLocation, mainWin->getRunInfo ());
}

void QtScriptWindow::checkScriptSaves (){
	niawg.niawgScript.checkSave (getProfile ().configLocation, mainWin->getRunInfo ());
	intensityAgilent.checkSave (getProfile ().configLocation, mainWin->getRunInfo ());
	masterScript.checkSave (getProfile ().configLocation, mainWin->getRunInfo ());
}

std::string QtScriptWindow::getSystemStatusString (){
	std::string status = "Intensity Agilent:\n\t" + intensityAgilent.getDeviceIdentity ();
	status = "\nNIAWG:\n";
	if (!NIAWG_SAFEMODE){
		status += "\tCode System is Active!\n";
		try{
			status += "\t" + niawg.core.fgenFlume.getDeviceInfo ();
		}
		catch (ChimeraError& err){
			status += "\tFailed to get device info! Error: " + err.trace ();
		}
	}
	else{
		status += "\tCode System is disabled! Enable in \"constants.h\"\n";
	}
	return status;
}

void QtScriptWindow::sendNiawgSoftwareTrig (){
	niawg.core.fgenFlume.sendSoftwareTrigger ();
}

void QtScriptWindow::streamNiawgWaveform (){
	niawg.core.streamWaveform ();
}

/* 
  This function retuns the names (just the names) of currently active scripts.
*/
scriptInfo<std::string> QtScriptWindow::getScriptNames (){
	scriptInfo<std::string> names;
	names.niawg = niawg.niawgScript.getScriptName ();
	names.intensityAgilent = intensityAgilent.agilentScript.getScriptName ();
	names.master = masterScript.getScriptName ();
	return names;
}

/*
  This function returns indicators of whether a given script has been saved or not.
*/
scriptInfo<bool> QtScriptWindow::getScriptSavedStatuses (){
	scriptInfo<bool> status;
	status.niawg = niawg.niawgScript.savedStatus ();
	status.intensityAgilent = intensityAgilent.agilentScript.savedStatus ();
	status.master = masterScript.savedStatus ();
	return status;
}

/*
  This function returns the current addresses of all files in all scripts.
*/
scriptInfo<std::string> QtScriptWindow::getScriptAddresses (){
	scriptInfo<std::string> addresses;
	addresses.niawg = niawg.niawgScript.getScriptPathAndName ();
	addresses.intensityAgilent = intensityAgilent.agilentScript.getScriptPathAndName ();
	addresses.master = masterScript.getScriptPathAndName ();
	return addresses;
}


void QtScriptWindow::setIntensityDefault (){
	try{
		intensityAgilent.setDefault (1);
	}
	catch (ChimeraError& err){
		reportErr (err.qtrace ());
	}
}

/// Commonly Called Functions
/*
	The following set of functions, mostly revolving around saving etc. of the script files, are called by all of the
	window objects because they are associated with the menu at the top of each screen
*/
void QtScriptWindow::newIntensityScript (){
	try{
		intensityAgilent.verifyScriptable ();
		intensityAgilent.checkSave (getProfile ().configLocation, mainWin->getRunInfo ());
		intensityAgilent.agilentScript.newScript ();
		updateConfigurationSavedStatus (false);
		intensityAgilent.agilentScript.updateScriptNameText (mainWin->getProfileSettings ().configLocation);
	}
	catch (ChimeraError& err){
		reportErr (err.qtrace ());
	}
}

void QtScriptWindow::openIntensityScript (IChimeraQtWindow* parent){
	try{
		intensityAgilent.verifyScriptable ();
		intensityAgilent.checkSave (getProfile ().configLocation, mainWin->getRunInfo ());
		std::string intensityOpenName = openWithExplorer (parent, Script::AGILENT_SCRIPT_EXTENSION);
		intensityAgilent.agilentScript.openParentScript ( intensityOpenName, getProfile ().configLocation,
														  mainWin->getRunInfo () );
		updateConfigurationSavedStatus (false);
		intensityAgilent.agilentScript.updateScriptNameText (getProfile ().configLocation);
	}
	catch (ChimeraError& err){
		reportErr (err.qtrace ());
	}
}

void QtScriptWindow::saveIntensityScript (){
	try{
		// channel 0 is the intensity channel, the 4th option is the scripting option.
		if (intensityAgilent.getOutputInfo ().channel[0].option == AgilentChannelMode::which::Script){
			intensityAgilent.agilentScript.saveScript (getProfile ().configLocation, mainWin->getRunInfo ());
			intensityAgilent.agilentScript.updateScriptNameText (getProfile ().configLocation);
		}
	}
	catch (ChimeraError& err){
		reportErr (err.qtrace ());
	}
}

void QtScriptWindow::saveIntensityScriptAs (IChimeraQtWindow* parent){
	try{
		intensityAgilent.verifyScriptable ();
		std::string extensionNoPeriod = intensityAgilent.agilentScript.getExtension ();
		if (extensionNoPeriod.size () == 0){
			return;
		}
		extensionNoPeriod = extensionNoPeriod.substr (1, extensionNoPeriod.size ());
		std::string newScriptAddress = saveWithExplorer (parent, extensionNoPeriod, getProfileSettings ());
		intensityAgilent.agilentScript.saveScriptAs (newScriptAddress, mainWin->getRunInfo ());
		updateConfigurationSavedStatus (false);
		intensityAgilent.agilentScript.updateScriptNameText (getProfile ().configLocation);
	}
	catch (ChimeraError& err){
		reportErr (err.qtrace ());
	}
}

// just a quick shortcut.
profileSettings QtScriptWindow::getProfile (){
	return mainWin->getProfileSettings ();
}

void QtScriptWindow::newNiawgScript (){
	try	{
		niawg.niawgScript.checkSave (getProfile ().configLocation, mainWin->getRunInfo ());
		niawg.niawgScript.newScript ();
		updateConfigurationSavedStatus (false);
		niawg.niawgScript.updateScriptNameText (getProfile ().configLocation);
	}
	catch (ChimeraError& err){
		reportErr (err.qtrace ());
	}
}

void QtScriptWindow::openNiawgScript (IChimeraQtWindow* parent){
	try{
		niawg.niawgScript.checkSave (getProfile ().configLocation, mainWin->getRunInfo ());
		std::string horizontalOpenName = openWithExplorer (parent, Script::NIAWG_SCRIPT_EXTENSION);
		niawg.niawgScript.openParentScript (horizontalOpenName, getProfile ().configLocation, mainWin->getRunInfo ());
		updateConfigurationSavedStatus (false);
		niawg.niawgScript.updateScriptNameText (getProfile ().configLocation);
	}
	catch (ChimeraError& err){
		reportErr (err.qtrace ());
	}
}

void QtScriptWindow::saveNiawgScript (){
	try{
		niawg.niawgScript.saveScript (getProfile ().configLocation, mainWin->getRunInfo ());
		niawg.niawgScript.updateScriptNameText (getProfile ().configLocation);
	}
	catch (ChimeraError& err){
		reportErr (err.qtrace ());
	}
}

void QtScriptWindow::saveNiawgScriptAs (IChimeraQtWindow* parent){
	std::string extensionNoPeriod = niawg.niawgScript.getExtension ();
	if (extensionNoPeriod.size () == 0){
		return;
	}
	extensionNoPeriod = extensionNoPeriod.substr (1, extensionNoPeriod.size ());
	std::string newScriptAddress = saveWithExplorer (parent, extensionNoPeriod,	getProfileSettings ());
	niawg.niawgScript.saveScriptAs (newScriptAddress, mainWin->getRunInfo ());
	updateConfigurationSavedStatus (false);
	niawg.niawgScript.updateScriptNameText (getProfile ().configLocation);
}

void QtScriptWindow::updateScriptNamesOnScreen (){
	niawg.niawgScript.updateScriptNameText (getProfile ().configLocation);
	niawg.niawgScript.updateScriptNameText (getProfile ().configLocation);
	intensityAgilent.agilentScript.updateScriptNameText (getProfile ().configLocation);
}

void QtScriptWindow::openIntensityScript (std::string name){
	intensityAgilent.agilentScript.openParentScript (name, getProfile ().configLocation, mainWin->getRunInfo ());
}

void QtScriptWindow::windowOpenConfig (ConfigStream& configFile){
	CodeTimer timer;
	timer.tick("Init");
	try{
		ConfigSystem::initializeAtDelim (configFile, "SCRIPTS");
	}
	catch (ChimeraError&){
		reportErr ("Failed to initialize configuration file at scripting window entry point \"SCRIPTS\".");
		return;
	}
	try{
		timer.tick("Getting Script Names...");
		configFile.get ();
		auto getlineFunc = ConfigSystem::getGetlineFunc (configFile.ver);
		std::string niawgName, masterName;
		if (configFile.ver.versionMajor < 3){
			std::string extraNiawgName;
			getlineFunc (configFile, extraNiawgName);
		}
		getlineFunc (configFile, niawgName);
		getlineFunc (configFile, masterName);
		ConfigSystem::checkDelimiterLine (configFile, "END_SCRIPTS");
		deviceOutputInfo info;
		timer.tick("Getting Int Agilent...");
		ConfigSystem::stdGetFromConfig (configFile, intensityAgilent.getCore (), info, Version ("4.0"));
		timer.tick("Setting Agilent Stuff");
		intensityAgilent.setOutputSettings (info);
		intensityAgilent.updateSettingsDisplay (mainWin->getProfileSettings ().configLocation, mainWin->getRunInfo ());
		try{
			timer.tick("Opening Niawg Script");
			openNiawgScript (niawgName);
		}
		catch (ChimeraError& err){
			auto answer = QMessageBox::question (this, "Open Failed",
				"ERROR: Failed to open NIAWG script file: " + qstr(niawgName) + ", with error \r\n"
				+ err.qtrace () + "\r\nAttempt to find file yourself?");
			if (answer == QMessageBox::Yes){
				openNiawgScript (openWithExplorer (nullptr, "nScript"));
			}
		}
		try{
			timer.tick("Opening Master Script");
			openMasterScript (masterName);
		}
		catch (ChimeraError& err){
			auto answer = QMessageBox::question (this, "Open Failed", "ERROR: Failed to open master script file: " 
				+ qstr(masterName) + ", with error \r\n" + err.qtrace () + "\r\nAttempt to find file yourself?");
			if (answer == QMessageBox::Yes){
				openMasterScript (openWithExplorer (nullptr, "mScript"));
			}
		}
		considerScriptLocations ();
		timer.tick("Niawg Open Config");
		niawg.handleOpenConfig (configFile);
		niawg.updateWindowEnabled ();
		//errBox(timer.getTimingMessage());
	}
	catch (ChimeraError& err)	{
		reportErr ("Scripting Window failed to read parameters from the configuration file.\n\n" + err.qtrace ());
	}
}

void QtScriptWindow::newMasterScript (){
	try {
		masterScript.checkSave (getProfile ().configLocation, mainWin->getRunInfo ());
		masterScript.newScript ();
		updateConfigurationSavedStatus (false);
		masterScript.updateScriptNameText (getProfile ().configLocation);
	}
	catch (ChimeraError & err) {
		reportErr (err.qtrace ());
	}
}

void QtScriptWindow::openMasterScript (IChimeraQtWindow* parent){
	try	{
		masterScript.checkSave (getProfile ().configLocation, mainWin->getRunInfo ());
		std::string openName = openWithExplorer (parent, Script::MASTER_SCRIPT_EXTENSION);
		masterScript.openParentScript (openName, getProfile ().configLocation, mainWin->getRunInfo ());
		updateConfigurationSavedStatus (false);
		masterScript.updateScriptNameText (getProfile ().configLocation);
	}
	catch (ChimeraError& err){
		reportErr ("Open Master Script Failed: " + err.qtrace () + "\r\n");
	}
}

void QtScriptWindow::saveMasterScript (){
	if (masterScript.isFunction ())	{
		masterScript.saveAsFunction ();
		return;
	}
	masterScript.saveScript (getProfile ().configLocation, mainWin->getRunInfo ());
	masterScript.updateScriptNameText (getProfile ().configLocation);
}

void QtScriptWindow::saveMasterScriptAs (IChimeraQtWindow* parent){
	std::string extensionNoPeriod = masterScript.getExtension ();
	if (extensionNoPeriod.size () == 0)	{
		return;
	}
	extensionNoPeriod = extensionNoPeriod.substr (1, extensionNoPeriod.size ());
	std::string newScriptAddress = saveWithExplorer (parent, extensionNoPeriod, getProfileSettings ());
	masterScript.saveScriptAs (newScriptAddress, mainWin->getRunInfo ());
	updateConfigurationSavedStatus (false);
	masterScript.updateScriptNameText (getProfile ().configLocation);
}

void QtScriptWindow::newMasterFunction (){
	try{
		masterScript.newFunction ();
	}
	catch (ChimeraError& exception){
		reportErr ("New Master function Failed: " + exception.qtrace () + "\r\n");
	}
}

void QtScriptWindow::saveMasterFunction (){
	try{
		masterScript.saveAsFunction ();
	}
	catch (ChimeraError& exception){
		reportErr ("Save Master Script Function Failed: " + exception.qtrace () + "\r\n");
	}
}

void QtScriptWindow::deleteMasterFunction (){
	// todo. Right now you can just delete the file itself...
}

void QtScriptWindow::windowSaveConfig (ConfigStream& saveFile){
	scriptInfo<std::string> addresses = getScriptAddresses ();
	// order matters!
	saveFile << "SCRIPTS\n";
	saveFile << "/*NIAWG Script Address:*/ " << addresses.niawg << "\n";
	saveFile << "/*Master Script Address:*/ " << addresses.master << "\n";
	saveFile << "END_SCRIPTS\n";
	intensityAgilent.handleSavingConfig (saveFile, mainWin->getProfileSettings ().configLocation, mainWin->getRunInfo ());
	niawg.handleSaveConfig (saveFile);
}

void QtScriptWindow::checkMasterSave (){
	masterScript.checkSave (getProfile ().configLocation, mainWin->getRunInfo ());
}

void QtScriptWindow::openMasterScript (std::string name){
	masterScript.openParentScript (name, getProfile ().configLocation, mainWin->getRunInfo ());
}

void QtScriptWindow::openNiawgScript (std::string name){
	niawg.niawgScript.openParentScript (name, getProfile ().configLocation, mainWin->getRunInfo ());
}

void QtScriptWindow::considerScriptLocations (){
	niawg.niawgScript.considerCurrentLocation (getProfile ().configLocation, mainWin->getRunInfo ());
	intensityAgilent.agilentScript.considerCurrentLocation (getProfile ().configLocation, mainWin->getRunInfo ());
}

void QtScriptWindow::updateProfile (std::string text){
	profileDisplay.update (text);
}

profileSettings QtScriptWindow::getProfileSettings (){
	return mainWin->getProfileSettings ();
}

void QtScriptWindow::updateConfigurationSavedStatus (bool status){
	mainWin->updateConfigurationSavedStatus (status);
}

void QtScriptWindow::setNiawgRunningState (bool newRunningState){
	niawg.core.setRunningState (newRunningState);
}

bool QtScriptWindow::niawgIsRunning () { return niawg.core.niawgIsRunning (); }
void QtScriptWindow::setNiawgDefaults () { 
	try {
		niawg.core.setDefaultWaveforms ();
	}
	catch (ChimeraError & err) {
		reportErr (err.qtrace ());
	}
}
void QtScriptWindow::restartNiawgDefaults () { niawg.core.restartDefault (); }
NiawgCore& QtScriptWindow::getNiawg () { return niawg.core; }
void QtScriptWindow::stopNiawg () { niawg.core.turnOff (); }

void QtScriptWindow::passNiawgIsOnPress (){
	if (niawg.core.isOn ()){
		niawg.core.turnOff ();
		//mainWin->checkAllMenus (ID_NIAWG_NIAWGISON, MF_UNCHECKED);
	}
	else{
		niawg.core.turnOn ();
		//mainWin->checkAllMenus (ID_NIAWG_NIAWGISON, MF_CHECKED);
	}
}

std::string QtScriptWindow::getNiawgErr (){
	return niawg.core.fgenFlume.getErrorMsg (-1);
}

void QtScriptWindow::passRerngModeComboChange (){
	niawg.rearrangeCtrl.updateActive ();
}

void QtScriptWindow::passExperimentRerngButton (){
	niawg.rearrangeCtrl.updateActive ();
}

void QtScriptWindow::fillExpDeviceList (DeviceList& list){
	list.list.push_back (niawg.core);
	list.list.push_back (intensityAgilent.getCore ());
}
