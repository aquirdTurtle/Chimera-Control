#include "stdafx.h"
#include "commonFunctions.h"
#include "CameraSettingsControl.h"
#include "PlotCtrl.h"
#include "PlottingInfo.h"
#include "AuxiliaryWindow.h"
#include "CameraWindow.h"
#include "MainWindow.h"
#include "realTimePlotterInput.h"
#include "MasterThreadInput.h"
#include "ATMCD32D.H"
#include <numeric>
#include "Thrower.h"

CameraWindow::CameraWindow() : CDialog(), 
								CameraSettings(&Andor), 
								dataHandler(DATA_SAVE_LOCATION),
								Andor( ANDOR_SAFEMODE )
{
};


IMPLEMENT_DYNAMIC(CameraWindow, CDialog)


BEGIN_MESSAGE_MAP(CameraWindow, CDialog)
	ON_WM_CTLCOLOR( )
	ON_WM_SIZE( )
	ON_WM_TIMER( )
	ON_WM_VSCROLL( )
	ON_WM_MOUSEMOVE( )

	ON_COMMAND_RANGE( MENU_ID_RANGE_BEGIN, MENU_ID_RANGE_END, &CameraWindow::passCommonCommand )
	ON_COMMAND_RANGE( PICTURE_SETTINGS_ID_START, PICTURE_SETTINGS_ID_END, &CameraWindow::passPictureSettings )
	ON_CONTROL_RANGE( CBN_SELENDOK, PICTURE_SETTINGS_ID_START, PICTURE_SETTINGS_ID_END, 
					  &CameraWindow::passPictureSettings )
	ON_CBN_SELENDOK( IDC_ATOM_GRID_COMBO, &CameraWindow::passAtomGridCombo )
	// these ids all go to the same function.
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_1_MIN_EDIT, IDC_PICTURE_1_MIN_EDIT, &CameraWindow::handlePictureEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_1_MAX_EDIT, IDC_PICTURE_1_MAX_EDIT, &CameraWindow::handlePictureEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_2_MIN_EDIT, IDC_PICTURE_2_MIN_EDIT, &CameraWindow::handlePictureEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_2_MAX_EDIT, IDC_PICTURE_2_MAX_EDIT, &CameraWindow::handlePictureEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_3_MIN_EDIT, IDC_PICTURE_3_MIN_EDIT, &CameraWindow::handlePictureEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_3_MAX_EDIT, IDC_PICTURE_3_MAX_EDIT, &CameraWindow::handlePictureEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_4_MIN_EDIT, IDC_PICTURE_4_MIN_EDIT, &CameraWindow::handlePictureEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_4_MAX_EDIT, IDC_PICTURE_4_MAX_EDIT, &CameraWindow::handlePictureEditChange )
	// 
	ON_COMMAND( IDC_SET_TEMPERATURE_BUTTON, &CameraWindow::passSetTemperaturePress)
	ON_COMMAND( IDOK, &CameraWindow::catchEnter)
	ON_COMMAND( IDC_SET_ANALYSIS_LOCATIONS, &CameraWindow::passManualSetAnalysisLocations)
	ON_COMMAND( IDC_SET_GRID_CORNER, &CameraWindow::passSetGridCorner)
	ON_COMMAND( IDC_DEL_GRID_BUTTON, &CameraWindow::passDelGrid)
	ON_COMMAND( IDC_CAMERA_CALIBRATION_BUTTON, &CameraWindow::calibrate)

	ON_CBN_SELENDOK( IDC_TRIGGER_COMBO, &CameraWindow::passTrigger )
	ON_CBN_SELENDOK( IDC_CAMERA_MODE_COMBO, &CameraWindow::passCameraMode )

	ON_REGISTERED_MESSAGE( eCameraFinishMessageID, &CameraWindow::onCameraFinish )
	ON_REGISTERED_MESSAGE( eCameraCalFinMessageID, &CameraWindow::onCameraCalFinish )
	ON_REGISTERED_MESSAGE( eCameraProgressMessageID, &CameraWindow::onCameraProgress )
	ON_REGISTERED_MESSAGE( eCameraCalProgMessageID, &CameraWindow::onCameraCalProgress )
	
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONUP()

	ON_NOTIFY(NM_RCLICK, IDC_PLOTTING_LISTVIEW, &CameraWindow::listViewRClick)
	ON_NOTIFY(NM_DBLCLK, IDC_PLOTTING_LISTVIEW, &CameraWindow::handleDblClick)
	ON_EN_KILLFOCUS(IDC_EM_GAIN_EDIT, &CameraWindow::handleEmGainChange )
	ON_CONTROL_RANGE(EN_KILLFOCUS, IDC_IMAGE_DIMS_START, IDC_IMAGE_DIMS_END, &CameraWindow::handleImageDimsEdit )

END_MESSAGE_MAP()


bool CameraWindow::wasJustCalibrated( )
{
	return justCalibrated;
}


bool CameraWindow::wantsAutoCal( )
{
	return CameraSettings.getAutoCal( );
}


void CameraWindow::calibrate( )
{
	commonFunctions::calibrateCameraBackground(scriptingWindowFriend, mainWindowFriend, this, auxWindowFriend );
}


void CameraWindow::passDelGrid( )
{
	try
	{
		analysisHandler.handleDeleteGrid( );
	}
	catch ( Error& err )
	{
		mainWindowFriend->getComm( )->sendError( err.what( ) );
	}
}


void CameraWindow::writeVolts( UINT currentVoltNumber, std::vector<float64> data )
{
	try
	{
		dataHandler.writeVolts( currentVoltNumber, data );
	}
	catch ( Error& err )
	{
		mainWindowFriend->getComm( )->sendError( err.what( ) );
	}
}


void CameraWindow::OnMouseMove( UINT thing, CPoint point )
{
	try
	{
		pics.handleMouse( point );
	}
	catch ( Error& err )
	{
		errBox( "Error! " + err.whatStr( ) );
	}
}


void CameraWindow::handleImageDimsEdit( UINT id )
{
	pics.setParameters( CameraSettings.getSettings().andor.imageSettings );
	CDC* dc = GetDC( );
	try
	{
		pics.redrawPictures( dc, selectedPixel, analysisHandler.getAnalysisLocs( ), analysisHandler.getGrids(), true,
							 mostRecentPicNum );
	}
	catch ( Error& err )
	{
		mainWindowFriend->getComm( )->sendError( err.what( ) );
	}
	ReleaseDC( dc );
}


void CameraWindow::handleEmGainChange()
{
	try
	{
		CameraSettings.setEmGain( );
	}
	catch ( Error err )
	{
		mainWindowFriend->getComm( )->sendError( err.what( ) );
	}
}


