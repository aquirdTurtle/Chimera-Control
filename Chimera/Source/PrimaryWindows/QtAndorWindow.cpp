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

QtAndorWindow::QtAndorWindow (QWidget* parent) : IChimeraWindowWidget (parent),
andorSettingsCtrl (),
dataHandler (DATA_SAVE_LOCATION),
andor (ANDOR_SAFEMODE),
pics (false, "ANDOR_PICTURE_MANAGER", false){
	statBox = new ColorBox ();
	setWindowTitle ("Andor Window");
}

QtAndorWindow::~QtAndorWindow ()
{

}

void QtAndorWindow::initializeWidgets ()
{
	andor.initializeClass (mainWin->getComm (), &imageTimes);
	POINT position = { 0,25 };
	statBox->initialize (position, this, 480, mainWin->getDevices ());
	alerts.alertMainThread (0);
	alerts.initialize (position, this);
	analysisHandler.initialize (position, this);
	andorSettingsCtrl.initialize (position, this);
	position = { 480, 25 };
	stats.initialize (position, this);
	for (auto pltInc : range (6))
	{
		std::vector<pPlotDataVec> nodata (0);
		//mainAnalysisPlots.push_back (new PlotCtrl (nodata, plotStyle::ErrorPlot, mainWin->getPlotPens (),
		//	mainWin->getPlotFont (), mainWin->getPlotBrushes (), { 0,0,0,0 }, "INACTIVE", false, false));
		//mainAnalysisPlots.back ()->init (position, 315, 130, this);
	}
	position = { 797, 25 };
	timer.initialize (position, this);
	position = { 797, 65 };
	pics.initialize (position, _myBrushes["Dark Green"], 530 * 2, 460 * 2 + 5, this);
	// end of literal initialization calls
	//pics.setSinglePicture (this, andorSettingsCtrl.getSettings ().andor.imageSettings);
	andor.setSettings (andorSettingsCtrl.getSettings ().andor);
}

LRESULT QtAndorWindow::handlePrepareForAcq (WPARAM wparam, LPARAM lparam)
{
	ASSERT (InSendMessage ());
	mainWin->getComm ()->sendStatus ("Preparing Andor Window for Acquisition...\n");
	AndorRunSettings* settings = (AndorRunSettings*)lparam;
	armCameraWindow (settings);
	return 0;
}


void QtAndorWindow::handlePlotPop (UINT id)
{
	for (auto& plt : mainAnalysisPlots)
	{
	}
}

LRESULT QtAndorWindow::onBaslerFinish (WPARAM wParam, LPARAM lParam)
{
	if (!cameraIsRunning ())
	{
		dataHandler.closeFile ();
	}
	return 0;
}


bool QtAndorWindow::wasJustCalibrated ()
{
	return justCalibrated;
}


bool QtAndorWindow::wantsAutoCal ()
{
	return andorSettingsCtrl.getAutoCal ();
}

void QtAndorWindow::writeVolts (UINT currentVoltNumber, std::vector<float64> data)
{
	try
	{
		dataHandler.writeVolts (currentVoltNumber, data);
	}
	catch (Error& err)
	{
		reportErr (err.trace ());
	}
}


void QtAndorWindow::handleImageDimsEdit ()
{
	try {
		pics.setParameters (andorSettingsCtrl.getSettings ().andor.imageSettings);
		pics.redrawPictures (selectedPixel, analysisHandler.getAnalysisLocs (), analysisHandler.getGrids (), true,
			mostRecentPicNum);
	}
	catch (Error& err)
	{
		reportErr (err.trace ());
	}
}


void QtAndorWindow::handleEmGainChange ()
{
	try {
		auto runSettings = andor.getAndorRunSettings ();
		andorSettingsCtrl.setEmGain (runSettings.emGainModeIsOn, runSettings.emGainLevel);
		auto settings = andorSettingsCtrl.getSettings ();
		runSettings.emGainModeIsOn = settings.andor.emGainModeIsOn;
		runSettings.emGainLevel = settings.andor.emGainLevel;
		andor.setSettings (runSettings);
		// and immediately change the EM gain mode.
		try
		{
			andor.setGainMode ();
		}
		catch (Error& err)
		{
			// this can happen e.g. if the camera is aquiring.
			reportErr (err.trace ());
		}
	}
	catch (Error err)
	{
		reportErr (err.trace ());
	}
}


std::string QtAndorWindow::getSystemStatusString ()
{
	std::string statusStr;
	statusStr = "\nAndor Camera:\n";
	if (!ANDOR_SAFEMODE)
	{
		statusStr += "\tCode System is Active!\n";
		statusStr += "\t" + andor.getSystemInfo ();
	}
	else
	{
		statusStr += "\tCode System is disabled! Enable in \"constants.h\"\n";
	}
	return statusStr;
}

void QtAndorWindow::windowSaveConfig (ConfigStream& saveFile)
{
	andorSettingsCtrl.handleSaveConfig (saveFile);
	pics.handleSaveConfig (saveFile);
	analysisHandler.handleSaveConfig (saveFile);
}

