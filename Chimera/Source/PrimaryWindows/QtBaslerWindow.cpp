#include "stdafx.h"
#include "QtBaslerWindow.h"
#include "Basler/BaslerCamera.h"
#include "Agilent/AgilentSettings.h"
#include <qdesktopwidget.h>
#include <PrimaryWindows/QtScriptWindow.h>
#include <PrimaryWindows/QtAndorWindow.h>
#include <PrimaryWindows/QtAuxiliaryWindow.h>
#include <PrimaryWindows/QtMainWindow.h>
#include <PrimaryWindows/QtBaslerWindow.h>
#include <PrimaryWindows/QtDeformableMirrorWindow.h>


QtBaslerWindow::QtBaslerWindow (QWidget* parent) : IChimeraQtWindow (parent), 
											picManager (true, "BASLER_PICTURE_MANAGER", true, Qt::FastTransformation){
	statBox = new ColorBox ();
	basCamCore = new BaslerCameraCore (this);
	if (!basCamCore->isInitialized ()){
		thrower ("ERROR: Camera not connected! Exiting program...");
	}
	setWindowTitle ("Basler Window");
}

QtBaslerWindow::~QtBaslerWindow (){
}

void QtBaslerWindow::initializeWidgets (){
	try{
		initializeControls ();
	}
	catch (ChimeraError& err){
		reportErr (err.qtrace ());
	}
}

void QtBaslerWindow::handlePrepareRequest (baslerSettings* settings){
	prepareWinForAcq (settings);
	runSettings = *settings;
}

void QtBaslerWindow::handleBaslerAutoscaleSelection (){
	if (autoScaleBaslerPictureData){
		autoScaleBaslerPictureData = false;
	}
	else{
		autoScaleBaslerPictureData = true;
	}
	picManager.setAutoScalePicturesOption (autoScaleBaslerPictureData);
}

baslerSettings QtBaslerWindow::getCurrentSettings (){
	return settingsCtrl.getCurrentSettings ();
}

/*
Load the settings appropriate for the mot size measurement and then start the camera.
*/
void QtBaslerWindow::startTemporaryAcquisition (baslerSettings settings){
	try	{
		handleDisarmPress ();
		currentRepNumber = 0;
		runningAutoAcq = true;
		tempAcqSettings = settings;
		picManager.setParameters (settings.dims);
	}
	catch (ChimeraError&){
		throwNested ("Failed to start temporary acquisition.");
	}
}

void QtBaslerWindow::handleSoftwareTrigger (){
	try	{
		basCamCore->softwareTrigger ();
	}
	catch (Pylon::TimeoutException&){
		reportErr ("Software trigger timed out!");
	}
}


void QtBaslerWindow::handleDisarmPress (){
	try{
		basCamCore->disarm ();
		isRunning = false;
		settingsCtrl.setStatus ("Camera Status: Idle");
	}
	catch (ChimeraError& err){
		reportErr ("Error! " + err.qtrace ());
		settingsCtrl.setStatus ("Camera Status: ERROR?!?!");
	}
}

void QtBaslerWindow::startDefaultAcquisition (){
	try	{
		if (basCamCore->isRunning ()){
			handleDisarmPress ();
		}
		currentRepNumber = 0;
		baslerSettings tempSettings;
		tempSettings.acquisitionMode = BaslerAcquisition::mode::Continuous;
		tempSettings.dims.left = 260;
		tempSettings.dims.right = 450;
		tempSettings.dims.top = 390;
		tempSettings.dims.bottom = 200;
		tempSettings.dims.horizontalBinning = 1;
		tempSettings.dims.verticalBinning = 1;

		tempSettings.exposureMode = BaslerAutoExposure::mode::Off;
		tempSettings.triggerMode = BaslerTrigger::mode::AutomaticSoftware;
		tempSettings.exposureTime = 100;
#ifdef _DEBUG
		tempSettings.frameRate = 1;
#else
		tempSettings.frameRate = 10;
#endif
		tempSettings.rawGain = 260;
		picManager.setParameters (tempSettings.dims);
		runExposureMode = tempSettings.exposureMode;
		// only important in safemode
		//tempSettings.repCount = tempSettings.acquisitionMode == BaslerAcquisition::mode::Finite ? 
		//						tempSettings.repCount : SIZE_MAX;
		settingsCtrl.setStatus ("Camera Status: Armed...");
		isRunning = true;
	}
	catch (ChimeraError& err){
		reportErr (err.qtrace ());
	}
}

