// created by Mark O. Brown

#include "stdafx.h"

#include "LowLevel/constants.h"
#include "GeneralObjects/SmartDC.h"

#include "BaslerWindow.h"
#include "AuxiliaryWindow.h"
#include "MainWindow.h"
#include "AndorWindow.h"
#include "ScriptingWindow.h"
#include "DeformableMirrorWindow.h"

#include "afxdialogex.h"


BaslerWindow::BaslerWindow( ) : picManager(true, "BASLER_PICTURE_MANAGER", true)
{}


// the message map. Allows me to handle various events in the system using functions I write myself.
BEGIN_MESSAGE_MAP( BaslerWindow, IChimeraWindow )
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_MOUSEMOVE()

	ON_COMMAND( ID_BASLER_SOFTWARE_TRIGGER, BaslerWindow::handleSoftwareTrigger )

	ON_CONTROL_RANGE(EN_CHANGE, IDC_MIN_BASLER_SLIDER_EDIT, IDC_MIN_BASLER_SLIDER_EDIT, &BaslerWindow::pictureRangeEditChange)
	ON_CONTROL_RANGE(EN_CHANGE, IDC_MAX_BASLER_SLIDER_EDIT, IDC_MAX_BASLER_SLIDER_EDIT, &BaslerWindow::pictureRangeEditChange)
	
	ON_MESSAGE( CustomMessages::BaslerProgressMessageID, &BaslerWindow::handleNewPics )
	ON_MESSAGE( CustomMessages::prepareBaslerWinAcq, &BaslerWindow::handlePrepareRequest )
	ON_CBN_SELENDOK( IDC_BASLER_EXPOSURE_MODE_COMBO, BaslerWindow::passExposureMode )
	ON_CBN_SELENDOK( IDC_BASLER_CAMERA_MODE_COMBO, BaslerWindow::passCameraMode)
	 
	ON_WM_RBUTTONUP()	
END_MESSAGE_MAP()


LRESULT BaslerWindow::handlePrepareRequest (WPARAM wParam, LPARAM lParam)
{
	ASSERT (InSendMessage ());
	baslerSettings* settings = (baslerSettings*)lParam;
	prepareWinForAcq (settings);
	return 0;
}


BaslerCameraCore& BaslerWindow::getCore ()
{
	return *basCamCore;
}

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


baslerSettings BaslerWindow::getCurrentSettings ( )
{
	return settingsCtrl.getCurrentSettings ( );
}


/*
Load the settings appropriate for the mot size measurement and then start the camera.
*/
void BaslerWindow::startTemporaryAcquisition ( baslerSettings settings )
{
	try
	{
		handleDisarmPress ( );
		currentRepNumber = 0;
		runningAutoAcq = true;
		tempAcqSettings = settings;
		picManager.setParameters ( settings.dims );
	}
	catch ( Error& )
	{
		throwNested ( "Failed to start temporary acquisition."  );
	}
}


