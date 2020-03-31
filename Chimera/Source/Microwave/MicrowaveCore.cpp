#include "stdafx.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ProfileSystem.h"
#include "MicrowaveCore.h"


MicrowaveCore::MicrowaveCore() : uwFlume(UW_SYSTEM_ADDRESS, UW_SYSTEM_SAFEMODE){}

void MicrowaveCore::programVariation (UINT variationNumber, std::vector<parameterType>& params)
{
	if (!experimentActive) { return; }
	if (!experimentSettings.control || experimentSettings.list.size () == 0)
	{
		// Nothing to program.
		return;
	}
	setPmSettings ();
	if (experimentSettings.list.size () == 1)
	{
		uwFlume.programSingleSetting (experimentSettings.list[0], variationNumber);
	}
	else
	{
		uwFlume.programList (experimentSettings.list, variationNumber);
	}
}

void MicrowaveCore::logSettings (DataLogger& log)
{

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

void MicrowaveCore::calculateVariations (std::vector<parameterType>& params, Communicator& comm)
{
	calculateVariations (params);
}
void MicrowaveCore::calculateVariations (std::vector<parameterType>& params)
{
	if (!experimentSettings.control)
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
	for (auto freqInc : range(experimentSettings.list.size()))
	{
		experimentSettings.list[freqInc].frequency.internalEvaluate (params, variations);
		experimentSettings.list[freqInc].power.internalEvaluate (params, variations);
	}
}

std::pair<DoRows::which, UINT> MicrowaveCore::getRsgTriggerLine ()
{
	return rsgTriggerLine;
}


UINT MicrowaveCore::getNumTriggers (microwaveSettings settings)
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

void MicrowaveCore::loadExpSettings (ConfigStream& stream)
{
	ProfileSystem::stdGetFromConfig (stream, *this, experimentSettings);
	experimentActive = experimentSettings.control;
}
