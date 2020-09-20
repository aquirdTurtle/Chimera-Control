#include "stdafx.h"
#include "QtAuxiliaryWindow.h"
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
#include <ExcessDialogs/doChannelInfoDialog.h>
#include <ExcessDialogs/AoSettingsDialog.h>

QtAuxiliaryWindow::QtAuxiliaryWindow (QWidget* parent) : IChimeraQtWindow (parent), 
topBottomTek (TOP_BOTTOM_TEK_SAFEMODE, TOP_BOTTOM_TEK_USB_ADDRESS, "TOP_BOTTOM_TEKTRONICS_AFG"),
eoAxialTek (EO_AXIAL_TEK_SAFEMODE, EO_AXIAL_TEK_USB_ADDRESS, "EO_AXIAL_TEKTRONICS_AFG"),
agilents{ Agilent{TOP_BOTTOM_AGILENT_SETTINGS,this}, Agilent{AXIAL_AGILENT_SETTINGS,this},
Agilent{FLASHING_AGILENT_SETTINGS,this}, Agilent{UWAVE_AGILENT_SETTINGS,this} },
	ttlBoard (this, DOFTDI_SAFEMODE, true),
	aoSys (this, ANALOG_OUT_SAFEMODE), configParameters (this, "CONFIG_PARAMETERS"),
	globalParameters (this, "GLOBAL_PARAMETERS"), dds (DDS_SAFEMODE),
	piezo1 (this, PIEZO_1_INFO), piezo2 (this, PIEZO_2_INFO), piezo3(this, PIEZO_3_INFO){	
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
	POINT loc{ 0, 25 };
	try{
		statBox->initialize (loc, this, 480, mainWin->getDevices (), 2);
		ttlBoard.initialize (loc, this);
		aoSys.initialize (loc, this);
		aiSys.initialize (loc, this);
		topBottomTek.initialize (loc, this, "Top-Bottom-Tek", "Top", "Bottom", 480);
		eoAxialTek.initialize (loc, this, "EO / Axial", "EO", "Axial", 480);
		RohdeSchwarzGenerator.initialize (loc, this);
		loc = POINT{ 480, 25 };

		agilents[whichAgTy::TopBottom].initialize (loc, "Top-Bottom-Agilent", 100, this);
		agilents[whichAgTy::Axial].initialize (loc, "Microwave-Axial-Agilent", 100, this);
		agilents[whichAgTy::Flashing].initialize (loc, "Flashing-Agilent", 100, this);
		agilents[whichAgTy::Microwave].initialize (loc, "Microwave-Agilent", 100, this);
		loc = POINT{ 1440, 25 };
		globalParameters.initialize (loc, this, "GLOBAL PARAMETERS", ParameterSysType::global);
		configParameters.initialize (loc, this, "CONFIGURATION PARAMETERS", ParameterSysType::config);
		dds.initialize (loc, this, "DDS SYSTEM");
		piezo1.initialize (loc, this, 240, { "Top-x", "Top-y", "Axial-y" });
		loc.x += 240;
		loc.y -= 85;
		piezo2.initialize (loc, this, 240, { "EO-x", "EO-y", "Axial-x" });
		configParameters.setParameterControlActive (false);
		loc.x -= 240;
		piezo3.initialize (loc, this, 240, { "Bot-x", "Bot-y", "---" });
		optimizer.initialize (loc, this);
		loc = POINT{ 960, 25 };
		aoPlots.resize (NUM_DAC_PLTS);
		dacData.resize (NUM_DAC_PLTS);
		unsigned linesPerDacPlot = 24 / dacData.size ();
		// initialize data structures.
		for (auto& dacPlotData : dacData){
			dacPlotData = std::vector<pPlotDataVec> (linesPerDacPlot);
			for (auto& data : dacPlotData){
				data = pPlotDataVec (new plotDataVec (100, { 0,0,0 }));
			}
		}
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
		ttlData.resize (NUM_TTL_PLTS);
		unsigned linesPerTtlPlot = 64 / ttlData.size ();
		for (auto& ttlPlotData : ttlData){
			ttlPlotData = std::vector<pPlotDataVec> (linesPerTtlPlot);
			for (auto& d : ttlPlotData){
				d = pPlotDataVec (new plotDataVec (100, { 0,0,0 }));
			}
		}
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
	std::vector<parameterType> configParams = configParameters.getAllConstants ();
	std::vector<parameterType> globals = globalParameters.getAllParams ();
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

// MESSAGE MAP FUNCTION
LRESULT QtAuxiliaryWindow::onLogVoltsMessage (WPARAM wp, LPARAM lp){
	aiSys.refreshCurrentValues ();
	aiSys.refreshDisplays ();
	andorWin->writeVolts (wp, aiSys.getCurrentValues ());
	return TRUE;
}


void QtAuxiliaryWindow::newAgilentScript (whichAgTy::agilentNames name){
	try{
		agilents[name].verifyScriptable ();
		mainWin->updateConfigurationSavedStatus (false);
		agilents[name].checkSave (mainWin->getProfileSettings ().configLocation, mainWin->getRunInfo ());
		agilents[name].agilentScript.newScript ();
		agilents[name].agilentScript.updateScriptNameText (mainWin->getProfileSettings ().configLocation);
	}
	catch (ChimeraError& err){
		reportErr (err.qtrace ());
	}
}

void QtAuxiliaryWindow::openAgilentScript (whichAgTy::agilentNames name, IChimeraQtWindow* parent)
{
	try
	{
		agilents[name].verifyScriptable ();
		mainWin->updateConfigurationSavedStatus (false);
		agilents[name].agilentScript.checkSave (mainWin->getProfileSettings ().configLocation,
			mainWin->getRunInfo ());
		std::string openFileName = openWithExplorer (parent, AGILENT_SCRIPT_EXTENSION);
		agilents[name].agilentScript.openParentScript (openFileName,
			mainWin->getProfileSettings ().configLocation, mainWin->getRunInfo ());
		agilents[name].agilentScript.updateScriptNameText (mainWin->getProfileSettings ().configLocation);
	}
	catch (ChimeraError& err)
	{
		reportErr (err.qtrace ());
	}
}


void QtAuxiliaryWindow::updateAgilent (whichAgTy::agilentNames name)
{
	try
	{
		mainWin->updateConfigurationSavedStatus (false);
		agilents[name].checkSave (mainWin->getProfileSettings ().configLocation, mainWin->getRunInfo ());
		agilents[name].readGuiSettings ();
	}
	catch (ChimeraError&)
	{
		throwNested ("Failed to update agilent.");
	}
}


void QtAuxiliaryWindow::saveAgilentScript (whichAgTy::agilentNames name)
{
	try
	{
		agilents[name].verifyScriptable ();
		mainWin->updateConfigurationSavedStatus (false);
		agilents[name].agilentScript.saveScript (mainWin->getProfileSettings ().configLocation,
			mainWin->getRunInfo ());
		agilents[name].agilentScript.updateScriptNameText (mainWin->getProfileSettings ().configLocation);
	}
	catch (ChimeraError& err)
	{
		reportErr (err.qtrace ());
	}
}


void QtAuxiliaryWindow::saveAgilentScriptAs (whichAgTy::agilentNames name, IChimeraQtWindow* parent)
{
	try
	{
		agilents[name].verifyScriptable ();
		mainWin->updateConfigurationSavedStatus (false);
		std::string extensionNoPeriod = agilents[name].agilentScript.getExtension ();
		if (extensionNoPeriod.size () == 0)
		{
			return;
		}
		extensionNoPeriod = extensionNoPeriod.substr (1, extensionNoPeriod.size ());
		std::string newScriptAddress = saveWithExplorer ( parent, extensionNoPeriod,
														  mainWin->getProfileSettings () );
		agilents[name].agilentScript.saveScriptAs (newScriptAddress, mainWin->getRunInfo ());
		agilents[name].agilentScript.updateScriptNameText (mainWin->getProfileSettings ().configLocation);
	}
	catch (ChimeraError& err)
	{
		reportErr (err.qtrace ());
	}
}

ParameterSystem& QtAuxiliaryWindow::getGlobals (){
	return globalParameters;
}

std::vector<std::reference_wrapper<PiezoCore> > QtAuxiliaryWindow::getPiezoControllers (){
	std::vector<std::reference_wrapper<PiezoCore> > controllers;
	controllers.push_back (piezo1.getCore ());
	controllers.push_back (piezo2.getCore ());
	controllers.push_back (piezo3.getCore ());
	return controllers;
}

std::pair<unsigned, unsigned> QtAuxiliaryWindow::getTtlBoardSize (){
	return ttlBoard.getTtlBoardSize ();
}

void QtAuxiliaryWindow::windowSaveConfig (ConfigStream& saveFile){
	// order matters! Don't change the order here.
	configParameters.handleSaveConfig (saveFile);
	ttlBoard.handleSaveConfig (saveFile);
	aoSys.handleSaveConfig (saveFile);
	for (auto& agilent : agilents){
		agilent.handleSavingConfig (saveFile, mainWin->getProfileSettings ().configLocation,
			mainWin->getRunInfo ());
	}
	topBottomTek.handleSaveConfig (saveFile);
	eoAxialTek.handleSaveConfig (saveFile);
	dds.handleSaveConfig (saveFile);
	piezo1.handleSaveConfig (saveFile);
	piezo2.handleSaveConfig (saveFile);
	piezo3.handleSaveConfig (saveFile);
	aiSys.handleSaveConfig (saveFile);
	RohdeSchwarzGenerator.handleSaveConfig (saveFile);
}

void QtAuxiliaryWindow::windowOpenConfig (ConfigStream& configFile){
	try{
		ConfigSystem::standardOpenConfig (configFile, configParameters.configDelim, &configParameters, Version ("4.0"));
		ConfigSystem::standardOpenConfig (configFile, "TTLS", &ttlBoard);
		ConfigSystem::standardOpenConfig (configFile, "DACS", &aoSys);
		aoSys.updateEdits ();
		for (auto& agilent : agilents){
			deviceOutputInfo info;
			ConfigSystem::stdGetFromConfig (configFile, agilent.getCore (), info, Version ("4.0"));
			agilent.setOutputSettings (info);
			agilent.updateSettingsDisplay (1, mainWin->getProfileSettings ().configLocation, mainWin->getRunInfo ());
		}
		tektronixInfo info;
		ConfigSystem::stdGetFromConfig (configFile, topBottomTek.getCore (), info);
		topBottomTek.setSettings (info);
		ConfigSystem::stdGetFromConfig (configFile, eoAxialTek.getCore (), info);
		eoAxialTek.setSettings (info);

		ConfigSystem::standardOpenConfig (configFile, topBottomTek.getDelim (), &topBottomTek, Version ("4.0"));
		ConfigSystem::standardOpenConfig (configFile, eoAxialTek.getDelim (), &eoAxialTek, Version ("4.0"));
		if (configFile.ver >= Version ("4.5")) {
			ConfigSystem::standardOpenConfig (configFile, dds.getDelim (), &dds, Version ("4.5"));
		}
		ConfigSystem::standardOpenConfig (configFile, piezo1.getConfigDelim (), &piezo1, Version ("4.6"));
		ConfigSystem::standardOpenConfig (configFile, piezo2.getConfigDelim (), &piezo2, Version ("4.6"));
		ConfigSystem::standardOpenConfig (configFile, piezo3.getConfigDelim (), &piezo3, Version ("5.2"));
		AiSettings settings;
		ConfigSystem::stdGetFromConfig (configFile, aiSys, settings, Version ("4.9"));
		aiSys.setAiSettings (settings);
		microwaveSettings uwsettings;
		ConfigSystem::stdGetFromConfig (configFile, RohdeSchwarzGenerator.getCore (), uwsettings, Version ("4.10"));
		RohdeSchwarzGenerator.setMicrowaveSettings (uwsettings);
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
	std::vector<parameterType> vars = configParameters.getAllParams ();
	std::vector<parameterType> vars2 = globalParameters.getAllParams ();
	vars.insert (vars.end (), vars2.begin (), vars2.end ());
	return vars;
}


void QtAuxiliaryWindow::clearVariables (){
	mainWin->updateConfigurationSavedStatus (false);
	configParameters.clearParameters ();
}

/*
void QtAuxiliaryWindow::addVariable (std::string name, bool constant, double value){
	parameterType var;
	var.name = name;
	var.constant = constant;
	var.constantValue = value;
	var.ranges.push_back ({ value, value + 1 });
	try{
		mainWin->updateConfigurationSavedStatus (false);
		configParameters.addParameter (var);
	}
	catch (ChimeraError&){
		throwNested ("Failed to Add a variable.");
	}
}*/

void QtAuxiliaryWindow::passRoundToDac (){
	mainWin->updateConfigurationSavedStatus (false);
	//aoSys.handleRoundToDac (mainWin);
}


void QtAuxiliaryWindow::setVariablesActiveState (bool activeState){
	mainWin->updateConfigurationSavedStatus (false);
	configParameters.setParameterControlActive (activeState);
}


unsigned QtAuxiliaryWindow::getTotalVariationNumber (){
	return configParameters.getTotalVariationNumber ();
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
		input->dacData = dacData;
		input->ttlData = ttlData;
		input->globalParameters = globalParameters.getAllParams ();
		if (aiSys.wantsQueryBetweenVariations ()) {
			input->numAiMeasurements = configParameters.getTotalVariationNumber ();
		}
	}
	catch (ChimeraError&) {
		throwNested ("Auxiliary window failed to fill master thread input.");
	}
}

AoSystem& QtAuxiliaryWindow::getAoSys () {
	return aoSys;
}

AiSystem& QtAuxiliaryWindow::getAiSys () {
	return aiSys;
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
	configStream << globalParameters.getCurrentNumberOfVariables () << "\n";
	/// Variables
	for (unsigned varInc : range (globalParameters.getCurrentNumberOfVariables ())){
		parameterType info = globalParameters.getVariableInfo (varInc);
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
		if (configStream.ver >= Version ("1.2")){
			std::string trash;
			configStream >> minString >> trash;
			if (trash != "-"){
				thrower (str ("Expected \"-\" in master config file between min and max values for variable ")
					+ name + ", dac" + str (dacInc) + ".");
			}
			configStream >> maxString;
		}
		configStream >> defaultValueString;
		try{
			defaultValue = boost::lexical_cast<double>(defaultValueString);
			if (configStream.ver >= Version ("1.2")){
				min = boost::lexical_cast<double>(minString);
				max = boost::lexical_cast<double>(maxString);
			}
			else{
				min = -10;
				max = 10;
			}
		}
		catch (boost::bad_lexical_cast&){
			throwNested ("Failed to load one of the default DAC values!");
		}
		std::string noteString = "";
		if (configStream.ver >= Version ("2.3")){
			std::string trash;
			configStream >> noteString;
		}
		aoSys.setName (dacInc, name);
		aoSys.setNote (dacInc, noteString);
		aoSys.setMinMax (dacInc, min, max);
		aoSys.prepareDacForceChange (dacInc, defaultValue, ttlBoard.getCore ());
		aoSys.updateEdits ();
		aoSys.setDefaultValue (dacInc, defaultValue);
	}
	// variables.
	if (configStream.ver >= Version ("1.1")){
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
		globalParameters.clearParameters ();
		for (int varInc = 0; varInc < varNum; varInc++){
			parameterType tempVar;
			tempVar.constant = true;
			tempVar.overwritten = false;
			tempVar.active = false;
			double value;
			configStream >> tempVar.name >> value;
			tempVar.constantValue = value;
			tempVar.ranges.push_back ({ value, value });
			globalParameters.addParameter (tempVar);
		}
		globalParameters.setTableviewColumnSize ();
	}
}

void QtAuxiliaryWindow::SetDacs (){
	// have the dac values change
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
	msg += "Analog In System:\n";
	if (!ANALOG_IN_SAFEMODE){
		msg += "\tCode System is Active!\n";
		msg += "\t" + aiSys.getSystemStatus () + "\n";
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
	msg += "Piezo System:\n";
	msg += "\tPiezo System is Active!\n";
	msg += "\tDevice List: " + piezo1.getPiezoDeviceList () + "\n";
	msg += "\t Device Info:\n" + str ("\t\t");
	msg += piezo1.getDeviceInfo () + "\n";
	msg += piezo2.getDeviceInfo () + "\n";
	msg += piezo3.getDeviceInfo () + "\n";
	msg += "- End Dev Info";
	return msg;
}


std::string QtAuxiliaryWindow::getVisaDeviceStatus (){
	std::string msg;
	msg += "----------------------------------------------------------------------------------- VISA Devices\n";
	msg += "Tektronix 1:\n\t" + topBottomTek.queryIdentity ();
	msg += "Tektronix 2:\n\t" + eoAxialTek.queryIdentity ();
	for (auto& agilent : agilents){
		msg += agilent.getCore ().configDelim + ":\n\t" + agilent.getDeviceIdentity ();
	}
	return msg;
}


std::string QtAuxiliaryWindow::getMicrowaveSystemStatus (){
	std::string msg;
	//msg += "----------------------------------------------------------------------------------- GPIB Devices:\n";
	msg += "Microwave System:\n";
	if (!(MICROWAVE_SYSTEM_DEVICE_TYPE == microwaveDevice::NONE)){
		msg += "\tCode System is Active!\n";
		msg += "\t" + RohdeSchwarzGenerator.getIdentity ();
	}
	else{
		msg += "\tCode System is disabled! Enable in \"constants.h\"";
	}
	return msg;
}

void QtAuxiliaryWindow::fillExpDeviceList (DeviceList& list){
	list.list.push_back (topBottomTek.getCore ());
	list.list.push_back (eoAxialTek.getCore ());
	list.list.push_back (RohdeSchwarzGenerator.getCore ());
	for (auto& ag : agilents){
		list.list.push_back (ag.getCore ());
	}
	list.list.push_back (aiSys);
	list.list.push_back (dds.getCore ());
	list.list.push_back (piezo1.getCore ());
	list.list.push_back (piezo2.getCore ());
	list.list.push_back (piezo3.getCore ());
}

