// created by Mark O. Brown
#include "stdafx.h"

#include "CameraSettingsControl.h"

#include "PrimaryWindows/QtAndorWindow.h"
#include "GeneralUtilityFunctions/miscCommonFunctions.h"
#include "ConfigurationSystems/ProfileSystem.h"

#include <boost/lexical_cast.hpp>


AndorCameraSettingsControl::AndorCameraSettingsControl() : imageDimensionsObj("andor"){
	AndorRunSettings& andorSettings = settings.andor;
}


void AndorCameraSettingsControl::initialize (POINT& pos, IChimeraWindowWidget* parent){
	header = new QLabel ("CAMERA SETTINGS", parent);
	header->setGeometry (pos.x, pos.y, 480, 25);

	cameraModeCombo = new CQComboBox (parent);
	cameraModeCombo->addItem ("Kinetic-Series-Mode");
	cameraModeCombo->addItem ("Accumulation-Mode");
	cameraModeCombo->addItem ("Video-Mode");
	cameraModeCombo->setGeometry (pos.x, pos.y += 25, 240, 25);
	cameraModeCombo->setCurrentIndex (0);
	parent->connect (cameraModeCombo, qOverload<int> (&QComboBox::currentIndexChanged), 
		[this, parent]() {
			updateCameraMode ();
			updateWindowEnabledStatus ();
		});
	settings.andor.acquisitionMode = AndorRunModes::mode::Kinetic;
	triggerCombo = new CQComboBox (parent);
	triggerCombo->setGeometry (pos.x + 240, pos.y, 240, 25);
	triggerCombo->addItems ({ "Internal-Trigger", "External-Trigger","Start-On-Trigger" });
	triggerCombo->setCurrentIndex (0);
	parent->connect (triggerCombo, qOverload<int> (&QComboBox::currentIndexChanged), 
		[this, parent]() {
			updateTriggerMode ();
			updateWindowEnabledStatus ();
		});
	settings.andor.triggerMode = AndorTriggerMode::mode::External;
	emGainBtn = new CQPushButton ("Set EM Gain (-1=OFF)", parent);
	emGainBtn->setGeometry (pos.x, pos.y += 25, 160, 20);
	parent->connect (emGainBtn, &QPushButton::released, [parent]() {
			parent->andorWin->handleEmGainChange ();
		});
	emGainEdit = new CQLineEdit ("-1", parent);
	emGainEdit->setGeometry (pos.x + 160, pos.y, 160, 20);
	emGainEdit->setToolTip( "Set the state & gain of the EM gain of the camera. Enter a negative number to turn EM Gain"
						   " mode off. The program will immediately change the state of the camera after changing this"
						   " edit." );
	//
	emGainDisplay = new QLabel ("OFF", parent);
	emGainDisplay->setGeometry (pos.x + 320, pos.y, 160, 20);
	// initialize settings.
	settings.andor.emGainLevel = 0;
	settings.andor.emGainModeIsOn = false;
	setTemperatureButton = new CQPushButton ("Set Camera Temperature (C)", parent);
	setTemperatureButton->setGeometry (pos.x, pos.y+=20, 270, 25);
	parent->connect (setTemperatureButton, &QPushButton::released, 
		[parent]() {
			parent->andorWin->passSetTemperaturePress ();
		});

	temperatureEdit = new CQLineEdit ("0", parent);
	temperatureEdit->setGeometry (pos.x + 270, pos.y, 80, 25);

	temperatureDisplay = new QLabel ("", parent);
	temperatureDisplay->setGeometry (pos.x + 350, pos.y, 80, 25);
	temperatureOffButton = new CQPushButton("OFF", parent);
	temperatureOffButton->setGeometry (pos.x + 430, pos.y, 50, 25);
	temperatureMsg = new QLabel ("Temperature control is disabled",parent);
	temperatureMsg->setGeometry (pos.x, pos.y+=25, 480, 50);
	pos.y += 50;
	//
	picSettingsObj.initialize( pos, parent );
	imageDimensionsObj.initialize( pos, parent, 1, 480 );

	// Accumulation Time
	accumulationCycleTimeLabel = new QLabel ("Accumulation Cycle Time", parent);
	accumulationCycleTimeLabel->setGeometry (pos.x, pos.y, 240, 25);

	accumulationCycleTimeEdit = new CQLineEdit ("0.1", parent);
	accumulationCycleTimeEdit->setGeometry (pos.x + 240, pos.y, 240, 25);

	// Accumulation Number
	accumulationNumberLabel = new QLabel ("Accumulation #", parent);
	accumulationNumberLabel->setGeometry (pos.x, pos.y+=25, 240, 25);
	accumulationNumberEdit = new CQLineEdit ("1", parent);
	accumulationNumberEdit->setGeometry (pos.x + 240, pos.y, 240, 25);

	// minimum kinetic cycle time (determined by camera)
	minKineticCycleTimeLabel = new QLabel ("Minimum Kinetic Cycle Time (s)", parent);
	minKineticCycleTimeLabel->setGeometry (pos.x, pos.y+=25, 240, 25);
	minKineticCycleTimeDisp = new QLabel ("---", parent);
	minKineticCycleTimeDisp->setGeometry (pos.x + 240, pos.y, 240, 25);

	/// Kinetic Cycle Time
	kineticCycleTimeLabel = new QLabel ("Kinetic Cycle Time (s)", parent);
	kineticCycleTimeLabel->setGeometry (pos.x, pos.y+=25, 240, 25);

	kineticCycleTimeEdit = new CQLineEdit ("0.1", parent);
	kineticCycleTimeEdit->setGeometry (pos.x+240, pos.y, 240, 25);
	pos.y += 25;
	//
	calControl.initialize( pos, parent );
	updateWindowEnabledStatus ();
}


