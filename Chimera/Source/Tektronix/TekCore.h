#pragma once

#include "GeneralFlumes/VisaFlume.h"
#include "ConfigurationSystems/Version.h"
#include "GeneralObjects/IDeviceCore.h"
#include "DataLogging/DataLogger.h"
#include "TektronixStructures.h"

class TekCore : public IDeviceCore
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		TekCore& operator=(const TekCore&) = delete;
		TekCore (const TekCore&) = delete;

		TekCore (bool safemode, std::string address, std::string configurationFileDelimiter);
		~TekCore ();
		std::string queryIdentity ();
		void calculateVariations (std::vector<parameterType>& parameters, ExpThreadWorker* threadworker);
		tektronixInfo getSettingsFromConfig (ConfigStream& configFile);
		void programVariation (UINT variation, std::vector<parameterType>& params);
		const std::string configDelim;
		std::string getDelim () { return configDelim; }
		void logSettings (DataLogger& logger);
		void loadExpSettings (ConfigStream& stream);
		void normalFinish () {};
		void errorFinish () {};
		void calculateVariations (std::vector<parameterType>& parameters);
	private:
		tektronixInfo experimentInfo;
		VisaFlume visaFlume;
};