std::string CameraWindow::getSystemStatusString()
{
	std::string statusStr; 
	statusStr = "\nAndor Camera:\n";
	if (!ANDOR_SAFEMODE)
	{
		statusStr += "\tCode System is Active!\n";
		statusStr += "\t" + Andor.getSystemInfo();
	}
	else
	{
		statusStr += "\tCode System is disabled! Enable in \"constants.h\"\n";
	}
	return statusStr;
}


void CameraWindow::handleNewConfig( std::ofstream& newFile )
{
	CameraSettings.handleNewConfig( newFile );
	pics.handleNewConfig( newFile );
	analysisHandler.handleNewConfig( newFile );
}


void CameraWindow::handleSaveConfig(std::ofstream& saveFile)
{
	CameraSettings.handleSaveConfig(saveFile);
	pics.handleSaveConfig(saveFile);
	analysisHandler.handleSaveConfig( saveFile );
}


void CameraWindow::handleOpeningConfig(std::ifstream& configFile, Version ver )
{
	// I could and perhaps should further subdivide this up.
	CameraSettings.handleOpenConfig(configFile, ver );
	pics.handleOpenConfig(configFile, ver );
	analysisHandler.handleOpenConfig( configFile, ver );
	if ( CameraSettings.getSettings( ).andor.picsPerRepetition == 1 )
	{
		pics.setSinglePicture( this, CameraSettings.getSettings( ).andor.imageSettings );
	}
	else
	{
		pics.setMultiplePictures( this, CameraSettings.getSettings().andor.imageSettings, 
								  CameraSettings.getSettings( ).andor.picsPerRepetition );
	}
	pics.resetPictureStorage( );
	std::array<int, 4> nums = CameraSettings.getSettings( ).palleteNumbers;
	pics.setPalletes( nums );

	CRect rect;
	GetWindowRect( &rect );
	OnSize( 0, rect.right - rect.left, rect.bottom - rect.top );
}


void CameraWindow::loadFriends(MainWindow* mainWin, ScriptingWindow* scriptWin, AuxiliaryWindow* masterWin)
{
	mainWindowFriend = mainWin;
	scriptingWindowFriend = scriptWin;
	auxWindowFriend = masterWin;
}


void CameraWindow::passManualSetAnalysisLocations()
{
	analysisHandler.onManualButtonPushed();
	mainWindowFriend->updateConfigurationSavedStatus( false );
}


void CameraWindow::passSetGridCorner( )
{
	analysisHandler.onCornerButtonPushed( );
	mainWindowFriend->updateConfigurationSavedStatus( false );
}


void CameraWindow::catchEnter()
{
	// the default handling is to close the window, so I need to catch it.
	errBox("Hello there!");
}


void CameraWindow::passAlwaysShowGrid()
{
	if (alwaysShowGrid)
	{
		alwaysShowGrid = false;
		menu.CheckMenuItem(ID_PICTURES_ALWAYSSHOWGRID, MF_UNCHECKED);
	}
	else
	{
		alwaysShowGrid = true;
		menu.CheckMenuItem(ID_PICTURES_ALWAYSSHOWGRID, MF_CHECKED);
	}
	CDC* dc = GetDC();
	pics.setAlwaysShowGrid(alwaysShowGrid, dc);	
	ReleaseDC(dc);
	pics.setSpecialGreaterThanMax(specialGreaterThanMax);
}


void CameraWindow::passCameraMode()
{
	CameraSettings.handleModeChange(this);
	mainWindowFriend->updateConfigurationSavedStatus( false );
}


void CameraWindow::abortCameraRun()
{
	int status;
	Andor.queryStatus(status);
	
	if (ANDOR_SAFEMODE)
	{
		// simulate as if you needed to abort.
		status = DRV_ACQUIRING;
	}
	if (status == DRV_ACQUIRING)
	{
		Andor.abortAcquisition();
		timer.setTimerDisplay( "Aborted" );
		Andor.setIsRunningState( false );
		// close the plotting thread.
		plotThreadAborting = true;
		plotThreadActive = false;
		atomCrunchThreadActive = false;
		// Wait until plotting thread is complete.
		WaitForSingleObject( plotThreadHandle, INFINITE );
		plotThreadAborting = false;
		// camera is no longer running.
		try
		{
			dataHandler.closeFile();
		}
		catch (Error& err)
		{
			mainWindowFriend->getComm()->sendError(err.what());
		}
		

		if (Andor.getAndorSettings().cameraMode != "Continuous Single Scans Mode")
		{
			int answer = promptBox("Acquisition Aborted. Delete Data file (data_" + str(dataHandler.getDataFileNumber())
									  + ".h5) for this run?",MB_YESNO );
			if (answer == IDYES)
			{
				try
				{
					dataHandler.deleteFile(mainWindowFriend->getComm());
				}
				catch (Error& err)
				{
					mainWindowFriend->getComm()->sendError(err.what());
				}
			}
		}
	}
	else if (status == DRV_IDLE)
	{
		Andor.setIsRunningState(false);
	}
}


bool CameraWindow::cameraIsRunning()
{
	return Andor.isRunning();
}


void CameraWindow::handlePictureEditChange( UINT id )
{
	try
	{
		pics.handleEditChange(id);
	}
	catch (Error& err)
	{
		// these errors seem more deserving of an error box.
		errBox(err.what());
	}
}


LRESULT CameraWindow::onCameraCalProgress( WPARAM wParam, LPARAM lParam )
{
	UINT picNum = lParam;
	if ( lParam == 0 )
	{
		// ???
		return NULL;
	}
	AndorRunSettings curSettings = Andor.getAndorSettings( );
	if ( lParam == -1 )
	{
		// last picture.
		picNum = curSettings.totalPicsInExperiment;
	}
	// need to call this before acquireImageData().
	Andor.updatePictureNumber( picNum );

	std::vector<std::vector<long>> picData;
	try
	{
		picData = Andor.acquireImageData( );
	}
	catch ( Error& err )
	{
		mainWindowFriend->getComm( )->sendError( err.what( ) );
		return NULL;
	}
	avgBackground.resize( picData.back( ).size( ) );
	for ( unsigned int i = 0; i < avgBackground.size( ); i++ )
	{
		avgBackground[i] += picData.back( )[i];
	}
	CDC* drawer = GetDC( );
	try
	{
		if ( picNum % curSettings.picsPerRepetition == 0 )
		{
			int counter = 0;
			for ( auto data : picData )
			{
				std::pair<int, int> minMax;
				minMax = stats.update( data, counter, selectedPixel, curSettings.imageSettings.width,
									   curSettings.imageSettings.height, picNum / curSettings.picsPerRepetition,
									   curSettings.totalPicsInExperiment / curSettings.picsPerRepetition );
				pics.drawPicture( drawer, counter, data, minMax );
				pics.drawDongles( drawer, selectedPixel, analysisHandler.getAnalysisLocs( ),
								  analysisHandler.getGrids( ), picNum );
				counter++;
			}
			timer.update( picNum / curSettings.picsPerRepetition, curSettings.repetitionsPerVariation,
						  curSettings.totalVariations, curSettings.picsPerRepetition );
		}
	}
	catch ( Error& err )
	{
		mainWindowFriend->getComm( )->sendError( err.what( ) );
	}
	ReleaseDC( drawer );
	mostRecentPicNum = picNum;
	return 0;
}



