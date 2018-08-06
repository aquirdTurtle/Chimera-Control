
// Basler-ControlDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BaslerWindow.h"
#include "AuxiliaryWindow.h"
#include "MainWindow.h"
#include "CameraWindow.h"
#include "ScriptingWindow.h"
#include "afxdialogex.h"
#include "constants.h"


BaslerWindow::BaslerWindow( ) : picManager(true)
{

}


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
	ON_COMMAND( IDC_BASLER_SET_ANALYSIS_LOCATIONS, &BaslerWindow::passSetLocationsButton)

	ON_CONTROL_RANGE(EN_CHANGE, IDC_MIN_BASLER_SLIDER_EDIT, IDC_MIN_BASLER_SLIDER_EDIT, &BaslerWindow::pictureRangeEditChange)
	ON_CONTROL_RANGE(EN_CHANGE, IDC_MAX_BASLER_SLIDER_EDIT, IDC_MAX_BASLER_SLIDER_EDIT, &BaslerWindow::pictureRangeEditChange)

	ON_REGISTERED_MESSAGE( ACE_PIC_READY, &BaslerWindow::handleNewPics )
	
	ON_CBN_SELENDOK( IDC_BASLER_EXPOSURE_MODE_COMBO, BaslerWindow::passExposureMode )
	ON_CBN_SELENDOK( IDC_BASLER_CAMERA_MODE_COMBO, BaslerWindow::passCameraMode)

	ON_COMMAND( IDCANCEL, &BaslerWindow::handleClose )

	ON_WM_RBUTTONUP()	
END_MESSAGE_MAP()


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
	motSizeSettings.dimensions = settingsCtrl.ScoutFullResolution;
	motSizeSettings.exposureMode = BaslerAutoExposure::mode::Off;
	motSizeSettings.exposureTime = 1e-3;
	motSizeSettings.frameRate = 100;
	motSizeSettings.rawGain = settingsCtrl.unityGainSetting;
	motSizeSettings.repCount = 100;
	motSizeSettings.triggerMode = BaslerTrigger::mode::AutomaticSoftware;
}



/*
Load the settings appropriate for the mot size measurement and then start the camera.
*/
void BaslerWindow::measureMotSize ( baslerSettings motSizeSettings )
{
	auto prevSettings = settingsCtrl.getCurrentSettings ( );
	handleDisarmPress ( );
	settingsCtrl.setSettings ( motSizeSettings );
	startCamera ( );
	// finish...
	settingsCtrl.setSettings ( prevSettings );
	// wait for the measurement to finish.
	auto res = WaitForSingleObject ( cameraController->getCameraThreadObj ( ), 1E5 );
}


/*
Load the settings appropriate for the mot temperature measurement and then start the camera.
*/
void BaslerWindow::setCameraForMotTempMeasurement ( )
{
	auto prevSettings = settingsCtrl.getCurrentSettings ( );
	baslerSettings motTempSettings;
	motTempSettings.acquisitionMode = BaslerAcquisition::mode::Finite;
	motTempSettings.dimensions = settingsCtrl.ScoutFullResolution;
	motTempSettings.exposureMode = BaslerAutoExposure::mode::Off;
	motTempSettings.exposureTime = 1e-3;
	motTempSettings.frameRate = 100;
	motTempSettings.rawGain = settingsCtrl.unityGainSetting;
	motTempSettings.repCount = 100;
	motTempSettings.triggerMode = BaslerTrigger::mode::External;
	handleDisarmPress ( );
	settingsCtrl.setSettings ( motTempSettings );
	startCamera ( );
}


void BaslerWindow::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );
}


void BaslerWindow::handleClose( )
{
	auto res = promptBox("Close the Camera Application?", MB_OKCANCEL );
	if ( res == IDOK )
	{
		CDialog::OnCancel( );
	}
}


