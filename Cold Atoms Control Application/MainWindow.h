#pragma once
#include "stdafx.h"
#include "errBox.h"
#include "ConfigurationFileSystem.h"
#include "DebuggingOptionsControl.h"
#include "constants.h"
#include "commonMessages.h"
#include "MainOptionsControl.h"
#include "StatusControl.h"
#include "StatusIndicator.h"
#include "Communicator.h"
//#define PROFILES_PATH
class ScriptingWindow;

class MainWindow : public CDialog
{
	using CDialog::CDialog;
	DECLARE_DYNAMIC(MainWindow);
	public:
		
		MainWindow(UINT id) : CDialog(id), profile(PROFILES_PATH)
		//MainWindow() : CDialog(), profile(PROFILES_PATH)
		{
			
			mainRGBs["Dark Grey"] = RGB(15, 15, 15);
			mainRGBs["Dark Grey Red"] = RGB(20, 12, 12);
			mainRGBs["Medium Grey"] = RGB(30, 30, 30);
			mainRGBs["Light Grey"] = RGB(60, 60, 60);
			mainRGBs["Green"] = RGB(50, 200, 50);
			mainRGBs["Red"] = RGB(200, 50, 50);
			mainRGBs["Blue"] = RGB(50, 50, 200);
			mainRGBs["Gold"] = RGB(218, 165, 32);
			mainRGBs["White"] = RGB(255, 255, 255);
			mainRGBs["Light Red"] = RGB(255, 100, 100);
			mainRGBs["Dark Red"] = RGB(150, 0, 0);
			mainRGBs["Light Blue"] = RGB(100, 100, 255);
			mainRGBs["Forest Green"] = RGB(34, 139, 34);
			mainRGBs["Dull Red"] = RGB(107, 35, 35);
			mainRGBs["Dark Lavender"] = RGB(100, 100, 205);
			mainRGBs["Teal"] = RGB(0, 255, 255);
			mainRGBs["Tan"] = RGB(210, 180, 140);
			mainRGBs["Purple"] = RGB(147, 112, 219);
			mainRGBs["Orange"] = RGB(255, 165, 0);
			mainRGBs["Brown"] = RGB(139, 69, 19);
			mainRGBs["Black"] = RGB(0, 0, 0);
			mainRGBs["Dark Blue"] = RGB(0, 0, 75);
			// there are less brushes because these are only used for backgrounds.
			mainBrushes["Dark Red"] = CreateSolidBrush(mainRGBs["Dark Red"]);
			mainBrushes["Gold"] = CreateSolidBrush(mainRGBs["Gold"]);
			mainBrushes["Dark Grey"] = CreateSolidBrush(mainRGBs["Dark Grey"]);
			mainBrushes["Dark Grey Red"] = CreateSolidBrush(mainRGBs["Dark Grey Red"]);
			mainBrushes["Medium Grey"] = CreateSolidBrush(mainRGBs["Medium Grey"]);
			mainBrushes["Light Grey"] = CreateSolidBrush(mainRGBs["Light Grey"]);
			mainBrushes["Green"] = CreateSolidBrush(mainRGBs["Green"]);
			mainBrushes["Red"] = CreateSolidBrush(mainRGBs["Red"]);
			mainBrushes["White"] = CreateSolidBrush(mainRGBs["White"]);
			mainBrushes["Dull Red"] = CreateSolidBrush(mainRGBs["Dull Red"]);
			mainBrushes["Dark Blue"] = CreateSolidBrush(mainRGBs["Dark Blue"]);
		}
		BOOL OnInitDialog() override;
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		void passCommonCommand(UINT id);
		profileSettings getCurentProfileSettings();
		bool checkProfileReady();
		bool checkProfileSave();
		bool setOrientation(std::string orientation);
		void updateConfigurationSavedStatus(bool status);
		std::string getNotes(std::string whichLevel);
		void setNotes(std::string whichLevel, std::string notes);
		std::vector<variable> getAllVariables();
		void clearVariables();
		void addVariable(std::string name, bool timelike, bool singleton, double value, int item);
		LRESULT onStatusTextMessage(WPARAM wParam, LPARAM lParam);
		LRESULT onErrorMessage(WPARAM wParam, LPARAM lParam);
		LRESULT onFatalErrorMessage(WPARAM wParam, LPARAM lParam);
		LRESULT onNormalFinishMessage(WPARAM wParam, LPARAM lParam);
		LRESULT onColoredEditMessage(WPARAM wParam, LPARAM lParam);
		LRESULT onDebugMessage(WPARAM wParam, LPARAM lParam);

		debugOptions getDebuggingOptions();
		mainOptions getMainOptions();
		void setDebuggingOptions(debugOptions options);
		void setMainOptions(mainOptions options);
		void updateStatusText(std::string whichStatus, std::string text);
		void addTimebar(std::string whichStatus);
		void setShortStatus(std::string text);
		void changeShortStatusColor(std::string color);
		void passDebugPress(UINT id);
		void passMainOptionsPress(UINT id);
		void listViewDblClick(NMHDR * pNotifyStruct, LRESULT * result);
		void listViewRClick(NMHDR * pNotifyStruct, LRESULT * result);
		void handleExperimentCombo();
		void handleCategoryCombo();
		void handleConfigurationCombo();
		void handleSequenceCombo();
		void handleOrientationCombo();
		void OnClose();
		void OnDestroy();
		void OnCancel() override;
		void passClear(UINT id);

		Communicator* getComm();
	private:
		DECLARE_MESSAGE_MAP();
		ScriptingWindow* theScriptingWindow;
		std::unordered_map<std::string, HBRUSH> mainBrushes;
		std::unordered_map<std::string, COLORREF> mainRGBs;		
		ConfigurationFileSystem profile;
		NoteSystem notes;
		VariableSystem variables;
		DebuggingOptionsControl debugger;
		MainOptionsControl settings;
		StatusControl mainStatus;
		StatusControl debugStatus;
		StatusControl errorStatus;
		StatusIndicator shortStatus;
		Communicator comm;
		friend bool commonMessages::handleCommonMessage(int msgID, CWnd* parent, MainWindow* comm, ScriptingWindow* scriptWin);

public:
	afx_msg void OnFileMyRun();
};

