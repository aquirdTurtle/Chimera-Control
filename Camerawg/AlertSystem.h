#pragma once
#include "Control.h"
#include <Mmsystem.h>
#include <mciapi.h>
#pragma comment(lib, "Winmm.lib")

struct cameraPositions;

class AlertSystem
{
	public:
		AlertSystem() : alertMessageID{ 0 } 
		{
			// load the music!
			mciSendString("open \"C:\\Users\\Regal Lab\\Documents\\Quantum Gas Assembly Control\\Camera\\Final Fantasy VII - Victory Fanfare [HQ].mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
		}
		~AlertSystem()
		{
			mciSendString( "close mp3", NULL, 0, NULL );
		}
		void initialize(cameraPositions& positions, CWnd* parent, bool isTriggerModeSensitive, int& id, fontMap fonts, 
						std::vector<CToolTipCtrl*>& tooltips);
		void alertMainThread(int level);
		void soundAlert();
		void rearrange(std::string cameraMode, std::string triggerMode, int width, int height, fontMap fonts);
		void handleCheckBoxPress();
		unsigned int getAlertThreshold();
		unsigned int getAlertMessageID();
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
		unsigned int alertMessageID = 0;
};