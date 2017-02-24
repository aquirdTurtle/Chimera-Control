#include "stdafx.h"
#include "Gpib.h"
#include "ni488.h"
#include <array>
#include "constants.h"

void Gpib::gpibWrite( int deviceID, std::string msg )
{
	int size = msg.size();
	int errCode = ERR;
	int result = ibwrt(deviceID, (void*)msg.c_str(), size);
	if ( result == ERR )
	{
		thrower( "gpib write failed!" + this->getErrMessage(iberr ));
	}
}


Gpib::Gpib()
{
	int hpone, agilentTwo, srsTwo, srsThree, pulseGen, pulseGen2, microHP, powerHP, agilent;
	// I think that a lot of these aren't actually doing anything...
	hpone = gpibIbdev( 17 );
	agilentTwo = gpibIbdev( 12 );
	srsTwo = gpibIbdev( 6 );
	srsThree = gpibIbdev( 19 );
	pulseGen = gpibIbdev( 7 );
	pulseGen2 = gpibIbdev( 4 );
	microHP = gpibIbdev( 10 );
	powerHP = gpibIbdev( 5 );
	agilent = gpibIbdev( 11 );
	return;
}

std::array<double, 3> Gpib::interpretKeyForRaman(std::array<std::string, 3> raman, key variationKey, 
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
			realFrequencies[ramanInc] = variationKey[raman[ramanInc]].first[variableNumber];
		}
	}
	
	return realFrequencies;
	
}


bool Gpib::copyIBVars()
{
	return true;
}


bool Gpib::ibwrti()
{
	return true;
}


bool Gpib::ibrd()
{
	return true;
}


bool Gpib::ibeot()
{
	
	return true;
}

// send message to address.
void Gpib::gpibSend(int address, std::string message)
{
	if ( !GPIB_SAFEMODE )
	{
		//errBox( "attempting to send message: " + message );
		Send( 0, address, (void*) message.c_str(), message.size(), NLend );
		if ( ibsta == ERR )
		{
			thrower( "GPIB ERROR: " + this->getErrMessage( iberr ) );
		}
		else
		{
			// errBox( "success??" );
		}
	}
	return;
}


bool Gpib::enableRemote()
{
	return true;
}

// gets the device descriptor ud. Input the device address.
int Gpib::gpibIbdev(int pad)
{
	// board address
	int bdindx = 0;   
	// see gpib dos manual for the rest
	int sad = 0; 
	int tmo = 13;
	int eot = 1;
	int eos = 0;
	int id = ibdev( bdindx, pad, sad, tmo, eot, eos );
	if ( id == -1 )
	{
		thrower( "ibdev failed!" );
	}
	return id;
}


