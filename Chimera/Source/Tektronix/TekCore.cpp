#include "stdafx.h"
#include "TekCore.h"
#include "TektronixStructures.h"

TekCore::TekCore (bool safemode, std::string address, std::string configurationFileDelimiter) : 
	visaFlume (safemode, address), configDelim (configurationFileDelimiter)
{
	try
	{
		visaFlume.open ();
	}
	catch (Error & err)
	{
		errBox ("Failed to initialize tektronics visa connection! " + err.trace ());
	}
}
TekCore::~TekCore ()
{
	visaFlume.close ();
}

void TekCore::programMachine (UINT variation, tektronixInfo& runInfo)
{
	if (runInfo.channels[0].control || runInfo.channels[1].control)
	{
		for (auto channelInc : range (runInfo.channels.size ()))
		{
			auto& channel = runInfo.channels[channelInc];
			auto ch_s = str (channelInc+1);
			if (channel.control)
			{
				if (channel.on)
				{
					visaFlume.write ("SOURCE" + ch_s + ":FREQ " + str (channel.mainFreq.getValue (variation)));
					visaFlume.write ("SOURCE" + ch_s + ":VOLT:UNIT DBM");
					visaFlume.write ("SOURCE" + ch_s + ":VOLT " + str (channel.power.getValue (variation)));
					visaFlume.write ("SOURCE" + ch_s + ":VOLT:OFFS 0");
					if (channel.fsk)
					{
						visaFlume.write ("SOURCE" + ch_s + ":FSKey:STATe On");
						visaFlume.write ("SOURCE" + ch_s + ":FSKey:FREQ " + str (channel.fskFreq.getValue (variation)));
						visaFlume.write ("SOURCE" + ch_s + ":FSKey:SOURce External");
					}
					else
					{
						visaFlume.write ("SOURCE" + ch_s + ":FSKey:STATe Off");
					}
					visaFlume.write ("OUTput" + ch_s + ":STATe ON");
				}
				else
				{
					visaFlume.write ("OUTput" + ch_s + ":STATe OFF");
				}
			}
		}
	}
}

std::string TekCore::queryIdentity ()
{
	try
	{
		auto res = visaFlume.identityQuery ();
		return res;
	}
	catch (Error & err)
	{
		return err.trace ();
	}
}

void TekCore::interpretKey ( std::vector<parameterType>& parameters, tektronixInfo& runInfo )
{
	UINT variations;
	UINT sequenceNumber;
	if (parameters.size () == 0)
	{
		variations = 1;
	}
	else
	{
		variations = parameters.front ().keyValues.size ();
	}
	for (auto& channel : runInfo.channels)
	{
		if (channel.on)
		{
			channel.mainFreq.internalEvaluate (parameters, variations);
			channel.power.internalEvaluate (parameters, variations);
			if (channel.fsk) { channel.fskFreq.internalEvaluate (parameters, variations); }
		}
	}
}
