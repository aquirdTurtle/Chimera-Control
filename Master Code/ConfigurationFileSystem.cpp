#include "stdafx.h"
#include "ConfigurationFileSystem.h"
#include "Windows.h"
#include <fstream>
#include "constants.h"
#include "Resource.h"
#include "Commctrl.h"
#include "textPromptDialogProcedure.h"
#include "fonts.h"
#include <boost/filesystem.hpp>
#include "myNIAWG.h"
#include "MasterWindow.h"


void ConfigurationFileSystem::rearrange(int width, int height, fontMap fonts)
{
	configLabel.rearrange("", "", width, height, fonts);
	configCombo.rearrange("", "", width, height, fonts);
	experimentLabel.rearrange("", "", width, height, fonts);
	experimentCombo.rearrange("", "", width, height, fonts);
	categoryLabel.rearrange("", "", width, height, fonts);
	categoryCombo.rearrange("", "", width, height, fonts);
	sequenceLabel.rearrange("", "", width, height, fonts);
	sequenceCombo.rearrange("", "", width, height, fonts);
	sequenceInfoDisplay.rearrange("", "", width, height, fonts);
	sequenceSavedIndicator.rearrange("", "", width, height, fonts);
	orientationLabel.rearrange("", "", width, height, fonts);
	orientationCombo.rearrange("", "", width, height, fonts);
	categorySavedIndicator.rearrange("", "", width, height, fonts);
	configurationSavedIndicator.rearrange("", "", width, height, fonts);
	experimentSavedIndicator.rearrange("", "", width, height, fonts);
}


ConfigurationFileSystem::ConfigurationFileSystem( std::string fileSystemPath ) : version(1.4)
{
	FILE_SYSTEM_PATH = fileSystemPath;
}


std::string ConfigurationFileSystem::getMasterAddressFromConfig()
{
	std::string configurationAddress;
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		configurationAddress = currentProfileSettings.pathIncludingCategory + currentProfileSettings.configuration + HORIZONTAL_EXTENSION;
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		configurationAddress = currentProfileSettings.pathIncludingCategory + currentProfileSettings.configuration + VERTICAL_EXTENSION;
	}
	else
	{
		thrower("ERROR: Unrecognized orientation: " + currentProfileSettings.orientation);
	}
	std::fstream configFile(configurationAddress);
	if (!configFile.is_open())
	{
		thrower("ERROR: Failed to open configuration file.");
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
		return getCurrentPathIncludingCategory() + address + MASTER_SCRIPT_EXTENSION;
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
		thrower("ERROR: Expected either \"LOCAL\" or \"NONLOCAL\" in configuration file, but instead found " + word);
	}
}


void ConfigurationFileSystem::saveEntireProfile(MasterWindow* Master)
{
	saveExperiment( Master );
	saveCategory( Master );
	saveConfiguration( Master );
	saveSequence( Master );
}


void ConfigurationFileSystem::checkSaveEntireProfile(MasterWindow* Master)
{
	if (!experimentIsSaved)
	{
		checkExperimentSave("Save Experiment Settings?", Master);
	}
	if (!categoryIsSaved)
	{
		checkCategorySave("Save Category Settings?", Master);
	}
	if (!configurationIsSaved)
	{
		checkConfigurationSave("Save Configuration Settings?", Master);
	}
	if (!sequenceIsSaved)
	{
		checkSequenceSave("Save Sequence Settings?", Master);
	}
}

void ConfigurationFileSystem::allSettingsReadyCheck(MasterWindow* Master)
{
	if (currentProfileSettings.configuration == "")
	{
		thrower("ERROR: Please set a configuration.");
	}
	// check all components of this class.
	experimentSettingsReadyCheck( Master );
	categorySettingsReadyCheck( Master );
	configurationSettingsReadyCheck( Master );
	sequenceSettingsReadyCheck( Master );
	// passed all checks.
}

void ConfigurationFileSystem::reloadAllCombos()
{
	// not implemented yet...?
	return;
}

/// ORIENTATION HANDLING

std::string ConfigurationFileSystem::getOrientation()
{
	return currentProfileSettings.orientation;
}

void ConfigurationFileSystem::setOrientation(std::string orientation)
{
	if (orientation != HORIZONTAL_ORIENTATION && orientation != VERTICAL_ORIENTATION)
	{
		thrower("ERROR: Tried to set non-standard orientation! Ask Mark about bugs.");
	}
	currentProfileSettings.orientation = orientation;
}

void ConfigurationFileSystem::orientationChangeHandler(MasterWindow* Master)
{
	long long ItemIndex = orientationCombo.GetCurSel();
	CString orientation;
	orientationCombo.GetLBText( ItemIndex, orientation );
	// reset some things.
	currentProfileSettings.orientation = str(orientation);
	if (currentProfileSettings.category != "")
	{
		if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
		{
			reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.pathIncludingCategory, str("*") 
						+ HORIZONTAL_EXTENSION, "__NONE__");
		}
		else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
		{
			reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.pathIncludingCategory, str("*") 
						+ VERTICAL_EXTENSION, "__NONE__");
		}
	}
	Master->notes.setConfigurationNotes("");
	currentProfileSettings.configuration = "";
	/// Load the relevant NIAWG script.
	myNIAWG::loadDefault();
}

/// CONFIGURATION LEVEL HANDLING

void ConfigurationFileSystem::newConfiguration(MasterWindow* Master)
{
	// check if category has been set yet.
	if (currentProfileSettings.category == "")
	{
		// check if the experiment has also not been set.
		if (currentProfileSettings.experiment == "")
		{
			thrower( "The Experiment and category have not yet been selected! Please select a category or create a new one before trying to save this "
					 "configuration." );
		}
		else
		{
			thrower( "The category has not yet been selected! Please select a category or create a new one before trying to save this configuration." );
		}
	}

	std::string configurationNameToSave = (const char*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, 
		(DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new configuration name.");
	if (configurationNameToSave == "")
	{
		// canceled
		return;
	}
	std::string newConfigurationPath = currentProfileSettings.pathIncludingCategory + configurationNameToSave;

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
		thrower( "ERROR: Unrecognized orientation! Ask Mark about bugs.");
	}
	std::ofstream newConfigurationFile(newConfigurationPath.c_str());
	if (!newConfigurationFile.is_open())
	{
		thrower( "ERROR: Failed to create new configuration file. Ask Mark about bugs.");
	}
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.pathIncludingCategory, str("*") + HORIZONTAL_EXTENSION, currentProfileSettings.configuration.c_str());
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.pathIncludingCategory, str("*") + VERTICAL_EXTENSION, currentProfileSettings.configuration.c_str());
	}
	else
	{
		thrower( "ERROR: Unrecognized orientation! Ask Mark about bugs." );
	}
}


