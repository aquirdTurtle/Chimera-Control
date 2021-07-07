#include "stdafx.h"
#include "QtAndorWindow.h"
#include "Agilent/AgilentSettings.h"
#include <qdesktopwidget.h>
#include <PrimaryWindows/QtScriptWindow.h>
#include <PrimaryWindows/QtAndorWindow.h>
#include <PrimaryWindows/QtAuxiliaryWindow.h>
#include <PrimaryWindows/QtMainWindow.h>
#include <PrimaryWindows/QtBaslerWindow.h>
#include <PrimaryWindows/QtDeformableMirrorWindow.h>
#include <RealTimeDataAnalysis/AnalysisThreadWorker.h>
#include <RealTimeDataAnalysis/AtomCruncherWorker.h>
#include <ExperimentThread/ExpThreadWorker.h>
#include <QThread.h>
#include <qdebug.h>


QtAndorWindow::QtAndorWindow (QWidget* parent) : IChimeraQtWindow (parent),
	andorSettingsCtrl (),
	dataHandler (DATA_SAVE_LOCATION, REMOTE_DATA_SAVE_LOCATION, this),
	andor (ANDOR_SAFEMODE),
	pics (false, "ANDOR_PICTURE_MANAGER", false, Qt::SmoothTransformation),
	imagingPiezo (this, IMG_PIEZO_INFO),
	analysisHandler (this)
{
	statBox = new ColorBox ();
	setWindowTitle ("Andor Window");
}

QtAndorWindow::~QtAndorWindow (){}

int QtAndorWindow::getDataCalNum () {
	return dataHandler.getCalibrationFileIndex ();
}

void QtAndorWindow::initializeWidgets (){
	andor.initializeClass (this, &imageTimes);
	QPoint position = { 0,25 };
	statBox->initialize (position, this, 480, mainWin->getDevices (), 2);
	alerts.alertMainThread (0);
	alerts.initialize (position, this);
	analysisHandler.initialize (position, this);
	andorSettingsCtrl.initialize (position, this, andor.getVertShiftSpeeds(), andor.getHorShiftSpeeds());
	imagingPiezo.initialize (position, this, 480, { "Horizontal Pzt.", "Disconnected", "Vertical Pzt." });
	position = { 480, 25 };
	stats.initialize (position, this);
	for (auto pltInc : range (6)){
		mainAnalysisPlots.push_back (new QCustomPlotCtrl(1, plotStyle::BinomialDataPlot, { 0,0,0,0 }, false, false));
		mainAnalysisPlots.back ()->init (position, 315, 130, this, "INACTIVE");
	}
	position = { 797, 25 };
	timer.initialize (position, this);
	position = { 797, 65 };
	pics.initialize (position, 540*2, 440*2 + 5, this);
	// end of literal initialization calls
	pics.setSinglePicture (andorSettingsCtrl.getConfigSettings ().andor.imageSettings);
	andor.setSettings (andorSettingsCtrl.getConfigSettings ().andor);

	QTimer* timer = new QTimer (this);
	connect (timer, &QTimer::timeout, [this]() {
		auto temp = andor.getTemperature ();
		andorSettingsCtrl.changeTemperatureDisplay (temp); 
		});
	timer->start (2000);
}

void QtAndorWindow::manualArmCamera () {
	try {
		double time;
		andor.armCamera (time);
	}
	catch (ChimeraError & err) {
		reportErr (qstr (err.trace ()));
	}
}

void QtAndorWindow::handlePrepareForAcq (AndorRunSettings* lparam, analysisSettings aSettings){
	try {
		reportStatus({ "Preparing Andor Window for Acquisition...\n", 0, "ANDOR_WINDOW" });
		AndorRunSettings* settings = (AndorRunSettings*)lparam;
		reportStatus({ settings->stringSummary(), 2, "ANDOR_WINDOW" });
		analysisHandler.setRunningSettings (aSettings);
		armCameraWindow (settings);
		completeCruncherStart ();
		completePlotterStart ();
	}
	catch (ChimeraError & err) {
		reportErr (qstr (err.trace ()));
	}
}

void QtAndorWindow::handlePlotPop (unsigned id){
	for (auto& plt : mainAnalysisPlots)	{
	}
}

bool QtAndorWindow::wasJustCalibrated (){
	return justCalibrated;
}

bool QtAndorWindow::wantsAutoCal (){
	return andorSettingsCtrl.getAutoCal ();
}

void QtAndorWindow::writeVolts (unsigned currentVoltNumber, std::vector<float64> data){
	try	{
		dataHandler.writeVolts (currentVoltNumber, data);
	}
	catch (ChimeraError& err){
		reportErr (qstr (err.trace ()));
	}
}

void QtAndorWindow::handleImageDimsEdit (){
	try {
		pics.setParameters (andorSettingsCtrl.getConfigSettings ().andor.imageSettings);
		QPainter painter (this);
		pics.redrawPictures (selectedPixel, analysisHandler.getRunningSettings().grids, true, mostRecentPicNum, painter); 
	}
	catch (ChimeraError& err){
		reportErr (qstr (err.trace ()));
	}
}

