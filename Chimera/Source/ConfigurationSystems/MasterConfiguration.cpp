// created by Mark O. Brown
#include "stdafx.h"
#include "MasterConfiguration.h"
#include "DigitalOutput/DoSystem.h"
#include "AnalogOutput/AoSystem.h"
#include <sys/stat.h>
#include "PrimaryWindows/QtAuxiliaryWindow.h"
#include "PrimaryWindows/QtAndorWindow.h"
#include "PrimaryWindows/QtMainWindow.h"
#include "PrimaryWindows/QtScriptWindow.h"
#include <string>
#include <fstream>

MasterConfiguration::MasterConfiguration(std::string address) : configurationFileAddress{address}{}

void MasterConfiguration::save(QtMainWindow* mainWin, QtAuxiliaryWindow* auxWin, QtAndorWindow* camWin) {
	/*
		information to save:
		- TTL names
		- TTL Values
		- DAC names
		- DAC Values
		- DAC Min/Max Values
		- global Variables
		- Andor View?
	*/
	// make sure that file exists
	FILE *file;
	fopen_s( &file, cstr(configurationFileAddress), "r" );
	if ( !file ){
		errBox( "WARNING: Master Configuration File Not Found! Saving this file should normally mean over-writing the previous one." );
	}
	else{
		fclose( file );
	}
	// open file
	std::fstream configFile;
	configFile.open(cstr(configurationFileAddress), std::ios::out);
	if (!configFile.is_open()){
		thrower ( "Master Configuration File Failed to Open! Changes cannot be saved. Attempted to open file in"
				" location " + configurationFileAddress );
	}
	ConfigStream configStream;
	// output version
	configStream << "Version " + version.str() + "\n";
	auxWin->handleMasterConfigSave(configStream);
	mainWin->handleMasterConfigSave (configStream);
	camWin->handleMasterConfigSave(configStream);
	configFile << configStream.str();
	configFile.close();
}

void MasterConfiguration::load(QtMainWindow* mainWin, QtAuxiliaryWindow* auxWin, QtAndorWindow* camWin){
	try {
		// make sure that file exists	
		FILE* file;
		fopen_s (&file, cstr (configurationFileAddress), "r");
		if (!file) {
			errBox ("WARNING: Master Configuration File Not Found! Cannot load Master Configuration File. No Default names for TTLs, DACs, or default values.");
		}
		else {
			fclose (file);
		}
		ConfigStream configFile (configurationFileAddress, true);
		ConfigSystem::getVersionFromFile (configFile);
		auxWin->handleMasterConfigOpen (configFile);
		mainWin->handleMasterConfigOpen (configFile);
		camWin->handleMasterConfigOpen (configFile);
		mainWin->scriptWin->updateDoAoNames ();
	}
	catch (ChimeraError & err) {
		mainWin->reportErr (err.qtrace ());
	}
}

