#include "stdafx.h"
#include "ConfigurationFileSystem.h"
#include "Windows.h"
#include <fstream>
#include "externals.h"
#include "constants.h"
#include "Resource.h"
#include "Commctrl.h"
#include "externals.h"
#include "textPromptDialogProcedure.h"
#include "fonts.h"
#include <boost/filesystem.hpp>
#include "NiawgController.h"

ConfigurationFileSystem::ConfigurationFileSystem(std::string fileSystemPath)
{
	FILE_SYSTEM_PATH = fileSystemPath;
	currentProfileSettings.orientation = HORIZONTAL_ORIENTATION;
}

// just looks at the info in a file and loads it into references, doesn't change anything in the gui or main settings.
void ConfigurationFileSystem::getConfigInfo( niawgPair<std::vector<std::fstream>>& scriptFiles, std::vector<std::fstream>& intensityScriptFiles,
											 profileSettings profile, std::vector<variable> singletons, std::vector<variable> variables,
											 bool programIntensity )
{
	scriptFiles[Vertical].resize( profile.sequenceConfigNames.size() );
	scriptFiles[Horizontal].resize( profile.sequenceConfigNames.size() );
	intensityScriptFiles.resize( profile.sequenceConfigNames.size() );
	/// gather information from every configuration in the sequence. /////////////////////////////////////////////////////////////////////
	for (int sequenceInc = 0; sequenceInc < profile.sequenceConfigNames.size(); sequenceInc++)
	{
		// open configuration file
		std::fstream configFile( profile.categoryPath + "\\" + profile.sequenceConfigNames[sequenceInc] );
		std::string intensityScriptAddress, version;
		niawgPair<std::string> niawgScriptAddresses;
		// first get version info:
		std::getline( configFile, version );
		/// load files
		for (auto axis : AXES)
		{
			getline( configFile, niawgScriptAddresses[axis] );
			scriptFiles[axis][sequenceInc].open( niawgScriptAddresses[axis] );
			if (!scriptFiles[axis][sequenceInc].is_open())
			{
				thrower( "ERROR: Failed to open vertical script file named: " + niawgScriptAddresses[axis]
						 + " found in configuration: " + profile.sequenceConfigNames[sequenceInc] + "\r\n" );
			}
		}
		/// load intensity file
		getline(configFile, intensityScriptAddress);
		if (programIntensity)
		{
			intensityScriptFiles[sequenceInc].open(intensityScriptAddress);
			if (!intensityScriptFiles[sequenceInc].is_open())
			{
				thrower("ERROR: Failed to open intensity script file named: " + intensityScriptAddress + " found in configuration: "
					+ profile.sequenceConfigNames[sequenceInc] + "\r\n");
			}
		}
		/// load variables
		int varNum;
		configFile >> varNum;
		// early version didn't have variable type indicators.
		if (version == "Version: 1.0")
		{
			for (int varInc = 0; varInc < varNum; varInc++)
			{
				std::string varName;
				configFile >> varName;
				variable tempVariable;
				tempVariable.name = varName;
				// assume certain things for old files. E.g. singletons didn't exist. 
				tempVariable.singleton = false;
				tempVariable.timelike = false;
				tempVariable.value = 0;
				bool alreadyExists = false;
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					if (tempVariable.name == variables[varInc].name)
					{
						alreadyExists = true;
						break;
					}
				}
				if (!alreadyExists)
				{
					// add new varying parameters.
					variables.push_back( tempVariable );
				}
			}
		}
		else if (version == "Version: 1.1")
		{
			for (int varInc = 0; varInc < varNum; varInc++)
			{
				variable tempVar;
				std::string varName, timelikeText, typeText, valueString;
				bool timelike;
				bool singleton;
				double value;
				configFile >> varName;
				configFile >> timelikeText;
				configFile >> typeText;
				configFile >> valueString;
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
					thrower( "ERROR: unknown timelike option. Check the formatting of the configuration file." );
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
					thrower( "ERROR: unknown variable type option. Check the formatting of the configuration file." );
				}
				try
				{
					value = std::stod( valueString );
				}
				catch (std::invalid_argument&)
				{
					thrower( "ERROR: Failed to convert value in configuration file for variable's double value. Value was: " + valueString );
				}
				tempVar.name = varName;
				tempVar.timelike = timelike;
				tempVar.singleton = singleton;
				tempVar.value = value;

				if (tempVar.singleton)
				{
					// handle singletons
					// check if it already has been loaded
					bool alreadyExists = false;
					for (int varInc = 0; varInc < singletons.size(); varInc++)
					{
						if (tempVar.name == singletons[varInc].name)
						{
							alreadyExists = true;
							break;
						}
					}
					if (!alreadyExists)
					{
						// load new singleton
						singletons.push_back( tempVar );
					}
				}
				else
				{
					// handle varying parameters
					bool alreadyExists = false;
					for (int varInc = 0; varInc < variables.size(); varInc++)
					{
						if (tempVar.name == variables[varInc].name)
						{
							alreadyExists = true;
							break;
						}
					}
					if (!alreadyExists)
					{
						// add new varying parameters.
						variables.push_back( tempVar );
					}
				}
			}
		}
		else
		{
			thrower( "ERROR: Unrecognized configuration version! Ask Mark about bugs." );
		}
	}
}

void ConfigurationFileSystem::saveEntireProfile(ScriptingWindow* scriptWindow, MainWindow* comm)
{
	saveExperimentOnly( comm );
	saveCategoryOnly( comm );
	saveConfigurationOnly( scriptWindow, comm );
	saveSequence();		
}


void ConfigurationFileSystem::checkSaveEntireProfile(ScriptingWindow* scriptWindow, MainWindow* comm)
{
	checkExperimentSave( "Save Experiment Settings?", comm );
	checkCategorySave( "Save Category Settings?", comm );
	checkConfigurationSave( "Save Configuration Settings?", scriptWindow, comm );
	checkSequenceSave( "Save Sequence Settings?" );
}


void ConfigurationFileSystem::allSettingsReadyCheck(ScriptingWindow* scriptWindow, MainWindow* comm)
{
	// check all components of this class.
	experimentSettingsReadyCheck( comm );
	categorySettinsReadyCheck();
	configurationSettingsReadyCheck( scriptWindow, comm );
	sequenceSettingsReadyCheck();
	// passed all checks.
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


void ConfigurationFileSystem::orientationChangeHandler(MainWindow* mainWin)
{
	Communicator* comm = mainWin->getComm();
	profileSettings profile = mainWin->getCurentProfileSettings();
	long long itemIndex = orientationCombo.GetCurSel();
	TCHAR orientation[256];
	orientationCombo.GetLBText(itemIndex, orientation);
	// reset some things.
	currentProfileSettings.orientation = std::string(orientation);
	if (currentProfileSettings.category != "")
	{
		if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
		{
			reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.categoryPath, 
						 std::string("*") + HORIZONTAL_EXTENSION, "__NONE__");
		}
		else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
		{
			reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.categoryPath, 
						 std::string("*") + VERTICAL_EXTENSION, "__NONE__");
		}
	}
	mainWin->setNotes("configuration", "");
	currentProfileSettings.configuration = "";
	/// Load the relevant NIAWG script.
	mainWin->restartNiawgDefaults();
}

/// CONFIGURATION LEVEL HANDLING

void ConfigurationFileSystem::newConfiguration(MainWindow* comm)
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
			thrower("The category has not yet been selected! Please select a category or create a new one before trying to save this configuration.");
		}
	}

	std::string configurationNameToSave = (const char*)DialogBoxParam(eGlobalInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new configuration name.");
	if (configurationNameToSave == "")
	{
		// canceled
		return;
	}
	std::string newConfigurationPath = currentProfileSettings.categoryPath + configurationNameToSave;

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
		thrower( "ERROR: Unrecognized orientation! Ask Mark about bugs." );
	}
	std::ofstream newConfigurationFile(newConfigurationPath.c_str());
	if (!newConfigurationFile.is_open())
	{
		thrower( "ERROR: Failed to create new configuration file. Ask Mark about bugs." );
	}
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.categoryPath, std::string("*") + HORIZONTAL_EXTENSION, currentProfileSettings.configuration.c_str());
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.categoryPath, std::string("*") + VERTICAL_EXTENSION, currentProfileSettings.configuration.c_str());
	}
	else
	{
		thrower( "ERROR: Unrecognized orientation! Ask Mark about bugs." );
	}
}


