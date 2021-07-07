// created by Mark O. Brown
#include "stdafx.h"

#include "AndorCameraSettingsControl.h"

#include "PrimaryWindows/QtAndorWindow.h"
#include "GeneralUtilityFunctions/miscCommonFunctions.h"
#include "ConfigurationSystems/ConfigSystem.h"

#include <boost/lexical_cast.hpp>

AndorCameraSettingsControl::AndorCameraSettingsControl() : imageDimensionsObj("andor"){
	AndorRunSettings& andorSettings = configSettings.andor;
}

void AndorCameraSettingsControl::initialize ( QPoint& pos, IChimeraQtWindow* parent, std::vector<std::string> vertSpeeds,
											  std::vector<std::string> horSpeeds ){
	auto& px = pos.rx (), & py = pos.ry ();
	header = new QLabel ("ANDOR CAMERA SETTINGS", parent);
	header->setGeometry (px, py, 480, 25);
	programNow = new QPushButton ("Program Now", parent);
	programNow->setGeometry (px, py+=25, 480, 25);
	parent->connect (programNow, &QPushButton::released, parent->andorWin, [parent, this]() {
			auto settings = getConfigSettings ().andor;
			parent->andorWin->handlePrepareForAcq (&settings, analysisSettings());
			parent->andorWin->manualArmCamera ();
		});
	controlAndorCameraCheck = new CQCheckBox ("Control Andor Camera?", parent);
	controlAndorCameraCheck->setGeometry (px, py += 25, 240, 25);
	controlAndorCameraCheck->setChecked (true);
	viewRunningSettings = new QCheckBox ("View Running Settings?", parent);
	viewRunningSettings->setGeometry (px+240, py, 240, 25);
	parent->connect (viewRunningSettings, &QCheckBox::stateChanged, [this]() {
		if (viewRunningSettings->isChecked ()) {
			// just changed to checked, so the settings should indicate the config settings still. 
			updateSettings ();
			updateWindowEnabledStatus ();
			updateDisplays ();
			currentlyUneditable = true;
		}
		else {
			updateWindowEnabledStatus ();
			updateDisplays ();
			currentlyUneditable = false;
		}
		});

	cameraModeCombo = new CQComboBox (parent);
	cameraModeCombo->addItem ("Kinetic-Series-Mode");
	cameraModeCombo->addItem ("Accumulation-Mode");
	cameraModeCombo->addItem ("Video-Mode");
	cameraModeCombo->setGeometry (px, py += 25, 240, 25);

	cameraModeCombo->setCurrentIndex (0);
	parent->connect (cameraModeCombo, qOverload<int> (&QComboBox::currentIndexChanged), 
		[this, parent]() {
			if (!viewRunningSettings->isChecked ()) {
				updateCameraMode ();
				updateWindowEnabledStatus ();
			}
		});
	configSettings.andor.acquisitionMode = AndorRunModes::mode::Kinetic;
	triggerCombo = new CQComboBox (parent);
	triggerCombo->setGeometry (px + 240, py, 240, 25);
	triggerCombo->addItems ({ "Internal-Trigger", "External-Trigger","Start-On-Trigger" });
	triggerCombo->setCurrentIndex (0);
	parent->connect (triggerCombo, qOverload<int> (&QComboBox::activated), 
		[this, parent]() {
			if (!viewRunningSettings->isChecked ()) {
				updateTriggerMode ();
				updateWindowEnabledStatus ();
			}
		});
	configSettings.andor.triggerMode = AndorTriggerMode::mode::External;

	frameTransferModeCombo = new CQComboBox(parent);
	frameTransferModeCombo->setGeometry (px, py += 25, 160, 25);
	frameTransferModeCombo->setToolTip ("Frame Transfer Mode OFF:\n"
		"Slower than when on. Cleans between images. Mechanical shutter may not be necessary.\n"
		"Frame Transfer Mode ON:\n"
		"Faster than when off. Does not clean between images, giving better background.\n"
		"Mechanical Shutter probably necessary unless imaging continuously.\n"
		"See iXonEM + hardware guide pg 42.\n");
	frameTransferModeCombo->addItems ({ "FTM: OFF!", "FTM: ON!" });
	frameTransferModeCombo->setCurrentIndex (0);
	configSettings.andor.frameTransferMode = 0;

	verticalShiftSpeedCombo = new CQComboBox (parent);
	verticalShiftSpeedCombo->setGeometry (px+160, py, 160, 25);
	verticalShiftSpeedCombo->setToolTip ("Vertical Shift Speed (Million Pixels per Second)");
	for (auto speed : vertSpeeds){ 
		verticalShiftSpeedCombo->addItem ("VS: " + qstr(speed));
	}
	verticalShiftSpeedCombo->setCurrentIndex (1);
	configSettings.andor.vertShiftSpeedSetting = 0;

	horizontalShiftSpeedCombo = new CQComboBox (parent);
	horizontalShiftSpeedCombo->setGeometry (px+320, py, 160, 25);
	for (auto speed : horSpeeds){
		horizontalShiftSpeedCombo->addItem ("HS: " + qstr (speed));
	}
	horizontalShiftSpeedCombo->setToolTip ("Horizontal Shift Speed (Million Pixels per Second)");
	horizontalShiftSpeedCombo->setCurrentIndex (0);
	configSettings.andor.horShiftSpeedSetting = 0;

	emGainBtn = new CQPushButton ("Set EM Gain (-1=OFF)", parent);
	emGainBtn->setGeometry (px, py += 25, 200, 20);
	parent->connect (emGainBtn, &QPushButton::released, [parent]() {
			parent->andorWin->handleEmGainChange ();
		});
	emGainEdit = new CQLineEdit ("-1", parent);
	emGainEdit->setGeometry (px + 200, py, 120, 20);
	emGainEdit->setToolTip( "Set the state & gain of the EM gain of the camera. Enter a negative number to turn EM Gain"
						   " mode off. The program will immediately change the state of the camera after changing this"
						   " edit." );
	//
	emGainDisplay = new QLabel ("OFF", parent);
	emGainDisplay->setGeometry (px + 320, py, 160, 20);
	// initialize settings.
	configSettings.andor.emGainLevel = 0;
	configSettings.andor.emGainModeIsOn = false;
	setTemperatureButton = new CQPushButton ("Set Camera Temperature (C)", parent);
	setTemperatureButton->setGeometry (px, py+=20, 270, 25);
	parent->connect (setTemperatureButton, &QPushButton::released, 
		[parent]() {
			parent->andorWin->passSetTemperaturePress ();
		});
	temperatureEdit = new CQLineEdit ("0", parent);
	temperatureEdit->setGeometry (px + 270, py, 80, 25);

	temperatureDisplay = new QLabel ("", parent);
	temperatureDisplay->setGeometry (px + 350, py, 80, 25);
	temperatureOffButton = new CQPushButton("OFF", parent);
	temperatureOffButton->setGeometry (px + 430, py, 50, 25);
	temperatureMsg = new QLabel ("Temperature control is disabled",parent);
	temperatureMsg->setGeometry (px, py+=25, 480, 50);
	py += 50;
	//
	picSettingsObj.initialize( pos, parent );
	imageDimensionsObj.initialize( pos, parent, 1, 480 );

	// Accumulation Time
	accumulationCycleTimeLabel = new QLabel ("Accum. Time", parent);
	accumulationCycleTimeLabel->setGeometry (px, py, 120, 25);
	accumulationCycleTimeLabel->setToolTip ("Accumulation Cycle Time (ms?)");

	accumulationCycleTimeEdit = new CQLineEdit ("0.1", parent);
	accumulationCycleTimeEdit->setGeometry (px + 120, py, 120, 25);
	accumulationCycleTimeEdit->setToolTip ("Accumulation Cycle Time (ms?)");

	// Accumulation Number
	accumulationNumberLabel = new QLabel ("Accum. #", parent);
	accumulationNumberLabel->setGeometry (px+240, py, 80, 25);
	accumulationNumberLabel->setToolTip ("Accumulation Number");
	accumulationNumberEdit = new CQLineEdit ("1", parent);
	accumulationNumberEdit->setGeometry (px + 320, py, 40, 25);
	accumulationNumberEdit->setToolTip ("Accumulation Number");

	verticalShiftVoltAmpLabel = new QLabel("V.S. Amp.", parent);
	verticalShiftVoltAmpLabel->setToolTip("Vertical Shift Voltage Amplitude. 0 is Default, increase from 0 (up to 4) to"
		"increase the voltage used to shift charges between pixels. Only relevant for large shift speeds.");
	verticalShiftVoltAmpLabel->setGeometry(px+360, py, 80, 25);

	verticalShiftVoltAmpEdit = new CQLineEdit("0", parent);
	verticalShiftVoltAmpEdit->setGeometry(px + 440, py, 40, 25);
	verticalShiftVoltAmpEdit->setText("0");

	// minimum kinetic cycle time (determined by camera)
	minKineticCycleTimeLabel = new QLabel ("Min. Kin. Time", parent);
	minKineticCycleTimeLabel->setGeometry (px, py+=25, 120, 25);
	minKineticCycleTimeLabel->setToolTip ("Minimum Kinetic Cycle Time (s)");

	minKineticCycleTimeDisp = new QLabel ("---", parent);
	minKineticCycleTimeDisp->setGeometry (px + 120, py, 120, 25);
	minKineticCycleTimeDisp->setToolTip ("Minimum Kinetic Cycle Time (s)");

	/// Kinetic Cycle Time
	kineticCycleTimeLabel = new QLabel ("Kin. Time", parent);
	kineticCycleTimeLabel->setGeometry (px+240, py, 120, 25);
	kineticCycleTimeLabel->setToolTip ("Kinetic Cycle Time (s)");

	kineticCycleTimeEdit = new CQLineEdit ("0.1", parent);
	kineticCycleTimeEdit->setGeometry (px+360, py, 120, 25);
	kineticCycleTimeEdit->setToolTip ("Kinetic Cycle Time (s)");
	py += 25;

	//
	calControl.initialize( pos, parent );
	updateWindowEnabledStatus ();
}