void QtAndorWindow::windowOpenConfig (ConfigStream& configFile)
{
	AndorRunSettings camSettings;
	try
	{
		ProfileSystem::stdGetFromConfig (configFile, andor, camSettings);
		andorSettingsCtrl.setRunSettings (camSettings);
		andorSettingsCtrl.updateImageDimSettings (camSettings.imageSettings);
		andorSettingsCtrl.updateRunSettingsFromPicSettings ();

	}
	catch (Error& err)
	{
		reportErr ("Failed to get Andor Camera Run settings from file! " + err.trace ());
	}
	try
	{
		auto picSettings = ProfileSystem::stdConfigGetter (configFile, "PICTURE_SETTINGS",
			AndorCameraSettingsControl::getPictureSettingsFromConfig);
		andorSettingsCtrl.updatePicSettings (picSettings);
	}
	catch (Error& err)
	{
		reportErr ("Failed to get Andor Camera Picture settings from file! " + err.trace ());
	}
	try
	{
		ProfileSystem::standardOpenConfig (configFile, pics.configDelim, &pics, Version ("4.0"));
	}
	catch (Error&)
	{
		reportErr ("Failed to load picture settings from config!");
	}
	try
	{
		ProfileSystem::standardOpenConfig (configFile, "DATA_ANALYSIS", &analysisHandler, Version ("4.0"));
	}
	catch (Error&)
	{
		reportErr ("Failed to load Data Analysis settings from config!");
	}
	try
	{
		if (andorSettingsCtrl.getSettings ().andor.picsPerRepetition == 1)
		{
			//pics.setSinglePicture (this, andorSettingsCtrl.getSettings ().andor.imageSettings);
		}
		else
		{
			//pics.setMultiplePictures (this, andorSettingsCtrl.getSettings ().andor.imageSettings,
			//	andorSettingsCtrl.getSettings ().andor.picsPerRepetition);
		}
		pics.resetPictureStorage ();
		std::array<int, 4> nums = andorSettingsCtrl.getSettings ().palleteNumbers;
		pics.setPalletes (nums);
	}
	catch (Error& e)
	{
		reportErr ("Andor Camera Window failed to read parameters from the configuration file.\n\n" + e.trace ());
	}
}

void QtAndorWindow::passAlwaysShowGrid ()
{
	if (alwaysShowGrid)
	{
		alwaysShowGrid = false;
		//mainWin->checkAllMenus (ID_PICTURES_ALWAYSSHOWGRID, MF_UNCHECKED);
	}
	else
	{
		alwaysShowGrid = true;
		//mainWin->checkAllMenus (ID_PICTURES_ALWAYSSHOWGRID, MF_CHECKED);
	}
	//SmartDC sdc (this);
	//pics.setAlwaysShowGrid (alwaysShowGrid, sdc.get ());
	pics.setSpecialGreaterThanMax (specialGreaterThanMax);
}

void QtAndorWindow::abortCameraRun ()
{
	int status = andor.queryStatus ();
	if (ANDOR_SAFEMODE)
	{
		// simulate as if you needed to abort.
		status = DRV_ACQUIRING;
	}
	if (status == DRV_ACQUIRING)
	{
		andor.abortAcquisition ();
		timer.setTimerDisplay ("Aborted");
		andor.setIsRunningState (false);
		// close the plotting thread.
		plotThreadAborting = true;
		plotThreadActive = false;
		atomCrunchThreadActive = false;
		// Wait until plotting thread is complete.
		while (true)
		{
			auto res = WaitForSingleObject (plotThreadHandle, 2e3);
			if (res == WAIT_TIMEOUT)
			{
				auto ans = promptBox ("The real time plotting thread is taking a while to close. Continue waiting?",
					MB_YESNO);
				if (ans == IDNO)
				{
					// This might indicate something about the code is gonna crash...
					break;
				}
			}
			else
			{
				break;
			}
		}
		plotThreadAborting = false;
		// camera is no longer running.
		try
		{
			dataHandler.closeFile ();
		}
		catch (Error& err)
		{
			reportErr (err.trace ());
		}


		if (andor.getAndorRunSettings ().acquisitionMode != AndorRunModes::mode::Video)
		{
			int answer = promptBox ("Acquisition Aborted. Delete Data file (data_" + str (dataHandler.getDataFileNumber ())
				+ ".h5) for this run?", MB_YESNO);
			if (answer == IDYES)
			{
				try
				{
					dataHandler.deleteFile (mainWin->getComm ());
				}
				catch (Error& err)
				{
					reportErr (err.trace ());
				}
			}
		}
	}
	else if (status == DRV_IDLE)
	{
		andor.setIsRunningState (false);
	}
}


bool QtAndorWindow::cameraIsRunning ()
{
	return andor.isRunning ();
}

LRESULT QtAndorWindow::onCameraCalProgress (WPARAM wParam, LPARAM lParam)
{
	UINT picNum = lParam;
	if (lParam == 0)
	{
		// ???
		return NULL;
	}
	AndorRunSettings curSettings = andor.getAndorRunSettings ();
	if (lParam == -1)
	{
		// last picture.
		picNum = curSettings.totalPicsInExperiment ();
	}
	// need to call this before acquireImageData().
	andor.updatePictureNumber (picNum);

	std::vector<Matrix<long>> picData;
	try
	{
		picData = andor.acquireImageData (mainWin->getComm ());
	}
	catch (Error& err)
	{
		reportErr (err.trace ());
		return NULL;
	}
	avgBackground = Matrix<long> (picData.back ().getRows (), picData.back ().getCols ());
	avgBackground = picData.back ();
	try
	{
		if (picNum % curSettings.picsPerRepetition == 0)
		{
			int counter = 0;
			for (auto data : picData)
			{
				std::pair<int, int> minMax;
				minMax = stats.update (data, counter, selectedPixel, picNum / curSettings.picsPerRepetition,
					curSettings.totalPicsInExperiment () / curSettings.picsPerRepetition);
				pics.drawBitmap ( data, minMax, counter);
				pics.drawDongles ( selectedPixel, analysisHandler.getAnalysisLocs (),
					analysisHandler.getGrids (), picNum, false);
				counter++;
			}
			timer.update (picNum / curSettings.picsPerRepetition, curSettings.repetitionsPerVariation,
				curSettings.totalVariations, curSettings.picsPerRepetition);
		}
	}
	catch (Error& err)
	{
		reportErr (err.trace ());
	}
	mostRecentPicNum = picNum;
	return 0;
}



