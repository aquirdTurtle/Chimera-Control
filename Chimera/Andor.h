// created by Mark O. Brown
#pragma once
#include <string>
#include "CameraImageDimensions.h"
#include "Communicator.h"
#include <process.h>
#include <mutex>
#include "ATMCD32D.h"
#include "AndorRunSettings.h"
#include "AndorFlume.h"
#include "Matrix.h"
#include "AndorTemperatureStatus.h"

/// /////////////////////////////////////////////////////
///			The Andor Class
/// /////////////////////////////////////////////////////

// This class is designed to facilitate interaction with the andor camera and
// is based around the andor SDK. I did not write the Andor SDK, this was obtained from andor. I did write everything
// else in this class.

// This is a "Core"-like class. there's no gui associated with this. 

class AndorCamera;

struct cameraThreadInput
{
	bool spuriousWakeupHandler;
	std::mutex runMutex;
	std::condition_variable signaler;
	Communicator* comm;
	// Andor is set to this in the constructor of the andor camera.
	AndorCamera* Andor;
	bool safemode;
	std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>>* imageTimes;
};

/// the important camera class.
class AndorCamera
{
	public:
		AndorCamera(bool safemode_opt);

		AndorRunSettings getAndorRunSettings();
		void pauseThread();
		void setSettings(AndorRunSettings settingsToSet);
		void armCamera( double& minKineticCycleTime );
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

		static UINT __stdcall cameraThread( void* voidPtr );		
		std::string getSystemInfo();
		void initializeClass( Communicator* comm, chronoTimes* imageTimes );
		void setCalibrating( bool cal );
		bool isCalibrating( );
		void abortAcquisition ( );
		int queryStatus (  );
		AndorTemperatureStatus getTemperature ( );
	private:

		void setAccumulationCycleTime ( );
		void setAcquisitionMode ( );
		void setFrameTransferMode ( );
		void setKineticCycleTime ( );
		void setReadMode ( );

		bool calInProgress = false;
		/// These are official settings and are the final say on what the camera does. Some unofficial 
		/// settings are stored in smaller classes.
		// If the experiment is running, these settings hold the options that the experiment is using.
		AndorRunSettings runSettings;

		AndorFlume flume;
		const bool safemode;
		// 
		bool cameraIsRunning;
		// set either of these to true in order to break corresponding threads out of their loops.
		bool plotThreadExitIndicator;
		bool cameraThreadExitIndicator = false;

		ULONGLONG currentPictureNumber;
		ULONGLONG currentRepetitionNumber;

		HANDLE plottingMutex;
		HANDLE imagesMutex;
		std::vector<Matrix<long>> experimentImageMatrices;
		std::vector<std::vector<long> > imagesOfExperiment;
		std::vector<std::vector<long> > imageVecQueue;
		UINT cameraThreadID = 0;

		cameraThreadInput threadInput;
};
