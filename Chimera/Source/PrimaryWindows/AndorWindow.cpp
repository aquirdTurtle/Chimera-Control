// created by Mark O. Brown
#include "stdafx.h"
#include "GeneralUtilityFunctions/commonFunctions.h"
#include "Andor/CameraSettingsControl.h"
#include "Plotting/PlotCtrl.h"
#include "Plotting/PlottingInfo.h"
#include "AuxiliaryWindow.h"
#include "AndorWindow.h"
#include "MainWindow.h"
#include "RealTimeDataAnalysis/realTimePlotterInput.h"
#include "ExperimentThread/MasterThreadInput.h"
#include "ExcessDialogs/ErrDialog.h"
#include "ATMCD32D.H"
#include <numeric>
#include "PrimaryWindows/BaslerWindow.h"

AndorWindow::AndorWindow ( ) : CDialog ( ),
							andorSettingsCtrl ( ),
							dataHandler ( DATA_SAVE_LOCATION ),
							andor ( ANDOR_SAFEMODE ),
							pics ( false, "ANDOR_PICTURE_MANAGER", false )
{};


IMPLEMENT_DYNAMIC(AndorWindow, CDialog)


BEGIN_MESSAGE_MAP ( AndorWindow, CDialog )
	ON_WM_CTLCOLOR ( )
	ON_WM_SIZE ( )
	ON_WM_TIMER ( )
	ON_WM_VSCROLL ( )
	ON_WM_MOUSEMOVE ( )
	ON_WM_PAINT()

	ON_COMMAND_RANGE ( MENU_ID_RANGE_BEGIN, MENU_ID_RANGE_END, &AndorWindow::passCommonCommand )
	ON_COMMAND_RANGE ( PICTURE_SETTINGS_ID_START, PICTURE_SETTINGS_ID_END, &AndorWindow::passPictureSettings )
	ON_CONTROL_RANGE ( CBN_SELENDOK, PICTURE_SETTINGS_ID_START, PICTURE_SETTINGS_ID_END,
					   &AndorWindow::passPictureSettings )
	ON_CBN_SELENDOK ( IDC_ATOM_GRID_COMBO, &AndorWindow::passAtomGridCombo )
	// these ids all go to the same function.
	ON_CONTROL_RANGE ( EN_CHANGE, IDC_PICTURE_1_MIN_EDIT, IDC_PICTURE_1_MIN_EDIT, &AndorWindow::handlePictureEditChange )
	ON_CONTROL_RANGE ( EN_CHANGE, IDC_PICTURE_1_MAX_EDIT, IDC_PICTURE_1_MAX_EDIT, &AndorWindow::handlePictureEditChange )
	ON_CONTROL_RANGE ( EN_CHANGE, IDC_PICTURE_2_MIN_EDIT, IDC_PICTURE_2_MIN_EDIT, &AndorWindow::handlePictureEditChange )
	ON_CONTROL_RANGE ( EN_CHANGE, IDC_PICTURE_2_MAX_EDIT, IDC_PICTURE_2_MAX_EDIT, &AndorWindow::handlePictureEditChange )
	ON_CONTROL_RANGE ( EN_CHANGE, IDC_PICTURE_3_MIN_EDIT, IDC_PICTURE_3_MIN_EDIT, &AndorWindow::handlePictureEditChange )
	ON_CONTROL_RANGE ( EN_CHANGE, IDC_PICTURE_3_MAX_EDIT, IDC_PICTURE_3_MAX_EDIT, &AndorWindow::handlePictureEditChange )
	ON_CONTROL_RANGE ( EN_CHANGE, IDC_PICTURE_4_MIN_EDIT, IDC_PICTURE_4_MIN_EDIT, &AndorWindow::handlePictureEditChange )
	ON_CONTROL_RANGE ( EN_CHANGE, IDC_PICTURE_4_MAX_EDIT, IDC_PICTURE_4_MAX_EDIT, &AndorWindow::handlePictureEditChange )
	// 
	ON_EN_CHANGE( IDC_PLOT_TIMER_EDIT, &AndorWindow::handlePlotTimerEdit )
	ON_COMMAND( IDC_SET_TEMPERATURE_BUTTON, &AndorWindow::passSetTemperaturePress)
	ON_COMMAND( IDOK, &AndorWindow::catchEnter)
	ON_COMMAND( IDC_SET_ANALYSIS_LOCATIONS, &AndorWindow::passManualSetAnalysisLocations)
	ON_COMMAND( IDC_SET_GRID_CORNER, &AndorWindow::passSetGridCorner)
	ON_COMMAND( IDC_DEL_GRID_BUTTON, &AndorWindow::passDelGrid)
	ON_COMMAND( IDC_CAMERA_CALIBRATION_BUTTON, &AndorWindow::calibrate)
	ON_COMMAND_RANGE(ID_PLOT_POP_IDS_BEGIN, ID_PLOT_POP_IDS_END, &AndorWindow::handlePlotPop)
	ON_CBN_SELENDOK( IDC_TRIGGER_COMBO, &AndorWindow::passTrigger )
	ON_CBN_SELENDOK( IDC_CAMERA_MODE_COMBO, &AndorWindow::passCameraMode )

	ON_MESSAGE ( MainWindow::AndorFinishMessageID, &AndorWindow::onCameraFinish )
	ON_MESSAGE ( MainWindow::AndorCalFinMessageID, &AndorWindow::onCameraCalFinish )
	ON_MESSAGE ( MainWindow::AndorProgressMessageID, &AndorWindow::onCameraProgress )
	ON_MESSAGE ( MainWindow::AndorCalProgMessageID, &AndorWindow::onCameraCalProgress )
	ON_MESSAGE ( MainWindow::BaslerFinMessageID, &AndorWindow::onBaslerFinish )
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONUP()

	ON_NOTIFY(NM_RCLICK, IDC_PLOTTING_LISTVIEW, &AndorWindow::listViewRClick)
	ON_NOTIFY(NM_DBLCLK, IDC_PLOTTING_LISTVIEW, &AndorWindow::handleDblClick)
	//ON_EN_KILLFOCUS(IDC_EM_GAIN_EDIT, &AndorWindow::handleEmGainChange )
	ON_COMMAND ( IDC_EM_GAIN_BTN, &AndorWindow::handleEmGainChange )
	ON_CONTROL_RANGE(EN_KILLFOCUS, IDC_IMAGE_DIMS_START, IDC_IMAGE_DIMS_END, &AndorWindow::handleImageDimsEdit )

END_MESSAGE_MAP()


void AndorWindow::handlePlotPop (UINT id)
{
	for (auto& plt : mainAnalysisPlots)
	{
		if (plt->handlePop (id, this)) { return; }
	}
}


void AndorWindow::handlePlotTimerEdit ( )
{
	analysisHandler.updatePlotTime ( );
}

LRESULT AndorWindow::onBaslerFinish ( WPARAM wParam, LPARAM lParam )
{
	if ( !cameraIsRunning ( ) )
	{
		dataHandler.closeFile ( );
	}
	return 0;
}


bool AndorWindow::wasJustCalibrated( )
{
	return justCalibrated;
}


bool AndorWindow::wantsAutoCal( )
{
	return andorSettingsCtrl.getAutoCal( );
}


void AndorWindow::calibrate( )
{
	commonFunctions::calibrateCameraBackground(scriptWin, mainWin, this, auxWin );
}


void AndorWindow::passDelGrid( )
{
	try
	{
		analysisHandler.handleDeleteGrid( );
	}
	catch ( Error& err )
	{
		mainWin->getComm( )->sendError( err.trace( ) );
	}
}


void AndorWindow::writeVolts( UINT currentVoltNumber, std::vector<float64> data )
{
	try
	{
		dataHandler.writeVolts( currentVoltNumber, data );
	}
	catch ( Error& err )
	{
		mainWin->getComm( )->sendError( err.trace( ) );
	}
}


void AndorWindow::OnMouseMove( UINT thing, CPoint point )
{
	try
	{
		pics.handleMouse( point );
	}
	catch ( Error& err )
	{
		mainWin->getComm ( )->sendError ( err.trace ( ) );
	}
}