LRESULT QtAndorWindow::onCameraProgress (WPARAM wParam, LPARAM lParam)
{
	currentPictureNum++;
	UINT picNum = currentPictureNum;
	if (picNum % 2 == 1)
	{
		mainThreadStartTimes.push_back (std::chrono::high_resolution_clock::now ());
	}
	if (lParam == 0)
	{
		// ???
		//return NULL;
	}
	AndorRunSettings curSettings = andor.getAndorRunSettings ();
	if (lParam == -1)
	{
		// last picture.
		//picNum = curSettings.totalPicsInExperiment();
	}
	if (lParam != currentPictureNum && lParam != -1)
	{
		if (curSettings.acquisitionMode != AndorRunModes::mode::Video)
		{
			//reportErr ( "WARNING: picture number reported by andor isn't matching the"
			//								  "camera window record?!?!?!?!?" );
		}
	}
	// need to call this before acquireImageData().
	andor.updatePictureNumber (picNum);

	std::vector<Matrix<long>> rawPicData;
	try
	{
		rawPicData = andor.acquireImageData (mainWin->getComm ());
	}
	catch (Error& err)
	{
		reportErr (err.trace ());
		return NULL;
	}
	std::vector<Matrix<long>> calPicData (rawPicData.size ());
	if (andorSettingsCtrl.getUseCal () && avgBackground.size () == rawPicData.front ().size ())
	{
		for (auto picInc : range (rawPicData.size ()))
		{
			calPicData[picInc] = Matrix<long> (rawPicData[picInc].getRows (), rawPicData[picInc].getCols (), 0);
			for (auto pixInc : range (rawPicData[picInc].size ()))
			{
				calPicData[picInc].data[pixInc] = (rawPicData[picInc].data[pixInc] - avgBackground.data[pixInc]);
			}
		}
	}
	else { calPicData = rawPicData; }

	if (picNum % 2 == 1)
	{
		imageGrabTimes.push_back (std::chrono::high_resolution_clock::now ());
	}
	{
		std::lock_guard<std::mutex> locker (imageQueueLock);
		imQueue.push_back ({ picNum, calPicData[(picNum - 1) % curSettings.picsPerRepetition] });
	}

	auto picsToDraw = andorSettingsCtrl.getImagesToDraw (calPicData);
	try
	{
		if (realTimePic)
		{
			std::pair<int, int> minMax;
			// draw the most recent pic.
			minMax = stats.update (picsToDraw.back (), picNum % curSettings.picsPerRepetition, selectedPixel,
				picNum / curSettings.picsPerRepetition,
				curSettings.totalPicsInExperiment () / curSettings.picsPerRepetition);
			pics.drawBitmap (picsToDraw.back (), minMax, picNum % curSettings.picsPerRepetition);

			timer.update (picNum / curSettings.picsPerRepetition, curSettings.repetitionsPerVariation,
				curSettings.totalVariations, curSettings.picsPerRepetition);
		}
		else if (picNum % curSettings.picsPerRepetition == 0)
		{
			int counter = 0;
			for (auto data : picsToDraw)
			{
				std::pair<int, int> minMax;
				minMax = stats.update (data, counter, selectedPixel, picNum / curSettings.picsPerRepetition,
					curSettings.totalPicsInExperiment () / curSettings.picsPerRepetition);
				if (minMax.second > 50000)
				{
					numExcessCounts++;
					if (numExcessCounts > 2)
					{
						// POTENTIALLY DANGEROUS TO CAMERA.
						// AUTO PAUSE THE EXPERIMENT. 
						// This can happen if a laser, particularly the axial raman laser, is left on during an image.
						// cosmic rays may occasionally trip it as well.
						commonFunctions::handleCommonMessage (ID_ACCELERATOR_F2, this);
						errBox ("EXCCESSIVE CAMERA COUNTS DETECTED!!!");
					}
				}
				else
				{
					numExcessCounts = 0;
				}
				pics.drawBitmap (data, minMax, counter);
				pics.drawDongles (selectedPixel, analysisHandler.getAnalysisLocs (),
					analysisHandler.getGrids (), picNum, analysisHandler.getDrawGridOption ());
				counter++;
			}
			timer.update (picNum / curSettings.picsPerRepetition, curSettings.repetitionsPerVariation,
				curSettings.totalVariations, curSettings.picsPerRepetition);
		}
	}
	catch (Error& err)
	{
		reportErr (err.trace ());
	}

	// write the data to the file.
	if (curSettings.acquisitionMode != AndorRunModes::mode::Video)
	{
		try
		{
			// important! write the original data, not the pic-to-draw, which can be a difference pic, or the calibrated
			// pictures, which can have the background subtracted.
			dataHandler.writeAndorPic (rawPicData[(picNum - 1) % curSettings.picsPerRepetition],
				curSettings.imageSettings);
		}
		catch (Error& err)
		{
			reportErr (err.trace ());
			mainWin->handlePause ();
		}
	}
	mostRecentPicNum = picNum;
	return 0;
}


void QtAndorWindow::wakeRearranger ()
{
	std::unique_lock<std::mutex> lock (rearrangerLock);
	rearrangerConditionVariable.notify_all ();
}


void QtAndorWindow::handleSpecialLessThanMinSelection ()
{
	if (specialLessThanMin)
	{
		specialLessThanMin = false;
		//mainWin->checkAllMenus (ID_PICTURES_LESS_THAN_MIN_SPECIAL, MF_UNCHECKED);
	}
	else
	{
		specialLessThanMin = true;
		//mainWin->checkAllMenus (ID_PICTURES_LESS_THAN_MIN_SPECIAL, MF_CHECKED);
	}
	pics.setSpecialLessThanMin (specialLessThanMin);
}