// note that this object doesn't actually store the camera state, it just uses it in passing to figure out whether 
// buttons should be on or off.
void AndorCameraSettingsControl::cameraIsOn(bool state){
	// Can't change em gain mode or camera settings once started.
	emGainEdit->setEnabled( !state );
	setTemperatureButton->setEnabled ( !state );
	temperatureOffButton->setEnabled ( !state );
}


void AndorCameraSettingsControl::setRunSettings(AndorRunSettings inputSettings){
	// try to set this time.
	picSettingsObj.setUnofficialExposures ( inputSettings.exposureTimes );
	picSettingsObj.setUnofficialPicsPerRep ( inputSettings.picsPerRepetition );
	///
	kineticCycleTimeEdit->setText(cstr(inputSettings.kineticCycleTime));
	accumulationCycleTimeEdit->setText (cstr(inputSettings.accumulationTime));
	int ind = cameraModeCombo->findData (AndorRunModes::toStr (inputSettings.acquisitionMode).c_str ());
	if (ind != -1) {
		cameraModeCombo->setCurrentIndex (ind);
	}
	ind = triggerCombo->findData (AndorTriggerMode::toStr (inputSettings.triggerMode).c_str ());
	if (ind != -1) {
		triggerCombo->setCurrentIndex (ind);
	}
	if ( inputSettings.acquisitionMode == AndorRunModes::mode::Video ){
		inputSettings.repetitionsPerVariation = INT_MAX;
	}
	else if ( inputSettings.acquisitionMode == AndorRunModes::mode::Kinetic ){
		
	}
	else if ( inputSettings.acquisitionMode == AndorRunModes::mode::Accumulate ){
		inputSettings.repetitionsPerVariation = INT_MAX;
	}
	else{
		thrower ( "ERROR: unrecognized camera mode: " + AndorRunModes::toStr(inputSettings.acquisitionMode) );
	}
	kineticCycleTimeEdit->setText (cstr(inputSettings.kineticCycleTime));
	accumulationCycleTimeEdit->setText (cstr(inputSettings.accumulationTime * 1000.0));
	accumulationNumberEdit->setText (cstr(inputSettings.accumulationNumber));
	temperatureEdit->setText (cstr(inputSettings.temperatureSetting));
}


void AndorCameraSettingsControl::handleSetTemperaturePress(){
	int temp;
	try{
		temp = boost::lexical_cast<int>(str(temperatureEdit->text ()));
	}
	catch ( boost::bad_lexical_cast&){
		throwNested("Error: Couldn't convert temperature input to a double! Check for unusual characters.");
	}
	settings.andor.temperatureSetting = temp;
}


void AndorCameraSettingsControl::updateTriggerMode( ){
	int itemIndex = triggerCombo->currentIndex( );
	if ( itemIndex == -1 ){
		return;
	}
	settings.andor.triggerMode = AndorTriggerMode::fromStr(str(triggerCombo->currentText ()));
}