void BaslerWindow::passSetLocationsButton()
{
	//picture.handleButtonClick();
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


void BaslerWindow::handleEnter( ) { errBox( "hi!" ); }


// this is suppose see where the mouse is at a given time so that if it is hovering over a pixel I can display the pixel count.
void BaslerWindow::OnMouseMove( UINT flags, CPoint point )
{
	try
	{
		picManager.handleMouse ( point );
	}
	catch (Error& err)
	{
		errBox( "Error! " + err.whatStr() );
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
		//int id = scrollbar->GetDlgCtrlID();
		try
		{
			picManager.handleScroll ( nSBCode, nPos, scrollbar );
			CDC* cdc = GetDC( );
			//picManager.redrawPictures (cdc, selectedPixel, ,,0 );
			ReleaseDC( cdc );
		}
		catch (Error& err)
		{
			errBox( "Error! " + err.whatStr() );
		}
	}
}


void BaslerWindow::pictureRangeEditChange( UINT id )
{
	try
	{
		picManager.handleEditChange ( id );
		//picture.handleEditChange( id );
	}
	catch (Error& err)
	{
		errBox( "Error! " + err.whatStr() );
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
		errBox( "Error! " + err.whatStr() );
		settingsCtrl.setStatus("Camera Status: ERROR?!?!");
	}
}


LRESULT BaslerWindow::handleNewPics( WPARAM wParam, LPARAM lParam )
{
	Matrix<long>* imageMatrix = (Matrix<long>*)lParam;
	imageMatrix->updateString ( );
 	mainWin->getComm ( )->sendColorBox ( System::Basler, 'G' );
 	long size = long( wParam );
  	try
 	{
 		currentRepNumber++;
 		CDC* cdc = GetDC();
		picManager.drawBitmap( cdc, *imageMatrix );
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
 			camWin->getLogger ( )->writeBaslerPic ( *imageMatrix, settingsCtrl.getCurrentSettings ( ).dimensions );
 		}
 		if (currentRepNumber == cameraController->getRepCounts())
 		{
 			cameraController->disarm();
 			isRunning = false;
 			settingsCtrl.setStatus("Camera Status: Finished finite acquisition.");
			mainWin->getComm ( )->sendBaslerFin ( );
 		}
 	}
	catch (Error& err)
	{
		errBox( err.what() );
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
		errBox( "Error! " + err.whatStr() );
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
		errBox( "Error! " + err.whatStr() );
	}
}


void BaslerWindow::handleArmPress()
{
	try
	{
		mainWin->getComm ( )->sendColorBox ( System::Basler, 'Y' );
		currentRepNumber = 0;
		baslerSettings tempSettings = settingsCtrl.loadCurrentSettings();
		cameraController->setParameters( tempSettings );
		imageParameters params;
		picManager.setParameters( tempSettings.dimensions );
		auto* dc = GetDC( );
		picManager.drawBackgrounds ( dc );
		ReleaseDC( dc );
		runExposureMode = tempSettings.exposureMode;
		imageWidth = tempSettings.dimensions.width();
		triggerThreadFlag = true;

		triggerThreadInput* input = new triggerThreadInput;
		input->width = tempSettings.dimensions.width();
		input->height = tempSettings.dimensions.height();
		input->frameRate = tempSettings.frameRate;
		input->parent = this;
		input->runningFlag = &triggerThreadFlag;

		cameraController->armCamera( input );
		settingsCtrl.setStatus("Camera Status: Armed...");
		isRunning = true;
	}
	catch (Error& err)
	{
		errBox( err.what() );
	}
}




bool BaslerWindow::baslerCameraIsRunning ( )
{
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
	brushMap brushes = mainWin->getBrushes ( );
	rgbMap rgbs = mainWin->getRgbs ( );
	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{
			pDC->SetTextColor(rgbs["White"]);
			pDC->SetBkColor( rgbs["Medium Grey"]);
			return *brushes["Medium Grey"];
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor( rgbs["White"]);
			pDC->SetBkColor( rgbs["Dark Grey"]);
			return *brushes["Dark Grey"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor( rgbs["White"]);
			pDC->SetBkColor( rgbs["Dark Grey"]);
			return *brushes["Dark Grey"];
		}
		default:
		{
			return *brushes["Solarized Base04"];
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
		errBox( err.what() );
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
		picManager.paint ( dc, size, mainWin->getBrushes()[ "Black" ] );
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
	picManager.handleOpenConfig ( configFile, ver );
	settingsCtrl.handleOpeningConfig ( configFile, ver );
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

	CMenu menu;
	SetMenu( &menu );
	cameraController = new BaslerCameras( this );
	if (!cameraController->isInitialized())
	{
		thrower("ERROR: Camera not connected! Exiting program..." );
	}
	int id = 1000;
	POINT pos = { 0,0 };
	POINT cameraDims = cameraController->getCameraDimensions();
	settingsCtrl.initialize( pos, id, this, cameraDims.x, cameraDims.y, cameraDims);
	settingsCtrl.setSettings( cameraController->getDefaultSettings() );
	std::unordered_map<std::string, CFont*> fontDummy;
	std::vector<CToolTipCtrl*> toolTipDummy;
	//stats.initialize( pos, this, id, fontDummy, toolTipDummy );

	POINT picPos = { 365, 0 };
	POINT dims = cameraController->getCameraDimensions();

	// scale to fill the window (approximately).
	dims.x *= 1.7;
	dims.y *= 1.7;

	CDC* cdc = GetDC( );
	auto brushes = mainWin->getBrushes ( );
	
	picManager.initialize ( picPos, this, id, brushes[ "Red" ], dims.x + picPos.x + 115, dims.y + picPos.y,
							mainWin->getBrightPlotPens(), mainWin->getPlotFont(), mainWin->getPlotBrushes() );
	picManager.setSinglePicture ( this, settingsCtrl.getCurrentSettings().dimensions );
	picManager.setPalletes ( { 1,1,1,1 } );
	this->RedrawWindow ( );
	//picture.drawBackground( cdc );
	ReleaseDC( cdc );
}

