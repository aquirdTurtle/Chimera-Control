// created by Mark O. Brown
#include "stdafx.h"

#include "Scripts/Script.h"

#include "PrimaryWindows/IChimeraQtWindow.h"
#include "GeneralUtilityFunctions/cleanString.h"
#include "ParameterSystem/ParameterSystem.h"
#include "ConfigurationSystems/ConfigSystem.h"
#include "PrimaryWindows/QtAuxiliaryWindow.h"
#include "DigitalOutput/DoSystem.h"
#include "GeneralObjects/RunInfo.h"
#include <PrimaryWindows/QtMainWindow.h>
#include <ExperimentThread/ExpThreadWorker.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include "boost/lexical_cast.hpp"
#include <qcombobox.h>
#include <QInputDialog.h>

void Script::initialize (int width, int height, QPoint& loc, IChimeraQtWindow* parent,
	std::string deviceTypeInput, std::string scriptHeader){
	auto& px = loc.rx (), & py = loc.ry ();
	deviceType = deviceTypeInput;
	ScriptableDevice devenum;
	if (deviceTypeInput == "NIAWG") {
		devenum = ScriptableDevice::NIAWG;
		extension = str (".") + NIAWG_SCRIPT_EXTENSION;
	}
	else if (deviceTypeInput == "Agilent") {
		devenum = ScriptableDevice::Agilent;
		extension = str (".") + AGILENT_SCRIPT_EXTENSION;
	}
	else if (deviceTypeInput == "Master") {
		devenum = ScriptableDevice::Master;
		extension = str (".") + MASTER_SCRIPT_EXTENSION;
	}
	else {
		thrower (": Device input type not recognized during construction of script control.  (A low level bug, "
			"this shouldn't happen)");
	}
	if (scriptHeader != "")	{
		title = new QLabel (cstr (scriptHeader), parent);
		title->setGeometry (px, py, width, 25);
		py += 25;
	}
	savedIndicator = new CQCheckBox ("Saved?", parent);
	savedIndicator->setGeometry (px, py, 80, 20);
	savedIndicator->setChecked (true);
	savedIndicator->setEnabled (false);
	fileNameText = new QLabel ("", parent);
	fileNameText->setGeometry (px + 80, py, width - 100, 20);
	isSaved = true;
	help = new QLabel ("?", parent);
	help->setGeometry (px + width - 20, py, 20, 20);
	if (deviceType == "Master"){
		help->setToolTip ("This is a script for programming master timing for TTLs, DACs, the RSG, and the raman outputs.\n"
							"Acceptable Commands:\n"
							"-      t++\n"
							"-      t+= [number] (space between = and number required)\n"
							"-      t= [number] (space between = and number required)\n"
							"-      on: [ttlName]\n"
							"-      off: [ttlName]\n"
							"-      pulseon: [ttlName] [pulseLength]\n"
							"-      pulseoff: [ttlName] [pulseLength]\n"
							"-      dac: [dacName] [voltage]\n"
							"-      dacarange: [dacName] [initValue] [finalValue] [rampTime] [rampInc]\n"
							"-      daclinspace: [dacName] [initValue] [finalValue] [rampTime] [numberOfSteps]\n"
							"-      def [functionName]([functionArguments]):\n"
							"-      call [functionName(argument1, argument2, etc...)]\n"
							"-      repeat: [numberOfTimesToRepeat]\n"
							"-           %Commands...\n"

							"-      end % (of repeat)\n"
							"-      callcppcode\n"
							"-      % marks a line as a comment. %% does the same and gives you a different color.\n"
							"-      extra white-space is generally fine and doesn't screw up analysis of the script. Format as you like.\n"
							"-      Simple Math (+-/*) is supported in the scripts as well. To insert a mathematical expresion, just \n"
							"-      add parenthesis () around the full expression");
	}
	else if( deviceType == "NIAWG"){
		help->setToolTip (">>> This is a script for programming the NI AWG 5451. <<<\n"
							"- the input format is referenced below using angled brackets <...>. Place the input on\n"
							" the line below the command in the format specified.\n"
							"- The ramping type options are currently \"lin\", \"tanh\" and \"nr\".\n"
							"- The associated c++ code has been designed to be flexible when it comes to trailing white spaces at the ends of\n"
							" lines and in between commands, so use whatever such formatting pleases your eyes.\n"
							"Accepted Commands:\n"
							"(a)Wait Commands\n"
							"\"waitTilTrig\"\n"
							"\"waitSet# <# of samples to wait>\"\n"
							"(b)Repeat Commands\n"
							"\"repeatSet# <# of times to repeat>\"\n"
							"\"repeatTilTrig\"\n"
							"\"repeatForever\"\n"
							"\"endRepeat\"\n"
							"(c)Logic Commands\n"
							"\"ifTrig\"\n"
							"\"else\"\n"
							"\"endIf\"\n"
							"(d)Constant Waveforms\n"
							"\"gen2const <freq1> <amp1> <phase1 (rad)>; <sim for 2nd>; <time> <t manage>\"\n"
							"(e)Amplitude Ramps\n"
							"\"gen2ampramp <freq1> <amp1 ramp type> <initial amp1> <final amp1> <phase1 (rad)>; <sim for 2nd>; <time> <t manage>\"\n"
							"(f)frequency Ramps\n"
							"\"gen2freq ramp <freq1 ramp type> <initial freq1> <final freq1> <amp1> <phase1 (rad)>; <sim for 2nd>; <time> <t manage>\"\n"
							"Etc.\n"
							"(g)Amplitude and Frequency Ramps\n"
							"\"gen2freq&ampramp <freq1 ramp type> <init freq1> <fin freq1> <amp ramp1 type> <init ramp1> <fin ramp1> <phase1 (rad)>;...\n"
							"...<similar for 2nd>; <time> <t manage>\"\n"
							"(j)Create marker event after last waveform\n"
							"\"markerEvent <samples after previous waveform to wait>\"\n");
	}
	else if (deviceType == "Agilent"){
		help->setToolTip (">>> Scripted Agilent Waveform Help <<<\n"
						"Accepted Commands (syntax for command is encased in <>)\n"
						"- hold <val> <time(ms)> <Continuation Type> <Possibly Repeat #> <#>\n"
						"- ramp <type> <initVal> <finVal(V)> <time(ms)> <Continuation Type> <Possibly Repeat #> <#>\n"
						"- pulse <pulse type> <vOffset> <amp> <pulse-width> <time-offset (ms)> <time(ms)> <Continuation Type> <Possibly Repeat #> <#>\n"
						"- modPulse <pulse-type> <vOffset> <amp> <pulse-width> <t-offset (ms)> <mod-Freq(MHz)> <mod-Phase(Rad)> <time(ms)> <Continuation Type> <Repeat #>\n"
						"The continuation type determines what the agilent does when it reaches the end of the <time> \n"
						"argument. Accepted Values for the continuation type are:\n"
						"- repeat <requires repeat #>\n"
						"- repeatUntilTrig\n"
						"- once\n"
						"- repeatForever\n"
						"- onceWaitTrig\n"
						"Accepted ramp types are:\n"
						"- nr (no ramp)\n"
						"- lin\n"
						"- tanh\n"
						"Accepted pulse types are:\n"
						"- sech, ~ sech(time/width)\n"
						"- gaussian, width = gaussian sigma\n"
						"- lorentzian, width = FWHM (curve is normalized)\n");
	}
	else{
		help->setToolTip ("No Help available");
	}
	py += 20;
	availableFunctionsCombo.combo = new CQComboBox (parent);
	availableFunctionsCombo.combo->setGeometry (px, py, width, 25);
	loadFunctions ();
	availableFunctionsCombo.combo->setCurrentIndex (0);
	parent->connect (availableFunctionsCombo.combo, qOverload<int> (&QComboBox::activated), [this, parent]() {
		try {
			auto addr = ConfigSystem::getMasterAddressFromConfig (parent->mainWin->getProfileSettings ());
			functionChangeHandler (addr);
		}
		catch (ChimeraError & err) {
			parent->reportErr (err.qtrace ());
		}});
	py += 25;

	edit = new CQTextEdit ("", parent);
	edit->setGeometry (px, py, width, height);
	edit->setAcceptRichText (false);
	QFont font;
	font.setFamily ("Courier");
	font.setStyleHint (QFont::Monospace);
	font.setFixedPitch (true);
	font.setPointSize (10);
	edit->setFont (font);
	edit->setTabStopDistance (40);

	parent->connect (edit, &QTextEdit::textChanged, [this, parent]() { updateSavedStatus (false); });
	highlighter = new SyntaxHighlighter (devenum, edit->document ());
	py += height;
}

