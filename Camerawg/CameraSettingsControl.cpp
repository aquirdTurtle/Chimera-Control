#include "stdafx.h"
#include "CameraSettingsControl.h"
#include "CameraWindow.h"


std::array<int, 4> CameraSettingsControl::getThresholds()
{
	return this->picSettingsObj.getThresholds();
}


void CameraSettingsControl::handleSetTemperatureOffPress()
{
	this->andorFriend->changeTemperatureSetting(true);
	//eCameraFileSystem.updateSaveStatus(false);
}


void CameraSettingsControl::handleSetTemperaturePress()
{
	if (this->andorFriend->isRunning())
	{
		errBox("ERROR: the camera (thinks that it?) is running. You can't change temperature settings during camera "
			"operation.");
		return;
	}
	AndorRunSettings cameraSettings = this->andorFriend->getSettings();
	CString text;
	this->temperatureEdit.GetWindowTextA(text);
	double temp;
	try
	{
		temp = std::stof(std::string(text));
	}
	catch (std::invalid_argument&)
	{
		errBox("Error: Couldn't convert temperature input to a double! Check for unusual characters.");
		return;
	}
	this->runSettings.temperatureSetting = temp;
	cameraSettings.temperatureSetting = temp;

	this->andorFriend->setTemperature();
	//eCameraFileSystem.updateSaveStatus(false);
	return;
}

void CameraSettingsControl::handleTriggerControl(CameraWindow* cameraWindow)
{
	CString triggerMode;
	long long itemIndex = this->triggerCombo.GetCurSel();
	if (itemIndex == -1)
	{
		return;
	}
	this->triggerCombo.GetLBText(itemIndex, triggerMode);
	runSettings.triggerMode = triggerMode;
	CRect rect;
	cameraWindow->GetWindowRect(&rect);
	cameraWindow->OnSize(0, rect.right - rect.left, rect.bottom - rect.top);
	/// TODO.
	//eCameraFileSystem.updateSaveStatus(false);
	return;
}

AndorRunSettings CameraSettingsControl::getSettings()
{
	return this->runSettings;
}