/*
]--- This function opens a given configuration file, sets all of the relevant parameters, and loads the associated scripts. 
*/
void ConfigurationFileSystem::openConfiguration(std::string configurationNameToOpen, ScriptingWindow* scriptWindow, MainWindow* comm)
{
	// no folder associated with configuraitons. They share the category folder.
	std::string path = currentProfileSettings.categoryPath + configurationNameToOpen;
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
		thrower("Opening of Configuration File Failed!");
	}
	currentProfileSettings.configuration = configurationNameToOpen;
	
	// not currenlty used because version 1.0.
	std::string version;
	std::getline(configurationFile, version);
	// Open Vertical Script
	std::string tempVerticalName, tempHorizontalName, tempIntensityName;
	getline(configurationFile, tempVerticalName);
	getline(configurationFile, tempHorizontalName);
	getline(configurationFile, tempIntensityName);

	scriptWindow->openVerticalScript(tempVerticalName);
	scriptWindow->openHorizontalScript(tempHorizontalName);
	scriptWindow->openIntensityScript(tempIntensityName);

	scriptWindow->considerScriptLocations();

	/// Get Variables
	// Number of Variables
	comm->clearVariables();
	int varNum;
	configurationFile >> varNum;
	if (varNum < 0 || varNum > 10)
	{
		int answer = MessageBox(0, ("ERROR: variable number retrieved from file appears suspicious. The number is " 
									 + std::to_string(varNum) + ". Is this accurate?").c_str(), 0, MB_YESNO);
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
			std::transform(varName.begin(), varName.end(), varName.begin(), ::tolower);
			comm->addVariable(varName, false, false, 0, varInc);
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
			std::transform(varName.begin(), varName.end(), varName.begin(), ::tolower);
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
				thrower("ERROR: unknown timelike option. Check the formatting of the configuration file.");
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
				thrower( "ERROR: unknown variable type option. Check the formatting of the configuration file." );
			}
			try
			{
				value = std::stod(valueString);
			}
			catch (std::invalid_argument&)
			{
				thrower("ERROR: Failed to convert value in configuration file for variable's double value. Value was: " + valueString);
			}
			comm->addVariable(varName, timelike, singleton, value, varInc);
		}
	}
	else if (version == "")
	{
		// nothing
	}
	else
	{
		thrower("ERROR: Unrecognized configuration version! Ask Mark about bugs.");
	}
	// add a blank line
	comm->addVariable("", false, false, 0, varNum);
	updateConfigurationSavedStatus(true);
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
		comm->setNotes("configuration", notes);
	}
	else
	{
		comm->setNotes("configuration", "");
	}
	updateConfigurationSavedStatus(true);
	// actually set this now
	scriptWindow->updateProfile(currentProfileSettings.category + "->" + currentProfileSettings.configuration);
	// close.
	configurationFile.close();
	if (currentProfileSettings.sequence == NULL_SEQUENCE)
	{
		// reload it.
		loadNullSequence();
	}
}

/*
]--- This function attempts to save the configuration given the configuration name in the argument. It throws errors and warnings if this 
]- is not a Normal Save, i.e. if the file doesn't already exist or if the user tries to pass an empty name as an argument. It returns 
]- false if the configuration got saved, true if something prevented the configuration from being saved.
*/
void ConfigurationFileSystem::saveConfigurationOnly(ScriptingWindow* scriptWindow, MainWindow* comm)
{
	std::string configurationNameToSave = currentProfileSettings.configuration;
	// check if category has been set yet.
	if (currentProfileSettings.category == "")
	{
		// check if the experiment has also not been set.
		if (currentProfileSettings.experiment == "")
		{
			thrower( "The Experiment and category have not yet been selected! Please select a category or create a new one before trying "
					 "to save this configuration." );
		}
		else
		{
			thrower("The category has not yet been selected! Please select a category or create a new one before trying to save this "
					 "configuration.");
		}
	}
	// check to make sure that this is a name.
	if (configurationNameToSave == "")
	{
		thrower( "ERROR: The program requested the saving of the configuration file to an empty name! This shouldn't happen, ask Mark "
				 "about bugs." );
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

	if (!ConfigurationFileSystem::fileOrFolderExists(currentProfileSettings.categoryPath + configurationNameToSave + extension))  
	{
		int answer = MessageBox(0, ("This configuration file appears to not exist in the expected location: " 
									 + currentProfileSettings.categoryPath + configurationNameToSave 
									 + extension + ". Continue by making a new configuration file?").c_str(), 0, MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
	}
	
	if (!experimentIsSaved)
	{
		int answer = MessageBox(0, "The Experiment settings have not yet been saved. Save them before the "
								 "configuration? (Optional)", 0, MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			saveExperimentOnly(comm);
		}
	}
	if (!categoryIsSaved)
	{
		int answer = MessageBox(0, "The Category settings have not yet been saved. Save them before the configuration?"
								 " (Optional)", 0, MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			saveCategoryOnly(comm);
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
	std::ofstream configurationSaveFile(currentProfileSettings.categoryPath + configurationNameToSave + extension);
	if (!configurationSaveFile.is_open())
	{
		thrower( "Couldn't save configuration file! Check the name for weird characters, or call Mark about bugs if everything seems right..." );
	}
	// That's the last prompt the user gets, so the save is final now.
	currentProfileSettings.configuration = configurationNameToSave;
	configurationSaveFile << "Version: 1.1\n";
	scriptInfo<std::string> addresses = scriptWindow->getScriptAddresses();
	// order matters!
	// vertical Script File Address
	configurationSaveFile << addresses.verticalNIAWG << "\n";
	// horizontal Script File Address
	configurationSaveFile << addresses.horizontalNIAWG << "\n";
	// Intensity Script File Address
	configurationSaveFile << addresses.intensityAgilent << "\n";
	// Number of Variables
	std::vector<variable> vars = comm->getAllVariables();
	configurationSaveFile << vars.size() << "\n";
	// Variable Names
	// This part changed in version 1.1.
	for (int varInc = 0; varInc < vars.size(); varInc++)
	{
		variable info = vars[varInc];
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
		configurationSaveFile << info.value << "\n";
	}
	
	std::string notes = comm->getNotes("configuration");
	configurationSaveFile << notes + "\n";
	configurationSaveFile << "END CONFIGURATION NOTES" << "\n";
	configurationSaveFile.close();
	updateConfigurationSavedStatus(true);
}

/*
]--- Identical to saveConfigurationOnly except that it prompts the user for a name with a dialog box instead of taking one.
*/
void ConfigurationFileSystem::saveConfigurationAs(ScriptingWindow* scriptWindow, MainWindow* comm)
{
	// check if category has been set yet.
	if (currentProfileSettings.category == "")
	{
		// check if the experiment has also not been set.
		if (currentProfileSettings.experiment == "")
		{
			thrower( "The Experiment and category have not yet been selected! Please select a category or create a new one before trying to save this configuration.");
		}
		else
		{
			thrower( "The category has not yet been selected! Please select a category or create a new one before trying to save this configuration.");
		}
	}
	std::string configurationNameToSave = (const char*)DialogBoxParam(eGlobalInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new configuration name.");
	if (configurationNameToSave == "")
	{
		// canceled
		return;
	}

	// check to make sure that this is a name.
	if (configurationNameToSave == "")
	{
		thrower( "ERROR: The program requested the saving of the configuration file to an empty name! This shouldn't happen, ask Mark about bugs." );
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
	if (ConfigurationFileSystem::fileOrFolderExists(currentProfileSettings.categoryPath + configurationNameToSave + extension))
	{
		int answer = MessageBox(0, "This configuration file name already exists! Overwrite it?", 0, MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
	}

	if (!experimentIsSaved)
	{
		int answer = MessageBox(0, "The Experiment settings have not yet been saved. Save them before the configuration? (Optional)", 0, 
								 MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			saveExperimentOnly(comm);
		}
	}
	if (!categoryIsSaved)
	{
		int answer = MessageBox(0, "The Category settings have not yet been saved. Save them before the configuration? (Optional)", 0, 
								 MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			saveCategoryOnly(comm);
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
	std::ofstream configurationSaveFile(currentProfileSettings.categoryPath + configurationNameToSave + extension);
	if (!configurationSaveFile.is_open())
	{
		thrower( "Couldn't save configuration file! Check the name for weird characters, or call Mark about bugs if everything seems right..." );
	}

	// That's the last prompt the user gets, so the save is final now.
	currentProfileSettings.configuration = configurationNameToSave;
	// Version info tells future code about formatting.
	configurationSaveFile << "Version: 1.1\n";
	scriptInfo<std::string> addresses = scriptWindow->getScriptAddresses();
	// order matters!
	// vertical Script File Address
	configurationSaveFile << addresses.verticalNIAWG << "\n";
	// horizontal Script File Address
	configurationSaveFile << addresses.horizontalNIAWG << "\n";
	// Intensity Script File Address
	configurationSaveFile << addresses.intensityAgilent << "\n";
	// Number of Variables
	std::vector<variable> vars = comm->getAllVariables();
	configurationSaveFile << vars.size() << "\n";
	// Variable Names
	// This part changed in version 1.1.
	for (int varInc = 0; varInc < vars.size(); varInc++)
	{
		variable info = vars[varInc];
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
		configurationSaveFile << info.value << "\n";
	}
	std::string notes = comm->getNotes("configuration");
	configurationSaveFile << notes + "\n";
	configurationSaveFile << "END CONFIGURATION NOTES" << "\n";
	configurationSaveFile.close();
	reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.categoryPath, "*" + extension, currentProfileSettings.configuration);
	updateConfigurationSavedStatus(true);
}


/*
]--- This function renames the currently set 
*/
void ConfigurationFileSystem::renameConfiguration()
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
					"configuration.");
			}
			else
			{
				thrower( "The category has not yet been selected! Please select a category or create a new one before trying to save this configuration.");
			}
		}
		else
		{
			thrower( "The Configuration has not yet been selected! Please select a category or create a new one before trying to rename it." );
		}
	}
	std::string newConfigurationName = (const char*)DialogBoxParam(eGlobalInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new configuration name.");
	if (newConfigurationName == "")
	{
		// canceled
		return;
	}
	std::string currentConfigurationLocation = currentProfileSettings.categoryPath + currentProfileSettings.configuration;
	std::string newConfigurationLocation = currentProfileSettings.categoryPath + newConfigurationName;
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
		reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.categoryPath, std::string("*") + HORIZONTAL_EXTENSION, "__NONE__");
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.categoryPath, std::string("*") + VERTICAL_EXTENSION, "__NONE__");
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
	int answer = MessageBox(0, ("Are you sure you want to delete the current configuration: " 
								 + currentProfileSettings.configuration).c_str(), 0, MB_YESNO);
	if (answer == IDNO)
	{
		return;
	}
	std::string currentConfigurationLocation = currentProfileSettings.categoryPath + currentProfileSettings.configuration;
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
	this->updateConfigurationSavedStatus(false);
	// just deleted the current configuration
	currentProfileSettings.configuration = "";
	// reset combo since the files have now changed after delete
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.categoryPath, std::string("*") + HORIZONTAL_EXTENSION, "__NONE__");
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.categoryPath, std::string("*") + VERTICAL_EXTENSION, "__NONE__");
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
		configurationSavedIndicator.SetCheck(BST_CHECKED);
	}
	else
	{
		configurationSavedIndicator.SetCheck(BST_UNCHECKED);
	}
}


