#pragma once

#include "Control.h"
#include <string>
#include <vector>
#include "VariableSystem.h"
#include "RunInfo.h"
#include "ConfigurationFileSystem.h"
#include <unordered_map>
#include "Combo.h"
#include "VariableSystem.h"
#include <string>

class Script
{
	public:
		Script(std::string functionsLocation, std::string deviceTypeInput, int& idStart);
		~Script();

		std::string getScriptText();
		bool functionChangeHandler(MasterWindow* master);

		bool colorEntireScript(MasterWindow* Master);
		bool colorScriptSection(DWORD beginingOfChange, DWORD endOfChange, MasterWindow* Master);
		COLORREF Script::getSyntaxColor(std::string word, std::string editType, std::vector<variable> variables, std::unordered_map<std::string, COLORREF> rgbs,
			bool& colorLine, std::array<std::array<std::string, 16>, 4> ttlNames, std::array<std::string, 24> dacNames);

		bool initialize(int width, int height, POINT& startingLocation, std::vector<CToolTipCtrl*>& toolTips, HWND parentWindow, MasterWindow* master);
		bool reorganizeControls();
		bool getControlIDRange(int& start, int& fin);
		INT_PTR colorControl(LPARAM lParam, WPARAM wParam);
		bool handleEditChange(MasterWindow* master);
		//bool handleEditChange();
		bool handleTimerCall(MasterWindow* Master);

		bool updateChildCombo(MasterWindow* Master);
		bool Script::changeView(std::string viewName, MasterWindow* Master, bool isFunction);
		bool childComboChangeHandler(WPARAM messageWParam, LPARAM messageLParam, MasterWindow* Master);
		bool checkChildSave(MasterWindow* Master);

		bool saveScript(MasterWindow* Master);
		bool saveScriptAs(std::string scriptAddress, MasterWindow* Master);
		bool renameScript(MasterWindow* Master);
		bool deleteScript(MasterWindow* Master);
		bool newScript(MasterWindow* Master);
		
		std::string getScriptPathAndName();
		std::string getScriptPath();
		std::string getScriptName();
		std::string getExtension();

		bool loadFile(std::string pathToFile);
		bool openParentScript(std::string parentScriptName, MasterWindow* Master);
		bool considerCurrentLocation(MasterWindow* Master);
		bool checkSave(MasterWindow* Master);
		bool updateSavedStatus(bool isSaved);
		bool coloringIsNeeded();
		bool updateScriptNameText(std::string path);

		bool reset();
		bool savedStatus();

		INT_PTR handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, std::unordered_map<std::string, HBRUSH> brushes);

		bool saveAsFunction();
		bool loadFunctions();

	private:
		CWnd syntaxTimer;
		const int idStart;
		const int idEnd;
		Control<CRichEditCtrl> edit;
		Control<CStatic> title;
		Control<CButton>  savedIndicator;
		Control<CComboBox>  childCombo;
		Control<CEdit> fileNameText;
		Control<CComboBox> availableFunctionsCombo;
		Control<CEdit> help;

		std::vector<std::string> functionDeclarations;

		std::string scriptName;
		std::string scriptPath;
		std::string scriptFullAddress;
		std::string scriptCategory;
		const std::string functionLocation;
		const std::string deviceType;
		std::string extension;
		bool isLocalReference;
		bool isSaved;
		// no locations for children, they must all be local. 
		std::vector<std::string> childrenNames;

		unsigned long editChangeBegin;
		unsigned long editChangeEnd;
		bool syntaxColoringIsCurrent;
};
