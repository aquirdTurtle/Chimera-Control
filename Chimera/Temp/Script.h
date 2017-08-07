#pragma once

#include "Control.h"
#include <string>
#include <vector>
#include "commonTypes.h"

struct profileSettings;

class Script
{
	public:
		std::string getScriptText();

		bool colorEntireScript(profileSettings profile, std::vector<variable> vars);
		bool colorScriptSection(DWORD beginingOfChange, DWORD endOfChange, profileSettings profile, std::vector<variable> vars);

		bool initialize(int width, int height, POINT& startingLocation, CWnd* parent, std::string deviceTypeInput, 
			int& idStart, fontMap fonts, std::vector<CToolTipCtrl*>& tooltips);

		void rearrange(int width, int height, fontMap fonts);

		INT_PTR colorControl(LPARAM lParam, WPARAM wParam);
		void handleEditChange();
		bool handleTimerCall(profileSettings profile, std::vector<variable> vars);

		bool updateChildCombo(profileSettings profile);
		bool changeView(std::string viewName, profileSettings profile, std::vector<variable> vars);
		bool childComboChangeHandler(MainWindow* mainWin, DeviceWindow* deviceWin);
		//bool childComboChangeHandler( MainWindow* mainWin );
		bool checkChildSave(profileSettings profile);

		std::string getSyntaxColor(std::string word, std::string editType, std::vector<variable> vars);
		bool saveScript(profileSettings profile, bool isParent, bool niawgIsRunning);
		bool saveScript(profileSettings profile, bool niawgIsRunning );
		bool saveScriptAs(std::string scriptAddress, bool isParent, bool niawgIsRunning );
		bool saveScriptAs(std::string scriptAddress, bool niawgIsRunning );
		bool renameScript(profileSettings profile);
		bool deleteScript(profileSettings profile);
		bool newScript(profileSettings profile, std::vector<variable> vars);
		std::string getScriptAddress();
		std::string getScriptName();
		std::string getExtension();
		bool loadFile(std::string pathToFile, profileSettings profile, std::vector<variable> vars);
		bool openParentScript(std::string parentScriptName, profileSettings profile, std::vector<variable> vars);
		bool considerCurrentLocation(profileSettings profile);
		bool checkSave(profileSettings profile, bool niawgIsRunning );
		bool updateSavedStatus(bool isSaved);
		bool coloringIsNeeded();
		
		bool updateScriptNameText();

		bool reset();
		bool savedStatus();

		void checkExtension(profileSettings profile);

	private:

		void cleanString(std::string &str);

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
		std::string currentViewName;
		bool currentViewIsParent;
		bool isLocalReference;
		bool isSaved;
		// no locations for children, they must all be local. 
		std::vector<std::string> childrenNames;
		

		unsigned long editChangeBegin;
		unsigned long editChangeEnd;
		bool syntaxColoringIsCurrent;

};
