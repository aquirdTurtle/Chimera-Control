#include "stdafx.h"

#include "MainWindow.h"
#include "AndorWindow.h"
#include "AuxiliaryWindow.h"
#include "ScriptingWindow.h"
#include "BaslerWindow.h"
#include "DeformableMirrorWindow.h"

#include "GeneralUtilityFunctions/commonFunctions.h"
#include "LowLevel/externals.h"
#include "ExperimentThread/autoCalConfigInfo.h"
#include "ExperimentThread/Communicator.h"

#include <future>


MainWindow::MainWindow( UINT id, CDialog* splash, chronoTime* startTime) : IChimeraWindow( id ), profile( PROFILES_PATH ),
	masterConfig( MASTER_CONFIGURATION_FILE_ADDRESS ),
	appSplash( splash ),
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

IMPLEMENT_DYNAMIC( MainWindow, IChimeraWindow)

BEGIN_MESSAGE_MAP (MainWindow, IChimeraWindow)
	ON_WM_CTLCOLOR ()
	ON_WM_SIZE ()
	ON_CBN_SELENDOK (IDC_SEQUENCE_COMBO, &handleSequenceCombo)
	ON_NOTIFY (NM_DBLCLK, IDC_SMS_TEXTING_LISTVIEW, &handleDblClick)
	ON_NOTIFY (NM_RCLICK, IDC_SMS_TEXTING_LISTVIEW, &handleRClick)
	ON_NOTIFY (NM_DBLCLK, IDC_SERVO_LISTVIEW, &ServoDblClick)
	ON_NOTIFY (NM_RCLICK, IDC_SERVO_LISTVIEW, &ServoRClick)
	ON_NOTIFY (NM_CUSTOMDRAW, IDC_SERVO_LISTVIEW, &drawServoListview)

	ON_EN_CHANGE (IDC_CONFIGURATION_NOTES, &notifyConfigUpdate)
	ON_EN_CHANGE (IDC_REPETITION_EDIT, &notifyConfigUpdate)
	ON_MESSAGE (CustomMessages::RepProgressMessageID, &onRepProgress)
	ON_MESSAGE (CustomMessages::StatusUpdateMessageID, &onStatusTextMessage)
	ON_MESSAGE (CustomMessages::ErrorUpdateMessageID, &onErrorMessage)
	ON_MESSAGE (CustomMessages::FatalErrorMessageID, &onFatalErrorMessage)
	ON_MESSAGE (CustomMessages::DebugUpdateMessageID, &onDebugMessage)
	ON_MESSAGE (CustomMessages::NoAtomsAlertMessageID, &onNoAtomsAlertMessage)
	ON_MESSAGE (CustomMessages::NoMotAlertMessageID, &onNoMotAlertMessage)
	ON_MESSAGE (CustomMessages::GeneralFinMsgID, &onFinish)

	ON_COMMAND_RANGE (IDC_DEBUG_OPTIONS_RANGE_BEGIN, IDC_DEBUG_OPTIONS_RANGE_END, &passDebugPress)
	ON_COMMAND_RANGE (IDC_MAIN_OPTIONS_RANGE_BEGIN, IDC_MAIN_OPTIONS_RANGE_END, &passMainOptionsPress)
	ON_COMMAND_RANGE (IDC_MAIN_STATUS_BUTTON, IDC_MAIN_STATUS_BUTTON, &passClear)
	ON_COMMAND_RANGE (IDC_ERROR_STATUS_BUTTON, IDC_ERROR_STATUS_BUTTON, &passClear)
	ON_COMMAND_RANGE (IDC_DEBUG_STATUS_BUTTON, IDC_DEBUG_STATUS_BUTTON, &passClear)
	ON_COMMAND_RANGE (ID_PLOT_POP_IDS_BEGIN, ID_PLOT_POP_IDS_END, &handlePlotPop)
	ON_COMMAND (IDC_SELECT_CONFIG_COMBO, &passConfigPress)
	ON_COMMAND (IDC_SERVO_CAL, &runServos)
	ON_CBN_SELENDOK (IDC_SERVO_UNITS_COMBO, &handleServoUnitsComboChange)
	ON_MESSAGE ( CustomMessages::AutoServoMessage, &autoServo)
	ON_WM_RBUTTONUP( )
	ON_WM_LBUTTONUP( )
	ON_WM_PAINT( )
	ON_WM_TIMER( )
