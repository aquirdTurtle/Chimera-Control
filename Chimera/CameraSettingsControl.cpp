#include "stdafx.h"
#include "CameraSettingsControl.h"
#include "CameraWindow.h"
#include "miscellaneousCommonFunctions.h"


CameraSettingsControl::CameraSettingsControl(AndorCamera* friendInitializer) : picSettingsObj(this)
{
	andorFriend = friendInitializer;
	// initialize settings. Most of these have been picked to match initial settings set in the "initialize" 
	// function.
	runSettings.exposureTimes = { 0.026f };
	runSettings.picsPerRepetition = 1;
	runSettings.kineticCycleTime = 0.1f;
	runSettings.repetitionsPerVariation = 10;
	runSettings.totalVariations = 3;
	runSettings.totalPicsInExperiment = 30;
	runSettings.totalPicsInVariation = 10;
	// the read mode never gets changed currently. we always want images.
	runSettings.readMode = 4;
	runSettings.acquisitionMode = 3;
	runSettings.emGainModeIsOn = false;
	runSettings.showPicsInRealTime = false;
	runSettings.triggerMode = "External Trigger";
}


// note that this object doesn't actually store the camera state, it just uses it in passing to figure out whether 
// buttons should be on or off.
void CameraSettingsControl::cameraIsOn(bool state)
{
	// Can't change em gain mode or camera settings once started.
	emGainButton.EnableWindow( !state );
	setTemperatureButton.EnableWindow( !state );
	temperatureOffButton.EnableWindow( !state );
}


std::array<int, 4> CameraSettingsControl::getThresholds()
{
	return picSettingsObj.getThresholds();
}

void CameraSettingsControl::setRunSettings(AndorRunSettings inputSettings)
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
	cameraModeCombo.SelectString(0, cstr(inputSettings.cameraMode));
	if (inputSettings.cameraMode == "Continuous Single Scans Mode")
	{
		inputSettings.acquisitionMode = 5;
		inputSettings.totalPicsInVariation = INT_MAX;
	}
	else if (inputSettings.cameraMode == "Kinetic Series Mode")
	{
		inputSettings.acquisitionMode = 3;
	}
	else if (inputSettings.cameraMode == "Accumulate Mode")
	{
		inputSettings.acquisitionMode = 2;
		inputSettings.totalPicsInVariation = INT_MAX;
	}
	kineticCycleTimeEdit.SetWindowTextA(cstr(inputSettings.kineticCycleTime));
	accumulationCycleTimeEdit.SetWindowTextA(cstr(inputSettings.accumulationTime * 1000.0));
	accumulationNumberEdit.SetWindowTextA(cstr(inputSettings.accumulationNumber));
	temperatureEdit.SetWindowTextA(cstr(inputSettings.temperatureSetting));
}


void CameraSettingsControl::handleSetTemperatureOffPress()
{
	andorFriend->changeTemperatureSetting(true);
}


void CameraSettingsControl::handleSetTemperaturePress()
{
	if (andorFriend->isRunning())
	{
		thrower( "ERROR: the camera (thinks that it?) is running. You can't change temperature settings during camera "
				 "operation." );
	}
	
	//runSettings = andorFriend->getSettings();
	CString text;
	temperatureEdit.GetWindowTextA(text);
	int temp;
	try
	{
		temp = std::stoi(str(text));
	}
	catch (std::invalid_argument&)
	{
		thrower("Error: Couldn't convert temperature input to a double! Check for unusual characters.");
	}
	runSettings.temperatureSetting = temp;
	andorFriend->setSettings(runSettings);

	andorFriend->setTemperature();
	//eCameraFileSystem.updateSaveStatus(false);
}


void CameraSettingsControl::handleTriggerControl(CameraWindow* cameraWindow)
{
	CString triggerMode;
	int itemIndex = triggerCombo.GetCurSel();
	if (itemIndex == -1)
	{
		return;
	}
	triggerCombo.GetLBText(itemIndex, triggerMode);
	runSettings.triggerMode = triggerMode;
	CRect rect;
	cameraWindow->GetWindowRect(&rect);
	cameraWindow->OnSize(0, rect.right - rect.left, rect.bottom - rect.top);
}

