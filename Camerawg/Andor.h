#pragma once
#include <string>
#include "CameraImageParameters.h"
#include <vector>

struct AndorBaseSettings
{

};

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
	float kinetiCycleTime;
	float accumulationTime;
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

class AndorCamera
{
	public:
		AndorCamera::AndorCamera();
		AndorRunSettings getSettings();
		void setSettings(AndorRunSettings settingsToSet);
		int setSystem(CameraWindow* camWin, Communicator* comm);
		BOOL acquireImageData(Communicator* comm);
		int setAcquisitionMode(Communicator* comm);
		int setReadMode(Communicator* comm);
		int setTemperature(Communicator* comm);
		int setExposures(Communicator* comm);
		int setImageParametersToCamera(Communicator* comm);
		int setKineticCycleTime(Communicator* comm);
		int setScanNumber(Communicator* comm);
		int setFrameTransferMode(Communicator* comm);
		int checkAcquisitionTimings(float& kinetic, float& accumulation, std::vector<float>& exposures, Communicator* comm);
		void confirmAcquisitionTimings(float& kinetic, float& accumulation, std::vector<float>& exposures, Communicator* comm);
		int getStatus(Communicator* comm);
		int startAcquisition(Communicator* comm);
		int setAccumulationCycleTime(Communicator* comm);
		int setNumberAccumulations(bool isKinetic, Communicator* comm);
	
		int setTriggerMode(Communicator* comm);
		int setGainMode(Communicator* comm);

		void drawDataWindow(void);

		void changeTemperatureSetting(bool temperatureControlOff, Communicator* comm);

		std::string andorErrorChecker(int errorCode);
	private:
		/// These are official settings and are the final say on what the camera does. Some unofficial 
		/// settings are stored in smaller classes.
		// If the experiment is running, these settings hold the options that the experiment is using.
		AndorBaseSettings baseSettings;
		AndorRunSettings runSettings;
		// ??? 
		imageParameters readImageParameters;
		imageParameters runningImageParameters;
		// 
		bool cameraIsRunning;
		int currentRepetitionNumber;
		HANDLE plottingMutex;
		// ???
		HANDLE imagesMutex;
		bool plotThreadExitIndicator;

		std::vector<std::vector<long> > imagesOfExperiment;
		std::vector<std::vector<long> > imageVecQueue;
};
