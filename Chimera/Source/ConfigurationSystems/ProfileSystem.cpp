// created by Mark O. Brown
#include "stdafx.h"
#include <boost/filesystem.hpp>
#include <fstream>

#include "ConfigurationSystems/ProfileSystem.h"
#include "NIAWG/NiawgCore.h"
#include "Andor/AndorCameraCore.h"
#include "PrimaryWindows/QtAuxiliaryWindow.h"
#include "PrimaryWindows/QtAndorWindow.h"
#include "PrimaryWindows/QtScriptWindow.h"
#include "PrimaryWindows/QtBaslerWindow.h"
#include "PrimaryWindows/QtMainWindow.h"
#include "PrimaryWindows/QtDeformableMirrorWindow.h"
#include "ExcessDialogs/openWithExplorer.h"
#include "ExcessDialogs/saveWithExplorer.h"
#include <qdebug.h>
#include "Commctrl.h"

ProfileSystem::ProfileSystem(std::string fileSystemPath, IChimeraQtWindow* parent) : IChimeraSystem(parent) {
	FILE_SYSTEM_PATH = fileSystemPath;
}

void ProfileSystem::initialize( POINT& pos, IChimeraQtWindow* win){
	configDisplay = new QLabel ("No Configuruation Selected!", win);
	configDisplay->setStyleSheet(" QLabel{ font: bold 8pt; }");
	configDisplay->setGeometry (QRect (pos.x, pos.y, 700, 25));
	configurationSavedIndicator = new QCheckBox ("Saved?", win);
	configurationSavedIndicator->setGeometry (QRect (pos.x + 860, pos.y, 100, 25));
	configurationSavedIndicator->setChecked (true);
	selectConfigButton = new QPushButton ("Open Config.", win);
	selectConfigButton->setGeometry (QRect (pos.x + 700, pos.y, 160, 25));
	win->connect (selectConfigButton, &QPushButton::released, [this, win]() {
		try {
			handleSelectConfigButton (win);
		}
		catch (ChimeraError & err) {
			emit error (err.qtrace ());
		}});
	pos.y += 25;
	updateConfigurationSavedStatus( true );
}

std::string ProfileSystem::getNiawgScriptAddrFromConfig(ConfigStream& configStream){
	// open configuration file and grab the niawg script file address from it.
	initializeAtDelim (configStream, "SCRIPTS");
	configStream.get ();
	auto getlineF = ProfileSystem::getGetlineFunc (configStream.ver);
	std::string niawgScriptAddresses;
	getlineF (configStream, niawgScriptAddresses);
	return niawgScriptAddresses;
}

void ProfileSystem::saveEntireProfile(IChimeraQtWindow* win){
	saveConfigurationOnly( win );
}

void ProfileSystem::checkSaveEntireProfile(IChimeraQtWindow* win){
	checkConfigurationSave( "Save Configuration Settings?", win );
}

/*
Before Version 5.0, if a string or expression was empty it would just output nothing to the config file, leading to 
a lot of empty lines. That was fine, because std::getline would just return "" correctly. However, My ScriptStream 
class and derivatives will eat those empty lines while eating comments, so I had to add an "empty-string" handling
mechanism to the writing of the ConfigStream class. But this creates a back-wards compatibility issue with the older
configurations. This function is a nice way to get around that - when getline is needed for reading strings, use this 
function to get a different version of getline depending on which version the file is. Older versions will use the 
std::getline version while new versions will use the comment-eating version.
*/
std::function<void (ScriptStream&, std::string&)> ProfileSystem::getGetlineFunc (Version& ver){
	if (ver >= Version ("5.0")){
		return [](ScriptStream& fid, std::string& expr) {expr = fid.getline (); };
	}
	else{
		return [](ScriptStream& fid, std::string& expr) {std::getline (fid, expr); };
	}
}

void ProfileSystem::getVersionFromFile( ConfigStream& file ){
	file.clear ( );
	file.seekg ( 0, std::ios::beg );
	std::string versionStr;
	// Version is saved in format "Version: x.x"
	// eat the "version" word"
	file >> versionStr;
	file >> versionStr;
	file.ver = Version( versionStr );
}


