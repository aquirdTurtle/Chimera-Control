#include "stdafx.h"
#include "QtScriptWindow.h"
#include <qdesktopwidget.h>
#include <PrimaryWindows/QtScriptWindow.h>
#include <PrimaryWindows/QtAndorWindow.h>
#include <PrimaryWindows/QtAuxiliaryWindow.h>
#include <PrimaryWindows/QtMainWindow.h>
#include <ExcessDialogs/saveWithExplorer.h>
#include <ExcessDialogs/openWithExplorer.h>

QtScriptWindow::QtScriptWindow (QWidget* parent) : IChimeraQtWindow (parent),
		masterScript(this) {
	statBox = new ColorBox ();
	setWindowTitle ("Script Window");
}

QtScriptWindow::~QtScriptWindow (){
}

void QtScriptWindow::initializeWidgets (){
	int id = 2000;
	QPoint startLocation = { 0, 50 };
	startLocation = { 2 * 640, 50 };
	masterScript.initialize (640, 900, startLocation, this, "Master", "Master Script");
	startLocation = { 1000, 25 };
	statBox->initialize (startLocation, this, 900, mainWin->getDevices ());
	profileDisplay.initialize ({ 0,25 }, this);
	updateDoAoNames ();
	updateVarNames ();
}

void QtScriptWindow::updateVarNames() {
	auto params = auxWin->getAllParams ();
	masterScript.highlighter->setOtherParams (params);
	masterScript.highlighter->setLocalParams (masterScript.getLocalParams ());
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

void QtScriptWindow::checkScriptSaves (){
	masterScript.checkSave (getProfile ().configLocation);
}

std::string QtScriptWindow::getSystemStatusString (){
	std::string status;
	return status;
}

/* 
  This function retuns the names (just the names) of currently active scripts.
*/
scriptInfo<std::string> QtScriptWindow::getScriptNames (){
	scriptInfo<std::string> names;
	names.master = masterScript.getScriptName ();
	return names;
}

/*
  This function returns indicators of whether a given script has been saved or not.
*/
scriptInfo<bool> QtScriptWindow::getScriptSavedStatuses (){
	scriptInfo<bool> status;
	status.master = masterScript.savedStatus ();
	return status;
}

/*
  This function returns the current addresses of all files in all scripts.
*/
scriptInfo<std::string> QtScriptWindow::getScriptAddresses (){
	scriptInfo<std::string> addresses;
	addresses.master = masterScript.getScriptPathAndName ();
	return addresses;
}

/// Commonly Called Functions
/*
	The following set of functions, mostly revolving around saving etc. of the script files, are called by all of the
	window objects because they are associated with the menu at the top of each screen
*/

// just a quick shortcut.
profileSettings QtScriptWindow::getProfile (){
	return mainWin->getProfileSettings ();
}

void QtScriptWindow::windowOpenConfig (ConfigStream& configFile){
	try{
		ConfigSystem::initializeAtDelim (configFile, "SCRIPTS");
	}
	catch (ChimeraError&){
		reportErr ("Failed to initialize configuration file at scripting window entry point \"SCRIPTS\".");
		return;
	}
	try{
		configFile.get ();
		auto getlineFunc = ConfigSystem::getGetlineFunc (configFile.ver);
		std::string masterName;
		getlineFunc (configFile, masterName);
		ConfigSystem::checkDelimiterLine (configFile, "END_SCRIPTS");
		try{
			openMasterScript (masterName);
		}
		catch (ChimeraError& err){
			auto answer = QMessageBox::question (this, "Open Failed", "ERROR: Failed to open master script file: " 
				+ qstr(masterName) + ", with error \r\n" + err.qtrace () + "\r\nAttempt to find file yourself?");
			if (answer == QMessageBox::Yes){
				openMasterScript (openWithExplorer (nullptr, "mScript"));
			}
		}
	}
	catch (ChimeraError& err)	{
		reportErr ("Scripting Window failed to read parameters from the configuration file.\n\n" + err.qtrace ());
	}
}

void QtScriptWindow::newMasterScript (){
	try {
		masterScript.checkSave (getProfile ().configLocation);
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
		masterScript.checkSave (getProfile ().configLocation);
		std::string openName = openWithExplorer (parent, Script::MASTER_SCRIPT_EXTENSION);
		masterScript.openParentScript (openName, getProfile ().configLocation);
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
	masterScript.saveScript (getProfile ().configLocation);
	masterScript.updateScriptNameText (getProfile ().configLocation);
}

void QtScriptWindow::saveMasterScriptAs (IChimeraQtWindow* parent){
	std::string extensionNoPeriod = masterScript.getExtension ();
	if (extensionNoPeriod.size () == 0)	{
		return;
	}
	extensionNoPeriod = extensionNoPeriod.substr (1, extensionNoPeriod.size ());
	std::string newScriptAddress = saveWithExplorer (parent, extensionNoPeriod, getProfileSettings ());
	masterScript.saveScriptAs (newScriptAddress);
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
	saveFile << "/*Master Script Address:*/ " << addresses.master << "\n";
	saveFile << "END_SCRIPTS\n";
}

void QtScriptWindow::checkMasterSave (){
	masterScript.checkSave (getProfile ().configLocation);
}

void QtScriptWindow::openMasterScript (std::string name){
	masterScript.openParentScript (name, getProfile ().configLocation);
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

void QtScriptWindow::fillExpDeviceList (DeviceList& list) {
}