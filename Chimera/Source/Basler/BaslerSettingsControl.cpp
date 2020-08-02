// created by Mark O. Brown
#include "stdafx.h"
#include "BaslerSettingsControl.h"
#include "ConfigurationSystems/Version.h"
#include "LowLevel/constants.h"
#include "ConfigurationSystems/ProfileSystem.h"
#include <boost/lexical_cast.hpp>

BaslerSettingsControl::BaslerSettingsControl ( ) : dims("scout")
{}


void BaslerSettingsControl::initialize ( POINT& pos, int picWidth, int picHeight, POINT cameraDims, 
	IChimeraQtWindow* win )
{
	int width = 300;
	lastTime = 0;
	statusText = new QLabel ("Camera Status: IDLE", win);
	statusText->setGeometry (pos.x, pos.y, width, 50);
	////
	repText = new QLabel ("Pics Per Rep:", win);
	repText->setGeometry (pos.x, pos.y+=50, 200, 25);
	repEdit = new QLineEdit ("100", win);
	repEdit->setGeometry (pos.x + 200, pos.y, 100, 25);
	cameraMode = new QComboBox (win);
	cameraMode->setGeometry (pos.x, pos.y+=25, 300, 25);
	cameraMode->addItems ({ "Finite-Acquisition","Continuous-Acquisition" });
	cameraMode->setCurrentIndex (0);
	repEdit->setEnabled ( false );

	
	std::string exposureTimeText;
	#ifdef USB_CAMERA
	exposureTimeText = "Exposure Time (us):";
	#elif defined FIREWIRE_CAMERA
	exposureTimeText = "Raw Time (# X 20 = us):";
	#endif
	exposureText = new QLabel (exposureTimeText.c_str(), win);
	exposureText->setGeometry (pos.x, pos.y+=25, 200, 25);

	exposureEdit = new QLineEdit ("1000", win);
	exposureEdit->setGeometry (pos.x + 200, pos.y, 100, 25);
	exposureModeCombo = new QComboBox (win);
	exposureModeCombo->setGeometry (pos.x, pos.y += 25, 300, 25);
	exposureModeCombo->addItems ({ "Auto-Exposure-Continuous","Auto-Exposure-Off","Auto-Exposure-Once" });
	exposureModeCombo->setCurrentIndex (0);

	dims.initialize (pos, win, 2, 300);
	triggerCombo = new QComboBox (win);
	triggerCombo->setGeometry (pos.x, pos.y, 300, 25);
	triggerCombo->addItems ({ "External-Trigger","Automatic-Software-Trigger","Manual-Software-Trigger" });
	triggerCombo->setCurrentIndex (0);
	frameRateText = new QLabel ("Frame Rate (pics/s)", win);
	frameRateText->setGeometry (pos.x, pos.y += 25, 150, 25);
	frameRateEdit = new QLineEdit ("0.25", win);
	frameRateEdit->setGeometry (pos.x + 150, pos.y, 75, 25);
	realFrameRate = new QLabel ("", win);
	realFrameRate->setGeometry (pos.x + 225, pos.y, 75, 25);
	gainCombo = new QComboBox (win);
	gainCombo->setGeometry (pos.x, pos.y += 25, 300, 25);
	gainCombo->addItems ({ "Auto-Gain-Continuous","Auto-Gain-Once","Auto-Gain-Off" });
	gainCombo->setCurrentIndex( 0 );
	gainText = new QLabel ("Raw G ain (260-?)", win);
	gainText->setGeometry (pos.x, pos.y += 25, 150, 25);
	gainEdit = new QLineEdit ("260", win);
	gainEdit->setGeometry (pos.x + 150, pos.y, 150, 25);
	realGainText = new QLabel ("Real Gain:", win);
	realGainText->setGeometry (pos.x, pos.y += 25, pos.x + 150, 25);
	realGainStatus = new QLabel ("", win);
	realGainStatus->setGeometry (pos.x+150, pos.y, 150, 25);
	motThreshold = new QLabel ("Mot-Threshold", win);
	motThreshold->setGeometry (pos.x, pos.y += 25, 150, 25);
	motThresholdEdit = new QLineEdit ("0", win);
	motThresholdEdit->setGeometry (pos.x + 150, pos.y, 125, 25);
	motLoadedColorbox = new QLabel ("", win);
	motLoadedColorbox->setGeometry (pos.x + 275, pos.y, 25, 25);
	handleCameraMode ();
}

void BaslerSettingsControl::redrawMotIndicator ( ){ 
	//motLoadedColorbox.RedrawWindow ( );
}


// assumes called on every 10 pics.
void BaslerSettingsControl::handleFrameRate(){
	ULONG currentTime = GetTickCount();
	ULONG timePerPic = (currentTime - lastTime)/10.0;
	if (timePerPic == 0){
		// avoid dividing by 0.
		timePerPic++;
	}
	realFrameRate->setText(cstr(1000.0 / timePerPic,5));
	lastTime = currentTime;
}


double BaslerSettingsControl::getMotThreshold ( ){
	try{
		return boost::lexical_cast<double>(str(motThresholdEdit->text( )));
	}
	catch ( boost::bad_lexical_cast& ){
		return 0;
	}
}


void BaslerSettingsControl::setStatus(std::string status){
	statusText->setText(status.c_str());
}


