#include "stdafx.h"
#include "AlertSystem.h"
#include "reorganizeControl.h"
#include <Mmsystem.h>
#include <mciapi.h>
#pragma comment(lib, "Winmm.lib")
#include "CameraWindow.h"



unsigned int AlertSystem::getAlertThreshold()
{
	return alertThreshold;
}


void AlertSystem::setAlertThreshold()
{
	CString text;
	alertThresholdEdit.GetWindowTextA( text );
	try
	{
		alertThreshold = std::stoi( std::string( text ) );
	}
	catch (std::invalid_argument& )
	{
		thrower( "ERROR: Alert threshold must be an integer!" );
	}
}


void AlertSystem::initialize(cameraPositions& pos, CWnd* parent, bool isTriggerModeSensitive, int& id, 
							  std::unordered_map<std::string, CFont*> fonts, std::vector<CToolTipCtrl*>& tooltips)
{
	alertMessageID = RegisterWindowMessage("ID_NOT_LOADING_ATOMS");
	/// Title
	title.ksmPos = { pos.ksmPos.x, pos.ksmPos.y, pos.ksmPos.x + 480, pos.ksmPos.y + 25 };
	title.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y + 25 };
	title.cssmPos = { -1,-1,-1,-1 };
	title.ID = id++;
	title.triggerModeSensitive = isTriggerModeSensitive;
	title.Create("ALERT SYSTEM", WS_BORDER | WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, title.ksmPos, parent, title.ID);
	title.fontType = "Heading";
	pos.ksmPos.y += 25;
	pos.amPos.y += 25;
	/// Use Alerts Checkbox
	alertsActiveCheckBox.ksmPos = { pos.ksmPos.x + 0, pos.ksmPos.y, pos.ksmPos.x + 160, pos.ksmPos.y + 20 };
	alertsActiveCheckBox.amPos = { pos.amPos.x + 0, pos.amPos.y, pos.amPos.x + 160, pos.amPos.y + 20 };
	alertsActiveCheckBox.cssmPos = { -1,-1,-1,-1 };
	alertsActiveCheckBox.ID = id++;
	alertsActiveCheckBox.triggerModeSensitive = isTriggerModeSensitive;
	if (alertsActiveCheckBox.ID != IDC_ALERTS_BOX)
	{
		throw;
	}
	alertsActiveCheckBox.Create("Use?", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_READONLY | BS_CHECKBOX, alertsActiveCheckBox.ksmPos, parent, alertsActiveCheckBox.ID);
	alertsActiveCheckBox.fontType = "Normal";
	/// Alert threshold text
	alertThresholdText.ksmPos = { pos.ksmPos.x + 160, pos.ksmPos.y, pos.ksmPos.x + 320, pos.ksmPos.y + 20 };
	alertThresholdText.amPos = { pos.amPos.x + 160, pos.amPos.y, pos.amPos.x + 320, pos.amPos.y + 20 };
	alertThresholdText.cssmPos = { -1,-1,-1,-1 };
	alertThresholdText.ID = id++;
	alertThresholdText.triggerModeSensitive = isTriggerModeSensitive;
	alertThresholdText.Create("Alert Threshold:", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, alertThresholdText.ksmPos, parent, alertThresholdText.ID);
	alertThresholdText.fontType = "Normal";
	/// Alert threshold edit
	alertThresholdEdit.ksmPos = { pos.ksmPos.x + 320, pos.ksmPos.y, pos.ksmPos.x + 480, pos.ksmPos.y + 20 };
	alertThresholdEdit.amPos = { pos.amPos.x + 320, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y + 20 };
	alertThresholdEdit.cssmPos = { -1,-1,-1,-1 };
	alertThresholdEdit.ID = id++;
	alertThresholdEdit.triggerModeSensitive = isTriggerModeSensitive;
	alertThresholdEdit.Create(WS_CHILD | WS_VISIBLE | ES_CENTER, alertThresholdEdit.ksmPos, parent, alertThresholdEdit.ID);
	alertThresholdEdit.SetWindowTextA("10");
	alertThresholdEdit.fontType = "Normal";
	pos.ksmPos.y += 20;
	pos.amPos.y += 20;
	/// Sound checkbox
	// soundAtFinshCheckBox.hwnd
	soundAtFinshCheckBox.ksmPos = { pos.ksmPos.x + 0, pos.ksmPos.y, pos.ksmPos.x + 480, pos.ksmPos.y + 20 };
	soundAtFinshCheckBox.amPos = { pos.amPos.x + 0, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y + 20 };
	soundAtFinshCheckBox.cssmPos = { -1,-1,-1,-1 };
	soundAtFinshCheckBox.ID = id++;
	soundAtFinshCheckBox.triggerModeSensitive = isTriggerModeSensitive;
	soundAtFinshCheckBox.Create("Play Sound at Finish?", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_READONLY | BS_AUTOCHECKBOX, soundAtFinshCheckBox.ksmPos,
		parent, soundAtFinshCheckBox.ID);
	soundAtFinshCheckBox.fontType = "Normal";
	pos.ksmPos.y += 20;
	pos.amPos.y += 20;
}

void AlertSystem::alertMainThread(int runsWithoutAtoms)
{
	int* alertLevel = new int;
	if (runsWithoutAtoms == alertThreshold)
	{
		*alertLevel = 2;
		//PostMessage(eCameraWindowHandle, alertMessageID, 0, (LPARAM)alertLevel);
	}
	else if (runsWithoutAtoms % alertThreshold == 0)
	{
		*alertLevel = 1;
		//PostMessage(eCameraWindowHandle, alertMessageID, 0, (LPARAM)alertLevel);
	}
	// don't sound the alert EVERY time... hence the % above.
}


void AlertSystem::soundAlert()
{
	Beep(523, 100);
	Beep(523, 100);
	Beep(523, 100);
	return ;
}


void AlertSystem::rearrange(std::string cameraMode, std::string triggerMode, int width, int height, 
							 std::unordered_map<std::string, CFont*> fonts)
{
	title.rearrange(cameraMode, triggerMode, width, height, fonts);
	alertsActiveCheckBox.rearrange(cameraMode, triggerMode, width, height, fonts);
	alertThresholdText.rearrange(cameraMode, triggerMode, width, height, fonts);
	alertThresholdEdit.rearrange(cameraMode, triggerMode, width, height, fonts);
	soundAtFinshCheckBox.rearrange(cameraMode, triggerMode, width, height, fonts);
}


void AlertSystem::handleCheckBoxPress()
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
}


unsigned int AlertSystem::getAlertMessageID()
{
	return alertMessageID;
}


bool AlertSystem::alertsAreToBeUsed()
{
	return useAlerts;
}


void AlertSystem::playSound()
{
	mciSendString("play mp3 from 0", NULL, 0, NULL);
}


void AlertSystem::stopSound()
{
	mciSendString("stop mp3", NULL, 0, NULL);
}


bool AlertSystem::soundIsToBePlayed()
{
	return soundAtFinshCheckBox.GetCheck();
}