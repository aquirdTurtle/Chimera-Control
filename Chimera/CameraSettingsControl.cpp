#include "stdafx.h"
#include "CameraSettingsControl.h"
#include "AndorWindow.h"
#include "miscCommonFunctions.h"
#include "Thrower.h"
#include <boost/lexical_cast.hpp>


AndorCameraSettingsControl::AndorCameraSettingsControl(AndorCamera* friendInitializer) : picSettingsObj(this)
{
	andorFriend = friendInitializer;
	// initialize settings. Most of these have been picked to match initial settings set in the "initialize" 
	// function.
	AndorRunSettings& andorSettings = settings.andor;
	andorSettings.exposureTimes = { 0.026f };
	andorSettings.picsPerRepetition = 1;
	andorSettings.kineticCycleTime = 0.1f;
	andorSettings.repetitionsPerVariation = 10;
	andorSettings.totalVariations = 3;
	//andorSettings.totalPicsInExperiment = 30;
	//andorSettings.totalPicsInVariation = 10;
	// the read mode never gets changed currently. we always want images.
	andorSettings.readMode = 4;
	andorSettings.acquisitionMode = AndorRunModes::Kinetic;
	andorSettings.emGainModeIsOn = false;
	andorSettings.showPicsInRealTime = false;
	andorSettings.triggerMode = AndorTriggerMode::External;
}