void QtBaslerWindow::handleNewPics (Matrix<long> imageMatrix){
	try	{
		currentRepNumber++;
		//auto runSettings = basCamCore->getRunningSettings ();
		auto minMax = stats.update (imageMatrix, 0, { 0,0 }, currentRepNumber, runSettings.totalPictures ());
		QPainter painter (this);
		picManager.drawBitmap (imageMatrix, minMax, 0, std::vector<atomGrid> (), 0, false, painter);
		picManager.updatePlotData ();
		//picManager.drawDongles ({ 0,0 }, std::vector<coordinate> (), std::vector<atomGrid> (), 0);
		if (runExposureMode == BaslerAutoExposure::mode::Continuous){
			settingsCtrl.updateExposure (basCamCore->getCurrentExposure ());
		}
		settingsCtrl.setStatus ("Camera Status: Acquiring Pictures.");
		if (currentRepNumber % 10 == 0)	{
			settingsCtrl.handleFrameRate ();
		}
		if (!basCamCore->isContinuous ()){
			// don't write data if continuous, that's a recipe for disaster.
			andorWin->getLogger ().writeBaslerPic (imageMatrix);
		}
		if (currentRepNumber == runSettings.totalPictures ()){
			// handle balser finish
			basCamCore->disarm ();
			isRunning = false;
			runningAutoAcq = false;
			settingsCtrl.setStatus ("Camera Status: Finished finite acquisition.");
			// tell the andor window that the basler camera finished so that the data file can be handled appropriately.
			//mainWin->getComm ()->sendBaslerFin ();
			if (!andorWin->cameraIsRunning ()){
				// else it will close when the basler camera finishes.
				andorWin->getLogger ().normalCloseFile ();
			}
		}
		if (stats.getMostRecentStats ().avgv < settingsCtrl.getMotThreshold ()){
			motLoaded = false;
			loadMotConsecutiveFailures++;
			if (andorWin->wantsNoMotAlert ()){
				if (loadMotConsecutiveFailures > andorWin->getNoMotThreshold ()){
					//mainWin->getComm ()->sendNoMotAlert ();
				}
			}
		}
		else{
			motLoaded = true;
			loadMotConsecutiveFailures = 0;
		}
		settingsCtrl.redrawMotIndicator ();
	}
	catch (ChimeraError& err){
		reportErr(err.qtrace ());
		settingsCtrl.setStatus ("Camera Status: ERROR?!?!?");
	}
}


void QtBaslerWindow::passCameraMode (){
	try{
		settingsCtrl.handleCameraMode ();
	}
	catch (ChimeraError& err){
		reportErr ("Error! " + err.qtrace ());
	}
}

void QtBaslerWindow::passExposureMode (){
	try{
		settingsCtrl.handleExposureMode ();
	}
	catch (ChimeraError& err){
		reportErr ("Error! " + err.qtrace ());
	}
}

void QtBaslerWindow::prepareWinForAcq (baslerSettings* settings){
	try{
		currentRepNumber = 0;
		picManager.setParameters (settings->dims);

		runExposureMode = settings->exposureMode;
		isRunning = true;
	}
	catch (ChimeraError& err){
		reportErr (err.qtrace ());
	}
}

bool QtBaslerWindow::baslerCameraIsRunning (){
	if (BASLER_SAFEMODE){
		return isRunning;
	}
	return basCamCore->isRunning ();
}

bool QtBaslerWindow::baslerCameraIsContinuous (){
	return basCamCore->isContinuous ();
}

void QtBaslerWindow::windowOpenConfig (ConfigStream& configFile){
	ConfigSystem::standardOpenConfig (configFile, picManager.configDelim, &picManager, Version ("4.0"));
	baslerSettings settings;
	ConfigSystem::stdGetFromConfig (configFile, *basCamCore, settings, Version ("4.0"));
	settingsCtrl.setSettings (settings);
}

void QtBaslerWindow::windowSaveConfig (ConfigStream& configFile){
	picManager.handleSaveConfig (configFile);
	settingsCtrl.handleSavingConfig (configFile);
}

void QtBaslerWindow::initializeControls (){
	QPoint pos = { 0,25 };
	QPoint dims = basCamCore->getCameraDimensions ();
	statBox->initialize (pos, this, 300, mainWin->getDevices ());
	settingsCtrl.initialize (pos, dims.x(), dims.y(), dims, this);
	settingsCtrl.setSettings (basCamCore->getDefaultSettings ());
	stats.initialize (pos, this);

	pos = { 365, 25 };
	// scale to fill the window (approximately).
	dims.rx() *= 1.55;
	dims.ry() *= 1.55;
	picManager.initialize (pos, dims.x() + pos.x() + 115, dims.y() + pos.y(), this, 1);
	picManager.setSinglePicture (settingsCtrl.getCurrentSettings ().dims);
	picManager.setPalletes ({ 1,1,1,1 });
}

void QtBaslerWindow::fillExpDeviceList (DeviceList& list){
	list.list.push_back (*basCamCore);
}
