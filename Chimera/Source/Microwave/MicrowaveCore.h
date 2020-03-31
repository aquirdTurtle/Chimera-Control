#pragma once
#include "RsgFlume.h"
#include "ConfigurationSystems/Version.h"
#include "Microwave/WindFreakFlume.h"
#include "GeneralObjects/IDeviceCore.h"
#include "Microwave/microwaveSettings.h"
#include "LowLevel/constants.h"

class MicrowaveCore : public IDeviceCore
{
	public:
		MicrowaveCore ();
		std::string queryIdentity ();
		void setFmSettings ();
		void setPmSettings ();
		void programVariation (UINT variationNumber, std::vector<parameterType>& params);
		void calculateVariations (std::vector<parameterType>& params, Communicator& comm);
		void calculateVariations (std::vector<parameterType>& params);
		std::pair<DoRows::which, UINT> getRsgTriggerLine ();
		UINT getNumTriggers (microwaveSettings settings);
		microwaveSettings getSettingsFromConfig (ConfigStream& openFile);
		std::string configDelim = "MICROWAVE_SYSTEM";
		std::string getDelim () { return configDelim; }
		void logSettings (DataLogger& log);
		void loadExpSettings (ConfigStream& stream);
		void normalFinish () {};
		void errorFinish () {};
		microwaveSettings experimentSettings;
	private:
		const double triggerTime = 0.01;
		const std::pair<DoRows::which, UINT> rsgTriggerLine = { DoRows::which::C, 3 };
		MICROWAVE_FLUME uwFlume;
		
};