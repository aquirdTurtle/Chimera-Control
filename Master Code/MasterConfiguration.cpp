#include "stdafx.h"
#include "MasterConfiguration.h"
#include <string>
#include "TtlSystem.h"
#include "DacSystem.h"
#include <fstream>
#include <sys/stat.h>

MasterConfiguration::MasterConfiguration(std::string address) : configurationFileAddress{address}, version{"1.3"}
{

}

void MasterConfiguration::save(TtlSystem* ttls, DacSystem* dacs, VariableSystem* globalVars)
{
	/*
	information to save:
	- TTL names
	- TTL Values
	- DAC names
	- DAC Values
	- DAC Min/Max Values
	*/
	// make sure that file exists
	FILE *file;
	fopen_s( &file, configurationFileAddress.c_str(), "r" );
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
	configFile.open(configurationFileAddress.c_str(), std::ios::out);
	if (!configFile.is_open())
	{
		thrower( "ERROR: Master Configuration File Failed to Open! Changes cannot be saved. Attempted to open file in"
				" location " + configurationFileAddress );
	}
	// output version
	configFile << "Version " + version + "\n";
	// save info
	/// ttls
	for (int ttlRowInc = 0; ttlRowInc < ttls->getNumberOfTTLRows(); ttlRowInc++)
	{
		for (int ttlNumberInc = 0; ttlNumberInc < ttls->getNumberOfTTLsPerRow(); ttlNumberInc++)
		{
			std::string name = ttls->getName( ttlRowInc, ttlNumberInc );
			if ( name == "" )
			{
				// then no name has been set, so create the default name.
				switch ( ttlRowInc )
				{
					case 0:
						name = "A";
						break;
					case 1:
						name = "B";
						break;
					case 2:
						name = "C";
						break;
					case 3:
						name = "D";
						break;
				}
				name += std::to_string( ttlNumberInc );
			}
			configFile << name << "\n";
			configFile << defaultTTLs[ttlRowInc][ttlNumberInc] << "\n";
		}
	}
	// DAC Names
	for (int dacInc = 0; dacInc < dacs->getNumberOfDacs(); dacInc++)
	{
		std::string name = dacs->getName( dacInc );
		std::pair<double, double> minMax = dacs->getDacRange(dacInc);
		if ( name == "" )
		{
			// then the name hasn't been set, so create the default name
			name = "Dac" + std::to_string( dacInc );
		}
		configFile << name << "\n";
		configFile << minMax.first << " - " << minMax.second << "\n";
		configFile << defaultDACs[dacInc] << "\n";
	}

	// Number of Variables
	configFile << globalVars->getCurrentNumberOfVariables() << "\n";
	/// Variables
	
	for (int varInc = 0; varInc < globalVars->getCurrentNumberOfVariables(); varInc++)
	{
		variable info = globalVars->getVariableInfo( varInc );
		configFile << info.name << " ";
		configFile << info.ranges.front().initialValue << "\n";
		// all globals are constants, no need to output anything else.
	}
	configFile.close();
}

