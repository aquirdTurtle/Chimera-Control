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

		bool checkScriptSaves();
		void getFriends(MainWindow* mainWin, CameraWindow* camWin, DeviceWindow* masterWin);
		void fillMasterThreadInput(MasterThreadInput* input);
		scriptInfo<std::string> getScriptNames();
		scriptInfo<bool> getScriptSavedStatuses();
		scriptInfo<std::string> getScriptAddresses();
		profileSettings getCurrentProfileSettings();
		BOOL PreTranslateMessage(MSG* pMsg);

		void checkMasterSave();

		void handleSavingConfig(std::ofstream& saveFile);

		void updateScriptNamesOnScreen();
		void updateProfile(std::string text);
		void considerScriptLocations();
		void recolorScripts();

		void newIntensityScript();
		void openIntensityScript(CWnd* parent);
		void openIntensityScript(std::string name);
		void saveIntensityScript();
		void saveIntensityScriptAs(CWnd* parent);
		void agilentEditChange();

		void newVerticalScript();
		void openVerticalScript(CWnd* parent);
		void openVerticalScript(std::string name);
		void saveVerticalScript();
		void saveVerticalScriptAs(CWnd* parent);
		void verticalEditChange();

		void newHorizontalScript();
		void openHorizontalScript(CWnd* parent);
		void openHorizontalScript(std::string name);
		void saveHorizontalScript();
		void saveHorizontalScriptAs(CWnd* parent);
		void horizontalEditChange();

		void newMasterScript();
		void openMasterScript(CWnd* parent);
		void openMasterScript(std::string name);
		void saveMasterScript();
		void saveMasterScriptAs(CWnd* parent);
		void newMasterFunction();
		void saveMasterFunction();
		void deleteMasterFunction();
		void masterEditChange();
		Communicator* comm();
		//void openMasterScript(std::string name);

		void changeBoxColor( systemInfo<char> colors );
		void updateConfigurationSavedStatus(bool status);
		void OnCancel() override;
		void OnSize(UINT nType, int cx, int cy);

		void handleHorizontalScriptComboChange();
		void handleVerticalScriptComboChange();
		void handleAgilentScriptComboChange();
		void handleOpenConfig(std::ifstream& configFile, double version);
		void catchEnter();
		profileSettings getProfile();
	private:
		DECLARE_MESSAGE_MAP();
		
		MainWindow* mainWindowFriend;
		CameraWindow* cameraWindowFriend;
		DeviceWindow* deviceWindowFriend;
		//
		cToolTips tooltips;

		Script verticalNiawgScript, horizontalNiawgScript, intensityAgilentScript, masterScript;
		ColorBox statusBox;
		ProfileIndicator profileDisplay;
};