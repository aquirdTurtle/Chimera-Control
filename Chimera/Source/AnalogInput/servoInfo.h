// created by Mark O. Brown
#pragma once
#include "DigitalOutput/DoRows.h"

struct servoInfo{
	// in this case the system doesn't try to feed back, it just checks the value. generally should coincide with a 
	// larger tolerance. 
	bool monitorOnly = false;
	std::string servoName="";
	bool active = false;
	unsigned int aiInChan=0;
	unsigned int aoControlChannel=0;
	double setPoint=0;
	double controlValue=0;
	// in % from old value
	double changeInCtrl=0;
	double tolerance = 0.02;
	double gain = 0.02;
	bool servoed=false;
	double mostRecentResult = 0;
	std::vector<std::pair<DoRows::which, unsigned> > ttlConfig;
	std::vector<std::pair<unsigned, double>> aoConfig;
	bool currentlyServoing = false;
	unsigned int avgNum=100;
};
