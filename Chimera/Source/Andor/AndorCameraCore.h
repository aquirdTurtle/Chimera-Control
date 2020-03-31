// created by Mark O. Brown
#pragma once
#include "CameraImageDimensions.h"
#include "ExperimentThread/Communicator.h"
#include "ATMCD32D.h"
#include "AndorRunSettings.h"
#include "AndorFlume.h"
#include "GeneralObjects/Matrix.h"
#include "GeneralObjects/IDeviceCore.h"
#include "AndorTemperatureStatus.h"
#include <string>
#include <process.h>
#include <mutex>
#include <condition_variable>

/// /////////////////////////////////////////////////////
///			The Andor Class
/// /////////////////////////////////////////////////////

// This class is designed to facilitate interaction with the andor camera and
// is based around the andor SDK. I did not write the Andor SDK, this was obtained from andor. I did write everything
// else in this class.

// This is a "Core"-like class. there's no gui associated with this. 

class AndorCameraCore;

struct cameraThreadInput
{
	bool expectingAcquisition;
	std::timed_mutex* runMutex;
	std::condition_variable_any signaler;
	Communicator* comm;
	// Andor is set to this in the constructor of the andor camera.
	AndorCameraCore* Andor;
	bool safemode;
	std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>>* imageTimes;
};

/// the important camera class.
class AndorCameraCore : public IDeviceCore
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		AndorCameraCore& operator=(const AndorCameraCore&) = delete;
		AndorCameraCore (const AndorCameraCore&) = delete;
		AndorCameraCore(bool safemode_opt);
		AndorRunSettings getSettingsFromConfig (ConfigStream& configFile);
		AndorRunSettings getAndorRunSettings();
		void pauseThread();
		void setSettings(AndorRunSettings settingsToSet);
		void armCamera( double& minKineticCycleTime );
		std::vector<Matrix<long>> acquireImageData(Communicator* comm);
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
		std::string configDelim = "CAMERA_SETTINGS";
		std::string getDelim () { return configDelim; }
		void logSettings (DataLogger& log);
		void loadExpSettings (ConfigStream& stream);
		void calculateVariations (std::vector<parameterType>& params, Communicator& comm);
		void normalFinish ();
		void errorFinish ();
		void programVariation (UINT variationInc, std::vector<parameterType>& params);

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
		AndorRunSettings expRunSettings;

		AndorFlume flume;
		const bool safemode;
		// 
		bool cameraIsRunning;
		// set either of these to true in order to break corresponding threads out of their loops.
		bool plotThreadExitIndicator;
		bool cameraThreadExitIndicator = false;

		ULONGLONG currentPictureNumber;
		ULONGLONG currentRepetitionNumber;
		std::timed_mutex camThreadMutex;
		HANDLE plottingMutex;
		std::vector<Matrix<long> > repImages;
		UINT cameraThreadID = 0;

		cameraThreadInput threadInput;
};
