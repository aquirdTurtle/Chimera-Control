#include "stdafx.h"
#include "AlertSystem.h"
#include <Mmsystem.h>
#include <mciapi.h>
#pragma comment(lib, "Winmm.lib")
#include "CameraWindow.h"
#include "miscellaneousCommonFunctions.h"


UINT AlertSystem::getAlertThreshold()
{
	return alertThreshold;
}


void AlertSystem::setAlertThreshold()
{
	CString text;
	alertThresholdEdit.GetWindowTextA( text );
	try
	{
		alertThreshold = std::stoi( str( text ) );
	}
	catch (std::invalid_argument& )
	{
		thrower( "ERROR: Alert threshold must be an integer!" );
	}
}


void AlertSystem::initialize( cameraPositions& pos, CWnd* parent, bool isTriggerModeSensitive, int& id, 
							  cToolTips& tooltips )
{
	alertMessageID = RegisterWindowMessage("ID_NOT_LOADING_ATOMS");
	/// Title
	title.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	title.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	title.videoPos = { -1,-1,-1,-1 };
	title.triggerModeSensitive = isTriggerModeSensitive;
	title.Create("ALERT SYSTEM", WS_BORDER | WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, title.seriesPos, 
				 parent, id++);
	title.fontType = HeadingFont;
	/// Use Alerts Checkbox
	alertsActiveCheckBox.seriesPos = { pos.seriesPos.x + 0, pos.seriesPos.y, pos.seriesPos.x + 160, pos.seriesPos.y + 20 };
	alertsActiveCheckBox.amPos = { pos.amPos.x + 0, pos.amPos.y, pos.amPos.x + 160, pos.amPos.y + 20 };
	alertsActiveCheckBox.videoPos = { -1,-1,-1,-1 };
	alertsActiveCheckBox.triggerModeSensitive = isTriggerModeSensitive;
	alertsActiveCheckBox.Create( "Use?", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_READONLY | BS_CHECKBOX, 
								 alertsActiveCheckBox.seriesPos, parent, IDC_ALERTS_BOX );

	/// Alert threshold text
	alertThresholdText.seriesPos = { pos.seriesPos.x + 160, pos.seriesPos.y, pos.seriesPos.x + 320, pos.seriesPos.y + 20 };
	alertThresholdText.amPos = { pos.amPos.x + 160, pos.amPos.y, pos.amPos.x + 320, pos.amPos.y + 20 };
	alertThresholdText.videoPos = { -1,-1,-1,-1 };
	alertThresholdText.triggerModeSensitive = isTriggerModeSensitive;
	alertThresholdText.Create( "Alert Threshold:", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, 
							   alertThresholdText.seriesPos, parent, id++);
	/// Alert threshold edit
	alertThresholdEdit.seriesPos = { pos.seriesPos.x + 320, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y + 20 };
	alertThresholdEdit.amPos = { pos.amPos.x + 320, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y + 20 };
	alertThresholdEdit.videoPos = { -1,-1,-1,-1 };
	alertThresholdEdit.triggerModeSensitive = isTriggerModeSensitive;
	alertThresholdEdit.Create(WS_CHILD | WS_VISIBLE | ES_CENTER, alertThresholdEdit.seriesPos, parent, id++);
	alertThresholdEdit.SetWindowTextA("10");
	pos.seriesPos.y += 20;
	pos.amPos.y += 20;
	/// Sound checkbox
	// soundAtFinshCheckBox.hwnd
	soundAtFinshCheckBox.seriesPos = { pos.seriesPos.x + 0, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y + 20 };
	soundAtFinshCheckBox.amPos = { pos.amPos.x + 0, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y + 20 };
	soundAtFinshCheckBox.videoPos = { -1,-1,-1,-1 };
	soundAtFinshCheckBox.triggerModeSensitive = isTriggerModeSensitive;
	soundAtFinshCheckBox.Create( "Play Sound at Finish?", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_READONLY 
								| BS_AUTOCHECKBOX, soundAtFinshCheckBox.seriesPos, parent, id++);
	pos.seriesPos.y += 20;
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
							 fontMap fonts)
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


UINT AlertSystem::getAlertMessageID()
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