// note that this object doesn't actually store the camera state, it just uses it in passing to figure out whether 
// buttons should be on or off.
void AndorCameraSettingsControl::cameraIsOn(bool on){
	currentlyRunning = on;
	// Can't change em gain mode or temperature settings once started.
	emGainEdit->setEnabled( !currentlyRunning);
	setTemperatureButton->setEnabled ( !currentlyRunning);
	temperatureOffButton->setEnabled ( !currentlyRunning);
}

void AndorCameraSettingsControl::setConfigSettings (AndorRunSettings inputSettings) {
	configSettings.andor = inputSettings;
	updateDisplays ();
}

void AndorCameraSettingsControl::updateDisplays () {

	auto optionsIn = viewRunningSettings->isChecked () ? currentlyRunningSettings : configSettings.andor;
	controlAndorCameraCheck->setChecked (optionsIn.controlCamera);
	kineticCycleTimeEdit->setText (qstr(optionsIn.kineticCycleTime));
	accumulationCycleTimeEdit->setText (qstr(optionsIn.accumulationTime));
	int ind = cameraModeCombo->findText (AndorRunModes::toStr (optionsIn.acquisitionMode).c_str ());
	if (ind != -1) {
		cameraModeCombo->setCurrentIndex (ind);
	}
	ind = triggerCombo->findText (AndorTriggerMode::toStr (optionsIn.triggerMode).c_str ());
	if (ind != -1) {
		triggerCombo->setCurrentIndex (ind);
	}
	kineticCycleTimeEdit->setText (qstr(optionsIn.kineticCycleTime));
	accumulationCycleTimeEdit->setText (qstr(optionsIn.accumulationTime * 1000.0));
	accumulationNumberEdit->setText (qstr(optionsIn.accumulationNumber));
	temperatureEdit->setText (qstr(optionsIn.temperatureSetting));
	imageDimensionsObj.setImageParametersFromInput (optionsIn.imageSettings);

	verticalShiftSpeedCombo->setCurrentIndex (optionsIn.vertShiftSpeedSetting);
	horizontalShiftSpeedCombo->setCurrentIndex (optionsIn.horShiftSpeedSetting);
	frameTransferModeCombo->setCurrentIndex (optionsIn.frameTransferMode);

	picSettingsObj.setUnofficialExposures (optionsIn.exposureTimes);
	picSettingsObj.setUnofficialPicsPerRep (optionsIn.picsPerRepetition);
	
	verticalShiftVoltAmpEdit->setText(qstr(optionsIn.verticalShiftVoltageAmplitude));
	emGainEdit->setText(qstr(optionsIn.emGainLevel));

}