AndorRunSettings CameraSettingsControl::getSettings()
{
	return runSettings;
}

void CameraSettingsControl::rearrange( std::string cameraMode, std::string triggerMode, int width, int height, fontMap fonts )
{
	imageDimensionsObj.rearrange( cameraMode, triggerMode, width, height, fonts );
	picSettingsObj.rearrange( cameraMode, triggerMode, width, height, fonts );
	header.rearrange( cameraMode, triggerMode, width, height, fonts );
	cameraModeCombo.rearrange( cameraMode, triggerMode, width, height, fonts );
	emGainButton.rearrange( cameraMode, triggerMode, width, height, fonts );
	emGainDisplay.rearrange( cameraMode, triggerMode, width, height, fonts );
	emGainEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	triggerCombo.rearrange( cameraMode, triggerMode, width, height, fonts );
	setTemperatureButton.rearrange( cameraMode, triggerMode, width, height, fonts );
	temperatureOffButton.rearrange( cameraMode, triggerMode, width, height, fonts );
	temperatureEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	temperatureDisplay.rearrange( cameraMode, triggerMode, width, height, fonts );
	temperatureMessage.rearrange( cameraMode, triggerMode, width, height, fonts );
	kineticCycleTimeEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	kineticCycleTimeLabel.rearrange( cameraMode, triggerMode, width, height, fonts );
	accumulationCycleTimeEdit.rearrange(cameraMode, triggerMode, width, height, fonts);
	accumulationCycleTimeLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	accumulationNumberEdit.rearrange(cameraMode, triggerMode, width, height, fonts);
	accumulationNumberLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	minKineticCycleTimeLabel.rearrange( cameraMode, triggerMode, width, height, fonts );
	minKineticCycleTimeEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
}


void CameraSettingsControl::setEmGain(AndorCamera* andorObj)
{
	CString emGainText;
	emGainEdit.GetWindowTextA(emGainText);
	int emGain;
	try
	{
		emGain = std::stoi(str(emGainText));
	}
	catch (std::invalid_argument&)
	{
		thrower("ERROR: Couldn't convert EM Gain text to integer.");
	}
	// < 0 corresponds to NOT USING EM NIAWG_GAIN (using conventional gain).
	if (emGain < 0)
	{
		runSettings.emGainModeIsOn = false;
		runSettings.emGainLevel = 0;
		emGainDisplay.SetWindowTextA("OFF");
	}
	else
	{
		runSettings.emGainModeIsOn = true;
		runSettings.emGainLevel = emGain;
		emGainDisplay.SetWindowTextA(cstr("Gain: X" + str(runSettings.emGainLevel)));
	}
	// Change the andor settings.
	AndorRunSettings settings = andorObj->getSettings();
	settings.emGainLevel = runSettings.emGainLevel;
	settings.emGainModeIsOn = runSettings.emGainModeIsOn;
	andorObj->setSettings(settings);
	// and immediately change the EM gain mode.
	andorObj->setGainMode();
	emGainEdit.RedrawWindow();
}


void CameraSettingsControl::setVariationNumber(UINT varNumber)
{
	runSettings.totalVariations = varNumber;
	if (runSettings.totalVariations * runSettings.totalPicsInVariation > INT_MAX)
	{
		thrower( "ERROR: Trying to take too many pictures! Maximum picture number is " + str( INT_MAX ) );
	}
	runSettings.totalPicsInExperiment = int(runSettings.totalVariations * runSettings.totalPicsInVariation);
}


void CameraSettingsControl::setRepsPerVariation(UINT repsPerVar)
{
	runSettings.repetitionsPerVariation = repsPerVar;
	runSettings.totalPicsInVariation = runSettings.repetitionsPerVariation * runSettings.picsPerRepetition;
	if (runSettings.totalVariations * runSettings.totalPicsInVariation > INT_MAX)
	{
		thrower( "ERROR: Trying to take too many pictures! Maximum picture number is " + str( INT_MAX ) );
	}
	runSettings.totalPicsInExperiment = int(runSettings.totalVariations * runSettings.totalPicsInVariation);
}