void ProfileSystem::openConfigFromPath( std::string pathToConfig, IChimeraQtWindow* win){
	std::ifstream configFileRaw( pathToConfig );
	// check if opened correctly.
	if ( !configFileRaw.is_open( ) ){
		errBox ( "Opening of Configuration File Failed!" );
		return;
	}
	ConfigStream cStream (configFileRaw);
	cStream.setCase (false);
	configFileRaw.close ();
	int slashPos = pathToConfig.find_last_of( '/' );
	int extensionPos = pathToConfig.find_last_of( '.' );
	currentProfile.configuration = pathToConfig.substr( slashPos + 1, extensionPos - slashPos - 1 );
	currentProfile.configLocation = pathToConfig.substr( 0, slashPos);
	slashPos = currentProfile.configLocation.find_last_of( '/' );
	currentProfile.parentFolderName = currentProfile.configLocation.substr( slashPos + 1, 
																		    currentProfile.configLocation.size( ) );
	currentProfile.configLocation += "/";
	std::string dispPath;
	configDisplay->setText (cstr (currentProfile.configuration));
	configDisplay->setToolTip (qstr (currentProfile.configuration + ": " + pathToConfig));
	try	{
		getVersionFromFile(cStream);
		win->scriptWin->windowOpenConfig(cStream );
		win->andorWin->windowOpenConfig(cStream );
		win->auxWin->windowOpenConfig(cStream );
		win->mainWin->windowOpenConfig(cStream );
		if (cStream.ver >= Version ( "3.4" ) ){
			win->basWin->windowOpenConfig (cStream );
		}
		if (cStream.ver >= Version ("5.0")){
			//dmWin->windowOpenConfig (cStream);
		}
	}
	catch ( ChimeraError& err ){
		errBox( "ERROR: Unhandled error while opening configuration files!\n\n" + err.trace() );
	}
	updateConfigurationSavedStatus ( true );
}


void ProfileSystem::initializeAtDelim ( ConfigStream& configStream, std::string delimiter, Version minVer ){
	ProfileSystem::getVersionFromFile ( configStream );
	if ( configStream.ver < minVer ){
		thrower ( "Configuration version (" + configStream.ver.str() +  ") less than minimum version (" + minVer.str() + ")" );
	}
	try{
		ProfileSystem::jumpToDelimiter (configStream, delimiter );
	}
	catch ( ChimeraError& ){
		throwNested ( "Failed to initialize at a delimiter!" );
	}
}


void ProfileSystem::jumpToDelimiter ( ConfigStream& configStream, std::string delimiter ){
	while ( !configStream.eof() ){
		try{
			checkDelimiterLine (configStream, delimiter );
			// if reaches this point it was successful. The file should now be pointing to just beyond the delimiter.
			return;
		}
		catch ( ChimeraError& ){
			if (configStream.peek () == EOF) {
				break;
			}
			// didn't find delimiter, try next input.
		}
	}
	// reached end of file.
	thrower ( "Failed to jump to a delimiter! Delimiter was: " + delimiter + "." );
}

// small convenience function that I use while opening a file.
void ProfileSystem::checkDelimiterLine(ConfigStream& openFile, std::string delimiter){
	std::string checkStr;
	openFile >> checkStr;
	if (checkStr != delimiter){
		thrower ("ERROR: Expected \"" + delimiter + "\" in configuration file, but instead found \"" + checkStr + "\"");
	}
}


// version with break condition. If returns true, calling function should break out of the loop which is checking the
// line.
bool ProfileSystem::checkDelimiterLine(ConfigStream& openFile, std::string delimiter, std::string breakCondition ){
	std::string checkStr;
	openFile >> checkStr;
	if ( checkStr != delimiter ){
		if ( checkStr == breakCondition ){
			return true;
		}
		thrower ( "ERROR: Expected \"" + delimiter + "\" in configuration file, but instead found \"" + checkStr + "\"" );
	}
	return false;
}



