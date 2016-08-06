#pragma once
#include "Control.h"
#include <Mmsystem.h>
#include <mciapi.h>
#pragma comment(lib, "Winmm.lib")

class AlertSystem
{
	public:
		AlertSystem(int idStart) : alertMessageID{ 0 } 
		{
			alertsActiveCheckBox.ID = idStart;
			alertThresholdEdit.ID = idStart + 1;
			mciSendString("open \"C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Camera\\Final Fantasy VII - Victory Fanfare [HQ].mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
		}

		~AlertSystem();
		bool initialize(POINT& topLeftPositionKinetic, POINT& topLeftPositionAccumulate, POINT& topLeftPositionContinuous,
			HWND parentWindow, bool isTriggerModeSensitive);
		bool alertMainThread();
		bool soundAlert();
		bool reorganizeControls(RECT parentRectangle, std::string cameraMode);
		bool handleCheckBoxPress(WPARAM messageWParam, LPARAM messageLParam);
		unsigned int getAlertThreshold();
		unsigned int getAlertMessageID();
		bool setAlertThreshold();
		bool alertsAreToBeUsed();
		bool soundIsToBePlayed();
		bool playSound();
		bool stopSound();
	private:
		Control title;
		Control alertsActiveCheckBox;
		Control alertThresholdText;
		Control alertThresholdEdit;
		Control soundAtFinshCheckBox;
		int alertThreshold;
		bool useAlerts;
		unsigned int alertMessageID = 0;
};