void AndorCameraSettingsControl::initialize( cameraPositions& pos, int& id, CWnd* parent, cToolTips& tooltips )
{
	/// Header
	header.setPositions ( pos, 0, 0, 480, 25, true, false, true );
	header.Create( "CAMERA SETTINGS", NORM_HEADER_OPTIONS, header.seriesPos, parent, id++ );
	header.fontType = fontTypes::HeadingFont;

	/// camera mode
	cameraModeCombo.setPositions ( pos, 0, 0, 480, 100, false, false, true );
	cameraModeCombo.Create( NORM_COMBO_OPTIONS, cameraModeCombo.seriesPos, parent, IDC_CAMERA_MODE_COMBO );
	cameraModeCombo.AddString( "Kinetic-Series-Mode" );
	cameraModeCombo.AddString( "Accumulation Mode" );
	cameraModeCombo.AddString( "Video Mode" );
	cameraModeCombo.SelectString( 0, "Kinetic-Series-Mode" );
	//settings.andor.cameraMode = "Kinetic-Series-Mode";
	settings.andor.acquisitionMode = AndorRunModes::Kinetic;
	pos.amPos.y += 25;
	pos.videoPos.y += 25;
	pos.seriesPos.y += 25;
	/// EM Gain
	emGainEdit.setPositions ( pos, 0, 0, 240, 20, false, false, true );
	emGainEdit.Create( NORM_EDIT_OPTIONS, emGainEdit.seriesPos, parent, IDC_EM_GAIN_EDIT );
	emGainEdit.setToolTip( "Set the state & gain of the EM gain of the camera. Enter a negative number to turn EM Gain"
						   " mode off. The program will immediately change the state of the camera after changing this"
						   " edit.", tooltips, parent );
	//
	emGainDisplay.setPositions ( pos, 240, 0, 240, 20, true, false, true );
	emGainDisplay.Create( "OFF", NORM_STATIC_OPTIONS, emGainDisplay.seriesPos, parent, id++ );
	// initialize settings.
	settings.andor.emGainLevel = 0;
	settings.andor.emGainModeIsOn = false;
	// trigger combo
	triggerCombo.setPositions ( pos, 0, 0, 480, 800, false, false, true );
	triggerCombo.Create( NORM_COMBO_OPTIONS, triggerCombo.seriesPos, parent, IDC_TRIGGER_COMBO );
	triggerCombo.AddString( "Internal-Trigger" );
	triggerCombo.AddString( "External-Trigger" );
	triggerCombo.AddString( "Start-On-Trigger" );
	// Select default trigger
	triggerCombo.SelectString( 0, "External-Trigger" );
	pos.seriesPos.y += 25;
	pos.amPos.y += 25;
	pos.videoPos.y += 25;
	settings.andor.triggerMode = AndorTriggerMode::External;

	setTemperatureButton.setPositions ( pos, 0, 0, 270, 25, false, false, true );
	setTemperatureButton.Create( "Set Camera Temperature (C)", NORM_PUSH_OPTIONS, setTemperatureButton.seriesPos,
								 parent, IDC_SET_TEMPERATURE_BUTTON );
	temperatureEdit.setPositions ( pos, 270, 0, 80, 25, false, false, true );
	temperatureEdit.Create( NORM_EDIT_OPTIONS, temperatureEdit.seriesPos, parent, id++ );
	temperatureEdit.SetWindowTextA( "0" );
	temperatureDisplay.setPositions ( pos, 350, 0, 80, 25, false, false, true );
	temperatureDisplay.Create( "", NORM_STATIC_OPTIONS, temperatureDisplay.seriesPos, parent, id++ );
	temperatureOffButton.setPositions ( pos, 430, 0, 50, 25, true, false, true );
	temperatureOffButton.Create( "OFF", NORM_PUSH_OPTIONS, temperatureOffButton.seriesPos, parent, id++ );
	temperatureMsg.setPositions ( pos, 0, 0, 480, 50, true, false, true );
	temperatureMsg.Create( "Temperature control is disabled", NORM_STATIC_OPTIONS, temperatureMsg.seriesPos, parent, id++ );
	//
	picSettingsObj.initialize( pos, parent, id );

	imageDimensionsObj.initialize( pos, parent, false, id );

	/// REPETITIONS PER VARIATION

	// Accumulation Time
	accumulationCycleTimeLabel.videoPos = accumulationCycleTimeLabel.seriesPos = { -1,-1,-1,-1 };
	accumulationCycleTimeLabel.amPos = { pos.amPos.x,pos.amPos.y,pos.amPos.x + 240,pos.amPos.y + 25 };
	accumulationCycleTimeLabel.Create( "Accumulation Cycle Time", NORM_STATIC_OPTIONS,
									   accumulationCycleTimeLabel.seriesPos, parent, id++ );

	accumulationCycleTimeEdit.seriesPos  = accumulationCycleTimeEdit.videoPos = { -1,-1,-1,-1 };
	accumulationCycleTimeEdit.amPos = { pos.amPos.x + 240,pos.amPos.y,pos.amPos.x + 480, pos.amPos.y += 25 };
	accumulationCycleTimeEdit.Create( NORM_EDIT_OPTIONS, accumulationCycleTimeEdit.seriesPos, parent, id++ );
	accumulationCycleTimeEdit.SetWindowTextA( "0.1" );

	// Accumulation Number
	accumulationNumberLabel.seriesPos = accumulationNumberLabel.videoPos = { -1,-1,-1,-1 };
	accumulationNumberLabel.amPos = { pos.amPos.x,pos.amPos.y,pos.amPos.x + 240,pos.amPos.y + 25 };
	accumulationNumberLabel.Create( "Accumulation #", NORM_STATIC_OPTIONS, accumulationNumberLabel.seriesPos, parent, id++ );
	//
	accumulationNumberEdit.seriesPos = accumulationNumberEdit.videoPos = { -1,-1,-1,-1 };
	accumulationNumberEdit.amPos = { pos.amPos.x + 240,pos.amPos.y,pos.amPos.x + 480,pos.amPos.y += 25 };
	accumulationNumberEdit.Create( NORM_EDIT_OPTIONS, accumulationNumberEdit.seriesPos, parent, id++ );
	accumulationNumberEdit.SetWindowTextA( "1" );

	// minimum kinetic cycle time (determined by camera)
	minKineticCycleTimeLabel.setPositions ( pos, 0, 0, 240, 25, false, false, false, true );
	minKineticCycleTimeLabel.Create( "Minimum Kinetic Cycle Time (s)", NORM_STATIC_OPTIONS, 
									 minKineticCycleTimeLabel.seriesPos, parent, id++ );
	
	minKineticCycleTimeLabel.setPositions ( pos, 240, 0, 240, 25, true, false, false, true );
	minKineticCycleTimeDisp.Create( NORM_STATIC_OPTIONS, minKineticCycleTimeDisp.seriesPos, parent, id++ );
	minKineticCycleTimeDisp.SetWindowTextA( "" );

	/// Kinetic Cycle Time
	// Kinetic Cycle Time Label
	kineticCycleTimeLabel.setPositions ( pos, 0, 0, 240, 25, false, false, false, true );
	kineticCycleTimeLabel.triggerModeSensitive = -1;
	kineticCycleTimeLabel.Create( "Kinetic Cycle Time", NORM_STATIC_OPTIONS, kineticCycleTimeLabel.seriesPos, parent, id++ );

	// Kinetic Cycle Time Edit
	kineticCycleTimeEdit.setPositions ( pos, 240, 0, 240, 25, true, false, false, true );
	kineticCycleTimeEdit.triggerModeSensitive = -1;
	kineticCycleTimeEdit.Create( NORM_EDIT_OPTIONS, kineticCycleTimeEdit.seriesPos, parent, id++ );
	kineticCycleTimeEdit.SetWindowTextA( "0.1" );
	//
	calControl.initialize( pos, id, parent, tooltips );
}



