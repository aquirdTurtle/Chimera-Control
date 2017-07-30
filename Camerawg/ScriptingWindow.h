#pragma once

#include "stdafx.h"
#include "Script.h"
#include "ColorBox.h"
#include "ProfileIndicator.h"

class MainWindow;
class CameraWindow;

template <typename type> struct scriptInfo
{
	type horizontalNIAWG;
	type verticalNIAWG;
	type intensityAgilent;
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
		void getFriends(MainWindow* mainWindowPointer, CameraWindow* camWin);
		
		scriptInfo<std::string> getScriptNames();
		scriptInfo<bool> getScriptSavedStatuses();
		scriptInfo<std::string> getScriptAddresses();
		profileSettings getCurrentProfileSettings();
		BOOL PreTranslateMessage(MSG* pMsg);

		void updateScriptNamesOnScreen();
		void updateProfile(std::string text);
		void considerScriptLocations();
		void recolorScripts();

		int newIntensityScript();
		int openIntensityScript(HWND parentWindow);
		int openIntensityScript(std::string name);
		int saveIntensityScript();
		int saveIntensityScriptAs(HWND parentWindow);

		int newVerticalScript();
		int openVerticalScript(HWND parentWindow);
		int openVerticalScript(std::string name);
		int saveVerticalScript();
		int saveVerticalScriptAs(HWND parentWindow);

		int newHorizontalScript();
		int openHorizontalScript(HWND parentWindow);
		int openHorizontalScript(std::string name);
		int saveHorizontalScript();
		int saveHorizontalScriptAs(HWND parentWindow);

		void changeBoxColor( colorBoxes<char> colors );
		void updateConfigurationSavedStatus(bool status);
		void OnCancel() override;
		void OnSize(UINT nType, int cx, int cy);

		void handleHorizontalScriptComboChange();
		void handleVerticalScriptComboChange();
		void handleAgilentScriptComboChange();
		void catchEnter();
	private:
		DECLARE_MESSAGE_MAP();
		
		MainWindow* mainWindowFriend;
		CameraWindow* cameraWindowFriend;
		//
		std::vector<CToolTipCtrl*> tooltips;

		Script verticalNiawgScript, horizontalNiawgScript, intensityAgilentScript;
		ColorBox statusBox;
		ProfileIndicator profileDisplay;
};
