// created by Mark O. Brown

#include "stdafx.h"
#include "BaslerWindow.h"
#include "AuxiliaryWindow.h"
#include "MainWindow.h"
#include "AndorWindow.h"
#include "ScriptingWindow.h"
#include "afxdialogex.h"
#include "constants.h"


BaslerWindow::BaslerWindow( ) : picManager(true, "BASLER_PICTURE_MANAGER")
{}


// the message map. Allows me to handle various events in the system using functions I write myself.
BEGIN_MESSAGE_MAP( BaslerWindow, CDialogEx )
	
	ON_COMMAND_RANGE ( MENU_ID_RANGE_BEGIN, MENU_ID_RANGE_END, &BaslerWindow::passCommonCommand )

	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_MOUSEMOVE()

	ON_COMMAND( ID_BASLER_SOFTWARE_TRIGGER, BaslerWindow::handleSoftwareTrigger )
	ON_COMMAND( IDOK, &BaslerWindow::handleEnter )

	ON_CONTROL_RANGE(EN_CHANGE, IDC_MIN_BASLER_SLIDER_EDIT, IDC_MIN_BASLER_SLIDER_EDIT, &BaslerWindow::pictureRangeEditChange)
	ON_CONTROL_RANGE(EN_CHANGE, IDC_MAX_BASLER_SLIDER_EDIT, IDC_MAX_BASLER_SLIDER_EDIT, &BaslerWindow::pictureRangeEditChange)

	ON_MESSAGE( MainWindow::BaslerProgressMessageID, &BaslerWindow::handleNewPics )
	
	ON_CBN_SELENDOK( IDC_BASLER_EXPOSURE_MODE_COMBO, BaslerWindow::passExposureMode )
	ON_CBN_SELENDOK( IDC_BASLER_CAMERA_MODE_COMBO, BaslerWindow::passCameraMode)
	 
	ON_COMMAND( IDCANCEL, &BaslerWindow::handleClose )
	 
	ON_WM_RBUTTONUP()	
END_MESSAGE_MAP()


void BaslerWindow::handleBaslerAutoscaleSelection ( )
{
	if ( autoScaleBaslerPictureData )
	{
		autoScaleBaslerPictureData = false;
		mainWin->checkAllMenus ( ID_BASLER_AUTOSCALE, MF_UNCHECKED );
	}
	else
	{
		autoScaleBaslerPictureData = true;
		mainWin->checkAllMenus ( ID_BASLER_AUTOSCALE, MF_CHECKED );
	}
	picManager.setAutoScalePicturesOption ( autoScaleBaslerPictureData );
}

void BaslerWindow::setMenuCheck ( UINT menuItem, UINT itemState )
{
	menu.CheckMenuItem ( menuItem, itemState );
}


baslerSettings BaslerWindow::getCurrentSettings ( )
{
	return settingsCtrl.getCurrentSettings ( );
}


void BaslerWindow::loadFriends ( MainWindow* mainWin_, ScriptingWindow* scriptWin_, AndorWindow* camWin_, 
								 AuxiliaryWindow* auxWin_ )
{
	mainWin = mainWin_;
	scriptWin = scriptWin_;
	camWin = camWin_;
	auxWin = auxWin_;
}


void BaslerWindow::passCommonCommand ( UINT id )
{
	try
	{
		commonFunctions::handleCommonMessage ( id, this, mainWin, scriptWin, camWin, auxWin, this );
	}
	catch ( Error& err )
	{
		// catch any extra errors that handleCommonMessage doesn't explicitly handle.
		errBox ( err.what ( ) );
	}
}


void BaslerWindow::fillMotSizeInput ( baslerSettings& motSizeSettings )
{
	motSizeSettings.acquisitionMode = BaslerAcquisition::mode::Finite; 

	motSizeSettings.dims.left = 250;
	motSizeSettings.dims.right = 450;
	motSizeSettings.dims.top = 390;
	motSizeSettings.dims.bottom = 200;
	motSizeSettings.dims.horizontalBinning = 1;
	motSizeSettings.dims.verticalBinning = 1;
	
	motSizeSettings.exposureMode = BaslerAutoExposure::mode::Off; 
	motSizeSettings.exposureTime = 100;
	motSizeSettings.frameRate = 100;
	motSizeSettings.rawGain = settingsCtrl.unityGainSetting; 
	motSizeSettings.repCount = 50;
	motSizeSettings.triggerMode = BaslerTrigger::mode::External; 
}


