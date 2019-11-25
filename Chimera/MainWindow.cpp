#include "stdafx.h"
#include "commonFunctions.h"
#include "MainWindow.h"
#include "AndorWindow.h"
#include "AuxiliaryWindow.h"
#include "ScriptingWindow.h"
#include "BaslerWindow.h"
#include <future>
#include "externals.h"


MainWindow::MainWindow( UINT id, CDialog* splash, chronoTime* startTime) : CDialog( id ), profile( PROFILES_PATH ),
	masterConfig( MASTER_CONFIGURATION_FILE_ADDRESS ),
	appSplash( splash ),
	niawg( DioRows::which::B, 14, NIAWG_SAFEMODE ),
	masterRepumpScope( MASTER_REPUMP_SCOPE_ADDRESS, MASTER_REPUMP_SCOPE_SAFEMODE, 4, "D2 F=1 & Master Lasers Scope" ),
	motScope( MOT_SCOPE_ADDRESS, MOT_SCOPE_SAFEMODE, 2, "D2 F=2 Laser Scope" )
{
	programStartTime = startTime;
	startupTimes.push_back(chronoClock::now());

	/// the following are all equivalent to:
	// mainFonts["Font name"] = new CFont;
	// mainFonts["Font name"].CreateFontA(...);
	(mainFonts["Smaller Font Max"] = new CFont)
		->CreateFontA(27, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Normal Font Max"] = new CFont)
		->CreateFontA(33, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Heading Font Max"] = new CFont)
		->CreateFontA(42, 0, 0, 0, FW_DONTCARE, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Old Sans Black"));
	(mainFonts["Code Font Max"] = new CFont)
		->CreateFontA(32, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));
	(mainFonts["Small Code Font Max"] = new CFont)
		->CreateFontA (25, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT ("Consolas"));
	(mainFonts["Larger Font Max"] = new CFont)
		->CreateFontA(40, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Very Larger Font Max"] = new CFont)
		->CreateFontA( 80, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					   CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT( "Arial" ) );
	//
	(mainFonts["Smaller Font Large"] = new CFont)
		->CreateFontA(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Normal Font Large"] = new CFont)
		->CreateFontA(20, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Heading Font Large"] = new CFont)
		->CreateFontA(24, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Old Sans Black"));
	(mainFonts["Code Font Large"] = new CFont)
		->CreateFontA(16, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));
	(mainFonts["Small Code Font Large"] = new CFont)
		->CreateFontA (12, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT ("Consolas"));
	(mainFonts["Larger Font Large"] = new CFont)
		->CreateFontA(40, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Very Larger Font Large"] = new CFont)
		->CreateFontA( 60, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					   CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT( "Arial" ) );

	//
	(mainFonts["Smaller Font Med"] = new CFont)
		->CreateFontA(8, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Normal Font Med"] = new CFont)
		->CreateFontA(12, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Heading Font Med"] = new CFont)
		->CreateFontA(16, 0, 0, 0, FW_DONTCARE, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Old Sans Black"));
	(mainFonts["Code Font Med"] = new CFont)
		->CreateFontA(10, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));
	(mainFonts["Small Code Font Med"] = new CFont)
		->CreateFontA (9, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT ("Consolas"));
	(mainFonts["Larger Font Med"] = new CFont)
		->CreateFontA(22, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Very Larger Font Med"] = new CFont)
		->CreateFontA( 40, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					   CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT( "Arial" ) );
	//
	(mainFonts["Smaller Font Small"] = new CFont)
		->CreateFontA(6, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Normal Font Small"] = new CFont)
		->CreateFontA(8, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Heading Font Small"] = new CFont)
		->CreateFontA(12, 0, 0, 0, FW_DONTCARE, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Old Sans Black"));
	(mainFonts["Code Font Small"] = new CFont)
		->CreateFontA(7, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));
	(mainFonts["Small Code Font Small"] = new CFont)
		->CreateFontA (5, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT ("Consolas"));
	(mainFonts["Larger Font Small"] = new CFont)
		->CreateFontA(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Very Larger Font Small"] = new CFont)
		->CreateFontA( 32, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					   CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT( "Arial" ) );
	//
	(plotfont = new CFont)
		->CreateFontA( 12/*20*/, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					   CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT( "Arial" ) );
}

IMPLEMENT_DYNAMIC( MainWindow, CDialog )

BEGIN_MESSAGE_MAP( MainWindow, CDialog )
	ON_WM_CTLCOLOR( )
	ON_WM_SIZE( )
	ON_CBN_SELENDOK( IDC_SEQUENCE_COMBO, &MainWindow::handleSequenceCombo )
	ON_NOTIFY( NM_DBLCLK, IDC_SMS_TEXTING_LISTVIEW, &MainWindow::handleDblClick )
	ON_NOTIFY( NM_RCLICK, IDC_SMS_TEXTING_LISTVIEW, &MainWindow::handleRClick )
	ON_NOTIFY (NM_DBLCLK, IDC_SERVO_LISTVIEW, &MainWindow::ServoDblClick)
	ON_NOTIFY (NM_RCLICK, IDC_SERVO_LISTVIEW, &MainWindow::ServoRClick)
	ON_NOTIFY (NM_CUSTOMDRAW, IDC_SERVO_LISTVIEW, &MainWindow::drawServoListview)

	ON_EN_CHANGE( IDC_CONFIGURATION_NOTES, &MainWindow::notifyConfigUpdate )
	ON_EN_CHANGE( IDC_REPETITION_EDIT, &MainWindow::notifyConfigUpdate )
	ON_MESSAGE ( RepProgressMessageID, &MainWindow::onRepProgress )
	ON_MESSAGE ( StatusUpdateMessageID, &MainWindow::onStatusTextMessage )
	ON_MESSAGE ( ErrorUpdateMessageID, &MainWindow::onErrorMessage )
	ON_MESSAGE ( FatalErrorMessageID, &MainWindow::onFatalErrorMessage )
	ON_MESSAGE ( DebugUpdateMessageID, &MainWindow::onDebugMessage )
	ON_MESSAGE ( NoAtomsAlertMessageID, &MainWindow::onNoAtomsAlertMessage )
	ON_MESSAGE ( NoMotAlertMessageID, &MainWindow::onNoMotAlertMessage )
	ON_MESSAGE ( GeneralFinMsgID, &MainWindow::onFinish )
	ON_COMMAND_RANGE( ID_ACCELERATOR_ESC, ID_ACCELERATOR_ESC, &MainWindow::passCommonCommand )
	ON_COMMAND_RANGE( ID_ACCELERATOR_F5, ID_ACCELERATOR_F5, &MainWindow::passCommonCommand )
	ON_COMMAND_RANGE( ID_ACCELERATOR_F2, ID_ACCELERATOR_F2, &MainWindow::passCommonCommand )
	ON_COMMAND_RANGE( ID_ACCELERATOR_F1, ID_ACCELERATOR_F1, &MainWindow::passCommonCommand )
	ON_COMMAND_RANGE( MENU_ID_RANGE_BEGIN, MENU_ID_RANGE_END, &MainWindow::passCommonCommand )
	ON_COMMAND_RANGE( IDC_DEBUG_OPTIONS_RANGE_BEGIN, IDC_DEBUG_OPTIONS_RANGE_END, &MainWindow::passDebugPress )
	ON_COMMAND_RANGE( IDC_MAIN_OPTIONS_RANGE_BEGIN, IDC_MAIN_OPTIONS_RANGE_END, &MainWindow::passMainOptionsPress )
	ON_COMMAND_RANGE( IDC_MAIN_STATUS_BUTTON, IDC_MAIN_STATUS_BUTTON, &MainWindow::passClear )
	ON_COMMAND_RANGE( IDC_ERROR_STATUS_BUTTON, IDC_ERROR_STATUS_BUTTON, &MainWindow::passClear )
	ON_COMMAND_RANGE( IDC_DEBUG_STATUS_BUTTON, IDC_DEBUG_STATUS_BUTTON, &MainWindow::passClear )
	ON_COMMAND( IDC_SELECT_CONFIG_COMBO, &MainWindow::passConfigPress )
	ON_COMMAND( IDOK,  &MainWindow::catchEnter)
	ON_COMMAND (IDC_SERVO_CAL, &runServos)
	ON_MESSAGE (MainWindow::AutoServoMessage, &autoServo)
	ON_COMMAND( IDC_RERNG_EXPERIMENT_BUTTON, &MainWindow::passExperimentRerngButton )
	ON_CBN_SELENDOK ( IDC_RERNG_MODE_COMBO, &MainWindow::passRerngModeComboChange )
	ON_WM_RBUTTONUP( )
	ON_WM_LBUTTONUP( )
	ON_WM_PAINT( )
	ON_WM_TIMER( )
END_MESSAGE_MAP()


void MainWindow::drawServoListview (NMHDR* pNMHDR, LRESULT* pResult)
{
	try
	{
		servos.handleDraw (pNMHDR, pResult);
	}
	catch (Error & err)
	{
		comm.sendError (err.trace ());
	}
}


void MainWindow::passRerngModeComboChange ( )
{
	rearrangeControl.updateActive ( );
}


void MainWindow::handleThresholdAnalysis ( )
{
	auto grid = TheAndorWindow->getMainAtomGrid ( );
	auto dateStr = TheAndorWindow->getMostRecentDateString ( );
	auto fid = TheAndorWindow->getMostRecentFid ( );
	auto ppr = TheAndorWindow->getPicsPerRep ( );
	std::string gridString = "[" + str ( grid.topLeftCorner.row-1 ) + "," + str(grid.topLeftCorner.column-1 ) + ","
		+ str(grid.pixelSpacing) + "," + str(grid.width) + "," + str(grid.height) + "]";
	python.thresholdAnalysis (dateStr, fid, gridString, ppr);
}


LRESULT MainWindow::onFinish ( WPARAM wp, LPARAM lp )
{
	ExperimentType type = static_cast<ExperimentType>( wp );
	switch ( type )
	{
		case ExperimentType::Normal:
		{
			onNormalFinishMessage ( );
			break;
		}
		case ExperimentType::LoadMot:
			break;
		case ExperimentType::MotSize:
			onMotNumCalFin ( );
			break;
		case ExperimentType::MotTemperature:
			onMotTempCalFin ( );
			break;
		case ExperimentType::PgcTemperature:
			onPgcTempCalFin ( );
			break;
		case ExperimentType::GreyTemperature:
			onGreyTempCalFin ( );
			break;
		case ExperimentType::MachineOptimization:
			onMachineOptRoundFin ( );
			break;
	}
	return 0;
}


void MainWindow::onMotNumCalFin ( )
{
	commonFunctions::handleCommonMessage ( ID_MOT_TEMP_CAL, this, this, TheScriptingWindow, TheAndorWindow,
										   TheAuxiliaryWindow, TheBaslerWindow );
}


void MainWindow::onMotTempCalFin ( )
{
	commonFunctions::handleCommonMessage ( ID_PGC_TEMP_CAL, this, this, TheScriptingWindow, TheAndorWindow,
										   TheAuxiliaryWindow, TheBaslerWindow );
}


void MainWindow::onPgcTempCalFin ( )
{
	commonFunctions::handleCommonMessage ( ID_GREY_TEMP_CAL, this, this, TheScriptingWindow, TheAndorWindow,
										   TheAuxiliaryWindow, TheBaslerWindow );
}


void MainWindow::onGreyTempCalFin ( )
{
	infoBox ( "Finished MOT Calibrations." );
}

void MainWindow::onMachineOptRoundFin (  )
{
	// do normal finish
	onNormalFinishMessage ( );
	Sleep ( 1000 );
	// then restart.
	commonFunctions::handleCommonMessage ( ID_MACHINE_OPTIMIZATION, this, this, TheScriptingWindow, TheAndorWindow, 
										   TheAuxiliaryWindow, TheBaslerWindow );
}



void MainWindow::passExperimentRerngButton( )
{
	rearrangeControl.updateActive ( );
}

void MainWindow::OnTimer( UINT_PTR id )
{
	OnPaint( );
}


void MainWindow::loadCameraCalSettings( ExperimentThreadInput* input )
{
	input->seq.name = "CameraCal";
	input->seq.sequence.resize( 1 );
	input->seq.sequence[0].configuration = "Camera-Calibration";
	input->seq.sequence[0].configLocation = CAMERA_CAL_ROUTINE_ADDRESS;
	input->seq.sequence[0].parentFolderName = "Camera";
	// the calibration procedure doesn't need the NIAWG at all.
	input->runNiawg = false;
	input->skipNext = NULL;
	input->rerngGuiForm = rearrangeControl.getParams( );
	input->rerngGuiForm.active = false;
	input->expType = ExperimentType::CameraCal;
}


BOOL MainWindow::handleAccelerators( HACCEL m_haccel, LPMSG lpMsg )
{
	if ( TheAuxiliaryWindow != NULL )
	{
		return TheAuxiliaryWindow->handleAccelerators( m_haccel, lpMsg );
	}
	else
	{
		return FALSE;
	}
}


void MainWindow::OnPaint( )
{
	CDialog::OnPaint( );
	if ( !masterThreadManager.runningStatus() )
	{
		CRect size;
		GetClientRect( &size );
		CDC* cdc = GetDC( );
		cdc->SetBkColor( _myRGBs["Main-Bkgd"] );
		UINT width = size.right - size.left, height = size.bottom - size.top;
		masterRepumpScope.refreshPlot( cdc, width, height, _myBrushes["Main-Bkgd"], _myBrushes["Interactable-Bkgd"] );
		motScope.refreshPlot( cdc, width, height, _myBrushes["Main-Bkgd"], _myBrushes["Interactable-Bkgd"] );
		ReleaseDC( cdc );
	}
}


std::vector<Gdiplus::Pen*> MainWindow::getPlotPens( )
{
	return plotPens;
}


std::vector<Gdiplus::SolidBrush*> MainWindow::getPlotBrushes( )
{
	return plotBrushes;
}


std::vector<Gdiplus::Pen*> MainWindow::getBrightPlotPens( )
{
	return brightPlotPens;
}


std::vector<Gdiplus::SolidBrush*> MainWindow::getBrightPlotBrushes( )
{
	return brightPlotBrushes;
}


void MainWindow::OnRButtonUp( UINT stuff, CPoint clickLocation )
{
	TheAndorWindow->stopSound( );
}


void MainWindow::OnLButtonUp( UINT stuff, CPoint clickLocation )
{
	TheAndorWindow->stopSound( );
}


void MainWindow::passConfigPress( )
{	
	try
	{
		profile.handleSelectConfigButton( this, TheScriptingWindow, this, TheAuxiliaryWindow, TheAndorWindow, 
										  TheBaslerWindow );
	}
	catch ( Error& err )
	{
		comm.sendError( err.trace( ) );
	}
}


void MainWindow::passNiawgIsOnPress( )
{
	if ( niawg.isOn() )
	{
		niawg.turnOff( );
		checkAllMenus ( ID_NIAWG_NIAWGISON, MF_UNCHECKED );
	}
	else
	{
		niawg.turnOn( );
		checkAllMenus ( ID_NIAWG_NIAWGISON, MF_CHECKED );
	}
}


LRESULT MainWindow::onNoMotAlertMessage( WPARAM wp, LPARAM lp )
{
	try
	{
		if ( TheAndorWindow->wantsAutoPause ( ) )
		{
			masterThreadManager.pause ( );
			checkAllMenus ( ID_RUNMENU_PAUSE, MF_CHECKED );
			comm.sendColorBox ( System::Master, 'Y' );
		}
		auto async
			= std::async ( std::launch::async, [] { Beep ( 1000, 100 ); } );
		time_t t = time ( 0 );
		struct tm now;
		localtime_s ( &now, &t );
		std::string message = "Experiment Stopped loading the MOT at ";
		if ( now.tm_hour < 10 )
		{
			message += "0";
		}
		message += str ( now.tm_hour ) + ":";
		if ( now.tm_min < 10 )
		{
			message += "0";
		}
		message += str ( now.tm_min ) + ":";
		if ( now.tm_sec < 10 )
		{
			message += "0";
		}
		message += str ( now.tm_sec );
		texter.sendMessage ( message, &python, "Mot" );
	}
	catch ( Error& err )
	{
		comm.sendError ( err.what ( ) );
	}
	return 0;
}


LRESULT MainWindow::onNoAtomsAlertMessage( WPARAM wp, LPARAM lp )
{
	try
	{	
		if ( TheAndorWindow->wantsAutoPause( ) )
		{
			masterThreadManager.pause( );
			checkAllMenus ( ID_RUNMENU_PAUSE, MF_CHECKED );
			comm.sendColorBox( System::Master, 'Y' );
		}
		auto asyncbeep = std::async( std::launch::async, [] { Beep( 1000, 100 ); } );
		time_t t = time( 0 );
		struct tm now;
		localtime_s( &now, &t );
		std::string message = "Experiment Stopped loading atoms at ";
		if ( now.tm_hour < 10 )
		{
			message += "0";
		}
		message += str( now.tm_hour ) + ":";
		if ( now.tm_min < 10 )
		{
			message += "0";
		}
		message += str( now.tm_min ) + ":";
		if ( now.tm_sec < 10 )
		{
			message += "0";
		}
		message += str( now.tm_sec );
		texter.sendMessage( message, &python, "Loading" );
	}
	catch ( Error& err )
	{
		comm.sendError( err.trace( ) );
	}
	return 0;
}


CFont* MainWindow::getPlotFont( )
{
	return plotfont;
}


BOOL MainWindow::OnInitDialog( )
{
	SetWindowText ( "Main Window" );

	startupTimes.push_back(chronoClock::now());
	eMainWindowHwnd = this;
	for ( auto elem : GIST_RAINBOW_RGB )
	{
		Gdiplus::Color c( 50, BYTE( elem[0] ), BYTE( elem[1] ), BYTE( elem[2] ) );
		Gdiplus::SolidBrush* b = new Gdiplus::SolidBrush( c );
		Gdiplus::Pen* p = new Gdiplus::Pen( b );
		Gdiplus::Color c_bright( 255, BYTE( elem[0] ), BYTE( elem[1] ), BYTE( elem[2] ) );
		Gdiplus::SolidBrush* b_bright = new Gdiplus::SolidBrush( c_bright );
		Gdiplus::Pen* p_bright = new Gdiplus::Pen( b_bright );
		plotBrushes.push_back( b );
		plotPens.push_back( p );
		brightPlotBrushes.push_back( b_bright );
		brightPlotPens.push_back( p_bright );
	}
	// don't redraw until the first OnSize.
	SetRedraw( false );
	
	/// initialize niawg.
	try
	{
		niawg.initialize( );
	}
	catch ( Error& except )
	{
		errBox( "NIAWG failed to Initialize! Error: " + except.trace( ) );
	}
	try
	{
		niawg.setDefaultWaveforms( this );
		// but the default starts in the horizontal configuration, so switch back and start in this config.
		restartNiawgDefaults( );
	}
	catch ( Error& exception )
	{
		errBox( "Failed to start niawg default waveforms! Niawg gave the following error message: " 
				+ exception.trace( ) );
	}
	// not done with the script, it will not stay on the NIAWG, so I need to keep track of it so thatI can reload it onto the NIAWG when necessary.	
	/// Initialize Windows
	std::string which = "";
	try
	{
		which = "Scripting";
		TheScriptingWindow = new ScriptingWindow;
		which = "Camera";
		TheAndorWindow = new AndorWindow;
		which = "Auxiliary";
		TheAuxiliaryWindow = new AuxiliaryWindow;
		which = "Basler";
		TheBaslerWindow = new BaslerWindow;
	}
	catch ( Error& err )
	{
		errBox( "FATAL ERROR: " + which + " Window constructor failed! Error: " + err.trace( ) );
		forceExit ( );
		return -1;
	}
	TheScriptingWindow->loadFriends( this, TheAndorWindow, TheAuxiliaryWindow, TheBaslerWindow );
	TheAndorWindow->loadFriends( this, TheScriptingWindow, TheAuxiliaryWindow, TheBaslerWindow );
	TheAuxiliaryWindow->loadFriends( this, TheScriptingWindow, TheAndorWindow, TheBaslerWindow );
	TheBaslerWindow->loadFriends ( this, TheScriptingWindow, TheAndorWindow, TheAuxiliaryWindow );
	startupTimes.push_back(chronoClock::now());
	try
	{
		// these each call oninitdialog after the create call. Hence the try / catch.
		TheScriptingWindow->Create( IDD_LARGE_TEMPLATE, GetDesktopWindow() );
		TheAndorWindow->Create( IDD_LARGE_TEMPLATE, GetDesktopWindow( ) );
		TheAuxiliaryWindow->Create( IDD_LARGE_TEMPLATE, GetDesktopWindow( ) );
		TheBaslerWindow->Create ( IDD_LARGE_TEMPLATE, GetDesktopWindow ( ) );
	}
	catch ( Error& err )
	{
		errBox( "FATAL ERROR: Failed to create window! " + err.trace( ) );
		forceExit ( );
		return -1;
	}
	/// initialize main window controls.
	comm.initialize( this, TheScriptingWindow, TheAndorWindow, TheAuxiliaryWindow );
	int id = 1000;
	POINT controlLocation = { 0,0 };
	mainStatus.initialize( controlLocation, this, id, 870, "EXPERIMENT STATUS", RGB( 100, 100, 250 ), tooltips, IDC_MAIN_STATUS_BUTTON );
	boxes.initialize ( controlLocation, id, this, 960, tooltips);
	shortStatus.initialize (controlLocation, this, id, tooltips);
	controlLocation = { 480, 0 };
	errorStatus.initialize( controlLocation, this, id, 420, "ERROR STATUS", RGB( 100, 0, 0 ), tooltips, 
							IDC_ERROR_STATUS_BUTTON );
	debugStatus.initialize( controlLocation, this, id, 420, "DEBUG STATUS", RGB( 13, 152, 186 ), tooltips, 
							IDC_DEBUG_STATUS_BUTTON );
	controlLocation = { 960, 0 };
	profile.initialize( controlLocation, this, id, tooltips );
	controlLocation = { 960, 175 };
	notes.initialize( controlLocation, this, id, tooltips);
	masterRepumpScope.initialize( controlLocation, 480, 130, this, getPlotPens( ), getPlotFont( ), getPlotBrushes(), 
								  "Master/Repump" );
	motScope.initialize( controlLocation, 480, 130, this, getPlotPens( ), getPlotFont( ), getPlotBrushes( ), "MOT" );
	servos.initialize (controlLocation, tooltips, this, id, &TheAuxiliaryWindow->getAiSys (),
		&TheAuxiliaryWindow->getAoSys (), &TheAuxiliaryWindow->getTtlBoard (), &TheAuxiliaryWindow->getGlobals ());
	controlLocation = { 1440, 50 };
	repetitionControl.initialize( controlLocation, tooltips, this, id );
	mainOptsCtrl.initialize( id, controlLocation, this, tooltips );
	rearrangeControl.initialize( id, controlLocation, this, tooltips );
	debugger.initialize( id, controlLocation, this, tooltips );
	texter.initialize( controlLocation, this, id, tooltips );

	menu.LoadMenu( IDR_MAIN_MENU );
	SetMenu( &menu );

	// just initializes the rectangles.
	TheAndorWindow->redrawPictures( true );
	try
	{
		masterConfig.load( this, TheAuxiliaryWindow, TheAndorWindow );
	}
	catch ( Error& err )
	{
		errBox( err.trace( ) );
	}
	startupTimes.push_back(chronoClock::now());
	ShowWindow( SW_MAXIMIZE );
	TheAndorWindow->ShowWindow( SW_MAXIMIZE );
	TheScriptingWindow->ShowWindow( SW_MAXIMIZE );
	TheAuxiliaryWindow->ShowWindow( SW_MAXIMIZE );
	TheBaslerWindow->ShowWindow ( SW_MAXIMIZE );
	std::vector<CDialog*> windows = { TheBaslerWindow, NULL, TheAndorWindow, this, TheScriptingWindow, TheAuxiliaryWindow };
	EnumDisplayMonitors( NULL, NULL, monitorHandlingProc, reinterpret_cast<LPARAM>(&windows) );
	// hide the splash just before the first window requiring input pops up.
	appSplash->ShowWindow( SW_HIDE );
	startupTimes.push_back(chronoClock::now());
	std::string timingMsg;
	auto t1 = std::chrono::duration_cast<std::chrono::milliseconds>(*programStartTime - startupTimes[0]).count();
	auto t2 = std::chrono::duration_cast<std::chrono::milliseconds>(startupTimes[1] - *programStartTime).count();
	auto t3 = std::chrono::duration_cast<std::chrono::milliseconds>(startupTimes[2] - startupTimes[1]).count();
	auto t4 = std::chrono::duration_cast<std::chrono::milliseconds>(startupTimes[3] - startupTimes[2]).count();
	auto t5 = std::chrono::duration_cast<std::chrono::milliseconds>(startupTimes[4] - startupTimes[3]).count();
	timingMsg = "Constructor to init-instance: " + str(t1) + "\nInitInstance to OnInitDialog:" + str(t2) 
			  + "\nOnInitDialog To Creating Windows:" + str(t3) + "\nCreating Windows to showing windows:" + str(t4) 
			  + "\nShowing Windows to here:" + str(t5);
	// errBox(timingMsg); // can be used to debug some of the startup times if these end up being long
	/// summarize system status.
	try
	{
		// ordering of aux window pieces is a bit funny because I want the devices grouped by type, not by window.
		std::string initializationString;
		initializationString += getSystemStatusString( );
		initializationString += TheAuxiliaryWindow->getOtherSystemStatusMsg( );
		initializationString += TheAndorWindow->getSystemStatusString( );
		initializationString += TheAuxiliaryWindow->getVisaDeviceStatus( );
		initializationString += TheScriptingWindow->getSystemStatusString( );
		initializationString += TheAuxiliaryWindow->getGpibDeviceStatus( );
		infoBox( initializationString );
	}
	catch ( Error& err )
	{
		errBox( err.trace( ) );
	}
	SetTimer( 1, 10000, NULL );
	// set up the threads that update the scope data.
	_beginthreadex( NULL, NULL, &MainWindow::scopeRefreshProcedure, &masterRepumpScope, NULL, NULL );
	_beginthreadex( NULL, NULL, &MainWindow::scopeRefreshProcedure, &motScope, NULL, NULL );
	//

	updateConfigurationSavedStatus( true );
	return TRUE;
}

void MainWindow::showHardwareStatus ( )
{
	try
	{
		// ordering of aux window pieces is a bit funny because I want the devices grouped by type, not by window.
		std::string initializationString;
		initializationString += getSystemStatusString ( );
		initializationString += TheAuxiliaryWindow->getOtherSystemStatusMsg ( );
		initializationString += TheAndorWindow->getSystemStatusString ( );
		initializationString += TheAuxiliaryWindow->getVisaDeviceStatus ( );
		initializationString += TheScriptingWindow->getSystemStatusString ( );
		initializationString += TheAuxiliaryWindow->getGpibDeviceStatus ( );
		infoBox ( initializationString );
	}
	catch ( Error& err )
	{
		errBox ( err.trace ( ) );
	}
}


// just notifies the profile object that the configuration is no longer saved.
void MainWindow::notifyConfigUpdate( )
{
	profile.updateConfigurationSavedStatus( false );
}


void MainWindow::catchEnter( )
{
	// the default handling is to close the window, so I need to catch it.

}


BOOL CALLBACK MainWindow::monitorHandlingProc( _In_ HMONITOR hMonitor, _In_ HDC hdcMonitor,
											   _In_ LPRECT lprcMonitor, _In_ LPARAM dwData )
{
	static UINT count = 0;
	std::vector<CDialog*>* windows = reinterpret_cast<std::vector<CDialog*>*>(dwData);
	if ( count == 1 )
	{
		// skip the tall monitor.
		count++;
		return TRUE;
	}
	if ( count < 6 )
	{
		if (windows->at (count) != NULL) 
		{
			windows->at (count)->MoveWindow (lprcMonitor);
		}
		else
		{
			errBox ("Error in monitorHandlingProc! Tried to move \"NULL\" Window to monitor.");
		}
	}
	count++;
	return TRUE;
}


void MainWindow::checkAllMenus ( UINT menuItem, UINT itemState )
{
	setMenuCheck ( menuItem, itemState );
	TheAuxiliaryWindow->setMenuCheck ( menuItem, itemState );
	TheAndorWindow->setMenuCheck ( menuItem, itemState );
	TheBaslerWindow->setMenuCheck ( menuItem, itemState );
	TheScriptingWindow->setMenuCheck ( menuItem, itemState );
}


void MainWindow::setMenuCheck ( UINT menuItem, UINT itemState )
{
	menu.CheckMenuItem ( menuItem, itemState );
}


void MainWindow::handlePause()
{
	if (masterThreadManager.runningStatus())
	{
		if (masterThreadManager.getIsPaused())
		{
			// then it's currently paused, so unpause it.
			checkAllMenus ( ID_RUNMENU_PAUSE, MF_UNCHECKED );
			masterThreadManager.unPause();
			comm.sendColorBox( System::Master, 'G' );
		}
		else
		{
			// then not paused so pause it.
			checkAllMenus ( ID_RUNMENU_PAUSE, MF_CHECKED );
			comm.sendColorBox( System::Master, 'Y' );
			masterThreadManager.pause();
		}
	}
	else
	{
		comm.sendStatus("Can't pause, experiment was not running.\r\n");
	}
}


LRESULT MainWindow::onRepProgress(WPARAM wParam, LPARAM lParam)
{
	repetitionControl.updateNumber(lParam);
	return NULL;
}


void MainWindow::handleNewConfig( std::ofstream& newFile )
{
	notes.handleNewConfig( newFile );
	mainOptsCtrl.handleNewConfig( newFile );
	debugger.handleNewConfig( newFile );
	repetitionControl.handleNewConfig( newFile );
	rearrangeControl.handleNewConfig( newFile );
}


void MainWindow::handleSaveConfig(std::ofstream& saveFile)
{
	notes.handleSaveConfig(saveFile);
	mainOptsCtrl.handleSaveConfig(saveFile);
	debugger.handleSaveConfig(saveFile);
	repetitionControl.handleSaveConfig(saveFile);
	rearrangeControl.handleSaveConfig( saveFile );
}


void MainWindow::handleOpeningConfig(std::ifstream& configFile, Version ver )
{
	try
	{
		ProfileSystem::standardOpenConfig ( configFile, "CONFIGURATION_NOTES", &notes);
		mainOptsCtrl.setOptions ( ProfileSystem::stdGetFromConfig ( configFile, "MAIN_OPTIONS", 
																		MainOptionsControl::getMainOptionsFromConfig ) );
		ProfileSystem::standardOpenConfig ( configFile, "DEBUGGING_OPTIONS", &debugger );
		repetitionControl.setRepetitions ( ProfileSystem::stdGetFromConfig ( configFile, "REPETITIONS", 
																				  Repetitions::getRepsFromConfig ));
		ProfileSystem::standardOpenConfig ( configFile, "REARRANGEMENT_INFORMATION", &rearrangeControl );
	}
	catch ( Error& )
	{
		throwNested ( "Main Window failed to read parameters from the configuration file." );
	}
}


void MainWindow::OnSize(UINT nType, int cx, int cy)
{
	SetRedraw( false );
	masterRepumpScope.rearrange( cx, cy, getFonts( ) );
	motScope.rearrange( cx, cy, getFonts( ) );
	profile.rearrange(cx, cy, getFonts());
	notes.rearrange(cx, cy, getFonts());
	debugger.rearrange(cx, cy, getFonts());
	mainOptsCtrl.rearrange(cx, cy, getFonts());
	mainStatus.rearrange(cx, cy, getFonts());
	debugStatus.rearrange(cx, cy, getFonts());
	errorStatus.rearrange(cx, cy, getFonts());
	texter.rearrange(cx, cy, getFonts());
	shortStatus.rearrange(cx, cy, getFonts());
	boxes.rearrange( cx, cy, getFonts());
	repetitionControl.rearrange(cx, cy, getFonts());
	rearrangeControl.rearrange( cx, cy, getFonts( ) );
	servos.rearrange (cx, cy, getFonts ());
	SetRedraw();
	RedrawWindow();
}


void MainWindow::setNiawgRunningState( bool newRunningState )
{
	niawg.setRunningState( newRunningState );
}


bool MainWindow::niawgIsRunning()
{
	return niawg.niawgIsRunning();
}



BOOL MainWindow::PreTranslateMessage(MSG* pMsg)
{
	for (UINT toolTipInc = 0; toolTipInc < tooltips.size(); toolTipInc++)
	{
		tooltips[toolTipInc]->RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}


void MainWindow::setNiawgDefaults()
{
	niawg.setDefaultWaveforms(this);
}


fontMap MainWindow::getFonts()
{
	return mainFonts;
}


void MainWindow::passClear(UINT id)
{
	if (id == IDC_MAIN_STATUS_BUTTON)
	{
		mainStatus.clear();
	}
	else if (id == IDC_ERROR_STATUS_BUTTON)
	{
		errorStatus.clear();
	}
	else if (id == IDC_DEBUG_STATUS_BUTTON)
	{
		debugStatus.clear();
	}
}


void MainWindow::forceExit ( )
{
	// this closes the program without prompting the user for saving things. Most used for a failed program 
	// initialization.
	passCommonCommand ( ID_FORCE_EXIT );
}


void MainWindow::OnCancel()
{
	// the standard exit when e.g. the user hit's the X button. Prompts the user to save things and leave things in a 
	// good state.
	passCommonCommand(ID_FILE_MY_EXIT);
}


void MainWindow::OnClose()
{
	passCommonCommand(WM_CLOSE);
}


void MainWindow::stopNiawg()
{
	niawg.turnOff();
}


UINT MainWindow::getRepNumber()
{
	return repetitionControl.getRepetitionNumber();
}



std::string MainWindow::getSystemStatusString()
{
	std::string status;
	status = "NIAWG:\n";
	if ( !NIAWG_SAFEMODE )
	{
		status += "\tCode System is Active!\n";
		try
		{
			status += "\t" + niawg.fgenConduit.getDeviceInfo ( );
		}
		catch ( Error& err )
		{
			status += "\tFailed to get device info! Error: " + err.trace ( );
		}
	}
	else
	{
		status += "\tCode System is disabled! Enable in \"constants.h\"\n";
	}
	status += "\nMOT Scope:\n";
	if ( !MOT_SCOPE_SAFEMODE )
	{
		status += "\tCode System is Active!\n";
		try
		{
			status += "\t" + motScope.getScopeInfo( );
		}
		catch ( Error& err )
		{
			status += "\tFailed to get device info! Error: " + err.trace ( );
		}
	}
	else
	{
		status += "\tCode System is disabled! Enable in \"constants.h\"\r\n";
	}
	status += "Master/Repump Scope:\n";
	if ( !MASTER_REPUMP_SCOPE_SAFEMODE )
	{
		status += "\tCode System is Active!\n";
		try
		{
			status += "\t" + masterRepumpScope.getScopeInfo( );
		}
		catch ( Error& err )
		{
			status += "\tFailed to get device info! Error: " + err.trace ( );
		}
	}
	else
	{
		status += "\tCode System is disabled! Enable in \"constants.h\"\r\n";
	}
	return status;
}


bool MainWindow::masterIsRunning()
{
	return masterThreadManager.runningStatus();
}


RunInfo MainWindow::getRunInfo()
{
	return systemRunningInfo;
}


void MainWindow::restartNiawgDefaults()
{
	niawg.restartDefault();
}

HBRUSH MainWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	auto result = profile.handleColoring ( pWnd->GetDlgCtrlID ( ), pDC );
	if ( result ) { return *result; }
	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{
			int num = pWnd->GetDlgCtrlID();
			CBrush* ret = shortStatus.handleColor(pWnd, pDC);
			if (ret)
			{
				return *ret;
			}
			ret = boxes.handleColoring( num, pDC );
			if ( ret )
			{
				return *ret;
			}
			else
			{
				pDC->SetTextColor(_myRGBs["Text"]);
				pDC->SetBkColor(_myRGBs["Static-Bkgd"]);
				return *_myBrushes["Static-Bkgd"];
			}
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor(_myRGBs[ "MainWin-Text" ]);
			pDC->SetBkColor(_myRGBs["Interactable-Bkgd"]);
			return *_myBrushes["Interactable-Bkgd"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor(_myRGBs["Text"]);
			pDC->SetBkColor(_myRGBs["Interactable-Bkgd"]);
			return *_myBrushes["Interactable-Bkgd"];
		}
		default:
		{
			return *_myBrushes["Main-Bkgd"];
		}
	}
	return NULL;
}


void MainWindow::passCommonCommand(UINT id)
{
	// pass the command id to the common function, filling in the pointers to the windows which own objects needed.
	try
	{
		commonFunctions::handleCommonMessage ( id, this, this, TheScriptingWindow, TheAndorWindow, 
											   TheAuxiliaryWindow, TheBaslerWindow );
	}
	catch (Error& exception)
	{
		errBox ( exception.what () );
	}
}


HANDLE MainWindow::startExperimentThread( ExperimentThreadInput* input )
{
	return masterThreadManager.startExperimentThread(input);
}


void MainWindow::fillRedPgcTempProfile ( ExperimentThreadInput* input )
{
	// this function needs to be called before aux fills.
	input->seq.name = "pgcTemp";
	input->seq.sequence.resize ( 1 );
	input->profile.configuration = "Automated-PGC-Temperature-Measurement";
	input->profile.configLocation = PGC_ROUTINES_ADDRESS;
	input->profile.parentFolderName = "PGC";
	input->seq.sequence[ 0 ] = input->profile;
	input->runAndor = false;
}


void MainWindow::fillGreyPgcTempProfile ( ExperimentThreadInput* input )
{
	// this function needs to be called before aux fills.
	input->seq.name = "greyPgcTemp";
	input->seq.sequence.resize ( 1 );
	input->profile.configuration = "Automated-Grey-PGC-Temperature-Measurement";
	input->profile.configLocation = PGC_ROUTINES_ADDRESS;
	input->profile.parentFolderName = "PGC";
	input->seq.sequence[ 0 ] = input->profile;
	input->runAndor = false;
}


void MainWindow::fillMotTempProfile ( ExperimentThreadInput* input )
{
	// this function needs to be called before aux fills.
	input->seq.name = "motTemp";
	input->seq.sequence.resize ( 1 );
	input->profile.configuration = "Automated-MOT-Temperature-Measurement";
	input->profile.configLocation = MOT_ROUTINES_ADDRESS;
	input->profile.parentFolderName = "MOT";
	input->seq.sequence[ 0 ] = input->profile;
	input->runAndor = false;
}


void MainWindow::fillTempInput ( ExperimentThreadInput* input )
{
	// the mot procedure doesn't need the NIAWG at all.
	input->runNiawg = false;
	input->skipNext = NULL;
	input->rerngGuiForm = rearrangeControl.getParams ( );
	input->rerngGuiForm.active = false;
	input->runAndor = false;
}


void MainWindow::fillMotInput( ExperimentThreadInput* input )
{
	input->profile.configuration = "Set MOT Settings";
	input->profile.configLocation = MOT_ROUTINES_ADDRESS;
	input->profile.parentFolderName = "MOT";
	input->seq.name = "loadMot";
	input->seq.sequence.resize( 1 );
	input->seq.sequence[ 0 ] = input->seq.sequence[ 0 ] = input->profile;

	// the mot procedure doesn't need the NIAWG at all.
	input->runNiawg = false;
 	input->skipNext = NULL;
 	input->rerngGuiForm = rearrangeControl.getParams( ); 
 	input->rerngGuiForm.active = false; 
	input->runAndor = false;
}


void MainWindow::fillMotSizeInput ( ExperimentThreadInput* input )
{
	input->profile.configuration = "Mot_Size_Measurement";
	input->profile.configLocation = MOT_ROUTINES_ADDRESS;
	input->profile.parentFolderName = "MOT";
	input->seq.name = "loadMot";
	input->seq.sequence.resize ( 1 );
	input->seq.sequence[ 0 ] = input->seq.sequence[ 0 ] = input->profile;

	// the mot procedure doesn't need the NIAWG at all.
	input->runNiawg = false;
	input->skipNext = NULL;
	input->rerngGuiForm = rearrangeControl.getParams ( );
	input->rerngGuiForm.active = false;
	input->runAndor = false;
}


unsigned int __stdcall MainWindow::scopeRefreshProcedure( void* voidInput )
{
	// this thread just continuously requests new info from the scopes. The input is just a pointer to the scope 
	// object.
	ScopeViewer* input = (ScopeViewer*)voidInput;
	while ( true )
	{
		try
		{
			input->refreshData ( );
		}
		catch ( Error&  )
		{
			// ???
		}
	}
}


void MainWindow::fillMasterThreadSequence( ExperimentThreadInput* input )
{
	input->seq = profile.getSeqSettings( );
}


NiawgController& MainWindow::getNiawg ( )
{
	return niawg;
}


Communicator& MainWindow::getCommRef ( )
{
	return comm;
}


void MainWindow::fillMasterThreadInput(ExperimentThreadInput* input)
{
	input->debugOptions = debugger.getOptions();
	input->profile = profile.getProfileSettings();
	input->seq = profile.getSeqSettings( );
	input->rerngGuiForm = rearrangeControl.getParams( );
}


EmbeddedPythonHandler& MainWindow::getPython ( )
{
	return python;
}


void MainWindow::logParams(DataLogger* logger, ExperimentThreadInput* input)
{
	logger->logMasterInput(input);
	logger->logServoInfo ( getServoinfo ( ) );
}


profileSettings MainWindow::getProfileSettings()
{
	return profile.getProfileSettings();
}


seqSettings MainWindow::getSeqSettings( )
{
	return profile.getSeqSettings( );
}



void MainWindow::checkProfileReady()
{
	profile.allSettingsReadyCheck( TheScriptingWindow, this, TheAuxiliaryWindow, TheAndorWindow, TheBaslerWindow );
}


void MainWindow::checkProfileSave()
{
	profile.checkSaveEntireProfile( TheScriptingWindow, this, TheAuxiliaryWindow, TheAndorWindow, TheBaslerWindow );
}


void MainWindow::updateConfigurationSavedStatus(bool status)
{
	profile.updateConfigurationSavedStatus(status);
}


std::string MainWindow::getNotes()
{
	return notes.getConfigurationNotes();
}


void MainWindow::setNotes(std::string newNotes)
{
	notes.setConfigurationNotes(newNotes);
}


debugInfo MainWindow::getDebuggingOptions()
{
	return debugger.getOptions();
}


void MainWindow::setDebuggingOptions(debugInfo options)
{
	debugger.setOptions(options);
}


mainOptions MainWindow::getMainOptions()
{
	return mainOptsCtrl.getOptions();
}


void MainWindow::updateStatusText(std::string whichStatus, std::string text)
{
	std::transform(whichStatus.begin(), whichStatus.end(), whichStatus.begin(), ::tolower);
	if (whichStatus == "error")
	{
		errorStatus.addStatusText(text);
	}
	else if (whichStatus == "debug")
	{
		debugStatus.addStatusText(text);
	}
	else if (whichStatus == "main")
	{
		mainStatus.addStatusText(text);
	}
	else
	{
		thrower ( "Main Window's updateStatusText function recieved a bad argument for which status"
				 " control to update. Options are \"error\", \"debug\", and \"main\", but recieved " + whichStatus);
	}
}


void MainWindow::addTimebar(std::string whichStatus)
{
	std::transform(whichStatus.begin(), whichStatus.end(), whichStatus.begin(), ::tolower);
	if (whichStatus == "error")
	{
		errorStatus.appendTimebar();
	}
	else if (whichStatus == "debug")
	{
		debugStatus.appendTimebar();
	}
	else if (whichStatus == "main")
	{
		mainStatus.appendTimebar();
	}
	else
	{
		thrower ("Main Window's addTimebar function recieved a bad argument for which status"
			" control to update. Options are \"error\", \"debug\", and \"main\", but recieved " + whichStatus + ". This"
			"exception can be safely ignored.");
	}
}


void MainWindow::setShortStatus(std::string text)
{
	shortStatus.setText(text);
}


void MainWindow::changeShortStatusColor(std::string color)
{
	shortStatus.setColor(color);
}


void MainWindow::passDebugPress(UINT id)
{
	profile.updateConfigurationSavedStatus( false );
}


void MainWindow::passMainOptionsPress(UINT id)
{
	profile.updateConfigurationSavedStatus( false );
}


void MainWindow::handleDblClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	// effectively disable this control in the case of safemode.
	if ( !PYTHON_SAFEMODE )
	{
		texter.updatePersonInfo( );
		profile.updateConfigurationSavedStatus( false );
	}
}


void MainWindow::handleRClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	texter.deletePersonInfo();
	profile.updateConfigurationSavedStatus(false);
}


void MainWindow::handleSequenceCombo()
{
	try
	{
		profile.sequenceChangeHandler();
	}
	catch (Error& err)
	{
		getComm()->sendError(err.trace());
	}
}


void MainWindow::changeBoxColor( systemInfo<char> colors )
{
	boxes.changeColor( colors );
	if (colors.camera == 'R' || colors.niawg == 'R')
	{
		changeShortStatusColor("R");
	}
	else if (colors.camera == 'Y' || colors.niawg == 'Y')
	{
		changeShortStatusColor("Y");
	}
	else if (colors.camera == 'G' || colors.niawg == 'G')
	{
		changeShortStatusColor("G");
	}
	else
	{
		changeShortStatusColor("B");
	}
}


void MainWindow::abortMasterThread()
{
	if (masterThreadManager.runningStatus())
	{
		masterThreadManager.abort();
		autoF5_AfterFinish = false;
	}
	else
	{
		thrower ("Can't abort, experiment was not running.\r\n");
	}
}

bool MainWindow::experimentIsPaused( )
{
	return masterThreadManager.getIsPaused( );
}



LRESULT MainWindow::onStatusTextMessage(WPARAM wParam, LPARAM lParam)
{
	char* pointerToMessage = (char*)lParam;
	std::string statusMessage(pointerToMessage);
	delete[] pointerToMessage;
	mainStatus.addStatusText(statusMessage);
	return 0;
}


LRESULT MainWindow::onErrorMessage(WPARAM wParam, LPARAM lParam)
{
	// for simple warnings, it just posts the message.
	char* pointerToMessage = (char*)lParam;
	std::string statusMessage(pointerToMessage);
	delete[] pointerToMessage;
	if ( statusMessage == "Andor camera, NIAWG, Master, and Basler camera were not running. Can't Abort.\r\n" )
	{
		errorStatus.addStatusText ( statusMessage );
	}
	else if ( statusMessage != "" )
	{
		errorStatus.addStatusText( statusMessage );
		auto asyncbeep = std::async( std::launch::async, [] { Beep( 1000, 1000 ); } );
	}
	return 0;
}


LRESULT MainWindow::onFatalErrorMessage(WPARAM wParam, LPARAM lParam)
{
	autoF5_AfterFinish = false;
	// normal msg stuff
	char* pointerToMessage = (char*)lParam;
	std::string statusMessage(pointerToMessage);
	delete[] pointerToMessage;
	errorStatus.addStatusText(statusMessage);
	// resetting things.
	TheScriptingWindow->setIntensityDefault();
	std::string msgText = "Exited with Error!\r\nPassively Outputting Default Waveform.";
	changeShortStatusColor("R");
	comm.sendColorBox( System::Niawg, 'R' );
	try
	{
		niawg.restartDefault();
		comm.sendError("EXITED WITH ERROR!");
		comm.sendColorBox( System::Niawg, 'R' );
		comm.sendStatus("EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n");
	}
	catch (Error& except)
	{
		comm.sendError("EXITED WITH ERROR! " + except.trace());
		comm.sendColorBox( System::Niawg, 'R' );
		comm.sendStatus("EXITED WITH ERROR!\r\nNIAWG RESTART FAILED!\r\n");
	}
	setNiawgRunningState( false );
	auto asyncbeep = std::async ( std::launch::async, [] { Beep ( 800, 50 ); } );
	errBox ( statusMessage );
	return 0;
}


void MainWindow::stopRearranger( )
{
	niawg.turnOffRerng( );
}


void MainWindow::waitForRearranger( )
{
	niawg.waitForRerng( true );
}


void MainWindow::onNormalFinishMessage()
{
	TheScriptingWindow->setIntensityDefault();
	std::string msgText = "Passively Outputting Default Waveform";
	setShortStatus(msgText);
	changeShortStatusColor("B");
	stopRearranger( );
	TheAndorWindow->wakeRearranger();
	TheAndorWindow->cleanUpAfterExp ( );
	handleFinish ( );
	comm.sendColorBox( System::Niawg, 'B' );
	try
	{
		niawg.restartDefault();
	}
	catch ( Error& except )
	{
		comm.sendError( "The niawg finished normally, but upon restarting the default waveform, threw the "
						"following error: " + except.trace( ) );
		comm.sendColorBox( System::Niawg, 'B' );
		comm.sendStatus( "ERROR!\r\n" );
	}
	setNiawgRunningState( false );
	try
	{
		waitForRearranger( );
	}
	catch ( Error& err )
	{
		comm.sendError( err.trace( ) );
	}
	if ( TheAndorWindow->wantsThresholdAnalysis ( ) ) 
	{
		handleThresholdAnalysis ( );
	}
	if ( autoF5_AfterFinish )
	{
		commonFunctions::handleCommonMessage ( ID_ACCELERATOR_F5, this, this, TheScriptingWindow, TheAndorWindow,
											   TheAuxiliaryWindow, TheBaslerWindow );
		autoF5_AfterFinish = false;
	}
}


void MainWindow::handleFinish()
{
	time_t t = time(0);
	struct tm now;
	localtime_s(&now, &t);
	std::string message = "Experiment Completed at ";
	if (now.tm_hour < 10)
	{
		message += "0";
	}
	message += str(now.tm_hour) + ":";
	if (now.tm_min < 10)
	{
		message += "0";
	}
	message += str(now.tm_min) + ":";
	if (now.tm_sec < 10)
	{
		message += "0";
	}
	message += str(now.tm_sec);
	try
	{
		texter.sendMessage(message, &python, "Finished");
	}
	catch (Error& err)
	{
		comm.sendError(err.trace());
	}
}


Communicator* MainWindow::getComm()
{
	return &comm;
}


LRESULT MainWindow::onDebugMessage(WPARAM wParam, LPARAM lParam)
{
	char* pointerToMessage = (char*)lParam;
	std::string statusMessage(pointerToMessage);
	delete[] pointerToMessage;
	debugStatus.addStatusText(statusMessage);
	return 0;
}

// MESSAGE MAP FUNCTION
void MainWindow::ServoRClick (NMHDR* pNotifyStruct, LRESULT* result)
{
	try
	{
		updateConfigurationSavedStatus (false);
		servos.deleteServo ();
	}
	catch (Error & err)
	{
		comm.sendError ("Servo Right-Click Handler Failed.\n" + err.trace ());
	}
}

// MESSAGE MAP FUNCTION
void MainWindow::ServoDblClick (NMHDR* pNotifyStruct, LRESULT* result)
{
	try
	{
		updateConfigurationSavedStatus (false);
		servos.handleListViewClick ();
	}
	catch (Error & err)
	{
		comm.sendError ("Servo Double-Click handler failed." + err.trace ());
	}
}

// MESSAGE MAP FUNCTION
LRESULT MainWindow::autoServo (WPARAM w, LPARAM l)
{
	try
	{
		updateConfigurationSavedStatus (false);
		if (servos.autoServo ())
		{
			runServos ();
		}
	}
	catch (Error & err)
	{
		comm.sendError ("Auto-Servo Failed.\n" + err.trace ());
	}
	return TRUE;
}

// MESSAGE MAP FUNCTION
void MainWindow::runServos ()
{
	try
	{
		updateConfigurationSavedStatus (false);
		comm.sendStatus ("Running Servos...\r\n");
		servos.runAll ();
	}
	catch (Error & err)
	{
		comm.sendError ("Running Servos failed.\n" + err.trace ());
	}
}

std::vector<servoInfo> MainWindow::getServoinfo ()
{
	return servos.getServoInfo ();
}

void MainWindow::handleMasterConfigOpen (std::stringstream& configStream, Version version)
{
	servos.handleOpenMasterConfig (configStream, version);
}

void MainWindow::handleMasterConfigSave (std::stringstream& configStream)
{
	servos.handleSaveMasterConfig (configStream);
}