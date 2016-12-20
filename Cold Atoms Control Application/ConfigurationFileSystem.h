#pragma once
#include <vector>
#include <string>
#include "Control.h"

class MainWindow;
class ScriptingWindow;

/*
]- This is a structure used for containing a set of parameters that define a profile.
]- It's used heavily by the configuration file system, but not exclusively by it.
*/
struct profileSettings
{
	std::string configuration;
	std::string experiment;
	std::string category;
	std::string sequence;
	std::string orientation;
	std::string pathIncludingExperiment;
	std::string pathIncludingCategory;

	std::vector<std::string> sequenceConfigurationNames;
};

/*
]--- This singleton class manages the entire "profile" system, where "profiles" are my term for the entirety of the settings in the code (strange word choice I
]- know). It consists of the relevant controls, some saved indicators that can be checked to determine if the user should be prompted to save at a given point,
]- and all of the functions for saving, renaming, deleting, and creating new levels within the code. 
*/
class ConfigurationFileSystem
{
	public:
		ConfigurationFileSystem(std::string fileSystemPath);
		~ConfigurationFileSystem();

		bool saveEntireProfile(ScriptingWindow* scriptWindow, MainWindow* comm);
		bool checkSaveEntireProfile(ScriptingWindow* scriptWindow, MainWindow* comm);
		bool allSettingsReadyCheck(ScriptingWindow* scriptWindow, MainWindow* comm);
		bool reloadAllCombos();

		bool orientationChangeHandler(MainWindow* comm);
		std::string getOrientation();
		bool setOrientation(std::string);

		bool saveSequence();
		bool saveSequenceAs();
		bool renameSequence();
		bool deleteSequence();
		bool newSequence(CWnd* parent);
		bool openSequence(std::string sequenceName);
		bool updateSequenceSavedStatus(bool isSaved);
		bool sequenceSettingsReadyCheck();
		bool checkSequenceSave(std::string prompt);
		bool sequenceChangeHandler();
		std::string getSequenceNamesString();
		bool loadNullSequence();
		bool addToSequence(CWnd* parent);
		std::vector<std::string> getSequenceNames();
		bool reloadSequence(std::string sequenceToReload);

		bool saveExperimentOnly(MainWindow* comm);
		bool newExperiment();
		bool saveExperimentAs(MainWindow* comm);
		bool renameExperiment(MainWindow* comm);
		bool deleteExperiment();
		bool openExperiment(std::string experimentToOpen, ScriptingWindow* scriptWindow, MainWindow* comm);
		bool updateExperimentSavedStatus(bool isSaved);
		bool experimentSettingsReadyCheck(MainWindow* comm);
		bool checkExperimentSave(std::string prompt, MainWindow* comm);
		bool experimentChangeHandler(ScriptingWindow* scriptWindow, MainWindow* comm);
		std::string getCurrentExperiment();

		bool saveConfigurationOnly(ScriptingWindow* scriptWindow, MainWindow* comm);
		bool newConfiguration(MainWindow* comm);
		bool saveConfigurationAs(ScriptingWindow* scriptWindow, MainWindow* comm);
		bool renameConfiguration();
		bool deleteConfiguration();
		bool openConfiguration(std::string configurationNameToOpen, ScriptingWindow* scriptWindow, MainWindow* comm);
		bool updateConfigurationSavedStatus(bool isSaved);
		bool configurationSettingsReadyCheck(ScriptingWindow* scriptWindow, MainWindow* comm);
		bool checkConfigurationSave(std::string prompt, ScriptingWindow* scriptWindow, MainWindow* comm);
		bool configurationChangeHandler(ScriptingWindow* scriptWindow, MainWindow* comm);

		bool saveCategoryOnly(MainWindow* comm);
		bool renameCategory();
		bool newCategory();
		bool deleteCategory();
		bool saveCategoryAs(MainWindow* comm);
		bool openCategory(std::string categoryToOpen, ScriptingWindow* scriptWindow, MainWindow* comm);
		bool updateCategorySavedStatus(bool isSaved);
		bool categorySettinsReadyCheck();
		bool checkCategorySave(std::string prompt, MainWindow* comm);
		bool categoryChangeHandler(ScriptingWindow* scriptWindow, MainWindow* comm);
		std::string getCurrentCategory();
		std::string getCurrentPathIncludingCategory();
		profileSettings getCurrentProfileSettings();

		std::vector<std::string> searchForFiles(std::string locationToSearch, std::string extensions);
		bool reloadCombo(HWND comboToReload, std::string locationToLook, std::string extension, std::string nameToLoad);
		std::string getComboText();
		bool fileOrFolderExists(std::string filePathway);
		void updateSaveStatus(bool savedStatus);
		bool fullyDeleteFolder(std::string folderToDelete);

		bool initializeControls(POINT& topLeftPosition, CWnd* parent, int& id);
		bool reorganizeControls(RECT parentRectangle, std::string mode);
		
	private:
		profileSettings currentProfileSettings;
		std::string FILE_SYSTEM_PATH;
		bool configurationIsSaved;
		bool categoryIsSaved;
		bool experimentIsSaved;
		bool sequenceIsSaved;
		
		Control<CStatic> configLabel;
		Control<CComboBox> configCombo;
		Control<CStatic> experimentLabel;
		Control<CComboBox> experimentCombo;
		Control<CStatic> categoryLabel;
		Control<CComboBox> categoryCombo;
		Control<CStatic> sequenceLabel;
		Control<CComboBox> sequenceCombo;
		Control<CEdit> sequenceInfoDisplay;
		Control<CButton> sequenceSavedIndicator;
		Control<CStatic> orientationLabel;
		Control<CComboBox> orientationCombo;
		Control<CButton> categorySavedIndicator;
		Control<CButton> configurationSavedIndicator;
		Control<CButton> experimentSavedIndicator;
};
