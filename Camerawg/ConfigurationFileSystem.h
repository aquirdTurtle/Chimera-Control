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
	// Note: The experiment (category) path include the expriment (category) name in the string.
	std::string experimentPath;
	// Note: The experiment (category) path include the expriment (category) name in the string.
	std::string categoryPath;
	std::vector<std::string> sequenceConfigNames;
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

		void saveEntireProfile(ScriptingWindow* scriptWindow, MainWindow* comm);
		void checkSaveEntireProfile(ScriptingWindow* scriptWindow, MainWindow* comm);
		void allSettingsReadyCheck(ScriptingWindow* scriptWindow, MainWindow* comm);
		void reloadAllCombos();

		void orientationChangeHandler(MainWindow* comm);
		std::string getOrientation();
		void setOrientation(std::string);

		void saveSequence();
		void saveSequenceAs();
		void renameSequence();
		void deleteSequence();
		void newSequence(CWnd* parent);
		void openSequence(std::string sequenceName);
		void updateSequenceSavedStatus(bool isSaved);
		bool sequenceSettingsReadyCheck();
		bool checkSequenceSave(std::string prompt);
		void sequenceChangeHandler();
		std::string getSequenceNamesString();
		void loadNullSequence();
		void addToSequence(CWnd* parent);
		std::vector<std::string> getSequenceNames();
		void reloadSequence(std::string sequenceToReload);

		void saveExperimentOnly(MainWindow* comm);
		void newExperiment();
		void saveExperimentAs(MainWindow* comm);
		void renameExperiment(MainWindow* comm);
		void deleteExperiment();
		void openExperiment(std::string experimentToOpen, ScriptingWindow* scriptWindow, MainWindow* comm);
		void updateExperimentSavedStatus(bool isSaved);
		void experimentSettingsReadyCheck(MainWindow* comm);
		bool checkExperimentSave(std::string prompt, MainWindow* comm);
		void experimentChangeHandler(ScriptingWindow* scriptWindow, MainWindow* comm);
		std::string getCurrentExperiment();

		void saveConfigurationOnly(ScriptingWindow* scriptWindow, MainWindow* comm);
		void newConfiguration(MainWindow* comm);
		void saveConfigurationAs(ScriptingWindow* scriptWindow, MainWindow* comm);
		void renameConfiguration();
		void deleteConfiguration();
		void openConfiguration(std::string configurationNameToOpen, ScriptingWindow* scriptWindow, MainWindow* comm);
		void updateConfigurationSavedStatus(bool isSaved);
		bool configurationSettingsReadyCheck(ScriptingWindow* scriptWindow, MainWindow* comm);
		bool checkConfigurationSave(std::string prompt, ScriptingWindow* scriptWindow, MainWindow* comm);
		void configurationChangeHandler(ScriptingWindow* scriptWindow, MainWindow* comm);

		void saveCategoryOnly(MainWindow* comm);
		void renameCategory();
		void newCategory();
		void deleteCategory();
		void saveCategoryAs(MainWindow* comm);
		void openCategory(std::string categoryToOpen, ScriptingWindow* scriptWindow, MainWindow* comm);
		void updateCategorySavedStatus(bool isSaved);
		bool categorySettinsReadyCheck();
		bool checkCategorySave(std::string prompt, MainWindow* comm);
		void categoryChangeHandler(ScriptingWindow* scriptWindow, MainWindow* comm);
		std::string getCurrentCategory();
		std::string getCurrentPathIncludingCategory();
		profileSettings getCurrentProfileSettings();

		std::vector<std::string> searchForFiles(std::string locationToSearch, std::string extensions);
		void reloadCombo(HWND comboToReload, std::string locationToLook, std::string extension, std::string nameToLoad);
		std::string getComboText();
		bool fileOrFolderExists(std::string filePathway);
		void updateSaveStatus(bool savedStatus);
		void fullyDeleteFolder(std::string folderToDelete);

		void initializeControls( POINT& topLeftPosition, CWnd* parent, int& id,
								 std::unordered_map<std::string, CFont*> fonts, std::vector<CToolTipCtrl*>& tooltips );
		void rearrange(RECT parentRectangle, std::string mode);
		
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