void AndorCameraSettingsControl::setRunSettings(AndorRunSettings inputSettings){
	currentlyRunningSettings = inputSettings;
	picSettingsObj.setUnofficialExposures ( inputSettings.exposureTimes );
	picSettingsObj.setUnofficialPicsPerRep ( inputSettings.picsPerRepetition );
	///
	updateDisplays ();
}


void AndorCameraSettingsControl::handleSetTemperaturePress(){
	try{
		configSettings.andor.temperatureSetting = boost::lexical_cast<int>(str(temperatureEdit->text ()));
	}
	catch ( boost::bad_lexical_cast&){
		throwNested("Error: Couldn't convert temperature input to a double! Check for unusual characters.");
	}
}

void AndorCameraSettingsControl::updateTriggerMode( ){
	if (currentlyUneditable) {
		return;
	}
	int itemIndex = triggerCombo->currentIndex( );
	if ( itemIndex == -1 ){
		return;
	}
	configSettings.andor.triggerMode = AndorTriggerMode::fromStr(str(triggerCombo->currentText ()));
}

unsigned AndorCameraSettingsControl::getHsSpeed () {
	return horizontalShiftSpeedCombo->currentIndex ();
}
unsigned AndorCameraSettingsControl::getVsSpeed () {
	return verticalShiftSpeedCombo->currentIndex ();
}
unsigned AndorCameraSettingsControl::getFrameTransferMode () {
	return frameTransferModeCombo->currentIndex ();
}
int AndorCameraSettingsControl::getVerticalShiftVoltageAmplitude() {
	if (verticalShiftVoltAmpEdit == nullptr) {
		return 0;
	}
	auto txt = verticalShiftVoltAmpEdit->text();
	try {
		int ind = boost::lexical_cast<int>(str(txt));
		if (ind < 0 || ind > 4) {
			thrower("Vertical Shift Voltage Amplitude index not a valid index! should be between 0 and 4!");
		}
		return ind;
	}
	catch (boost::bad_lexical_cast&) {
		throwNested("Failed to Convert vertical shift voltage amplitude to an index! Should be 0 to 4.");
	}
}

