// created by Mark O. Brown
#pragma once
#include <string>
#include "GeneralImaging/imageParameters.h"

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
	UINT picsPerRep;
	UINT repsPerVar;
	UINT variations;
	BaslerTrigger::mode triggerMode;
	double frameRate;
	imageParameters dims;
	UINT totalPictures () { return picsPerRep * repsPerVar * variations; }
};


