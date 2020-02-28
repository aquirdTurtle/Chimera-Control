// created by Mark O. Brown
#include "stdafx.h"
#include "AlertSystem.h"
#include <Mmsystem.h>
#include <mciapi.h>
#pragma comment(lib, "Winmm.lib")
#include "PrimaryWindows/AndorWindow.h"
#include "GeneralUtilityFunctions/miscCommonFunctions.h"
#include <boost/lexical_cast.hpp>

void AlertSystem::initialize( POINT& pos, CWnd* parent, bool isTriggerModeSensitive, int& id, cToolTips& tooltips )
{
	alertMessageID = RegisterWindowMessage( "ID_NOT_LOADING_ATOMS" );

	title.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	title.Create( "ALERT SYSTEM", NORM_HEADER_OPTIONS, title.sPos, parent, id++ );
	title.fontType = fontTypes::HeadingFont;
	
	atomsAlertActiveCheckBox.sPos = { pos.x, pos.y, pos.x + 120, pos.y + 20 };
	atomsAlertActiveCheckBox.Create( "If No Atoms?", NORM_CHECK_OPTIONS, atomsAlertActiveCheckBox.sPos, parent, id++ );
	atomsAlertActiveCheckBox.SetCheck( false );
	
	motAlertActiveCheckBox.sPos = { pos.x + 120, pos.y, pos.x + 240, pos.y + 20 };
	motAlertActiveCheckBox.Create ( "If No MOT?", NORM_CHECK_OPTIONS, motAlertActiveCheckBox.sPos, parent, id++ );
	motAlertActiveCheckBox.SetCheck ( true );
	
	alertThresholdText.sPos = { pos.x+240, pos.y, pos.x + 360, pos.y + 20 };
	alertThresholdText.Create( "Alert Threshold:", NORM_STATIC_OPTIONS, alertThresholdText.sPos, parent, id++ );
	
	alertThresholdEdit.sPos = { pos.x + 320, pos.y, pos.x + 480, pos.y += 20 };
	alertThresholdEdit.Create( NORM_EDIT_OPTIONS, alertThresholdEdit.sPos, parent, id++ );
	alertThresholdEdit.SetWindowTextA( "10" );

	autoPauseAtAlert.sPos = { pos.x, pos.y, pos.x + 240, pos.y + 20 };
	autoPauseAtAlert.Create( "Automatically Pause on alert?", NORM_CHECK_OPTIONS, autoPauseAtAlert.sPos, parent, id++ );
	autoPauseAtAlert.SetCheck( true );
	/// Sound checkbox
	soundAtFinishCheck.sPos = { pos.x + 240, pos.y, pos.x + 480, pos.y += 20 };
	soundAtFinishCheck.Create( "Play Sound at Finish?", NORM_CHECK_OPTIONS, soundAtFinishCheck.sPos, parent, id++ );
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
		throwNested ( "ERROR: alert threshold failed to reduce to unsigned long!" );
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
		throwNested ( "ERROR: Alert threshold must be an integer!" );
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


void AlertSystem::rearrange( int width, int height, fontMap fonts)
{
	autoPauseAtAlert.rearrange(  width, height, fonts );
	title.rearrange(width, height, fonts);
	atomsAlertActiveCheckBox.rearrange(width, height, fonts);
	alertThresholdText.rearrange(width, height, fonts);
	alertThresholdEdit.rearrange(width, height, fonts);
	soundAtFinishCheck.rearrange(width, height, fonts);
	motAlertActiveCheckBox.rearrange ( width, height, fonts );
}


UINT AlertSystem::getAlertMessageID()
{
	return alertMessageID;
}


bool AlertSystem::wantsAtomAlerts()
{
	return atomsAlertActiveCheckBox.GetCheck();
}


bool AlertSystem::wantsMotAlerts ( )
{
	return motAlertActiveCheckBox.GetCheck ( );
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
	return soundAtFinishCheck.GetCheck();
}