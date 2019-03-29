#include "stdafx.h"
#include "AuxiliaryWindow.h"
#include "DioSettingsDialog.h"
#include "AoSettingsDialog.h"
#include "TextPromptDialog.h"
#include "DioSystem.h"
#include "AndorWindow.h"
#include "MainWindow.h"
#include "Control.h"
#include "commonFunctions.h"
#include "openWithExplorer.h"
#include "saveWithExplorer.h"
#include "agilentStructures.h"
#include "Thrower.h"
#include "range.h"
#include <boost/lexical_cast.hpp>

AuxiliaryWindow::AuxiliaryWindow() : CDialog(), 
									 topBottomTek(TOP_BOTTOM_TEK_SAFEMODE, TOP_BOTTOM_TEK_USB_ADDRESS, "TOP_BOTTOM_TEKTRONICS_AFG"), 
									 eoAxialTek(EO_AXIAL_TEK_SAFEMODE, EO_AXIAL_TEK_USB_ADDRESS, "EO_AXIAL_TEKTRONICS_AFG" ),
									 agilents{ TOP_BOTTOM_AGILENT_SETTINGS, AXIAL_AGILENT_SETTINGS,
												FLASHING_AGILENT_SETTINGS, UWAVE_AGILENT_SETTINGS },
									ttlBoard( true, true, DIO_SAFEMODE ),
									aoSys( ANALOG_OUT_SAFEMODE ), configVariables( "CONFIG_PARAMETERS" ), 
									globalVariables("GLOBAL_PARAMETERS" )
{}


BOOL AuxiliaryWindow::handleAccelerators ( HACCEL m_haccel, LPMSG lpMsg )
{
	return globalVariables.handleAccelerators ( m_haccel, lpMsg );
}


IMPLEMENT_DYNAMIC( AuxiliaryWindow, CDialog )


BEGIN_MESSAGE_MAP( AuxiliaryWindow, CDialog )
	ON_WM_TIMER( )

	ON_WM_CTLCOLOR( )
	ON_WM_SIZE( )

	ON_COMMAND_RANGE( MENU_ID_RANGE_BEGIN, MENU_ID_RANGE_END, &AuxiliaryWindow::passCommonCommand )
	ON_COMMAND_RANGE( TTL_ID_BEGIN, TTL_ID_END, &AuxiliaryWindow::handleTtlPush )

	ON_COMMAND( TTL_HOLD, &handlTtlHoldPush )
	ON_COMMAND( ID_DAC_SET_BUTTON, &SetDacs )
	ON_COMMAND( IDC_ZERO_TTLS, &zeroTtls )
	ON_COMMAND( IDC_ZERO_DACS, &zeroDacs )
	ON_COMMAND( IDOK, &handleEnter )
	ON_COMMAND( TOP_BOTTOM_TEK_START, &passTopBottomTekProgram )
	ON_COMMAND(	EO_AXIAL_TEK_START, &passEoAxialTekProgram )
	ON_COMMAND( ID_GET_ANALOG_IN_VALUES, &GetAnalogInSnapshot )
	ON_COMMAND( IDC_SERVO_CAL, &runServos )
	ON_COMMAND( IDC_MACHINE_OPTIMIZE, &autoOptimize )

	ON_MESSAGE ( MainWindow::AutoServoMessage, &autoServo )
	ON_MESSAGE ( MainWindow::LogVoltsMessageID, &AuxiliaryWindow::onLogVoltsMessage )

	ON_COMMAND_RANGE( IDC_TOP_BOTTOM_CHANNEL1_BUTTON, IDC_UWAVE_PROGRAM, &AuxiliaryWindow::handleAgilentOptions )
	ON_COMMAND_RANGE( TOP_BOTTOM_TEK_START, EO_AXIAL_TEK_START+99, &AuxiliaryWindow::handleTektronicsButtons )
	
	ON_CONTROL_RANGE( CBN_SELENDOK, IDC_TOP_BOTTOM_AGILENT_COMBO, IDC_TOP_BOTTOM_AGILENT_COMBO, 
					  &AuxiliaryWindow::handleAgilentCombo )
	ON_CONTROL_RANGE( CBN_SELENDOK, IDC_AXIAL_AGILENT_COMBO, IDC_AXIAL_AGILENT_COMBO, 
					  &AuxiliaryWindow::handleAgilentCombo )
	ON_CONTROL_RANGE( CBN_SELENDOK, IDC_FLASHING_AGILENT_COMBO, IDC_FLASHING_AGILENT_COMBO, 
					  &AuxiliaryWindow::handleAgilentCombo )
	ON_CONTROL_RANGE( CBN_SELENDOK, IDC_UWAVE_AGILENT_COMBO, IDC_UWAVE_AGILENT_COMBO, 
					  &AuxiliaryWindow::handleAgilentCombo )
	

	ON_CONTROL_RANGE( EN_CHANGE, ID_DAC_FIRST_EDIT, (ID_DAC_FIRST_EDIT + 23), &AuxiliaryWindow::DacEditChange )
	ON_NOTIFY( LVN_COLUMNCLICK, IDC_CONFIG_VARS_LISTVIEW, &AuxiliaryWindow::ConfigVarsColumnClick )
	ON_NOTIFY( NM_DBLCLK, IDC_CONFIG_VARS_LISTVIEW, &AuxiliaryWindow::ConfigVarsDblClick )
	ON_NOTIFY( NM_RCLICK, IDC_CONFIG_VARS_LISTVIEW, &AuxiliaryWindow::ConfigVarsRClick )
	ON_NOTIFY( NM_DBLCLK, IDC_GLOBAL_VARS_LISTVIEW, &AuxiliaryWindow::GlobalVarDblClick )
	ON_NOTIFY( NM_RCLICK, IDC_GLOBAL_VARS_LISTVIEW, &AuxiliaryWindow::GlobalVarRClick )
	ON_NOTIFY ( NM_DBLCLK, IDC_MACHINE_OPTIMIZE_LISTVIEW, &AuxiliaryWindow::OptParamDblClick )
	ON_NOTIFY ( NM_RCLICK, IDC_MACHINE_OPTIMIZE_LISTVIEW, &AuxiliaryWindow::OptParamRClick )
	ON_NOTIFY ( NM_DBLCLK, IDC_SERVO_LISTVIEW, &AuxiliaryWindow::ServoDblClick )
	ON_NOTIFY ( NM_RCLICK, IDC_SERVO_LISTVIEW, &AuxiliaryWindow::ServoRClick )

	ON_NOTIFY_RANGE( NM_CUSTOMDRAW, IDC_GLOBAL_VARS_LISTVIEW, IDC_GLOBAL_VARS_LISTVIEW, &AuxiliaryWindow::drawVariables )
	ON_NOTIFY_RANGE( NM_CUSTOMDRAW, IDC_CONFIG_VARS_LISTVIEW, IDC_CONFIG_VARS_LISTVIEW, &AuxiliaryWindow::drawVariables )
	// catches all the edits in the tektronics controls
	ON_CONTROL_RANGE( EN_CHANGE, TOP_BOTTOM_TEK_START, EO_AXIAL_TEK_START + 99, &AuxiliaryWindow::invalidateSaved )

	ON_CONTROL_RANGE( EN_CHANGE, IDC_TOP_BOTTOM_EDIT, IDC_TOP_BOTTOM_EDIT, &AuxiliaryWindow::handleAgilentEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_FLASHING_EDIT, IDC_FLASHING_EDIT, &AuxiliaryWindow::handleAgilentEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_AXIAL_EDIT, IDC_AXIAL_EDIT, &AuxiliaryWindow::handleAgilentEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_UWAVE_EDIT, IDC_UWAVE_EDIT, &AuxiliaryWindow::handleAgilentEditChange )
	ON_WM_RBUTTONUP( )
	ON_WM_LBUTTONUP( )
	ON_WM_TIMER( )
	ON_WM_PAINT( )
