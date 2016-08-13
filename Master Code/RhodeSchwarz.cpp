#include "stdafx.h"
#include "RhodeSchwarz.h"
#include "GPIB.h"
#include "constants.h"
//RhodeSchwarz::RhodeSchwarz(){}
bool RhodeSchwarz::initialize()
{
	// do I need to do anything here???
	return true;
}

bool RhodeSchwarz::interpretKey(std::unordered_map<std::string, std::vector<double>> key, unsigned int variationNum)
{
	for (int freqInc = 0; freqInc < this->frequencyForms.size(); freqInc++)
	{
		try
		{
			frequencies.push_back(std::stod(frequencyForms[freqInc]));
		}
		catch (std::invalid_argument &exception)
		{
			// test if it's a variable.
			frequencies.push_back(key[frequencyForms[freqInc]][variationNum]);
		}
	}
	return true;
}

// Essentially gets called by a script command.
bool RhodeSchwarz::addFrequency(std::string frequency)
{
	this->frequencyForms.push_back(frequency);
	return true;
}

bool RhodeSchwarz::programRSG(GPIB* gpibHandler)
{
	if (this->frequencyForms.size() == 0)
	{
		// nothing to do.
		return true;
	}
	else if (this->frequencyForms.size() == 1)
	{
		gpibHandler->gpibSend(RSG_ADDRESS, "OUTPUT ON");
		gpibHandler->gpibSend(RSG_ADDRESS, "SOURce:FREQuency:MODE CW");
		gpibHandler->gpibSend(RSG_ADDRESS, "FREQ " + std::to_string(frequencies[0]) + " GHz");
		gpibHandler->gpibSend(RSG_ADDRESS, "POW " + std::to_string(RSG_POWER) + " dBm");
		gpibHandler->gpibSend(RSG_ADDRESS, "OUTP ON");
	}
	else
	{
		gpibHandler->gpibSend(RSG_ADDRESS, "OUTP ON");
		gpibHandler->gpibSend(RSG_ADDRESS, "SOURce:LIST:SEL 'freqList" + std::to_string(frequencies.size()) + "'");
		std::string frequencyList = "SOURce:LIST:FREQ " + std::to_string(frequencies[0]);
		std::string powerList = "SOURce:LIST:POW " + std::to_string(RSG_POWER) + "dBm";
		for (int freqInc = 1; freqInc < frequencies.size(); freqInc++)
		{
			frequencyList += ", ";
			frequencyList += std::to_string(frequencies[freqInc]) + " GHz";
			powerList += ", ";
			powerList += std::to_string(RSG_POWER) + "dBm";
		}
		gpibHandler->gpibSend(RSG_ADDRESS, frequencyList.c_str());
		gpibHandler->gpibSend(RSG_ADDRESS, powerList.c_str());
		gpibHandler->gpibSend(RSG_ADDRESS, "SOURce:LIST:MODE STEP");
		gpibHandler->gpibSend(RSG_ADDRESS, "SOURce:LIST:TRIG:SOURce EXT");
		gpibHandler->gpibSend(RSG_ADDRESS, "SOURce:FREQ:MODE LIST");
	}
	return true;
}

bool RhodeSchwarz::clearFrequencies()
{
	this->frequencies.clear();
	return true;
}
std::vector<std::string> RhodeSchwarz::getFrequencyForms()
{
	return this->frequencyForms;
}