bool ConfigurationFileSystem::configurationSettingsReadyCheck(ScriptingWindow* scriptWindow, MainWindow* comm)
{
	// prompt for save.
	if (checkConfigurationSave("There are unsaved configuration settings. Would you like to save the current configuration before starting?", scriptWindow, comm))
	{
		// canceled
		return true;
	}
	return false;
}

bool ConfigurationFileSystem::checkConfigurationSave(std::string prompt, ScriptingWindow* scriptWindow, MainWindow* comm)
{
	if (!this->configurationIsSaved)
	{
		int answer = MessageBox(0, prompt.c_str(), 0, MB_YESNOCANCEL);
		if (answer == IDYES)
		{
			this->saveConfigurationOnly(scriptWindow, comm);
		}
		else if (answer == IDCANCEL)
		{
			return true;
		}
	}
	return false;
}

void ConfigurationFileSystem::configurationChangeHandler(ScriptingWindow* scriptWindow, MainWindow* comm)
{
	if (!configurationIsSaved)
	{
		if (checkConfigurationSave("The current configuration is unsaved. Save current configuration before changing?", scriptWindow, comm))
		{
			configCombo.SelectString(0, currentProfileSettings.configuration.c_str());
			return;
		}
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
	configCombo.GetLBText(itemIndex, configurationToOpen);
	openConfiguration( configurationToOpen, scriptWindow, comm );
	// it'd be confusing if these notes stayed here.
}


/// CATEGORY LEVEL HANDLING
/*
]--- This function attempts to save the category given the category name in the argument. It throws errors and warnings if this is not a Normal
]- Save, i.e. if the file doesn't already exist or if the user tries to pass an empty name as an argument. It returns false if the category got saved,
]- true if something prevented the category from being saved.
*/
void ConfigurationFileSystem::saveCategoryOnly(MainWindow* comm)
{
	std::string categoryNameToSave = currentProfileSettings.category;
	// check if experiment has been set
	if (currentProfileSettings.experiment == "")
	{
		thrower("The Experiment has not yet been selected! Please select a Experiment or create a new one before trying to save this "
			"category." );
	}
	// check to make sure that this is a name.
	if (categoryNameToSave == "")
	{
		thrower( "ERROR: The program requested the saving of the category file to an empty name! This shouldn't happen, ask Mark about bugs." );
	}

	// check if file already exists. No extension, looking for a folder here. 
	if (!ConfigurationFileSystem::fileOrFolderExists(currentProfileSettings.categoryPath + categoryNameToSave + CATEGORY_EXTENSION))
	{
		int answer = MessageBox( 0, ("This category file appears to not exist in the expected location: " 
									  + currentProfileSettings.categoryPath + categoryNameToSave
									  + CATEGORY_EXTENSION + ".  Continue by making a new category file?").c_str(), 0, MB_OKCANCEL );
		if (answer == IDCANCEL)
		{
			return;
		}
	}

	if (!experimentIsSaved)
	{
		int answer = MessageBox(0, "The Experiment settings have not yet been saved. Save them before the category? (Optional)", 0, 
								 MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
		else if (answer == IDYES)
		{
			saveExperimentOnly( comm );
		}
	}
	std::fstream categoryFileToSave(currentProfileSettings.categoryPath + categoryNameToSave + CATEGORY_EXTENSION, std::ios::out);
	if (!categoryFileToSave.is_open())
	{
		thrower( "ERROR: failed to save category file! Ask mark about bugs." );
	}
	categoryFileToSave << "Version: 1.0\n";
	std::string categoryNotes = comm->getNotes("category");
	categoryFileToSave << categoryNotes + "\n";
	categoryFileToSave << "END CATEGORY NOTES\n";
	currentProfileSettings.category = categoryNameToSave;
	currentProfileSettings.categoryPath = currentProfileSettings.experimentPath + categoryNameToSave + "\\";
	updateCategorySavedStatus(true);
}


std::string ConfigurationFileSystem::getCurrentPathIncludingCategory()
{
	return currentProfileSettings.categoryPath;
}


/*
]--- identical to saveCategoryOnly except that 
*/
void ConfigurationFileSystem::saveCategoryAs( MainWindow* comm )
{
	// check if experiment has been set
	if (currentProfileSettings.experiment == "")
	{
		thrower( "The Experiment has not yet been selected! Please select a Experiment or create a new one before trying to save this "
				 "category." );
	}
	std::string categoryNameToSave = (const char*)DialogBoxParam( eGlobalInstance, MAKEINTRESOURCE( IDD_TEXT_PROMPT_DIALOG ), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new configuration name." );
	if (categoryNameToSave == "")
	{
		thrower( "ERROR: The program requested the saving of the category file to an empty name! This shouldn't happen, ask Mark about bugs." );
	}

	// check if file already exists. No extension, looking for a folder here. 
	if (!ConfigurationFileSystem::fileOrFolderExists( currentProfileSettings.categoryPath + categoryNameToSave + CATEGORY_EXTENSION ))
	{
		int answer = MessageBox( 0, ("This category file appears to not exist in the expected location: " 
									  + currentProfileSettings.categoryPath + categoryNameToSave
									  + CATEGORY_EXTENSION + ".  Continue by making a new category file?").c_str(), 0, MB_OKCANCEL );
		if (answer == IDCANCEL)
		{
			return;
		}
	}

	if (!experimentIsSaved)
	{
		int answer = MessageBox( 0, "The Experiment settings have not yet been saved. Save them before the category? (Optional)", 0, 
								 MB_YESNOCANCEL );
		if (answer == IDCANCEL)
		{
			return;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			saveExperimentOnly( comm );
		}
	}
	// need to make a new folder as well.
	int result = CreateDirectory( (currentProfileSettings.experimentPath + categoryNameToSave).c_str(), 0 );
	if (result == 0)
	{
		thrower( "ERROR: failed to create new category directory during category save as! Ask Mark about Bugs." );
	}
	std::fstream categoryFileToSave( currentProfileSettings.categoryPath + categoryNameToSave + CATEGORY_EXTENSION, std::ios::out );
	if (!categoryFileToSave.is_open())
	{
		thrower( "ERROR: failed to save category file! Ask mark about bugs." );
	}
	categoryFileToSave << "Version: 1.0\n";

	std::string categoryNotes = comm->getNotes( "category" );
	categoryFileToSave << categoryNotes + "\n";
	categoryFileToSave << "END CATEGORY NOTES\n";
	currentProfileSettings.category = categoryNameToSave;
	currentProfileSettings.categoryPath = currentProfileSettings.experimentPath + categoryNameToSave + "\\";
	updateCategorySavedStatus( true );
}


/*
]---
*/
void ConfigurationFileSystem::renameCategory()
{
	// TODO: this is a bit more complicated because of the way that all of the configuration fle locations are currently set.
	thrower("This feature still needs implementing! It doesn't work right now");
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
			thrower( "The Experiment and category have not yet been selected! Please select a category or create a new one before trying "
					 "to save this category." );
		}
		else
		{
			thrower( "The category has not yet been selected! Please select a category or create a new one before trying to save this "
					 "category." );
		}
	}
	int answer = MessageBox(0, ("Are you sure you want to delete the current Category and all configurations within? The current category "
								 "is: " + currentProfileSettings.category).c_str(), 0, MB_YESNO);
	if (answer == IDNO)
	{
		return;
	}
	answer = MessageBox(0, ("Are you really sure? The current category is: " + currentProfileSettings.category).c_str(), 0, MB_YESNO);
	if (answer == IDNO)
	{
		return;
	}
	std::string currentCategoryLocation = currentProfileSettings.experimentPath + currentProfileSettings.category;
	fullyDeleteFolder( currentCategoryLocation );
	updateCategorySavedStatus(false);
	currentProfileSettings.category = "";
	currentProfileSettings.categoryPath == "";
	reloadCombo(categoryCombo.GetSafeHwnd(), currentProfileSettings.experimentPath, "*", "__NONE__");
}


void ConfigurationFileSystem::newCategory()
{
	// check if experiment has been set
	if (currentProfileSettings.experiment == "")
	{
		thrower( "The Experiment has not yet been selected! Please select a Experiment or create a new one before trying to save this "
				 "category." );
	}
	std::string categoryNameToSave = (const char*)DialogBoxParam( eGlobalInstance, MAKEINTRESOURCE( IDD_TEXT_PROMPT_DIALOG ), 0,
		(DLGPROC) textPromptDialogProcedure, (LPARAM)"Please enter name for the new Category." );
	// check to make sure that this is a name.
	if (categoryNameToSave == "")
	{
		// canceled
		return;
	}

	// check if file already exists. No extension, looking for a folder here. 
	if (ConfigurationFileSystem::fileOrFolderExists(currentProfileSettings.experimentPath + categoryNameToSave))
	{
		thrower( "This category name already exists! If it doesn't appear in the combo, try taking a look at what's in the relvant folder..." );
	}
	int result = CreateDirectory((currentProfileSettings.experimentPath + categoryNameToSave).c_str(), 0);
	if (result == 0)
	{
		thrower("ERROR: failed to create category directory! Ask Mark about bugs.");
	}
	std::ofstream categorySaveFolder(currentProfileSettings.experimentPath + categoryNameToSave + "\\" + categoryNameToSave + CATEGORY_EXTENSION);
	categorySaveFolder.close();
	reloadCombo(categoryCombo.GetSafeHwnd(), currentProfileSettings.experimentPath, "*", currentProfileSettings.category);
}


void ConfigurationFileSystem::openCategory(std::string categoryToOpen, ScriptingWindow* scriptWindow, MainWindow* comm)
{
	// this gets called from the file menu.
	// Assign based on the comboBox Item entry.
	std::string path = currentProfileSettings.experimentPath + categoryToOpen + "\\" + categoryToOpen + CATEGORY_EXTENSION;
	std::ifstream categoryConfigOpenFile(path.c_str());
	// check if opened correctly.
	if (!categoryConfigOpenFile.is_open())
	{
		thrower( "Opening of Category Configuration File Failed!" );
	}
	currentProfileSettings.category = categoryToOpen;
	currentProfileSettings.categoryPath = currentProfileSettings.experimentPath + categoryToOpen + "\\";
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
		comm->setNotes("category", notes);
	}
	else
	{
		comm->setNotes("category", "");
	}
	scriptWindow->updateProfile("");
	// close.
	categoryConfigOpenFile.close();
	updateCategorySavedStatus(true);
}