// note that this object doesn't actually store the camera state, it just uses it in passing to figure out whether 
// buttons should be on or off.
void AndorCameraSettingsControl::cameraIsOn(bool state)
{
	// Can't change em gain mode or camera settings once started.
	emGainEdit.EnableWindow( !state );
	setTemperatureButton.EnableWindow( !state );
	temperatureOffButton.EnableWindow( !state );
}


void AndorCameraSettingsControl::setRunSettings(AndorRunSettings inputSettings)
{
	if (inputSettings.emGainModeIsOn == false || inputSettings.emGainLevel < 0)
	{
		emGainEdit.SetWindowTextA("-1");
		emGainDisplay.SetWindowTextA("OFF");
	}
	else
	{
		emGainEdit.SetWindowTextA(cstr(inputSettings.emGainLevel));
		emGainDisplay.SetWindowTextA(cstr("X" + str(inputSettings.emGainLevel)));
	}
	andorFriend->setGainMode();
	// try to set this time.
	picSettingsObj.setExposureTimes(inputSettings.exposureTimes, andorFriend);
	// now check actual times.
	checkTimings(inputSettings.exposureTimes);
	///
	kineticCycleTimeEdit.SetWindowTextA(cstr(inputSettings.kineticCycleTime));
	accumulationCycleTimeEdit.SetWindowTextA(cstr(inputSettings.accumulationTime));
	cameraModeCombo.SelectString ( 0, AndorRunModeText ( inputSettings.acquisitionMode ).c_str() );// cstr ( inputSettings.cameraMode ));
	if ( inputSettings.acquisitionMode == AndorRunModes::Video )
	{
		inputSettings.repetitionsPerVariation = INT_MAX;
	}
	else if ( inputSettings.acquisitionMode == AndorRunModes::Kinetic )
	{
		
	}
	else if ( inputSettings.acquisitionMode == AndorRunModes::Accumulate )
	{
		inputSettings.repetitionsPerVariation = INT_MAX;
	}
	else
	{
		thrower ( "ERROR: unrecognized camera mode: " + AndorRunModeText(inputSettings.acquisitionMode) );
	}
	/*
	if (inputSettings.cameraMode == "Video-Mode")
	{
		inputSettings.acquisitionMode = AndorRunModes::Video;
		inputSettings.totalPicsInVariation = INT_MAX;
	}
	else if (inputSettings.cameraMode == "Kinetic-Series-Mode")
	{
		inputSettings.acquisitionMode = AndorRunModes::Kinetic;
	}
	else if ( inputSettings.cameraMode == "Accumulate-Mode" )
	{
		inputSettings.acquisitionMode = AndorRunModes::Accumulate;
		inputSettings.totalPicsInVariation = INT_MAX;
	}
	else
	{
		thrower ( "ERROR: unrecognized camera mode: " + inputSettings.cameraMode );
	}
	*/
	kineticCycleTimeEdit.SetWindowTextA(cstr(inputSettings.kineticCycleTime));
	accumulationCycleTimeEdit.SetWindowTextA(cstr(inputSettings.accumulationTime * 1000.0));
	accumulationNumberEdit.SetWindowTextA(cstr(inputSettings.accumulationNumber));
	temperatureEdit.SetWindowTextA(cstr(inputSettings.temperatureSetting));
}


void AndorCameraSettingsControl::handleSetTemperatureOffPress()
{
	andorFriend->changeTemperatureSetting(true);
}


void AndorCameraSettingsControl::handleSetTemperaturePress()
{
	if (andorFriend->isRunning())
	{
		thrower ( "ERROR: the camera (thinks that it?) is running. You can't change temperature settings during camera "
				 "operation." );
	}
	CString text;
	temperatureEdit.GetWindowTextA(text);
	int temp;
	try
	{
		temp = boost::lexical_cast<int>(str(text));
	}
	catch ( boost::bad_lexical_cast&)
	{
		throwNested("Error: Couldn't convert temperature input to a double! Check for unusual characters.");
	}
	settings.andor.temperatureSetting = temp;
	andorFriend->setSettings(settings.andor );

	andorFriend->setTemperature();
}


void AndorCameraSettingsControl::updateTriggerMode( )
{
	CString triggerMode;
	int itemIndex = triggerCombo.GetCurSel( );
	if ( itemIndex == -1 )
	{
		return;
	}
	triggerCombo.GetLBText( itemIndex, triggerMode );
	settings.andor.triggerMode = AndorTriggerModeFromText(std::string(triggerMode));
}


