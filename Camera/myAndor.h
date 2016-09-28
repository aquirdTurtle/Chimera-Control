#pragma once
#include <string>
#include "CameraImageParameters.h"

namespace myAndor
{
	int setSystem(void);
	BOOL acquireImageData(void);

	int setAcquisitionMode(void);
	int setReadMode(void);
	int setTemperature(void);
	int setExposures(std::vector<float> exposureTimesToSet);
	int setImageParametersToCamera(imageParameters tempImageParam);
	int setKineticCycleTime(void);
	int setScanNumber(void);
	int setFrameTransferMode(int mode);
	int checkAcquisitionTimings(double& kinetic, double& accumulation, std::vector<float>& exposures);
	int getStatus();
	int startAcquisition();
	int setAccumulationCycleTime();
	int setNumberAccumulations(int kinetic);
	
	int setTriggerMode();
	int setGainMode();

	void drawDataWindow(void);

	void changeTemperatureSetting(bool temperatureControlOff);

	std::string andorErrorChecker(int errorCode);
}
