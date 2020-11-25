#pragma once
#include "ParameterSystem/ParameterSystemStructures.h"
#include "GeneralFlumes/VisaFlume.h"
#include "DigitalOutput/DoRows.h"
#include "GeneralObjects/IDeviceCore.h"
#include "ConfigurationSystems/ConfigStream.h"
#include <vector>
#include <string>
#include <AnalogInput/calInfo.h>

class DoCore;

class AgilentCore : public IDeviceCore {
	public:
		// THIS CLASS IS NOT COPYABLE.
		AgilentCore& operator=(const AgilentCore&) = delete;
		AgilentCore (const AgilentCore&) = delete;

		AgilentCore (const agilentSettings& settings);
		~AgilentCore ();
		void programBurstMode (int channel, bool burstOption);
		void initialize ();
		void setAgilent (unsigned variation, std::vector<parameterType>& params, deviceOutputInfo runSettings);
		void prepAgilentSettings (unsigned channel);
		std::string getDelim () { return configDelim; }
		const std::string configDelim;
		bool connected ();
		std::pair<DoRows::which, unsigned> getTriggerLine ();
		std::string getDeviceIdentity ();
		void logSettings (DataLogger& log, ExpThreadWorker* threadworker);
		void convertInputToFinalSettings (unsigned chan, deviceOutputInfo& info,
										  std::vector<parameterType>& variables = std::vector<parameterType> ());
		static double convertPowerToSetPoint (double power, bool conversionOption, calResult calibraiton);
		void setScriptOutput (unsigned varNum, scriptedArbInfo scriptInfo, unsigned channel);
		void setDC (int channel, dcInfo info, unsigned variation);
		void setExistingWaveform (int channel, preloadedArbInfo info);
		void setSquare (int channel, squareInfo info, unsigned variation);
		void setSine (int channel, sineInfo info, unsigned variation);
		void outputOff (int channel);
		void analyzeAgilentScript (scriptedArbInfo& infoObj, std::vector<parameterType>& vars, std::string& warnings);
		void setDefault (int channel);
		std::vector<std::string> getStartupCommands ();
		void programSetupCommands ();
		std::string getDeviceInfo ();
		void handleScriptVariation (unsigned variation, scriptedArbInfo& scriptInfo, unsigned channel,
									 std::vector<parameterType>& params);
		deviceOutputInfo getSettingsFromConfig (ConfigStream& file);
		void loadExpSettings (ConfigStream& script);
		void calculateVariations (std::vector<parameterType>& params, ExpThreadWorker* threadworker);
		void programVariation (unsigned variation, std::vector<parameterType>& params, ExpThreadWorker* threadworker);
		void checkTriggers (unsigned variationInc, DoCore& ttls, ExpThreadWorker* threadWorker);
		void normalFinish () {};
		void errorFinish () {};
		void setCalibration (calResult newCal, unsigned chan);
		void setRunSettings (deviceOutputInfo newSettings);
	private:
		const int AGILENT_DEFAULT_POWER = 10;

		deviceOutputInfo expRunSettings;
		const unsigned long sampleRate;
		const std::string memoryLoc;
		const agilentSettings initSettings;
		// not that important, just used to check that number of triggers in script matches number in agilent.
		const DoRows::which triggerRow;
		const unsigned triggerNumber;
		VisaFlume visaFlume;
		bool isConnected;
		std::string deviceInfo;

		std::array<calResult, 2> calibrations;

		// includes burst commands, trigger commands, etc. This is a place for any commands which don't have a 
		// GUI control option. You could also use this to put commands that should be the same for all configurations.
		const std::vector<std::string> setupCommands;

		/* a list of polynomial coefficients for the calibration.
		auto& cc = calibrationCoefficients
		Volt = cc[0] + c[1]*sp + c[2]*sp^2 + ...
		*/
		//const std::vector<double> calibrationCoefficients;
		const std::string agilentName;
};