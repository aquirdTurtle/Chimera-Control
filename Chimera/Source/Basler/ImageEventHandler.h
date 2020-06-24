#pragma once

#include <pylon/PylonIncludes.h>
#include <pylon/PylonGUI.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
#include <pylon/1394/Basler1394InstantCamera.h>
//#include <PrimaryWindows/IChimeraWindowWidget.h>

class BaslerGrabThreadWorker;
class IChimeraWindowWidget;

class ImageEventHandler : public Pylon::CImageEventHandler
{
	public:
		ImageEventHandler (IChimeraWindowWidget* parentHandle);
		virtual void OnImageGrabbed (Pylon::CInstantCamera& camera, const Pylon::CGrabResultPtr& grabResult);
	private:
		IChimeraWindowWidget* parent;
		BaslerGrabThreadWorker* worker;
};