void Script::functionChangeHandler(std::string configPath){
	int selection = availableFunctionsCombo.combo->currentIndex( );
	if ( selection != -1 ){
		std::string text = str (availableFunctionsCombo.combo->currentText ());
		text = text.substr( 0, text.find_first_of( '(' ) );
		changeView(text, true, configPath );
	}
}

Script::Script(IChimeraQtWindow* parent) : IChimeraSystem(parent) {
	isSaved = true;
	editChangeEnd = 0;
	editChangeBegin = ULONG_MAX;
}

std::string Script::getScriptPath(){
	return scriptPath;
}

std::string Script::getScriptText(){
	if (!edit){
		return "";
	}
	return str(edit->toPlainText());
}

void Script::updateSavedStatus(bool scriptIsSaved){
	isSaved = scriptIsSaved;
	savedIndicator->setChecked ( scriptIsSaved );
}


std::vector<parameterType> Script::getLocalParams () {
	if (!edit) { return{}; }
	auto text = edit->toPlainText ();
	std::stringstream fileTextStream = std::stringstream (str (text));
	ScriptStream ss (fileTextStream.str ());
	auto localVars = ExpThreadWorker::getLocalParameters (ss);
	return localVars;
}

void Script::changeView(std::string viewName, bool isFunction, std::string configPath){
	if (viewName == "Parent Script"){
		loadFile(configPath);
	}
	else if (isFunction){
		loadFile(FUNCTIONS_FOLDER_LOCATION + viewName + "." + FUNCTION_EXTENSION);
	}
	else{
		// load child
		loadFile(configPath + viewName);
	}

	// colorEntireScript(params, rgbs, ttlNames, dacNames);
	// the view is fresh from a file, so it's saved.
	updateSavedStatus(true);
}


