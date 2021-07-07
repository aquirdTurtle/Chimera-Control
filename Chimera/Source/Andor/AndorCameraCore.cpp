// created by Mark O. Brown
#include "stdafx.h"
#include "ATMCD32D.h"
#include "Andor/AndorCameraCore.h"
#include "Andor/AndorTriggerModes.h"
#include "Andor/AndorRunMode.h"
#include "ConfigurationSystems/ConfigSystem.h"
#include "MiscellaneousExperimentOptions/Repetitions.h"
#include <Andor/AndorCameraThreadWorker.h>
#include <PrimaryWindows/QtMainWindow.h>
#include <PrimaryWindows/QtAndorWindow.h>
#include <ExperimentThread/ExpThreadWorker.h>
#include <qdebug.h>
#include <qthread.h>
#include <chrono>
#include <process.h>
#include <algorithm>
#include <numeric>
#include <random>

std::string AndorCameraCore::getSystemInfo(){
	std::string info;
	// can potentially get more info from this.
	//AndorCapabilities capabilities;
	//getCapabilities( capabilities );
	info += "Camera Model: " + flume.getHeadModel() + "\n\t";
	int num; 
	flume.getSerialNumber(num);
	info += "Camera Serial Number: " + str(num) + "\n";
	return info;
}

AndorRunSettings AndorCameraCore::getSettingsFromConfig (ConfigStream& configFile){
	AndorRunSettings tempSettings; 
	tempSettings.imageSettings = ConfigSystem::stdConfigGetter ( configFile, "CAMERA_IMAGE_DIMENSIONS", 
																  ImageDimsControl::getImageDimSettingsFromConfig);
	ConfigSystem::initializeAtDelim (configFile, "CAMERA_SETTINGS");
	if (configFile.ver >= Version ("5.6")) {
		configFile >> tempSettings.controlCamera;
	}
	else {
		tempSettings.controlCamera = true;
	}
	configFile.get ();
	std::string txt = configFile.getline ();
	tempSettings.triggerMode = AndorTriggerMode::fromStr (txt);
	configFile >> tempSettings.emGainModeIsOn;
	configFile >> tempSettings.emGainLevel;
	txt = configFile.getline ();
	if (txt == AndorRunModes::toStr (AndorRunModes::mode::Video) || txt == "Video Mode"){
		tempSettings.acquisitionMode = AndorRunModes::mode::Video;
		tempSettings.repetitionsPerVariation = INT_MAX;
	}
	else if (txt == AndorRunModes::toStr (AndorRunModes::mode::Kinetic) || txt == "Kinetic Series Mode"){
		tempSettings.acquisitionMode = AndorRunModes::mode::Kinetic;
	}
	else if (txt == AndorRunModes::toStr (AndorRunModes::mode::Accumulate) || txt == "Accumulate Mode")	{
		tempSettings.acquisitionMode = AndorRunModes::mode::Accumulate;
	}
	else{
		thrower ("ERROR: Unrecognized camera mode: " + txt);
	}
	configFile >> tempSettings.kineticCycleTime;
	configFile >> tempSettings.accumulationTime;
	configFile >> tempSettings.accumulationNumber;
	configFile >> tempSettings.temperatureSetting;
	if (configFile.ver > Version ("4.7")){
		unsigned numExposures = 0;
		configFile >> numExposures;
		tempSettings.exposureTimes.resize (numExposures);
		for (auto& exp : tempSettings.exposureTimes){
			configFile >> exp;
		}
		configFile >> tempSettings.picsPerRepetition;
	}
	else { 
		tempSettings.picsPerRepetition = 1;
		tempSettings.exposureTimes.clear ();
		tempSettings.exposureTimes.push_back (1e-3);
	} 
	if (configFile.ver >= Version ("5.8")) {
		configFile >> tempSettings.horShiftSpeedSetting;
		configFile >> tempSettings.vertShiftSpeedSetting;
	} 
	else { 
		tempSettings.horShiftSpeedSetting = 0;
		tempSettings.vertShiftSpeedSetting = 0;
	} 
	if (configFile.ver > Version ("5.10")) {
		configFile >> tempSettings.frameTransferMode;
	}
	else {
		tempSettings.frameTransferMode = 0;
	}
	if (configFile.ver >= Version("5.13")) {
		configFile >> tempSettings.verticalShiftVoltageAmplitude;
	}
	else {
		tempSettings.verticalShiftVoltageAmplitude = 0;
	}
	return tempSettings;
} 


