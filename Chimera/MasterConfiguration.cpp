#include "stdafx.h"
#include "MasterConfiguration.h"
#include "DioSystem.h"
#include "AoSystem.h"
#include <sys/stat.h>
#include "AuxiliaryWindow.h"
#include "AndorWindow.h"
#include "MainWindow.h"
#include "Thrower.h"
#include <string>
#include <fstream>



MasterConfiguration::MasterConfiguration(std::string address) : configurationFileAddress{address}
{}


void MasterConfiguration::save(MainWindow* mainWin, AuxiliaryWindow* auxWin, AndorWindow* camWin) 
{
	/*
		information to save:
		- TTL names
		- TTL Values
		- DAC names
		- DAC Values
		- DAC Min/Max Values
		- global Variables
		- Camera View?
	*/
	// make sure that file exists
	FILE *file;
	fopen_s( &file, cstr(configurationFileAddress), "r" );
	if ( !file )
	{
		errBox( "WARNING: Master Configuration File Not Found! Saving this file should normally mean over-writing the previous one." );
	}
	else
	{
		fclose( file );
	}
	// open file
	std::fstream configFile;
	configFile.open(cstr(configurationFileAddress), std::ios::out);
	if (!configFile.is_open())
	{
		thrower ( "ERROR: Master Configuration File Failed to Open! Changes cannot be saved. Attempted to open file in"
				" location " + configurationFileAddress );
	}
	std::stringstream configStream;
	// output version
	configStream << "Version " + version.str() + "\n";
	auxWin->handleMasterConfigSave(configStream);
	camWin->handleMasterConfigSave(configStream);
	configFile << configStream.str();
	configFile.close();
}


void MasterConfiguration::load(MainWindow* mainWin, AuxiliaryWindow* auxWin, AndorWindow* camWin)
{
	// make sure that file exists	
	FILE *file;
	fopen_s( &file, cstr(configurationFileAddress), "r" );
	if ( !file )
	{
		errBox("WARNING: Master Configuration File Not Found! Cannot load Master Configuration File. No Default names for TTLs, DACs, or default values.");
	}
	else
	{
		fclose( file );
	}
	// open file
	std::fstream configFile;
	configFile.open(cstr(configurationFileAddress), std::ios::in);
	if (!configFile.is_open())
	{
		thrower ("ERROR: Master Configuration File Failed to Open! No Default names for TTLs, DACs, or default values.");
	}
	std::stringstream configStream;
	configStream << configFile.rdbuf();
	// output version
	std::string versionStr;
	// actually want to do this twice just to eat first word, which is "version".
	configStream >> versionStr;
	configStream >> versionStr;
	Version ver( versionStr );
	// Initialize ttl, dac, and servo structures.
	auxWin->handleMasterConfigOpen( configStream, ver );
	// initialize camera window
	camWin->handleMasterConfigOpen( configStream, ver);

	configFile.close();
}

