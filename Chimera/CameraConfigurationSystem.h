 #pragma once

#include <string>
#include "Control.h"
#include "Windows.h"
#include <vector>
#include "Andor.h"

struct cameraPositions;

class CameraConfigurationSystem
{
	public:
		CameraConfigurationSystem(std::string fileSystemPath);
		~CameraConfigurationSystem();
		AndorRunSettings openConfiguration( std::string configurationNameToOpen, AndorRunSettings baseSettings );
		void saveConfiguration(bool isFromSaveAs, AndorRunSettings settings );
		void saveConfigurationAs(std::string newConfigurationName, AndorRunSettings settings );
		void renameConfiguration(std::string newConfigurationName );
		void deleteConfiguration();
		int checkSave();
		void initialize( cameraPositions& positions, CWnd* parent, bool isTriggerModeSensitive, int& id );
		void reorganizeControls(RECT parentRectangle, std::string mode);

		std::vector<std::string> searchForFiles(std::string locationToSearch, std::string extensions);
		void reloadCombo(std::string nameToLoad);
		std::string getComboText();
		bool fileExists(std::string filePathway);
		void updateSaveStatus(bool saved);

	private:
		std::string configurationName;
		std::string FILE_SYSTEM_PATH;
		bool configurationSaved;
		Control<CStatic> configLabel;
		Control<CComboBox> configCombo;
};