AndorCameraCore::AndorCameraCore( bool safemode_opt ) : safemode( safemode_opt ), flume( safemode_opt ){
	runSettings.emGainModeIsOn = false;
	flume.initialize( );
	flume.setBaselineClamp( 1 );
	flume.setBaselineOffset( 0 );
	flume.setDMAParameters( 1, 0.0001f );
}

void AndorCameraCore::initializeClass(IChimeraQtWindow* parent, chronoTimes* imageTimes){
	threadInput.imageTimes = imageTimes;
	threadInput.Andor = this;
	threadInput.expectingAcquisition = false;
	threadInput.safemode = safemode;
	threadInput.runMutex = &camThreadMutex;
	// begin the camera wait thread.
	AndorCameraThreadWorker* worker = new AndorCameraThreadWorker (&threadInput);
	QThread* thread = new QThread;
	worker->moveToThread (thread);
	parent->mainWin->connect (worker, &AndorCameraThreadWorker::notify,
							  parent->mainWin, &QtMainWindow::handleNotification);

	parent->andorWin->connect (worker, &AndorCameraThreadWorker::pictureTaken,
							   parent->andorWin, &QtAndorWindow::onCameraProgress);

	parent->mainWin->connect (thread, SIGNAL (started ()), worker, SLOT (process ()));
	parent->mainWin->connect (thread, SIGNAL (finished ()), thread, SLOT (deleteLater ()));
	thread->start (QThread::TimeCriticalPriority);

}

void AndorCameraCore::updatePictureNumber( unsigned __int64 newNumber ){
	currentPictureNumber = newNumber;
}

/* 
 * pause the camera thread which watches the camera for pictures
 */
void AndorCameraCore::pauseThread(){
	// andor should not be taking images anymore at this point.
	threadInput.expectingAcquisition = false;
}

/*
 * this should get called when the camera finishes running. right now this is very simple.
 */
void AndorCameraCore::onFinish(){
	//threadInput.signaler.notify_all();
	cameraIsRunning = false;
}

void AndorCameraCore::setCalibrating( bool cal ){
	calInProgress = cal;
}

bool AndorCameraCore::isCalibrating( ){
	return calInProgress;
}

/*
 * Get whatever settings the camera is currently using in it's operation, assuming it's operating.
 */
AndorRunSettings AndorCameraCore::getAndorRunSettings(){
	return runSettings;
}

void AndorCameraCore::setSettings(AndorRunSettings settingsToSet){
	runSettings = settingsToSet;
}

void AndorCameraCore::setAcquisitionMode(){
	flume.setAcquisitionMode(int(runSettings.acquisitionMode));
}

std::vector<std::string> AndorCameraCore::getVertShiftSpeeds () {
	auto numSpeeds = flume.getNumberVSSpeeds ();
	std::vector<std::string> speeds (numSpeeds);
	for (auto speedNum : range(numSpeeds)) {
		speeds[speedNum] = str(flume.getVSSpeed (speedNum),2);
	}
	return speeds;
}

std::vector<std::string> AndorCameraCore::getHorShiftSpeeds () {
	auto numSpeeds = flume.getNumberHSSpeeds ();
	std::vector<std::string> speeds (numSpeeds);
	for (auto speedNum : range (numSpeeds)) {
		speeds[speedNum] = str(flume.getHSSpeed (1,0,speedNum),2);
	}
	return speeds;
}

/* 
 * Large function which initializes a given camera image run.
 */
