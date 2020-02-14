// created by Mark O. Brown
#pragma once

#include "GeneralImaging/PictureControl.h"
#include <pylon/PylonIncludes.h>
#include <pylon/PylonGUI.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
#include <pylon/1394/Basler1394InstantCamera.h>
#include "BaslerSettingsControl.h"
//#include "PrimaryWindows/MainWindow.h"
#include "BaslerWrapper.h"
#include "LowLevel/constants.h"
#include <atomic>

struct triggerThreadInput
{
	double frameRate;
	BaslerWrapper* camera;
	//CWnd* parent;
	// only actually needed for debug mode.
	//ULONG height;
	//ULONG width;
	// only used in debug mode.
	//std::atomic<bool>* runningFlag;
};

// the object for an actual camera.  doesn't handle gui things itself, just the interface from my code to the camera object.
class BaslerCameraCore
{
	public:
		// important constructor to initialize camera
		BaslerCameraCore( CWnd* parent );
		~BaslerCameraCore();
		bool isRunning ( );
		void setBaslserAcqParameters( baslerSettings settings );
		void setDefaultParameters();
		void armCamera( double frameRate );
		void disarm();
		static void triggerThread(void* input);
		void softwareTrigger();
		POINT getCameraDimensions();
		void reOpenCamera( CWnd* parent );
		std::string getCameraInfo();
		baslerSettings getDefaultSettings();
		double getCurrentExposure();
		unsigned int getRepCounts();
		bool isContinuous();
		bool isInitialized();
		HANDLE getCameraThreadObj ( );
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
		HANDLE cameraTrigThread;
		bool cameraInitialized;
};

