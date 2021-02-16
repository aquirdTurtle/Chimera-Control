// created by Mark O. Brown
#pragma once
#include <Mmsystem.h>
#include <mciapi.h>
#include "PrimaryWindows/IChimeraQtWindow.h"
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
			mciSendString( cstr( str( "open \"" ) + MUSIC_LOCATION + "\" type mpegvideo alias mp3" ), nullptr, 0, nullptr );
		}
		~AlertSystem() { mciSendString( "close mp3", nullptr, 0, nullptr ); }
		void initialize( QPoint& positions, IChimeraQtWindow* parent );
		void alertMainThread( int level );
		void soundAlert();
		unsigned getAlertThreshold();
		unsigned getAlertMessageID();
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
		unsigned alertMessageID = 0;
};