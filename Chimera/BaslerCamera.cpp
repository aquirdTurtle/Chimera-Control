#include "stdafx.h"
#include "BaslerCamera.h"
#include "PictureControl.h"
#include "stdint.h"
#include "constants.h"
#include <cmath>
#include <algorithm>
#include <functional>
#include "MainWindow.h"

// important constructor;
// Create an instant camera object with the camera device found first. At this point this class is really only meant to 
// work with a single camera of one type at a time. Not sure what would happen if you had multiple cameras set up at 
// once.
BaslerCameras::BaslerCameras(CWnd* parent)
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

bool BaslerCameras::isRunning ( )
{ 
	return camera->isGrabbing ( );
}

bool BaslerCameras::isInitialized()
{
	return cameraInitialized;
}

// send a software trigger to the camera after waiting to make sure it's ready to recieve said trigger.
void BaslerCameras::softwareTrigger()
{
	camera->waitForFrameTriggerReady( 5000 );
	camera->executeSoftwareTrigger();
}


// important deconstructor.
BaslerCameras::~BaslerCameras()
{
	Pylon::PylonTerminate();
	delete camera;
}


// get some information about the camera from the camera itself through pylon.
std::string BaslerCameras::getCameraInfo()
{
	// Get camera device information.
	return "Camera Device Information\n=========================\nVendor           : " + std::string( camera->DeviceVendorName.GetValue() )
			+ "\nModel            : " + std::string( camera->DeviceModelName.GetValue() ) + "\nFirmware version : "
			+ std::string( camera->DeviceFirmwareVersion.GetValue() ) ;
}


// Can change this for nicer defaults.
baslerSettings BaslerCameras::getDefaultSettings()
{
	baslerSettings defaultSettings;
	POINT dim = getCameraDimensions();
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
void BaslerCameras::setDefaultParameters()
{
	setParameters( getDefaultSettings() );
}

// general function you should use for setting camera settings based on the input.
void BaslerCameras::setParameters( baslerSettings settings )
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
void BaslerCameras::reOpenCamera(CWnd* parent)
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
POINT BaslerCameras::getCameraDimensions()
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
void BaslerCameras::armCamera( triggerThreadInput* input )
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


HANDLE BaslerCameras::getCameraThreadObj ( )
{
	return cameraTrigThread;
}


// 
double BaslerCameras::getCurrentExposure()
{
	return camera->getCurrentExposure();
}

//
unsigned int BaslerCameras::getRepCounts()
{
	return runSettings.repCount;
}

//
bool BaslerCameras::isContinuous()
{
	return runSettings.acquisitionMode == BaslerAcquisition::mode::Continuous;
}


// this is the thread that programatically software-triggers the camera when triggering internally.
void BaslerCameras::triggerThread( void* voidInput )
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
				PostMessage(*input->parent, MainWindow::BaslerProgressMessageID, 672 * 512, (LPARAM)imageMatrix);
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
void BaslerCameras::disarm()
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
int64_t BaslerCameras::Adjust( int64_t val, int64_t minimum, int64_t maximum, int64_t inc )
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


// initialize the camera using the fundamental settings I use for all cameras. 
void BaslerWrapper::init( CWnd* parent )
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			Open();
			// prepare the image event handler
			RegisterImageEventHandler( new ImageEventHandler( parent ), Pylon::RegistrationMode_ReplaceAll, 
									   Pylon::Cleanup_Delete );
			TriggerMode.SetValue( cameraParams::TriggerMode_On );
		}
		catch (Pylon::GenericException& err)
		{
			throwNested( "Failed to register event handler and set trigger mode." );
		}
	}
}


/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
///					Pylon Wrappers
/// 
/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * The rest of the functions in this file are simple wrappers for pylon functinos. They serve several purposes. 
 *		- they allow easy handling of errors.
		- More specifically, they allow me to standardize error handling. All my API wrappers handle errors by throwing
			an "Error" Object of my own making.
			- Pylon's error handling is actually already pretty close to my own error handling, in that it throws
				a specialized exception. However, I oftentimes here just make it return /my/ specialized exception 
				instead so that I can catch all errors simultaneously in my larger programs.
		- They allow easy bypass of the function call if the program is being run in safemode, oftentimes returning 
			dummy values that should allow the code to continue sensibly.
		- For the basler code specifically, they allow me to taylor the call to the situations when I'm using a 
			firewire camera or a usb camera without interupting the logical flow above. 
 */


int BaslerWrapper::getMinOffsetX()
{
	if (BASLER_SAFEMODE)
	{
		return 0;
	}
	try
	{
		return OffsetX.GetMin();
	}
	catch (Pylon::GenericException& err)
	{
		throwNested( "Failed to get min x offset." );
	}
}