void AndorCameraCore::armCamera( double& minKineticCycleTime, ExpThreadWorker* threadworker){
	/// Set a bunch of parameters.
	// Set to 1 MHz readout rate in both cases
	flume.setADChannel(1);
	if (runSettings.emGainModeIsOn)	{
		flume.setHSSpeed(0, runSettings.horShiftSpeedSetting);
		qDebug () << "Horizontal Shift Speed: " << flume.getHSSpeed (1, 0, runSettings.horShiftSpeedSetting);
	}
	else {
		flume.setHSSpeed(1, runSettings.horShiftSpeedSetting);
		qDebug () << "Horizontal Shift Speed: " << flume.getHSSpeed (1, 1, runSettings.horShiftSpeedSetting);
	}
	flume.setVSAmplitude(runSettings.verticalShiftVoltageAmplitude);
	flume.setVSSpeed (runSettings.vertShiftSpeedSetting);
	qDebug () << "Vertical Shift Speed: " << flume.getVSSpeed (runSettings.vertShiftSpeedSetting);
	setAcquisitionMode();
	setReadMode();
	setExposures();
	setImageParametersToCamera();
	// Set Mode-Specific Parameters
	if (runSettings.acquisitionMode == AndorRunModes::mode::Video){
		setFrameTransferMode();
		setKineticCycleTime ();
	}
	else if (runSettings.acquisitionMode == AndorRunModes::mode::Kinetic){
		setKineticCycleTime();
		setScanNumber();
		// set this to 1.
		setNumberAccumulations(true);
		setFrameTransferMode ( );
	}
	else if (runSettings.acquisitionMode == AndorRunModes::mode::Accumulate){
		setAccumulationCycleTime();
		setNumberAccumulations(false);
	}
	setGainMode(threadworker);
	
	setCameraTriggerMode();

	flume.queryStatus();

	/// Do some plotting stuffs
	// get the min time after setting everything else.
	minKineticCycleTime = getMinKineticCycleTime( );

	cameraIsRunning = true;
	std::unique_lock<std::timed_mutex> lock (camThreadMutex, std::chrono::milliseconds(1000));
	if (!lock.owns_lock ())
	{/* Then the thread couldn't get a lock on the spurious wakeup check, but that means that the thread is just
	 waiting for an acquisition... should fix this behavior, but should be okay to continue.*/	}

	 // remove the spurious wakeup check.
	threadInput.expectingAcquisition = true;
	// notify the thread that the experiment has started..
	threadInput.signaler.notify_all();
	qDebug () << "Vertical Shift Speed: " << flume.getVSSpeed (runSettings.vertShiftSpeedSetting);
	flume.startAcquisition();
}

void AndorCameraCore::preparationChecks () {
	try {
		auto res = flume.checkForNewImages ();
		// success?!?
		thrower ("In preparation section, looks like there are already images in the camera???");
	}
	catch (ChimeraError & err) {
		// the expected result is throwing NO_NEW_DATA. 
		if (err.whatBare () != flume.getErrorMsg (DRV_NO_NEW_DATA)) {
			try {
				flume.andorErrorChecker (flume.queryStatus ());
			}
			catch (ChimeraError & err2) {
				throwNested ("Error seen while checking for new images: " + err.trace ()
					+ ", Camera Status: " + err2.trace () + ", Camera is running bool: " + str (cameraIsRunning));
			}
			throwNested ("Error seen while checking for new images: " + err.trace ()
				+ ", Camera Status: DRV_SUCCESS, Camera is running bool: " + str (cameraIsRunning));
		}
	}
}


/* 
 * This function checks for new pictures, if they exist it gets them, and shapes them into the array which holds all of
 * the pictures for a given repetition.
 */
