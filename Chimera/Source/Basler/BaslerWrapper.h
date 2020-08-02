#pragma once

#include "PrimaryWindows/CustomMessages.h"

#include <Basler/BaslerGrabThreadWorker.h>

#include <pylon/PylonIncludes.h>
#include <pylon/PylonGUI.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
#include <pylon/1394/Basler1394InstantCamera.h>
#include <PrimaryWindows/IChimeraQtWindow.h>
#include <qthread.h>

// wrapper class for modifying for safemode and to standardize error handling.
class BaslerWrapper : public cameraType
{
	using cameraType::cameraType;
	public:
		// THIS CLASS IS NOT COPYABLE.
		BaslerWrapper& operator=(const BaslerWrapper&) = delete;
		BaslerWrapper (const BaslerWrapper&) = delete;

		void init (IChimeraQtWindow* parent );

		int getMinOffsetX ();
		int getCurrentOffsetX ();
		void setOffsetX (int offset);

		int getMinOffsetY ();
		int getCurrentOffsetY ();
		void setOffsetY (int offset);

		int getMaxWidth ();
		int getCurrentWidth ();
		int getMaxHeight ();
		int getCurrentHeight ();

		double getExposureMax ();
		double getExposureMin ();
		double getCurrentExposure ();
		void setExposure (double exposureTime);
		void setExposureAuto (cameraParams::ExposureAutoEnums mode);

		void setWidth (int width);
		void setHeight (int height);
		void setHorBin (int binning);
		void setVertBin (int binning);

		void stopGrabbing ();
		bool isGrabbing ();

		void waitForFrameTriggerReady (unsigned int timeout);
		void executeSoftwareTrigger ();

		void setTriggerSource (cameraParams::TriggerSourceEnums mode);
		void startGrabbing (unsigned int picturesToGrab, Pylon::EGrabStrategy grabStrat);
		std::vector<long> retrieveResult (unsigned int timeout);

		void setPixelFormat (cameraParams::PixelFormatEnums pixelFormat);

		void setGainMode (std::string mode);
		void setGain (int gainValue);
		int getMinGain ();
};