void BaslerWindow::fillTemperatureMeasurementInput ( baslerSettings& settings )
{
	settings.acquisitionMode = BaslerAcquisition::mode::Finite;

	settings.dims.left = 250;
	settings.dims.right = 450;
	settings.dims.top = 440;
	settings.dims.bottom = 150;
	settings.dims.horizontalBinning = 1;
	settings.dims.verticalBinning = 1;

	settings.exposureMode = BaslerAutoExposure::mode::Off;
	settings.exposureTime = 50;
	settings.frameRate = 100;
	settings.rawGain = settingsCtrl.unityGainSetting;
	settings.repCount = 50;
	settings.triggerMode = BaslerTrigger::mode::External;
}


/*
Load the settings appropriate for the mot size measurement and then start the camera.
*/
void BaslerWindow::startTemporaryAcquisition ( baslerSettings motSizeSettings )
{
	handleDisarmPress ( );
	currentRepNumber = 0;
	runningAutoAcq = true;
	tempAcqSettings = motSizeSettings;
	cameraController->setParameters ( motSizeSettings );
	picManager.setParameters ( motSizeSettings.dims );
	triggerThreadInput* input = new triggerThreadInput;
	input->width = motSizeSettings.dims.width ( );
	input->height = motSizeSettings.dims.height ( );
	input->frameRate = motSizeSettings.frameRate;
	input->parent = this;
	input->runningFlag = &triggerThreadFlag;
	cameraController->armCamera ( input );
}


/*
Load the settings appropriate for the mot temperature measurement and then start the camera.
*/
void BaslerWindow::setCameraForMotTempMeasurement ( )
{
	auto prevSettings = settingsCtrl.getCurrentSettings ( );
	baslerSettings motTempSettings;
	motTempSettings.acquisitionMode = BaslerAcquisition::mode::Finite;
	motTempSettings.dims = settingsCtrl.ScoutFullResolution;
	motTempSettings.exposureMode = BaslerAutoExposure::mode::Off;
	motTempSettings.exposureTime = 100;
	motTempSettings.frameRate = 100;
	motTempSettings.rawGain = settingsCtrl.unityGainSetting;
	motTempSettings.repCount = 100;
	motTempSettings.triggerMode = BaslerTrigger::mode::External;
	handleDisarmPress ( );
	settingsCtrl.setSettings ( motTempSettings );
	startCamera ( );
}


// I think I can get rid of this...
void BaslerWindow::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );
}


void BaslerWindow::handleClose( )
{
	try
	{
		passCommonCommand ( ID_FILE_MY_EXIT );
	}
	catch ( Error& err )
	{
		errBox ( "Failed to close?!?!?\n" + err.trace() );
	}
}


void BaslerWindow::OnRButtonUp( UINT stuff, CPoint clickLocation )
{
	CDC* cdc = GetDC( );
	try
	{
		coordinate box = picManager.getClickLocation ( clickLocation );
		selectedPixel = box;
		//picManager.redrawPictures ( cdc, selectedPixel, analysisHandler.getAnalysisLocs ( ),
		//					  analysisHandler.getGrids ( ), false, mostRecentPicNum );
	}
	catch ( Error& err )
	{
		errBox ( err.what ( ) );
	}
	//picture.handleRightClick(clickLocation, cdc);
	ReleaseDC( cdc );
}


void BaslerWindow::handleEnter( ) { errBox( "\-.-/" ); }


// this is suppose see where the mouse is at a given time so that if it is hovering over a pixel I can display the pixel count.
void BaslerWindow::OnMouseMove( UINT flags, CPoint point )
{
	try
	{
		picManager.handleMouse ( point );
	}
	catch (Error& err)
	{
		errBox( "Error! " + err.trace() );
	}
}


void BaslerWindow::handleSoftwareTrigger()
{
	try
	{
		cameraController->softwareTrigger();
	}
	catch (Pylon::TimeoutException&)
	{
		errBox( "Software trigger timed out!" );
	}
}


