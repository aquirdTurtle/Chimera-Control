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

ConfigurationFileSystem::ConfigurationFileSystem(std::string fileSystemPath, int& id)
{
	FILE_SYSTEM_PATH = fileSystemPath;
	this->currentProfileSettings.orientation = HORIZONTAL_ORIENTATION;
	this->experimentLabel.ID = id;
	this->experimentSavedIndicator.ID = id + 1;
	this->categoryLabel.ID = id + 2;
	this->categorySavedIndicator.ID = id + 3;
	this->experimentCombo.ID = id + 4;
	if (experimentCombo.ID != EXPERIMENT_COMBO_ID)
	{
		MessageBox(0, "ERROR: EXPERIMENT_COMBO_ID didn't match actual id!", 0, 0);
	}
	this->categoryCombo.ID = id + 5;
	if (categoryCombo.ID != CATEGORY_COMBO_ID)
	{
		MessageBox(0, "ERROR: CATEGORY_COMBO_ID didn't match actual id!", 0, 0);
	}
	this->orientationLabel.ID = id + 6;
	this->configLabel.ID = id + 7;
	this->configurationSavedIndicator.ID = id + 8;
	this->orientationCombo.ID = id + 9;
	if (orientationCombo.ID != ORIENTATION_COMBO_ID)
	{
		MessageBox(0, "ERROR: ORIENTATION_COMBO_ID didn't match actual id!", 0, 0);
	}
	this->configCombo.ID = id + 10;
	if (configCombo.ID != CONFIGURATION_COMBO_ID)
	{
		MessageBox(0, "ERROR: CONFIGURATION_COMBO_ID doesn't match actual combo id!", 0, 0);
	}
	this->sequenceLabel.ID = id + 11;
	this->sequenceCombo.ID = id + 12;
	if (sequenceCombo.ID != SEQUENCE_COMBO_ID)
	{
		MessageBox(0, "ERROR: SEQUENCE_COMBO_ID doesn't match actual id!", 0, 0);
	}
	this->sequenceInfoDisplay.ID = id + 13;
	id += 14;
}

ConfigurationFileSystem::~ConfigurationFileSystem()
{
	// nothing for destructor right nowFget
}

std::string ConfigurationFileSystem::getMasterAddressFromConfig()
{
	std::string configurationAddress;
	if (this->currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		configurationAddress = this->pathIncludingCategory + this->currentProfileSettings.configuration + HORIZONTAL_EXTENSION;
	}
	else if (this->currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		configurationAddress = this->pathIncludingCategory + this->currentProfileSettings.configuration + VERTICAL_EXTENSION;
	}
	else
	{
		errBox("ERROR: Unrecognized orientation: " + this->currentProfileSettings.orientation);
		return "";
	}
	std::fstream configFile(configurationAddress);
	if (!configFile.is_open())
	{
		errBox("ERROR: Failed to open configuration file.");
		return "";
	}
	// get the first couple lines...
	std::string line, word, address;
	std::getline(configFile, line);
	std::getline(configFile, line);
	configFile >> word;
	if (word == "LOCAL")
	{
		configFile.get();
		getline(configFile, address);
		return this->getCurrentPathIncludingCategory() + address + MASTER_SCRIPT_EXTENSION;
	}
	else if (word == "NONLOCAL")
	{
		std::string newPath;
		configFile.get();
		getline(configFile, newPath);
		//configurationFile >> newPath;
		return newPath;
	}
	else
	{
		errBox("ERROR: Expected either \"LOCAL\" or \"NONLOCAL\" in configuration file, but instead found " + word);
		return false;
	}

}

bool ConfigurationFileSystem::saveEntireProfile(MasterWindow* Master)
{
	// save experiment
	if (!this->saveExperimentOnly(Master))
	{
		return false;
	}
	// save category
	if (!this->saveCategoryOnly(Master))
	{
		return false;
	}
	// save configuration
	if (!this->saveConfigurationOnly(Master))
	{
		return false;
	}
	// save sequence
	if (!this->saveSequence(Master))
	{
		return false;
	}
	return true;
}
bool ConfigurationFileSystem::checkSaveEntireProfile(MasterWindow* Master)
{
	if (!this->checkExperimentSave("Save Experiment Settings?", Master))
	{
		return false;
	}
	if (!this->checkCategorySave("Save Category Settings?", Master))
	{
		return false;
	}
	if (!this->checkConfigurationSave("Save Configuration Settings?", Master))
	{
		return false;
	}
	if (!this->checkSequenceSave("Save Sequence Settings?", Master))
	{
		return false;
	}
	return true;
}

bool ConfigurationFileSystem::allSettingsReadyCheck(MasterWindow* Master)
{
	if (this->currentProfileSettings.configuration == "")
	{
		errBox("ERROR: Please set a configuration.");
		return false;
	}
	// check all components of this class.
	if (!this->experimentSettingsReadyCheck(Master))
	{
		return false;
	}
	else if (!this->categorySettingsReadyCheck(Master))
	{
		return false;
	}
	else if (!this->configurationSettingsReadyCheck(Master))
	{
		return false;
	}
	else if (!this->sequenceSettingsReadyCheck(Master))
	{
		return false;
	}
	// passed all checks.
	return true;
}

bool ConfigurationFileSystem::reloadAllCombos()
{
	return true;
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
		return false;
	}
	currentProfileSettings.orientation = orientation;
	return true;
}

bool ConfigurationFileSystem::orientationChangeHandler(MasterWindow* Master)
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
	Master->notes.setConfigurationNotes("");
	currentProfileSettings.configuration = "";
	/// Load the relevant NIAWG script.
	myNIAWG::loadDefault();

	return true;
}

/// CONFIGURATION LEVEL HANDLING

bool ConfigurationFileSystem::newConfiguration(MasterWindow* Master)
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

	std::string configurationNameToSave = (const char*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new configuration name.");
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
		return false;
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
	return true;
}