void QtAndorWindow::handleSpecialGreaterThanMaxSelection ()
{
	if (specialGreaterThanMax)
	{
		specialGreaterThanMax = false;
		//mainWin->checkAllMenus (ID_PICTURES_GREATER_THAN_MAX_SPECIAL, MF_UNCHECKED);
	}
	else
	{
		specialGreaterThanMax = true;
		//mainWin->checkAllMenus (ID_PICTURES_GREATER_THAN_MAX_SPECIAL, MF_CHECKED);
	}
	pics.setSpecialGreaterThanMax (specialGreaterThanMax);
}


void QtAndorWindow::handleAutoscaleSelection ()
{
	if (autoScalePictureData)
	{
		autoScalePictureData = false;
		//mainWin->checkAllMenus (ID_PICTURES_AUTOSCALEPICTURES, MF_UNCHECKED);
	}
	else
	{
		autoScalePictureData = true;
		//mainWin->checkAllMenus (ID_PICTURES_AUTOSCALEPICTURES, MF_CHECKED);
	}
	pics.setAutoScalePicturesOption (autoScalePictureData);
}


LRESULT QtAndorWindow::onCameraCalFinish (WPARAM wParam, LPARAM lParam)
{
	// notify the andor object that it is done.
	andor.onFinish ();
	andor.pauseThread ();
	andor.setCalibrating (false);
	justCalibrated = true;
	andorSettingsCtrl.cameraIsOn (false);
	// normalize.
	for (auto& p : avgBackground)
	{
		p /= 100.0;
	}
	// if auto cal is selected, always assume that the user was trying to start with F5.
	if (andorSettingsCtrl.getAutoCal ())
	{
		//PostMessageA (WM_COMMAND, MAKEWPARAM (ID_ACCELERATOR_F5, 0));
	}
	return 0;
}


dataPoint QtAndorWindow::getMainAnalysisResult ()
{
	return mostRecentAnalysisResult;
}

std::mutex& QtAndorWindow::getActivePlotMutexRef ()
{
	return activePlotMutex;
}


void QtAndorWindow::cleanUpAfterExp ()
{
	plotThreadActive = false;
	atomCrunchThreadActive = false;
	dataHandler.closeFile ();
}


LRESULT QtAndorWindow::onCameraFinish (WPARAM wParam, LPARAM lParam)
{
	// notify the andor object that it is done.
	andor.onFinish ();
	//andor.pauseThread();
	if (alerts.soundIsToBePlayed ())
	{
		alerts.playSound ();
	}
	//mainWin->getComm()->sendStatus( "Andor has finished taking pictures and is no longer running.\r\n" );
	andorSettingsCtrl.cameraIsOn (false);
	// rearranger thread handles these right now.
	mainThreadStartTimes.clear ();

	crunchFinTimes.clear ();
	crunchSeesTimes.clear ();
	scriptWin->stopRearranger ();
	wakeRearranger ();
	{
		std::lock_guard<std::mutex> lock (activePlotMutex);
		if (activeDlgPlots.size () != 0)
		{
			mostRecentAnalysisResult = activeDlgPlots.back ()->getMainAnalysisResult ();
		}
	}
	return 0;
}


int QtAndorWindow::getMostRecentFid ()
{
	return dataHandler.getDataFileNumber ();

}

int QtAndorWindow::getPicsPerRep ()
{
	return andorSettingsCtrl.getSettings ().andor.picsPerRepetition;
}

std::string QtAndorWindow::getMostRecentDateString ()
{
	return dataHandler.getMostRecentDateString ();
}


bool QtAndorWindow::wantsThresholdAnalysis ()
{
	return analysisHandler.wantsThresholdAnalysis ();
}

atomGrid QtAndorWindow::getMainAtomGrid ()
{
	return analysisHandler.getAtomGrid (0);
}


void QtAndorWindow::armCameraWindow (AndorRunSettings* settings)
{
	//readImageParameters ();
	pics.setNumberPicturesActive (settings->picsPerRepetition);
	if (settings->picsPerRepetition == 1)
	{
		//pics.setSinglePicture (this, settings->imageSettings);
	}
	else
	{
		//pics.setMultiplePictures (this, settings->imageSettings, settings->picsPerRepetition);
	}
	pics.resetPictureStorage ();
	//pics.setParameters (settings->imageSettings);
	//redrawPictures (false);
	andorSettingsCtrl.setRepsPerVariation (settings->repetitionsPerVariation);
	andorSettingsCtrl.setVariationNumber (settings->totalVariations);
	pics.setSoftwareAccumulationOptions (andorSettingsCtrl.getSoftwareAccumulationOptions ());
	// turn some buttons off.
	andorSettingsCtrl.cameraIsOn (true);
	//SmartDC sdc (this);
	//pics.refreshBackgrounds( sdc.get ());
	stats.reset ();
	analysisHandler.updateDataSetNumberEdit (dataHandler.getNextFileNumber () - 1);
}


bool QtAndorWindow::getCameraStatus ()
{
	return andor.isRunning ();
}


void QtAndorWindow::handleDblClick (NMHDR* info, LRESULT* lResult)
{
	try
	{
		analysisHandler.handleDoubleClick (&mainWin->getFonts (), andorSettingsCtrl.getSettings ().andor.picsPerRepetition);
	}
	catch (Error& err)
	{
		reportErr (err.trace ());
	}
	mainWin->updateConfigurationSavedStatus (false);
}


void QtAndorWindow::listViewRClick (NMHDR* info, LRESULT* lResult)
{
	analysisHandler.handleRClick ();
	mainWin->updateConfigurationSavedStatus (false);
}

void QtAndorWindow::stopSound ()
{
	alerts.stopSound ();
}


/*
 *
 */