void AndorCameraSettingsControl::handleTriggerChange(AndorWindow* cameraWindow)
{
	updateTriggerMode( );
	CRect rect;
	cameraWindow->GetWindowRect(&rect);
	cameraWindow->OnSize(0, rect.right - rect.left, rect.bottom - rect.top);
}


void AndorCameraSettingsControl::updateSettings()
{
	// update all settings with current values from controls
	settings.andor.exposureTimes =		picSettingsObj.getUsedExposureTimes( );
	settings.thresholds =				picSettingsObj.getThresholds( );
	settings.palleteNumbers =			picSettingsObj.getPictureColors( );
	settings.andor.picsPerRepetition =	picSettingsObj.getPicsPerRepetition( );
	
	settings.andor.imageSettings = getImageParameters( );
	settings.andor.kineticCycleTime = getKineticCycleTime( );
	settings.andor.accumulationTime = getAccumulationCycleTime( );
	settings.andor.accumulationNumber = getAccumulationNumber( );

	setEmGain( );
	updateCameraMode( );
	updateTriggerMode( );
}


AndorCameraSettings AndorCameraSettingsControl::getSettings()
{
	updateSettings( );
	return settings;
}


AndorCameraSettings AndorCameraSettingsControl::getCalibrationSettings( )
{
	AndorCameraSettings calSettings;
	calSettings.andor.acquisitionMode = AndorRunModes::Kinetic;
	//calSettings.andor.cameraMode = "Kinetic-Series-Mode";
	calSettings.andor.emGainLevel = 0;
	calSettings.andor.emGainModeIsOn = false;
	calSettings.andor.exposureTimes = { float(10e-3) };
	// want to calibrate the image area to be used in the experiment, so...
	calSettings.andor.imageSettings = imageDimensionsObj.getImageParameters( );
	calSettings.andor.kineticCycleTime = 10e-3;
	calSettings.andor.picsPerRepetition = 1;
	calSettings.andor.readMode = 4;
	calSettings.andor.repetitionsPerVariation = 100;
	calSettings.andor.showPicsInRealTime = false;
	calSettings.andor.temperatureSetting = -60;
	calSettings.andor.totalVariations = 1;
	calSettings.andor.triggerMode = AndorTriggerMode::External;
	return calSettings;
}


bool AndorCameraSettingsControl::getAutoCal( )
{
	return calControl.autoCal( );
}


bool AndorCameraSettingsControl::getUseCal( )
{
	return calControl.use( );
}


void AndorCameraSettingsControl::rearrange( AndorRunModes cameraMode, AndorTriggerMode triggerMode, int width, int height, fontMap fonts )
{
	imageDimensionsObj.rearrange( cameraMode, triggerMode, width, height, fonts );
	picSettingsObj.rearrange( cameraMode, triggerMode, width, height, fonts );
	header.rearrange( cameraMode, triggerMode, width, height, fonts );
	cameraModeCombo.rearrange( cameraMode, triggerMode, width, height, fonts );
	emGainDisplay.rearrange( cameraMode, triggerMode, width, height, fonts );
	emGainEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	triggerCombo.rearrange( cameraMode, triggerMode, width, height, fonts );
	setTemperatureButton.rearrange( cameraMode, triggerMode, width, height, fonts );
	temperatureOffButton.rearrange( cameraMode, triggerMode, width, height, fonts );
	temperatureEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	temperatureDisplay.rearrange( cameraMode, triggerMode, width, height, fonts );
	temperatureMsg.rearrange( cameraMode, triggerMode, width, height, fonts );
	kineticCycleTimeEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	kineticCycleTimeLabel.rearrange( cameraMode, triggerMode, width, height, fonts );
	accumulationCycleTimeEdit.rearrange(cameraMode, triggerMode, width, height, fonts);
	accumulationCycleTimeLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	accumulationNumberEdit.rearrange(cameraMode, triggerMode, width, height, fonts);
	accumulationNumberLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	minKineticCycleTimeLabel.rearrange( cameraMode, triggerMode, width, height, fonts );
	minKineticCycleTimeDisp.rearrange( cameraMode, triggerMode, width, height, fonts );
	calControl.rearrange( cameraMode, triggerMode, width, height, fonts );
}


