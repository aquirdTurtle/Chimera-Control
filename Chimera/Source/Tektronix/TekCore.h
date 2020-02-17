#pragma once

#include "GeneralFlumes/VisaFlume.h"
#include "TektronixStructures.h"

class TekCore
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		TekCore& operator=(const TekCore&) = delete;
		TekCore (const TekCore&) = delete;

		TekCore (bool safemode, std::string address, std::string configurationFileDelimiter);
		~TekCore ();
		std::string queryIdentity ();
		void interpretKey (std::vector<std::vector<parameterType>>& parameters, tektronixInfo& runInfo);
		void programMachine (UINT variation, tektronixInfo& runInfo);
		const std::string configDelim;
	private:
		VisaFlume visaFlume;
};