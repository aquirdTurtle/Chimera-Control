#pragma once

#include <pylon/PylonIncludes.h>
#include <pylon/PylonGUI.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
#include <pylon/1394/Basler1394InstantCamera.h>

class BaslerGrabThreadWorker;
class IChimeraQtWindow;

class ImageEventHandler : public Pylon::CImageEventHandler{
	public:
		ImageEventHandler (IChimeraQtWindow* parentHandle);
		virtual void OnImageGrabbed (Pylon::CInstantCamera& camera, const Pylon::CGrabResultPtr& grabResult);
	private:
		IChimeraQtWindow* parent = nullptr;
		BaslerGrabThreadWorker* worker=nullptr;
};