END_MESSAGE_MAP()


void AuxiliaryWindow::invalidateSaved ( UINT id )
{
	mainWin->updateConfigurationSavedStatus ( false );
}


void AuxiliaryWindow::setMenuCheck ( UINT menuItem, UINT itemState )
{
	menu.CheckMenuItem ( menuItem, itemState );
}



void AuxiliaryWindow::passCommonCommand ( UINT id )
{
	try
	{
		commonFunctions::handleCommonMessage ( id, this, mainWin, scriptWin, camWin, this, basWin );
	}
	catch ( Error& err )
	{
		// catch any extra errors that handleCommonMessage doesn't explicitly handle.
		errBox ( err.what ( ) );
	}
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::OptParamDblClick ( NMHDR * pNotifyStruct, LRESULT * result )
{
	try
	{
		mainWin->updateConfigurationSavedStatus ( false );
		optimizer.handleListViewClick( );
	}
	catch ( Error& exception )
	{
		sendErr ( "Variables Double Click Handler : \n" + exception.trace ( ) + "\n" );
	}
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::ServoRClick ( NMHDR * pNotifyStruct, LRESULT * result )
{
	try
	{
		mainWin->updateConfigurationSavedStatus ( false );
		servos.deleteServo ( );
	}
	catch ( Error& err )
	{
		sendErr ( "Servo Right-Click Handler Failed.\n" + err.trace ( ) );
	}
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::ServoDblClick ( NMHDR * pNotifyStruct, LRESULT * result )
{
	try
	{
		mainWin->updateConfigurationSavedStatus ( false );
		servos.handleListViewClick ( );
	}
	catch ( Error& err )
	{
		sendErr ( "Servo Double-Click handler failed." + err.trace ( ) );
	}
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::OptParamRClick ( NMHDR * pNotifyStruct, LRESULT * result )
{
	try
	{
		mainWin->updateConfigurationSavedStatus ( false );
		optimizer.deleteParam( );
	}
	catch ( Error& exception )
	{
		sendErr ( "Variables Right Click Handler : " + exception.trace ( ) + "\r\n" );
	}
	mainWin->updateConfigurationSavedStatus ( false );
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::autoOptimize ( )
{
	try
	{
		auto res = promptBox ( "Start Machine optimization using the currently selected configuration parameters?", 
							   MB_YESNO );
		if ( res == IDNO )
		{
			return;
		}
		optimizer.reset ( );
		commonFunctions::handleCommonMessage ( ID_MACHINE_OPTIMIZATION, this, mainWin, scriptWin, camWin, this, basWin );
	}
	catch ( Error& err )
	{
		// catch any extra errors that handleCommonMessage doesn't explicitly handle.
		errBox ( err.what ( ) );
	}
}


void AuxiliaryWindow::updateOptimization ( ExperimentInput input )
{
	optimizer.verifyOptInput ( input );
	dataPoint resultValue = camWin->getMainAnalysisResult ( );
	auto params = optimizer.getOptParams ( );
	optimizer.updateParams ( input, resultValue, camWin->getLogger() );
	std::string msg = "Next Optimization: ";
	for ( auto& param : params )
	{
		msg += param->name + ": " + str ( param->currentValue ) + ";";
	}
	msg += "\r\n";
	sendStatus ( msg );
}

// MESSAGE MAP FUNCTION
LRESULT AuxiliaryWindow::autoServo(WPARAM w, LPARAM l )
{
	try
	{
		mainWin->updateConfigurationSavedStatus ( false );
		if ( servos.autoServo ( ) )
		{
			runServos ( );
		}
	}
	catch ( Error& err )
	{
		sendErr ( "Auto-Servo Failed.\n" + err.trace ( ) );
	}
	return TRUE;
}


// MESSAGE MAP FUNCTION
void AuxiliaryWindow::runServos( )
{
	try
	{
		mainWin->updateConfigurationSavedStatus ( false );
		sendStatus ( "Running Servos...\r\n" );
		servos.runAll( );
	}
	catch ( Error& err )
	{
		sendErr( "Running Servos failed.\n" + err.trace( ) );
	}
}

// MESSAGE MAP FUNCTION
LRESULT AuxiliaryWindow::onLogVoltsMessage( WPARAM wp, LPARAM lp )
{
	aiSys.refreshCurrentValues( );
	aiSys.refreshDisplays( );
	camWin->writeVolts( wp, aiSys.getCurrentValues() );

	return TRUE;
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::GetAnalogInSnapshot( )
{
	aiSys.refreshCurrentValues( );
	aiSys.refreshDisplays( );
}


void AuxiliaryWindow::OnPaint( )
{
	CDialog::OnPaint( );
	if ( !mainWin->masterIsRunning( ) )
	{
		CRect size;
		GetClientRect( &size );
		CDC* cdc = GetDC( );
		// for some reason I suddenly started needing to do this. I know that memDC redraws the background, but it used to 
		// work without this and I don't know what changed. I used to do:
		cdc->SetBkColor( _myRGBs["Main-Bkgd"] );
		long width = size.right - size.left, height = size.bottom - size.top;
		// each dc gets initialized with the rect for the corresponding plot. That way, each dc only overwrites the area 
		// for a single plot.
		for ( auto& ttlPlt : ttlPlots )
		{
			ttlPlt->setCurrentDims( width, height );
			ttlPlt->drawPlot ( cdc, _myBrushes[ "Main-Bkgd" ], _myBrushes[ "Interactable-Bkgd" ] );
		}
		for ( auto& dacPlt : aoPlots )
		{
			dacPlt->setCurrentDims( width, height );
			dacPlt->drawPlot ( cdc, _myBrushes[ "Main-Bkgd" ], _myBrushes[ "Interactable-Bkgd" ] );
		}
		ReleaseDC( cdc );
	}
}


std::vector<servoInfo> AuxiliaryWindow::getServoinfo ( )
{
	return servos.getServoInfo ( );
}


void AuxiliaryWindow::OnRButtonUp( UINT stuff, CPoint clickLocation )
{
	camWin->stopSound( );
}

void AuxiliaryWindow::OnLButtonUp( UINT stuff, CPoint clickLocation )
{
	camWin->stopSound( );
}


void AuxiliaryWindow::newAgilentScript( whichAg::agilentNames name)
{
	try
	{
		if ( !agilents[name].scriptingModeIsSelected( ) )
		{
			thrower ( "please set current agilent channel to scripting mode before attempting to create a script!" );
		}
		mainWin->updateConfigurationSavedStatus( false );
		agilents[name].checkSave( mainWin->getProfileSettings( ).categoryPath, mainWin->getRunInfo( ) );
		agilents[name].agilentScript.newScript( );
		agilents[name].agilentScript.updateScriptNameText( mainWin->getProfileSettings( ).categoryPath );
		agilents[name].agilentScript.colorEntireScript( getAllVariables( ), getTtlNames( ), getDacInfo ( ) );
	}
	catch ( Error& err )
	{
		sendErr( err.trace( ) );
	}

}


void AuxiliaryWindow::openAgilentScript( whichAg::agilentNames name, CWnd* parent)
{
	try
	{
		if ( !agilents[name].scriptingModeIsSelected( ) )
		{
			thrower ( "please set current agilent channel to scripting mode before attempting to open a script!" );
		}
		mainWin->updateConfigurationSavedStatus( false );		
		agilents[name].agilentScript.checkSave( mainWin->getProfileSettings( ).categoryPath, 
												mainWin->getRunInfo( ) );
		std::string openFileName = openWithExplorer( parent, AGILENT_SCRIPT_EXTENSION );
		agilents[name].agilentScript.openParentScript( openFileName, 
													   mainWin->getProfileSettings( ).categoryPath,
													   mainWin->getRunInfo( ) );
		agilents[name].agilentScript.updateScriptNameText( mainWin->getProfileSettings( ).categoryPath );
	}
	catch ( Error& err )
	{
		sendErr( err.trace( ) );
	}
}


void AuxiliaryWindow::updateAgilent( whichAg::agilentNames name )
{
	try
	{
		mainWin->updateConfigurationSavedStatus ( false );
		agilents[name].handleInput( mainWin->getProfileSettings( ).categoryPath,
									mainWin->getRunInfo( ) );
	}
	catch ( Error& )
	{
		throwNested( "Failed to update agilent." );
	}
}


void AuxiliaryWindow::saveAgilentScript( whichAg::agilentNames name )
{
	try
	{
		if ( !agilents[name].scriptingModeIsSelected( ) )
		{
			thrower ( "please set current agilent channel to scripting mode before attempting to save script!" );
		}
		mainWin->updateConfigurationSavedStatus( false );
		agilents[name].agilentScript.saveScript( mainWin->getProfileSettings( ).categoryPath,
												   mainWin->getRunInfo( ) );
		agilents[name].agilentScript.updateScriptNameText( mainWin->getProfileSettings( ).categoryPath );
	}
	catch ( Error& err )
	{
		sendErr( err.trace( ) );
	}
}


void AuxiliaryWindow::saveAgilentScriptAs( whichAg::agilentNames name, CWnd* parent )
{
	try
	{
		if ( !agilents[name].scriptingModeIsSelected( ) )
		{
			thrower ( "please set current agilent channel to scripting mode before attempting to save script!" );
		}
		mainWin->updateConfigurationSavedStatus( false );
		std::string extensionNoPeriod = agilents[name].agilentScript.getExtension( );
		if ( extensionNoPeriod.size( ) == 0 )
		{
			return;
		}
		extensionNoPeriod = extensionNoPeriod.substr( 1, extensionNoPeriod.size( ) );
		std::string newScriptAddress = saveWithExplorer( parent, extensionNoPeriod,
														 mainWin->getProfileSettings( ) );
		agilents[name].agilentScript.saveScriptAs( newScriptAddress, mainWin->getRunInfo( ) );
		agilents[name].agilentScript.updateScriptNameText( mainWin->getProfileSettings( ).categoryPath );
	}
	catch ( Error& err )
	{
		sendErr( err.trace( ) );
	}
}


void AuxiliaryWindow::OnTimer( UINT_PTR eventID )
{
	if ( eventID == 2 )
	{
		// don't query while experiment is running and getting querying between variations, this may cause a 
		// race condition.
		if ( aiSys.wantsContinuousQuery( ) && (!mainWin->masterIsRunning( ) 
												|| !aiSys.wantsQueryBetweenVariations() ) )
		{
			GetAnalogInSnapshot( );
		}
	}
	if ( eventID == 1 )
	{
		OnPaint( );
	}
	else
	{
		for ( auto& agilent : agilents )
		{
			agilent.agilentScript.handleTimerCall( getAllVariables( ), getTtlNames( ), getDacInfo ( ) );
		}
	}
}


Agilent& AuxiliaryWindow::whichAgilent( UINT id )
{
	if ( id >= IDC_TOP_BOTTOM_CHANNEL1_BUTTON && id <= IDC_TOP_BOTTOM_PROGRAM 
		 || id == IDC_TOP_BOTTOM_CALIBRATION_BUTTON )
	{
		return agilents[whichAg::TopBottom];
	}
	else if ( id >= IDC_AXIAL_CHANNEL1_BUTTON && id <= IDC_AXIAL_PROGRAM
			  || id == IDC_AXIAL_CALIBRATION_BUTTON )
	{
		return agilents[whichAg::Axial];
	}
	else if ( id >= IDC_FLASHING_CHANNEL1_BUTTON && id <= IDC_FLASHING_PROGRAM
			  || id == IDC_FLASHING_CALIBRATION_BUTTON )
	{
		return agilents[whichAg::Flashing];
	}
	else if ( id >= IDC_UWAVE_CHANNEL1_BUTTON && id <= IDC_UWAVE_PROGRAM
			  || id == IDC_UWAVE_CALIBRATION_BUTTON )
	{
		return agilents[whichAg::Microwave];
	}
	thrower ( "id seen in \"whichAgilent\" handler does not belong to any agilent!" );
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::handleAgilentEditChange( UINT id )
{
	Agilent& agilent = whichAgilent( id );
	try
	{
		mainWin->updateConfigurationSavedStatus ( false );
		agilent.agilentScript.handleEditChange( );
		SetTimer( SYNTAX_TIMER_ID, SYNTAX_TIMER_LENGTH, NULL );
	}
	catch ( Error& err )
	{
		sendErr( err.trace( ) );
	}
}


void AuxiliaryWindow::loadCameraCalSettings( MasterThreadInput* input )
{
	try
	{
		sendStatus( "Loading Camera-Cal Config...\r\n" );
		input->auxWin = this;
		input->quiet = true;
		input->ttls = &ttlBoard;
		input->aoSys = &aoSys;
		input->aiSys = &aiSys;
		input->globalControl = &globalVariables;
		input->comm = mainWin->getComm( );
		input->settings = { 0,0,0 };
		input->debugOptions = { 0, 0, 0, 0, 0, 0, 0, "", 0, 0, 0 };
		// don't get configuration variables. This calibration shouldn't depend on config variables.
		input->variables.clear( );
		input->variables.push_back( globalVariables.getEverything( ) );
		input->variableRangeInfo.clear ( );
		input->variableRangeInfo = configVariables.getRangeInfo ( );
		// Only do this once of course.
		input->repetitionNumber = 1;
		input->intensityAgilentNumber = -1;
		// Seems like I should just not initialize these or something...
		input->rsg = &RhodeSchwarzGenerator;
		input->topBottomTek = &topBottomTek;
		input->eoAxialTek = &eoAxialTek;
		input->runMaster = true;
		input->runNiawg = false;
		// Seems like I should just not initialize these or something...
		input->dacData = dacData;
		input->ttlData = ttlData;
	}
	catch ( Error& exception )
	{
		sendStatus( ": " + exception.trace( ) + " " + "\r\n" );
	}
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::passTopBottomTekProgram()
{
	try
	{
		topBottomTek.handleProgram();
		sendStatus( "Programmed Top/Bottom Tektronics Generator.\r\n" );
	}
	catch (Error& exception)
	{
		sendErr( "Error while programing top/Bottom Tektronics generator: " + exception.trace() + "\r\n" );
	}
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::passEoAxialTekProgram()
{
	try
	{
		eoAxialTek.handleProgram();
		sendStatus( "Programmed E.O.M / Axial Tektronics Generator.\r\n" );
	}
	catch (Error& exception)
	{
		sendErr( "Error while programing E.O.M. / Axial Tektronics generator: " + exception.trace() + "\r\n" );
	}
}


std::pair<UINT, UINT> AuxiliaryWindow::getTtlBoardSize()
{
	return ttlBoard.getTtlBoardSize();
}


void AuxiliaryWindow::handleNewConfig( std::ofstream& newFile )
{
	// order matters.
	configVariables.handleNewConfig( newFile );
	ttlBoard.handleNewConfig( newFile );
	aoSys.handleNewConfig( newFile );
	for ( auto& agilent : agilents )
	{
		agilent.handleNewConfig( newFile );
	}
	topBottomTek.handleNewConfig( newFile );
	eoAxialTek.handleNewConfig( newFile );
}


void AuxiliaryWindow::handleSaveConfig( std::ofstream& saveFile )
{
	// order matters.
	configVariables.handleSaveConfig( saveFile );
	ttlBoard.handleSaveConfig( saveFile );
	aoSys.handleSaveConfig( saveFile );
	for ( auto& agilent : agilents )
	{
		agilent.handleSavingConfig( saveFile, mainWin->getProfileSettings( ).categoryPath,
									mainWin->getRunInfo( ) );
	}
	topBottomTek.handleSaveConfig( saveFile );
	eoAxialTek.handleSaveConfig( saveFile );
}

void AuxiliaryWindow::handleOpeningConfig(std::ifstream& configFile, Version ver )
{
	try
	{
		ProfileSystem::standardOpenConfig ( configFile, configVariables.configDelim, &configVariables, Version ( "4.0" ) );
		ProfileSystem::standardOpenConfig ( configFile, "TTLS", &ttlBoard );
		ProfileSystem::standardOpenConfig ( configFile, "DACS", &aoSys );
		aoSys.updateEdits ( );
		ProfileSystem::standardOpenConfig ( configFile, agilents[ whichAg::TopBottom ].configDelim, &agilents[ whichAg::TopBottom ],
											Version ( "4.0" ) );
		agilents[ whichAg::TopBottom ].updateSettingsDisplay ( 1, mainWin->getProfileSettings ( ).categoryPath,
															   mainWin->getRunInfo ( ) );
		ProfileSystem::standardOpenConfig ( configFile, agilents[ whichAg::Axial ].configDelim, &agilents[ whichAg::Axial ],
											Version ( "4.0" ) );
		agilents[ whichAg::Axial ].updateSettingsDisplay ( 1, mainWin->getProfileSettings ( ).categoryPath,
														   mainWin->getRunInfo ( ) );
		ProfileSystem::standardOpenConfig ( configFile, agilents[ whichAg::Flashing ].configDelim, &agilents[ whichAg::Flashing ],
											Version ( "4.0" ) );
		agilents[ whichAg::Flashing ].updateSettingsDisplay ( 1, mainWin->getProfileSettings ( ).categoryPath,
															  mainWin->getRunInfo ( ) );
		if ( ver > Version ( "2.6" ) )
		{
			ProfileSystem::standardOpenConfig ( configFile, agilents[whichAg::Microwave].configDelim, &agilents[ whichAg::Microwave ],
												Version ( "4.0" ) );
			agilents[ whichAg::Microwave ].updateSettingsDisplay ( 1, mainWin->getProfileSettings ( ).categoryPath,
																   mainWin->getRunInfo ( ) );
		}
		ProfileSystem::standardOpenConfig ( configFile, topBottomTek.configDelim, &topBottomTek, Version ( "4.0" ) );
		ProfileSystem::standardOpenConfig ( configFile, eoAxialTek.configDelim, &eoAxialTek, Version ( "4.0" ) );
	}
	catch ( Error& )
	{
		throwNested ( "Auxiliary Window failed to read parameters from the configuration file." );
	}
}


UINT AuxiliaryWindow::getNumberOfDacs()
{
	return aoSys.getNumberOfDacs();
}


std::array<std::array<std::string, 16>, 4> AuxiliaryWindow::getTtlNames()
{
	return ttlBoard.getAllNames();
}


std::array<AoInfo, 24> AuxiliaryWindow::getDacInfo()
{
	return aoSys.getDacInfo();
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::drawVariables(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	if (id == IDC_GLOBAL_VARS_LISTVIEW)
	{
		globalVariables.handleDraw(pNMHDR, pResult );
	}
	else
	{
		configVariables.handleDraw(pNMHDR, pResult );
	}
}


// MESSAGE MAP FUNCTION
void AuxiliaryWindow::ConfigVarsDblClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	std::vector<Script*> scriptList;
	try
	{
		mainWin->updateConfigurationSavedStatus( false );
		configVariables.updateParameterInfo(scriptList, mainWin, this, &ttlBoard, &aoSys);
	}
	catch (Error& exception)
	{
		sendErr("Variables Double Click Handler : " + exception.trace() + "\r\n");
	}
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::ConfigVarsRClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	try
	{
		mainWin->updateConfigurationSavedStatus( false );
		configVariables.deleteVariable();
	}
	catch (Error& exception)
	{
		sendErr("Variables Right Click Handler : " + exception.trace() + "\r\n");
	}
}


std::vector<parameterType> AuxiliaryWindow::getAllVariables()
{
	std::vector<parameterType> vars = configVariables.getEverything();
	std::vector<parameterType> vars2 = globalVariables.getEverything();
	vars.insert(vars.end(), vars2.begin(), vars2.end());
	return vars;
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::GlobalVarDblClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	std::vector<Script*> scriptList;
	try
	{
		mainWin->updateConfigurationSavedStatus( false );
		globalVariables.updateParameterInfo(scriptList, mainWin, this, &ttlBoard, &aoSys);
	}
	catch (Error& exception)
	{
		sendErr("Global Variables Double Click Handler : " + exception.trace() + "\r\n");
	}
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::GlobalVarRClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	try
	{
		mainWin->updateConfigurationSavedStatus( false );
		globalVariables.deleteVariable();
	}
	catch (Error& exception)
	{
		sendErr("Global Variables Right Click Handler : " + exception.trace() + "\r\n");
	}
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::ConfigVarsColumnClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	try
	{
		mainWin->updateConfigurationSavedStatus( false );
		configVariables.handleColumnClick(pNotifyStruct, result);
	}
	catch (Error& exception)
	{
		sendErr("Handling config variable listview click.\n" + exception.trace() + "\r\n");
	}
}


void AuxiliaryWindow::clearVariables()
{
	mainWin->updateConfigurationSavedStatus ( false );
	configVariables.clearVariables();
}


void AuxiliaryWindow::addVariable(std::string name, bool constant, double value, int item)
{
	parameterType var;
	var.name = name;
	var.constant = constant;
	var.constantValue = value;
	var.ranges.push_back({ value, value+1 });
	try
	{
		mainWin->updateConfigurationSavedStatus ( false );
		configVariables.addConfigParameter ( var, item );
	}
	catch ( Error& )
	{
		throwNested ( "Failed to Add a variable." );
	}
}


void AuxiliaryWindow::loadFriends(MainWindow* mainWin_, ScriptingWindow* scriptWin_, AndorWindow* camWin_, 
								   BaslerWindow* basWin_)
{
	mainWin = mainWin_;
	scriptWin = scriptWin_;
	camWin = camWin_;
	basWin = basWin_;
}


void AuxiliaryWindow::passRoundToDac()
{
	mainWin->updateConfigurationSavedStatus ( false );
	aoSys.handleRoundToDac(mainWin);
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::handleTektronicsButtons(UINT id)
{
	topBottomTek.handleButtons ( id );
	eoAxialTek.handleButtons ( id );
	mainWin->updateConfigurationSavedStatus(false);
}


void AuxiliaryWindow::handleEnter()
{
	errBox("Hello, there!");
}


void AuxiliaryWindow::setVariablesActiveState(bool activeState)
{
	mainWin->updateConfigurationSavedStatus ( false );
	configVariables.setParameterControlActive(activeState);
}


UINT AuxiliaryWindow::getTotalVariationNumber()
{
	return configVariables.getTotalVariationNumber();
}


void AuxiliaryWindow::OnSize(UINT nType, int cx, int cy)
{
	SetRedraw( false );
	for ( auto& ttlPlt : ttlPlots )
	{
		ttlPlt->rearrange( cx, cy, getFonts( ) );
	}
	for ( auto& dacPlt : aoPlots )
	{
		dacPlt->rearrange( cx, cy, getFonts( ) );
	}
	topBottomTek.rearrange(cx, cy, getFonts());
	eoAxialTek.rearrange(cx, cy, getFonts());

	for ( auto& ag : agilents )
	{
		ag.rearrange( cx, cy, getFonts( ) );
	}

	RhodeSchwarzGenerator.rearrange(cx, cy, getFonts());

	ttlBoard.rearrange(cx, cy, getFonts());
	aoSys.rearrange(cx, cy, getFonts());
	aiSys.rearrange( cx, cy, getFonts( ) );

	configVariables.rearrange( cx, cy, getFonts( ) );
	globalVariables.rearrange( cx, cy, getFonts( ) );
	servos.rearrange( cx, cy, getFonts( ) );
	optimizer.rearrange ( cx, cy, getFonts ( ) );

	statusBox.rearrange( cx, cy, getFonts());
	SetRedraw();
	RedrawWindow();
}


fontMap AuxiliaryWindow::getFonts()
{
	return mainWin->getFonts();
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::handleAgilentOptions( UINT id )
{
	Agilent& agilent = whichAgilent( id );
	// zero the id.
	id -= IDC_TOP_BOTTOM_CHANNEL1_BUTTON;
	int agilentNum = id / 7;
	// figure out which box it was.	
	// call the correct function.
	if (id % 7 == 0)
	{
		// channel 1
		agilent.handleChannelPress( 1, mainWin->getProfileSettings().categoryPath, 
									 mainWin->getRunInfo() );
	}
	else if (id % 7 == 1)
	{
		// channel 2
		agilent.handleChannelPress( 2, mainWin->getProfileSettings().categoryPath, 
									 mainWin->getRunInfo() );
	}
	// sync is just a check, no handling needed.
	else if (id % 7 == 6)
	{
		try
		{
			agilent.handleInput( mainWin->getProfileSettings().categoryPath, mainWin->getRunInfo() );
			agilent.setAgilent();
			sendStatus( "Programmed Agilent " + agilent.configDelim + ".\r\n" );
		}
		catch (Error& err)
		{
			sendErr( "Error while programming agilent " + agilent.configDelim + ": " + err.trace() + "\r\n" );
		}
	}
	// else it's a combo or edit that must be handled separately, not in an ON_COMMAND handling.
	mainWin->updateConfigurationSavedStatus( false );
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::handleAgilentCombo(UINT id)
{
	Agilent& ag = whichAgilent( id );
	try
	{
		ag.handleInput( mainWin->getProfileSettings( ).categoryPath, mainWin->getRunInfo( ) );
		ag.handleModeCombo( );
		ag.updateSettingsDisplay( mainWin->getProfileSettings( ).categoryPath, mainWin->getRunInfo( ) );
	}
	catch ( Error& err )
	{
		sendErr( "ERROR: error while handling agilent combo change: " + err.trace( ) );
	}
}


void AuxiliaryWindow::sendErr(std::string msg)
{
	mainWin->getComm()->sendError(msg);
}


void AuxiliaryWindow::sendStatus(std::string msg)
{
	mainWin->getComm()->sendStatus(msg);
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::zeroDacs( )
{
	try
	{
		mainWin->updateConfigurationSavedStatus ( false );
		aoSys.zeroDacs( &ttlBoard );
		sendStatus( "Zero'd DACs.\r\n" );
	}
	catch ( Error& exception )
	{
		sendStatus( "Failed to Zero DACs!!!\r\n" );
		sendErr( exception.trace( ) );
	}
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::zeroTtls()
{
	try
	{
		mainWin->updateConfigurationSavedStatus ( false );
		ttlBoard.zeroBoard();
		sendStatus( "Zero'd TTLs.\r\n" );
	}
	catch (Error& exception)
	{
		sendStatus( "Failed to Zero TTLs!!!\r\n" );
		sendErr( exception.trace() );
	}
}

/// these three at the moment are identical. keeping for the moment in case I find I need to change something.
void AuxiliaryWindow::loadTempSettings ( MasterThreadInput* input )
{
	try
	{
		input->auxWin = this;
		input->quiet = true;
		input->ttls = &ttlBoard;
		input->aoSys = &aoSys;
		input->aiSys = &aiSys;
		input->globalControl = &globalVariables;
		input->comm = mainWin->getComm ( );
		input->settings = { 0,0,0 };
		input->debugOptions = { 0, 0, 0, 0, 0, 0, 0, "", 0, 0, 0 };
		/// variables.
		std::vector<std::vector<parameterType>> experimentVars;
		// load the variables. This little loop is for letting configuration variables overwrite the globals.
		// the config variables are loaded directly from the file.
		std::vector<parameterType> configVars;
		try
		{
			configVars = ParameterSystem::getConfigParamsFromFile ( input->seq.sequence[ 0 ].configFilePath ( ) );
		}
		catch ( Error& err )
		{
			throwNested ( "Error seen while loading mot temperature settings" );
		}
		std::vector<parameterType> globals = globalVariables.getEverything ( );
		experimentVars.push_back ( ParameterSystem::combineParametersForExperimentThread ( configVars, globals ) );
		globalVariables.setUsages ( { globals } );
		input->variableRangeInfo = ParameterSystem::getRangeInfoFromFile ( input->seq.sequence[ 0 ].configFilePath ( ) );
		input->variables = experimentVars;
		///
		// Only set it once, clearly.
		input->repetitionNumber = 1;
		input->rsg = &RhodeSchwarzGenerator;
		input->intensityAgilentNumber = -1;
		input->topBottomTek = &topBottomTek;
		input->eoAxialTek = &eoAxialTek;
		input->runMaster = true;
		input->runNiawg = false;
		input->dacData = dacData;
		input->ttlData = ttlData;
	}
	catch ( Error& exception )
	{
		sendStatus ( "Failed to load temp settings." + exception.trace ( ) + "\r\n" );
	}
}


void AuxiliaryWindow::loadMotSettings(MasterThreadInput* input)
{
	try
	{
		sendStatus("Loading MOT Configuration...\r\n" );
		input->auxWin = this;
		input->quiet = true;
		input->ttls = &ttlBoard;
		input->aoSys = &aoSys;
		input->aiSys = &aiSys;
		input->globalControl = &globalVariables;
		input->comm = mainWin->getComm();
		input->settings = { 0,0,0 };
		input->debugOptions = { 0, 0, 0, 0, 0, 0, 0, "", 0, 0, 0 };
		// don't get configuration variables. The MOT shouldn't depend on config variables.
		input->variables.clear( );
		input->variables.push_back(globalVariables.getEverything());
		input->variableRangeInfo.clear ( ); 
		input->variableRangeInfo.push_back ( configVariables.defaultRangeInfo );
		input->variableRangeInfo[ 0 ].variations = 1;
		// Only set it once, clearly.
		input->repetitionNumber = 1;
		input->rsg = &RhodeSchwarzGenerator;
		input->intensityAgilentNumber = -1;
		input->topBottomTek = &topBottomTek;
		input->eoAxialTek = &eoAxialTek;
		input->runMaster = true;
		input->runNiawg = false;
		input->dacData = dacData;
		input->ttlData = ttlData;
	}
	catch (Error& exception)
	{
		sendStatus("Failed to load mot settings.\n" + exception.trace() + "\r\n" );
	}
}

// MESSAGE MAP FUNCTION
// Gets called after alt-f4 or X button is pressed.
void AuxiliaryWindow::OnCancel()
{
	passCommonCommand(ID_FILE_MY_EXIT);
}


void AuxiliaryWindow::fillMasterThreadInput( MasterThreadInput* input )
{
	try
	{
		input->auxWin = this;
		input->ttls = &ttlBoard;
		input->aoSys = &aoSys;
		input->aiSys = &aiSys;
		input->globalControl = &globalVariables;
		input->dacData = dacData;
		input->ttlData = ttlData;
		/// variables.
		std::vector<std::vector<parameterType>> experimentVars;
		for ( auto seqFile : input->seq.sequence )
		{
			// load the variables. This little loop is for letting configuration variables overwrite the globals.
			// the config variables are loaded directly from the file.
			std::vector<parameterType> configVars = ParameterSystem::getConfigParamsFromFile ( seqFile.configFilePath ( ) );
			std::vector<parameterType> globals = globalVariables.getEverything ( );
			experimentVars.push_back ( ParameterSystem::combineParametersForExperimentThread ( configVars, globals ) );
			globalVariables.setUsages ( { globals } );
		}
		input->variableRangeInfo.clear ( );
		input->variableRangeInfo = configVariables.getRangeInfo ( );
		input->variables = experimentVars;
		input->constants.resize ( input->variables.size ( ) );
		// it's important to do this after the key is generated so that the constants have their values.
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
		input->rsg = &RhodeSchwarzGenerator;
		for ( auto& agilent : agilents )
		{
			input->agilents.push_back ( &agilent );
		}
		topBottomTek.getTekSettings ( );
		eoAxialTek.getTekSettings ( );
		input->topBottomTek = &topBottomTek;
		input->eoAxialTek = &eoAxialTek;
	}
	catch ( Error& )
	{
		throwNested ( "Auxiliary window failed to fill master thread input." );
	}
}


void AuxiliaryWindow::changeBoxColor(systemInfo<char> colors)
{
	statusBox.changeColor(colors);
}


void AuxiliaryWindow::handleAbort()
{
	ttlBoard.unshadeTtls();
	aoSys.unshadeDacs();
	if ( optimizer.isInMiddleOfOptimizing ( ) )
	{
		if ( promptBox ( "Save Optimization Data?", MB_YESNO ) == IDYES )
		{
			optimizer.onFinOpt ( );
		}		
	}
}


void AuxiliaryWindow::handleMasterConfigSave(std::stringstream& configStream)
{
	// save info
	/// ttls
	for (UINT ttlRowInc = 0; ttlRowInc < ttlBoard.getTtlBoardSize().first; ttlRowInc++)
	{
		for (UINT ttlNumberInc = 0; ttlNumberInc < ttlBoard.getTtlBoardSize().second; ttlNumberInc++)
		{
			std::string name = ttlBoard.getName(ttlRowInc, ttlNumberInc);
			if (name == "")
			{
				// then no name has been set, so create the default name.
				switch (ttlRowInc)
				{
					case 0:
						name = "A";
						break;
					case 1:
						name = "B";
						break;
					case 2:
						name = "C";
						break;
					case 3:
						name = "D";
						break;
				}
				name += str(ttlNumberInc);
			}
			configStream << name << "\n";

			configStream << ttlBoard.getDefaultTtl(ttlRowInc, ttlNumberInc) << "\n";
		}
	}
	// DAC Names
	for (UINT dacInc = 0; dacInc < aoSys.getNumberOfDacs(); dacInc++)
	{
		std::string name = aoSys.getName(dacInc);
		std::pair<double, double> minMax = aoSys.getDacRange(dacInc);
		if (name == "")
		{
			// then the name hasn't been set, so create the default name
			name = "Dac" + str(dacInc);
		}
		configStream << name << "\n";
		configStream << minMax.first << " - " << minMax.second << "\n";
		configStream << aoSys.getDefaultValue(dacInc) << "\n";
		configStream << aoSys.getNote ( dacInc ) << "\n";
	}

	// Number of Variables
	configStream << globalVariables.getCurrentNumberOfVariables() << "\n";
	/// Variables
	for (UINT varInc : range( globalVariables.getCurrentNumberOfVariables() ) )
	{
		parameterType info = globalVariables.getVariableInfo(varInc);
		configStream << info.name << " ";
		configStream << info.constantValue << "\n";
		// all globals are constants, no need to output anything else.
	}
	servos.handleSaveMasterConfig( configStream );
}


void AuxiliaryWindow::handleMasterConfigOpen(std::stringstream& configStream, Version version)
{
	ttlBoard.resetTtlEvents();
	ttlBoard.prepareForce();
	aoSys.resetDacEvents();
	aoSys.prepareForce();
	for (UINT ttlRowInc : range( ttlBoard.getTtlBoardSize().first))
	{
		for (UINT ttlNumberInc : range( ttlBoard.getTtlBoardSize().second ) )
		{
			std::string name;
			std::string defaultStatusString;
			bool defaultStatus;
			configStream >> name >> defaultStatusString;
			try
			{
				// In file the booleans are stored as "0" or "1".
				defaultStatus = boost::lexical_cast<int>(defaultStatusString);
			}
			catch ( boost::bad_lexical_cast&)
			{
				throwNested("Failed to load one of the default ttl values!");
			}

			ttlBoard.setName(ttlRowInc, ttlNumberInc, name, toolTips, this);
			ttlBoard.forceTtl(ttlRowInc, ttlNumberInc, defaultStatus);
			ttlBoard.updateDefaultTtl(ttlRowInc, ttlNumberInc, defaultStatus);
		}
	}
	// getting aoSys.
	for (UINT dacInc : range( aoSys.getNumberOfDacs()) )
	{
		std::string name, defaultValueString, minString, maxString;
		double defaultValue, min, max;

		configStream >> name;
		if (version >= Version("1.2"))
		{
			std::string trash;
			configStream >> minString >> trash;
			if (trash != "-")
			{
				thrower ( str("Expected \"-\" in master config file between min and max values for variable ") 
							   + name + ", dac"  + str(dacInc) + ".");
			}
			configStream >> maxString;
		}
		configStream >> defaultValueString;
		try
		{
			defaultValue = boost::lexical_cast<double>(defaultValueString);
			if (version >= Version( "1.2"))
			{
				min = boost::lexical_cast<double>(minString);
				max = boost::lexical_cast<double>(maxString);
			}
			else
			{
				min = -10;
				max = 10;
			}
		}
		catch ( boost::bad_lexical_cast&)
		{
			throwNested("Failed to load one of the default DAC values!");
		}

		std::string noteString = "";

		if ( version >= Version ( "2.3" ) )
		{
			std::string trash;
			configStream >> noteString;
		}

		aoSys.setName(dacInc, name, toolTips, this);
		aoSys.setNote ( dacInc, noteString, toolTips, this );
		aoSys.setMinMax(dacInc, min, max);
		aoSys.prepareDacForceChange(dacInc, defaultValue, &ttlBoard);
		aoSys.updateEdits( );
		aoSys.setDefaultValue(dacInc, defaultValue);
	}
	// variables.
	if (version >= Version( "1.1" ))
	{
		int varNum;
		configStream >> varNum;

		if (varNum < 0 || varNum > 1000)
		{
			int answer = promptBox("ERROR: variable number retrieved from file appears suspicious. The number is "
									+ str(varNum) + ". Is this accurate?", MB_YESNO);
			if (answer == IDNO)
			{
				// don't try to load anything.
				varNum = 0;
				return;
			}
		}
		// Number of Variables
		globalVariables.clearVariables();
		for (int varInc = 0; varInc < varNum; varInc++)
		{
			parameterType tempVar;
			tempVar.constant = true;
			tempVar.overwritten = false;
			tempVar.active = false;
			double value;
			configStream >> tempVar.name >> value;
			tempVar.constantValue = value;
			tempVar.ranges.push_back ( { value, value } );
			globalVariables.addGlobalParameter(tempVar, varInc);
		}
	}
	parameterType tempVar;
	tempVar.name = "";
	globalVariables.addGlobalParameter(tempVar, -1);
	//
	servos.handleOpenMasterConfig( configStream, version );
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::SetDacs()
{
	// have the dac values change
	sendStatus( "----------------------\r\nSetting Dacs... " );
	try
	{
		mainWin->updateConfigurationSavedStatus ( false );
		aoSys.forceDacs( &ttlBoard );
		sendStatus( "Finished Setting Dacs.\r\n" );
	}
	catch (Error& exception)
	{
		errBox( exception.trace() );
		sendStatus( ": " + exception.trace() + "\r\n" );
		sendErr( exception.trace() );
	}
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::DacEditChange(UINT id)
{
	try
	{
		mainWin->updateConfigurationSavedStatus( false );
		aoSys.handleEditChange(id - ID_DAC_FIRST_EDIT);
	}
	catch (Error& err)
	{
		sendErr(err.trace());
	}
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::handleTtlPush(UINT id)
{
	try
	{
		mainWin->updateConfigurationSavedStatus( false );
		ttlBoard.handleTTLPress( id );
	}
	catch (Error& exception)
	{
		sendErr( "TTL Press Handler Failed.\n" + exception.trace() + "\r\n" );
	}
}

// MESSAGE MAP FUNCTION
void AuxiliaryWindow::handlTtlHoldPush()
{
	try
	{
		mainWin->updateConfigurationSavedStatus ( false );
		ttlBoard.handleHoldPress();
	}
	catch (Error& exception)
	{
		sendErr( "TTL Hold Handler Failed: " + exception.trace() + "\r\n" );
	}
}


void AuxiliaryWindow::ViewOrChangeTTLNames()
{
	mainWin->updateConfigurationSavedStatus( false );
	ttlInputStruct input;
	input.ttls = &ttlBoard;
	input.toolTips = toolTips;
	TtlSettingsDialog dialog(&input, IDD_VIEW_AND_CHANGE_TTL_NAMES);
	dialog.DoModal();
}


void AuxiliaryWindow::ViewOrChangeDACNames()
{
	mainWin->updateConfigurationSavedStatus( false );
	aoInputStruct input;
	input.aoSys = &aoSys;
	input.toolTips = toolTips;
	AoSettingsDialog dialog(&input, IDD_VIEW_AND_CHANGE_DAC_NAMES);
	dialog.DoModal();
}


void AuxiliaryWindow::Exit()
{
	EndDialog(0);
}


HBRUSH AuxiliaryWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH result;
	for ( auto& ag : agilents )
	{
		result = ag.handleColorMessage( pWnd, pDC );
		if ( result != NULL )
		{
			return result;
		}
	}
	result = ttlBoard.handleColorMessage(pWnd, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = aoSys.handleColorMessage(pWnd, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = topBottomTek.handleColorMessage(pWnd, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = eoAxialTek.handleColorMessage(pWnd, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = *statusBox.handleColoring(pWnd->GetDlgCtrlID(), pDC );
	if (result != NULL)
	{
		return result;
	}

	// default colors
	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{
			pDC->SetTextColor(_myRGBs["Text"]);
			pDC->SetBkColor( _myRGBs["Static-Bkgd"]);
			return *_myBrushes["Static-Bkgd"];
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor( _myRGBs["AuxWin-Text"]);
			pDC->SetBkColor( _myRGBs["Interactable-Bkgd"]);
			return *_myBrushes["Interactable-Bkgd"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor( _myRGBs["AuxWin-Text"]);
			pDC->SetBkColor( _myRGBs["Interactable-Bkgd"]);
			return *_myBrushes["Interactable-Bkgd"];
		}
		default:
			return *_myBrushes["Main-Bkgd"];
	}
}


BOOL AuxiliaryWindow::PreTranslateMessage(MSG* pMsg)
{
	for (UINT toolTipInc = 0; toolTipInc < toolTips.size(); toolTipInc++)
	{
		toolTips[toolTipInc]->RelayEvent(pMsg);
	}
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN)
		{
			auto win = GetFocus ( );
			bool up = pMsg->wParam == VK_UP;
			if ( aoSys.handleArrow ( win, up ) )
			{
				mainWin->updateConfigurationSavedStatus ( false );
				try
				{
					aoSys.forceDacs ( &ttlBoard );
				}
				catch ( Error& err )
				{
					sendErr ( "Failed to change dacs - caught during quick change handling. " + err.trace ( ) + "\r\n" );
				}
				return TRUE;
			}			
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}


BOOL AuxiliaryWindow::OnInitDialog()
{
	SetWindowText ( "Auxiliary Window" );
	// don't redraw until the first OnSize.
	SetRedraw( false );

	int id = 4000;
	POINT controlLocation{ 0, 0 };
	try
	{
		statusBox.initialize( controlLocation, id, this, 480, toolTips );
		ttlBoard.initialize( controlLocation, toolTips, this, id );
		aoSys.initialize( controlLocation, toolTips, this, id );
		aiSys.initialize( controlLocation, this, id );
		topBottomTek.initialize( controlLocation, this, id, "Top-Bottom-Tek", "Top", "Bottom", 480, TOP_BOTTOM_TEK_START);
		eoAxialTek.initialize( controlLocation, this, id, "EO / Axial", "EO", "Axial", 480, EO_AXIAL_TEK_START );
		RhodeSchwarzGenerator.initialize( controlLocation, toolTips, this, id );
		controlLocation = POINT{ 480, 0 };
		
		agilents[whichAg::TopBottom].initialize( controlLocation, toolTips, this, id, "Top-Bottom-Agilent", 100,
												 _myRGBs["Interactable-Bkgd"] );
		agilents[whichAg::Axial].initialize( controlLocation, toolTips, this, id, "Microwave-Axial-Agilent", 100,
											 _myRGBs["Interactable-Bkgd"] );
		agilents[whichAg::Flashing].initialize( controlLocation, toolTips, this, id, "Flashing-Agilent", 100, 
												_myRGBs["Interactable-Bkgd"] );
		agilents[whichAg::Microwave].initialize( controlLocation, toolTips, this, id, "Microwave-Agilent", 100,
												 _myRGBs["Interactable-Bkgd"] );
		controlLocation = POINT{ 1440, 0 };
		globalVariables.initialize( controlLocation, toolTips, this, id, "GLOBAL PARAMETERS",
									IDC_GLOBAL_VARS_LISTVIEW, ParameterSysType::global );
		configVariables.initialize( controlLocation, toolTips, this, id, "CONFIGURATION PARAMETERS",
									IDC_CONFIG_VARS_LISTVIEW, ParameterSysType::config );
		configVariables.setParameterControlActive( false );

		servos.initialize( controlLocation, toolTips, this, id, &aiSys, &aoSys, &ttlBoard, &globalVariables );
		optimizer.initialize ( controlLocation, toolTips, this, id );
		controlLocation = POINT{ 960, 0 };
		aoPlots.resize( NUM_DAC_PLTS );
		dacData.resize( NUM_DAC_PLTS );
		UINT linesPerDacPlot = 24 / dacData.size( );
		// initialize data structures.
		for ( auto& dacPlotData : dacData )
		{
			dacPlotData = std::vector<pPlotDataVec>( linesPerDacPlot );
			for ( auto& d : dacPlotData )
			{
				d = pPlotDataVec( new plotDataVec( 100, { 0,0,0 } ) );
			}
		}
		// initialize plot controls.
		UINT dacPlotSize = 500 / NUM_DAC_PLTS;
		for ( auto dacPltCount : range(aoPlots.size()))
		{
			std::string titleTxt;
			switch ( dacPltCount )
			{
			case 0:
				titleTxt = "DACs: 0-7";
				break;
			case 1:
				titleTxt = "DACs: 8-15";
				break;
			case 2:
				titleTxt = "DACs: 16-23";
				break;
			}
			aoPlots[ dacPltCount ] = new PlotCtrl ( dacData[ dacPltCount ], plotStyle::DacPlot, mainWin->getBrightPlotPens ( ),
													mainWin->getPlotFont ( ), mainWin->getBrightPlotBrushes ( ), 
													std::vector<int>(), titleTxt );
			aoPlots[dacPltCount]->init( controlLocation, 480, dacPlotSize, this );
			controlLocation.y += dacPlotSize;
		}
		// ttl plots are similar to aoSys.
		ttlPlots.resize( NUM_TTL_PLTS );
		ttlData.resize( NUM_TTL_PLTS );
		UINT linesPerTtlPlot =  64 / ttlData.size( );
		for ( auto& ttlPlotData : ttlData )
		{
			ttlPlotData = std::vector<pPlotDataVec>( linesPerTtlPlot );
			for ( auto& d : ttlPlotData )
			{
				d = pPlotDataVec( new plotDataVec( 100, { 0,0,0 } ) );
			}
		}
		UINT ttlPlotSize = 500 / NUM_TTL_PLTS;
		for ( auto ttlPltCount : range( ttlPlots.size( ) ) )
		{
			// currently assuming 4 ttl plots...
			std::string titleTxt;
			switch ( ttlPltCount )
			{
			case 0:
				titleTxt = "Ttls: Row A";
				break;
			case 1:
				titleTxt = "Ttls: Row B";
				break;
			case 2:
				titleTxt = "Ttls: Row C";
				break;
			case 3:
				titleTxt = "Ttls: Row D";
				break;
			}
			ttlPlots[ttlPltCount] = new PlotCtrl( ttlData[ttlPltCount], plotStyle::TtlPlot, mainWin->getBrightPlotPens( ),
												  mainWin->getPlotFont( ), mainWin->getBrightPlotBrushes( ), 
												  std::vector<int>(), titleTxt );
			ttlPlots[ttlPltCount]->init( controlLocation, 480, ttlPlotSize, this );
			controlLocation.y += ttlPlotSize;
		}
	}
	catch (Error& exeption)
	{
		errBox( exeption.trace() );
	}
	SetTimer( 1, 10000, NULL );
	SetTimer( 2, 1000, NULL );

	menu.LoadMenu( IDR_MAIN_MENU );
	SetMenu( &menu );
	return TRUE;
}


std::string AuxiliaryWindow::getOtherSystemStatusMsg( )
{
	// controls are done. Report the initialization defaultStatus...
	std::string msg;
	msg += "DIO System:\n";
	if ( !ttlBoard.getViewpointSafemode() )
	{
		msg += "\tCode System is active!\n";
		msg += "\t" + ttlBoard.getSystemInfo( ) + "\n";
	}
	else
	{
		msg += "\tCode System is disabled! Enable in \"constants.h\"\n";
	}
	msg += "Analog Out System:\n";
	if ( !ANALOG_OUT_SAFEMODE )
	{
		msg += "\tCode System is Active!\n";
		msg += "\t" + aoSys.getSystemInfo( ) + "\n";
	}
	else
	{
		msg += "\tCode System is disabled! Enable in \"constants.h\"\n";
	}
	msg += "Analog In System:\n";
	if ( !ANALOG_IN_SAFEMODE )
	{
		msg += "\tCode System is Active!\n";
		msg += "\t" + aiSys.getSystemStatus( ) + "\n";
	}
	else
	{
		msg += "\tCode System is disabled! Enable in \"constants.h\"\n";
	}
	return msg;
}


std::string AuxiliaryWindow::getVisaDeviceStatus( )
{
	std::string msg;
	msg += "----------------------------------------------------------------------------------- VISA Devices\n";
	msg += "Tektronics 1:\n\t" + topBottomTek.queryIdentity( );
	msg += "Tektronics 2:\n\t" + eoAxialTek.queryIdentity( );
	for ( auto& agilent : agilents )
	{
		msg += agilent.configDelim + ":\n\t" + agilent.getDeviceIdentity( );
	}
	return msg;
}


std::string AuxiliaryWindow::getGpibDeviceStatus( )
{
	std::string msg;
	msg += "----------------------------------------------------------------------------------- GPIB Devices:\n";
	msg += "RSG:\n";
	if ( RSG_SAFEMODE )
	{
		msg += "\tCode System is Active!\n";
		msg += "\t" + RhodeSchwarzGenerator.getIdentity( );
	}
	else
	{
		msg += "\tCode System is disabled! Enable in \"constants.h\"";
	}
	return msg;
}