void QtAndorWindow::passSetTemperaturePress ()
{
	try
	{
		if (andor.isRunning ())
		{
			thrower ("ERROR: the camera (thinks that it?) is running. You can't change temperature settings during camera "
				"operation.");
		}
		andorSettingsCtrl.handleSetTemperaturePress ();
		auto settings = andorSettingsCtrl.getSettings ();
		andor.setSettings (settings.andor);
		andor.setTemperature ();
	}
	catch (Error& err)
	{
		reportErr (err.trace ());
	}
	mainWin->updateConfigurationSavedStatus (false);
}


/*
 *
 */
void QtAndorWindow::OnTimer (UINT_PTR id)
{
	if (id == 0) // temperature checking
	{
		auto temp = andor.getTemperature ();
		andorSettingsCtrl.changeTemperatureDisplay (temp);
	}
	else if (id == 1) // auto run calibrations.
	{
		if (AUTO_CALIBRATE && !mainWin->masterIsRunning ())
		{
			// check that it's past 5AM, don't want to interrupt late night progress. 
			std::time_t time = std::time (0);
			std::tm now;
			::localtime_s (&now, &time);
			if (now.tm_hour > 5)
			{
				try
				{
					dataHandler.assertCalibrationFilesExist ();
				}
				catch (Error&)
				{
					// files don't exist, run calibration. 
					try
					{
						commonFunctions::handleCommonMessage (ID_ACCELERATOR_F11, this);
					}
					catch (Error& err)
					{
						reportErr ("Failed to automatically start calibrations!" + err.trace ());
					}
				}
			}
		}
	}
}

void QtAndorWindow::handlePictureSettings ()
{
	selectedPixel = { 0,0 };
	andorSettingsCtrl.handlePictureSettings ();
	if (andorSettingsCtrl.getSettings ().andor.picsPerRepetition == 1)
	{
		//pics.setSinglePicture (this, andorSettingsCtrl.getSettings ().andor.imageSettings);
	}
	else
	{
		//pics.setMultiplePictures (this, andorSettingsCtrl.getSettings ().andor.imageSettings,
		//	andorSettingsCtrl.getSettings ().andor.picsPerRepetition);
	}
	pics.resetPictureStorage ();
	std::array<int, 4> nums = andorSettingsCtrl.getSettings ().palleteNumbers;
	pics.setPalletes (nums);
}

/*
Check that the camera is idle, or not aquiring pictures. Also checks that the data analysis handler isn't active.
*/
void QtAndorWindow::checkCameraIdle ()
{
	if (andor.isRunning ())
	{
		thrower ("Camera is already running! Please Abort to restart.\r\n");
	}
	if (analysisHandler.getLocationSettingStatus ())
	{
		thrower ("Please finish selecting analysis points before starting the camera!\r\n");
	}
	// make sure it's idle.
	try
	{
		andor.queryStatus ();
		if (ANDOR_SAFEMODE)
		{
			thrower ("DRV_IDLE");
		}
	}
	catch (Error& exception)
	{
		if (exception.whatBare () != "DRV_IDLE")
		{
			throwNested (" while querying andor status to check if idle.");
		}
	}
}

void QtAndorWindow::handleMasterConfigSave (std::stringstream& configStream)
{
	andorSettingsCtrl.handelSaveMasterConfig (configStream);
}


void QtAndorWindow::handleMasterConfigOpen (ConfigStream& configStream)
{
	mainWin->updateConfigurationSavedStatus (false);
	selectedPixel = { 0,0 };
	andorSettingsCtrl.handleOpenMasterConfig (configStream, this);
	pics.setParameters (andorSettingsCtrl.getSettings ().andor.imageSettings);
	redrawPictures (true);
}


DataLogger& QtAndorWindow::getLogger ()
{
	return dataHandler;
}