void CameraSettingsControl::handleTimer()
{
	// This case displays the current temperature in the main window. When the temp stabilizes at the desired 
	// level the appropriate message is displayed.
	// initial value is only relevant for safemode.
	int currentTemperature = INT_MAX;
	int setTemperature = INT_MAX;
	try
	{
		// in this case you expect it to throw.
		setTemperature = andorFriend->getSettings().temperatureSetting;
		andorFriend->getTemperature(currentTemperature);
		if ( ANDOR_SAFEMODE ) { thrower( "SAFEMODE" ); }
	}
	catch (Error& exception)
	{
		// if not stable this won't get changed.
		if (exception.whatBare() == "DRV_TEMPERATURE_STABILIZED")
		{
			currentControlColor = "Green";
			temperatureDisplay.SetWindowTextA(cstr(setTemperature));
			temperatureMessage.SetWindowTextA(cstr("Temperature has stabilized at " + str(currentTemperature) 
											  + " (C)\r\n"));
		}
		else if (exception.whatBare() == "DRV_TEMPERATURE_NOT_REACHED")
		{
			currentControlColor = "Red";
			temperatureDisplay.SetWindowTextA(cstr(setTemperature));
			temperatureMessage.SetWindowTextA(cstr("Set temperature not yet reached. Current temperature is " 
											  + str(currentTemperature) + " (C)\r\n"));
		}
		else if (exception.whatBare() == "DRV_TEMPERATURE_NOT_STABILIZED")
		{
			currentControlColor = "Red";
			temperatureDisplay.SetWindowTextA(cstr(setTemperature));
			temperatureMessage.SetWindowTextA(cstr("Temperature of " + str(currentTemperature) 
											  + " (C) reached but not stable."));
		}
		else if (exception.whatBare() == "DRV_TEMPERATURE_DRIFT")
		{
			currentControlColor = "Red";
			temperatureDisplay.SetWindowTextA(cstr(setTemperature));
			temperatureMessage.SetWindowTextA(cstr("Temperature had stabilized but has since drifted. Temperature: " 
											  + str(currentTemperature)));
		}
		else if (exception.whatBare() == "DRV_TEMPERATURE_OFF")
		{
			currentControlColor = "Red";
			temperatureDisplay.SetWindowTextA(cstr(setTemperature));
			temperatureMessage.SetWindowTextA(cstr("Temperature control is off. Temperature: " + str(currentTemperature)));
		}
		else if (exception.whatBare() == "DRV_ACQUIRING")
		{
			// doesn't change color of temperature control. This way the color of the control represents the state of
			// the temperature right before the acquisition started, so that you can tell if you remembered to let it
			// completely stabilize or not.
			temperatureDisplay.SetWindowTextA(cstr(setTemperature));
			temperatureMessage.SetWindowTextA("Camera is Acquiring data. No Temperature updates are available.");
		}
		else if (exception.whatBare() == "SAFEMODE")
		{
			currentControlColor = "Red";
			temperatureDisplay.SetWindowTextA(cstr(setTemperature));
			temperatureMessage.SetWindowTextA("Application is running in Safemode... No Real Temperature Data is available.");
		}
		else
		{
			currentControlColor = "Red";
			temperatureDisplay.SetWindowTextA(cstr(currentTemperature));
			temperatureMessage.SetWindowTextA(cstr("Unexpected Temperature Code: " + exception.whatBare() + ". Temperature: " 
												+ str(currentTemperature)));
		}
	}
}


void CameraSettingsControl::updateRunSettingsFromPicSettings( )
{
	runSettings.exposureTimes = picSettingsObj.getUsedExposureTimes( );
	runSettings.picsPerRepetition = picSettingsObj.getPicsPerRepetition( );
	runSettings.totalPicsInVariation = runSettings.picsPerRepetition * runSettings.repetitionsPerVariation;
	if ( runSettings.totalVariations * runSettings.totalPicsInVariation > INT_MAX )
	{
		thrower( "ERROR: Trying to take too many pictures! Maximum picture number is " + str( INT_MAX ) );
	}
	runSettings.totalPicsInExperiment = runSettings.totalVariations * runSettings.totalPicsInVariation;
}


