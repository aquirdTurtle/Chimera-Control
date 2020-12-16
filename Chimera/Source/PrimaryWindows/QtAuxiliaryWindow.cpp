#include "stdafx.h"
#include "QtAuxiliaryWindow.h"
#include <qdesktopwidget.h>
#include <PrimaryWindows/QtScriptWindow.h>
#include <PrimaryWindows/QtAndorWindow.h>
#include <PrimaryWindows/QtAuxiliaryWindow.h>
#include <PrimaryWindows/QtMainWindow.h>
#include <ExcessDialogs/saveWithExplorer.h>
#include <ExcessDialogs/openWithExplorer.h>
#include <ExcessDialogs/doChannelInfoDialog.h>
#include <ExcessDialogs/AoSettingsDialog.h>

QtAuxiliaryWindow::QtAuxiliaryWindow (QWidget* parent) : IChimeraQtWindow (parent), 
	ttlBoard (this, DOFTDI_SAFEMODE, true),
	aoSys (this, ANALOG_OUT_SAFEMODE), configParamCtrl (this, "CONFIG_PARAMETERS"),
	globalParamCtrl (this, "GLOBAL_PARAMETERS"), dds (this, DDS_SAFEMODE){	
	statBox = new ColorBox ();
	setWindowTitle ("Auxiliary Window");
}

QtAuxiliaryWindow::~QtAuxiliaryWindow () {}

bool QtAuxiliaryWindow::eventFilter (QObject* obj, QEvent* event){
	if (aoSys.eventFilter (obj, event)) {
		try {
			aoSys.forceDacs (ttlBoard.getCore (), { 0, ttlBoard.getCurrentStatus () });
		}
		catch (ChimeraError& err) {
			reportErr (err.qtrace ());
		}
		return true;
	}
	return QMainWindow::eventFilter (obj, event);
}

void QtAuxiliaryWindow::initializeWidgets (){
	QPoint loc{ 0, 25 };
	try{
		statBox->initialize (loc, this, 480, mainWin->getDevices (), 2);
		ttlBoard.initialize (loc, this);
		aoSys.initialize (loc, this);
		loc = QPoint{ 1440, 25 };
		loc.ry() = 25;
		globalParamCtrl.initialize (loc, this, "GLOBAL PARAMETERS", ParameterSysType::global, 480, 500);
		configParamCtrl.initialize (loc, this, "CONFIGURATION PARAMETERS", ParameterSysType::config);
		configParamCtrl.setParameterControlActive (false);
		dds.initialize (loc, this, "DDS SYSTEM");
		optimizer.initialize (loc, this);

		loc = QPoint{ 960, 25 };
		aoPlots.resize (NUM_DAC_PLTS);
		// initialize plot controls.
		unsigned dacPlotSize = 500 / NUM_DAC_PLTS;
		for (auto dacPltCount : range (aoPlots.size ())){
			std::string titleTxt;
			switch (dacPltCount){
			case 0:
				titleTxt = "DACs: 0-7";
				break;
			case 1:
				titleTxt = "DACs: 8-15";
				break;
			case 2:
				titleTxt = "DACs: 16-23";
				break;
			}
			aoPlots[dacPltCount] = new PlotCtrl (8, plotStyle::DacPlot, std::vector<int> (), titleTxt);
			aoPlots[dacPltCount]->init (loc, 480, dacPlotSize, this);
		}
		// ttl plots are similar to aoSys.
		ttlPlots.resize (NUM_TTL_PLTS);
		unsigned ttlPlotSize = 500 / NUM_TTL_PLTS;
		for (auto ttlPltCount : range (ttlPlots.size ())){
			// currently assuming 4 ttl plots...
			std::string titleTxt;
			switch (ttlPltCount){
			case 0:
				titleTxt = "Ttls: Row A";
				break;
			case 1:
				titleTxt = "Ttls: Row B";
				break;
			case 2:
				titleTxt = "Ttls: Row C";
				break;
			case 3:
				titleTxt = "Ttls: Row D";
				break;
			}
			ttlPlots[ttlPltCount] = new PlotCtrl (16, plotStyle::TtlPlot, std::vector<int> (), titleTxt);
			ttlPlots[ttlPltCount]->init (loc, 480, ttlPlotSize, this);
		}
	}
	catch (ChimeraError& err){
		errBox ("Failed to initialize auxiliary window properly! Trace: " + err.trace ());
		//throwNested ("FATAL ERROR: Failed to initialize Auxiliary window properly!");
	}
}

