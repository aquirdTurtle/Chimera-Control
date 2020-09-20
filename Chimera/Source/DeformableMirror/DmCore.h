#pragma once
#include "DmFlume.h"
#include "ParameterSystem/Expression.h"
#include "GeneralObjects/IDeviceCore.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "DmOutputForm.h"
#include "DmProfileCreator.h"

class DmProfileCreator;

class DmCore : public IDeviceCore{
	public:
	    DmCore(std::string Number, bool safemodeOption);
		std::string getDeviceInfo();
		void setSerial(std::string Number);
		void initialize();
		void setMap();
		void pokePiston(unsigned int piston, double value);
		void zeroPistons();
		void readDMArray(std::vector<double> &testArray);
		void testMirror();
		void loadArray(double *A);
		int getActCount();
		std::string getDelim () { return configDelim; }
		std::vector<double> getActuatorValues();
		void handleSaveConfig( ConfigStream& newFile, DMOutputForm out );
		DMOutputForm getSettingsFromConfig(ConfigStream& configFile);
		static void interpretKey(std::vector<std::vector<parameterType>>& variables, DmCore& DM);
		void ProgramNow(unsigned variation);
		DMOutputForm getInfo();
		void setCurrentInfo(DMOutputForm form);
		void initialCheck(unsigned variation, std::string& warnings);
		void logSettings (DMOutputForm settings, DataLogger& log);
		std::string configDelim = "DM";
		void programVariation (unsigned variation, std::vector<parameterType>& params, ExpThreadWorker* threadworker) {};
		void calculateVariations (std::vector<parameterType>& params, ExpThreadWorker* threadworker) {};
		void logSettings (DataLogger& logger, ExpThreadWorker* threadworker) {};
		void loadExpSettings (ConfigStream& stream) {};
		void normalFinish () {};
		void errorFinish () {};
	private:
		DmProfileCreator profile;
		DmFlume DM;
		std::vector<unsigned int> map_lut = std::vector<unsigned int>(MAX_DM_SIZE, 0);
		std::vector<double> valueArray;
		std::string serial;
		bool boolOfSerial;
		DMOutputForm currentInfo;
};