void AndorCameraSettingsControl::updateSettings(){
	if (currentlyUneditable) {
		return;
	}
	// update all settings with current values from controls
	configSettings.andor.controlCamera =		controlAndorCameraCheck->isChecked ();
	configSettings.andor.exposureTimes =		picSettingsObj.getUsedExposureTimes( );
	configSettings.thresholds =				picSettingsObj.getThresholds( );
	configSettings.palleteNumbers =			picSettingsObj.getPictureColors( );
	configSettings.andor.picsPerRepetition =	picSettingsObj.getPicsPerRepetition( );
	configSettings.picScaleFactor = picSettingsObj.getPicScaleFactor ();
	configSettings.andor.verticalShiftVoltageAmplitude = getVerticalShiftVoltageAmplitude();
	configSettings.andor.imageSettings = readImageParameters( );
	configSettings.andor.kineticCycleTime = getKineticCycleTime( );
	configSettings.andor.accumulationTime = getAccumulationCycleTime( );
	configSettings.andor.accumulationNumber = getAccumulationNumber( );

	updateCameraMode( );
	updateTriggerMode( );

	configSettings.andor.horShiftSpeedSetting = getHsSpeed ();
	configSettings.andor.vertShiftSpeedSetting = getVsSpeed ();
	configSettings.andor.frameTransferMode = getFrameTransferMode ();
	
}

std::array<softwareAccumulationOption, 4> AndorCameraSettingsControl::getSoftwareAccumulationOptions ( ){
	return picSettingsObj.getSoftwareAccumulationOptions();
}

AndorCameraSettings AndorCameraSettingsControl::getConfigSettings(){
	updateSettings ();
	return configSettings;
}