END_MESSAGE_MAP()


void MainWindow::handleServoUnitsComboChange ()
{
	servos.refreshListview ();
}


void MainWindow::handlePlotPop(UINT id)
{
	masterRepumpScope.handlePlotPop (id, this); 
	motScope.handlePlotPop (id, this);
}


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


void MainWindow::handleThresholdAnalysis ()
{
	auto grid = andorWin->getMainAtomGrid ();
	auto dateStr = andorWin->getMostRecentDateString ();
	auto fid = andorWin->getMostRecentFid ();
	auto ppr = andorWin->getPicsPerRep ();
	std::string gridString = "[" + str (grid.topLeftCorner.row - 1) + "," + str (grid.topLeftCorner.column - 1) + ","
		+ str (grid.pixelSpacing) + "," + str (grid.width) + "," + str (grid.height) + "]";
	try
	{
		python.thresholdAnalysis (dateStr, fid, gridString, ppr);
	}
	catch (Error & err)
	{
		comm.sendError ("Threshold Analysis Failed! " + err.trace ());
	}
}


LRESULT MainWindow::onFinish ( WPARAM wp, LPARAM lp )
{
	ExperimentType type = static_cast<ExperimentType>( wp );
	switch ( type )
	{
		case ExperimentType::Normal:
			onNormalFinishMessage ( );
			break;
		case ExperimentType::LoadMot:
			break;
		case ExperimentType::AutoCal:
			onAutoCalFin ();
			break;
		case ExperimentType::MachineOptimization:
			onMachineOptRoundFin ( );
			break;
	}
	return 0;
}


UINT MainWindow::getAutoCalNumber () { return autoCalNum; }


void MainWindow::onAutoCalFin ()
{
	try
	{
		scriptWin->restartNiawgDefaults ();
	}
	catch (Error & except)
	{
		comm.sendError ("The niawg finished normally, but upon restarting the default waveform, threw the "
			"following error: " + except.trace ());
		comm.sendColorBox (System::Niawg, 'B');
		comm.sendStatus ("ERROR!\r\n");
	}
	scriptWin->setNiawgRunningState (false);
	andorWin->cleanUpAfterExp ();
	autoCalNum++;
	if (autoCalNum >= AUTO_CAL_LIST.size ())
	{
		// then just finished the calibrations.
		autoCalNum = 0;
		infoBox ("Finished Automatic Calibrations.");
	}
	else
	{
		commonFunctions::handleCommonMessage ( ID_ACCELERATOR_F11, this );
	}
}

void MainWindow::onGreyTempCalFin ( ) { infoBox ( "Finished MOT Calibrations." ); }

void MainWindow::onMachineOptRoundFin (  )
{
	// do normal finish
	onNormalFinishMessage ( );
	Sleep ( 1000 );
	// then restart.
	commonFunctions::handleCommonMessage ( ID_MACHINE_OPTIMIZATION, this );
}


void MainWindow::OnTimer( UINT_PTR id ) { OnPaint( ); }


void MainWindow::loadCameraCalSettings( ExperimentThreadInput* input )
{
	input->skipNext = NULL;
	input->expType = ExperimentType::CameraCal;
}


BOOL MainWindow::handleAccelerators( HACCEL m_haccel, LPMSG lpMsg )
{
	if ( auxWin != NULL )
	{
		return auxWin->handleAccelerators( m_haccel, lpMsg );
	}
	else
	{
		return FALSE;
	}
}


