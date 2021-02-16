// created by Mark O. Brown
#include "stdafx.h"

#include "BaslerCamera.h"
#include "BaslerWrapper.h"

#include "GeneralImaging/PictureControl.h"
#include "ConfigurationSystems/ConfigSystem.h"
#include "MiscellaneousExperimentOptions/Repetitions.h"
#include <ExperimentThread/ExpThreadWorker.h>

#include "stdint.h"
#include <cmath>
#include <algorithm>
#include <functional>
#include <qdebug.h>

baslerSettings BaslerCameraCore::getSettingsFromConfig (ConfigStream& configFile){
	if (configFile.ver < Version ("4.0")){
		thrower ("Basler settings requires version 4.0+ Configuration files");
	}
	baslerSettings newSettings;
	if (configFile.ver > Version ("5.6")) {
		configFile >> newSettings.expActive;
	}
	else {
		newSettings.expActive = true;
	}
	std::string txt;

	configFile >> txt;
	newSettings.acquisitionMode = BaslerAcquisition::fromStr (txt);
	std::string test;
	try{
		configFile >> test;
		newSettings.dims.left = boost::lexical_cast<int>(test);
		configFile >> test;
		newSettings.dims.top = boost::lexical_cast<int>(test);
		configFile >> test;
		newSettings.dims.right = boost::lexical_cast<int>(test);
		configFile >> test;
		newSettings.dims.bottom = boost::lexical_cast<int>(test);
	}
	catch (boost::bad_lexical_cast&){
		throwNested ("Basler control failed to convert dimensions recorded in the config file "
			"to integers");
	}
	configFile >> newSettings.dims.horizontalBinning;
	configFile >> newSettings.dims.verticalBinning;
	configFile >> txt;
	newSettings.exposureMode = BaslerAutoExposure::fromStr (txt);
	configFile >> newSettings.exposureTime;
	configFile >> newSettings.frameRate;
	configFile >> newSettings.rawGain;
	configFile >> newSettings.picsPerRep;
	configFile >> txt;
	newSettings.triggerMode = BaslerTrigger::fromStr (txt);
	return newSettings;
}
// important constructor;
// Create an instant camera object with the camera device found first. At this point this class is really only meant to 
// work with a single camera of one type at a time. Not sure what would happen if you had multiple cameras set up at 
// once.
BaslerCameraCore::BaslerCameraCore(IChimeraQtWindow* parent ){
	Pylon::PylonInitialize();
	Pylon::CDeviceInfo info;
	info.SetDeviceClass( cameraType::DeviceClass() );
	if (!BASLER_SAFEMODE){
		try{
			cameraType* temp;
			temp = new BaslerWrapper(Pylon::CTlFactory::GetInstance().CreateFirstDevice(info));
			camera = dynamic_cast<BaslerWrapper*>(temp);
			camera->init( parent );
			cameraInitialized = true;
		}
		catch (Pylon::GenericException&){
			cameraInitialized = false;
		}
	}
	else{
		camera->init( parent );
		cameraInitialized = true;
	}
}

bool BaslerCameraCore::isRunning ( ){ 
	return camera->isGrabbing ( );
}

bool BaslerCameraCore::isInitialized(){
	return cameraInitialized;
}

// send a software trigger to the camera after waiting to make sure it's ready to recieve said trigger.
void BaslerCameraCore::softwareTrigger(){
	camera->waitForFrameTriggerReady( 5000 );
	camera->executeSoftwareTrigger();
}


// important deconstructor.
BaslerCameraCore::~BaslerCameraCore(){
	Pylon::PylonTerminate();
	delete camera;
}


// get some information about the camera from the camera itself through pylon.
std::string BaslerCameraCore::getCameraInfo(){
	// Get camera device information.
	return "Camera Device Information\n=========================\nVendor           : " + std::string( camera->DeviceVendorName.GetValue() )
			+ "\nModel            : " + std::string( camera->DeviceModelName.GetValue() ) + "\nFirmware version : "
			+ std::string( camera->DeviceFirmwareVersion.GetValue() ) ;
}