void BaslerSettingsControl::updateExposure( double exposure ){
	exposureEdit->setText( cstr( exposure,5 ) );
}


void BaslerSettingsControl::handleCameraMode(){
	auto text = cameraMode->currentText ();
	if (text == "Finite-Acquisition"){
		repEdit->setEnabled(true);
	}
	else{
		repEdit->setEnabled (false);
	}
}


baslerSettings BaslerSettingsControl::getCurrentSettings(){
	loadCurrentSettings ( );
	return currentSettings;
}


/*
Updates the internal object with gui settings
*/
baslerSettings BaslerSettingsControl::loadCurrentSettings ( ){
	isReady = false;
	int selection = exposureModeCombo->currentIndex ( );
	if ( selection == -1 ){
		thrower  ( "Please select an exposure mode for the basler camera." );
	}
	currentSettings.exposureMode = BaslerAutoExposure::fromStr(str(exposureModeCombo->currentText ()));
	if ( currentSettings.exposureMode == BaslerAutoExposure::mode::Off ){
		try{
			currentSettings.exposureTime = boost::lexical_cast<double> ( str( exposureEdit->text() ) );
			if ( currentSettings.exposureTime <= 0 ){
				thrower  ( "Invalid Basler exposure time!" );
			}
		}
		catch ( boost::bad_lexical_cast& ){
			throwNested ( "Error! Please input a valid double for the exposure time." );
		}
	}
	currentSettings.acquisitionMode = BaslerAcquisition::fromStr( str(cameraMode->currentText() ));
	if ( currentSettings.acquisitionMode == BaslerAcquisition::mode::Finite ){
		try{
			currentSettings.picsPerRep = boost::lexical_cast<int> ( str(repEdit->text()) );
			if ( currentSettings.picsPerRep == 0 ){
				thrower  ( "ERROR! Repetition count must be strictly positive." );
			}
		}
		catch ( boost::bad_lexical_cast& ){
			throwNested ( "Error! Please input a valid positive integer for the rep count." );
		}
	}
	dims.readImageParameters ();
	currentSettings.dims = dims.getImageParameters ();

	#ifdef USB_CAMERA
		currentSettings.dims.checkConsistency ( "ace" );
	#endif
	#ifdef FIREWIRE_CAMERA
		currentSettings.dims.checkConsistency ( "scout" );
	#endif
	if (currentSettings.dims.horizontalBinning > 4 || currentSettings.dims.verticalBinning > 4)	{
		thrower ( "Binning on a camera cannot exceed 4 pixels per bin!\r\n" );
	}
	currentSettings.triggerMode = BaslerTrigger::fromStr( str(triggerCombo->currentText()) );
	try{
		currentSettings.frameRate = boost::lexical_cast<double>(str(frameRateEdit->text()));
	}
	catch ( boost::bad_lexical_cast&){
		throwNested( std::string("ERROR! Please enter a valid float for the frame rate. ") );
	}
	isReady = true;
	return currentSettings;
}



void BaslerSettingsControl::handleSavingConfig ( ConfigStream& configFile ){ 
	loadCurrentSettings( );
	configFile  << "BASLER_CAMERA_SETTINGS\n";
	configFile  << BaslerAcquisition::toStr(currentSettings.acquisitionMode) 
				<< "\n/*Left:*/ " << currentSettings.dims.left 
				<< "\n/*Top:*/ " << currentSettings.dims.top 
				<< "\n/*Right:*/ " << currentSettings.dims.right 
				<< "\n/*Bottom:*/ " << currentSettings.dims.bottom 
				<< "\n/*H-Bin:*/ " << currentSettings.dims.horizontalBinning 
				<< "\n/*V-Bin:*/ " << currentSettings.dims.verticalBinning 
				<< "\n/*Exposure Mode:*/ " << BaslerAutoExposure::toStr(currentSettings.exposureMode) 
				<< "\n/*Exposure Time:*/ " << currentSettings.exposureTime 
				<< "\n/*Frame Rate:*/ " << currentSettings.frameRate 
				<< "\n/*Raw Gain:*/ " << currentSettings.rawGain 
				<< "\n/*Pics Per Rep:*/ " << currentSettings.picsPerRep 
				<< "\n/*Trigger Mode:*/ " << BaslerTrigger::toStr(currentSettings.triggerMode) 
				<< "\n";
	configFile << "END_BASLER_CAMERA_SETTINGS\n";
}



void BaslerSettingsControl::setSettings ( baslerSettings newSettings ){
	currentSettings = newSettings;
	cameraMode->setCurrentIndex (int(currentSettings.acquisitionMode));
	dims.setImageParametersFromInput (currentSettings.dims);
	exposureModeCombo->setCurrentIndex (int(currentSettings.exposureMode));
	exposureEdit->setText( cstr ( currentSettings.exposureTime,6 ) );
	frameRateEdit->setText ( cstr ( currentSettings.frameRate,6 ) );
	gainEdit->setText ( cstr ( currentSettings.rawGain,6 ) );
	repEdit->setText ( cstr ( currentSettings.picsPerRep) );
	triggerCombo->setCurrentIndex (int (currentSettings.triggerMode));
}


void BaslerSettingsControl::handleExposureMode(){
	if (exposureModeCombo->currentText() == "Auto Exposure Off") {
		exposureEdit->setEnabled(true);
	}
	else {
		exposureEdit->setEnabled(false);
	}
}