void ConfigurationFileSystem::updateCategorySavedStatus(bool isSaved)
{
	categoryIsSaved = isSaved;
	if (isSaved)
	{
		categorySavedIndicator.SetCheck(BST_CHECKED);
	}
	else
	{
		categorySavedIndicator.SetCheck(BST_UNCHECKED);
	}
}


bool ConfigurationFileSystem::categorySettinsReadyCheck()
{
	if (!categoryIsSaved)
	{
		if (checkSequenceSave( "There are unsaved category settings. Would you like to save the current category before starting?" ))
		{
			// canceled
			return true;
		}
	}
	return false;
}

bool ConfigurationFileSystem::checkCategorySave(std::string prompt, MainWindow* comm)
{
	if (!categoryIsSaved)
	{
		int answer = MessageBox(0, prompt.c_str(), 0, MB_YESNOCANCEL);
		if (answer == IDYES)
		{
			this->saveCategoryOnly(comm);
		}
		else if (answer == IDCANCEL)
		{
			return true;
		}
	}
	return false;
}

void ConfigurationFileSystem::categoryChangeHandler(ScriptingWindow* scriptWindow, MainWindow* comm)
{
	if (!categoryIsSaved)
	{
		if (checkCategorySave("The current category is unsaved. Save current category before changing?", comm))
		{
			categoryCombo.SelectString(0, currentProfileSettings.category.c_str());
			return;
		}
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
	categoryCombo.GetLBText(itemIndex, categoryConfigToOpen);
	openCategory( std::string( categoryConfigToOpen ), scriptWindow, comm );

	// it'd be confusing if these notes stayed here.
	comm->setNotes("configuration", "");
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.categoryPath, 
					 std::string("*") + HORIZONTAL_EXTENSION, "__NONE__");
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		reloadCombo(configCombo.GetSafeHwnd(), currentProfileSettings.categoryPath, 
					 std::string("*") + VERTICAL_EXTENSION, "__NONE__");
	}
	currentProfileSettings.configuration = "";
	reloadSequence(NULL_SEQUENCE);
}


