
#include "stdafx.h"
#include "GPIB.h"
#include "ni488.h"
#include <array>


std::array<double, 3> GPIB::interpretKeyForRaman(std::array<std::string, 3> raman, std::unordered_map<std::string, std::vector<double>> key, 
	unsigned int variableNumber)
{
	std::array<double, 3> realFrequencies;
	for (int ramanInc = 0; ramanInc < realFrequencies.size(); ramanInc++)
	{
		if (raman[ramanInc] == "")
		{
			continue;
		}
		try
		{
			double freq = std::stod(raman[ramanInc]);
			realFrequencies[ramanInc] = freq;
		}
		catch (std::invalid_argument&)
		{
			realFrequencies[ramanInc] = key[raman[ramanInc]][variableNumber];
		}
	}
	return realFrequencies;
}

bool GPIB::registerGPIB()
{
	return true;
}
bool GPIB::unregisterGPIB()
{
	return true;
}
bool GPIB::copyIBVars()
{
	return true;
}
bool GPIB::ibwrt()
{
	return true;
}
bool GPIB::ibwrti()
{
	return true;
}
bool GPIB::ibrd()
{
	return true;
}
bool GPIB::ibeot()
{
	return true;
}

// send message to address.
bool GPIB::gpibSend(int address, std::string message)
{
	Send(0, address, (void*)message.c_str(), 0, NLend);
	return true;
}
bool GPIB::enableRemote()
{
	return true;
}
bool GPIB::ildev()
{
	return true;
}

bool GPIB::programRamanFGs(double topFreq, double bottomFreq, double axialFreq)
{
	//Dim topRamanPow, botRamanPow, axialRamanPow, EOramanPow, topRamanCoolFreq, botRamanCoolFreq, axialRamanCoolFreq, EOramanFreq, specFreqs As Double

	//-------------------------------------------------------------------------------------
	//Programming AFG3102, GPIB 25, used for generating the top and bottom Raman RF tones:
	// ' 70 MHz spectroscopy frequency for both top/bottom beams
	double specFreqs = 70000000;
	// this is the standard value, but may need to be a global variable that we can set above/vary...
	double topRamanCoolFreq = topFreq; 
	//'-2.8 'this is the power we use to get high efficiency deflection(and 41 kHz carrier Rabi on 151210)
	double topRamanPow = -3.03;
	// ' this is the standard value, but may need to be a global variable that we can set above/vary...
	double botRamanCoolFreq = bottomFreq;
	//'-2.32 ' - 2.1 '-3.8 'this is the power we use to get 41 kHz carrier Rabi on 151210
	double botRamanPow = -4.95;
	//
	if (true)
	{
		this->gpibSend(25, "SOURCE1:FREQ " + std::to_string(specFreqs));
		this->gpibSend(25, "SOURCE1:FSKey:STATe On");
		this->gpibSend(25, "SOURCE1:FSKey:FREQ " + std::to_string(topRamanCoolFreq));
		this->gpibSend(25, "SOURCE1:FSKey:SOURce External");
		this->gpibSend(25, "SOURCE1:VOLT:UNIT DBM");
		if (topRamanPow < -2)
		{
			this->gpibSend(25, "SOURCE1:VOLT " + std::to_string(topRamanPow));
		}
		else
		{
			//printnow "AO power set too high!"
		}
		this->gpibSend(25, "SOURCE1:VOLT:OFFS 0");
		this->gpibSend(25, "OUTput1:STATe ON");

		//----------------------------------------

		this->gpibSend(25, "SOURCE2:FREQ " + std::to_string(specFreqs));
		this->gpibSend(25, "SOURCE2:FSKey:STATe On");
		this->gpibSend(25, "SOURCE2:FSKey:FREQ " + std::to_string(botRamanCoolFreq));
		this->gpibSend(25, "SOURCE2:FSKey:SOURce External");
		this->gpibSend(25, "SOURCE2:VOLT:UNIT DBM");

		if (botRamanPow < -2)
		{
			this->gpibSend(25, "SOURCE2:VOLT " + std::to_string(botRamanPow));
		}
		else
		{
			//printnow "AO power set too high!"
		}
		this->gpibSend(25, "SOURCE2:VOLT:OFFS 0");
		this->gpibSend(25, "OUTput2:STATe ON");
	}
	else
	{
		this->gpibSend(25, "OUTPut1:STATe OFF");
		this->gpibSend(25, "OUTPut2:STATe OFF");
	}

		//-------------------------------------------------------------------------------------
		//Programming AFG3102, GPIB 24, used for generating Axial and EO Raman beam RF tones:

	double EOramanFreq = 80000000;
	// '-18.5 ' - 5.5 '-3 'this is the power we use to get high efficiency deflection(and 41 kHz carrier Rabi on 151210)
	double EOramanPow = -20;
	// ' this is the standard value, but may need to be a global variable that we can set above/vary...
	double axialRamanCoolFreq = axialFreq;
	//'-4.5 ' - 2.56 'this is the power we use to get 41 kHz carrier Rabi on 151210
	double axialRamanPow = -5.5; 

	if (true)
	{
		this->gpibSend(24, "SOURCE1:FUNC SIN");
		this->gpibSend(24, "SOURCE1:FREQuency:MODE CW");
		this->gpibSend(24, "SOURCE1:FREQ " + std::to_string(EOramanFreq));
		this->gpibSend(24, "SOURCE1:VOLT:UNIT DBM");
		if (EOramanPow < -2)
		{
			this->gpibSend(24, "SOURCE1:VOLT " + std::to_string(EOramanPow));
		}
		else
		{
			//printnow "Raman EO RF power too high!"
		}
		this->gpibSend(24, "SOURCE1:VOLT:OFFS 0");
		this->gpibSend(24, "OUTput1:STATe ON");

		//-----------------------

		this->gpibSend(24, "SOURCE2:FREQ " + std::to_string(specFreqs));
		this->gpibSend(24, "SOURCE2:FSKey:STATe On");
		this->gpibSend(24, "SOURCE2:FSKey:FREQ " + std::to_string(axialRamanCoolFreq));
		this->gpibSend(24, "SOURCE2:FSKey:SOURce External");
		this->gpibSend(24, "SOURCE2:VOLT:UNIT DBM");
		if (axialRamanPow < -2)
		{
			//
			this->gpibSend(24, "SOURCE2:VOLT " + std::to_string(axialRamanPow));

		}
		else
		{
			//printnow "Axial Raman power too high!"
		}
		this->gpibSend(24, "SOURCE2:VOLT:OFFS 0");
		this->gpibSend(24, "OUTput2:STATe ON");
	}
	else
	{
		this->gpibSend(24, "OUTPut1:STATe OFF");
		this->gpibSend(24, "OUTPut2:STATe OFF");
	}

	//	t = t + 0.01 ????????????????????????????????

	return true;
}
