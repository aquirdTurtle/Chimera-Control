#include "stdafx.h"
#include "BaslerWrapper.h"
#include <PrimaryWindows/IChimeraQtWindow.h>
#include <PrimaryWindows/QtBaslerWindow.h>
#include <Basler/ImageEventHandler.h>

// initialize the camera using the fundamental settings I use for all cameras. 
void BaslerWrapper::init (IChimeraQtWindow* parent ){
	if (!BASLER_SAFEMODE){
		try	{
			Open ();
			// prepare the image event handler
			RegisterImageEventHandler ( new ImageEventHandler ( parent ), Pylon::RegistrationMode_ReplaceAll,
										Pylon::Cleanup_Delete );

			TriggerMode.SetValue (cameraParams::TriggerMode_On);
		}
		catch (Pylon::GenericException&){
			throwNested ("Failed to register event handler and set trigger mode.");
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
			an "ChimeraError" Object of my own making.
			- Pylon's error handling is actually already pretty close to my own error handling, in that it throws
				a specialized exception. However, I oftentimes here just make it return /my/ specialized exception
				instead so that I can catch all errors simultaneously in my larger programs.
		- They allow easy bypass of the function call if the program is being run in safemode, oftentimes returning
			dummy values that should allow the code to continue sensibly.
		- For the basler code specifically, they allow me to taylor the call to the situations when I'm using a
			firewire camera or a usb camera without interupting the logical flow above.
 */


int BaslerWrapper::getMinOffsetX (){
	if (BASLER_SAFEMODE){
		return 0;
	}
	try	{
		return OffsetX.GetMin ();
	}
	catch (Pylon::GenericException&){
		throwNested ("Failed to get min x offset.");
	}
}

int BaslerWrapper::getMinOffsetY (){
	if (BASLER_SAFEMODE){
		return 0;
	}
	try	{
		return OffsetY.GetMin ();
	}
	catch (Pylon::GenericException&){
		throwNested ("Failed to get y min offset");
	}
}

int BaslerWrapper::getMaxWidth (){
	if (BASLER_SAFEMODE){
		return 672;
	}
	try	{
		return OffsetX.GetMax ();
	}
	catch (Pylon::GenericException&){
		throwNested ("failed to Get maximum width");
	}
}

int BaslerWrapper::getMaxHeight (){
	if (BASLER_SAFEMODE){
		return 512;
	}
	try	{
		return OffsetY.GetMin ();
	}
	catch (Pylon::GenericException&){
		throwNested ("Failed to get maximum height");
	}
}

int BaslerWrapper::getMinGain (){
	if (BASLER_SAFEMODE){
		return 260;
	}
	try	{
#ifdef USB_CAMERA
		return Gain.GetMin ();
#elif defined FIREWIRE_CAMERA
		return GainRaw.GetMin ();
#endif
	}
	catch (Pylon::GenericException&){
		throwNested ("Failed to get Minimum gain");
	}
}


void BaslerWrapper::setOffsetX (int offset){
	if (!BASLER_SAFEMODE){
		try	{
			OffsetX.SetValue (offset);
		}
		catch (Pylon::GenericException&){
			throwNested ("Failed to set x offset");
		}
	}
}

void BaslerWrapper::setOffsetY (int offset){
	if (!BASLER_SAFEMODE){
		try{
			OffsetY.SetValue (offset);
		}
		catch (Pylon::GenericException&){
			throwNested ("Failed to Set Y Offset");
		}
	}
}

void BaslerWrapper::setWidth (int width){
	if (!BASLER_SAFEMODE){
		try	{
			Width.SetValue (width);
		}
		catch (Pylon::GenericException& err){
			throwNested (str("Failed to set width: ") + err.what());
		}
	}
}


void BaslerWrapper::setHeight (int height)
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			Height.SetValue (height);
		}
		catch (Pylon::GenericException&)
		{
			throwNested ("Failed to set height");
		}
	}
}


void BaslerWrapper::setHorBin (int binning)
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			BinningHorizontal.SetValue (binning);
		}
		catch (Pylon::GenericException&)
		{
			throwNested ("Failed to set horizontal binning ");
		}
	}
}


void BaslerWrapper::setVertBin (int binning){
	if (!BASLER_SAFEMODE){
		try{
			BinningVertical.SetValue (binning);
		}
		catch (Pylon::GenericException&){
			throwNested ("Failed to set vertical binning.");
		}
	}
}

void BaslerWrapper::stopGrabbing (){
	if (!BASLER_SAFEMODE){
		try{
			StopGrabbing ();
		}
		catch (Pylon::GenericException&){
			throwNested ("Failed to stop grabbing.");
		}
	}
}

bool BaslerWrapper::isGrabbing (){
	if (BASLER_SAFEMODE){
		return true;
	}
	try{
		return IsGrabbing ();
	}
	catch (Pylon::GenericException&){
		throwNested ("Failed to query if grabbing.");
	}
}

// not being used???
std::vector<long> BaslerWrapper::retrieveResult (unsigned int timeout){
	Pylon::CGrabResultPtr resultPtr;
	RetrieveResult (timeout, resultPtr, Pylon::TimeoutHandling_ThrowException);
	if (!resultPtr->GrabSucceeded ()){
		thrower ("" + str (resultPtr->GetErrorCode ()) + " " + std::string (resultPtr->GetErrorDescription ().c_str ()));
	}
	const uint16_t* pImageBuffer = (uint16_t*)resultPtr->GetBuffer ();
	int width = resultPtr->GetWidth ();
	int height = resultPtr->GetHeight ();
	std::vector<long> image (pImageBuffer, pImageBuffer + width * height);
	return image;
}


