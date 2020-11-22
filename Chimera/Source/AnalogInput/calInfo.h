#pragma once
#include <string>
#include <vector>
#include <DigitalOutput/DoRows.h>
#include <Agilent/whichAg.h>

struct calInfo {
	std::string calibrationName = "";
	bool active = false;
	unsigned int aiInChan = 0;
	unsigned int aoControlChannel = 0;
	bool useAg = false;
	AgilentEnum::name whichAg;
	unsigned agChannel;
	std::vector<double> controlPoints;
	std::vector<double> resultValues;
	// in % from old value
	bool calibrated = false;
	std::vector<std::pair<DoRows::which, unsigned> > ttlConfig;
	std::vector<std::pair<unsigned, double>> aoConfig;
	bool currentlyCalibrating = false;
	unsigned int avgNum = 100;
	std::vector<double> calibrationCoefficients;
};