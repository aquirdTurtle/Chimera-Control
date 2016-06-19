#include "stdafx.h"
#include "ConfigurationFileSystem.h"
#include "Windows.h"
#include <fstream>
#include "constants.h"
#include "Resource.h"
#include "appendText.h"
#include "Commctrl.h"
#include "textPromptDialogProcedure.h"
#include "fonts.h"
#include <boost/filesystem.hpp>
#include "myNIAWG.h"
#include "MasterWindow.h"

ConfigurationFileSystem::ConfigurationFileSystem(std::string fileSystemPath)
{
	FILE_SYSTEM_PATH = fileSystemPath;
	currentProfileSettings.orientation = HORIZONTAL_ORIENTATION;
}
ConfigurationFileSystem::~ConfigurationFileSystem()
{
	// nothing for destructor right now
}

bool ConfigurationFileSystem::saveEntireProfile(MasterWindow& Master)
{
	// save experiment
	if (this->saveExperimentOnly(Master))
	{
		return true;
	}
	// save category
	if (this->saveCategoryOnly(Master))
	{
		return true;
	}
	// save configuration
	if (this->saveConfigurationOnly(Master))
	{
		return true;
	}
	// save sequence
	if (this->saveSequence(Master))
	{
		return true;
	}
	return false;
}
bool ConfigurationFileSystem::checkSaveEntireProfile(MasterWindow& Master)
{
	if (this->checkExperimentSave("Save Experiment Settings?", Master))
	{
		return true;
	}
	if (this->checkCategorySave("Save Category Settings?", Master))
	{
		return true;
	}
	if (this->checkConfigurationSave("Save Configuration Settings?", Master))
	{
		return true;
	}
	if (this->checkSequenceSave("Save Sequence Settings?", Master))
	{
		return true;
	}
	return false;
}

bool ConfigurationFileSystem::allSettingsReadyCheck(MasterWindow& Master)
{
	// check all components of this class.
	if (this->experimentSettingsReadyCheck(Master))
	{
		return true;
	}
	else if (this->categorySettingsReadyCheck(Master))
	{
		return true;
	}
	else if (this->configurationSettingsReadyCheck(Master))
	{
		return true;
	}
	else if (this->sequenceSettingsReadyCheck(Master))
	{
		return true;
	}
	// passed all checks.
	return false;
}

bool ConfigurationFileSystem::reloadAllCombos()
{
	return false;
}

/// ORIENTATION HANDLING

std::string ConfigurationFileSystem::getOrientation()
{
	return currentProfileSettings.orientation;
}

bool ConfigurationFileSystem::setOrientation(std::string orientation)
{
	if (orientation != HORIZONTAL_ORIENTATION && orientation != VERTICAL_ORIENTATION)
	{
		MessageBox(0, "ERROR: Tried to set non-standard orientation! Ask Mark about bugs.", 0, 0);
		return true;
	}
	currentProfileSettings.orientation = orientation;
	return false;
}

bool ConfigurationFileSystem::orientationChangeHandler(MasterWindow& Master)
{
	long long ItemIndex = SendMessage(orientationCombo.hwnd, CB_GETCURSEL, 0, 0);
	TCHAR orientation[256];
	SendMessage(orientationCombo.hwnd, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)orientation);
	// reset some things.
	currentProfileSettings.orientation = std::string(orientation);
	if (currentProfileSettings.category != "")
	{
		if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
		{
			this->reloadCombo(configCombo.hwnd, pathIncludingCategory, std::string("*") + HORIZONTAL_EXTENSION, "__NONE__");
		}
		else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
		{
			this->reloadCombo(configCombo.hwnd, pathIncludingCategory, std::string("*") + VERTICAL_EXTENSION, "__NONE__");
		}
	}
	Master.notes.setConfigurationNotes("");
	currentProfileSettings.configuration = "";
	/// Load the relevant NIAWG script.
	myNIAWG::loadDefault();

	return false;
}

/// CONFIGURATION LEVEL HANDLING

bool ConfigurationFileSystem::newConfiguration(MasterWindow& Master)
{
	// check if category has been set yet.
	if (currentProfileSettings.category == "")
	{
		// check if the experiment has also not been set.
		if (currentProfileSettings.experiment == "")
		{
			MessageBox(0, "The Experiment and category have not yet been selected! Please select a category or create a new one before trying to save this "
				"configuration.", 0, 0);
		}
		else
		{
			MessageBox(0, "The category has not yet been selected! Please select a category or create a new one before trying to save this configuration.",
				0, 0);
		}
		return false;
	}

	std::string configurationNameToSave = (const char*)DialogBoxParam(Master.programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new configuration name.");
	if (configurationNameToSave == "")
	{
		// canceled
		return false;
	}
	std::string newConfigurationPath = pathIncludingCategory + configurationNameToSave;

	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		newConfigurationPath += HORIZONTAL_EXTENSION;
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		newConfigurationPath += VERTICAL_EXTENSION;
	}
	else
	{
		MessageBox(0, "ERROR: Unrecognized orientation! Ask Mark about bugs.", 0, 0);
	}
	std::ofstream newConfigurationFile(newConfigurationPath.c_str());
	if (!newConfigurationFile.is_open())
	{
		MessageBox(0, "ERROR: Failed to create new configuration file. Ask Mark about bugs.", 0, 0);
		return true;
	}
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		this->reloadCombo(configCombo.hwnd, pathIncludingCategory, std::string("*") + HORIZONTAL_EXTENSION, currentProfileSettings.configuration.c_str());
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		this->reloadCombo(configCombo.hwnd, pathIncludingCategory, std::string("*") + VERTICAL_EXTENSION, currentProfileSettings.configuration.c_str());
	}
	else
	{
		MessageBox(0, "ERROR: Unrecognized orientation! Ask Mark about bugs.", 0, 0);
	}
	return false;
}

/*
]--- This function opens a given configuration file, sets all of the relevant parameters, and loads the associated scripts. 
*/
bool ConfigurationFileSystem::openConfiguration(std::string configurationNameToOpen, MasterWindow& Master)
{
	// no folder associated with configuraitons. They share the category folder.
	std::string path = pathIncludingCategory + configurationNameToOpen;
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		path += HORIZONTAL_EXTENSION;
	}
	else
	{
		path += VERTICAL_EXTENSION;
	}
	std::ifstream configurationFile(path.c_str());
	// check if opened correctly.
	if (!configurationFile.is_open())
	{
		MessageBox(0, "Opening of Configuration File Failed!", 0, 0);
		return true;
	}
	currentProfileSettings.configuration = configurationNameToOpen;
	
	// not currenlty used because version 1.0.
	std::string version;
	std::getline(configurationFile, version);

	/// Get Variables
	// Number of Variables
	Master.variables.clearVariables();
	int varNum;
	configurationFile >> varNum;
	if (varNum < 0 || varNum > 10)
	{
		int answer = MessageBox(0, ("ERROR: variable number retrieved from file appears suspicious. The number is " + std::to_string(varNum) + ". Is this accurate?").c_str(), 0, MB_YESNO);
		if (answer == IDNO)
		{
			// don't try to load anything.
			varNum = 0;
		}
	}
	// early version didn't have variable type indicators.
	if (version == "Version: 1.0")
	{
		for (int varInc = 0; varInc < varNum; varInc++)
		{
			std::string varName;
			configurationFile >> varName;
			Master.variables.addVariable(varName, false, false, 0, varInc);
		}
	}
	else if (version == "Version: 1.1")
	{
		for (int varInc = 0; varInc < varNum; varInc++)
		{

			std::string varName, timelikeText, typeText, valueString;
			bool timelike;
			bool singleton;
			double value;
			configurationFile >> varName;
			configurationFile >> timelikeText;
			configurationFile >> typeText;
			configurationFile >> valueString;
			if (timelikeText == "Timelike")
			{
				timelike = true;
			}
			else if (timelikeText == "Not_Timelike")
			{
				timelike = false;
			}
			else
			{
				MessageBox(0, "ERROR: unknown timelike option. Check the formatting of the configuration file.", 0, 0);
				return true;
			}
			if (typeText == "Singleton")
			{
				singleton = true;
			}
			else if (typeText == "From_Master")
			{
				singleton = false;
			}
			else
			{
				MessageBox(0, "ERROR: unknown variable type option. Check the formatting of the configuration file.", 0, 0);
				return true;
			}
			try
			{
				value = std::stod(valueString);
			}
			catch (std::invalid_argument& exception)
			{
				MessageBox(0, ("ERROR: Failed to convert value in configuration file for variable's double value. Value was: " + valueString).c_str(), 0, 0);
				break;
			}
			Master.variables.addVariable(varName, timelike, singleton, value, varInc);
		}
	}
	else if (version == "")
	{
		// nothing
	}
	else
	{
		MessageBox(0, "ERROR: Unrecognized configuration version! Ask Mark about bugs.", 0, 0);
		return true;
	}
	// add a blank line
	Master.variables.addVariable("", false, false, 0, varNum);
	this->updateConfigurationSavedStatus(true);
	currentProfileSettings.configuration = configurationNameToOpen;
	std::string notes;
	std::string tempNote;
	// no need to get a newline since this should be he first thing in the file.
	configurationFile.get();
	std::getline(configurationFile, tempNote);
	if (tempNote != "END CONFIGURATION NOTES")
	{
		while (configurationFile && tempNote != "END CONFIGURATION NOTES")
		{
			notes += tempNote + "\r\n";
			std::getline(configurationFile, tempNote);
		}
		if (notes.size() > 2)
		{
			notes = notes.substr(0, notes.size() - 2);
		}
		Master.notes.setConfigurationNotes(notes);
	}
	else
	{
		Master.notes.setConfigurationNotes("");
	}
	this->updateConfigurationSavedStatus(true);
	// actually set this now
	//SetWindowText(eConfigurationDisplayInScripting, (currentProfileSettings.category + "->" + currentProfileSettings.configuration).c_str());
	// close.
	configurationFile.close();
	if (currentProfileSettings.sequence == NULL_SEQUENCE)
	{
		// reload it.
		this->loadNullSequence(Master);
	}
	return false;
}