LRESULT CameraWindow::onCameraProgress( WPARAM wParam, LPARAM lParam )
{
	UINT picNum = lParam;
	if ( picNum % 2 == 1 )
	{
		mainThreadStartTimes.push_back( std::chrono::high_resolution_clock::now( ) );
	}
	if (lParam == 0)
	{
		// ???
		return NULL;
	}
	AndorRunSettings curSettings = Andor.getAndorSettings( );
	if ( lParam == -1 )
	{
		// last picture.
		picNum = curSettings.totalPicsInExperiment;
	}

	// need to call this before acquireImageData().
	Andor.updatePictureNumber( picNum );
	
	std::vector<std::vector<long>> rawPicData;
	try
	{
		rawPicData = Andor.acquireImageData();
	}
	catch (Error& err)
	{
		mainWindowFriend->getComm()->sendError( err.what() );
		return NULL;
	}
	std::vector<std::vector<long>> calPicData( rawPicData.size( ) );
	if ( CameraSettings.getUseCal( ) && avgBackground.size() == rawPicData.front().size() )
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
		std::lock_guard<std::mutex> locker( plotLock );
		// TODO: add check to check if this is needed.
		imageQueue.push_back( calPicData[(picNum - 1) % curSettings.picsPerRepetition] );
	}

	auto picsToDraw = CameraSettings.getImagesToDraw( calPicData );

	CDC* drawer = GetDC( );
	try
	{
		if (realTimePic)
		{
			std::pair<int, int> minMax;
			// draw the most recent pic.
			minMax = stats.update( picsToDraw.back(), picNum % curSettings.picsPerRepetition, selectedPixel,
								   curSettings.imageSettings.width, curSettings.imageSettings.height,
								   picNum / curSettings.picsPerRepetition,
								   curSettings.totalPicsInExperiment / curSettings.picsPerRepetition );

			pics.drawPicture( drawer, picNum % curSettings.picsPerRepetition, picsToDraw.back(), minMax );

			timer.update( picNum / curSettings.picsPerRepetition, curSettings.repetitionsPerVariation,
						  curSettings.totalVariations, curSettings.picsPerRepetition );
		}
		else if (picNum % curSettings.picsPerRepetition == 0)
		{
			int counter = 0;
			for (auto data : picsToDraw )
			{
				std::pair<int, int> minMax;
				minMax = stats.update( data, counter, selectedPixel, curSettings.imageSettings.width,
									   curSettings.imageSettings.height, picNum / curSettings.picsPerRepetition,
									   curSettings.totalPicsInExperiment / curSettings.picsPerRepetition );

				pics.drawPicture( drawer, counter, data, minMax );
				pics.drawDongles( drawer, selectedPixel, analysisHandler.getAnalysisLocs(), 
								  analysisHandler.getGrids(), picNum );
				counter++;
			}
			timer.update( picNum / curSettings.picsPerRepetition, curSettings.repetitionsPerVariation,
						  curSettings.totalVariations, curSettings.picsPerRepetition );
		}
	}
	catch (Error& err)
	{
		mainWindowFriend->getComm()->sendError( err.what() );
	}

	ReleaseDC( drawer );

	// write the data to the file.
	if (curSettings.cameraMode != "Continuous Single Scans Mode")
	{
		try
		{
			// important! write the original data, not the pic-to-draw, which can be a difference pic, or the calibrated
			// pictures, which can have the background subtracted.
			dataHandler.writePic( picNum, rawPicData[(picNum - 1) % curSettings.picsPerRepetition],
								  curSettings.imageSettings );
		}
		catch (Error& err)
		{
			mainWindowFriend->getComm()->sendError( err.what() );
		}
	}
	mostRecentPicNum = picNum;
	return 0;
}


void CameraWindow::wakeRearranger( )
{
	std::unique_lock<std::mutex> lock( rearrangerLock );
	rearrangerConditionVariable.notify_all( );
}


void CameraWindow::handleSpecialLessThanMinSelection()
{
	if (specialLessThanMin)
	{
		specialLessThanMin = false;
		menu.CheckMenuItem(ID_PICTURES_LESS_THAN_MIN_SPECIAL, MF_UNCHECKED);
	}
	else
	{
		specialLessThanMin = true;
		menu.CheckMenuItem(ID_PICTURES_LESS_THAN_MIN_SPECIAL, MF_CHECKED);
	}
	pics.setSpecialLessThanMin(specialLessThanMin);
}


void CameraWindow::handleSpecialGreaterThanMaxSelection()
{
	if (specialGreaterThanMax)
	{
		specialGreaterThanMax = false;
		menu.CheckMenuItem(ID_PICTURES_GREATER_THAN_MAX_SPECIAL, MF_UNCHECKED);
	}
	else
	{
		specialGreaterThanMax = true;
		menu.CheckMenuItem(ID_PICTURES_GREATER_THAN_MAX_SPECIAL, MF_CHECKED);
	}
	pics.setSpecialGreaterThanMax(specialGreaterThanMax);
}


void CameraWindow::handleAutoscaleSelection()
{
	if (autoScalePictureData)
	{
		autoScalePictureData = false;
		menu.CheckMenuItem(ID_PICTURES_AUTOSCALEPICTURES, MF_UNCHECKED);
	}
	else
	{
		autoScalePictureData = true;
		menu.CheckMenuItem(ID_PICTURES_AUTOSCALEPICTURES, MF_CHECKED);
	}
	pics.setAutoScalePicturesOption(autoScalePictureData);
}


LRESULT CameraWindow::onCameraCalFinish( WPARAM wParam, LPARAM lParam )
{
	// notify the andor object that it is done.
	Andor.onFinish( );
	Andor.pauseThread( );
	Andor.setCalibrating( false );
	justCalibrated = true;
	mainWindowFriend->getComm( )->sendColorBox( System::Camera, 'B' );
	CameraSettings.cameraIsOn( false );
	// normalize.
	for ( auto& p : avgBackground )
	{
		p /= 100.0;
	}
	// if auto cal is selected, always assume that the user was trying to start with F5.
	if ( CameraSettings.getAutoCal( ) ) 
	{
		PostMessageA( WM_COMMAND, MAKEWPARAM( ID_ACCELERATOR_F5, 0 ) );
	}
	return 0;
}