std::vector<Matrix<long>> AndorCameraCore::acquireImageData (){
	try	{
		try	{
			flume.checkForNewImages ();
		}
		catch (ChimeraError& err) {
			try {
				flume.andorErrorChecker (flume.queryStatus ());
			}
			catch (ChimeraError & err2){
				throwNested ("Error seen while checking for new images: " + err.trace() 
					+ ", Camera Status:" + err2.trace() + ", Camera is running bool: " + str(cameraIsRunning));
			}
			throwNested ("Error seen while checking for new images: " + err.trace ()
				+ ", Camera Status: DRV_SUCCESS, Camera is running bool: " + str (cameraIsRunning));
		}
		// each image processed from the call from a separate windows message
		// If there is no data the acquisition must have been aborted
		int experimentPictureNumber = runSettings.showPicsInRealTime ? 0
			: (((currentPictureNumber - 1) % runSettings.totalPicsInVariation ()) % runSettings.picsPerRepetition);
		if (experimentPictureNumber == 0){
			repImages.clear ();
			repImages.resize (runSettings.showPicsInRealTime ? 1 : runSettings.picsPerRepetition);
		}
		auto& imSettings = runSettings.imageSettings;
		Matrix<long> tempImage (imSettings.width (), imSettings.height (), 0);
		repImages[experimentPictureNumber] = Matrix<long> (imSettings.height (), imSettings.width (), 0);
		if (!safemode){
			try	{
				flume.getOldestImage(tempImage);
			}
			catch (ChimeraError &)	{
				// let the blank image roll through to keep the image numbers going sensibly.
				//throwNested ("Error while calling getOldestImage.");
			}
			// immediately rotate
			for (auto imageVecInc : range(repImages[experimentPictureNumber].size ())){
				repImages[experimentPictureNumber].data[imageVecInc] = tempImage.data[((imageVecInc
					% imSettings.width ()) + 1) * imSettings.height () - imageVecInc / imSettings.width () - 1];
			}
		}
		else{
			for (auto imageVecInc : range (repImages[experimentPictureNumber].size ()))	{
				std::random_device rd;
				std::mt19937 e2 (rd ());
				std::normal_distribution<> dist (180, 20);
				std::normal_distribution<> dist2 (350, 100);
				tempImage.data[imageVecInc] = dist (e2) + 10;
				if (((imageVecInc / imSettings.width ()) % 2 == 1) && ((imageVecInc % imSettings.width ()) % 2 == 1)){
					// can have an atom here.
					if (unsigned (rand ()) % 300 > imageVecInc + 50){
						// use the exposure time and em gain level 
						tempImage.data[imageVecInc] += runSettings.exposureTimes[experimentPictureNumber] * 1e3 * dist2 (e2);
						if (runSettings.emGainModeIsOn)
						{
							tempImage.data[imageVecInc] *= runSettings.emGainLevel;
						}
					}
				}
			}
			auto& ims = runSettings.imageSettings;
			for (auto rowI : range (repImages[experimentPictureNumber].getRows ()))	{
				for (auto colI : range (repImages[experimentPictureNumber].getCols ()))	{
					repImages[experimentPictureNumber] (rowI, colI) = tempImage (tempImage.getRows()-colI-1, rowI);
				}
			}
		}
		return repImages;
	}
	catch (ChimeraError &){
		throwNested ("Error Seen in acquireImageData.");
	}
}


// sets this based on internal settings object.
void AndorCameraCore::setCameraTriggerMode(){
	std::string errMsg;
	int trigType;
	if (runSettings.triggerMode == AndorTriggerMode::mode::Internal){
		trigType = 0;
	}
	else if (runSettings.triggerMode == AndorTriggerMode::mode::External){
		trigType = 1;
	}
	else if (runSettings.triggerMode == AndorTriggerMode::mode::StartOnTrigger){
		trigType = 6;
	}
	flume.setTriggerMode(trigType);
}


void AndorCameraCore::setTemperature(){
	// Get the current temperature
	if (runSettings.temperatureSetting < -60 || runSettings.temperatureSetting > 25){
		auto answer = QMessageBox::question(nullptr, "Temperature Warning!", "Warning: The selected temperature is "
			"outside the \"normal\" temperature range of the camera (-60 through 25 C). Proceed anyways?");
		if (answer == QMessageBox::No){
			return;
		}
	}
	// Proceedure to initiate cooling
	changeTemperatureSetting( false );
}