/*
]--- This function opens a given configuration file, sets all of the relevant parameters, and loads the associated scripts. 
*/
bool ConfigurationFileSystem::openConfiguration(std::string configurationNameToOpen, MasterWindow* Master)
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
		return false;
	}
	currentProfileSettings.configuration = configurationNameToOpen;
	
	// not currenlty used because version 1.0.
	std::string version;
	std::getline(configurationFile, version);
	/// Get Variables
	// early version didn't have variable type indicators.
	if (version != "Master Version: 1.0")
	{
		if (version == "")
		{
			// nothing
			return false;
		}
		else
		{
			MessageBox(0, "ERROR: Unrecognized configuration version! Ask Mark about bugs.", 0, 0);
			return false;
		}
	}
	std::string masterText;
	std::getline(configurationFile, masterText);

	if (masterText != "MASTER SCRIPT:")
	{
		errBox("ERRORL Expected \"MASTER SCRIPT:\" in configuration file, but instead found " + masterText);
		return false;
	}
	configurationFile >> masterText;
	if (masterText == "LOCAL")
	{
		configurationFile >> masterText;
		std::string newPath = this->getCurrentPathIncludingCategory() + masterText + MASTER_SCRIPT_EXTENSION;
		Master->masterScript.openParentScript(newPath, Master);
	}
	else if (masterText == "NONLOCAL")
	{
		std::string newPath;
		configurationFile.get();
		getline(configurationFile, newPath);
		//configurationFile >> newPath;
		Master->masterScript.openParentScript(newPath, Master);
	}
	else
	{
		errBox("ERROR: Expected either \"LOCAL\" or \"NONLOCAL\" in configuration file, but instead found " + masterText);
		return false;
	}
	std::string line;
	configurationFile >> line;
	if (line != "VARIABLES:")
	{
		errBox("ERROR: Expected \"VARIABLES\" in configuration file but instead there was " + line);
		return false;
	}
	int varNum;
	configurationFile >> varNum;
	if (varNum < 0 || varNum > 10)
	{
		int answer = MessageBox(0, ("ERROR: variable number retrieved from file appears suspicious. The number is " + std::to_string(varNum) + ". Is this accurate?").c_str(), 0, MB_YESNO);
		if (answer == IDNO)
		{
			// don't try to load anything.
			varNum = 0;
			return false;
		}
	}
	// Number of Variables
	Master->variables.clearVariables();

	for (int varInc = 0; varInc < varNum; varInc++)
	{
		variable tempVar;
		std::string varName, timelikeText, typeText, valueString;
		bool timelike;
		bool singleton;
		double value;
		configurationFile >> tempVar.name;
		configurationFile >> timelikeText;
		if (timelikeText == "Timelike")
		{
			tempVar.timelike = true;
		}
		else if (timelikeText == "Not_Timelike")
		{
			tempVar.timelike = false;
		}
		else
		{
			errBox("ERROR: unknown timelike option. Check the formatting of the configuration file. Default to not timelike.");
			tempVar.timelike = false;
		}

		configurationFile >> typeText;
		if (typeText == "Singleton")
		{
			tempVar.singleton = true;
		}
		else if (typeText == "From_Master")
		{
			tempVar.singleton = false;
		}
		else
		{
			errBox("ERROR: unknown variable type option. Check the formatting of the configuration file. Default to singleton.");
			tempVar.singleton = true;
		}
		int rangeNumber;
		configurationFile >> rangeNumber;
		if (rangeNumber < 1)
		{
			errBox("ERROR: Bad range number! setting it to 1, but found " + std::to_string(rangeNumber) + " in the file.");
			rangeNumber = 1;
		}
		Master->variables.setVariationRangeNumber(rangeNumber);
		// check if the range is actually too small.
		for (int rangeInc = 0; rangeInc < rangeNumber; rangeInc++)
		{
			double initValue, finValue;
			unsigned int variations;
			configurationFile >> initValue;
			configurationFile >> finValue;
			configurationFile >> variations;
			tempVar.ranges.push_back({ initValue, finValue, variations });
		}
		// shouldn't be because of 1 forcing earlier.
		if (tempVar.ranges.size() == 0)
		{
			// make sure it has at least one entry.
			tempVar.ranges.push_back({ 0,0,1 });
		}
		Master->variables.addVariable(tempVar, varInc);
	}
	// add the last line.
	variable empty;
	empty.name = "";
	//Master->variables.addVariable(empty, -1);
	//
	std::string ttlText;
	configurationFile >> ttlText;
	if (ttlText != "TTLS:")
	{
		errBox("ERROR: expected \"TTLS:\" in configuration file but instead found " + ttlText);
		return false;
	}
	for (int ttlRowInc = 0; ttlRowInc < Master->ttlBoard.getNumberOfTTLRows(); ttlRowInc++)
	{
		for (int ttlNumberInc = 0; ttlNumberInc < Master->ttlBoard.getNumberOfTTLsPerRow(); ttlNumberInc++)
		{
			std::string ttlString;
			configurationFile >> ttlString;
			try
			{
				bool state = std::stoi(ttlString);
				Master->ttlBoard.forceTTL(ttlRowInc, ttlNumberInc, state);
			}
			catch (std::invalid_argument& exception)
			{
				errBox("ERROR: the ttl status of \"" + ttlString + "\"failed to convert to a bool!");
				return false;
			}
		}
	}
	std::string dacText;
	configurationFile >> dacText;
	if (dacText != "DACS:")
	{
		errBox("ERROR: Expected \"DACS:\" in configuration file but instead found " + dacText);
		return false;
	}
	for (int dacInc = 0; dacInc < Master->dacBoards.getNumberOfDACs(); dacInc++)
	{
		std::string dacString;
		configurationFile >> dacString;
		try
		{
			double dacValue = std::stod(dacString);
			Master->dacBoards.forceDacChange(dacInc, dacValue, &Master->ttlBoard);
		}
		catch (std::invalid_argument& exception)
		{
			errBox("ERROR: failed to convert dac value to voltage. string was " + dacString);
			return false;
		}
	}
	std::string repText;
	configurationFile >> repText;
	if (repText != "REPETITIONS:")
	{
		errBox("ERROR: Expected \"REPETITIONS:\" in configuration file, but instead found " + repText);
		return false;
	}
	configurationFile >> repText;
	try
	{
		int repNum = std::stoi(repText);
		Master->repetitionControl.setRepetitions(repNum);
	}
	catch (std::invalid_argument& exception)
	{
		errBox("ERROR: failed to convert repetition text to integer. Text was " + repText);
		return false;
	}

	// add a blank line
	variable tempVar;
	tempVar.name = "";
	tempVar.singleton = true;
	tempVar.timelike = false;
	tempVar.ranges.push_back({ 0,0,0 });
	Master->variables.addVariable(tempVar, varNum);
	this->updateConfigurationSavedStatus(true);
	currentProfileSettings.configuration = configurationNameToOpen;
	std::string notes;
	std::string tempNote;
	std::string notesNote;
	configurationFile >> notesNote;
	if (notesNote != "NOTES:")
	{
		errBox("ERROR: Expected \"NOTES:\" in configuration file but instead found " + notesNote);
		return false;
	}
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
		Master->notes.setConfigurationNotes(notes);
	}
	else
	{
		Master->notes.setConfigurationNotes("");
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
	return true;
}