void AndorCameraSettingsControl::updateSettings(){
	// update all settings with current values from controls
	settings.andor.exposureTimes =		picSettingsObj.getUsedExposureTimes( );
	settings.thresholds =				picSettingsObj.getThresholds( );
	settings.palleteNumbers =			picSettingsObj.getPictureColors( );
	settings.andor.picsPerRepetition =	picSettingsObj.getPicsPerRepetition( );
	
	settings.andor.imageSettings = getImageParameters( );
	settings.andor.kineticCycleTime = getKineticCycleTime( );
	settings.andor.accumulationTime = getAccumulationCycleTime( );
	settings.andor.accumulationNumber = getAccumulationNumber( );

	updateCameraMode( );
	updateTriggerMode( );
}


std::array<softwareAccumulationOption, 4> AndorCameraSettingsControl::getSoftwareAccumulationOptions ( ){
	return picSettingsObj.getSoftwareAccumulationOptions();
}


AndorCameraSettings AndorCameraSettingsControl::getSettings(){
	updateSettings( );
	return settings;
}


AndorCameraSettings AndorCameraSettingsControl::getCalibrationSettings( ){
	AndorCameraSettings calSettings;
	calSettings.andor.acquisitionMode = AndorRunModes::mode::Kinetic;
	calSettings.andor.emGainLevel = 0;
	calSettings.andor.emGainModeIsOn = false;
	calSettings.andor.exposureTimes = { float(10e-3) };
	// want to calibrate the image area to be used in the experiment, so...
	calSettings.andor.imageSettings = imageDimensionsObj.getImageParameters( );
	calSettings.andor.kineticCycleTime = 10e-3f;
	calSettings.andor.picsPerRepetition = 1;
	calSettings.andor.readMode = 4;
	calSettings.andor.repetitionsPerVariation = 100;
	calSettings.andor.showPicsInRealTime = false;
	calSettings.andor.temperatureSetting = -60;
	calSettings.andor.totalVariations = 1;
	calSettings.andor.triggerMode = AndorTriggerMode::mode::External;
	return calSettings;
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
		settings.andor.emGainModeIsOn = false;
		settings.andor.emGainLevel = 0;
		emGainDisplay->setText("OFF");
	}
	else{
		settings.andor.emGainModeIsOn = true;
		settings.andor.emGainLevel = emGain;
		emGainDisplay->setText(cstr("Gain: X" + str(settings.andor.emGainLevel)));
	}
	// Change the andor settings.
	std::string promptMsg = "";
	if ( emGainCurrentlyOn != settings.andor.emGainModeIsOn ){
		promptMsg += "Set Andor EM Gain State to " + str(settings.andor.emGainModeIsOn ? "ON" : "OFF");
	}
	if ( currentEmGainLevel != settings.andor.emGainLevel ){
		if ( promptMsg != "" ){
			promptMsg += ", ";
		}
		promptMsg += "Set Andor EM Gain Level to " + str(settings.andor.emGainLevel);
	}
	if ( promptMsg != "" ){
		promptMsg += "?";
		int result = promptBox( promptMsg, MB_YESNO );
		if ( result == IDNO ){
			thrower ( "Aborting camera settings update at EM Gain update!" );
		}
	}
}


void AndorCameraSettingsControl::setVariationNumber(UINT varNumber){
	AndorRunSettings& andorSettings = settings.andor;
	andorSettings.totalVariations = varNumber;
	if ( andorSettings.totalPicsInExperiment() > INT_MAX){
		thrower ( "ERROR: Trying to take too many pictures! Maximum picture number is " + str( INT_MAX ) );
	}
}


void AndorCameraSettingsControl::setRepsPerVariation(UINT repsPerVar){
	AndorRunSettings& andorSettings = settings.andor;
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
	settings.andor.exposureTimes = picSettingsObj.getUsedExposureTimes( );
	settings.andor.picsPerRepetition = picSettingsObj.getPicsPerRepetition( );
	if ( settings.andor.totalPicsInExperiment ( ) > INT_MAX ){
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
		settings.andor.kineticCycleTime = boost::lexical_cast<float>( str(kineticCycleTimeEdit->text ()) );
		kineticCycleTimeEdit->setText( cstr( settings.andor.kineticCycleTime ) );
	}
	catch ( boost::bad_lexical_cast& ){
		settings.andor.kineticCycleTime = 0.1f;
		kineticCycleTimeEdit->setText ( cstr( settings.andor.kineticCycleTime ) );
		throwNested( "Please enter a valid float for the kinetic cycle time." );
	}
	return settings.andor.kineticCycleTime;
}


