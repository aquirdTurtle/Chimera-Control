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
		Script();
		~Script();

		std::string getScriptText();
		void functionChangeHandler(std::string categoryPath);

		void rearrange(UINT width, UINT height, fontMap fonts);

		void colorEntireScript(std::vector<variable> vars, rgbMap rgbs, 
							   std::array<std::array<std::string, 16>, 4> ttlNames, std::array<std::string, 24> dacNames);
		void colorScriptSection(DWORD beginingOfChange, DWORD endOfChange, std::vector<variable> vars, rgbMap rgbs, 
								std::array<std::array<std::string, 16>, 4> ttlNames,
								std::array<std::string, 24> dacNames);
		COLORREF getSyntaxColor(std::string word, std::string editType, std::vector<variable> variables,
										 rgbMap rgbs, bool& colorLine, 
										 std::array<std::array<std::string, 16>, 4> ttlNames, 
										 std::array<std::string, 24> dacNames);

		void initialize( int width, int height, POINT& startingLocation, std::vector<CToolTipCtrl*>& toolTips,
						 ScriptingWindow* scriptWin, int& id, std::string deviceTypeInput );
		void reorganizeControls();
		INT_PTR colorControl(LPARAM lParam, WPARAM wParam);
		void handleEditChange();

		void handleTimerCall(std::vector<variable> vars, rgbMap rgbs, 
							 std::array<std::array<std::string, 16>, 4> ttlNames, std::array<std::string, 24> dacNames);

		void changeView(std::string viewName, bool isFunction, std::string categoryPath);

		void saveScript(std::string location, RunInfo info);
		void saveScriptAs(std::string location, RunInfo info);
		void renameScript(std::string categoryPath);
		void deleteScript(std::string categoryPath);
		void newScript(std::string orientation);
		void newFunction();

		std::string getScriptPathAndName();
		std::string getScriptPath();
		std::string getScriptName();
		std::string getExtension();

		void loadFile(std::string pathToFile);
		void openParentScript(std::string parentScriptFileAndPath, std::string categoryPath, RunInfo info);
		void considerCurrentLocation(std::string categoryPath, RunInfo info);
		void checkSave(std::string categoryPath, RunInfo info);
		void updateSavedStatus(bool isSaved);
		bool coloringIsNeeded();
		void updateScriptNameText(std::string path);

		void reset();
		bool savedStatus();

		INT_PTR handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, brushMap brushes);

		void saveAsFunction();
		void loadFunctions();

	private:
		CWnd syntaxTimer;
		Control<CRichEditCtrl> edit;
		Control<CStatic> title;
		Control<CButton>  savedIndicator;
		Control<CEdit> fileNameText;
		Control<CComboBox> availableFunctionsCombo;
		Control<CEdit> help;

		std::string scriptName;
		std::string scriptPath;
		std::string scriptFullAddress;
		std::string scriptCategory;
		std::string functionLocation;
		std::string deviceType;
		std::string extension;
		bool isLocalReference;
		bool isSaved;

		unsigned long editChangeBegin;
		unsigned long editChangeEnd;
		bool syntaxColoringIsCurrent;
};