int BaslerWrapper::getMinOffsetY()
{
	if (BASLER_SAFEMODE)
	{
		return 0;
	}
	try
	{
		return OffsetY.GetMin();
	}
	catch (Pylon::GenericException&)
	{
		throwNested( "Failed to get y min offset" );
	}
}

int BaslerWrapper::getMaxWidth()
{
	if (BASLER_SAFEMODE)
	{
		return 672;
	}
	try
	{
		return OffsetX.GetMax();
	}
	catch (Pylon::GenericException& err)
	{
		throwNested( "failed to Get maximum width" );
	}
}

int BaslerWrapper::getMaxHeight()
{
	if (BASLER_SAFEMODE)
	{
		return 512;
	}
	try
	{
		return OffsetY.GetMin();
	}
	catch (Pylon::GenericException& err)
	{
		throwNested( "Failed to get maximum height" );
	}
}

int BaslerWrapper::getMinGain()
{
	if (BASLER_SAFEMODE)
	{
		return 260;
	}
	try
	{
		#ifdef USB_CAMERA
			return Gain.GetMin();
		#elif defined FIREWIRE_CAMERA
			return GainRaw.GetMin();
		#endif
	}
	catch (Pylon::GenericException& err)
	{
		throwNested( "Failed to get Minimum gain" );
	}
}


void BaslerWrapper::setOffsetX( int offset )
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			OffsetX.SetValue( offset );
		}
		catch (Pylon::GenericException& err)
		{
			throwNested( "Failed to set x offset" );
		}

	}
}

void BaslerWrapper::setOffsetY( int offset )
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			OffsetY.SetValue(offset);
		}
		catch (Pylon::GenericException& err)
		{
			throwNested("Failed to Set Y Offset");
		}
	}
}

void BaslerWrapper::setWidth( int width )
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			Width.SetValue( width );
		}
		catch (Pylon::GenericException& err)
		{
			throwNested( "Failed to set width" );
		}
	}
}


void BaslerWrapper::setHeight( int height )
{
	if (!BASLER_SAFEMODE)
	{
		try
		{	
			Height.SetValue( height );
		}
		catch (Pylon::GenericException& err)
		{
			throwNested( "Failed to set height" );
		}
	}
}


void BaslerWrapper::setHorBin( int binning )
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			BinningHorizontal.SetValue( binning );
		}
		catch (Pylon::GenericException& err)
		{
			throwNested( "Failed to set horizontal binning " );
		}
	}
}


void BaslerWrapper::setVertBin( int binning )
{
	if (!BASLER_SAFEMODE)
	{
		try
		{			
			BinningVertical.SetValue( binning );
		}
		catch (Pylon::GenericException& err)
		{
			throwNested( "Failed to set vertical binning." );
		}
	}
}

void BaslerWrapper::stopGrabbing()
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			StopGrabbing();
		}
		catch (Pylon::GenericException& err)
		{
			throwNested( "Failed to stop grabbing." );
		}
	}
}

bool BaslerWrapper::isGrabbing()
{
	if (BASLER_SAFEMODE)
	{
		return true;
	}
	try
	{
		return IsGrabbing();
	}
	catch (Pylon::GenericException& err)
	{
		throwNested( "Failed to query if grabbing." );
	}
}

// not being used???
std::vector<long> BaslerWrapper::retrieveResult( unsigned int timeout )
{
	Pylon::CGrabResultPtr resultPtr;
	RetrieveResult( timeout, resultPtr, Pylon::TimeoutHandling_ThrowException );
	if (!resultPtr->GrabSucceeded())
	{
		thrower ( "" + str( resultPtr->GetErrorCode() ) + " " + std::string( resultPtr->GetErrorDescription().c_str() ) );
	}
	const uint16_t *pImageBuffer = (uint16_t *)resultPtr->GetBuffer();
	int width = resultPtr->GetWidth();
	int height = resultPtr->GetHeight();
	std::vector<long> image( pImageBuffer, pImageBuffer + width * height );
	return image;
}


int BaslerWrapper::getCurrentHeight()
{
	if (BASLER_SAFEMODE)
	{
		return 512;
	}
	try
	{
		return Height.GetValue();
	}
	catch (Pylon::GenericException& err)
	{
		throwNested( "Failed to get current height" );
	}
}


int BaslerWrapper::getCurrentWidth()
{
	if (BASLER_SAFEMODE)
	{
		return 672;
	}
	try
	{
		return Width.GetValue();
	}
	catch (Pylon::GenericException& err)
	{
		throwNested( "Failed to get current width" );
	}
}