/*
]--- This function attempts to save the configuration given the configuration name in the argument. It throws errors and warnings if this is not a "normal" 
]- Save, i.e. if the file doesn't already exist or if the user tries to pass an empty name as an argument. It returns false if the configuration got saved,
]- true if something prevented the configuration from being saved.
*/
bool ConfigurationFileSystem::saveConfigurationOnly(MasterWindow* Master)
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
		return false;
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
		return false;
	}

	if (!ConfigurationFileSystem::fileOrFolderExists(pathIncludingCategory + configurationNameToSave + extension))
	{
		int answer = MessageBox(0, ("This configuration file appears to not exist in the expected location: " + pathIncludingCategory + configurationNameToSave 
			+ extension + ". Continue by making a new configuration file?").c_str(), 0, MB_OKCANCEL);
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
			return true;
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
			return true;
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
		return false;
	}
	std::ofstream configurationSaveFile(pathIncludingCategory + configurationNameToSave + extension);
	if (!configurationSaveFile.is_open())
	{
		MessageBox(0, "Couldn't save configuration file! Check the name for weird characters, or call Mark about bugs if everything seems right...", 0, 0);
		return false;
	}
	// That's the last prompt the user gets, so the save is final now.
	currentProfileSettings.configuration = configurationNameToSave;
	configurationSaveFile << "Master Version: 1.0\n";
	/// master script
	configurationSaveFile << "MASTER SCRIPT:\n";
	// keep track of whether the script is saved locally or not. This should make renaming things easier. 
	if (Master->masterScript.getScriptPath() == this->getCurrentPathIncludingCategory())
	{
		configurationSaveFile << "LOCAL " << Master->masterScript.getScriptName() << "\n";
	}
	else
	{
		configurationSaveFile << "NONLOCAL " << Master->masterScript.getScriptPathAndName() << "\n";
	}
	configurationSaveFile << "VARIABLES:\n";
	// Number of Variables
	configurationSaveFile << Master->variables.getCurrentNumberOfVariables() << "\n";
	/// Variable Names
	for (int varInc = 0; varInc < Master->variables.getCurrentNumberOfVariables(); varInc++)
	{
		variable info = Master->variables.getVariableInfo(varInc);
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
		configurationSaveFile << info.ranges.size() << " ";
		for (int rangeInc = 0; rangeInc < info.ranges.size(); rangeInc++)
		{
			configurationSaveFile << info.ranges[rangeInc].initialValue << " ";
			configurationSaveFile << info.ranges[rangeInc].finalValue << " ";
			configurationSaveFile << info.ranges[rangeInc].variations << " ";
		}
		configurationSaveFile << "\n";
	}
	/// ttl settings
	configurationSaveFile << "TTLS:\n";
	for (int ttlRowInc = 0; ttlRowInc < Master->ttlBoard.getNumberOfTTLRows(); ttlRowInc++)
	{
		for (int ttlNumberInc = 0; ttlNumberInc < Master->ttlBoard.getNumberOfTTLsPerRow(); ttlNumberInc++)
		{
			configurationSaveFile << Master->ttlBoard.getTTL_Status(ttlRowInc, ttlNumberInc) << "\n";
		}
	}
	/// dac settings
	configurationSaveFile << "DACS:\n";
	for (int dacInc = 0; dacInc < Master->dacBoards.getNumberOfDACs(); dacInc++)
	{
		configurationSaveFile << Master->dacBoards.getDAC_Value(dacInc) << "\n";
	}
	/// repetitions
	configurationSaveFile << "REPETITIONS:\n";
	configurationSaveFile << Master->repetitionControl.getRepetitionNumber() << "\n";

	configurationSaveFile << "NOTES:\n";
	std::string notes = Master->notes.getConfigurationNotes();
	configurationSaveFile << notes + "\n";
	configurationSaveFile << "END CONFIGURATION NOTES" << "\n";
	configurationSaveFile.close();
	this->updateConfigurationSavedStatus(true);
	return true;
}

/*
]--- Identical to saveConfigurationOnly except that it prompts the user for a name with a dialog box instead of taking one.
*/
bool ConfigurationFileSystem::saveConfigurationAs(MasterWindow* Master)
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
	std::string configurationNameToSave = (const char*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new configuration name.");
	if (configurationNameToSave == "")
	{
		// canceled
		return false;
	}

	// check to make sure that this is a name.
	if (configurationNameToSave == "")
	{
		MessageBox(0, "ERROR: The program requested the saving of the configuration file to an empty name! This shouldn't happen, ask Mark about bugs.", 0, 0);
		return false;
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
		return false;
	}

	// check if file already exists
	if (ConfigurationFileSystem::fileOrFolderExists(pathIncludingCategory + configurationNameToSave + extension))
	{
		int answer = MessageBox(0, "This configuration file name already exists! Overwrite it?", 0, MB_OKCANCEL);
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
		return false;
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
	configurationSaveFile << "Master Version: 1.0\n";
	/// master script
	configurationSaveFile << "MASTER SCRIPT:\n";
	// keep track of whether the script is saved locally or not. This should make renaming things easier. 
	if (Master->masterScript.getScriptPath() == this->getCurrentPathIncludingCategory())
	{
		configurationSaveFile << "LOCAL " << Master->masterScript.getScriptName() << "\n";
	}
	else
	{
		configurationSaveFile << "NONLOCAL " << Master->masterScript.getScriptPathAndName() << "\n";
	}
	configurationSaveFile << "VARIABLES:\n";
	// Number of Variables
	configurationSaveFile << Master->variables.getCurrentNumberOfVariables() << "\n";
	/// Variable Names
	for (int varInc = 0; varInc < Master->variables.getCurrentNumberOfVariables(); varInc++)
	{
		variable info = Master->variables.getVariableInfo(varInc);
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
		configurationSaveFile << info.ranges.size() + " ";
		for (int rangeInc = 0; rangeInc < info.ranges.size(); rangeInc++)
		{
			configurationSaveFile << info.ranges[rangeInc].initialValue << " ";
			configurationSaveFile << info.ranges[rangeInc].finalValue << " ";
			configurationSaveFile << info.ranges[rangeInc].variations << " ";
		}
		configurationSaveFile << "\n";

	}
	/// ttl settings
	configurationSaveFile << "TTLS:\n";
	for (int ttlRowInc = 0; ttlRowInc < Master->ttlBoard.getNumberOfTTLRows(); ttlRowInc++)
	{
		for (int ttlNumberInc = 0; ttlNumberInc < Master->ttlBoard.getNumberOfTTLsPerRow(); ttlNumberInc++)
		{
			configurationSaveFile << Master->ttlBoard.getTTL_Status(ttlRowInc, ttlNumberInc) << "\n";
		}
	}
	/// dac settings
	configurationSaveFile << "DACS:\n";
	for (int dacInc = 0; dacInc < Master->dacBoards.getNumberOfDACs(); dacInc++)
	{
		configurationSaveFile << Master->dacBoards.getDAC_Value(dacInc) << "\n";
	}
	/// repetitions
	configurationSaveFile << "REPETITIONS:\n";
	configurationSaveFile << Master->repetitionControl.getRepetitionNumber() << "\n";

	configurationSaveFile << "NOTES:\n";
	std::string notes = Master->notes.getConfigurationNotes();
	configurationSaveFile << notes + "\n";
	configurationSaveFile << "END CONFIGURATION NOTES" << "\n";

	configurationSaveFile.close();
	this->reloadCombo(configCombo.hwnd, pathIncludingCategory, "*" + extension, currentProfileSettings.configuration);
	this->updateConfigurationSavedStatus(true);

	return true;
}

/*
]--- This function renames the currently set 
*/
bool ConfigurationFileSystem::renameConfiguration(MasterWindow* Master)
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
	std::string newConfigurationName = (const char*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new configuration name.");
	if (newConfigurationName == "")
	{
		// canceled
		return false;
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
		return false;
	}
	int result = MoveFile(currentConfigurationLocation.c_str(), newConfigurationLocation.c_str());
	if (result == 0)
	{
		MessageBox(0, "Renaming of the configuration file Failed! Ask Mark about bugs", 0, 0);
		return false;
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
		return false;
	}
	return true;
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
		return false;
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
		return false;
	}
	int result = DeleteFile(currentConfigurationLocation.c_str());
	if (result == 0)
	{
		MessageBox(0, "ERROR: Deleteing the configuration file failed!", 0, 0);
		return false;
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
		return false;
	}
	return true;
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
	return true;
}

