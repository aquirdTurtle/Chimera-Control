#pragma once
#include "ConfigurationSystems/Version.h"
#include "Microwave/WindFreakFlume.h"
#include "GeneralObjects/IDeviceCore.h"
#include "Microwave/microwaveSettings.h"
#include "LowLevel/constants.h"

class MicrowaveCore : public IDeviceCore{
	public:
		MicrowaveCore ();
		std::string queryIdentity ();
		void setFmSettings ();
		void setPmSettings ();
		void programVariation (unsigned variationNumber, std::vector<parameterType>& params, ExpThreadWorker* threadworker);
		void calculateVariations (std::vector<parameterType>& params, ExpThreadWorker* threadworker);
		std::pair<DoRows::which, unsigned> getUWaveTriggerLine ();
		unsigned getNumTriggers (microwaveSettings settings);
		microwaveSettings getSettingsFromConfig (ConfigStream& openFile);
		std::string configDelim = "MICROWAVE_SYSTEM";
		std::string getDelim () { return configDelim; }
		void logSettings (DataLogger& log, ExpThreadWorker* threadworker);
		void loadExpSettings (ConfigStream& stream);
		void normalFinish () {};
		void errorFinish () {};
		std::string getCurrentList ();
		microwaveSettings experimentSettings;
		WindFreakFlume uwFlume;

		void setTrigTime (double time);

	private:
		double triggerTime = 0.5;
		const std::pair<DoRows::which, unsigned> uwaveTriggerLine = { DoRows::which::D, 0 };
};