void AndorCameraSettingsControl::setEmGain()
{
	CString emGainText;
	emGainEdit.GetWindowTextA(emGainText);
	if ( emGainText == "" )
	{
		// set to off.
		emGainText = "-1";
	}
	int emGain;
	try
	{
		emGain = boost::lexical_cast<int>(str(emGainText));
	}
	catch ( boost::bad_lexical_cast&)
	{
		throwNested("ERROR: Couldn't convert EM Gain text to integer! Aborting!");
	}
	// < 0 corresponds to NOT USING EM GAIN (using conventional gain).
	if (emGain < 0)
	{
		settings.andor.emGainModeIsOn = false;
		settings.andor.emGainLevel = 0;
		emGainDisplay.SetWindowTextA("OFF");
	}
	else
	{
		settings.andor.emGainModeIsOn = true;
		settings.andor.emGainLevel = emGain;
		emGainDisplay.SetWindowTextA(cstr("Gain: X" + str(settings.andor.emGainLevel)));
	}
	// Change the andor settings.
	AndorRunSettings andorSettings = andorFriend->getAndorSettings();
	std::string promptMsg = "";
	if ( andorSettings.emGainModeIsOn != settings.andor.emGainModeIsOn )
	{
		promptMsg += "Set Andor EM Gain State to ";
		if ( settings.andor.emGainModeIsOn )
		{
			promptMsg += "ON";
		}
		else
		{
			promptMsg += "OFF";
		}
	}
	if ( andorSettings.emGainLevel != settings.andor.emGainLevel )
	{
		if ( promptMsg != "" )
		{
			promptMsg += ", ";
		}
		promptMsg += "Set Andor EM Gain Level to " + str(settings.andor.emGainLevel);
	}
	if ( promptMsg != "" )
	{
		promptMsg += "?";
		int result = promptBox( promptMsg, MB_YESNO );
		if ( result == IDNO )
		{
			thrower ( "Aborting camera settings update at EM Gain update!" );
		}
	}
	andorSettings.emGainLevel = settings.andor.emGainLevel;
	andorSettings.emGainModeIsOn = settings.andor.emGainModeIsOn;
	andorFriend->setSettings( andorSettings );
	// and immediately change the EM gain mode.
	try
	{
		andorFriend->setGainMode( );
	}
	catch ( Error& err )
	{
		// this can happen e.g. if the camera is aquiring.
		errBox( err.trace( ) );
	}
	emGainEdit.RedrawWindow();
}


void AndorCameraSettingsControl::setVariationNumber(UINT varNumber)
{
	AndorRunSettings& andorSettings = settings.andor;
	andorSettings.totalVariations = varNumber;
	if ( andorSettings.totalPicsInExperiment() > INT_MAX)
	{
		thrower ( "ERROR: Trying to take too many pictures! Maximum picture number is " + str( INT_MAX ) );
	}
}


void AndorCameraSettingsControl::setRepsPerVariation(UINT repsPerVar)
{
	AndorRunSettings& andorSettings = settings.andor;
	andorSettings.repetitionsPerVariation = repsPerVar;
	if ( andorSettings.totalPicsInExperiment() > INT_MAX)
	{
		thrower ( "ERROR: Trying to take too many pictures! Maximum picture number is " + str( INT_MAX ) );
	}
}


void AndorCameraSettingsControl::handleTimer()
{
	// This case displays the current temperature in the main window. When the temp stabilizes at the desired 
	// level the appropriate message is displayed.
	// initial value is only relevant for safemode.
	int currentTemperature = INT_MAX;
	int setTemperature = INT_MAX;
	try
	{
		// in this case you expect it to throw.
		setTemperature = andorFriend->getAndorSettings().temperatureSetting;
		andorFriend->getTemperature(currentTemperature);
		if ( ANDOR_SAFEMODE ) { thrower ( "SAFEMODE" ); }
	}
	catch (Error& exception)
	{
		// if not stable this won't get changed.
		if (exception.whatBare() == "DRV_TEMPERATURE_STABILIZED")
		{
			temperatureDisplay.SetWindowTextA(cstr(setTemperature));
			temperatureMsg.SetWindowTextA(cstr("Temperature has stabilized at " + str(currentTemperature) 
											  + " (C)\r\n"));
		}
		else if (exception.whatBare() == "DRV_TEMPERATURE_NOT_REACHED")
		{
			temperatureDisplay.SetWindowTextA(cstr(setTemperature));
			temperatureMsg.SetWindowTextA(cstr("Set temperature not yet reached. Current temperature is " 
											  + str(currentTemperature) + " (C)\r\n"));
		}
		else if (exception.whatBare() == "DRV_TEMPERATURE_NOT_STABILIZED")
		{
			temperatureDisplay.SetWindowTextA(cstr(setTemperature));
			temperatureMsg.SetWindowTextA(cstr("Temperature of " + str(currentTemperature) 
											  + " (C) reached but not stable."));
		}
		else if (exception.whatBare() == "DRV_TEMPERATURE_DRIFT")
		{
			temperatureDisplay.SetWindowTextA(cstr(setTemperature));
			temperatureMsg.SetWindowTextA(cstr("Temperature had stabilized but has since drifted. Temperature: " 
											  + str(currentTemperature)));
		}
		else if (exception.whatBare() == "DRV_TEMPERATURE_OFF")
		{
			temperatureDisplay.SetWindowTextA(cstr(setTemperature));
			temperatureMsg.SetWindowTextA(cstr("Temperature control is off. Temperature: " + str(currentTemperature)));
		}
		else if (exception.whatBare() == "DRV_ACQUIRING")
		{
			// doesn't change color of temperature control. This way the color of the control represents the state of
			// the temperature right before the acquisition started, so that you can tell if you remembered to let it
			// completely stabilize or not.
			temperatureDisplay.SetWindowTextA(cstr(setTemperature));
			temperatureMsg.SetWindowTextA("Camera is Acquiring data. No Temperature updates are available.");
		}
		else if (exception.whatBare() == "SAFEMODE")
		{
			temperatureDisplay.SetWindowTextA(cstr(setTemperature));
			temperatureMsg.SetWindowTextA("Application is running in Safemode... No Real Temperature Data is available.");
		}
		else
		{
			temperatureDisplay.SetWindowTextA(cstr(currentTemperature));
			temperatureMsg.SetWindowTextA(cstr("Unexpected Temperature Code: " + exception.whatBare() + ". Temperature: " 
												+ str(currentTemperature)));
		}
	}
}


