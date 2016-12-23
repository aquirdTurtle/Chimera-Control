#include "stdafx.h"
#include "AlertSystem.h"
#include "reorganizeControl.h"
#include <Mmsystem.h>
#include <mciapi.h>
#pragma comment(lib, "Winmm.lib")

AlertSystem::~AlertSystem()
{
	mciSendString("close mp3", NULL, 0, NULL);
}

unsigned int AlertSystem::getAlertThreshold()
{
	return alertThreshold;
}

bool AlertSystem::setAlertThreshold()
{
	CString text;
	alertThresholdEdit.GetWindowTextA(text);
	try
	{
		alertThreshold = std::stoi(std::string(text));
	}
	catch (std::invalid_argument& exception)
	{
		MessageBox(0, "ERROR: Alert threshold must be an integer!", 0, 0);
		return true;
	}
	return false;
}

bool AlertSystem::initialize(POINT& topLeftPositionKinetic, POINT& topLeftPositionAccumulate, POINT& topLeftPositionContinuous,
	CWnd* parent, bool isTriggerModeSensitive, int& id)
{
	this->alertMessageID = RegisterWindowMessage("ID_NOT_LOADING_ATOMS");
	/// Title
	this->title.ksmPos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 25 };
	title.amPos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 25 };
	title.cssmPos = { -1,-1,-1,-1 };
	title.ID = id++;
	title.Create("ALERT SYSTEM", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, title.ksmPos, parent, title.ID);
	title.fontType = "Heading";
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	/// Use Alerts Checkbox
	this->alertsActiveCheckBox.ksmPos = { topLeftPositionKinetic.x + 0, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 160, topLeftPositionKinetic.y + 20 };
	alertsActiveCheckBox.amPos = { topLeftPositionAccumulate.x + 0, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 160, topLeftPositionAccumulate.y + 20 };
	alertsActiveCheckBox.cssmPos = { -1,-1,-1,-1 };
	alertsActiveCheckBox.ID = id++;
	alertsActiveCheckBox.Create("Use?", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_READONLY | BS_CHECKBOX, alertsActiveCheckBox.ksmPos, parent, alertsActiveCheckBox.ID);
	alertsActiveCheckBox.fontType = "Normal";
	/// Alert threshold text
	this->alertThresholdText.ksmPos = { topLeftPositionKinetic.x + 160, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 320, topLeftPositionKinetic.y + 20 };
	alertThresholdText.amPos = { topLeftPositionAccumulate.x + 160, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 320, topLeftPositionAccumulate.y + 20 };
	alertThresholdText.cssmPos = { -1,-1,-1,-1 };
	alertThresholdText.ID = id++;
	alertThresholdText.Create("Alert Threshold:", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, alertThresholdText.ksmPos, parent, alertThresholdText.ID);
	alertThresholdText.fontType = "Normal";
	/// Alert threshold edit
	this->alertThresholdEdit.ksmPos = { topLeftPositionKinetic.x + 320, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 20 };
	alertThresholdEdit.amPos = { topLeftPositionAccumulate.x + 320, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 20 };
	alertThresholdEdit.cssmPos = { -1,-1,-1,-1 };
	alertThresholdEdit.ID = id++;
	alertThresholdEdit.Create(WS_CHILD | WS_VISIBLE | ES_CENTER, alertThresholdEdit.ksmPos, parent, alertThresholdEdit.ID);
	alertThresholdEdit.SetWindowTextA("10");
	alertThresholdEdit.fontType = "Normal";
	topLeftPositionKinetic.y += 20;
	topLeftPositionAccumulate.y += 20;
	/// Sound checkbox
	// soundAtFinshCheckBox.hwnd
	this->soundAtFinshCheckBox.ksmPos = { topLeftPositionKinetic.x + 0, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 20 };
	soundAtFinshCheckBox.amPos = { topLeftPositionAccumulate.x + 0, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 20 };
	soundAtFinshCheckBox.cssmPos = { -1,-1,-1,-1 };
	soundAtFinshCheckBox.ID = id++;
	soundAtFinshCheckBox.Create("Play Sound at Finish?", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_READONLY | BS_AUTOCHECKBOX, soundAtFinshCheckBox.ksmPos,
		parent, soundAtFinshCheckBox.ID);
	soundAtFinshCheckBox.fontType = "Normal";
	topLeftPositionKinetic.y += 20;
	topLeftPositionAccumulate.y += 20;
	return false;
}

bool AlertSystem::alertMainThread(int runsWithoutAtoms)
{
	int* alertLevel = new int;
	if (runsWithoutAtoms == this->alertThreshold)
	{
		*alertLevel = 2;
		//PostMessage(eCameraWindowHandle, alertMessageID, 0, (LPARAM)alertLevel);
	}
	else if (runsWithoutAtoms % this->alertThreshold == 0)
	{
		*alertLevel = 1;
		//PostMessage(eCameraWindowHandle, alertMessageID, 0, (LPARAM)alertLevel);
	}
	// don't sound the alert EVERY time... hence the % above.
	return false;
}
bool AlertSystem::soundAlert()
{
	Beep(523, 100);
	Beep(523, 100);
	Beep(523, 100);
	return false;
}

bool AlertSystem::reorganizeControls(std::string cameraMode, std::string triggerMode, int width, int height)
{
	title.rearrange(cameraMode, triggerMode, width, height);
	alertsActiveCheckBox.rearrange(cameraMode, triggerMode, width, height);
	alertThresholdText.rearrange(cameraMode, triggerMode, width, height);
	alertThresholdEdit.rearrange(cameraMode, triggerMode, width, height);
	soundAtFinshCheckBox.rearrange(cameraMode, triggerMode, width, height);
	return false;
}

bool AlertSystem::handleCheckBoxPress(WPARAM messageWParam, LPARAM messageLParam)
{
	if (LOWORD(messageWParam) == alertsActiveCheckBox.ID)
	{
		BOOL checked = alertsActiveCheckBox.GetCheck();
		if (checked)
		{
			alertsActiveCheckBox.SetCheck(0);
			useAlerts = false;
		}
		else
		{
			alertsActiveCheckBox.SetCheck(1);
			useAlerts = true;
		}
		return false;
	}
	else
	{
		return true;
	}
}


unsigned int AlertSystem::getAlertMessageID()
{
	return alertMessageID;
}
bool AlertSystem::alertsAreToBeUsed()
{
	return useAlerts;
}

bool AlertSystem::playSound()
{
	mciSendString("play mp3 from 0", NULL, 0, NULL);
	return false;
}
bool AlertSystem::stopSound()
{
	mciSendString("stop mp3", NULL, 0, NULL);
	return false;
}
bool AlertSystem::soundIsToBePlayed()
{
	return soundAtFinshCheckBox.GetCheck();
}