AndorRunSettings AndorCameraSettingsControl::getRunningSettings () {
	return currentlyRunningSettings;
}

bool AndorCameraSettingsControl::getAutoCal( ){
	return calControl.autoCal( );
}

bool AndorCameraSettingsControl::getUseCal( ){
	return calControl.use( );
}

void AndorCameraSettingsControl::setEmGain( bool emGainCurrentlyOn, int currentEmGainLevel ){
	auto emGainText = emGainEdit->text();
	if ( emGainText == "" ){
		// set to off.
		emGainText = "-1";
	}
	int emGain;
	try{
		emGain = boost::lexical_cast<int>(str(emGainText));
	}
	catch ( boost::bad_lexical_cast&){
		throwNested("ERROR: Couldn't convert EM Gain text to integer! Aborting!");
	}
	// < 0 corresponds to NOT USING EM GAIN (using conventional gain).
	if (emGain < 0){
		configSettings.andor.emGainModeIsOn = false;
		configSettings.andor.emGainLevel = 0;
		emGainDisplay->setText("OFF");
	}
	else{
		configSettings.andor.emGainModeIsOn = true;
		configSettings.andor.emGainLevel = emGain;
		emGainDisplay->setText(cstr("Gain: X" + str(configSettings.andor.emGainLevel)));
	}
	// Change the andor settings.
	std::string promptMsg = "";
	if ( emGainCurrentlyOn != configSettings.andor.emGainModeIsOn ){
		promptMsg += "Set Andor EM Gain State to " + str(configSettings.andor.emGainModeIsOn ? "ON" : "OFF");
	}
	if ( currentEmGainLevel != configSettings.andor.emGainLevel ){
		if ( promptMsg != "" ){
			promptMsg += ", ";
		}
		promptMsg += "Set Andor EM Gain Level to " + str(configSettings.andor.emGainLevel);
	}
	if ( promptMsg != "" ){
		promptMsg += "?";
		auto result = QMessageBox::question (nullptr, "Andor Settings", qstr(promptMsg));
		if ( result == QMessageBox::No ){
			thrower ( "Aborting camera settings update at EM Gain update!" );
		}
	}
}

void AndorCameraSettingsControl::setVariationNumber(unsigned varNumber){
	AndorRunSettings& andorSettings = configSettings.andor;
	andorSettings.totalVariations = varNumber;
	if ( andorSettings.totalPicsInExperiment() > INT_MAX){
		thrower ( "ERROR: Trying to take too many pictures! Maximum picture number is " + str( INT_MAX ) );
	}
}

void AndorCameraSettingsControl::setRepsPerVariation(unsigned repsPerVar){
	AndorRunSettings& andorSettings = configSettings.andor;
	andorSettings.repetitionsPerVariation = repsPerVar;
	if ( andorSettings.totalPicsInExperiment() > INT_MAX){
		thrower ( "ERROR: Trying to take too many pictures! Maximum picture number is " + str( INT_MAX ) );
	}
}

void AndorCameraSettingsControl::changeTemperatureDisplay( AndorTemperatureStatus stat ){
	temperatureDisplay->setText ( cstr ( stat.temperatureSetting ) );
	temperatureMsg->setText ( cstr ( stat.msg ) );
}

void AndorCameraSettingsControl::updateRunSettingsFromPicSettings( ){
	configSettings.andor.exposureTimes = picSettingsObj.getUsedExposureTimes( );
	configSettings.andor.picsPerRepetition = picSettingsObj.getPicsPerRepetition( );
	if ( configSettings.andor.totalPicsInExperiment ( ) > INT_MAX ){
		thrower ( "ERROR: Trying to take too many pictures! Maximum picture number is " + str( INT_MAX ) );
	}
}

void AndorCameraSettingsControl::handlePictureSettings(){
	picSettingsObj.handleOptionChange();
	updateRunSettingsFromPicSettings( );
}

