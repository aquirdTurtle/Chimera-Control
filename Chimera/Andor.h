#pragma once
#include <string>
#include "CameraImageDimensions.h"
#include "Communicator.h"
#include <process.h>
#include <mutex>
#include "ATMCD32D.h"
#include "AndorRunSettings.h"

/// /////////////////////////////////////////////////////
/// 
///			The Andor Class
///
/// /////////////////////////////////////////////////////

// This class is designed to facilitate interaction with the andor camera and
// is based around the andor SDK. I did not write the Andor SDK, this was obtained from andor. I did write everything
// else in this class.

class AndorCamera;

struct cameraThreadInput
{
	bool spuriousWakeupHandler;
	std::mutex runMutex;
	std::condition_variable signaler;
	Communicator* comm;
	// Andor is set to this in the constructor of the andor camera.
	AndorCamera* Andor;
	std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>>* imageTimes;
};

/// the all-important camera class.
class AndorCamera
{
	public:
		AndorCamera::AndorCamera();

		/// Andor Wrappers, in alphabetical order. Versions that take no parameters just insert current settings into 
		// the versions that take parameters. Note that my wrapper names don't always match the andor SDK names. If 
		// looking for specific sdk functions, search in the cpp file.
		void abortAcquisition();
		void checkForNewImages();
		void getAcquisitionProgress( long& seriesNumber );
		void getAcquisitionProgress( long& accumulationNumber, long& seriesNumber );
		void getAcquisitionTimes(float& exposure, float& accumulation, float& kinetic);
		void getAdjustedRingExposureTimes(int size, float* timesArray);
		void getNumberOfPreAmpGains(int& number);
		void getOldestImage(std::vector<long>& dataArray);
		void getPreAmpGain(int index, float& gain);
		void queryStatus();
		void queryStatus(int& status);
		void getTemperatureRange(int& min, int& max);
		void getTemperature(int& temp);

		void setAccumulationCycleTime();
		void setAccumulationCycleTime(float time);
		void setAccumulationNumber(int number);
		void setAcquisitionMode();
		void setAcquisitionMode(int mode);
		void setADChannel(int channel); 
		void setEmCcdGain(int gain);
		void setEmGainSettingsAdvanced(int state);
		void setFrameTransferMode();
		void setFrameTransferMode(int mode);
		void setHSSpeed(int type, int index);
		void setImage(int hBin, int vBin, int lBorder, int rBorder, int tBorder, int bBorder);
		void setKineticCycleTime();
		void setKineticCycleTime(float cycleTime);
		void setNumberKinetics(int number);
		void setOutputAmplifier(int type);
		void setPreAmpGain(int index);
		void setReadMode();
		void setReadMode(int mode);
		void setRingExposureTimes(int sizeOfTimesArray, float* arrayOfTimes);
		void setTemperature(int temp);
		void setTriggerMode(int mode);
		void startAcquisition();

		void temperatureControlOn();
		void temperatureControlOff();

		void waitForAcquisition();

		void getCapabilities( AndorCapabilities& caps );
		void getSerialNumber( int& num );
		std::string getHeadModel();

		/// End Andor sdk wrappers.

		// all of the following do something more interesting.
		//AndorCamera::AndorCamera();
		AndorRunSettings getSettings();
		void pauseThread();
		void setSettings(AndorRunSettings settingsToSet);
		void armCamera(CameraWindow* camWin, double& minKineticCycleTime);
		std::vector<std::vector<long>> acquireImageData();
		void setTemperature();
		void setExposures();
		void setImageParametersToCamera();
		void setScanNumber();
		double getMinKineticCycleTime( );
		void checkAcquisitionTimings(float& kinetic, float& accumulation, std::vector<float>& exposures);
		void setNumberAccumulations(bool isKinetic);
		void setCameraTriggerMode();
		void onFinish();
		bool isRunning();
		void setIsRunningState( bool state );
		void updatePictureNumber( ULONGLONG newNumber );
		void setGainMode();
		void changeTemperatureSetting(bool temperatureControlOff);
		void andorErrorChecker(int errorCode);
		
		void initialize();
		void setBaselineClamp(int clamp);
		void setBaselineOffset(int offset);
		void setDMAParameters(int maxImagesPerDMA, float secondsPerDMA);

		static UINT __stdcall cameraThread( void* voidPtr );
		
		void initializeClass( Communicator* comm,
										   std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>>* imageTimes );
		std::string getSystemInfo();

	private:
		/// These are official settings and are the final say on what the camera does. Some unofficial 
		/// settings are stored in smaller classes.
		// If the experiment is running, these settings hold the options that the experiment is using.
		//AndorBaseSettings baseSettings;
		AndorRunSettings runSettings;
		// ??? 
		imageParameters readImageParameters;
		imageParameters runningImageParameters;
		// 
		bool cameraIsRunning;
		// set either of these to true in order to break corresponding threads out of their loops.
		bool plotThreadExitIndicator;
		bool cameraThreadExitIndicator = false;

		ULONGLONG currentPictureNumber;
		ULONGLONG currentRepetitionNumber;

		HANDLE plottingMutex;
		// ???
		HANDLE imagesMutex;
		//
		std::vector<std::vector<long> > imagesOfExperiment;
		std::vector<std::vector<long> > imageVecQueue;
		UINT cameraThreadID = 0;

		cameraThreadInput threadInput;

};