void CameraSettingsControl::rearrange(std::string cameraMode, std::string triggerMode, int width, int height, std::unordered_map<std::string, CFont*> fonts)
{
	this->imageParametersObj.rearrange(cameraMode, triggerMode, width, height, fonts);
	this->picSettingsObj.rearrange(cameraMode, triggerMode, width, height, fonts);
	this->header.rearrange(cameraMode, triggerMode, width, height, fonts);
	this->emGainButton.rearrange(cameraMode, triggerMode, width, height, fonts);
	this->emGainDisplay.rearrange(cameraMode, triggerMode, width, height, fonts);
	this->emGainEdit.rearrange(cameraMode, triggerMode, width, height, fonts);
	this->triggerCombo.rearrange(cameraMode, triggerMode, width, height, fonts);
	this->triggerLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	setTemperatureButton.rearrange(cameraMode, triggerMode, width, height, fonts);
	temperatureOffButton.rearrange(cameraMode, triggerMode, width, height, fonts);
	temperatureEdit.rearrange(cameraMode, triggerMode, width, height, fonts);
	temperatureDisplay.rearrange(cameraMode, triggerMode, width, height, fonts);
	temperatureMessage.rearrange(cameraMode, triggerMode, width, height, fonts);
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
	catch (std::invalid_argument &exception)
	{
		errBox("ERROR: Couldn't convert EM Gain text to integer.");
		return;
	}
	// < 0 corresponds to NOT USING EM GAIN (using conventional gain).
	if (emGain < 0)
	{
		this->runSettings.emGainModeIsOn = false;
		this->runSettings.emGainLevel = 0;
		emGainDisplay.SetWindowTextA("OFF");
	}
	else
	{
		this->runSettings.emGainModeIsOn = true;
		this->runSettings.emGainLevel = emGain;
		emGainDisplay.SetWindowTextA(("Gain: X" + std::to_string(runSettings.emGainLevel)).c_str());
	}
	// Change the andor settings.
	AndorRunSettings settings = andorObj->getSettings();
	settings.emGainLevel = this->runSettings.emGainLevel;
	settings.emGainModeIsOn = this->runSettings.emGainModeIsOn;
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
	int temperature = 25;
	try
	{
		// in this case you expect it to throw.
		andorFriend->getTemperature(temperature);
		if ( ANDOR_SAFEMODE ) { thrower( "SAFEMODE" ); }
	}
	catch (my_exception& exception)
	{
		// if not stable this won't get changed.
		if (exception.whatBare() == "DRV_TEMPERATURE_STABILIZED")
		{
			currentControlColor = "Green";
			temperatureDisplay.SetWindowTextA(cstr(temperature));
			temperatureMessage.SetWindowTextA(("Temperature has stabilized at " + std::to_string(temperature) + " (C)\r\n").c_str());
		}
		else if (exception.whatBare() == "DRV_TEMPERATURE_NOT_REACHED")
		{
			currentControlColor = "Red";
			temperatureDisplay.SetWindowTextA(cstr(temperature));
			temperatureMessage.SetWindowTextA(("Current temperature is " + std::to_string(temperature) + " (C)\r\n").c_str());
		}
		else if (exception.whatBare() == "DRV_TEMPERATURE_NOT_STABILIZED")
		{
			currentControlColor = "Red";
			temperatureDisplay.SetWindowTextA(cstr(temperature));
			temperatureMessage.SetWindowTextA(("Temperature of " + std::to_string(temperature) + " (C) reached but not stable.").c_str());
		}
		else if (exception.whatBare() == "DRV_TEMPERATURE_DRIFT")
		{
			currentControlColor = "Red";
			temperatureDisplay.SetWindowTextA(cstr(temperature));
			temperatureMessage.SetWindowTextA(("Temperature had stabilized but has since drifted. Temperature: " + std::to_string(temperature)).c_str());
		}
		else if (exception.whatBare() == "DRV_TEMPERATURE_OFF")
		{
			currentControlColor = "Red";
			temperatureDisplay.SetWindowTextA(cstr(temperature));
			temperatureMessage.SetWindowTextA(("Temperature control is off. Temperature: " + std::to_string(temperature)).c_str());
		}
		else if (exception.whatBare() == "DRV_ACQUIRING")
		{
			// doesn't change color of temperature control. This way the color of the control represents the state of
			// the temperature right before the acquisition started, so that you can tell if you remembered to let it
			// completely stabilize or not.
			temperatureDisplay.SetWindowTextA(cstr(temperature));
			temperatureMessage.SetWindowTextA("Camera is Acquiring data. No Temperature updates are available.");
		}
		else if (exception.whatBare() == "SAFEMODE")
		{
			currentControlColor = "Red";
			const char * test = cstr(temperature);
			temperatureDisplay.SetWindowTextA(cstr(temperature));
			temperatureMessage.SetWindowTextA("Application is running in Safemode... No Real Temperature Data is available.");
		}
		else
		{
			currentControlColor = "Red";
			currentControlColor.c_str();
			temperatureDisplay.SetWindowTextA(cstr(temperature));
			temperatureMessage.SetWindowTextA(("Unexpected Temperature Code: " + exception.whatBare() + ". Temperature: " + std::to_string(temperature)).c_str());
		}
	}
	return;
}

void CameraSettingsControl::handlePictureSettings(UINT id, AndorCamera* andorObj)
{
	this->picSettingsObj.handleOptionChange(id, andorObj);
	this->runSettings.exposureTimes = this->picSettingsObj.getUsedExposureTimes();
	this->runSettings.totalPicsInExperiment = this->picSettingsObj.getPicsPerRepetition();
	return;
}