/*
]--- This function opens a given configuration file, sets all of the relevant parameters, and loads the associated scripts. 
*/
void ConfigurationFileSystem::openConfiguration(std::string configurationNameToOpen, MasterWindow* Master)
{
	// no folder associated with configuraitons. They share the category folder.
	std::string path = currentProfileSettings.pathIncludingCategory + configurationNameToOpen;
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
		thrower( "Opening of Configuration File Failed!" );
	}
	currentProfileSettings.configuration = configurationNameToOpen;
	
	// not currenlty used because version 1.0.
	std::string versionStr;
	double version;
	// eat the first two words, "Master" and "Version:".
	configurationFile >> versionStr;
	configurationFile >> versionStr;
	configurationFile >> versionStr;
	try
	{
		version = std::stod(versionStr);
	}
	catch (std::invalid_argument& version)
	{
		if (versionStr == "")
		{
			return;
		}
		thrower("ERROR: Unrecognized configuration version! Ask Mark about bugs.");
	}
	std::string masterScriptFile;
	configurationFile.get();
	std::getline(configurationFile, masterScriptFile);

	if (masterScriptFile != "MASTER SCRIPT:")
	{
		thrower("ERRORL Expected \"MASTER SCRIPT:\" in configuration file, but instead found " + masterScriptFile);
	}
	configurationFile >> masterScriptFile;
	// figure out if the master script file was saved locally or in a specific location.
	if (masterScriptFile == "LOCAL")
	{
		configurationFile.get();
		getline( configurationFile, masterScriptFile );
		std::string newPath = getCurrentPathIncludingCategory() + masterScriptFile + MASTER_SCRIPT_EXTENSION;
		Master->masterScript.openParentScript(newPath, Master);
	}
	else if (masterScriptFile == "NONLOCAL")
	{
		std::string newPath;
		configurationFile.get();
		getline(configurationFile, newPath);
		Master->masterScript.openParentScript(newPath, Master);
	}
	else
	{
		thrower("ERROR: Expected either \"LOCAL\" or \"NONLOCAL\" in configuration file, but instead found " + masterScriptFile);
	}
	std::string line;
	configurationFile >> line;
	if (line != "VARIABLES:")
	{
		thrower("ERROR: Expected \"VARIABLES\" in configuration file but instead there was " + line);
	}
	int varNum;
	configurationFile >> varNum;
	if (varNum < 0 || varNum > 1000)
	{
		int answer = MessageBox(0, ("ERROR: variable number retrieved from file appears suspicious. The number is " 
								+ str(varNum) + ". Is this accurate?").c_str(), 0, MB_YESNO);
		if (answer == IDNO)
		{
			// don't try to load anything.
			varNum = 0;
			return;
		}
	}
	// Number of Variables
	Master->configVariables.clearVariables();

	for (int varInc = 0; varInc < varNum; varInc++)
	{
		variable tempVar;
		std::string varName, timelikeText, typeText, valueString;
		bool timelike;
		bool constant;
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
			thrower("ERROR: unknown timelike option. Check the formatting of the configuration file. Default to not timelike.");
			tempVar.timelike = false;
		}

		configurationFile >> typeText;
		if (typeText == "Constant")
		{
			tempVar.constant = true;
		}
		else if (typeText == "Variable")
		{
			tempVar.constant = false;
		}
		else
		{
			thrower("ERROR: unknown variable type option. Check the formatting of the configuration file. Default to constant.");
			tempVar.constant = true;
		}
		int rangeNumber;
		configurationFile >> rangeNumber;
		// I think it's unlikely to ever need more than 2 or 3 ranges.
		if (rangeNumber < 1 || rangeNumber > 1000)
		{
			errBox("ERROR: Bad range number! setting it to 1, but found " + str(rangeNumber) + " in the file.");
			rangeNumber = 1;
		}
		Master->configVariables.setVariationRangeNumber(rangeNumber, 0);
		// check if the range is actually too small.
		for (int rangeInc = 0; rangeInc < rangeNumber; rangeInc++)
		{
			double initValue, finValue;
			unsigned int variations;
			bool leftInclusive, rightInclusive;
			configurationFile >> initValue;
			configurationFile >> finValue;
			configurationFile >> variations;
			if (version >= 1.2)
			{
				configurationFile >> leftInclusive;
				configurationFile >> rightInclusive;
			}
			else
			{
				leftInclusive = false;
				rightInclusive = true;
			}
			tempVar.ranges.push_back({ initValue, finValue, variations, leftInclusive, rightInclusive });
		}
		// shouldn't be because of 1 forcing earlier.
		if (tempVar.ranges.size() == 0)
		{
			// make sure it has at least one entry.
			tempVar.ranges.push_back({ 0,0,1, false, true });
		}
		Master->configVariables.addConfigVariable(tempVar, varInc);
	}

	std::string ttlText;
	configurationFile >> ttlText;
	if (ttlText != "TTLS:")
	{
		thrower("ERROR: expected \"TTLS:\" in configuration file but instead found " + ttlText);
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
				Master->ttlBoard.forceTtl(ttlRowInc, ttlNumberInc, state);
			}
			catch (std::invalid_argument& exception)
			{
				thrower("ERROR: the ttl status of \"" + ttlString + "\"failed to convert to a bool!");
			}
		}
	}
	std::string dacText;
	configurationFile >> dacText;
	if (dacText != "DACS:")
	{
		thrower("ERROR: Expected \"DACS:\" in configuration file but instead found " + dacText);
	}
	for (int dacInc = 0; dacInc < Master->dacBoards.getNumberOfDacs(); dacInc++)
	{
		std::string dacString;
		configurationFile >> dacString;
		try
		{
			double dacValue = std::stod(dacString);
			Master->dacBoards.prepareDacForceChange(dacInc, dacValue, &Master->ttlBoard);
		}
		catch (std::invalid_argument& exception)
		{
			thrower("ERROR: failed to convert dac value to voltage. string was " + dacString);
		}
	}
	std::string repText;
	configurationFile >> repText;
	if (repText != "REPETITIONS:")
	{
		thrower("ERROR: Expected \"REPETITIONS:\" in configuration file, but instead found " + repText);
	}
	configurationFile >> repText;
	try
	{
		int repNum = std::stoi(repText);
		Master->repetitionControl.setRepetitions(repNum);
	}
	catch (std::invalid_argument& exception)
	{
		thrower("ERROR: failed to convert repetition text to integer. Text was " + repText);
	}

	// add a blank line
	variable tempVar;
	tempVar.name = "";
	tempVar.constant = true;
	tempVar.timelike = false;
	tempVar.ranges.push_back({ 0,0,0, false, true });
	Master->configVariables.addConfigVariable(tempVar, varNum);
	updateConfigurationSavedStatus(true);
	currentProfileSettings.configuration = configurationNameToOpen;

	Master->topBottomAgilent.readConfigurationFile( configurationFile );
	Master->uWaveAxialAgilent.readConfigurationFile( configurationFile );
	Master->flashingAgilent.readConfigurationFile( configurationFile );

	std::string notes;
	std::string tempNote;
	std::string notesNote;
	configurationFile >> notesNote;
	if (notesNote != "NOTES:")
	{
		thrower("ERROR: Expected \"NOTES:\" in configuration file but instead found " + notesNote);
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
	// actually set this now
	//SetWindowText(eConfigurationDisplayInScripting, (currentProfileSettings.category + "->" + currentProfileSettings.configuration).c_str());
	// close.
	if (currentProfileSettings.sequence == NULL_SEQUENCE)
	{
		// reload it.
		loadNullSequence(Master);
	}
	if (version >= 1.4)
	{
		std::getline(configurationFile, tempNote);
		if (tempNote != "TEKTRONICS1:")
		{
			thrower("ERROR: Expected \"TEKTRONICS1:\" in configuration file but instead found " + tempNote + "!");
		}
		std::getline(configurationFile, tempNote);
		if (tempNote != "CHANNEL1:")
		{
			thrower("ERROR: Expected \"CHANNEL1:\" in configuration file but instead found " + tempNote + "!");
		}
		tektronicsInfo tekInfo;
		configurationFile >> tekInfo.channels.first.on;
		configurationFile >> tekInfo.channels.first.fsk;
		configurationFile.get();
		std::getline(configurationFile, tekInfo.channels.first.power);
		std::getline(configurationFile, tekInfo.channels.first.mainFreq);
		std::getline(configurationFile, tekInfo.channels.first.fskFreq);
		std::getline(configurationFile, tempNote);
		if (tempNote != "CHANNEL2:")
		{
			thrower("ERROR: Expected \"CHANNEL2:\" in configuration file but instead found " + tempNote + "!");
		}
		configurationFile >> tekInfo.channels.second.on;
		configurationFile >> tekInfo.channels.second.fsk;
		configurationFile.get();
		std::getline(configurationFile, tekInfo.channels.second.power);
		std::getline(configurationFile, tekInfo.channels.second.mainFreq);
		std::getline(configurationFile, tekInfo.channels.second.fskFreq);
		std::getline(configurationFile, tempNote);
		if (tempNote != "END TEKTRONICS1")
		{
			thrower("ERROR: Expected \"TEKTRONICS1\" in configuration file but instead found " + tempNote + "!");
		}
		Master->tektronics1.setSettings(tekInfo);

		std::getline(configurationFile, tempNote);
		if (tempNote != "TEKTRONICS2:")
		{
			thrower("ERROR: Expected \"TEKTRONICS2:\" in configuration file but instead found " + tempNote + "!");
		}
		std::getline(configurationFile, tempNote);
		if (tempNote != "CHANNEL1:")
		{
			thrower("ERROR: Expected \"CHANNEL1:\" in configuration file but instead found " + tempNote + "!");
		}
		configurationFile >> tekInfo.channels.first.on;
		configurationFile >> tekInfo.channels.first.fsk;
		configurationFile.get();
		std::getline(configurationFile, tekInfo.channels.first.power);
		std::getline(configurationFile, tekInfo.channels.first.mainFreq);
		std::getline(configurationFile, tekInfo.channels.first.fskFreq);
		std::getline(configurationFile, tempNote);
		if (tempNote != "CHANNEL2:")
		{
			thrower("ERROR: Expected \"CHANNEL2:\" in configuration file but instead found " + tempNote + "!");
		}
		configurationFile >> tekInfo.channels.second.on;
		configurationFile >> tekInfo.channels.second.fsk;
		configurationFile.get();
		std::getline(configurationFile, tekInfo.channels.second.power);
		std::getline(configurationFile, tekInfo.channels.second.mainFreq);
		std::getline(configurationFile, tekInfo.channels.second.fskFreq);
		std::getline(configurationFile, tempNote);
		if (tempNote != "END TEKTRONICS2")
		{
			thrower("ERROR: Expected \"TEKTRONICS1\" in configuration file but instead found " + tempNote + "!");
		}
		Master->tektronics2.setSettings(tekInfo);
	}
	else
	{
		tektronicsInfo tekInfo;
		tekInfo.channels.first = tekInfo.channels.second = { 0,0,"","","" };
		Master->tektronics1.setSettings(tekInfo);
		Master->tektronics2.setSettings(tekInfo);
	}
	updateConfigurationSavedStatus( true );
	configurationFile.close();
}