double AndorCameraSettingsControl::getKineticCycleTime( ){
	if (!kineticCycleTimeEdit) {
		return 0;
	}
	try{
		configSettings.andor.kineticCycleTime = boost::lexical_cast<float>( str(kineticCycleTimeEdit->text ()) );
		kineticCycleTimeEdit->setText( cstr( configSettings.andor.kineticCycleTime ) );
	}
	catch ( boost::bad_lexical_cast& ){
		configSettings.andor.kineticCycleTime = 0.1f;
		kineticCycleTimeEdit->setText ( cstr( configSettings.andor.kineticCycleTime ) );
		throwNested( "Please enter a valid float for the kinetic cycle time." );
	}
	return configSettings.andor.kineticCycleTime;
}

double AndorCameraSettingsControl::getAccumulationCycleTime( ){
	if (!accumulationCycleTimeEdit){
		return 0;
	}
	try	{
		configSettings.andor.accumulationTime = boost::lexical_cast<float>( str(accumulationCycleTimeEdit->text ()) );
		accumulationCycleTimeEdit->setText( cstr( configSettings.andor.accumulationTime ) );
	}
	catch ( boost::bad_lexical_cast& ){
		configSettings.andor.accumulationTime = 0.1f;
		accumulationCycleTimeEdit->setText( cstr( configSettings.andor.accumulationTime ) );
		throwNested( "Please enter a valid float for the accumulation cycle time." );
	}
	return configSettings.andor.accumulationTime;
}

unsigned AndorCameraSettingsControl::getAccumulationNumber( ){
	if (!accumulationNumberEdit){
		return 0;
	}
	try	{
		configSettings.andor.accumulationNumber = boost::lexical_cast<long>( str(accumulationNumberEdit->text ()) );
		accumulationNumberEdit->setText( cstr( configSettings.andor.accumulationNumber ) );
	}
	catch ( boost::bad_lexical_cast& ){
		configSettings.andor.accumulationNumber = 1;
		accumulationNumberEdit->setText( cstr( configSettings.andor.accumulationNumber ) );
		throwNested( "Please enter a valid float for the Accumulation number." );
	}
	return configSettings.andor.accumulationNumber;
}

void AndorCameraSettingsControl::updatePicSettings ( andorPicSettingsGroup settings ){
	picSettingsObj.updateAllSettings ( settings );
}

void AndorCameraSettingsControl::updateImageDimSettings( imageParameters settings ){
	imageDimensionsObj.setImageParametersFromInput ( settings );
}

andorPicSettingsGroup AndorCameraSettingsControl::getPictureSettingsFromConfig (ConfigStream& configFile ){
	return PictureSettingsControl::getPictureSettingsFromConfig ( configFile );
}

void AndorCameraSettingsControl::handleSaveConfig(ConfigStream& saveFile){
	updateSettings ();
	saveFile << "CAMERA_SETTINGS\n";
	saveFile << "/*Control Andor:*/\t\t\t" << configSettings.andor.controlCamera << "\n";
	saveFile << "/*Trigger Mode:*/\t\t\t" << AndorTriggerMode::toStr(configSettings.andor.triggerMode) << "\n";
	saveFile << "/*EM-Gain Is On:*/\t\t\t" << configSettings.andor.emGainModeIsOn << "\n";
	saveFile << "/*EM-Gain Level:*/\t\t\t" << configSettings.andor.emGainLevel << "\n";
	saveFile << "/*Acquisition Mode:*/\t\t" << AndorRunModes::toStr (configSettings.andor.acquisitionMode) << "\n";
	saveFile << "/*Kinetic Cycle Time:*/\t\t" << configSettings.andor.kineticCycleTime << "\n";
	saveFile << "/*Accumulation Time:*/\t\t" << configSettings.andor.accumulationTime << "\n";
	saveFile << "/*Accumulation Number:*/\t" << configSettings.andor.accumulationNumber << "\n";
	saveFile << "/*Camera Temperature:*/\t\t" << configSettings.andor.temperatureSetting << "\n";
	saveFile << "/*Number of Exposures:*/\t" << configSettings.andor.exposureTimes.size ( ) 
			 << "\n/*Exposure Times:*/\t\t\t";
	for ( auto exposure : configSettings.andor.exposureTimes ){
		saveFile << exposure << " ";
	}
	saveFile << "\n/*Andor Pics Per Rep:*/\t\t" << configSettings.andor.picsPerRepetition;
	saveFile << "\n/*Horizontal Shift Speed:*/\t" << configSettings.andor.horShiftSpeedSetting;
	saveFile << "\n/*Vertical Shift Speed:*/\t" << configSettings.andor.vertShiftSpeedSetting;
	saveFile << "\n/*Frame Transfer Mode:*/\t" << configSettings.andor.frameTransferMode;
	saveFile << "\n/*vertical Shift Voltage Amplitude Index:*/\t" << configSettings.andor.verticalShiftVoltageAmplitude;
	saveFile << "\nEND_CAMERA_SETTINGS\n";
	picSettingsObj.handleSaveConfig(saveFile);
	imageDimensionsObj.handleSave (saveFile);
}


