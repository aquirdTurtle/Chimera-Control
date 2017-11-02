#include "stdafx.h"

#include "ProfileSystem.h"
#include "TextPromptDialog.h"
#include "NiawgController.h"
#include "AuxiliaryWindow.h"
#include "Andor.h"
#include "CameraWindow.h"
#include "openWithExplorer.h"
#include "saveWithExplorer.h"
#include <fstream>
#include "Commctrl.h"
#include <boost/filesystem.hpp>


ProfileSystem::ProfileSystem(std::string fileSystemPath)
{
	FILE_SYSTEM_PATH = fileSystemPath;
	currentProfile.sequence = NULL_SEQUENCE;
}


void ProfileSystem::initialize( POINT& pos, CWnd* parent, int& id, cToolTips& tooltips )
{
	// 
	configDisplay.sPos = { pos.x, pos.y, pos.x + 860, pos.y + 25 };
	configDisplay.Create( "No Configuration Selected!", NORM_STATIC_OPTIONS, configDisplay.sPos, parent, id++ );
	configurationSavedIndicator.sPos = { pos.x + 860, pos.y, pos.x + 960, pos.y += 25 };
	configurationSavedIndicator.Create( "Saved?", NORM_CHECK_OPTIONS | BS_LEFTTEXT,
										configurationSavedIndicator.sPos, parent, id++ );
	configurationSavedIndicator.SetCheck( BST_CHECKED );
	updateConfigurationSavedStatus( true );
	selectConfigButton.sPos = { pos.x + 480, pos.y, pos.x + 960, pos.y + 25 };
	selectConfigButton.Create( "Open Configuration", NORM_PUSH_OPTIONS, selectConfigButton.sPos, parent, 
							   IDC_SELECT_CONFIG_COMBO );
	/// SEQUENCE
	sequenceLabel.sPos = { pos.x, pos.y, pos.x + 380, pos.y + 25 };
	sequenceLabel.Create( "SEQUENCE", NORM_STATIC_OPTIONS, sequenceLabel.sPos, parent, id++ );
	sequenceLabel.fontType = HeadingFont;
	sequenceSavedIndicator.sPos = { pos.x + 380, pos.y, pos.x + 470, pos.y += 25 };
	sequenceSavedIndicator.Create( "Saved?", NORM_CWND_OPTIONS | BS_CHECKBOX | BS_LEFTTEXT,
								   sequenceSavedIndicator.sPos, parent, id++ );
	sequenceSavedIndicator.SetCheck( BST_CHECKED );
	updateSequenceSavedStatus( true );
	// combo
	sequenceCombo.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 800 };
	sequenceCombo.Create( NORM_COMBO_OPTIONS, sequenceCombo.sPos, parent, IDC_SEQUENCE_COMBO);
	sequenceCombo.AddString( "NULL SEQUENCE" );
	sequenceCombo.SetCurSel( 0 );
	sequenceCombo.SetItemHeight( 0, 50 );
	pos.y += 25;
	// display
	sequenceInfoDisplay.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 100 };
	sequenceInfoDisplay.Create( NORM_STATIC_OPTIONS | ES_CENTER | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL,
								sequenceInfoDisplay.sPos, parent, id++ );
	sequenceInfoDisplay.SetWindowTextA( "Sequence of Configurations to Run:\r\n" );

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
	saveConfigurationOnly( scriptWindow, mainWin, auxWindow, camWin);
	saveSequence();		
}


void ProfileSystem::checkSaveEntireProfile(ScriptingWindow* scriptWindow, MainWindow* mainWin, 
													 AuxiliaryWindow* auxWin, CameraWindow* camWin)
{
	//checkExperimentSave( "Save Experiment Settings?", mainWin);
	//checkCategorySave( "Save Category Settings?", mainWin );
	checkConfigurationSave( "Save Configuration Settings?", scriptWindow, mainWin, auxWin, camWin);
	checkSequenceSave( "Save Sequence Settings?" );
}


void ProfileSystem::allSettingsReadyCheck(ScriptingWindow* scriptWindow, MainWindow* mainWin, 
													AuxiliaryWindow* auxWin, CameraWindow* camWin)
{
	// check all components of this class.
	configurationSettingsReadyCheck( scriptWindow, mainWin, auxWin, camWin);
	sequenceSettingsReadyCheck();
	// passed all checks.
}

