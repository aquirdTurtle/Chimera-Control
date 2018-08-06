#pragma once
#include <string>
#include "imageParameters.h"

struct BaslerAutoExposure
{
	enum class mode
	{
		Continuous,
		Once,
		Off
	};
	static std::string toStr( mode m );
	static mode fromStr ( std::string txt );
};


struct BaslerTrigger
{
	enum class mode
	{
		External,
		AutomaticSoftware,
		ManualSoftware
	};
	static std::string toStr ( mode m );
	static mode fromStr ( std::string txt );
};

struct BaslerAcquisition
{
	enum class mode
	{
		Finite,
		Continuous
	};
	static std::string toStr( mode m );
	static mode fromStr( std::string m );
};


struct baslerSettings
{
	unsigned int rawGain;
	BaslerAutoExposure::mode exposureMode;
	double exposureTime;
	BaslerAcquisition::mode acquisitionMode;
	unsigned int repCount;
	BaslerTrigger::mode triggerMode;
	double frameRate;
	imageParameters dimensions;
};


