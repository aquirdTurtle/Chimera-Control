#include "stdafx.h"
#include "AlertSystem.h"
#include <Mmsystem.h>
#include <mciapi.h>
#pragma comment(lib, "Winmm.lib")
#include "CameraWindow.h"
#include "miscCommonFunctions.h"
#include "Thrower.h"
#include <boost/lexical_cast.hpp>

void AlertSystem::initialize( cameraPositions& pos, CWnd* parent, bool isTriggerModeSensitive, int& id,
							  cToolTips& tooltips )
{
	alertMessageID = RegisterWindowMessage( "ID_NOT_LOADING_ATOMS" );
	/// Title
	title.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	title.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	title.videoPos = { -1,-1,-1,-1 };
	title.triggerModeSensitive = isTriggerModeSensitive;
	title.Create( "ALERT SYSTEM", NORM_HEADER_OPTIONS, title.seriesPos, parent, id++ );
	title.fontType = fontTypes::HeadingFont;
	/// Use Alerts Checkbox
	alertsActiveCheckBox.seriesPos = { pos.seriesPos.x + 0, pos.seriesPos.y, pos.seriesPos.x + 160, pos.seriesPos.y + 20 };
	alertsActiveCheckBox.amPos = { pos.amPos.x + 0, pos.amPos.y, pos.amPos.x + 160, pos.amPos.y + 20 };
	alertsActiveCheckBox.videoPos = { -1,-1,-1,-1 };
	alertsActiveCheckBox.triggerModeSensitive = isTriggerModeSensitive;
	alertsActiveCheckBox.Create( "Use?", NORM_CHECK_OPTIONS, alertsActiveCheckBox.seriesPos, parent, IDC_ALERTS_BOX );
	alertsActiveCheckBox.SetCheck( true );
	/// Alert threshold text
	alertThresholdText.seriesPos = { pos.seriesPos.x + 160, pos.seriesPos.y, pos.seriesPos.x + 320, pos.seriesPos.y + 20 };
	alertThresholdText.amPos = { pos.amPos.x + 160, pos.amPos.y, pos.amPos.x + 320, pos.amPos.y + 20 };
	alertThresholdText.videoPos = { -1,-1,-1,-1 };
	alertThresholdText.triggerModeSensitive = isTriggerModeSensitive;
	alertThresholdText.Create( "Alert Threshold:", NORM_STATIC_OPTIONS, alertThresholdText.seriesPos, parent, id++ );
	/// Alert threshold edit
	alertThresholdEdit.seriesPos = { pos.seriesPos.x + 320, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 20 };
	alertThresholdEdit.amPos = { pos.amPos.x + 320, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 20 };
	alertThresholdEdit.videoPos = { -1,-1,-1,-1 };
	alertThresholdEdit.triggerModeSensitive = isTriggerModeSensitive;
	alertThresholdEdit.Create( NORM_EDIT_OPTIONS, alertThresholdEdit.seriesPos, parent, id++ );
	alertThresholdEdit.SetWindowTextA( "10" );

	autoPauseAtAlert.seriesPos = { pos.seriesPos.x + 0, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 20 };
	autoPauseAtAlert.amPos = { pos.amPos.x + 0, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 20 };
	autoPauseAtAlert.videoPos = { -1,-1,-1,-1 };
	autoPauseAtAlert.triggerModeSensitive = isTriggerModeSensitive;
	autoPauseAtAlert.Create( "Automatically Pause on alert?", NORM_CHECK_OPTIONS, autoPauseAtAlert.seriesPos, parent,
							 id++ );
	autoPauseAtAlert.SetCheck( true );
	/// Sound checkbox
	soundAtFinshCheck.seriesPos = { pos.seriesPos.x + 0, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 20 };
	soundAtFinshCheck.amPos = { pos.amPos.x + 0, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 20 };
	soundAtFinshCheck.videoPos = { -1,-1,-1,-1 };
	soundAtFinshCheck.triggerModeSensitive = isTriggerModeSensitive;
	soundAtFinshCheck.Create( "Play Sound at Finish?", NORM_CHECK_OPTIONS, soundAtFinshCheck.seriesPos, parent, id++ );
}


bool AlertSystem::wantsAutoPause( )
{
	return autoPauseAtAlert.GetCheck( );
}


UINT AlertSystem::getAlertThreshold()
{
	CString txt;
	alertThresholdEdit.GetWindowTextA( txt );
	try
	{
		alertThreshold = boost::lexical_cast<unsigned long>( str(txt) );
	}
	catch ( boost::bad_lexical_cast& )
	{
		thrower( "ERROR: alert threshold failed to reduce to unsigned long!" );
	}
	return alertThreshold;
}


void AlertSystem::setAlertThreshold()
{
	CString text;
	alertThresholdEdit.GetWindowTextA( text );
	try
	{
		alertThreshold = boost::lexical_cast<int>( str( text ) );
	}
	catch ( boost::bad_lexical_cast& )
	{
		thrower( "ERROR: Alert threshold must be an integer!" );
	}
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
}


void AlertSystem::rearrange(AndorRunModes cameraMode, AndorTriggerMode triggerMode, int width, int height, 
							 fontMap fonts)
{
	autoPauseAtAlert.rearrange( cameraMode, triggerMode, width, height, fonts );
	title.rearrange(cameraMode, triggerMode, width, height, fonts);
	alertsActiveCheckBox.rearrange(cameraMode, triggerMode, width, height, fonts);
	alertThresholdText.rearrange(cameraMode, triggerMode, width, height, fonts);
	alertThresholdEdit.rearrange(cameraMode, triggerMode, width, height, fonts);
	soundAtFinshCheck.rearrange(cameraMode, triggerMode, width, height, fonts);
}


UINT AlertSystem::getAlertMessageID()
{
	return alertMessageID;
}


bool AlertSystem::alertsAreToBeUsed()
{
	return alertsActiveCheckBox.GetCheck();
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
	return soundAtFinshCheck.GetCheck();
}