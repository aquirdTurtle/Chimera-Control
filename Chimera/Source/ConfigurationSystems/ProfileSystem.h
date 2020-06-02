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
#include "GeneralObjects/IDeviceCore.h"
#include "PrimaryWindows/IChimeraWindowWidget.h"
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox.h>

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

		void saveEntireProfile(IChimeraWindowWidget* win );
		void checkSaveEntireProfile(IChimeraWindowWidget* win);
		void allSettingsReadyCheck(IChimeraWindowWidget* win);
		static std::function<void (ScriptStream&, std::string&)> getGetlineFunc (Version& ver);

		void saveConfigurationOnly(IChimeraWindowWidget* win);
		void saveConfigurationAs(IChimeraWindowWidget* win);
		void renameConfiguration();
		void deleteConfiguration();
		void openConfigFromPath( std::string pathToConfig, IChimeraWindowWidget* win);
		static void getVersionFromFile( ConfigStream& file );
		static std::string getNiawgScriptAddrFromConfig(ConfigStream& configStream);
		static std::string getMasterAddressFromConfig( profileSettings profile );
		void updateConfigurationSavedStatus( bool isSaved );
		bool configurationSettingsReadyCheck(IChimeraWindowWidget* win);
		bool checkConfigurationSave(std::string prompt, IChimeraWindowWidget* win);
		profileSettings getProfileSettings();

		static std::vector<std::string> searchForFiles(std::string locationToSearch, std::string extensions);
		static void reloadCombo (QComboBox* combo, std::string locationToLook, std::string extension,
								  std::string nameToLoad );
		bool fileOrFolderExists ( std::string filePathway );
		void fullyDeleteFolder ( std::string folderToDelete );
		void initialize( POINT& topLeftPosition, IChimeraWindowWidget* win);
		void rearrange( int width, int height, fontMap fonts );
		void handleSelectConfigButton(IChimeraWindowWidget* win);
		
		template <class sysType>
		static void standardOpenConfig ( ConfigStream& openFile, std::string delim, std::string endDelim, 
										 sysType* this_in, Version minVer = Version("0.0"));
		// an overload with the default end delimiter: "END_" + delim. Basically everything /should/ use the default.
		template <class sysType>
		static void standardOpenConfig (ConfigStream& openFile, std::string delim, sysType* this_in,
										 Version minVer = Version ( "0.0" ) );
		static void checkDelimiterLine (ConfigStream& openFile, std::string keyword );
		static bool checkDelimiterLine(ConfigStream& openFile, std::string delimiter, std::string breakCondition );
		static void jumpToDelimiter (ConfigStream& openFile, std::string delimiter );
		static void initializeAtDelim ( ConfigStream& openFile, std::string delimiter, Version minVer=Version("0.0") );
		template <class coreType, class returnT>
		static void stdGetFromConfig ( ConfigStream& configStream, coreType& sys, returnT& settings, 
									   Version minVer=Version("0.0"));
		template <class returnType>
		static returnType stdConfigGetter (ConfigStream& configStream, std::string delim,
			returnType (*getterFunc)(ConfigStream&), Version minVer=Version("0.0"));
		CBrush* handleColoring (int id, CDC* pDC);
	private:
		profileSettings currentProfile; 
		std::string FILE_SYSTEM_PATH;
		bool configurationIsSaved;
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

		QCheckBox* configurationSavedIndicator;
		QPushButton* selectConfigButton;
		QLabel* configDisplay;
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
	// void ( *sysType::openFunction )( ConfigStream& f )
	try
	{
		ProfileSystem::initializeAtDelim ( configStream, delim, minVer );
	}
	catch ( Error& e )
	{
		errBox ( "Failed to initialize config file for " + delim + "!\n\n" + e.trace ( ) );
		return;
	}
	try
	{
		this_in->handleOpenConfig( configStream );
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
static returnType ProfileSystem::stdConfigGetter (ConfigStream& configStream, std::string delim,
												  returnType (*getterFunc)(ConfigStream&), Version minVer)
{
	// a template functor. The getter here should get whatever is wanted from the file and return it. 
	// return type must have a default constructor so that the function knows what to do if fails.
	returnType res = returnType ();
	try
	{
		ProfileSystem::initializeAtDelim (configStream, delim, minVer);
	}
	catch (Error & e)
	{
		errBox ("Failed to initialize config file for " + delim + "!\n\n" + e.trace ());
		return res;
	}
	try
	{
		res = (*getterFunc) (configStream);
	}
	catch (Error & e)
	{
		errBox ("Failed to gather information from config file for " + delim + "!\n\n" + e.trace ());
		return res;
	}
	try
	{
		ProfileSystem::checkDelimiterLine (configStream, "END_" + delim);
	}
	catch (Error & e)
	{
		errBox ("End delimiter for the " + delim + " control was not found. This might indicate that the "
			"control did not initialize properly.\n\n" + e.trace ());
	}
	return res;
}

template <class coreType, class returnT>
static void ProfileSystem::stdGetFromConfig ( ConfigStream& configStream, coreType& core, returnT& settings, Version minVer)
{
	// a template functor. The getter here should get whatever is wanted from the file and return it. 
	try
	{
		ProfileSystem::initializeAtDelim (configStream, core.getDelim (), minVer);
	}
	catch (Error & e)
	{
		errBox ("Failed to initialize config file for " + core.getDelim () + "!\n\n" + e.trace ());
	}
	try
	{
		settings = core.getSettingsFromConfig (configStream);
	}
	catch (Error & e)
	{
		errBox ("Failed to gather information from config file for " + core.getDelim() + "!\n\n" + e.trace ());
	}
	try
	{
		ProfileSystem::checkDelimiterLine (configStream, "END_" + core.getDelim ());
	}
	catch (Error & e)
	{
		errBox ("End delimiter for the " + core.getDelim () + " control was not found. This might indicate that the "
				"control did not initialize properly.\n\n" + e.trace ());
	}
}