void CameraSettingsControl::initialize(cameraPositions& pos, int& id, CWnd* parent, std::unordered_map<std::string, CFont*> fonts, std::vector<CToolTipCtrl*>& tooltips)
{
	/// Header
	header.ksmPos = { pos.ksmPos.x, pos.ksmPos.y, pos.ksmPos.x + 480,
		pos.ksmPos.y + 25 };
	header.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 480,
		pos.amPos.y + 25 };
	header.cssmPos = { pos.cssmPos.x, pos.cssmPos.y, pos.cssmPos.x + 480,
		pos.cssmPos.y + 25 };
	header.ID = id++;
	header.Create("CAMERA SETTINGS", WS_BORDER | WS_CHILD | WS_VISIBLE | ES_READONLY | ES_CENTER, header.ksmPos,
		parent, header.ID);	
	header.fontType = "Heading";
	pos.ksmPos.y += 25;
	pos.amPos.y += 25;
	pos.cssmPos.y += 25;
	
	/// EM Gain
	emGainButton.ksmPos = { pos.ksmPos.x, pos.ksmPos.y, pos.ksmPos.x + 120, pos.ksmPos.y + 20 };
	emGainButton.cssmPos = emGainButton.amPos = emGainButton.ksmPos;
	emGainButton.ID = id++;
	if (emGainButton.ID != IDC_SET_EM_GAIN_BUTTON)
	{
		throw;
	}
	emGainButton.Create("Set EM Gain", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, emGainButton.ksmPos, parent, emGainButton.ID);
	emGainButton.fontType = "Normal";
	emGainButton.setToolTip("Set the state & gain of the EM gain of the camera. Enter a negative number to turn EM Gain"
		" mode off. The program will immediately change the state of the camera after pressing this button.", tooltips, 
		parent, fonts["Normal Font"]);
	//
	emGainEdit.ksmPos = { pos.ksmPos.x + 120, pos.ksmPos.y, pos.ksmPos.x + 300, pos.ksmPos.y + 20 };
	emGainEdit.cssmPos = emGainEdit.amPos = emGainEdit.ksmPos;
	emGainEdit.ID = id++;
	emGainEdit.Create(WS_CHILD | WS_VISIBLE | BS_RIGHT, emGainEdit.ksmPos, parent, emGainEdit.ID);
	emGainEdit.fontType = "Normal";
	//
	emGainDisplay.ksmPos = { pos.ksmPos.x + 300, pos.ksmPos.y, pos.ksmPos.x + 480, pos.ksmPos.y + 20 };
	emGainDisplay.cssmPos = emGainDisplay.amPos = emGainDisplay.ksmPos;
	emGainDisplay.ID = id++;
	emGainDisplay.Create("OFF", WS_CHILD | WS_VISIBLE | BS_RIGHT | ES_READONLY | ES_CENTER, emGainDisplay.ksmPos, parent, emGainDisplay.ID);
	emGainDisplay.fontType = "Normal";
	// initialize settings.
	runSettings.emGainLevel = 0;
	runSettings.emGainModeIsOn = false;
	//
	pos.ksmPos.y += 20;
	pos.amPos.y += 20;
	pos.cssmPos.y += 20;
	// Trigger Text

	triggerLabel.ksmPos = { pos.ksmPos.x, pos.ksmPos.y, pos.ksmPos.x + 240, pos.ksmPos.y + 25 };
 	triggerLabel.cssmPos = { pos.cssmPos.x, pos.cssmPos.y, pos.cssmPos.x + 240, pos.cssmPos.y + 25 };
 	triggerLabel.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 240, pos.amPos.y + 25 };
 	triggerLabel.ID = id++;
 	triggerLabel.Create("Trigger Mode:", WS_CHILD | WS_VISIBLE | ES_CENTER, triggerLabel.ksmPos, parent, triggerLabel.ID);
 	triggerLabel.fontType = "Normal";
 	// trigger combo
 	triggerCombo.ksmPos = { pos.ksmPos.x + 240, pos.ksmPos.y, pos.ksmPos.x + 480, pos.ksmPos.y + 800 };
 	triggerCombo.cssmPos = { pos.cssmPos.x + 240, pos.cssmPos.y,pos.cssmPos.x + 480, pos.cssmPos.y + 800 };
 	triggerCombo.amPos = { pos.amPos.x + 240, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y + 800 };
	triggerCombo.ID = id++;
	if (triggerCombo.ID != IDC_TRIGGER_COMBO)
	{
		throw;
	}
	triggerCombo.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, triggerCombo.ksmPos, parent, triggerCombo.ID);
	// set options for the combo
	triggerCombo.fontType = "Normal";
 	triggerCombo.AddString("Internal");
 	triggerCombo.AddString("External");
 	triggerCombo.AddString("Start On Trigger");
 	// Select default trigger
 	triggerCombo.SelectString(0, "External");
 	pos.ksmPos.y += 25;
	pos.amPos.y += 25;
	pos.cssmPos.y += 25;
	runSettings.triggerMode = "External";
	// Set temperature Button
	setTemperatureButton.ksmPos = { pos.ksmPos.x, pos.ksmPos.y, pos.ksmPos.x + 270, pos.ksmPos.y + 25};
	setTemperatureButton.cssmPos = setTemperatureButton.amPos = setTemperatureButton.ksmPos;
	setTemperatureButton.ID = id++;
	setTemperatureButton.Create("Set Camera Temperature (C)", WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON, setTemperatureButton.ksmPos, 
		parent, setTemperatureButton.ID);
	setTemperatureButton.fontType = "Normal";
	if (setTemperatureButton.ID != IDC_SET_TEMPERATURE_BUTTON)
	{
		throw;
	}
	// Temperature Edit
	temperatureEdit.ksmPos = { pos.ksmPos.x + 270, pos.ksmPos.y, pos.ksmPos.x + 350, pos.ksmPos.y + 25 };
	temperatureEdit.cssmPos = temperatureEdit.amPos = temperatureEdit.ksmPos;
	temperatureEdit.ID = id++;
	temperatureEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, temperatureEdit.ksmPos, parent, temperatureEdit.ID);
	temperatureEdit.SetWindowTextA("0");
	temperatureEdit.fontType = "Normal";
	// Temperature Setting Display
	temperatureDisplay.ksmPos = { pos.ksmPos.x + 350, pos.ksmPos.y, pos.ksmPos.x + 430, pos.ksmPos.y + 25 };
	temperatureDisplay.cssmPos = temperatureDisplay.amPos = temperatureDisplay.ksmPos;
	temperatureDisplay.ID = id++;
	temperatureDisplay.Create("", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY, temperatureDisplay.ksmPos, parent, temperatureDisplay.ID);
	temperatureDisplay.fontType = "Normal";
	// Temperature Control Off Button
	temperatureOffButton.ksmPos = { pos.ksmPos.x + 430, pos.ksmPos.y, pos.ksmPos.x + 480, pos.ksmPos.y + 25 };
	temperatureOffButton.cssmPos = temperatureOffButton.amPos = temperatureOffButton.ksmPos;
	temperatureOffButton.Create("OFF",
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON, temperatureOffButton.ksmPos, parent, temperatureOffButton.ID);
	temperatureOffButton.fontType = "Normal";
	pos.ksmPos.y += 25;
	// Temperature Message Display
	temperatureMessage.ksmPos = { pos.ksmPos.x, pos.ksmPos.y, pos.ksmPos.x + 480, pos.ksmPos.y + 50 };
	temperatureMessage.cssmPos = temperatureMessage.amPos = temperatureMessage.ksmPos;
	temperatureMessage.ID = id++;
	temperatureMessage.Create("Temperature control is disabled", WS_CHILD | WS_VISIBLE | SS_LEFT, temperatureMessage.ksmPos, parent, temperatureMessage.ID);
	temperatureMessage.fontType = "Normal";
	pos.ksmPos.y += 50;
	//
	picSettingsObj.initialize(pos.ksmPos, pos.cssmPos, pos.amPos, parent, id);
	imageParametersObj.initialize(pos.ksmPos, pos.amPos, pos.cssmPos, parent, false, id);
}