void AndorCameraCore::setReadMode(){
	flume.setReadMode(runSettings.readMode);
}


void AndorCameraCore::setExposures(){
	if (runSettings.exposureTimes.size() > 0 && runSettings.exposureTimes.size() <= 16){
		flume.setRingExposureTimes(runSettings.exposureTimes.size(), runSettings.exposureTimes.data());
	}
	else{
		thrower ("ERROR: Invalid size for vector of exposure times, value of " + str(runSettings.exposureTimes.size()) + ".");
	}
}


void AndorCameraCore::setImageParametersToCamera(){
	auto& im = runSettings.imageSettings;
	if (((im.bottom - im.top + 1) % im.verticalBinning) != 0) {
		qDebug() << "(bottom - top + 1) % vertical binning must be 0!";
	}
	if (((im.right - im.left + 1) % im.horizontalBinning) != 0) {
		qDebug () << "(right - left + 1) % horizontal binning must be 0!";
	}
	flume.setImage( im.verticalBinning, im.horizontalBinning, im.bottom, im.top,  im.left, im.right );
}


void AndorCameraCore::setKineticCycleTime(){
	flume.setKineticCycleTime(runSettings.kineticCycleTime);
}


void AndorCameraCore::setScanNumber()
{
	if (runSettings.totalPicsInExperiment() == 0 && runSettings.totalPicsInVariation() != 0){
		// all is good. The first variable has not been set yet.
	}
	else if (runSettings.totalPicsInVariation() == 0){
		thrower ("ERROR: Scan Number Was Zero.\r\n");
	}
	else{
		//flume.setNumberKinetics(int(runSettings.totalPicsInExperiment()));
		flume.setNumberKinetics ( int ( runSettings.totalPicsInVariation ( ) ) );
	}
}


void AndorCameraCore::setFrameTransferMode(){
	flume.setFrameTransferMode(runSettings.frameTransferMode);
}


/*
 * exposures should be initialized to be the correct size. Nothing else matters for the inputs, they get 
 * over-written.
 * throws exception if fails
 */
void AndorCameraCore::checkAcquisitionTimings(float& kinetic, float& accumulation, std::vector<float>& exposures)
{
	if ( exposures.size ( ) == 0 ){
		return;
	}
	float tempExposure, tempAccumTime, tempKineticTime;
	float * timesArray = nullptr;
	std::string errMsg;
	if (safemode)
	{
		// if in safemode initialize this stuff to the values to be outputted.
		if (exposures.size() > 0){
			tempExposure = exposures[0];
		}
		else{
			tempExposure = 0;
		}
		tempAccumTime = accumulation;
		tempKineticTime = kinetic;
	}
	else{
		tempExposure = 0;
		tempAccumTime = 0;
		tempKineticTime = 0;
	}
	// It is necessary to get the actual times as the system will calculate the
	// nearest possible time. eg if you set exposure time to be 0, the system
	// will use the closest value (around 0.01s)
	timesArray = new float[exposures.size()];
	if (!safemode){
		flume.getAcquisitionTimes(tempExposure, tempAccumTime, tempKineticTime);
		flume.getAdjustedRingExposureTimes(int(exposures.size()), timesArray);
	}
	else {
		for (unsigned exposureInc = 0; exposureInc < exposures.size(); exposureInc++){
			timesArray[exposureInc] = exposures[exposureInc];
		}
	}
	// Set times
	if (exposures.size() > 0){
		for (unsigned exposureInc = 0; exposureInc < exposures.size(); exposureInc++){
			exposures[exposureInc] = timesArray[exposureInc];
		}
		delete[] timesArray;
	}
	accumulation = tempAccumTime;
	kinetic = tempKineticTime;
}

void AndorCameraCore::setAccumulationCycleTime(){
	flume.setAccumulationCycleTime(runSettings.accumulationTime);
}

