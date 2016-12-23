#include "stdafx.h"
#include "CameraSettingsControl.h"
#include "CameraWindow.h"

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

void CameraSettingsControl::initialize(cameraPositions& pos, int& id, CWnd* parent)
{
	/// Header
	header.ksmPos = { pos.ksmPos.x, pos.ksmPos.y, pos.ksmPos.x + 480,
		pos.ksmPos.y + 25 };
	header.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 480,
		pos.amPos.y + 25 };
	header.cssmPos = { pos.cssmPos.x, pos.cssmPos.y, pos.cssmPos.x + 480,
		pos.cssmPos.y + 25 };
	header.ID = id++;
	header.Create("CAMERA SETTINGS", WS_CHILD | WS_VISIBLE | ES_READONLY | ES_CENTER, header.ksmPos,
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
	// Temperature Edit
	temperatureEdit.ksmPos = { pos.ksmPos.x + 270, pos.ksmPos.y, pos.ksmPos.x + 350, pos.ksmPos.y + 25 };
	temperatureEdit.cssmPos = temperatureEdit.amPos = temperatureEdit.ksmPos;
	temperatureEdit.ID = id++;
	temperatureEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, temperatureEdit.ksmPos, parent, temperatureEdit.ID);
	temperatureEdit.SetWindowTextA("0");
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