/*
]--- This function attempts to save the configuration given the configuration name in the argument. It throws errors and warnings if this 
]- is not a Normal Save, i.e. if the file doesn't already exist or if the user tries to pass an empty name as an argument. It returns 
]- false if the configuration got saved, true if something prevented the configuration from being saved.
*/
void ProfileSystem::saveConfigurationOnly(IChimeraQtWindow* win){
	std::string configNameToSave = currentProfile.configuration;
	// check to make sure that this is a name.
	if (configNameToSave == ""){
		thrower ( "ERROR: Please select a configuration before saving!" );
	}
	// check if file already exists
	if (!ProfileSystem::fileOrFolderExists(currentProfile.configLocation + configNameToSave + "." + CONFIG_EXTENSION)){
		auto answer = QMessageBox::question (win, qstr ("New Config?"),
			qstr ("This configuration file appears to not exist in the expected location: " 
				+ currentProfile.configLocation + configNameToSave
				+ "." + CONFIG_EXTENSION + ". Continue by making a new configuration file?"));
		if (answer == QMessageBox::No) {
			return;
		}
	}

	ConfigStream saveStream;
	// That's the last prompt the user gets, so the save is final now.
	currentProfile.configuration = configNameToSave;
	// version 2.0 started when the unified coding system (the chimera system) began, and the profile system underwent
	// dramatic changes in order to 
	saveStream << std::setprecision( 13 );
	saveStream << "Version: " + version.str() + "\n";
	// give it to each window, allowing each window to save its relevant contents to the config file. Order matters.
	win->scriptWin->windowSaveConfig(saveStream);
	win->andorWin->windowSaveConfig(saveStream);
	win->auxWin->windowSaveConfig(saveStream);
	win->mainWin->windowSaveConfig(saveStream);
	win->basWin->windowSaveConfig (saveStream);
	std::ofstream configSaveFile (currentProfile.configLocation + configNameToSave + "." + CONFIG_EXTENSION);
	if (!configSaveFile.is_open ()){
		thrower ("Couldn't save configuration file! Check the name for weird characters, or call Mark about bugs if "
			"everything seems right...");
	}
	configSaveFile << saveStream.str ();
	configSaveFile.close();
	updateConfigurationSavedStatus(true);
}

/*
]--- Identical to saveConfigurationOnly except that it prompts the user for a name with a dialog box instead of taking one.
*/
void ProfileSystem::saveConfigurationAs(IChimeraQtWindow* win){
	std::string configurationPathToSave = saveWithExplorer (win, CONFIG_EXTENSION, currentProfile);
	if ( configurationPathToSave == ""){
		// canceled
		return;
	}	

	int slashPos = configurationPathToSave.find_last_of( '/' );
	int extensionPos = configurationPathToSave.find_last_of( '.' );
	currentProfile.configuration = configurationPathToSave.substr( slashPos + 1, extensionPos - slashPos - 1 );
	currentProfile.configLocation = configurationPathToSave.substr( 0, slashPos );
	slashPos = currentProfile.configLocation.find_last_of( '/' );
	currentProfile.parentFolderName = currentProfile.configLocation.substr( slashPos + 1,
																		  currentProfile.configLocation.size( ) );
	currentProfile.configLocation += "/";
	ConfigStream configSaveStream;
	// That's the last prompt the user gets, so the save is final now.
	// Version info tells future code about formatting.
	configSaveStream << std::setprecision( 13 );
	configSaveStream << "Version: " + version.str() + "\n";
	// give it to each window, allowing each window to save its relevant contents to the config file. Order matters.
	win->scriptWin->windowSaveConfig(configSaveStream);
	win->andorWin->windowSaveConfig(configSaveStream);
	win->auxWin->windowSaveConfig(configSaveStream);
	win->mainWin->windowSaveConfig(configSaveStream);
	win->basWin->windowSaveConfig (configSaveStream);
	// check if file already exists
	std::ofstream configSaveFile (configurationPathToSave);
	if (!configSaveFile.is_open ()){
		thrower ("Couldn't save configuration file! Check the name for weird characters, or call Mark about bugs if "
			"everything seems right...");
	}
	configSaveFile << configSaveStream.str ();
	configSaveFile.close();
	updateConfigurationSavedStatus(true);
}