void QtAndorWindow::handleEmGainChange (){
	try {
		auto runSettings = andor.getAndorRunSettings ();
		andorSettingsCtrl.setEmGain (runSettings.emGainModeIsOn, runSettings.emGainLevel);
		auto settings = andorSettingsCtrl.getConfigSettings ();
		runSettings.emGainModeIsOn = settings.andor.emGainModeIsOn;
		runSettings.emGainLevel = settings.andor.emGainLevel;
		andor.setSettings (runSettings);
		// and immediately change the EM gain mode.
		try	{
			andor.setGainMode (nullptr);
		}
		catch (ChimeraError& err){
			// this can happen e.g. if the camera is aquiring.
			reportErr (qstr (err.trace ()));
		}
	}
	catch (ChimeraError err){
		reportErr (qstr (err.trace ()));
	}
}


std::string QtAndorWindow::getSystemStatusString (){
	std::string statusStr;
	statusStr = "\nAndor Camera:\n";
	if (!ANDOR_SAFEMODE){
		statusStr += "\tCode System is Active!\n";
		statusStr += "\t" + andor.getSystemInfo ();
	}
	else{
		statusStr += "\tCode System is disabled! Enable in \"constants.h\"\n";
	}
	return statusStr;
}

void QtAndorWindow::windowSaveConfig (ConfigStream& saveFile){
	andorSettingsCtrl.handleSaveConfig (saveFile);
	pics.handleSaveConfig (saveFile);
	analysisHandler.handleSaveConfig (saveFile);
	imagingPiezo.handleSaveConfig (saveFile);
}

void QtAndorWindow::windowOpenConfig (ConfigStream& configFile){
	AndorRunSettings camSettings;
	try	{
		ConfigSystem::stdGetFromConfig (configFile, andor, camSettings);
		andorSettingsCtrl.setConfigSettings (camSettings);
		andorSettingsCtrl.updateImageDimSettings (camSettings.imageSettings);
		andorSettingsCtrl.updateRunSettingsFromPicSettings ();
	}
	catch (ChimeraError& err){
		reportErr (qstr("Failed to get Andor Camera Run settings from file! " + err.trace ()));
	}
	try	{
		auto picSettings = ConfigSystem::stdConfigGetter (configFile, "PICTURE_SETTINGS",
			AndorCameraSettingsControl::getPictureSettingsFromConfig);
		andorSettingsCtrl.updatePicSettings (picSettings);
	}
	catch (ChimeraError& err)	{
		reportErr (qstr ("Failed to get Andor Camera Picture settings from file! " + err.trace ()));
	}
	try	{
		ConfigSystem::standardOpenConfig (configFile, pics.configDelim, &pics, Version ("4.0"));
	}
	catch (ChimeraError&)	{
		reportErr ("Failed to load picture settings from config!");
	}
	try	{
		ConfigSystem::standardOpenConfig (configFile, "DATA_ANALYSIS", &analysisHandler, Version ("4.0"));
	}
	catch (ChimeraError&){
		reportErr ("Failed to load Data Analysis settings from config!");
	}
	try	{
		pics.resetPictureStorage ();
		std::array<int, 4> nums = andorSettingsCtrl.getConfigSettings ().palleteNumbers;
		pics.setPalletes (nums);
	}
	catch (ChimeraError& e){
		reportErr (qstr ("Andor Camera Window failed to read parameters from the configuration file.\n\n" + e.trace ()));
	}
	ConfigSystem::standardOpenConfig (configFile, imagingPiezo.getConfigDelim (), &imagingPiezo, Version ("5.3"));
	analysisHandler.updateUnofficialPicsPerRep (andorSettingsCtrl.getConfigSettings ().andor.picsPerRepetition);
}

void QtAndorWindow::passAlwaysShowGrid (){
	if (alwaysShowGrid)	{
		alwaysShowGrid = false;
	}
	else{
		alwaysShowGrid = true;
	}
	pics.setSpecialGreaterThanMax (specialGreaterThanMax);
}

