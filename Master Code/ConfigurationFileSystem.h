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
class MasterWindow;
class ConfigurationFileSystem
{
	public:
		ConfigurationFileSystem(std::string fileSystemPath);
		~ConfigurationFileSystem();

		bool saveEntireProfile(MasterWindow& Master);
		bool checkSaveEntireProfile(MasterWindow& Master);
		bool allSettingsReadyCheck(MasterWindow& Master);
		bool reloadAllCombos();

		bool orientationChangeHandler(MasterWindow& Master);
		std::string getOrientation();
		bool setOrientation(std::string);

		bool saveSequence(MasterWindow& Master);
		bool saveSequenceAs(MasterWindow& Master);
		bool renameSequence(MasterWindow& Master);
		bool deleteSequence(MasterWindow& Master);
		bool newSequence(MasterWindow& Master);
		bool openSequence(std::string sequenceName, MasterWindow& Master);
		bool updateSequenceSavedStatus(bool isSaved);
		bool sequenceSettingsReadyCheck(MasterWindow& Master);
		bool checkSequenceSave(std::string prompt, MasterWindow& Master);
		bool sequenceChangeHandler(MasterWindow& Master);
		std::string getSequenceNamesString();
		bool loadNullSequence(MasterWindow& Master);
		bool addToSequence(MasterWindow& Master);
		std::vector<std::string> getSequenceNames();
		bool reloadSequence(std::string sequenceToReload, MasterWindow& Master);

		bool saveExperimentOnly(MasterWindow& Master);
		bool newExperiment(MasterWindow& Master);
		bool saveExperimentAs(MasterWindow& Master);
		bool renameExperiment();
		bool deleteExperiment();
		bool openExperiment(std::string experimentToOpen, MasterWindow& Master);
		bool updateExperimentSavedStatus(bool isSaved);
		bool experimentSettingsReadyCheck(MasterWindow& Master);
		bool checkExperimentSave(std::string prompt, MasterWindow& Master);
		bool experimentChangeHandler(MasterWindow& Master);

		bool saveConfigurationOnly(MasterWindow& Master);
		bool newConfiguration(MasterWindow& Master);
		bool saveConfigurationAs(MasterWindow& Master);
		bool renameConfiguration(MasterWindow& Master);
		bool deleteConfiguration();
		bool openConfiguration(std::string configurationNameToOpen, MasterWindow& Master);
		bool updateConfigurationSavedStatus(bool isSaved);
		bool configurationSettingsReadyCheck(MasterWindow& Master);
		bool checkConfigurationSave(std::string prompt, MasterWindow& Master);
		bool configurationChangeHandler(MasterWindow& Master);

		bool saveCategoryOnly(MasterWindow& Master);
		bool renameCategory();
		bool newCategory(MasterWindow& Master);
		bool saveCategoryAs(MasterWindow& Master);
		bool deleteCategory();
		bool openCategory(std::string categoryToOpen, MasterWindow& Master);
		bool updateCategorySavedStatus(bool isSaved);
		bool categorySettingsReadyCheck(MasterWindow& Master);
		bool checkCategorySave(std::string prompt, MasterWindow& Master);
		bool categoryChangeHandler(MasterWindow& Master);
		std::string getCurrentPathIncludingCategory();

		std::vector<std::string> searchForFiles(std::string locationToSearch, std::string extensions);
		bool reloadCombo(HWND comboToReload, std::string locationToLook, std::string extension, std::string nameToLoad);
		std::string getComboText();
		bool fileOrFolderExists(std::string filePathway);
		void updateSaveStatus(bool savedStatus);
		bool fullyDeleteFolder(std::string folderToDelete);

		bool initialize(POINT& topLeftPosition, MasterWindow& Master);
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