/*
]--- This function renames the currently set 
*/
void ProfileSystem::renameConfiguration(){
	// check if configuration has been set yet.
	if (currentProfile.configuration == "")	{
		thrower ( "The Configuration has not yet been selected! Please select a configuration or create a new one before "
					"trying to rename it." );
	}

	std::string newConfigurationName;
	//TextPromptDialog dialog(&newConfigurationName, "Please enter new configuration name.", currentProfile.configuration);
	//dialog.DoModal();

	if (newConfigurationName == ""){
		// canceled
		return;
	}
	std::string currentConfigurationLocation = currentProfile.configLocation + currentProfile.configuration + "." 
		+ CONFIG_EXTENSION;
	std::string newConfigurationLocation = currentProfile.configLocation + newConfigurationName + "." + CONFIG_EXTENSION;
	int result = MoveFile(cstr(currentConfigurationLocation), cstr(newConfigurationLocation));
	if (result == 0){
		thrower ( "Renaming of the configuration file Failed! Ask Mark about bugs" );
	}
	currentProfile.configuration = newConfigurationName;
}


/*
]--- 
*/
void ProfileSystem::deleteConfiguration(){
	// check if configuration has been set yet.
	if (currentProfile.configuration == ""){
		thrower ( "The Configuration has not yet been selected! Please select a configuration or create a new one before "
				 "trying to rename it." );
	}
	auto answer = QMessageBox::question ( NULL, qstr ("Delete Config?"),
		qstr ("Are you sure you want to delete the current configuration: " 
			+ currentProfile.configuration));
	if (answer == QMessageBox::No) {
		return;
	}
	std::string currentConfigurationLocation = currentProfile.configLocation + currentProfile.configuration + "." + CONFIG_EXTENSION;
	int result = DeleteFile(cstr(currentConfigurationLocation));
	if (result == 0){
		thrower ( "ERROR: Deleteing the configuration file failed!" );
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
void ProfileSystem::updateConfigurationSavedStatus(bool isSaved){
	if ( configurationSavedIndicator == NULL ){
		return;
	}
	configurationIsSaved = isSaved;
	configurationSavedIndicator->setChecked (isSaved);
}


bool ProfileSystem::configurationSettingsReadyCheck(IChimeraQtWindow* win){
	// prompt for save.
	if (checkConfigurationSave( "There are unsaved configuration settings. Would you like to save the current "
								"configuration before starting?", win)){
		// canceled
		return true;
	}
	return false;
}


bool ProfileSystem::checkConfigurationSave( std::string prompt, IChimeraQtWindow* win){
	if (!configurationIsSaved){
		auto answer = QMessageBox::question (NULL, qstr ("Check Save?"), qstr(prompt), QMessageBox::Yes 
			| QMessageBox::No | QMessageBox::Cancel );
		if (answer == QMessageBox::Yes){
			saveConfigurationOnly( win );
		}
		else if (answer == QMessageBox::Cancel){
			return true;
		}
	}
	return false;
}

void ProfileSystem::handleSelectConfigButton(IChimeraQtWindow* win){	
	if ( !configurationIsSaved && currentProfile.configuration != ""){
		if (checkConfigurationSave ("The current configuration \"" + currentProfile.configuration
			+ "\" is unsaved. Save current configuration before changing?", win)) {
			// user canceled. 
			return;
		}
	}
	std::string fileaddress = openWithExplorer( win, CONFIG_EXTENSION );
	if (fileaddress == "") {
		return; // canceled
	}
	openConfigFromPath( fileaddress, win);
}

std::string ProfileSystem::getMasterAddressFromConfig(profileSettings profile){
	std::ifstream configF(profile.configFilePath());
	if (!configF.is_open()){
		thrower ("While trying to get the master script address from the config file " + profile.configFilePath ( ) 
				 + ", the config file failed to open!");
	}
	ConfigStream stream (configF);
	std::string line, word, address;
	getVersionFromFile(stream );
	if ( stream.ver.versionMajor < 3 ){
		line = stream.getline ();
	}
	line = stream.getline (); 
	line = stream.getline ();
	std::string newPath = stream.getline ();
	return newPath;
}


std::vector<std::string> ProfileSystem::searchForFiles( std::string locationToSearch, std::string extensions ){
	// Re-add the entries back in and figure out which one is the current one.
	std::vector<std::string> names;
	std::string search_path = locationToSearch + "\\" + extensions;
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	if (extensions == "*"){
		hFind = FindFirstFileEx( cstr(search_path), FindExInfoStandard, &fd, FindExSearchLimitToDirectories, NULL, 0 );
	}
	else{
		hFind = FindFirstFile( cstr(search_path), &fd );
	}
	if (hFind != INVALID_HANDLE_VALUE){
		do{
			// if looking for folders
			if (extensions == "*"){
				if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY){
					if (str( fd.cFileName ) != "." && str( fd.cFileName ) != ".."){
						names.push_back( fd.cFileName );
					}
				}
			}
			else{
				if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
					names.push_back( fd.cFileName );
				}
			}
		} while (FindNextFile( hFind, &fd ));
		FindClose( hFind );
	}

	// Remove suffix from file names and...
	for (unsigned configListInc = 0; configListInc < names.size(); configListInc++){
		if (extensions == "*" || extensions == "*.*" || extensions == str( "*." ) + SEQUENCE_EXTENSION
			|| extensions == str("*.") + PLOTTING_EXTENSION || extensions == str( "*." ) + CONFIG_EXTENSION 
			 || extensions == str("*.") + FUNCTION_EXTENSION ){
			names[configListInc] = names[configListInc].substr( 0, names[configListInc].size() - (extensions.size() - 1) );
		}
		else{
			names[configListInc] = names[configListInc].substr( 0, names[configListInc].size() - extensions.size() );
		}
	}
	// Make the final vector out of the unique objects left.
	return names;
}