void AndorCameraSettingsControl::updateCameraMode( ){
	/* updates settings.andor.cameraMode based on combo selection, then updates 
		settings.andor.acquisitionMode and other settings depending on the mode.
	*/
	if (currentlyUneditable) {
		return;
	}
	int sel = cameraModeCombo->currentIndex( );
	if ( sel == -1 ){
		return;
	}
	std::string txt (str(cameraModeCombo->currentText ()));
	if ( txt == AndorRunModes::toStr (AndorRunModes::mode::Video) || txt == "Video Mode" ){
		configSettings.andor.acquisitionMode = AndorRunModes::mode::Video;
		configSettings.andor.repetitionsPerVariation = INT_MAX;
	}
	else if ( txt == AndorRunModes::toStr ( AndorRunModes::mode::Kinetic ) || txt == "Kinetic Series Mode" ){
		configSettings.andor.acquisitionMode = AndorRunModes::mode::Kinetic;
	}
	else if ( txt == AndorRunModes::toStr ( AndorRunModes::mode::Accumulate ) 
		|| txt == "Accumulate Mode" || txt == "Accumulation Mode" ){
		configSettings.andor.acquisitionMode = AndorRunModes::mode::Accumulate;
	}
	else{
		thrower  ( "ERROR: unrecognized combo for andor run mode text???" );
	}
}

void AndorCameraSettingsControl::updateWindowEnabledStatus (){
	controlAndorCameraCheck->setEnabled (!viewRunningSettings->isChecked ());
	cameraModeCombo->setEnabled (!viewRunningSettings->isChecked ());
	emGainEdit->setEnabled (!viewRunningSettings->isChecked () && !currentlyRunning);
	emGainBtn->setEnabled (!viewRunningSettings->isChecked ());
	triggerCombo->setEnabled (!viewRunningSettings->isChecked ());

	auto settings = getConfigSettings ();
	accumulationCycleTimeEdit->setEnabled(settings.andor.acquisitionMode == AndorRunModes::mode::Accumulate 
		&& !viewRunningSettings->isChecked ());
	accumulationNumberEdit->setEnabled (settings.andor.acquisitionMode == AndorRunModes::mode::Accumulate 
		&& !viewRunningSettings->isChecked ());
	kineticCycleTimeEdit->setEnabled (settings.andor.acquisitionMode == AndorRunModes::mode::Video 
		&& !viewRunningSettings->isChecked ());
	
	imageDimensionsObj.updateEnabledStatus (viewRunningSettings->isChecked ());
	picSettingsObj.setEnabledStatus (viewRunningSettings->isChecked ());

}

void AndorCameraSettingsControl::updateMinKineticCycleTime( double time ){
	minKineticCycleTimeDisp->setText( cstr( time ) );
}

imageParameters AndorCameraSettingsControl::readImageParameters(){
	return imageDimensionsObj.readImageParameters( );
}

void AndorCameraSettingsControl::setImageParameters(imageParameters newSettings){
	imageDimensionsObj.setImageParametersFromInput(newSettings);
}

