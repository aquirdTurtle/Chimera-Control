#pragma once

#include <vector>
#include <string>
#include "Control.h"
#include <unordered_map>

/*
 * This is a structure used for containing a set of parameters that define a profile.
 * It's used heavily by the configuration file system, but not exclusively by it.
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
};

/*
 * This constant class manages the entire "profile" system, where "profiles" are my term for the entirety of the settings in the code (strange word choice I
 * know). It consists of the relevant controls, some saved indicators that can be checked to determine if the user should be prompted to save at a given point,
 * and all of the functions for saving, renaming, deleting, and creating new levels within the code.
 */f
class MasterWindow;
class ConfigurationFileSystem
{
	public:
		ConfigurationFileSystem(std::string fileSystemPath);

		void saveEntireProfile(MasterWindow* Master);
		void checkSaveEntireProfile(MasterWindow* Master);
		void allSettingsReadyCheck(MasterWindow* Master);
		void reloadAllCombos();

		void orientationChangeHandler(MasterWindow* Master);
		std::string getOrientation();
		void setOrientation(std::string);

		void saveSequence(MasterWindow* Master);
		void saveSequenceAs(MasterWindow* Master);
		void renameSequence(MasterWindow* Master);
		void deleteSequence(MasterWindow* Master);
		void newSequence(MasterWindow* Master);
		void openSequence(std::string sequenceName, MasterWindow* Master);
		void updateSequenceSavedStatus( bool isSaved);
		void sequenceSettingsReadyCheck(MasterWindow* Master);
		void checkSequenceSave(std::string prompt, MasterWindow* Master);
		void sequenceChangeHandler(MasterWindow* Master);
		std::string getSequenceNamesString();
		void loadNullSequence(MasterWindow* Master);
		void addToSequence(MasterWindow* Master);
		std::vector<std::string> getSequenceNames();
		void reloadSequence(std::string sequenceToReload, MasterWindow* Master);

		void saveExperiment(MasterWindow* Master);
		void newExperiment(MasterWindow* Master);
		void saveExperimentAs(MasterWindow* Master);
		void renameExperiment();
		void deleteExperiment();
		void openExperiment(std::string experimentToOpen, MasterWindow* Master);
		void updateExperimentSavedStatus( bool isSaved);
		void experimentSettingsReadyCheck(MasterWindow* Master);
		void checkExperimentSave(std::string prompt, MasterWindow* Master);
		void experimentChangeHandler(MasterWindow* Master);

		void saveConfiguration(MasterWindow* Master);
		void newConfiguration(MasterWindow* Master);
		void saveConfigurationAs(MasterWindow* Master);
		void renameConfiguration(MasterWindow* Master);
		void deleteConfiguration();
		void openConfiguration(std::string configurationNameToOpen, MasterWindow* Master);
		void updateConfigurationSavedStatus( bool isSaved);
		void configurationSettingsReadyCheck(MasterWindow* Master);
		void checkConfigurationSave(std::string prompt, MasterWindow* Master);
		void configurationChangeHandler(MasterWindow* Master);

		void rearrange(int width, int height, fontMap fonts);

		void saveCategory(MasterWindow* Master);
		void renameCategory();
		void newCategory(MasterWindow* Master);
		void saveCategoryAs(MasterWindow* Master);
		void deleteCategory();
		void openCategory(std::string categoryToOpen, MasterWindow* Master);
		void updateCategorySavedStatus( bool isSaved);
		void categorySettingsReadyCheck(MasterWindow* Master);
		void checkCategorySave(std::string prompt, MasterWindow* Master);
		void categoryChangeHandler(MasterWindow* Master);

		std::string getCurrentPathIncludingCategory();

		std::vector<std::string> searchForFiles(std::string locationToSearch, std::string extensions);
		void reloadCombo(HWND comboToReload, std::string locationToLook, std::string extension, std::string nameToLoad);
		std::string getComboText();
		bool fileOrFolderExists(std::string filePathway);
		void updateSaveStatus(bool savedStatus);
		void fullyDeleteFolder(std::string folderToDelete);

		void initialize( POINT& topLeftPosition, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master, int& id );
		void reorganizeControls(RECT parentRectangle, std::string mode);

		INT_PTR handleColorMessage( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, brushMap brushes);

		std::string getMasterAddressFromConfig();

	private:
		profileSettings currentProfile;
		std::string FILE_SYSTEM_PATH;

		const double version;

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
		std::vector<std::string> sequenceConfigurationNames;
};