void CameraSettingsControl::handlePictureSettings(UINT id, AndorCamera* andorObj)
{
	picSettingsObj.handleOptionChange(id, andorObj);
	updateRunSettingsFromPicSettings( );
}


/*
 * This function checks things that don't have "Set" buttons. should be called to load the most recent values.
 */
void CameraSettingsControl::updatePassivelySetSettings()
{
	CString text;
	kineticCycleTimeEdit.GetWindowTextA(text);
	try
	{
		runSettings.kineticCycleTime = std::stof(str(text));
		kineticCycleTimeEdit.SetWindowTextA(cstr(runSettings.kineticCycleTime));
	}
	catch (std::invalid_argument&)
	{
		runSettings.kineticCycleTime = 0.1f;
		kineticCycleTimeEdit.SetWindowTextA(cstr(runSettings.kineticCycleTime));
		thrower("Please enter a valid float for the kinetic cycle time.");
	}

	accumulationCycleTimeEdit.GetWindowTextA(text);
	try
	{
		runSettings.accumulationTime = std::stof(str(text));
		accumulationCycleTimeEdit.SetWindowTextA(cstr(runSettings.accumulationTime));
	}
	catch (std::invalid_argument&)
	{
		runSettings.accumulationTime = 0.1f;
		accumulationCycleTimeEdit.SetWindowTextA(cstr(runSettings.accumulationTime));
		thrower("Please enter a valid float for the accumulation cycle time.");
	}

	accumulationNumberEdit.GetWindowTextA(text);
	try
	{
		runSettings.accumulationNumber = std::stol(str(text));
		accumulationNumberEdit.SetWindowTextA(cstr(runSettings.accumulationNumber));
	}
	catch (std::invalid_argument&)
	{
		runSettings.accumulationNumber = 1;
		accumulationNumberEdit.SetWindowTextA(cstr(runSettings.accumulationNumber));
		thrower("Please enter a valid float for the Accumulation number.");
	}
}


std::array<int, 4> CameraSettingsControl::getPaletteNumbers()
{
	return picSettingsObj.getPictureColors();
}