void AndorCameraSettingsControl::updateRunSettingsFromPicSettings( )
{
	settings.andor.exposureTimes = picSettingsObj.getUsedExposureTimes( );
	settings.andor.picsPerRepetition = picSettingsObj.getPicsPerRepetition( );
	if ( settings.andor.totalPicsInExperiment ( ) > INT_MAX )
	{
		thrower ( "ERROR: Trying to take too many pictures! Maximum picture number is " + str( INT_MAX ) );
	}
}


void AndorCameraSettingsControl::handlePictureSettings(UINT id, AndorCamera* andorObj)
{
	picSettingsObj.handleOptionChange(id, andorObj);
	updateRunSettingsFromPicSettings( );
}


double AndorCameraSettingsControl::getKineticCycleTime( )
{
	CString text;
	kineticCycleTimeEdit.GetWindowTextA( text );
	try
	{
		settings.andor.kineticCycleTime = boost::lexical_cast<float>( str( text ) );
		kineticCycleTimeEdit.SetWindowTextA( cstr( settings.andor.kineticCycleTime ) );
	}
	catch ( boost::bad_lexical_cast& )
	{
		settings.andor.kineticCycleTime = 0.1f;
		kineticCycleTimeEdit.SetWindowTextA( cstr( settings.andor.kineticCycleTime ) );
		throwNested( "Please enter a valid float for the kinetic cycle time." );
	}
	return settings.andor.kineticCycleTime;
}


double AndorCameraSettingsControl::getAccumulationCycleTime( )
{
	CString text;
	accumulationCycleTimeEdit.GetWindowTextA( text );
	try
	{
		settings.andor.accumulationTime = boost::lexical_cast<float>( str( text ) );
		accumulationCycleTimeEdit.SetWindowTextA( cstr( settings.andor.accumulationTime ) );
	}
	catch ( boost::bad_lexical_cast& )
	{
		settings.andor.accumulationTime = 0.1f;
		accumulationCycleTimeEdit.SetWindowTextA( cstr( settings.andor.accumulationTime ) );
		throwNested( "Please enter a valid float for the accumulation cycle time." );
	}
	return settings.andor.accumulationTime;
}


UINT AndorCameraSettingsControl::getAccumulationNumber( )
{
	CString text;
	accumulationNumberEdit.GetWindowTextA( text );
	try
	{
		settings.andor.accumulationNumber = boost::lexical_cast<long>( str( text ) );
		accumulationNumberEdit.SetWindowTextA( cstr( settings.andor.accumulationNumber ) );
	}
	catch ( boost::bad_lexical_cast& )
	{
		settings.andor.accumulationNumber = 1;
		accumulationNumberEdit.SetWindowTextA( cstr( settings.andor.accumulationNumber ) );
		throwNested( "Please enter a valid float for the Accumulation number." );
	}
	return settings.andor.accumulationNumber;
}