bool Script::isFunction ( ){
	int sel = availableFunctionsCombo.combo->currentIndex();
	QString text;
	if ( sel != -1 ){
		text = availableFunctionsCombo.combo->currentText();
	}
	else{
		text = "";
	}
	return text != "Parent Script";// && text != "";
}

//
void Script::saveScript(std::string configPath, RunInfo info){
	if (configPath == ""){
		thrower (": Please select a configuration before trying to save a script!\r\n");
	}
	if (isSaved && scriptName != ""){
		// shoudln't need to do anything
		return;
	}
	if ( isFunction() ){
		errBox( "The current view is not the parent view. Please switch to the parent view before saving to "
				"save the script, or use the save-function option to save the current function." );
		return;
	}
	if ( scriptName == "" ){
		std::string newName;
		newName = str(QInputDialog::getText (edit, "New Script Name", ("Please enter new name for the " + deviceType + " script " + scriptName + ".",
											 scriptName).c_str()));
		if (newName == ""){
			// canceled
			return;
		}
		std::string path = configPath + newName + extension;
		saveScriptAs(path, info);
	}
	if (info.running){
		for (unsigned scriptInc = 0; scriptInc < info.currentlyRunningScripts.size(); scriptInc++){
			if (scriptName == info.currentlyRunningScripts[scriptInc]){
				thrower ("System is currently running. You can't save over any files in use by the system while"
						 " it runs, which includes the NIAWG scripts and the intensity script.");
			}
		}
	}
	auto text = edit->toPlainText();
	std::fstream saveFile(configPath + scriptName + extension, std::fstream::out);
	if (!saveFile.is_open()){
		thrower ("Failed to open script file: " + configPath + scriptName + extension);
	}
	saveFile << str(text);
	saveFile.close();
	scriptFullAddress = configPath + scriptName + extension;
	scriptPath = configPath;
	updateSavedStatus(true);
	emit notification({ "Finished saving script.\n", 0, deviceType });
}

//
void Script::saveScriptAs(std::string location, RunInfo info){
	if (location == ""){
		return;
	}
	if (info.running){
		for (unsigned scriptInc = 0; scriptInc < info.currentlyRunningScripts.size(); scriptInc++){
			if (scriptName == info.currentlyRunningScripts[scriptInc]){
				thrower ("System is currently running. You can't save over any files in use by the system while "
						 "it runs, which includes the horizontal and vertical AOM scripts and the intensity script.");
			}
		}
	}
	auto text = edit->toPlainText();
	std::fstream saveFile(location, std::fstream::out);
	if (!saveFile.is_open()){
		thrower ("Failed to open script file: " + location);
	}
	saveFile << str(text);
	char fileChars[_MAX_FNAME];
	char dirChars[_MAX_FNAME];
	char pathChars[_MAX_FNAME];
	int myError = _splitpath_s(cstr(location), dirChars, _MAX_FNAME, pathChars, _MAX_FNAME, fileChars, _MAX_FNAME, nullptr, 0);
	scriptName = str(fileChars);
	scriptPath = str(fileChars) + str(pathChars);
	saveFile.close();
	scriptFullAddress = location;
	updateScriptNameText(location);
	updateSavedStatus(true);
	emit notification({ "Finished saving script.\n", 0, deviceType });
}

