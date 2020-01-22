#include "stdafx.h"
#include "MicrowaveCore.h"


MicrowaveCore::MicrowaveCore() :
	uwFlume(UW_SYSTEM_ADDRESS, UW_SYSTEM_SAFEMODE)
//	rsgFlume (RSG_ADDRESS, MICROWAVE_SYSTEM_DEVICE_TYPE != microwaveDevice::RohdeSchwarzGenerator),
//	wfFlume(MICROWAVE_SYSTEM_DEVICE_TYPE != microwaveDevice::WindFreak, WIND_FREAK_ADDR)
{
}

void MicrowaveCore::programRsg (UINT variationNumber, microwaveSettings settings)
{
	if (!settings.control || settings.list.size () == 0)
	{
		// Nothing to program.
		return;
	}
	setPmSettings ();
	if (settings.list.size () == 1)
	{
		uwFlume.programSingleSetting (settings.list[0], variationNumber);
	}
	else
	{
		uwFlume.programList (settings.list, variationNumber);
	}
}

std::string MicrowaveCore::queryIdentity ()
{
	return uwFlume.queryIdentity ();
}


void MicrowaveCore::setFmSettings ()
{
	uwFlume.setFmSettings ();
}


void MicrowaveCore::setPmSettings ()
{
	uwFlume.setPmSettings ();
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

std::pair<DioRows::which, UINT> MicrowaveCore::getRsgTriggerLine ()
{
	return rsgTriggerLine;
}


UINT MicrowaveCore::getNumTriggers (UINT variationNumber, microwaveSettings settings)
{
	return settings.list.size () == 1 ? 0 : settings.list.size ();
}