/// CONFIGURATION LEVEL HANDLING

void ProfileSystem::newConfiguration( MainWindow* mainWin, AuxiliaryWindow* auxWin, CameraWindow* camWin, 
									  ScriptingWindow* scriptWin )
{
	std::string newConfigPath = openWithExplorer(mainWin, "Config");
	if (newConfigPath == "")
	{
		// canceled
		return;
	}
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
}


void ProfileSystem::openConfigFromPath( std::string pathToConfig, ScriptingWindow* scriptWin, MainWindow* mainWin,
										CameraWindow* camWin, AuxiliaryWindow* auxWin )
{
	std::ifstream configFile( pathToConfig );
	// check if opened correctly.
	if ( !configFile.is_open( ) )
	{
		thrower( "Opening of Configuration File Failed!" );
	}
	int slashPos = pathToConfig.find_last_of( '\\' );
	int extensionPos = pathToConfig.find_last_of( '.' );
	currentProfile.configuration = pathToConfig.substr( slashPos + 1, extensionPos - slashPos - 1 );
	currentProfile.categoryPath = pathToConfig.substr( 0, slashPos);
	slashPos = currentProfile.categoryPath.find_last_of( '\\' );
	currentProfile.parentFolderName = currentProfile.categoryPath.substr( slashPos + 1, 
																		  currentProfile.categoryPath.size( ) );
	currentProfile.categoryPath += "\\";
	configDisplay.SetWindowTextA( cstr( currentProfile.configuration + ": " + pathToConfig ) );
	std::string versionStr;
	try
	{
		// Version is saved in format "Master Version: x.x"
		// eat the "version" word"
		configFile >> versionStr;
		configFile >> versionStr;
		double version;
		int versionMajor, versionMinor;
		try
		{
			version = std::stod( versionStr );
			int periodPos = versionStr.find_last_of( '.' );
			std::string tempStr( versionStr.substr( 0, periodPos ));
			versionMajor = std::stod( tempStr );
			tempStr = versionStr.substr( periodPos + 1, versionStr.size( ) );
			versionMinor = std::stod( tempStr );
		}
		catch ( std::invalid_argument& )
		{
			thrower( "ERROR: Version string failed to convert to double while opening configuration!" );
		}
		scriptWin->handleOpenConfig( configFile, versionMajor, versionMinor );
		camWin->handleOpeningConfig( configFile, versionMajor, versionMinor );
		auxWin->handleOpeningConfig( configFile, versionMajor, versionMinor );
		mainWin->handleOpeningConfig( configFile, versionMajor, versionMinor );
	}
	catch ( Error& err )
	{
		errBox( "ERROR: Failed to open configuration file! Error was:\r\n" + err.whatStr( ) + "\r\nThe code will now "
				"open the config file in atom so that you can attempt to fix the issue." );
		ShellExecute( 0, "open", cstr( pathToConfig ), NULL, NULL, NULL );
	}
	/// finish up
	updateConfigurationSavedStatus( true );
	auxWin->setVariablesActiveState( true );
	// actually set this now
	scriptWin->updateProfile( currentProfile.parentFolderName + "->" + currentProfile.configuration );
	// close.
	configFile.close( );
	if ( currentProfile.sequence == NULL_SEQUENCE )
	{
		// reload it.
		loadNullSequence( );
	}
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
	// check to make sure that this is a name.
	if (configNameToSave == "")
	{
		thrower( "ERROR: The program requested saving the configuration file to an empty name! This shouldn't happen, ask Mark "
				 "about bugs." );
	}
	// check if file already exists
	if (!ProfileSystem::fileOrFolderExists(currentProfile.categoryPath + configNameToSave + "." + CONFIG_EXTENSION))  
	{
		int answer = promptBox("This configuration file appears to not exist in the expected location: " 
								+ currentProfile.categoryPath + configNameToSave 
								+ "." + CONFIG_EXTENSION + ". Continue by making a new configuration file?", MB_OKCANCEL );
		if (answer == IDCANCEL)
		{
			return;
		}
	}

	std::ofstream configSaveFile(currentProfile.categoryPath + configNameToSave + "." + CONFIG_EXTENSION);
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
	std::string configurationPathToSave = saveWithExplorer( mainWin, CONFIG_EXTENSION, currentProfile );
	if ( configurationPathToSave == "")
	{
		// canceled
		return;
	}	
	// check if file already exists
	std::ofstream configurationSaveFile( configurationPathToSave);
	if (!configurationSaveFile.is_open())
	{
		thrower( "Couldn't save configuration file! Check the name for weird characters, or call Mark about bugs if "
				 "everything seems right..." );
	}
	int slashPos = configurationPathToSave.find_last_of( '\\' );
	int extensionPos = configurationPathToSave.find_last_of( '.' );
	currentProfile.configuration = configurationPathToSave.substr( slashPos + 1, extensionPos - slashPos - 1 );
	currentProfile.categoryPath = configurationPathToSave.substr( 0, slashPos );
	slashPos = currentProfile.categoryPath.find_last_of( '\\' );
	currentProfile.parentFolderName = currentProfile.categoryPath.substr( slashPos + 1,
																		  currentProfile.categoryPath.size( ) );
	currentProfile.categoryPath += "\\";
	// That's the last prompt the user gets, so the save is final now.
	// Version info tells future code about formatting.
	configurationSaveFile << "Version: "+ str(versionMain) + "." + str(versionSub) + "\n";
	scriptInfo<std::string> addresses = scriptWindow->getScriptAddresses();
	// order matters!
	configurationSaveFile << addresses.verticalNIAWG << "\n";
	configurationSaveFile << addresses.horizontalNIAWG << "\n";
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
	std::string notes = mainWin->getNotes();
	configurationSaveFile << notes + "\n";
	configurationSaveFile << "END CONFIGURATION NOTES" << "\n";
	configurationSaveFile.close();
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
		thrower( "The Configuration has not yet been selected! Please select a category or create a new one before "
					"trying to rename it." );
	}

	std::string newConfigurationName;
	TextPromptDialog dialog(&newConfigurationName, "Please enter new configuration name.");
	dialog.DoModal();

	if (newConfigurationName == "")
	{
		// canceled
		return;
	}
	std::string currentConfigurationLocation = currentProfile.categoryPath + currentProfile.configuration + "." 
		+ CONFIG_EXTENSION;
	std::string newConfigurationLocation = currentProfile.categoryPath + newConfigurationName + "." + CONFIG_EXTENSION;
	int result = MoveFile(cstr(currentConfigurationLocation), cstr(newConfigurationLocation));
	if (result == 0)
	{
		thrower( "Renaming of the configuration file Failed! Ask Mark about bugs" );
	}
	currentProfile.configuration = newConfigurationName;
}