/// EXPERIMENT LEVEL HANDLING
void ConfigurationFileSystem::saveExperimentOnly(MainWindow* comm)
{
	std::string experimentNameToSave = currentProfileSettings.experiment;
	// check that the experiment name is not empty.
	if (experimentNameToSave == "")
	{
		thrower( "ERROR: Please properly select the experiment or create a new one (\'new experiment\') before trying to save it!" );
	}
	// check if file already exists
	if (!ConfigurationFileSystem::fileOrFolderExists(FILE_SYSTEM_PATH + experimentNameToSave + "\\" + experimentNameToSave 
													  + EXPERIMENT_EXTENSION))
	{
		int answer = MessageBox(0, ("This experiment file appears to not exist in the expected location: " + FILE_SYSTEM_PATH + "   \r\n."
									 "Continue by making a new experiment file?").c_str(), 0, MB_OKCANCEL);
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

	debugInfo options = comm->getDebuggingOptions();
	mainOptions settings = comm->getMainOptions();
	/// Start Outputting information
	/// THIS IS EXPERIMENT SAVED STUFF. T>T
	// this can be checked by reading functions to see what format to expect. From now on, a version will always be outputted at the beginning of the file.
	// (05/29/2016)
	experimentSaveFile << "Version: 1.2\n";
	// NOTE: Dummy variables used to be outputted here. 
	// NOTE: accumulations used to be outputted here.
	// get var files from master option
	experimentSaveFile << settings.getVariables << "\n";
	// output waveform read progress option
	experimentSaveFile << options.showReadProgress << "\n";
	// output waveform write progress option
	experimentSaveFile << options.showWriteProgress << "\n";
	// connect to master option
	experimentSaveFile << settings.connectToMaster << "\n";
	// output correction waveform time option
	experimentSaveFile << options.showCorrectionTimes << "\n";
	// Output intensity programming option.
	experimentSaveFile << settings.programIntensity << "\n";
	// Output more run info option.
	experimentSaveFile << options.outputExcessInfo << "\n";
	// notes.

	std::string notes = comm->getNotes("experiment");
	experimentSaveFile << notes << "\n";
	experimentSaveFile << "END EXPERIMENT NOTES" << "\n";
	// And done.
	experimentSaveFile.close();
	currentProfileSettings.experiment = experimentNameToSave;
	// update the save path. 
	currentProfileSettings.experimentPath = FILE_SYSTEM_PATH + experimentNameToSave + "\\";
	// update the configuration saved statis for "this" object.
	updateExperimentSavedStatus(true);
	reloadCombo(experimentCombo.GetSafeHwnd(), FILE_SYSTEM_PATH, "*", currentProfileSettings.experiment);
}


void ConfigurationFileSystem::saveExperimentAs(MainWindow* comm)
{
	if (currentProfileSettings.experiment == "")
	{
		thrower("Please select an experiment before using \"Save As\"");
	}
	std::string experimentNameToSave = (const char*)DialogBoxParam(eGlobalInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new experiment name.");
	// check that the experiment name is not empty. 
	if (experimentNameToSave == "")
	{
		// canceled.
		return;
	}

	// check if file already exists
	if (ConfigurationFileSystem::fileOrFolderExists( FILE_SYSTEM_PATH + experimentNameToSave + ".eConfig" ))
	{
		int answer = MessageBox( 0, ("This experiment name appears to already exist in the expected location: " + FILE_SYSTEM_PATH + "."
									  "Overwrite this file?").c_str(), 0, MB_OKCANCEL );
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
		thrower( "Couldn't save configuration file! Check the name for weird characters, or call Mark about bugs if everything seems "
				 "right..." );
	}
	// That's the last prompt the user gets, so the save is final now.
	currentProfileSettings.experiment = experimentNameToSave;

	debugInfo options = comm->getDebuggingOptions();
	mainOptions settings = comm->getMainOptions();
	/// Start Outputting information
	/// THIS IS EXPERIMENT SAVED STUFF. T>T
	// this can be checked by reading functions to see what format to expect. From now on, a version will always be outputted at the beginning of the file.
	// (05/29/2016)
	experimentSaveFile << "Version: 1.1\n";
	// NOTE: Dummy variables used to be outputted here. 
	// Note: accumulations used to be outputted here.
	// get var files from master option
	experimentSaveFile << settings.getVariables << "\n";
	// output waveform read progress option
	experimentSaveFile << options.showReadProgress << "\n";
	// output waveform write progress option
	experimentSaveFile << options.showWriteProgress << "\n";
	// connect to master option
	experimentSaveFile << settings.connectToMaster << "\n";
	// output correction waveform time option
	experimentSaveFile << options.showCorrectionTimes << "\n";
	// Output intensity programming option.
	experimentSaveFile << settings.programIntensity << "\n";
	// Output more run info option.
	experimentSaveFile << options.outputExcessInfo << "\n";
	// notes.
	std::string notes = comm->getNotes("experiment");
	experimentSaveFile << notes << "\n";
	experimentSaveFile << "END EXPERIMENT NOTES" << "\n";
	// And done.
	experimentSaveFile.close();
	// update the save path. 
	currentProfileSettings.experimentPath = FILE_SYSTEM_PATH + experimentNameToSave + "\\";
	// update the configuration saved statis for "this" object.
	updateExperimentSavedStatus(true);
	reloadCombo(experimentCombo.GetSafeHwnd(), FILE_SYSTEM_PATH, "*", currentProfileSettings.experiment);
}


void ConfigurationFileSystem::renameExperiment(MainWindow* comm)
{
	// check if saved
	if (!experimentIsSaved)
	{
		if (checkExperimentSave("Save experiment before renaming it?", comm))
		{
			return;
		}
	}
	// get name
	std::string experimentNameToSave = (const char*)DialogBoxParam(eGlobalInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, 
										(DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new experiment name.");
	// check if file already exists. No extension, looking for a folder here. 
	if (ConfigurationFileSystem::fileOrFolderExists(FILE_SYSTEM_PATH + experimentNameToSave))
	{
		int answer = MessageBox(0, "This experiment name already exists!", 0, MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
	}
	std::string newExperimentConfigLocation = FILE_SYSTEM_PATH + experimentNameToSave + "\\" + experimentNameToSave + EXPERIMENT_EXTENSION;
	std::string currentExperimentConfigLocation = FILE_SYSTEM_PATH + experimentNameToSave + "\\" + currentProfileSettings.experiment + EXPERIMENT_EXTENSION;
	int result = MoveFile((FILE_SYSTEM_PATH + currentProfileSettings.experiment).c_str(), (FILE_SYSTEM_PATH + experimentNameToSave).c_str());
	if (result == 0)
	{
		thrower( "ERROR: Moving the experiment folder failed!" );
	}
	result = MoveFile(currentExperimentConfigLocation.c_str(), newExperimentConfigLocation.c_str());
	if (result == 0)
	{
		thrower( "Moving the experiment folder failed!" );
	}
	// TODO: program the code to go through all of the category and configuration file names and change addresses, or change format of how these are referenced 
	// in configuraiton file.
	currentProfileSettings.experiment = experimentNameToSave;
	reloadCombo(experimentCombo.GetSafeHwnd(), FILE_SYSTEM_PATH, "*", currentProfileSettings.experiment);
}

void ConfigurationFileSystem::deleteExperiment()
{
	if (currentProfileSettings.experiment == "")
	{
		thrower( "No experiment has been set!" );
		return;
	}
	int answer = MessageBox( 0, ("Are you sure that you'd like to delete the current experiment and all categories and configurations "
								  "within? Current Experiment: " + currentProfileSettings.experiment).c_str(), 0, MB_YESNO );
	if (answer == IDNO)
	{
		return;
	}
	answer = MessageBox( 0, ("Are you really really sure? Current Experiment: " + currentProfileSettings.experiment).c_str(), 0, MB_YESNO );
	if (answer == IDNO)
	{
		return;
	}
	std::string experimentConfigLocation = FILE_SYSTEM_PATH + currentProfileSettings.experiment + "\\" + currentProfileSettings.experiment + EXPERIMENT_EXTENSION;
	if (DeleteFile( experimentConfigLocation.c_str() ))
	{
		thrower( "Deleting .eConfig file failed! Ask Mark about bugs." );
	}
	fullyDeleteFolder( currentProfileSettings.experimentPath + currentProfileSettings.experiment );
	reloadCombo( experimentCombo.GetSafeHwnd(), FILE_SYSTEM_PATH, "*", "__NONE__" );
	updateExperimentSavedStatus( false );
	currentProfileSettings.experiment = "";
	currentProfileSettings.experimentPath = "";
	return;
}


void ConfigurationFileSystem::newExperiment()
{
	std::string newExperimentName = (const char*)DialogBoxParam(eGlobalInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0,
																(DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter name for the new Experiment.");
	std::string newExperimentPath = FILE_SYSTEM_PATH + newExperimentName;
	CreateDirectory(newExperimentPath.c_str(), 0);
	std::ofstream newExperimentConfigFile;
	newExperimentConfigFile.open((newExperimentPath + "\\" + newExperimentName + EXPERIMENT_EXTENSION).c_str());
	reloadCombo(experimentCombo.GetSafeHwnd(), FILE_SYSTEM_PATH, "*", currentProfileSettings.experiment);
}

void ConfigurationFileSystem::openExperiment(std::string experimentToOpen, ScriptingWindow* scriptWindow, MainWindow* comm)
{
	// this gets called from the file menu.
	// Assign based on the comboBox Item entry.
	std::string path = FILE_SYSTEM_PATH + experimentToOpen + "\\" + experimentToOpen + EXPERIMENT_EXTENSION;
	std::ifstream experimentConfigOpenFile(path.c_str());
	// check if opened correctly.
	if (!experimentConfigOpenFile.is_open())
	{
		thrower( "Opening of Experiment Configuration File Failed!" );
	}
	currentProfileSettings.experiment = experimentToOpen;
	currentProfileSettings.experimentPath = FILE_SYSTEM_PATH + experimentToOpen + "\\";
	reloadCombo(categoryCombo.GetSafeHwnd(), currentProfileSettings.experimentPath, "*", "__NONE__");
	// since no category is currently loaded...
	SendMessage(configCombo.GetSafeHwnd(), CB_RESETCONTENT, 0, 0);
	currentProfileSettings.category = "";
	currentProfileSettings.configuration = "";
	// it'd be confusing if this category-specific text remained after the category get set to blank.
	comm->setNotes("category", "");
	comm->setNotes("configuration", "");

	// no category saved currently.
	currentProfileSettings.categoryPath = currentProfileSettings.experimentPath;
	updateExperimentSavedStatus(true);
	/// Set the Configuration combobox.
	// Get all files in the relevant directory.
	std::vector<std::string> configurationNames;
	//this->reloadCombo(experimentCombo.hwnd, experimentPath, CATEGORY_EXTENSION, "__NONE__");
	std::string version;
	std::getline(experimentConfigOpenFile, version);/// Load Values from the experiment config file.
	// Accumulations Number
	if (version == "Version: 1.0" || version == "Version: 1.1")
	{
		std::string trash;
		experimentConfigOpenFile >> trash;
	}
	mainOptions settings = comm->getMainOptions();
	// get var files from master option
	experimentConfigOpenFile >> settings.getVariables;
	debugInfo options = comm->getDebuggingOptions();
	// output waveform read progress option
	experimentConfigOpenFile >> options.showReadProgress;
	// output waveform write progress option
	experimentConfigOpenFile >> options.showWriteProgress;
	if (version == "Version: 1.0")
	{
		// log current script option
		std::string garbage;
		experimentConfigOpenFile >> garbage;
	}
	// connect to master option
	experimentConfigOpenFile >> settings.getVariables;
	// output correction waveform time option
	experimentConfigOpenFile >> options.showCorrectionTimes;
	// program the agilent intensity functino generator option
	experimentConfigOpenFile >> settings.programIntensity;
	experimentConfigOpenFile >> options.outputExcessInfo;
	comm->setMainOptions(settings);
	comm->setDebuggingOptions(options);
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
		comm->setNotes("experiment", notes);
	}
	else
	{
		comm->setNotes("experiment", "");
	}
	scriptWindow->updateProfile("");
	// close.
	experimentConfigOpenFile.close();
	updateExperimentSavedStatus(true);
}


void ConfigurationFileSystem::updateExperimentSavedStatus(bool isSaved)
{
	experimentIsSaved = isSaved;
	if (isSaved)
	{
		experimentSavedIndicator.SetCheck(BST_CHECKED);
	}
	else
	{
		experimentSavedIndicator.SetCheck(BST_UNCHECKED);
	}
}


void ConfigurationFileSystem::experimentSettingsReadyCheck(MainWindow* comm)
{
	if (!experimentIsSaved)
	{
		if (checkExperimentSave("There are unsaved Experiment settings. Would you like to save the current experimnet before starting?", comm))
		{
			// canceled
			return; // ???
		}
	}
	return;
}


bool ConfigurationFileSystem::checkExperimentSave(std::string prompt, MainWindow* comm)
{
	if (!this->experimentIsSaved)
	{
		int answer = MessageBox(0, prompt.c_str(), 0, MB_YESNOCANCEL);
		if (answer == IDYES)
		{
			saveExperimentOnly(comm);
		}
		else if (answer == IDCANCEL)
		{
			return true;
		}
	}
	return false;
}


void ConfigurationFileSystem::experimentChangeHandler(ScriptingWindow* scriptWindow, MainWindow* comm)
{
	if (!experimentIsSaved)
	{
		if (checkExperimentSave("The current experiment is unsaved. Save Current Experiment before Changing?", comm))
		{
			return;
		}
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
	experimentCombo.GetLBText(itemIndex, experimentConfigToOpen);
	openExperiment( std::string( experimentConfigToOpen ), scriptWindow, comm );
	reloadSequence(NULL_SEQUENCE);
}


void ConfigurationFileSystem::loadNullSequence()
{
	currentProfileSettings.sequence = NULL_SEQUENCE;
	// only current configuration loaded
	currentProfileSettings.sequenceConfigNames.clear();
	if (currentProfileSettings.configuration != "")
	{
		if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
		{
			currentProfileSettings.sequenceConfigNames.push_back(currentProfileSettings.configuration + HORIZONTAL_EXTENSION);
		}
		else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
		{
			currentProfileSettings.sequenceConfigNames.push_back(currentProfileSettings.configuration + VERTICAL_EXTENSION);
		}
		else
		{
			thrower( "ERROR: orientation not recognized! Ask Mark about bugs." );
		}
		// change edit
		sequenceInfoDisplay.SetWindowTextA("Sequence of Configurations to Run:\r\n");
		appendText(("1. " + this->currentProfileSettings.sequenceConfigNames[0] + "\r\n"), sequenceInfoDisplay);
	}
	else
	{
		sequenceInfoDisplay.SetWindowTextA("Sequence of Configurations to Run:\r\n");
		appendText("No Configuration Loaded\r\n", sequenceInfoDisplay);
	}
	sequenceCombo.SelectString(0, NULL_SEQUENCE);
	updateSequenceSavedStatus(true);
}


void ConfigurationFileSystem::addToSequence(CWnd* parent)
{
	if (currentProfileSettings.configuration == "")
	{
		// nothing to add.
		return;
	}
	if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
	{
		currentProfileSettings.sequenceConfigNames.push_back(currentProfileSettings.configuration + HORIZONTAL_EXTENSION);
	}
	else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
	{
		currentProfileSettings.sequenceConfigNames.push_back(currentProfileSettings.configuration + VERTICAL_EXTENSION);
	}
	// add text to display.
	appendText( std::to_string( currentProfileSettings.sequenceConfigNames.size() ) + ". "
				+ currentProfileSettings.sequenceConfigNames.back() + "\r\n", sequenceInfoDisplay );
	updateSequenceSavedStatus(false);
}

/// SEQUENCE HANDLING
void ConfigurationFileSystem::sequenceChangeHandler()
{
	// get the name
	long long itemIndex = sequenceCombo.GetCurSel(); 
	TCHAR sequenceName[256];
	sequenceCombo.GetLBText(itemIndex, sequenceName);
	if (itemIndex == -1)
	{
		// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
		// is blank. just break out, this is fine.
		return;
	}
	if (std::string(sequenceName) == NULL_SEQUENCE)
	{
		loadNullSequence();
		return;
	}
	else
	{
		openSequence(sequenceName);
	}
	// else not null_sequence.
	reloadSequence(currentProfileSettings.sequence);
	updateSequenceSavedStatus(true);
}


void ConfigurationFileSystem::reloadSequence(std::string sequenceToReload)
{
	reloadCombo(sequenceCombo.GetSafeHwnd(), currentProfileSettings.categoryPath, std::string("*") + SEQUENCE_EXTENSION, sequenceToReload);
	sequenceCombo.AddString(NULL_SEQUENCE);
	if (sequenceToReload == NULL_SEQUENCE)
	{
		loadNullSequence();
	}
}


void ConfigurationFileSystem::saveSequence()
{
	if (currentProfileSettings.category == "")
	{
		if (currentProfileSettings.experiment == "")
		{
			thrower("Please set category and experiment before saving sequence.");
		}
		else
		{
			thrower("Please set category before saving sequence.");
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
		result = (TCHAR*)DialogBoxParam(eGlobalInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, textPromptDialogProcedure, (LPARAM)"Please enter a name for this sequence: ");
		if (std::string(result) == "")
		{
			return;
		}
		currentProfileSettings.sequence = result;
	}
	std::fstream sequenceSaveFile(currentProfileSettings.categoryPath + "\\" + currentProfileSettings.sequence + SEQUENCE_EXTENSION, std::fstream::out);
	if (!sequenceSaveFile.is_open())
	{
		thrower( "ERROR: Couldn't open sequence file for saving!" );
	}
	sequenceSaveFile << "Version: 1.0\n";
	for (int sequenceInc = 0; sequenceInc < this->currentProfileSettings.sequenceConfigNames.size(); sequenceInc++)
	{
		sequenceSaveFile << this->currentProfileSettings.sequenceConfigNames[sequenceInc] + "\n";
	}
	sequenceSaveFile.close();
	reloadSequence(currentProfileSettings.sequence);
	updateSequenceSavedStatus(true);
}


void ConfigurationFileSystem::saveSequenceAs()
{
	// prompt for name
	TCHAR* result = NULL;
	result = (TCHAR*)DialogBoxParam(eGlobalInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, textPromptDialogProcedure, (LPARAM)"Please Enter a new Sequence Name:");
	//
	if (result == NULL || std::string(result) == "")
	{
		// user canceled or entered nothing
		return;
	}
	if (std::string(result) == NULL_SEQUENCE)
	{
		// nothing to save;
		return;
	}
	// if not saved...
	std::fstream sequenceSaveFile(currentProfileSettings.categoryPath + "\\" + std::string(result) + SEQUENCE_EXTENSION, std::fstream::out);
	if (!sequenceSaveFile.is_open())
	{
		thrower( "ERROR: Couldn't open sequence file for saving!" );
	}
	currentProfileSettings.sequence = std::string(result);
	sequenceSaveFile << "Version: 1.0\n";
	for (int sequenceInc = 0; sequenceInc < this->currentProfileSettings.sequenceConfigNames.size(); sequenceInc++)
	{
		sequenceSaveFile << this->currentProfileSettings.sequenceConfigNames[sequenceInc] + "\n";
	}
	sequenceSaveFile.close();
	updateSequenceSavedStatus(true);
}


void ConfigurationFileSystem::renameSequence()
{
	// check if configuration has been set yet.
	if (currentProfileSettings.sequence == "" || currentProfileSettings.sequence == NULL_SEQUENCE)
	{
		thrower( "Please select a sequence for renaming." );
	}
	std::string newSequenceName = (const char*)DialogBoxParam( eGlobalInstance, MAKEINTRESOURCE( IDD_TEXT_PROMPT_DIALOG ), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new configuration name." );
	if (newSequenceName == "")
	{
		// canceled
		return;
	}
	int result = MoveFile( (currentProfileSettings.categoryPath + currentProfileSettings.sequence + SEQUENCE_EXTENSION).c_str(),
		(currentProfileSettings.categoryPath + newSequenceName + SEQUENCE_EXTENSION).c_str() );
	if (result == 0)
	{
		thrower( "Renaming of the sequence file Failed! Ask Mark about bugs" );
	}
	currentProfileSettings.sequence = newSequenceName;
	reloadSequence( currentProfileSettings.sequence );
	updateSequenceSavedStatus( true );
}


void ConfigurationFileSystem::deleteSequence()
{
	// check if configuration has been set yet.
	if (currentProfileSettings.sequence == "" || currentProfileSettings.sequence == NULL_SEQUENCE)
	{
		thrower("Please select a sequence for deleting.");
	}
	int answer = MessageBox(0, ("Are you sure you want to delete the current sequence: " + currentProfileSettings.sequence).c_str(), 0, 
							 MB_YESNO);
	if (answer == IDNO)
	{
		return;
	}
	std::string currentSequenceLocation = currentProfileSettings.categoryPath + currentProfileSettings.sequence + SEQUENCE_EXTENSION;
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
	reloadSequence("__NONE__");
}


void ConfigurationFileSystem::newSequence(CWnd* parent)
{
	// prompt for name
	TCHAR* result = NULL;
	result = (TCHAR*)DialogBoxParam(eGlobalInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), parent->GetSafeHwnd(), textPromptDialogProcedure, (LPARAM)"Please Enter a new Sequence Name:");
	//
	if (result == NULL || std::string(result) == "")
	{
		// user canceled or entered nothing
		return;
	}
	// try to open the file.
	std::fstream sequenceFile(currentProfileSettings.categoryPath + "\\" + result + SEQUENCE_EXTENSION, std::fstream::out);
	if (!sequenceFile.is_open())
	{
		thrower( "Couldn't create a file with this sequence name! Make sure there are no forbidden characters in your name." );
	}
	std::string newSequenceName = std::string(result);
	sequenceFile << newSequenceName + "\n";
	// output current configuration
	//eSequenceFileNames.clear();
	if (newSequenceName == "")
	{
		return;
	}
	// reload combo.
	reloadSequence(currentProfileSettings.sequence);
}


void ConfigurationFileSystem::openSequence(std::string sequenceName)
{
	// try to open the file
	std::fstream sequenceFile(currentProfileSettings.categoryPath + sequenceName + SEQUENCE_EXTENSION);
	if (!sequenceFile.is_open())
	{
		thrower("ERROR: sequence file failed to open! Make sure the sequence with address ..." 
				 + currentProfileSettings.categoryPath + sequenceName + SEQUENCE_EXTENSION + " exists.");
	}
	currentProfileSettings.sequence = std::string(sequenceName);
	// read the file
	std::string version;
	std::getline(sequenceFile, version);
	//
	currentProfileSettings.sequenceConfigNames.clear();
	std::string tempName;
	getline(sequenceFile, tempName);
	while (sequenceFile)
	{
		currentProfileSettings.sequenceConfigNames.push_back(tempName);
		getline(sequenceFile, tempName);
	}
	// update the edit
	sequenceInfoDisplay.SetWindowTextA("Configuration Sequence:\r\n");
	for (int sequenceInc = 0; sequenceInc < currentProfileSettings.sequenceConfigNames.size(); sequenceInc++)
	{
		appendText( std::to_string( sequenceInc + 1 ) + ". " + currentProfileSettings.sequenceConfigNames[sequenceInc] + "\r\n",
					sequenceInfoDisplay );
	}
	updateSequenceSavedStatus(true);
}


void ConfigurationFileSystem::updateSequenceSavedStatus(bool isSaved)
{
	sequenceIsSaved = isSaved;
	if (isSaved)
	{
		sequenceSavedIndicator.SetCheck(BST_CHECKED);
	}
	else
	{
		sequenceSavedIndicator.SetCheck(BST_UNCHECKED);
	}
}


bool ConfigurationFileSystem::sequenceSettingsReadyCheck()
{
	if (!sequenceIsSaved)
	{
		if (checkSequenceSave("There are unsaved sequence settings. Would you like to save the current sequence before starting?"))
		{
			// canceled
			return true;
		}
	}
	return false;
}


bool ConfigurationFileSystem::checkSequenceSave(std::string prompt)
{
	if (!sequenceIsSaved)
	{
		int answer = MessageBox(0, prompt.c_str(), 0, MB_YESNOCANCEL);
		if (answer == IDYES)
		{
			saveSequence();
		}
		else if (answer == IDCANCEL)
		{
			return false;
		}
	}
	return true;
}


std::vector<std::string> ConfigurationFileSystem::getSequenceNames()
{
	return currentProfileSettings.sequenceConfigNames;
}


std::string ConfigurationFileSystem::getSequenceNamesString()
{
	std::string namesString = "";
	if (currentProfileSettings.sequence != "NO SEQUENCE")
	{
		namesString += "Sequence:\r\n";
		for (int sequenceInc = 0; sequenceInc < this->currentProfileSettings.sequenceConfigNames.size(); sequenceInc++)
		{
			namesString += "\t" + std::to_string(sequenceInc) + ": " + this->currentProfileSettings.sequenceConfigNames[sequenceInc] + "\r\n";
		}
	}
	return namesString;
}


void ConfigurationFileSystem::initializeControls( POINT& topLeftPos, CWnd* parent, int& id, fontMap fonts, std::vector<CToolTipCtrl*>& tooltips )
{
	// Experiment Type
	experimentLabel.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 20 };
	experimentLabel.ID = id++;
	experimentLabel.Create( "EXPERIMENT", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, experimentLabel.sPos,
							parent, experimentLabel.ID );
	experimentLabel.SetFont( fonts["Heading Font"] );
	// Experiment Saved Indicator
	experimentSavedIndicator.sPos = { topLeftPos.x + 360, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 20 };
	experimentSavedIndicator.ID = id++;
	experimentSavedIndicator.Create( "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT,
									 experimentSavedIndicator.sPos, parent, experimentSavedIndicator.ID );
	experimentSavedIndicator.SetFont( fonts["Normal Font"] );
	experimentSavedIndicator.SetCheck( BST_CHECKED );
	updateExperimentSavedStatus( true );
	// Category Title
	categoryLabel.sPos = { topLeftPos.x + 480, topLeftPos.y, topLeftPos.x + 960, topLeftPos.y + 20 };
	categoryLabel.ID = id++;
	categoryLabel.Create( "CATEGORY", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, categoryLabel.sPos, parent,
						  categoryLabel.ID );
	categoryLabel.SetFont( fonts["Heading Font"] );
	//
	categorySavedIndicator.sPos = { topLeftPos.x + 480 + 380, topLeftPos.y, topLeftPos.x + 960, topLeftPos.y + 20 };
	categorySavedIndicator.ID = id++;
	categorySavedIndicator.Create( "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT,
								   categorySavedIndicator.sPos, parent, categorySavedIndicator.ID );
	categorySavedIndicator.SetFont( fonts["Normal Font"] );
	categorySavedIndicator.SetCheck( BST_CHECKED );
	updateCategorySavedStatus( true );
	topLeftPos.y += 20;
	// Experiment Combo
	experimentCombo.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 800 };
	experimentCombo.ID = id++;
	if (experimentCombo.ID != IDC_EXPERIMENT_COMBO)
	{
		throw;
	}
	experimentCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
							experimentCombo.sPos, parent, experimentCombo.ID );
	experimentCombo.SetFont( fonts["Normal Font"] );
	this->reloadCombo( experimentCombo.GetSafeHwnd(), PROFILES_PATH, std::string( "*" ), "__NONE__" );
	// Category Combo
	categoryCombo.sPos = { topLeftPos.x + 480, topLeftPos.y, topLeftPos.x + 960, topLeftPos.y + 800 };
	categoryCombo.ID = id++;
	if (categoryCombo.ID != IDC_CATEGORY_COMBO)
	{
		throw;
	}
	categoryCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
						  categoryCombo.sPos, parent, categoryCombo.ID );
	categoryCombo.SetFont( fonts["Normal Font"] );
	topLeftPos.y += 25;
	// Orientation Title
	orientationLabel.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 120, topLeftPos.y + 20 };
	orientationLabel.ID = id++;
	orientationLabel.Create( "ORIENTATION", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, orientationLabel.sPos,
							 parent, orientationLabel.ID );
	orientationLabel.SetFont( fonts["Heading Font"] );
	// Configuration Title
	configLabel.sPos = { topLeftPos.x + 120, topLeftPos.y, topLeftPos.x + 960, topLeftPos.y + 20 };
	configLabel.ID = id++;
	configLabel.Create( "CONFIGURATION", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, configLabel.sPos, parent, configLabel.ID );
	configLabel.SetFont( fonts["Heading Font"] );
	// Configuration Saved Indicator
	configurationSavedIndicator.sPos = { topLeftPos.x + 860, topLeftPos.y, topLeftPos.x + 960, topLeftPos.y + 20 };
	configurationSavedIndicator.ID = id++;
	configurationSavedIndicator.Create( "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT,
										configurationSavedIndicator.sPos, parent, configurationSavedIndicator.ID );
	configurationSavedIndicator.SetFont( fonts["Normal Font"] );
	configurationSavedIndicator.SetCheck( BST_CHECKED );
	updateConfigurationSavedStatus( true );
	topLeftPos.y += 20;
	//eConfigurationSaved = true;
	// orientation combo
	std::vector<std::string> orientationNames;
	orientationNames.push_back( "Horizontal" );
	orientationNames.push_back( "Vertical" );
	orientationCombo.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 120, topLeftPos.y + 800 };
	orientationCombo.ID = id++;
	if (orientationCombo.ID != IDC_ORIENTATION_COMBO)
	{
		throw;
	}
	orientationCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
							 orientationCombo.sPos, parent, orientationCombo.ID );
	orientationCombo.SetFont( fonts["Normal Font"] );
	for (int comboInc = 0; comboInc < orientationNames.size(); comboInc++)
	{
		orientationCombo.AddString( orientationNames[comboInc].c_str() );
	}
	orientationCombo.SetCurSel( 0 );
	// configuration combo
	configCombo.sPos = { topLeftPos.x + 120, topLeftPos.y, topLeftPos.x + 960, topLeftPos.y + 800 };
	configCombo.ID = id++;
	if (configCombo.ID != IDC_CONFIGURATION_COMBO)
	{
		throw;
	}
	configCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, configCombo.sPos,
						parent, configCombo.ID );
	configCombo.SetFont( fonts["Normal Font"] );
	topLeftPos.y += 25;
	/// SEQUENCE
	sequenceLabel.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 20 };
	sequenceLabel.ID = id++;
	sequenceLabel.Create( "SEQUENCE", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, sequenceLabel.sPos, parent,
						  sequenceLabel.ID );
	sequenceLabel.SetFont( fonts["Heading Font"] );
	topLeftPos.y += 20;
	// combo
	sequenceCombo.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 800 };
	sequenceCombo.ID = id++;
	if (sequenceCombo.ID != IDC_SEQUENCE_COMBO)
	{
		throw;
	}
	sequenceCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
						 sequenceCombo.sPos, parent, sequenceCombo.ID );
	sequenceCombo.SetFont( fonts["Normal Font"] );
	sequenceCombo.AddString( "NULL SEQUENCE" );
	sequenceCombo.SetCurSel( 0 );
	sequenceCombo.SetItemHeight(0, 50);
	topLeftPos.y += 25;
	// display
	sequenceInfoDisplay.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 100 };
	sequenceInfoDisplay.ID = id++;
	sequenceInfoDisplay.Create( ES_READONLY | WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL,
								sequenceInfoDisplay.sPos, parent, sequenceInfoDisplay.ID );
	sequenceInfoDisplay.SetWindowTextA( "Sequence of Configurations to Run:\r\n" );
	sequenceSavedIndicator.sPos = { topLeftPos.x + 860, topLeftPos.y, topLeftPos.x + 960, topLeftPos.y + 20 };
	// saved indicator
	sequenceSavedIndicator.ID = id++;
	sequenceSavedIndicator.Create( "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT,
								   sequenceSavedIndicator.sPos, parent, sequenceSavedIndicator.ID );
	sequenceSavedIndicator.SetFont( fonts["Normal Font"] );
	sequenceSavedIndicator.SetCheck( BST_CHECKED );
	updateSequenceSavedStatus( true );
}


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