void BaslerWindow::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* scrollbar )
{
	if (nSBCode == SB_THUMBPOSITION ||  nSBCode == SB_THUMBTRACK)
	{
		try
		{
			CDC* cdc = GetDC ( );
			picManager.handleScroll ( nSBCode, nPos, scrollbar, cdc );
			ReleaseDC( cdc );
		}
		catch (Error& err)
		{
			errBox( "Error! " + err.trace() );
		}
	}
}


void BaslerWindow::pictureRangeEditChange( UINT id )
{
	try
	{
		mainWin->updateConfigurationSavedStatus ( false );
		picManager.handleEditChange ( id );
		//picture.handleEditChange( id );
	}
	catch (Error& err)
	{
		errBox( "Error! " + err.trace() );
	}
}


void BaslerWindow::handleDisarmPress()
{
	try
	{
		mainWin->getComm ( )->sendColorBox ( System::Basler, 'B' );
		cameraController->disarm();
		triggerThreadFlag = false;
		isRunning = false;
		settingsCtrl.setStatus("Camera Status: Idle");
	}
	catch (Error& err)
	{
		errBox( "Error! " + err.trace() );
		settingsCtrl.setStatus("Camera Status: ERROR?!?!");
	}
}


void BaslerWindow::startDefaultAcquisition ( )
{ 
	try
	{
		if ( cameraController->isRunning ( ) )
		{
			handleDisarmPress ( );
		}
		mainWin->getComm ( )->sendColorBox ( System::Basler, 'Y' );
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
		tempSettings.frameRate = 20;
		#endif
		tempSettings.rawGain = 260;

		cameraController->setParameters ( tempSettings );
		picManager.setParameters ( tempSettings.dims );
		auto* dc = GetDC ( );
		picManager.drawBackgrounds ( dc );
		ReleaseDC ( dc );
		runExposureMode = tempSettings.exposureMode;
		imageWidth = tempSettings.dims.width ( );
		// only important in safemode
		triggerThreadFlag = true;

		triggerThreadInput* input = new triggerThreadInput;
		input->width = tempSettings.dims.width ( );
		input->height = tempSettings.dims.height ( );
		input->frameRate = tempSettings.frameRate;
		input->parent = this;
		input->runningFlag = &triggerThreadFlag;
		tempSettings.repCount = tempSettings.acquisitionMode == BaslerAcquisition::mode::Finite ? 
								tempSettings.repCount : SIZE_MAX;
		cameraController->armCamera ( input );
		settingsCtrl.setStatus ( "Camera Status: Armed..." );
		isRunning = true;
	}
	catch ( Error& err )
	{
		errBox ( err.what ( ) );
	}
}


LRESULT BaslerWindow::handleNewPics( WPARAM wParam, LPARAM lParam )
{
	Matrix<long>* imageMatrix = (Matrix<long>*)lParam;
 	mainWin->getComm ( )->sendColorBox ( System::Basler, 'G' );
 	long size = long( wParam );
  	try
 	{
 		currentRepNumber++;
 		CDC* cdc = GetDC();
		auto minMax = stats.update ( *imageMatrix, 0, { 0,0 }, currentRepNumber, settingsCtrl.getCurrentSettings ( ).repCount );
		
		picManager.drawBitmap( cdc, *imageMatrix, minMax );
 		picManager.updatePlotData ( );
 		picManager.drawDongles ( cdc, { 0,0 }, std::vector<coordinate>(), std::vector<atomGrid>(), 0 );
		ReleaseDC( cdc );
		if (runExposureMode == BaslerAutoExposure::mode::Continuous)
		{ 
			settingsCtrl.updateExposure( cameraController->getCurrentExposure() );
		}
 		settingsCtrl.setStatus("Camera Status: Acquiring Pictures.");
 		if (currentRepNumber % 10 == 0)
 		{
 			settingsCtrl.handleFrameRate();
 		}
 		if (!cameraController->isContinuous())
 		{
			camWin->getLogger ( )->writeBaslerPic ( *imageMatrix, 
									runningAutoAcq ? tempAcqSettings.dims : settingsCtrl.getCurrentSettings ( ).dims );
 		}
 		if (currentRepNumber == cameraController->getRepCounts())
 		{
 			cameraController->disarm();
 			isRunning = false;
			runningAutoAcq = false;
			triggerThreadFlag = false;
 			settingsCtrl.setStatus("Camera Status: Finished finite acquisition.");
			mainWin->getComm ( )->sendBaslerFin ( );
			mainWin->getComm ( )->sendColorBox ( System::Basler, 'B' );
 		}
		if ( stats.getMostRecentStats ( ).avgv < settingsCtrl.getMotThreshold ( ) )
		{
			motLoaded = false;
			loadMotConsecutiveFailures++;
			if ( camWin->wantsNoMotAlert ( ) )
			{
				if ( loadMotConsecutiveFailures > camWin->getNoMotThreshold ( ) )
				{
					mainWin->getComm ( )->sendNoMotAlert ( );
				}
			}
		}
		else
		{
			motLoaded = true;
			loadMotConsecutiveFailures = 0;
		}
		settingsCtrl.redrawMotIndicator ( );
 	}
	catch (Error& err)
	{
		errBox( err.trace() );
		settingsCtrl.setStatus("Camera Status: ERROR?!?!?");
	}
	OnPaint( );
	// always delete
	delete imageMatrix;
	return 0;
}