void CameraSettingsControl::initialize( cameraPositions& pos, int& id, CWnd* parent, cToolTips& tooltips)
{
	/// Header
	header.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	header.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	header.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	header.Create("CAMERA SETTINGS", WS_BORDER | WS_CHILD | WS_VISIBLE | ES_READONLY | ES_CENTER, header.seriesPos, 
				  parent, id++);
	header.fontType = HeadingFont;

	/// camera mode
	cameraModeCombo.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y + 100 };
	cameraModeCombo.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y + 100 };
	cameraModeCombo.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y + 100 };
	
	cameraModeCombo.Create( WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, cameraModeCombo.seriesPos, parent, 
							IDC_CAMERA_MODE_COMBO );
	cameraModeCombo.AddString("Kinetic Series Mode");
	cameraModeCombo.AddString("Accumulation Mode");
	cameraModeCombo.AddString("Video Mode");
	cameraModeCombo.SelectString(0, "Kinetic Series Mode");
	runSettings.cameraMode = "Kinetic Series Mode";
	pos.amPos.y += 25;
	pos.videoPos.y += 25;
	pos.seriesPos.y += 25;
	/// EM Gain
	emGainButton.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 120, pos.seriesPos.y + 20 };
	emGainButton.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 120, pos.videoPos.y + 20 };
	emGainButton.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 120, pos.amPos.y + 20 };
	emGainButton.Create("Set EM Gain", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, emGainButton.seriesPos, parent, 
						 IDC_SET_EM_GAIN_BUTTON );
	emGainButton.setToolTip( "Set the state & gain of the EM gain of the camera. Enter a negative number to turn EM Gain"
							 " mode off. The program will immediately change the state of the camera after pressing this button.", tooltips,
							 parent );
						//
	emGainEdit.seriesPos = { pos.seriesPos.x + 120, pos.seriesPos.y, pos.seriesPos.x + 300, pos.seriesPos.y + 20 };
	emGainEdit.amPos = { pos.amPos.x + 120, pos.amPos.y, pos.amPos.x + 300, pos.amPos.y + 20 };
	emGainEdit.videoPos = { pos.videoPos.x + 120, pos.videoPos.y, pos.videoPos.x + 300, pos.videoPos.y + 20 };
	emGainEdit.Create(WS_CHILD | WS_VISIBLE | BS_RIGHT, emGainEdit.seriesPos, parent, id++);
	//
	emGainDisplay.seriesPos = { pos.seriesPos.x + 300, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y + 20 };
	emGainDisplay.videoPos = { pos.videoPos.x + 300, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y + 20 };
	emGainDisplay.amPos = { pos.amPos.x + 300, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y + 20 };
	emGainDisplay.Create( "OFF", WS_CHILD | WS_VISIBLE | BS_RIGHT | ES_READONLY | ES_CENTER, emGainDisplay.seriesPos, 
						  parent, id++);
	// initialize settings.
	runSettings.emGainLevel = 0;
	runSettings.emGainModeIsOn = false;
	//
	pos.seriesPos.y += 20;
	pos.amPos.y += 20;
	pos.videoPos.y += 20;

	// trigger combo
	triggerCombo.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y + 800 };
	triggerCombo.videoPos = { pos.videoPos.x, pos.videoPos.y,pos.videoPos.x + 480, pos.videoPos.y + 800 };
	triggerCombo.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y + 800 };
	triggerCombo.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, triggerCombo.seriesPos, parent, IDC_TRIGGER_COMBO );
	// set options for the combo
	triggerCombo.AddString("Internal Trigger");
	triggerCombo.AddString("External Trigger");
	triggerCombo.AddString("Start On Trigger");
	// Select default trigger
	triggerCombo.SelectString(0, "External Trigger");
	pos.seriesPos.y += 25;
	pos.amPos.y += 25;
	pos.videoPos.y += 25;
	runSettings.triggerMode = "External Trigger";
	// Set temperature Button
	setTemperatureButton.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 270, pos.seriesPos.y + 25 };
	setTemperatureButton.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 270, pos.videoPos.y + 25 };
	setTemperatureButton.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 270, pos.amPos.y + 25 };
	setTemperatureButton.Create("Set Camera Temperature (C)", WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON, setTemperatureButton.seriesPos,
								parent, IDC_SET_TEMPERATURE_BUTTON );
	// Temperature Edit
	temperatureEdit.seriesPos = { pos.seriesPos.x + 270, pos.seriesPos.y, pos.seriesPos.x + 350, pos.seriesPos.y + 25 };
	temperatureEdit.videoPos = { pos.videoPos.x + 270, pos.videoPos.y, pos.videoPos.x + 350, pos.videoPos.y + 25 };
	temperatureEdit.amPos = { pos.amPos.x + 270, pos.amPos.y, pos.amPos.x + 350, pos.amPos.y + 25 };
	temperatureEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, temperatureEdit.seriesPos, parent, id++);
	temperatureEdit.SetWindowTextA("0");
	// Temperature Setting Display
	temperatureDisplay.seriesPos = { pos.seriesPos.x + 350, pos.seriesPos.y, pos.seriesPos.x + 430, pos.seriesPos.y + 25 };
	temperatureDisplay.videoPos = { pos.videoPos.x + 350, pos.videoPos.y, pos.videoPos.x + 430, pos.videoPos.y + 25 };
	temperatureDisplay.amPos = { pos.amPos.x + 350, pos.amPos.y, pos.amPos.x + 430, pos.amPos.y + 25 };
	temperatureDisplay.Create("", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY, temperatureDisplay.seriesPos, parent, id++);
	// Temperature Control Off Button
	temperatureOffButton.seriesPos = { pos.seriesPos.x + 430, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y + 25 };
	temperatureOffButton.videoPos = { pos.videoPos.x + 430, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y + 25 };
	temperatureOffButton.amPos = { pos.amPos.x + 430, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y + 25 };
	temperatureOffButton.Create("OFF", WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON, temperatureOffButton.seriesPos, parent, id++);
	pos.seriesPos.y += 25;
	pos.amPos.y += 25;
	pos.videoPos.y += 25;
	// Temperature Message Display
	temperatureMessage.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y + 50 };
	temperatureMessage.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y + 50 };
	temperatureMessage.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y + 50 };	
	temperatureMessage.Create( "Temperature control is disabled", WS_CHILD | WS_VISIBLE | SS_LEFT, 
							  temperatureMessage.seriesPos, parent, id++);
	pos.seriesPos.y += 50;
	pos.amPos.y += 50;
	pos.videoPos.y += 50;
	//
	picSettingsObj.initialize(pos, parent, id);

	imageDimensionsObj.initialize(pos, parent, false, id);

	/// REPETITIONS PER VARIATION

	// Accumulation Time
	accumulationCycleTimeLabel.seriesPos = { -1,-1,-1,-1 };
	accumulationCycleTimeLabel.videoPos = { -1,-1,-1,-1 };
	accumulationCycleTimeLabel.amPos = { pos.amPos.x,pos.amPos.y,pos.amPos.x +240,pos.amPos.y + 25 };
	accumulationCycleTimeLabel.Create( "Accumulation Cycle Time", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
									   accumulationCycleTimeLabel.seriesPos, parent, id++);

	accumulationCycleTimeEdit.seriesPos = { -1,-1,-1,-1 };
	accumulationCycleTimeEdit.videoPos = { -1,-1,-1,-1 };
	accumulationCycleTimeEdit.amPos = { pos.amPos.x + 240,pos.amPos.y,pos.amPos.x + 480, pos.amPos.y += 25 };
	accumulationCycleTimeEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER , accumulationCycleTimeEdit.seriesPos, parent, 
									 id++);
	accumulationCycleTimeEdit.SetWindowTextA("0.1");
	
	// Accumulation Number
	accumulationNumberLabel.seriesPos = { -1,-1,-1,-1 };
	accumulationNumberLabel.videoPos = { -1,-1,-1,-1 };
	accumulationNumberLabel.amPos = { pos.amPos.x,pos.amPos.y,pos.amPos.x + 240,pos.amPos.y + 25 };
	accumulationNumberLabel.Create( "Accumulation #", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
								    accumulationNumberLabel.seriesPos, parent, id++);
	//
	accumulationNumberEdit.seriesPos = { -1,-1,-1,-1 };
	accumulationNumberEdit.videoPos = { -1,-1,-1,-1 };
	accumulationNumberEdit.amPos = { pos.amPos.x + 240,pos.amPos.y,pos.amPos.x + 480,pos.amPos.y += 25 };
	accumulationNumberEdit.Create( WS_CHILD | WS_VISIBLE | WS_BORDER , accumulationNumberEdit.seriesPos, 
								   parent, id++ );
	accumulationNumberEdit.SetWindowTextA("1");

	// minimum kinetic cycle time (determined by camera)
	minKineticCycleTimeLabel.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 240, pos.seriesPos.y + 25 };
	minKineticCycleTimeLabel.videoPos = { -1,-1,-1,-1 };
	minKineticCycleTimeLabel.amPos = { -1,-1,-1,-1 };
	minKineticCycleTimeLabel.Create( "Minimum Kinetic Cycle Time (s)", WS_CHILD | WS_VISIBLE | WS_BORDER, 
									 minKineticCycleTimeLabel.seriesPos, parent, id++ );
	
	minKineticCycleTimeEdit.seriesPos = { pos.seriesPos.x + 240, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	minKineticCycleTimeEdit.videoPos = { -1,-1,-1,-1 };
	minKineticCycleTimeEdit.amPos = { -1,-1,-1,-1 };
	minKineticCycleTimeEdit.Create( WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, minKineticCycleTimeEdit.seriesPos,
									parent, id++ );
	minKineticCycleTimeEdit.SetWindowTextA( "" );

	/// Kinetic Cycle Time
	// Kinetic Cycle Time Label
	kineticCycleTimeLabel.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 240, pos.seriesPos.y + 25 };
	kineticCycleTimeLabel.videoPos = { -1,-1,-1,-1 };
	kineticCycleTimeLabel.amPos = { -1,-1,-1,-1 };
	kineticCycleTimeLabel.triggerModeSensitive = -1;
	kineticCycleTimeLabel.Create( "Kinetic Cycle Time", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, kineticCycleTimeLabel.seriesPos,
								  parent, id++ );

	// Kinetic Cycle Time Edit
	kineticCycleTimeEdit.seriesPos = { pos.seriesPos.x + 240, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	kineticCycleTimeEdit.videoPos = { -1,-1,-1,-1 };
	kineticCycleTimeEdit.amPos = { -1,-1,-1,-1 };
	kineticCycleTimeEdit.triggerModeSensitive = -1;
	kineticCycleTimeEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER, kineticCycleTimeEdit.seriesPos, parent, id++);
	kineticCycleTimeEdit.SetWindowTextA("0.1");
}


