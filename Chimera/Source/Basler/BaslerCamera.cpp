// created by Mark O. Brown
#include "stdafx.h"
#include "BaslerCamera.h"
#include "BaslerWrapper.h"
#include "GeneralImaging/PictureControl.h"
#include "stdint.h"
#include "LowLevel/constants.h"
#include "PrimaryWindows/MainWindow.h"

#include <cmath>
#include <algorithm>
#include <functional>

// important constructor;
// Create an instant camera object with the camera device found first. At this point this class is really only meant to 
// work with a single camera of one type at a time. Not sure what would happen if you had multiple cameras set up at 
// once.
BaslerCameraCore::BaslerCameraCore(CWnd* parent)
{
	Pylon::PylonInitialize();
	Pylon::CDeviceInfo info;
	info.SetDeviceClass( cameraType::DeviceClass() );
	if (!BASLER_SAFEMODE)
	{
		try
		{
			cameraType* temp;
			temp = new BaslerWrapper(Pylon::CTlFactory::GetInstance().CreateFirstDevice(info));
			camera = dynamic_cast<BaslerWrapper*>(temp);
			camera->init( parent );
			cameraInitialized = true;
		}
		catch (Pylon::GenericException&)
		{
			cameraInitialized = false;
		}
	}
	else
	{
		camera->init( parent );
		cameraInitialized = true;
	}
}

bool BaslerCameraCore::isRunning ( )
{ 
	return camera->isGrabbing ( );
}

bool BaslerCameraCore::isInitialized()
{
	return cameraInitialized;
}

// send a software trigger to the camera after waiting to make sure it's ready to recieve said trigger.
void BaslerCameraCore::softwareTrigger()
{
	camera->waitForFrameTriggerReady( 5000 );
	camera->executeSoftwareTrigger();
}


// important deconstructor.
BaslerCameraCore::~BaslerCameraCore()
{
	Pylon::PylonTerminate();
	delete camera;
}


// get some information about the camera from the camera itself through pylon.
std::string BaslerCameraCore::getCameraInfo()
{
	// Get camera device information.
	return "Camera Device Information\n=========================\nVendor           : " + std::string( camera->DeviceVendorName.GetValue() )
			+ "\nModel            : " + std::string( camera->DeviceModelName.GetValue() ) + "\nFirmware version : "
			+ std::string( camera->DeviceFirmwareVersion.GetValue() ) ;
}


// Can change this for nicer defaults.
baslerSettings BaslerCameraCore::getDefaultSettings()
{
	baslerSettings defaultSettings;
	POINT dim = getCameraDimensions();
	defaultSettings.repCount = 100;
	defaultSettings.dims.left = 1;
	defaultSettings.dims.right = dim.x; 
	defaultSettings.dims.horizontalBinning = 4;
	defaultSettings.dims.top = dim.y;
	defaultSettings.dims.bottom = 1;
	defaultSettings.dims.verticalBinning = 4;
	defaultSettings.exposureMode = BaslerAutoExposure::mode::Off;
	defaultSettings.exposureTime = 1000;
	defaultSettings.frameRate = 0.25;
	defaultSettings.rawGain = camera->getMinGain();
	return defaultSettings;
}

// set the default parameters defined in the function above.
void BaslerCameraCore::setDefaultParameters()
{
	setParameters( getDefaultSettings() );
}

