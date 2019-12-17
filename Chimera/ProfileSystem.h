// created by Mark O. Brown
#pragma once
#include "Control.h"
#include "commonTypes.h"
#include "profileSettings.h"
#include "NiawgStructures.h"
#include <vector>
#include <string>
#include "Version.h"

class MainWindow;
class ScriptingWindow;
class AuxiliaryWindow;
class AndorWindow;
class BaslerWindow;
class AuxiliaryWindow2;

/*
    This singleton class manages the entire "profile" system, where "profiles" are my term for the entirety of the 
	settings in the code (strange word choice I	know). It consists of the relevant controls, some saved indicators 
	that can be checked to determine if the user should be prompted to save at a given point, and all of the functions 
	for saving, renaming, deleting, and creating new levels within the code. 
*/
class ProfileSystem
{
	public:
		ProfileSystem(std::string fileSystemPath);

		void saveEntireProfile( ScriptingWindow* scriptWindow, MainWindow* mainWin, AuxiliaryWindow* auxWin,
								AndorWindow* camWin, BaslerWindow* basWin, AuxiliaryWindow2 *auxWin2 );
		void checkSaveEntireProfile( ScriptingWindow* scriptWindow, MainWindow* mainWin, AuxiliaryWindow* auxWin,
									 AndorWindow* camWin, BaslerWindow* basWin, AuxiliaryWindow2 *auxWin2);
		void allSettingsReadyCheck( ScriptingWindow* scriptWindow, MainWindow* mainWin, AuxiliaryWindow* auxWin,
									AndorWindow* camWin, BaslerWindow* basWin, AuxiliaryWindow2 *auxWin2);

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
		void reloadSequence(std::string sequenceToReload);

		void saveConfigurationOnly( ScriptingWindow* scriptWindow, MainWindow* mainWin, AuxiliaryWindow* auxWin, 
								    AndorWindow* camWin, BaslerWindow* basWin, AuxiliaryWindow2 *auxWin2);
		void newConfiguration( MainWindow* mainWin, AuxiliaryWindow* auxWin, AndorWindow* camWin,
							   ScriptingWindow* scriptWin, AuxiliaryWindow2 *auxWin2);
		void saveConfigurationAs( ScriptingWindow* scriptWindow, MainWindow* mainWin, AuxiliaryWindow* auxWin, 
								  AndorWindow* camWin, BaslerWindow* basWin, AuxiliaryWindow2 *auxWin2);
		void renameConfiguration();
		void deleteConfiguration();
		void openConfigFromPath( std::string pathToConfig, ScriptingWindow* scriptWin, MainWindow* mainWin,
								 AndorWindow* camWin, AuxiliaryWindow* auxWin, BaslerWindow* basWin, AuxiliaryWindow2 *auxWin2);
		static void getVersionFromFile( std::ifstream& f, Version& ver );
		static std::string getNiawgScriptAddrFromConfig(  profileSettings profile );
		static std::string getMasterAddressFromConfig( profileSettings profile );
		void updateConfigurationSavedStatus( bool isSaved );
		bool configurationSettingsReadyCheck(ScriptingWindow* scriptWindow, MainWindow* mainWin,  AuxiliaryWindow* auxWin, 
											 AndorWindow* camWin, BaslerWindow* basWin, AuxiliaryWindow2 *auxWin2);
		bool checkConfigurationSave(std::string prompt, ScriptingWindow* scriptWindow, MainWindow* mainWin, 
									AuxiliaryWindow* auxWin, AndorWindow* camWin, BaslerWindow* basWin, AuxiliaryWindow2 *auxWin2);
		std::string getCurrentPathIncludingCategory();
		profileSettings getProfileSettings();
		seqSettings getSeqSettings( );

		static std::vector<std::string> searchForFiles(std::string locationToSearch, std::string extensions);
		static void reloadCombo ( HWND comboToReload, std::string locationToLook, std::string extension,
								  std::string nameToLoad );
		bool fileOrFolderExists ( std::string filePathway );
		void fullyDeleteFolder ( std::string folderToDelete );
		void initialize( POINT& topLeftPosition, CWnd* parent, int& id, cToolTips& tooltips );
		void rearrange( int width, int height, fontMap fonts );
		void handleSelectConfigButton( CWnd* parent, ScriptingWindow* scriptWindow, MainWindow* mainWin,
									   AuxiliaryWindow* auxWin, AndorWindow* camWin, BaslerWindow* basWin, AuxiliaryWindow2 *auxWin2);
		