bool ConfigurationFileSystem::configurationSettingsReadyCheck(MasterWindow* Master)
{
	if (!configurationIsSaved)
	{
		// prompt for save.
		if (!this->checkConfigurationSave("There are unsaved configuration settings. Would you like to save the current configuration before starting?", Master))
		{
			// canceled
			return false;
		}
	}
	return true;
}

bool ConfigurationFileSystem::checkConfigurationSave(std::string prompt, MasterWindow* Master)
{
	int answer = MessageBox(0, prompt.c_str(), 0, MB_YESNOCANCEL);
	if (answer == IDYES)
	{
		this->saveConfigurationOnly(Master);
	}
	else if (answer == IDCANCEL)
	{
		return false;
	}
	return true;
}

bool ConfigurationFileSystem::configurationChangeHandler(MasterWindow* Master)
{
	if (!configurationIsSaved)
	{
		if (!this->checkConfigurationSave("The current configuration is unsaved. Save current configuration before changing?", Master))
		{
			SendMessage(configCombo.hwnd, CB_SELECTSTRING, 0, (LPARAM)currentProfileSettings.configuration.c_str());
			return false;
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
	if (!this->openConfiguration(configurationToOpen, Master))
	{
		return false;
	}
	// it'd be confusing if these notes stayed here.
	return true;
}
/// CATEGORY LEVEL HANDLING
/*
]--- This function attempts to save the category given the category name in the argument. It throws errors and warnings if this is not a "normal"
]- Save, i.e. if the file doesn't already exist or if the user tries to pass an empty name as an argument. It returns false if the category got saved,
]- true if something prevented the category from being saved.
*/
bool ConfigurationFileSystem::saveCategoryOnly(MasterWindow* Master)
{
	std::string categoryNameToSave = currentProfileSettings.category;
	// check if experiment has been set
	if (currentProfileSettings.experiment == "")
	{
		MessageBox(0, "The Experiment has not yet been selected! Please select a Experiment or create a new one before trying to save this "
			"category.", 0, 0);
		return false;
	}
	// check to make sure that this is a name.
	if (categoryNameToSave == "")
	{
		MessageBox(0, "ERROR: The program requested the saving of the category file to an empty name! This shouldn't happen, ask Mark about bugs.", 0, 0);
		return false;
	}

	// check if file already exists. No extension, looking for a folder here. 
	if (!ConfigurationFileSystem::fileOrFolderExists(pathIncludingCategory + categoryNameToSave + CATEGORY_EXTENSION))
	{
		int answer = MessageBox(0, ("This category file appears to not exist in the expected location: " + pathIncludingCategory + categoryNameToSave 
			+ CATEGORY_EXTENSION + ".  Continue by making a new category file?").c_str(), 0, MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return false;
		}
	}

	if (!experimentIsSaved)
	{
		int answer = MessageBox(0, "The Experiment settings have not yet been saved. Save them before the category? (Optional)", 0, MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return false;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			if (!this->saveExperimentOnly(Master))
			{
				return false;
			}
		}
	}
	std::fstream categoryFileToSave(pathIncludingCategory + categoryNameToSave + CATEGORY_EXTENSION, std::ios::out);
	if (!categoryFileToSave.is_open())
	{
		MessageBox(0, "ERROR: failed to save category file! Ask mark about bugs.", 0, 0);
		return false;
	}
	categoryFileToSave << "Version: 1.0\n";
	std::string categoryNotes = Master->notes.getCategoryNotes();
	categoryFileToSave << categoryNotes + "\n";
	categoryFileToSave << "END CATEGORY NOTES\n";
	currentProfileSettings.category = categoryNameToSave;
	pathIncludingCategory = pathIncludingExperiment + categoryNameToSave + "\\";
	this->updateCategorySavedStatus(true);
	return true;
}

std::string ConfigurationFileSystem::getCurrentPathIncludingCategory()
{
	return pathIncludingCategory;
}

/*
]--- identical to saveCategoryOnly except that 
*/
bool ConfigurationFileSystem::saveCategoryAs(MasterWindow* Master)
{
	// check if experiment has been set
	if (currentProfileSettings.experiment == "")
	{
		MessageBox(0, "The Experiment has not yet been selected! Please select a Experiment or create a new one before trying to save this "
			"category.", 0, 0);
		return false;
	}
	std::string categoryNameToSave = (const char*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new configuration name.");
	if (categoryNameToSave == "")
	{
		MessageBox(0, "ERROR: The program requested the saving of the category file to an empty name! This shouldn't happen, ask Mark about bugs.", 0, 0);
		return false;
	}

	// check if file already exists. No extension, looking for a folder here. 
	if (!ConfigurationFileSystem::fileOrFolderExists(pathIncludingCategory + categoryNameToSave + CATEGORY_EXTENSION))
	{
		int answer = MessageBox(0, ("This category file appears to not exist in the expected location: " + pathIncludingCategory + categoryNameToSave
			+ CATEGORY_EXTENSION + ".  Continue by making a new category file?").c_str(), 0, MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return false;
		}
	}

	if (!experimentIsSaved)
	{
		int answer = MessageBox(0, "The Experiment settings have not yet been saved. Save them before the category? (Optional)", 0, MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return false;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			if (!this->saveExperimentOnly(Master))
			{
				return false;
			}
		}
	}
	// need to make a new folder as well.
	int result = CreateDirectory((pathIncludingExperiment + categoryNameToSave).c_str(), 0);
	if (result == 0)
	{
		MessageBox(0, "ERROR: failed to create new category directory during category save as! Ask Mark about Bugs.", 0, 0);
		return false;
	}
	std::fstream categoryFileToSave(pathIncludingCategory + categoryNameToSave + CATEGORY_EXTENSION, std::ios::out);
	if (!categoryFileToSave.is_open())
	{
		MessageBox(0, "ERROR: failed to save category file! Ask mark about bugs.", 0, 0);
		return false;
	}
	categoryFileToSave << "Version: 1.0\n";
	std::string categoryNotes = Master->notes.getCategoryNotes();
	categoryFileToSave << categoryNotes + "\n";
	categoryFileToSave << "END CATEGORY NOTES\n";
	currentProfileSettings.category = categoryNameToSave;
	pathIncludingCategory = pathIncludingExperiment + categoryNameToSave + "\\";
	this->updateCategorySavedStatus(true);
	return true;
}

/*
]---
*/
bool ConfigurationFileSystem::renameCategory()
{
	// TODO: this is a bit more complicated because of the way that all of the configuration fle locations are currently set.
	MessageBox(0, "This feature still needs implementing! It doesn't work right now", 0, 0);
	return true;
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
		return false;
	}
	answer = MessageBox(0, ("Are you really sure? The current category is: " + currentProfileSettings.category).c_str(), 0, MB_YESNO);
	if (answer == IDNO)
	{
		return false;
	}
	std::string currentCategoryLocation = pathIncludingExperiment + currentProfileSettings.category;
	if (!this->fullyDeleteFolder(currentCategoryLocation))
	{
		return false;
	}
	this->updateCategorySavedStatus(false);
	currentProfileSettings.category = "";
	pathIncludingCategory == "";
	this->reloadCombo(categoryCombo.hwnd, pathIncludingExperiment, "*", "__NONE__");
	return true;
}

bool ConfigurationFileSystem::newCategory(MasterWindow* Master)
{
	// check if experiment has been set
	if (currentProfileSettings.experiment == "")
	{
		MessageBox(0, "The Experiment has not yet been selected! Please select a Experiment or create a new one before trying to save this "
			"category.", 0, 0);
		return false;
	}
	std::string categoryNameToSave = (const char*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0,
		(DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter name for the new Category.");
	// check to make sure that this is a name.
	if (categoryNameToSave == "")
	{
		// canceled
		return false;
	}

	// check if file already exists. No extension, looking for a folder here. 
	if (ConfigurationFileSystem::fileOrFolderExists(pathIncludingExperiment + categoryNameToSave))
	{
		MessageBox(0, "This category name already exists! If it doesn't appear in the combo, try taking a look at what's in the relvant folder...", 0, 0);
		return false;
	}
	int result = CreateDirectory((pathIncludingExperiment + categoryNameToSave).c_str(), 0);
	if (result == 0)
	{
		MessageBox(0, "ERROR: failed to create category directory! Ask Mark about bugs.", 0, 0);
		return false;
	}
	std::ofstream categorySaveFolder(pathIncludingExperiment + categoryNameToSave + "\\" + categoryNameToSave + CATEGORY_EXTENSION);
	categorySaveFolder.close();
	this->reloadCombo(categoryCombo.hwnd, pathIncludingExperiment, "*", currentProfileSettings.category);
	return true;
}

bool ConfigurationFileSystem::openCategory(std::string categoryToOpen, MasterWindow* Master)
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
		Master->notes.setCategoryNotes(notes);
	}
	else
	{
		Master->notes.setCategoryNotes("");
	}
	//SetWindowText(eConfigurationDisplayInScripting, "");
	// close.
	categoryConfigOpenFile.close();
	this->updateCategorySavedStatus(true);
	return true;
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
	return true;
}

bool ConfigurationFileSystem::categorySettingsReadyCheck(MasterWindow* Master)
{
	if (!categoryIsSaved)
	{
		if (this->checkSequenceSave("There are unsaved category settings. Would you like to save the current category before starting?", Master))
		{
			// canceled
			return false;
		}
	}
	return true;
}

bool ConfigurationFileSystem::checkCategorySave(std::string prompt, MasterWindow* Master)
{
	int answer = MessageBox(0, prompt.c_str(), 0, MB_YESNOCANCEL);
	if (answer == IDYES)
	{
		this->saveCategoryOnly(Master);
	}
	else if (answer == IDCANCEL)
	{
		return false;
	}
	return true;
}

bool ConfigurationFileSystem::categoryChangeHandler(MasterWindow* Master)
{
	if (!categoryIsSaved)
	{
		if (!this->checkCategorySave("The current category is unsaved. Save current category before changing?", Master))
		{
			SendMessage(categoryCombo.hwnd, CB_SELECTSTRING, 0, (LPARAM)currentProfileSettings.category.c_str());
			return false;
		}
	}
	// get current item.
	long long itemIndex = SendMessage(categoryCombo.hwnd, CB_GETCURSEL, 0, 0);
	if (itemIndex == -1)
	{
		// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
		// is blank. Just break out, this is fine.
		return false;
	}
	TCHAR categoryConfigToOpen[256];
	// Send CB_GETLBTEXT message to get the item.
	SendMessage(categoryCombo.hwnd, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)categoryConfigToOpen);
	if (!this->openCategory(std::string(categoryConfigToOpen), Master))
	{
		return false;
	}
	// it'd be confusing if these notes stayed here.
	Master->notes.setConfigurationNotes("");
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
	return true;
}

/// EXPERIMENT LEVEL HANDLING
bool ConfigurationFileSystem::saveExperimentOnly(MasterWindow* Master)
{
	std::string experimentNameToSave = currentProfileSettings.experiment;
	// check that the experiment name is not empty.
	if (experimentNameToSave == "")
	{
		MessageBox(0, "ERROR: Please properly select the experiment or create a new one (\'new experiment\') before trying to save it!", 0, 0);
		return false;
	}
	// check if file already exists
	if (!ConfigurationFileSystem::fileOrFolderExists(FILE_SYSTEM_PATH + experimentNameToSave + "\\" + experimentNameToSave + EXPERIMENT_EXTENSION))
	{
		int answer = MessageBox(0, ("This experiment file appears to not exist in the expected location: " + FILE_SYSTEM_PATH + "   \r\n."
			"Continue by making a new experiment file?").c_str(), 0, MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return false;
		}
	}
	std::ofstream experimentSaveFile(FILE_SYSTEM_PATH + experimentNameToSave + "\\" + experimentNameToSave + EXPERIMENT_EXTENSION);
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
	std::string notes = Master->notes.getExperimentNotes();
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
	return true;
}

bool ConfigurationFileSystem::saveExperimentAs(MasterWindow* Master)
{
	if (currentProfileSettings.experiment == "")
	{
		MessageBox(0, "Please select an experiment before using \"Save As\"", 0, 0);
		return false;
	}
	std::string experimentNameToSave = (const char*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new experiment name.");
	// check that the experiment name is not empty. 
	if (experimentNameToSave == "")
	{
		// canceled.
		return false;
	}

	// check if file already exists
	if (ConfigurationFileSystem::fileOrFolderExists(FILE_SYSTEM_PATH + experimentNameToSave + ".eConfig"))
	{
		int answer = MessageBox(0, ("This experiment name appears to already exist in the expected location: " + FILE_SYSTEM_PATH + "."
			"Overwrite this file?").c_str(), 0, MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return false;
		}
	}

	int result = CreateDirectory((FILE_SYSTEM_PATH + experimentNameToSave).c_str(), 0);
	if (result == 0)
	{
		MessageBox(0, "ERROR: failed to create new experiment directory during save as! Ask Mark about bugs.", 0, 0);
		return false;
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
	std::string notes = Master->notes.getExperimentNotes();
	experimentSaveFile << notes << "\n";
	experimentSaveFile << "END EXPERIMENT NOTES" << "\n";
	// And done.
	experimentSaveFile.close();
	// update the save path. 
	pathIncludingExperiment = FILE_SYSTEM_PATH + experimentNameToSave + "\\";
	// update the configuration saved statis for "this" object.
	this->updateExperimentSavedStatus(true);
	this->reloadCombo(experimentCombo.hwnd, FILE_SYSTEM_PATH, "*", currentProfileSettings.experiment);
	return true;
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
		return false;
	}
	int answer = MessageBox(0, ("Are you sure that you'd like to delete the current experiment and all categories and configurations within? Current Experiment: " + currentProfileSettings.experiment).c_str(), 0, MB_YESNO);
	if (answer == IDNO)
	{
		return false;
	}
	answer = MessageBox(0, ("Are you really really sure? Current Experiment: " + currentProfileSettings.experiment).c_str(), 0, MB_YESNO);
	if (answer == IDNO)
	{
		return false;
	}
	std::string experimentConfigLocation = FILE_SYSTEM_PATH + currentProfileSettings.experiment + "\\" + currentProfileSettings.experiment + EXPERIMENT_EXTENSION;
	if (DeleteFile(experimentConfigLocation.c_str()))
	{
		MessageBox(0, "Deleting .eConfig file failed! Ask Mark about bugs.", 0, 0);
		return false;
	}
	if (!this->fullyDeleteFolder(pathIncludingExperiment + currentProfileSettings.experiment))
	{
		return false;
	}
	this->reloadCombo(experimentCombo.hwnd, FILE_SYSTEM_PATH, "*", "__NONE__");
	updateExperimentSavedStatus(false);
	currentProfileSettings.experiment = "";
	pathIncludingExperiment = "";
	return true;
}

bool ConfigurationFileSystem::newExperiment(MasterWindow* Master)
{
	std::string newExperimentName = (const char*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0,
																(DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter name for the new Experiment.");
	std::string newExperimentPath = FILE_SYSTEM_PATH + newExperimentName;
	CreateDirectory(newExperimentPath.c_str(), 0);
	std::ofstream newExperimentConfigFile;
	newExperimentConfigFile.open((newExperimentPath + "\\" + newExperimentName + EXPERIMENT_EXTENSION).c_str());
	this->reloadCombo(experimentCombo.hwnd, FILE_SYSTEM_PATH, "*", currentProfileSettings.experiment);
	return true;
}

bool ConfigurationFileSystem::openExperiment(std::string experimentToOpen, MasterWindow* Master)
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
		return false;
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
		Master->notes.setExperimentNotes(notes);
	}
	else
	{
		Master->notes.setExperimentNotes("");
	}

	//SetWindowText(eConfigurationDisplayInScripting, "");
	// close.
	experimentConfigOpenFile.close();
	this->updateExperimentSavedStatus(true);
	return true;
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
	return true;
}

bool ConfigurationFileSystem::experimentSettingsReadyCheck(MasterWindow* Master)
{
	if (!experimentIsSaved)
	{
		if (!this->checkExperimentSave("There are unsaved Experiment settings. Would you like to save the current experimnet before starting?", Master))
		{
			// canceled
			return false;
		}
	}
	return true;
}
bool ConfigurationFileSystem::checkExperimentSave(std::string prompt, MasterWindow* Master)
{
	int answer = MessageBox(0, prompt.c_str(), 0, MB_YESNOCANCEL);
	if (answer == IDYES)
	{
		this->saveExperimentOnly(Master);
	}
	else if (answer == IDCANCEL)
	{
		return false;
	}
	return true;
}

bool ConfigurationFileSystem::experimentChangeHandler(MasterWindow* Master)
{
	if (!experimentIsSaved)
	{
		if (!this->checkExperimentSave("The current experiment is unsaved. Save Current Experiment before Changing?", Master))
		{
			return false;
		}
	}
	// get current item.
	long long itemIndex = SendMessage(experimentCombo.hwnd, CB_GETCURSEL, 0, 0);
	if (itemIndex == -1)
	{
		// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
		// is blank. Just break out, this is fine.
		return false;
	}
	TCHAR experimentConfigToOpen[256];
	// Send CB_GETLBTEXT message to get the item.
	SendMessage(experimentCombo.hwnd, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)experimentConfigToOpen);
	if (!this->openExperiment(std::string(experimentConfigToOpen), Master))
	{
		return false;
	}
	this->reloadCombo(categoryCombo.hwnd, pathIncludingExperiment, "*", "__NONE__");
	// it'd be confusing if this category-specific text remained after the category get set to blank.
	Master->notes.setCategoryNotes("");
	Master->notes.setConfigurationNotes("");
	return true;
}

bool ConfigurationFileSystem::loadNullSequence(MasterWindow* Master)
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
			return false;
		}
		// change edit
		SendMessage(sequenceInfoDisplay.hwnd, WM_SETTEXT, 256, (LPARAM)"Sequence of Configurations to Run:\r\n");
		appendText("1. " + sequenceConfigurationNames[0] + "\r\n", IDC_SEQUENCE_DISPLAY, Master->masterWindowHandle);
	}
	else
	{
		SendMessage(sequenceInfoDisplay.hwnd, WM_SETTEXT, 256, (LPARAM)"Sequence of Configurations to Run:\r\n");
		appendText("No Configuration Loaded\r\n", IDC_SEQUENCE_DISPLAY, Master->masterWindowHandle);
	}
	SendMessage(sequenceCombo.hwnd, CB_SELECTSTRING, 0, (LPARAM)NULL_SEQUENCE);
	this->updateSequenceSavedStatus(true);
	return true;
}