void QtAuxiliaryWindow::handleDoAoPlotData (const std::vector<std::vector<plotDataVec>>& doData,
											const std::vector<std::vector<plotDataVec>>& aoData){
	for (auto ttlPlotNum: range(ttlPlots.size())){
		ttlPlots[ttlPlotNum]->setData (doData[ttlPlotNum]);
	}
	for (auto aoPlotNum : range (aoPlots.size ())) {
		aoPlots[aoPlotNum]->setData (aoData[aoPlotNum]);
	}
}

std::vector<parameterType> QtAuxiliaryWindow::getUsableConstants (){
	// This generates a usable set of constants (mostly for "Program Now" commands") based on the current GUI settings.
	// imporantly, when running the experiment proper, the saved config settings are what is used to determine 
	// parameters, not the gui setttings.
	std::vector<parameterType> configParams = configParamCtrl.getAllConstants ();
	std::vector<parameterType> globals = globalParamCtrl.getAllParams ();
	std::vector<parameterType> params = ParameterSystem::combineParams (configParams, globals);
	ScanRangeInfo constantRange;
	constantRange.defaultInit ();
	ParameterSystem::generateKey (params, false, constantRange);
	return params;
}

void QtAuxiliaryWindow::updateOptimization (AllExperimentInput& input){
	optimizer.verifyOptInput (input);
	dataPoint resultValue = andorWin->getMainAnalysisResult ();
	auto params = optimizer.getOptParams ();
	//optimizer.updateParams ( input, resultValue, camWin->getLogger() );
	std::string msg = "Next Optimization: ";
	for (auto& param : params){
		msg += param->name + ": " + str (param->currentValue) + ";";
	}
	msg += "\r\n";
	reportStatus (qstr(msg));
}

ParameterSystem& QtAuxiliaryWindow::getGlobals (){
	return globalParamCtrl;
}

std::pair<unsigned, unsigned> QtAuxiliaryWindow::getTtlBoardSize (){
	return ttlBoard.getTtlBoardSize ();
}

void QtAuxiliaryWindow::windowSaveConfig (ConfigStream& saveFile){
	// order matters! Don't change the order here.
	configParamCtrl.handleSaveConfig (saveFile);
	ttlBoard.handleSaveConfig (saveFile);
	aoSys.handleSaveConfig (saveFile);
	dds.handleSaveConfig (saveFile);
}

void QtAuxiliaryWindow::windowOpenConfig (ConfigStream& configFile){
	try{
		ConfigSystem::standardOpenConfig (configFile, configParamCtrl.configDelim, &configParamCtrl);
		ConfigSystem::standardOpenConfig (configFile, "TTLS", &ttlBoard);
		ConfigSystem::standardOpenConfig (configFile, "DACS", &aoSys);
		aoSys.updateEdits ();
		ConfigSystem::standardOpenConfig (configFile, dds.getDelim (), &dds);
	}
	catch (ChimeraError&){
		throwNested ("Auxiliary Window failed to read parameters from the configuration file.");
	}
}


unsigned QtAuxiliaryWindow::getNumberOfDacs (){
	return aoSys.getNumberOfDacs ();
}


Matrix<std::string> QtAuxiliaryWindow::getTtlNames (){
	return ttlBoard.getCore ().getAllNames ();
}


std::array<AoInfo, 24> QtAuxiliaryWindow::getDacInfo (){
	return aoSys.getDacInfo ();
}

std::vector<parameterType> QtAuxiliaryWindow::getAllParams (){
	std::vector<parameterType> vars = configParamCtrl.getAllParams ();
	std::vector<parameterType> vars2 = globalParamCtrl.getAllParams ();
	vars.insert (vars.end (), vars2.begin (), vars2.end ());
	return vars;
}


void QtAuxiliaryWindow::clearVariables (){
	mainWin->updateConfigurationSavedStatus (false);
	configParamCtrl.clearParameters ();
}

void QtAuxiliaryWindow::passRoundToDac (){
	mainWin->updateConfigurationSavedStatus (false);
	//aoSys.handleRoundToDac (mainWin);
}


void QtAuxiliaryWindow::setVariablesActiveState (bool activeState){
	mainWin->updateConfigurationSavedStatus (false);
	configParamCtrl.setParameterControlActive (activeState);
}


unsigned QtAuxiliaryWindow::getTotalVariationNumber (){
	return configParamCtrl.getTotalVariationNumber ();
}


void QtAuxiliaryWindow::zeroDacs (){
	try{
		mainWin->updateConfigurationSavedStatus (false);
		aoSys.zeroDacs (ttlBoard.getCore (), { 0, ttlBoard.getCurrentStatus () });
		reportStatus ("Zero'd DACs.\n");
	}
	catch (ChimeraError& exception){
		reportStatus ("Failed to Zero DACs!!!\n");
		reportErr (exception.qtrace ());
	}
}