void AndorCameraSettingsControl::checkIfReady(){
	if ( picSettingsObj.getUsedExposureTimes().size() == 0 ){
		thrower ("Please Set at least one exposure time.");
	}
	if ( !imageDimensionsObj.checkReady() ){
		thrower ("Please set the image parameters.");
	}
	if ( configSettings.andor.picsPerRepetition <= 0 ){
		thrower ("ERROR: Please set the number of pictures per repetition to a positive non-zero value.");
	}
	if ( configSettings.andor.acquisitionMode == AndorRunModes::mode::Kinetic ){
		if ( configSettings.andor.kineticCycleTime == 0 && configSettings.andor.triggerMode == AndorTriggerMode::mode::Internal ){
			thrower ("ERROR: Since you are running in internal trigger mode, please Set a kinetic cycle time.");
		}
		if ( configSettings.andor.repetitionsPerVariation <= 0 ){
			thrower ("ERROR: Please set the \"Repetitions Per Variation\" variable to a positive non-zero value.");
		}
		if ( configSettings.andor.totalVariations <= 0 ){
			thrower ("ERROR: Please set the number of variations to a positive non-zero value.");
		}
	}
	if ( configSettings.andor.acquisitionMode == AndorRunModes::mode::Accumulate ){
		if ( configSettings.andor.accumulationNumber <= 0 ){
			thrower ("ERROR: Please set the current Accumulation Number to a positive non-zero value.");
		}
		if ( configSettings.andor.accumulationTime <= 0 ){
			thrower ("ERROR: Please set the current Accumulation Time to a positive non-zero value.");
		}
	}
}

void AndorCameraSettingsControl::handelSaveMasterConfig ( std::stringstream& configFile ){
	imageParameters settings = getConfigSettings ( ).andor.imageSettings;
	configFile << settings.left << " " << settings.right << " " << settings.horizontalBinning << " ";
	configFile << settings.bottom << " " << settings.top << " " << settings.verticalBinning << "\n";
	// introduced in version 2.2
	configFile << getAutoCal ( ) << " " << getUseCal ( ) << "\n";
}

void AndorCameraSettingsControl::handleOpenMasterConfig ( ConfigStream& configStream, QtAndorWindow* camWin ){
	imageParameters settings = getConfigSettings ( ).andor.imageSettings;
	std::string tempStr;
	try	{
		configStream >> tempStr;
		settings.left = boost::lexical_cast<long> ( tempStr );
		configStream >> tempStr;
		settings.right = boost::lexical_cast<long> ( tempStr );
		configStream >> tempStr;
		settings.horizontalBinning = boost::lexical_cast<long> ( tempStr );
		configStream >> tempStr;
		settings.bottom = boost::lexical_cast<long> ( tempStr );
		configStream >> tempStr;
		settings.top = boost::lexical_cast<long> ( tempStr );
		configStream >> tempStr;
		settings.verticalBinning = boost::lexical_cast<long> ( tempStr );
		setImageParameters ( settings );
	}
	catch ( boost::bad_lexical_cast& ){
		throwNested ( "ERROR: Bad value (i.e. failed to convert to long) seen in master configueration file while attempting "
				  "to load camera dimensions!" );
	}
	bool autoCal, useCal;
	if ( configStream.ver > Version ( "2.1" ) ){
		configStream >> autoCal >> useCal;
		calControl.setAutoCal ( autoCal );
		calControl.setUse ( useCal );
	}

}


std::vector<Matrix<long>> AndorCameraSettingsControl::getImagesToDraw ( const std::vector<Matrix<long>>& rawData ){
	std::vector<Matrix<long>> imagesToDraw ( rawData.size ( ) );
	auto options = picSettingsObj.getDisplayTypeOptions ( );
	for ( auto picNum : range ( rawData.size ( ) ) ){
		if ( !options[ picNum ].isDiff ){
			imagesToDraw[ picNum ] = rawData[ picNum ];
		}
		else{
			// the whichPic variable is 1-indexed.
			if ( options[ picNum ].whichPicForDif >= rawData.size ( ) ){
				imagesToDraw[ picNum ] = rawData[ picNum ];
			}
			else{
				imagesToDraw[ picNum ] = Matrix<long>(rawData[picNum].getRows(), rawData[picNum].getCols(), 0);
				for ( auto i : range ( rawData[ picNum ].size ( ) ) ){
					imagesToDraw[ picNum ].data[ i ] = rawData[ picNum ].data[ i ] - rawData[ options[ picNum ].whichPicForDif - 1 ].data[ i ];
				}
			}
		}
	}
	return imagesToDraw;
}

Qt::TransformationMode AndorCameraSettingsControl::getTransformationMode (){
	return picSettingsObj.getTransformationMode();
}
