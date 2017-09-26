#pragma once
#include <vector>
#include <string>
#include "Control.h"
#include "ProfileSystem.h"
#include "NiawgController.h"
#include "commonTypes.h"
#include "profileSettings.h"

class MainWindow;
class ScriptingWindow;

/*
]--- This singleton class manages the entire "profile" system, where "profiles" are my term for the entirety of the settings in the code (strange word choice I
]- know). It consists of the relevant controls, some saved indicators that can be checked to determine if the user should be prompted to save at a given point,
]- and all of the functions for saving, renaming, deleting, and creating new levels within the code. 
*/
class ProfileSystem
{
	public:
		ProfileSystem(std::string fileSystemPath);

		void saveEntireProfile( ScriptingWindow* scriptWindow, MainWindow* mainWin, AuxiliaryWindow* auxWin,
							    CameraWindow* camWin );
		void checkSaveEntireProfile( ScriptingWindow* scriptWindow, MainWindow* mainWin, AuxiliaryWindow* auxWin, 
									 CameraWindow* camWin);
		void allSettingsReadyCheck(ScriptingWindow* scriptWindow, MainWindow* mainWin, AuxiliaryWindow* auxWin, 
								   CameraWindow* camWin);
		std::string getMasterAddressFromConfig();
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

		void saveExperimentOnly(MainWindow* mainWin);
		void newExperiment();
		void saveExperimentAs(MainWindow* mainWin);
		void renameExperiment(MainWindow* mainWin);
		void deleteExperiment();
		void openExperiment(std::string experimentToOpen, ScriptingWindow* scriptWindow, MainWindow* mainWin);
		void updateExperimentSavedStatus(bool isSaved);
		void experimentSettingsReadyCheck(MainWindow* mainWin);
		bool checkExperimentSave(std::string prompt, MainWindow* mainWin);
		void experimentChangeHandler(ScriptingWindow* scriptWindow, MainWindow* mainWin);
		std::string getCurrentExperiment();

		void saveConfigurationOnly( ScriptingWindow* scriptWindow, MainWindow* mainWin, AuxiliaryWindow* auxWin, 
								    CameraWindow* camWin );
		void newConfiguration( MainWindow* mainWin, AuxiliaryWindow* auxWin, CameraWindow* camWin,
							   ScriptingWindow* scriptWin );
		void saveConfigurationAs(ScriptingWindow* scriptWindow, MainWindow* mainWin, AuxiliaryWindow* auxWin);
		void renameConfiguration();
		void deleteConfiguration();
		void openConfig( std::string configurationNameToOpen, ScriptingWindow* scriptWindow, MainWindow* mainWin, 
						 CameraWindow* camWin, AuxiliaryWindow* auxWin );

		static void openNiawgFiles( niawgPair<std::vector<std::fstream>>& scriptFiles, profileSettings profile, 
								   bool programNiawg );
		void updateConfigurationSavedStatus( bool isSaved );
		bool configurationSettingsReadyCheck(ScriptingWindow* scriptWindow, MainWindow* mainWin, 
											 AuxiliaryWindow* auxWin, CameraWindow* camWin);
		bool checkConfigurationSave(std::string prompt, ScriptingWindow* scriptWindow, MainWindow* mainWin, 
									AuxiliaryWindow* auxWin, CameraWindow* camWin);
		void configurationChangeHandler( ScriptingWindow* scriptWindow, MainWindow* mainWin, AuxiliaryWindow* auxWin,
										 CameraWindow* camWin);
		
		void saveCategoryOnly(MainWindow* mainWin);
		void renameCategory();
		void newCategory();
		void deleteCategory();
		void saveCategoryAs(MainWindow* mainWin);
		void openCategory(std::string categoryToOpen, ScriptingWindow* scriptWindow, MainWindow* mainWin);
		void updateCategorySavedStatus(bool isSaved);
		bool categorySettinsReadyCheck();
		bool checkCategorySave(std::string prompt, MainWindow* mainWin);
		void categoryChangeHandler(ScriptingWindow* scriptWindow, MainWindow* mainWin);
		std::string getCurrentCategory();
		std::string getCurrentPathIncludingCategory();
		profileSettings getProfileSettings();

		static std::vector<std::string> searchForFiles(std::string locationToSearch, std::string extensions);
		static void reloadCombo(HWND comboToReload, std::string locationToLook, std::string extension, std::string nameToLoad);
		std::string getComboText();
		bool fileOrFolderExists( std::string filePathway );
		//void updateSaveStatus(bool savedStatus);
		void fullyDeleteFolder(std::string folderToDelete);
		void initialize( POINT& topLeftPosition, CWnd* parent, int& id, cToolTips& tooltips );
		void rearrange(int width, int height, fontMap fonts);
		static void checkDelimiterLine(std::ifstream& openFile, std::string keyword);
		static bool checkDelimiterLine( std::ifstream& openFile, std::string delimiter, std::string breakCondition );
	private:
		profileSettings currentProfile;
		std::string FILE_SYSTEM_PATH;
		bool configurationIsSaved;
		bool categoryIsSaved;
		bool experimentIsSaved;
		bool sequenceIsSaved;
		
		// combine for version. e.g. 2.5
		const int versionMain = 2;
		const int versionSub = 5;

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
