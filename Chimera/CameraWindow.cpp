#include "stdafx.h"
#include "commonFunctions.h"
#include "CameraSettingsControl.h"
#include "PlottingInfo.h"
#include "ATMCD32D.H"
#include "AuxiliaryWindow.h"
#include "CameraWindow.h"
#include "realTimePlotterInput.h"

CameraWindow::CameraWindow() : CDialog(), CameraSettings(&Andor), dataHandler(DATA_SAVE_LOCATION), 
                               plotter(GNUPLOT_LOCATION)
{
	/// test the plotter quickly
	plotter.send( "set title \"Gnuplot is Working. You can close this window at any time.\"" );
	plotter.send("plot '-'");
	std::vector<double> data(100);
	int count = 0;
	for ( auto& dat : data )
	{
		dat = -(count - 50)*(count - 50);
		count++;
	}
	plotter.sendData(data);
};


IMPLEMENT_DYNAMIC(CameraWindow, CDialog)


BEGIN_MESSAGE_MAP(CameraWindow, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_VSCROLL()

	ON_COMMAND_RANGE(MENU_ID_RANGE_BEGIN, MENU_ID_RANGE_END, &CameraWindow::passCommonCommand)
	ON_COMMAND_RANGE(PICTURE_SETTINGS_ID_START, PICTURE_SETTINGS_ID_END, &CameraWindow::passPictureSettings)
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
	//ON_COMMAND( IDC_SET_IMAGE_PARAMETERS_BUTTON, &CameraWindow::readImageParameters)
	ON_COMMAND( IDC_SET_EM_GAIN_BUTTON, &CameraWindow::setEmGain)
	ON_COMMAND( IDC_ALERTS_BOX, &CameraWindow::passAlertPress)
	ON_COMMAND( IDC_SET_TEMPERATURE_BUTTON, &CameraWindow::passSetTemperaturePress)
	ON_COMMAND( IDOK, &CameraWindow::catchEnter)
	ON_COMMAND( IDC_SET_ANALYSIS_LOCATIONS, &CameraWindow::passManualSetAnalysisLocations)
	ON_COMMAND( IDC_SET_GRID_CORNER, &CameraWindow::passSetGridCorner)

	ON_CBN_SELENDOK(IDC_TRIGGER_COMBO, &CameraWindow::passTrigger)
	ON_CBN_SELENDOK( IDC_CAMERA_MODE_COMBO, &CameraWindow::passCameraMode )

	ON_REGISTERED_MESSAGE( eCameraFinishMessageID, &CameraWindow::onCameraFinish )
	ON_REGISTERED_MESSAGE( eCameraProgressMessageID, &CameraWindow::onCameraProgress )
	
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONUP()

	ON_NOTIFY(NM_RCLICK, IDC_PLOTTING_LISTVIEW, &CameraWindow::listViewRClick)
	ON_NOTIFY(NM_DBLCLK, IDC_PLOTTING_LISTVIEW, &CameraWindow::handleDblClick)

	ON_COMMAND(IDC_ALERTS_BOX, &CameraWindow::passAlertPress)
END_MESSAGE_MAP()


std::string CameraWindow::getSystemStatusString()
{
	std::string statusStr; 
	statusStr = "\n\n>>> Andor Camera <<<\n";
	if (!ANDOR_SAFEMODE)
	{
		statusStr += "Code System is Active!\n";
		statusStr += Andor.getSystemInfo();
	}
	else
	{
		statusStr += "Code System is disabled! Enable in \"constants.h\"\n";
	}
	return statusStr;
}


void CameraWindow::handleNewConfig( std::ofstream& newFile )
{
	CameraSettings.handleNewConfig( newFile );
	pics.handleNewConfig( newFile );
	analysisHandler.handleNewConfig( newFile );
	// todo: include plotter info
}


void CameraWindow::handleSaveConfig(std::ofstream& saveFile)
{
	CameraSettings.handleSaveConfig(saveFile);
	pics.handleSaveConfig(saveFile);
	analysisHandler.handleSaveConfig( saveFile );
	// TODO: plotter
}


void CameraWindow::handleOpeningConfig(std::ifstream& configFile, double version)
{
	// I could and perhaps should further subdivide this up.
	CameraSettings.handleOpenConfig(configFile, version);
	pics.handleOpenConfig(configFile, version);
	analysisHandler.handleOpenConfig( configFile, version );
	if ( CameraSettings.getSettings( ).picsPerRepetition == 1 )
	{
		pics.setSinglePicture( this, CameraSettings.readImageParameters( this )  );
	}
	else
	{
		pics.setMultiplePictures( this, CameraSettings.readImageParameters( this ), 
								  CameraSettings.getSettings( ).picsPerRepetition );
	}
	pics.resetPictureStorage( );
	std::array<int, 4> nums = CameraSettings.getPaletteNumbers( );
	pics.setPalletes( nums );

	CRect rect;
	GetWindowRect( &rect );
	OnSize( 0, rect.right - rect.left, rect.bottom - rect.top );
	/*
	int plotNumber;
	configFile >> plotNumber;
	configFile.get();
	for (int plotInc = 0; plotInc < plotNumber; plotInc++)
	{
		std::string plotName;
		std::getline(configFile, plotName);
	}
	*/
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
	Andor.queryIdentity(status);
	
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
		plotThreadActive = false;
		atomCrunchThreadActive = false;
		// Wait until plotting thread is complete.
		WaitForSingleObject( plotThreadHandle, INFINITE );
		// camera is no longer running.
		try
		{
			dataHandler.closeFile();
		}
		catch (Error& err)
		{
			mainWindowFriend->getComm()->sendError(err.what());
		}
		

		if (Andor.getSettings().cameraMode != "Continuous Single Scans Mode")
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


LRESULT CameraWindow::onCameraProgress( WPARAM wParam, LPARAM lParam )
{
	UINT pictureNumber = lParam;
	if ( pictureNumber % 2 == 1 )
	{
		mainThreadStartTimes.push_back( std::chrono::high_resolution_clock::now( ) );
	}
	if (lParam == 0)
	{
		return NULL;
	}
	Andor.updatePictureNumber( pictureNumber );
	std::vector<std::vector<long>> picData;
	try
	{
		picData = Andor.acquireImageData();
	}
	catch (Error& err)
	{
		mainWindowFriend->getComm()->sendError( err.what() );
		return NULL;
	}
	
	if ( pictureNumber % 2 == 1 )
	{
		imageGrabTimes.push_back( std::chrono::high_resolution_clock::now( ) );
	}
	AndorRunSettings currentSettings = Andor.getSettings();
	//
	{
		std::lock_guard<std::mutex> locker( plotLock );
		// add check to check if this is needed.
		imageQueue.push_back( picData[(pictureNumber - 1) % currentSettings.picsPerRepetition] );
	}

	CDC* drawer = GetDC( );
	try
	{
		if (realTimePic)
		{
			std::pair<int, int> minMax;
			// draw the most recent pic.
			minMax = stats.update( picData.back(), pictureNumber % currentSettings.picsPerRepetition, selectedPixel,
								   currentSettings.imageSettings.width, currentSettings.imageSettings.height,
								   pictureNumber / currentSettings.picsPerRepetition,
								   currentSettings.totalPicsInExperiment / currentSettings.picsPerRepetition );
			pics.drawPicture( drawer, pictureNumber % currentSettings.picsPerRepetition, picData.back(), minMax );
			timer.update( pictureNumber / currentSettings.picsPerRepetition, currentSettings.repetitionsPerVariation,
						  currentSettings.totalVariations, currentSettings.picsPerRepetition );
		}
		else if (pictureNumber % currentSettings.picsPerRepetition == 0)
		{

			int counter = 0;
			for (auto data : picData)
			{
				std::pair<int, int> minMax;
				minMax = stats.update( data, counter, selectedPixel, currentSettings.imageSettings.width,
									   currentSettings.imageSettings.height, pictureNumber / currentSettings.picsPerRepetition,
									   currentSettings.totalPicsInExperiment / currentSettings.picsPerRepetition );

				pics.drawPicture( drawer, counter, data, minMax );
				pics.drawDongles( drawer, selectedPixel, analysisHandler.getAnalysisLocs(), analysisHandler.getAtomGrid() );
				counter++;
			}
			timer.update( pictureNumber / currentSettings.picsPerRepetition, currentSettings.repetitionsPerVariation,
						  currentSettings.totalVariations, currentSettings.picsPerRepetition );
		}

	}
	catch (Error& err)
	{
		mainWindowFriend->getComm()->sendError( err.what() );
	}

	ReleaseDC( drawer );

	// write the data to the file.
	if (currentSettings.cameraMode != "Continuous Single Scans Mode")
	{
		try
		{
			dataHandler.writePic( pictureNumber, picData[(pictureNumber - 1) % currentSettings.picsPerRepetition],
								  currentSettings.imageSettings );
		}
		catch (Error& err)
		{
			mainWindowFriend->getComm()->sendError( err.what() );
		}
	}
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
	mainWindowFriend->getComm()->sendColorBox( Camera, 'B' );
	mainWindowFriend->getComm()->sendStatus( "Camera has finished taking pictures and is no longer running.\r\n" );
	CameraSettings.cameraIsOn( false );
	mainWindowFriend->handleFinish();
	plotThreadActive = false;
	atomCrunchThreadActive = false;
	/*
	std::vector<double> imageToMainTime, mainToGrabTime, grabToCrunchTime, crunchCrunchingTime;
	for ( auto inc : range( imageTimes.size( ) ) )
	{
		imageToMainTime.push_back( std::chrono::duration<double>( mainThreadStartTimes[inc] 
																  - imageTimes[inc] ).count( ) );
		mainToGrabTime.push_back( std::chrono::duration<double>( imageGrabTimes[inc] 
																 - mainThreadStartTimes[inc] ).count( ) );
		grabToCrunchTime.push_back( std::chrono::duration<double>( crunchSeesTimes[inc] 
																   - imageGrabTimes[inc] ).count( ) );
		crunchCrunchingTime.push_back( std::chrono::duration<double>( crunchFinTimes[inc] 
																	  - crunchSeesTimes[inc] ).count( ) );
	}
	*/
	// rearranger thread handles these right now.
	//imageTimes.clear();
	//imageGrabTimes.clear();
	mainThreadStartTimes.clear();
	crunchFinTimes.clear( );
	crunchSeesTimes.clear( );
	/*
	std::ofstream dataFile( TIMING_OUTPUT_LOCATION + "CamTimeLog.txt" );

	if ( !dataFile.is_open( ) )
	{
		errBox( "ERROR: data file failed to open for rearrangement log!" );
	}
	dataFile << "imageToCamera "
		<< "grabTime "
		<< "grabToCrunch "
		<< "crunchingTime\n";
	for ( auto count : range( imageToMainTime.size( ) ) )
	{
		dataFile << imageToMainTime[count] << " " 
			<< mainToGrabTime[count] << " " 
			<< grabToCrunchTime[count] << " " 
			<< crunchCrunchingTime[count] << "\n";
	}
	dataFile.close( );
	*/
	//Sleep( 5000 );
	mainWindowFriend->stopRearranger( );
	wakeRearranger( );
	return 0;
}


void CameraWindow::startCamera()
{
	mainWindowFriend->getComm()->sendColorBox(Camera, 'Y');
	// turn some buttons off.
	CameraSettings.cameraIsOn( true );
	//
	CDC* dc = GetDC();
	pics.refreshBackgrounds( dc );
	ReleaseDC(dc);
	stats.reset();
	// I used to initialize the data logger here.
	analysisHandler.updateDataSetNumberEdit( dataHandler.getNextFileNumber() - 1 );
	double minKineticTime;
	Andor.armCamera( this, minKineticTime );
	CameraSettings.updateMinKineticCycleTime( minKineticTime );
	mainWindowFriend->getComm()->sendColorBox(Camera, 'G');
}


bool CameraWindow::getCameraStatus()
{
	return Andor.isRunning();
}


void CameraWindow::handleDblClick(NMHDR* info, LRESULT* lResult)
{
	analysisHandler.handleDoubleClick(&mainWindowFriend->getFonts(), CameraSettings.getSettings().picsPerRepetition );
}


void CameraWindow::listViewRClick( NMHDR* info, LRESULT* lResult )
{
	analysisHandler.handleRClick();
}


void CameraWindow::OnLButtonUp(UINT stuff, CPoint loc)
{
	alerts.stopSound();
}


// pics looks up the location itself.
void CameraWindow::OnRButtonUp( UINT stuff, CPoint clickLocation )
{
	alerts.stopSound();
	CDC* dc = GetDC();
	try
	{
		if (analysisHandler.buttonClicked())
		{
			coordinate loc = pics.handleRClick(clickLocation);
			if (loc.row != -1)
			{
				analysisHandler.handlePictureClick(loc);
				pics.redrawPictures(dc, selectedPixel, analysisHandler.getAnalysisLocs(), 
									 analysisHandler.getAtomGrid());
			}
		}
		else
		{
			coordinate box = pics.handleRClick(clickLocation);
			if (box.row != -1)
			{
				selectedPixel = box;
				pics.redrawPictures(dc, selectedPixel, analysisHandler.getAnalysisLocs(), 
									 analysisHandler.getAtomGrid( ) );
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
void CameraWindow::passAlertPress()
{
	alerts.handleCheckBoxPress();
}

/*
 *
 */
void CameraWindow::passTrigger()
{
	CameraSettings.handleTriggerControl(this);
	mainWindowFriend->updateConfigurationSavedStatus( false );
}


void CameraWindow::passPictureSettings( UINT id )
{
	handlePictureSettings( id );
	mainWindowFriend->updateConfigurationSavedStatus( false );
}


void CameraWindow::handlePictureSettings(UINT id)
{
	selectedPixel = { 0,0 };
	CameraSettings.handlePictureSettings(id, &Andor);
	if (CameraSettings.getSettings().picsPerRepetition == 1)
	{
		pics.setSinglePicture( this, CameraSettings.readImageParameters( this ) );
	}
	else
	{
		pics.setMultiplePictures( this, CameraSettings.readImageParameters( this ), 
								  CameraSettings.getSettings().picsPerRepetition);
	}
	pics.resetPictureStorage();
	std::array<int, 4> nums = CameraSettings.getPaletteNumbers();
	pics.setPalletes(nums);

	CRect rect;
	GetWindowRect(&rect);
	OnSize(0, rect.right - rect.left, rect.bottom - rect.top);
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


void CameraWindow::OnSize( UINT nType, int cx, int cy )
{
	SetRedraw( false );
	AndorRunSettings settings = CameraSettings.getSettings();
	stats.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, mainWindowFriend->getFonts() );
	CameraSettings.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, mainWindowFriend->getFonts() );
	box.rearrange( cx, cy, mainWindowFriend->getFonts() );
	pics.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, mainWindowFriend->getFonts() );
	alerts.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, mainWindowFriend->getFonts() );
	analysisHandler.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, mainWindowFriend->getFonts() );
	pics.setParameters( CameraSettings.readImageParameters( this ) );
	//RedrawWindow();
	CDC* dc = GetDC();
	try
	{
		pics.redrawPictures(dc, selectedPixel, analysisHandler.getAnalysisLocs(), analysisHandler.getAtomGrid( ) );
	}
	catch (Error& err)
	{
		mainWindowFriend->getComm()->sendError(err.what());
	}
	ReleaseDC(dc);
	timer.rearrange(settings.cameraMode, settings.triggerMode, cx, cy, mainWindowFriend->getFonts());
	SetRedraw();
	RedrawWindow();
}


void CameraWindow::setEmGain()
{
	try 
	{
		CameraSettings.setEmGain( &Andor );
	}
	catch (Error& exception)
	{
		errBox( exception.what() );
	}
}


void CameraWindow::handleMasterConfigSave(std::stringstream& configStream)
{
	imageParameters settings = CameraSettings.getSettings().imageSettings;
	configStream << settings.left << " " << settings.right << " " << settings.horizontalBinning << " ";
	configStream << settings.bottom << " " << settings.top << " " << settings.verticalBinning << "\n";
}


void CameraWindow::handleMasterConfigOpen(std::stringstream& configStream, double version)
{
	imageParameters settings = CameraSettings.getSettings().imageSettings;
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
		thrower("ERROR: Bad value seen in master configueration file while attempting to load camera settings!");
	}
}


DataLogger* CameraWindow::getLogger()
{
	return &dataHandler;
}

void CameraWindow::prepareCamera( ExperimentInput& input )
{
	if ( Andor.isRunning() )
	{
		thrower( "Camera is already running! Please Abort to restart.\r\n" );
	}
	if ( analysisHandler.getLocationSettingStatus() )
	{
		thrower( "Please finish selecting analysis points before starting the camera!\r\n" );
	}
	// make sure it's idle.
	try
	{
		Andor.queryIdentity();
		if ( ANDOR_SAFEMODE )
		{
			thrower( "DRV_IDLE" );
		}
	}
	catch ( Error& exception )
	{
		if ( exception.whatBare() != "DRV_IDLE" )
		{
			throw;
		}
	}

	CDC* dc = GetDC();
	pics.refreshBackgrounds(dc);
	ReleaseDC(dc);
	// I used to mandate use of a button to change image parameters. Now I don't have the button and just always 
	// update at this point.
	readImageParameters( );
	//
	CameraSettings.updatePassivelySetSettings();
	pics.setNumberPicturesActive( CameraSettings.getSettings().picsPerRepetition );
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
	input.camSettings = CameraSettings.getSettings();
	/// start the camera.
	Andor.setSettings( input.camSettings );
}

void CameraWindow::prepareAtomCruncher( ExperimentInput& input )
{
	input.cruncherInput = new atomCruncherInput;
	input.cruncherInput->plotterActive = plotThreadActive;
	input.cruncherInput->imageDims = CameraSettings.getSettings( ).imageSettings;
	atomCrunchThreadActive = true;
	input.cruncherInput->plotterNeedsImages = input.plotterInput->needsCounts;
	input.cruncherInput->cruncherThreadActive = &atomCrunchThreadActive;
	input.cruncherInput->imageQueue = &imageQueue;
	// options
	if ( input.masterInput )
	{
		input.cruncherInput->rearrangerActive = input.masterInput->rearrangeInfo.active;
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
	plotterPictureQueue.clear( );
	input.cruncherInput->plotterImageQueue = &plotterPictureQueue;
	plotterAtomQueue.clear( );
	input.cruncherInput->plotterAtomQueue = &plotterAtomQueue;
	rearrangerAtomQueue.clear( );
	input.cruncherInput->rearrangerAtomQueue = &rearrangerAtomQueue;
	input.cruncherInput->thresholds = CameraSettings.getThresholds();
	input.cruncherInput->picsPerRep = CameraSettings.getSettings().picsPerRepetition;
	input.cruncherInput->gridInfo = analysisHandler.getAtomGrid( );
	input.cruncherInput->catchPicTime = &crunchSeesTimes;
	input.cruncherInput->finTime = &crunchFinTimes;
	input.cruncherInput->rearrangerConditionWatcher = &rearrangerConditionVariable;
}


void CameraWindow::startAtomCruncher(ExperimentInput& input)
{
	UINT atomCruncherID;
	atomCruncherThreadHandle = (HANDLE)_beginthreadex( 0, 0, CameraWindow::atomCruncherProcedure, (void*)input.cruncherInput,
													   0, &atomCruncherID );
}

void CameraWindow::preparePlotter( ExperimentInput& input )
{
	/// start the plotting thread.
	plotThreadActive = true;
	imageQueue.clear();
	plotterAtomQueue.clear();
	input.plotterInput = new realTimePlotterInput;
	input.plotterInput->active = &plotThreadActive;
	input.plotterInput->imageQueue = &plotterPictureQueue;
	input.plotterInput->imageShape = CameraSettings.getSettings().imageSettings;
	input.plotterInput->picsPerVariation = mainWindowFriend->getRepNumber() * CameraSettings.getSettings().picsPerRepetition;
	input.plotterInput->variations = auxWindowFriend->getTotalVariationNumber();
	input.plotterInput->picsPerRep = CameraSettings.getSettings().picsPerRepetition;
	input.plotterInput->alertThreshold = alerts.getAlertThreshold();
	input.plotterInput->wantAlerts = alerts.alertsAreToBeUsed();
	input.plotterInput->comm = mainWindowFriend->getComm();
	input.plotterInput->plotLock = &plotLock;
	input.plotterInput->numberOfRunsToAverage = 5;
	input.plotterInput->plottingFrequency = analysisHandler.getPlotFreq( );
	if ( input.masterInput )
	{
		input.plotterInput->key = input.masterInput->key->getKeyValueArray( );
	}
	else
	{
		std::vector<double> dummyKey;
		// make a large dummy array to be used. In principle if the users uses a plotter without a master thread for
		// a long time this could crash... 
		dummyKey.resize( 1000 );
		input.plotterInput->key = dummyKey;
	}
	input.plotterInput->plotter = &plotter;
	input.plotterInput->atomQueue = &plotterAtomQueue;
	analysisHandler.fillPlotThreadInput( input.plotterInput );
}


void CameraWindow::startPlotterThread( ExperimentInput& input )
{
	UINT plottingThreadID;
	if ((input.plotterInput->atomGridInfo.topLeftCorner == coordinate(0,0) 
		 && input.plotterInput->analysisLocations.size() == 0) || input.plotterInput->plotInfo.size() == 0)
	{
		plotThreadActive = false;
	}
	else
	{
		// start the plotting thread
		plotThreadActive = true;
		plotThreadHandle = (HANDLE)_beginthreadex( 0, 0, DataAnalysisControl::plotterProcedure, (void*)input.plotterInput,
												   0, &plottingThreadID );
	}
}


AndorRunSettings CameraWindow::getRunSettings()
{
	return Andor.getSettings();
}


// this thread has one purpose: watch the image vector thread for new images, determine where atoms are, and pass them
// to the threads waiting on atom info.

// should consider modifying so that it can use an array of locations. At the moment doesn't.
UINT __stdcall CameraWindow::atomCruncherProcedure(void* inputPtr)
{
	atomCruncherInput* input = (atomCruncherInput*)inputPtr; 
	std::vector<long> monitoredPixelIndecies;

	if ( input->gridInfo.topLeftCorner == coordinate( 0, 0 ) )
	{
		return 0;
	}
	
	for ( auto columnInc : range( input->gridInfo.width ) )
	{
		for ( auto rowInc : range( input->gridInfo.height ) )
		{
			ULONG pixelRow = (input->gridInfo.topLeftCorner.row - 1) + rowInc * input->gridInfo.pixelSpacing;
			ULONG pixelColumn = (input->gridInfo.topLeftCorner.column - 1) + columnInc * input->gridInfo.pixelSpacing;
			if ( pixelRow >= input->imageDims.height || pixelColumn >= input->imageDims.width )
			{
				errBox( "ERROR: Grid appears to include pixels outside the image frame! Not allowed, seen by atom "
						"cruncher thread" );
				return 0;
			}
			int index = ((input->imageDims.height - 1 - pixelRow) * input->imageDims.width + pixelColumn);
			if ( index >= input->imageDims.width * input->imageDims.height )
			{
				// shouldn't happen after I finish debugging.
				errBox( "ERROR: Math error! Somehow, the pixel indexes appear within bounds, but the calculated index"
						" is larger than the image is!" );
				return 0;
			}
			monitoredPixelIndecies.push_back( index );
		}
	}
	/*
	The pixels get re-packed into an array like this:
	std::vector<std::vector<bool>> source;
	source.resize( info.targetRows );
	UINT count = 0;
	for ( auto rowCount : range( info.targetRows ) )
	{
		std::vector<bool> tempRow( info.targetCols );
		for ( auto& elem : tempRow )
		{
			bool atom = (*input->atomsQueue)[0][count++];
			elem = atom;
		}
		source[source.size( ) - 1 - rowCount] = tempRow;
	}
	*/
	UINT imageCount = 0;
	// loop watching the image queue.
	while (*input->cruncherThreadActive || input->imageQueue->size() != 0)
	{
		// if no images wait until images. Should probably change to be event based.
		if (input->imageQueue->size() == 0)
		{
			continue;
		}
		if ( imageCount % 2 == 0 )
		{
			input->catchPicTime->push_back( std::chrono::high_resolution_clock::now( ) );
		}
		// only contains the counts for the pixels being monitored.
		std::vector<long> tempImagePixels( monitoredPixelIndecies.size( ) );
		// only contains the boolean true/false of whether an atom passed a threshold or not. 
		std::vector<bool> tempAtomArray( monitoredPixelIndecies.size() );
		UINT count = 0;
		// scope for the lock_guard. I want to free the lock as soon as possible, so add extra small scope.
		{
			std::lock_guard<std::mutex> locker( *input->imageLock );				
			for ( auto pixelIndex : monitoredPixelIndecies )
			{
				tempImagePixels[count] = (*input->imageQueue)[0][pixelIndex];
				count++;
			}
		}
		count = 0;
		for ( auto& pix : tempImagePixels )
		{
			if ( pix >= input->thresholds[imageCount % input->picsPerRep] )
			{
				tempAtomArray[count] = true;
			}
			count++;
		}
		if ( input->rearrangerActive )
		{
			// copies the array if first pic of rep. Only looks at first picture because its rearranging. Could change
			// if we need to do funny experiments, just need to change rearranger handling.
			if ( imageCount % input->picsPerRep == 0 )
			{
				{
					std::lock_guard<std::mutex> locker( *input->rearrangerLock );
					(*input->rearrangerAtomQueue).push_back( tempAtomArray );
					input->rearrangerConditionWatcher->notify_all( );
				}
				input->finTime->push_back( std::chrono::high_resolution_clock::now( ) );
			}
		}
		if (input->plotterActive)
		{
			// copies the array. Right now I'm assuming that the thread always needs atoms, which is not a good 
			// assumption.
			(*input->plotterAtomQueue).push_back(tempAtomArray);

			if (input->plotterNeedsImages)
			{
				(*input->plotterImageQueue).push_back(tempImagePixels);
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
	imageParameters currentImageParameters = CameraSettings.readImageParameters( this );
	bool errCheck = false;
	for (UINT plotInc = 0; plotInc < plots.size(); plotInc++)
	{
		PlottingInfo tempInfoCheck(PLOT_FILES_SAVE_LOCATION + "\\" + plots[plotInc] + ".plot");
		if (tempInfoCheck.getPicNumber() != CameraSettings.getSettings().picsPerRepetition)
		{
			thrower( "ERROR: one of the plots selected, " + plots[plotInc] + ", is not built for the currently "
					 "selected number of pictures per experiment. Please revise either the current setting or the plot"
					 " file." );
		}
		tempInfoCheck.setGroups( analysisHandler.getAnalysisLocs() );
		std::vector<std::pair<UINT, UINT>> plotLocations = tempInfoCheck.getAllPixelLocations();
	}
	std::string dialogMsg;
	dialogMsg = "Starting Parameters:\r\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\r\n";
	dialogMsg += "Current Camera Temperature Setting: " + str(
		CameraSettings.getSettings().temperatureSetting ) + "\r\n";
	dialogMsg += "Exposure Times: ";
	for (UINT exposureInc = 0; exposureInc < this->CameraSettings.getSettings().exposureTimes.size(); exposureInc++)
	{
		dialogMsg += str( CameraSettings.getSettings().exposureTimes[exposureInc] * 1000 ) + ", ";
	}
	dialogMsg += "\r\n";
	dialogMsg += "Image Settings: " + str( currentImageParameters.left ) + " - " + str( currentImageParameters.right ) + ", "
		+ str( currentImageParameters.bottom ) + " - " + str( currentImageParameters.top ) + "\r\n";
	dialogMsg += "\r\n";
	dialogMsg += "Kintetic Cycle Time: " + str( CameraSettings.getSettings().kineticCycleTime ) + "\r\n";
	dialogMsg += "Pictures per Repetition: " + str( CameraSettings.getSettings().picsPerRepetition ) + "\r\n";
	dialogMsg += "Repetitions per Variation: " + str( CameraSettings.getSettings().totalPicsInVariation ) + "\r\n";
	dialogMsg += "Variations per Experiment: " + str( CameraSettings.getSettings().totalVariations ) + "\r\n";
	dialogMsg += "Total Pictures per Experiment: " + str( CameraSettings.getSettings().totalPicsInExperiment ) + "\r\n";
	dialogMsg += "Real-Time Atom Detection Thresholds: ";

	for (UINT exposureInc = 0; exposureInc < CameraSettings.getThresholds().size(); exposureInc++)
	{
		dialogMsg += str( CameraSettings.getThresholds()[exposureInc] ) + ", ";
	}

	dialogMsg += "\r\n";
	dialogMsg += "Current Plotting Options: \r\n";

	for (UINT plotInc = 0; plotInc < plots.size(); plotInc++)
	{
		dialogMsg += "\t" + plots[plotInc] + "\r\n";
	}

	return dialogMsg;
}


void CameraWindow::fillMasterThreadInput( MasterThreadInput* input )
{
	input->atomQueueForRearrangement = &rearrangerAtomQueue;
	input->rearrangerLock = &rearrangerLock;
	input->andorsImageTimes = &imageTimes;
	input->grabTimes = &imageGrabTimes;
	input->conditionVariableForRearrangement = &rearrangerConditionVariable;
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
	alerts.initialize( positions, this, false, id, tooltips );
	analysisHandler.initialize( positions, id, this, tooltips, false, mainWindowFriend->getRgbs() );
	CameraSettings.initialize( positions, id, this, tooltips );
	POINT position = { 480, 0 };
	stats.initialize( position, this, id, tooltips );
	positions.sPos = { 757, 0 };
	timer.initialize( positions, this, false, id, tooltips );
	position = { 757, 40 };
	pics.initialize( position, this, id, tooltips, mainWindowFriend->getBrushes()["Dark Green"] );
	//
	pics.setSinglePicture( this, CameraSettings.readImageParameters( this ) );
	Andor.setSettings( CameraSettings.getSettings() );

	// load the menu
	menu.LoadMenu( IDR_MAIN_MENU );
	SetMenu( &menu );

	// final steps
	//ShowWindow( SW_MAXIMIZE );
	SetTimer( NULL, 1000, NULL );

	CRect rect;
	GetWindowRect( &rect );
	OnSize( 0, rect.right - rect.left, rect.bottom - rect.top );
	return TRUE;
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
				pDC->SetTextColor( rgbs["White"] );
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
			pDC->SetTextColor(rgbs["White"]);
			pDC->SetBkColor(rgbs["Dark Grey"]);
			return *brushes["Dark Grey"];
		}
		default:
		{
			return *brushes["Solarized Base04"];
		}
	}
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
		imageParameters parameters = CameraSettings.readImageParameters( this );
		pics.setParameters( parameters );
	}
	catch (Error& exception)
	{
		//errBox( "Error!" );
		Communicator* comm = mainWindowFriend->getComm();
		comm->sendColorBox( Camera, 'R' );
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