void MainWindow::OnPaint( )
{
	CDialog::OnPaint( );
	if ( !expThreadManager.runningStatus() )
	{
		CRect size;
		GetClientRect( &size );
		SmartDC sdc (this);
		sdc.get()->SetBkColor( _myRGBs["Main-Bkgd"] );
		UINT width = size.right - size.left, height = size.bottom - size.top;
		masterRepumpScope.refreshPlot( sdc.get(), width, height, _myBrushes["Main-Bkgd"], _myBrushes["Interactable-Bkgd"] );
		motScope.refreshPlot( sdc.get(), width, height, _myBrushes["Main-Bkgd"], _myBrushes["Interactable-Bkgd"] );
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


void MainWindow::OnRButtonUp( UINT stuff, CPoint clickLocation ) { 	andorWin->stopSound( ); }
void MainWindow::OnLButtonUp (UINT stuff, CPoint clickLocation) { andorWin->stopSound (); }


void MainWindow::passConfigPress( )
{	
	try
	{
		profile.handleSelectConfigButton( this, scriptWin, this, auxWin, andorWin, basWin, dmWin );
	}
	catch ( Error& err )
	{
		comm.sendError( err.trace( ) );
	}
}


LRESULT MainWindow::onNoMotAlertMessage( WPARAM wp, LPARAM lp )
{
	try
	{
		if ( andorWin->wantsAutoPause ( ) )
		{
			expThreadManager.pause ( );
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
		if ( andorWin->wantsAutoPause( ) )
		{
			expThreadManager.pause( );
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
	SetWindowTextA ( "Main Window" );

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
	// not done with the script, it will not stay on the NIAWG, so I need to keep track of it so thatI can reload it onto the NIAWG when necessary.	
	/// Initialize Windows
	std::string which = "";
	try
	{
		mainWin = this;
		which = "Scripting";
		scriptWin = new ScriptingWindow;
		which = "Camera";
		andorWin = new AndorWindow;
		which = "Auxiliary";
		auxWin = new AuxiliaryWindow;
		which = "Basler";
		basWin = new BaslerWindow;
		which = "DmWin";
		dmWin = new DeformableMirrorWindow;
	}
	catch ( Error& err )
	{
		errBox( "FATAL ERROR: " + which + " Window constructor failed! Error: " + err.trace( ) );
		forceExit ( );
		return -1;
	}
	scriptWin->loadFriends( this, scriptWin, auxWin, basWin, dmWin, andorWin );
	andorWin->loadFriends (this, scriptWin, auxWin, basWin, dmWin, andorWin);
	auxWin->loadFriends (this, scriptWin, auxWin, basWin, dmWin, andorWin);
	basWin->loadFriends (this, scriptWin, auxWin, basWin, dmWin, andorWin);
	dmWin->loadFriends (this, scriptWin, auxWin, basWin, dmWin, andorWin);
	startupTimes.push_back(chronoClock::now());
	try
	{
		// these each call oninitdialog after the create call. Hence the try / catch.
		scriptWin->Create( IDD_LARGE_TEMPLATE, GetDesktopWindow() );
		andorWin->Create( IDD_LARGE_TEMPLATE, GetDesktopWindow( ) );
		auxWin->Create( IDD_LARGE_TEMPLATE, GetDesktopWindow( ) );
		basWin->Create ( IDD_LARGE_TEMPLATE, GetDesktopWindow ( ) );
		dmWin->Create (IDD_LARGE_TEMPLATE, GetDesktopWindow ());
	}
	catch ( Error& err )
	{
		errBox( "FATAL ERROR: Failed to create window! " + err.trace( ) );
		forceExit ( );
		return -1;
	}
	/// initialize main window controls.
	comm.initialize( this );
	int id = 1000;
	POINT controlLocation = { 0,0 };
	mainStatus.initialize( controlLocation, this, id, 870, "EXPERIMENT STATUS", RGB( 100, 100, 250 ), toolTips, IDC_MAIN_STATUS_BUTTON );
	statBox.initialize ( controlLocation, id, this, 960, toolTips);
	shortStatus.initialize (controlLocation, this, id, toolTips);
	controlLocation = { 480, 0 };
	errorStatus.initialize( controlLocation, this, id, 420, "ERROR STATUS", RGB( 100, 0, 0 ), toolTips,
							IDC_ERROR_STATUS_BUTTON );
	debugStatus.initialize( controlLocation, this, id, 420, "DEBUG STATUS", RGB( 13, 152, 186 ), toolTips,
							IDC_DEBUG_STATUS_BUTTON );
	controlLocation = { 960, 0 };
	profile.initialize( controlLocation, this, id, toolTips);
	controlLocation = { 960, 175 };
	notes.initialize( controlLocation, this, id, toolTips);
	masterRepumpScope.initialize( controlLocation, 480, 130, this, getPlotPens( ), getPlotFont( ), getPlotBrushes(),
								  ID_MASTER_REPUMP_SCOPE_VIEWER_POP_ID,	"Master/Repump" );
	motScope.initialize( controlLocation, 480, 130, this, getPlotPens( ), getPlotFont( ), getPlotBrushes( ), 
						 ID_MOT_SCOPE_VIEWER_POP_ID, "MOT" );
	servos.initialize ( controlLocation, toolTips, this, id, &auxWin->getAiSys (), &auxWin->getAoSys (),
						auxWin->getTtlSystem(), &auxWin->getGlobals ());
	controlLocation = { 1440, 50 };
	repetitionControl.initialize( controlLocation, toolTips, this, id );
	mainOptsCtrl.initialize( id, controlLocation, this, toolTips);
	debugger.initialize( id, controlLocation, this, toolTips);
	texter.initialize( controlLocation, this, id, toolTips);

	menu.LoadMenu( IDR_MAIN_MENU );
	SetMenu( &menu );

	// just initializes the rectangles.
	andorWin->redrawPictures( true );
	try
	{
		masterConfig.load( this, auxWin, andorWin);
	}
	catch ( Error& err )
	{
		errBox( err.trace( ) );
	}
	startupTimes.push_back(chronoClock::now());
	ShowWindow( SW_MAXIMIZE );
	// for some reason without this the main window was defaulting to topmost. 
	SetWindowPos (&wndBottom, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE );
	
	andorWin->ShowWindow( SW_MAXIMIZE );
	scriptWin->ShowWindow( SW_MAXIMIZE );
	auxWin->ShowWindow( SW_MAXIMIZE );
	basWin->ShowWindow ( SW_MAXIMIZE );
	dmWin->ShowWindow (SW_MAXIMIZE);

	std::vector<CDialog*> windows = { basWin, NULL, andorWin, this, scriptWin, auxWin, dmWin };
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
		initializationString += auxWin->getOtherSystemStatusMsg( );
		initializationString += andorWin->getSystemStatusString( );
		initializationString += auxWin->getVisaDeviceStatus( );
		initializationString += scriptWin->getSystemStatusString( );
		initializationString += auxWin->getMicrowaveSystemStatus( );
		infoBox( initializationString );
	}
	catch ( Error& err )
	{
		errBox( err.trace( ) );
	}
	SetTimer( 1, 10000, NULL );
	// set up the threads that update the scope data.
	_beginthreadex( NULL, NULL, &MainWindow::scopeRefreshProcedure, &masterRepumpScope, NULL, NULL);
	_beginthreadex( NULL, NULL, &MainWindow::scopeRefreshProcedure, &motScope, NULL, NULL );
	//
	updateConfigurationSavedStatus( true );
	return IChimeraWindow::OnInitDialog ();
}


void MainWindow::showHardwareStatus ( )
{
	try
	{
		// ordering of aux window pieces is a bit funny because I want the devices grouped by type, not by window.
		std::string initializationString;
		initializationString += getSystemStatusString ( );
		initializationString += auxWin->getOtherSystemStatusMsg ( );
		initializationString += andorWin->getSystemStatusString ( );
		initializationString += auxWin->getVisaDeviceStatus ( );
		initializationString += scriptWin->getSystemStatusString ( );
		initializationString += auxWin->getMicrowaveSystemStatus ( );
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
	auxWin->setMenuCheck ( menuItem, itemState );
	andorWin->setMenuCheck ( menuItem, itemState );
	basWin->setMenuCheck ( menuItem, itemState );
	scriptWin->setMenuCheck ( menuItem, itemState );
	dmWin->setMenuCheck (menuItem, itemState);
}


void MainWindow::handlePause()
{
	if (expThreadManager.runningStatus())
	{
		if (expThreadManager.getIsPaused())
		{
			// then it's currently paused, so unpause it.
			checkAllMenus ( ID_RUNMENU_PAUSE, MF_UNCHECKED );
			expThreadManager.unPause();
			comm.sendColorBox( System::Master, 'G' );
		}
		else
		{
			// then not paused so pause it.
			checkAllMenus ( ID_RUNMENU_PAUSE, MF_CHECKED );
			comm.sendColorBox( System::Master, 'Y' );
			expThreadManager.pause();
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

void MainWindow::windowSaveConfig(ConfigStream& saveFile)
{
	notes.handleSaveConfig(saveFile);
	mainOptsCtrl.handleSaveConfig(saveFile);
	debugger.handleSaveConfig(saveFile);
	repetitionControl.handleSaveConfig(saveFile);
}

void MainWindow::windowOpenConfig(ConfigStream& configStream, Version ver )
{
	try
	{
		ProfileSystem::standardOpenConfig ( configStream, "CONFIGURATION_NOTES", &notes);
		mainOptsCtrl.setOptions ( ProfileSystem::stdConfigGetter ( configStream, "MAIN_OPTIONS", 
																	MainOptionsControl::getSettingsFromConfig ) );
		ProfileSystem::standardOpenConfig ( configStream, "DEBUGGING_OPTIONS", &debugger );
		repetitionControl.setRepetitions ( ProfileSystem::stdConfigGetter ( configStream, "REPETITIONS", 
																			 Repetitions::getSettingsFromConfig ));
		
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
	statBox.rearrange( cx, cy, getFonts());
	repetitionControl.rearrange(cx, cy, getFonts());
	servos.rearrange (cx, cy, getFonts ());
	SetRedraw();
	RedrawWindow();
}

fontMap MainWindow::getFonts() { return mainFonts; }

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

UINT MainWindow::getRepNumber() { return repetitionControl.getRepetitionNumber(); }

std::string MainWindow::getSystemStatusString()
{
	std::string status;
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

HBRUSH MainWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	auto result = profile.handleColoring ( pWnd->GetDlgCtrlID ( ), pDC );
	if ( result ) { return *result; }
	return IChimeraWindow::OnCtlColor (pDC, pWnd, nCtlColor);
	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{
			int num = pWnd->GetDlgCtrlID();
			CBrush* ret = shortStatus.handleColor(pWnd, pDC);
			if (ret) { return *ret; }
		}
	}
	return IChimeraWindow::OnCtlColor (pDC, pWnd, nCtlColor);
}


HANDLE MainWindow::startExperimentThread( ExperimentThreadInput* input )
{
	return expThreadManager.startExperimentThread(input);
}


void MainWindow::fillMotInput( ExperimentThreadInput* input )
{
	input->profile.configuration = "Set MOT Settings";
	input->profile.configLocation = MOT_ROUTINES_ADDRESS;
	input->profile.parentFolderName = "MOT";
	// the mot procedure doesn't need the NIAWG at all.
 	input->skipNext = NULL;
	input->runList.andor = false;
}


unsigned int __stdcall MainWindow::scopeRefreshProcedure( void* voidInput )
{
	// this thread continuously requests new info from the scopes. The input is just a pointer to the scope object.
	ScopeViewer* input = (ScopeViewer*)voidInput;
	while ( true )
	{
		try	{ input->refreshData ( ); }
		catch ( Error&  ) { /* ??? */ }
	}
}

bool MainWindow::masterIsRunning () { return expThreadManager.runningStatus (); }
RunInfo MainWindow::getRunInfo () { return systemRunningInfo; }
Communicator& MainWindow::getCommRef ( ) { return comm; }
EmbeddedPythonHandler& MainWindow::getPython () { return python; }
profileSettings MainWindow::getProfileSettings () { return profile.getProfileSettings (); }
seqSettings MainWindow::getSeqSettings () { return profile.getSeqSettings (); }
std::string MainWindow::getNotes () { return notes.getConfigurationNotes (); }
void MainWindow::setNotes (std::string newNotes) { notes.setConfigurationNotes (newNotes); }
debugInfo MainWindow::getDebuggingOptions () { return debugger.getOptions (); }
void MainWindow::setDebuggingOptions (debugInfo options) { debugger.setOptions (options); }
mainOptions MainWindow::getMainOptions () { return mainOptsCtrl.getOptions (); }
void MainWindow::setShortStatus (std::string text) { shortStatus.setText (text); }
void MainWindow::changeShortStatusColor (std::string color) { shortStatus.setColor (color); }
void MainWindow::passDebugPress (UINT id) { profile.updateConfigurationSavedStatus (false); }
void MainWindow::passMainOptionsPress (UINT id) { profile.updateConfigurationSavedStatus (false); }
bool MainWindow::experimentIsPaused () { return expThreadManager.getIsPaused (); }
Communicator* MainWindow::getComm () { return &comm; }


void MainWindow::fillMasterThreadInput(ExperimentThreadInput* input)
{
	input->debugOptions = debugger.getOptions();
	input->profile = profile.getProfileSettings();
}


void MainWindow::logParams(DataLogger* logger, ExperimentThreadInput* input)
{
	logger->logMasterInput(input);
	logger->logServoInfo ( getServoinfo ( ) );
}


void MainWindow::checkProfileReady()
{
	profile.allSettingsReadyCheck( scriptWin, this, auxWin, andorWin, basWin );
}


void MainWindow::checkProfileSave()
{
	profile.checkSaveEntireProfile( scriptWin, this, auxWin, andorWin, basWin);
}


void MainWindow::updateConfigurationSavedStatus(bool status)
{
	profile.updateConfigurationSavedStatus(status);
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
	IChimeraWindow::changeBoxColor (colors);
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
	if (expThreadManager.runningStatus())
	{
		expThreadManager.abort();
		autoF5_AfterFinish = false;
	}
	else { thrower ("Can't abort, experiment was not running.\r\n"); }
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
	scriptWin->setIntensityDefault();
	std::string msgText = "Exited with Error!\r\nPassively Outputting Default Waveform.";
	changeShortStatusColor("R");
	comm.sendColorBox( System::Niawg, 'R' );
	try
	{
		scriptWin->restartNiawgDefaults ();
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
	scriptWin->setNiawgRunningState( false );
	auto asyncbeep = std::async ( std::launch::async, [] { Beep ( 800, 50 ); } );
	errBox ( statusMessage );
	return 0;
}


void MainWindow::onNormalFinishMessage()
{
	scriptWin->setIntensityDefault();
	setShortStatus("Passively Outputting Default Waveform");
	changeShortStatusColor("B");
	scriptWin->stopRearranger( );
	andorWin->wakeRearranger();
	andorWin->cleanUpAfterExp ( );
	handleFinish ( );
	comm.sendColorBox( System::Niawg, 'B' );
	try	{ scriptWin->restartNiawgDefaults (); }
	catch ( Error& except )
	{
		comm.sendError( "The niawg finished normally, but upon restarting the default waveform, threw the "
						"following error: " + except.trace( ) );
		comm.sendColorBox( System::Niawg, 'B' );
		comm.sendStatus( "ERROR!\r\n" );
	}
	scriptWin->setNiawgRunningState( false );
	try { scriptWin->waitForRearranger (); }
	catch ( Error& err ) { comm.sendError( err.trace( ) ); }
	if (andorWin->wantsThresholdAnalysis ()) { handleThresholdAnalysis (); }
	if ( autoF5_AfterFinish )
	{
		commonFunctions::handleCommonMessage ( ID_ACCELERATOR_F5, this);
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
		if (servos.wantsCalAutoServo ())
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
		servos.runAll (comm);
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