int BaslerWrapper::getCurrentHeight (){
	if (BASLER_SAFEMODE){
		return 512;
	}
	try{
		return Height.GetValue ();
	}
	catch (Pylon::GenericException&){
		throwNested ("Failed to get current height");
	}
}


int BaslerWrapper::getCurrentWidth (){
	if (BASLER_SAFEMODE){
		return 672;
	}
	try{
		return Width.GetValue ();
	}
	catch (Pylon::GenericException&){
		throwNested ("Failed to get current width");
	}
}

int BaslerWrapper::getCurrentOffsetX (){
	if (BASLER_SAFEMODE){
		return 0;
	}
	try{
		return OffsetX.GetValue ();
	}
	catch (Pylon::GenericException&){
		throwNested ("Failed to get current x offset");
	}
}

int BaslerWrapper::getCurrentOffsetY (){
	if (BASLER_SAFEMODE){
		return 0;
	}
	try{
		return OffsetY.GetValue ();
	}
	catch (Pylon::GenericException&){
		throwNested ("Failed to get current y offset.");
	}
}

void BaslerWrapper::setPixelFormat (cameraParams::PixelFormatEnums pixelFormat){
	if (!BASLER_SAFEMODE){
		try	{
			PixelFormat.SetValue (pixelFormat);
		}
		catch (Pylon::GenericException&){
			throwNested ("Failed to set pixel format.");
		}
	}
}

void BaslerWrapper::setGainMode (std::string mode){
	if (!BASLER_SAFEMODE){
		try	{
			GainAuto.FromString (mode.c_str ());
		}
		catch (Pylon::GenericException&){
			throwNested ("Failed to set gain mode.");
		}
	}
}

void BaslerWrapper::setGain (int gainValue){
	if (!BASLER_SAFEMODE){
		try	{
#ifdef USB_CAMERA
			Gain.SetValue (gainValue);
#elif defined FIREWIRE_CAMERA
			GainRaw.SetValue (gainValue);
#endif
		}
		catch (Pylon::GenericException&){
			throwNested ("Failed to set gain.");
		}
	}
}


void BaslerWrapper::waitForFrameTriggerReady (unsigned int timeout){
	if (!BASLER_SAFEMODE){
		try{
			WaitForFrameTriggerReady (timeout, Pylon::TimeoutHandling_ThrowException);
		}
		catch (Pylon::GenericException&){
			throwNested ("Failed to wait for frame trigger to be ready.");
		}
	}
}

void BaslerWrapper::executeSoftwareTrigger (){
	if (!BASLER_SAFEMODE){
		try{
			ExecuteSoftwareTrigger ();
		}
		catch (Pylon::GenericException&){
			throwNested ("Failed to execute software trigger.");
		}
	}
}

void BaslerWrapper::setTriggerSource (cameraParams::TriggerSourceEnums mode){
	if (!BASLER_SAFEMODE){
		try{
			TriggerSource.SetValue (mode);
		}
		catch (Pylon::GenericException&){
			throwNested ("Failed to set trigger source.");
		}
	}
}

void BaslerWrapper::startGrabbing (unsigned int picturesToGrab, Pylon::EGrabStrategy grabStrat){
	if (!BASLER_SAFEMODE){
		try{
			StartGrabbing (picturesToGrab, grabStrat, Pylon::GrabLoop_ProvidedByInstantCamera);
		}
		catch (Pylon::GenericException&){
			throwNested ("Failed to start grabbing.");
		}
	}
}

// returns in us
double BaslerWrapper::getExposureMax (){
	if (!BASLER_SAFEMODE){
		try	{
#ifdef USB_CAMERA
			return ExposureTime.GetMax ();
#elif defined FIREWIRE_CAMERA
			return ExposureTimeRaw.GetMax ();
#endif
		}
		catch (Pylon::GenericException&){
			throwNested ("Failed to get max exposure.");
		}
	}
	return 1000000;
}

double BaslerWrapper::getExposureMin ()
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
#ifdef USB_CAMERA
			return ExposureTime.GetMin ();
#elif defined FIREWIRE_CAMERA
			return ExposureTimeRaw.GetMin ();
#endif
		}
		catch (Pylon::GenericException&)
		{
			throwNested ("Failed to get min exposure.");
		}
	}
	return 0;
}

double BaslerWrapper::getCurrentExposure ()
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
#ifdef USB_CAMERA
			return ExposureTime.GetValue ();
#elif defined FIREWIRE_CAMERA
			return ExposureTimeRaw.GetValue ();
#endif
		}
		catch (Pylon::GenericException&)
		{
			throwNested ("Failed to get current exposure.");
		}
	}
	return 10000;
}

void BaslerWrapper::setExposure (double exposureTime)
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
#ifdef USB_CAMERA
			ExposureTime.SetValue (exposureTime);
#elif defined FIREWIRE_CAMERA
			ExposureTimeRaw.SetValue (exposureTime);
#endif
		}
		catch (Pylon::GenericException&)
		{
			throwNested ("Failed to set exposure.");
		}
	}
}


void BaslerWrapper::setExposureAuto (cameraParams::ExposureAutoEnums mode)
{
	if (!BASLER_SAFEMODE)
	{
		try
		{
			ExposureAuto.SetValue (mode);
		}
		catch (Pylon::GenericException&)
		{
			throwNested ("Failed to set auto exposure mode.");
		}
	}
}