void AndorCameraSettingsControl::handleOpenConfig(std::ifstream& configFile, Version ver)
{
	AndorRunSettings tempSettings;
	configFile.get( );
	std::string txt;
	std::getline(configFile, txt);
	tempSettings.triggerMode = AndorTriggerModeFromText(txt);
	configFile >> tempSettings.emGainModeIsOn;
	configFile >> tempSettings.emGainLevel;
	configFile.get();
	std::getline ( configFile, txt );
	if (txt == AndorRunModeText(AndorRunModes::Video) || txt == "Video Mode" )
	{
		tempSettings.acquisitionMode = AndorRunModes::Video;
		tempSettings.repetitionsPerVariation = INT_MAX;
	}
	else if ( txt == AndorRunModeText ( AndorRunModes::Kinetic ) || txt == "Kinetic Series Mode" )
	{
		tempSettings.acquisitionMode = AndorRunModes::Kinetic;
		//settings.andor.repetitionsPerVariation = settings.andor.totalPicsInVariation / settings.andor.picsPerRepetition;
	}
	else if ( txt == AndorRunModeText ( AndorRunModes::Accumulate ) || txt == "Accumulate Mode" )
	{
		tempSettings.acquisitionMode = AndorRunModes::Accumulate;
		//tempSettings.totalPicsInVariation = INT_MAX;
	}
	else
	{
		thrower ("ERROR: Unrecognized camera mode!");
	}
	updateCameraMode ( );
	configFile >> tempSettings.kineticCycleTime;
	configFile >> tempSettings.accumulationTime;
	configFile >> tempSettings.accumulationNumber;
	configFile >> tempSettings.temperatureSetting;
 	setRunSettings(tempSettings);
 	ProfileSystem::checkDelimiterLine(configFile, "END_CAMERA_SETTINGS");
	
	picSettingsObj.handleOpenConfig(configFile, ver, andorFriend);
	
	updateRunSettingsFromPicSettings( );
	if (ver > Version("2.4" ) )
	{
		imageDimensionsObj.handleOpen( configFile, ver );
	}
}


void AndorCameraSettingsControl::handleNewConfig( std::ofstream& newFile )
{
	newFile << "CAMERA_SETTINGS\n";
	newFile << "External-Trigger" << "\n";
	newFile << 0 << "\n";
	newFile << 0 << "\n";
	newFile << "Kinetic-Series-Mode" << "\n";
	newFile << 1000 << "\n";
	newFile << 1000 << "\n";
	newFile << 2 << "\n";
	newFile << 25 << "\n";
	newFile << "END_CAMERA_SETTINGS\n";
	picSettingsObj.handleNewConfig( newFile );
	imageDimensionsObj.handleNew( newFile );
}


void AndorCameraSettingsControl::handleSaveConfig(std::ofstream& saveFile)
{
	saveFile << "CAMERA_SETTINGS\n";
	saveFile << AndorTriggerModeText(settings.andor.triggerMode) << "\n";
	saveFile << settings.andor.emGainModeIsOn << "\n";
	saveFile << settings.andor.emGainLevel << "\n";
	saveFile << AndorRunModeText(settings.andor.acquisitionMode) << "\n";
	saveFile << settings.andor.kineticCycleTime << "\n";
	saveFile << settings.andor.accumulationTime << "\n";
	saveFile << settings.andor.accumulationNumber << "\n";
	saveFile << settings.andor.temperatureSetting << "\n";
	saveFile << "END_CAMERA_SETTINGS\n";

	picSettingsObj.handleSaveConfig(saveFile);
	imageDimensionsObj.handleSave( saveFile );
}


void AndorCameraSettingsControl::updateCameraMode( )
{
	/* 
		updates settings.andor.cameraMode based on combo selection, then updates 
		settings.andor.acquisitionMode and other settings depending on the mode.
	*/
	int sel = cameraModeCombo.GetCurSel( );
	if ( sel == -1 )
	{
		return;
	}
	CString mode;
	cameraModeCombo.GetLBText( sel, mode );
	std::string txt ( mode );
	//settings.andor.cameraMode = mode;
	if ( txt == AndorRunModeText(AndorRunModes::Video) || txt == "Video Mode" )
	{
		settings.andor.acquisitionMode = AndorRunModes::Video;
		settings.andor.repetitionsPerVariation = INT_MAX;
	}
	else if ( txt == AndorRunModeText ( AndorRunModes::Kinetic ) || txt == "Kinetic Series Mode" )
	{
		settings.andor.acquisitionMode = AndorRunModes::Kinetic;
	}
	else if ( txt == AndorRunModeText ( AndorRunModes::Accumulate ) || txt == "Accumulate Mode" )
	{
		settings.andor.acquisitionMode = AndorRunModes::Accumulate;
	}
	else
	{
		thrower  ( "ERROR: unrecognized combo for andor run mode text???" );
	}
}


void AndorCameraSettingsControl::handleModeChange( AndorWindow* cameraWindow )
{
	updateCameraMode( );

	CRect rect;
	cameraWindow->GetWindowRect( &rect );
	cameraWindow->OnSize( 0, rect.right - rect.left, rect.bottom - rect.top );
}