LRESULT CameraWindow::onCameraFinish( WPARAM wParam, LPARAM lParam )
{
	// notify the andor object that it is done.
	Andor.onFinish();
	Andor.pauseThread();
	if (alerts.soundIsToBePlayed())
	{
		alerts.playSound();
	}
	dataHandler.closeFile();
	mainWindowFriend->getComm()->sendColorBox( System::Camera, 'B' );
	mainWindowFriend->getComm()->sendStatus( "Camera has finished taking pictures and is no longer running.\r\n" );
	CameraSettings.cameraIsOn( false );
	mainWindowFriend->handleFinish();
	plotThreadActive = false;
	atomCrunchThreadActive = false;
	// rearranger thread handles these right now.
	mainThreadStartTimes.clear();
	crunchFinTimes.clear( );
	crunchSeesTimes.clear( );
	mainWindowFriend->stopRearranger( );
	wakeRearranger( );
	return 0;
}


void CameraWindow::startCamera()
{
	mainWindowFriend->getComm()->sendColorBox( System::Camera, 'Y');
	// turn some buttons off.
	CameraSettings.cameraIsOn( true );
	CDC* dc = GetDC();
	pics.refreshBackgrounds( dc );
	ReleaseDC(dc);
	stats.reset();
	// I used to initialize the data logger here.
	analysisHandler.updateDataSetNumberEdit( dataHandler.getNextFileNumber() - 1 );
	double minKineticTime;
	Andor.armCamera( this, minKineticTime );
	CameraSettings.updateMinKineticCycleTime( minKineticTime );
	mainWindowFriend->getComm()->sendColorBox(System::Camera, 'G');
}


bool CameraWindow::getCameraStatus()
{
	return Andor.isRunning();
}


void CameraWindow::handleDblClick(NMHDR* info, LRESULT* lResult)
{
	try
	{
		analysisHandler.handleDoubleClick( &mainWindowFriend->getFonts( ), CameraSettings.getSettings( ).andor.picsPerRepetition );
	}
	catch ( Error& err )
	{
		mainWindowFriend->getComm( )->sendError( err.what( ) );
	}
	mainWindowFriend->updateConfigurationSavedStatus( false );
}


void CameraWindow::listViewRClick( NMHDR* info, LRESULT* lResult )
{
	analysisHandler.handleRClick();
	mainWindowFriend->updateConfigurationSavedStatus( false );
}


void CameraWindow::OnLButtonUp(UINT stuff, CPoint loc)
{
	stopSound( );
}


void CameraWindow::stopSound( )
{
	alerts.stopSound( );
}


// pics looks up the location itself.
void CameraWindow::OnRButtonUp( UINT stuff, CPoint clickLocation )
{
	stopSound( );
	analysisHandler.saveGridParams( );
	CDC* dc = GetDC();
	try
	{
		if (analysisHandler.buttonClicked())
		{
			coordinate loc = pics.getClickLocation(clickLocation);
			if (loc.row != -1)
			{
				analysisHandler.handlePictureClick(loc);
				pics.redrawPictures( dc, selectedPixel, analysisHandler.getAnalysisLocs(), 
									 analysisHandler.getGrids(), false, mostRecentPicNum );
			}
		}
		else
		{
			coordinate box = pics.getClickLocation(clickLocation);
			if (box.row != -1)
			{
				selectedPixel = box;
				pics.redrawPictures( dc, selectedPixel, analysisHandler.getAnalysisLocs(), 
									 analysisHandler.getGrids( ), false, mostRecentPicNum );
			}
		}
	}
	catch (Error& err)
	{
		if ( err.whatBare( ) != "not found" )
		{
			mainWindowFriend->getComm( )->sendError( err.what( ) );
		}
	}
	ReleaseDC(dc);
}


/*
 *
 */
void CameraWindow::passSetTemperaturePress()
{
	try
	{
		CameraSettings.handleSetTemperaturePress();
	}
	catch (Error& err)
	{
		mainWindowFriend->getComm()->sendError(err.what());
	}
	mainWindowFriend->updateConfigurationSavedStatus( false );
}


/*
 *
 */
void CameraWindow::OnTimer(UINT_PTR id)
{
	CameraSettings.handleTimer();
}


/*
 *
 */
void CameraWindow::passTrigger()
{
	CameraSettings.handleTriggerChange(this);
	mainWindowFriend->updateConfigurationSavedStatus( false );
}


void CameraWindow::passAtomGridCombo( )
{
	try
	{
		analysisHandler.handleAtomGridCombo( );
	}
	catch ( Error& err )
	{
		mainWindowFriend->getComm( )->sendError( err.what( ) );
	}
}

/*
	This func doesn't make much sense...
*/
void CameraWindow::passPictureSettings( UINT id )
{
	handlePictureSettings( id );
	mainWindowFriend->updateConfigurationSavedStatus( false );
}


void CameraWindow::handlePictureSettings(UINT id)
{
	selectedPixel = { 0,0 };
	CameraSettings.handlePictureSettings(id, &Andor);
	if (CameraSettings.getSettings().andor.picsPerRepetition == 1)
	{
		pics.setSinglePicture( this, CameraSettings.getSettings( ).andor.imageSettings );
	}
	else
	{
		pics.setMultiplePictures( this, CameraSettings.getSettings( ).andor.imageSettings,
								  CameraSettings.getSettings().andor.picsPerRepetition);
	}
	pics.resetPictureStorage();
	std::array<int, 4> nums = CameraSettings.getSettings( ).palleteNumbers;
	pics.setPalletes(nums);

	CRect rect;
	GetWindowRect(&rect);
	OnSize(0, rect.right - rect.left, rect.bottom - rect.top);
	mainWindowFriend->updateConfigurationSavedStatus( false );
}

/*
Check that the camera is idle, or not aquiring pictures. Also checks that the data analysis handler isn't active.
*/
void CameraWindow::checkCameraIdle( )
{
	if ( Andor.isRunning( ) )
	{
		thrower( "Camera is already running! Please Abort to restart.\r\n" );
	}
	if ( analysisHandler.getLocationSettingStatus( ) )
	{
		thrower( "Please finish selecting analysis points before starting the camera!\r\n" );
	}
	// make sure it's idle.
	try
	{
		Andor.queryStatus( );
		if ( ANDOR_SAFEMODE )
		{
			thrower( "DRV_IDLE" );
		}
	}
	catch ( Error& exception )
	{
		if ( exception.whatBare( ) != "DRV_IDLE" )
		{
			throw;
		}
	}
}