/*
]--- This function attempts to save the configuration given the configuration name in the argument. It throws errors and warnings if this is not a "normal" 
]- Save, i.e. if the file doesn't already exist or if the user tries to pass an empty name as an argument. It returns false if the configuration got saved,
]- true if something prevented the configuration from being saved.
*/
bool ConfigurationFileSystem::saveConfigurationOnly(MasterWindow& Master)
{
	std::string configurationNameToSave = currentProfileSettings.configuration;
	// check if category has been set yet.
	if (currentProfileSettings.category == "")
	{
		// check if the experiment has also not been set.
		if (currentProfileSettings.experiment == "")
		{
			MessageBox(0, "The Experiment and category have not yet been selected! Please select a category or create a new one before trying to save this "
				"configuration.", 0, 0);
		}
		else
		{
			MessageBox(0, "The category has not yet been selected! Please select a category or create a new one before trying to save this configuration.",
				0, 0);
		}
		return false;
	}
	// check to make sure that this is a name.
	if (configurationNameToSave == "")
	{
		MessageBox(0, "ERROR: The program requested the saving of the configuration file to an empty name! This shouldn't happen, ask Mark about bugs.", 0, 0);
		return true;
	}

	// check if file already exists
	std::string extension;
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		extension = HORIZONTAL_EXTENSION;
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		extension = VERTICAL_EXTENSION;
	}
	else
	{
		MessageBox(0, "ERROR: unrecognized orientation! Ask Mark about bugs.", 0, 0);
		return true;
	}

	if (!ConfigurationFileSystem::fileOrFolderExists(pathIncludingCategory + configurationNameToSave + extension))
	{
		int answer = MessageBox(0, ("This configuration file appears to not exist in the expected location: " + pathIncludingCategory + configurationNameToSave 
			+ extension + ". Continue by making a new configuration file?").c_str(), 0, MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return false;
		}
	}
	
	if (!experimentIsSaved)
	{
		int answer = MessageBox(0, "The Experiment settings have not yet been saved. Save them before the configuration? (Optional)", 0, MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return false;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			this->saveExperimentOnly(Master);
		}
	}
	if (!categoryIsSaved)
	{
		int answer = MessageBox(0, "The Category settings have not yet been saved. Save them before the configuration? (Optional)", 0, MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return false;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			this->saveCategoryOnly(Master);
		}
	}
	
	// else open it.
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		extension = HORIZONTAL_EXTENSION;
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		extension = VERTICAL_EXTENSION;
	}
	else
	{
		MessageBox(0, "ERROR: Unrecognized orientation! Ask Mark about bugs.", 0, 0);
		return true;
	}
	std::ofstream configurationSaveFile(pathIncludingCategory + configurationNameToSave + extension);
	if (!configurationSaveFile.is_open())
	{
		MessageBox(0, "Couldn't save configuration file! Check the name for weird characters, or call Mark about bugs if everything seems right...", 0, 0);
		return false;
	}
	// That's the last prompt the user gets, so the save is final now.
	currentProfileSettings.configuration = configurationNameToSave;
	configurationSaveFile << "Version: 1.1\n";
	// Number of Variables
	configurationSaveFile << Master.variables.getCurrentNumberOfVariables() << "\n";
	// Variable Names
	// This part changed in version 1.1.
	for (int varInc = 0; varInc < Master.variables.getCurrentNumberOfVariables(); varInc++)
	{
		variable info = Master.variables.getVariableInfo(varInc);
		configurationSaveFile << info.name << " ";
		if (info.timelike)
		{
			configurationSaveFile << "Timelike ";
		}
		else
		{
			configurationSaveFile << "Not_Timelike ";
		}
		if (info.singleton)
		{
			configurationSaveFile << "Singleton ";
		}
		else
		{
			configurationSaveFile << "From_Master ";
		}
		configurationSaveFile << info.initialValue << "\n";
	}
	std::string notes = Master.notes.getConfigurationNotes();
	configurationSaveFile << notes + "\n";
	configurationSaveFile << "END CONFIGURATION NOTES" << "\n";
	configurationSaveFile.close();
	this->updateConfigurationSavedStatus(true);
	return false;
}

/*
]--- Identical to saveConfigurationOnly except that it prompts the user for a name with a dialog box instead of taking one.
*/
bool ConfigurationFileSystem::saveConfigurationAs(MasterWindow& Master)
{
	// check if category has been set yet.
	if (currentProfileSettings.category == "")
	{
		// check if the experiment has also not been set.
		if (currentProfileSettings.experiment == "")
		{
			MessageBox(0, "The Experiment and category have not yet been selected! Please select a category or create a new one before trying to save this "
				"configuration.", 0, 0);
		}
		else
		{
			MessageBox(0, "The category has not yet been selected! Please select a category or create a new one before trying to save this configuration.",
				0, 0);
		}
		return false;
	}
	std::string configurationNameToSave = (const char*)DialogBoxParam(Master.programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new configuration name.");
	if (configurationNameToSave == "")
	{
		// canceled
		return 0;
	}

	// check to make sure that this is a name.
	if (configurationNameToSave == "")
	{
		MessageBox(0, "ERROR: The program requested the saving of the configuration file to an empty name! This shouldn't happen, ask Mark about bugs.", 0, 0);
		return true;
	}
	
	std::string extension;
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		extension = HORIZONTAL_EXTENSION;
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		extension = VERTICAL_EXTENSION;
	}
	else
	{
		MessageBox(0, "ERROR: Unrecognized orientation! Ask Mark about bugs.", 0, 0);
		return true;
	}

	// check if file already exists
	if (ConfigurationFileSystem::fileOrFolderExists(pathIncludingCategory + configurationNameToSave + extension))
	{
		int answer = MessageBox(0, "This configuration file name already exists! Overwrite it?", 0, MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return true;
		}
	}

	if (!experimentIsSaved)
	{
		int answer = MessageBox(0, "The Experiment settings have not yet been saved. Save them before the configuration? (Optional)", 0, MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return false;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			this->saveExperimentOnly(Master);
		}
	}
	if (!categoryIsSaved)
	{
		int answer = MessageBox(0, "The Category settings have not yet been saved. Save them before the configuration? (Optional)", 0, MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return false;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			this->saveCategoryOnly(Master);
		}
	}

	// else open it.
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		extension = HORIZONTAL_EXTENSION;
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		extension = VERTICAL_EXTENSION;
	}
	else
	{
		MessageBox(0, "ERROR: Unrecognized orientation! Ask Mark about bugs.", 0, 0);
		return true;
	}
	std::ofstream configurationSaveFile(pathIncludingCategory + configurationNameToSave + extension);
	if (!configurationSaveFile.is_open())
	{
		MessageBox(0, "Couldn't save configuration file! Check the name for weird characters, or call Mark about bugs if everything seems right...", 0, 0);
		return false;
	}

	// That's the last prompt the user gets, so the save is final now.
	currentProfileSettings.configuration = configurationNameToSave;
	// Version info tells future code about formatting.
	configurationSaveFile << "Version: 1.1\n";
\
	configurationSaveFile << Master.variables.getCurrentNumberOfVariables() << "\n";
	// Variable Names
	// This part changed in version 1.1.
	for (int varInc = 0; varInc < Master.variables.getCurrentNumberOfVariables(); varInc++)
	{
		variable info = Master.variables.getVariableInfo(varInc);
		configurationSaveFile << info.name << " ";
		if (info.timelike)
		{
			configurationSaveFile << "Timelike ";
		}
		else
		{
			configurationSaveFile << "Not_Timelike ";
		}
		if (info.singleton)
		{
			configurationSaveFile << "Singleton ";
		}
		else
		{
			configurationSaveFile << "From_Master ";
		}
		configurationSaveFile << info.initialValue << "\n";
	}
	std::string notes = Master.notes.getConfigurationNotes();
	configurationSaveFile << notes + "\n";
	configurationSaveFile << "END CONFIGURATION NOTES" << "\n";
	configurationSaveFile.close();
	this->reloadCombo(configCombo.hwnd, pathIncludingCategory, "*" + extension, currentProfileSettings.configuration);
	this->updateConfigurationSavedStatus(true);

	return 0;
}

