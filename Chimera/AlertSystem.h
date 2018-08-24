#pragma once
#include "myButton.h"
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
		void rearrange( AndorRunModes cameraMode, AndorTriggerMode triggerMode, int width, int height, fontMap fonts );
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
		Control<CStatic> title;
		Control<CleanCheck> atomsAlertActiveCheckBox;
		Control<CleanCheck> motAlertActiveCheckBox;
		Control<CStatic> alertThresholdText;
		Control<CEdit> alertThresholdEdit;
		Control<CleanCheck> soundAtFinshCheck;
		Control<CleanCheck> autoPauseAtAlert;
		int alertThreshold;
		bool useAlerts;
		bool autoPause;
		UINT alertMessageID = 0;
};