// general function you should use for setting camera settings based on the input.
void BaslerCameraCore::setParameters( baslerSettings settings )
{
	/// Set the AOI:

	/* 
		there is never a problem making things smaller. the danger is over-reaching the maximum whose bounds change dynamically.
		the basler API is very picky about the order of these things. Ways for this to crash:
		- Setting offset large (before making width smaller) pushes the right border past max width
		- setting width large (before making offset smaller) pushes right border past max width
		- setting the binning large (before changing the width)
	*/

	// start from a safe place.
	camera->setOffsetX(0);
	camera->setWidth(16);
	camera->setHorBin(1);
	camera->setOffsetY(0);
	camera->setHeight(16);
	camera->setVertBin(1);

	// suppose you start real small. 16 1x1 pixels.  then, this always works:
	// set x offset (no chance of pushing right because width is small)
	// set width (no chance of pushing right off because of binning because binning is minimal so potential value for rightmost point is 
	//		maximal.
	// set binning 
	camera->setOffsetX(settings.dims.left);
	camera->setWidth(settings.dims.horRawPixelNum());
	camera->setHorBin(settings.dims.horizontalBinning);
	camera->setOffsetY(settings.dims.bottom);
	camera->setHeight(settings.dims.vertRawPixelNum());
	camera->setVertBin(settings.dims.verticalBinning);
	
	/// set other parameters
	#ifdef USB_CAMERA
		camera->setPixelFormat( cameraParams::PixelFormat_Mono10 );
	#elif defined FIREWIRE_CAMERA
		camera->setPixelFormat( cameraParams::PixelFormat_Mono16 );
	#endif
	
	camera->setGainMode("Off");
	camera->setGain( camera->getMinGain() );
	//amera->AcquisitionFrameRateEnable.SetValue(true);
	//Basler_UsbCameraParams::AcquisitionModeEnums::AcquisitionMode_Continuous
	//camera->AcquisitionMode.SetValue(Basler_UsbCameraParams::AcquisitionModeEnums::AcquisitionMode_Continuous);
	//camera->AcquisitionFrameRate.SetValue(settings.frameRate);
	// exposure mode
	if (settings.exposureMode == BaslerAutoExposure::mode::Continuous)
	{
		camera->setExposureAuto( cameraParams::ExposureAuto_Continuous );
	}
	else if (settings.exposureMode == BaslerAutoExposure::mode::Off)
	{
		camera->setExposureAuto( cameraParams::ExposureAuto_Off );

		if (!(settings.exposureTime >= camera->getExposureMin() && settings.exposureTime <= camera->getExposureMax()))
		{
			thrower ( "exposure time must be between " + str( camera->getExposureMin() ) + " and " 
					 + str( camera->getExposureMax()) );
		}
		camera->setExposure( settings.exposureTime );
	}
	else if (settings.exposureMode == BaslerAutoExposure::mode::Once)
	{
		camera->setExposureAuto( cameraParams::ExposureAuto_Once );
	}

	if (settings.acquisitionMode != BaslerAcquisition::mode::Finite)
	{
		settings.repCount = SIZE_MAX;
	}

	if (settings.triggerMode == BaslerTrigger::mode::External)
	{
		#ifdef FIREWIRE_CAMERA
			camera->setTriggerSource(cameraParams::TriggerSource_Line1);
		#elif defined USB_CAMERA
			camera->setTriggerSource(cameraParams::TriggerSource_Line3);
		#endif
	}
	else if (settings.triggerMode == BaslerTrigger::mode::AutomaticSoftware )
	{
		camera->setTriggerSource( cameraParams::TriggerSource_Software );
	}
	else if (settings.triggerMode == BaslerTrigger::mode::ManualSoftware )
	{
		camera->setTriggerSource( cameraParams::TriggerSource_Software );
	}
	runSettings = settings;
}


// I can potentially use this to reopen the camera if e.g. the user disconnects. Don't think this is really implemented
// yet.
void BaslerCameraCore::reOpenCamera(CWnd* parent)
{
	Pylon::CDeviceInfo info;
	info.SetDeviceClass( cameraType::DeviceClass() );
	if (!BASLER_SAFEMODE)
	{
		delete camera;
		cameraType* temp;
		temp = new cameraType( Pylon::CTlFactory::GetInstance().CreateFirstDevice( info ) );
		camera = dynamic_cast<BaslerWrapper*>(temp);
	}
	camera->init(parent);
}

// get the dimensions of the camera. This is tricky because while I can get info about each parameter easily through
// pylon, several of the parameters, such as the width, are context-sensitive and return the max values as possible 
// given other camera settings at the moment (e.g. the binning).
POINT BaslerCameraCore::getCameraDimensions()
{
	if (BASLER_SAFEMODE)
	{
		return { 672, 512 };
	}
	POINT dim;
	// record the original offsets and bins.
	POINT offsets, bins;
	offsets.x = camera->OffsetX.GetValue();
	offsets.y = camera->OffsetY.GetValue();
	bins.x = camera->BinningHorizontal.GetValue();
	bins.y = camera->BinningVertical.GetValue();
	
	// TODO:
	// change to my apif
	camera->BinningHorizontal.SetValue( camera->BinningHorizontal.GetMin() );
	camera->BinningVertical.SetValue( camera->BinningVertical.GetMin() );
	camera->OffsetX.SetValue( camera->OffsetX.GetMin() );
	camera->OffsetY.SetValue( camera->OffsetY.GetMin() );

	dim.x = camera->Width.GetMax() - 1;
	dim.y = camera->Height.GetMax() - 1;

	camera->OffsetX.SetValue( offsets.x );
	camera->OffsetY.SetValue( offsets.y );
	camera->BinningHorizontal.SetValue( bins.x );
	camera->BinningVertical.SetValue( bins.y );

	return dim;
}


