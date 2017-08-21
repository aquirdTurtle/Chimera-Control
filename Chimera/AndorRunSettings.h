#pragma once
#include "imageParameters.h"
#include <string>
#include <vector>

// this structure contains all of the main options which are necessary to set when starting a camera acquisition. All
// of these settings should be possibly modified by the user of the UI.
struct AndorRunSettings
{
	imageParameters imageSettings;
	//
	bool emGainModeIsOn;
	int emGainLevel;
	int readMode;
	int acquisitionMode;
	int frameTransferMode;
	std::string triggerMode;
	std::string cameraMode;
	bool showPicsInRealTime;
	//
	float kineticCycleTime;
	float accumulationTime;
	int accumulationNumber;
	std::vector<float> exposureTimes;
	//
	int picsPerRepetition;
	int repetitionsPerVariation;
	int totalVariations;
	int totalPicsInExperiment;
	int totalPicsInVariation;
	// 
	int temperatureSetting;
};