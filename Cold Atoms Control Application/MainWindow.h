#pragma once
#include "stdafx.h"
#include "errBox.h"
#include "ConfigurationFileSystem.h"
#include "DebuggingOptionsControl.h"
#include "constants.h"
#include "commonMessages.h"
#include "MainOptionsControl.h"
//#define PROFILES_PATH
class ScriptingWindow;

class MainWindow : public CDialog
{
	using CDialog::CDialog;
	DECLARE_DYNAMIC(MainWindow);
	public:
		MainWindow(UINT id) : CDialog(id), profile(PROFILES_PATH)
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
		void getFriends(ScriptingWindow* mainWindowPointer);
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
		debugOptions getDebuggingOptions();
		mainOptions getMainOptions();
		void setDebuggingOptions(debugOptions options);
	private:
		DECLARE_MESSAGE_MAP();
		ScriptingWindow* scriptingWindowFriend;
		std::unordered_map<std::string, HBRUSH> mainBrushes;
		std::unordered_map<std::string, COLORREF> mainRGBs;		
		ConfigurationFileSystem profile;
		NoteSystem notes;
		VariableSystem variables;
		DebuggingOptionsControl debugger;
		MainOptionsControl settings;
		friend bool commonMessages::handleCommonMessage(int msgID, CWnd* parent, MainWindow* mainWin, ScriptingWindow* scriptWin);
};