// Can change this for nicer defaults.
baslerSettings BaslerCameraCore::getDefaultSettings(){
	baslerSettings defaultSettings;
	QPoint dim = getCameraDimensions();
	defaultSettings.picsPerRep = 1;
	defaultSettings.dims.left = 1;
	defaultSettings.dims.right = dim.x(); 
	defaultSettings.dims.horizontalBinning = 4;
	defaultSettings.dims.top = dim.y();
	defaultSettings.dims.bottom = 1;
	defaultSettings.dims.verticalBinning = 4;
	defaultSettings.exposureMode = BaslerAutoExposure::mode::Off;
	defaultSettings.exposureTime = 1000;
	defaultSettings.frameRate = 0.25;
	defaultSettings.rawGain = camera->getMinGain();
	return defaultSettings;
}

// set the default parameters defined in the function above.
void BaslerCameraCore::setDefaultParameters(){
	setBaslserAcqParameters( getDefaultSettings() );
}

// general function you should use for setting camera settings based on the input.
void BaslerCameraCore::setBaslserAcqParameters (baslerSettings settings) {
	/// Set the AOI:
	/*
		there is never a problem making things smaller. the annoying problem is over-reaching the maximum whose 
		bounds change dynamically, where the basler api would then throw an error, even if you were about to change 
		another parameter to make it work out in the end. The basler API is very picky about the order of these things. 
		Ways for this to crash:
		- Setting offset large (before making width smaller) pushes the right border past max width
		- setting width large (before making offset smaller) pushes right border past max width
		- setting the binning large (before changing the width)
	*/
	// So, start from a safe place.
	camera->setOffsetX (0);
	camera->setWidth (16);
	camera->setHorBin (1);
	camera->setOffsetY (0);
	camera->setHeight (16);
	camera->setVertBin (1);

	// suppose you start real small. 16 1x1 pixels.  then, this always works:
	// set x offset (no chance of pushing right because width is small)
	// set width (no chance of pushing right off because of binning because binning is minimal so potential value 
	// for rightmost point is maximal.
	// set binning 
	camera->setOffsetX (settings.dims.left);
	camera->setWidth (settings.dims.horRawPixelNum ());
	camera->setHorBin (settings.dims.horizontalBinning);
	camera->setOffsetY (settings.dims.bottom);
	camera->setHeight (settings.dims.vertRawPixelNum ());
	camera->setVertBin (settings.dims.verticalBinning);

	/// set other parameters
#ifdef USB_CAMERA
	camera->setPixelFormat (cameraParams::PixelFormat_Mono10);
#elif defined FIREWIRE_CAMERA
	camera->setPixelFormat (cameraParams::PixelFormat_Mono16);
#endif

	camera->setGainMode ("Off");
	camera->setGain (camera->getMinGain ());
	//camera->AcquisitionFrameRateEnable.SetValue(true);
	//Basler_UsbCameraParams::AcquisitionModeEnums::AcquisitionMode_Continuous
	//camera->AcquisitionMode.SetValue(Basler_UsbCameraParams::AcquisitionModeEnums::AcquisitionMode_Continuous);
	//camera->AcquisitionFrameRate.SetValue(settings.frameRate);
	// exposure mode
	if (settings.acquisitionMode == BaslerAcquisition::mode::Continuous) {
		settings.picsPerRep = UINT_MAX;
	}
	if (settings.exposureMode == BaslerAutoExposure::mode::Continuous){
		camera->setExposureAuto( cameraParams::ExposureAuto_Continuous );
	}
	else if (settings.exposureMode == BaslerAutoExposure::mode::Off){
		camera->setExposureAuto( cameraParams::ExposureAuto_Off );

		if (!(settings.exposureTime >= camera->getExposureMin() && settings.exposureTime <= camera->getExposureMax())){
			thrower ( "exposure time must be between " + str( camera->getExposureMin() ) + " and " 
					 + str( camera->getExposureMax()) );
		}
		camera->setExposure( settings.exposureTime );
	}
	else if (settings.exposureMode == BaslerAutoExposure::mode::Once){
		camera->setExposureAuto( cameraParams::ExposureAuto_Once );
	}

	if (settings.triggerMode == BaslerTrigger::mode::External){
		#ifdef FIREWIRE_CAMERA
			camera->setTriggerSource(cameraParams::TriggerSource_Line1);
		#elif defined USB_CAMERA
			camera->setTriggerSource(cameraParams::TriggerSource_Line3);
		#endif
	}
	else if (settings.triggerMode == BaslerTrigger::mode::AutomaticSoftware ){
		camera->setTriggerSource( cameraParams::TriggerSource_Software );
	}
	else if (settings.triggerMode == BaslerTrigger::mode::ManualSoftware ){
		camera->setTriggerSource( cameraParams::TriggerSource_Software );
	}
	runSettings = settings;
}