		template <class sysType>
		static void standardOpenConfig ( std::ifstream& openFile, std::string delim, std::string endDelim, 
										 sysType* this_in, Version minVer = Version("0.0"));
		// an overload with the default end delimiter: "END_" + delim. Basically everything /should/ use the default.
		template <class sysType>
		static void standardOpenConfig ( std::ifstream& openFile, std::string delim, sysType* this_in, 
										 Version minVer = Version ( "0.0" ) );
		template <class returnType>
		static returnType stdGetFromConfig ( std::ifstream& openFile, std::string delim, 
												  returnType ( *getter )( std::ifstream&, Version ),
												  Version minVer = Version( "0.0" ) );
		static void checkDelimiterLine ( std::ifstream& openFile, std::string keyword );
		static bool checkDelimiterLine( std::ifstream& openFile, std::string delimiter, std::string breakCondition );
		static void jumpToDelimiter ( std::ifstream& openFile, std::string delimiter );
		static void initializeAtDelim ( std::ifstream& openFile, std::string delimiter, Version& ver, 
										Version minVer=Version("0.0") );
	private:
		profileSettings currentProfile; 
		seqSettings currentSequence;
		std::string FILE_SYSTEM_PATH;
		bool configurationIsSaved;
		bool categoryIsSaved;
		bool experimentIsSaved;
		bool sequenceIsSaved;
		// version = str(versionMain) + "." + str(versionSub)
		// I try to use version sub changes for small changes and version main for big formatting (sometimes backwards 
		// incompatible) changes

		// version 3.6: rerngMode added instead of various bools
		// Version 3.7: stop reporting initial ttl and dac values in configs.
		// Version 4.0: changed the way I read in data for different controls. Each control re-opens the file and scans  
		// untill it finds the appropriate section for it to read. Makes it so that if one thing fails, everything  
		// afterwards doesn't also fail.
		// Version 4.1: Added auto threshold analysis option
		// Version 4.2: Agilent channel mode added and saving this as text instead of index
		// Version 4.3: Refactored parameter system range structure to include range info separate for each dimension.
		// Version 4.4: added software accumulation picture options
		// Version 4.5: Added DDS system to save.
		// Version 4.6: Added Piezo System 1 to save.
		// Version 4.7: Adding DM system to configuration file
		const Version version = Version( "4.7" );
		Control<CStatic> sequenceLabel;
		Control<CComboBox> sequenceCombo;
		Control<CEdit> sequenceInfoDisplay;
		Control<CButton> sequenceSavedIndicator;
		Control<CButton> configurationSavedIndicator;
		Control<CButton> selectConfigButton;
		Control<CStatic> configDisplay;
};

template <class sysType>
static void ProfileSystem::standardOpenConfig ( std::ifstream& openFile, std::string delim, sysType* this_in, Version minVer )
{
	ProfileSystem::standardOpenConfig ( openFile, delim, "END_" + delim, this_in, minVer );
}

template <class sysType>
static void ProfileSystem::standardOpenConfig ( std::ifstream& openFile, std::string delim, std::string endDelim, sysType* this_in,
												Version minVer)
{
	// sysType must have a member function of the form
	// void ( *sysType::openFunction )( std::ifstream& f, Version ver )
	Version ver;
	try
	{
		ProfileSystem::initializeAtDelim ( openFile, delim, ver, minVer );
	}
	catch ( Error& e )
	{
		errBox ( "Failed to initialize config file for " + delim + "!\n\n" + e.trace ( ) );
		return;
	}
	try
	{
		this_in->handleOpenConfig( openFile, ver );
	}
	catch ( Error& e )
	{
		errBox ( "Failed to gather information from config file for " + delim + "!\n\n" + e.trace ( ) );
		return;
	}
	try
	{
		ProfileSystem::checkDelimiterLine ( openFile, endDelim );
	}
	catch ( Error& e )
	{
		errBox ( "End delimiter for the " + delim + " control was not found. This might indicate that the "
				 "control did not initialize properly.\n\n" + e.trace() );
	}
}

template <class returnType>
static returnType ProfileSystem::stdGetFromConfig ( std::ifstream& openFile, std::string delim, 
													     returnType ( *getter )( std::ifstream&, Version ),
													     Version minVer )
{
	// a template functor. The getter here should get whatever is wanted from the file and return it. 
	Version ver;
	// return type must have a default constructor so that the function knows what to do if fails.
	returnType res = returnType();
	try
	{
		ProfileSystem::initializeAtDelim ( openFile, delim, ver, minVer );
	}
	catch ( Error& e )
	{
		errBox ( "Failed to initialize config file for " + delim + "!\n\n" + e.trace ( ) );
		return res;
	}
	try
	{
		res = (*getter) ( openFile, ver );
	}
	catch ( Error& e )
	{
		errBox ( "Failed to gather information from config file for " + delim + "!\n\n" + e.trace ( ) );
		return res;
	}
	try
	{
		ProfileSystem::checkDelimiterLine ( openFile, "END_" + delim );
	}
	catch ( Error& e )
	{
		errBox ( "End delimiter for the " + delim + " control was not found. This might indicate that the "
				 "control did not initialize properly.\n\n" + e.trace ( ) );
	}
	return res;
}