std::vector<std::string> ConfigurationFileSystem::searchForFiles( std::string locationToSearch, std::string extensions )
{
	// Re-add the entries back in and figure out which one is the current one.
	std::vector<std::string> names;
	std::string search_path = locationToSearch + "\\" + extensions;
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	if (extensions == "*")
	{
		hFind = FindFirstFileEx( search_path.c_str(), FindExInfoStandard, &fd, FindExSearchLimitToDirectories, NULL, 0 );
	}
	else
	{
		hFind = FindFirstFile( search_path.c_str(), &fd );
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
					if (std::string( fd.cFileName ) != "." && std::string( fd.cFileName ) != "..")
					{
						names.push_back( fd.cFileName );
					}
				}
			}
			else
			{
				if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					names.push_back( fd.cFileName );
				}
			}
		} while (FindNextFile( hFind, &fd ));
		FindClose( hFind );
	}

	// Remove suffix from file names and...
	for (int configListInc = 0; configListInc < names.size(); configListInc++)
	{
		if (extensions == "*" || extensions == "*.*" || extensions == std::string( "*" ) + HORIZONTAL_EXTENSION
			 || extensions == std::string( "*" ) + VERTICAL_EXTENSION || extensions == std::string( "*" ) + SEQUENCE_EXTENSION
			 || extensions == std::string( "*" ) + CATEGORY_EXTENSION || extensions == std::string( "*" ) + EXPERIMENT_EXTENSION)
		{
			names[configListInc] = names[configListInc].substr( 0, names[configListInc].size() - (extensions.size() - 1) );
		}
		else
		{
			names[configListInc] = names[configListInc].substr( 0, names[configListInc].size() - extensions.size() );
		}
	}
	// Make the final vector out of the unique objects left.
	return names;
}


