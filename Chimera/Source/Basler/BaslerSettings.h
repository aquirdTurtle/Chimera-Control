// created by Mark O. Brown
#pragma once
#include <string>
#include "GeneralImaging/imageParameters.h"

struct BaslerAutoExposure
{
	enum class mode
	{
		Continuous,
		Off,
		Once
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
	bool on;
	unsigned int rawGain;
	BaslerAutoExposure::mode exposureMode= BaslerAutoExposure::mode::Off;
	double exposureTime;
	BaslerAcquisition::mode acquisitionMode = BaslerAcquisition::mode::Continuous;
	UINT picsPerRep;
	UINT repsPerVar;
	UINT variations;
	BaslerTrigger::mode triggerMode = BaslerTrigger::mode::External;
	double frameRate;
	imageParameters dims;
	UINT totalPictures () { return picsPerRep * repsPerVar * variations; }
};