void AndorCameraSettingsControl::checkTimings(std::vector<float>& exposureTimes)
{
	checkTimings(settings.andor.kineticCycleTime, settings.andor.accumulationTime, exposureTimes);
}


void AndorCameraSettingsControl::checkTimings(float& kineticCycleTime, float& accumulationTime, std::vector<float>& exposureTimes)
{
	andorFriend->checkAcquisitionTimings(kineticCycleTime, accumulationTime, exposureTimes);
}


void AndorCameraSettingsControl::updateMinKineticCycleTime( double time )
{
	minKineticCycleTimeDisp.SetWindowTextA( cstr( time ) );
}


imageParameters AndorCameraSettingsControl::getImageParameters()
{
	return imageDimensionsObj.readImageParameters( );
}


CBrush* AndorCameraSettingsControl::handleColor( int idNumber, CDC* colorer )
{
	return picSettingsObj.colorControls( idNumber, colorer );
}


void AndorCameraSettingsControl::setImageParameters(imageParameters newSettings, AndorWindow* camWin)
{
	imageDimensionsObj.setImageParametersFromInput(newSettings, camWin);
}


void AndorCameraSettingsControl::checkIfReady()
{
	if ( picSettingsObj.getUsedExposureTimes().size() == 0 )
	{
		thrower ("Please Set at least one exposure time.");
	}
	if ( !imageDimensionsObj.checkReady() )
	{
		thrower ("Please set the image parameters.");
	}
	if ( settings.andor.picsPerRepetition <= 0 )
	{
		thrower ("ERROR: Please set the number of pictures per repetition to a positive non-zero value.");
	}
	if ( settings.andor.acquisitionMode == AndorRunModes::Kinetic )
	{
		if ( settings.andor.kineticCycleTime == 0 && settings.andor.triggerMode == AndorTriggerMode::Internal )
		{
			thrower ("ERROR: Since you are running in internal trigger mode, please Set a kinetic cycle time.");
		}
		if ( settings.andor.repetitionsPerVariation <= 0 )
		{
			thrower ("ERROR: Please set the \"Repetitions Per Variation\" variable to a positive non-zero value.");
		}
		if ( settings.andor.totalVariations <= 0 )
		{
			thrower ("ERROR: Please set the number of variations to a positive non-zero value.");
		}
	}
	if ( settings.andor.acquisitionMode == AndorRunModes::Accumulate )
	{
		if ( settings.andor.accumulationNumber <= 0 )
		{
			thrower ("ERROR: Please set the current Accumulation Number to a positive non-zero value.");
		}
		if ( settings.andor.accumulationTime <= 0 )
		{
			thrower ("ERROR: Please set the current Accumulation Time to a positive non-zero value.");
		}
	}
}


void AndorCameraSettingsControl::handelSaveMasterConfig ( std::stringstream& configFile )
{
	imageParameters settings = getSettings ( ).andor.imageSettings;
	configFile << settings.left << " " << settings.right << " " << settings.horizontalBinning << " ";
	configFile << settings.bottom << " " << settings.top << " " << settings.verticalBinning << "\n";
	// introduced in version 2.2
	configFile << getAutoCal ( ) << " " << getUseCal ( ) << "\n";
}


void AndorCameraSettingsControl::handleOpenMasterConfig ( std::stringstream& configStream, Version ver, AndorWindow* camWin )
{
	imageParameters settings = getSettings ( ).andor.imageSettings;
	std::string tempStr;
	try
	{
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
		setImageParameters ( settings, camWin );
	}
	catch ( boost::bad_lexical_cast& )
	{
		throwNested ( "ERROR: Bad value (i.e. failed to convert to long) seen in master configueration file while attempting "
				  "to load camera dimensions!" );
	}

	bool autoCal, useCal;
	if ( ver > Version ( "2.1" ) )
	{
		configStream >> autoCal >> useCal;
		calControl.setAutoCal ( autoCal );
		calControl.setUse ( useCal );
	}

}


std::vector<std::vector<long>> AndorCameraSettingsControl::getImagesToDraw ( const std::vector<std::vector<long>>& rawData )
{
	std::vector<std::vector<long>> imagesToDraw ( rawData.size ( ) );
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
				imagesToDraw[ picNum ].resize ( rawData[ picNum ].size ( ) );
				for ( auto i : range ( rawData[ picNum ].size ( ) ) )
				{
					imagesToDraw[ picNum ][ i ] = rawData[ picNum ][ i ] - rawData[ options[ picNum ].whichPicForDif - 1 ][ i ];
				}
			}
		}
	}
	return imagesToDraw;
}