void BaslerWindow::passCameraMode()
{
	try
	{
		settingsCtrl.handleCameraMode();
	}
	catch (Error& err)
	{
		errBox( "Error! " + err.trace() );
	}
}


void BaslerWindow::passExposureMode()
{
	try
	{
		settingsCtrl.handleExposureMode();
	}
	catch (Error& err)
	{
		errBox( "Error! " + err.trace() );
	}
}


void BaslerWindow::handleArmPress()
{
	try
	{
		if ( cameraController->isRunning ( ) )
		{
			handleDisarmPress ( );
		}
		mainWin->getComm ( )->sendColorBox ( System::Basler, 'Y' );
		currentRepNumber = 0;
		baslerSettings tempSettings = settingsCtrl.loadCurrentSettings();
		cameraController->setParameters( tempSettings );
		imageParameters params;
		picManager.setParameters( tempSettings.dims );
		auto* dc = GetDC( );
		picManager.drawBackgrounds ( dc );
		ReleaseDC( dc );
		runExposureMode = tempSettings.exposureMode;
		imageWidth = tempSettings.dims.width();
		// only important in safemode
		triggerThreadFlag = true;

		triggerThreadInput* input = new triggerThreadInput;
		input->width = tempSettings.dims.width();
		input->height = tempSettings.dims.height();
		input->frameRate = tempSettings.frameRate;
		input->parent = this;
		input->runningFlag = &triggerThreadFlag;

		cameraController->armCamera( input );
		settingsCtrl.setStatus("Camera Status: Armed...");
		isRunning = true;
	}
	catch (Error& err)
	{
		errBox( err.trace() );
	}
}


bool BaslerWindow::baslerCameraIsRunning ( )
{
	if ( BASLER_SAFEMODE )
	{
		return isRunning;
	}
	return cameraController->isRunning ( );
}


bool BaslerWindow::baslerCameraIsContinuous ( )
{
	return cameraController->isContinuous ( );
}


void BaslerWindow::startCamera ( )
{ 
	if ( cameraController->isRunning( ) )
	{
		cameraController->disarm ( );
	}
	handleArmPress ( );
}

void BaslerWindow::OnSize( UINT nType, int cx, int cy )
{
	auto fonts = mainWin->getFonts ( );
	auto r = picManager.getPicArea ( );
	InvalidateRect ( &r );
	picManager.rearrange ( cx, cy, fonts );
	settingsCtrl.rearrange(cx, cy, fonts);
	stats.rearrange(cx, cy, fonts );
	if ( horGraph )
	{
		horGraph->rearrange( cx, cy, fonts );
	}
	if ( vertGraph )
	{
		vertGraph->rearrange( cx, cy, fonts );
	}
}


