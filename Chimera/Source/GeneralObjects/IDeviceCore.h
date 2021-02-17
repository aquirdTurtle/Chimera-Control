#pragma once

#include "ParameterSystem/ParameterSystemStructures.h"
#include <string>
#include <vector>
#include <qobject.h>

class ConfigStream;
class DataLogger;
class ExpThreadWorker;

// inheritance
// an abstract interface class which all device cores inherit from
class IDeviceCore : public QObject{
	public:
		// virtual void programNow () = 0;
		virtual void loadExpSettings (ConfigStream& stream) = 0;
		virtual void logSettings (DataLogger& logger, ExpThreadWorker* threadworker) = 0;
		virtual void calculateVariations (std::vector<parameterType>& params, ExpThreadWorker* threadworker) = 0;
		virtual void programVariation (unsigned variation, std::vector<parameterType>& params, 
										ExpThreadWorker* threadworker) = 0;
		void notify(statusMsg msg, ExpThreadWorker* threadworker);
		virtual void normalFinish () = 0;
		virtual void errorFinish () = 0;
		virtual std::string getDelim()=0;
		bool experimentActive;
};

