#pragma once

#include "ParameterSystem/ParameterSystemStructures.h"
#include <string>
#include <vector>
#include <qobject.h>

class ConfigStream;
class DataLogger;
class ExpThreadWorker;

class IDeviceCore : public QObject{
	public:
		// virtual void programNow () = 0;
		virtual void loadExpSettings (ConfigStream& stream) = 0;
		virtual void logSettings (DataLogger& logger) = 0;
		virtual void calculateVariations (std::vector<parameterType>& params, ExpThreadWorker* threadworker) = 0;
		virtual void programVariation (unsigned variation, std::vector<parameterType>& params) = 0;
		virtual void normalFinish () = 0;
		virtual void errorFinish () = 0;
		virtual std::string getDelim()=0;
		bool experimentActive;
};