/*
]--- This function renames the currently set 
*/
bool ConfigurationFileSystem::renameConfiguration(MasterWindow& Master)
{
	// check if configuration has been set yet.
	if (currentProfileSettings.configuration == "")
	{
		if (currentProfileSettings.category == "")
		{
			// check if the experiment has also not been set.
			if (currentProfileSettings.experiment == "")
			{
				MessageBox(0, "The Experiment and category have not yet been selected! Please select a category or create a new one before trying to save this "
					"configuration.", 0, 0);
			}
			else
			{
				MessageBox(0, "The category has not yet been selected! Please select a category or create a new one before trying to save this configuration.",
					0, 0);
			}
			return false;
		}
		else
		{
			MessageBox(0, "The Configuration has not yet been selected! Please select a category or create a new one before trying to rename it.", 0, 0);
		}
	}
	std::string newConfigurationName = (const char*)DialogBoxParam(Master.programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new configuration name.");
	if (newConfigurationName == "")
	{
		// canceled
		return true;
	}
	std::string currentConfigurationLocation = pathIncludingCategory + currentProfileSettings.configuration;
	std::string newConfigurationLocation = pathIncludingCategory + newConfigurationName;
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		currentConfigurationLocation += HORIZONTAL_EXTENSION;
		newConfigurationLocation += HORIZONTAL_EXTENSION;
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		currentConfigurationLocation += VERTICAL_EXTENSION;
		newConfigurationLocation += VERTICAL_EXTENSION;
	}
	else
	{
		MessageBox(0, "ERROR: Orientation Unrecognized! Ask Mark about bugs.", 0, 0);
		return true;
	}
	int result = MoveFile(currentConfigurationLocation.c_str(), newConfigurationLocation.c_str());
	if (result == 0)
	{
		MessageBox(0, "Renaming of the configuration file Failed! Ask Mark about bugs", 0, 0);
		return true;
	}
	currentProfileSettings.configuration = newConfigurationName;
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		this->reloadCombo(configCombo.hwnd, pathIncludingCategory, std::string("*") + HORIZONTAL_EXTENSION, "__NONE__");
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		this->reloadCombo(configCombo.hwnd, pathIncludingCategory, std::string("*") + VERTICAL_EXTENSION, "__NONE__");
	}
	else
	{
		MessageBox(0, "ERROR: Orientation Unrecognized while reloading config combo! Ask Mark about bugs.", 0, 0);
		return true;
	}
	return false;
}

/*
]--- 
*/
bool ConfigurationFileSystem::deleteConfiguration()
{
	// check if configuration has been set yet.
	if (currentProfileSettings.configuration == "")
	{
		if (currentProfileSettings.category == "")
		{
			// check if the experiment has also not been set.
			if (currentProfileSettings.experiment == "")
			{
				MessageBox(0, "The Experiment and category have not yet been selected! Please select a category or create a new one before trying to save this "
					"configuration.", 0, 0);
			}
			else
			{
				MessageBox(0, "The category has not yet been selected! Please select a category or create a new one before trying to save this configuration.",
					0, 0);
			}
			return false;
		}
		else
		{
			MessageBox(0, "The Configuration has not yet been selected! Please select a category or create a new one before trying to rename it.", 0, 0);
		}
	}
	int answer = MessageBox(0, ("Are you sure you want to delete the current configuration: " + currentProfileSettings.configuration).c_str(), 0, MB_YESNO);
	if (answer == IDNO)
	{
		return true;
	}
	std::string currentConfigurationLocation = pathIncludingCategory + currentProfileSettings.configuration;
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		currentConfigurationLocation += HORIZONTAL_EXTENSION;
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		currentConfigurationLocation += VERTICAL_EXTENSION;
	}
	else
	{
		MessageBox(0, "ERROR: Invalid orientation! Ask Mark about bugs.", 0, 0);
		return true;
	}
	int result = DeleteFile(currentConfigurationLocation.c_str());
	if (result == 0)
	{
		MessageBox(0, "ERROR: Deleteing the configuration file failed!", 0, 0);
		return true;
	}
	// since the configuration this (may have been) was saved to is gone, no saved version of current code.
	this->updateConfigurationSavedStatus(false);
	// just deleted the current configuration
	currentProfileSettings.configuration = "";
	// reset combo since the files have now changed after delete
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		this->reloadCombo(configCombo.hwnd, pathIncludingCategory, std::string("*") + HORIZONTAL_EXTENSION, "__NONE__");
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		this->reloadCombo(configCombo.hwnd, pathIncludingCategory, std::string("*") + VERTICAL_EXTENSION, "__NONE__");
	}
	else
	{
		MessageBox(0, "ERROR: unrecognized orientation while resetting combobox! Ask Mark about bugs.", 0, 0);
			return true;
	}
	return false;
}

