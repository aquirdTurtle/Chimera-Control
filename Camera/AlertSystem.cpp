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
	char text[256];
	SendMessage(alertThresholdEdit.hwnd, WM_GETTEXT, 256, (LPARAM)text);
	try
	{
		alertThreshold = std::stoi(text);
	}
	catch (std::invalid_argument& exception)
	{
		MessageBox(0, "ERROR: Alert threshold must be an integer!", 0, 0);
		return true;
	}
	return false;
}

bool AlertSystem::initialize(POINT& topLeftPositionKinetic, POINT& topLeftPositionAccumulate, POINT& topLeftPositionContinuous,
	HWND parentWindow, bool isTriggerModeSensitive)
{
	this->alertMessageID = RegisterWindowMessage("ID_NOT_LOADING_ATOMS");
	/// Title
	this->title.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 270, topLeftPositionKinetic.y + 25 };
	title.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 270, topLeftPositionAccumulate.y + 25 };
	title.continuousSingleScansModePos = { -1,-1,-1,-1 };
	RECT initPos = title.kineticSeriesModePos;
	title.hwnd = CreateWindowEx(0, "STATIC", "Alert System", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	title.fontType = "Heading";
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	/// Use Alerts Checkbox
	this->alertsActiveCheckBox.kineticSeriesModePos = { topLeftPositionKinetic.x + 0, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 60, topLeftPositionKinetic.y + 20 };
	alertsActiveCheckBox.accumulateModePos = { topLeftPositionAccumulate.x + 0, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 60, topLeftPositionAccumulate.y + 20 };
	alertsActiveCheckBox.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = alertsActiveCheckBox.kineticSeriesModePos;
	alertsActiveCheckBox.hwnd = CreateWindowEx(0, "BUTTON", "Use?", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_READONLY | BS_CHECKBOX,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)alertsActiveCheckBox.ID, eHInst, NULL);
	alertsActiveCheckBox.fontType = "Normal";
	/// Alert threshold text
	this->alertThresholdText.kineticSeriesModePos = { topLeftPositionKinetic.x + 60, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 220, topLeftPositionKinetic.y + 20 };
	alertThresholdText.accumulateModePos = { topLeftPositionAccumulate.x + 60, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 220, topLeftPositionAccumulate.y + 20 };
	alertThresholdText.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = alertThresholdText.kineticSeriesModePos;
	alertThresholdText.hwnd = CreateWindowEx(0, "STATIC", "Alert Threshold:", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	alertThresholdText.fontType = "Normal";
	/// Alert threshold edit
	this->alertThresholdEdit.kineticSeriesModePos = { topLeftPositionKinetic.x + 220, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 270, topLeftPositionKinetic.y + 20 };
	alertThresholdEdit.accumulateModePos = { topLeftPositionAccumulate.x + 220, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 270, topLeftPositionAccumulate.y + 20 };
	alertThresholdEdit.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = alertThresholdEdit.kineticSeriesModePos;
	alertThresholdEdit.hwnd = CreateWindowEx(0, "EDIT", "10", WS_CHILD | WS_VISIBLE | ES_CENTER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)alertThresholdEdit.ID, eHInst, NULL);
	alertThresholdEdit.fontType = "Normal";
	topLeftPositionKinetic.y += 20;
	topLeftPositionAccumulate.y += 20;
	/// Sound checkbox
	//soundAtFinshCheckBox.hwnd
	this->soundAtFinshCheckBox.kineticSeriesModePos = { topLeftPositionKinetic.x + 0, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 270, topLeftPositionKinetic.y + 20 };
	soundAtFinshCheckBox.accumulateModePos = { topLeftPositionAccumulate.x + 0, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 270, topLeftPositionAccumulate.y + 20 };
	soundAtFinshCheckBox.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = soundAtFinshCheckBox.kineticSeriesModePos;
	soundAtFinshCheckBox.hwnd = CreateWindowEx(0, "BUTTON", "Play Sound at Finish?", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_READONLY | BS_AUTOCHECKBOX,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
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
		PostMessage(eCameraWindowHandle, alertMessageID, 0, (LPARAM)alertLevel);
	}
	else if (runsWithoutAtoms % this->alertThreshold == 0)
	{
		*alertLevel = 1;
		PostMessage(eCameraWindowHandle, alertMessageID, 0, (LPARAM)alertLevel);
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

bool AlertSystem::reorganizeControls(RECT parentRectangle, std::string cameraMode)
{
	reorganizeControl(title, cameraMode, parentRectangle);
	reorganizeControl(alertsActiveCheckBox, cameraMode, parentRectangle);
	reorganizeControl(alertThresholdText, cameraMode, parentRectangle);
	reorganizeControl(alertThresholdEdit, cameraMode, parentRectangle);
	reorganizeControl(soundAtFinshCheckBox, cameraMode, parentRectangle);
	return false;
}

bool AlertSystem::handleCheckBoxPress(WPARAM messageWParam, LPARAM messageLParam)
{
	if (LOWORD(messageWParam) == alertsActiveCheckBox.ID)
	{
		BOOL checked = SendMessage(alertsActiveCheckBox.hwnd, BM_GETCHECK, 0, 0);
		if (checked)
		{
			SendMessage(alertsActiveCheckBox.hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
			useAlerts = false;
		}
		else
		{

			SendMessage(alertsActiveCheckBox.hwnd, BM_SETCHECK, BST_CHECKED, 0);
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
	BOOL checked = SendMessage(soundAtFinshCheckBox.hwnd, BM_GETCHECK, 0, 0);
	return checked;
}