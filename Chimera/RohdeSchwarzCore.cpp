#include "stdafx.h"
#include "RohdeSchwarzCore.h"


RohdeSchwarzCore::RohdeSchwarzCore() : gpibFlume (RSG_ADDRESS, RSG_SAFEMODE) {}

void RohdeSchwarzCore::programRsg (UINT variationNumber)
{
	if (events[variationNumber].size () == 0)
	{
		// nothing to do.
		return;
	}
	else
	{
		setPmSettings ();
		if (events[variationNumber].size () == 1)
		{
			gpibFlume.send ("OUTPUT ON");
			gpibFlume.send ("SOURce:FREQuency:MODE CW");
			gpibFlume.send ("FREQ " + str (events[variationNumber][0].frequency, 13) + " GHz");
			gpibFlume.send ("POW " + str (events[variationNumber][0].power, 13) + " dBm");
			gpibFlume.send ("OUTP ON");
		}
		else
		{
			gpibFlume.send ("OUTP ON");
			gpibFlume.send ("SOURce:LIST:SEL 'freqList" + str (events.size ()) + "'");
			std::string freqList = "SOURce:LIST:FREQ " + str (events[variationNumber][0].frequency, 13) + " GHz";
			std::string powerList = "SOURce:LIST:POW " + str (events[variationNumber][0].power, 13) + "dBm";
			for (UINT eventInc = 1; eventInc < events[variationNumber].size (); eventInc++)
			{
				freqList += ", ";
				freqList += str (events[variationNumber][eventInc].frequency, 13) + " GHz";
				powerList += ", ";
				powerList += str (events[variationNumber][eventInc].power, 13) + "dBm";
			}
			gpibFlume.send (cstr (freqList));
			gpibFlume.send (cstr (powerList));
			gpibFlume.send ("SOURce:LIST:MODE STEP");
			gpibFlume.send ("SOURce:LIST:TRIG:SOURce EXT");
			gpibFlume.send ("SOURce:FREQ:MODE LIST");
		}
	}
}

std::string RohdeSchwarzCore::queryIdentity ()
{
	return gpibFlume.queryIdentity ();
}


void RohdeSchwarzCore::setFmSettings ()
{
	gpibFlume.send ("SOURCE:PM1:STATe OFF");
	gpibFlume.send ("SOURCE:FM:MODE NORMal");
	gpibFlume.send ("SOURCE:FM:RATio 100PCT");
	gpibFlume.send ("SOURCE:FM1:SOURce EXT1");
	gpibFlume.send ("SOURCE:FM1:DEViation 20kHz");
	gpibFlume.send ("SOURCE:FM1:STATe ON");
}


void RohdeSchwarzCore::setPmSettings ()
{
	gpibFlume.send ("SOURCE:FM1:STATe OFF");
	gpibFlume.send ("SOURCE:PM:MODE HDEViation");
	gpibFlume.send ("SOURCE:PM:RATio 100PCT");
	gpibFlume.send ("SOURCE:PM1:SOURce EXT1");
	gpibFlume.send ("SOURCE:PM1:DEViation 6.28RAD");
	gpibFlume.send ("SOURCE:PM1:STATe ON");
}

/*
 * The following function takes the existing list of events (already evaluated for a particular variation) and
 * orders them in time.
 */
void RohdeSchwarzCore::orderEvents (UINT variation)
{
	std::vector<rsgEvent> newOrder;
	for (auto event : events[variation])
	{
		bool set = false;
		int count = 0;
		// deal with the first case.
		if (newOrder.size () == 0)
		{
			newOrder.push_back (event);
			continue;
		}

		for (UINT eventInc = 0; eventInc < newOrder.size (); eventInc++)
		{
			if (newOrder[eventInc].time > event.time)
			{
				newOrder.insert (newOrder.begin () + count, event);
				set = true;
				break;
			}
		}

		if (!set)
		{
			newOrder.push_back (event);
		}
	}
	events[variation] = newOrder;
}


void RohdeSchwarzCore::interpretKey (std::vector<std::vector<parameterType>>& params)
{
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
	events.clear ();
	events.resize (variations);
	for (auto seqNum : range (sequencNumber))
	{
		for (auto variationNumber : range(variations))
		{
			for (auto freqInc : range(eventForms.size()))
			{
				rsgEvent event;
				event.frequency = eventForms[freqInc].frequency.evaluate (params[seqNum], variationNumber);
				event.power = eventForms[freqInc].power.evaluate (params[seqNum], variationNumber);
				/// deal with time!
				if (eventForms[freqInc].time.first.size () == 0)
				{
					event.time = eventForms[freqInc].time.second;
				}
				else
				{
					event.time = 0;
					for (auto timeStr : eventForms[freqInc].time.first)
					{
						event.time += timeStr.evaluate (params[seqNum], variationNumber);
					}
					event.time += eventForms[freqInc].time.second;
				}
				events[variationNumber].push_back (event);
			}
		}
	}
}

// Essentially gets called by a script command.
void RohdeSchwarzCore::addFrequency (rsgEventForm info)
{
	eventForms.push_back (info);
}


void RohdeSchwarzCore::clearFrequencies ()
{
	eventForms.clear ();
	events.clear ();
}

std::vector<rsgEventForm> RohdeSchwarzCore::getFrequencyForms ()
{
	return eventForms;
}


std::pair<DioRows::which, UINT> RohdeSchwarzCore::getRsgTriggerLine ()
{
	return rsgTriggerLine;
}


UINT RohdeSchwarzCore::getNumTriggers (UINT variationNumber)
{
	return events[variationNumber].size () == 1 ? 0 : events[variationNumber].size ();
}