void AndorWindow::handleImageDimsEdit( UINT id )
{
	try
	{
		SmartDC sdc (this);
		pics.setParameters (andorSettingsCtrl.getSettings ().andor.imageSettings);
		pics.redrawPictures( sdc.get (), selectedPixel, analysisHandler.getAnalysisLocs( ), analysisHandler.getGrids(), true,
							 mostRecentPicNum );
	}
	catch ( Error& err )
	{
		mainWin->getComm( )->sendError( err.trace( ) );
	}
}


void AndorWindow::handleEmGainChange()
{
	try
	{
		auto runSettings = andor.getAndorRunSettings ( ); 
		andorSettingsCtrl.setEmGain(runSettings.emGainModeIsOn, runSettings.emGainLevel );
		auto settings = andorSettingsCtrl.getSettings ( );
		runSettings.emGainModeIsOn = settings.andor.emGainModeIsOn;
		runSettings.emGainLevel = settings.andor.emGainLevel;
		andor.setSettings ( runSettings );
		// and immediately change the EM gain mode.
		try
		{
			andor.setGainMode ( );
		}
		catch ( Error& err )
		{
			// this can happen e.g. if the camera is aquiring.
			errBox ( err.trace ( ) );
		}
	}
	catch ( Error err )
	{
		mainWin->getComm( )->sendError( err.trace( ) );
	}
}


std::string AndorWindow::getSystemStatusString()
{
	std::string statusStr; 
	statusStr = "\nAndor Camera:\n";
	if (!ANDOR_SAFEMODE)
	{
		statusStr += "\tCode System is Active!\n";
		statusStr += "\t" + andor.getSystemInfo();
	}
	else
	{
		statusStr += "\tCode System is disabled! Enable in \"constants.h\"\n";
	}
	return statusStr;
}


void AndorWindow::handleNewConfig( std::ofstream& newFile )
{
	andorSettingsCtrl.handleNewConfig( newFile );
	pics.handleNewConfig( newFile );
	analysisHandler.handleNewConfig( newFile );
}


void AndorWindow::handleSaveConfig(std::ofstream& saveFile)
{
	andorSettingsCtrl.handleSaveConfig(saveFile);
	pics.handleSaveConfig(saveFile);
	analysisHandler.handleSaveConfig( saveFile );
}


void AndorWindow::handleOpeningConfig ( std::ifstream& configFile, Version ver )
{
	AndorRunSettings camSettings;
	try
	{
		camSettings = ProfileSystem::stdGetFromConfig ( configFile, "CAMERA_SETTINGS", 
															 AndorCameraSettingsControl::getRunSettingsFromConfig );
		andorSettingsCtrl.setRunSettings ( camSettings );
	}
	catch ( Error& err )
	{
		errBox ( "Failed to get Andor Camera Run settings from file! " + err.trace() );
	}
	try
	{
		auto picSettings = ProfileSystem::stdGetFromConfig ( configFile, "PICTURE_SETTINGS",
															 AndorCameraSettingsControl::getPictureSettingsFromConfig );
		andorSettingsCtrl.updatePicSettings ( picSettings );
	}
	catch ( Error& err )
	{
		errBox ( "Failed to get Andor Camera Picture settings from file! " + err.trace ( ) );
	}
	try
	{
		camSettings.imageSettings = ProfileSystem::stdGetFromConfig ( configFile, "CAMERA_IMAGE_DIMENSIONS",
																  AndorCameraSettingsControl::getImageDimSettingsFromConfig );
		andorSettingsCtrl.updateImageDimSettings ( camSettings.imageSettings );
		andorSettingsCtrl.updateRunSettingsFromPicSettings ();
	}
	catch ( Error& err )
	{
		errBox ( "Failed to get Andor Image Dimension settings from file! " + err.trace ( ) );
	}
	try
	{
		ProfileSystem::standardOpenConfig (configFile, pics.configDelim, &pics, Version ("4.0"));
	}
	catch (Error & err)
	{
		errBox ("Failed to load picture settings from config!");
	}
	try
	{
		ProfileSystem::standardOpenConfig (configFile, "DATA_ANALYSIS", &analysisHandler, Version ("4.0"));
	}
	catch (Error & err)
	{
		errBox ("Failed to load Data Analysis settings from config!");
	}
	try
	{
		if ( andorSettingsCtrl.getSettings ( ).andor.picsPerRepetition == 1 )
		{
			pics.setSinglePicture ( this, andorSettingsCtrl.getSettings ( ).andor.imageSettings );
		}
		else
		{
			pics.setMultiplePictures ( this, andorSettingsCtrl.getSettings ( ).andor.imageSettings,
									   andorSettingsCtrl.getSettings ( ).andor.picsPerRepetition );
		}
		pics.resetPictureStorage ( );
		std::array<int, 4> nums = andorSettingsCtrl.getSettings ( ).palleteNumbers;
		pics.setPalletes ( nums );
		CRect rect;
		GetWindowRect ( &rect );
		OnSize ( 0, rect.right - rect.left, rect.bottom - rect.top );
	}
	catch ( Error& e )
	{
		errBox ( "Andor Camera Window failed to read parameters from the configuration file.\n\n" + e.trace() );
	}
}


void AndorWindow::loadFriends(MainWindow* mainWin_, ScriptingWindow* scriptWin_, AuxiliaryWindow* auxWin_,
								BaslerWindow* basWin_)
{
	mainWin = mainWin_;
	scriptWin = scriptWin_;
	auxWin = auxWin_;
	basWin = basWin_;
}


void AndorWindow::passManualSetAnalysisLocations()
{
	analysisHandler.onManualButtonPushed();
	mainWin->updateConfigurationSavedStatus( false );
}


void AndorWindow::passSetGridCorner( )
{
	analysisHandler.onCornerButtonPushed( );
	mainWin->updateConfigurationSavedStatus( false );
}


void AndorWindow::catchEnter()
{
	// the default handling is to close the window, so I need to catch it.
	errBox("Hello there!");
}


void AndorWindow::passAlwaysShowGrid()
{
	if (alwaysShowGrid)
	{
		alwaysShowGrid = false;
		mainWin->checkAllMenus ( ID_PICTURES_ALWAYSSHOWGRID, MF_UNCHECKED );
	}
	else
	{
		alwaysShowGrid = true;
		mainWin->checkAllMenus ( ID_PICTURES_ALWAYSSHOWGRID, MF_CHECKED );
	}
	SmartDC sdc (this);
	pics.setAlwaysShowGrid(alwaysShowGrid, sdc.get ());
	pics.setSpecialGreaterThanMax(specialGreaterThanMax);
}


void AndorWindow::passCameraMode()
{
	andorSettingsCtrl.handleModeChange(this);
	CRect rect;
	GetClientRect ( &rect );
	OnSize ( 0, rect.right - rect.left, rect.bottom - rect.top );
	mainWin->updateConfigurationSavedStatus( false );
}