/*
]--- This function attempts to save the configuration given the configuration name in the argument. It throws errors and warnings if this is not a "normal" 
]- Save, i.e. if the file doesn't already exist or if the user tries to pass an empty name as an argument. It returns false if the configuration got saved,
]- true if something prevented the configuration from being saved.
*/
void ConfigurationFileSystem::saveConfiguration( MasterWindow* Master )
{
	std::string configurationNameToSave = currentProfileSettings.configuration;
	// check if category has been set yet.
	if (currentProfileSettings.category == "")
	{
		// check if the experiment has also not been set.
		if (currentProfileSettings.experiment == "")
		{
			thrower( "The Experiment and category have not yet been selected! Please select a category or create a new one before trying to save this "
					 "configuration." );
		}
		else
		{
			thrower( "The category has not yet been selected! Please select a category or create a new one before trying to save this configuration." );
		}
	}
	// check to make sure that this is a name.
	if (configurationNameToSave == "")
	{
		thrower( "ERROR: The program requested the saving of the configuration file to an empty name! This shouldn't happen, ask Mark about bugs." );
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
		thrower( "ERROR: unrecognized orientation! Ask Mark about bugs." );
	}

	if (!ConfigurationFileSystem::fileOrFolderExists( currentProfileSettings.pathIncludingCategory + configurationNameToSave + extension ))
	{
		int answer = MessageBox( 0, ("This configuration file appears to not exist in the expected location: " + currentProfileSettings.pathIncludingCategory + configurationNameToSave
									  + extension + ". Continue by making a new configuration file?").c_str(), 0, MB_OKCANCEL );
		if (answer == IDCANCEL)
		{
			return;
		}
	}

	if (!experimentIsSaved)
	{
		int answer = MessageBox( 0, "The Experiment settings have not yet been saved. Save them before the configuration? (Optional)", 0, MB_YESNOCANCEL );
		if (answer == IDCANCEL)
		{
			return;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			saveExperiment( Master );
		}
	}
	if (!categoryIsSaved)
	{
		int answer = MessageBox( 0, "The Category settings have not yet been saved. Save them before the configuration? (Optional)", 0, MB_YESNOCANCEL );
		if (answer == IDCANCEL)
		{
			return;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			saveCategory( Master );
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
		thrower( "ERROR: Unrecognized orientation! Ask Mark about bugs." );
	}
	std::ofstream configurationSaveFile( currentProfileSettings.pathIncludingCategory + configurationNameToSave + extension );
	if (!configurationSaveFile.is_open())
	{
		thrower( "Couldn't save configuration file! Check the name for weird characters, or call Mark about bugs if everything seems right..." );
	}
	// That's the last prompt the user gets, so the save is final now.
	currentProfileSettings.configuration = configurationNameToSave;
	configurationSaveFile << "Master Version: " + str(version) + "\n";
	/// master script
	configurationSaveFile << "MASTER SCRIPT:\n";
	// keep track of whether the script is saved locally or not. This should make renaming things easier. 
	if (Master->masterScript.getScriptPath() == getCurrentPathIncludingCategory())
	{
		configurationSaveFile << "LOCAL " << Master->masterScript.getScriptName() << "\n";
	}
	else
	{
		configurationSaveFile << "NONLOCAL " << Master->masterScript.getScriptPathAndName() << "\n";
	}
	configurationSaveFile << "VARIABLES:\n";
	// Number of Variables
	configurationSaveFile << Master->configVariables.getCurrentNumberOfVariables() << "\n";
	/// Variable Names
	for (int varInc = 0; varInc < Master->configVariables.getCurrentNumberOfVariables(); varInc++)
	{
		variable info = Master->configVariables.getVariableInfo( varInc );
		configurationSaveFile << info.name << " ";
		if (info.timelike)
		{
			configurationSaveFile << "Timelike ";
		}
		else
		{
			configurationSaveFile << "Not_Timelike ";
		}
		if (info.constant)
		{
			configurationSaveFile << "Constant ";
		}
		else
		{
			configurationSaveFile << "Variable ";
		}
		configurationSaveFile << info.ranges.size() << " ";
		for (int rangeInc = 0; rangeInc < info.ranges.size(); rangeInc++)
		{
			configurationSaveFile << info.ranges[rangeInc].initialValue << " ";
			configurationSaveFile << info.ranges[rangeInc].finalValue << " ";
			configurationSaveFile << info.ranges[rangeInc].variations << " ";
			configurationSaveFile << info.ranges[rangeInc].leftInclusive << " ";
			configurationSaveFile << info.ranges[rangeInc].rightInclusive << " ";
		}
		configurationSaveFile << "\n";
	}
	/// ttl settings
	configurationSaveFile << "TTLS:\n";
	for (int ttlRowInc = 0; ttlRowInc < Master->ttlBoard.getNumberOfTTLRows(); ttlRowInc++)
	{
		for (int ttlNumberInc = 0; ttlNumberInc < Master->ttlBoard.getNumberOfTTLsPerRow(); ttlNumberInc++)
		{
			configurationSaveFile << Master->ttlBoard.getTTL_Status( ttlRowInc, ttlNumberInc ) << "\n";
		}
	}
	/// dac settings
	configurationSaveFile << "DACS:\n";
	for (int dacInc = 0; dacInc < Master->dacBoards.getNumberOfDacs(); dacInc++)
	{
		configurationSaveFile << Master->dacBoards.getDacValue( dacInc ) << "\n";
	}
	/// repetitionNumber
	configurationSaveFile << "REPETITIONS:\n";
	configurationSaveFile << Master->repetitionControl.getRepetitionNumber() << "\n";

	configurationSaveFile << Master->topBottomAgilent.getConfigurationString();
	configurationSaveFile << Master->uWaveAxialAgilent.getConfigurationString();
	configurationSaveFile << Master->flashingAgilent.getConfigurationString();

	configurationSaveFile << "NOTES:\n";
	std::string notes = Master->notes.getConfigurationNotes();
	configurationSaveFile << notes + "\n";
	configurationSaveFile << "END CONFIGURATION NOTES" << "\n";

	configurationSaveFile << "TEKTRONICS1:\n";
	configurationSaveFile << "CHANNEL1:\n";
	tektronicsInfo tekInfo = Master->tektronics1.getSettings();
	configurationSaveFile << tekInfo.channels.first.on << "\n" << tekInfo.channels.first.fsk << "\n"
		<< tekInfo.channels.first.power << "\n" << tekInfo.channels.first.mainFreq << "\n"
		<< tekInfo.channels.first.fskFreq << "\n";
	configurationSaveFile << "CHANNEL2:\n";
	configurationSaveFile << tekInfo.channels.second.on << "\n" << tekInfo.channels.second.fsk << "\n"
		<< tekInfo.channels.second.power << "\n" << tekInfo.channels.second.mainFreq << "\n"
		<< tekInfo.channels.second.fskFreq << "\n";
	configurationSaveFile << "END TEKTRONICS1" << "\n";

	configurationSaveFile << "TEKTRONICS2:\n";
	configurationSaveFile << "CHANNEL1:\n";
	tekInfo = Master->tektronics2.getSettings();
	configurationSaveFile << tekInfo.channels.first.on << "\n" << tekInfo.channels.first.fsk << "\n"
		<< tekInfo.channels.first.power << "\n" << tekInfo.channels.first.mainFreq << "\n"
		<< tekInfo.channels.first.fskFreq << "\n";
	configurationSaveFile << "CHANNEL2:\n";
	configurationSaveFile << tekInfo.channels.second.on << "\n" << tekInfo.channels.second.fsk << "\n"
		<< tekInfo.channels.second.power << "\n" << tekInfo.channels.second.mainFreq << "\n"
		<< tekInfo.channels.second.fskFreq << "\n";
	configurationSaveFile << "END TEKTRONICS2" << "\n";
	
	configurationSaveFile.close();
	updateConfigurationSavedStatus( true );
}

/*
]--- Identical to saveConfiguration except that it prompts the user for a name with a dialog box instead of taking one.
*/
void ConfigurationFileSystem::saveConfigurationAs(MasterWindow* Master)
{
	// check if category has been set yet.
	if (currentProfileSettings.category == "")
	{
		// check if the experiment has also not been set.
		if (currentProfileSettings.experiment == "")
		{
			thrower( "The Experiment and category have not yet been selected! Please select a category or create a new one before trying to save this "
					 "configuration." );
		}
		else
		{
			thrower( "The category has not yet been selected! Please select a category or create a new one before trying to save this configuration." );
		}
	}
	std::string configurationNameToSave = (const char*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new configuration name.");
	if (configurationNameToSave == "")
	{
		// canceled
		return;
	}

	// check to make sure that this is a name.
	if (configurationNameToSave == "")
	{
		thrower("ERROR: The program requested the saving of the configuration file to an empty name! This shouldn't happen, ask Mark about bugs.");
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
		thrower( "ERROR: Unrecognized orientation! Ask Mark about bugs." );
	}

	// check if file already exists
	if (ConfigurationFileSystem::fileOrFolderExists( currentProfileSettings.pathIncludingCategory + configurationNameToSave + extension))
	{
		int answer = MessageBox(0, "This configuration file name already exists! Overwrite it?", 0, MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
	}

	if (!experimentIsSaved)
	{
		int answer = MessageBox(0, "The Experiment settings have not yet been saved. Save them before the configuration? (Optional)", 0, MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			saveExperiment(Master);
		}
	}
	if (!categoryIsSaved)
	{
		int answer = MessageBox(0, "The Category settings have not yet been saved. Save them before the configuration? (Optional)", 0, MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			saveCategory(Master);
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
		thrower( "ERROR: Unrecognized orientation! Ask Mark about bugs." );
	}
	std::ofstream configurationSaveFile( currentProfileSettings.pathIncludingCategory + configurationNameToSave + extension);
	if (!configurationSaveFile.is_open())
	{
		thrower( "Couldn't save configuration file! Check the name for weird characters, or call Mark about bugs if everything seems right..." );
	}

	// That's the last prompt the user gets, so the save is final now.
	currentProfileSettings.configuration = configurationNameToSave;
	// Version info tells future code about formatting.
	configurationSaveFile << "Master Version: 1.0\n";
	/// master script
	configurationSaveFile << "MASTER SCRIPT:\n";
	// keep track of whether the script is saved locally or not. This should make renaming things easier. 
	if (Master->masterScript.getScriptPath() == getCurrentPathIncludingCategory())
	{
		configurationSaveFile << "LOCAL " << Master->masterScript.getScriptName() << "\n";
	}
	else
	{
		configurationSaveFile << "NONLOCAL " << Master->masterScript.getScriptPathAndName() << "\n";
	}
	configurationSaveFile << "VARIABLES:\n";
	// Number of Variables
	configurationSaveFile << Master->configVariables.getCurrentNumberOfVariables() << "\n";
	/// Variable Names
	for (int varInc = 0; varInc < Master->configVariables.getCurrentNumberOfVariables(); varInc++)
	{
		variable info = Master->configVariables.getVariableInfo(varInc);
		configurationSaveFile << info.name << " ";
		if (info.timelike)
		{
			configurationSaveFile << "Timelike ";
		}
		else
		{
			configurationSaveFile << "Not_Timelike ";
		}
		if (info.constant)
		{
			configurationSaveFile << "Constant ";
		}
		else
		{
			configurationSaveFile << "Variable ";
		}
		configurationSaveFile << info.ranges.size() + " ";
		for (int rangeInc = 0; rangeInc < info.ranges.size(); rangeInc++)
		{
			configurationSaveFile << info.ranges[rangeInc].initialValue << " ";
			configurationSaveFile << info.ranges[rangeInc].finalValue << " ";
			configurationSaveFile << info.ranges[rangeInc].variations << " ";
			configurationSaveFile << info.ranges[rangeInc].leftInclusive << " ";
			configurationSaveFile << info.ranges[rangeInc].rightInclusive << " ";
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
	for (int dacInc = 0; dacInc < Master->dacBoards.getNumberOfDacs(); dacInc++)
	{
		configurationSaveFile << Master->dacBoards.getDacValue(dacInc) << "\n";
	}
	/// repetitionNumber
	configurationSaveFile << "REPETITIONS:\n";
	configurationSaveFile << Master->repetitionControl.getRepetitionNumber() << "\n";

	configurationSaveFile << "NOTES:\n";
	std::string notes = Master->notes.getConfigurationNotes();
	configurationSaveFile << notes + "\n";
	configurationSaveFile << "END CONFIGURATION NOTES" << "\n";

	configurationSaveFile.close();
	reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.pathIncludingCategory, "*" + extension, currentProfileSettings.configuration);
	updateConfigurationSavedStatus(true);

	configurationSaveFile << "TEKTRONICS1:\n";
	configurationSaveFile << "CHANNEL1:\n";
	tektronicsInfo tekInfo = Master->tektronics1.getSettings();
	configurationSaveFile << tekInfo.channels.first.on << " " << tekInfo.channels.first.fsk << " " 
		<< tekInfo.channels.first.power << " " << tekInfo.channels.first.mainFreq << " " 
		<< tekInfo.channels.first.fskFreq << "\n";
	configurationSaveFile << "CHANNEL2:\n";
	configurationSaveFile << tekInfo.channels.second.on << " " << tekInfo.channels.second.fsk << " "
		<< tekInfo.channels.second.power << " " << tekInfo.channels.second.mainFreq << " "
		<< tekInfo.channels.second.fskFreq << "\n";
	configurationSaveFile << "END TEKTRONICS1" << "\n";

	configurationSaveFile << "TEKTRONICS2:\n";
	configurationSaveFile << "CHANNEL1:\n";
	tekInfo = Master->tektronics2.getSettings();
	configurationSaveFile << tekInfo.channels.first.on << " " << tekInfo.channels.first.fsk << " "
		<< tekInfo.channels.first.power << " " << tekInfo.channels.first.mainFreq << " "
		<< tekInfo.channels.first.fskFreq << "\n";
	configurationSaveFile << "CHANNEL2:\n";
	configurationSaveFile << tekInfo.channels.second.on << " " << tekInfo.channels.second.fsk << " "
		<< tekInfo.channels.second.power << " " << tekInfo.channels.second.mainFreq << " "
		<< tekInfo.channels.second.fskFreq << "\n";
	configurationSaveFile << "END TEKTRONICS2" << "\n";

}


/*
]--- This function renames the currently set configuraiton.
*/
void ConfigurationFileSystem::renameConfiguration(MasterWindow* Master)
{
	// check if configuration has been set yet.
	if (currentProfileSettings.configuration == "")
	{
		if (currentProfileSettings.category == "")
		{
			// check if the experiment has also not been set.
			if (currentProfileSettings.experiment == "")
			{
				thrower( "The Experiment and category have not yet been selected! Please select a category or create a new one before trying to save this "
						 "configuration." );
			}
			else
			{
				thrower( "The category has not yet been selected! Please select a category or create a new one before trying to save this configuration." );
			}
		}
		else
		{
			thrower( "The Configuration has not yet been selected! Please select a category or create a new one before trying to rename it." );
		}
	}
	std::string newConfigurationName = (const char*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new configuration name.");
	if (newConfigurationName == "")
	{
		// canceled
		return;
	}
	std::string currentConfigurationLocation = currentProfileSettings.pathIncludingCategory + currentProfileSettings.configuration;
	std::string newConfigurationLocation = currentProfileSettings.pathIncludingCategory + newConfigurationName;
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
		thrower( "ERROR: Orientation Unrecognized! Ask Mark about bugs." );
	}
	int result = MoveFile(currentConfigurationLocation.c_str(), newConfigurationLocation.c_str());
	if (result == 0)
	{
		thrower( "Renaming of the configuration file Failed! Ask Mark about bugs" );
	}
	currentProfileSettings.configuration = newConfigurationName;
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.pathIncludingCategory, str("*") + HORIZONTAL_EXTENSION, newConfigurationName);
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.pathIncludingCategory, str("*") + VERTICAL_EXTENSION, newConfigurationName);
	}
	else
	{
		thrower( "ERROR: Orientation Unrecognized while reloading config combo! Ask Mark about bugs." );
	}
}

/*
]--- 
*/
void ConfigurationFileSystem::deleteConfiguration()
{
	// check if configuration has been set yet.
	if (currentProfileSettings.configuration == "")
	{
		if (currentProfileSettings.category == "")
		{
			// check if the experiment has also not been set.
			if (currentProfileSettings.experiment == "")
			{
				thrower( "The Experiment and category have not yet been selected! Please select a category or create a"
						" new one before trying to save this configuration." );
			}
			else
			{
				thrower( "The category has not yet been selected! Please select a category or create a new one before"
						" trying to save this configuration." );
			}
		}
		else
		{
			thrower( "The Configuration has not yet been selected! Please select a category or create a new one before"
					" trying to rename it.", 0, 0);
		}
	}
	int answer = MessageBox(0, ("Are you sure you want to delete the current configuration: " + currentProfileSettings.configuration).c_str(), 0, MB_YESNO);
	if (answer == IDNO)
	{
		return;
	}
	std::string currentConfigurationLocation = currentProfileSettings.pathIncludingCategory + currentProfileSettings.configuration;
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
		thrower( "ERROR: Invalid orientation! Ask Mark about bugs." );
	}
	int result = DeleteFile(currentConfigurationLocation.c_str());
	if (result == 0)
	{
		thrower( "ERROR: Deleteing the configuration file failed!" );
	}
	// since the configuration this (may have been) was saved to is gone, no saved version of current code.
	updateConfigurationSavedStatus(false);
	// just deleted the current configuration
	currentProfileSettings.configuration = "";
	// reset combo since the files have now changed after delete
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.pathIncludingCategory, str("*") 
					+ HORIZONTAL_EXTENSION, "__NONE__");
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.pathIncludingCategory, str("*") 
					+ VERTICAL_EXTENSION, "__NONE__");
	}
	else
	{
		thrower( "ERROR: unrecognized orientation while resetting combobox! Ask Mark about bugs." );
	}
}