void AndorCameraCore::setNumberAccumulations(bool isKinetic){
	std::string errMsg;
	if (isKinetic){
		// right now, kinetic series mode always has one accumulation. could add this feature later if desired to do 
		// both kinetic and accumulation. Not sure there's actually much of a reason to use accumulations, seems 
		// equivalent to software accumulation.
		//setNumberAccumulations(true); // ???
		flume.setAccumulationNumber(1);
	}
	else{
		// ???
		// setNumberAccumulations(false); // ???
		flume.setAccumulationNumber(runSettings.accumulationNumber);
	}
}


void AndorCameraCore::setGainMode(ExpThreadWorker* threadworker){
	if (!runSettings.emGainModeIsOn){
		// Set Gain
		int numGain;
		flume.getNumberOfPreAmpGains(numGain);
		flume.setPreAmpGain(2);
		float myGain;
		flume.getPreAmpGain(2, myGain);
		// 1 is for conventional gain mode.
		flume.setOutputAmplifier(1);
		notify(statusMsg("Using Andor Conventional Gain\n", 1, configDelim), threadworker);
	}
	else{
		// 0 is for em gain mode.
		flume.setOutputAmplifier(0);
		flume.setPreAmpGain(2);
		if (runSettings.emGainLevel > 300){
			flume.setEmGainSettingsAdvanced(1);
		}
		else{
			flume.setEmGainSettingsAdvanced(0);
		}
		flume.setEmCcdGain(runSettings.emGainLevel);
		notify( statusMsg("Using Andor EM Gain!!! Gain Value: " + str(runSettings.emGainLevel) +"\n", 0, configDelim), 
			    threadworker );
	}
}


void AndorCameraCore::changeTemperatureSetting(bool turnTemperatureControlOff){
	int minimumAllowedTemp, maximumAllowedTemp;
	// the default, in case the program is in safemode.
	minimumAllowedTemp = -60;
	maximumAllowedTemp = 25;
	// check if temp is in valid range
	flume.getTemperatureRange(minimumAllowedTemp, maximumAllowedTemp);
	if (runSettings.temperatureSetting < minimumAllowedTemp || runSettings.temperatureSetting > maximumAllowedTemp)	{
		thrower ("ERROR: Temperature is out of range\r\n");
	}
	else{
		// if it is in range, switch on cooler and set temp
		if (turnTemperatureControlOff == false)	{
			flume.temperatureControlOn();
		}
		else{
			flume.temperatureControlOff();
		}
	}

	if (turnTemperatureControlOff == false){
		flume.setTemperature(runSettings.temperatureSetting);
	}
	else{
		thrower ("Temperature Control has been turned off.\r\n");
	}
}

AndorTemperatureStatus AndorCameraCore::getTemperature ( ){
	AndorTemperatureStatus stat;
	stat.temperatureSetting = getAndorRunSettings().temperatureSetting;
	try{
		if (ANDOR_SAFEMODE) { stat.andorRawMsg = "SAFEMODE"; }
		else {
			auto msgCode = flume.getTemperature (stat.temperature);
			stat.andorRawMsg = flume.getErrorMsg (msgCode);
		}
		// if not stable this won't get changed.
		if (stat.andorRawMsg != "DRV_ACQUIRING") {
			mostRecentTemp = stat.temperature;
		}
		if (stat.andorRawMsg == "DRV_TEMPERATURE_STABILIZED") {
			stat.msg = "Temperature has stabilized at " + str (stat.temperature) + " (C)\r\n";
		}
		else if (stat.andorRawMsg == "DRV_TEMPERATURE_NOT_REACHED") {
			stat.msg = "Set temperature not yet reached. Current temperature is " + str (stat.temperature) + " (C)\r\n";
		}
		else if (stat.andorRawMsg == "DRV_TEMPERATURE_NOT_STABILIZED") {
			stat.msg = "Temperature of " + str (stat.temperature) + " (C) reached but not stable.";
		}
		else if (stat.andorRawMsg == "DRV_TEMPERATURE_DRIFT") {
			stat.msg = "Temperature had stabilized but has since drifted. Temperature: " + str (stat.temperature);
		}
		else if (stat.andorRawMsg == "DRV_TEMPERATURE_OFF") {
			stat.msg = "Temperature control is off. Temperature: " + str (stat.temperature);
		}
		else if (stat.andorRawMsg == "DRV_ACQUIRING") {
			// doesn't change color of temperature control. This way the color of the control represents the state of
			// the temperature right before the acquisition started, so that you can tell if you remembered to let it
			// completely stabilize or not.
			stat.msg = "Camera is Acquiring data. No updates are available. \r\nMost recent temperature: "
				+ str (mostRecentTemp);
		}
		else if (stat.andorRawMsg == "SAFEMODE") {
			stat.msg = "Device is running in Safemode... No Real Temperature Data is available.";
		}
		else {
			stat.msg = "Unexpected Temperature Message: " + stat.andorRawMsg + ". Temperature: "
				+ str (stat.temperature);
		}
	}
	catch ( ChimeraError& ){
		throwNested ("Failed to get temperature from andor?!");
	}
	return stat;
}

