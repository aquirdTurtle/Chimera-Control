#include "stdafx.h"
#include "CameraSettingsControl.h"
#include "CameraWindow.h"

// note that this object doesn't actually store the camera state, it just uses it in passing to figure out whether 
// buttons should be on or off.
void CameraSettingsControl::cameraIsOn(bool state)
{
	// Can't change em gain mode or camera settings once started.
	emGainButton.EnableWindow( !state );
	setTemperatureButton.EnableWindow( !state );
	temperatureOffButton.EnableWindow( !state );
	picSettingsObj.cameraIsOn( state );
	imageDimensionsObj.cameraIsOn( state );
}

std::array<int, 4> CameraSettingsControl::getThresholds()
{
	return picSettingsObj.getThresholds();
}


void CameraSettingsControl::handleSetTemperatureOffPress()
{
	andorFriend->changeTemperatureSetting(true);
	//eCameraFileSystem.updateSaveStatus(false);
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
	double temp;
	try
	{
		temp = std::stof(std::string(text));
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

void CameraSettingsControl::handleSetRepsPerVar()
{
	CString tempStr;
	repsPerVarEdit.GetWindowTextA(tempStr);
	// GetWindowText(eRepetitionsPerVariationEdit.hwnd, (LPSTR)tempStr.c_str(), 20);
	std::string input(tempStr);
	try
	{
		runSettings.repetitionsPerVariation = std::stoi(input);
		runSettings.totalPicsInVariation = runSettings.repetitionsPerVariation * runSettings.picsPerRepetition;
		runSettings.totalPicsInExperiment = runSettings.totalVariations * runSettings.totalPicsInVariation;
	}
	catch (std::invalid_argument &exception)
	{
		thrower("ERROR: number entered was not valid.");
		runSettings.repetitionsPerVariation = 1;
		runSettings.totalPicsInVariation = runSettings.repetitionsPerVariation * runSettings.picsPerRepetition;
		runSettings.totalPicsInExperiment = runSettings.totalVariations * runSettings.totalPicsInVariation;
	}
	repsPerVarDisp.SetWindowTextA(cstr(runSettings.repetitionsPerVariation));
	//
	//eCameraFileSystem.updateSaveStatus(false);
	repsPerVarEdit.RedrawWindow();
}


void CameraSettingsControl::handleSetVarNum()
{
	CString tempStr;
	varNumEdit.GetWindowTextA(tempStr);
	// GetWindowText(eRepetitionsPerVariationEdit.hwnd, (LPSTR)tempStr.c_str(), 20);
	std::string input(tempStr);
	try
	{
		runSettings.totalVariations = std::stoi(input);
		runSettings.totalPicsInExperiment = runSettings.totalVariations * runSettings.totalPicsInVariation;
	}
	catch (std::invalid_argument &exception)
	{
		thrower("ERROR: number entered was not valid.");
		runSettings.totalVariations = 1;
		runSettings.totalPicsInExperiment = runSettings.totalVariations * runSettings.totalPicsInVariation;
	}
	varNumDisp.SetWindowTextA(cstr(runSettings.totalVariations));
	//
	//eCameraFileSystem.updateSaveStatus(false);
	varNumEdit.RedrawWindow();
}


void CameraSettingsControl::handleTriggerControl(CameraWindow* cameraWindow)
{
	CString triggerMode;
	long long itemIndex = triggerCombo.GetCurSel();
	if (itemIndex == -1)
	{
		return;
	}
	triggerCombo.GetLBText(itemIndex, triggerMode);
	runSettings.triggerMode = triggerMode;
	CRect rect;
	cameraWindow->GetWindowRect(&rect);
	cameraWindow->OnSize(0, rect.right - rect.left, rect.bottom - rect.top);
	/// TODO.
	//eCameraFileSystem.updateSaveStatus(false);
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
	setRepsPerVar.rearrange(cameraMode, triggerMode, width, height, fonts);
	repsPerVarEdit.rearrange(cameraMode, triggerMode, width, height, fonts);
	repsPerVarDisp.rearrange(cameraMode, triggerMode, width, height, fonts);
	setVarNum.rearrange(cameraMode, triggerMode, width, height, fonts);
	varNumEdit.rearrange(cameraMode, triggerMode, width, height, fonts);
	varNumDisp.rearrange(cameraMode, triggerMode, width, height, fonts);
}

void CameraSettingsControl::setEmGain(AndorCamera* andorObj)
{
	CString emGainText;
	emGainEdit.GetWindowTextA(emGainText);
	int emGain;
	try
	{
		emGain = std::stoi(std::string(emGainText));
	}
	catch (std::invalid_argument&)
	{
		thrower("ERROR: Couldn't convert EM Gain text to integer.");
	}
	// < 0 corresponds to NOT USING EM GAIN (using conventional gain).
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
		emGainDisplay.SetWindowTextA(("Gain: X" + std::to_string(runSettings.emGainLevel)).c_str());
	}
	// Change the andor settings.
	AndorRunSettings settings = andorObj->getSettings();
	settings.emGainLevel = runSettings.emGainLevel;
	settings.emGainModeIsOn = runSettings.emGainModeIsOn;
	andorObj->setSettings(settings);
	// and immediately change the EM gain mode.
	andorObj->setGainMode();
	/// TODO
	//eCameraFileSystem.updateSaveStatus(false);
	emGainEdit.RedrawWindow();
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
			temperatureMessage.SetWindowTextA(("Temperature has stabilized at " + std::to_string(currentTemperature) + " (C)\r\n").c_str());
		}
		else if (exception.whatBare() == "DRV_TEMPERATURE_NOT_REACHED")
		{
			currentControlColor = "Red";
			temperatureDisplay.SetWindowTextA(cstr(setTemperature));
			temperatureMessage.SetWindowTextA(("Set temperature not yet reached. Current temperature is " + std::to_string(currentTemperature) + " (C)\r\n").c_str());
		}
		else if (exception.whatBare() == "DRV_TEMPERATURE_NOT_STABILIZED")
		{
			currentControlColor = "Red";
			temperatureDisplay.SetWindowTextA(cstr(setTemperature));
			temperatureMessage.SetWindowTextA(("Temperature of " + std::to_string(currentTemperature) + " (C) reached but not stable.").c_str());
		}
		else if (exception.whatBare() == "DRV_TEMPERATURE_DRIFT")
		{
			currentControlColor = "Red";
			temperatureDisplay.SetWindowTextA(cstr(setTemperature));
			temperatureMessage.SetWindowTextA(("Temperature had stabilized but has since drifted. Temperature: " + std::to_string(currentTemperature)).c_str());
		}
		else if (exception.whatBare() == "DRV_TEMPERATURE_OFF")
		{
			currentControlColor = "Red";
			temperatureDisplay.SetWindowTextA(cstr(setTemperature));
			temperatureMessage.SetWindowTextA(("Temperature control is off. Temperature: " + std::to_string(currentTemperature)).c_str());
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
			temperatureMessage.SetWindowTextA(("Unexpected Temperature Code: " + exception.whatBare() + ". Temperature: " 
												+ str(currentTemperature)).c_str());
		}
	}
}


