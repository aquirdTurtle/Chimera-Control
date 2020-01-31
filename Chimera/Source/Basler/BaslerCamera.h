// created by Mark O. Brown
#pragma once

#include "GeneralImaging/PictureControl.h"
#include <pylon/PylonIncludes.h>
#include <pylon/PylonGUI.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
#include <pylon/1394/Basler1394InstantCamera.h>
#include "BaslerSettingsControl.h"
#include "PrimaryWindows/MainWindow.h"
#include "LowLevel/constants.h"
#include <atomic>

class BaslerCameras;
class BaslerWrapper;

struct triggerThreadInput
{
	double frameRate;
	BaslerWrapper* camera;
	CWnd* parent;
	// only actually needed for debug mode.
	ULONG height;
	ULONG width;
	// only used in debug mode.
	std::atomic<bool>* runningFlag;
};

// wrapper class for modifying for safemode and to standardize error handling.
class BaslerWrapper : public cameraType
{
	
	using cameraType::cameraType;
	public:
		void init( CWnd* parent );
		
		int getMinOffsetX();
		int getCurrentOffsetX();
		void setOffsetX( int offset );

		int getMinOffsetY();
		int getCurrentOffsetY();
		void setOffsetY( int offset );

		int getMaxWidth();
		int getCurrentWidth();
		int getMaxHeight();
		int getCurrentHeight();
		
		double getExposureMax();
		double getExposureMin();
		double getCurrentExposure();
		void setExposure( double exposureTime );
		void setExposureAuto(cameraParams::ExposureAutoEnums mode);

		void setWidth( int width );
		void setHeight( int height );
		void setHorBin( int binning );
		void setVertBin( int binning );

		void stopGrabbing();
		bool isGrabbing();

		void waitForFrameTriggerReady(unsigned int timeout);
		void executeSoftwareTrigger();
		
		void setTriggerSource(cameraParams::TriggerSourceEnums mode);
		void startGrabbing( unsigned int picturesToGrab, Pylon::EGrabStrategy grabStrat );
		std::vector<long> retrieveResult( unsigned int timeout );

		void setPixelFormat( cameraParams::PixelFormatEnums pixelFormat );

		void setGainMode( std::string mode );
		void setGain(int gainValue);
		int getMinGain();
};

// the object for an actual camera.  doesn't handle gui things itself, just the interface from my code to the camera object.
class BaslerCameras
{
	public:
		// important constructor to initialize camera
		BaslerCameras( CWnd* parent );
		~BaslerCameras();
		bool isRunning ( );
		void setParameters( baslerSettings settings );
		void setDefaultParameters();
		void armCamera( triggerThreadInput* input);
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
		//bool continuousImaging;
		//bool autoTrigger;
		//unsigned int repCounts;
		bool cameraInitialized;
};


class ImageEventHandler : public Pylon::CImageEventHandler
{
	public:
		ImageEventHandler(CWnd* parentHandle) : Pylon::CImageEventHandler()
		{
			parent = parentHandle;
		}

		virtual void OnImageGrabbed( Pylon::CInstantCamera& camera, const Pylon::CGrabResultPtr& grabResult )
		{
			try
			{
				// Image grabbed successfully?
				if (grabResult->GrabSucceeded())
				{
					const uint16_t *pImageBuffer = (uint16_t *)grabResult->GetBuffer();
					int width = grabResult->GetWidth();
					int vertBinNumber = grabResult->GetHeight();
					Matrix<long>* imageMatrix; 
					imageMatrix = new Matrix<long>( vertBinNumber, width,
													std::vector<long> ( pImageBuffer, pImageBuffer + width * vertBinNumber ) );
					for (auto& elem : *imageMatrix)
					{
						elem *= 256.0 / 1024.0;
					}
					parent->PostMessageA( MainWindow::BaslerProgressMessageID, grabResult->GetWidth( ) * grabResult->GetHeight( ),
										  (LPARAM)imageMatrix );
					
				}
				else
				{
					thrower("" + str(grabResult->GetErrorCode()) + " " 
							 + std::string(grabResult->GetErrorDescription().c_str()));
				}
			}
			catch (Pylon::RuntimeException& )
			{
				throwNested("Error! Failed to handle image grabbing");
			}
		}
	private:
		CWnd* parent;
};	
