#pragma once

#include "PiezoFlume.h"
#include "SerialPiezoFlume.h"
#include "GeneralObjects/IDeviceCore.h"
#include "ParameterSystem/Expression.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "Scripts/ScriptStream.h"
#include <Piezo/piezoChan.h>

struct piezoSettings
{
	piezoChan<Expression> pztValues;
	bool ctrlPzt;
};

class PiezoCore : public IDeviceCore
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		PiezoCore& operator=(const PiezoCore&) = delete;
		PiezoCore(const PiezoCore&) = delete;

		PiezoCore (piezoSetupInfo info);
		void initialize (  );
		std::string getDeviceInfo ( );
		std::string getDeviceList ( );
		void programVariation ( unsigned variationNumber, std::vector<parameterType>& params, ExpThreadWorker* threadworker);
		piezoSettings getSettingsFromConfig (ConfigStream& file );
		void programAll ( piezoChan<double> vals );
		double getCurrentXVolt ( );
		void programXNow ( double val );
		double getCurrentYVolt ( );
		void programYNow ( double val );
		double getCurrentZVolt ( );
		void programZNow ( double val );
		void calculateVariations ( std::vector<parameterType>& params, ExpThreadWorker* threadworker);
		const std::string configDelim;
		std::string getDelim () { return configDelim; }
		void logSettings (DataLogger& log, ExpThreadWorker* threadworker);
		void loadExpSettings (ConfigStream& stream);
		void normalFinish () {};
		void errorFinish () {};
		static std::string systemScope;

	private:
		const PiezoType controllerType;
		PiezoFlume flume;
		SerialPiezoFlume serFlume;
		piezoSettings expSettings;
};

