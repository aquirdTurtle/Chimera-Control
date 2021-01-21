// created by Mark O. Brown
#pragma once
#include "GeneralImaging/imageParameters.h"
#include "Andor/AndorRunMode.h"
#include "Andor/AndorTriggerModes.h"
#include <string>
#include <vector>

// this structure contains all of the main options which are necessary to set when starting a camera acquisition. All
// of these settings should be possibly modified by the user of the UI.
struct AndorRunSettings{
	unsigned horShiftSpeedSetting = 0;
	unsigned vertShiftSpeedSetting = 0;

	imageParameters imageSettings;
	bool controlCamera = true;
	//
	bool emGainModeIsOn = false;
	int emGainLevel = 0;
	int readMode = 4;
	AndorRunModes::mode acquisitionMode = AndorRunModes::mode::Kinetic;
	/* frameTransferMode = 0 (off).
	Slower than when on. Cleans between images. Mech. shutter may not be necessary.
	   frameTransferMode = 1 (on).
	Faster than when off. Does not clean between images, giving better background. Mech. Shutter may be necessary.
    See iXonEM+ hardware guide pg 42.
	*/
	int frameTransferMode = 1;
	AndorTriggerMode::mode triggerMode = AndorTriggerMode::mode::External;
	bool showPicsInRealTime = false;
	//
	float kineticCycleTime = 0.1f;
	float accumulationTime = 0;
	int accumulationNumber = 1;
	std::vector<float> exposureTimes = { 0.026f };
	//
	unsigned picsPerRepetition=1;
	unsigned __int64 repetitionsPerVariation=10;
	unsigned __int64 totalVariations = 3;
	unsigned __int64 totalPicsInVariation();
	// this is an int to reflect that the final number that's programmed to the camera is an int
	int totalPicsInExperiment();
	int temperatureSetting = 25;
};

/*
	- Includes AndorRunSettings, which are the settings that the camera itself cares about.
	- Also some auxiliary settings which are never directly programmed to the camera, but are key for the way the 
	camera is used in the code.
*/
struct AndorCameraSettings{
	AndorRunSettings andor;
	// not directly programmed to camera
	std::array<std::vector<int>, 4> thresholds;
	std::array<int, 4> palleteNumbers;
	double mostRecentTemp;
};