int AndorCameraCore::queryStatus ( ){
	return flume.queryStatus ( );
}

bool AndorCameraCore::isRunning ( ){
	return cameraIsRunning;
}

double AndorCameraCore::getMinKineticCycleTime ( ){
	// get the currently set kinetic cycle time.
	float minKineticCycleTime, dummy1, dummy2;
	flume.setKineticCycleTime ( 0 );
	flume.getAcquisitionTimes ( dummy1, dummy2, minKineticCycleTime );

	// re-set whatever's currently in the settings.
	setKineticCycleTime ( );
	return minKineticCycleTime;
}

void AndorCameraCore::setIsRunningState ( bool state ){
	cameraIsRunning = state;
}

void AndorCameraCore::abortAcquisition ( ){
	flume.abortAcquisition ( );
}

void AndorCameraCore::logSettings (DataLogger& log, ExpThreadWorker* threadworker){
	try	{
		if (!experimentActive)	{
			if (threadworker != nullptr) {
				notify({ qstr("Not logging Andor info!\n"), 0 }, threadworker);
			}
			H5::Group andorGroup (log.file.createGroup ("/Andor:Off"));
			return;
		}
		// in principle there are some other low level settings or things that aren't used very often which I could include 
		// here. I'm gonna leave this for now though.
		H5::Group andorGroup (log.file.createGroup ("/Andor"));
		hsize_t rank1[] = { 1 };
		// pictures. These are permanent members of the class for speed during the writing process.	
		if (expRunSettings.acquisitionMode == AndorRunModes::mode::Kinetic) {
			hsize_t setDims[] = { unsigned __int64 (expRunSettings.totalPicsInExperiment ()), expRunSettings.imageSettings.width (),
				expRunSettings.imageSettings.height () };
			hsize_t picDims[] = { 1, expRunSettings.imageSettings.width (), expRunSettings.imageSettings.height () };
			log.AndorPicureSetDataSpace = H5::DataSpace (3, setDims);
			log.AndorPicDataSpace = H5::DataSpace (3, picDims);
			log.AndorPictureDataset = andorGroup.createDataSet ( "Pictures", H5::PredType::NATIVE_LONG, 
																 log.AndorPicureSetDataSpace);
			log.currentAndorPicNumber = 0;
		}
		else {
			/*
				hsize_t setDims[] = { 0, settings.imageSettings.width (), settings.imageSettings.height () };
				hsize_t picDims[] = { 1, settings.imageSettings.width (), settings.imageSettings.height () };
				AndorPicureSetDataSpace = H5::DataSpace (3, setDims);
				AndorPicDataSpace = H5::DataSpace (3, picDims);
				AndorPictureDataset = andorGroup.createDataSet ("Pictures: N/A", H5::PredType::NATIVE_LONG, AndorPicureSetDataSpace);
			*/
		}
		if (log.AndorPicureSetDataSpace.getId () == -1) {
			thrower ("Failed to initialize AndorPicureSetDataSpace???");
		}
		if (log.AndorPicDataSpace.getId () == -1) {
			thrower ("Failed to initialize AndorPicDataSpace???");
		}
		if (log.AndorPictureDataset.getId () == -1) {
			thrower ("Failed to initialize AndorPictureDataset???");
		}
		log.andorDataSetShouldBeValid = true;
		log.writeDataSet (int (expRunSettings.acquisitionMode), "Camera-Mode", andorGroup);
		log.writeDataSet (expRunSettings.exposureTimes, "Exposure-Times", andorGroup);
		log.writeDataSet (AndorTriggerMode::toStr (expRunSettings.triggerMode), "Trigger-Mode", andorGroup);
		log.writeDataSet (expRunSettings.emGainModeIsOn, "EM-Gain-Mode-On", andorGroup);
		if (expRunSettings.emGainModeIsOn) {
			log.writeDataSet (expRunSettings.emGainLevel, "EM-Gain-Level", andorGroup);
		}
		else{
			log.writeDataSet (-1, "NA:EM-Gain-Level", andorGroup);
		}
		// image settings
		H5::Group imageDims = andorGroup.createGroup ("Image-Dimensions");
		log.writeDataSet (expRunSettings.imageSettings.top, "Top", andorGroup);
		log.writeDataSet (expRunSettings.imageSettings.bottom, "Bottom", andorGroup);
		log.writeDataSet (expRunSettings.imageSettings.left, "Left", andorGroup);
		log.writeDataSet (expRunSettings.imageSettings.right, "Right", andorGroup);
		log.writeDataSet (expRunSettings.imageSettings.horizontalBinning, "Horizontal-Binning", andorGroup);
		log.writeDataSet (expRunSettings.imageSettings.verticalBinning, "Vertical-Binning", andorGroup);
		log.writeDataSet (expRunSettings.temperatureSetting, "Temperature-Setting", andorGroup);
		log.writeDataSet (expRunSettings.picsPerRepetition, "Pictures-Per-Repetition", andorGroup);
		log.writeDataSet (expRunSettings.repetitionsPerVariation, "Repetitions-Per-Variation", andorGroup);
		log.writeDataSet (expRunSettings.totalVariations, "Total-Variation-Number", andorGroup);
	}
	catch (H5::Exception err){
		if (threadworker != nullptr) {
			notify({ qstr("Failed to log andor info!!\n"), 0 }, threadworker);
		}
		log.logError (err);
		throwNested ("ERROR: Failed to log andor parameters in HDF5 file: " + err.getDetailMsg ());
	}
}