DoSystem* QtAuxiliaryWindow::getTtlSystem (){
	return &ttlBoard;
}

DoCore& QtAuxiliaryWindow::getTtlCore (){
	return ttlBoard.getCore ();
}

void QtAuxiliaryWindow::fillMasterThreadInput (ExperimentThreadInput* input){
	try	{
		input->globalParameters = globalParamCtrl.getAllParams ();
	}
	catch (ChimeraError&) {
		throwNested ("Auxiliary window failed to fill master thread input.");
	}
}

AoSystem& QtAuxiliaryWindow::getAoSys () {
	return aoSys;
}

void QtAuxiliaryWindow::handleAbort (){
	if (optimizer.isInMiddleOfOptimizing ()){
		auto answer = QMessageBox::question (NULL, qstr ("Save Opt?"), qstr ("Save Optimization Data?"), 
			QMessageBox::Yes | QMessageBox::No);
		if (answer == QMessageBox::Yes){
			optimizer.onFinOpt ();
		}
	}
}

void QtAuxiliaryWindow::handleMasterConfigSave (std::stringstream& configStream){
	/// ttls
	for (auto row : DoRows::allRows){
		for (unsigned ttlNumberInc = 0; ttlNumberInc < ttlBoard.getTtlBoardSize ().second; ttlNumberInc++){
			std::string name = ttlBoard.getName (row, ttlNumberInc);
			if (name == ""){
				name = DoRows::toStr (row) + str (ttlNumberInc);
			}
			configStream << name << "\n";
			configStream << ttlBoard.getDefaultTtl (row, ttlNumberInc) << "\n";
		}
	}
	// DAC Names
	for (unsigned dacInc = 0; dacInc < aoSys.getNumberOfDacs (); dacInc++){
		std::string name = aoSys.getName (dacInc);
		std::pair<double, double> minMax = aoSys.getDacRange (dacInc);
		if (name == ""){
			// then the name hasn't been set, so create the default name
			name = "Dac" + str (dacInc);
		}
		configStream << name << "\n";
		configStream << minMax.first << " - " << minMax.second << "\n";
		configStream << aoSys.getDefaultValue (dacInc) << "\n";
		configStream << aoSys.getNote (dacInc) << "\n";
	}

	// Number of Variables
	configStream << globalParamCtrl.getCurrentNumberOfVariables () << "\n";
	/// Variables
	for (unsigned varInc : range (globalParamCtrl.getCurrentNumberOfVariables ())){
		parameterType info = globalParamCtrl.getVariableInfo (varInc);
		configStream << info.name << " ";
		configStream << info.constantValue << "\n";
		// all globals are constants, no need to output anything else.
	}
}

void QtAuxiliaryWindow::handleNormalFin () {
	try {
		aoSys.stopDacs ();
		aoSys.setDacStatusNoForceOut (aoSys.getFinalSnapshot ());
		ttlBoard.setTtlStatusNoForceOut (ttlBoard.getCore().getFinalSnapshot ());
	}
	catch (ChimeraError&) { /* this gets thrown if no dac events. just continue.*/ }
}