void CameraSettingsControl::handleOpenConfig(std::ifstream& configFile, double version)
{
	ProfileSystem::checkDelimiterLine(configFile, "CAMERA_SETTINGS");
	AndorRunSettings tempSettings;
	configFile.get( );
	std::getline(configFile, tempSettings.triggerMode);	
	configFile >> tempSettings.emGainModeIsOn;
	configFile >> tempSettings.emGainLevel;
	configFile.get();
	std::getline(configFile, tempSettings.cameraMode);
	if (tempSettings.cameraMode == "Video Mode")
	{
		tempSettings.acquisitionMode = 5;
		tempSettings.totalPicsInVariation = INT_MAX;
	}
	else if (tempSettings.cameraMode == "Kinetic Series Mode")
	{
		tempSettings.acquisitionMode = 3;
	}
	else if (tempSettings.cameraMode == "Accumulate Mode")
	{
		tempSettings.acquisitionMode = 2;
		tempSettings.totalPicsInVariation = INT_MAX;
	}
	else
	{
		thrower("ERROR: Unrecognized camera mode!");
	}
	configFile >> tempSettings.kineticCycleTime;
	configFile >> tempSettings.accumulationTime;
	configFile >> tempSettings.accumulationNumber;
	configFile >> tempSettings.temperatureSetting; 
 	setRunSettings(tempSettings);
 	ProfileSystem::checkDelimiterLine(configFile, "END_CAMERA_SETTINGS");
	picSettingsObj.handleOpenConfig(configFile, version, andorFriend);
	updateRunSettingsFromPicSettings( );
	if ( version > 2.41 )
	{
		imageDimensionsObj.handleOpen( configFile, version );
	}
}