/*
]--- 
*/
void ProfileSystem::deleteConfiguration()
{
	// check if configuration has been set yet.
	if (currentProfile.configuration == "")
	{
		thrower( "The Configuration has not yet been selected! Please select a category or create a new one before "
				 "trying to rename it." );
	}
	int answer = promptBox("Are you sure you want to delete the current configuration: " 
								 + currentProfile.configuration, MB_YESNO);
	if (answer == IDNO)
	{
		return;
	}
	std::string currentConfigurationLocation = currentProfile.categoryPath + currentProfile.configuration + "." + CONFIG_EXTENSION;
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


bool ProfileSystem::checkConfigurationSave( std::string prompt, ScriptingWindow* scriptWindow, MainWindow* mainWin, 
											AuxiliaryWindow* auxWin, CameraWindow* camWin )
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


std::string ProfileSystem::getCurrentPathIncludingCategory()
{
	return currentProfile.categoryPath;
}


void ProfileSystem::handleSelectConfigButton(CWnd* parent, ScriptingWindow* scriptWindow, MainWindow* mainWin,
											  AuxiliaryWindow* auxWin, CameraWindow* camWin )
{	
	if ( !configurationIsSaved )
	{
		if ( checkConfigurationSave( "The current configuration is unsaved. Save current configuration before changing?",
									 scriptWindow, mainWin, auxWin, camWin ) )
		{
			// TODO
			return;
		}
	}
	std::string fileaddress = openWithExplorer( parent, CONFIG_EXTENSION );
	openConfigFromPath( fileaddress, scriptWindow, mainWin, camWin, auxWin );	
}



void ProfileSystem::loadNullSequence()
{
	currentProfile.sequence = NULL_SEQUENCE;
	// only current configuration loaded
	currentProfile.sequenceConfigNames.clear();
	if (currentProfile.configuration != "")
	{
		currentProfile.sequenceConfigNames.push_back(currentProfile.configuration + "." + CONFIG_EXTENSION );
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
	currentProfile.sequenceConfigNames.push_back(currentProfile.configuration + "." + CONFIG_EXTENSION );
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
		// This means that the oreintation combo was set before the Configuration list combo was set so that the 
		// configuration list combo is blank. just break out, this is fine.
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
	reloadCombo(sequenceCombo.GetSafeHwnd(), currentProfile.categoryPath, str("*") + "." + SEQUENCE_EXTENSION, sequenceToReload);
	sequenceCombo.AddString(NULL_SEQUENCE);
	if (sequenceToReload == NULL_SEQUENCE)
	{
		loadNullSequence();
	}
}


void ProfileSystem::saveSequence()
{
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
	std::fstream sequenceSaveFile( currentProfile.categoryPath + "\\" + currentProfile.sequence + "." 
								   + SEQUENCE_EXTENSION, std::fstream::out);
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
	std::fstream sequenceSaveFile(currentProfile.categoryPath + "\\" + str(result) + "." + SEQUENCE_EXTENSION, 
								   std::fstream::out);
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
	int result = MoveFile( cstr(currentProfile.categoryPath + currentProfile.sequence + "." + SEQUENCE_EXTENSION),
						   cstr(currentProfile.categoryPath + newSequenceName + "." + SEQUENCE_EXTENSION) );
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
	std::string currentSequenceLocation = currentProfile.categoryPath + currentProfile.sequence + "." 
		+ SEQUENCE_EXTENSION;
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
	std::fstream sequenceFile(currentProfile.categoryPath + "\\" + result + "." + SEQUENCE_EXTENSION, std::fstream::out);
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
	std::fstream sequenceFile(currentProfile.categoryPath + sequenceName + "." + SEQUENCE_EXTENSION);
	if (!sequenceFile.is_open())
	{
		thrower("ERROR: sequence file failed to open! Make sure the sequence with address ..." 
				 + currentProfile.categoryPath + sequenceName + "." + SEQUENCE_EXTENSION + " exists.");
	}
	currentProfile.sequence = str(sequenceName);
	// read the file
	std::string version;
	std::getline(sequenceFile, version);
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
		if (checkSequenceSave("There are unsaved sequence settings. Would you like to save the current sequence before"
							   " starting?"))
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
	configurationAddress = currentProfile.categoryPath + currentProfile.configuration + "." + CONFIG_EXTENSION;
	std::fstream configFile(configurationAddress);
	if (!configFile.is_open())
	{
		thrower("ERROR: Failed to open configuration file.");
	}
	std::string line, word, address;
	std::getline(configFile, line);
	std::getline(configFile, line);
	std::getline(configFile, line);
	std::getline(configFile, line);
	std::string newPath;
	getline(configFile, newPath);
	return newPath;
}


void ProfileSystem::rearrange(int width, int height, fontMap fonts)
{
	sequenceLabel.rearrange( width, height, fonts);
	sequenceCombo.rearrange( width, height, fonts);
	sequenceInfoDisplay.rearrange( width, height, fonts);
	sequenceSavedIndicator.rearrange( width, height, fonts);
	configurationSavedIndicator.rearrange( width, height, fonts);
	configDisplay.rearrange( width, height, fonts );
	selectConfigButton.rearrange( width, height, fonts );
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
		if (extensions == "*" || extensions == "*.*" || extensions == str( "*." ) + SEQUENCE_EXTENSION
			|| extensions == str("*.") + PLOTTING_EXTENSION || extensions == str( "*." ) + CONFIG_EXTENSION 
			 || extensions == str("*.") + FUNCTION_EXTENSION )
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


void ProfileSystem::fullyDeleteFolder(std::string folderToDelete)
{
	// this used to call SHFileOperation. Boost is better. Much better. 
	uintmax_t filesRemoved = boost::filesystem::remove_all(cstr(folderToDelete));
	if (filesRemoved == 0)
	{
		thrower("Delete Failed! Ask mark about bugs.");
	}
}


profileSettings ProfileSystem::getProfileSettings()
{
	return currentProfile;
}