baslerSettings BaslerCameraCore::getRunningSettings (){
	return runSettings;
}

// I can potentially use this to reopen the camera if e.g. the user disconnects. Don't think this is really implemented
// yet.
void BaslerCameraCore::reOpenCamera(IChimeraQtWindow* parent ){
	Pylon::CDeviceInfo info;
	info.SetDeviceClass( cameraType::DeviceClass() );
	if (!BASLER_SAFEMODE){
		delete camera;
		cameraType* temp;
		temp = new cameraType( Pylon::CTlFactory::GetInstance().CreateFirstDevice( info ) );
		camera = dynamic_cast<BaslerWrapper*>(temp);
	}
	camera->init( parent );
}

// get the dimensions of the camera. This is tricky because while I can get info about each parameter easily through
// pylon, several of the parameters, such as the width, are context-sensitive and return the max values as possible 
// given other camera settings at the moment (e.g. the binning).
QPoint BaslerCameraCore::getCameraDimensions(){
	if (BASLER_SAFEMODE){
		return { 672, 512 };
	}
	QPoint dim;
	// record the original offsets and bins.
	QPoint offsets, bins;
	offsets.rx() = camera->OffsetX.GetValue();
	offsets.ry() = camera->OffsetY.GetValue();
	bins.rx() = camera->BinningHorizontal.GetValue();
	bins.ry() = camera->BinningVertical.GetValue();
	
	// TODO:
	// change to my apif
	camera->BinningHorizontal.SetValue( camera->BinningHorizontal.GetMin() );
	camera->BinningVertical.SetValue( camera->BinningVertical.GetMin() );
	camera->OffsetX.SetValue( camera->OffsetX.GetMin() );
	camera->OffsetY.SetValue( camera->OffsetY.GetMin() );

	dim.rx() = camera->Width.GetMax() - 1;
	dim.ry() = camera->Height.GetMax() - 1;

	camera->OffsetX.SetValue( offsets.x() );
	camera->OffsetY.SetValue( offsets.y() );
	camera->BinningHorizontal.SetValue( bins.x() );
	camera->BinningVertical.SetValue( bins.y() );

	return dim;
}


// get the camera "armed" (ready for aquisition). Actual start of camera taking pictures depends on things like the 
// trigger mode.
void BaslerCameraCore::armCamera( ){
	Pylon::EGrabStrategy grabStrat;
	if (runSettings.acquisitionMode == BaslerAcquisition::mode::Continuous ){
		grabStrat = Pylon::GrabStrategy_LatestImageOnly;
	}
	else{
		grabStrat = Pylon::GrabStrategy_OneByOne;
	}
	triggerThreadInput* input = new triggerThreadInput;
	input->camera = camera;
	input->frameRate = runSettings.frameRate;
	camera->startGrabbing ( runSettings.totalPictures(), grabStrat );
	if ( runSettings.triggerMode == BaslerTrigger::mode::AutomaticSoftware ){
		_beginthread( triggerThread, 0, input );
	}
}

// 
double BaslerCameraCore::getCurrentExposure(){
	return camera->getCurrentExposure();
}

//
unsigned int BaslerCameraCore::getPicsPerRep(){
	return runSettings.picsPerRep;
}

//
bool BaslerCameraCore::isContinuous(){
	return runSettings.acquisitionMode == BaslerAcquisition::mode::Continuous;
}

// this is the thread that programatically software-triggers the camera when triggering internally.
void BaslerCameraCore::triggerThread( void* voidInput ){
	int count = 0;
	triggerThreadInput* input = (triggerThreadInput*)voidInput;
	try{
		while (input->camera->isGrabbing())	{
			// adjust for frame rate
			Sleep(int(1000.0 / input->frameRate));
			try	{
				// Execute the software trigger. The call waits up to 100 ms for the camera
				// to be ready to be triggered.
				input->camera->waitForFrameTriggerReady(1000);
				input->camera->executeSoftwareTrigger();			
				count++;
			}
			catch (ChimeraError& err){
				qDebug () << cstr(err.trace ());
				// continue... should be stopping grabbing.
				break;
			}
		}
	}
	catch (Pylon::TimeoutException& timeoutErr){
		qDebug () << cstr("Trigger Thread Timeout Error!" + std::string (timeoutErr.what ()));
	}
	catch (Pylon::RuntimeException&){
		// probably aborted, that's fine. return.
	}
}