void ConfigurationFileSystem::reloadCombo( HWND comboToReload, std::string locationToLook, std::string extension, std::string nameToLoad )
{
	std::vector<std::string> names;
	// search for folders
	names = searchForFiles( locationToLook, extension );

	/// Get current selection
	long long itemIndex = SendMessage( comboToReload, CB_GETCURSEL, 0, 0 );
	TCHAR experimentConfigToOpen[256];
	std::string currentSelection;
	int currentInc = -1;
	if (itemIndex != -1)
	{
		// Send CB_GETLBTEXT message to get the item.
		SendMessage( comboToReload, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)experimentConfigToOpen );
		currentSelection = experimentConfigToOpen;
	}
	/// Reset stuffs
	SendMessage( comboToReload, CB_RESETCONTENT, 0, 0 );
	// Send list to object
	for (int comboInc = 0; comboInc < names.size(); comboInc++)
	{
		if (nameToLoad == names[comboInc])
		{
			currentInc = comboInc;
		}
		TCHAR * name = (TCHAR*)names[comboInc].c_str();
		SendMessage( comboToReload, CB_ADDSTRING, 0, (LPARAM)(name) );
	}
	// Set initial value
	SendMessage( comboToReload, CB_SETCURSEL, currentInc, 0 );
}

bool ConfigurationFileSystem::fileOrFolderExists(std::string filePathway)
{
	// got this from stack exchange. dunno how it works but it should be fast.
	struct stat buffer;
	return (stat(filePathway.c_str(), &buffer) == 0);
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
		thrower("Delete Failed! Ask mark about bugs.");
	}
}


std::string ConfigurationFileSystem::getCurrentCategory()
{
	return currentProfileSettings.category;
}

std::string ConfigurationFileSystem::getCurrentExperiment()
{
	return currentProfileSettings.experiment;
}

profileSettings ConfigurationFileSystem::getCurrentProfileSettings()
{
	return currentProfileSettings;
}