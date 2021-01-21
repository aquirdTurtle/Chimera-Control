#pragma once
#include <string>
#include <vector>
#include <DigitalOutput/DoRows.h>
#include <Agilent/whichAg.h>

struct calBase {
	static std::string dblVecToString (std::vector<double> ctrls) {
		std::string ctrlString;
		for (auto val : ctrls) {
			ctrlString += str (val, 8, true) + " ";
		}
		return ctrlString;
	}
};

struct calResult : public calBase {
	std::string calibrationName = "";
	unsigned polynomialOrder = 0;
	std::vector<double> ctrlVals;
	std::vector<double> resVals;
	std::vector<double> calibrationCoefficients;
	bool includesSqrt=true;
	double calmin = 0;
	double calmax = 0;
	std::string stringRepr () const {
		return calibrationName + " Calibration:"
			+ "\n================================"
			+ "\nControl Values: " + dblVecToString(ctrlVals) 
			+ "\nResult Values: " + dblVecToString(resVals) 
			+ "\nCalibration Coefficients: " + dblVecToString (calibrationCoefficients) 
			+ "\nIncludes Sqrt: " + ( includesSqrt ? "True" : "False" )
			+ "\nMin/Max: " + str (calmin) + "/" + str (calmax);
	}
};

struct calSettings : public calBase {
	bool active = false;
	unsigned int aiInChan = 0;
	unsigned int aoControlChannel = 0;
	bool useAg = false;
	AgilentEnum::name whichAg;
	unsigned agChannel;
	QString ctrlPtString;
	// in % from old value
	bool calibrated = false;
	std::vector<std::pair<DoRows::which, unsigned> > ttlConfig;
	std::vector<std::pair<unsigned, double>> aoConfig;
	bool currentlyCalibrating = false;
	unsigned int avgNum = 100;
	// importantly, this value reflects the setting for the next calibration, whereas the version in the result 
	// refers to whether that particular result includes the sqrt. 
	bool includeSqrt = true;
	calResult result;
	calResult historicalResult;
};

Q_DECLARE_METATYPE (calSettings)