// stop the camera from taking any pictures, even if triggered afterwards.
void BaslerCameraCore::disarm(){
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
int64_t BaslerCameraCore::Adjust( int64_t val, int64_t minimum, int64_t maximum, int64_t inc ){
	// Check the input parameters.
	if (inc <= 0){
		// Negative increments are invalid.
		throw LOGICAL_ERROR_EXCEPTION( "Unexpected increment %d", inc );
	}
	if (minimum > maximum){
		// Minimum must not be bigger than or equal to the maximum.
		throw LOGICAL_ERROR_EXCEPTION( "minimum bigger than maximum." );
	}
	if (val < minimum){
		return minimum;
	}
	if (val > maximum){
		return maximum;
	}
	// Check the increment.
	if (inc == 1){
		// Special case: all values are valid.
		return val;
	}
	else{
		// The value must be min + (n * inc).
		// Due to the integer division, the value will be rounded down.
		return minimum + (((val - minimum) / inc) * inc);
	}
}

void BaslerCameraCore::logSettings (DataLogger& log, ExpThreadWorker* threadworker){
	try{
		if (!experimentActive){
			H5::Group baslerGroup (log.file.createGroup ("/Basler:Off"));
			return;
		}
		H5::Group baslerGroup (log.file.createGroup ("/Basler"));
		hsize_t rank1[] = { 1 };
		// pictures. These are permanent members of the class for speed during the writing process.	
		hsize_t setDims[] = { unsigned __int64 (expRunSettings.totalPictures ()), expRunSettings.dims.width (),
							   expRunSettings.dims.height () };
		hsize_t picDims[] = { 1, expRunSettings.dims.width (), expRunSettings.dims.height () };
		log.BaslerPicureSetDataSpace = H5::DataSpace (3, setDims);
		log.BaslerPicDataSpace = H5::DataSpace (3, picDims);
		log.BaslerPictureDataset = baslerGroup.createDataSet ("Pictures", H5::PredType::NATIVE_LONG, 
															  log.BaslerPicureSetDataSpace);
		log.currentBaslerPicNumber = 0;
		log.writeDataSet (BaslerAcquisition::toStr (expRunSettings.acquisitionMode), "Camera-Mode", baslerGroup);
		log.writeDataSet (BaslerAutoExposure::toStr (expRunSettings.exposureMode), "Exposure-Mode", baslerGroup);
		log.writeDataSet (expRunSettings.exposureTime, "Exposure-Time", baslerGroup);
		log.writeDataSet (BaslerTrigger::toStr (expRunSettings.triggerMode), "Trigger-Mode", baslerGroup);
		// image settings
		H5::Group imageDims = baslerGroup.createGroup ("Image-Dimensions");
		log.writeDataSet (expRunSettings.dims.top, "Top", imageDims);
		log.writeDataSet (expRunSettings.dims.bottom, "Bottom", imageDims);
		log.writeDataSet (expRunSettings.dims.left, "Left", imageDims);
		log.writeDataSet (expRunSettings.dims.right, "Right", imageDims);
		log.writeDataSet (expRunSettings.dims.horizontalBinning, "Horizontal-Binning", imageDims);
		log.writeDataSet (expRunSettings.dims.verticalBinning, "Vertical-Binning", imageDims);
		log.writeDataSet (expRunSettings.frameRate, "Frame-Rate", baslerGroup);
		log.writeDataSet (expRunSettings.rawGain, "Raw-Gain", baslerGroup);
	}
	catch (H5::Exception err){
		log.logError (err);
		throwNested ("ERROR: Failed to log basler parameters in HDF5 file: " + err.getDetailMsg ());
	}
}

void BaslerCameraCore::loadExpSettings (ConfigStream& stream){
	ConfigSystem::stdGetFromConfig (stream, *this, expRunSettings, Version ("4.0"));
	expRunSettings.repsPerVar = ConfigSystem::stdConfigGetter ( stream, "REPETITIONS", 
																 Repetitions::getSettingsFromConfig);
	experimentActive = expRunSettings.expActive;
}

void BaslerCameraCore::calculateVariations (std::vector<parameterType>& params, ExpThreadWorker* threadworker){
	expRunSettings.variations = (params.size () == 0 ? 1 : params.front ().keyValues.size ());
	if (experimentActive){
		if (isRunning ()) {
			disarm();
		}
		emit threadworker->prepareBasler (&expRunSettings);
		setBaslserAcqParameters (expRunSettings);
		armCamera ();
	}
}

