#pragma once
#include <string>
#include "Control.h"
#include "Windows.h"
#include <vector>

class FileSystem 
{
	public:
		FileSystem(std::string fileSystemPath);
		~FileSystem();
		int openConfiguration(std::string configurationNameToOpen);
		int saveConfiguration(bool isFromSaveAs);
		int saveConfigurationAs(std::string newConfigurationName);
		int renameConfiguration(std::string newConfigurationName);
		int deleteConfiguration();
		int checkSave();
		int initializeControls(POINT& topLeftPositionKinetic, POINT& topLeftPositionAccumulate, POINT& topLeftPositionContinuous, HWND parentWindow, bool isTriggerModeSensitive);
		int reorganizeControls(RECT parentRectangle, std::string mode);

		std::vector<std::string> searchForFiles(std::string locationToSearch, std::string extensions);
		int reloadCombo(std::string nameToLoad);
		std::string getComboText();
		bool fileExists(std::string filePathway);
		void updateSaveStatus(bool saved);

	private:
		std::string configurationName;
		std::string FILE_SYSTEM_PATH;
		bool configurationSaved;
		Control configLabel;
		Control configCombo;
};