bool ConfigurationFileSystem::addToSequence(MasterWindow* Master)
{
	if (currentProfileSettings.configuration == "")
	{
		// nothing to add.
		return false;
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
	appendText(std::to_string(sequenceConfigurationNames.size()) + ". " + sequenceConfigurationNames.back() + "\r\n", IDC_SEQUENCE_DISPLAY, Master->masterWindowHandle);
	this->updateSequenceSavedStatus(false);
	return true;
}

/// SEQUENCE HANDLING
bool ConfigurationFileSystem::sequenceChangeHandler(MasterWindow* Master)
{
	// get the name
	long long itemIndex = SendMessage(sequenceCombo.hwnd, CB_GETCURSEL, 0, 0);
	TCHAR sequenceName[256];
	SendMessage(sequenceCombo.hwnd, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)sequenceName);
	if (itemIndex == -1)
	{
		// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
		// is blank. just break out, this is fine.
		return false;
	}
	if (std::string(sequenceName) == NULL_SEQUENCE)
	{
		this->loadNullSequence(Master);
		return true;
	}
	else
	{
		this->openSequence(sequenceName, Master);
	}
	// else not null_sequence.
	this->reloadSequence(currentProfileSettings.sequence, Master);
	this->updateSequenceSavedStatus(true);
	return true;
}
bool ConfigurationFileSystem::reloadSequence(std::string sequenceToReload, MasterWindow* Master)
{
	this->reloadCombo(sequenceCombo.hwnd, pathIncludingCategory, std::string("*") + SEQUENCE_EXTENSION, sequenceToReload);
	SendMessage(sequenceCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)NULL_SEQUENCE);
	if (sequenceToReload == NULL_SEQUENCE)
	{
		this->loadNullSequence(Master);
	}
	return true;
}
bool ConfigurationFileSystem::saveSequence(MasterWindow* Master)
{
	if (this->currentProfileSettings.category == "")
	{
		if (this->currentProfileSettings.experiment == "")
		{
			MessageBox(0, "Please set category and experiment before saving sequence.", 0, 0);
			return false;
		}
		else
		{
			MessageBox(0, "Please set category before saving sequence.", 0, 0);
			return false;
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
		result = (TCHAR*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, textPromptDialogProcedure, (LPARAM)"Please enter a name for this sequence: ");
		if (std::string(result) == "")
		{
			return false;
		}
		currentProfileSettings.sequence = result;
	}
	std::fstream sequenceSaveFile(pathIncludingCategory + "\\" + currentProfileSettings.sequence + SEQUENCE_EXTENSION, std::fstream::out);
	if (!sequenceSaveFile.is_open())
	{
		MessageBox(0, "ERROR: Couldn't open sequence file for saving!", 0, 0);
		return false;
	}
	sequenceSaveFile << "Version: 1.0\n";
	for (int sequenceInc = 0; sequenceInc < sequenceConfigurationNames.size(); sequenceInc++)
	{
		sequenceSaveFile << sequenceConfigurationNames[sequenceInc] + "\n";
	}
	sequenceSaveFile.close();
	this->reloadSequence(currentProfileSettings.sequence, Master);
	this->updateSequenceSavedStatus(true);
	return true;
}

bool ConfigurationFileSystem::saveSequenceAs(MasterWindow* Master)
{
	// prompt for name
	TCHAR* result = NULL;
	result = (TCHAR*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, textPromptDialogProcedure, (LPARAM)"Please Enter a new Sequence Name:");
	//
	if (result == NULL || std::string(result) == "")
	{
		// user canceled or entered nothing
		return false;
	}
	if (std::string(result) == NULL_SEQUENCE)
	{
		// nothing to save;
		return false;
	}
	// if not saved...
	std::fstream sequenceSaveFile(pathIncludingCategory + "\\" + std::string(result) + SEQUENCE_EXTENSION, std::fstream::out);
	if (!sequenceSaveFile.is_open())
	{
		MessageBox(0, "ERROR: Couldn't open sequence file for saving!", 0, 0);
		return false;
	}
	currentProfileSettings.sequence = std::string(result);
	sequenceSaveFile << "Version: 1.0\n";
	for (int sequenceInc = 0; sequenceInc < sequenceConfigurationNames.size(); sequenceInc++)
	{
		sequenceSaveFile << sequenceConfigurationNames[sequenceInc] + "\n";
	}
	sequenceSaveFile.close();
	this->updateSequenceSavedStatus(true);
	return true;
}

bool ConfigurationFileSystem::renameSequence(MasterWindow* Master)
{
	// check if configuration has been set yet.
	if (currentProfileSettings.sequence == "" || currentProfileSettings.sequence == NULL_SEQUENCE)
	{
		MessageBox(0, "Please select a sequence for renaming.", 0, 0);
		return false;
	}
	std::string newSequenceName = (const char*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new configuration name.");
	if (newSequenceName == "")
	{
		// canceled
		return false;
	}
	int result = MoveFile((pathIncludingCategory + currentProfileSettings.sequence + SEQUENCE_EXTENSION).c_str(), 
						  (pathIncludingCategory + newSequenceName + SEQUENCE_EXTENSION).c_str());
	if (result == 0)
	{
		MessageBox(0, "Renaming of the sequence file Failed! Ask Mark about bugs", 0, 0);
		return false;
	}
	currentProfileSettings.sequence = newSequenceName;
	this->reloadSequence(currentProfileSettings.sequence, Master);
	this->updateSequenceSavedStatus(true);
	return true;
}

bool ConfigurationFileSystem::deleteSequence(MasterWindow* Master)
{
	// check if configuration has been set yet.
	if (currentProfileSettings.sequence == "" || currentProfileSettings.sequence == NULL_SEQUENCE)
	{
		MessageBox(0, "Please select a sequence for deleting.", 0, 0);
		return false;
	}
	int answer = MessageBox(0, ("Are you sure you want to delete the current sequence: " + currentProfileSettings.sequence).c_str(), 0, MB_YESNO);
	if (answer == IDNO)
	{
		return false;
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
	return true;
}
bool ConfigurationFileSystem::newSequence(MasterWindow* Master)
{
	// prompt for name
	TCHAR* result = NULL;
	result = (TCHAR*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, textPromptDialogProcedure, (LPARAM)"Please Enter a new Sequence Name:");
	//
	if (result == NULL || std::string(result) == "")
	{
		// user canceled or entered nothing
		return false;
	}
	// try to open the file.
	std::fstream sequenceFile(pathIncludingCategory + "\\" + result + SEQUENCE_EXTENSION, std::fstream::out);
	if (!sequenceFile.is_open())
	{
		MessageBox(0, "Couldn't create a file with this sequence name! Make sure there are no forbidden characters in your name.", 0, 0);
		return false;
	}
	std::string newSequenceName = std::string(result);
	sequenceFile << newSequenceName + "\n";
	// output current configuration
	//eSequenceFileNames.clear();
	if (newSequenceName == "")
	{
		return false;
	}
	// reload combo.
	this->reloadSequence(currentProfileSettings.sequence, Master);
	return true;
}

bool ConfigurationFileSystem::openSequence(std::string sequenceName, MasterWindow* Master)
{
	// try to open the file
	std::fstream sequenceFile(pathIncludingCategory + sequenceName + SEQUENCE_EXTENSION);
	if (!sequenceFile.is_open())
	{
		MessageBox(0, ("ERROR: sequence file failed to open! Make sure the sequence with address ..." + pathIncludingCategory + sequenceName + SEQUENCE_EXTENSION + " exists.").c_str(), 0, 0);
		return false;
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
		appendText(std::to_string(sequenceInc + 1) + ". " + sequenceConfigurationNames[sequenceInc] + "\r\n", IDC_SEQUENCE_DISPLAY, Master->masterWindowHandle);
	}
	this->updateSequenceSavedStatus(true);
	return true;
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
	return true;
}

bool ConfigurationFileSystem::sequenceSettingsReadyCheck(MasterWindow* Master)
{
	if (!sequenceIsSaved)
	{
		if (!this->checkSequenceSave("There are unsaved sequence settings. Would you like to save the current sequence before starting?", Master))
		{
			// canceled
			return false;
		}
	}
	return true;
}

bool ConfigurationFileSystem::checkSequenceSave(std::string prompt, MasterWindow* Master)
{
	int answer = MessageBox(0, prompt.c_str(), 0, MB_YESNOCANCEL);
	if (answer == IDYES)
	{
		this->saveSequence(Master);
	}
	else if (answer == IDCANCEL)
	{
		return false;
	}
	return true;
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

bool ConfigurationFileSystem::initialize(POINT& topLeftPosition, HWND masterWindowHandle, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master)
{
	RECT currentRect;
	// Experiment Type
	currentRect = experimentLabel.position = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 20 };
	experimentLabel.hwnd = CreateWindowEx(NULL, "STATIC", "EXPERIMENT", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top, 
		masterWindowHandle, (HMENU)experimentLabel.ID, GetModuleHandle(NULL), NULL);
	SendMessage(experimentLabel.hwnd, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// Experiment Saved Indicator
	currentRect = experimentSavedIndicator.position = { topLeftPosition.x + 360, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 20 };
	experimentSavedIndicator.hwnd = CreateWindowEx(NULL, "BUTTON", "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top, 
		masterWindowHandle, (HMENU)experimentSavedIndicator.ID, GetModuleHandle(NULL), NULL);
	SendMessage(experimentSavedIndicator.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(experimentSavedIndicator.hwnd, BM_SETCHECK, BST_CHECKED, NULL);
	updateExperimentSavedStatus(true);
	// Category Title

	currentRect = categoryLabel.position = { topLeftPosition.x + 480, topLeftPosition.y, topLeftPosition.x + 960, topLeftPosition.y + 20 };
	categoryLabel.hwnd = CreateWindowEx(NULL, "STATIC", "CATEGORY", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top, 
		masterWindowHandle, (HMENU)categoryLabel.ID, GetModuleHandle(NULL), NULL);
	SendMessage(categoryLabel.hwnd, WM_SETFONT, WPARAM(sHeadingFont), TRUE);

	currentRect = categorySavedIndicator.position = { topLeftPosition.x + 480 + 380, topLeftPosition.y, topLeftPosition.x + 960, topLeftPosition.y + 20};
	categorySavedIndicator.hwnd = CreateWindowEx(NULL, "BUTTON", "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top, 
		masterWindowHandle, (HMENU)categorySavedIndicator.ID, GetModuleHandle(NULL), NULL);
	SendMessage(categorySavedIndicator.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(categorySavedIndicator.hwnd, BM_SETCHECK, BST_CHECKED, NULL);
	updateCategorySavedStatus(true);
	topLeftPosition.y += 20;
	// Experiment Combo
	currentRect = experimentCombo.position = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 800 };
	experimentCombo.hwnd = CreateWindowEx(NULL, TEXT("ComboBox"), "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top,
		masterWindowHandle, (HMENU)this->experimentCombo.ID, GetModuleHandle(NULL), NULL);
	SendMessage(experimentCombo.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	this->reloadCombo(experimentCombo.hwnd, PROFILES_PATH, std::string("*"), "__NONE__");
	// Category Combo
	currentRect = categoryCombo.position = { topLeftPosition.x + 480, topLeftPosition.y, topLeftPosition.x + 960, topLeftPosition.y + 800 };
	categoryCombo.hwnd = CreateWindowEx(NULL, TEXT("ComboBox"), "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top, 
		masterWindowHandle, (HMENU)this->categoryCombo.ID, GetModuleHandle(NULL), NULL);
	SendMessage(categoryCombo.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	topLeftPosition.y += 25;
	// Orientation Title
	currentRect = orientationLabel.position = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 120, topLeftPosition.y + 20 };
	orientationLabel.hwnd = CreateWindowEx(NULL, "STATIC", "ORIENTATION", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top, 
		masterWindowHandle, (HMENU)this->orientationLabel.ID, GetModuleHandle(NULL), NULL);
	SendMessage(orientationLabel.hwnd, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// Configuration Title
	currentRect = configLabel.position = { topLeftPosition.x + 120, topLeftPosition.y, topLeftPosition.x + 960, topLeftPosition.y + 20 };
	configLabel.hwnd = CreateWindowEx(NULL, "STATIC", "CONFIGURATION", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top, 
		masterWindowHandle, (HMENU)configLabel.ID, GetModuleHandle(NULL), NULL);
	SendMessage(configLabel.hwnd, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// Configuration Saved Indicator
	currentRect = configurationSavedIndicator.position = { topLeftPosition.x + 860, topLeftPosition.y, topLeftPosition.x + 960, topLeftPosition.y + 20 };
	configurationSavedIndicator.hwnd = CreateWindowEx(NULL, "BUTTON", "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top,
		masterWindowHandle, (HMENU)configurationSavedIndicator.ID, GetModuleHandle(NULL), NULL);
	SendMessage(configurationSavedIndicator.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(configurationSavedIndicator.hwnd, BM_SETCHECK, BST_CHECKED, NULL);
	updateConfigurationSavedStatus(true);
	topLeftPosition.y += 20;
	// orientation combo
	std::vector<std::string> orientationNames;
	orientationNames.push_back("Horizontal");
	orientationNames.push_back("Vertical");
	currentRect = orientationCombo.position = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 120, topLeftPosition.y + 800 };
	orientationCombo.hwnd = CreateWindowEx(NULL, TEXT("ComboBox"), "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top, 
		masterWindowHandle, (HMENU)orientationCombo.ID, GetModuleHandle(NULL), NULL);
	SendMessage(orientationCombo.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	for (int comboInc = 0; comboInc < orientationNames.size(); comboInc++)
	{
		SendMessage(orientationCombo.hwnd, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(_T(orientationNames[comboInc].c_str())));
	}
	SendMessage(orientationCombo.hwnd, CB_SETCURSEL, 0, 0);
	// configuration combo
	currentRect = configCombo.position = { topLeftPosition.x + 120, topLeftPosition.y, topLeftPosition.x + 960, topLeftPosition.y + 800 };
	configCombo.hwnd = CreateWindowEx(NULL, TEXT("ComboBox"), "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top, 
		masterWindowHandle, (HMENU)configCombo.ID, GetModuleHandle(NULL), NULL);
	SendMessage(configCombo.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	topLeftPosition.y += 25;
	/// SEQUENCE
	sequenceLabel.position = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 20 };
	currentRect = sequenceLabel.position;
	sequenceLabel.hwnd = CreateWindowEx(NULL, "STATIC", "SEQUENCE", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top,
		masterWindowHandle, (HMENU)sequenceLabel.ID, GetModuleHandle(NULL), NULL);
	SendMessage(sequenceLabel.hwnd, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	topLeftPosition.y += 20;
	// combo
	sequenceCombo.position = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 800 };
	currentRect = sequenceCombo.position;
	sequenceCombo.hwnd = CreateWindowEx(NULL, TEXT("ComboBox"), "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top,
		masterWindowHandle, (HMENU)sequenceCombo.ID, GetModuleHandle(NULL), NULL);
	SendMessage(sequenceCombo.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(sequenceCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"NULL SEQUENCE");
	SendMessage(sequenceCombo.hwnd, CB_SETCURSEL, 0, 0);
	topLeftPosition.y += 25;
	// display
	sequenceInfoDisplay.position = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 100 };
	currentRect = sequenceInfoDisplay.position;
	sequenceInfoDisplay.hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "Sequence of Configurations to Run:\r\n", ES_READONLY | WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top,
		masterWindowHandle, (HMENU)sequenceInfoDisplay.ID, GetModuleHandle(NULL), NULL);
	topLeftPosition.y += 100;
	return true;
}

bool ConfigurationFileSystem::reorganizeControls(RECT parentRectangle, std::string mode)
{
	return true;
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
	return true;
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
		return false;
	}
	return true;
}

INT_PTR ConfigurationFileSystem::handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, std::unordered_map<std::string, HBRUSH> brushes)
{
	DWORD controlID = GetDlgCtrlID((HWND)lParam);
	HDC hdcStatic = (HDC)wParam;
	if (controlID == this->categoryCombo.ID || controlID == this->experimentCombo.ID || controlID == this->configCombo.ID 
		|| controlID == this->orientationCombo.ID || controlID == this->sequenceCombo.ID)
	{
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		SetBkColor(hdcStatic, RGB(15, 15, 15));
		return (LRESULT)brushes["Dark Grey"];
	}
	else if (controlID == this->categoryLabel.ID || controlID == this->experimentLabel.ID || controlID == this->configLabel.ID 
		|| controlID == this->orientationLabel.ID || controlID == this->sequenceInfoDisplay.ID || controlID == this->sequenceLabel.ID 
		|| controlID == this->experimentSavedIndicator.ID || controlID == this->configurationSavedIndicator.ID || controlID == this->categorySavedIndicator.ID)
	{
		SetTextColor(hdcStatic, RGB(218, 165, 32));
		SetBkColor(hdcStatic, RGB(30, 30, 30));
		return (LRESULT)brushes["Medium Grey"];
	}
	else
	{
		return NULL;
	}
}