/*
]--- 
*/
void ConfigurationFileSystem::updateConfigurationSavedStatus(bool isSaved)
{
	configurationIsSaved = isSaved;
	if (isSaved)
	{
		configurationSavedIndicator.SetCheck( 1 );
	}
	else
	{
		configurationSavedIndicator.SetCheck( 0 );
	}
}


void ConfigurationFileSystem::configurationSettingsReadyCheck(MasterWindow* Master)
{
	if (!configurationIsSaved)
	{
		// prompt for save.
		checkConfigurationSave( "There are unsaved configuration settings. Would you like to save the current configuration before starting?", Master );
	}
}


void ConfigurationFileSystem::checkConfigurationSave(std::string prompt, MasterWindow* Master)
{
	int answer = MessageBox(0, prompt.c_str(), 0, MB_YESNOCANCEL);
	if (answer == IDYES)
	{
		saveConfiguration(Master);
	}
	else if (answer == IDCANCEL)
	{
		thrower("Cancel!");
	}
}

void ConfigurationFileSystem::configurationChangeHandler(MasterWindow* Master)
{
	if (!configurationIsSaved)
	{
		//?
		checkConfigurationSave( "The current configuration is unsaved. Save current configuration before changing?", Master );
		// configCombo.SelectString( 0, currentProfileSettings.configuration.c_str() );
	}
	// get current item.
	long long itemIndex = configCombo.GetCurSel();
	if (itemIndex == -1)
	{
		// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
		// is blank. Just break out, this is fine.
		return;
	}
	TCHAR configurationToOpen[256];
	// Send CB_GETLBTEXT message to get the item.
	configCombo.GetLBText( itemIndex, configurationToOpen );
	openConfiguration( configurationToOpen, Master );
	Master->notes.setActiveControls("configuration");
	Master->configVariables.setActive(true);

	// it'd be confusing if these notes stayed here.
}
/// CATEGORY LEVEL HANDLING
/*
]--- This function attempts to save the category given the category name in the argument. It throws errors and warnings if this is not a "normal"
]- Save, i.e. if the file doesn't already exist or if the user tries to pass an empty name as an argument. It returns false if the category got saved,
]- true if something prevented the category from being saved.
*/
void ConfigurationFileSystem::saveCategory(MasterWindow* Master)
{
	std::string categoryNameToSave = currentProfileSettings.category;
	// check if experiment has been set
	if (currentProfileSettings.experiment == "")
	{
		thrower( "The Experiment has not yet been selected! Please select a Experiment or create a new one before trying to save this "
				 "category." );
	}
	// check to make sure that this is a name.
	if (categoryNameToSave == "")
	{
		thrower("ERROR: The program requested the saving of the category file to an empty name! This shouldn't happen, ask Mark about bugs.");
	}

	// check if file already exists. No extension, looking for a folder here. 
	if (!ConfigurationFileSystem::fileOrFolderExists( currentProfileSettings.pathIncludingCategory + categoryNameToSave + CATEGORY_EXTENSION))
	{
		int answer = MessageBox(0, ("This category file appears to not exist in the expected location: " + currentProfileSettings.pathIncludingCategory 
									 + categoryNameToSave + CATEGORY_EXTENSION + ".  Continue by making a new category file?").c_str(), 0, MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
	}

	if (!experimentIsSaved)
	{
		int answer = MessageBox(0, "The Experiment settings have not yet been saved. Save them before the category? (Optional)", 0, MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			saveExperiment( Master );
		}
	}
	std::fstream categoryFileToSave( currentProfileSettings.pathIncludingCategory + categoryNameToSave + CATEGORY_EXTENSION, std::ios::out);
	if (!categoryFileToSave.is_open())
	{
		thrower("ERROR: failed to save category file! Ask mark about bugs.");
	}
	categoryFileToSave << "Version: 1.0\n";
	std::string categoryNotes = Master->notes.getCategoryNotes();
	categoryFileToSave << categoryNotes + "\n";
	categoryFileToSave << "END CATEGORY NOTES\n";
	currentProfileSettings.category = categoryNameToSave;
	currentProfileSettings.pathIncludingCategory = currentProfileSettings.pathIncludingExperiment + categoryNameToSave + "\\";
	updateCategorySavedStatus(true);
}

std::string ConfigurationFileSystem::getCurrentPathIncludingCategory()
{
	return currentProfileSettings.pathIncludingCategory;
}

/*
]--- identical to saveCategory except that 
*/
void ConfigurationFileSystem::saveCategoryAs(MasterWindow* Master)
{
	// check if experiment has been set
	if (currentProfileSettings.experiment == "")
	{
		thrower( "The Experiment has not yet been selected! Please select a Experiment or create a new one before trying to save this "
			"category." );
	}
	std::string categoryNameToSave = (const char*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new configuration name.");
	if (categoryNameToSave == "")
	{
		thrower( "ERROR: The program requested the saving of the category file to an empty name! This shouldn't happen, ask Mark about bugs." );
	}

	// check if file already exists. No extension, looking for a folder here. 
	if (!ConfigurationFileSystem::fileOrFolderExists( currentProfileSettings.pathIncludingCategory + categoryNameToSave + CATEGORY_EXTENSION))
	{
		int answer = MessageBox(0, ("This category file appears to not exist in the expected location: " + currentProfileSettings.pathIncludingCategory + categoryNameToSave
			+ CATEGORY_EXTENSION + ".  Continue by making a new category file?").c_str(), 0, MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
	}

	if (!experimentIsSaved)
	{
		int answer = MessageBox(0, "The Experiment settings have not yet been saved. Save them before the category? (Optional)", 0, MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			saveExperiment( Master );
		}
	}
	// need to make a new folder as well.
	int result = CreateDirectory((currentProfileSettings.pathIncludingExperiment + categoryNameToSave).c_str(), 0);
	if (result == 0)
	{
		thrower("ERROR: failed to create new category directory during category save as! Ask Mark about Bugs.");
	}
	std::fstream categoryFileToSave( currentProfileSettings.pathIncludingCategory + categoryNameToSave + CATEGORY_EXTENSION, std::ios::out);
	if (!categoryFileToSave.is_open())
	{
		thrower("ERROR: failed to save category file! Ask mark about bugs.");
	}
	categoryFileToSave << "Version: 1.0\n";
	std::string categoryNotes = Master->notes.getCategoryNotes();
	categoryFileToSave << categoryNotes + "\n";
	categoryFileToSave << "END CATEGORY NOTES\n";
	currentProfileSettings.category = categoryNameToSave;
	currentProfileSettings.pathIncludingCategory = currentProfileSettings.pathIncludingExperiment + categoryNameToSave + "\\";
	updateCategorySavedStatus(true);
}

/*
]---
*/
void ConfigurationFileSystem::renameCategory()
{
	// TODO: this is a bit more complicated because of the way that all of the configuration fle locations are currently set.
	thrower( "This feature still needs implementing! It doesn't work right now");
}

/*
]---
*/
void ConfigurationFileSystem::deleteCategory()
{
	// check if category has been set yet.
	if (currentProfileSettings.category == "")
	{
		// check if the experiment has also not been set.
		if (currentProfileSettings.experiment == "")
		{
			thrower( "The Experiment and category have not yet been selected! Please select a category or create a new one before trying to save this "
					 "category." );
		}
		else
		{
			thrower( "The category has not yet been selected! Please select a category or create a new one before trying to save this category." );
		}
	}
	int answer = MessageBox(0, ("Are you sure you want to delete the current Category and all configurations within? The current category is: " + currentProfileSettings.category).c_str(), 0, MB_YESNO);
	if (answer == IDNO)
	{
		return;
	}
	answer = MessageBox(0, ("Are you really sure? The current category is: " + currentProfileSettings.category).c_str(), 0, MB_YESNO);
	if (answer == IDNO)
	{
		return;
	}
	std::string currentCategoryLocation = currentProfileSettings.pathIncludingExperiment + currentProfileSettings.category;
	fullyDeleteFolder( currentCategoryLocation );
	updateCategorySavedStatus(false);
	currentProfileSettings.category = "";
	currentProfileSettings.pathIncludingCategory == "";
	reloadCombo(categoryCombo.GetSafeHwnd(), currentProfileSettings.pathIncludingExperiment, "*", "__NONE__");
}

void ConfigurationFileSystem::newCategory(MasterWindow* Master)
{
	// check if experiment has been set
	if (currentProfileSettings.experiment == "")
	{
		thrower("The Experiment has not yet been selected! Please select a Experiment or create a new one before trying to save this category.");
	}
	std::string categoryNameToSave = (const char*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0,
		(DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter name for the new Category.");
	// check to make sure that this is a name.
	if (categoryNameToSave == "")
	{
		// canceled
		return;
	}

	// check if file already exists. No extension, looking for a folder here. 
	if (ConfigurationFileSystem::fileOrFolderExists( currentProfileSettings.pathIncludingExperiment + categoryNameToSave))
	{
		thrower("This category name already exists! If it doesn't appear in the combo, try taking a look at what's in the relvant folder...");
	}
	int result = CreateDirectory((currentProfileSettings.pathIncludingExperiment + categoryNameToSave).c_str(), 0);
	if (result == 0)
	{
		thrower("ERROR: failed to create category directory! Ask Mark about bugs.");
	}
	std::ofstream categorySaveFolder( currentProfileSettings.pathIncludingExperiment + categoryNameToSave + "\\" + categoryNameToSave + CATEGORY_EXTENSION);
	categorySaveFolder.close();
	reloadCombo(categoryCombo.GetSafeHwnd(), currentProfileSettings.pathIncludingExperiment, "*", currentProfileSettings.category);
}



void ConfigurationFileSystem::openCategory(std::string categoryToOpen, MasterWindow* Master)
{
	// this gets called from the file menu.
	// Assign based on the comboBox Item entry.
	std::string path = currentProfileSettings.pathIncludingExperiment + categoryToOpen + "\\" + categoryToOpen + CATEGORY_EXTENSION;
	std::ifstream categoryConfigOpenFile(path.c_str());
	// check if opened correctly.
	if (!categoryConfigOpenFile.is_open())
	{
		thrower("Opening of Category Configuration File Failed!");
	}
	currentProfileSettings.category = categoryToOpen;
	currentProfileSettings.pathIncludingCategory = currentProfileSettings.pathIncludingExperiment + categoryToOpen + "\\";
	/// Set the Configuration combobox.
	// Get all files in the relevant directory.
	std::vector<std::string> configurationNames;

	std::string notes;
	std::string tempNote;
	std::string version;
	std::getline(categoryConfigOpenFile, version);	
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
	updateCategorySavedStatus(true);
}

void ConfigurationFileSystem::updateCategorySavedStatus(bool isSaved)
{
	categoryIsSaved = isSaved;
	if (isSaved)
	{
		categorySavedIndicator.SetCheck( true );
	}
	else
	{
		categorySavedIndicator.SetCheck( false );
	}
}

void ConfigurationFileSystem::categorySettingsReadyCheck(MasterWindow* Master)
{
	if (!categoryIsSaved)
	{
		checkSequenceSave( "There are unsaved category settings. Would you like to save the current category before starting?", 
						  Master );
	}
}

void ConfigurationFileSystem::checkCategorySave(std::string prompt, MasterWindow* Master)
{
	int answer = MessageBox(0, prompt.c_str(), 0, MB_YESNOCANCEL);
	if (answer == IDYES)
	{
		saveCategory(Master);
	}
	else if (answer == IDCANCEL)
	{
		thrower("Cancel!");
	}
}

void ConfigurationFileSystem::categoryChangeHandler(MasterWindow* Master)
{
	if (!categoryIsSaved)
	{
		//???
		checkCategorySave( "The current category is unsaved. Save current category before changing?", Master );
		//		categoryCombo.SelectString( 0, currentProfileSettings.category.c_str() );
	}
	// get current item.
	long long itemIndex = categoryCombo.GetCurSel();
	if (itemIndex == -1)
	{
		// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
		// is blank. Just break out, this is fine.
		return;
	}
	TCHAR categoryConfigToOpen[256];
	// Send CB_GETLBTEXT message to get the item.
	categoryCombo.GetLBText( itemIndex, categoryConfigToOpen );
	openCategory( str( categoryConfigToOpen ), Master );
	// it'd be confusing if these notes stayed here.
	Master->notes.setConfigurationNotes("");
	Master->notes.setActiveControls("category");
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.pathIncludingCategory, str("*") 
					+ HORIZONTAL_EXTENSION, "__NONE__");
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.pathIncludingCategory, str("*") 
					+ VERTICAL_EXTENSION, "__NONE__");
	}
	currentProfileSettings.configuration = "";
	reloadSequence(NULL_SEQUENCE, Master);
	Master->configVariables.setActive(false);
	categoryCombo.EnableWindow();
	orientationCombo.EnableWindow();
	configCombo.EnableWindow();
}

/// EXPERIMENT LEVEL HANDLING
void ConfigurationFileSystem::saveExperiment(MasterWindow* Master)
{
	std::string experimentNameToSave = currentProfileSettings.experiment;
	// check that the experiment name is not empty.
	if (experimentNameToSave == "")
	{
		thrower( "ERROR: Please properly select the experiment or create a new one (\'new experiment\') before trying to save it!" );
	}
	// check if file already exists
	if (!ConfigurationFileSystem::fileOrFolderExists(FILE_SYSTEM_PATH + experimentNameToSave + "\\" + experimentNameToSave + EXPERIMENT_EXTENSION))
	{
		int answer = MessageBox(0, ("This experiment file appears to not exist in the expected location: " 
								+ FILE_SYSTEM_PATH + "   \r\n.Continue by making a new experiment file?").c_str(), 0, MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
	}
	std::ofstream experimentSaveFile(FILE_SYSTEM_PATH + experimentNameToSave + "\\" + experimentNameToSave + EXPERIMENT_EXTENSION);
	if (!experimentSaveFile.is_open())
	{
		thrower( "Couldn't save configuration file! Check the name for weird characters, or call Mark about bugs if everything seems right..." );
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
	currentProfileSettings.pathIncludingExperiment = FILE_SYSTEM_PATH + experimentNameToSave + "\\";
	// update the configuration saved statis for "this" object.
	updateExperimentSavedStatus(true);
	reloadCombo(experimentCombo.GetSafeHwnd(), FILE_SYSTEM_PATH, "*", currentProfileSettings.experiment);
}


void ConfigurationFileSystem::saveExperimentAs(MasterWindow* Master)
{
	if (currentProfileSettings.experiment == "")
	{
		thrower("Please select an experiment before using \"Save As\"");
	}
	std::string experimentNameToSave = (const char*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new experiment name.");
	// check that the experiment name is not empty. 
	if (experimentNameToSave == "")
	{
		// canceled.
		return;
	}

	// check if file already exists
	if (ConfigurationFileSystem::fileOrFolderExists(FILE_SYSTEM_PATH + experimentNameToSave + ".eConfig"))
	{
		int answer = MessageBox(0, ("This experiment name appears to already exist in the expected location: " + FILE_SYSTEM_PATH + "."
			"Overwrite this file?").c_str(), 0, MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
	}

	int result = CreateDirectory((FILE_SYSTEM_PATH + experimentNameToSave).c_str(), 0);
	if (result == 0)
	{
		thrower( "ERROR: failed to create new experiment directory during save as! Ask Mark about bugs." );
	}
	std::ofstream experimentSaveFile(FILE_SYSTEM_PATH + experimentNameToSave + "\\" + experimentNameToSave + ".eConfig");
	if (!experimentSaveFile.is_open())
	{
		thrower( "Couldn't save configuration file! Check the name for weird characters, or call Mark about bugs if everything seems right..." );
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
	currentProfileSettings.pathIncludingExperiment = FILE_SYSTEM_PATH + experimentNameToSave + "\\";
	// update the configuration saved statis for "this" object.
	updateExperimentSavedStatus(true);
	reloadCombo(experimentCombo.GetSafeHwnd(), FILE_SYSTEM_PATH, "*", currentProfileSettings.experiment);
}


void ConfigurationFileSystem::renameExperiment()
{
	thrower("ERROR: This still needs implementing!");
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
	reloadCombo(experimentCombo.GetSafeHwnd(), FILE_SYSTEM_PATH, "*", currentProfileSettings.experiment);
	return false;
	*/
}

void ConfigurationFileSystem::deleteExperiment()
{
	if (currentProfileSettings.experiment == "")
	{
		thrower( "No experiment has been set!" );
	}
	int answer = MessageBox(0, ("Are you sure that you'd like to delete the current experiment and all categories and configurations within? Current Experiment: " + currentProfileSettings.experiment).c_str(), 0, MB_YESNO);
	if (answer == IDNO)
	{
		return;
	}
	answer = MessageBox(0, ("Are you really really sure? Current Experiment: " + currentProfileSettings.experiment).c_str(), 0, MB_YESNO);
	if (answer == IDNO)
	{
		return;
	}
	std::string experimentConfigLocation = FILE_SYSTEM_PATH + currentProfileSettings.experiment + "\\" + currentProfileSettings.experiment + EXPERIMENT_EXTENSION;
	if (DeleteFile(experimentConfigLocation.c_str()))
	{
		thrower( "Deleting .eConfig file failed! Ask Mark about bugs." );
	}
	fullyDeleteFolder( currentProfileSettings.pathIncludingExperiment + currentProfileSettings.experiment );
	reloadCombo(experimentCombo.GetSafeHwnd(), FILE_SYSTEM_PATH, "*", "__NONE__");
	updateExperimentSavedStatus(false);
	currentProfileSettings.experiment = "";
	currentProfileSettings.pathIncludingExperiment = "";
}

void ConfigurationFileSystem::newExperiment(MasterWindow* Master)
{
	std::string newExperimentName = (const char*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0,
																(DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter name for the new Experiment.");
	std::string newExperimentPath = FILE_SYSTEM_PATH + newExperimentName;
	CreateDirectory(newExperimentPath.c_str(), 0);
	std::ofstream newExperimentConfigFile;
	newExperimentConfigFile.open((newExperimentPath + "\\" + newExperimentName + EXPERIMENT_EXTENSION).c_str());
	reloadCombo(experimentCombo.GetSafeHwnd(), FILE_SYSTEM_PATH, "*", currentProfileSettings.experiment);
}

void ConfigurationFileSystem::openExperiment(std::string experimentToOpen, MasterWindow* Master)
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
		thrower( "Opening of Experiment Configuration File Failed!" );
	}
	currentProfileSettings.experiment = experimentToOpen;
	currentProfileSettings.pathIncludingExperiment = FILE_SYSTEM_PATH + experimentToOpen + "\\";
	updateExperimentSavedStatus(true);
	/// Set the Configuration combobox.
	// Get all files in the relevant directory.
	std::vector<std::string> configurationNames;
	//reloadCombo(experimentCombo.GetSafeHwnd(), pathIncludingExperiment, CATEGORY_EXTENSION, "__NONE__");
	std::string version;
	std::getline(experimentConfigOpenFile, version);
	/// Load Values from the experiment config file.
	std::string notes;
	std::string tempNote;
	// get the trailing newline after the >> operation.
	//experimentConfigOpenFile.get();
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
	updateExperimentSavedStatus(true);
}

void ConfigurationFileSystem::updateExperimentSavedStatus(bool isSaved)
{
	experimentIsSaved = isSaved;
	if (isSaved)
	{
		experimentSavedIndicator.SetCheck( true );
	}
	else
	{
		experimentSavedIndicator.SetCheck( false );
	}
}

void ConfigurationFileSystem::experimentSettingsReadyCheck(MasterWindow* Master)
{
	if (!experimentIsSaved)
	{
		checkExperimentSave( "There are unsaved Experiment settings. Would you like to save the current experimnet before starting?", Master );
	}
}

void ConfigurationFileSystem::checkExperimentSave(std::string prompt, MasterWindow* Master)
{
	int answer = MessageBox(0, prompt.c_str(), 0, MB_YESNOCANCEL);
	if (answer == IDYES)
	{
		saveExperiment(Master);
	}
	else if (answer == IDCANCEL)
	{
		thrower("Cancel!");
	}
}

void ConfigurationFileSystem::experimentChangeHandler(MasterWindow* Master)
{
	if (!experimentIsSaved)
	{
		checkExperimentSave( "The current experiment is unsaved. Save Current Experiment before Changing?", Master );
	}
	// get current item.
	long long itemIndex = experimentCombo.GetCurSel();
	if (itemIndex == -1)
	{
		// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
		// is blank. Just break out, this is fine.
		return;
	}
	TCHAR experimentConfigToOpen[256];
	// Send CB_GETLBTEXT message to get the item.
	experimentCombo.GetLBText( itemIndex, experimentConfigToOpen );
	openExperiment( str( experimentConfigToOpen ), Master );
	reloadCombo(categoryCombo.GetSafeHwnd(), currentProfileSettings.pathIncludingExperiment, "*", "__NONE__");
	// it'd be confusing if this category-specific text remained after the category get set to blank.
	Master->notes.setCategoryNotes("");
	Master->notes.setConfigurationNotes("");
	Master->notes.setActiveControls("experiment");
	Master->configVariables.setActive(false);
	experimentCombo.EnableWindow();
	orientationCombo.EnableWindow(false);
	configCombo.EnableWindow(false);
	categoryCombo.EnableWindow();

}

void ConfigurationFileSystem::loadNullSequence(MasterWindow* Master)
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
			thrower( "ERROR: orientation not recognized! Ask Mark about bugs." );
		}
		// change edit
		sequenceInfoDisplay.SetWindowTextA( "Sequence of Configurations to Run:\r\n" );
		appendText("1. " + sequenceConfigurationNames[0] + "\r\n", sequenceInfoDisplay);
	}
	else
	{
		sequenceInfoDisplay.SetWindowTextA( "Sequence of Configurations to Run:\r\n" );
		appendText("No Configuration Loaded\r\n", sequenceInfoDisplay);
	}
	sequenceCombo.SelectString( 0, NULL_SEQUENCE );
	updateSequenceSavedStatus(true);
}

void ConfigurationFileSystem::addToSequence(MasterWindow* Master)
{
	if (currentProfileSettings.configuration == "")
	{
		// nothing to add.
		return;
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
	appendText(str(sequenceConfigurationNames.size()) + ". " + sequenceConfigurationNames.back() + "\r\n",
			   sequenceInfoDisplay);
	updateSequenceSavedStatus(false);
}

/// SEQUENCE HANDLING
void ConfigurationFileSystem::sequenceChangeHandler(MasterWindow* Master)
{
	// get the name
	long long itemIndex = sequenceCombo.GetCurSel();
	TCHAR sequenceName[256];
	sequenceCombo.GetLBText( itemIndex, sequenceName );
	if (itemIndex == -1)
	{
		// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
		// is blank. just break out, this is fine.
		return;
	}
	if (str(sequenceName) == NULL_SEQUENCE)
	{
		loadNullSequence(Master);
		return;
	}
	else
	{
		openSequence(sequenceName, Master);
	}
	// else not null_sequence.
	reloadSequence(currentProfileSettings.sequence, Master);
	updateSequenceSavedStatus(true);
}


void ConfigurationFileSystem::reloadSequence(std::string sequenceToReload, MasterWindow* Master)
{
	reloadCombo(sequenceCombo.GetSafeHwnd(), currentProfileSettings.pathIncludingCategory, str("*") 
				+ SEQUENCE_EXTENSION, sequenceToReload);
	sequenceCombo.AddString( NULL_SEQUENCE );
	if (sequenceToReload == NULL_SEQUENCE)
	{
		loadNullSequence(Master);
	}
}


void ConfigurationFileSystem::saveSequence(MasterWindow* Master)
{
	if (currentProfileSettings.category == "")
	{
		if (currentProfileSettings.experiment == "")
		{
			thrower( "Please set category and experiment before saving sequence." );
		}
		else
		{
			thrower( "Please set category before saving sequence." );
		}
	}
	if (currentProfileSettings.sequence == NULL_SEQUENCE)
	{
		// nothing to save;
		return;
	}
	// if not saved...
	if (currentProfileSettings.sequence == "")
	{
		TCHAR* result = NULL;
		result = (TCHAR*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, textPromptDialogProcedure, (LPARAM)"Please enter a name for this sequence: ");
		if (str(result) == "")
		{
			return;
		}
		currentProfileSettings.sequence = result;
	}
	std::fstream sequenceSaveFile( currentProfileSettings.pathIncludingCategory + "\\" + currentProfileSettings.sequence + SEQUENCE_EXTENSION, std::fstream::out);
	if (!sequenceSaveFile.is_open())
	{
		thrower( "ERROR: Couldn't open sequence file for saving!" );
	}
	sequenceSaveFile << "Version: 1.0\n";
	for (int sequenceInc = 0; sequenceInc < sequenceConfigurationNames.size(); sequenceInc++)
	{
		sequenceSaveFile << sequenceConfigurationNames[sequenceInc] + "\n";
	}
	sequenceSaveFile.close();
	reloadSequence(currentProfileSettings.sequence, Master);
	updateSequenceSavedStatus(true);
}

void ConfigurationFileSystem::saveSequenceAs(MasterWindow* Master)
{
	// prompt for name
	TCHAR* result = NULL;
	result = (TCHAR*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, 
									textPromptDialogProcedure, (LPARAM)"Please Enter a new Sequence Name:");
	//
	if (result == NULL || str(result) == "")
	{
		// user canceled or entered nothing
		return;
	}
	if (str(result) == NULL_SEQUENCE)
	{
		// nothing to save;
		return;
	}
	// if not saved...
	std::fstream sequenceSaveFile( currentProfileSettings.pathIncludingCategory + "\\" + str(result) + SEQUENCE_EXTENSION, std::fstream::out);
	if (!sequenceSaveFile.is_open())
	{
		thrower( "ERROR: Couldn't open sequence file for saving!" );
	}
	currentProfileSettings.sequence = str(result);
	sequenceSaveFile << "Version: 1.0\n";
	for (int sequenceInc = 0; sequenceInc < sequenceConfigurationNames.size(); sequenceInc++)
	{
		sequenceSaveFile << sequenceConfigurationNames[sequenceInc] + "\n";
	}
	sequenceSaveFile.close();
	updateSequenceSavedStatus(true);
}


void ConfigurationFileSystem::renameSequence(MasterWindow* Master)
{
	// check if configuration has been set yet.
	if (currentProfileSettings.sequence == "" || currentProfileSettings.sequence == NULL_SEQUENCE)
	{
		thrower("Please select a sequence for renaming.");
	}
	std::string newSequenceName = (const char*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 
															   0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new configuration name.");
	if (newSequenceName == "")
	{
		// canceled
		return;
	}
	int result = MoveFile((currentProfileSettings.pathIncludingCategory + currentProfileSettings.sequence + SEQUENCE_EXTENSION).c_str(),
						  (currentProfileSettings.pathIncludingCategory + newSequenceName + SEQUENCE_EXTENSION).c_str());
	if (result == 0)
	{
		thrower( "Renaming of the sequence file Failed! Ask Mark about bugs" );
	}
	currentProfileSettings.sequence = newSequenceName;
	reloadSequence(currentProfileSettings.sequence, Master);
	updateSequenceSavedStatus(true);
}

void ConfigurationFileSystem::deleteSequence(MasterWindow* Master)
{
	// check if configuration has been set yet.
	if (currentProfileSettings.sequence == "" || currentProfileSettings.sequence == NULL_SEQUENCE)
	{
		thrower( "Please select a sequence for deleting." );
	}
	int answer = MessageBox(0, ("Are you sure you want to delete the current sequence: " + currentProfileSettings.sequence).c_str(), 0, MB_YESNO);
	if (answer == IDNO)
	{
		return;
	}
	std::string currentSequenceLocation = currentProfileSettings.pathIncludingCategory + currentProfileSettings.sequence + SEQUENCE_EXTENSION;
	int result = DeleteFile(currentSequenceLocation.c_str());
	if (result == 0)
	{
		thrower( "ERROR: Deleteing the configuration file failed!" );
	}
	// since the sequence this (may have been) was saved to is gone, no saved version of current code.
	updateSequenceSavedStatus(false);
	// just deleted the current configuration
	currentProfileSettings.sequence = "";
	// reset combo since the files have now changed after delete
	reloadSequence("__NONE__", Master);
}


void ConfigurationFileSystem::newSequence(MasterWindow* Master)
{
	// prompt for name
	TCHAR* result = NULL;
	result = (TCHAR*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, textPromptDialogProcedure, (LPARAM)"Please Enter a new Sequence Name:");
	//
	if (result == NULL || str(result) == "")
	{
		// user canceled or entered nothing
		return;
	}
	// try to open the file.
	std::fstream sequenceFile( currentProfileSettings.pathIncludingCategory + "\\" + result + SEQUENCE_EXTENSION, std::fstream::out);
	if (!sequenceFile.is_open())
	{
		thrower( "Couldn't create a file with this sequence name! Make sure there are no forbidden characters in your name." );
	}
	std::string newSequenceName = str(result);
	sequenceFile << newSequenceName + "\n";
	// output current configuration
	//eSequenceFileNames.clear();
	if (newSequenceName == "")
	{
		return;
	}
	// reload combo.
	reloadSequence(currentProfileSettings.sequence, Master);
}


void ConfigurationFileSystem::openSequence(std::string sequenceName, MasterWindow* Master)
{
	// try to open the file
	std::fstream sequenceFile( currentProfileSettings.pathIncludingCategory + sequenceName + SEQUENCE_EXTENSION);
	if (!sequenceFile.is_open())
	{
		thrower( "ERROR: sequence file failed to open! Make sure the sequence with address ..." + currentProfileSettings.pathIncludingCategory
				 + sequenceName + SEQUENCE_EXTENSION + " exists." );
	}
	currentProfileSettings.sequence = str(sequenceName);
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
	sequenceInfoDisplay.SetWindowTextA( "Configuration Sequence:\r\n" );
	for (int sequenceInc = 0; sequenceInc < sequenceConfigurationNames.size(); sequenceInc++)
	{
		appendText(str(sequenceInc + 1) + ". " + sequenceConfigurationNames[sequenceInc] + "\r\n",
				   sequenceInfoDisplay);
	}
	updateSequenceSavedStatus(true);
}

void ConfigurationFileSystem::updateSequenceSavedStatus(bool isSaved)
{
	sequenceIsSaved = isSaved;
	if (isSaved)
	{
		sequenceSavedIndicator.SetCheck( true );
	}
	else
	{
		sequenceSavedIndicator.SetCheck( false );
	}
}


void ConfigurationFileSystem::sequenceSettingsReadyCheck(MasterWindow* Master)
{
	if (!sequenceIsSaved)
	{
		checkSequenceSave( "There are unsaved sequence settings. Would you like to save the current sequence before starting?", Master );
	}
}


void ConfigurationFileSystem::checkSequenceSave(std::string prompt, MasterWindow* Master)
{
	int answer = MessageBox(0, prompt.c_str(), 0, MB_YESNOCANCEL);
	if (answer == IDYES)
	{
		saveSequence(Master);
	}
	else if (answer == IDCANCEL)
	{
		thrower("Cancel!");
	}
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
			namesString += "\t" + str(sequenceInc) + ": " + sequenceConfigurationNames[sequenceInc] + "\r\n";
		}
	}
	return namesString;
}


void ConfigurationFileSystem::initialize( POINT& pos, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master, 
										 int& id)
{
	// initialize this.	
	currentProfileSettings.orientation = HORIZONTAL_ORIENTATION;

	// Experiment Type
	experimentLabel.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 20 };
	experimentLabel.Create( "EXPERIMENT", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, experimentLabel.sPos, master, id++ );
	experimentLabel.fontType = HeadingFont;
	// Experiment Saved Indicator
	experimentSavedIndicator.sPos = { pos.x + 360, pos.y, pos.x + 480, pos.y + 20 };
	experimentSavedIndicator.Create( "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT, 
									 experimentSavedIndicator.sPos, master, id++ );
	experimentSavedIndicator.SetCheck( true );
	updateExperimentSavedStatus(true);
	// Category Title
	categoryLabel.sPos = { pos.x + 480, pos.y, pos.x + 960, pos.y + 20 };
	categoryLabel.Create( "CATEGORY", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, categoryLabel.sPos, master, id++ );
	categoryLabel.fontType = HeadingFont;
	//
	categorySavedIndicator.sPos = { pos.x + 480 + 380, pos.y, pos.x + 960, pos.y + 20};
	categorySavedIndicator.Create( "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT, categorySavedIndicator.sPos, master, id++ );
	categorySavedIndicator.SetCheck( true );
	updateCategorySavedStatus(true);
	pos.y += 20;
	// Experiment Combo
	experimentCombo.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 800 };
	experimentCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 
						   experimentCombo.sPos, master, id++);
	idVerify(experimentCombo, EXPERIMENT_COMBO_ID);
	reloadCombo(experimentCombo.GetSafeHwnd(), PROFILES_PATH, str("*"), "__NONE__");
	// Category Combo
	categoryCombo.sPos = { pos.x + 480, pos.y, pos.x + 960, pos.y + 800 };
	categoryCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 
						 categoryCombo.sPos, master, id++ );
	idVerify(categoryCombo, CATEGORY_COMBO_ID);
	pos.y += 25;
	// Orientation Title
	orientationLabel.sPos = { pos.x, pos.y, pos.x + 120, pos.y + 20 };
	orientationLabel.Create( "ORIENTATION", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, orientationLabel.sPos, 
							master, id++);
	orientationLabel.fontType = HeadingFont;
	// Configuration Title
	configLabel.sPos = { pos.x + 120, pos.y, pos.x + 960, pos.y + 20 };
	configLabel.Create( "CONFIGURATION", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, configLabel.sPos, master, id++);
	configLabel.fontType = HeadingFont;
	// Configuration Saved Indicator
	configurationSavedIndicator.sPos = { pos.x + 860, pos.y, pos.x + 960, pos.y + 20 };
	configurationSavedIndicator.Create( "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT, 
									   configurationSavedIndicator.sPos, master, id++ );
	configurationSavedIndicator.SetCheck( true );
	updateConfigurationSavedStatus(true);
	pos.y += 20;
	// orientation combo
	std::vector<std::string> orientationNames;
	orientationNames.push_back("Horizontal");
	orientationNames.push_back("Vertical");
	orientationCombo.sPos = { pos.x, pos.y, pos.x + 120, pos.y + 800 };
	orientationCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 
							orientationCombo.sPos, master, id++ );
	idVerify(orientationCombo, ORIENTATION_COMBO_ID);
	for (int comboInc = 0; comboInc < orientationNames.size(); comboInc++)
	{
		orientationCombo.AddString( orientationNames[comboInc].c_str() );
	}
	orientationCombo.SetCurSel(0);
	// configuration combo
	configCombo.sPos = { pos.x + 120, pos.y, pos.x + 960, pos.y + 800 };	
	configCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, configCombo.sPos, 
					   master, id++ );
	idVerify(configCombo, CONFIGURATION_COMBO_ID);
	pos.y += 25;
	/// SEQUENCE
	sequenceLabel.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 20 };
	sequenceLabel.Create( "SEQUENCE", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, sequenceLabel.sPos, master, 
						  id++ );
	sequenceLabel.fontType = HeadingFont;
	
	sequenceSavedIndicator.sPos = { pos.x + 360, pos.y, pos.x + 480, pos.y += 20 };
	sequenceSavedIndicator.Create( "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT, 
								  sequenceSavedIndicator.sPos, master, id++ );
	sequenceSavedIndicator.SetCheck( true );
	updateSequenceSavedStatus( true );

	// combo
	sequenceCombo.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 800 };
	sequenceCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 
						  sequenceCombo.sPos, master, id++ );
	idVerify(sequenceCombo, SEQUENCE_COMBO_ID); 
	sequenceCombo.SetCurSel( 0 );
	sequenceCombo.AddString( NULL_SEQUENCE );
	pos.y += 25;
	// display
	sequenceInfoDisplay.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 50 };
	sequenceInfoDisplay.Create( ES_READONLY | WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL,
								sequenceInfoDisplay.sPos, master, id++ );
	sequenceInfoDisplay.SetWindowTextA( "Sequence of Configurations to Run:\r\n" );
	
	configCombo.EnableWindow(false);
	categoryCombo.EnableWindow(false);
	orientationCombo.EnableWindow(false);

}


