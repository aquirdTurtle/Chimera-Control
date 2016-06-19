#pragma once

#include "Control.h"
#include <string>
#include <vector>
#include "VariableSystem.h"
#include "RunInfo.h"
#include "ConfigurationFileSystem.h"

class Script
{
	public:
		Script(std::string deviceTypeInput, int& idStart);
		~Script();
		std::string getScriptText();

		bool colorEntireScript(MasterWindow& Master);
		bool colorScriptSection(DWORD beginingOfChange, DWORD endOfChange, MasterWindow& Master);
		std::string getSyntaxColor(std::string word, std::string editType, MasterWindow& Master);

		bool initialize(int width, int height, POINT& startingLocation, MasterWindow& Master);
		bool reorganizeControls();
		bool getControlIDRange(int& start, int& fin);
		INT_PTR colorControl(LPARAM lParam, WPARAM wParam);
		bool handleEditChange(WPARAM wParam, LPARAM lParam, HWND parentWindow);
		bool handleTimerCall(MasterWindow& Master);

		bool updateChildCombo(MasterWindow& Master);
		bool changeView(std::string viewName, MasterWindow& Master);
		bool childComboChangeHandler(WPARAM messageWParam, LPARAM messageLParam, MasterWindow& Master);
		bool checkChildSave(MasterWindow& Master);

		bool saveScript(MasterWindow& Master);
		bool saveScriptAs(std::string scriptAddress, MasterWindow& Master);
		bool renameScript(MasterWindow& Master);
		bool deleteScript(MasterWindow& Master);
		bool newScript(MasterWindow& Master);
		std::string getScriptPathAndName();
		std::string getScriptName();
		std::string getExtension();
		bool loadFile(std::string pathToFile);
		bool openParentScript(std::string parentScriptName, MasterWindow& Master);
		bool considerCurrentLocation(MasterWindow& Master);
		bool checkSave(MasterWindow& Master);
		bool updateSavedStatus(bool isSaved);
		bool coloringIsNeeded();
		bool updateScriptNameText(std::string path);

		bool reset();
		bool savedStatus();

	private:
		const int idStart;
		const int idEnd;
		Control edit;
		const int editID;
		Control title;
		const int titleID;
		Control savedIndicator;
		const int savedIndicatorID;
		Control childCombo;
		const int childComboID;
		Control fileNameText;
		const int fileNameTextID;

		std::string scriptName;
		std::string scriptAddress;
		const std::string deviceType;
		std::string extension;
		bool isLocalReference;
		bool isSaved;
		// no locations for children, they must all be local. 
		std::vector<std::string> childrenNames;

		int editChangeBegin;
		int editChangeEnd;
		bool syntaxColoringIsCurrent;
};
