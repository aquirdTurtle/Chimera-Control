#include "stdafx.h"
#include "CameraWindow.h"
#include "commonFunctions.h"
#include "CameraSettingsControl.h"
#include "PlottingInfo.h"
#include "ATMCD32D.H"


IMPLEMENT_DYNAMIC(CameraWindow, CDialog)

BEGIN_MESSAGE_MAP(CameraWindow, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_VSCROLL()
	// menu stuff
	ON_COMMAND_RANGE(MENU_ID_RANGE_BEGIN, MENU_ID_RANGE_END, &CameraWindow::passCommonCommand)
	ON_COMMAND_RANGE(PICTURE_SETTINGS_ID_START, PICTURE_SETTINGS_ID_END, &CameraWindow::handlePictureSettings)
	// these ids all go to the same function.
	ON_CONTROL_RANGE(EN_CHANGE, IDC_PICTURE_1_MIN_EDIT, IDC_PICTURE_1_MIN_EDIT, &CameraWindow::handlePictureEditChange)
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_1_MAX_EDIT, IDC_PICTURE_1_MAX_EDIT, &CameraWindow::handlePictureEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_2_MIN_EDIT, IDC_PICTURE_2_MIN_EDIT, &CameraWindow::handlePictureEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_2_MAX_EDIT, IDC_PICTURE_2_MAX_EDIT, &CameraWindow::handlePictureEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_3_MIN_EDIT, IDC_PICTURE_3_MIN_EDIT, &CameraWindow::handlePictureEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_3_MAX_EDIT, IDC_PICTURE_3_MAX_EDIT, &CameraWindow::handlePictureEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_4_MIN_EDIT, IDC_PICTURE_4_MIN_EDIT, &CameraWindow::handlePictureEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_4_MAX_EDIT, IDC_PICTURE_4_MAX_EDIT, &CameraWindow::handlePictureEditChange )
	// 
	ON_COMMAND(IDC_SET_IMAGE_PARAMETERS_BUTTON, &CameraWindow::readImageParameters)
	ON_COMMAND(IDC_SET_EM_GAIN_BUTTON, &CameraWindow::setEmGain)
	ON_COMMAND(IDC_ALERTS_BOX, &CameraWindow::passAlertPress)
	ON_COMMAND(IDC_SET_TEMPERATURE_BUTTON, &CameraWindow::passSetTemperaturePress)

	ON_CBN_SELENDOK(IDC_TRIGGER_COMBO, &CameraWindow::passTrigger)
	ON_CBN_SELENDOK( IDC_CAMERA_MODE_COMBO, &CameraWindow::passCameraMode )

	ON_REGISTERED_MESSAGE( eCameraFinishMessageID, &CameraWindow::onCameraFinish )
	ON_REGISTERED_MESSAGE( eCameraProgressMessageID, &CameraWindow::onCameraProgress )

	ON_WM_RBUTTONUP()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_PLOTTING_LISTVIEW, &CameraWindow::listViewLClick)
	ON_NOTIFY(NM_DBLCLK, IDC_PLOTTING_LISTVIEW, &CameraWindow::listViewDblClick)
END_MESSAGE_MAP()


void CameraWindow::passCameraMode()
{
	CameraSettings.handleModeChange(this);
}


void CameraWindow::abortCameraRun()
{
	int status;
	Andor.getStatus(status);
	if (ANDOR_SAFEMODE)
	{
		status = DRV_ACQUIRING;
	}
	if (status == DRV_ACQUIRING)
	{
		Andor.abortAcquisition();
		timer.setTimerDisplay( "Aborted" );
		Andor.setIsRunningState( false );
		//ePlotThreadExitIndicator = false;
		//eThreadExitIndicator = false;
		// Wait until plotting thread is complete.
		//WaitForSingleObject( ePlottingThreadHandle, INFINITE );
		// camera is no longer running.
		
		/*
		if (eExperimentData.closeFits( errorMessage ))
		{
			appendText( errorMessage, IDC_ERROR_EDIT );
		}
		*/
		
		if (Andor.getSettings().cameraMode != "Continuous Single Scans Mode")
		{
			/*
			int answer = MessageBox( 0, "Acquisition Aborted. Delete Data (fits_#) and (key_#) files for this run?", 0, MB_YESNO );
			if (answer == IDYES)
			{
				if (eExperimentData.deleteFitsAndKey( errorMessage ))
				{
					appendText( errorMessage, IDC_ERROR_EDIT );
				}
				else
				{
					appendText( "Deleted .fits and copied key file for this run.", IDC_STATUS_EDIT );
				}
			}
			*/
		}
	}
}