// get the camera "armed" (ready for aquisition). Actual start of camera taking pictures depends on things like the 
// trigger mode.
void BaslerCameraCore::armCamera( triggerThreadInput* input )
{
	Pylon::EGrabStrategy grabStrat;
	if (runSettings.acquisitionMode == BaslerAcquisition::mode::Continuous )
	{
		grabStrat = Pylon::GrabStrategy_LatestImageOnly;
	}
	else
	{
		grabStrat = Pylon::GrabStrategy_OneByOne;
	}
	input->camera = camera;
	camera->startGrabbing( runSettings.repCount, grabStrat );
	if ( runSettings.triggerMode == BaslerTrigger::mode::AutomaticSoftware )
	{
		cameraTrigThread = (HANDLE)_beginthread( triggerThread, NULL, input );
	}
}


HANDLE BaslerCameraCore::getCameraThreadObj ( )
{
	return cameraTrigThread;
}


// 
double BaslerCameraCore::getCurrentExposure()
{
	return camera->getCurrentExposure();
}

//
unsigned int BaslerCameraCore::getRepCounts()
{
	return runSettings.repCount;
}

//
bool BaslerCameraCore::isContinuous()
{
	return runSettings.acquisitionMode == BaslerAcquisition::mode::Continuous;
}


// this is the thread that programatically software-triggers the camera when triggering internally.
void BaslerCameraCore::triggerThread( void* voidInput )
{
	int count = 0;
	triggerThreadInput* input = (triggerThreadInput*)voidInput;
	try
	{
		while (input->camera->isGrabbing())
		{
			// adjust for frame rate
			
			Sleep(int(1000.0 / input->frameRate));
			try
			{
				// Execute the software trigger. The call waits up to 100 ms for the camera
				// to be ready to be triggered.
				input->camera->waitForFrameTriggerReady(1000);
				input->camera->executeSoftwareTrigger();			
				count++;
			}
			catch (Error&)
			{
				// continue... should be stopping grabbing.
				break;
			}

			if (BASLER_SAFEMODE)
 			{ 
				if (!*input->runningFlag)
				{
					// aborting.
					return;
				}
 				// simulate successful grab 
				// need some way to communicate the width and height of the pic to this function... 
				Matrix<long>* imageMatrix = new Matrix<long>(input->height, input->width ); 
				UINT count = 0; 
				UINT rowNum = 0; 
				UINT colNum = 0; 
				for ( auto row : range( imageMatrix->getRows( ) ) )
				{
					for ( auto col : range( imageMatrix->getCols( ) ) )
					{
						(*imageMatrix)(row, col) = 300 + rand( ) % 10
							+ 300 * exp( -std::pow( (double( row ) - 100) / 10.0, 2 ) - std::pow( (double( col ) - 100) / 10.0, 2 ) )
							+ 300 * exp( -std::pow( (double( row ) - 200) / 10.0, 2 ) - std::pow( (double( col ) - 100) / 10.0, 2 ) );
					}
				}
				PostMessage(*input->parent, CustomMessages::BaslerProgressMessageID, 672 * 512, (LPARAM)imageMatrix);
			}
		}
	}
	catch (Pylon::TimeoutException& timeoutErr)
	{
		errBox( "Trigger Thread Timeout Error!" + std::string(timeoutErr.what()) );
	}
	catch (Pylon::RuntimeException&)
	{
		// aborted, that's fine. return.
	}
}


// stop the camera from taking any pictures, even if triggered afterwards.
void BaslerCameraCore::disarm()
{
	camera->stopGrabbing();
}

/*
 * Adjust value so it complies with range and increment passed.
 *
 * The parameter's minimum and maximum are always considered as valid values.
 * If the increment is larger than one, the returned value will be: min + (n * inc).
 * If the value doesn't meet these criteria, it will be rounded down so that it does.

 * this function comes from basler example code. Not sure if I'm using it right now.
 */
int64_t BaslerCameraCore::Adjust( int64_t val, int64_t minimum, int64_t maximum, int64_t inc )
{
	// Check the input parameters.
	if (inc <= 0)
	{
		// Negative increments are invalid.
		throw LOGICAL_ERROR_EXCEPTION( "Unexpected increment %d", inc );
	}
	if (minimum > maximum)
	{
		// Minimum must not be bigger than or equal to the maximum.
		throw LOGICAL_ERROR_EXCEPTION( "minimum bigger than maximum." );
	}

	// Check the lower bound.
	if (val < minimum)
	{
		return minimum;
	}

	// Check the upper bound.
	if (val > maximum)
	{
		return maximum;
	}

	// Check the increment.
	if (inc == 1)
	{
		// Special case: all values are valid.
		return val;
	}
	else
	{
		// The value must be min + (n * inc).
		// Due to the integer division, the value will be rounded down.
		return minimum + (((val - minimum) / inc) * inc);
	}
}


