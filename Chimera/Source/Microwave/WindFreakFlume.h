#pragma once

#include <string>
#include "GeneralFlumes/WinSerialFlume.h"
#include "Microwave/microwaveSettings.h"

// this is a small wrapper around a serial flume to introduce microwave-specific commands.
class WindFreakFlume : public WinSerialFlume {
	public:
		WindFreakFlume (std::string portAddress, bool safemode);
		std::string queryIdentity ();
		void setPmSettings ();
		void setFmSettings ();
		void programSingleSetting (microwaveListEntry setting, unsigned varNumber);
		void programList (std::vector<microwaveListEntry> list, unsigned varNum, double triggerTime);
		std::string getListString ();
};