BOOL CameraWindow::PreTranslateMessage(MSG* pMsg)
{
	for (UINT toolTipInc = 0; toolTipInc < tooltips.size(); toolTipInc++)
	{
		tooltips[toolTipInc]->RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}


void CameraWindow::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* scrollbar)
{
	pics.handleScroll(nSBCode, nPos, scrollbar);
}

// 3836, 1951
void CameraWindow::OnSize( UINT nType, int cx, int cy )
{
	SetRedraw( false );
	AndorRunSettings settings = CameraSettings.getSettings( ).andor;
	stats.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, mainWindowFriend->getFonts( ) );
	CameraSettings.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, mainWindowFriend->getFonts( ) );
	box.rearrange( cx, cy, mainWindowFriend->getFonts( ) );
	pics.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, mainWindowFriend->getFonts( ) );
	alerts.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, mainWindowFriend->getFonts( ) );
	analysisHandler.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, mainWindowFriend->getFonts( ) );
	pics.setParameters( CameraSettings.getSettings( ).andor.imageSettings );
	CDC* dc = GetDC( );
	try
	{
		pics.redrawPictures( dc, selectedPixel, analysisHandler.getAnalysisLocs( ), analysisHandler.getGrids(), false,
							 mostRecentPicNum );
	}
	catch ( Error& err )
	{
		mainWindowFriend->getComm( )->sendError( err.what( ) );
	}
	ReleaseDC( dc );
	timer.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, mainWindowFriend->getFonts( ) );
	SetRedraw( );
	RedrawWindow( );
}


void CameraWindow::setEmGain()
{
	try 
	{
		CameraSettings.setEmGain();
	}
	catch (Error& exception)
	{
		errBox( exception.what() );
	}
	mainWindowFriend->updateConfigurationSavedStatus( false );
}


void CameraWindow::handleMasterConfigSave(std::stringstream& configStream)
{
	imageParameters settings = CameraSettings.getSettings().andor.imageSettings;
	configStream << settings.left << " " << settings.right << " " << settings.horizontalBinning << " ";
	configStream << settings.bottom << " " << settings.top << " " << settings.verticalBinning << "\n";
}


void CameraWindow::handleMasterConfigOpen(std::stringstream& configStream, Version version)
{
	mainWindowFriend->updateConfigurationSavedStatus( false );
	imageParameters settings = CameraSettings.getSettings().andor.imageSettings;
	selectedPixel = { 0,0 };
	std::string tempStr;
	try
	{
		configStream >> tempStr;
		settings.left = std::stol(tempStr);
		configStream >> tempStr;
		settings.right = std::stol(tempStr);
		configStream >> tempStr;
		settings.horizontalBinning = std::stol(tempStr);
		configStream >> tempStr;
		settings.bottom = std::stol(tempStr);
		configStream >> tempStr;
		settings.top = std::stol(tempStr);
		configStream >> tempStr;
		settings.verticalBinning = std::stol(tempStr);
		settings.width = (settings.right - settings.left + 1) / settings.horizontalBinning;
		settings.height = (settings.top - settings.bottom + 1) / settings.verticalBinning;

		CameraSettings.setImageParameters(settings, this);
		pics.setParameters(settings);
		redrawPictures(true);
	}
	catch (std::invalid_argument&)
	{
		thrower("ERROR: Bad value (i.e. failed to convert to long) seen in master configueration file while attempting "
				 "to load camera dimensions!");
	}
}


DataLogger* CameraWindow::getLogger()
{
	return &dataHandler;
}


void CameraWindow::loadCameraCalSettings( ExperimentInput& input )
{
	redrawPictures( false );
	try
	{
		checkCameraIdle( );
	}
	catch ( Error& err)
	{
		mainWindowFriend->getComm( )->sendError( err.what( ) );
	}

	CDC* dc = GetDC( );
	pics.refreshBackgrounds( dc );
	ReleaseDC( dc );
	// I used to mandate use of a button to change image parameters. Now I don't have the button and just always 
	// update at this point.
	readImageParameters( );
	pics.setNumberPicturesActive( 1 );
	// biggest check here, camera settings includes a lot of things.
	CameraSettings.checkIfReady( );
	input.camSettings = CameraSettings.getCalibrationSettings( ).andor;
	// reset the image which is about to be calibrated.
	avgBackground.clear( );
	/// start the camera.
	Andor.setSettings( input.camSettings );
	Andor.setCalibrating(true);
}


void CameraWindow::prepareCamera( ExperimentInput& input )
{
	redrawPictures( false );
	checkCameraIdle( );
	CDC* dc = GetDC();
	pics.refreshBackgrounds(dc);
	ReleaseDC(dc);
	// I used to mandate use of a button to change image parameters. Now I don't have the button and just always 
	// update at this point.
	readImageParameters( );
	pics.setNumberPicturesActive( CameraSettings.getSettings().andor.picsPerRepetition );
	// this is a bit awkward at the moment.
	
	CameraSettings.setRepsPerVariation(mainWindowFriend->getRepNumber());
	UINT varNumber = auxWindowFriend->getTotalVariationNumber();
	if (varNumber == 0)
	{
		// this means that the user isn't varying anything, so effectively this should be 1.
		varNumber = 1;
	}
	CameraSettings.setVariationNumber(varNumber);
	
	// biggest check here, camera settings includes a lot of things.
	CameraSettings.checkIfReady();
	input.camSettings = CameraSettings.getSettings().andor;
	/// start the camera.
	Andor.setSettings( input.camSettings );
}

void CameraWindow::prepareAtomCruncher( ExperimentInput& input )
{
	input.cruncherInput = new atomCruncherInput;
	input.cruncherInput->plotterActive = plotThreadActive;
	input.cruncherInput->imageDims = CameraSettings.getSettings( ).andor.imageSettings;
	atomCrunchThreadActive = true;
	input.cruncherInput->plotterNeedsImages = input.plotterInput->needsCounts;
	input.cruncherInput->cruncherThreadActive = &atomCrunchThreadActive;
	skipNext = false;
	input.cruncherInput->skipNext = &skipNext;
	input.cruncherInput->imageQueue = &imageQueue;
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
	input.cruncherInput->imageLock = &imageLock;
	input.cruncherInput->plotLock = &plotLock;
	input.cruncherInput->rearrangerLock = &rearrangerLock;
	// what the thread fills.
	input.cruncherInput->grids = analysisHandler.getGrids( );
	// reinitialize
	plotterPictureQueue.clear( );// = std::vector<std::vector<std::vector<long>>>( input.cruncherInput->grids.size());
	plotterAtomQueue.clear( );// = std::vector<std::vector<std::vector<bool>>>( input.cruncherInput->grids.size( ) );
	input.cruncherInput->plotterImageQueue = &plotterPictureQueue;
	input.cruncherInput->plotterAtomQueue = &plotterAtomQueue;
	rearrangerAtomQueue.clear( );
	input.cruncherInput->rearrangerAtomQueue = &rearrangerAtomQueue;
	input.cruncherInput->thresholds = CameraSettings.getSettings( ).thresholds;
	input.cruncherInput->picsPerRep = CameraSettings.getSettings().andor.picsPerRepetition;
	// important, always uses the 0th atom grid for rearrangement and load-skip stuff.
	//input.cruncherInput->gridInfo = analysisHandler.getAtomGrid( 0 );
	
	input.cruncherInput->catchPicTime = &crunchSeesTimes;
	input.cruncherInput->finTime = &crunchFinTimes;
	input.cruncherInput->atomThresholdForSkip = mainWindowFriend->getMainOptions( ).atomThresholdForSkip;
	input.cruncherInput->rearrangerConditionWatcher = &rearrangerConditionVariable;
}


