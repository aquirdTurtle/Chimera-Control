// created by Mark O. Brown
#pragma once
#include "CustomMfcControlWrappers/myButton.h"
#include "CustomMfcControlWrappers/Control.h"
#include <Mmsystem.h>
#include <mciapi.h>
#include "Andor/cameraPositions.h"
#include "PrimaryWindows/IChimeraWindowWidget.h"
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>

#pragma comment(lib, "Winmm.lib")

class AlertSystem
{
	public:
		AlertSystem() : alertMessageID{ 0 }
		{
			// load the music!
			mciSendString( cstr( str( "open \"" ) + MUSIC_LOCATION + "\" type mpegvideo alias mp3" ), NULL, 0, NULL );
		}
		~AlertSystem() { mciSendString( "close mp3", NULL, 0, NULL ); }
		void initialize( POINT& positions, IChimeraWindowWidget* parent );
		void alertMainThread( int level );
		void soundAlert();
		void rearrange( int width, int height, fontMap fonts );
		UINT getAlertThreshold();
		UINT getAlertMessageID();
		void setAlertThreshold();
		bool wantsAtomAlerts();
		bool wantsMotAlerts ( );
		bool soundIsToBePlayed();
		void playSound();
		void stopSound();
		bool wantsAutoPause( );
	private:
		QLabel* title;
		QCheckBox* atomsAlertActiveCheckBox;
		QCheckBox* motAlertActiveCheckBox;
		QLabel* alertThresholdText;
		QLineEdit* alertThresholdEdit;
		QCheckBox* soundAtFinishCheck;
		QCheckBox* autoPauseAtAlert;
		int alertThreshold=-1;
		bool useAlerts=false;
		bool autoPause=false;
		UINT alertMessageID = 0;
};