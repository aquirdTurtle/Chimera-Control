#include "stdafx.h"
#include "MasterConfiguration.h"
#include <string>
#include "DioSystem.h"
#include "DacSystem.h"
#include <fstream>
#include <sys/stat.h>
#include "AuxiliaryWindow.h"
#include "CameraWindow.h"

MasterConfiguration::MasterConfiguration(std::string address) : configurationFileAddress{address}, version{"2.0"}
{}


void MasterConfiguration::save(MainWindow* mainWin, AuxiliaryWindow* auxWin, CameraWindow* camWin) 
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
		thrower( "ERROR: Master Configuration File Failed to Open! Changes cannot be saved. Attempted to open file in"
				" location " + configurationFileAddress );
	}
	std::stringstream configStream;
	// output version
	configStream << "Version " + version + "\n";
	auxWin->handleMasterConfigSave(configStream);
	camWin->handleMasterConfigSave(configStream);
	configFile << configStream.str();
	configFile.close();
}


void MasterConfiguration::load(MainWindow* mainWin, AuxiliaryWindow* auxWin, CameraWindow* camWin)
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
		thrower("ERROR: Master Configuration File Failed to Open! No Default names for TTLs, DACs, or default values.");
	}
	std::stringstream configStream;
	configStream << configFile.rdbuf();
	// output version
	std::string versionStr;
	double version;
	// actually want to do this twice just to eat first word, which is "version".
	configStream >> versionStr;
	configStream >> versionStr;
	try
	{
		version = std::stod(versionStr);
	}
	catch (std::invalid_argument&)
	{
		thrower("Bad Version String from master config file! String was " + versionStr);
	}
	// Initialize ttl and dac structures.
	auxWin->handleMasterConfigOpen( configStream, version );
	// initialize camera window
	camWin->handleMasterConfigOpen( configStream, version );

	configFile.close();
}

/*
void MasterConfiguration::updateDefaultDacs(DacSystem dacs)
{
	for (int dacInc = 0; dacInc < defaultDacs.size(); dacInc++)
	{
		defaultDacs[dacInc] = dacs.getDacValue(dacInc);
	}
}


void MasterConfiguration::updateDefaultTTLs(DioSystem ttls)
{
	for (int ttlRowInc = 0; ttlRowInc < ttls.getNumberOfTTLRows(); ttlRowInc++)
	{
		for (int ttlNumberInc = 0; ttlNumberInc < ttls.getNumberOfTTLsPerRow(); ttlNumberInc++)
		{
			defaultTtls[ttlRowInc][ttlNumberInc] = ttls.getTtlStatus(ttlRowInc, ttlNumberInc);
		}
	}
}
*/