void QtAndorWindow::abortCameraRun (){
	int status = andor.queryStatus ();
	if (ANDOR_SAFEMODE)	{
		// simulate as if you needed to abort.
		status = DRV_ACQUIRING;
	}
	if (status == DRV_ACQUIRING){
		andor.abortAcquisition ();
		timer.setTimerDisplay ("Aborted");
		andor.setIsRunningState (false);
		// close the plotting thread.
		plotThreadAborting = true;
		plotThreadActive = false;
		atomCrunchThreadActive = false;
		// Wait until plotting thread is complete.
		while (true){
			auto res = WaitForSingleObject (plotThreadHandle, 2e3);
			if (res == WAIT_TIMEOUT){
				auto answer = QMessageBox::question (this, qstr ("Close Real-Time Plotting?"), 
					"The real time plotting thread is taking a while to close. Continue waiting?");
				if (answer == QMessageBox::No) {
					// This might indicate something about the code is gonna crash...
					break;
				}
			}
			else{
				break;
			}
		}
		plotThreadAborting = false;
		// camera is no longer running.
		try	{
			dataHandler.normalCloseFile ();
		}
		catch (ChimeraError& err)	{
			reportErr (qstr (err.trace ()));
		}

		if (andor.getAndorRunSettings ().acquisitionMode != AndorRunModes::mode::Video){
			auto answer = QMessageBox::question(this, qstr("Delete Data?"), qstr("Acquisition Aborted. Delete Data "
				"file (data_" + str (dataHandler.getDataFileNumber ()) + ".h5) for this run?"));
			if (answer == QMessageBox::Yes){
				try	{
					dataHandler.deleteFile ();
				}
				catch (ChimeraError& err) {
					reportErr (qstr (err.trace ()));
				}
			}
		}
	}
	else if (status == DRV_IDLE) {
		andor.setIsRunningState (false);
	}
}

bool QtAndorWindow::cameraIsRunning (){
	return andor.isRunning ();
}

void QtAndorWindow::onCameraProgress (int picNumReported){
	currentPictureNum++;
	unsigned picNum = currentPictureNum;
	if (picNum % 2 == 1){
		mainThreadStartTimes.push_back (std::chrono::high_resolution_clock::now ());
	}
	AndorRunSettings curSettings = andor.getAndorRunSettings ();
	if (picNumReported == -1){
		// last picture.
		picNum = curSettings.totalPicsInExperiment();
	}
	if (picNumReported != currentPictureNum && picNumReported != -1){
		if (curSettings.acquisitionMode != AndorRunModes::mode::Video){
			//reportErr ( "WARNING: picture number reported by andor isn't matching the"
			//								  "camera window record?!?!?!?!?" );
		}
	}
	// need to call this before acquireImageData().
	andor.updatePictureNumber (picNum);
	std::vector<Matrix<long>> rawPicData;
	try	{
		rawPicData = andor.acquireImageData ();
	}
	catch (ChimeraError& err){
		reportErr (qstr (err.trace ()));
		mainWin->pauseExperiment ();
		return;
	}
	std::vector<Matrix<long>> calPicData (rawPicData.size ());
	if (andorSettingsCtrl.getUseCal () && avgBackground.size () == rawPicData.front ().size ()){
		for (auto picInc : range (rawPicData.size ())){
			calPicData[picInc] = Matrix<long> (rawPicData[picInc].getRows (), rawPicData[picInc].getCols (), 0);
			for (auto pixInc : range (rawPicData[picInc].size ())){
				calPicData[picInc].data[pixInc] = (rawPicData[picInc].data[pixInc] - avgBackground.data[pixInc]);
			}
		}
	}
	else { calPicData = rawPicData; }

	if (picNum % 2 == 1){
		imageGrabTimes.push_back (std::chrono::high_resolution_clock::now ());
	}
	emit newImage ({ picNum, calPicData[(picNum - 1) % curSettings.picsPerRepetition] }); 

	auto picsToDraw = andorSettingsCtrl.getImagesToDraw (calPicData);
	try	{
		if (realTimePic){
			std::pair<int, int> minMax;
			// draw the most recent pic.
			minMax = stats.update (pics.getAccumPicData(picNum % curSettings.picsPerRepetition), 
				picNum % curSettings.picsPerRepetition, selectedPixel,
				picNum / curSettings.picsPerRepetition,
				curSettings.totalPicsInExperiment () / curSettings.picsPerRepetition);
			QPainter painter (this);
			pics.drawBitmap (picsToDraw.back (), minMax, picNum % curSettings.picsPerRepetition,
				analysisHandler.getRunningSettings ().grids, picNum, 
				analysisHandler.getRunningSettings ().displayGridOption, painter);

			timer.update (picNum / curSettings.picsPerRepetition, curSettings.repetitionsPerVariation,
				curSettings.totalVariations, curSettings.picsPerRepetition);
		}
		else if (picNum % curSettings.picsPerRepetition == 0) {
			int counter = 0;
			for (auto data : picsToDraw) { 
				std::pair<int, int> minMax;
				if (pics.getSoftwareAccumulationOpt(counter).accumAll 
					|| pics.getSoftwareAccumulationOpt(counter).accumNum > 1) {
					minMax = stats.update(pics.getAccumPicData(counter), counter,
						selectedPixel, picNum / curSettings.picsPerRepetition,
						curSettings.totalPicsInExperiment() / curSettings.picsPerRepetition);
				}
				else {
					minMax = stats.update(data, counter, selectedPixel, picNum / curSettings.picsPerRepetition,
										  curSettings.totalPicsInExperiment() / curSettings.picsPerRepetition);
				}
				if (minMax.second > 50000){
					numExcessCounts++;
					if (numExcessCounts > 2){
						// POTENTIALLY DANGEROUS TO CAMERA.
						// AUTO PAUSE THE EXPERIMENT. 
						// This can happen if a laser, particularly the axial raman laser, is left on during an image.
						// cosmic rays may occasionally trip it as well, although this should be rare as it requires 
						// two consequtive images.
						reportErr ("EXCCESSIVE CAMERA COUNTS DETECTED!");
						commonFunctions::handleCommonMessage (ID_ACCELERATOR_F2, this);
						errBox ("EXCCESSIVE CAMERA COUNTS DETECTED!!!");
					}
				}
				else {
					numExcessCounts = 0;
				}
				QPainter painter (this);
				pics.drawBitmap ( data, minMax, counter, analysisHandler.getRunningSettings ().grids, picNum+counter,
								  analysisHandler.getRunningSettings ().displayGridOption, painter );
				pics.updatePlotData();
				counter++;
			}
			timer.update (picNum / curSettings.picsPerRepetition, curSettings.repetitionsPerVariation,
				curSettings.totalVariations, curSettings.picsPerRepetition);
		}
	}
	catch (ChimeraError& err){
		reportErr (qstr (err.trace ()));
		try {
			mainWin->pauseExperiment ();
		}
		catch (ChimeraError & err) {
			reportErr (qstr (err.trace ()));
		}
	}
	// write the data to the file.
	if (curSettings.acquisitionMode != AndorRunModes::mode::Video){
		try	{
			// important! write the original raw data, not the pic-to-draw, which can be a difference pic, or the calibrated
			// pictures, which can have the background subtracted.
			dataHandler.writeAndorPic ( rawPicData[(picNum - 1) % curSettings.picsPerRepetition],
									    curSettings.imageSettings );
		}
		catch (ChimeraError& err){
			reportErr (err.qtrace ());
			try {
				mainWin->pauseExperiment ();
			}
			catch (ChimeraError & err2) {
				reportErr (err2.qtrace ());
			}
		}
	}
	mostRecentPicNum = picNum;
}

