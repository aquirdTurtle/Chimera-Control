// created by Mark O. Brown
#pragma once
#include "DioRows.h"


struct servoInfo
{
	std::string servoName="";
	bool active = false;
	UINT aiInputChannel=0;
	UINT aoControlChannel=0;
	double setPoint=0;
	double controlValue=0;
	double tolerance = 0.02;
	double gain = 0.02;
	bool servoed=false;
	std::vector<std::pair<DioRows::which, UINT> > ttlConfig;
};