void CameraSettingsControl::handlePictureSettings(UINT id, AndorCamera* andorObj)
{
	picSettingsObj.handleOptionChange(id, andorObj);
	runSettings.exposureTimes = picSettingsObj.getUsedExposureTimes();
	runSettings.picsPerRepetition = picSettingsObj.getPicsPerRepetition();
	runSettings.totalPicsInVariation = runSettings.picsPerRepetition * runSettings.repetitionsPerVariation;
	runSettings.totalPicsInExperiment = runSettings.picsPerRepetition  * runSettings.repetitionsPerVariation * runSettings.totalVariations;
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
		runSettings.kinetiCycleTime = std::stof(std::string(text));
		kineticCycleTimeEdit.SetWindowTextA(cstr(runSettings.kinetiCycleTime));
	}
	catch (std::invalid_argument& err)
	{
		runSettings.kinetiCycleTime = 0.1f;
		kineticCycleTimeEdit.SetWindowTextA(cstr(runSettings.kinetiCycleTime));
		thrower("Please enter a valid float for the kinetic cycle time.");
	}
}

std::array<int, 4> CameraSettingsControl::getPaletteNumbers()
{
	return picSettingsObj.getPictureColors();
}

void CameraSettingsControl::initialize(cameraPositions& pos, int& id, CWnd* parent, fontMap fonts, std::vector<CToolTipCtrl*>& tooltips)
{
	/// Header
	header.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	header.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	header.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	header.ID = id++;
	header.Create("CAMERA SETTINGS", WS_BORDER | WS_CHILD | WS_VISIBLE | ES_READONLY | ES_CENTER, header.seriesPos, parent, header.ID);
	header.fontType = Heading;

	/// camera mode
	cameraModeCombo.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y + 100 };
	cameraModeCombo.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y + 100 };
	cameraModeCombo.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y + 100 };
	cameraModeCombo.ID = id++;
	if (cameraModeCombo.ID != IDC_CAMERA_MODE_COMBO)
	{
		throw;
	}
	cameraModeCombo.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, cameraModeCombo.seriesPos, parent, cameraModeCombo.ID);
	cameraModeCombo.fontType = Normal;
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
	emGainButton.ID = id++;
	if (emGainButton.ID != IDC_SET_EM_GAIN_BUTTON)
	{
		throw;
	}
	emGainButton.Create("Set EM Gain", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, emGainButton.seriesPos, parent, emGainButton.ID);
	emGainButton.fontType = Normal;
	emGainButton.setToolTip("Set the state & gain of the EM gain of the camera. Enter a negative number to turn EM Gain"
							" mode off. The program will immediately change the state of the camera after pressing this button.", tooltips,
							parent, fonts["Normal Font"]);
						//
	emGainEdit.seriesPos = { pos.seriesPos.x + 120, pos.seriesPos.y, pos.seriesPos.x + 300, pos.seriesPos.y + 20 };
	emGainEdit.amPos = { pos.amPos.x + 120, pos.amPos.y, pos.amPos.x + 300, pos.amPos.y + 20 };
	emGainEdit.videoPos = { pos.videoPos.x + 120, pos.videoPos.y, pos.videoPos.x + 300, pos.videoPos.y + 20 };
	emGainEdit.ID = id++;
	emGainEdit.Create(WS_CHILD | WS_VISIBLE | BS_RIGHT, emGainEdit.seriesPos, parent, emGainEdit.ID);
	emGainEdit.fontType = Normal;
	//
	emGainDisplay.seriesPos = { pos.seriesPos.x + 300, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y + 20 };
	emGainDisplay.videoPos = { pos.videoPos.x + 300, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y + 20 };
	emGainDisplay.amPos = { pos.amPos.x + 300, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y + 20 };
	emGainDisplay.ID = id++;
	emGainDisplay.Create("OFF", WS_CHILD | WS_VISIBLE | BS_RIGHT | ES_READONLY | ES_CENTER, emGainDisplay.seriesPos, parent, emGainDisplay.ID);
	emGainDisplay.fontType = Normal;
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
	triggerCombo.ID = id++;
	if (triggerCombo.ID != IDC_TRIGGER_COMBO)
	{
		throw;
	}
	triggerCombo.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, triggerCombo.seriesPos, parent, triggerCombo.ID);
	// set options for the combo
	triggerCombo.fontType = Normal;
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
	setTemperatureButton.ID = id++;
	setTemperatureButton.Create("Set Camera Temperature (C)", WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON, setTemperatureButton.seriesPos,
								parent, setTemperatureButton.ID);
	setTemperatureButton.fontType = Normal;
	if (setTemperatureButton.ID != IDC_SET_TEMPERATURE_BUTTON)
	{
		throw;
	}
	// Temperature Edit
	temperatureEdit.seriesPos = { pos.seriesPos.x + 270, pos.seriesPos.y, pos.seriesPos.x + 350, pos.seriesPos.y + 25 };
	temperatureEdit.videoPos = { pos.videoPos.x + 270, pos.videoPos.y, pos.videoPos.x + 350, pos.videoPos.y + 25 };
	temperatureEdit.amPos = { pos.amPos.x + 270, pos.amPos.y, pos.amPos.x + 350, pos.amPos.y + 25 };
	temperatureEdit.ID = id++;
	temperatureEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, temperatureEdit.seriesPos, parent, temperatureEdit.ID);
	temperatureEdit.SetWindowTextA("0");
	temperatureEdit.fontType = Normal;
	// Temperature Setting Display
	temperatureDisplay.seriesPos = { pos.seriesPos.x + 350, pos.seriesPos.y, pos.seriesPos.x + 430, pos.seriesPos.y + 25 };
	temperatureDisplay.videoPos = { pos.videoPos.x + 350, pos.videoPos.y, pos.videoPos.x + 430, pos.videoPos.y + 25 };
	temperatureDisplay.amPos = { pos.amPos.x + 350, pos.amPos.y, pos.amPos.x + 430, pos.amPos.y + 25 };
	temperatureDisplay.ID = id++;
	temperatureDisplay.Create("", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY, temperatureDisplay.seriesPos, parent, temperatureDisplay.ID);
	temperatureDisplay.fontType = Normal;
	// Temperature Control Off Button
	temperatureOffButton.seriesPos = { pos.seriesPos.x + 430, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y + 25 };
	temperatureOffButton.videoPos = { pos.videoPos.x + 430, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y + 25 };
	temperatureOffButton.amPos = { pos.amPos.x + 430, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y + 25 };
	temperatureOffButton.Create("OFF",
								WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON, temperatureOffButton.seriesPos, parent, temperatureOffButton.ID);
	temperatureOffButton.fontType = Normal;
	pos.seriesPos.y += 25;
	pos.amPos.y += 25;
	pos.videoPos.y += 25;
	// Temperature Message Display
	temperatureMessage.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y + 50 };
	temperatureMessage.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y + 50 };
	temperatureMessage.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y + 50 };	
	temperatureMessage.ID = id++;
	temperatureMessage.Create("Temperature control is disabled", WS_CHILD | WS_VISIBLE | SS_LEFT, temperatureMessage.seriesPos, parent,
							  temperatureMessage.ID);
	temperatureMessage.fontType = Normal;
	pos.seriesPos.y += 50;
	pos.amPos.y += 50;
	pos.videoPos.y += 50;
	//
	picSettingsObj.initialize(pos, parent, id);

	imageDimensionsObj.initialize(pos, parent, false, id);

	/// REPETITIONS PER VARIATION
	// Set Repetitions Per Variation Button
	setRepsPerVar.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 220, pos.seriesPos.y + 25 };
	setRepsPerVar.videoPos = { -1,-1,-1,-1 };
	setRepsPerVar.amPos = { -1,-1,-1,-1 };
	setRepsPerVar.ID = id++;
	if (setRepsPerVar.ID != IDC_SET_REPETITONS_PER_VARIATION_BUTTON)
	{
		throw;
	}
	setRepsPerVar.Create("Set Rep. / Variation", WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON, setRepsPerVar.seriesPos,
						 parent, setRepsPerVar.ID);
	setRepsPerVar.fontType = Normal;
	// Repetitions Per Variation Edit
	repsPerVarEdit.seriesPos = { pos.seriesPos.x + 220, pos.seriesPos.y, pos.seriesPos.x + 320, pos.seriesPos.y + 25 };
	repsPerVarEdit.videoPos = { -1,-1,-1,-1 };
	repsPerVarEdit.amPos = { -1,-1,-1,-1 };
	repsPerVarEdit.ID = id++;
	repsPerVarEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER, repsPerVarEdit.seriesPos, parent, repsPerVarEdit.ID);
	repsPerVarEdit.SetWindowTextA("10");
	repsPerVarEdit.fontType = Normal;

	// Repetitions Per Variation Display
	repsPerVarDisp.seriesPos = { pos.seriesPos.x + 320, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	repsPerVarDisp.videoPos = { -1,-1,-1,-1 };
	repsPerVarDisp.amPos = { -1,-1,-1,-1 };
	repsPerVarDisp.ID = id++;
	repsPerVarDisp.Create("10", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, repsPerVarDisp.seriesPos, parent,
						  repsPerVarDisp.ID);
	repsPerVarDisp.fontType = Normal;

	/// VARIATION NUMBER
	// Set Variation Number Button
	setVarNum.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x+220, pos.seriesPos.y + 25 };
	setVarNum.videoPos = { -1,-1,-1,-1 };
	setVarNum.amPos = { -1,-1,-1,-1 };
	setVarNum.ID = id++;
	if (setVarNum.ID != IDC_SET_VARIATION_NUMBER)
	{
		throw;
	}
	setVarNum.Create("Set # Variations", WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON, setVarNum.seriesPos, parent, setVarNum.ID);
	setVarNum.fontType = Normal;

	// Variation # Edit
	varNumEdit.seriesPos = { pos.seriesPos.x + 220, pos.seriesPos.y, pos.seriesPos.x + 320, pos.seriesPos.y + 25 };
	varNumEdit.videoPos = { -1,-1,-1,-1 };
	varNumEdit.amPos = { -1,-1,-1,-1 };
	varNumEdit.ID = id++;
	varNumEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER, varNumEdit.seriesPos, parent, varNumEdit.ID);
	varNumEdit.SetWindowTextA("3");
	varNumEdit.fontType = Normal;
	// Variation Number Display
	varNumDisp.seriesPos = { pos.seriesPos.x + 320, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	varNumDisp.videoPos = { -1,-1,-1,-1 };
	varNumDisp.amPos = { -1,-1,-1,-1 };
	varNumDisp.ID = id++;
	varNumDisp.Create("3", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, varNumDisp.seriesPos, parent, varNumDisp.ID);
	varNumDisp.fontType = Normal;

	/// Kinetic Cycle Time
	// Kinetic Cycle Time Label
	kineticCycleTimeLabel.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 240, pos.seriesPos.y + 25 };
	kineticCycleTimeLabel.videoPos = { -1,-1,-1,-1 };
	kineticCycleTimeLabel.amPos = { -1,-1,-1,-1 };
	kineticCycleTimeLabel.ID = id++;
	kineticCycleTimeLabel.triggerModeSensitive = -1;
	kineticCycleTimeLabel.Create("Kinetic Cycle Time", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, kineticCycleTimeLabel.seriesPos,
								 parent, kineticCycleTimeLabel.ID);
	kineticCycleTimeLabel.fontType = Normal;

	// Kinetic Cycle Time Edit
	kineticCycleTimeEdit.seriesPos = { pos.seriesPos.x + 240, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	kineticCycleTimeEdit.videoPos = { -1,-1,-1,-1 };
	kineticCycleTimeEdit.amPos = { -1,-1,-1,-1 };
	kineticCycleTimeEdit.ID = id++;
	kineticCycleTimeEdit.triggerModeSensitive = -1;
	kineticCycleTimeEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER, kineticCycleTimeEdit.seriesPos, parent, kineticCycleTimeEdit.ID);
	kineticCycleTimeEdit.SetWindowTextA("0.1");
	kineticCycleTimeEdit.fontType = Normal;
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
		/*
		if (ePicturesPerVariation != INT_MAX)
		{
			ePreviousPicturesPerSubSeries = ePicturesPerVariation;
		}
		*/
		runSettings.totalPicsInVariation = INT_MAX;
		runSettings.repetitionsPerVariation = runSettings.totalPicsInVariation / runSettings.picsPerRepetition;
		repsPerVarDisp.SetWindowTextA(cstr(runSettings.repetitionsPerVariation));

	}
	else if (runSettings.cameraMode == "Kinetic Series Mode")
	{
		runSettings.acquisitionMode = 3;
		//ePicturesPerVariation = ePreviousPicturesPerSubSeries;
		repsPerVarDisp.SetWindowTextA(cstr(runSettings.repetitionsPerVariation));
	}
	else if (runSettings.cameraMode == "Accumulate Mode")
	{
		runSettings.acquisitionMode = 2;
		/*
		if (ePicturesPerVariation != INT_MAX)
		{
			ePreviousPicturesPerSubSeries = ePicturesPerVariation;
		}
		*/		
		//ePicturesPerVariation = INT_MAX;
		//SendMessage(eRepetitionsPerVariationDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(ePicturesPerVariation).c_str());
	}


	CRect rect;
	cameraWindow->GetWindowRect( &rect );
	cameraWindow->OnSize( 0, rect.right - rect.left, rect.bottom - rect.top );
}

