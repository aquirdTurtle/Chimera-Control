#pragma once
#include "Control.h"
#include <Mmsystem.h>
#include <mciapi.h>
#include "cameraPositions.h"
#pragma comment(lib, "Winmm.lib")

class AlertSystem
{
	public:
		AlertSystem() : alertMessageID{ 0 }
		{
			// load the music!
			mciSendString( cstr( str( "open \"" ) + MUSIC_LOCATION + "\" type mpegvideo alias mp3" ), NULL, 0, NULL );
		}
		~AlertSystem()
		{
			mciSendString( "close mp3", NULL, 0, NULL );
		}
		void initialize( cameraPositions& positions, CWnd* parent, bool isTriggerModeSensitive, int& id,
						 cToolTips& tooltips );
		void alertMainThread( int level );
		void soundAlert();
		void rearrange( std::string cameraMode, std::string triggerMode, int width, int height, fontMap fonts );
		void handleCheckBoxPress();
		UINT getAlertThreshold();
		UINT getAlertMessageID();
		void setAlertThreshold();
		bool alertsAreToBeUsed();
		bool soundIsToBePlayed();
		void playSound();
		void stopSound();
	private:
		Control<CStatic> title;
		Control<CButton> alertsActiveCheckBox;
		Control<CStatic> alertThresholdText;
		Control<CEdit> alertThresholdEdit;
		Control<CButton> soundAtFinshCheckBox;
		int alertThreshold;
		bool useAlerts;
		UINT alertMessageID = 0;
};