#pragma once

#include "stdafx.h"
#include "Script.h"
#include "ColorBox.h"
#include "ProfileIndicator.h"

class MainWindow;
class CameraWindow;
class DeviceWindow;


// a convenient structure for containing one object for each script. For example, the address of each script.
template <typename type> struct scriptInfo
{
	type horizontalNIAWG;
	type verticalNIAWG;
	type intensityAgilent;
	type master;
};

class ScriptingWindow : public CDialog
{
	using CDialog::CDialog;
	DECLARE_DYNAMIC(ScriptingWindow);

	public:
		ScriptingWindow() : CDialog() {}

		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		
		BOOL OnInitDialog() override;
		void OnTimer(UINT_PTR eventID);

		void passCommonCommand(UINT id);
		void horizontalEditChange();
		void agilentEditChange();
		void verticalEditChange();
		bool checkScriptSaves();
		void getFriends(MainWindow* mainWin, CameraWindow* camWin, DeviceWindow* masterWin);
		
		scriptInfo<std::string> getScriptNames();
		scriptInfo<bool> getScriptSavedStatuses();
		scriptInfo<std::string> getScriptAddresses();
		profileSettings getCurrentProfileSettings();
		BOOL PreTranslateMessage(MSG* pMsg);

		void updateScriptNamesOnScreen();
		void updateProfile(std::string text);
		void considerScriptLocations();
		void recolorScripts();

		void newIntensityScript();
		void openIntensityScript(HWND parentWindow);
		void openIntensityScript(std::string name);
		void saveIntensityScript();
		void saveIntensityScriptAs(HWND parentWindow);

		void newVerticalScript();
		void openVerticalScript(HWND parentWindow);
		void openVerticalScript(std::string name);
		void saveVerticalScript();
		void saveVerticalScriptAs(HWND parentWindow);

		void newHorizontalScript();
		void openHorizontalScript(HWND parentWindow);
		void openHorizontalScript(std::string name);
		void saveHorizontalScript();
		void saveHorizontalScriptAs(HWND parentWindow);

		void changeBoxColor( systemInfo<char> colors );
		void updateConfigurationSavedStatus(bool status);
		void OnCancel() override;
		void OnSize(UINT nType, int cx, int cy);

		void handleHorizontalScriptComboChange();
		void handleVerticalScriptComboChange();
		void handleAgilentScriptComboChange();
		void catchEnter();
		profileSettings getProfile();
	private:
		DECLARE_MESSAGE_MAP();
		
		MainWindow* mainWindowFriend;
		CameraWindow* cameraWindowFriend;
		DeviceWindow* deviceWindowFriend;
		//
		std::vector<CToolTipCtrl*> tooltips;

		Script verticalNiawgScript, horizontalNiawgScript, intensityAgilentScript, masterScript;
		ColorBox statusBox;
		ProfileIndicator profileDisplay;
};
