#include "stdafx.h"
#include "CameraSettingsControl.h"

AndorRunSettings CameraSettingsControl::getSettings()
{
	return this->runSettings;
}

void CameraSettingsControl::reorganizeControls(std::string cameraMode, std::string triggerMode, int width, int height)
{
	this->imageParametersObj.reorganizeControls(cameraMode, triggerMode, width, height);
	this->picSettingsObj.reorganizeControls(cameraMode, triggerMode, width, height);
	this->header.rearrange(cameraMode, triggerMode, width, height);
	this->emGainButton.rearrange(cameraMode, triggerMode, width, height);
	this->emGainDisplay.rearrange(cameraMode, triggerMode, width, height);
	this->emGainEdit.rearrange(cameraMode, triggerMode, width, height);
	this->triggerCombo.rearrange(cameraMode, triggerMode, width, height);
	this->triggerLabel.rearrange(cameraMode, triggerMode, width, height);
	setTemperatureButton.rearrange(cameraMode, triggerMode, width, height);
	temperatureOffButton.rearrange(cameraMode, triggerMode, width, height);
	temperatureEdit.rearrange(cameraMode, triggerMode, width, height);
	temperatureDisplay.rearrange(cameraMode, triggerMode, width, height);
	temperatureMessage.rearrange(cameraMode, triggerMode, width, height);
}
void CameraSettingsControl::setEmGain(AndorCamera* andorObj, Communicator* comm)
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
	if (andorObj->setGainMode(comm) != 0)
	{
		return;
	}
	/// TODO
	//eCameraFileSystem.updateSaveStatus(false);
	emGainEdit.RedrawWindow();
}