void CameraSettingsControl::handleNewConfig( std::ofstream& newFile )
{
	newFile << "CAMERA_SETTINGS\n";
	newFile << "External Trigger" << "\n";
	newFile << 0 << "\n";
	newFile << 0 << "\n";
	newFile << "Kinetic Series Mode" << "\n";
	newFile << 1000 << "\n";
	newFile << 1000 << "\n";
	newFile << 2 << "\n";
	newFile << 25 << "\n";
	newFile << "END_CAMERA_SETTINGS\n";
	picSettingsObj.handleNewConfig( newFile );
	imageDimensionsObj.handleNew( newFile );
}


void CameraSettingsControl::handleSaveConfig(std::ofstream& saveFile)
{
	saveFile << "CAMERA_SETTINGS\n";
	saveFile << runSettings.triggerMode << "\n";
	saveFile << runSettings.emGainModeIsOn << "\n";
	saveFile << runSettings.emGainLevel << "\n";
	saveFile << runSettings.cameraMode << "\n";
	saveFile << runSettings.kineticCycleTime << "\n";
	saveFile << runSettings.accumulationTime << "\n";
	saveFile << runSettings.accumulationNumber << "\n";
	saveFile << runSettings.temperatureSetting << "\n";
	saveFile << "END_CAMERA_SETTINGS\n";

	picSettingsObj.handleSaveConfig(saveFile);
	imageDimensionsObj.handleSave( saveFile );
}


