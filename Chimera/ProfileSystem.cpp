#include "stdafx.h"

#include <fstream>
#include "Commctrl.h"
#include <boost/filesystem.hpp>

#include "ProfileSystem.h"
#include "TextPromptDialog.h"
#include "NiawgController.h"
#include "AuxiliaryWindow.h"
#include "Andor.h"
#include "CameraWindow.h"


ProfileSystem::ProfileSystem(std::string fileSystemPath)
{
	FILE_SYSTEM_PATH = fileSystemPath;
}


// just looks at the info in a file and loads it into references, doesn't change anything in the gui or main settings.
void ProfileSystem::openNiawgFiles( niawgPair<std::vector<std::fstream>>& scriptFiles, profileSettings profile, 
	bool programNiawg )
{
	scriptFiles[Vertical].resize( profile.sequenceConfigNames.size() );
	scriptFiles[Horizontal].resize( profile.sequenceConfigNames.size() );
	/// gather information from every configuration in the sequence. /////////////////////////////////////////////////////////////////////
	for (UINT sequenceInc = 0; sequenceInc < profile.sequenceConfigNames.size(); sequenceInc++)
	{
		// open configuration file
		std::ifstream configFile( profile.categoryPath + "\\" + profile.sequenceConfigNames[sequenceInc] );
		std::string intensityScriptAddress, version;
		niawgPair<std::string> niawgScriptAddresses;
		// first get version info:
		std::getline( configFile, version );
		/// load files
		checkDelimiterLine( configFile, "SCRIPTS" );
		configFile.get();
		for (auto axis : AXES)
		{
			getline( configFile, niawgScriptAddresses[axis] );
			if (programNiawg)
			{
				scriptFiles[axis][sequenceInc].open( niawgScriptAddresses[axis] );
				if (!scriptFiles[axis][sequenceInc].is_open())
				{
					thrower( "ERROR: Failed to open vertical script file named: " + niawgScriptAddresses[axis]
							 + " found in configuration: " + profile.sequenceConfigNames[sequenceInc] + "\r\n" );
				}
			}
		}
	}
}


void ProfileSystem::saveEntireProfile( ScriptingWindow* scriptWindow, MainWindow* mainWin, 
												 AuxiliaryWindow* auxWindow, CameraWindow* camWin)
{
	saveExperimentOnly( mainWin );
	saveCategoryOnly( mainWin );
	saveConfigurationOnly( scriptWindow, mainWin, auxWindow, camWin);
	saveSequence();		
}


void ProfileSystem::checkSaveEntireProfile(ScriptingWindow* scriptWindow, MainWindow* mainWin, 
													 AuxiliaryWindow* auxWin, CameraWindow* camWin)
{
	checkExperimentSave( "Save Experiment Settings?", mainWin);
	checkCategorySave( "Save Category Settings?", mainWin );
	checkConfigurationSave( "Save Configuration Settings?", scriptWindow, mainWin, auxWin, camWin);
	checkSequenceSave( "Save Sequence Settings?" );
}


void ProfileSystem::allSettingsReadyCheck(ScriptingWindow* scriptWindow, MainWindow* mainWin, 
													AuxiliaryWindow* auxWin, CameraWindow* camWin)
{
	// check all components of this class.
	experimentSettingsReadyCheck(mainWin);
	categorySettinsReadyCheck();
	configurationSettingsReadyCheck( scriptWindow, mainWin, auxWin, camWin);
	sequenceSettingsReadyCheck();
	// passed all checks.
}

/// CONFIGURATION LEVEL HANDLING

void ProfileSystem::newConfiguration( MainWindow* mainWin, AuxiliaryWindow* auxWin, CameraWindow* camWin, 
									  ScriptingWindow* scriptWin )
{
	// check if category has been set yet.
	if (currentProfile.category == "")
	{
		// check if the experiment has also not been set.
		if (currentProfile.experiment == "")
		{
			thrower( "The Experiment and category have not yet been selected! Please select a category or create a new"
					" one before trying to save this configuration." );
		}
		else
		{
			thrower("The category has not yet been selected! Please select a category or create a new one before "
					"trying to save this configuration.");
		}
	}

	std::string configNameToSave;
	TextPromptDialog dialog(&configNameToSave, "Please enter new configuration name.");
	dialog.DoModal();

	if (configNameToSave == "")
	{
		// canceled
		return;
	}
	std::string newConfigPath = currentProfile.categoryPath + configNameToSave + CONFIG_EXTENSION;

	std::ofstream newConfigFile(cstr(newConfigPath));
	if (!newConfigFile.is_open())
	{
		thrower( "ERROR: Failed to create new configuration file. Ask Mark about bugs." );
	}
	newConfigFile << "Version: " + str( versionMain ) + "." + str( versionSub ) + "\n";
	// give it to each window, allowing each window to save its relevant contents to the config file. Order matters.
	scriptWin->handleNewConfig( newConfigFile );
	camWin->handleNewConfig( newConfigFile );
	auxWin->handleNewConfig( newConfigFile );
	mainWin->handleNewConfig( newConfigFile );
	newConfigFile.close( );
	reloadCombo(configCombo.GetSafeHwnd(), currentProfile.categoryPath, str("*") + CONFIG_EXTENSION,
		cstr(currentProfile.configuration));
}


/*
]--- This function opens a given configuration file, sets all of the relevant parameters, and loads the associated scripts. 
*/
void ProfileSystem::openConfig( std::string configurationNameToOpen, ScriptingWindow* scriptWin, MainWindow* mainWin, 
								CameraWindow* camWin, AuxiliaryWindow* auxWin )
{
	// no folder associated with configuraitons. They share the category folder.
	std::string path = currentProfile.categoryPath + configurationNameToOpen + CONFIG_EXTENSION;
	currentProfile.configuration = configurationNameToOpen;
	std::ifstream configFile(path);
	// check if opened correctly.
	if (!configFile.is_open())
	{
		thrower("Opening of Configuration File Failed!");
	}
	currentProfile.configuration = configurationNameToOpen;
	
	std::string versionStr;
	// Version is saved in format "Master Version: x.x"
	// eat the "version" word"
	configFile >> versionStr;
	configFile >> versionStr;
	double version;
	try
	{
		version = std::stod(versionStr);
	}
	catch (std::invalid_argument&)
	{
		thrower("ERROR: Version string failed to convert to double while opening configuration!");
	}

	/// give to scripting window
	scriptWin->handleOpenConfig(configFile, version);

	/// give to camera window
	camWin->handleOpeningConfig(configFile, version);

	/// give to device window
	auxWin->handleOpeningConfig(configFile, version);

	/// give to main window
	mainWin->handleOpeningConfig(configFile, version);
	
	/// finish up
	updateConfigurationSavedStatus(true);
	// actually set this now
	scriptWin->updateProfile(currentProfile.category + "->" + currentProfile.configuration);
	// close.
	configFile.close();
	if (currentProfile.sequence == NULL_SEQUENCE)
	{
		// reload it.
		loadNullSequence();
	}
	currentProfile.configuration = configurationNameToOpen;
}


