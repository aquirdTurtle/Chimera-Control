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
		Script(std::string functionsLocation, std::string deviceTypeInput);
		~Script();

		std::string getScriptText();
		void functionChangeHandler(MasterWindow* master);

		void rearrange(UINT width, UINT height, fontMap fonts);

		void colorEntireScript(MasterWindow* Master);
		void colorScriptSection(DWORD beginingOfChange, DWORD endOfChange, MasterWindow* Master);
		COLORREF getSyntaxColor(std::string word, std::string editType, std::vector<variable> variables, 
										 rgbMap rgbs, bool& colorLine, 
										 std::array<std::array<std::string, 16>, 4> ttlNames, 
										 std::array<std::string, 24> dacNames);

		void initialize( int width, int height, POINT& startingLocation, std::vector<CToolTipCtrl*>& toolTips,
								 MasterWindow* master, int& id );
		void reorganizeControls();
		INT_PTR colorControl(LPARAM lParam, WPARAM wParam);
		void handleEditChange(MasterWindow* master);
		//bool handleEditChange();
		void handleTimerCall(MasterWindow* Master);

		void changeView(std::string viewName, MasterWindow* Master, bool isFunction);

		void saveScript(MasterWindow* Master);
		void saveScriptAs(std::string scriptAddress, MasterWindow* Master);
		void renameScript(MasterWindow* Master);
		void deleteScript(MasterWindow* Master);
		void newScript(MasterWindow* Master);
		void newFunction(MasterWindow* Master);

		std::string getScriptPathAndName();
		std::string getScriptPath();
		std::string getScriptName();
		std::string getExtension();

		void loadFile(std::string pathToFile, MasterWindow* Master);
		void openParentScript(std::string parentScriptName, MasterWindow* Master);
		void considerCurrentLocation(MasterWindow* Master);
		void checkSave(MasterWindow* Master);
		void updateSavedStatus(bool isSaved);
		bool coloringIsNeeded();
		void updateScriptNameText(std::string path);

		void reset(MasterWindow* master);
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
		const std::string functionLocation;
		const std::string deviceType;
		std::string extension;
		bool isLocalReference;
		bool isSaved;

		unsigned long editChangeBegin;
		unsigned long editChangeEnd;
		bool syntaxColoringIsCurrent;
};
