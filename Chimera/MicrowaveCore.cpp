#include "stdafx.h"
#include "MicrowaveCore.h"


MicrowaveCore::MicrowaveCore() : gpibFlume (RSG_ADDRESS, RSG_SAFEMODE) {}

void MicrowaveCore::programRsg (UINT variationNumber, microwaveSettings settings)
{
	if (!settings.control)
	{
		return;
	}
	auto list = settings.list;
	if (!settings.control || list.size () == 0)
	{
		// Don't program anything.
		return;
	}
	else
	{
		setPmSettings ();
		if (list.size () == 1)
		{
			gpibFlume.send ("OUTPUT ON");
			gpibFlume.send ("SOURce:FREQuency:MODE CW");
			gpibFlume.send ("FREQ " + str (list[0].frequency.getValue(variationNumber), 13) + " GHz");
			gpibFlume.send ("POW " + str (list[0].power.getValue (variationNumber), 13) + " dBm");
			gpibFlume.send ("OUTP ON");
		}
		else
		{
			gpibFlume.send ("OUTP ON");
			gpibFlume.send ("SOURce:LIST:SEL 'freqList" + str (list.size ()) + "'");
			std::string freqList = "SOURce:LIST:FREQ " + str (list[0].frequency.getValue (variationNumber), 13) + " GHz";
			std::string powerList = "SOURce:LIST:POW " + str (list[0].power.getValue (variationNumber), 13) + "dBm";
			for (UINT eventInc = 1; eventInc < list.size (); eventInc++)
			{
				freqList += ", ";
				freqList += str (list[eventInc].frequency.getValue (variationNumber), 13) + " GHz";
				powerList += ", ";
				powerList += str (list[eventInc].power.getValue (variationNumber), 13) + "dBm";
			}
			gpibFlume.send (cstr (freqList));
			gpibFlume.send (cstr (powerList));
			gpibFlume.send ("SOURce:LIST:MODE STEP");
			gpibFlume.send ("SOURce:LIST:TRIG:SOURce EXT");
			gpibFlume.send ("SOURce:FREQ:MODE LIST");
		}
	}
}

std::string MicrowaveCore::queryIdentity ()
{
	return gpibFlume.queryIdentity ();
}


void MicrowaveCore::setFmSettings ()
{
	gpibFlume.send ("SOURCE:PM1:STATe OFF");
	gpibFlume.send ("SOURCE:FM:MODE NORMal");
	gpibFlume.send ("SOURCE:FM:RATio 100PCT");
	gpibFlume.send ("SOURCE:FM1:SOURce EXT1");
	gpibFlume.send ("SOURCE:FM1:DEViation 20kHz");
	gpibFlume.send ("SOURCE:FM1:STATe ON");
}


void MicrowaveCore::setPmSettings ()
{
	gpibFlume.send ("SOURCE:FM1:STATe OFF");
	gpibFlume.send ("SOURCE:PM:MODE HDEViation");
	gpibFlume.send ("SOURCE:PM:RATio 100PCT");
	gpibFlume.send ("SOURCE:PM1:SOURce EXT1");
	gpibFlume.send ("SOURCE:PM1:DEViation 6.28RAD");
	gpibFlume.send ("SOURCE:PM1:STATe ON");
}


void MicrowaveCore::interpretKey (std::vector<std::vector<parameterType>>& params, microwaveSettings& settings)
{
	if (!settings.control)
	{
		return;
	}
	UINT variations;
	UINT sequencNumber;
	if (params.size () == 0)
	{
		thrower ("ERROR: empty variables! no sequence size!");
	}
	else if (params[0].size () == 0)
	{
		variations = 1;
	}
	else
	{
		variations = params.front ().front ().keyValues.size ();
	}
	sequencNumber = params.size ();
	/// imporantly, this sizes the relevant structures.
	for (auto seqNum : range (sequencNumber))
	{
		for (auto freqInc : range(settings.list.size()))
		{
			settings.list[freqInc].frequency.internalEvaluate (params[seqNum], variations);
			settings.list[freqInc].power.internalEvaluate (params[seqNum], variations);
		}
	}
}

// Essentially gets called by a script command.
/*void MicrowaveCore::addFrequency (microwaveListEntry info)
{
	eventForms.push_back (info);
}
void MicrowaveCore::clearFrequencies ()
{
	eventForms.clear ();
}
std::vector<microwaveListEntry> MicrowaveCore::getFrequencyForms ()
{
	return eventForms;
}
*/

std::pair<DioRows::which, UINT> MicrowaveCore::getRsgTriggerLine ()
{
	return rsgTriggerLine;
}


UINT MicrowaveCore::getNumTriggers (UINT variationNumber, microwaveSettings settings)
{
	return settings.list.size () == 1 ? 0 : settings.list.size ();
}