void AndorCameraCore::loadExpSettings (ConfigStream& stream){
	ConfigSystem::stdGetFromConfig (stream, *this, expRunSettings);
	expRunSettings.repetitionsPerVariation = ConfigSystem::stdConfigGetter (stream, "REPETITIONS", 
																			 Repetitions::getSettingsFromConfig);
	expAnalysisSettings = ConfigSystem::stdConfigGetter (stream, "DATA_ANALYSIS", 
														  DataAnalysisControl::getAnalysisSettingsFromFile); 
	experimentActive = expRunSettings.controlCamera;
}

void AndorCameraCore::calculateVariations (std::vector<parameterType>& params, ExpThreadWorker* threadworker){
	expRunSettings.totalVariations = (params.size () == 0 ? 1 : params.front ().keyValues.size ());;
	if (experimentActive){
		setSettings (expRunSettings);
		emit threadworker->prepareAndor (&expRunSettings, expAnalysisSettings);
	}
}

void AndorCameraCore::programVariation (unsigned variationInc, std::vector<parameterType>& params, ExpThreadWorker* threadworker){
	if (experimentActive){
		double kinTime;
		armCamera (kinTime, threadworker);
	}
}

void AndorCameraCore::normalFinish (){
}

void AndorCameraCore::errorFinish (){
	try	{
		abortAcquisition ();
	}
	catch (ChimeraError &) { /*Probably just idle.*/ }
}
