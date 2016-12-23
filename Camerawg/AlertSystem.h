#pragma once
#include "Control.h"
#include <Mmsystem.h>
#include <mciapi.h>
#pragma comment(lib, "Winmm.lib")

class AlertSystem
{
	public:
		AlertSystem() : alertMessageID{ 0 } 
		{
			mciSendString("open \"C:\\Users\\Regal Lab\\Documents\\Quantum Gas Assembly Control\\Camera\\Final Fantasy VII - Victory Fanfare [HQ].mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
		}
		~AlertSystem();
		bool initialize(POINT& topLeftPositionKinetic, POINT& topLeftPositionAccumulate, POINT& topLeftPositionContinuous,
			CWnd* parent, bool isTriggerModeSensitive, int& id);
		bool alertMainThread(int level);
		bool soundAlert();
		bool reorganizeControls(std::string cameraMode, std::string triggerMode, int width, int height);
		bool handleCheckBoxPress(WPARAM messageWParam, LPARAM messageLParam);
		unsigned int getAlertThreshold();
		unsigned int getAlertMessageID();
		bool setAlertThreshold();
		bool alertsAreToBeUsed();
		bool soundIsToBePlayed();
		bool playSound();
		bool stopSound();
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