void AndorWindow::abortCameraRun()
{
	int status = andor.queryStatus ( );	
	if (ANDOR_SAFEMODE)
	{
		// simulate as if you needed to abort.
		status = DRV_ACQUIRING;
	}
	if (status == DRV_ACQUIRING)
	{
		andor.abortAcquisition();
		timer.setTimerDisplay( "Aborted" );
		andor.setIsRunningState( false );
		// close the plotting thread.
		plotThreadAborting = true;
		plotThreadActive = false;
		atomCrunchThreadActive = false;
		// Wait until plotting thread is complete.
		while ( true )
		{
			auto res = WaitForSingleObject ( plotThreadHandle, 2e3 );
			if ( res == WAIT_TIMEOUT )
			{
				auto ans = promptBox ( "The real time plotting thread is taking a while to close. Continue waiting?",
									   MB_YESNO );
				if ( ans == IDNO )
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
			dataHandler.closeFile();
		}
		catch (Error& err)
		{
			mainWin->getComm()->sendError(err.trace());
		}
		

		if (andor.getAndorRunSettings().acquisitionMode != AndorRunModes::mode::Video)
		{
			int answer = promptBox("Acquisition Aborted. Delete Data file (data_" + str(dataHandler.getDataFileNumber())
									  + ".h5) for this run?",MB_YESNO );
			if (answer == IDYES)
			{
				try
				{
					dataHandler.deleteFile(mainWin->getComm());
				}
				catch (Error& err)
				{
					mainWin->getComm()->sendError(err.trace());
				}
			}
		}
	}
	else if (status == DRV_IDLE)
	{
		andor.setIsRunningState(false);
	}
}


bool AndorWindow::cameraIsRunning()
{
	return andor.isRunning();
}


void AndorWindow::handlePictureEditChange( UINT id )
{
	try
	{
		pics.handleEditChange(id);
	}
	catch (Error& err)
	{
		// these errors seem more deserving of an error box.
		errBox(err.trace());
	}
}


LRESULT AndorWindow::onCameraCalProgress( WPARAM wParam, LPARAM lParam )
{
	UINT picNum = lParam;
	if ( lParam == 0 )
	{
		// ???
		return NULL;
	}
	AndorRunSettings curSettings = andor.getAndorRunSettings( );
	if ( lParam == -1 )
	{
		// last picture.
		picNum = curSettings.totalPicsInExperiment();
	}
	// need to call this before acquireImageData().
	andor.updatePictureNumber( picNum );

	std::vector<std::vector<long>> picData;
	try
	{
		picData = andor.acquireImageData(mainWin->getComm());
	}
	catch ( Error& err )
	{
		mainWin->getComm( )->sendError( err.trace( ) );
		return NULL;
	}
	avgBackground.resize( picData.back( ).size( ) );
	for ( unsigned int i = 0; i < avgBackground.size( ); i++ )
	{
		avgBackground[i] += picData.back( )[i];
	}
	SmartDC sdc (this);
	try
	{
		if ( picNum % curSettings.picsPerRepetition == 0 )
		{
			int counter = 0;
			for ( auto data : picData )
			{
				std::pair<int, int> minMax;
				minMax = stats.update( data, counter, selectedPixel, curSettings.imageSettings.width(),
									   curSettings.imageSettings.height(), picNum / curSettings.picsPerRepetition,
									   curSettings.totalPicsInExperiment() / curSettings.picsPerRepetition );
				pics.drawPicture( sdc.get(), counter, data, minMax );
				pics.drawDongles( sdc.get (), selectedPixel, analysisHandler.getAnalysisLocs( ),
								  analysisHandler.getGrids( ), picNum, false );
				counter++;
			}
			timer.update( picNum / curSettings.picsPerRepetition, curSettings.repetitionsPerVariation,
						  curSettings.totalVariations, curSettings.picsPerRepetition );
		}
	}
	catch ( Error& err )
	{
		mainWin->getComm( )->sendError( err.trace( ) );
	}
	mostRecentPicNum = picNum;
	return 0;
}



LRESULT AndorWindow::onCameraProgress( WPARAM wParam, LPARAM lParam )
{
	currentPictureNum++;
	UINT picNum = currentPictureNum;
	if ( picNum % 2 == 1 )
	{
		mainThreadStartTimes.push_back( std::chrono::high_resolution_clock::now( ) );
	}
	if (lParam == 0)
	{
		// ???
		//return NULL;
	}
	AndorRunSettings curSettings = andor.getAndorRunSettings( );
	if ( lParam == -1 )
	{
		// last picture.
		//picNum = curSettings.totalPicsInExperiment();
	}
	if ( lParam != currentPictureNum && lParam != -1 )
	{
		if ( curSettings.acquisitionMode != AndorRunModes::mode::Video )
		{
			//mainWin->getComm ( )->sendError ( "WARNING: picture number reported by andor isn't matching the"
			//								  "camera window record?!?!?!?!?" );
		}
	}

	// need to call this before acquireImageData().
	andor.updatePictureNumber( picNum );
	
	std::vector<std::vector<long>> rawPicData;
	try
	{
		rawPicData = andor.acquireImageData(mainWin->getComm ());
	}
	catch (Error& err)
	{
		mainWin->getComm()->sendError( err.trace() );
		return NULL;
	}
	std::vector<std::vector<long>> calPicData( rawPicData.size( ) );
	if ( andorSettingsCtrl.getUseCal( ) && avgBackground.size() == rawPicData.front().size() )
	{
		for ( UINT picInc = 0; picInc < rawPicData.size(); picInc++ )
		{
			for ( UINT pixInc = 0; pixInc < rawPicData[picInc].size( ); pixInc++ )
			{
				calPicData[picInc].push_back( rawPicData[picInc][pixInc] - avgBackground[pixInc] );
			}
		}
	}
	else
	{
		calPicData = rawPicData;
	}


	if ( picNum % 2 == 1 )
	{
		imageGrabTimes.push_back( std::chrono::high_resolution_clock::now( ) );
	}
	{
		std::lock_guard<std::mutex> locker( imageQueueLock );
		imQueue.push_back ( { picNum, calPicData[ ( picNum - 1 ) % curSettings.picsPerRepetition ] } );
	}

	auto picsToDraw = andorSettingsCtrl.getImagesToDraw( calPicData );
	try
	{
		SmartDC sdc (this);
		if (realTimePic)
		{
			std::pair<int, int> minMax;
			// draw the most recent pic.
			minMax = stats.update( picsToDraw.back(), picNum % curSettings.picsPerRepetition, selectedPixel,
								   curSettings.imageSettings.width(), curSettings.imageSettings.height(),
								   picNum / curSettings.picsPerRepetition,
								   curSettings.totalPicsInExperiment() / curSettings.picsPerRepetition );
			pics.drawPicture( sdc.get (), picNum % curSettings.picsPerRepetition, picsToDraw.back(), minMax );

			timer.update( picNum / curSettings.picsPerRepetition, curSettings.repetitionsPerVariation,
						  curSettings.totalVariations, curSettings.picsPerRepetition );
		}
		else if (picNum % curSettings.picsPerRepetition == 0)
		{
			int counter = 0;
			for ( auto data : picsToDraw )
			{
				std::pair<int, int> minMax;
				minMax = stats.update ( data, counter, selectedPixel, curSettings.imageSettings.width ( ),
										curSettings.imageSettings.height ( ), picNum / curSettings.picsPerRepetition,
										curSettings.totalPicsInExperiment ( ) / curSettings.picsPerRepetition );
				if ( minMax.second > 50000 )
				{
					numExcessCounts++;
					if (numExcessCounts > 2)
					{
						// POTENTIALLY DANGEROUS TO CAMERA.
						// AUTO PAUSE THE EXPERIMENT. 
						// This can happen if a laser, particularly the axial raman laser, is left on during an image.
						// cosmic rays may occasionally trip it as well.
						commonFunctions::handleCommonMessage (ID_ACCELERATOR_F2, this, mainWin, scriptWin, this,
							auxWin, basWin);
						errBox ("EXCCESSIVE CAMERA COUNTS DETECTED!!!");
					}
				}
				else
				{
					numExcessCounts = 0;
				}
				pics.drawPicture( sdc.get (), counter, data, minMax );
				pics.drawDongles( sdc.get (), selectedPixel, analysisHandler.getAnalysisLocs(),
								  analysisHandler.getGrids(), picNum, analysisHandler.getDrawGridOption() );
				counter++;
			}
			timer.update( picNum / curSettings.picsPerRepetition, curSettings.repetitionsPerVariation,
						  curSettings.totalVariations, curSettings.picsPerRepetition );
		}
	}
	catch (Error& err)
	{
		mainWin->getComm()->sendError( err.trace() );
	}

	// write the data to the file.
	if (curSettings.acquisitionMode != AndorRunModes::mode::Video)
	{
		try
		{
			// important! write the original data, not the pic-to-draw, which can be a difference pic, or the calibrated
			// pictures, which can have the background subtracted.
			dataHandler.writeAndorPic( rawPicData[(picNum - 1) % curSettings.picsPerRepetition], 
									   curSettings.imageSettings );
		}
		catch (Error& err)
		{
			mainWin->getComm()->sendError( err.trace() );
			mainWin->handlePause ( );
		}
	}
	mostRecentPicNum = picNum;
	return 0;
}


void AndorWindow::wakeRearranger( )
{
	std::unique_lock<std::mutex> lock( rearrangerLock );
	rearrangerConditionVariable.notify_all( );
}


void AndorWindow::setMenuCheck ( UINT menuItem, UINT itemState )
{
	menu.CheckMenuItem ( menuItem, itemState );
}


void AndorWindow::handleSpecialLessThanMinSelection()
{
	if (specialLessThanMin)
	{
		specialLessThanMin = false;
		mainWin->checkAllMenus ( ID_PICTURES_LESS_THAN_MIN_SPECIAL, MF_UNCHECKED );
	}
	else
	{
		specialLessThanMin = true;
		mainWin->checkAllMenus ( ID_PICTURES_LESS_THAN_MIN_SPECIAL, MF_CHECKED );
	}
	pics.setSpecialLessThanMin(specialLessThanMin);
}


void AndorWindow::handleSpecialGreaterThanMaxSelection()
{
	if (specialGreaterThanMax)
	{
		specialGreaterThanMax = false;
		mainWin->checkAllMenus ( ID_PICTURES_GREATER_THAN_MAX_SPECIAL, MF_UNCHECKED );
	}
	else
	{
		specialGreaterThanMax = true;
		mainWin->checkAllMenus ( ID_PICTURES_GREATER_THAN_MAX_SPECIAL, MF_CHECKED );
	}
	pics.setSpecialGreaterThanMax(specialGreaterThanMax);
}


void AndorWindow::handleAutoscaleSelection()
{
	if (autoScalePictureData)
	{
		autoScalePictureData = false;
		mainWin->checkAllMenus ( ID_PICTURES_AUTOSCALEPICTURES, MF_UNCHECKED );
	}
	else
	{
		autoScalePictureData = true;
		mainWin->checkAllMenus ( ID_PICTURES_AUTOSCALEPICTURES, MF_CHECKED );
	}
	pics.setAutoScalePicturesOption(autoScalePictureData);
}


LRESULT AndorWindow::onCameraCalFinish( WPARAM wParam, LPARAM lParam )
{
	// notify the andor object that it is done.
	andor.onFinish( );
	andor.pauseThread( );
	andor.setCalibrating( false );
	justCalibrated = true;
	mainWin->getComm( )->sendColorBox( System::Camera, 'B' );
	andorSettingsCtrl.cameraIsOn( false );
	// normalize.
	for ( auto& p : avgBackground )
	{
		p /= 100.0;
	}
	// if auto cal is selected, always assume that the user was trying to start with F5.
	if ( andorSettingsCtrl.getAutoCal( ) ) 
	{
		PostMessageA( WM_COMMAND, MAKEWPARAM( ID_ACCELERATOR_F5, 0 ) );
	}
	return 0;
}


dataPoint AndorWindow::getMainAnalysisResult ( )
{
	return mostRecentAnalysisResult;
}

std::mutex& AndorWindow::getActivePlotMutexRef ( )
{
	return activePlotMutex;
}


void AndorWindow::cleanUpAfterExp ( )
{
	plotThreadActive = false;
	atomCrunchThreadActive = false;
	dataHandler.closeFile ( );
}


LRESULT AndorWindow::onCameraFinish( WPARAM wParam, LPARAM lParam )
{
	// notify the andor object that it is done.
	andor.onFinish();
	//andor.pauseThread();
	if (alerts.soundIsToBePlayed())
	{
		alerts.playSound();
	}
	mainWin->getComm()->sendColorBox( System::Camera, 'B' );
	mainWin->getComm()->sendStatus( "Camera has finished taking pictures and is no longer running.\r\n" );
	andorSettingsCtrl.cameraIsOn( false );
	// rearranger thread handles these right now.
	mainThreadStartTimes.clear();
	
	crunchFinTimes.clear( );
	crunchSeesTimes.clear( );
	mainWin->stopRearranger( );
	wakeRearranger( );
	{
		std::lock_guard<std::mutex> lock ( activePlotMutex );
		if ( activeDlgPlots.size ( ) != 0 )
		{
			mostRecentAnalysisResult = activeDlgPlots.back ( )->getMainAnalysisResult ( );
		}
	}
	return 0;
}


int AndorWindow::getMostRecentFid ( )
{
	return dataHandler.getDataFileNumber ( );
	
}

int AndorWindow::getPicsPerRep ( )
{
	return andorSettingsCtrl.getSettings ( ).andor.picsPerRepetition;
}

std::string AndorWindow::getMostRecentDateString ( )
{
	return dataHandler.getMostRecentDateString ( );
}


bool AndorWindow::wantsThresholdAnalysis ( )
{
	return analysisHandler.wantsThresholdAnalysis ( );
}

atomGrid AndorWindow::getMainAtomGrid ( )
{
	return analysisHandler.getAtomGrid ( 0 );
}


void AndorWindow::armCameraWindow()
{
	// expecting that settings have already been set...
	mainWin->getComm()->sendColorBox( System::Camera, 'Y');
	// turn some buttons off.
	andorSettingsCtrl.cameraIsOn( true );
	SmartDC sdc (this);
	pics.refreshBackgrounds( sdc.get ());
	stats.reset();
	analysisHandler.updateDataSetNumberEdit( dataHandler.getNextFileNumber() - 1 );
	mainWin->getComm()->sendColorBox(System::Camera, 'G');
}


bool AndorWindow::getCameraStatus()
{
	return andor.isRunning();
}


void AndorWindow::handleDblClick(NMHDR* info, LRESULT* lResult)
{
	try
	{
		analysisHandler.handleDoubleClick( &mainWin->getFonts( ), andorSettingsCtrl.getSettings( ).andor.picsPerRepetition );
	}
	catch ( Error& err )
	{
		mainWin->getComm( )->sendError( err.trace( ) );
	}
	mainWin->updateConfigurationSavedStatus( false );
}


void AndorWindow::listViewRClick( NMHDR* info, LRESULT* lResult )
{
	analysisHandler.handleRClick();
	mainWin->updateConfigurationSavedStatus( false );
}


void AndorWindow::OnLButtonUp(UINT stuff, CPoint loc)
{
	stopSound( );
}


void AndorWindow::stopSound( )
{
	alerts.stopSound( );
}


// pics looks up the location itself.
void AndorWindow::OnRButtonUp( UINT stuff, CPoint clickLocation )
{
	stopSound( );
	analysisHandler.saveGridParams( );
	try
	{
		SmartDC sdc (this);
		if (analysisHandler.buttonClicked())
		{
			coordinate loc = pics.getClickLocation(clickLocation);
			if (loc.row != -1)
			{
				analysisHandler.handlePictureClick(loc);
				pics.redrawPictures( sdc.get (), selectedPixel, analysisHandler.getAnalysisLocs(),
									 analysisHandler.getGrids(), false, mostRecentPicNum );
			}
		}
		else
		{
			coordinate box = pics.getClickLocation(clickLocation);
			if (box.row != -1)
			{
				selectedPixel = box;
				pics.redrawPictures( sdc.get (), selectedPixel, analysisHandler.getAnalysisLocs(),
									 analysisHandler.getGrids( ), false, mostRecentPicNum );
			}
		}
	}
	catch (Error& err)
	{
		if ( err.whatBare( ) != "click location not found" )
		{
			mainWin->getComm( )->sendError( err.trace( ) );
		}
	}
}


/*
 *
 */
void AndorWindow::passSetTemperaturePress()
{
	try
	{
		if ( andor.isRunning ( ) )
		{
			thrower ( "ERROR: the camera (thinks that it?) is running. You can't change temperature settings during camera "
					  "operation." );
		}
		andorSettingsCtrl.handleSetTemperaturePress();
		auto settings = andorSettingsCtrl.getSettings ( );
		andor.setSettings ( settings.andor );
		andor.setTemperature ( );
	}
	catch (Error& err)
	{
		mainWin->getComm()->sendError(err.trace());
	}
	mainWin->updateConfigurationSavedStatus( false );
}


/*
 *
 */
void AndorWindow::OnTimer(UINT_PTR id)
{
	auto temp = andor.getTemperature();
	andorSettingsCtrl.changeTemperatureDisplay(temp);
	OnPaint ();
}


/*
 *
 */
void AndorWindow::passTrigger()
{
	andorSettingsCtrl.handleTriggerChange(this);
	mainWin->updateConfigurationSavedStatus( false );
}


void AndorWindow::passAtomGridCombo( )
{
	try
	{
		analysisHandler.handleAtomGridCombo( );
	}
	catch ( Error& err )
	{
		mainWin->getComm( )->sendError( err.trace( ) );
	}
}

/*
	This func doesn't make much sense...
*/
void AndorWindow::passPictureSettings( UINT id )
{
	try
	{
		handlePictureSettings ( id );
		mainWin->updateConfigurationSavedStatus ( false );
	}
	catch ( Error& err )
	{
		mainWin->getComm ( )->sendError ( "Failed to handle picture Settings!\n" + err.trace ( ) );
	}
}


void AndorWindow::handlePictureSettings(UINT id)
{
	selectedPixel = { 0,0 };
	andorSettingsCtrl.handlePictureSettings(id);
	if (andorSettingsCtrl.getSettings().andor.picsPerRepetition == 1)
	{
		pics.setSinglePicture( this, andorSettingsCtrl.getSettings( ).andor.imageSettings );
	}
	else
	{
		pics.setMultiplePictures( this, andorSettingsCtrl.getSettings( ).andor.imageSettings,
								  andorSettingsCtrl.getSettings().andor.picsPerRepetition);
	}
	pics.resetPictureStorage();
	std::array<int, 4> nums = andorSettingsCtrl.getSettings( ).palleteNumbers;
	pics.setPalletes(nums);

	CRect rect;
	GetClientRect(&rect);
	OnSize(0, rect.right - rect.left, rect.bottom - rect.top);
	mainWin->updateConfigurationSavedStatus( false );
}

/*
Check that the camera is idle, or not aquiring pictures. Also checks that the data analysis handler isn't active.
*/
void AndorWindow::checkCameraIdle( )
{
	if ( andor.isRunning( ) )
	{
		thrower ( "Camera is already running! Please Abort to restart.\r\n" );
	}
	if ( analysisHandler.getLocationSettingStatus( ) )
	{
		thrower ( "Please finish selecting analysis points before starting the camera!\r\n" );
	}
	// make sure it's idle.
	try
	{
		andor.queryStatus( );
		if ( ANDOR_SAFEMODE )
		{
			thrower ( "DRV_IDLE" );
		}
	}
	catch ( Error& exception )
	{
		if ( exception.whatBare( ) != "DRV_IDLE" )
		{
			throwNested ( " while querying andor status to check if idle." );
		}
	}
}


BOOL AndorWindow::PreTranslateMessage(MSG* pMsg)
{
	for (UINT toolTipInc = 0; toolTipInc < tooltips.size(); toolTipInc++)
	{
		tooltips[toolTipInc]->RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void AndorWindow::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* scrollbar)
{
	try
	{
		SmartDC sdc (this);
		pics.handleScroll ( nSBCode, nPos, scrollbar, sdc.get ());
	}
	catch ( Error& err )
	{
		mainWin->getComm ( )->sendError ( err.what ( ) );
	}
}

// 3836, 1951
void AndorWindow::OnSize( UINT nType, int cx, int cy )
{
	stats.rearrange ( cx, cy, mainWin->getFonts ( ) );
	for (auto* plt : mainAnalysisPlots)
	{
		plt->rearrange (cx, cy, mainWin->getFonts ());
	}
	box.rearrange ( cx, cy, mainWin->getFonts ( ) );
	pics.rearrange ( cx, cy, mainWin->getFonts ( ) );
	try
	{
		SetRedraw ( false );
		auto settings = andorSettingsCtrl.getSettings ( ).andor;
		andorSettingsCtrl.rearrange ( settings.acquisitionMode, settings.triggerMode, cx, cy, mainWin->getFonts ( ) );
		alerts.rearrange ( settings.acquisitionMode, settings.triggerMode, cx, cy, mainWin->getFonts ( ) );
		analysisHandler.rearrange ( settings.acquisitionMode, settings.triggerMode, cx, cy, mainWin->getFonts ( ) );
		pics.setParameters ( settings.imageSettings );
		timer.rearrange ( settings.acquisitionMode, settings.triggerMode, cx, cy, mainWin->getFonts ( ) );
	}
	catch ( Error& err )
	{
		mainWin->getComm ( )->sendError ( "Error while getting Andor Camera settings for OnSize!" + err.trace() );
	}
	try
	{
		SmartDC sdc (this);
		pics.redrawPictures( sdc.get (), selectedPixel, analysisHandler.getAnalysisLocs( ), analysisHandler.getGrids(), false,
							 mostRecentPicNum );
	}
	catch ( Error& err )
	{
		mainWin->getComm( )->sendError( err.trace( ) );
	}	
	SetRedraw( );
	RedrawWindow( );
}


void AndorWindow::handleMasterConfigSave(std::stringstream& configStream)
{
	andorSettingsCtrl.handelSaveMasterConfig(configStream);
}


void AndorWindow::handleMasterConfigOpen(std::stringstream& configStream, Version version)
{
	mainWin->updateConfigurationSavedStatus( false );
	selectedPixel = { 0,0 }; 
	andorSettingsCtrl.handleOpenMasterConfig(configStream, version, this);
	pics.setParameters(andorSettingsCtrl.getSettings().andor.imageSettings);
	redrawPictures(true);
}


DataLogger& AndorWindow::getLogger()
{
	return dataHandler;
}


void AndorWindow::loadCameraCalSettings( AllExperimentInput& input )
{
	redrawPictures( false );
	try
	{
		checkCameraIdle( );
	}
	catch ( Error& err)
	{
		mainWin->getComm( )->sendError( err.trace( ) );
	}
	SmartDC sdc (this);
	pics.refreshBackgrounds( sdc.get ());
	// I used to mandate use of a button to change image parameters. Now I don't have the button and just always 
	// update at this point.
	readImageParameters( );
	pics.setNumberPicturesActive( 1 );
	// biggest check here, camera settings includes a lot of things.
	andorSettingsCtrl.checkIfReady( );
	input.AndorSettings = andorSettingsCtrl.getCalibrationSettings( ).andor;
	// reset the image which is about to be calibrated.
	avgBackground.clear( );
	/// start the camera.
	andor.setSettings( input.AndorSettings );
	andor.setCalibrating(true);
}

AndorCameraCore& AndorWindow::getCamera ( )
{
	return andor;
}

void AndorWindow::prepareAndor( AllExperimentInput& input )
{
	currentPictureNum = 0;
	input.includesAndorRun = true;
	redrawPictures( false );
	checkCameraIdle( );
	SmartDC sdc (this);
	pics.refreshBackgrounds(sdc.get ());
	readImageParameters( );
	pics.setNumberPicturesActive( andorSettingsCtrl.getSettings().andor.picsPerRepetition );
	// this is a bit awkward at the moment.
	andorSettingsCtrl.setRepsPerVariation(mainWin->getRepNumber());
	UINT varNumber = auxWin->getTotalVariationNumber();
	if (varNumber == 0)
	{
		// this means that the user isn't varying anything, so effectively this should be 1.
		varNumber = 1;
	}
	andorSettingsCtrl.setVariationNumber(varNumber);
	// biggest check here, camera settings includes a lot of things.
	andorSettingsCtrl.checkIfReady();
	pics.setSoftwareAccumulationOptions ( andorSettingsCtrl.getSoftwareAccumulationOptions() );
	input.AndorSettings = andorSettingsCtrl.getSettings().andor;
}


void AndorWindow::prepareAtomCruncher( AllExperimentInput& input )
{
	input.cruncherInput = new atomCruncherInput;
	input.cruncherInput->plotterActive = plotThreadActive;
	input.cruncherInput->imageDims = andorSettingsCtrl.getSettings( ).andor.imageSettings;
	atomCrunchThreadActive = true;
	input.cruncherInput->plotterNeedsImages = input.masterInput->plotterInput->needsCounts;
	input.cruncherInput->cruncherThreadActive = &atomCrunchThreadActive;
	skipNext = false;
	input.cruncherInput->skipNext = &skipNext;
	input.cruncherInput->imQueue = &imQueue;
	// options
	if ( input.masterInput )
	{
		input.cruncherInput->rearrangerActive = input.masterInput->rerngGuiForm.active;
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
	input.cruncherInput->grids = analysisHandler.getGrids( );
	// reinitialize
	plotterPictureQueue.clear( );
	plotterAtomQueue.clear( );
	input.cruncherInput->plotterImageQueue = &plotterPictureQueue;
	input.cruncherInput->plotterAtomQueue = &plotterAtomQueue;
	rearrangerAtomQueue.clear( );
	input.cruncherInput->rearrangerAtomQueue = &rearrangerAtomQueue;
	input.cruncherInput->thresholds = andorSettingsCtrl.getSettings( ).thresholds;
	input.cruncherInput->picsPerRep = andorSettingsCtrl.getSettings().andor.picsPerRepetition;	
	input.cruncherInput->catchPicTime = &crunchSeesTimes;
	input.cruncherInput->finTime = &crunchFinTimes;
	input.cruncherInput->atomThresholdForSkip = mainWin->getMainOptions( ).atomThresholdForSkip;
	input.cruncherInput->rearrangerConditionWatcher = &rearrangerConditionVariable;
}


void AndorWindow::startAtomCruncher(AllExperimentInput& input)
{
	UINT atomCruncherID;
	atomCruncherThreadHandle = (HANDLE)_beginthreadex( 0, 0, AndorWindow::atomCruncherProcedure,
													   (void*)input.cruncherInput, 0, &atomCruncherID );
}


bool AndorWindow::wantsAutoPause( )
{
	return alerts.wantsAutoPause( );
}

std::atomic<bool>& AndorWindow::getPlotThreadActiveRef ( )
{
	return plotThreadActive;
}

std::atomic<HANDLE>& AndorWindow::getPlotThreadHandleRef ( )
{
	return plotThreadHandle;
}

void AndorWindow::preparePlotter( AllExperimentInput& input )
{
	/// start the plotting thread.
	plotThreadActive = true;
	plotThreadAborting = false;
	imQueue.clear();
	plotterAtomQueue.clear();
	input.masterInput->plotterInput = new realTimePlotterInput( analysisHandler.getPlotTime() );
	auto& pltInput = input.masterInput->plotterInput;
	pltInput->plotPens = mainWin->getPlotPens ( );
	pltInput->plotBrushes = mainWin->getPlotBrushes ( );
	pltInput->plotFont = mainWin->getPlotFont ( );
	pltInput->plotParentWindow = this;
	pltInput->cameraSettings = andorSettingsCtrl.getSettings ( );
	pltInput->aborting = &plotThreadAborting;
	pltInput->active = &plotThreadActive;
	pltInput->imQueue = &plotterPictureQueue;
	pltInput->imageShape = andorSettingsCtrl.getSettings().andor.imageSettings;
	pltInput->picsPerVariation = mainWin->getRepNumber() * andorSettingsCtrl.getSettings().andor.picsPerRepetition;
	pltInput->variations = auxWin->getTotalVariationNumber();
	pltInput->picsPerRep = andorSettingsCtrl.getSettings().andor.picsPerRepetition;
	pltInput->alertThreshold = alerts.getAlertThreshold();
	pltInput->wantAtomAlerts = alerts.wantsAtomAlerts();
	pltInput->comm = mainWin->getComm();
	pltInput->plotLock = &plotLock;
	pltInput->numberOfRunsToAverage = 5;
	pltInput->plottingFrequency = analysisHandler.getPlotFreq( );
	pltInput->atomQueue = &plotterAtomQueue;
	analysisHandler.fillPlotThreadInput( pltInput );

	// remove old plots that aren't trying to sustain.
	activeDlgPlots.erase( std::remove_if( activeDlgPlots.begin(), activeDlgPlots.end(), PlotDialog::removeQuery ), 
					   activeDlgPlots.end() );	
	std::vector<double> dummyKey;
	dummyKey.resize ( input.masterInput->numVariations );
	pltInput->key = dummyKey;
	UINT count = 0;
	for ( auto& e : pltInput->key )
	{
		e = count++;
	}
	UINT mainPlotInc = 0;
	for ( auto plotParams : pltInput->plotInfo )
	{
		// Create vector of data to be shared between plotter and data analysis handler. 
		std::vector<pPlotDataVec> data;
		// assume 1 data set...
		UINT numDataSets = 1;
		// +1 for average line
		UINT numLines = numDataSets * ( pltInput->grids[plotParams.whichGrid].height
										* pltInput->grids[plotParams.whichGrid].width + 1 );
		data.resize( numLines );
		for ( auto& line : data )
		{
			line = pPlotDataVec( new plotDataVec( pltInput->key.size( ), { 0, -1, 0 } ) );
			line->resize( pltInput->key.size( ) );
			// initialize x axis for all data sets.
			UINT count = 0;
			for ( auto& keyItem : pltInput->key )
			{
				line->at( count++ ).x = keyItem;
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
				PlotDialog* plot = new PlotDialog (data, style, mainWin->getPlotPens (),
					mainWin->getPlotFont (), mainWin->getPlotBrushes (),
					analysisHandler.getPlotTime (), andorSettingsCtrl.getSettings ().thresholds[0],
					plotIds,
					plotParams.name);
				plot->Create (IDD_PLOT_DIALOG, this);
				plot->ShowWindow (SW_SHOW);
				activeDlgPlots.push_back (plot);
				pltInput->dataArrays.push_back (data);
				usedDlg = true;
				break;
			}
			if (mainAnalysisPlots[mainPlotInc]->wantsSustain ())
			{
				// skip this one then.
				mainPlotInc++;
				continue;
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

void AndorWindow::OnPaint ()
{
	CDialog::OnPaint ();
	CRect size;
	GetClientRect (&size);
	SmartDC sdc (this);
	// for some reason I suddenly started needing to do this. I know that memDC redraws the background, but it used to 
	// work without this and I don't know what changed. I used to do:
	sdc.get()->SetBkColor (_myRGBs["Main-Bkgd"]);
	long width = size.right - size.left, height = size.bottom - size.top;
	// each dc gets initialized with the rect for the corresponding plot. That way, each dc only overwrites the area 
	// for a single plot.
	for (auto& plt : mainAnalysisPlots)
	{
		plt->setCurrentDims (width, height);
		plt->drawPlot (sdc.get (), _myBrushes["Main-Bkgd"], _myBrushes["Interactable-Bkgd"]);
	}
}


bool AndorWindow::wantsNoMotAlert ( )
{
	if ( cameraIsRunning() )
	{
		return alerts.wantsMotAlerts ( );
	}
	else
	{
		return false;
	}
}


UINT AndorWindow::getNoMotThreshold ( )
{
	return alerts.getAlertThreshold ( );
}

void AndorWindow::startPlotterThread( AllExperimentInput& input )
{
	bool gridHasBeenSet = false;
	auto& pltInput = input.masterInput->plotterInput;
	for ( auto gridInfo : pltInput->grids )
	{
		if ( !(gridInfo.topLeftCorner == coordinate( 0, 0 )) )
		{
			gridHasBeenSet = true;
			break;
		}
	}
	UINT plottingThreadID;
	if ((!gridHasBeenSet && pltInput->analysisLocations.size() == 0)
		 || pltInput->plotInfo.size() == 0)
	{
		plotThreadActive = false;
	}
	else
	{
		if ( input.AndorSettings.totalPicsInExperiment() * pltInput->analysisLocations.size()
			 / pltInput->plottingFrequency > 1000 )
		{
			infoBox( "Warning: The number of pictures * points to analyze in the experiment is very large,"
					 " and the plotting period is fairly small. Consider increasing the plotting period. " );
		}
		// start the plotting thread
		plotThreadActive = true;
		plotThreadHandle = (HANDLE)_beginthreadex( 0, 0, DataAnalysisControl::plotterProcedure, (void*) pltInput,
												   0, &plottingThreadID );
	}
}

// this thread has one purpose: watch the image vector thread for new images, determine where atoms are 
// (the atom crunching part), and pass them to the threads waiting on atom info.
// should consider modifying so that it can use an array of locations. At the moment doesn't.
// "Crunching" here is meant to be reminiscent of number-crunching. It's super-repeatative.
UINT __stdcall AndorWindow::atomCruncherProcedure(void* inputPtr)
{
	atomCruncherInput* input = (atomCruncherInput*)inputPtr; 
	auto gridSize = input->grids.size ( );
	if ( input->grids[0].topLeftCorner == coordinate( 0, 0 ) || gridSize == 0 )
	{
		return 0;
	}
	std::vector<std::vector<long>> monitoredPixelIndecies( gridSize );
	// preparing for the crunching
	for (auto gridInc : range(gridSize))
	{
		auto& grid = input->grids[ gridInc ];
		for (auto columnInc : range(grid.width))
		{
			for (auto rowInc : range(grid.height))
			{
				ULONG pixelRow = ( grid.topLeftCorner.row - 1) + rowInc * grid.pixelSpacing;
				ULONG pixelColumn = ( grid.topLeftCorner.column - 1)  + columnInc * grid.pixelSpacing;
				if ( pixelRow >= input->imageDims.height() || pixelColumn >= input->imageDims.width() )
				{
					errBox( "atom grid appears to include pixels outside the image frame! Not allowed, seen by atom "
							"cruncher thread" );
					return 0;
				}
				int index = ((input->imageDims.height() - 1 - pixelRow) * input->imageDims.width() + pixelColumn);
				if ( index >= input->imageDims.width() * input->imageDims.height() )
				{
					// shouldn't happen after I finish debugging.
					errBox( "Math error! Somehow, the pixel indexes appear within bounds, but the calculated index"
							" is larger than the image is!  (A low level bug, this shouldn't happen)" );
					return 0;
				}
				monitoredPixelIndecies[gridInc].push_back( index );
			}
		}
	}
	for ( auto picThresholds : input->thresholds )
	{
		if ( picThresholds.size ( ) != 1 && picThresholds.size ( ) != input->grids[ 0 ].numAtoms ( ) )
		{
			errBox ( "the list of thresholds isn't size 1 (constant) or the size of the number of atoms in the "
					 "first grid! Size is " + str(picThresholds.size()) + "and grid size is " + 
					 str(input->grids[ 0 ].numAtoms ( )) );
			return 0;
		}
	}
	UINT imageCount = 0;   
	// loop watching the image queue.
	while (*input->cruncherThreadActive || input->imQueue->size() != 0)
	{
		// if no images wait until images. Should probably change to be event based, but want this to be fast...
		if (input->imQueue->size () == 0)
		{
			continue;
		}
		if ( imageCount % 2 == 0 ) 
		{
			input->catchPicTime->push_back( chronoClock::now( ) );
		}
		// tempImagePixels[grid][pixel]; only contains the counts for the pixels being monitored.
		imageQueue tempImagePixels( gridSize );
		// tempAtomArray[grid][pixel]; only contains the boolean true/false of whether an atom passed a threshold or not. 
		atomQueue tempAtomArray( gridSize );
		for (auto gridInc : range(gridSize))
		{
			tempAtomArray[gridInc].image = std::vector<bool>( monitoredPixelIndecies[gridInc].size( ) );
			tempImagePixels[gridInc].image = std::vector<long>( monitoredPixelIndecies[gridInc].size( ) );
		}
		for ( auto gridInc : range(gridSize) )
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
				tempImagePixels[ gridInc ].repNum = ( *input->imQueue )[ 0 ].repNum;
				tempAtomArray[ gridInc ].repNum = ( *input->imQueue )[ 0 ].repNum;
				///*** Deal with 1st element entirely first, as this is important for the rearranger thread and the 
				/// load-skip both of which are very time-sensitive.
				for ( auto pixelIndex : monitoredPixelIndecies[gridInc] )
				{
					if (pixelIndex > (*input->imQueue)[0].image.size ())
					{
						/*errBox ("Monitored pixel indecies included pixel out of image?!?! pixel: " + str(pixelIndex) 
							+ ", size: " + str((*input->imQueue)[0].image.size()) );*/
					}
					else
					{
						tempImagePixels[gridInc].image[count++] = (*input->imQueue)[0].image[pixelIndex];
					}
				}
			}
			count = 0;
			for ( auto& pix : tempImagePixels[gridInc].image )
			{
				auto& picThresholds = input->thresholds[ imageCount % input->picsPerRep ];
				if ( pix >= picThresholds[count % picThresholds.size() ] )
				{
					tempAtomArray[gridInc].image[count] = true;
				}
				count++;
			}
			// explicitly deal with the rearranger thread and load skip as soon as possible, these are time-critical.
			if ( gridInc == 0 )
			{
				if ( input->rearrangerActive ) ///
				{
					// copies the array if first pic of rep. Only looks at first picture because its rearranging. Could change
					// if we need to do funny experiments, just need to change rearranger handling.
					if ( imageCount % input->picsPerRep == 0 )
					{
						{
							std::lock_guard<std::mutex> locker( *input->rearrangerLock );
							(*input->rearrangerAtomQueue).push_back( tempAtomArray[0] );
							input->rearrangerConditionWatcher->notify_all( );
						}
						input->finTime->push_back( chronoClock::now( ) );
					}
				}
				// if last picture of repetition, check for loadskip condition.
				if ( imageCount % input->picsPerRep == input->picsPerRep - 1 ) ///
				{
					UINT numAtoms = std::accumulate( tempAtomArray[0].image.begin( ), tempAtomArray[0].image.end( ), 0 );
					*input->skipNext = (numAtoms >= input->atomThresholdForSkip);
				}
			}
		}
		if ( input->plotterActive ) 
		{
			// copies the array. Right now I'm assuming that the thread always needs atoms, which is not a good 
			// assumption potentially...
			std::lock_guard<std::mutex> locker( *input->plotLock );
			(*input->plotterAtomQueue).push_back( tempAtomArray );
			if ( input->plotterNeedsImages )
			{
				(*input->plotterImageQueue).push_back( tempImagePixels );
			}
		}
		imageCount++;
		std::lock_guard<std::mutex> locker( *input->imageQueueLock );
		// if no images wait until images. Should probably change to be event based, but want this to be fast...
		if (input->imQueue->size () != 0)
		{
			(*input->imQueue).erase ((*input->imQueue).begin ());
		}
	}
	return 0;
}


std::string AndorWindow::getStartMessage()
{
	// get selected plots
	auto andrSttngs = andorSettingsCtrl.getSettings ().andor;
	std::vector<std::string> plots = analysisHandler.getActivePlotList();
	imageParameters currentImageParameters = andrSttngs.imageSettings;
	bool errCheck = false;
	for (UINT plotInc = 0; plotInc < plots.size(); plotInc++)
	{
		PlottingInfo tempInfoCheck(PLOT_FILES_SAVE_LOCATION + "\\" + plots[plotInc] + ".plot");
		if (tempInfoCheck.getPicNumber() != andrSttngs.picsPerRepetition)
		{
			thrower ( ": one of the plots selected, " + plots[plotInc] + ", is not built for the currently "
					 "selected number of pictures per experiment. Please revise either the current setting or the plot"
					 " file." );
		}
		tempInfoCheck.setGroups( analysisHandler.getAnalysisLocs() );
		std::vector<std::pair<UINT, UINT>> plotLocations = tempInfoCheck.getAllPixelLocations();
	}
	std::string dialogMsg;
	dialogMsg = "Camera Parameters:\r\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\r\n";
	dialogMsg += "Current Camera Temperature Setting:\r\n\t" + str(
		andrSttngs.temperatureSetting ) + "\r\n";
	dialogMsg += "Exposure Times: ";
	for (auto& time : andrSttngs.exposureTimes)
	{
		dialogMsg += str( time * 1000 ) + ", ";
	}
	dialogMsg += "\r\n";
	dialogMsg += "Image Settings:\r\n\t" + str( currentImageParameters.left ) + " - " + str( currentImageParameters.right ) + ", "
		+ str( currentImageParameters.bottom ) + " - " + str( currentImageParameters.top ) + "\r\n";
	dialogMsg += "\r\n";
	dialogMsg += "Kintetic Cycle Time:\r\n\t" + str(andrSttngs.kineticCycleTime ) + "\r\n";
	dialogMsg += "Pictures per Repetition:\r\n\t" + str(andrSttngs.picsPerRepetition ) + "\r\n";
	dialogMsg += "Repetitions per Variation:\r\n\t" + str(andrSttngs.totalPicsInVariation() ) + "\r\n";
	dialogMsg += "Variations per Experiment:\r\n\t" + str(andrSttngs.totalVariations ) + "\r\n";
	dialogMsg += "Total Pictures per Experiment:\r\n\t" + str(andrSttngs.totalPicsInExperiment() ) + "\r\n";
	
	dialogMsg += "Real-Time Atom Detection Thresholds:\r\n\t";
	UINT count = 0;
	for (auto& picThresholds : andorSettingsCtrl.getSettings().thresholds)
	{
		dialogMsg += "Pic " + str ( count ) + " thresholds: ";
		for ( auto thresh : picThresholds )
		{
			dialogMsg += str ( thresh ) + ", ";
		}
		dialogMsg += "\r\n";
		count++;
	}

	dialogMsg += "\r\nReal-Time Plots:\r\n";
	for (UINT plotInc = 0; plotInc < plots.size(); plotInc++)
	{
		dialogMsg += "\t" + plots[plotInc] + "\r\n";
	}

	return dialogMsg;
}


void AndorWindow::fillMasterThreadInput( ExperimentThreadInput* input )
{
	currentPictureNum = 0;
	// starting a not-calibration, so reset this.
	justCalibrated = false;
	input->atomQueueForRearrangement = &rearrangerAtomQueue;
	input->rearrangerLock = &rearrangerLock;
	input->andorsImageTimes = &imageTimes;
	input->grabTimes = &imageGrabTimes;
	input->analysisGrid = analysisHandler.getAtomGrid( 0 );
	input->conditionVariableForRerng = &rearrangerConditionVariable;
	input->runAndor = true;
}


void AndorWindow::setTimerText( std::string timerText )
{
	timer.setTimerDisplay( timerText );
}


void AndorWindow::OnCancel()
{
	try
	{
		passCommonCommand( ID_FILE_MY_EXIT );
	}
	catch (Error& exception)
	{
		errBox( exception.trace() );
	}
}


cToolTips AndorWindow::getToolTips()
{
	return tooltips;
}


BOOL AndorWindow::OnInitDialog ( )
{
	SetWindowText ( "Andor Camera Control" );
	// don't redraw until the first OnSize.
	SetRedraw ( false );
	andor.initializeClass ( mainWin->getComm ( ), &imageTimes );
	cameraPositions positions;
	// all of the initialization functions increment and use the id, so by the end it will be 3000 + # of controls.
	int id = 3000;
	positions.sPos = { 0, 0 };
	box.initialize ( positions.sPos, id, this, 480, tooltips );
	positions.videoPos = positions.amPos = positions.seriesPos = positions.sPos;
	alerts.alertMainThread ( 0 );
	alerts.initialize ( positions, this, false, id, tooltips );
	analysisHandler.initialize ( positions, id, this, tooltips, false );
	andorSettingsCtrl.initialize ( positions, id, this, tooltips );
	POINT position = { 480, 0 };
	stats.initialize ( position, this, id, tooltips );
	for (auto pltInc : range (6))
	{
		std::vector<pPlotDataVec> nodata(0);
		mainAnalysisPlots.push_back (new PlotCtrl (nodata, plotStyle::ErrorPlot, mainWin->getPlotPens (),
			mainWin->getPlotFont (), mainWin->getPlotBrushes (), { 0,0,0,0 }, "INACTIVE", false, false));
		mainAnalysisPlots.back ()->init (position, 315, 130, this, plotIds++);
	}
	positions.sPos = { 797, 0 };
	timer.initialize ( positions, this, false, id, tooltips );
	position = { 797, 40 };
	pics.initialize ( position, this, id, _myBrushes[ "Dark Green" ], 550 * 2, 460 * 2 + 5, 
					 { IDC_PICTURE_1_MIN_EDIT, IDC_PICTURE_1_MAX_EDIT,
					   IDC_PICTURE_2_MIN_EDIT, IDC_PICTURE_2_MAX_EDIT,
					   IDC_PICTURE_3_MIN_EDIT, IDC_PICTURE_3_MAX_EDIT, 
					   IDC_PICTURE_4_MIN_EDIT, IDC_PICTURE_4_MAX_EDIT } );
	// end of literal initialization calls
	pics.setSinglePicture( this, andorSettingsCtrl.getSettings( ).andor.imageSettings );
	andor.setSettings( andorSettingsCtrl.getSettings().andor );
	menu.LoadMenu( IDR_MAIN_MENU );
	SetMenu( &menu );
	SetTimer( NULL, 2000, NULL );
	CRect rect;
	GetWindowRect( &rect );
	OnSize( 0, rect.right - rect.left, rect.bottom - rect.top );
	return TRUE;
}

void AndorWindow::setDataType( std::string dataType )
{
	stats.updateType( dataType );
}


void AndorWindow::redrawPictures( bool andGrid )
{
	try
	{
		SmartDC sdc (this);
		pics.refreshBackgrounds( sdc.get ());
		if (andGrid)
		{
			pics.drawGrids( sdc.get ());
		}
	}
	catch (Error& err)
	{
		mainWin->getComm()->sendError( err.trace() );
	}
	// currently don't attempt to redraw previous picture data.
}


HBRUSH AndorWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	CBrush * result;
	int num = pWnd->GetDlgCtrlID();

	result = andorSettingsCtrl.handleColor(num, pDC );
	HBRUSH res = *result;
	if (res) { return res; }
	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{			
			CBrush* result = box.handleColoring(num, pDC);
			if (result)
			{
				return *result;
			}
			else
			{
				pDC->SetTextColor( _myRGBs["Text"] );
				pDC->SetBkColor( _myRGBs["Static-Bkgd"] );
				return *_myBrushes["Static-Bkgd"];
			}
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor( _myRGBs["AndorWin-Text"]);
			pDC->SetBkColor( _myRGBs["Interactable-Bkgd"]);
			return *_myBrushes["Interactable-Bkgd"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor( _myRGBs["AndorWin-Text"]);
			pDC->SetBkColor( _myRGBs["Interactable-Bkgd"]);
			return *_myBrushes["Interactable-Bkgd"];
		}
		default:
		{
			return *_myBrushes["Main-Bkgd"];
		}
	}
}


std::atomic<bool>* AndorWindow::getSkipNextAtomic( )
{
	return &skipNext;
}


void AndorWindow::stopPlotter( )
{
	plotThreadAborting = true;
}


void AndorWindow::passCommonCommand(UINT id)
{
	try
	{
		commonFunctions::handleCommonMessage( id, this, mainWin, scriptWin, this, auxWin, basWin );
	}
	catch (Error& err)
	{
		// catch any extra errors that handleCommonMessage doesn't explicitly handle.
		errBox( err.trace() );
	}
}


// this is typically a little redundant to call, but can use to make sure things are set to off.
void AndorWindow::assertOff()
{
	andorSettingsCtrl.cameraIsOn(false);
	plotThreadActive = false;
	atomCrunchThreadActive = false;
}


void AndorWindow::readImageParameters()
{
	selectedPixel = { 0,0 };
	try
	{
		redrawPictures(false);
		imageParameters parameters = andorSettingsCtrl.getSettings( ).andor.imageSettings;
		pics.setParameters( parameters );
	}
	catch (Error& exception)
	{
		Communicator* comm = mainWin->getComm();
		comm->sendColorBox( System::Camera, 'R' );
		comm->sendError( exception.trace() + "\r\n" );
	}
	SmartDC sdc (this);
	pics.drawGrids(sdc.get ());
}

void AndorWindow::changeBoxColor(systemInfo<char> colors)
{
	box.changeColor(colors);
}