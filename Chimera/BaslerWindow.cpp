
// Basler-ControlDlg.cpp : implementation file
//

#include "stdafx.h"
//#include "BaslerControlApp.h"
#include "BaslerWindow.h"
#include "AuxiliaryWindow.h"
#include "MainWindow.h"
#include "CameraWindow.h"
#include "ScriptingWindow.h"
#include "afxdialogex.h"
#include "constants.h"


BaslerWindow::BaslerWindow( /*=NULL*/ ) 
{

}


void BaslerWindow::loadFriends ( MainWindow* mainWin_, ScriptingWindow* scriptWin_, CameraWindow* camWin_, AuxiliaryWindow* auxWin_ )
{
	mainWin = mainWin_;
	scriptWin = scriptWin_;
	camWin = camWin_;
	auxWin = auxWin_;
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
		cameraController->disarm();
		triggerThreadFlag = false;
		isRunning = false;
		settings.setStatus("Camera Status: Idle");
	}
	catch (Error& err)
	{
		errBox( "Error! " + err.whatStr() );
		settings.setStatus("Camera Status: ERROR?!?!");
	}
}


LRESULT BaslerWindow::handleNewPics( WPARAM wParam, LPARAM lParam )
{
	Matrix<long>* imageMatrix = (Matrix<long>*)lParam;
	std::vector<long>* image = (std::vector<long>*) lParam;
 	long size = long( wParam );
 	try
	{
		currentRepNumber++;
		CDC* cdc = GetDC();
		//picManager.drawDongles ( cdc, selectedPixel, , , 0);
		picManager.drawBitmap( cdc, *imageMatrix );
		//picture.drawCircle ( cdc, selectedLocation );
		//picture.drawAnalysisMarkers ( dc, analysisLocs, grids );
		//picture.drawPicNum ( dc, pictureNumber - getNumberActive ( ) + count++ );
		//picture.updatePlotData( );
		//picture.drawDongles( cdc, *imageMatrix );
		ReleaseDC( cdc );
		//picture.setHoverValue();
		if (runExposureMode == "Auto Exposure Continuous")
		{
			settings.updateExposure( cameraController->getCurrentExposure() );
		}
		
		//stats.update( *imageMatrix, 0, { 0,0 }, settings.getCurrentSettings().dimensions.horBinNumber, currentRepNumber,
		//			  cameraController->getRepCounts() );
		settings.setStatus("Camera Status: Acquiring Pictures.");
		if (currentRepNumber % 10 == 0)
		{
			settings.handleFrameRate();
		}
		if (currentRepNumber == 1 && !cameraController->isContinuous())
		{
			//saver.save( *imageMatrix, imageWidth );
		}
		else if (!cameraController->isContinuous())
		{
			//saver.append( *imageMatrix, imageWidth );
		}
		if (currentRepNumber == cameraController->getRepCounts())
		{
			cameraController->disarm();
			//saver.close();
			isRunning = false;
			settings.setStatus("Camera Status: Finished finite acquisition.");
		}
	}
	catch (Error& err)
	{
		errBox( err.what() );
		settings.setStatus("Camera Status: ERROR?!?!?");
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
		settings.handleCameraMode();
		//saver.handleModeChange(settings.loadCurrentSettings(cameraController->getCameraDimensions()).cameraMode);
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
		settings.handleExposureMode();
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
		currentRepNumber = 0;
		baslerSettings tempSettings = settings.loadCurrentSettings();
		cameraController->setParameters( tempSettings );
		imageParameters params;
		picManager.setParameters( tempSettings.dimensions );
		//( tempSettings.dimensions );
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
		settings.setStatus("Camera Status: Armed...");
		isRunning = true;
	}
	catch (Error& err)
	{
		errBox( err.what() );
	}
}


void BaslerWindow::OnSize( UINT nType, int cx, int cy )
{
	auto fonts = mainWin->getFonts ( );
	picManager.rearrange ( cx, cy, fonts );
	//picture.rearrange("", "", cx, cy, fonts);
	settings.rearrange(cx, cy, fonts);
	stats.rearrange(cx, cy, fonts );
	//saver.rearrange(cx, cy, mainFonts);
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
		//picture.paint( dc, size, mainBrushes["Black"] );
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
	settings.handleOpeningConfig ( configFile, ver );
}


void BaslerWindow::handleSavingConfig ( std::ofstream& configFile )
{
	picManager.handleSaveConfig ( configFile );
	settings.handleSavingConfig ( configFile );
}


void BaslerWindow::initializeControls()
{
	#ifdef FIREWIRE_CAMERA
		SetWindowText("Firewire Basler Camera Control");
	#elif defined USB_CAMERA
		SetWindowText("USB Basler Camera Control");
	#endif

	CMenu menu;
	//menu.LoadMenu( IDR_MENU1 );
	SetMenu( &menu );
	cameraController = new BaslerCameras( this );
	if (!cameraController->isInitialized())
	{
		thrower("ERROR: Camera not connected! Exiting program..." );
	}
	int id = 1000;
	POINT pos = { 0,0 };
	POINT cameraDims = cameraController->getCameraDimensions();
	settings.initialize( pos, id, this, cameraDims.x, cameraDims.y, cameraDims);
	settings.setSettings( cameraController->getDefaultSettings() );
	std::unordered_map<std::string, CFont*> fontDummy;
	std::vector<CToolTipCtrl*> toolTipDummy;
	//stats.initialize( pos, this, id, fontDummy, toolTipDummy );
	//saver.initialize( pos, id, this );

	POINT picPos = { 300, 0 };
	POINT dims = cameraController->getCameraDimensions();

	// scale to fill the window (approximately).
	dims.x *= 1.7;
	dims.y *= 1.7;

	//picture.initialize( picPos, this, id, dims.x + picPos.x + 115, dims.y + picPos.y, mainBrushes["Red"], 
	//					brightPlotPens, plotfont, brightPlotBrushes );
	//picture.recalculateGrid( cameraController->getDefaultSettings().dimensions );
	CDC* cdc = GetDC( );
	auto brushes = mainWin->getBrushes ( );
	picManager.initialize ( picPos, this, id, brushes[ "Red" ] );
	//picture.drawBackground( cdc );
	ReleaseDC( cdc );
}