void CameraSettingsControl::handleModeChange( CameraWindow* cameraWindow )
{
	int sel = cameraModeCombo.GetCurSel();
	if (sel == -1)
	{
		return;
	}
	CString mode;
	cameraModeCombo.GetLBText( sel, mode );
	runSettings.cameraMode = mode;


	//						
	if (runSettings.cameraMode == "Video Mode")
	{
		runSettings.acquisitionMode = 5;
		runSettings.totalPicsInVariation = INT_MAX;
		runSettings.repetitionsPerVariation = runSettings.totalPicsInVariation / runSettings.picsPerRepetition;

	}
	else if (runSettings.cameraMode == "Kinetic Series Mode")
	{
		runSettings.acquisitionMode = 3;
	}
	else if (runSettings.cameraMode == "Accumulate Mode")
	{
		runSettings.acquisitionMode = 2;
	}


	CRect rect;
	cameraWindow->GetWindowRect( &rect );
	cameraWindow->OnSize( 0, rect.right - rect.left, rect.bottom - rect.top );
}


void CameraSettingsControl::checkTimings(std::vector<float>& exposureTimes)
{
	checkTimings(runSettings.kineticCycleTime, runSettings.accumulationTime, exposureTimes);
}


void CameraSettingsControl::checkTimings(float& kineticCycleTime, float& accumulationTime, std::vector<float>& exposureTimes)
{
	andorFriend->checkAcquisitionTimings(kineticCycleTime, accumulationTime, exposureTimes);
}


void CameraSettingsControl::updateMinKineticCycleTime( double time )
{
	minKineticCycleTimeEdit.SetWindowTextA( cstr( time ) );
}


imageParameters CameraSettingsControl::readImageParameters(CameraWindow* camWin)
{
	imageParameters parameters = imageDimensionsObj.readImageParameters( camWin );
	runSettings.imageSettings = parameters;
	return parameters;
}

CBrush* CameraSettingsControl::handleColor( int idNumber, CDC* colorer, brushMap brushes, rgbMap rgbs )
{
	return picSettingsObj.colorControls( idNumber, colorer, brushes, rgbs );
}


void CameraSettingsControl::setImageParameters(imageParameters newSettings, CameraWindow* camWin)
{
	imageDimensionsObj.setImageParametersFromInput(newSettings, camWin);
}


void CameraSettingsControl::checkIfReady()
{
	if ( picSettingsObj.getUsedExposureTimes().size() == 0 )
	{
		thrower("Please Set at least one exposure time.");
	}
	if ( !imageDimensionsObj.checkReady() )
	{
		thrower("Please set the image parameters.");
	}
	if ( runSettings.picsPerRepetition <= 0 )
	{
		thrower("ERROR: Please set the number of pictures per repetition to a positive non-zero value.");
	}
	if ( runSettings.cameraMode == "Kinetic Series Mode" )
	{
		if ( runSettings.kineticCycleTime == 0 && runSettings.triggerMode == "Internal Trigger" )
		{
			thrower("ERROR: Since you are running in internal trigger mode, please Set a kinetic cycle time.");
		}
		if ( runSettings.repetitionsPerVariation <= 0 )
		{
			thrower("ERROR: Please set the \"Repetitions Per Variation\" variable to a positive non-zero value.");
		}
		if ( runSettings.totalVariations <= 0 )
		{
			thrower("ERROR: Please set the number of variations to a positive non-zero value.");
		}
	}
	if ( runSettings.cameraMode == "Accumulate Mode" )
	{
		if ( runSettings.accumulationNumber <= 0 )
		{
			thrower("ERROR: Please set the current Accumulation Number to a positive non-zero value.");
		}
		if ( runSettings.accumulationTime <= 0 )
		{
			thrower("ERROR: Please set the current Accumulation Time to a positive non-zero value.");
		}
	}
}