void QtAuxiliaryWindow::handleMasterConfigOpen (ConfigStream& configStream){
	ttlBoard.getCore ().resetTtlEvents ();
	ttlBoard.getCore ().prepareForce ();
	aoSys.resetDacEvents ();
	aoSys.prepareForce ();
	for (auto row : DoRows::allRows){
		for (unsigned ttlNumberInc : range (ttlBoard.getTtlBoardSize ().second)){
			std::string name;
			std::string defaultStatusString;
			bool defaultStatus;
			configStream >> name >> defaultStatusString;
			try {
				// In file the booleans are stored as "0" or "1".
				defaultStatus = boost::lexical_cast<int>(defaultStatusString);
			}
			catch (boost::bad_lexical_cast&){
				throwNested ("Failed to load one of the default ttl values!");
			}
			ttlBoard.setName (row, ttlNumberInc, name);
			ttlBoard.updateDefaultTtl (row, ttlNumberInc, defaultStatus);
		}
	}
	// getting aoSys.
	for (unsigned dacInc : range (aoSys.getNumberOfDacs ())){
		std::string name, defaultValueString, minString, maxString;
		double defaultValue, min, max;
		configStream >> name;
		std::string trash;
		configStream >> minString >> trash;
		if (trash != "-"){
			thrower (str ("Expected \"-\" in master config file between min and max values for variable ")
				+ name + ", dac" + str (dacInc) + ".");
		}
		configStream >> maxString;
		configStream >> defaultValueString;
		try{
			defaultValue = boost::lexical_cast<double>(defaultValueString);
			min = boost::lexical_cast<double>(minString);
			max = boost::lexical_cast<double>(maxString);
		}
		catch (boost::bad_lexical_cast&){
			throwNested ("Failed to load one of the default DAC values!");
		}
		std::string noteString = "";
		noteString = configStream.getline ();
		aoSys.setName (dacInc, name);
		aoSys.setNote (dacInc, noteString);
		aoSys.setMinMax (dacInc, min, max);
		aoSys.prepareDacForceChange (dacInc, defaultValue, ttlBoard.getCore ());
		aoSys.updateEdits ();
		aoSys.setDefaultValue (dacInc, defaultValue);
	}
	// variables.
	int varNum;
	configStream >> varNum;
	if (varNum < 0 || varNum > 1000){
		auto answer = QMessageBox::question (NULL, qstr ("Suspicious?"), qstr ("Variable number retrieved from "
			"file appears suspicious. The number is " + str (varNum) + ". Is this accurate?"), QMessageBox::Yes
			| QMessageBox::No);
		if (answer == QMessageBox::No){
			// don't try to load anything.
			varNum = 0;
			return;
		}
	}
	// Number of Variables
	globalParamCtrl.clearParameters ();
	for (int varInc = 0; varInc < varNum; varInc++){
		parameterType tempVar;
		tempVar.constant = true;
		tempVar.overwritten = false;
		tempVar.active = false;
		double value;
		configStream >> tempVar.name >> value;
		tempVar.constantValue = value;
		tempVar.ranges.push_back ({ value, value });
		globalParamCtrl.addParameter (tempVar);
	}
	globalParamCtrl.setTableviewColumnSize ();
}

void QtAuxiliaryWindow::updateExpActiveInfo (std::vector<parameterType> expParams) {
	globalParamCtrl.setUsages (expParams);
	configParamCtrl.setUsages (expParams);
}

void QtAuxiliaryWindow::SetDacs (){
	reportStatus ("----------------------\r\nSetting Dacs... ");
	try{
		mainWin->updateConfigurationSavedStatus (false);
		aoSys.forceDacs (ttlBoard.getCore (), { 0, ttlBoard.getCurrentStatus () });
		reportStatus ("Finished Setting Dacs.\r\n");
	}
	catch (ChimeraError& exception){
		errBox (exception.trace ());
		reportStatus (": " + exception.qtrace () + "\r\n");
		reportErr (exception.qtrace ());
	}
}

void QtAuxiliaryWindow::ViewOrChangeTTLNames (){
	mainWin->updateConfigurationSavedStatus (false);
	ttlInputStruct input;
	input.ttls = &ttlBoard;
	doChannelInfoDialog* dialog = new doChannelInfoDialog (&input);
	dialog->setStyleSheet (chimeraStyleSheets::stdStyleSheet());
	dialog->exec ();
}


void QtAuxiliaryWindow::ViewOrChangeDACNames (){
	mainWin->updateConfigurationSavedStatus (false);
	aoInputStruct input;
	input.aoSys = &aoSys;
	AoSettingsDialog* dialog = new AoSettingsDialog (&input);
	dialog->setStyleSheet (chimeraStyleSheets::stdStyleSheet ());
	dialog->exec ();
}

std::string QtAuxiliaryWindow::getOtherSystemStatusMsg (){
	// controls are done. Report the initialization defaultStatus...
	std::string msg;
	msg += "DO System:\n";
	if (!ttlBoard.getFtFlumeSafemode ()){
		msg += "\tDO System is active!\n";
		msg += "\t" + ttlBoard.getDoSystemInfo () + "\n";
		//ttlBoard.ftdi_disconnect();
		msg += "\t Bites Written \n";

	}
	else{
		msg += "\tCode System is disabled! Enable in \"constants.h\"\n";
	}

	msg += "Analog Out System:\n";
	if (!ANALOG_OUT_SAFEMODE){
		msg += "\tCode System is Active!\n";
		msg += "\t" + aoSys.getSystemInfo () + "\n";
	}
	else{
		msg += "\tCode System is disabled! Enable in \"constants.h\"\n";
	}
	msg += "DDS System:\n";
	if (!DDS_SAFEMODE){
		msg += "\tDDS System is Active!\n";
		msg += "\t" + dds.getSystemInfo () + "\n";
		msg += "\t" + dds.getSystemInfo ();
	}
	else{
		msg += "\tDDS System is disabled! Enable in \"constants.h\"\n";
	}
	return msg;
}

std::string QtAuxiliaryWindow::getVisaDeviceStatus (){
	std::string msg;
	return msg;
}

void QtAuxiliaryWindow::fillExpDeviceList (DeviceList& list){
	list.list.push_back (dds.getCore ());
}