void QtAndorWindow::handleSetAnalysisPress (){
	analysisHandler.saveGridParams ();
}

void QtAndorWindow::wakeRearranger (){
	std::unique_lock<std::mutex> lock (rearrangerLock);
	rearrangerConditionVariable.notify_all ();
}

void QtAndorWindow::handleSpecialLessThanMinSelection (){
	if (specialLessThanMin)	{
		specialLessThanMin = false;
		//mainWin->checkAllMenus (ID_PICTURES_LESS_THAN_MIN_SPECIAL, MF_UNCHECKED);
	}
	else{
		specialLessThanMin = true;
		//mainWin->checkAllMenus (ID_PICTURES_LESS_THAN_MIN_SPECIAL, MF_CHECKED);
	}
	pics.setSpecialLessThanMin (specialLessThanMin);
}

void QtAndorWindow::handleSpecialGreaterThanMaxSelection (){
	if (specialGreaterThanMax){
		specialGreaterThanMax = false;
		//mainWin->checkAllMenus (ID_PICTURES_GREATER_THAN_MAX_SPECIAL, MF_UNCHECKED);
	}
	else{
		specialGreaterThanMax = true;
		//mainWin->checkAllMenus (ID_PICTURES_GREATER_THAN_MAX_SPECIAL, MF_CHECKED);
	}
	pics.setSpecialGreaterThanMax (specialGreaterThanMax);
}

LRESULT QtAndorWindow::onCameraCalFinish (WPARAM wParam, LPARAM lParam){
	// notify the andor object that it is done.
	andor.onFinish ();
	andor.pauseThread ();
	andor.setCalibrating (false);
	justCalibrated = true;
	andorSettingsCtrl.cameraIsOn (false);
	// normalize.
	for (auto& pix : avgBackground){
		pix /= 100.0;
	}
	// if auto cal is selected, always assume that the user was trying to start with F5.
	if (andorSettingsCtrl.getAutoCal ()){
		//PostMessageA (WM_COMMAND, MAKEWPARAM (ID_ACCELERATOR_F5, 0));
	}
	return 0;
}

dataPoint QtAndorWindow::getMainAnalysisResult (){
	return mostRecentAnalysisResult;
}

void QtAndorWindow::cleanUpAfterExp (){
	try {
		plotThreadActive = false;
		atomCrunchThreadActive = false;
		dataHandler.normalCloseFile ();
	}
	catch (ChimeraError & err) {
		reportErr ("Andor Window Failed to clean up after exp! Error: " + err.qtrace ());
	}
}

int QtAndorWindow::getMostRecentFid (){
	return dataHandler.getDataFileNumber ();
}

int QtAndorWindow::getPicsPerRep (){
	return andorSettingsCtrl.getConfigSettings ().andor.picsPerRepetition;
}

std::string QtAndorWindow::getMostRecentDateString (){
	return dataHandler.getMostRecentDateString ();
}

bool QtAndorWindow::wantsThresholdAnalysis (){
	return analysisHandler.getRunningSettings ().autoThresholdAnalysisOption;
}