void CameraWindow::startAtomCruncher(ExperimentInput& input)
{
	UINT atomCruncherID;
	atomCruncherThreadHandle = (HANDLE)_beginthreadex( 0, 0, CameraWindow::atomCruncherProcedure,
													   (void*)input.cruncherInput, 0, &atomCruncherID );
}


bool CameraWindow::wantsAutoPause( )
{
	return alerts.wantsAutoPause( );
}


void CameraWindow::preparePlotter( ExperimentInput& input )
{
	/// start the plotting thread.
	plotThreadActive = true;
	plotThreadAborting = false;
	imageQueue.clear();
	plotterAtomQueue.clear();
	input.plotterInput = new realTimePlotterInput;
	input.plotterInput->aborting = &plotThreadAborting;
	input.plotterInput->active = &plotThreadActive;
	input.plotterInput->imageQueue = &plotterPictureQueue;
	input.plotterInput->imageShape = CameraSettings.getSettings().andor.imageSettings;
	input.plotterInput->picsPerVariation = mainWindowFriend->getRepNumber() * CameraSettings.getSettings().andor.picsPerRepetition;
	input.plotterInput->variations = auxWindowFriend->getTotalVariationNumber();
	input.plotterInput->picsPerRep = CameraSettings.getSettings().andor.picsPerRepetition;
	input.plotterInput->alertThreshold = alerts.getAlertThreshold();
	input.plotterInput->wantAlerts = alerts.alertsAreToBeUsed();
	input.plotterInput->comm = mainWindowFriend->getComm();
	input.plotterInput->plotLock = &plotLock;
	input.plotterInput->numberOfRunsToAverage = 5;
	input.plotterInput->plottingFrequency = analysisHandler.getPlotFreq( );
	if ( input.masterInput )
	{
		input.plotterInput->key = ParameterSystem::getKeyValues( input.masterInput->variables[0] );
	}
	else
	{
		std::vector<double> dummyKey;
		// make a large dummy array to be used. In principle if the users uses a plotter without a master thread for
		// a long time this could crash...  TODO take care of this!
		dummyKey.resize( 100 );
		input.plotterInput->key = dummyKey;
		UINT count = 0;
		for ( auto& e : input.plotterInput->key )
		{
			e = count++;
		}
	}
	input.plotterInput->atomQueue = &plotterAtomQueue;
	analysisHandler.fillPlotThreadInput( input.plotterInput );
	// remove old plots that aren't trying to sustain.
	activePlots.erase( std::remove_if( activePlots.begin(), activePlots.end(), PlotDialog::removeQuery ), 
					   activePlots.end() );
	for ( auto plotParams : input.plotterInput->plotInfo )
	{
		// Create vector of data to be shared btween plotter and data analysis handler. 
		std::vector<pPlotDataVec> data;
		// assume 1 data set...
		UINT numDataSets = 1;
		// +1 for average
		UINT numLines = numDataSets * ( input.plotterInput->grids[plotParams.whichGrid].height 
										* input.plotterInput->grids[plotParams.whichGrid].width + 1 );
		data.resize( numLines );
		for ( auto& line : data )
		{
			line = pPlotDataVec( new plotDataVec( input.plotterInput->key.size( ), { 0, -1, 0 } ) );
			line->resize( input.plotterInput->key.size( ) );
			// initialize x axis for all data sets.
			UINT count = 0;
			for ( auto& keyItem : input.plotterInput->key )
			{
				line->at( count++ ).x = keyItem;
			}
		}
		plotStyle style;
		if ( plotParams.isHist )
		{
			style = plotStyle::HistPlot;
		}
		else
		{
			style = plotStyle::ErrorPlot;
		}
		// start a PlotDialog dialog
		PlotDialog* plot = new PlotDialog( data, style, mainWindowFriend->getPlotPens(), 
										   mainWindowFriend->getPlotFont( ), mainWindowFriend->getPlotBrushes( ), 
										   analysisHandler.getPlotTime(), plotParams.name );
		plot->Create( IDD_PLOT_DIALOG, 0 );
		plot->ShowWindow( SW_SHOW );
		activePlots.push_back( plot );
		input.plotterInput->dataArrays.push_back( data );
	}
}


void CameraWindow::startPlotterThread( ExperimentInput& input )
{
	bool gridHasBeenSet = false;
	for ( auto gridInfo : input.plotterInput->grids )
	{
		if ( !(gridInfo.topLeftCorner == coordinate( 0, 0 )) )
		{
			gridHasBeenSet = true;
			break;
		}
	}
	UINT plottingThreadID;
	if ((!gridHasBeenSet	&& input.plotterInput->analysisLocations.size() == 0) 
		 || input.plotterInput->plotInfo.size() == 0)
	{
		plotThreadActive = false;
	}
	else
	{
		if ( input.camSettings.totalPicsInExperiment * input.plotterInput->analysisLocations.size()
			 / input.plotterInput->plottingFrequency > 1000 )
		{
			infoBox( "Warning: The number of pictures * points to analyze in the experiment is very large,"
					 " and the plotting period is fairly small. Consider increasing the plotting period. " );
		}
		// start the plotting thread
		plotThreadActive = true;
		plotThreadHandle = (HANDLE)_beginthreadex( 0, 0, DataAnalysisControl::plotterProcedure, (void*)input.plotterInput,
												   0, &plottingThreadID );
	}
}


