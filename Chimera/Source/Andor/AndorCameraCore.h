// created by Mark O. Brown
#pragma once
#include "CameraImageDimensions.h"

#include "AndorRunSettings.h"
#include "AndorFlume.h"
#include "GeneralObjects/Matrix.h"
#include "GeneralObjects/IDeviceCore.h"
#include "AndorTemperatureStatus.h"
#include <RealTimeDataAnalysis/analysisSettings.h>
#include <Andor/cameraThreadInput.h>
#include "ATMCD32D.h"
#include <string>
#include <process.h>
#include <mutex>
#include <condition_variable>
/// /////////////////////////////////////////////////////
///			The Andor Class
/// /////////////////////////////////////////////////////

// This class is designed to facilitate interaction with the andor camera and
// is based around the andor SDK. I did not writebtn the Andor SDK, this was obtained from andor. I did writebtn everything
// else in this class.

class AndorCameraCore;

/// the important camera class.
class AndorCameraCore : public IDeviceCore{
	public:
		// THIS CLASS IS NOT COPYABLE.
		AndorCameraCore& operator=(const AndorCameraCore&) = delete;
		AndorCameraCore (const AndorCameraCore&) = delete;
		AndorCameraCore(bool safemode_opt);
		AndorRunSettings getSettingsFromConfig (ConfigStream& configFile);
		AndorRunSettings getAndorRunSettings();
		void pauseThread();
		void setSettings(AndorRunSettings settingsToSet);
		void armCamera( double& minKineticCycleTime, ExpThreadWorker* threadworker=nullptr);
		std::vector<Matrix<long>> acquireImageData();
		void preparationChecks ();
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
		void updatePictureNumber( unsigned __int64 newNumber );
		void setGainMode(ExpThreadWorker* threadworker);
		void changeTemperatureSetting(bool temperatureControlOff);

		//static unsigned __stdcall cameraThread( void* voidPtr );		
		std::string getSystemInfo();
		void initializeClass(IChimeraQtWindow* parent, chronoTimes* imageTimes );
		void setCalibrating( bool cal );
		bool isCalibrating( );
		void abortAcquisition ( );
		int queryStatus (  );
		AndorTemperatureStatus getTemperature ( );
		std::string configDelim = "CAMERA_SETTINGS";
		std::string getDelim () { return configDelim; }
		void logSettings (DataLogger& log, ExpThreadWorker* threadworker);
		void loadExpSettings (ConfigStream& stream);
		void calculateVariations (std::vector<parameterType>& params, ExpThreadWorker* threadworker);
		void normalFinish ();
		void errorFinish ();
		void programVariation (unsigned variationInc, std::vector<parameterType>& params, ExpThreadWorker* threadworker);
		std::vector<std::string> getVertShiftSpeeds ();
		std::vector<std::string> getHorShiftSpeeds ();
	private:
		
		void setAccumulationCycleTime ( );
		void setAcquisitionMode ( );
		void setFrameTransferMode ( );
		void setKineticCycleTime ( );
		void setReadMode ( );
		int mostRecentTemp=20;

		bool calInProgress = false;
		/// These are official settings and are the final say on what the camera does. Some unofficial 
		/// settings are stored in smaller classes.
		// If the experiment is running, these settings hold the options that the experiment is using.
		AndorRunSettings runSettings;
		AndorRunSettings expRunSettings;
		analysisSettings expAnalysisSettings;

		AndorFlume flume;
		const bool safemode;
		// 
		bool cameraIsRunning=false;
		// set either of these to true in order to break corresponding threads out of their loops.
		bool plotThreadExitIndicator=false;
		bool cameraThreadExitIndicator = false;
		bool dataSetShouldBeValid = false;
		unsigned __int64 currentPictureNumber=0;
		unsigned __int64 currentRepetitionNumber=0;
		std::timed_mutex camThreadMutex;
		HANDLE plottingMutex;
		std::vector<Matrix<long> > repImages;
		unsigned cameraThreadID = 0;

		cameraThreadInput threadInput;

		friend class AndorCameraThreadWorker;
};