atomGrid QtAndorWindow::getMainAtomGrid (){
	return analysisHandler.getRunningSettings ().grids[0];
}


void QtAndorWindow::armCameraWindow (AndorRunSettings* settings){
	pics.setNumberPicturesActive (settings->picsPerRepetition);
	if (settings->picsPerRepetition == 1){
		pics.setSinglePicture (settings->imageSettings);
	}
	else{
		pics.setMultiplePictures (settings->imageSettings, settings->picsPerRepetition);
	}
	pics.resetPictureStorage ();
	pics.setParameters (settings->imageSettings);
	redrawPictures (false);
	andorSettingsCtrl.setRunSettings (*settings);
	andorSettingsCtrl.setRepsPerVariation (settings->repetitionsPerVariation);
	andorSettingsCtrl.setVariationNumber (settings->totalVariations);
	pics.setSoftwareAccumulationOptions (andorSettingsCtrl.getSoftwareAccumulationOptions ());
	try {
		andor.preparationChecks ();
	}
	catch (ChimeraError & err) {
		reportErr (err.qtrace ());
	}
	// turn some buttons off.
	andorSettingsCtrl.cameraIsOn (true);
	stats.reset ();
	analysisHandler.updateDataSetNumberEdit (dataHandler.getNextFileNumber () - 1);
}

bool QtAndorWindow::getCameraStatus (){
	return andor.isRunning ();
}

void QtAndorWindow::stopSound (){
	alerts.stopSound ();
}

void QtAndorWindow::passSetTemperaturePress (){
	try{
		if (andor.isRunning ()){
			thrower ("ERROR: the camera (thinks that it?) is running. You can't change temperature settings during camera "
				"operation.");
		}
		andorSettingsCtrl.handleSetTemperaturePress ();
		auto settings = andorSettingsCtrl.getConfigSettings ();
		andor.setSettings (settings.andor);
		andor.setTemperature ();
	}
	catch (ChimeraError& err){
		reportErr (qstr (err.trace ()));
	}
	mainWin->updateConfigurationSavedStatus (false);
}

void QtAndorWindow::assertDataFileClosed () {
	dataHandler.assertClosed ();
}

void QtAndorWindow::handlePictureSettings (){
	selectedPixel = { 0,0 };
	andorSettingsCtrl.handlePictureSettings ();
	auto configSettings = andorSettingsCtrl.getConfigSettings();
	if (configSettings.andor.picsPerRepetition == 1){
		pics.setSinglePicture (configSettings.andor.imageSettings);
	}
	else{
		pics.setMultiplePictures (configSettings.andor.imageSettings, configSettings.andor.picsPerRepetition );
	}
	pics.resetPictureStorage ();
	std::array<int, 4> nums = configSettings.palleteNumbers;
	pics.setPalletes (nums);
	analysisHandler.updateUnofficialPicsPerRep (configSettings.andor.picsPerRepetition);
	pics.setScaleFactor (configSettings.picScaleFactor);
	pics.setSoftwareAccumulationOptions(andorSettingsCtrl.getSoftwareAccumulationOptions());
}

/* Check that the camera is idle, or not aquiring pictures. Also checks that the data analysis handler isn't active.
*/
void QtAndorWindow::checkCameraIdle (){
	if (andor.isRunning ()){
		thrower ("Camera is already running! Please Abort to restart.\r\n");
	}
	// make sure it's idle.
	try{
		andor.queryStatus ();
		if (ANDOR_SAFEMODE){
			thrower ("DRV_IDLE");
		}
	}
	catch (ChimeraError& exception){
		if (exception.whatBare () != "DRV_IDLE"){
			throwNested (" while querying andor status to check if idle.");
		}
	}
}

void QtAndorWindow::handleMasterConfigSave (std::stringstream& configStream){
	andorSettingsCtrl.handelSaveMasterConfig (configStream);
}

void QtAndorWindow::handleMasterConfigOpen (ConfigStream& configStream){
	mainWin->updateConfigurationSavedStatus (false);
	selectedPixel = { 0,0 };
	andorSettingsCtrl.handleOpenMasterConfig (configStream, this);
	pics.setParameters (andorSettingsCtrl.getConfigSettings ().andor.imageSettings);
	redrawPictures (true);
}

DataLogger& QtAndorWindow::getLogger (){
	return dataHandler;
}

void QtAndorWindow::loadCameraCalSettings (AllExperimentInput& input){
	redrawPictures (false);
	try{
		checkCameraIdle ();
	}
	catch (ChimeraError& err){
		reportErr (qstr (err.trace ()));
	}
	// I used to mandate use of a button to change image parameters. Now I don't have the button and just always 
	// update at this point.
	readImageParameters ();
	pics.setNumberPicturesActive (1);
	// biggest check here, camera settings includes a lot of things.
	andorSettingsCtrl.checkIfReady ();
	// reset the image which is about to be calibrated.
	avgBackground = Matrix<long> (0, 0);
	/// start the camera.
	andor.setCalibrating (true);
}

