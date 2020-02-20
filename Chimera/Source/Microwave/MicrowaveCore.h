#pragma once
#include "RsgFlume.h"
#include "Microwave/WindFreakFlume.h"
#include "Microwave/microwaveSettings.h"
#include "LowLevel/constants.h"

class MicrowaveCore
{
	public:
		MicrowaveCore ();
		std::string queryIdentity ();
		void setFmSettings ();
		void setPmSettings ();
		void programRsg (UINT variationNumber, microwaveSettings settings);
		void interpretKey (std::vector<std::vector<parameterType>>& params, microwaveSettings& settings);
		std::pair<DoRows::which, UINT> getRsgTriggerLine ();
		UINT getNumTriggers (UINT variationNumber, microwaveSettings settings);
	private:
		const double triggerTime = 0.01;
		const std::pair<DoRows::which, UINT> rsgTriggerLine = { DoRows::which::C, 3 };
		MICROWAVE_FLUME uwFlume;
		//RsgFlume rsgFlume;
		//WindFreakFlume wfFlume;
};