void Gpib::programRamanFGs(double topFreq, double bottomFreq, double axialFreq)
{
	// ' 70 MHz spectroscopy frequency for both top/bottom beams
	double specFreqs = 70000000;
	
	double topRamanCoolFreq = topFreq; 
	//'-2.8 'this is the power we use to get high efficiency deflection(and 41 kHz carrier Rabi on 151210)
	double topRamanPow = -3.03;
	
	double botRamanCoolFreq = bottomFreq;
	//'-2.32 ' - 2.1 '-3.8 'this is the power we use to get 41 kHz carrier Rabi on 151210
	double botRamanPow = -4.95;
	//
	double EOramanFreq = 80000000;
	// '-18.5 ' - 5.5 '-3 'this is the power we use to get high efficiency deflection(and 41 kHz carrier Rabi on 151210)
	double EOramanPow = -20;

	double axialRamanCoolFreq = axialFreq;
	//'-4.5 ' - 2.56 'this is the power we use to get 41 kHz carrier Rabi on 151210
	double axialRamanPow = -5.5;

	//
	// Programming Tektronics 3102 AFG #1 , Gpib 25, used for generating the top and bottom Raman RF tones:
	// 
	if (true)
	{
		this->gpibSend( TEKTRONICS_AFG_1_ADDRESS, "SOURCE1:FREQ " + std::to_string(specFreqs));
		this->gpibSend( TEKTRONICS_AFG_1_ADDRESS, "SOURCE1:FSKey:STATe On");
		this->gpibSend( TEKTRONICS_AFG_1_ADDRESS, "SOURCE1:FSKey:FREQ " + std::to_string(topRamanCoolFreq));
		this->gpibSend( TEKTRONICS_AFG_1_ADDRESS, "SOURCE1:FSKey:SOURce External");
		this->gpibSend( TEKTRONICS_AFG_1_ADDRESS, "SOURCE1:VOLT:UNIT DBM");
		if (topRamanPow < -2)
		{
			this->gpibSend( TEKTRONICS_AFG_1_ADDRESS, "SOURCE1:VOLT " + std::to_string(topRamanPow));
		}
		else
		{
			errBox( "AO power set too high!" );
		}
		this->gpibSend( TEKTRONICS_AFG_1_ADDRESS, "SOURCE1:VOLT:OFFS 0");
		this->gpibSend( TEKTRONICS_AFG_1_ADDRESS, "OUTput1:STATe ON");

		//---------Source 2-------------------------------

		this->gpibSend( TEKTRONICS_AFG_1_ADDRESS, "SOURCE2:FREQ " + std::to_string(specFreqs));
		this->gpibSend( TEKTRONICS_AFG_1_ADDRESS, "SOURCE2:FSKey:STATe On");
		this->gpibSend( TEKTRONICS_AFG_1_ADDRESS, "SOURCE2:FSKey:FREQ " + std::to_string(botRamanCoolFreq));
		this->gpibSend( TEKTRONICS_AFG_1_ADDRESS, "SOURCE2:FSKey:SOURce External");
		this->gpibSend( TEKTRONICS_AFG_1_ADDRESS, "SOURCE2:VOLT:UNIT DBM");

		if (botRamanPow < -2)
		{
			this->gpibSend( TEKTRONICS_AFG_1_ADDRESS, "SOURCE2:VOLT " + std::to_string(botRamanPow));
		}
		else
		{
			errBox( "AO power set too high!" );
		}
		this->gpibSend( TEKTRONICS_AFG_1_ADDRESS, "SOURCE2:VOLT:OFFS 0");
		this->gpibSend( TEKTRONICS_AFG_1_ADDRESS, "OUTput2:STATe ON");
	}
	else
	{
		this->gpibSend( TEKTRONICS_AFG_1_ADDRESS, "OUTPut1:STATe OFF");
		this->gpibSend( TEKTRONICS_AFG_1_ADDRESS, "OUTPut2:STATe OFF");
	}

	//-------------------------------------------------------------------------------------
	/// Programming AFG3102, Gpib 24, used for generating Axial and EO Raman beam RF tones:

	if (true)
	{
		this->gpibSend( TEKTRONICS_AFG_2_ADDRESS, "SOURCE1:FUNC SIN");
		this->gpibSend( TEKTRONICS_AFG_2_ADDRESS, "SOURCE1:FREQuency:MODE CW");
		this->gpibSend( TEKTRONICS_AFG_2_ADDRESS, "SOURCE1:FREQ " + std::to_string(EOramanFreq));
		this->gpibSend( TEKTRONICS_AFG_2_ADDRESS, "SOURCE1:VOLT:UNIT DBM");
		if (EOramanPow < -2)
		{
			this->gpibSend( TEKTRONICS_AFG_2_ADDRESS, "SOURCE1:VOLT " + std::to_string(EOramanPow));
		}
		else
		{
			errBox( "Raman EO RF power too high!" );
		}
		this->gpibSend( TEKTRONICS_AFG_2_ADDRESS, "SOURCE1:VOLT:OFFS 0");
		this->gpibSend( TEKTRONICS_AFG_2_ADDRESS, "OUTput1:STATe ON");

		//-------Source 2----------------

		this->gpibSend( TEKTRONICS_AFG_2_ADDRESS, "SOURCE2:FREQ " + std::to_string(specFreqs));
		this->gpibSend( TEKTRONICS_AFG_2_ADDRESS, "SOURCE2:FSKey:STATe On");
		this->gpibSend( TEKTRONICS_AFG_2_ADDRESS, "SOURCE2:FSKey:FREQ " + std::to_string(axialRamanCoolFreq));
		this->gpibSend( TEKTRONICS_AFG_2_ADDRESS, "SOURCE2:FSKey:SOURce External");
		this->gpibSend( TEKTRONICS_AFG_2_ADDRESS, "SOURCE2:VOLT:UNIT DBM");
		if (axialRamanPow < -2)
		{
			//
			this->gpibSend( TEKTRONICS_AFG_2_ADDRESS, "SOURCE2:VOLT " + std::to_string(axialRamanPow));

		}
		else
		{
			errBox( "Axial Raman power too high!" );
		}
		this->gpibSend( TEKTRONICS_AFG_2_ADDRESS, "SOURCE2:VOLT:OFFS 0");
		this->gpibSend( TEKTRONICS_AFG_2_ADDRESS, "OUTput2:STATe ON");
	}
	else
	{
		this->gpibSend( TEKTRONICS_AFG_2_ADDRESS, "OUTPut1:STATe OFF");
		this->gpibSend( TEKTRONICS_AFG_2_ADDRESS, "OUTPut2:STATe OFF");
	}

	//	t = t + 0.01 ????????????????????????????????
	return;
}

std::string Gpib::getErrMessage( long errCode )
{
	switch ( errCode )
	{
		case EDVR:
			return "Code 0: System Error";
		case ECIC:
			return "Code 1: Function requires GPIB interface to be CIC";
		case ENOL:
			return "Code 2: No listeners on the GPIB";
		case EADR:
			return "Code 3: GPIB interface not addressed correctly";
		case EARG:
			return "Code 4: Invalid argument to function call";
		case ESAC:
			return "Code 5: GPIB interface not System controller as required";
		case EABO:
			return "Code 6: I/O operation aborted (timeout)";
		case ENEB:
			return "Code 7: Nonexistent GPIB interface";
		case EDMA:
			return "Code 8: DMA error";
		case EOIP:
			return "Code 10: Asynchronous I/O in progress";
		case ECAP:
			return "Code 11: No capability for operation";
		case EFSO:
			return "Code 12: File system error";
		case EBUS:
			return "Code 14: GPIB bus error";
		case ESTB:
			return "Code 15: Serial poll status byte queue overflow";
		case ESRQ:
			return "Code 16: SRQ stuck in ON position";
		case ETAB:
			return "Code 20: Table Problem";
		default:
			return "Unrecognized Error Code! Code was " + std::to_string( errCode );
	}
}
