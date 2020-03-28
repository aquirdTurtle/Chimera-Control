#include "stdafx.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ProfileSystem.h"
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


void MicrowaveCore::interpretKey (std::vector<parameterType>& params, microwaveSettings& settings)
{
	if (!settings.control)
	{
		return;
	}
	UINT variations;
	if (params.size () == 0)
	{
		variations = 1;
	}
	else
	{
		variations = params.front ().keyValues.size ();
	}
	/// imporantly, this sizes the relevant structures.
	for (auto freqInc : range(settings.list.size()))
	{
		settings.list[freqInc].frequency.internalEvaluate (params, variations);
		settings.list[freqInc].power.internalEvaluate (params, variations);
	}
}

std::pair<DoRows::which, UINT> MicrowaveCore::getRsgTriggerLine ()
{
	return rsgTriggerLine;
}


UINT MicrowaveCore::getNumTriggers (UINT variationNumber, microwaveSettings settings)
{
	return settings.list.size () == 1 ? 0 : settings.list.size ();
}

microwaveSettings MicrowaveCore::getSettingsFromConfig (ConfigStream& openFile, Version ver)
{
	microwaveSettings settings;
	auto getlineF = ProfileSystem::getGetlineFunc (ver);
	openFile >> settings.control;
	UINT numInList = 0;
	openFile >> numInList;
	if (numInList > 100)
	{
		auto res = promptBox ("Detected suspiciously large number of microwave settings in microwave list. Number of list entries"
							  " was " + str (numInList) + ". Is this acceptable?", MB_YESNO);
		if (!res)
		{
			thrower ("Detected suspiciously large number of microwave settings in microwave list. Number of list entries"
					 " was " + str (numInList) + ".");
		}
	}
	settings.list.resize (numInList);
	if (numInList > 0)
	{
		openFile.get ();
	}
	for (auto num : range (numInList))
	{
		getlineF (openFile, settings.list[num].frequency.expressionStr);
		getlineF (openFile, settings.list[num].power.expressionStr);
	}
	return settings;
}