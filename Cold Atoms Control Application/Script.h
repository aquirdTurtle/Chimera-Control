#pragma once

#include "Control.h"
#include <string>
#include <vector>

struct profileSettings;

class Script
{
	public:
		Script();
		~Script();
		std::string getScriptText();

		bool colorEntireScript(profileSettings profileInfo, std::vector<variable> vars);
		bool colorScriptSection(DWORD beginingOfChange, DWORD endOfChange, profileSettings profileInfo, std::vector<variable> vars);

		bool initializeControls(int width, int height, POINT& startingLocation, CWnd* parent, std::string deviceTypeInput, int& idStart);

		bool reorganizeControls();

		INT_PTR colorControl(LPARAM lParam, WPARAM wParam);
		bool handleEditChange();
		bool handleTimerCall(profileSettings profileInfo, std::vector<variable> vars);

		bool updateChildCombo(profileSettings profileInfo);
		bool changeView(std::string viewName, profileSettings profileInfo, std::vector<variable> vars);
		bool childComboChangeHandler(ScriptingWindow* scriptWin, MainWindow* mainWin);
		bool checkChildSave(profileSettings profileInfo);

		std::string getSyntaxColor(std::string word, std::string editType, std::vector<variable> vars);
		bool saveScript(profileSettings profileInfo);
		bool saveScriptAs(std::string scriptAddress);
		bool renameScript(profileSettings profileInfo);
		bool deleteScript(profileSettings profileInfo);
		bool newScript(profileSettings profileInfo, std::vector<variable> vars);
		std::string getScriptAddress();
		std::string getScriptName();
		std::string getExtension();
		bool loadFile(std::string pathToFile, profileSettings profileInfo, std::vector<variable> vars);
		bool openParentScript(std::string parentScriptName, profileSettings profileInfo, std::vector<variable> vars);
		bool considerCurrentLocation(profileSettings profileInfo);
		bool checkSave(profileSettings profileInfo);
		bool updateSavedStatus(bool isSaved);
		bool coloringIsNeeded();
		
		bool updateScriptNameText();

		bool reset();
		bool savedStatus();

	private:
		Control<CRichEditCtrl> edit;
		Control<CStatic> title;
		Control<CButton> savedIndicator;
		Control<CComboBox> childCombo;
		Control<CEdit> fileNameText;

		std::string scriptExperiment;
		std::string scriptCategory;
		std::string scriptName;
		std::string scriptAddress;

		std::string deviceType;
		std::string extension;
		bool isLocalReference;
		bool isSaved;
		// no locations for children, they must all be local. 
		std::vector<std::string> childrenNames;

		unsigned long editChangeBegin;
		unsigned long editChangeEnd;
		bool syntaxColoringIsCurrent;

};