void ConfigurationFileSystem::reorganizeControls(RECT parentRectangle, std::string mode)
{
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
					if (str(fd.cFileName) != "." && str(fd.cFileName) != "..")
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
		if (extensions == "*" || extensions == "*.*" || extensions == str("*") + HORIZONTAL_EXTENSION 
			|| extensions == str("*") + VERTICAL_EXTENSION || extensions == str("*") + SEQUENCE_EXTENSION
			|| extensions == str("*") + CATEGORY_EXTENSION || extensions == str("*") + EXPERIMENT_EXTENSION)
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


void ConfigurationFileSystem::reloadCombo(HWND comboToReload, std::string locationToLook, std::string extension, std::string nameToLoad)
{
	std::vector<std::string> names;
	// search for folders
	names = searchForFiles(locationToLook, extension);

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
}


bool ConfigurationFileSystem::fileOrFolderExists(std::string filePathway)
{
	
	FILE *file;
	fopen_s( &file, filePathway.c_str(), "r" );
	if ( !file )
	{		
		return false;
	}
	else
	{
		fclose( file );
		return true;
	}
	// got this from stack exchange. dunno how it works but it should be fast.
	// doesn't work on xp???
	// struct stat buffer;
	// return (stat(filePathway.c_str(), &buffer) == 0);
}


std::string ConfigurationFileSystem::getComboText()
{
	int selectionNum = configCombo.GetCurSel();
	if (selectionNum == -1)
	{
		return "";
	}
	else
	{
		TCHAR text[256];
		configCombo.GetLBText( selectionNum, text );
		return text;
	}
}


void ConfigurationFileSystem::fullyDeleteFolder(std::string folderToDelete)
{
	// this used to call SHFileOperation. Boost is better. Much better. 
	int filesRemoved = boost::filesystem::remove_all(folderToDelete.c_str());
	if (filesRemoved == 0)
	{
		thrower( "Delete Failed! Ask mark about bugs." );
	}
}


INT_PTR ConfigurationFileSystem::handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, brushMap brushes)
{
	DWORD controlID = GetDlgCtrlID((HWND)lParam);
	HDC hdcStatic = (HDC)wParam;
	if (controlID == categoryCombo.GetDlgCtrlID() || controlID == experimentCombo.GetDlgCtrlID() 
		|| controlID == configCombo.GetDlgCtrlID() || controlID == orientationCombo.GetDlgCtrlID() 
		|| controlID == sequenceCombo.GetDlgCtrlID())
	{
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		SetBkColor(hdcStatic, RGB(15, 15, 15));
		return (LRESULT)brushes["Dark Grey"];
	}
	else if (controlID == categoryLabel.GetDlgCtrlID() || controlID == experimentLabel.GetDlgCtrlID() 
			 || controlID == configLabel.GetDlgCtrlID() || controlID == orientationLabel.GetDlgCtrlID() 
			 || controlID == sequenceInfoDisplay.GetDlgCtrlID() || controlID == sequenceLabel.GetDlgCtrlID()
		|| controlID == experimentSavedIndicator.GetDlgCtrlID() || controlID == configurationSavedIndicator.GetDlgCtrlID()
			 || controlID == categorySavedIndicator.GetDlgCtrlID())
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



