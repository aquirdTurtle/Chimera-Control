#include "stdafx.h"
#include "commonFunctions.h"
#include "MainWindow.h"
#include "CameraWindow.h"
#include "AuxiliaryWindow.h"
#include "ScriptingWindow.h"
#include "BaslerWindow.h"
#include <future>
#include "Thrower.h"
#include "externals.h"


MainWindow::MainWindow( UINT id, CDialog* splash, chronoTime* startTime) : CDialog( id ), profile( PROFILES_PATH ),
	masterConfig( MASTER_CONFIGURATION_FILE_ADDRESS ),
	appSplash( splash ),
	niawg( 1, 14, NIAWG_SAFEMODE ),
	masterRepumpScope( MASTER_REPUMP_SCOPE_ADDRESS, MASTER_REPUMP_SCOPE_SAFEMODE, 4 ),
	motScope( MOT_SCOPE_ADDRESS, MOT_SCOPE_SAFEMODE, 2 )
{
	programStartTime = startTime;
	startupTimes.push_back(chronoClock::now());
	// create all the main rgbs and brushes. I want to make sure this happens before other windows are created.
	mainRGBs["Light Green"]			= RGB( 163,	190, 140);
	mainRGBs["Slate Grey"]			= RGB( 101,	115, 126);
	mainRGBs["Pale Pink"]			= RGB( 180,	142, 173);
	mainRGBs["Musky Red"]			= RGB( 191,	97,	 106);
	// this "base04", while not listed on the solarized web site, is used by the visual studio solarized for edit area.
	// it's a nice darker color that matches the solarized pallete.
	mainRGBs["Solarized Base04"]	= RGB( 0,	30,  38 );
	mainRGBs["Solarized Base03"]	= RGB( 0,	43,  54 );
	mainRGBs["Solarized Base02"]	= RGB( 7,	54,  66 );
	mainRGBs["Solarized Base01"]	= RGB( 88,	110, 117 );
	mainRGBs["Solarized Base00"]	= RGB( 101, 123, 131 );
	mainRGBs["Solarized Base0"]		= RGB( 131, 148, 150 );
	mainRGBs["Solarized Base1"]		= RGB( 101, 123, 131 );
	mainRGBs["Solarized Base2"]		= RGB( 238, 232, 213 );
	mainRGBs["Solarized Base3"]		= RGB( 253, 246, 227 );
	mainRGBs["Solarized Red"]		= RGB( 220, 50,  47);
	mainRGBs["Solarized Violet"]	= RGB( 108, 113, 196);
	mainRGBs["Solarized Cyan"]		= RGB( 42,	161, 152);
	mainRGBs["Solarized Green"]		= RGB( 133, 153, 0);
	mainRGBs["Solarized Blue"]		= RGB( 38,	139, 210);
	mainRGBs["Solarized Magenta"]	= RGB( 211, 54,  130);
	mainRGBs["Solarized Orange"]	= RGB( 203, 75,  22);
	mainRGBs["Solarized Yellow"]	= RGB( 181, 137, 0);
	mainRGBs["Slate Green"]			= RGB( 23,	84,  81);
	mainRGBs["Dark Grey"]			= RGB( 15,	15,  15);
	mainRGBs["Dark Grey Red"]		= RGB( 20,	12,  12);
	mainRGBs["Medium Grey"]			= RGB( 30,	30,  30);
	mainRGBs["Light Grey"]			= RGB( 60,	60,  60);
	mainRGBs["Green"]				= RGB( 50,	200, 50);
	mainRGBs["Red"]					= RGB( 200,	50,  50);
	mainRGBs["Blue"]				= RGB( 50,	50,  200);
	mainRGBs["Gold"]				= RGB( 218,	165, 32);
	mainRGBs["White"]				= RGB( 255, 255, 255);
	mainRGBs["Light Red"]			= RGB( 255, 100, 100);
	mainRGBs["Dark Red"]			= RGB( 150, 0,   0);
	mainRGBs["Light Blue"]			= RGB( 100, 100, 255);
	mainRGBs["Forest Green"]		= RGB( 34,	139, 34);
	mainRGBs["Dark Green"]			= RGB( 0,	50,  0);
	mainRGBs["Dull Red"]			= RGB( 107, 35,  35);
	mainRGBs["Dark Lavender"]		= RGB( 100, 100, 205);
	mainRGBs["Teal"]				= RGB( 0,	255, 255);
	mainRGBs["Tan"]					= RGB( 210, 180, 140);
	mainRGBs["Purple"]				= RGB( 147, 112, 219);
	mainRGBs["Orange"]				= RGB( 255, 165, 0);
	mainRGBs["Brown"]				= RGB( 139, 69,  19);
	mainRGBs["Black"]				= RGB( 0,	0,   0);
	mainRGBs["Dark Blue"]			= RGB( 0,	0,   75);
	// there are less brushes because these are only used for backgrounds.
	// the following single-line statements are all equivalent to these two statements:
	// mainBrushes["Name"] = new CBrush;
	// mainBrushes["Name"]->CreateSolidBrush(...);
	(mainBrushes["Black"]				= new CBrush)->CreateSolidBrush( mainRGBs["Black"] );
	(mainBrushes["Dark Red"]			= new CBrush)->CreateSolidBrush( mainRGBs["Dark Red"]);
	(mainBrushes["Gold"]				= new CBrush)->CreateSolidBrush( mainRGBs["Gold"]);
	(mainBrushes["Dark Grey"]			= new CBrush)->CreateSolidBrush( mainRGBs["Dark Grey"]);
	(mainBrushes["Dark Grey Red"]		= new CBrush)->CreateSolidBrush( mainRGBs["Dark Grey Red"]);
	(mainBrushes["Medium Grey"]			= new CBrush)->CreateSolidBrush( mainRGBs["Medium Grey"]);
	(mainBrushes["Light Grey"]			= new CBrush)->CreateSolidBrush( mainRGBs["Light Grey"]);
	(mainBrushes["Green"]				= new CBrush)->CreateSolidBrush( mainRGBs["Green"]);
	(mainBrushes["Red"]					= new CBrush)->CreateSolidBrush( mainRGBs["Red"]);
	(mainBrushes["White"]				= new CBrush)->CreateSolidBrush( mainRGBs["White"]);
	(mainBrushes["Dull Red"]			= new CBrush)->CreateSolidBrush( mainRGBs["Dull Red"]);
	(mainBrushes["Dark Blue"]			= new CBrush)->CreateSolidBrush( mainRGBs["Dark Blue"]);
	(mainBrushes["Dark Green"]			= new CBrush)->CreateSolidBrush( mainRGBs["Dark Green"]);
	(mainBrushes["Solarized Blue"]		= new CBrush)->CreateSolidBrush( mainRGBs["Solarized Blue"] );
	(mainBrushes["Solarized Base03"]	= new CBrush)->CreateSolidBrush( mainRGBs["Solarized Base03"] );
	(mainBrushes["Solarized Base02"]	= new CBrush)->CreateSolidBrush( mainRGBs["Solarized Base02"] );
	(mainBrushes["Solarized Base01"]	= new CBrush)->CreateSolidBrush( mainRGBs["Solarized Base01"] );
	(mainBrushes["Solarized Base04"]	= new CBrush)->CreateSolidBrush( mainRGBs["Solarized Base04"] );
	/// the following are all equivalent to:
	// mainFonts["Font name"] = new CFont;
	// mainFonts["Font name"].CreateFontA(...);
	(mainFonts["Smaller Font Max"] = new CFont)
		->CreateFontA(27, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Normal Font Max"] = new CFont)
		->CreateFontA(34, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Heading Font Max"] = new CFont)
		->CreateFontA(42, 0, 0, 0, FW_DONTCARE, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Old Sans Black"));
	(mainFonts["Code Font Max"] = new CFont)
		->CreateFontA(32, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));
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
	ON_COMMAND( IDC_RERNG_EXPERIMENT_BUTTON, &MainWindow::passExperimentRerngButton )
	ON_CBN_SELENDOK ( IDC_RERNG_MODE_COMBO, &MainWindow::passRerngModeComboChange )
	ON_WM_RBUTTONUP( )
	ON_WM_LBUTTONUP( )
	ON_WM_PAINT( )
	ON_WM_TIMER( )
END_MESSAGE_MAP()


void MainWindow::passRerngModeComboChange ( )
{
	rearrangeControl.updateActive ( );
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
	infoBox ( "Congratulations!" );
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


void MainWindow::loadCameraCalSettings( MasterThreadInput* input )
{
	input->comm = &comm;
	ParameterSystem::generateKey( input->variables, input->settings.randomizeVariations, input->variableRangeInfo );
	input->constants = std::vector<std::vector<parameterType>>( input->variables.size( ) );
	for ( auto seqInc : range( input->variables.size( ) ) )
	{
		for ( auto& variable : input->variables[seqInc] )
		{
			if ( variable.constant )
			{
				input->constants[seqInc].push_back( variable );
			}
		}
	}
	input->seq.name = "CameraCal";
	input->seq.sequence.resize( 1 );
	input->seq.sequence[0].configuration = "Camera-Calibration";
	input->seq.sequence[0].categoryPath = CAMERA_CAL_ROUTINE_ADDRESS;
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
		cdc->SetBkColor( getRgbs( )["Solarized Base 04"] );
		UINT width = size.right - size.left, height = size.bottom - size.top;
		masterRepumpScope.refreshPlot( cdc, width, height, getBrushes( )["Solarized Base04"], getBrushes( )["Black"] );
		motScope.refreshPlot( cdc, width, height, getBrushes( )["Solarized Base04"], getBrushes( )["Black"] );
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
		menu.CheckMenuItem( ID_NIAWG_NIAWGISON, MF_UNCHECKED );
	}
	else
	{
		niawg.turnOn( );
		menu.CheckMenuItem( ID_NIAWG_NIAWGISON, MF_CHECKED );
	}
}


LRESULT MainWindow::onNoMotAlertMessage( WPARAM wp, LPARAM lp )
{
	try
	{
		if ( TheAndorWindow->wantsAutoPause ( ) )
		{
			masterThreadManager.pause ( );
			menu.CheckMenuItem ( ID_RUNMENU_PAUSE, MF_CHECKED );
			comm.sendColorBox ( System::Master, 'Y' );
		}
		auto asyncbeep = std::async ( std::launch::async, [] { Beep ( 1000, 100 ); } );
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
			menu.CheckMenuItem( ID_RUNMENU_PAUSE, MF_CHECKED );
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
	eMainWindowHwnd = GetSafeHwnd( );
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
		errBox( "ERROR: NIAWG failed to Initialize! Error: " + except.trace( ) );
	}
	try
	{
		niawg.setDefaultWaveforms( this );
		// but the default starts in the horizontal configuration, so switch back and start in this config.
		restartNiawgDefaults( );
	}
	catch ( Error& exception )
	{
		errBox( "ERROR: failed to start niawg default waveforms! Niawg gave the following error message: " 
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
		errBox( err.trace( ) );
	}
	/// initialize main window controls.
	comm.initialize( this, TheScriptingWindow, TheAndorWindow, TheAuxiliaryWindow );
	int id = 1000;
	POINT controlLocation = { 0,0 };
	mainStatus.initialize( controlLocation, this, id, 975, "EXPERIMENT STATUS", RGB( 100, 100, 250 ), tooltips, IDC_MAIN_STATUS_BUTTON );
	controlLocation = { 480, 0 };
	errorStatus.initialize( controlLocation, this, id, 480, "ERROR STATUS", RGB( 100, 0, 0 ), tooltips, 
							IDC_ERROR_STATUS_BUTTON );
	debugStatus.initialize( controlLocation, this, id, 480, "DEBUG STATUS", RGB( 13, 152, 186 ), tooltips, 
							IDC_DEBUG_STATUS_BUTTON );
	controlLocation = { 960, 0 };
	profile.initialize( controlLocation, this, id, tooltips );
	controlLocation = { 960, 175 };
	notes.initialize( controlLocation, this, id, tooltips);
	masterRepumpScope.initialize( controlLocation, 480, 250, this, getPlotPens( ), getPlotFont( ), getPlotBrushes(), 
								  "Master/Repump" );
	motScope.initialize( controlLocation, 480, 250, this, getPlotPens( ), getPlotFont( ), getPlotBrushes( ), "MOT" );
	controlLocation = { 1440, 50 };
	repetitionControl.initialize( controlLocation, tooltips, this, id );
	settings.initialize( id, controlLocation, this, tooltips, getRgbs() );
	rearrangeControl.initialize( id, controlLocation, this, tooltips );
	debugger.initialize( id, controlLocation, this, tooltips );
	texter.initialize( controlLocation, this, id, tooltips, mainRGBs );
	controlLocation = { 960, 910 };

	boxes.initialize( controlLocation, id, this, 960, tooltips );
	shortStatus.initialize( controlLocation, this, id, tooltips );
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
	std::vector<CDialog*> windows = {NULL, this, TheAndorWindow, TheScriptingWindow, TheAuxiliaryWindow };
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
	timingMsg = "Constructor to init-instance: " + str(t1) + "\nInitInstance to OnInitDialog:" + str(t2) + "\nOnInitDialog To Creating Windows:"
		+ str(t3) + "\nCreating Windows to showing windows:" + str(t4) + "\nShowing Windows to here:" + str(t5);
	errBox(timingMsg);
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
	if ( count == 0 )
	{
		// skip the high monitor.
		count++;
		return TRUE;
	}
	if ( count < 5 )
	{
		windows->at(count)->MoveWindow( lprcMonitor );
	}
	count++;
	return TRUE;
}

void MainWindow::handlePause()
{
	if (masterThreadManager.runningStatus())
	{
		if (masterThreadManager.getIsPaused())
		{
			// then it's currently paused, so unpause it.
			menu.CheckMenuItem(ID_RUNMENU_PAUSE, MF_UNCHECKED);
			masterThreadManager.unPause();
			comm.sendColorBox( System::Master, 'G' );
		}
		else
		{
			// then not paused so pause it.
			menu.CheckMenuItem(ID_RUNMENU_PAUSE, MF_CHECKED);
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
	settings.handleNewConfig( newFile );
	debugger.handleNewConfig( newFile );
	repetitionControl.handleNewConfig( newFile );
	rearrangeControl.handleNewConfig( newFile );
}


void MainWindow::handleSaveConfig(std::ofstream& saveFile)
{
	notes.handleSaveConfig(saveFile);
	settings.handleSaveConfig(saveFile);
	debugger.handleSaveConfig(saveFile);
	repetitionControl.handleSaveConfig(saveFile);
	rearrangeControl.handleSaveConfig( saveFile );
}


void MainWindow::handleOpeningConfig(std::ifstream& configFile, Version ver )
{
	try
	{
		notes.handleOpenConfig ( configFile, ver );
		settings.handleOpenConfig ( configFile, ver );
		debugger.handleOpenConfig ( configFile, ver );
		repetitionControl.handleOpenConfig ( configFile, ver );
		rearrangeControl.handleOpenConfig ( configFile, ver );
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
	settings.rearrange(cx, cy, getFonts());
	mainStatus.rearrange(cx, cy, getFonts());
	debugStatus.rearrange(cx, cy, getFonts());
	errorStatus.rearrange(cx, cy, getFonts());
	texter.rearrange(cx, cy, getFonts());
	shortStatus.rearrange(cx, cy, getFonts());
	boxes.rearrange( cx, cy, getFonts());
	repetitionControl.rearrange(cx, cy, getFonts());
	rearrangeControl.rearrange( cx, cy, getFonts( ) );
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


void MainWindow::OnCancel()
{
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
		status += "\tCode System is disabled! Enable in \"constants.h\"\r\n";
	}
	status += "MOT Scope:\n";
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
	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{
			int num = pWnd->GetDlgCtrlID();
			CBrush* ret = shortStatus.handleColor(pWnd, pDC, mainRGBs, mainBrushes);
			if (ret)
			{
				return *ret;
			}
			ret = boxes.handleColoring( num, pDC, mainBrushes, mainRGBs );
			if ( ret )
			{
				return *ret;
			}
			else
			{
				pDC->SetTextColor(mainRGBs["Solarized Base0"]);
				pDC->SetBkColor(mainRGBs["Medium Grey"]);
				return *mainBrushes["Medium Grey"];
			}
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor(mainRGBs["Solarized Blue"]);
			pDC->SetBkColor(mainRGBs["Solarized Base03"]);
			return *mainBrushes["Solarized Base03"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor(mainRGBs["Solarized Base0"]);
			pDC->SetBkColor(mainRGBs["Solarized Base02"]);
			return *mainBrushes["Solarized Base02"];
		}
		default:
		{
			return *mainBrushes["Solarized Base04"];
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


HANDLE MainWindow::startExperimentThread( MasterThreadInput* input, bool isTurnOnMot )
{
	return masterThreadManager.startExperimentThread(input);
}


void MainWindow::fillRedPgcTempProfile ( MasterThreadInput* input )
{
	// this function needs to be called before aux fills.
	input->seq.name = "pgcTemp";
	input->seq.sequence.resize ( 1 );
	input->profile.configuration = "Automated-PGC-Temperature-Measurement";
	input->profile.categoryPath = PGC_ROUTINES_ADDRESS;
	input->profile.parentFolderName = "PGC";
	input->seq.sequence[ 0 ] = input->profile;
}


void MainWindow::fillGreyPgcTempProfile ( MasterThreadInput* input )
{
	// this function needs to be called before aux fills.
	input->seq.name = "greyPgcTemp";
	input->seq.sequence.resize ( 1 );
	input->profile.configuration = "Automated-Grey-PGC-Temperature-Measurement";
	input->profile.categoryPath = PGC_ROUTINES_ADDRESS;
	input->profile.parentFolderName = "PGC";
	input->seq.sequence[ 0 ] = input->profile;
}


void MainWindow::fillMotTempProfile ( MasterThreadInput* input )
{
	// this function needs to be called before aux fills.
	input->seq.name = "motTemp";
	input->seq.sequence.resize ( 1 );
	input->profile.configuration = "Automated-MOT-Temperature-Measurement";
	input->profile.categoryPath = MOT_ROUTINES_ADDRESS;
	input->profile.parentFolderName = "MOT";
	input->seq.sequence[ 0 ] = input->profile;
}


void MainWindow::fillTempInput ( MasterThreadInput* input )
{
	input->comm = &comm;
	ParameterSystem::generateKey ( input->variables, input->settings.randomizeVariations, input->variableRangeInfo );
	input->constants = std::vector<std::vector<parameterType>> ( input->variables.size ( ) );
	for ( auto seqInc : range ( input->variables.size ( ) ) )
	{
		for ( auto& variable : input->variables[ seqInc ] )
		{
			if ( variable.constant )
			{
				input->constants[ seqInc ].push_back ( variable );

			}
		}
	}
	input->repetitionNumber = 5;
	// the mot procedure doesn't need the NIAWG at all.
	input->runNiawg = false;
	input->skipNext = NULL;
	input->rerngGuiForm = rearrangeControl.getParams ( );
	input->rerngGuiForm.active = false;
	input->expType = ExperimentType::LoadMot;

}


void MainWindow::fillMotInput( MasterThreadInput* input )
{
	input->comm = &comm;
	ParameterSystem::generateKey( input->variables, input->settings.randomizeVariations, input->variableRangeInfo );
	input->constants = std::vector<std::vector<parameterType>>( input->variables.size( ) );
	for (auto seqInc : range(input->variables.size()))
	{
		for ( auto& variable : input->variables[seqInc] )
		{
			if ( variable.constant )
			{
				input->constants[seqInc].push_back( variable );
			}
		}
	}
	input->profile.configuration = "Set MOT Settings";
	input->profile.categoryPath = MOT_ROUTINES_ADDRESS;
	input->profile.parentFolderName = "MOT";
	input->seq.name = "loadMot";
	input->seq.sequence.resize( 1 );
	input->seq.sequence[ 0 ] = input->seq.sequence[ 0 ] = input->profile;

	// the mot procedure doesn't need the NIAWG at all.
	input->runNiawg = false;
 	input->skipNext = NULL;
 	input->rerngGuiForm = rearrangeControl.getParams( ); 
 	input->rerngGuiForm.active = false; 
}


void MainWindow::fillMotSizeInput ( MasterThreadInput* input )
{
	input->comm = &comm;
	ParameterSystem::generateKey ( input->variables, input->settings.randomizeVariations, input->variableRangeInfo );
	input->constants = std::vector<std::vector<parameterType>> ( input->variables.size ( ) );
	for ( auto seqInc : range ( input->variables.size ( ) ) )
	{
		for ( auto& variable : input->variables[ seqInc ] )
		{
			if ( variable.constant )
			{
				input->constants[ seqInc ].push_back ( variable );
			}
		}
	}
	input->profile.configuration = "Mot_Size_Measurement";
	input->profile.categoryPath = MOT_ROUTINES_ADDRESS;
	input->profile.parentFolderName = "MOT";
	input->seq.name = "loadMot";
	input->seq.sequence.resize ( 1 );
	input->seq.sequence[ 0 ] = input->seq.sequence[ 0 ] = input->profile;

	// the mot procedure doesn't need the NIAWG at all.
	input->runNiawg = false;
	input->skipNext = NULL;
	input->rerngGuiForm = rearrangeControl.getParams ( );
	input->rerngGuiForm.active = false;
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


void MainWindow::fillMasterThreadSequence( MasterThreadInput* input )
{
	input->seq = profile.getSeqSettings( );
}


void MainWindow::fillMasterThreadInput(MasterThreadInput* input)
{
	input->python = &this->python;
	input->settings = settings.getOptions();
	input->repetitionNumber = getRepNumber();
	input->debugOptions = debugger.getOptions();
	input->profile = profile.getProfileSettings();
	input->seq = profile.getSeqSettings( );
	input->niawg = &niawg;
	input->comm = &comm;
	ParameterSystem::generateKey( input->variables, input->settings.randomizeVariations, input->variableRangeInfo );
	input->rerngGuiForm = rearrangeControl.getParams( );
}


void MainWindow::logParams(DataLogger* logger, MasterThreadInput* input)
{
	logger->logMasterParameters(input);
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
	return settings.getOptions();
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
	if ( statusMessage != "" )
	{
		errorStatus.addStatusText( statusMessage );
		auto asyncbeep = std::async( std::launch::async, [] { Beep( 1000, 100 ); } );
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


// I think I can delete this...
void MainWindow::onNormalFinishMessage()
{
	TheScriptingWindow->setIntensityDefault();
	std::string msgText = "Passively Outputting Default Waveform";
	setShortStatus(msgText);
	changeShortStatusColor("B");
	stopRearranger( );
	TheAndorWindow->wakeRearranger();
	comm.sendColorBox( System::Niawg, 'B' );
	try
	{
		niawg.restartDefault();
	}
	catch ( Error& except )
	{
		comm.sendError( "ERROR! The niawg finished normally, but upon restarting the default waveform, threw the "
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
	if ( autoF5_AfterFinish )
	{
		commonFunctions::handleCommonMessage ( ID_ACCELERATOR_F5, this, this, TheScriptingWindow, TheAndorWindow,
											   TheAuxiliaryWindow, TheBaslerWindow );
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


brushMap MainWindow::getBrushes()
{
	return mainBrushes;
}


rgbMap MainWindow::getRgbs()
{
	return mainRGBs;
}