void CameraSettingsControl::checkTimings(std::vector<float> exposureTimes)
{
	checkTimings(runSettings.kinetiCycleTime, runSettings.accumulationTime, exposureTimes);
}

void CameraSettingsControl::checkTimings(float kineticCycleTime, float accumulationTime, 
										 std::vector<float> exposureTimes)
{
	andorFriend->checkAcquisitionTimings(kineticCycleTime, accumulationTime, exposureTimes);
}

imageParameters CameraSettingsControl::readImageParameters(CameraWindow* camWin)
{
	return this->imageParametersObj.readImageParameters(camWin);
}

CBrush* CameraSettingsControl::handleColor(int idNumber, CDC* colorer, std::unordered_map<std::string, CBrush*> brushes,
	std::unordered_map<std::string, COLORREF> rgbs)
{
	return this->picSettingsObj.colorControls(idNumber, colorer, brushes, rgbs);
}



void CameraSettingsControl::checkIfReady()
{

	if ( this->picSettingsObj.getUsedExposureTimes().size() == 0 )
	{
		thrower("Please Set at least one exposure time.");
		return;
	}
	if ( !this->imageParametersObj.checkReady() )
	{
		thrower("Please set the image parameters.");
		return;
	}
	if ( this->runSettings.picsPerRepetition <= 0 )
	{
		thrower("ERROR: Please set the number of pictures per experiment to a positive non-zero value.");
		return;
	}
	if ( this->runSettings.cameraMode == "Kinetic Series Mode" )
	{
		if ( runSettings.kinetiCycleTime == 0 && runSettings.triggerMode == "Internal" )
		{
			thrower("ERROR: Since you are running in internal trigger mode, please Set a kinetic cycle time.");
			return;
		}
		if ( runSettings.repetitionsPerVariation <= 0 )
		{
			thrower("ERROR: Please set the \"Experiments per Stack\" variable to a positive non-zero value.");
			return;
		}
		if ( runSettings.totalVariations <= 0 )
		{
			thrower("ERROR: Please set the number of accumulation stacks to a positive non-zero value.");
			return;
		}

	}
	if ( runSettings.cameraMode == "Accumulate Mode" )
	{
		if ( runSettings.totalAccumulationNumber <= 0 )
		{
		
			thrower("ERROR: Please set the current Accumulation Number to a positive non-zero value.");
			return;
		}
		if ( runSettings.accumulationTime <= 0 )
		{
			thrower("ERROR: Please set the current Accumulation Time to a positive non-zero value.");
			return;
		}
	}

	return;
}