bool CameraWindow::cameraIsRunning()
{
	return Andor.isRunning();
}


void CameraWindow::handlePictureEditChange( UINT id )
{
	pics.handleEditChange( id );
}


LRESULT CameraWindow::onCameraProgress( WPARAM wParam, LPARAM lParam)
{
	unsigned long long pictureNumber = lParam;
	Andor.updatePictureNumber( pictureNumber );
	std::vector<std::vector<long>> picData = Andor.acquireImageData();
	CDC* drawer = GetDC();
	AndorRunSettings currentSettings = Andor.getSettings();
	if (realTimePic)
	{
		// draw the most recent pic.
		pics.drawPicture( drawer, pictureNumber % currentSettings.picsPerRepetition, picData.back() );
		timer.update( pictureNumber / currentSettings.picsPerRepetition, currentSettings.repetitionsPerVariation, 
							currentSettings.totalVariations, currentSettings.picsPerRepetition );
		stats.update( picData.back(), pictureNumber % currentSettings.picsPerRepetition, selectedPixel,
					  currentSettings.imageSettings.width,
					  pictureNumber / currentSettings.picsPerRepetition,
					  currentSettings.totalPicsInExperiment / currentSettings.picsPerRepetition );
	}
	else if (pictureNumber % currentSettings.picsPerRepetition == 0)
	{
		int counter = 0;
		for (auto data : picData)
		{
			stats.update( data, counter, selectedPixel, currentSettings.imageSettings.width,
						  pictureNumber / currentSettings.picsPerRepetition, 
						  currentSettings.totalPicsInExperiment / currentSettings.picsPerRepetition );
			pics.drawPicture( drawer, counter, data );
			pics.drawDongles( this, selectedPixel );
			counter++;
		}
		timer.update( pictureNumber / currentSettings.picsPerRepetition, currentSettings.repetitionsPerVariation,
					  currentSettings.totalVariations, currentSettings.picsPerRepetition );
	}
	return 0;
}

LRESULT CameraWindow::onCameraFinish( WPARAM wParam, LPARAM lParam )
{
	// notify the andor object that it is done.
	Andor.onFinish();
	Andor.pauseThread();
	mainWindowFriend->getComm()->sendColorBox( { /*niawg*/'-', /*camera*/'B', /*intensity*/'-' } );
	mainWindowFriend->getComm()->sendStatus( "Camera has finished taking pictures and is no longer running.\r\n" );
	CameraSettings.cameraIsOn( false );
	return 0;
}

void CameraWindow::startCamera()
{
	// turn some buttons off.
	CameraSettings.cameraIsOn( true );
	pics.refreshBackgrounds( this );
	stats.reset();
	Andor.setSystem( this );
}

bool CameraWindow::getCameraStatus()
{
	return Andor.isRunning();
}

void CameraWindow::listViewDblClick(NMHDR* info, LRESULT* lResult)
{
	dataHandler.handleDoubleClick();
}

void CameraWindow::listViewLClick( NMHDR* info, LRESULT* lResult )
{
	dataHandler.handleRClick();
}

// pics looks up the location itself.
void CameraWindow::OnRButtonUp( UINT stuff, CPoint clickLocation )
{
	std::pair<int, int> box = pics.handleRClick( clickLocation );
	if (box.first != -1)
	{
		selectedPixel = box;
		pics.redrawPictures(this, selectedPixel);
	}
}

/*
 *
 */
void CameraWindow::passSetTemperaturePress()
{
	CameraSettings.handleSetTemperaturePress();
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
}


void CameraWindow::handlePictureSettings(UINT id)
{
	CameraSettings.handlePictureSettings(id, &Andor);
	if (CameraSettings.getSettings().picsPerRepetition == 1)
	{
		pics.setSinglePicture(this, selectedPixel, CameraSettings.readImageParameters( this ) );
	}
	else
	{
		pics.setMultiplePictures( this, selectedPixel, CameraSettings.readImageParameters( this ) );
	}
}


BOOL CameraWindow::PreTranslateMessage(MSG* pMsg)
{
	for (int toolTipInc = 0; toolTipInc < tooltips.size(); toolTipInc++)
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
	AndorRunSettings settings = CameraSettings.getSettings();
	stats.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, this->mainWindowFriend->getFonts() );
	CameraSettings.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, this->mainWindowFriend->getFonts() );
	box.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, this->mainWindowFriend->getFonts() );
	pics.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, this->mainWindowFriend->getFonts() );
	alerts.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, this->mainWindowFriend->getFonts() );
	dataHandler.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, this->mainWindowFriend->getFonts() );

	pics.setParameters( CameraSettings.readImageParameters( this ) );
	RedrawWindow();
	pics.redrawPictures( this, selectedPixel );
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

