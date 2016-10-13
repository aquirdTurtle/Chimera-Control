#pragma once

#include "Control.h"
#include <string>
#include <vector>

class Script
{
	public:
		Script();
		~Script();
		std::string getScriptText();

		bool colorEntireScript();
		bool colorScriptSection(DWORD beginingOfChange, DWORD endOfChange);

		bool initializeControls(int width, int height, POINT& startingLocation, HWND parent, std::string deviceTypeInput, int& idStart);
		bool reorganizeControls();
		bool getControlIDRange(int& start, int& fin);
		INT_PTR colorControl(LPARAM lParam, WPARAM wParam);
		bool handleEditChange(WPARAM wParam, LPARAM lParam);
		bool handleTimerCall();

		bool updateChildCombo();
		bool changeView(std::string viewName);
		bool childComboChangeHandler(WPARAM messageWParam, LPARAM messageLParam);
		bool checkChildSave();

		std::string getSyntaxColor(std::string word, std::string editType);
		bool saveScript();
		bool saveScriptAs(std::string scriptAddress);
		bool renameScript();
		bool deleteScript();
		bool newScript();
		std::string getScriptPathAndName();
		std::string getScriptName();
		std::string getExtension();
		bool loadFile(std::string pathToFile);
		bool openParentScript(std::string parentScriptName);
		bool considerCurrentLocation();
		bool checkSave();
		bool updateSavedStatus(bool isSaved);
		bool coloringIsNeeded();
		
		bool updateScriptNameText();

		bool reset();
		bool savedStatus();

	private:
		const int idStart;
		const int idEnd;
		HwndControl edit;
		HwndControl title;
		HwndControl savedIndicator;
		HwndControl childCombo;
		HwndControl fileNameText;

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

		int editChangeBegin;
		int editChangeEnd;
		bool syntaxColoringIsCurrent;

};