// this thread has one purpose: watch the image vector thread for new images, determine where atoms are 
// (the atom crunching part), and pass them to the threads waiting on atom info.
// should consider modifying so that it can use an array of locations. At the moment doesn't.
UINT __stdcall CameraWindow::atomCruncherProcedure(void* inputPtr)
{
	atomCruncherInput* input = (atomCruncherInput*)inputPtr; 
	// monitoredPixelIndecies[grid][pixelNumber]
	if ( input->grids[0].topLeftCorner == coordinate( 0, 0 ) || input->grids.size( ) == 0 )
	{
		return 0;
	}
	std::vector<std::vector<long>> monitoredPixelIndecies(input->grids.size());
	// TODO: change to loop over all grids...
	for ( UINT gridInc =0; gridInc < input->grids.size( ); gridInc++)
	{
		for ( UINT columnInc = 0; columnInc < input->grids[gridInc].width; columnInc++ )
		{
			for ( UINT rowInc =0; rowInc < input->grids[gridInc].height; rowInc++ )
			{
				ULONG pixelRow = (input->grids[gridInc].topLeftCorner.row - 1) 
					+ rowInc * input->grids[gridInc].pixelSpacing;
				ULONG pixelColumn = (input->grids[gridInc].topLeftCorner.column - 1) 
					+ columnInc * input->grids[gridInc].pixelSpacing;
				if ( pixelRow >= input->imageDims.height || pixelColumn >= input->imageDims.width )
				{
					errBox( "ERROR: atom grid appears to include pixels outside the image frame! Not allowed, seen by atom "
							"cruncher thread" );
					return 0;
				}
				int index = ((input->imageDims.height - 1 - pixelRow) * input->imageDims.width + pixelColumn);
				if ( index >= input->imageDims.width * input->imageDims.height )
				{
					// shouldn't happen after I finish debugging.
					errBox( "ERROR: Math error! Somehow, the pixel indexes appear within bounds, but the calculated index"
							" is larger than the image is!  (A low level bug, this shouldn't happen)" );
					return 0;
				}
				monitoredPixelIndecies[gridInc].push_back( index );
			}
		}
	}
	   
	UINT imageCount = 0;   
	// loop watching the image queue.
	while (*input->cruncherThreadActive || input->imageQueue->size() != 0)
	{
		// if no images wait until images. Should probably change to be event based, but want this to be fast...
		if (input->imageQueue->size() == 0)
		{
			continue;
		}
		if ( imageCount % 2 == 0 )
		{
			input->catchPicTime->push_back( std::chrono::high_resolution_clock::now( ) );
		}
		
		// tempImagePixels[grid][pixel]; only contains the counts for the pixels being monitored.
		std::vector<std::vector<long>> tempImagePixels( input->grids.size( ) );
		// tempAtomArray[grid][pixel]; only contains the boolean true/false of whether an atom passed a threshold or not. 
		std::vector<std::vector<bool>> tempAtomArray( input->grids.size( ) );											  
		for (UINT gridInc = 0; gridInc < input->grids.size(); gridInc++)
		{
			tempAtomArray[gridInc] = std::vector<bool>( monitoredPixelIndecies[gridInc].size( ) );
			tempImagePixels[gridInc] = std::vector<long>( monitoredPixelIndecies[gridInc].size( ) );
		}
		for ( UINT gridInc = 0; gridInc < input->grids.size( ); gridInc++ )
		{
			///*** Deal with 1st element entirely first, as this is important for the rearranger thread and the load-skip.
			UINT count = 0;
			{ // scope for the lock_guard. I want to free the lock as soon as possible, so add extra small scope.
				std::lock_guard<std::mutex> locker( *input->imageLock );				
				for ( auto pixelIndex : monitoredPixelIndecies[gridInc] )
				{
					tempImagePixels[gridInc][count++] = (*input->imageQueue)[0][pixelIndex];
				}
			}
			count = 0;
			for ( auto& pix : tempImagePixels[gridInc] )
			{
				if ( pix >= input->thresholds[imageCount % input->picsPerRep] )
				{
					tempAtomArray[gridInc][count] = true;
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
						input->finTime->push_back( std::chrono::high_resolution_clock::now( ) );
					}
				}
				// if last picture of repetition, check for loadskip condition.
				if ( imageCount % input->picsPerRep == input->picsPerRep - 1 ) ///
				{
					UINT numAtoms = std::accumulate( tempAtomArray[0].begin( ), tempAtomArray[0].end( ), 0 );
					*input->skipNext = (numAtoms >= input->atomThresholdForSkip);
				}
			}
		}

		if ( input->plotterActive ) ///
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
		std::lock_guard<std::mutex> locker( *input->imageLock );
		(*input->imageQueue).erase((*input->imageQueue).begin());		
	}
	return 0;
}


std::string CameraWindow::getStartMessage()
{
	// get selected plots
	std::vector<std::string> plots = analysisHandler.getActivePlotList();
	imageParameters currentImageParameters = CameraSettings.getSettings( ).andor.imageSettings;
	bool errCheck = false;
	for (UINT plotInc = 0; plotInc < plots.size(); plotInc++)
	{
		PlottingInfo tempInfoCheck(PLOT_FILES_SAVE_LOCATION + "\\" + plots[plotInc] + ".plot");
		if (tempInfoCheck.getPicNumber() != CameraSettings.getSettings().andor.picsPerRepetition)
		{
			thrower( "ERROR: one of the plots selected, " + plots[plotInc] + ", is not built for the currently "
					 "selected number of pictures per experiment. Please revise either the current setting or the plot"
					 " file." );
		}
		tempInfoCheck.setGroups( analysisHandler.getAnalysisLocs() );
		std::vector<std::pair<UINT, UINT>> plotLocations = tempInfoCheck.getAllPixelLocations();
	}
	std::string dialogMsg;
	dialogMsg = "Camera Parameters:\r\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\r\n";
	dialogMsg += "Current Camera Temperature Setting:\r\n\t" + str(
		CameraSettings.getSettings().andor.temperatureSetting ) + "\r\n";
	dialogMsg += "Exposure Times: ";
	for (auto& time : CameraSettings.getSettings().andor.exposureTimes)
	{
		dialogMsg += str( time * 1000 ) + ", ";
	}
	dialogMsg += "\r\n";
	dialogMsg += "Image Settings:\r\n\t" + str( currentImageParameters.left ) + " - " + str( currentImageParameters.right ) + ", "
		+ str( currentImageParameters.bottom ) + " - " + str( currentImageParameters.top ) + "\r\n";
	dialogMsg += "\r\n";
	dialogMsg += "Kintetic Cycle Time:\r\n\t" + str( CameraSettings.getSettings().andor.kineticCycleTime ) + "\r\n";
	dialogMsg += "Pictures per Repetition:\r\n\t" + str( CameraSettings.getSettings().andor.picsPerRepetition ) + "\r\n";
	dialogMsg += "Repetitions per Variation:\r\n\t" + str( CameraSettings.getSettings().andor.totalPicsInVariation ) + "\r\n";
	dialogMsg += "Variations per Experiment:\r\n\t" + str( CameraSettings.getSettings().andor.totalVariations ) + "\r\n";
	dialogMsg += "Total Pictures per Experiment:\r\n\t" + str( CameraSettings.getSettings().andor.totalPicsInExperiment ) + "\r\n";
	
	dialogMsg += "Real-Time Atom Detection Thresholds:\r\n\t";
	for (auto& threshold : CameraSettings.getSettings().thresholds)
	{
		dialogMsg += str( threshold ) + ", ";
	}

	dialogMsg += "\r\nReal-Time Plots:\r\n";
	for (UINT plotInc = 0; plotInc < plots.size(); plotInc++)
	{
		dialogMsg += "\t" + plots[plotInc] + "\r\n";
	}

	return dialogMsg;
}


