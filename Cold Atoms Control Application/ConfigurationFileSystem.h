#pragma once
#include <vector>
#include <string>
#include "Control.h"
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

		bool saveEntireProfile();
		bool checkSaveEntireProfile();
		bool allSettingsReadyCheck();
		bool reloadAllCombos();

		bool orientationChangeHandler(HWND parentWindow);
		std::string getOrientation();
		bool setOrientation(std::string);

		bool saveSequence();
		bool saveSequenceAs();
		bool renameSequence();
		bool deleteSequence();
		bool newSequence(HWND parentWindow);
		bool openSequence(std::string sequenceName);
		bool updateSequenceSavedStatus(bool isSaved);
		bool sequenceSettingsReadyCheck();
		bool checkSequenceSave(std::string prompt);
		bool sequenceChangeHandler();
		std::string getSequenceNamesString();
		bool loadNullSequence();
		bool addToSequence(HWND parentWindow);
		std::vector<std::string> getSequenceNames();
		bool reloadSequence(std::string sequenceToReload);

		bool saveExperimentOnly();
		bool newExperiment();
		bool saveExperimentAs();
		bool renameExperiment();
		bool deleteExperiment();
		bool openExperiment(std::string experimentToOpen, HWND parentWindow);
		bool updateExperimentSavedStatus(bool isSaved);
		bool experimentSettingsReadyCheck();
		bool checkExperimentSave(std::string prompt);
		bool experimentChangeHandler(HWND parentWindow);

		bool saveConfigurationOnly();
		bool newConfiguration();
		bool saveConfigurationAs();
		bool renameConfiguration();
		bool deleteConfiguration();
		bool openConfiguration(std::string configurationNameToOpen, HWND parentWindow);
		bool updateConfigurationSavedStatus(bool isSaved);
		bool configurationSettingsReadyCheck();
		bool checkConfigurationSave(std::string prompt);
		bool configurationChangeHandler(HWND parentWindow);

		bool saveCategoryOnly();
		bool renameCategory();
		bool newCategory();
		bool deleteCategory();
		bool saveCategoryAs();
		bool openCategory(std::string categoryToOpen, HWND parentWindow);
		bool updateCategorySavedStatus(bool isSaved);
		bool categorySettinsReadyCheck();
		bool checkCategorySave(std::string prompt);
		bool categoryChangeHandler(HWND parentWindow);
		std::string getCurrentPathIncludingCategory();

		std::vector<std::string> searchForFiles(std::string locationToSearch, std::string extensions);
		bool reloadCombo(HWND comboToReload, std::string locationToLook, std::string extension, std::string nameToLoad);
		std::string getComboText();
		bool fileOrFolderExists(std::string filePathway);
		void updateSaveStatus(bool savedStatus);
		bool fullyDeleteFolder(std::string folderToDelete);

		bool initializeControls(POINT& topLeftPosition, HWND parentWindow);
		bool reorganizeControls(RECT parentRectangle, std::string mode);
		
	private:
		profileSettings currentProfileSettings;
		std::string FILE_SYSTEM_PATH;
		std::string pathIncludingExperiment;
		std::string pathIncludingCategory;
		bool configurationIsSaved;
		bool categoryIsSaved;
		bool experimentIsSaved;
		bool sequenceIsSaved;
		Control configLabel;
		Control configCombo;
		Control experimentLabel;
		Control experimentCombo;
		Control categoryLabel;
		Control categoryCombo;
		Control sequenceLabel;
		Control sequenceCombo;
		Control sequenceInfoDisplay;
		Control sequenceSavedIndicator;
		Control orientationLabel;
		Control orientationCombo;
		Control categorySavedIndicator;
		Control configurationSavedIndicator;
		Control experimentSavedIndicator;
		std::vector<std::string> sequenceConfigurationNames;
};
