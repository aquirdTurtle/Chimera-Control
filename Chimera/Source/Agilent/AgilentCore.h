#pragma once
#include "ParameterSystem/ParameterSystemStructures.h"
#include "GeneralFlumes/VisaFlume.h"
#include "DigitalOutput/DoRows.h"
#include <vector>
#include <string>


class AgilentCore
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		AgilentCore& operator=(const AgilentCore&) = delete;
		AgilentCore (const AgilentCore&) = delete;

		AgilentCore (const agilentSettings& settings);
		~AgilentCore ();
		void initialize ();
		void setAgilent (UINT variation, std::vector<parameterType>& params, deviceOutputInfo runSettings);
		//void setAgilent (deviceOutputInfo runSettings);
		void prepAgilentSettings (UINT channel);
		const std::string configDelim;
		bool connected ();
		std::pair<DoRows::which, UINT> getTriggerLine ();
		std::string getDeviceIdentity ();
		void convertInputToFinalSettings (UINT variation, UINT chan, deviceOutputInfo& info,
											std::vector<parameterType>& variables = std::vector<parameterType> ());
		static double convertPowerToSetPoint (double power, bool conversionOption, std::vector<double> calibCoeff);
		void setScriptOutput (UINT varNum, scriptedArbInfo scriptInfo, UINT channel);
		void setDC (int channel, dcInfo info, UINT variation);
		void setExistingWaveform (int channel, preloadedArbInfo info);
		void setSquare (int channel, squareInfo info, UINT variation);
		void setSine (int channel, sineInfo info, UINT variation);
		void outputOff (int channel);
		void analyzeAgilentScript (scriptedArbInfo& infoObj, std::vector<parameterType>& vars, std::string& warnings);
		void setDefault (int channel);
		std::vector<std::string> getStartupCommands ();
		void programSetupCommands ();
		std::string getDeviceInfo ();
		void handleScriptVariation (UINT variation, scriptedArbInfo& scriptInfo, UINT channel, 
			std::vector<parameterType>& params);
	private:
		const double sampleRate;
		const std::string memoryLoc;
		const agilentSettings initSettings;
		// not that important, just used to check that number of triggers in script matches number in agilent.
		const DoRows::which triggerRow;
		const UINT triggerNumber;
		VisaFlume visaFlume;
		bool isConnected;
		std::string deviceInfo;
		// includes burst commands, trigger commands, etc. This is a place for any commands which don't have a 
		// GUI control option. You could also use this to put commands that should be the same for all configurations.
		const std::vector<std::string> setupCommands;
		/* a list of polynomial coefficients for the calibration.
		auto& cc = calibrationCoefficients
		Volt = cc[0] + c[1]*sp + c[2]*sp^2 + ...
		*/
		const std::vector<double> calibrationCoefficients;
		const std::string agilentName;
};