HBRUSH BaslerWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{
			int num = pWnd->GetDlgCtrlID ( );
			if ( num == IDC_MOT_LOADED_INDICATOR )
			{
				if ( !motLoaded )
				{
					pDC->SetBkColor ( _myRGBs[ "Red" ] );
					return *_myBrushes[ "Red" ];
				}
				else
				{
					pDC->SetBkColor ( _myRGBs[ "Green" ] );
					return *_myBrushes[ "Green" ];
				}
			}
			pDC->SetTextColor ( _myRGBs[ "Text" ] );
			pDC->SetBkColor ( _myRGBs[ "Static-Bkgd" ] );
			return *_myBrushes[ "Static-Bkgd" ];
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor( _myRGBs["BasWin-Text"]);
			pDC->SetBkColor( _myRGBs["Interactable-Bkgd"]);
			return *_myBrushes["Interactable-Bkgd"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor( _myRGBs["BasWin-Text"]);
			pDC->SetBkColor( _myRGBs["Interactable-Bkgd"]);
			return *_myBrushes["Interactable-Bkgd"];
		}
		default:
		{
			return *_myBrushes["Main-Bkgd"];
		}
	}
}


BOOL BaslerWindow::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	ShowWindow( SW_SHOWMAXIMIZED );
	try
	{
		initializeControls();
	}
	catch (Error& err)
	{
		errBox( err.trace() );
		EndDialog(-1);
		return FALSE;
	}
	CRect rect;
	GetWindowRect(&rect);
	OnSize(0, rect.right - rect.left, rect.bottom - rect.top);
	menu.LoadMenu ( IDR_MAIN_MENU );
	SetMenu ( &menu );
	return TRUE;  // return TRUE  unless you set the focus to a control
}


// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void BaslerWindow::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
		auto* dc = GetDC( );
		CRect size;
		GetClientRect( &size );
		picManager.paint ( dc, size, _myBrushes[ "Interactable-Bkgd" ] );
		ReleaseDC( dc );
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR BaslerWindow::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void BaslerWindow::handleOpeningConfig ( std::ifstream& configFile, Version ver )
{
	ProfileSystem::standardOpenConfig ( configFile, picManager.configDelim, &picManager, Version ( "4.0" ) );
	ProfileSystem::standardOpenConfig ( configFile, "BASLER_CAMERA_SETTINGS", &settingsCtrl, Version ( "4.0" ) );
}


void BaslerWindow::handleSavingConfig ( std::ofstream& configFile )
{
	picManager.handleSaveConfig ( configFile );
	settingsCtrl.handleSavingConfig ( configFile );
}


void BaslerWindow::initializeControls()
{
	#ifdef FIREWIRE_CAMERA
		SetWindowText("Firewire Basler Camera Control");
	#elif defined USB_CAMERA
		SetWindowText("USB Basler Camera Control");
	#endif

	cameraController = new BaslerCameras( this );
	if (!cameraController->isInitialized())
	{
		thrower ("ERROR: Camera not connected! Exiting program..." );
	}
	int id = 1000;
	POINT pos = { 0,0 };
	POINT cameraDims = cameraController->getCameraDimensions();
	settingsCtrl.initialize( pos, id, this, cameraDims.x, cameraDims.y, cameraDims);
	settingsCtrl.setSettings( cameraController->getDefaultSettings() );
	std::unordered_map<std::string, CFont*> fontDummy;
	std::vector<CToolTipCtrl*> toolTipDummy;
	stats.initialize( pos, this, id, toolTipDummy );

	POINT picPos = { 365, 0 };
	POINT dims = cameraController->getCameraDimensions();

	// scale to fill the window (approximately).
	dims.x *= 1.65;
	dims.y *= 1.65;

	CDC* cdc = GetDC( );
	
	picManager.initialize ( picPos, this, id, _myBrushes[ "Red" ], dims.x + picPos.x + 115, dims.y + picPos.y,
						   { IDC_MIN_BASLER_SLIDER_EDIT, IDC_MAX_BASLER_SLIDER_EDIT, NULL,NULL,NULL,NULL,NULL,NULL},
							mainWin->getBrightPlotPens(), mainWin->getPlotFont(), mainWin->getPlotBrushes() );
	picManager.setSinglePicture ( this, settingsCtrl.getCurrentSettings().dims );
	picManager.setPalletes ( { 1,1,1,1 } );
	this->RedrawWindow ( );
	picManager.drawBackgrounds( cdc );
	
	ReleaseDC( cdc );
}