AndorCameraCore& QtAndorWindow::getCamera (){
	return andor;
}


void QtAndorWindow::prepareAtomCruncher (AllExperimentInput& input){
	input.cruncherInput = new atomCruncherInput;
	input.cruncherInput->plotterActive = plotThreadActive;
	input.cruncherInput->imageDims = andorSettingsCtrl.getRunningSettings().imageSettings;
	atomCrunchThreadActive = true;
	input.cruncherInput->plotterNeedsImages = input.masterInput->plotterInput->needsCounts;
	input.cruncherInput->cruncherThreadActive = &atomCrunchThreadActive;
	skipNext = false;
	input.cruncherInput->skipNext = &skipNext;
	//input.cruncherInput->imQueue = &imQueue;
	// options
	if (input.masterInput){
		auto& niawg = input.masterInput->devices.getSingleDevice< NiawgCore > ();
		input.cruncherInput->rearrangerActive = niawg.expRerngOptions.active;
	}
	else{
		input.cruncherInput->rearrangerActive = false;
	}
	input.cruncherInput->grids = analysisHandler.getRunningSettings ().grids;
	input.cruncherInput->thresholds = andorSettingsCtrl.getConfigSettings ().thresholds;
	input.cruncherInput->picsPerRep = andorSettingsCtrl.getRunningSettings ().picsPerRepetition;
	input.cruncherInput->catchPicTime = &crunchSeesTimes;
	input.cruncherInput->finTime = &crunchFinTimes;
	input.cruncherInput->atomThresholdForSkip = mainWin->getMainOptions ().atomSkipThreshold;
	input.cruncherInput->rearrangerConditionWatcher = &rearrangerConditionVariable;
}

bool QtAndorWindow::wantsAutoPause (){
	return alerts.wantsAutoPause ();
}

void QtAndorWindow::completeCruncherStart () {
	auto* cruncherInput = new atomCruncherInput;
	cruncherInput->plotterActive = plotThreadActive;
	cruncherInput->imageDims = andorSettingsCtrl.getRunningSettings ().imageSettings;
	atomCrunchThreadActive = true;
	cruncherInput->plotterNeedsImages = true;// input.masterInput->plotterInput->needsCounts;
	cruncherInput->cruncherThreadActive = &atomCrunchThreadActive;
	skipNext = false;
	cruncherInput->skipNext = &skipNext;
	cruncherInput->rearrangerActive = false;
	cruncherInput->grids = analysisHandler.getRunningSettings ().grids;
	cruncherInput->thresholds = andorSettingsCtrl.getConfigSettings ().thresholds;
	cruncherInput->picsPerRep = andorSettingsCtrl.getRunningSettings ().picsPerRepetition;
	cruncherInput->catchPicTime = &crunchSeesTimes;
	cruncherInput->finTime = &crunchFinTimes;
	cruncherInput->atomThresholdForSkip = mainWin->getMainOptions ().atomSkipThreshold;
	cruncherInput->rearrangerConditionWatcher = &rearrangerConditionVariable;

	atomCruncherWorker = new CruncherThreadWorker (cruncherInput);
	QThread* thread = new QThread;
	atomCruncherWorker->moveToThread (thread);


	connect (mainWin->getExpThread(), &QThread::finished, atomCruncherWorker, &QObject::deleteLater);

	connect (thread, &QThread::started, atomCruncherWorker, &CruncherThreadWorker::init);
	connect (thread, &QThread::finished, thread, &CruncherThreadWorker::deleteLater);
	connect (this, &QtAndorWindow::newImage, atomCruncherWorker, &CruncherThreadWorker::handleImage);
	thread->start ();
}