void BaslerWindow::OnRButtonUp( UINT stuff, CPoint clickLocation )
{
	try
	{
		coordinate box = picManager.getClickLocation ( clickLocation );
		selectedPixel = box;
	}
	catch ( Error& err )
	{
		errBox ( err.what ( ) );
	}
}

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
		basCamCore->softwareTrigger();
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
			SmartDC sdc(this);
			picManager.handleScroll ( nSBCode, nPos, scrollbar, sdc.get ());
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
		basCamCore->disarm();
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
		if ( basCamCore->isRunning ( ) )
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
		tempSettings.frameRate = 10;
		#endif
		tempSettings.rawGain = 260;
		picManager.setParameters ( tempSettings.dims );
		SmartDC sdc (this);
		picManager.drawBackgrounds ( sdc.get ());
		runExposureMode = tempSettings.exposureMode;
		// only important in safemode
		//tempSettings.repCount = tempSettings.acquisitionMode == BaslerAcquisition::mode::Finite ? 
		//						tempSettings.repCount : SIZE_MAX;
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
		SmartDC sdc (this);
		auto runSttngs = basCamCore->getRunningSettings ();
		auto minMax = stats.update ( *imageMatrix, 0, { 0,0 }, currentRepNumber, runSttngs.totalPictures() );
		picManager.drawBitmap( sdc.get (), *imageMatrix, minMax, 0 );
 		picManager.updatePlotData ( );
 		picManager.drawDongles ( sdc.get(), { 0,0 }, std::vector<coordinate>(), std::vector<atomGrid>(), 0 );
		if (runExposureMode == BaslerAutoExposure::mode::Continuous)
		{ 
			settingsCtrl.updateExposure( basCamCore->getCurrentExposure() );
		}
 		settingsCtrl.setStatus("Camera Status: Acquiring Pictures.");
 		if (currentRepNumber % 10 == 0)
 		{
 			settingsCtrl.handleFrameRate();
 		}
 		if (!basCamCore->isContinuous())
 		{
			// don't write data if continuous, that's a recipe for disaster.
			andorWin->getLogger ( ).writeBaslerPic ( *imageMatrix );
 		}
 		if (currentRepNumber == runSttngs.totalPictures())
 		{
			// handle balser finish
 			basCamCore->disarm();
 			isRunning = false;
			runningAutoAcq = false;
 			settingsCtrl.setStatus("Camera Status: Finished finite acquisition.");
			// tell the andor window that the basler camera finished so that the data file can be handled appropriately.
			mainWin->getComm ( )->sendBaslerFin ( );
			mainWin->getComm ( )->sendColorBox ( System::Basler, 'B' );
			if (!andorWin->cameraIsRunning() )
			{
				// else it will close when the basler camera finishes.
				andorWin->getLogger ( ).closeFile ( );
			}
 		}
		if ( stats.getMostRecentStats ( ).avgv < settingsCtrl.getMotThreshold ( ) )
		{
			motLoaded = false;
			loadMotConsecutiveFailures++;
			if (andorWin->wantsNoMotAlert ( ) )
			{
				if ( loadMotConsecutiveFailures > andorWin->getNoMotThreshold ( ) )
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


void BaslerWindow::prepareWinForAcq(baslerSettings* settings)
{
	try
	{
		mainWin->getComm ( )->sendColorBox ( System::Basler, 'Y' );
		currentRepNumber = 0;
		picManager.setParameters( settings->dims );

		SmartDC sdc (this);
		picManager.drawBackgrounds ( sdc.get ());
		runExposureMode = settings->exposureMode;
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
	return basCamCore->isRunning ( );
}


bool BaslerWindow::baslerCameraIsContinuous ( )
{
	return basCamCore->isContinuous ( );
}

void BaslerWindow::OnSize( UINT nType, int cx, int cy )
{
	auto fonts = mainWin->getFonts ( );
	auto r = picManager.getPicArea ( );
	InvalidateRect ( &r );
	statBox.rearrange (cx, cy, fonts);
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
		int num = pWnd->GetDlgCtrlID ();
		if (num == IDC_MOT_LOADED_INDICATOR)
		{
			if (!motLoaded)
			{
				pDC->SetBkColor (_myRGBs["Red"]);
				return *_myBrushes["Red"];
			}
			else
			{
				pDC->SetBkColor (_myRGBs["Green"]);
				return *_myBrushes["Green"];
			}
		}
	}
	}
	return IChimeraWindow::OnCtlColor (pDC, pWnd, nCtlColor);
}


BOOL BaslerWindow::OnInitDialog()
{
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
	return IChimeraWindow::OnInitDialog ();
}


// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void BaslerWindow::OnPaint ()
{
	if (IsIconic ())
	{
		CPaintDC dc (this); // device context for painting

		SendMessage (WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc ()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics (SM_CXICON);
		int cyIcon = GetSystemMetrics (SM_CYICON);
		CRect rect;
		GetClientRect (&rect);
		int x = (rect.Width () - cxIcon + 1) / 2;
		int y = (rect.Height () - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon (x, y, m_hIcon);
	}
	else
	{
		IChimeraWindow::OnPaint ();
		CRect size;
		GetClientRect (&size);
		SmartDC sdc (this);
		picManager.paint (sdc.get (), size, _myBrushes["Interactable-Bkgd"]);
	}
}


void BaslerWindow::windowOpenConfig ( ConfigStream& configFile, Version ver )
{
	ProfileSystem::standardOpenConfig ( configFile, picManager.configDelim, &picManager, Version ( "4.0" ) );
	baslerSettings settings;
	ProfileSystem::stdGetFromConfig (configFile, "BASLER_CAMERA_SETTINGS", *basCamCore, settings, Version ("4.0"));
	settingsCtrl.setSettings (settings);
}


void BaslerWindow::windowSaveConfig ( ConfigStream& configFile )
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
	basCamCore = new BaslerCameraCore( this );
	if (!basCamCore->isInitialized())
	{
		thrower ("ERROR: Camera not connected! Exiting program..." );
	}
	int id = 1000;
	POINT pos = { 0,0 };
	POINT dims = basCamCore->getCameraDimensions ();
	statBox.initialize (pos, id, this, 300, toolTips );
	settingsCtrl.initialize( pos, id, this, dims.x, dims.y, dims);
	settingsCtrl.setSettings( basCamCore->getDefaultSettings() );
	std::vector<CToolTipCtrl*> toolTipDummy;
	stats.initialize( pos, this, id, toolTipDummy );

	pos = { 365, 0 };
	// scale to fill the window (approximately).
	dims.x *= 1.65;
	dims.y *= 1.65;
	picManager.initialize (pos, this, id, _myBrushes[ "Red" ], dims.x + pos.x + 115, dims.y + pos.y,
						   { IDC_MIN_BASLER_SLIDER_EDIT, IDC_MAX_BASLER_SLIDER_EDIT, NULL,NULL,NULL,NULL,NULL,NULL},
							mainWin->getBrightPlotPens(), mainWin->getPlotFont(), mainWin->getPlotBrushes() );
	picManager.setSinglePicture ( this, settingsCtrl.getCurrentSettings().dims );
	picManager.setPalletes ( { 1,1,1,1 } );
	this->RedrawWindow ( );
	SmartDC sdc (this);
	picManager.drawBackgrounds( sdc.get ());
}