void QtAndorWindow::loadCameraCalSettings (AllExperimentInput& input)
{
	redrawPictures (false);
	try
	{
		checkCameraIdle ();
	}
	catch (Error& err)
	{
		reportErr (err.trace ());
	}
	pics.refreshBackgrounds ();
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

AndorCameraCore& QtAndorWindow::getCamera ()
{
	return andor;
}


void QtAndorWindow::prepareAtomCruncher (AllExperimentInput& input)
{
	input.cruncherInput = new atomCruncherInput;
	input.cruncherInput->plotterActive = plotThreadActive;
	input.cruncherInput->imageDims = andorSettingsCtrl.getSettings ().andor.imageSettings;
	atomCrunchThreadActive = true;
	input.cruncherInput->plotterNeedsImages = input.masterInput->plotterInput->needsCounts;
	input.cruncherInput->cruncherThreadActive = &atomCrunchThreadActive;
	skipNext = false;
	input.cruncherInput->skipNext = &skipNext;
	input.cruncherInput->imQueue = &imQueue;
	// options
	if (input.masterInput)
	{
		auto& niawg = input.masterInput->devices.getSingleDevice< NiawgCore > ();
		input.cruncherInput->rearrangerActive = niawg.expRerngOptions.active;
	}
	else
	{
		input.cruncherInput->rearrangerActive = false;
	}
	// locks
	input.cruncherInput->imageQueueLock = &imageQueueLock;
	input.cruncherInput->plotLock = &plotLock;
	input.cruncherInput->rearrangerLock = &rearrangerLock;
	// what the thread fills.
	input.cruncherInput->grids = analysisHandler.getGrids ();
	// reinitialize
	plotterPictureQueue.clear ();
	plotterAtomQueue.clear ();
	input.cruncherInput->plotterImageQueue = &plotterPictureQueue;
	input.cruncherInput->plotterAtomQueue = &plotterAtomQueue;
	rearrangerAtomQueue.clear ();
	input.cruncherInput->rearrangerAtomQueue = &rearrangerAtomQueue;
	input.cruncherInput->thresholds = andorSettingsCtrl.getSettings ().thresholds;
	input.cruncherInput->picsPerRep = andorSettingsCtrl.getSettings ().andor.picsPerRepetition;
	input.cruncherInput->catchPicTime = &crunchSeesTimes;
	input.cruncherInput->finTime = &crunchFinTimes;
	input.cruncherInput->atomThresholdForSkip = mainWin->getMainOptions ().atomSkipThreshold;
	input.cruncherInput->rearrangerConditionWatcher = &rearrangerConditionVariable;
}


void QtAndorWindow::startAtomCruncher (AllExperimentInput& input)
{
	UINT atomCruncherID;
	atomCruncherThreadHandle = (HANDLE)_beginthreadex (0, 0, QtAndorWindow::atomCruncherProcedure,
		(void*)input.cruncherInput, 0, &atomCruncherID);
}


bool QtAndorWindow::wantsAutoPause ()
{
	return alerts.wantsAutoPause ();
}

std::atomic<bool>& QtAndorWindow::getPlotThreadActiveRef ()
{
	return plotThreadActive;
}

std::atomic<HANDLE>& QtAndorWindow::getPlotThreadHandleRef ()
{
	return plotThreadHandle;
}

void QtAndorWindow::preparePlotter (AllExperimentInput& input)
{
	/// start the plotting thread.
	plotThreadActive = true;
	plotThreadAborting = false;
	imQueue.clear ();
	plotterAtomQueue.clear ();
	input.masterInput->plotterInput = new realTimePlotterInput (analysisHandler.getPlotTime ());
	auto& pltInput = input.masterInput->plotterInput;
	//pltInput->plotPens = mainWin->getPlotPens ();
	//pltInput->plotBrushes = mainWin->getPlotBrushes ();
	pltInput->plotFont = mainWin->getPlotFont ();
	pltInput->plotParentWindow = this;
	pltInput->cameraSettings = andorSettingsCtrl.getSettings ();
	pltInput->aborting = &plotThreadAborting;
	pltInput->active = &plotThreadActive;
	pltInput->imQueue = &plotterPictureQueue;
	pltInput->imageShape = andorSettingsCtrl.getSettings ().andor.imageSettings;
	pltInput->picsPerVariation = mainWin->getRepNumber () * andorSettingsCtrl.getSettings ().andor.picsPerRepetition;
	pltInput->variations = auxWin->getTotalVariationNumber ();
	pltInput->picsPerRep = andorSettingsCtrl.getSettings ().andor.picsPerRepetition;
	pltInput->alertThreshold = alerts.getAlertThreshold ();
	pltInput->wantAtomAlerts = alerts.wantsAtomAlerts ();
	pltInput->comm = mainWin->getComm ();
	pltInput->plotLock = &plotLock;
	pltInput->numberOfRunsToAverage = 5;
	pltInput->plottingFrequency = analysisHandler.getPlotFreq ();
	pltInput->atomQueue = &plotterAtomQueue;
	analysisHandler.fillPlotThreadInput (pltInput);

	// remove old plots that aren't trying to sustain.
	activeDlgPlots.erase (std::remove_if (activeDlgPlots.begin (), activeDlgPlots.end (), PlotDialog::removeQuery),
		activeDlgPlots.end ());
	std::vector<double> dummyKey;
	dummyKey.resize (input.masterInput->numVariations);
	pltInput->key = dummyKey;
	UINT count = 0;
	for (auto& e : pltInput->key)
	{
		e = count++;
	}
	UINT mainPlotInc = 0;
	for (auto plotParams : pltInput->plotInfo)
	{
		// Create vector of data to be shared between plotter and data analysis handler. 
		std::vector<pPlotDataVec> data;
		// assume 1 data set...
		UINT numDataSets = 1;
		// +1 for average line
		UINT numLines = numDataSets * (pltInput->grids[plotParams.whichGrid].height
			* pltInput->grids[plotParams.whichGrid].width + 1);
		data.resize (numLines);
		for (auto& line : data)
		{
			line = pPlotDataVec (new plotDataVec (pltInput->key.size (), { 0, -1, 0 }));
			line->resize (pltInput->key.size ());
			// initialize x axis for all data sets.
			UINT count = 0;
			for (auto& keyItem : pltInput->key)
			{
				line->at (count++).x = keyItem;
			}
		}
		bool usedDlg = false;
		plotStyle style = plotParams.isHist ? plotStyle::HistPlot : plotStyle::ErrorPlot;
		while (true)
		{
			if (mainPlotInc >= 6)
			{
				// put extra plots in dialogs.
				// start a PlotDialog dialog
				/*PlotDialog* plot = new PlotDialog (data, style, mainWin->getPlotPens (),
					mainWin->getPlotFont (), mainWin->getPlotBrushes (),
					analysisHandler.getPlotTime (), andorSettingsCtrl.getSettings ().thresholds[0],
					plotIds,
					plotParams.name);
				//plot->Create (IDD_PLOT_DIALOG, this);
				plot->ShowWindow (SW_SHOW);
				activeDlgPlots.push_back (plot);*/
				pltInput->dataArrays.push_back (data);
				usedDlg = true;
				break;
			}
			break;
		}
		if (!usedDlg && mainPlotInc < 6)
		{
			mainAnalysisPlots[mainPlotInc]->setStyle (style);
			mainAnalysisPlots[mainPlotInc]->setData (data);
			mainAnalysisPlots[mainPlotInc]->setThresholds (andorSettingsCtrl.getSettings ().thresholds[0]);
			mainAnalysisPlots[mainPlotInc]->setTitle (plotParams.name);
			pltInput->dataArrays.push_back (data);
			mainPlotInc++;
		}
	}
}

bool QtAndorWindow::wantsNoMotAlert ()
{
	if (cameraIsRunning ())
	{
		return alerts.wantsMotAlerts ();
	}
	else
	{
		return false;
	}
}


UINT QtAndorWindow::getNoMotThreshold ()
{
	return alerts.getAlertThreshold ();
}

void QtAndorWindow::startPlotterThread (AllExperimentInput& input)
{
	bool gridHasBeenSet = false;
	auto& pltInput = input.masterInput->plotterInput;
	for (auto gridInfo : pltInput->grids)
	{
		if (!(gridInfo.topLeftCorner == coordinate (0, 0)))
		{
			gridHasBeenSet = true;
			break;
		}
	}
	UINT plottingThreadID;
	if ((!gridHasBeenSet && pltInput->analysisLocations.size () == 0) || pltInput->plotInfo.size () == 0)
	{
		plotThreadActive = false;
	}
	else
	{
		// start the plotting thread
		plotThreadActive = true;
		plotThreadHandle = (HANDLE)_beginthreadex (0, 0, DataAnalysisControl::plotterProcedure, (void*)pltInput,
			0, &plottingThreadID);
	}
}

// this thread has one purpose: watch the image vector thread for new images, determine where atoms are 
// (the atom crunching part), and pass them to the threads waiting on atom info.
// should consider modifying so that it can use an array of locations. At the moment doesn't.
// "Crunching" here is meant to be reminiscent of number-crunching. It's super-repeatative.
UINT __stdcall QtAndorWindow::atomCruncherProcedure (void* inputPtr)
{
	atomCruncherInput* input = (atomCruncherInput*)inputPtr;
	auto gridSize = input->grids.size ();
	if (input->grids[0].topLeftCorner == coordinate (0, 0) || gridSize == 0)
	{
		return 0;
	}
	std::vector<std::vector<long>> monitoredPixelIndecies (gridSize);
	// preparing for the crunching
	for (auto gridInc : range (gridSize))
	{
		auto& grid = input->grids[gridInc];
		for (auto columnInc : range (grid.width))
		{
			for (auto rowInc : range (grid.height))
			{
				ULONG pixelRow = (grid.topLeftCorner.row - 1) + rowInc * grid.pixelSpacing;
				ULONG pixelColumn = (grid.topLeftCorner.column - 1) + columnInc * grid.pixelSpacing;
				if (pixelRow >= input->imageDims.height () || pixelColumn >= input->imageDims.width ())
				{
					errBox ("atom grid appears to include pixels outside the image frame! Not allowed, seen by atom "
						"cruncher thread");
					return 0;
				}
				int index = ((input->imageDims.height () - 1 - pixelRow) * input->imageDims.width () + pixelColumn);
				if (index >= input->imageDims.width () * input->imageDims.height ())
				{
					// shouldn't happen after I finish debugging.
					errBox ("Math error! Somehow, the pixel indexes appear within bounds, but the calculated index"
						" is larger than the image is!  (A low level bug, this shouldn't happen)");
					return 0;
				}
				monitoredPixelIndecies[gridInc].push_back (index);
			}
		}
	}
	for (auto picThresholds : input->thresholds)
	{
		if (picThresholds.size () != 1 && picThresholds.size () != input->grids[0].numAtoms ())
		{
			errBox ("the list of thresholds isn't size 1 (constant) or the size of the number of atoms in the "
				"first grid! Size is " + str (picThresholds.size ()) + "and grid size is " +
				str (input->grids[0].numAtoms ()));
			return 0;
		}
	}
	UINT imageCount = 0;
	// loop watching the image queue.
	while (*input->cruncherThreadActive || input->imQueue->size () != 0)
	{
		// if no images wait until images. Should probably change to be event based, but want this to be fast...
		if (input->imQueue->size () == 0)
		{
			continue;
		}
		if (imageCount % 2 == 0)
		{
			input->catchPicTime->push_back (chronoClock::now ());
		}
		// tempImagePixels[grid][pixel]; only contains the counts for the pixels being monitored.
		PixListQueue tempImagePixels (gridSize);
		// tempAtomArray[grid][pixel]; only contains the boolean true/false of whether an atom passed a threshold or not. 
		atomQueue tempAtomArray (gridSize);
		for (auto gridInc : range (gridSize))
		{
			tempAtomArray[gridInc].image = std::vector<bool> (monitoredPixelIndecies[gridInc].size ());
			tempImagePixels[gridInc].image = std::vector<long> (monitoredPixelIndecies[gridInc].size ());
		}
		for (auto gridInc : range (gridSize))
		{
			UINT count = 0;
			{
				// if no images wait until images. Should probably change to be event based, but want this to be fast...
				std::lock_guard<std::mutex> locker (*input->imageQueueLock);
				if (input->imQueue->size () == 0)
				{
					continue;
				}
				// scope for the lock_guard. I want to free the lock as soon as possible, so add extra small scope.
				tempImagePixels[gridInc].repNum = (*input->imQueue)[0].repNum;
				tempAtomArray[gridInc].repNum = (*input->imQueue)[0].repNum;
				///*** Deal with 1st element entirely first, as this is important for the rearranger thread and the 
				/// load-skip both of which are very time-sensitive.
				for (auto pixelIndex : monitoredPixelIndecies[gridInc])
				{
					if (pixelIndex > (*input->imQueue)[0].image.size ())
					{
						/*errBox ("Monitored pixel indecies included pixel out of image?!?! pixel: " + str(pixelIndex)
							+ ", size: " + str((*input->imQueue)[0].image.size()) );*/
					}
					else
					{
						tempImagePixels[gridInc].image[count++] = (*input->imQueue)[0].image.data[pixelIndex];
					}
				}
			}
			count = 0;
			for (auto& pix : tempImagePixels[gridInc].image)
			{
				auto& picThresholds = input->thresholds[imageCount % input->picsPerRep];
				if (pix >= picThresholds[count % picThresholds.size ()])
				{
					tempAtomArray[gridInc].image[count] = true;
				}
				count++;
			}
			// explicitly deal with the rearranger thread and load skip as soon as possible, these are time-critical.
			if (gridInc == 0)
			{
				if (input->rearrangerActive) ///
				{
					// copies the array if first pic of rep. Only looks at first picture because its rearranging. Could change
					// if we need to do funny experiments, just need to change rearranger handling.
					if (imageCount % input->picsPerRep == 0)
					{
						{
							std::lock_guard<std::mutex> locker (*input->rearrangerLock);
							(*input->rearrangerAtomQueue).push_back (tempAtomArray[0]);
							input->rearrangerConditionWatcher->notify_all ();
						}
						input->finTime->push_back (chronoClock::now ());
					}
				}
				// if last picture of repetition, check for loadskip condition.
				if (imageCount % input->picsPerRep == input->picsPerRep - 1) ///
				{
					UINT numAtoms = std::accumulate (tempAtomArray[0].image.begin (), tempAtomArray[0].image.end (), 0);
					*input->skipNext = (numAtoms >= input->atomThresholdForSkip);
				}
			}
		}
		if (input->plotterActive)
		{
			// copies the array. Right now I'm assuming that the thread always needs atoms, which is not a good 
			// assumption potentially...
			std::lock_guard<std::mutex> locker (*input->plotLock);
			(*input->plotterAtomQueue).push_back (tempAtomArray);
			if (input->plotterNeedsImages)
			{
				(*input->plotterImageQueue).push_back (tempImagePixels);
			}
		}
		imageCount++;
		std::lock_guard<std::mutex> locker (*input->imageQueueLock);
		// if no images wait until images. Should probably change to be event based, but want this to be fast...
		if (input->imQueue->size () != 0)
		{
			(*input->imQueue).erase ((*input->imQueue).begin ());
		}
	}
	return 0;
}


std::string QtAndorWindow::getStartMessage ()
{
	// get selected plots
	auto andrSttngs = andorSettingsCtrl.getSettings ().andor;
	std::vector<std::string> plots = analysisHandler.getActivePlotList ();
	imageParameters currentImageParameters = andrSttngs.imageSettings;
	bool errCheck = false;
	for (UINT plotInc = 0; plotInc < plots.size (); plotInc++)
	{
		PlottingInfo tempInfoCheck (PLOT_FILES_SAVE_LOCATION + "\\" + plots[plotInc] + ".plot");
		if (tempInfoCheck.getPicNumber () != andrSttngs.picsPerRepetition)
		{
			thrower (": one of the plots selected, " + plots[plotInc] + ", is not built for the currently "
				"selected number of pictures per experiment. Please revise either the current setting or the plot"
				" file.");
		}
		tempInfoCheck.setGroups (analysisHandler.getAnalysisLocs ());
		std::vector<std::pair<UINT, UINT>> plotLocations = tempInfoCheck.getAllPixelLocations ();
	}
	std::string dialogMsg;
	dialogMsg = "Camera Parameters:\r\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\r\n";
	dialogMsg += "Current Camera Temperature Setting:\r\n\t" + str (
		andrSttngs.temperatureSetting) + "\r\n";
	dialogMsg += "Exposure Times: ";
	for (auto& time : andrSttngs.exposureTimes)
	{
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
	UINT count = 0;
	for (auto& picThresholds : andorSettingsCtrl.getSettings ().thresholds)
	{
		dialogMsg += "Pic " + str (count) + " thresholds: ";
		for (auto thresh : picThresholds)
		{
			dialogMsg += str (thresh) + ", ";
		}
		dialogMsg += "\r\n";
		count++;
	}

	dialogMsg += "\r\nReal-Time Plots:\r\n";
	for (UINT plotInc = 0; plotInc < plots.size (); plotInc++)
	{
		dialogMsg += "\t" + plots[plotInc] + "\r\n";
	}

	return dialogMsg;
}


void QtAndorWindow::fillMasterThreadInput (ExperimentThreadInput* input)
{
	currentPictureNum = 0;
	// starting a not-calibration, so reset this.
	justCalibrated = false;
	input->atomQueueForRearrangement = &rearrangerAtomQueue;
	input->rearrangerLock = &rearrangerLock;
	input->andorsImageTimes = &imageTimes;
	input->grabTimes = &imageGrabTimes;
	input->analysisGrid = analysisHandler.getAtomGrid (0);
	input->conditionVariableForRerng = &rearrangerConditionVariable;
}


void QtAndorWindow::setTimerText (std::string timerText)
{
	timer.setTimerDisplay (timerText);
}

void QtAndorWindow::setDataType (std::string dataType)
{
	stats.updateType (dataType);
}


void QtAndorWindow::redrawPictures (bool andGrid)
{
	try
	{
		pics.refreshBackgrounds ();
		if (andGrid)
		{
			pics.drawGrids ();
		}
	}
	catch (Error& err)
	{
		reportErr (err.trace ());
	}
	// currently don't attempt to redraw previous picture data.
}


std::atomic<bool>* QtAndorWindow::getSkipNextAtomic ()
{
	return &skipNext;
}


void QtAndorWindow::stopPlotter ()
{
	plotThreadAborting = true;
}


// this is typically a little redundant to call, but can use to make sure things are set to off.
void QtAndorWindow::assertOff ()
{
	andorSettingsCtrl.cameraIsOn (false);
	plotThreadActive = false;
	atomCrunchThreadActive = false;
}


void QtAndorWindow::readImageParameters ()
{
	selectedPixel = { 0,0 };
	try
	{
		redrawPictures (false);
		imageParameters parameters = andorSettingsCtrl.getSettings ().andor.imageSettings;
		pics.setParameters (parameters);
	}
	catch (Error& exception)
	{
		Communicator* comm = mainWin->getComm ();
		reportErr (exception.trace () + "\r\n");
	}
	pics.drawGrids ();
}


void QtAndorWindow::fillExpDeviceList (DeviceList& list)
{
	list.list.push_back (andor);
}


