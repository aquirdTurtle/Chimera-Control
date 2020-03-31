#pragma once

//#include "DataLogging/DataLogger.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "ParameterSystem/ParameterSystemStructures.h"
#include "ExperimentThread/Communicator.h"
#include <string>
#include <vector>
#include "afxwin.h"

class DataLogger;

class IDeviceCore
{
	public:
		// virtual void programNow () = 0;
		virtual void programVariation (UINT variation, std::vector<parameterType>& params) = 0;
		virtual void calculateVariations (std::vector<parameterType>& params, Communicator& comm) = 0;
		virtual void logSettings (DataLogger& logger)=0;
		virtual void loadExpSettings (ConfigStream& stream) = 0;
		virtual void normalFinish () = 0;
		virtual void errorFinish () = 0;
		// most devices have a function like this, but I want to make the class independent of the data type.
		// virtual DeviceSettings getSettingsFromConfig (ConfigStream& stream, Version ver) = 0;
		virtual std::string getDelim()=0;
		bool experimentActive;
};