//
void Script::checkSave(std::string configPath, RunInfo info){
	if (isSaved){
		// don't need to do anything
		return;
	}
	// test first non-commented word of text to see if this looks like a function or not.
	auto text = edit->toPlainText();
	ScriptStream tempStream;
	tempStream << str(text);
	std::string word;
	tempStream >> word;
	if (word == "def"){
		auto res = QMessageBox::question (nullptr, "Save?", qstr("Current " + deviceType + " function file is unsaved. Save it?"),
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (res == QMessageBox::Cancel){
			thrower ("Cancel!");
		}
		else if (res == QMessageBox::No){
			return;
		}
		else if (res == QMessageBox::Yes){
			saveAsFunction();
			return;
		}
	}
	// else it's a normal script file.
	if (scriptName == ""){
		auto res = QMessageBox::question (nullptr, "Save?", qstr ("Current " + deviceType + " script file is unsaved and unnamed. Save it with a with new name?"),
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (res == QMessageBox::Cancel){
			thrower ("Cancel!");
		}
		else if (res == QMessageBox::No){
			return;
		}
		else if (res == QMessageBox::Yes){
			std::string newName;
			newName = str (QInputDialog::getText (edit, "New Script Name", ("Please enter new name for the " + deviceType + " script " + scriptName + ".",
				scriptName).c_str ()));
			std::string path = configPath + newName + extension;
			saveScriptAs(path, info);
			return;
		}
	}
	else{
		auto answer = QMessageBox::question (nullptr, qstr ("Save Script?"),
			qstr ("The " + deviceType + " script file is unsaved. Save it as " + scriptName + extension + "?"), 
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );
		if (answer == QMessageBox::Cancel) {
			thrower ("Cancel!");
		}
		if (answer == QMessageBox::No) {}
		if (answer == QMessageBox::Yes) {
			saveScript(configPath, info);
		}
	}
}


//
void Script::renameScript(std::string configPath){
	if (scriptName == ""){
		// ??? don't know why I need this here.
		return;
	}
	std::string newName;
	newName = str (QInputDialog::getText (edit, "New Script Name", ("Please enter new name for the " + deviceType
		+ " script " + scriptName + ".",
		scriptName).c_str ()));
	if (newName == ""){
		// canceled
		return;
	}
	int result = MoveFile(cstr(configPath + scriptName + extension), cstr(configPath + newName + extension));
	if (result == 0){
		thrower ("Failed to rename file. (A low level bug? this shouldn't happen)");
	}
	scriptFullAddress = configPath + scriptName + extension;
	scriptPath = configPath;
}

//
void Script::deleteScript(std::string configPath){
	if (scriptName == "") {
		return;
	}
	// check to make sure:
	auto answer = QMessageBox::question (nullptr, qstr ("Delete Script?"), 
		qstr ("Are you sure you want to delete the script file " + scriptName + "?"), QMessageBox::Yes | QMessageBox::No);
	if (answer == QMessageBox::No) {
		return;
	}
	int result = DeleteFile(cstr(configPath + scriptName + extension));
	if (result == 0){
		thrower ("Deleting script file failed!  (A low level bug, this shouldn't happen)");
	}
	else {
		scriptName = "";
		scriptPath = "";
		scriptFullAddress = "";
	}
}


// the differences between this and new script are that this opens the default function instead of the default script
// and that this does not reset the script config, etc. 
void Script::newFunction(){
	std::string tempName;
	tempName = DEFAULT_SCRIPT_FOLDER_PATH;
	if (deviceType == "Master"){
		tempName += "DEFAULT_FUNCTION.func";
	}
	else{
		thrower ("tried to load new function with non-master script? Only the master script supports functions"
				 " currently.");
	}
	loadFile(tempName);
	availableFunctionsCombo.combo->setCurrentIndex (-1);
}

//
void Script::newScript(){
	std::string tempName;
	tempName = DEFAULT_SCRIPT_FOLDER_PATH;
	if (deviceType == "NIAWG"){
		tempName += "DEFAULT_SCRIPT.nScript";
	}
	else if (deviceType == "Agilent"){
		tempName += "DEFAULT_INTENSITY_SCRIPT.aScript";
	}
	else if (deviceType == "Master"){
		tempName += "DEFAULT_MASTER_SCRIPT.mScript";
	}	
	reset();
	loadFile(tempName);
}


void Script::openParentScript(std::string parentScriptFileAndPath, std::string configPath, RunInfo info){
	if (parentScriptFileAndPath == "" || parentScriptFileAndPath == "NONE"){
		return;
	}
	char fileChars[_MAX_FNAME];
	char extChars[_MAX_EXT];
	char dirChars[_MAX_FNAME];
	char pathChars[_MAX_FNAME];
	int myError = _splitpath_s(cstr(parentScriptFileAndPath), dirChars, _MAX_FNAME, pathChars, _MAX_FNAME, fileChars, 
								_MAX_FNAME, extChars, _MAX_EXT);
	std::string extStr(extChars);
	if (deviceType == "NIAWG"){
		if (extStr != str(".") + NIAWG_SCRIPT_EXTENSION){
			thrower ("Attempted to open non-NIAWG script inside NIAWG script control.");
		}
	}
	else if (deviceType == "Agilent"){
		if (extStr != str( "." ) + AGILENT_SCRIPT_EXTENSION){
			thrower ("Attempted to open non-agilent script from agilent script control.");
		}
	}
	else if (deviceType == "Master"){
		if (extStr != str( "." ) + MASTER_SCRIPT_EXTENSION){
			thrower ("Attempted to open non-master script from master script control!");
		}
	}
	else{
		thrower ("Unrecognized device type inside script control!  (A low level bug, this shouldn't happen).");
	}
	loadFile( parentScriptFileAndPath );
	scriptName = str(fileChars);
	scriptFullAddress = parentScriptFileAndPath;
	updateSavedStatus(true);
	std::string scriptLocation = parentScriptFileAndPath;
	std::replace (scriptLocation.begin (), scriptLocation.end (), '\\', '/');
	int sPos = scriptLocation.find_last_of ('/');
	scriptLocation = scriptLocation.substr(0, sPos);	
	if (scriptLocation + "/" != configPath && configPath != ""){
		auto answer = QMessageBox::question (nullptr, qstr ("Location?"), qstr ("The requested " + deviceType 
			+ " script: " + parentScriptFileAndPath + " is not "
			"currently located in the current configuration folder. This is recommended so that "
			"scripts related to a particular configuration are reserved to that configuration "
			"folder. Copy script to current configuration folder?"), QMessageBox::Yes | QMessageBox::No);
		if (answer == QMessageBox::Yes){
			std::string scriptName = parentScriptFileAndPath.substr(sPos+1, parentScriptFileAndPath.size());
			std::string path = configPath + scriptName;
			saveScriptAs(path, info);
		}
	}
	updateScriptNameText( configPath );
	int index = availableFunctionsCombo.combo->findText ("Parent Script");
	if (index != -1) { // -1 for not found
		availableFunctionsCombo.combo->setCurrentIndex (index);
	}
}

/*
]---	This function only puts the given file on the edit for this class, it doesn't change current settings parameters. 
		It's used bare when just changing the
]-		view of the edit, while it's used with some surrounding changes for loading a new parent.
 */
void Script::loadFile(std::string pathToFile){
	std::fstream openFile;
	openFile.open(pathToFile, std::ios::in);
	if (!openFile.is_open()){
		reset();
		thrower ("Failed to open script file: " + pathToFile + ".");
	}
	std::string tempLine;
	std::string fileText;
	while (std::getline(openFile, tempLine)){
		//cleanString(tempLine);
		fileText += tempLine + "\n";
	}
	// put the default into the new control.
	edit->setText(cstr(fileText));
	openFile.close();
	emit notification({ qstr("Finished loading " + deviceType + " file\n"),1, qstr(deviceType) });
}


void Script::reset(){
	if (!availableFunctionsCombo.combo || !edit) {
		return;
	}
	int index = availableFunctionsCombo.combo->findText ("Parent Script");
	if (index != -1) { // -1 for not found
		availableFunctionsCombo.combo->setCurrentIndex (index);
	}
	scriptName = "";
	scriptPath = "";
	scriptFullAddress = "";
	updateSavedStatus(false);
	fileNameText->setText("");
	edit->setText("");
}

bool Script::savedStatus(){
	return isSaved;
}

std::string Script::getScriptPathAndName(){
	return scriptFullAddress;
}

std::string Script::getScriptName(){
	return scriptName;
}

void Script::considerCurrentLocation(std::string configPath, RunInfo info){
	if (scriptFullAddress.size() > 0){
		std::string scriptLocation = scriptFullAddress;
		std::replace (scriptLocation.begin (), scriptLocation.end (), '\\', '/');
		int sPos = scriptLocation.find_last_of ('/');
		scriptLocation = scriptLocation.substr (0, sPos);

		if (scriptLocation + "/" != configPath){
			auto answer = QMessageBox::question (nullptr, qstr ("Location?"), qstr ("The requested " + deviceType 
				+ " script location: \"" + scriptLocation + "\" "
				"is not currently located in the current configuration folder. This is recommended"
				" so that scripts related to a particular configuration are reserved to that "
				"configuration folder. Copy script to current configuration folder?"), QMessageBox::Yes | QMessageBox::No);
			if (answer == QMessageBox::Yes) {
				std::string scriptName = scriptFullAddress.substr(sPos, scriptFullAddress.size());
				scriptFullAddress = configPath + scriptName;
				scriptPath = configPath;
				saveScriptAs(scriptFullAddress, info);
			}
		}
	}
}

std::string Script::getExtension(){
	return extension;
}

void Script::updateScriptNameText(std::string configPath){
	// there are some \\ on the endOfWord of the path by default.
	configPath = configPath.substr(0, configPath.size() - 1);
	int sPos = configPath.find_last_of('\\');
	if (sPos != -1)	{
		std::string parentFolder = configPath.substr(sPos + 1, configPath.size());
		std::string text = parentFolder + "->" + scriptName;
		fileNameText->setText(cstr(text));
	}
	else{
		fileNameText->setText(qstr(scriptName));
	}
}

void Script::setScriptText(std::string text){
	if (!edit) {
		return;
	}
	edit->setText( cstr( text ) );
}

void Script::saveAsFunction(){
	// check to make sure that the current script is defined like a function
	auto text = edit->toPlainText();
	ScriptStream stream;
	stream << str(text);
	std::string word;
	stream >> word;
	if (word != "def"){
		thrower ("Function declarations must begin with \"def\".");
	}
	std::string line;
	line = stream.getline( '\r' );
	int pos = line.find_first_of("(");
	if (pos == std::string::npos){
		thrower ("No \"(\" found in function name. If there are no arguments, use empty parenthesis \"()\"");
	}
	int initNamePos = line.find_first_not_of(" \t");
	std::string functionName = line.substr(initNamePos, line.find_first_of("("));
	if (functionName.find_first_of(" ") != std::string::npos){
		thrower ("Function name included a space! Name was" + functionName);
	}
	std::string path = FUNCTIONS_FOLDER_LOCATION + functionName + "." + FUNCTION_EXTENSION;
	FILE *file;
	fopen_s( &file, cstr(path), "r" );
	if ( !file ){
		//
	}
	else{
		emit notification({ "Overwriting function definition for function at " + qstr(path) + "...\r\n", 0, qstr(deviceType) });
		fclose ( file );
	}
	std::fstream functionFile(path, std::ios::out);
	if (!functionFile.is_open()){
		thrower ("the function file failed to open!");
	}
	functionFile << str(text);
	functionFile.close();
	// refresh this.
	loadFunctions();
	int index = availableFunctionsCombo.combo->findText (cstr (functionName));
	if (index != -1) { // -1 for not found
		availableFunctionsCombo.combo->setCurrentIndex (index);
	}
	updateSavedStatus( true );
	emit notification({ "Finished saving script as a function.\n",0, deviceType});
}

void Script::setEnabled ( bool enabled, bool functionsEnabled ){
	if (!availableFunctionsCombo.combo || !edit ) {
		return;
	}
	edit->setReadOnly (!enabled);
	availableFunctionsCombo.combo->setEnabled( functionsEnabled );
}

void Script::loadFunctions(){
	availableFunctionsCombo.loadFunctions( );
}

