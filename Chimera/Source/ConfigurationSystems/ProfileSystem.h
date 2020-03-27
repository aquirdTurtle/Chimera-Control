// created by Mark O. Brown
#pragma once
#include "Control.h"
#include "GeneralObjects/commonTypes.h"
#include "profileSettings.h"
#include "NIAWG/NiawgStructures.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "Scripts/ScriptStream.h"
#include <vector>
#include <string>
#include "Version.h"

class MainWindow;
class ScriptingWindow;
class AuxiliaryWindow;
class AndorWindow;
class BaslerWindow;
class DeformableMirrorWindow;

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
								AndorWindow* camWin, BaslerWindow* basWin );
		void checkSaveEntireProfile( ScriptingWindow* scriptWindow, MainWindow* mainWin, AuxiliaryWindow* auxWin,
									 AndorWindow* camWin, BaslerWindow* basWin );
		void allSettingsReadyCheck( ScriptingWindow* scriptWindow, MainWindow* mainWin, AuxiliaryWindow* auxWin,
									AndorWindow* camWin, BaslerWindow* basWin );
		static std::function<void (ScriptStream&, std::string&)> getGetlineFunc (Version& ver);
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
								    AndorWindow* camWin, BaslerWindow* basWin );
		void saveConfigurationAs( ScriptingWindow* scriptWindow, MainWindow* mainWin, AuxiliaryWindow* auxWin, 
								  AndorWindow* camWin, BaslerWindow* basWin );
		void renameConfiguration();
		void deleteConfiguration();
		void openConfigFromPath( std::string pathToConfig, ScriptingWindow* scriptWin, MainWindow* mainWin,
								 AndorWindow* camWin, AuxiliaryWindow* auxWin, BaslerWindow* basWin,
								 DeformableMirrorWindow* dmWin);
		static void getVersionFromFile( ConfigStream& file, Version& ver );
		static std::string getNiawgScriptAddrFromConfig(  profileSettings profile );
		static std::string getMasterAddressFromConfig( profileSettings profile );
		void updateConfigurationSavedStatus( bool isSaved );
		bool configurationSettingsReadyCheck(ScriptingWindow* scriptWindow, MainWindow* mainWin, 
											 AuxiliaryWindow* auxWin, AndorWindow* camWin, BaslerWindow* basWin );
		bool checkConfigurationSave(std::string prompt, ScriptingWindow* scriptWindow, MainWindow* mainWin, 
									AuxiliaryWindow* auxWin, AndorWindow* camWin, BaslerWindow* basWin );
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
									   AuxiliaryWindow* auxWin, AndorWindow* camWin, BaslerWindow* basWin,
									   DeformableMirrorWindow* dmWin);
		
		template <class sysType>
		static void standardOpenConfig ( ConfigStream& openFile, std::string delim, std::string endDelim, 
										 sysType* this_in, Version minVer = Version("0.0"));
		// an overload with the default end delimiter: "END_" + delim. Basically everything /should/ use the default.
		template <class sysType>
		static void standardOpenConfig (ConfigStream& openFile, std::string delim, sysType* this_in,
										 Version minVer = Version ( "0.0" ) );
		template <class returnType>
		static returnType stdGetFromConfig (ConfigStream& openFile, std::string delim,
												  returnType ( *getter )(ConfigStream&, Version ),
												  Version minVer = Version( "0.0" ) );
		static void checkDelimiterLine (ConfigStream& openFile, std::string keyword );
		static bool checkDelimiterLine(ConfigStream& openFile, std::string delimiter, std::string breakCondition );
		static void jumpToDelimiter (ConfigStream& openFile, std::string delimiter );
		static void initializeAtDelim (ConfigStream& openFile, std::string delimiter, Version& ver,
										Version minVer=Version("0.0") );
		CBrush* handleColoring ( int id, CDC* pDC );
	private:
		profileSettings currentProfile; 
		seqSettings currentSequence;
		std::string FILE_SYSTEM_PATH;
		bool configurationIsSaved;
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
		// Version 4.7: Added display grid check
		// Version 4.8: Moved pics per rep and exposure times to the CameraSettingsControl part of the config.
		// Version 4.9: added aisystem settings to config file.
		// Version 4.10: Added Microwave system settings to config file.
		// Version 4.11: Moved niawg system to script window, changing order of file.
		// Version 4.12: Added "Control Niawg" option.
		/// Version 5.0: Revamped reading and writing to the files to use Scriptstream, supporting comments. Includes
		// a variety of minor formatting changes and a bunch of comments into the file.
		const Version version = Version( "5.0" );
		Control<CStatic> sequenceLabel;
		Control<CComboBox> sequenceCombo;
		Control<CEdit> sequenceInfoDisplay;
		Control<CButton> sequenceSavedIndicator;
		Control<CButton> configurationSavedIndicator;
		Control<CButton> selectConfigButton;
		Control<CStatic> configDisplay;
};

template <class sysType>
static void ProfileSystem::standardOpenConfig (ConfigStream& openFile, std::string delim, sysType* this_in, Version minVer )
{
	ProfileSystem::standardOpenConfig ( openFile, delim, "END_" + delim, this_in, minVer );
}

template <class sysType>
static void ProfileSystem::standardOpenConfig ( ConfigStream& configStream, std::string delim, std::string endDelim, 
												sysType* this_in, Version minVer )
{
	// sysType must have a member function of the form
	// void ( *sysType::openFunction )( std::ifstream& f, Version ver )
	Version ver;
	try
	{
		ProfileSystem::initializeAtDelim ( configStream, delim, ver, minVer );
	}
	catch ( Error& e )
	{
		errBox ( "Failed to initialize config file for " + delim + "!\n\n" + e.trace ( ) );
		return;
	}
	try
	{
		this_in->handleOpenConfig( configStream, ver );
	}
	catch ( Error& e )
	{
		errBox ( "Failed to gather information from config file for " + delim + "!\n\n" + e.trace ( ) );
		return;
	}
	try
	{
		ProfileSystem::checkDelimiterLine ( configStream, endDelim );
	}
	catch ( Error& e )
	{
		errBox ( "End delimiter for the " + delim + " control was not found. This might indicate that the "
				 "control did not initialize properly.\n\n" + e.trace() );
	}
}

template <class returnType>
static returnType ProfileSystem::stdGetFromConfig ( ConfigStream& configStream, std::string delim, 
													returnType ( *getterFunc )(ConfigStream&, Version ),
													Version minVer )
{
	// a template functor. The getter here should get whatever is wanted from the file and return it. 
	Version ver;
	// return type must have a default constructor so that the function knows what to do if fails.
	returnType res = returnType();
	try
	{
		ProfileSystem::initializeAtDelim ( configStream, delim, ver, minVer );
	}
	catch ( Error& e )
	{
		errBox ( "Failed to initialize config file for " + delim + "!\n\n" + e.trace ( ) );
		return res;
	}
	try
	{
		res = (*getterFunc) ( configStream, ver );
	}
	catch ( Error& e )
	{
		errBox ( "Failed to gather information from config file for " + delim + "!\n\n" + e.trace ( ) );
		return res;
	}
	try
	{
		ProfileSystem::checkDelimiterLine ( configStream, "END_" + delim );
	}
	catch ( Error& e )
	{
		errBox ( "End delimiter for the " + delim + " control was not found. This might indicate that the "
				 "control did not initialize properly.\n\n" + e.trace ( ) );
	}
	return res;
}