int BaslerWrapper::getCurrentOffsetX()
{
	if (BASLER_SAFEMODE)
	{
		return 0;
	}
	try
	{
		return OffsetX.GetValue();
	}
	catch (Pylon::GenericException& err)
	{
		throwNested( "Failed to get current x offset" );
	}
}


int BaslerWrapper::getCurrentOffsetY()
{
	if (BASLER_SAFEMODE)
	{
		return 0;
	}
	try
	{
		return OffsetY.GetValue();
	}
	catch (Pylon::GenericException& err)
	{
		throwNested( "Failed to get current y offset." );
	}
}


void BaslerWrapper::setPixelFormat( cameraParams::PixelFormatEnums pixelFormat)
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			PixelFormat.SetValue( pixelFormat );
		}
		catch (Pylon::GenericException& err)
		{
			throwNested( "Failed to set pixel format." );
		}
	}
}

void BaslerWrapper::setGainMode( std::string mode )
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			GainAuto.FromString( mode.c_str() );
		}
		catch (Pylon::GenericException& err)
		{
			throwNested( "Failed to set gain mode." );
		}
	}
}

void BaslerWrapper::setGain( int gainValue )
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			#ifdef USB_CAMERA
				Gain.SetValue( gainValue );
			#elif defined FIREWIRE_CAMERA
				GainRaw.SetValue( gainValue );
			#endif
		}
		catch (Pylon::GenericException& err)
		{
			throwNested( "Failed to set gain." );
		}
	}
}


void BaslerWrapper::waitForFrameTriggerReady( unsigned int timeout )
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			WaitForFrameTriggerReady( timeout, Pylon::TimeoutHandling_ThrowException );
		}
		catch (Pylon::GenericException& err)
		{
			throwNested( "Failed to wait for frame trigger to be ready." );
		}
	}
}



void BaslerWrapper::executeSoftwareTrigger()
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			ExecuteSoftwareTrigger();
		}
		catch (Pylon::GenericException& err)
		{
			throwNested( "Failed to execute software trigger." );
		}
	}
}


void BaslerWrapper::setTriggerSource(cameraParams::TriggerSourceEnums mode)
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			TriggerSource.SetValue(mode);
		}
		catch (Pylon::GenericException& err)
		{
			throwNested("Failed to set trigger source.");
		}
	}
}

void BaslerWrapper::startGrabbing( unsigned int picturesToGrab, Pylon::EGrabStrategy grabStrat )
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			StartGrabbing( picturesToGrab, grabStrat, Pylon::GrabLoop_ProvidedByInstantCamera );
		}
		catch (Pylon::GenericException& err)
		{
			throwNested( "Failed to start grabbing." );
		}
	}
}

// returns in us
double BaslerWrapper::getExposureMax()
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			#ifdef USB_CAMERA
				return ExposureTime.GetMax();
			#elif defined FIREWIRE_CAMERA
				return ExposureTimeRaw.GetMax();
			#endif
		}
		catch (Pylon::GenericException& err)
		{
			throwNested( "Failed to get max exposure." );
		}
	}
	return 1000000;
}

double BaslerWrapper::getExposureMin()
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			#ifdef USB_CAMERA
				return ExposureTime.GetMin();
			#elif defined FIREWIRE_CAMERA
				return ExposureTimeRaw.GetMin();
			#endif
		}
		catch (Pylon::GenericException& err)
		{
			throwNested( "Failed to get min exposure." );
		}
	}
	return 0;
}

double BaslerWrapper::getCurrentExposure()
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			#ifdef USB_CAMERA
				return ExposureTime.GetValue();
			#elif defined FIREWIRE_CAMERA
				return ExposureTimeRaw.GetValue();
			#endif
		}
		catch (Pylon::GenericException& err)
		{
			throwNested( "Failed to get current exposure." );
		}
	}
	return 10000;
}

void BaslerWrapper::setExposure( double exposureTime )
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			#ifdef USB_CAMERA
				ExposureTime.SetValue( exposureTime );
			#elif defined FIREWIRE_CAMERA
				ExposureTimeRaw.SetValue( exposureTime );
			#endif
		}
		catch (Pylon::GenericException& err)
		{
			throwNested( "Failed to set exposure." );
		}
	}
}


void BaslerWrapper::setExposureAuto(cameraParams::ExposureAutoEnums mode)
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			ExposureAuto.SetValue(mode);
		}
		catch (Pylon::GenericException& err)
		{
			throwNested("Failed to set auto exposure mode.");
		}
	}
}