double AndorCameraSettingsControl::getAccumulationCycleTime( )
{
	CString text;
	if (!accumulationCycleTimeEdit){
		return 0;
	}
	try	{
		settings.andor.accumulationTime = boost::lexical_cast<float>( str(accumulationCycleTimeEdit->text ()) );
		accumulationCycleTimeEdit->setText( cstr( settings.andor.accumulationTime ) );
	}
	catch ( boost::bad_lexical_cast& ){
		settings.andor.accumulationTime = 0.1f;
		accumulationCycleTimeEdit->setText( cstr( settings.andor.accumulationTime ) );
		throwNested( "Please enter a valid float for the accumulation cycle time." );
	}
	return settings.andor.accumulationTime;
}


UINT AndorCameraSettingsControl::getAccumulationNumber( )
{
	if (!accumulationNumberEdit){
		return 0;
	}
	try	{
		settings.andor.accumulationNumber = boost::lexical_cast<long>( str(accumulationNumberEdit->text ()) );
		accumulationNumberEdit->setText( cstr( settings.andor.accumulationNumber ) );
	}
	catch ( boost::bad_lexical_cast& ){
		settings.andor.accumulationNumber = 1;
		accumulationNumberEdit->setText( cstr( settings.andor.accumulationNumber ) );
		throwNested( "Please enter a valid float for the Accumulation number." );
	}
	return settings.andor.accumulationNumber;
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
	updateSettings ( ); 
	saveFile << "CAMERA_SETTINGS\n";
	saveFile << "/*Trigger Mode:*/\t\t\t" << AndorTriggerMode::toStr(settings.andor.triggerMode) << "\n";
	saveFile << "/*EM-Gain Is On:*/\t\t\t" << settings.andor.emGainModeIsOn << "\n";
	saveFile << "/*EM-Gain Level:*/\t\t\t" << settings.andor.emGainLevel << "\n";
	saveFile << "/*Acquisition Mode:*/\t\t" << AndorRunModes::toStr (settings.andor.acquisitionMode) << "\n";
	saveFile << "/*Kinetic Cycle Time:*/\t\t" << settings.andor.kineticCycleTime << "\n";
	saveFile << "/*Accumulation Time:*/\t\t" << settings.andor.accumulationTime << "\n";
	saveFile << "/*Accumulation Number:*/\t" << settings.andor.accumulationNumber << "\n";
	saveFile << "/*Camera Temperature:*/\t\t" << settings.andor.temperatureSetting << "\n";
	saveFile << "/*Number of Exposures:*/\t" << settings.andor.exposureTimes.size ( ) 
			 << "\n/*Exposure Times:*/\t\t\t";
	for ( auto exposure : settings.andor.exposureTimes ){
		saveFile << exposure << " ";
	}
	saveFile << "\n/*Andor Pics Per Rep:*/\t\t" << settings.andor.picsPerRepetition << "\n";
	saveFile << "END_CAMERA_SETTINGS\n";
	picSettingsObj.handleSaveConfig(saveFile);
	imageDimensionsObj.handleSave (saveFile);
}


void AndorCameraSettingsControl::updateCameraMode( ){
	/* 
		updates settings.andor.cameraMode based on combo selection, then updates 
		settings.andor.acquisitionMode and other settings depending on the mode.
	*/
	int sel = cameraModeCombo->currentIndex( );
	if ( sel == -1 ){
		return;
	}
	std::string txt (str(cameraModeCombo->currentText ()));
	if ( txt == AndorRunModes::toStr (AndorRunModes::mode::Video) || txt == "Video Mode" ){
		settings.andor.acquisitionMode = AndorRunModes::mode::Video;
		settings.andor.repetitionsPerVariation = INT_MAX;
	}
	else if ( txt == AndorRunModes::toStr ( AndorRunModes::mode::Kinetic ) || txt == "Kinetic Series Mode" ){
		settings.andor.acquisitionMode = AndorRunModes::mode::Kinetic;
	}
	else if ( txt == AndorRunModes::toStr ( AndorRunModes::mode::Accumulate ) 
		|| txt == "Accumulate Mode" || txt == "Accumulation Mode" ){
		settings.andor.acquisitionMode = AndorRunModes::mode::Accumulate;
	}
	else{
		thrower  ( "ERROR: unrecognized combo for andor run mode text???" );
	}
}

