#include "stdafx.h"  
#include <iostream> 
#include <vector> 
#include <string> 
#include "afxwin.h"

struct agilentSettings
{
	bool safemode;
	std::string address;
	ULONG sampleRate;
	std::string outputImpedance;
	std::string filterState;
	// "INT" or "USB"
	std::string memoryLocation;
	std::string deviceName;

	// button ids.
	ULONG chan1ButtonId;
	ULONG chan2ButtonId;
	ULONG syncButtonId;
	ULONG agilentComboId;
	ULONG functionComboId;
	ULONG editId;
	ULONG programButtonId;
	ULONG calibrationButtonId;

	ULONG triggerRow;
	ULONG triggerNumber;
	std::string configurationFileDelimiter;

	std::vector<double> calibrationCoeff;
};

int main( )
{

	const agilentSettings test = {
		0,
		"",
		10e6,
		"50",
		"NORMal",
		"INT",
		"UWave",
		0, 0,
		0, 0,
		0, 0, 0,
		0,
		0, 0,
		"MICROWAVE_AGILENT_AWG",
		{ 0.000505870656651, -0.0108632090621 }
	};

	std::cout << test.calibrationCoeff.size ( );
	std::cin.get ( );

	return 0;
}
 