void CameraSettingsControl::initialize(POINT& ksmPos, POINT& cssmPos, POINT& amPos, int& id, CWnd* parent)
{
	/// Header
	header.ksmPos = { ksmPos.x, ksmPos.y, ksmPos.x + 480,
		ksmPos.y + 25 };
	header.amPos = { amPos.x, amPos.y, amPos.x + 480,
		amPos.y + 25 };
	header.cssmPos = { cssmPos.x, cssmPos.y, cssmPos.x + 480,
		cssmPos.y + 25 };
	header.ID = id++;
	header.Create("CAMERA SETTINGS", WS_CHILD | WS_VISIBLE | ES_READONLY | ES_CENTER, header.ksmPos,
		parent, header.ID);
	header.fontType = "Heading";
	ksmPos.y += 25;
	amPos.y += 25;
	cssmPos.y += 25;


	/// EM Gain
	emGainButton.ksmPos = { ksmPos.x, ksmPos.y, ksmPos.x + 120, ksmPos.y + 20 };
	emGainButton.cssmPos = emGainButton.amPos = emGainButton.ksmPos;
	emGainButton.ID = id++;
	if (emGainButton.ID != IDC_SET_EM_GAIN_BUTTON)
	{
		throw;
	}
	emGainButton.Create("Set EM Gain", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, emGainButton.ksmPos, parent, emGainButton.ID);
	emGainButton.fontType = "Normal";
	//
	emGainEdit.ksmPos = { ksmPos.x + 120, ksmPos.y, ksmPos.x + 300, ksmPos.y + 20 };
	emGainEdit.cssmPos = emGainEdit.amPos = emGainEdit.ksmPos;
	emGainEdit.ID = id++;
	emGainEdit.Create(WS_CHILD | WS_VISIBLE | BS_RIGHT, emGainEdit.ksmPos, parent, emGainEdit.ID);
	emGainEdit.fontType = "Normal";
	//
	emGainDisplay.ksmPos = { ksmPos.x + 300, ksmPos.y, ksmPos.x + 480, ksmPos.y + 20 };
	emGainDisplay.cssmPos = emGainDisplay.amPos = emGainDisplay.ksmPos;
	emGainDisplay.ID = id++;
	emGainDisplay.Create("OFF", WS_CHILD | WS_VISIBLE | BS_RIGHT | ES_READONLY | ES_CENTER, emGainDisplay.ksmPos, parent, emGainDisplay.ID);
	emGainDisplay.fontType = "Normal";
	// initialize settings.
	runSettings.emGainLevel = 0;
	runSettings.emGainModeIsOn = false;
	//
	ksmPos.y += 20;
	amPos.y += 20;
	cssmPos.y += 20;
	// Trigger Text

	triggerLabel.ksmPos = { ksmPos.x, ksmPos.y, ksmPos.x + 240, ksmPos.y + 25 };
	triggerLabel.cssmPos = { cssmPos.x, cssmPos.y, cssmPos.x + 240, cssmPos.y + 25 };
	triggerLabel.amPos = { amPos.x, amPos.y, amPos.x + 240, amPos.y + 25 };
	triggerLabel.ID = id++;
	triggerLabel.Create("Trigger Mode:", WS_CHILD | WS_VISIBLE | ES_CENTER, triggerLabel.ksmPos, parent, triggerLabel.ID);
	triggerLabel.fontType = "Normal";
	// trigger combo
	triggerCombo.ksmPos = { ksmPos.x + 240, ksmPos.y, ksmPos.x + 480, ksmPos.y + 800 };
	triggerCombo.cssmPos = { cssmPos.x + 240, cssmPos.y,cssmPos.x + 480, cssmPos.y + 800 };
	triggerCombo.amPos = { amPos.x + 240, amPos.y, amPos.x + 480, amPos.y + 800 };
	triggerCombo.ID = id++;
	triggerCombo.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, triggerCombo.ksmPos, parent, triggerCombo.ID);
	// set options for the combo
	triggerCombo.fontType = "Normal";
	triggerCombo.AddString("Internal");
	triggerCombo.AddString("External");
	triggerCombo.AddString("Start On Trigger");
	// Select default trigger
	triggerCombo.SelectString(0, "External");
	ksmPos.y += 25;
	amPos.y += 25;
	cssmPos.y += 25;
	runSettings.triggerMode = "External";
	// Set temperature Button
	setTemperatureButton.ksmPos = { ksmPos.x, ksmPos.y, ksmPos.x + 270, ksmPos.y + 25};
	setTemperatureButton.cssmPos = setTemperatureButton.amPos = setTemperatureButton.ksmPos;
	setTemperatureButton.ID = id++;
	setTemperatureButton.Create("Set Camera Temperature (C)", WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON, setTemperatureButton.ksmPos, 
		parent, setTemperatureButton.ID);
	setTemperatureButton.fontType = "Normal";
	// Temperature Edit
	temperatureEdit.ksmPos = { ksmPos.x + 270, ksmPos.y, ksmPos.x + 350, ksmPos.y + 25 };
	temperatureEdit.cssmPos = temperatureEdit.amPos = temperatureEdit.ksmPos;
	temperatureEdit.ID = id++;
	temperatureEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, temperatureEdit.ksmPos, parent, temperatureEdit.ID);
	temperatureEdit.SetWindowTextA("0");
	// Temperature Setting Display
	temperatureDisplay.ksmPos = { ksmPos.x + 350, ksmPos.y, ksmPos.x + 430, ksmPos.y + 25 };
	temperatureDisplay.cssmPos = temperatureDisplay.amPos = temperatureDisplay.ksmPos;
	temperatureDisplay.ID = id++;
	temperatureDisplay.Create("", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY, temperatureDisplay.ksmPos, parent, temperatureDisplay.ID);
	temperatureDisplay.fontType = "Normal";
	// Temperature Control Off Button
	temperatureOffButton.ksmPos = { ksmPos.x + 430, ksmPos.y, ksmPos.x + 480, ksmPos.y + 25 };
	temperatureOffButton.cssmPos = temperatureOffButton.amPos = temperatureOffButton.ksmPos;
	temperatureOffButton.Create("OFF",
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON, temperatureOffButton.ksmPos, parent, temperatureOffButton.ID);
	temperatureOffButton.fontType = "Normal";
	ksmPos.y += 25;
	// Temperature Message Display
	temperatureMessage.ksmPos = { ksmPos.x, ksmPos.y, ksmPos.x + 480, ksmPos.y + 50 };
	temperatureMessage.cssmPos = temperatureMessage.amPos = temperatureMessage.ksmPos;
	temperatureMessage.ID = id++;
	temperatureMessage.Create("Temperature control is disabled", WS_CHILD | WS_VISIBLE | SS_LEFT, temperatureMessage.ksmPos, parent, temperatureMessage.ID);
	temperatureMessage.fontType = "Normal";
	ksmPos.y += 50;
	//
	picSettingsObj.initialize(ksmPos, cssmPos, amPos, parent, id);
	imageParametersObj.initialize(ksmPos, amPos, cssmPos, parent, false, id);
}
void CameraSettingsControl::checkTimings(std::vector<float> exposureTimes, Communicator* comm)
{
	checkTimings(runSettings.kinetiCycleTime, runSettings.accumulationTime, exposureTimes, comm);
}
void CameraSettingsControl::checkTimings(float kineticCycleTime, float accumulationTime, 
										 std::vector<float> exposureTimes, Communicator* comm)
{
	andorFriend->checkAcquisitionTimings(kineticCycleTime, accumulationTime, exposureTimes, comm);
}

imageParameters CameraSettingsControl::readImageParameters(CameraWindow* camWin, Communicator* comm)
{
	return this->imageParametersObj.readImageParameters(camWin, comm);
}