void CameraWindow::fillMasterThreadInput( MasterThreadInput* input )
{
	// starting not-calibration, so reset this.
	justCalibrated = false;
	input->atomQueueForRearrangement = &rearrangerAtomQueue;
	input->rearrangerLock = &rearrangerLock;
	input->andorsImageTimes = &imageTimes;
	input->grabTimes = &imageGrabTimes;
	input->analysisGrid = analysisHandler.getAtomGrid( 0 );
	input->conditionVariableForRerng = &rearrangerConditionVariable;
}


void CameraWindow::setTimerText( std::string timerText )
{
	timer.setTimerDisplay( timerText );
}


void CameraWindow::OnCancel()
{
	try
	{
		passCommonCommand( ID_FILE_MY_EXIT );
	}
	catch (Error& exception)
	{
		errBox( exception.what() );
	}
}


cToolTips CameraWindow::getToolTips()
{
	return tooltips;
}


BOOL CameraWindow::OnInitDialog()
{
	// don't redraw until the first OnSize.
	SetRedraw( false );
	Andor.initializeClass( mainWindowFriend->getComm(), &imageTimes );
	cameraPositions positions;
	// all of the initialization functions increment and use the id, so by the end it will be 3000 + # of controls.
	int id = 3000;
	positions.sPos = { 0, 0 };
	box.initialize( positions.sPos, id, this, 480, tooltips );
	positions.videoPos = positions.amPos = positions.seriesPos = positions.sPos;
	alerts.alertMainThread( 0 );
	alerts.initialize( positions, this, false, id, tooltips );
	analysisHandler.initialize( positions, id, this, tooltips, false, mainWindowFriend->getRgbs() );
	CameraSettings.initialize( positions, id, this, tooltips );
	POINT position = { 480, 0 };
	stats.initialize( position, this, id, tooltips );
	positions.sPos = { 797, 0 };
	timer.initialize( positions, this, false, id, tooltips );
	position = { 797, 40 };
	pics.initialize( position, this, id, tooltips, mainWindowFriend->getBrushes()["Dark Green"] );
	// end of literal initialization calls
	pics.setSinglePicture( this, CameraSettings.getSettings( ).andor.imageSettings );
	// set initial settings.
	Andor.setSettings( CameraSettings.getSettings().andor );
	// load the menu
	menu.LoadMenu( IDR_MAIN_MENU );
	SetMenu( &menu );
	// final steps
	SetTimer( NULL, 1000, NULL );
	CRect rect;
	GetWindowRect( &rect );
	OnSize( 0, rect.right - rect.left, rect.bottom - rect.top );
	return TRUE;
}

void CameraWindow::setDataType( std::string dataType )
{
	stats.updateType( dataType );
}


void CameraWindow::redrawPictures( bool andGrid )
{
	CDC* dc = GetDC();
	try
	{
		pics.refreshBackgrounds( dc );
		if (andGrid)
		{
			pics.drawGrids( dc );
		}
		ReleaseDC( dc );
	}
	catch (Error& err)
	{
		ReleaseDC( dc );
		mainWindowFriend->getComm()->sendError( err.what() );
	}
	// currently don't attempt to redraw previous picture data.
}


HBRUSH CameraWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	brushMap brushes = mainWindowFriend->getBrushes();
	rgbMap rgbs = mainWindowFriend->getRgbs();
	HBRUSH result;
	int num = pWnd->GetDlgCtrlID();

	result = *CameraSettings.handleColor(num, pDC, mainWindowFriend->getBrushes(), mainWindowFriend->getRgbs());
	if (result) { return result; }

	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{			
			CBrush* result = box.handleColoring(num, pDC, brushes, rgbs);
			if (result)
			{
				return *result;
			}
			else
			{
				pDC->SetTextColor( rgbs["Solarized Base0"] );
				pDC->SetBkColor( rgbs["Medium Grey"] );
				return *brushes["Medium Grey"];
			}
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor(rgbs["Solarized Green"]);
			pDC->SetBkColor(rgbs["Solarized Base02"]);
			return *brushes["Solarized Base02"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor(rgbs["Solarized Base0"]);
			pDC->SetBkColor(rgbs["Dark Grey"]);
			return *brushes["Dark Grey"];
		}
		default:
		{
			return *brushes["Solarized Base04"];
		}
	}
}


std::atomic<bool>* CameraWindow::getSkipNextAtomic( )
{
	return &skipNext;
}


void CameraWindow::stopPlotter( )
{
	plotThreadAborting = true;
}


void CameraWindow::passCommonCommand(UINT id)
{
	try
	{
		commonFunctions::handleCommonMessage( id, this, mainWindowFriend, scriptingWindowFriend, this, 
											  auxWindowFriend );
	}
	catch (Error& err)
	{
		// catch any extra errors that handleCommonMessage doesn't explicitly handle.
		errBox( err.what() );
	}
}


// this is typically a little redundant to call, but can use to make sure things are set to off.
void CameraWindow::assertOff()
{
	CameraSettings.cameraIsOn(false);
	plotThreadActive = false;
	atomCrunchThreadActive = false;
}


void CameraWindow::readImageParameters()
{
	selectedPixel = { 0,0 };
	try
	{
		redrawPictures(false);
		imageParameters parameters = CameraSettings.getSettings( ).andor.imageSettings;
		pics.setParameters( parameters );
	}
	catch (Error& exception)
	{
		Communicator* comm = mainWindowFriend->getComm();
		comm->sendColorBox( System::Camera, 'R' );
		comm->sendError( exception.whatStr() + "\r\n" );
	}
	CDC* dc = GetDC();
	pics.drawGrids(dc);
	ReleaseDC(dc);
}

void CameraWindow::changeBoxColor(systemInfo<char> colors)
{
	box.changeColor(colors);
}