void MasterConfiguration::load(TtlSystem* ttls, DacSystem& dacs, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master, VariableSystem* globalVars)
{
	// make sure that file exists	
	FILE *file;
	fopen_s( &file, configurationFileAddress.c_str(), "r" );
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
	configFile.open(configurationFileAddress.c_str(), std::ios::in);
	if (!configFile.is_open())
	{
		thrower("ERROR: Master Configuration File Failed to Open! No Default names for TTLs, DACs, or default values.");
	}
	// okay things going well. Initialize ttl and dac structures.
	ttls->resetTtlEvents();
	ttls->prepareForce();
	dacs.resetDacEvents();
	dacs.prepareForce();
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
	catch (std::invalid_argument& err)
	{
		thrower("Bad Version String from master config file! String was " + versionStr);
	}
	// save info
	for (int ttlRowInc = 0; ttlRowInc < defaultTTLs.size(); ttlRowInc++)
	{
		for (int ttlNumberInc = 0; ttlNumberInc < defaultTTLs[ttlRowInc].size(); ttlNumberInc++)
		{
			std::string name;
			std::string statusString;
			bool status;
			configStream >> name;
			configStream >> statusString;
			try
			{
				// should actually be zero or one, but just just convert to bool
				status = std::stoi(statusString);
			}
			catch (std::invalid_argument& exception)
			{
				thrower("ERROR: Failed to load one of the default ttl values!");
			}

			ttls->setName(ttlRowInc, ttlNumberInc, name, toolTips, master);
			ttls->forceTtl(ttlRowInc, ttlNumberInc, status);
			defaultTTLs[ttlRowInc][ttlNumberInc] = status;
		}
	}
	// getting dacs.
	for (int dacInc = 0; dacInc < dacs.getNumberOfDacs(); dacInc++)
	{
		std::string name;
		std::string defaultValueString;
		double defaultValue;
		std::string minString;
		std::string maxString;
		double min;
		double max;
		configStream >> name;
		if (version >= 1.2)
		{
			configStream >> minString;
			std::string trash;
			configStream >> trash;
			if (trash != "-")
			{
				thrower("ERROR: Expected \"-\" in config file between min and max values!");
			}
			configStream >> maxString;
		}
		configStream >> defaultValueString;
		try
		{
			defaultValue = std::stod(defaultValueString);
			if (version >= 1.2)
			{
				min = std::stod(minString);
				max = std::stod(maxString);
			}
			else
			{
				min = -10;
				max = 10;
			}
		}
		catch (std::invalid_argument& exception)
		{
			thrower("ERROR: Failed to load one of the default DAC values!");
		}
		dacs.setName(dacInc, name, toolTips, master);
		dacs.setMinMax(dacInc, min, max);
		dacs.prepareDacForceChange(dacInc, defaultValue, ttls);
		defaultDACs[dacInc] = defaultValue;
	}

	if (version >= 1.1)
	{
		int varNum;
		configStream >> varNum;
		if (varNum < 0 || varNum > 1000)
		{
			int answer = MessageBox( 0, ("ERROR: variable number retrieved from file appears suspicious. The number is " 
									+ std::to_string( varNum ) + ". Is this accurate?").c_str(), 0, MB_YESNO );
			if (answer == IDNO)
			{
				// don't try to load anything.
				varNum = 0;
				return;
			}
		}
		// Number of Variables
		globalVars->clearVariables();
		for (int varInc = 0; varInc < varNum; varInc++)
		{
			variable tempVar;
			tempVar.constant = true;
			tempVar.overwritten = false;
			tempVar.active = false;
			double value;
			configStream >> tempVar.name;
			configStream >> value;
			tempVar.ranges.push_back( { value, value, 0, false, true } );
			globalVars->addGlobalVariable( tempVar, varInc );
		}
	}

	variable tempVar;
	tempVar.name = "";
	globalVars->addGlobalVariable( tempVar, -1 );
	configFile.close();
}


void MasterConfiguration::updateDefaultDacs(DacSystem dacs)
{
	for (int dacInc = 0; dacInc < defaultDACs.size(); dacInc++)
	{
		defaultDACs[dacInc] = dacs.getDacValue(dacInc);
	}
}


void MasterConfiguration::updateDefaultTTLs(TtlSystem ttls)
{
	for (int ttlRowInc = 0; ttlRowInc < ttls.getNumberOfTTLRows(); ttlRowInc++)
	{
		for (int ttlNumberInc = 0; ttlNumberInc < ttls.getNumberOfTTLsPerRow(); ttlNumberInc++)
		{
			defaultTTLs[ttlRowInc][ttlNumberInc] = ttls.getTTL_Status(ttlRowInc, ttlNumberInc);
		}
	}
}