/*
]--- 
*/
bool ConfigurationFileSystem::updateConfigurationSavedStatus(bool isSaved)
{
	configurationIsSaved = isSaved;
	if (isSaved)
	{
		SendMessage(configurationSavedIndicator.hwnd, BM_SETCHECK, BST_CHECKED, 0);
	}
	else
	{
		SendMessage(configurationSavedIndicator.hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	return false;
}

bool ConfigurationFileSystem::configurationSettingsReadyCheck(MasterWindow& Master)
{
	if (!configurationIsSaved)
	{
		// prompt for save.
		if (this->checkConfigurationSave("There are unsaved configuration settings. Would you like to save the current configuration before starting?", Master))
		{
			// canceled
			return true;
		}
	}
	return false;
}

bool ConfigurationFileSystem::checkConfigurationSave(std::string prompt, MasterWindow& Master)
{
	int answer = MessageBox(0, prompt.c_str(), 0, MB_YESNOCANCEL);
	if (answer == IDYES)
	{
		this->saveConfigurationOnly(Master);
	}
	else if (answer == IDCANCEL)
	{
		return true;
	}
	return false;
}

bool ConfigurationFileSystem::configurationChangeHandler(MasterWindow& Master)
{
	if (!configurationIsSaved)
	{
		if (this->checkConfigurationSave("The current configuration is unsaved. Save current configuration before changing?", Master))
		{
			SendMessage(configCombo.hwnd, CB_SELECTSTRING, 0, (LPARAM)currentProfileSettings.configuration.c_str());
			return true;
		}
	}
	// get current item.
	long long itemIndex = SendMessage(configCombo.hwnd, CB_GETCURSEL, 0, 0);
	if (itemIndex == -1)
	{
		// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
		// is blank. Just break out, this is fine.
		return true;
	}
	TCHAR configurationToOpen[256];
	// Send CB_GETLBTEXT message to get the item.
	SendMessage(configCombo.hwnd, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)configurationToOpen);
	if (this->openConfiguration(configurationToOpen, Master))
	{
		return true;
	}
	// it'd be confusing if these notes stayed here.
	return false;
}
/// CATEGORY LEVEL HANDLING
/*
]--- This function attempts to save the category given the category name in the argument. It throws errors and warnings if this is not a "normal"
]- Save, i.e. if the file doesn't already exist or if the user tries to pass an empty name as an argument. It returns false if the category got saved,
]- true if something prevented the category from being saved.
*/
bool ConfigurationFileSystem::saveCategoryOnly(MasterWindow& Master)
{
	std::string categoryNameToSave = currentProfileSettings.category;
	// check if experiment has been set
	if (currentProfileSettings.experiment == "")
	{
		MessageBox(0, "The Experiment has not yet been selected! Please select a Experiment or create a new one before trying to save this "
			"category.", 0, 0);
		return true;
	}
	// check to make sure that this is a name.
	if (categoryNameToSave == "")
	{
		MessageBox(0, "ERROR: The program requested the saving of the category file to an empty name! This shouldn't happen, ask Mark about bugs.", 0, 0);
		return true;
	}

	// check if file already exists. No extension, looking for a folder here. 
	if (!ConfigurationFileSystem::fileOrFolderExists(pathIncludingCategory + categoryNameToSave + CATEGORY_EXTENSION))
	{
		int answer = MessageBox(0, ("This category file appears to not exist in the expected location: " + pathIncludingCategory + categoryNameToSave 
			+ CATEGORY_EXTENSION + ".  Continue by making a new category file?").c_str(), 0, MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return true;
		}
	}

	if (!experimentIsSaved)
	{
		int answer = MessageBox(0, "The Experiment settings have not yet been saved. Save them before the category? (Optional)", 0, MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return true;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			if (this->saveExperimentOnly(Master))
			{
				return true;
			}
		}
	}
	std::fstream categoryFileToSave(pathIncludingCategory + categoryNameToSave + CATEGORY_EXTENSION, std::ios::out);
	if (!categoryFileToSave.is_open())
	{
		MessageBox(0, "ERROR: failed to save category file! Ask mark about bugs.", 0, 0);
		return true;
	}
	categoryFileToSave << "Version: 1.0\n";
	std::string categoryNotes = Master.notes.getCategoryNotes();
	categoryFileToSave << categoryNotes + "\n";
	categoryFileToSave << "END CATEGORY NOTES\n";
	currentProfileSettings.category = categoryNameToSave;
	pathIncludingCategory = pathIncludingExperiment + categoryNameToSave + "\\";
	this->updateCategorySavedStatus(true);
	return false;
}

std::string ConfigurationFileSystem::getCurrentPathIncludingCategory()
{
	return pathIncludingCategory;
}

/*
]--- identical to saveCategoryOnly except that 
*/
bool ConfigurationFileSystem::saveCategoryAs(MasterWindow& Master)
{
	// check if experiment has been set
	if (currentProfileSettings.experiment == "")
	{
		MessageBox(0, "The Experiment has not yet been selected! Please select a Experiment or create a new one before trying to save this "
			"category.", 0, 0);
		return true;
	}
	std::string categoryNameToSave = (const char*)DialogBoxParam(Master.programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new configuration name.");
	if (categoryNameToSave == "")
	{
		MessageBox(0, "ERROR: The program requested the saving of the category file to an empty name! This shouldn't happen, ask Mark about bugs.", 0, 0);
		return true;
	}

	// check if file already exists. No extension, looking for a folder here. 
	if (!ConfigurationFileSystem::fileOrFolderExists(pathIncludingCategory + categoryNameToSave + CATEGORY_EXTENSION))
	{
		int answer = MessageBox(0, ("This category file appears to not exist in the expected location: " + pathIncludingCategory + categoryNameToSave
			+ CATEGORY_EXTENSION + ".  Continue by making a new category file?").c_str(), 0, MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return true;
		}
	}

	if (!experimentIsSaved)
	{
		int answer = MessageBox(0, "The Experiment settings have not yet been saved. Save them before the category? (Optional)", 0, MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return true;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			if (this->saveExperimentOnly(Master))
			{
				return true;
			}
		}
	}
	// need to make a new folder as well.
	int result = CreateDirectory((pathIncludingExperiment + categoryNameToSave).c_str(), 0);
	if (result == 0)
	{
		MessageBox(0, "ERROR: failed to create new category directory during category save as! Ask Mark about Bugs.", 0, 0);
		return true;
	}
	std::fstream categoryFileToSave(pathIncludingCategory + categoryNameToSave + CATEGORY_EXTENSION, std::ios::out);
	if (!categoryFileToSave.is_open())
	{
		MessageBox(0, "ERROR: failed to save category file! Ask mark about bugs.", 0, 0);
		return true;
	}
	categoryFileToSave << "Version: 1.0\n";
	std::string categoryNotes = Master.notes.getCategoryNotes();
	categoryFileToSave << categoryNotes + "\n";
	categoryFileToSave << "END CATEGORY NOTES\n";
	currentProfileSettings.category = categoryNameToSave;
	pathIncludingCategory = pathIncludingExperiment + categoryNameToSave + "\\";
	this->updateCategorySavedStatus(true);
	return false;
}

/*
]---
*/
bool ConfigurationFileSystem::renameCategory()
{
	// TODO: this is a bit more complicated because of the way that all of the configuration fle locations are currently set.
	MessageBox(0, "This feature still needs implementing! It doesn't work right now", 0, 0);
	return false;
}

/*
]---
*/
bool ConfigurationFileSystem::deleteCategory()
{
	// check if category has been set yet.
	if (currentProfileSettings.category == "")
	{
		// check if the experiment has also not been set.
		if (currentProfileSettings.experiment == "")
		{
			MessageBox(0, "The Experiment and category have not yet been selected! Please select a category or create a new one before trying to save this "
				"category.", 0, 0);
		}
		else
		{
			MessageBox(0, "The category has not yet been selected! Please select a category or create a new one before trying to save this category.",
				0, 0);
		}
		return false;
	}
	int answer = MessageBox(0, ("Are you sure you want to delete the current Category and all configurations within? The current category is: " + currentProfileSettings.category).c_str(), 0, MB_YESNO);
	if (answer == IDNO)
	{
		return true;
	}
	answer = MessageBox(0, ("Are you really sure? The current category is: " + currentProfileSettings.category).c_str(), 0, MB_YESNO);
	if (answer == IDNO)
	{
		return true;
	}
	std::string currentCategoryLocation = pathIncludingExperiment + currentProfileSettings.category;
	if (this->fullyDeleteFolder(currentCategoryLocation))
	{
		return true;
	}
	this->updateCategorySavedStatus(false);
	currentProfileSettings.category = "";
	pathIncludingCategory == "";
	this->reloadCombo(categoryCombo.hwnd, pathIncludingExperiment, "*", "__NONE__");
	return false;
}

bool ConfigurationFileSystem::newCategory(MasterWindow& Master)
{
	// check if experiment has been set
	if (currentProfileSettings.experiment == "")
	{
		MessageBox(0, "The Experiment has not yet been selected! Please select a Experiment or create a new one before trying to save this "
			"category.", 0, 0);
		return true;
	}
	std::string categoryNameToSave = (const char*)DialogBoxParam(Master.programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0,
		(DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter name for the new Category.");
	// check to make sure that this is a name.
	if (categoryNameToSave == "")
	{
		// canceled
		return true;
	}

	// check if file already exists. No extension, looking for a folder here. 
	if (ConfigurationFileSystem::fileOrFolderExists(pathIncludingExperiment + categoryNameToSave))
	{
		MessageBox(0, "This category name already exists! If it doesn't appear in the combo, try taking a look at what's in the relvant folder...", 0, 0);
		return true;
	}
	int result = CreateDirectory((pathIncludingExperiment + categoryNameToSave).c_str(), 0);
	if (result == 0)
	{
		MessageBox(0, "ERROR: failed to create category directory! Ask Mark about bugs.", 0, 0);
		return true;
	}
	std::ofstream categorySaveFolder(pathIncludingExperiment + categoryNameToSave + "\\" + categoryNameToSave + CATEGORY_EXTENSION);
	categorySaveFolder.close();
	this->reloadCombo(categoryCombo.hwnd, pathIncludingExperiment, "*", currentProfileSettings.category);
	return false;
}

bool ConfigurationFileSystem::openCategory(std::string categoryToOpen, MasterWindow& Master)
{
	// this gets called from the file menu.
	// Assign based on the comboBox Item entry.
	std::string path = pathIncludingExperiment + categoryToOpen + "\\" + categoryToOpen + CATEGORY_EXTENSION;
	std::ifstream categoryConfigOpenFile(path.c_str());
	// check if opened correctly.
	if (!categoryConfigOpenFile.is_open())
	{
		MessageBox(0, "Opening of Category Configuration File Failed!", 0, 0);
		return false;
	}
	currentProfileSettings.category = categoryToOpen;
	pathIncludingCategory = pathIncludingExperiment + categoryToOpen + "\\";
	/// Set the Configuration combobox.
	// Get all files in the relevant directory.
	std::vector<std::string> configurationNames;

	std::string notes;
	std::string tempNote;
	std::string version;
	std::getline(categoryConfigOpenFile, version);
	categoryConfigOpenFile.get();
	
	std::getline(categoryConfigOpenFile, tempNote);
	if (tempNote != "END CATEGORY NOTES")
	{
		while (categoryConfigOpenFile && tempNote != "END CATEGORY NOTES")
		{
			notes += tempNote + "\r\n";
			std::getline(categoryConfigOpenFile, tempNote);
		}
		Master.notes.setCategoryNotes(notes);
	}
	else
	{
		Master.notes.setCategoryNotes("");
	}
	//SetWindowText(eConfigurationDisplayInScripting, "");
	// close.
	categoryConfigOpenFile.close();
	this->updateCategorySavedStatus(true);
	return false;
}

bool ConfigurationFileSystem::updateCategorySavedStatus(bool isSaved)
{
	categoryIsSaved = isSaved;
	if (isSaved)
	{
		SendMessage(categorySavedIndicator.hwnd, BM_SETCHECK, BST_CHECKED, 0);
	}
	else
	{
		SendMessage(categorySavedIndicator.hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	return false;
}

bool ConfigurationFileSystem::categorySettingsReadyCheck(MasterWindow& Master)
{
	if (!categoryIsSaved)
	{
		if (this->checkSequenceSave("There are unsaved category settings. Would you like to save the current category before starting?", Master))
		{
			// canceled
			return true;
		}
	}
	return false;
}

bool ConfigurationFileSystem::checkCategorySave(std::string prompt, MasterWindow& Master)
{
	int answer = MessageBox(0, prompt.c_str(), 0, MB_YESNOCANCEL);
	if (answer == IDYES)
	{
		this->saveCategoryOnly(Master);
	}
	else if (answer == IDCANCEL)
	{
		return true;
	}
	return false;
}

bool ConfigurationFileSystem::categoryChangeHandler(MasterWindow& Master)
{
	if (!categoryIsSaved)
	{
		if (this->checkCategorySave("The current category is unsaved. Save current category before changing?", Master))
		{
			SendMessage(categoryCombo.hwnd, CB_SELECTSTRING, 0, (LPARAM)currentProfileSettings.category.c_str());
			return true;
		}
	}
	// get current item.
	long long itemIndex = SendMessage(categoryCombo.hwnd, CB_GETCURSEL, 0, 0);
	if (itemIndex == -1)
	{
		// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
		// is blank. Just break out, this is fine.
		return true;
	}
	TCHAR categoryConfigToOpen[256];
	// Send CB_GETLBTEXT message to get the item.
	SendMessage(categoryCombo.hwnd, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)categoryConfigToOpen);
	if (this->openCategory(std::string(categoryConfigToOpen), Master))
	{
		return true;
	}
	// it'd be confusing if these notes stayed here.
	Master.notes.setConfigurationNotes("");
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		this->reloadCombo(configCombo.hwnd, pathIncludingCategory, std::string("*") + HORIZONTAL_EXTENSION, "__NONE__");
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		this->reloadCombo(configCombo.hwnd, pathIncludingCategory, std::string("*") + VERTICAL_EXTENSION, "__NONE__");
	}
	currentProfileSettings.configuration = "";
	this->reloadSequence(NULL_SEQUENCE, Master);
	return false;
}

/// EXPERIMENT LEVEL HANDLING
bool ConfigurationFileSystem::saveExperimentOnly(MasterWindow& Master)
{
	std::string experimentNameToSave = currentProfileSettings.experiment;
	// check that the experiment name is not empty.
	if (experimentNameToSave == "")
	{
		MessageBox(0, "ERROR: Please properly select the experiment or create a new one (\'new experiment\') before trying to save it!", 0, 0);
		return true;
	}
	// check if file already exists
	if (!ConfigurationFileSystem::fileOrFolderExists(FILE_SYSTEM_PATH + experimentNameToSave + "\\" + experimentNameToSave + EXPERIMENT_EXTENSION))
	{
		int answer = MessageBox(0, ("This experiment file appears to not exist in the expected location: " + FILE_SYSTEM_PATH + "   \r\n."
			"Continue by making a new experiment file?").c_str(), 0, MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return true;
		}
	}
	std::ofstream experimentSaveFile(FILE_SYSTEM_PATH + experimentNameToSave + "\\" + experimentNameToSave + EXPERIMENT_EXTENSION);
	if (!experimentSaveFile.is_open())
	{
		MessageBox(0, "Couldn't save configuration file! Check the name for weird characters, or call Mark about bugs if everything seems right...", 0, 0);
		return true;
	}
	// That's the last prompt the user gets, so the save is final now.
	currentProfileSettings.experiment = experimentNameToSave;

	/// Start Outputting information
	/// THIS IS EXPERIMENT SAVED STUFF. T>T
	// this can be checked by reading functions to see what format to expect. From now on, a version will always be outputted at the beginning of the file.
	// (05/29/2016)
	experimentSaveFile << "Version: 1.0\n";
	// NOTE: Dummy variables used to be outputted here. 
	// notes.
	std::string notes = Master.notes.getExperimentNotes();
	experimentSaveFile << notes << "\n";
	experimentSaveFile << "END EXPERIMENT NOTES" << "\n";
	// And done.
	experimentSaveFile.close();
	currentProfileSettings.experiment = experimentNameToSave;
	// update the save path. 
	pathIncludingExperiment = FILE_SYSTEM_PATH + experimentNameToSave + "\\";
	// update the configuration saved statis for "this" object.
	this->updateExperimentSavedStatus(true);
	this->reloadCombo(experimentCombo.hwnd, FILE_SYSTEM_PATH, "*", currentProfileSettings.experiment);
	return false;
}

bool ConfigurationFileSystem::saveExperimentAs(MasterWindow& Master)
{
	if (currentProfileSettings.experiment == "")
	{
		MessageBox(0, "Please select an experiment before using \"Save As\"", 0, 0);
		return true;
	}
	std::string experimentNameToSave = (const char*)DialogBoxParam(Master.programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new experiment name.");
	// check that the experiment name is not empty. 
	if (experimentNameToSave == "")
	{
		// canceled.
		return true;
	}

	// check if file already exists
	if (ConfigurationFileSystem::fileOrFolderExists(FILE_SYSTEM_PATH + experimentNameToSave + ".eConfig"))
	{
		int answer = MessageBox(0, ("This experiment name appears to already exist in the expected location: " + FILE_SYSTEM_PATH + "."
			"Overwrite this file?").c_str(), 0, MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return true;
		}
	}

	int result = CreateDirectory((FILE_SYSTEM_PATH + experimentNameToSave).c_str(), 0);
	if (result == 0)
	{
		MessageBox(0, "ERROR: failed to create new experiment directory during save as! Ask Mark about bugs.", 0, 0);
		return true;
	}
	std::ofstream experimentSaveFile(FILE_SYSTEM_PATH + experimentNameToSave + "\\" + experimentNameToSave + ".eConfig");
	if (!experimentSaveFile.is_open())
	{
		MessageBox(0, "Couldn't save configuration file! Check the name for weird characters, or call Mark about bugs if everything seems right...", 0, 0);
		return false;
	}
	// That's the last prompt the user gets, so the save is final now.
	currentProfileSettings.experiment = experimentNameToSave;

	/// Start Outputting information
	/// THIS IS EXPERIMENT SAVED STUFF. T>T
	// this can be checked by reading functions to see what format to expect. From now on, a version will always be outputted at the beginning of the file.
	// (05/29/2016)
	experimentSaveFile << "Version: 1.0\n";
	// NOTE: Dummy variables used to be outputted here. 
	// notes.
	std::string notes = Master.notes.getExperimentNotes();
	experimentSaveFile << notes << "\n";
	experimentSaveFile << "END EXPERIMENT NOTES" << "\n";
	// And done.
	experimentSaveFile.close();
	// update the save path. 
	pathIncludingExperiment = FILE_SYSTEM_PATH + experimentNameToSave + "\\";
	// update the configuration saved statis for "this" object.
	this->updateExperimentSavedStatus(true);
	this->reloadCombo(experimentCombo.hwnd, FILE_SYSTEM_PATH, "*", currentProfileSettings.experiment);
	return false;
}

bool ConfigurationFileSystem::renameExperiment()
{
	MessageBox(0, "ERROR: This still needs implementing!", 0, 0);
	return false;
	/*
	// check if saved
	if (!experimentIsSaved)
	{
		if (checkExperimentSave("Save experiment before renaming it?", Master))
		{
			return true;
		}
	}
	// get name
	std::string experimentNameToSave = (const char*)DialogBoxParam(Master.programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0,
										(DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new experiment name.");
	// check if file already exists. No extension, looking for a folder here. 
	if (ConfigurationFileSystem::fileOrFolderExists(FILE_SYSTEM_PATH + experimentNameToSave))
	{
		int answer = MessageBox(0, "This experiment name already exists!", 0, MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return true;
		}
	}
	std::string newExperimentConfigLocation = FILE_SYSTEM_PATH + experimentNameToSave + "\\" + experimentNameToSave + EXPERIMENT_EXTENSION;
	std::string currentExperimentConfigLocation = FILE_SYSTEM_PATH + experimentNameToSave + "\\" + currentProfileSettings.experiment + EXPERIMENT_EXTENSION;
	int result = MoveFile((FILE_SYSTEM_PATH + currentProfileSettings.experiment).c_str(), (FILE_SYSTEM_PATH + experimentNameToSave).c_str());
	if (result == 0)
	{
		MessageBox(0, "Moving the experiment folder failed!", 0, 0);
		return true;
	}
	result = MoveFile(currentExperimentConfigLocation.c_str(), newExperimentConfigLocation.c_str());
	if (result == 0)
	{
		MessageBox(0, "Moving the experiment folder failed!", 0, 0);
		return true;
	}
	// TODO: program the code to go through all of the category and configuration file names and change addresses, or change format of how these are referenced 
	// in configuraiton file.
	currentProfileSettings.experiment = experimentNameToSave;
	this->reloadCombo(experimentCombo.hwnd, FILE_SYSTEM_PATH, "*", currentProfileSettings.experiment);
	return false;
	*/
}

bool ConfigurationFileSystem::deleteExperiment()
{
	if (currentProfileSettings.experiment == "")
	{
		MessageBox(0, "No experiment has been set!", 0, 0);
		return true;
	}
	int answer = MessageBox(0, ("Are you sure that you'd like to delete the current experiment and all categories and configurations within? Current Experiment: " + currentProfileSettings.experiment).c_str(), 0, MB_YESNO);
	if (answer == IDNO)
	{
		return true;
	}
	answer = MessageBox(0, ("Are you really really sure? Current Experiment: " + currentProfileSettings.experiment).c_str(), 0, MB_YESNO);
	if (answer == IDNO)
	{
		return true;
	}
	std::string experimentConfigLocation = FILE_SYSTEM_PATH + currentProfileSettings.experiment + "\\" + currentProfileSettings.experiment + EXPERIMENT_EXTENSION;
	if (DeleteFile(experimentConfigLocation.c_str()))
	{
		MessageBox(0, "Deleting .eConfig file failed! Ask Mark about bugs.", 0, 0);
		return true;
	}
	if (this->fullyDeleteFolder(pathIncludingExperiment + currentProfileSettings.experiment))
	{
		return true;
	}
	this->reloadCombo(experimentCombo.hwnd, FILE_SYSTEM_PATH, "*", "__NONE__");
	updateExperimentSavedStatus(false);
	currentProfileSettings.experiment = "";
	pathIncludingExperiment = "";
	return false;
}

bool ConfigurationFileSystem::newExperiment(MasterWindow& Master)
{
	std::string newExperimentName = (const char*)DialogBoxParam(Master.programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0,
																(DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter name for the new Experiment.");
	std::string newExperimentPath = FILE_SYSTEM_PATH + newExperimentName;
	CreateDirectory(newExperimentPath.c_str(), 0);
	std::ofstream newExperimentConfigFile;
	newExperimentConfigFile.open((newExperimentPath + "\\" + newExperimentName + EXPERIMENT_EXTENSION).c_str());
	this->reloadCombo(experimentCombo.hwnd, FILE_SYSTEM_PATH, "*", currentProfileSettings.experiment);
	return false;
}

bool ConfigurationFileSystem::openExperiment(std::string experimentToOpen, MasterWindow& Master)
{
	// this gets called from the file menu.
	// Assign based on the comboBox Item entry.
	std::string path = FILE_SYSTEM_PATH + experimentToOpen + "\\" + experimentToOpen + EXPERIMENT_EXTENSION;
	std::ifstream experimentConfigOpenFile(path.c_str());
	//C:\Users\Regal Lab\Documents\Quantum Gas Assembly Control\Profiles\Spectroscopy\Spectroscopy.eConfig
	//C:\Users\Regal Lab\Documents\NIAWG Control Application\Profiles\Spectroscopy
	// check if opened correctly.
	if (!experimentConfigOpenFile.is_open())
	{
		MessageBox(0, "Opening of Experiment Configuration File Failed!", 0, 0);
		return true;
	}
	currentProfileSettings.experiment = experimentToOpen;
	pathIncludingExperiment = FILE_SYSTEM_PATH + experimentToOpen + "\\";
	this->updateExperimentSavedStatus(true);
	/// Set the Configuration combobox.
	// Get all files in the relevant directory.
	std::vector<std::string> configurationNames;
	//this->reloadCombo(experimentCombo.hwnd, pathIncludingExperiment, CATEGORY_EXTENSION, "__NONE__");
	std::string version;
	std::getline(experimentConfigOpenFile, version);
	/// Load Values from the experiment config file.
	std::string notes;
	std::string tempNote;
	// get the trailing newline after the >> operation.
	experimentConfigOpenFile.get();
	std::getline(experimentConfigOpenFile, tempNote);
	if (tempNote != "END EXPERIMENT NOTES")
	{
		while (experimentConfigOpenFile && tempNote != "END EXPERIMENT NOTES")
		{
			notes += tempNote + "\r\n";
			std::getline(experimentConfigOpenFile, tempNote);
		}
		Master.notes.setExperimentNotes(notes);
	}
	else
	{
		Master.notes.setExperimentNotes("");
	}

	//SetWindowText(eConfigurationDisplayInScripting, "");
	// close.
	experimentConfigOpenFile.close();
	this->updateExperimentSavedStatus(true);
	return false;
}

bool ConfigurationFileSystem::updateExperimentSavedStatus(bool isSaved)
{
	experimentIsSaved = isSaved;
	if (isSaved)
	{
		SendMessage(experimentSavedIndicator.hwnd, BM_SETCHECK, BST_CHECKED, 0);
	}
	else
	{
		SendMessage(experimentSavedIndicator.hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	return false;
}

bool ConfigurationFileSystem::experimentSettingsReadyCheck(MasterWindow& Master)
{
	if (!experimentIsSaved)
	{
		if (this->checkExperimentSave("There are unsaved Experiment settings. Would you like to save the current experimnet before starting?", Master))
		{
			// canceled
			return true;
		}
	}
	return false;
}
bool ConfigurationFileSystem::checkExperimentSave(std::string prompt, MasterWindow& Master)
{
	int answer = MessageBox(0, prompt.c_str(), 0, MB_YESNOCANCEL);
	if (answer == IDYES)
	{
		this->saveExperimentOnly(Master);
	}
	else if (answer == IDCANCEL)
	{
		return true;
	}
	return false;
}

bool ConfigurationFileSystem::experimentChangeHandler(MasterWindow& Master)
{
	if (!experimentIsSaved)
	{
		if (this->checkExperimentSave("The current experiment is unsaved. Save Current Experiment before Changing?", Master))
		{
			return true;
		}
	}
	// get current item.
	long long itemIndex = SendMessage(experimentCombo.hwnd, CB_GETCURSEL, 0, 0);
	if (itemIndex == -1)
	{
		// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
		// is blank. Just break out, this is fine.
		return true;
	}
	TCHAR experimentConfigToOpen[256];
	// Send CB_GETLBTEXT message to get the item.
	SendMessage(experimentCombo.hwnd, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)experimentConfigToOpen);
	if (this->openExperiment(std::string(experimentConfigToOpen), Master))
	{
		return true;
	}
	this->reloadCombo(categoryCombo.hwnd, pathIncludingExperiment, "*", "__NONE__");
	// it'd be confusing if this category-specific text remained after the category get set to blank.
	Master.notes.setCategoryNotes("");
	Master.notes.setConfigurationNotes("");
	return false;
}

bool ConfigurationFileSystem::loadNullSequence(MasterWindow& Master)
{
	currentProfileSettings.sequence = NULL_SEQUENCE;
	// only current configuration loaded
	sequenceConfigurationNames.clear();
	if (currentProfileSettings.configuration != "")
	{
		if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
		{
			sequenceConfigurationNames.push_back(currentProfileSettings.configuration + HORIZONTAL_EXTENSION);
		}
		else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
		{
			sequenceConfigurationNames.push_back(currentProfileSettings.configuration + VERTICAL_EXTENSION);
		}
		else
		{
			MessageBox(0, "ERROR: orientation not recognized! Ask Mark about bugs.", 0, 0);
			return true;
		}
		// change edit
		SendMessage(sequenceInfoDisplay.hwnd, WM_SETTEXT, 256, (LPARAM)"Sequence of Configurations to Run:\r\n");
		appendText("1. " + sequenceConfigurationNames[0] + "\r\n", IDC_SEQUENCE_DISPLAY, Master.masterWindowHandle);
	}
	else
	{
		SendMessage(sequenceInfoDisplay.hwnd, WM_SETTEXT, 256, (LPARAM)"Sequence of Configurations to Run:\r\n");
		appendText("No Configuration Loaded\r\n", IDC_SEQUENCE_DISPLAY, Master.masterWindowHandle);
	}
	SendMessage(sequenceCombo.hwnd, CB_SELECTSTRING, 0, (LPARAM)NULL_SEQUENCE);
	this->updateSequenceSavedStatus(true);
	return false;
}

bool ConfigurationFileSystem::addToSequence(MasterWindow& Master)
{
	if (currentProfileSettings.configuration == "")
	{
		// nothing to add.
		return true;
	}
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		sequenceConfigurationNames.push_back(currentProfileSettings.configuration + HORIZONTAL_EXTENSION);
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		sequenceConfigurationNames.push_back(currentProfileSettings.configuration + VERTICAL_EXTENSION);
	}
	// add text to display.
	appendText(std::to_string(sequenceConfigurationNames.size()) + ". " + sequenceConfigurationNames.back() + "\r\n", IDC_SEQUENCE_DISPLAY, Master.masterWindowHandle);
	this->updateSequenceSavedStatus(false);
	return false;
}

/// SEQUENCE HANDLING
bool ConfigurationFileSystem::sequenceChangeHandler(MasterWindow& Master)
{
	// get the name
	long long itemIndex = SendMessage(sequenceCombo.hwnd, CB_GETCURSEL, 0, 0);
	TCHAR sequenceName[256];
	SendMessage(sequenceCombo.hwnd, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)sequenceName);
	if (itemIndex == -1)
	{
		// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
		// is blank. just break out, this is fine.
		return true;
	}
	if (std::string(sequenceName) == NULL_SEQUENCE)
	{
		this->loadNullSequence(Master);
		return false;
	}
	else
	{
		this->openSequence(sequenceName, Master);
	}
	// else not null_sequence.
	this->reloadSequence(currentProfileSettings.sequence, Master);
	this->updateSequenceSavedStatus(true);
	return false;
}
bool ConfigurationFileSystem::reloadSequence(std::string sequenceToReload, MasterWindow& Master)
{
	this->reloadCombo(sequenceCombo.hwnd, pathIncludingCategory, std::string("*") + SEQUENCE_EXTENSION, sequenceToReload);
	SendMessage(sequenceCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)NULL_SEQUENCE);
	if (sequenceToReload == NULL_SEQUENCE)
	{
		this->loadNullSequence(Master);
	}
	return false;
}
bool ConfigurationFileSystem::saveSequence(MasterWindow& Master)
{
	if (this->currentProfileSettings.category == "")
	{
		if (this->currentProfileSettings.experiment == "")
		{
			MessageBox(0, "Please set category and experiment before saving sequence.", 0, 0);
			return true;
		}
		else
		{
			MessageBox(0, "Please set category before saving sequence.", 0, 0);
			return true;
		}
	}
	if (currentProfileSettings.sequence == NULL_SEQUENCE)
	{
		// nothing to save;
		return true;
	}
	// if not saved...
	if (currentProfileSettings.sequence == "")
	{
		TCHAR* result = NULL;
		result = (TCHAR*)DialogBoxParam(Master.programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, textPromptDialogProcedure, (LPARAM)"Please enter a name for this sequence: ");
		if (std::string(result) == "")
		{
			return true;
		}
		currentProfileSettings.sequence = result;
	}
	std::fstream sequenceSaveFile(pathIncludingCategory + "\\" + currentProfileSettings.sequence + SEQUENCE_EXTENSION, std::fstream::out);
	if (!sequenceSaveFile.is_open())
	{
		MessageBox(0, "ERROR: Couldn't open sequence file for saving!", 0, 0);
		return -1;
	}
	sequenceSaveFile << "Version: 1.0\n";
	for (int sequenceInc = 0; sequenceInc < sequenceConfigurationNames.size(); sequenceInc++)
	{
		sequenceSaveFile << sequenceConfigurationNames[sequenceInc] + "\n";
	}
	sequenceSaveFile.close();
	this->reloadSequence(currentProfileSettings.sequence, Master);
	this->updateSequenceSavedStatus(true);
	return false;
}

bool ConfigurationFileSystem::saveSequenceAs(MasterWindow& Master)
{
	// prompt for name
	TCHAR* result = NULL;
	result = (TCHAR*)DialogBoxParam(Master.programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, textPromptDialogProcedure, (LPARAM)"Please Enter a new Sequence Name:");
	//
	if (result == NULL || std::string(result) == "")
	{
		// user canceled or entered nothing
		return true;
	}
	if (std::string(result) == NULL_SEQUENCE)
	{
		// nothing to save;
		return true;
	}
	// if not saved...
	std::fstream sequenceSaveFile(pathIncludingCategory + "\\" + std::string(result) + SEQUENCE_EXTENSION, std::fstream::out);
	if (!sequenceSaveFile.is_open())
	{
		MessageBox(0, "ERROR: Couldn't open sequence file for saving!", 0, 0);
		return -1;
	}
	currentProfileSettings.sequence = std::string(result);
	sequenceSaveFile << "Version: 1.0\n";
	for (int sequenceInc = 0; sequenceInc < sequenceConfigurationNames.size(); sequenceInc++)
	{
		sequenceSaveFile << sequenceConfigurationNames[sequenceInc] + "\n";
	}
	sequenceSaveFile.close();
	this->updateSequenceSavedStatus(true);
	return false;
}

bool ConfigurationFileSystem::renameSequence(MasterWindow& Master)
{
	// check if configuration has been set yet.
	if (currentProfileSettings.sequence == "" || currentProfileSettings.sequence == NULL_SEQUENCE)
	{
		MessageBox(0, "Please select a sequence for renaming.", 0, 0);
		return true;
	}
	std::string newSequenceName = (const char*)DialogBoxParam(Master.programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new configuration name.");
	if (newSequenceName == "")
	{
		// canceled
		return true;
	}
	int result = MoveFile((pathIncludingCategory + currentProfileSettings.sequence + SEQUENCE_EXTENSION).c_str(), 
						  (pathIncludingCategory + newSequenceName + SEQUENCE_EXTENSION).c_str());
	if (result == 0)
	{
		MessageBox(0, "Renaming of the sequence file Failed! Ask Mark about bugs", 0, 0);
		return true;
	}
	currentProfileSettings.sequence = newSequenceName;
	this->reloadSequence(currentProfileSettings.sequence, Master);
	this->updateSequenceSavedStatus(true);
	return false;
}

bool ConfigurationFileSystem::deleteSequence(MasterWindow& Master)
{
	// check if configuration has been set yet.
	if (currentProfileSettings.sequence == "" || currentProfileSettings.sequence == NULL_SEQUENCE)
	{
		MessageBox(0, "Please select a sequence for deleting.", 0, 0);
		return true;
	}
	int answer = MessageBox(0, ("Are you sure you want to delete the current sequence: " + currentProfileSettings.sequence).c_str(), 0, MB_YESNO);
	if (answer == IDNO)
	{
		return true;
	}
	std::string currentSequenceLocation = pathIncludingCategory + currentProfileSettings.sequence + SEQUENCE_EXTENSION;
	int result = DeleteFile(currentSequenceLocation.c_str());
	if (result == 0)
	{
		MessageBox(0, "ERROR: Deleteing the configuration file failed!", 0, 0);
		return true;
	}
	// since the sequence this (may have been) was saved to is gone, no saved version of current code.
	this->updateSequenceSavedStatus(false);
	// just deleted the current configuration
	currentProfileSettings.sequence = "";
	// reset combo since the files have now changed after delete
	this->reloadSequence("__NONE__", Master);
	return false;
}
bool ConfigurationFileSystem::newSequence(MasterWindow& Master)
{
	// prompt for name
	TCHAR* result = NULL;
	result = (TCHAR*)DialogBoxParam(Master.programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, textPromptDialogProcedure, (LPARAM)"Please Enter a new Sequence Name:");
	//
	if (result == NULL || std::string(result) == "")
	{
		// user canceled or entered nothing
		return true;
	}
	// try to open the file.
	std::fstream sequenceFile(pathIncludingCategory + "\\" + result + SEQUENCE_EXTENSION, std::fstream::out);
	if (!sequenceFile.is_open())
	{
		MessageBox(0, "Couldn't create a file with this sequence name! Make sure there are no forbidden characters in your name.", 0, 0);
		return -1;
	}
	std::string newSequenceName = std::string(result);
	sequenceFile << newSequenceName + "\n";
	// output current configuration
	//eSequenceFileNames.clear();
	if (newSequenceName == "")
	{
		return true;
	}
	// reload combo.
	this->reloadSequence(currentProfileSettings.sequence, Master);
	return false;
}

bool ConfigurationFileSystem::openSequence(std::string sequenceName, MasterWindow& Master)
{
	// try to open the file
	std::fstream sequenceFile(pathIncludingCategory + sequenceName + SEQUENCE_EXTENSION);
	if (!sequenceFile.is_open())
	{
		MessageBox(0, ("ERROR: sequence file failed to open! Make sure the sequence with address ..." + pathIncludingCategory + sequenceName + SEQUENCE_EXTENSION + " exists.").c_str(), 0, 0);
		return true;
	}
	currentProfileSettings.sequence = std::string(sequenceName);
	// read the file
	std::string version;
	std::getline(sequenceFile, version);
	//
	sequenceConfigurationNames.clear();
	std::string tempName;
	getline(sequenceFile, tempName);
	while (sequenceFile)
	{
		sequenceConfigurationNames.push_back(tempName);
		getline(sequenceFile, tempName);
	}
	// update the edit
	SendMessage(sequenceInfoDisplay.hwnd, WM_SETTEXT, 256, (LPARAM)"Configuration Sequence:\r\n");
	for (int sequenceInc = 0; sequenceInc < sequenceConfigurationNames.size(); sequenceInc++)
	{
		appendText(std::to_string(sequenceInc + 1) + ". " + sequenceConfigurationNames[sequenceInc] + "\r\n", IDC_SEQUENCE_DISPLAY, Master.masterWindowHandle);
	}
	this->updateSequenceSavedStatus(true);
	return false;
}

bool ConfigurationFileSystem::updateSequenceSavedStatus(bool isSaved)
{
	sequenceIsSaved = isSaved;
	if (isSaved)
	{
		SendMessage(sequenceSavedIndicator.hwnd, BM_SETCHECK, BST_CHECKED, 0);
	}
	else
	{
		SendMessage(sequenceSavedIndicator.hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	return false;
}

bool ConfigurationFileSystem::sequenceSettingsReadyCheck(MasterWindow& Master)
{
	if (!sequenceIsSaved)
	{
		if (this->checkSequenceSave("There are unsaved sequence settings. Would you like to save the current sequence before starting?", Master))
		{
			// canceled
			return true;
		}
	}
	return false;
}

bool ConfigurationFileSystem::checkSequenceSave(std::string prompt, MasterWindow& Master)
{
	int answer = MessageBox(0, prompt.c_str(), 0, MB_YESNOCANCEL);
	if (answer == IDYES)
	{
		this->saveSequence(Master);
	}
	else if (answer == IDCANCEL)
	{
		return true;
	}
	return false;
}
std::vector<std::string> ConfigurationFileSystem::getSequenceNames()
{
	return sequenceConfigurationNames;
}

std::string ConfigurationFileSystem::getSequenceNamesString()
{
	std::string namesString = "";
	if (currentProfileSettings.sequence != "NO SEQUENCE")
	{
		namesString += "Sequence:\r\n";
		for (int sequenceInc = 0; sequenceInc < sequenceConfigurationNames.size(); sequenceInc++)
		{
			namesString += "\t" + std::to_string(sequenceInc) + ": " + sequenceConfigurationNames[sequenceInc] + "\r\n";
		}
	}
	return namesString;
}

bool ConfigurationFileSystem::initialize(POINT& topLeftPosition, MasterWindow& Master)
{
	RECT currentRect;
	// Experiment Type
	currentRect = experimentLabel.normalPos = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 20 };
	experimentLabel.hwnd = CreateWindowEx(NULL, "STATIC", "EXPERIMENT", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top, 
		Master.masterWindowHandle, (HMENU)-1, Master.programInstance, NULL);
	SendMessage(experimentLabel.hwnd, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// Experiment Saved Indicator
	currentRect = experimentSavedIndicator.normalPos = { topLeftPosition.x + 360, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 20 };
	experimentSavedIndicator.hwnd = CreateWindowEx(NULL, "BUTTON", "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top, 
		Master.masterWindowHandle, (HMENU)IDC_SAVE_EXPERIMENT_INDICATOR_BUTTON, Master.programInstance, NULL);
	SendMessage(experimentSavedIndicator.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(experimentSavedIndicator.hwnd, BM_SETCHECK, BST_CHECKED, NULL);
	updateExperimentSavedStatus(true);
	//eExperimentSaved = true;
	// Category Title
	currentRect = categoryLabel.normalPos = { topLeftPosition.x + 480, topLeftPosition.y, topLeftPosition.x + 960, topLeftPosition.y + 20 };
	categoryLabel.hwnd = CreateWindowEx(NULL, "STATIC", "CATEGORY", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top, 
		Master.masterWindowHandle, (HMENU)-1, Master.programInstance, NULL);
	SendMessage(categoryLabel.hwnd, WM_SETFONT, WPARAM(sHeadingFont), TRUE);

	currentRect = categorySavedIndicator.normalPos = { topLeftPosition.x + 480 + 380, topLeftPosition.y, topLeftPosition.x + 960, topLeftPosition.y + 20};
	categorySavedIndicator.hwnd = CreateWindowEx(NULL, "BUTTON", "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top, 
		Master.masterWindowHandle, (HMENU)IDC_SAVE_CATEGORY_INDICATOR_BUTTON, Master.programInstance, NULL);
	SendMessage(categorySavedIndicator.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(categorySavedIndicator.hwnd, BM_SETCHECK, BST_CHECKED, NULL);
	updateCategorySavedStatus(true);
	topLeftPosition.y += 20;
	// Experiment Combo
	currentRect = experimentCombo.normalPos = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 800 };
	experimentCombo.hwnd = CreateWindowEx(NULL, TEXT("ComboBox"), "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top,
		Master.masterWindowHandle, (HMENU)IDC_EXPERIMENT_COMBO, Master.programInstance, NULL);
	SendMessage(experimentCombo.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	this->reloadCombo(experimentCombo.hwnd, PROFILES_PATH, std::string("*"), "__NONE__");
	// Category Combo
	currentRect = categoryCombo.normalPos = { topLeftPosition.x + 480, topLeftPosition.y, topLeftPosition.x + 960, topLeftPosition.y + 800 };
	categoryCombo.hwnd = CreateWindowEx(NULL, TEXT("ComboBox"), "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top, 
		Master.masterWindowHandle, (HMENU)IDC_CATEGORY_COMBO, Master.programInstance, NULL);
	SendMessage(categoryCombo.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	topLeftPosition.y += 25;
	// Orientation Title
	currentRect = orientationLabel.normalPos = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 120, topLeftPosition.y + 20 };
	orientationLabel.hwnd = CreateWindowEx(NULL, "STATIC", "ORIENTATION", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top, 
		Master.masterWindowHandle, (HMENU)-1, Master.programInstance, NULL);
	SendMessage(orientationLabel.hwnd, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// Configuration Title
	currentRect = configLabel.normalPos = { topLeftPosition.x + 120, topLeftPosition.y, topLeftPosition.x + 960, topLeftPosition.y + 20 };
	configLabel.hwnd = CreateWindowEx(NULL, "STATIC", "CONFIGURATION", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top, 
		Master.masterWindowHandle, (HMENU)-1, Master.programInstance, NULL);
	SendMessage(configLabel.hwnd, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// Configuration Saved Indicator
	currentRect = configurationSavedIndicator.normalPos = { topLeftPosition.x + 860, topLeftPosition.y, topLeftPosition.x + 960, topLeftPosition.y + 20 };
	configurationSavedIndicator.hwnd = CreateWindowEx(NULL, "BUTTON", "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top,
		Master.masterWindowHandle, (HMENU)IDC_SAVE_CONFIGURATION_INDICATOR_BUTTON, Master.programInstance, NULL);
	SendMessage(configurationSavedIndicator.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(configurationSavedIndicator.hwnd, BM_SETCHECK, BST_CHECKED, NULL);
	updateConfigurationSavedStatus(true);
	topLeftPosition.y += 20;
	//eConfigurationSaved = true;
	// orientation combo
	std::vector<std::string> orientationNames;
	orientationNames.push_back("Horizontal");
	orientationNames.push_back("Vertical");
	currentRect = orientationCombo.normalPos = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 120, topLeftPosition.y + 800 };
	orientationCombo.hwnd = CreateWindowEx(NULL, TEXT("ComboBox"), "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top, 
		Master.masterWindowHandle, (HMENU)IDC_ORIENTATION_COMBO, Master.programInstance, NULL);
	SendMessage(orientationCombo.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	for (int comboInc = 0; comboInc < orientationNames.size(); comboInc++)
	{
		SendMessage(orientationCombo.hwnd, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(_T(orientationNames[comboInc].c_str())));
	}
	SendMessage(orientationCombo.hwnd, CB_SETCURSEL, 0, 0);
	// configuration combo
	currentRect = configCombo.normalPos = { topLeftPosition.x + 120, topLeftPosition.y, topLeftPosition.x + 960, topLeftPosition.y + 800 };
	configCombo.hwnd = CreateWindowEx(NULL, TEXT("ComboBox"), "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top, 
		Master.masterWindowHandle,
		(HMENU)IDC_CONFIGURATION_COMBO, Master.programInstance, NULL);
	SendMessage(configCombo.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	topLeftPosition.y += 25;
	/// SEQUENCE
	sequenceLabel.normalPos = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 20 };
	currentRect = sequenceLabel.normalPos;
	sequenceLabel.hwnd = CreateWindowEx(NULL, "STATIC", "SEQUENCE", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top,
		Master.masterWindowHandle, (HMENU)IDC_SEQUENCE_TEXT, Master.programInstance, NULL);
	SendMessage(sequenceLabel.hwnd, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	topLeftPosition.y += 20;
	// combo
	sequenceCombo.normalPos = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 800 };
	currentRect = sequenceCombo.normalPos;
	sequenceCombo.hwnd = CreateWindowEx(NULL, TEXT("ComboBox"), "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top,
		Master.masterWindowHandle, (HMENU)IDC_SEQUENCE_COMBO, Master.programInstance, NULL);
	SendMessage(sequenceCombo.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(sequenceCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"NULL SEQUENCE");
	SendMessage(sequenceCombo.hwnd, CB_SETCURSEL, 0, 0);
	topLeftPosition.y += 25;
	// display
	sequenceInfoDisplay.normalPos = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 100 };
	currentRect = sequenceInfoDisplay.normalPos;
	sequenceInfoDisplay.hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "Sequence of Configurations to Run:\r\n", ES_READONLY | WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top,
		Master.masterWindowHandle, (HMENU)IDC_SEQUENCE_DISPLAY, Master.programInstance, NULL);
	topLeftPosition.y += 100;
	return 0;
}

bool ConfigurationFileSystem::reorganizeControls(RECT parentRectangle, std::string mode)
{

	return 0;
}

std::vector<std::string> ConfigurationFileSystem::searchForFiles(std::string locationToSearch, std::string extensions)
{
	// Re-add the entries back in and figure out which one is the current one.
	std::vector<std::string> names;
	std::string search_path = locationToSearch + "\\" + extensions;
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	if (extensions == "*")
	{
		hFind = FindFirstFileEx(search_path.c_str(), FindExInfoStandard, &fd, FindExSearchLimitToDirectories, NULL, 0);
	}
	else
	{
		hFind = FindFirstFile(search_path.c_str(), &fd);
	}
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			// if looking for folders
			if (extensions == "*")
			{
				if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
				{
					if (std::string(fd.cFileName) != "." && std::string(fd.cFileName) != "..")
					{
						names.push_back(fd.cFileName);
					}
				}
			}
			else
			{
				if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					names.push_back(fd.cFileName);
				}
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}

	// Remove suffix from file names and...
	for (int configListInc = 0; configListInc < names.size(); configListInc++)
	{
		if (extensions == "*" || extensions == "*.*" || extensions == std::string("*") + HORIZONTAL_EXTENSION 
			|| extensions == std::string("*") + VERTICAL_EXTENSION || extensions == std::string("*") + SEQUENCE_EXTENSION
			|| extensions == std::string("*") + CATEGORY_EXTENSION || extensions == std::string("*") + EXPERIMENT_EXTENSION)
		{
			names[configListInc] = names[configListInc].substr(0, names[configListInc].size() - (extensions.size() - 1));
		}
		else
		{
			names[configListInc] = names[configListInc].substr(0, names[configListInc].size() - extensions.size());
		}
	}
	// Make the final vector out of the unique objects left.
	return names;
}

bool ConfigurationFileSystem::reloadCombo(HWND comboToReload, std::string locationToLook, std::string extension, std::string nameToLoad)
{
	std::vector<std::string> names;
	// search for folders
	names = this->searchForFiles(locationToLook, extension);

	/// Get current selection
	long long itemIndex = SendMessage(comboToReload, CB_GETCURSEL, 0, 0);
	TCHAR experimentConfigToOpen[256];
	std::string currentSelection;
	int currentInc = -1;
	if (itemIndex != -1)
	{
		// Send CB_GETLBTEXT message to get the item.
		SendMessage(comboToReload, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)experimentConfigToOpen);
		currentSelection = experimentConfigToOpen;
	}
	/// Reset stuffs
	SendMessage(comboToReload, CB_RESETCONTENT, 0, 0);
	// Send list to object
	for (int comboInc = 0; comboInc < names.size(); comboInc++)
	{
		if (nameToLoad == names[comboInc])
		{
			currentInc = comboInc;
		}
		TCHAR * name = (TCHAR*)names[comboInc].c_str();
		SendMessage(comboToReload, CB_ADDSTRING, 0, (LPARAM)(name));
	}
	// Set initial value
	SendMessage(comboToReload, CB_SETCURSEL, currentInc, 0);
	return 0;
}

bool ConfigurationFileSystem::fileOrFolderExists(std::string filePathway)
{
	// got this from stack exchange. dunno how it works but it should be fast.
	struct stat buffer;
	return (stat(filePathway.c_str(), &buffer) == 0);
}

std::string ConfigurationFileSystem::getComboText()
{
	int selectionNum = SendMessage(configCombo.hwnd, CB_GETCURSEL, 0, 0);
	if (selectionNum == -1)
	{
		return "";
	}
	else
	{
		TCHAR text[256];
		SendMessage(configCombo.hwnd, CB_GETLBTEXT, selectionNum, (LPARAM)text);
		return text;
	}
}

bool ConfigurationFileSystem::fullyDeleteFolder(std::string folderToDelete)
{
	// this used to call SHFileOperation. Boost is better. Much better. 
	int filesRemoved = boost::filesystem::remove_all(folderToDelete.c_str());
	if (filesRemoved == 0)
	{
		std::string errMsg = "Delete Failed! Ask mark about bugs.";
		MessageBox(0, errMsg.c_str(), 0, 0);
		return true;
	}
	return false;
}