void CameraWindow::prepareCamera()
{
	if ( Andor.isRunning() )
	{
		thrower( "System is already running! Please Abort to restart.\r\n" );
	}
	if ( dataHandler.getLocationSettingStatus() )
	{
		thrower( "Please finish selecting analysis points!" );
	}
	// biggest check here, camera settings includes a lot of things.
	CameraSettings.checkIfReady();
	try
	{
		Andor.getStatus();
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
	pics.refreshBackgrounds(this);
	//
	pics.setNumberPicturesActive( CameraSettings.getSettings().picsPerRepetition );
	/// start the plotting thread.
	/*
	// set default colors and linewidths on plots
	this->currentRepetitionNumber = 1;
	// Create the Mutex. This function just opens the mutex if it already exists.
	plottingMutex = CreateMutexEx( 0, NULL, FALSE, MUTEX_ALL_ACCESS );
	// prepare for start of thread.
	plotThreadExitIndicator = true;
	/// TODO!
	// start thread.

	unsigned int * plottingThreadID = NULL;
	std::vector<std::string>* argPlotNames;
	argPlotNames = new std::vector<std::string>;
	for (int plotNameInc = 0; plotNameInc < eCurrentPlotNames.size(); plotNameInc++)
	{
	argPlotNames->push_back(eCurrentPlotNames[plotNameInc]);
	}
	// clear this before starting the thread.
	eImageVecQueue.clear();
	// start the plotting thread.
	ePlottingThreadHandle = (HANDLE)_beginthreadex(0, 0, plotterProcedure, argPlotNames, 0,
	plottingThreadID);
	*/
	/// start the camera.
	Andor.setSettings( CameraSettings.getSettings() );
}

std::string CameraWindow::getStartMessage()
{
	// get selected plots
	std::vector<std::string> plots = dataHandler.getActivePlotList();
	imageParameters currentImageParameters = CameraSettings.readImageParameters( this );
	bool errCheck = false;
	for (int plotInc = 0; plotInc < plots.size(); plotInc++)
	{
		PlottingInfo tempInfoCheck;
		if (tempInfoCheck.loadPlottingInfoFromFile( PLOT_FILES_SAVE_LOCATION + plots[plotInc] + ".plot" ) == -1)
		{
			thrower( "Failed to load a plot file: " + plots[plotInc] );
		}
		if (tempInfoCheck.getPictureNumber() != this->CameraSettings.getSettings().picsPerRepetition)
		{
			thrower( "ERROR: one of the plots selected, " + plots[plotInc] + ", is not built for the currently "
					 "selected number of pictures per experiment. Please revise either the current setting or the plot"
					 " file." );
		}
		tempInfoCheck.setGroups( dataHandler.getAtomLocations() );
		std::vector<std::pair<int, int>> plotLocations = tempInfoCheck.getAllPixelLocations();
	}
	std::string dialogMsg;
	dialogMsg = "Starting Parameters:\r\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\r\n";
	dialogMsg += "Current Camera Temperature Setting: " + str(
		CameraSettings.getSettings().temperatureSetting ) + "\r\n";
	dialogMsg += "Exposure Times: ";
	for (int exposureInc = 0; exposureInc < this->CameraSettings.getSettings().exposureTimes.size(); exposureInc++)
	{
		dialogMsg += str( CameraSettings.getSettings().exposureTimes[exposureInc] * 1000 ) + ", ";
	}
	dialogMsg += "\r\n";
	dialogMsg += "Image Settings: " + str( currentImageParameters.leftBorder ) + " - " + str( currentImageParameters.rightBorder ) + ", "
		+ str( currentImageParameters.topBorder ) + " - " + str( currentImageParameters.bottomBorder ) + "\r\n";
	dialogMsg += "\r\n";
	dialogMsg += "Kintetic Cycle Time: " + str( CameraSettings.getSettings().kinetiCycleTime ) + "\r\n";
	dialogMsg += "Pictures per Repetition: " + str( CameraSettings.getSettings().picsPerRepetition ) + "\r\n";
	dialogMsg += "Repetitions per Variation: " + str( CameraSettings.getSettings().totalPicsInVariation ) + "\r\n";
	dialogMsg += "Variations per Experiment: " + str( CameraSettings.getSettings().totalVariations ) + "\r\n";
	dialogMsg += "Total Pictures per Experiment: " + str( CameraSettings.getSettings().totalPicsInExperiment ) + "\r\n";
	dialogMsg += "Real-Time Atom Detection Thresholds: ";

	for (int exposureInc = 0; exposureInc < CameraSettings.getThresholds().size(); exposureInc++)
	{
		dialogMsg += str( CameraSettings.getThresholds()[exposureInc] ) + ", ";
	}

	dialogMsg += "\r\n";
	dialogMsg += "Current Plotting Options: \r\n";

	for (int plotInc = 0; plotInc < plots.size(); plotInc++)
	{
		dialogMsg += "\t" + plots[plotInc] + "\r\n";
	}

	return dialogMsg;
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


std::vector<CToolTipCtrl*> CameraWindow::getToolTips()
{
	return tooltips;
}


BOOL CameraWindow::OnInitDialog()
{
	cameraPositions positions;
	// all of the initialization functions increment and use the id, so by the end it will be 3000 + # of controls.
	int id = 3000;
	positions.sPos = { 0, 0 };
	box.initialize( positions.sPos, id, this, 480, mainWindowFriend->getFonts(), tooltips );
	positions.videoPos = positions.amPos = positions.seriesPos = positions.sPos;
	alerts.initialize( positions, this, false, id, mainWindowFriend->getFonts(), tooltips );
	dataHandler.initialize( positions, id, this, mainWindowFriend->getFonts(), tooltips, false );
	CameraSettings.initialize( positions, id, this, mainWindowFriend->getFonts(), tooltips );
	POINT position = { 480, 0 };
	stats.initialize( position, this, id, mainWindowFriend->getFonts(), tooltips );
	positions.amPos = positions.seriesPos = positions.videoPos = { 757, 0 };
	timer.initialize( positions, this, false, id, mainWindowFriend->getFonts(), tooltips );
	position = positions.seriesPos;
	pics.initialize( position, this, id, mainWindowFriend->getFonts(), tooltips, mainWindowFriend->getBrushes()["Dark Green"] );
	pics.setSinglePicture( this, { 0,0 }, CameraSettings.readImageParameters( this ) );
	// load the menu
	CMenu menu;
	menu.LoadMenu( IDR_MAIN_MENU );
	SetMenu( &menu );
	// final steps
	ShowWindow( SW_MAXIMIZE );
	SetTimer( NULL, 1000, NULL );

	CRect rect;
	GetWindowRect( &rect );
	OnSize( 0, rect.right - rect.left, rect.bottom - rect.top );
	return TRUE;
}


void CameraWindow::redrawPictures( bool andGrid )
{
	pics.refreshBackgrounds( this );
	if (andGrid)
	{
		pics.drawGrids( this );
	}
}


HBRUSH CameraWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	std::unordered_map<std::string, CBrush*> brushes = mainWindowFriend->getBrushes();
	std::unordered_map<std::string, COLORREF> rgbs = mainWindowFriend->getRGB();
	HBRUSH result;
	int num = pWnd->GetDlgCtrlID();

	result = *CameraSettings.handleColor(num, pDC, mainWindowFriend->getBrushes(), mainWindowFriend->getRGB());
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
			pDC->SetTextColor(rgbs["White"]);
			pDC->SetBkColor(rgbs["Dark Green"]);
			return *brushes["Dark Green"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor(rgbs["White"]);
			pDC->SetBkColor(rgbs["Dark Grey"]);
			return *brushes["Dark Grey"];
		}
		default:
		{
			return *brushes["Light Grey"];
		}
	}
}

void CameraWindow::passCommonCommand(UINT id)
{
	try
	{
		commonFunctions::handleCommonMessage( id, this, mainWindowFriend, scriptingWindowFriend, this );
	}
	catch (Error& err)
	{
		// catch any extra errors that handleCommonMessage doesn't explicitly handle.
		errBox( err.what() );
	}
}

void CameraWindow::readImageParameters()
{
	redrawPictures(false);
	try
	{
		imageParameters parameters = CameraSettings.readImageParameters( this );
		pics.setParameters( parameters );
	}
	catch (Error& exception)
	{
		errBox( "Error!" );
		Communicator* comm = mainWindowFriend->getComm();
		comm->sendColorBox( { /*niawg*/'-', /*camera*/'R', /*intensity*/'-' } );
		comm->sendError( exception.whatStr() + "\r\n" );
	}
	pics.drawGrids( this );
}

void CameraWindow::changeBoxColor(colorBoxes<char> colors)
{
	box.changeColor(colors);
}
