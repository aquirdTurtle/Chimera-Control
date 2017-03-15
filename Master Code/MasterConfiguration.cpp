#include "stdafx.h"
#include "MasterConfiguration.h"
#include <string>
#include "TTL_System.h"
#include "DAC_System.h"
#include <fstream>

MasterConfiguration::MasterConfiguration(std::string address) : configurationFileAddress{address}, version{"1.0"}
{

}
bool MasterConfiguration::save(TTL_System* ttls, DAC_System* dacs)
{
	/*
	information to save:
	- TTL names
	- TTL Values
	- DAC names
	- DAC Values
	*/
	// make sure that file exists
	struct stat buffer;
	if (stat(this->configurationFileAddress.c_str(), &buffer) != 0)
	{
		MessageBox(0, "WARNING: Master Configuration File Not Found! Saving this file should normally mean over-writing the previous one.", 0, 0);
	}
	// open file
	std::fstream configFile;
	configFile.open(this->configurationFileAddress.c_str(), std::ios::out);
	if (!configFile.is_open())
	{
		MessageBox(0, "ERROR: Master Configuration File Failed to Open! Changes cannot be saved.", 0, 0);
		return false;
	}
	// output version
	configFile << "Version " + this->version + "\n";
	// save info
	for (int ttlRowInc = 0; ttlRowInc < ttls->getNumberOfTTLRows(); ttlRowInc++)
	{
		for (int ttlNumberInc = 0; ttlNumberInc < ttls->getNumberOfTTLsPerRow(); ttlNumberInc++)
		{
			configFile << ttls->getName(ttlRowInc, ttlNumberInc) << "\n";
			configFile << this->defaultTTLs[ttlRowInc][ttlNumberInc] << "\n";
		}
	}
	for (int dacInc = 0; dacInc < dacs->getNumberOfDACs(); dacInc++)
	{
		configFile << dacs->getName(dacInc) << "\n";
		configFile << this->defaultDACs[dacInc] << "\n";
	}
	configFile.close();
	return true;
}

bool MasterConfiguration::load(TTL_System* ttls, DAC_System& dacs, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master)
{
	// make sure that file exists

	struct stat buffer;
	if (stat(this->configurationFileAddress.c_str(), &buffer) != 0)
	{
		MessageBox(0, "WARNING: Master Configuration File Not Found! Cannot load Master Configuration File. No Default names for TTLs, DACs, or default values.", 0, 0);
	}
	// open file
	std::fstream configFile;
	configFile.open(this->configurationFileAddress.c_str(), std::ios::in);
	if (!configFile.is_open())
	{
		MessageBox(0, "ERROR: Master Configuration File Failed to Open! No Default names for TTLs, DACs, or default values.", 0, 0);
		return false;
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
			ttls->forceTTL(ttlRowInc, ttlNumberInc, status);
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
		dacs.forceDacChange(dacInc, value, ttls);
		this->defaultDACs[dacInc] = value;
	}
	configFile.close();
	return true;
}

bool MasterConfiguration::updateDefaultDacs(DAC_System dacs)
{
	for (int dacInc = 0; dacInc < this->defaultDACs.size(); dacInc++)
	{
		this->defaultDACs[dacInc] = dacs.getDAC_Value(dacInc);
	}
	return true;
}

bool MasterConfiguration::updateDefaultTTLs(TTL_System ttls)
{
	for (int ttlRowInc = 0; ttlRowInc < ttls.getNumberOfTTLRows(); ttlRowInc++)
	{
		for (int ttlNumberInc = 0; ttlNumberInc < ttls.getNumberOfTTLsPerRow(); ttlNumberInc++)
		{
			this->defaultTTLs[ttlRowInc][ttlNumberInc] = ttls.getTTL_Status(ttlRowInc, ttlNumberInc);
		}
	}
	return true;
}
