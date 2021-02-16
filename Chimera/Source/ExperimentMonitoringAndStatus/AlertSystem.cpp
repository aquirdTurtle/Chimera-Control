// created by Mark O. Brown
#include "stdafx.h"
#include "AlertSystem.h"
#include <Mmsystem.h>
#include <mciapi.h>
#pragma comment(lib, "Winmm.lib")
#include "PrimaryWindows/QtAndorWindow.h"
#include "GeneralUtilityFunctions/miscCommonFunctions.h"
#include <boost/lexical_cast.hpp>


void AlertSystem::initialize( QPoint& pos, IChimeraQtWindow* parent ){
	auto& px = pos.rx (), & py = pos.ry ();

	title = new QLabel ("ALERT SYSTEM", parent);
	title->setGeometry (px, py, 480, 25);
	
	atomsAlertActiveCheckBox = new QCheckBox ("If No Atoms?", parent);
	atomsAlertActiveCheckBox->setGeometry (px, py+=25, 120, 20);
	atomsAlertActiveCheckBox->setChecked( false );
	
	motAlertActiveCheckBox = new QCheckBox ("If No MOT?", parent);
	motAlertActiveCheckBox->setGeometry (px + 120, py, 120, 20);
	motAlertActiveCheckBox->setChecked ( true );
	
	alertThresholdText = new QLabel ("Alert Threshold:", parent);
	alertThresholdText->setGeometry (px + 240, py, 120, 20);
	
	alertThresholdEdit = new QLineEdit ("10", parent);
	alertThresholdEdit->setGeometry (px + 360, py, 120, 20);

	autoPauseAtAlert = new QCheckBox ("Automatically Pause on Alert?", parent);
	autoPauseAtAlert->setGeometry (px, py += 20, 240, 20);
	autoPauseAtAlert->setChecked ( true );
	/// Sound checkbox
	soundAtFinishCheck = new QCheckBox ("Play Sound at Finish?", parent);
	soundAtFinishCheck->setGeometry (px + 240, py, 240, 20);
	py += 20;
}


bool AlertSystem::wantsAutoPause( )
{
	return autoPauseAtAlert->isChecked( );
}


unsigned AlertSystem::getAlertThreshold()
{
	try
	{
		alertThreshold = boost::lexical_cast<unsigned long>( str(alertThresholdEdit->text()) );
	}
	catch ( boost::bad_lexical_cast& )
	{
		throwNested ( "ERROR: alert threshold failed to reduce to unsigned long!" );
	}
	return alertThreshold;
}


void AlertSystem::setAlertThreshold()
{
	try
	{
		alertThreshold = boost::lexical_cast<int>( str(alertThresholdEdit->text()) );
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
	//Beep(523, 100);
	//Beep(523, 100);
	//Beep(523, 100);
}




unsigned AlertSystem::getAlertMessageID()
{
	return alertMessageID;
}


bool AlertSystem::wantsAtomAlerts()
{
	return atomsAlertActiveCheckBox->isChecked();
}


bool AlertSystem::wantsMotAlerts ( )
{
	return motAlertActiveCheckBox->isChecked();
}


void AlertSystem::playSound()
{
	mciSendString("play mp3 from 0", nullptr, 0, nullptr);
}


void AlertSystem::stopSound()
{
	mciSendString("stop mp3", nullptr, 0, nullptr);
}


bool AlertSystem::soundIsToBePlayed()
{
	return soundAtFinishCheck->isChecked();
}