// created by Mark O. Brown
#pragma once

#include "BaslerSettingsControl.h"
#include "BaslerWrapper.h"
#include "BaslerSettings.h"

#include "GeneralImaging/PictureControl.h"
#include "DataLogging/DataLogger.h"
#include "GeneralObjects/IDeviceCore.h"
#include <PrimaryWindows/IChimeraQtWindow.h>

#include <pylon/PylonIncludes.h>
#include <pylon/PylonGUI.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
#include <pylon/1394/Basler1394InstantCamera.h>

#include <atomic>

struct triggerThreadInput{
	double frameRate;
	BaslerWrapper* camera;
};

// the object for an actual camera.  doesn't handle gui things itself, just the interface from my code to the camera object.
class BaslerCameraCore : public IDeviceCore{
	public:
		// THIS CLASS IS NOT COPYABLE.
		BaslerCameraCore& operator=(const BaslerCameraCore&) = delete;
		BaslerCameraCore (const BaslerCameraCore&) = delete;

		// important constructor to initialize camera
		BaslerCameraCore( IChimeraQtWindow* parent );
		~BaslerCameraCore();
		void logSettings (DataLogger& logger, ExpThreadWorker* threadworker);
		baslerSettings getSettingsFromConfig (ConfigStream& configFile);
		bool isRunning ( );
		void setBaslserAcqParameters( baslerSettings settings );
		void setDefaultParameters();
		void armCamera( );
		void disarm();
		static void triggerThread(void* input);
		void softwareTrigger();
		QPoint getCameraDimensions();
		void reOpenCamera(IChimeraQtWindow* parent );
		std::string getCameraInfo();
		baslerSettings getDefaultSettings();
		double getCurrentExposure();
		unsigned int getPicsPerRep ();
		bool isContinuous();
		bool isInitialized();
		baslerSettings getRunningSettings ();
		std::string configDelim = "BASLER_CAMERA_SETTINGS";
		std::string getDelim () { return configDelim; }
		void loadExpSettings (ConfigStream& stream);
		void calculateVariations (std::vector<parameterType>& params, ExpThreadWorker* threadworker);
		void programVariation (unsigned variation, std::vector<parameterType>& params, ExpThreadWorker* threadworker) {};
		void errorFinish () {};
		void normalFinish () {};

		// Adjust value so it complies with range and increment passed.
		//
		// The parameter's minimum and maximum are always considered as valid values.
		// If the increment is larger than one, the returned value will be: min + (n * inc).
		// If the value doesn't meet these criteria, it will be rounded down so that it does.
		int64_t Adjust( int64_t val, int64_t minimum, int64_t maximum, int64_t inc );
	private:
		BaslerWrapper* camera;
		// official copy.
		baslerSettings runSettings;
		baslerSettings expRunSettings;
		HANDLE cameraTrigThread;
		bool cameraInitialized;
};