// I had issues writing an MFC version of this with a Control<CComboBox> argument, so this is still written in Win32.
void ProfileSystem::reloadCombo( QComboBox* combo, std::string locationToLook, std::string extension,
								 std::string nameToLoad ){
	std::vector<std::string> names;
	names = searchForFiles( locationToLook, extension );
	/// Get current selection
	auto itemIndex = combo->currentIndex ();
	QString experimentConfigToOpen;
	std::string currentSelection;
	int currentInc = -1;
	if (itemIndex != -1){
		experimentConfigToOpen = combo->itemText (itemIndex);
		// Send CB_GETLBTEXT message to get the item.
		currentSelection = str(experimentConfigToOpen);
	}
	/// Reset stuffs
	combo->clear ();
	// Send list to object
	for (auto comboInc : range(names.size())){
		if (nameToLoad == names[comboInc]){
			currentInc = comboInc;
		}
		combo->insertItem (0, qstr (names[comboInc]));
		//combo->addItem (0, cstr (names[comboInc]));
	}
	// Set initial value
	combo->setCurrentIndex (currentInc);
}

bool ProfileSystem::fileOrFolderExists(std::string filePathway){
	// got this from stack exchange. dunno how it works but it should be fast.
	struct stat buffer;
	return (stat(cstr(filePathway), &buffer) == 0);
}

void ProfileSystem::fullyDeleteFolder(std::string folderToDelete){
	// this used to call SHFileOperation. Boost is better. Much better. 
	uintmax_t filesRemoved = boost::filesystem::remove_all(cstr(folderToDelete));
	if (filesRemoved == 0){
		thrower ("Delete Failed! Ask mark about bugs.");
	}
}

profileSettings ProfileSystem::getProfileSettings(){
	return currentProfile;
}