void QtAndorWindow::completePlotterStart () {
	/// start the plotting thread.
	plotThreadActive = true;
	plotThreadAborting = false;
	auto* pltInput = new realTimePlotterInput (analysisHandler.getPlotTime ());
	pltInput->plotParentWindow = this;
	pltInput->aborting = &plotThreadAborting;
	pltInput->active = &plotThreadActive;

	
	auto camSettings = andorSettingsCtrl.getRunningSettings ();
	pltInput->variations = camSettings.totalVariations;
	pltInput->picsPerVariation = camSettings.totalPicsInVariation();

	pltInput->imageShape = camSettings.imageSettings;
	pltInput->picsPerRep = camSettings.picsPerRepetition;
	
	pltInput->alertThreshold = alerts.getAlertThreshold ();
	pltInput->wantAtomAlerts = alerts.wantsAtomAlerts ();
	pltInput->numberOfRunsToAverage = 5;
	pltInput->plottingFrequency = analysisHandler.getPlotFreq ();
	analysisHandler.fillPlotThreadInput (pltInput);
	// remove old plots that aren't trying to sustain.
	unsigned mainPlotInc = 0;
	for (auto plotParams : pltInput->plotInfo) {
		plotStyle style = plotParams.isHist ? plotStyle::HistPlot : plotStyle::BinomialDataPlot;
		if (mainPlotInc < 6) {
			mainAnalysisPlots[mainPlotInc]->setStyle (style);
			mainAnalysisPlots[mainPlotInc]->setThresholds (andorSettingsCtrl.getConfigSettings ().thresholds[0]);
			mainAnalysisPlots[mainPlotInc]->setTitle (plotParams.name);
			mainPlotInc++;
		}
	}

	bool gridHasBeenSet = false;
	for (auto gridInfo : pltInput->grids) {
		if (!(gridInfo.topLeftCorner == coordinate (0, 0))) {
			gridHasBeenSet = true;
			break;
		}
	}
	if ((!gridHasBeenSet) || pltInput->plotInfo.size () == 0) {
		plotThreadActive = false;
	}
	else {
		// start the plotting thread
		plotThreadActive = true;
		analysisThreadWorker = new AnalysisThreadWorker (pltInput);
		QThread* thread = new QThread;
		analysisThreadWorker->moveToThread (thread);
		connect (thread, &QThread::started, analysisThreadWorker, &AnalysisThreadWorker::init);
		connect (thread, &QThread::finished, thread, &QThread::deleteLater);
		connect (thread, &QThread::finished, analysisThreadWorker, &AnalysisThreadWorker::deleteLater);
		
		connect (mainWin->getExpThreadWorker(), &ExpThreadWorker::plot_Xvals_determined,
				 analysisThreadWorker, &AnalysisThreadWorker::setXpts);
		connect (mainWin->getExpThread(), &QThread::finished, analysisThreadWorker, &QObject::deleteLater);

		connect (analysisThreadWorker, &AnalysisThreadWorker::newPlotData, this,
			[this](std::vector<std::vector<dataPoint>> data, int plotNum) {mainAnalysisPlots[plotNum]->setData (data); });
		if (atomCruncherWorker) {
			connect (atomCruncherWorker, &CruncherThreadWorker::atomArray,
				analysisThreadWorker, &AnalysisThreadWorker::handleNewPic);
			connect (atomCruncherWorker, &CruncherThreadWorker::pixArray,
				analysisThreadWorker, &AnalysisThreadWorker::handleNewPix);
		}
		thread->start ();
	}
}

bool QtAndorWindow::wantsNoMotAlert (){
	if (cameraIsRunning ()){
		return alerts.wantsMotAlerts ();
	}
	else{
		return false;
	}
}

unsigned QtAndorWindow::getNoMotThreshold (){
	return alerts.getAlertThreshold ();
}

std::string QtAndorWindow::getStartMessage (){
	// get selected plots
	auto andrSttngs = andorSettingsCtrl.getConfigSettings ().andor;
	std::vector<std::string> plots = analysisHandler.getActivePlotList ();
	imageParameters currentImageParameters = andrSttngs.imageSettings;
	bool errCheck = false;
	for (unsigned plotInc = 0; plotInc < plots.size (); plotInc++){
		PlottingInfo tempInfoCheck (PLOT_FILES_SAVE_LOCATION + "\\" + plots[plotInc] + ".plot");
		if (tempInfoCheck.getPicNumber () != andrSttngs.picsPerRepetition){
			thrower (": one of the plots selected, " + plots[plotInc] + ", is not built for the currently "
					 "selected number of pictures per experiment. (" + str(andrSttngs.picsPerRepetition) 
					 + ") Please revise either the current setting or the plot file.");
		}
	}
	std::string dialogMsg;
	dialogMsg = "Camera Parameters:\r\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\r\n";
	dialogMsg += "Current Camera Temperature Setting:\r\n\t" + str (
		andrSttngs.temperatureSetting) + "\r\n";
	dialogMsg += "Exposure Times: ";
	for (auto& time : andrSttngs.exposureTimes){
		dialogMsg += str (time * 1000) + ", ";
	}
	dialogMsg += "\r\n";
	dialogMsg += "Image Settings:\r\n\t" + str (currentImageParameters.left) + " - " + str (currentImageParameters.right) + ", "
		+ str (currentImageParameters.bottom) + " - " + str (currentImageParameters.top) + "\r\n";
	dialogMsg += "\r\n";
	dialogMsg += "Kintetic Cycle Time:\r\n\t" + str (andrSttngs.kineticCycleTime) + "\r\n";
	dialogMsg += "Pictures per Repetition:\r\n\t" + str (andrSttngs.picsPerRepetition) + "\r\n";
	dialogMsg += "Repetitions per Variation:\r\n\t" + str (andrSttngs.totalPicsInVariation ()) + "\r\n";
	dialogMsg += "Variations per Experiment:\r\n\t" + str (andrSttngs.totalVariations) + "\r\n";
	dialogMsg += "Total Pictures per Experiment:\r\n\t" + str (andrSttngs.totalPicsInExperiment ()) + "\r\n";

	dialogMsg += "Real-Time Atom Detection Thresholds:\r\n\t";
	unsigned count = 0;
	for (auto& picThresholds : andorSettingsCtrl.getConfigSettings ().thresholds){
		dialogMsg += "Pic " + str (count) + " thresholds: ";
		for (auto thresh : picThresholds){
			dialogMsg += str (thresh) + ", ";
		}
		dialogMsg += "\r\n";
		count++;
	}
	dialogMsg += "\r\nReal-Time Plots:\r\n";
	for (unsigned plotInc = 0; plotInc < plots.size (); plotInc++){
		dialogMsg += "\t" + plots[plotInc] + "\r\n";
	}
	return dialogMsg;
}

