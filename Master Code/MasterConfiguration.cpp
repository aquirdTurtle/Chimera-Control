#include "stdafx.h"
#include "MasterConfiguration.h"
#include <string>
#include "TTL_System.h"
#include "DAC_System.h"
#include <fstream>
#include <sys/stat.h>

MasterConfiguration::MasterConfiguration(std::string address) : configurationFileAddress{address}, version{"1.1"}
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
	configFile.open(this->configurationFileAddress.c_str(), std::ios::out);
	if (!configFile.is_open())
	{
		thrower( "ERROR: Master Configuration File Failed to Open! Changes cannot be saved. Attempted to open file in"
				" location " + configurationFileAddress );
		return;
	}
	// output version
	configFile << "Version " + this->version + "\n";
	// save info
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
			configFile << this->defaultTTLs[ttlRowInc][ttlNumberInc] << "\n";
		}
	}
	for (int dacInc = 0; dacInc < dacs->getNumberOfDACs(); dacInc++)
	{
		std::string name = dacs->getName( dacInc );
		if ( name == "" )
		{
			// then the name hasn't been set, so create the default name
			name = "Dac" + std::to_string( dacInc );
		}
		configFile << name << "\n";
		configFile << this->defaultDACs[dacInc] << "\n";
	}

	// Number of Variables
	configFile << globalVars->getCurrentNumberOfVariables() << "\n";
	/// Variable Names
	for (int varInc = 0; varInc < globalVars->getCurrentNumberOfVariables(); varInc++)
	{
		variable info = globalVars->getVariableInfo( varInc );
		configFile << info.name << " ";
		configFile << info.ranges.front().initialValue << " ";
		configFile << "\n";
	}
	configFile.close();
	return;
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
	configFile.open(this->configurationFileAddress.c_str(), std::ios::in);
	if (!configFile.is_open())
	{
		thrower("ERROR: Master Configuration File Failed to Open! No Default names for TTLs, DACs, or default values.");
		return;
	}
	std::stringstream configStream;
	configStream << configFile.rdbuf();
	// output version
	std::string version;
	// actually want to do this twice just to eat first word, which is "version".
	configStream >> version;
	configStream >> version;
	// save info
	for (int ttlRowInc = 0; ttlRowInc < this->defaultTTLs.size(); ttlRowInc++)
	{
		for (int ttlNumberInc = 0; ttlNumberInc < this->defaultTTLs[ttlRowInc].size(); ttlNumberInc++)
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
				MessageBox(0, "ERROR: Failed to load one of the default ttl values!", 0, 0);
				break;
			}

			ttls->setName(ttlRowInc, ttlNumberInc, name, toolTips, master);
			ttls->forceTtl(ttlRowInc, ttlNumberInc, status);
			this->defaultTTLs[ttlRowInc][ttlNumberInc] = status;
		}
	}

	for (int dacInc = 0; dacInc < dacs.getNumberOfDACs(); dacInc++)
	{
		std::string name;
		std::string valueString;
		double value;
		configStream >> name;
		configStream >> valueString;
		try
		{
			value = std::stod(valueString);
		}
		catch (std::invalid_argument& exception)
		{
			MessageBox(0, "ERROR: Failed to load one of the default DAC values!", 0, 0);
			break;
		}
		dacs.setName(dacInc, name, toolTips, master);
		dacs.prepareDacForceChange(dacInc, value, ttls);
		this->defaultDACs[dacInc] = value;
	}

	if (version == "1.1")
	{

		int varNum;
		configStream >> varNum;
		if (varNum < 0 || varNum > 10)
		{
			int answer = MessageBox( 0, ("ERROR: variable number retrieved from file appears suspicious. The number is " + std::to_string( varNum ) + ". Is this accurate?").c_str(), 0, MB_YESNO );
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
			double value;
			configStream >> tempVar.name;
			configStream >> value;
			tempVar.ranges.push_back( { value, value, 0 } );
			globalVars->addGlobalVariable( tempVar, varInc );
		}

	}
	variable tempVar;
	tempVar.name = "";
	globalVars->addGlobalVariable( tempVar, -1 );
	configFile.close();
	return;
}


void MasterConfiguration::updateDefaultDacs(DacSystem dacs)
{
	for (int dacInc = 0; dacInc < this->defaultDACs.size(); dacInc++)
	{
		this->defaultDACs[dacInc] = dacs.getDAC_Value(dacInc);
	}
}


void MasterConfiguration::updateDefaultTTLs(TtlSystem ttls)
{
	for (int ttlRowInc = 0; ttlRowInc < ttls.getNumberOfTTLRows(); ttlRowInc++)
	{
		for (int ttlNumberInc = 0; ttlNumberInc < ttls.getNumberOfTTLsPerRow(); ttlNumberInc++)
		{
			this->defaultTTLs[ttlRowInc][ttlNumberInc] = ttls.getTTL_Status(ttlRowInc, ttlNumberInc);
		}
	}
}