// small convenience function that I use while opening a file.
void ProfileSystem::checkDelimiterLine(std::ifstream& openFile, std::string delimiter)
{
	std::string checkStr;
	openFile >> checkStr;
	if (checkStr != delimiter)
	{
		thrower("ERROR: Expected \"" + delimiter + "\" in configuration file, but instead found \"" + checkStr + "\"");
	}
}

// version with break condition. If returns true, calling function should break out of the loop which is checking the
// line.
bool ProfileSystem::checkDelimiterLine( std::ifstream& openFile, std::string delimiter, std::string breakCondition )
{
	std::string checkStr;
	openFile >> checkStr;
	if ( checkStr != delimiter )
	{
		if ( checkStr == breakCondition )
		{
			return true;
		}
		thrower( "ERROR: Expected \"" + delimiter + "\" in configuration file, but instead found \"" + checkStr + "\"" );
	}
	return false;
}



/*
]--- This function attempts to save the configuration given the configuration name in the argument. It throws errors and warnings if this 
]- is not a Normal Save, i.e. if the file doesn't already exist or if the user tries to pass an empty name as an argument. It returns 
]- false if the configuration got saved, true if something prevented the configuration from being saved.
*/
void ProfileSystem::saveConfigurationOnly( ScriptingWindow* scriptWindow, MainWindow* mainWin, 
													 AuxiliaryWindow* auxWin, CameraWindow* camWin )
{
	std::string configNameToSave = currentProfile.configuration;
	// check if category has been set yet.
	if (currentProfile.category == "")
	{
		// check if the experiment has also not been set.
		if (currentProfile.experiment == "")
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
	if (configNameToSave == "")
	{
		thrower( "ERROR: The program requested saving the configuration file to an empty name! This shouldn't happen, ask Mark "
				 "about bugs." );
	}

	// check if file already exists

	if (!ProfileSystem::fileOrFolderExists(currentProfile.categoryPath + configNameToSave + CONFIG_EXTENSION))  
	{
		int answer = promptBox("This configuration file appears to not exist in the expected location: " 
									 + currentProfile.categoryPath + configNameToSave 
									 + CONFIG_EXTENSION + ". Continue by making a new configuration file?", MB_OKCANCEL );
		if (answer == IDCANCEL)
		{
			return;
		}
	}
	
	if (!experimentIsSaved)
	{
		int answer = promptBox( "The Experiment settings have not yet been saved. Save them before the "
								 "configuration? (Optional)", MB_YESNOCANCEL );
		if (answer == IDCANCEL)
		{
			return;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			saveExperimentOnly(mainWin);
		}
	}
	if (!categoryIsSaved)
	{
		int answer = promptBox("The Category settings have not yet been saved. Save them before the configuration?"
								 " (Optional)", MB_YESNOCANCEL );
		if (answer == IDCANCEL)
		{
			return;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			saveCategoryOnly(mainWin);
		}
	}
	
	std::ofstream configSaveFile(currentProfile.categoryPath + configNameToSave + CONFIG_EXTENSION);
	if (!configSaveFile.is_open())
	{
		thrower( "Couldn't save configuration file! Check the name for weird characters, or call Mark about bugs if "
			"everything seems right..." );
	}
	// That's the last prompt the user gets, so the save is final now.
	currentProfile.configuration = configNameToSave;
	// version 2.0 started when the unified coding system (the chimera system) began, and the profile system underwent
	// dramatic changes in order to 
	configSaveFile << std::setprecision( 13 );
	configSaveFile << "Version: " + str(versionMain) + "." + str(versionSub) + "\n";
	// give it to each window, allowing each window to save its relevant contents to the config file. Order matters.
	scriptWindow->handleSavingConfig(configSaveFile);
	camWin->handleSaveConfig(configSaveFile);
	auxWin->handleSaveConfig(configSaveFile);
	mainWin->handleSaveConfig(configSaveFile);
	configSaveFile.close();
	updateConfigurationSavedStatus(true);
}

/*
]--- Identical to saveConfigurationOnly except that it prompts the user for a name with a dialog box instead of taking one.
*/
void ProfileSystem::saveConfigurationAs(ScriptingWindow* scriptWindow, MainWindow* mainWin, AuxiliaryWindow* auxWin)
{
	// check if category has been set yet.
	if (currentProfile.category == "")
	{
		// check if the experiment has also not been set.
		if (currentProfile.experiment == "")
		{
			thrower( "The Experiment and category have not yet been selected! Please select a category or create a new one "
				"before trying to save this configuration.");
		}
		else
		{
			thrower( "The category has not yet been selected! Please select a category or create a new one before trying to "
					 "save this configuration." );
		}
	}
	std::string configurationNameToSave;
	TextPromptDialog dialog(&configurationNameToSave, "Please enter new configuration name.");
	dialog.DoModal();

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
	
	// check if file already exists
	if (ProfileSystem::fileOrFolderExists(currentProfile.categoryPath + configurationNameToSave + CONFIG_EXTENSION ))
	{
		int answer = promptBox("This configuration file name already exists! Overwrite it?", MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
	}

	if (!experimentIsSaved)
	{
		int answer = promptBox("The Experiment settings have not yet been saved. Save them before the configuration? "
								 "(Optional)", MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			saveExperimentOnly(mainWin);
		}
	}
	if (!categoryIsSaved)
	{
		int answer = promptBox("The Category settings have not yet been saved. Save them before the configuration? (Optional)",
								 MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			saveCategoryOnly(mainWin);
		}
	}

	std::ofstream configurationSaveFile(currentProfile.categoryPath + configurationNameToSave + CONFIG_EXTENSION );
	if (!configurationSaveFile.is_open())
	{
		thrower( "Couldn't save configuration file! Check the name for weird characters, or call Mark about bugs if everything seems right..." );
	}

	// That's the last prompt the user gets, so the save is final now.
	currentProfile.configuration = configurationNameToSave;
	// Version info tells future code about formatting.
	configurationSaveFile << "Version: "+ str(versionMain) + "." + str(versionSub) + "\n";
	scriptInfo<std::string> addresses = scriptWindow->getScriptAddresses();
	// order matters!
	// vertical Script File Address
	configurationSaveFile << addresses.verticalNIAWG << "\n";
	// horizontal Script File Address
	configurationSaveFile << addresses.horizontalNIAWG << "\n";
	// Intensity Script File Address
	configurationSaveFile << addresses.intensityAgilent << "\n";
	// Number of Variables
	std::vector<variableType> vars = auxWin->getAllVariables();
	configurationSaveFile << vars.size() << "\n";
	// Variable Names
	// This part changed in version 1.1.
	for (UINT varInc = 0; varInc < vars.size(); varInc++)
	{
		variableType info = vars[varInc];
		configurationSaveFile << info.name << " ";
		if (info.constant)
		{
			configurationSaveFile << "Singleton ";
		}
		else
		{
			configurationSaveFile << "From_Master ";
		}
		configurationSaveFile << info.ranges.front().initialValue << "\n";
	}
	std::string notes = mainWin->getNotes("configuration");
	configurationSaveFile << notes + "\n";
	configurationSaveFile << "END CONFIGURATION NOTES" << "\n";
	configurationSaveFile.close();
	reloadCombo(configCombo.GetSafeHwnd(), currentProfile.categoryPath, str("*") + CONFIG_EXTENSION, currentProfile.configuration);
	updateConfigurationSavedStatus(true);
}


/*
]--- This function renames the currently set 
*/
void ProfileSystem::renameConfiguration()
{
	// check if configuration has been set yet.
	if (currentProfile.configuration == "")
	{
		if (currentProfile.category == "")
		{
			// check if the experiment has also not been set.
			if (currentProfile.experiment == "")
			{
				thrower( "The Experiment and category have not yet been selected! Please select a category or create a new "
						 "one before trying to save this configuration.");
			}
			else
			{
				thrower( "The category has not yet been selected! Please select a category or create a new one before trying"
						 " to save this configuration.");
			}
		}
		else
		{
			thrower( "The Configuration has not yet been selected! Please select a category or create a new one before "
					 "trying to rename it." );
		}
	}

	std::string newConfigurationName;
	TextPromptDialog dialog(&newConfigurationName, "Please enter new configuration name.");
	dialog.DoModal();

	if (newConfigurationName == "")
	{
		// canceled
		return;
	}
	std::string currentConfigurationLocation = currentProfile.categoryPath + currentProfile.configuration + CONFIG_EXTENSION;
	std::string newConfigurationLocation = currentProfile.categoryPath + newConfigurationName + CONFIG_EXTENSION;
	int result = MoveFile(cstr(currentConfigurationLocation), cstr(newConfigurationLocation));
	if (result == 0)
	{
		thrower( "Renaming of the configuration file Failed! Ask Mark about bugs" );
	}
	currentProfile.configuration = newConfigurationName;
	reloadCombo(configCombo.GetSafeHwnd(), currentProfile.categoryPath, str("*") + CONFIG_EXTENSION, "__NONE__");
}


/*
]--- 
*/
void ProfileSystem::deleteConfiguration()
{
	// check if configuration has been set yet.
	if (currentProfile.configuration == "")
	{
		if (currentProfile.category == "")
		{
			// check if the experiment has also not been set.
			if (currentProfile.experiment == "")
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
	int answer = promptBox("Are you sure you want to delete the current configuration: " 
								 + currentProfile.configuration, MB_YESNO);
	if (answer == IDNO)
	{
		return;
	}
	std::string currentConfigurationLocation = currentProfile.categoryPath + currentProfile.configuration + CONFIG_EXTENSION;
	int result = DeleteFile(cstr(currentConfigurationLocation));
	if (result == 0)
	{
		thrower( "ERROR: Deleteing the configuration file failed!" );
	}
	// since the configuration this (may have been) was saved to is gone, no saved version of current code.
	this->updateConfigurationSavedStatus(false);
	// just deleted the current configuration
	currentProfile.configuration = "";
	// reset combo since the files have now changed after delete
		reloadCombo(configCombo.GetSafeHwnd(), currentProfile.categoryPath, str("*") + CONFIG_EXTENSION, "__NONE__");
}

/*
]--- 
*/
void ProfileSystem::updateConfigurationSavedStatus(bool isSaved)
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


bool ProfileSystem::configurationSettingsReadyCheck( ScriptingWindow* scriptWindow, MainWindow* mainWin, 
															   AuxiliaryWindow* auxWin, CameraWindow* camWin )
{
	// prompt for save.
	if (checkConfigurationSave( "There are unsaved configuration settings. Would you like to save the current "
								"configuration before starting?", scriptWindow, mainWin, auxWin, camWin))
	{
		// canceled
		return true;
	}
	return false;
}

bool ProfileSystem::checkConfigurationSave(std::string prompt, ScriptingWindow* scriptWindow, 
													 MainWindow* mainWin, AuxiliaryWindow* auxWin, CameraWindow* camWin)
{
	if (!configurationIsSaved)
	{
		int answer = promptBox(prompt, MB_YESNOCANCEL);
		if (answer == IDYES)
		{
			saveConfigurationOnly(scriptWindow, mainWin, auxWin, camWin);
		}
		else if (answer == IDCANCEL)
		{
			return true;
		}
	}
	return false;
}


void ProfileSystem::configurationChangeHandler( ScriptingWindow* scriptWindow, MainWindow* mainWin, 
														  AuxiliaryWindow* auxWin, CameraWindow* camWin )
{
	if (!configurationIsSaved)
	{
		if (checkConfigurationSave( "The current configuration is unsaved. Save current configuration before changing?",
									scriptWindow, mainWin, auxWin, camWin))
		{
			configCombo.SelectString(0, cstr(currentProfile.configuration));
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
	configCombo.GetLBText(int(itemIndex), configurationToOpen);
	openConfig( configurationToOpen, scriptWindow, mainWin, camWin, auxWin );
	// it'd be confusing if these notes stayed here.
}


/// CATEGORY LEVEL HANDLING
/*
]--- This function attempts to save the category given the category name in the argument. It throws errors and warnings if this is not a Normal
]- Save, i.e. if the file doesn't already exist or if the user tries to pass an empty name as an argument. It returns false if the category got saved,
]- true if something prevented the category from being saved.
*/
void ProfileSystem::saveCategoryOnly(MainWindow* mainWin)
{
	std::string categoryNameToSave = currentProfile.category;
	// check if experiment has been set
	if (currentProfile.experiment == "")
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
	if (!ProfileSystem::fileOrFolderExists(currentProfile.categoryPath + categoryNameToSave + CATEGORY_EXTENSION))
	{
		int answer = promptBox("This category file appears to not exist in the expected location: " 
									  + currentProfile.categoryPath + categoryNameToSave
									  + CATEGORY_EXTENSION + ".  Continue by making a new category file?", MB_OKCANCEL );
		if (answer == IDCANCEL)
		{
			return;
		}
	}

	if (!experimentIsSaved)
	{
		int answer = promptBox("The Experiment settings have not yet been saved. Save them before the category? (Optional)", 
								 MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
		else if (answer == IDYES)
		{
			saveExperimentOnly( mainWin );
		}
	}
	std::fstream categoryFileToSave(currentProfile.categoryPath + categoryNameToSave + CATEGORY_EXTENSION, std::ios::out);
	if (!categoryFileToSave.is_open())
	{
		thrower( "ERROR: failed to save category file! Ask mark about bugs." );
	}
	categoryFileToSave << "Version: 1.0\n";
	std::string categoryNotes = mainWin->getNotes("category");
	categoryFileToSave << categoryNotes + "\n";
	categoryFileToSave << "END CATEGORY NOTES\n";
	currentProfile.category = categoryNameToSave;
	currentProfile.categoryPath = currentProfile.experimentPath + categoryNameToSave + "\\";
	updateCategorySavedStatus(true);
}


std::string ProfileSystem::getCurrentPathIncludingCategory()
{
	return currentProfile.categoryPath;
}


/*
]--- identical to saveCategoryOnly except that 
*/
void ProfileSystem::saveCategoryAs( MainWindow* mainWin )
{
	// check if experiment has been set
	if (currentProfile.experiment == "")
	{
		thrower( "The Experiment has not yet been selected! Please select a Experiment or create a new one before trying to save this "
				 "category." );
	}

	std::string categoryNameToSave;
	TextPromptDialog dialog(&categoryNameToSave, "Please enter new category name.");
	dialog.DoModal();

	if (categoryNameToSave == "")
	{
		thrower( "ERROR: The program requested the saving of the category file to an empty name! This shouldn't happen, ask Mark about bugs." );
	}

	// check if file already exists. No extension, looking for a folder here. 
	if (!ProfileSystem::fileOrFolderExists( currentProfile.categoryPath + categoryNameToSave + CATEGORY_EXTENSION ))
	{
		int answer = promptBox("This category file appears to not exist in the expected location: " 
									  + currentProfile.categoryPath + categoryNameToSave
									  + CATEGORY_EXTENSION + ".  Continue by making a new category file?", MB_OKCANCEL );
		if (answer == IDCANCEL)
		{
			return;
		}
	}

	if (!experimentIsSaved)
	{
		int answer = promptBox("The Experiment settings have not yet been saved. Save them before the category? (Optional)", 
								 MB_YESNOCANCEL );
		if (answer == IDCANCEL)
		{
			return;
		}
		else if (answer == IDYES)
		{
			// save the experiment!
			saveExperimentOnly( mainWin );
		}
	}
	// need to make a new folder as well.
	int result = CreateDirectory( cstr(currentProfile.experimentPath + categoryNameToSave), 0 );
	if (result == 0)
	{
		thrower( "ERROR: failed to create new category directory during category save as! Ask Mark about Bugs." );
	}
	std::fstream categoryFileToSave( currentProfile.categoryPath + categoryNameToSave + CATEGORY_EXTENSION, std::ios::out );
	if (!categoryFileToSave.is_open())
	{
		thrower( "ERROR: failed to save category file! Ask mark about bugs." );
	}
	categoryFileToSave << "Version: 1.0\n";

	std::string categoryNotes = mainWin->getNotes( "category" );
	categoryFileToSave << categoryNotes + "\n";
	categoryFileToSave << "END CATEGORY NOTES\n";
	currentProfile.category = categoryNameToSave;
	currentProfile.categoryPath = currentProfile.experimentPath + categoryNameToSave + "\\";
	updateCategorySavedStatus( true );
}


/*
]---
*/
void ProfileSystem::renameCategory()
{
	// TODO: this is a bit more complicated because of the way that all of the configuration fle locations are currently set.
	thrower("This feature still needs implementing! It doesn't work right now");
}

/*
]---
*/
void ProfileSystem::deleteCategory()
{
	// check if category has been set yet.
	if (currentProfile.category == "")
	{
		// check if the experiment has also not been set.
		if (currentProfile.experiment == "")
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
	int answer = promptBox("Are you sure you want to delete the current Category and all configurations within? The current category "
								 "is: " + currentProfile.category, MB_YESNO);
	if (answer == IDNO)
	{
		return;
	}
	answer = promptBox("Are you really sure? The current category is: " + currentProfile.category, MB_YESNO);
	if (answer == IDNO)
	{
		return;
	}
	std::string currentCategoryLocation = currentProfile.experimentPath + currentProfile.category;
	fullyDeleteFolder( currentCategoryLocation );
	updateCategorySavedStatus(false);
	currentProfile.category = "";
	currentProfile.categoryPath == "";
	reloadCombo(categoryCombo.GetSafeHwnd(), currentProfile.experimentPath, "*", "__NONE__");
}


void ProfileSystem::newCategory()
{
	// check if experiment has been set
	if (currentProfile.experiment == "")
	{
		thrower( "The Experiment has not yet been selected! Please select a Experiment or create a new one before"
				" trying to save this category." );
	}

	std::string categoryNameToSave;
	TextPromptDialog dialog(&categoryNameToSave, "Please enter new category name.");
	dialog.DoModal();

	// check to make sure that this is a name.
	if (categoryNameToSave == "")
	{
		// canceled
		return;
	}

	// check if file already exists. No extension, looking for a folder here. 
	if (ProfileSystem::fileOrFolderExists(currentProfile.experimentPath + categoryNameToSave))
	{
		thrower( "This category name already exists! If it doesn't appear in the combo, try taking a look at what's in the relvant folder..." );
	}
	int result = CreateDirectory(cstr(currentProfile.experimentPath + categoryNameToSave), 0);
	if (result == 0)
	{
		thrower("ERROR: failed to create category directory! Ask Mark about bugs.");
	}
	std::ofstream categorySaveFolder(currentProfile.experimentPath + categoryNameToSave + "\\" + categoryNameToSave + CATEGORY_EXTENSION);
	categorySaveFolder.close();
	reloadCombo(categoryCombo.GetSafeHwnd(), currentProfile.experimentPath, "*", currentProfile.category);
}


void ProfileSystem::openCategory(std::string categoryToOpen, ScriptingWindow* scriptWindow, MainWindow* mainWin)
{
	// this gets called from the file menu.
	// Assign based on the comboBox Item entry.
	std::string path = currentProfile.experimentPath + categoryToOpen + "\\" + categoryToOpen + CATEGORY_EXTENSION;
	std::ifstream categoryConfigOpenFile(path);
	// check if opened correctly.
	if (!categoryConfigOpenFile.is_open())
	{
		thrower( "Opening of Category Configuration File Failed!" );
	}
	currentProfile.category = categoryToOpen;
	currentProfile.categoryPath = currentProfile.experimentPath + categoryToOpen + "\\";
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
		mainWin->setNotes("category", notes);
	}
	else
	{
		mainWin->setNotes("category", "");
	}
	scriptWindow->updateProfile("");
	// close.
	categoryConfigOpenFile.close();
	updateCategorySavedStatus(true);
}


void ProfileSystem::updateCategorySavedStatus(bool isSaved)
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


bool ProfileSystem::categorySettinsReadyCheck()
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

bool ProfileSystem::checkCategorySave(std::string prompt, MainWindow* mainWin)
{
	if (!categoryIsSaved)
	{
		int answer = promptBox(prompt, MB_YESNOCANCEL);
		if (answer == IDYES)
		{
			this->saveCategoryOnly(mainWin);
		}
		else if (answer == IDCANCEL)
		{
			return true;
		}
	}
	return false;
}

void ProfileSystem::categoryChangeHandler(ScriptingWindow* scriptWindow, MainWindow* mainWin)
{
	if (!categoryIsSaved)
	{
		if (checkCategorySave("The current category is unsaved. Save current category before changing?", mainWin))
		{
			categoryCombo.SelectString(0, cstr(currentProfile.category));
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
	categoryCombo.GetLBText(int(itemIndex), categoryConfigToOpen);
	openCategory( str( categoryConfigToOpen ), scriptWindow, mainWin );

	// it'd be confusing if these notes stayed here.
	mainWin->setNotes("configuration", "");
	reloadCombo(configCombo.GetSafeHwnd(), currentProfile.categoryPath, 
					 str("*") + CONFIG_EXTENSION, "__NONE__");
	currentProfile.configuration = "";
	reloadSequence(NULL_SEQUENCE);
}


/// EXPERIMENT LEVEL HANDLING
void ProfileSystem::saveExperimentOnly(MainWindow* mainWin)
{
	std::string experimentNameToSave = currentProfile.experiment;
	// check that the experiment name is not empty.
	if (experimentNameToSave == "")
	{
		thrower( "ERROR: Please properly select the experiment or create a new one (\'new experiment\') before trying"
				 " to save it!" );
	}
	// check if file already exists
	if (!ProfileSystem::fileOrFolderExists( FILE_SYSTEM_PATH + experimentNameToSave + "\\" + experimentNameToSave 
											+ EXPERIMENT_EXTENSION))
	{
		int answer = promptBox("This experiment file appears to not exist in the expected location: " 
								+ FILE_SYSTEM_PATH + "   \r\n.Continue by making a new experiment file?", 
								MB_OKCANCEL);
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
	currentProfile.experiment = experimentNameToSave;
	/// Start Outputting information

	std::string notes = mainWin->getNotes("experiment");
	experimentSaveFile << notes << "\n";
	experimentSaveFile << "END EXPERIMENT NOTES" << "\n";
	// And done.
	experimentSaveFile.close();
	currentProfile.experiment = experimentNameToSave;
	// update the save path. 
	currentProfile.experimentPath = FILE_SYSTEM_PATH + experimentNameToSave + "\\";
	// update the configuration saved statis for "this" object.
	updateExperimentSavedStatus(true);
	reloadCombo(experimentCombo.GetSafeHwnd(), FILE_SYSTEM_PATH, "*", currentProfile.experiment);
}


void ProfileSystem::saveExperimentAs(MainWindow* mainWin)
{
	if (currentProfile.experiment == "")
	{
		thrower("Please select an experiment before using \"Save As\"");
	}
	std::string experimentNameToSave;
	TextPromptDialog dialog(&experimentNameToSave, "Please enter a new experiment name.");
	dialog.DoModal();

	// check that the experiment name is not empty. 
	if (experimentNameToSave == "")
	{
		// canceled.
		return;
	}

	// check if file already exists
	if (ProfileSystem::fileOrFolderExists( FILE_SYSTEM_PATH + experimentNameToSave + ".eConfig" ))
	{
		int answer = promptBox("This experiment name appears to already exist in the expected location: " + FILE_SYSTEM_PATH + "."
									  "Overwrite this file?", MB_OKCANCEL );
		if (answer == IDCANCEL)
		{
			return;
		}
	}

	int result = CreateDirectory(cstr(FILE_SYSTEM_PATH + experimentNameToSave), 0);
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
	currentProfile.experiment = experimentNameToSave;

	debugInfo options = mainWin->getDebuggingOptions();
	mainOptions settings = mainWin->getMainOptions();
	/// Start Outputting information
	// notes.
	std::string notes = mainWin->getNotes("experiment");
	experimentSaveFile << notes << "\n";
	experimentSaveFile << "END EXPERIMENT NOTES" << "\n";
	// And done.
	experimentSaveFile.close();
	// update the save path. 
	currentProfile.experimentPath = FILE_SYSTEM_PATH + experimentNameToSave + "\\";
	// update the configuration saved statis for "this" object.
	updateExperimentSavedStatus(true);
	reloadCombo(experimentCombo.GetSafeHwnd(), FILE_SYSTEM_PATH, "*", currentProfile.experiment);
}


void ProfileSystem::renameExperiment(MainWindow* mainWin)
{
	// check if saved
	if (!experimentIsSaved)
	{
		if (checkExperimentSave("Save experiment before renaming it?", mainWin))
		{
			return;
		}
	}
	// get name
	std::string experimentNameToSave;
	TextPromptDialog dialog(&experimentNameToSave, "Please enter a new experiment name.");
	dialog.DoModal();

	// check if file already exists. No extension, looking for a folder here. 
	if (ProfileSystem::fileOrFolderExists(FILE_SYSTEM_PATH + experimentNameToSave))
	{
		int answer = promptBox("This experiment name already exists!", MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return;
		}
	}
	std::string newExperimentConfigLocation = FILE_SYSTEM_PATH + experimentNameToSave + "\\" + experimentNameToSave + EXPERIMENT_EXTENSION;
	std::string currentExperimentConfigLocation = FILE_SYSTEM_PATH + experimentNameToSave + "\\" + currentProfile.experiment + EXPERIMENT_EXTENSION;
	int result = MoveFileA(cstr(FILE_SYSTEM_PATH + currentProfile.experiment), cstr(FILE_SYSTEM_PATH + experimentNameToSave));
	if (result == 0)
	{
		thrower( "ERROR: Moving the experiment folder failed!" );
	}
	result = MoveFile(cstr(currentExperimentConfigLocation), cstr(newExperimentConfigLocation));
	if (result == 0)
	{
		thrower( "Moving the experiment folder failed!" );
	}
	// TODO: program the code to go through all of the category and configuration file names and change addresses, or change format of how these are referenced 
	// in configuraiton file.
	currentProfile.experiment = experimentNameToSave;
	reloadCombo(experimentCombo.GetSafeHwnd(), FILE_SYSTEM_PATH, "*", currentProfile.experiment);
}

void ProfileSystem::deleteExperiment()
{
	if (currentProfile.experiment == "")
	{
		thrower( "No experiment has been set!" );
		return;
	}
	int answer = promptBox("Are you sure that you'd like to delete the current experiment and all categories and configurations "
								  "within? Current Experiment: " + currentProfile.experiment, MB_YESNO );
	if (answer == IDNO)
	{
		return;
	}
	answer = promptBox("Are you really really sure? Current Experiment: " + currentProfile.experiment, MB_YESNO );
	if (answer == IDNO)
	{
		return;
	}
	fullyDeleteFolder( currentProfile.experimentPath);
	reloadCombo( experimentCombo.GetSafeHwnd(), FILE_SYSTEM_PATH, "*", "__NONE__" );
	updateExperimentSavedStatus( false );
	currentProfile.experiment = "";
	currentProfile.experimentPath = "";
	return;
}


void ProfileSystem::newExperiment()
{
	std::string newExperimentName;
	TextPromptDialog dialog(&newExperimentName, "Please enter a new experiment name.");
	dialog.DoModal();
	std::string newExperimentPath = FILE_SYSTEM_PATH + newExperimentName;
	CreateDirectory(cstr(newExperimentPath), 0);
	std::ofstream newExperimentConfigFile;
	newExperimentConfigFile.open(cstr(newExperimentPath + "\\" + newExperimentName + EXPERIMENT_EXTENSION));
	reloadCombo(experimentCombo.GetSafeHwnd(), FILE_SYSTEM_PATH, "*", currentProfile.experiment);
}

void ProfileSystem::openExperiment(std::string experimentToOpen, ScriptingWindow* scriptWindow, MainWindow* mainWin)
{
	// this gets called from the file menu.
	// Assign based on the comboBox Item entry.
	std::string path = FILE_SYSTEM_PATH + experimentToOpen + "\\" + experimentToOpen + EXPERIMENT_EXTENSION;
	std::ifstream experimentConfigOpenFile(cstr(path));
	// check if opened correctly.
	if (!experimentConfigOpenFile.is_open())
	{
		thrower( "Opening of Experiment Configuration File Failed!" );
	}
	currentProfile.experiment = experimentToOpen;
	currentProfile.experimentPath = FILE_SYSTEM_PATH + experimentToOpen + "\\";
	reloadCombo(categoryCombo.GetSafeHwnd(), currentProfile.experimentPath, "*", "__NONE__");
	// since no category is currently loaded...
	SendMessage(configCombo.GetSafeHwnd(), CB_RESETCONTENT, 0, 0);
	currentProfile.category = "";
	currentProfile.configuration = "";
	// it'd be confusing if this category-specific text remained after the category get set to blank.
	mainWin->setNotes("category", "");
	mainWin->setNotes("configuration", "");

	// no category saved currently.
	currentProfile.categoryPath = currentProfile.experimentPath;
	updateExperimentSavedStatus(true);
	/// Set the Configuration combobox.
	// Get all files in the relevant directory.
	std::vector<std::string> configurationNames;
	// reloadCombo(experimentCombo.hwnd, experimentPath, CATEGORY_EXTENSION, "__NONE__");
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
		mainWin->setNotes("experiment", notes);
	}
	else
	{
		mainWin->setNotes("experiment", "");
	}
	scriptWindow->updateProfile("");
	// close.
	experimentConfigOpenFile.close();
	updateExperimentSavedStatus(true);
}


void ProfileSystem::updateExperimentSavedStatus(bool isSaved)
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


void ProfileSystem::experimentSettingsReadyCheck(MainWindow* mainWin)
{
	if (!experimentIsSaved)
	{
		if (checkExperimentSave("There are unsaved Experiment settings. Would you like to save the current experimnet before starting?", mainWin))
		{
			// canceled
			return; // ???
		}
	}
	return;
}


bool ProfileSystem::checkExperimentSave(std::string prompt, MainWindow* mainWin)
{
	if (!this->experimentIsSaved)
	{
		int answer = promptBox(prompt, MB_YESNOCANCEL);
		if (answer == IDYES)
		{
			saveExperimentOnly(mainWin);
		}
		else if (answer == IDCANCEL)
		{
			return true;
		}
	}
	return false;
}


void ProfileSystem::experimentChangeHandler(ScriptingWindow* scriptWindow, MainWindow* mainWin)
{
	if (!experimentIsSaved)
	{
		if (checkExperimentSave("The current experiment is unsaved. Save Current Experiment before Changing?", mainWin))
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
	experimentCombo.GetLBText(int(itemIndex), experimentConfigToOpen);
	openExperiment( str( experimentConfigToOpen ), scriptWindow, mainWin );
	reloadSequence(NULL_SEQUENCE);
}


void ProfileSystem::loadNullSequence()
{
	currentProfile.sequence = NULL_SEQUENCE;
	// only current configuration loaded
	currentProfile.sequenceConfigNames.clear();
	if (currentProfile.configuration != "")
	{
		currentProfile.sequenceConfigNames.push_back(currentProfile.configuration + CONFIG_EXTENSION );
		// change edit
		sequenceInfoDisplay.SetWindowTextA("Sequence of Configurations to Run:\r\n");
		appendText(("1. " + this->currentProfile.sequenceConfigNames[0] + "\r\n"), sequenceInfoDisplay);
	}
	else
	{
		sequenceInfoDisplay.SetWindowTextA("Sequence of Configurations to Run:\r\n");
		appendText("No Configuration Loaded\r\n", sequenceInfoDisplay);
	}
	sequenceCombo.SelectString(0, NULL_SEQUENCE);
	updateSequenceSavedStatus(true);
}


void ProfileSystem::addToSequence(CWnd* parent)
{
	if (currentProfile.configuration == "")
	{
		// nothing to add.
		return;
	}
	currentProfile.sequenceConfigNames.push_back(currentProfile.configuration + CONFIG_EXTENSION );
	// add text to display.
	appendText( str( currentProfile.sequenceConfigNames.size() ) + ". "
				+ currentProfile.sequenceConfigNames.back() + "\r\n", sequenceInfoDisplay );
	updateSequenceSavedStatus(false);
}

/// SEQUENCE HANDLING
void ProfileSystem::sequenceChangeHandler()
{
	// get the name
	long long itemIndex = sequenceCombo.GetCurSel(); 
	TCHAR sequenceName[256];
	sequenceCombo.GetLBText(int(itemIndex), sequenceName);
	if (itemIndex == -1)
	{
		// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
		// is blank. just break out, this is fine.
		return;
	}
	if (str(sequenceName) == NULL_SEQUENCE)
	{
		loadNullSequence();
		return;
	}
	else
	{
		openSequence(sequenceName);
	}
	// else not null_sequence.
	reloadSequence(currentProfile.sequence);
	updateSequenceSavedStatus(true);
}


void ProfileSystem::reloadSequence(std::string sequenceToReload)
{
	reloadCombo(sequenceCombo.GetSafeHwnd(), currentProfile.categoryPath, str("*") + SEQUENCE_EXTENSION, sequenceToReload);
	sequenceCombo.AddString(NULL_SEQUENCE);
	if (sequenceToReload == NULL_SEQUENCE)
	{
		loadNullSequence();
	}
}


void ProfileSystem::saveSequence()
{
	if (currentProfile.category == "")
	{
		if (currentProfile.experiment == "")
		{
			thrower("Please set category and experiment before saving sequence.");
		}
		else
		{
			thrower("Please set category before saving sequence.");
		}
	}
	if (currentProfile.sequence == NULL_SEQUENCE)
	{
		// nothing to save;
		return;
	}
	// if not saved...
	if (currentProfile.sequence == "")
	{
		std::string result;
		TextPromptDialog dialog(&result, "Please enter a new name for this sequence.");
		dialog.DoModal();

		if (result == "")
		{
			return;
		}
		currentProfile.sequence = result;
	}
	std::fstream sequenceSaveFile(currentProfile.categoryPath + "\\" + currentProfile.sequence + SEQUENCE_EXTENSION, std::fstream::out);
	if (!sequenceSaveFile.is_open())
	{
		thrower( "ERROR: Couldn't open sequence file for saving!" );
	}
	sequenceSaveFile << "Version: 1.0\n";
	for (UINT sequenceInc = 0; sequenceInc < this->currentProfile.sequenceConfigNames.size(); sequenceInc++)
	{
		sequenceSaveFile << this->currentProfile.sequenceConfigNames[sequenceInc] + "\n";
	}
	sequenceSaveFile.close();
	reloadSequence(currentProfile.sequence);
	updateSequenceSavedStatus(true);
}


void ProfileSystem::saveSequenceAs()
{
	// prompt for name
	std::string result;
	TextPromptDialog dialog(&result, "Please enter a new name for this sequence.");
	dialog.DoModal();
	//
	if (result == "" || result == "")
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
	std::fstream sequenceSaveFile(currentProfile.categoryPath + "\\" + str(result) + SEQUENCE_EXTENSION, std::fstream::out);
	if (!sequenceSaveFile.is_open())
	{
		thrower( "ERROR: Couldn't open sequence file for saving!" );
	}
	currentProfile.sequence = str(result);
	sequenceSaveFile << "Version: 1.0\n";
	for (UINT sequenceInc = 0; sequenceInc < currentProfile.sequenceConfigNames.size(); sequenceInc++)
	{
		sequenceSaveFile << currentProfile.sequenceConfigNames[sequenceInc] + "\n";
	}
	sequenceSaveFile.close();
	updateSequenceSavedStatus(true);
}


void ProfileSystem::renameSequence()
{
	// check if configuration has been set yet.
	if (currentProfile.sequence == "" || currentProfile.sequence == NULL_SEQUENCE)
	{
		thrower( "Please select a sequence for renaming." );
	}
	std::string newSequenceName;
	TextPromptDialog dialog(&newSequenceName, "Please enter a new name for this sequence.");
	dialog.DoModal();
	if (newSequenceName == "")
	{
		// canceled
		return;
	}
	int result = MoveFile( cstr(currentProfile.categoryPath + currentProfile.sequence + SEQUENCE_EXTENSION),
						   cstr(currentProfile.categoryPath + newSequenceName + SEQUENCE_EXTENSION) );
	if (result == 0)
	{
		thrower( "Renaming of the sequence file Failed! Ask Mark about bugs" );
	}
	currentProfile.sequence = newSequenceName;
	reloadSequence( currentProfile.sequence );
	updateSequenceSavedStatus( true );
}


void ProfileSystem::deleteSequence()
{
	// check if configuration has been set yet.
	if (currentProfile.sequence == "" || currentProfile.sequence == NULL_SEQUENCE)
	{
		thrower("Please select a sequence for deleting.");
	}
	int answer = promptBox("Are you sure you want to delete the current sequence: " + currentProfile.sequence, 
							 MB_YESNO);
	if (answer == IDNO)
	{
		return;
	}
	std::string currentSequenceLocation = currentProfile.categoryPath + currentProfile.sequence + SEQUENCE_EXTENSION;
	int result = DeleteFile(cstr(currentSequenceLocation));
	if (result == 0)
	{
		thrower( "ERROR: Deleteing the configuration file failed!" );
	}
	// since the sequence this (may have been) was saved to is gone, no saved version of current code.
	updateSequenceSavedStatus(false);
	// just deleted the current configuration
	currentProfile.sequence = "";
	// reset combo since the files have now changed after delete
	reloadSequence("__NONE__");
}


void ProfileSystem::newSequence(CWnd* parent)
{
	// prompt for name
	std::string result;
	TextPromptDialog dialog(&result, "Please enter a new name for this sequence.");
	dialog.DoModal();

	if (result == "")
	{
		// user canceled or entered nothing
		return;
	}
	// try to open the file.
	std::fstream sequenceFile(currentProfile.categoryPath + "\\" + result + SEQUENCE_EXTENSION, std::fstream::out);
	if (!sequenceFile.is_open())
	{
		thrower( "Couldn't create a file with this sequence name! Make sure there are no forbidden characters in your name." );
	}
	std::string newSequenceName = str(result);
	sequenceFile << newSequenceName + "\n";
	// output current configuration
	if (newSequenceName == "")
	{
		return;
	}
	// reload combo.
	reloadSequence(currentProfile.sequence);
}


void ProfileSystem::openSequence(std::string sequenceName)
{
	// try to open the file
	std::fstream sequenceFile(currentProfile.categoryPath + sequenceName + SEQUENCE_EXTENSION);
	if (!sequenceFile.is_open())
	{
		thrower("ERROR: sequence file failed to open! Make sure the sequence with address ..." 
				 + currentProfile.categoryPath + sequenceName + SEQUENCE_EXTENSION + " exists.");
	}
	currentProfile.sequence = str(sequenceName);
	// read the file
	std::string version;
	std::getline(sequenceFile, version);
	//
	currentProfile.sequenceConfigNames.clear();
	std::string tempName;
	getline(sequenceFile, tempName);
	while (sequenceFile)
	{
		currentProfile.sequenceConfigNames.push_back(tempName);
		getline(sequenceFile, tempName);
	}
	// update the edit
	sequenceInfoDisplay.SetWindowTextA("Configuration Sequence:\r\n");
	for (UINT sequenceInc = 0; sequenceInc < currentProfile.sequenceConfigNames.size(); sequenceInc++)
	{
		appendText( str( sequenceInc + 1 ) + ". " + currentProfile.sequenceConfigNames[sequenceInc] + "\r\n",
					sequenceInfoDisplay );
	}
	updateSequenceSavedStatus(true);
}


void ProfileSystem::updateSequenceSavedStatus(bool isSaved)
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


bool ProfileSystem::sequenceSettingsReadyCheck()
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


bool ProfileSystem::checkSequenceSave(std::string prompt)
{
	if (!sequenceIsSaved)
	{
		int answer = promptBox(prompt, MB_YESNOCANCEL);
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


std::vector<std::string> ProfileSystem::getSequenceNames()
{
	return currentProfile.sequenceConfigNames;
}


std::string ProfileSystem::getSequenceNamesString()
{
	std::string namesString = "";
	if (currentProfile.sequence != "NO SEQUENCE")
	{
		namesString += "Sequence:\r\n";
		for (UINT sequenceInc = 0; sequenceInc < currentProfile.sequenceConfigNames.size(); sequenceInc++)
		{
			namesString += "\t" + str(sequenceInc) + ": " + currentProfile.sequenceConfigNames[sequenceInc] + "\r\n";
		}
	}
	return namesString;
}


std::string ProfileSystem::getMasterAddressFromConfig()
{
	std::string configurationAddress;
	configurationAddress = currentProfile.categoryPath + currentProfile.configuration + CONFIG_EXTENSION;
	std::fstream configFile(configurationAddress);
	if (!configFile.is_open())
	{
		thrower("ERROR: Failed to open configuration file.");
	}
	// get the first couple lines...
	std::string line, word, address;
	std::getline(configFile, line);
	std::getline(configFile, line);
	std::getline(configFile, line);
	std::getline(configFile, line);
	std::string newPath;
	getline(configFile, newPath);
	return newPath;
}


void ProfileSystem::initialize( POINT& pos, CWnd* parent, int& id, cToolTips& tooltips )
{
	// Experiment Type
	experimentLabel.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 25 };
	experimentLabel.Create( "EXPERIMENT", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, experimentLabel.sPos,
							parent, id++ );
	experimentLabel.fontType = HeadingFont;
	// Experiment Saved Indicator
	experimentSavedIndicator.sPos = { pos.x + 360, pos.y, pos.x + 480, pos.y + 25 };
	experimentSavedIndicator.Create( "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT,
									 experimentSavedIndicator.sPos, parent, id++ );
	experimentSavedIndicator.SetCheck( BST_CHECKED );
	updateExperimentSavedStatus( true );
	// Category Title
	categoryLabel.sPos = { pos.x + 480, pos.y, pos.x + 960, pos.y + 25 };
	categoryLabel.Create( "CATEGORY", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, categoryLabel.sPos, parent, id++);
	categoryLabel.fontType = HeadingFont;
	//
	categorySavedIndicator.sPos = { pos.x + 480 + 380, pos.y, pos.x + 960, pos.y + 25 };
	categorySavedIndicator.Create( "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT,
								   categorySavedIndicator.sPos, parent, id++ );
	categorySavedIndicator.SetCheck( BST_CHECKED );
	updateCategorySavedStatus( true );
	pos.y += 25;
	// Experiment Combo
	experimentCombo.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 800 };
	experimentCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_TABSTOP,
							experimentCombo.sPos, parent, IDC_EXPERIMENT_COMBO );
	reloadCombo( experimentCombo.GetSafeHwnd(), PROFILES_PATH, str( "*" ), "__NONE__" );
	// Category Combo
	categoryCombo.sPos = { pos.x + 480, pos.y, pos.x + 960, pos.y + 800 };
	categoryCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_TABSTOP,
						  categoryCombo.sPos, parent, IDC_CATEGORY_COMBO );
	pos.y += 25;
	// Configuration Title
	configLabel.sPos = { pos.x, pos.y, pos.x + 960, pos.y + 25 };
	configLabel.Create( "CONFIGURATION", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, configLabel.sPos, parent, id++);
	configLabel.fontType = HeadingFont;
	// Configuration Saved Indicator
	configurationSavedIndicator.sPos = { pos.x + 860, pos.y, pos.x + 960, pos.y + 25 };
	configurationSavedIndicator.Create( "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT,
										configurationSavedIndicator.sPos, parent, id++);
	configurationSavedIndicator.SetCheck( BST_CHECKED );
	updateConfigurationSavedStatus( true );
	pos.y += 25;
	// configuration combo
	configCombo.sPos = { pos.x, pos.y, pos.x + 960, pos.y + 800 };
	configCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_TABSTOP, 
					    configCombo.sPos, parent, IDC_CONFIGURATION_COMBO );
	pos.y += 25;
	/// SEQUENCE
	sequenceLabel.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 25 };
	sequenceLabel.Create( "SEQUENCE", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, sequenceLabel.sPos, parent, id++);
	sequenceLabel.fontType = HeadingFont;
	pos.y += 25;
	// combo
	sequenceCombo.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 800 };
	sequenceCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
						  sequenceCombo.sPos, parent, IDC_SEQUENCE_COMBO );
	sequenceCombo.AddString( "NULL SEQUENCE" );
	sequenceCombo.SetCurSel( 0 );
	sequenceCombo.SetItemHeight(0, 50);
	pos.y += 25;
	// display
	sequenceInfoDisplay.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 100 };
	sequenceInfoDisplay.Create( ES_READONLY | WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL,
								sequenceInfoDisplay.sPos, parent, id++);
	sequenceInfoDisplay.SetWindowTextA( "Sequence of Configurations to Run:\r\n" );
	sequenceSavedIndicator.sPos = { pos.x + 860, pos.y, pos.x + 960, pos.y + 20 };
	// saved indicator
	sequenceSavedIndicator.Create( "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT,
								   sequenceSavedIndicator.sPos, parent, id++);
	sequenceSavedIndicator.SetCheck( BST_CHECKED );
	updateSequenceSavedStatus( true );
}


void ProfileSystem::rearrange(int width, int height, fontMap fonts)
{
	configLabel.rearrange( width, height, fonts);
	configCombo.rearrange( width, height, fonts);
	experimentLabel.rearrange( width, height, fonts);
	experimentCombo.rearrange( width, height, fonts);
	categoryLabel.rearrange( width, height, fonts);
	categoryCombo.rearrange( width, height, fonts);
	sequenceLabel.rearrange( width, height, fonts);
	sequenceCombo.rearrange( width, height, fonts);
	sequenceInfoDisplay.rearrange( width, height, fonts);
	sequenceSavedIndicator.rearrange( width, height, fonts);
	orientationLabel.rearrange( width, height, fonts);
	orientationCombo.rearrange( width, height, fonts);
	categorySavedIndicator.rearrange( width, height, fonts);
	configurationSavedIndicator.rearrange( width, height, fonts);
	experimentSavedIndicator.rearrange( width, height, fonts);
}


std::vector<std::string> ProfileSystem::searchForFiles( std::string locationToSearch, std::string extensions )
{
	// Re-add the entries back in and figure out which one is the current one.
	std::vector<std::string> names;
	std::string search_path = locationToSearch + "\\" + extensions;
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	if (extensions == "*")
	{
		hFind = FindFirstFileEx( cstr(search_path), FindExInfoStandard, &fd, FindExSearchLimitToDirectories, NULL, 0 );
	}
	else
	{
		hFind = FindFirstFile( cstr(search_path), &fd );
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
					if (str( fd.cFileName ) != "." && str( fd.cFileName ) != "..")
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
	for (UINT configListInc = 0; configListInc < names.size(); configListInc++)
	{
		if (extensions == "*" || extensions == "*.*" || extensions == str( "*" ) + HORIZONTAL_EXTENSION
			 || extensions == str( "*" ) + VERTICAL_EXTENSION || extensions == str( "*" ) + SEQUENCE_EXTENSION
			 || extensions == str( "*" ) + CATEGORY_EXTENSION || extensions == str( "*" ) + EXPERIMENT_EXTENSION
			|| extensions == str("*") + PLOTTING_EXTENSION || extensions == str( "*" ) + CONFIG_EXTENSION || extensions == str("*") + FUNCTION_EXTENSION )
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

// I had issues writing an MFC version of this with a Control<CComboBox> argument, so this is still written in Win32.
void ProfileSystem::reloadCombo( HWND comboToReload, std::string locationToLook, std::string extension,
								 std::string nameToLoad )
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
	for (UINT comboInc = 0; comboInc < names.size(); comboInc++)
	{
		if (nameToLoad == names[comboInc])
		{
			currentInc = comboInc;
		}
		SendMessage( comboToReload, CB_ADDSTRING, 0, (LPARAM)cstr(names[comboInc]));
	}
	// Set initial value
	SendMessage( comboToReload, CB_SETCURSEL, currentInc, 0 );
}

bool ProfileSystem::fileOrFolderExists(std::string filePathway)
{
	// got this from stack exchange. dunno how it works but it should be fast.
	struct stat buffer;
	return (stat(cstr(filePathway), &buffer) == 0);
}

std::string ProfileSystem::getComboText()
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


void ProfileSystem::fullyDeleteFolder(std::string folderToDelete)
{
	// this used to call SHFileOperation. Boost is better. Much better. 
	uintmax_t filesRemoved = boost::filesystem::remove_all(cstr(folderToDelete));
	if (filesRemoved == 0)
	{
		thrower("Delete Failed! Ask mark about bugs.");
	}
}


std::string ProfileSystem::getCurrentCategory()
{
	return currentProfile.category;
}

std::string ProfileSystem::getCurrentExperiment()
{
	return currentProfile.experiment;
}

profileSettings ProfileSystem::getProfileSettings()
{
	return currentProfile;
}