void QtAndorWindow::fillMasterThreadInput (ExperimentThreadInput* input){
	currentPictureNum = 0;
	// starting a not-calibration, so reset this.
	justCalibrated = false;
	input->rearrangerLock = &rearrangerLock;
	input->andorsImageTimes = &imageTimes;
	input->grabTimes = &imageGrabTimes;
	input->conditionVariableForRerng = &rearrangerConditionVariable;
}

void QtAndorWindow::setTimerText (std::string timerText){
	timer.setTimerDisplay (timerText);
}

void QtAndorWindow::setDataType (std::string dataType){
	stats.updateType (dataType);
}

void QtAndorWindow::redrawPictures (bool andGrid){
	try	{
		if (andGrid){
			QPainter painter (this);
			pics.drawGrids (painter);
		}
		// ??? should there be handling here???
	}
	catch (ChimeraError& err){
		reportErr (err.qtrace ());
	}
	// currently don't attempt to redraw previous picture data.
}

std::atomic<bool>* QtAndorWindow::getSkipNextAtomic (){
	return &skipNext;
}

void QtAndorWindow::stopPlotter (){
	plotThreadAborting = true;
}

// this is typically a little redundant to call, but can use to make sure things are set to off.
void QtAndorWindow::assertOff (){
	andorSettingsCtrl.cameraIsOn (false);
	plotThreadActive = false;
	atomCrunchThreadActive = false;
}

void QtAndorWindow::readImageParameters (){
	selectedPixel = { 0,0 };
	try	{
		redrawPictures (false);
		imageParameters parameters = andorSettingsCtrl.getConfigSettings ().andor.imageSettings;
		pics.setParameters (parameters);
	}
	catch (ChimeraError& exception){
		reportErr (exception.qtrace () + "\r\n");
	}
	QPainter painter (this);
	pics.drawGrids (painter);
}

void QtAndorWindow::fillExpDeviceList (DeviceList& list){
	list.list.push_back (andor);
}

piezoChan<double> QtAndorWindow::getAlignmentVals () {
	auto& core = imagingPiezo.getCore ();
	return { core.getCurrentXVolt (), core.getCurrentYVolt (), core.getCurrentZVolt () };
}

void QtAndorWindow::handleNormalFinish (profileSettings finishedProfile) {
	wakeRearranger ();
	cleanUpAfterExp ();
	handleBumpAnalysis (finishedProfile);
	assertOff();
}

void QtAndorWindow::copyDataFile(std::string specialName) {
	dataHandler.copyDataFile(specialName);
}

void QtAndorWindow::handleBumpAnalysis (profileSettings finishedProfile) {
	std::ifstream configFileRaw (finishedProfile.configFilePath ());
	// check if opened correctly.
	if (!configFileRaw.is_open ()) {
		errBox ("Opening of Configuration File for bump analysis Failed!");
		return;
	}
	ConfigStream cStream (configFileRaw);
	cStream.setCase (false);
	configFileRaw.close ();
	ConfigSystem::getVersionFromFile (cStream);
	ConfigSystem::jumpToDelimiter (cStream, "DATA_ANALYSIS");
	auto settings = analysisHandler.getAnalysisSettingsFromFile (cStream);
	// get the options from the config file, not from the current config settings. this is important especially for 
	// handling this in the calibration. 
	if (settings.autoBumpOption) {
		auto grid = andorWin->getMainAtomGrid ();
		auto dateStr = andorWin->getMostRecentDateString ();
		auto fid = andorWin->getMostRecentFid ();
		auto ppr = andorWin->getPicsPerRep ();
		try {
			auto res = pythonHandler.runCarrierAnalysis (dateStr, fid, grid, this);
			auto name =	settings.bumpParam;
			// zero is the default.
			if (name != "" && res != 0) {
				auxWin->getGlobals ().adjustVariableValue (str (name, 13, false, true), res);
			}
			reportStatus({ qstr("Successfully completed auto bump analysis and set variable \"" + name + "\" to value "
						   + str(res) + "\n"), 0, "ANDOR_WINDOW" });
		}
		catch (ChimeraError & err) {
			reportErr ("Bump Analysis Failed! " + err.qtrace ());
		}
	}
}

NewPythonHandler* QtAndorWindow::getPython() {
	return &pythonHandler;
}

void QtAndorWindow::handleTransformationModeChange () {
	auto mode = andorSettingsCtrl.getTransformationMode ();
	pics.setTransformationMode (mode);
}