void AndorCameraSettingsControl::updateWindowEnabledStatus (){
	auto settings = getSettings ();
	accumulationCycleTimeEdit->setEnabled(settings.andor.acquisitionMode == AndorRunModes::mode::Accumulate);
	accumulationNumberEdit->setEnabled (settings.andor.acquisitionMode == AndorRunModes::mode::Accumulate);
	kineticCycleTimeEdit->setEnabled (settings.andor.acquisitionMode == AndorRunModes::mode::Video);
}

void AndorCameraSettingsControl::updateMinKineticCycleTime( double time ){
	minKineticCycleTimeDisp->setText( cstr( time ) );
}

imageParameters AndorCameraSettingsControl::getImageParameters(){
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
	if ( settings.andor.picsPerRepetition <= 0 ){
		thrower ("ERROR: Please set the number of pictures per repetition to a positive non-zero value.");
	}
	if ( settings.andor.acquisitionMode == AndorRunModes::mode::Kinetic ){
		if ( settings.andor.kineticCycleTime == 0 && settings.andor.triggerMode == AndorTriggerMode::mode::Internal ){
			thrower ("ERROR: Since you are running in internal trigger mode, please Set a kinetic cycle time.");
		}
		if ( settings.andor.repetitionsPerVariation <= 0 ){
			thrower ("ERROR: Please set the \"Repetitions Per Variation\" variable to a positive non-zero value.");
		}
		if ( settings.andor.totalVariations <= 0 ){
			thrower ("ERROR: Please set the number of variations to a positive non-zero value.");
		}
	}
	if ( settings.andor.acquisitionMode == AndorRunModes::mode::Accumulate ){
		if ( settings.andor.accumulationNumber <= 0 ){
			thrower ("ERROR: Please set the current Accumulation Number to a positive non-zero value.");
		}
		if ( settings.andor.accumulationTime <= 0 ){
			thrower ("ERROR: Please set the current Accumulation Time to a positive non-zero value.");
		}
	}
}

void AndorCameraSettingsControl::handelSaveMasterConfig ( std::stringstream& configFile ){
	imageParameters settings = getSettings ( ).andor.imageSettings;
	configFile << settings.left << " " << settings.right << " " << settings.horizontalBinning << " ";
	configFile << settings.bottom << " " << settings.top << " " << settings.verticalBinning << "\n";
	// introduced in version 2.2
	configFile << getAutoCal ( ) << " " << getUseCal ( ) << "\n";
}

void AndorCameraSettingsControl::handleOpenMasterConfig ( ConfigStream& configStream, QtAndorWindow* camWin ){
	imageParameters settings = getSettings ( ).andor.imageSettings;
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


std::vector<Matrix<long>> AndorCameraSettingsControl::getImagesToDraw ( const std::vector<Matrix<long>>& rawData )
{
	std::vector<Matrix<long>> imagesToDraw ( rawData.size ( ) );
	auto options = picSettingsObj.getDisplayTypeOptions ( );
	for ( auto picNum : range ( rawData.size ( ) ) )
	{
		if ( !options[ picNum ].isDiff )
		{
			imagesToDraw[ picNum ] = rawData[ picNum ];
		}
		else
		{
			// the whichPic variable is 1-indexed.
			if ( options[ picNum ].whichPicForDif >= rawData.size ( ) )
			{
				imagesToDraw[ picNum ] = rawData[ picNum ];
			}
			else
			{
				imagesToDraw[ picNum ] = Matrix<long>(rawData[picNum].getRows(), rawData[picNum].getCols(), 0);
				for ( auto i : range ( rawData[ picNum ].size ( ) ) )
				{
					imagesToDraw[ picNum ].data[ i ] = rawData[ picNum ].data[ i ] - rawData[ options[ picNum ].whichPicForDif - 1 ].data[ i ];
				}
			}
		}
	}
	return imagesToDraw;
}