void CameraSettingsControl::checkTimings(std::vector<float> exposureTimes)
{
	checkTimings(runSettings.kinetiCycleTime, runSettings.accumulationTime, exposureTimes);
}

void CameraSettingsControl::checkTimings(float kineticCycleTime, float accumulationTime, std::vector<float> exposureTimes)
{
	andorFriend->checkAcquisitionTimings(kineticCycleTime, accumulationTime, exposureTimes);
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
		if ( runSettings.kinetiCycleTime == 0 && runSettings.triggerMode == "Internal Trigger" )
		{
			thrower("ERROR: Since you are running in internal trigger mode, please Set a kinetic cycle time.");
		}
		if ( runSettings.repetitionsPerVariation <= 0 )
		{
			thrower("ERROR: Please set the \"Repetitions Per Variation\" variable to a positive non-zero value.");
		}
		if ( runSettings.totalVariations <= 0 )
		{
			thrower("ERROR: Please set the number of accumulation stacks to a positive non-zero value.");
		}
	}
	if ( runSettings.cameraMode == "Accumulate Mode" )
	{
		if ( runSettings.totalAccumulationNumber <= 0 )
		{
			thrower("ERROR: Please set the current Accumulation Number to a positive non-zero value.");
		}
		if ( runSettings.accumulationTime <= 0 )
		{
			thrower("ERROR: Please set the current Accumulation Time to a positive non-zero value.");
		}
	}
}
