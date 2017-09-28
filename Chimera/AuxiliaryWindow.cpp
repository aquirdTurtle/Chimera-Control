#include "stdafx.h"
#include "AuxiliaryWindow.h"
#include "Control.h"
#include "DioSettingsDialog.h"
#include "DacSettingsDialog.h"
#include "TextPromptDialog.h"
#include "DioSystem.h"
#include "explorerOpen.h"
#include "commonFunctions.h"
#include "openWithExplorer.h"
#include "saveTextFileFromEdit.h"

AuxiliaryWindow::AuxiliaryWindow() : CDialog(), 
									 topBottomAgilent(TOP_BOTTOM_AGILENT_SAFEMODE, TOP_BOTTOM_AGILENT_USB_ADDRESS),								
									 uWaveAxialAgilent(UWAVE_AXIAL_AGILENT_SAFEMODE, UWAVE_AXIAL_AGILENT_USB_ADDRESS), 
									 flashingAgilent(FLASHING_SAFEMODE, FLASHING_AGILENT_USB_ADDRESS), 
									 topBottomTek(TOP_BOTTOM_TEK_SAFEMODE, TOP_BOTTOM_TEK_USB_ADDRESS), 
									 eoAxialTek(EO_AXIAL_TEK_SAFEMODE, EO_AXIAL_TEK_USB_ADDRESS)
{ 
}

IMPLEMENT_DYNAMIC( AuxiliaryWindow, CDialog )

BEGIN_MESSAGE_MAP( AuxiliaryWindow, CDialog )
	
	ON_WM_TIMER()

	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	
	ON_COMMAND_RANGE( MENU_ID_RANGE_BEGIN, MENU_ID_RANGE_END, &AuxiliaryWindow::passCommonCommand)
	ON_COMMAND_RANGE( TTL_ID_BEGIN, TTL_ID_END, &AuxiliaryWindow::handleTtlPush )

	ON_COMMAND( TTL_HOLD, &handlTtlHoldPush )
	ON_COMMAND( ID_DAC_SET_BUTTON, &SetDacs )
	ON_COMMAND( IDC_ZERO_TTLS, &zeroTtls )
	ON_COMMAND( IDC_ZERO_DACS, &zeroDacs )
	ON_COMMAND( IDOK, &handleEnter)
	ON_COMMAND( TOP_BOTTOM_PROGRAM, &passTopBottomTekProgram )
	ON_COMMAND( EO_AXIAL_PROGRAM, &passEoAxialTekProgram )
	
	ON_COMMAND_RANGE( IDC_TOP_BOTTOM_CHANNEL1_BUTTON, IDC_FLASHING_PROGRAM, &AuxiliaryWindow::handleAgilentOptions )
	ON_COMMAND_RANGE( TOP_ON_OFF, AXIAL_FSK, &AuxiliaryWindow::handleTektronicsButtons )	
	ON_CBN_SELENDOK( IDC_TOP_BOTTOM_AGILENT_COMBO, &AuxiliaryWindow::handleTopBottomAgilentCombo )
 	ON_CBN_SELENDOK( IDC_AXIAL_UWAVE_AGILENT_COMBO, &AuxiliaryWindow::handleAxialUWaveAgilentCombo )
 	ON_CBN_SELENDOK( IDC_FLASHING_AGILENT_COMBO, &AuxiliaryWindow::handleFlashingAgilentCombo )	

	ON_CONTROL_RANGE( EN_CHANGE, ID_DAC_FIRST_EDIT, (ID_DAC_FIRST_EDIT + 23), &AuxiliaryWindow::DacEditChange )
	ON_NOTIFY( LVN_COLUMNCLICK, IDC_CONFIG_VARS_LISTVIEW, &AuxiliaryWindow::ConfigVarsColumnClick )
	ON_NOTIFY( NM_DBLCLK, IDC_CONFIG_VARS_LISTVIEW, &AuxiliaryWindow::ConfigVarsDblClick )
	ON_NOTIFY( NM_RCLICK, IDC_CONFIG_VARS_LISTVIEW, &AuxiliaryWindow::ConfigVarsRClick )

	ON_NOTIFY( NM_DBLCLK, IDC_GLOBAL_VARS_LISTVIEW, &AuxiliaryWindow::GlobalVarDblClick )
	ON_NOTIFY( NM_RCLICK, IDC_GLOBAL_VARS_LISTVIEW, &AuxiliaryWindow::GlobalVarRClick )
	ON_NOTIFY_RANGE( NM_CUSTOMDRAW, IDC_GLOBAL_VARS_LISTVIEW, IDC_GLOBAL_VARS_LISTVIEW, &AuxiliaryWindow::drawVariables )
	ON_NOTIFY_RANGE( NM_CUSTOMDRAW, IDC_CONFIG_VARS_LISTVIEW, IDC_CONFIG_VARS_LISTVIEW, &AuxiliaryWindow::drawVariables )

	ON_EN_CHANGE( IDC_TOP_BOTTOM_EDIT, &AuxiliaryWindow::handleTopBottomEditChange )
	ON_EN_CHANGE( IDC_FLASHING_EDIT, &AuxiliaryWindow::handleFlashingEditChange )
	ON_EN_CHANGE( IDC_AXIAL_UWAVE_EDIT, &AuxiliaryWindow::handleAxialUwaveEditChange )

	//ON_COMMAND( ID_ACCELERATOR40121, &AuxiliaryWindow::OnAccelerator40121 )
END_MESSAGE_MAP()


void AuxiliaryWindow::newTopBottomAgilentScript()
{
	try
	{
		topBottomAgilent.checkSave( mainWindowFriend->getProfileSettings().categoryPath, mainWindowFriend->getRunInfo() );
		topBottomAgilent.agilentScript.newScript( );
		mainWindowFriend->updateConfigurationSavedStatus( false );
		topBottomAgilent.agilentScript.updateScriptNameText( mainWindowFriend->getProfileSettings().categoryPath );
		topBottomAgilent.agilentScript.colorEntireScript( getAllVariables(), mainWindowFriend->getRgbs(),
														  getTtlNames(), getDacNames() );
	}
	catch (Error& err)
	{
		sendErr( err.what() );
	}
}


void AuxiliaryWindow::openTopBottomAgilentScript( CWnd* parent )
{
	try
	{
		topBottomAgilent.agilentScript.checkSave( mainWindowFriend->getProfileSettings().categoryPath, mainWindowFriend->getRunInfo() );
		std::string openFileName = openWithExplorer( parent, AGILENT_SCRIPT_EXTENSION);
		topBottomAgilent.agilentScript.openParentScript( openFileName, 
														 mainWindowFriend->getProfileSettings().categoryPath, 
														 mainWindowFriend->getRunInfo() );
		mainWindowFriend->updateConfigurationSavedStatus( false );
		topBottomAgilent.agilentScript.updateScriptNameText( mainWindowFriend->getProfileSettings().categoryPath );
	}
	catch (Error& err)
	{
		sendErr( err.what() );
	}
}


void AuxiliaryWindow::saveTopBottomAgilentScript()
{
	try
	{
		topBottomAgilent.agilentScript.saveScript( mainWindowFriend->getProfileSettings().categoryPath, 
												   mainWindowFriend->getRunInfo() );
		topBottomAgilent.agilentScript.updateScriptNameText( mainWindowFriend->getProfileSettings().categoryPath );
	}
	catch (Error& err)
	{
		sendErr( err.what() );
	}
}


void AuxiliaryWindow::saveTopBottomAgilentScriptAs( CWnd* parent )
{
	try
	{
		std::string extensionNoPeriod = topBottomAgilent.agilentScript.getExtension();
		if (extensionNoPeriod.size() == 0)
		{
			return;
		}
		extensionNoPeriod = extensionNoPeriod.substr( 1, extensionNoPeriod.size() );
		std::string newScriptAddress = saveWithExplorer( parent, extensionNoPeriod, 
															 mainWindowFriend->getProfileSettings() );
		topBottomAgilent.agilentScript.saveScriptAs( newScriptAddress, mainWindowFriend->getRunInfo() );
		mainWindowFriend->updateConfigurationSavedStatus( false );
		topBottomAgilent.agilentScript.updateScriptNameText( mainWindowFriend->getProfileSettings().categoryPath );
	}
	catch (Error& err)
	{
		sendErr( err.what() );
	}
}


void AuxiliaryWindow::newAxialUwaveAgilentScript()
{
	try
	{
		uWaveAxialAgilent.agilentScript.checkSave( mainWindowFriend->getProfileSettings().categoryPath, mainWindowFriend->getRunInfo() );
		uWaveAxialAgilent.agilentScript.newScript( );
		mainWindowFriend->updateConfigurationSavedStatus( false );
		uWaveAxialAgilent.agilentScript.updateScriptNameText( mainWindowFriend->getProfileSettings().categoryPath );
		uWaveAxialAgilent.agilentScript.colorEntireScript( getAllVariables(), mainWindowFriend->getRgbs(),
														  getTtlNames(), getDacNames() );

	}
	catch (Error& err)
	{
		sendErr( err.what() );
	}
}


void AuxiliaryWindow::openAxialUwaveAgilentScript( CWnd* parent )
{
	try
	{
		uWaveAxialAgilent.agilentScript.checkSave( mainWindowFriend->getProfileSettings().categoryPath, mainWindowFriend->getRunInfo() );
		std::string openFileName = openWithExplorer( parent, AGILENT_SCRIPT_EXTENSION );
		uWaveAxialAgilent.agilentScript.openParentScript( openFileName,
														 mainWindowFriend->getProfileSettings().categoryPath,
														 mainWindowFriend->getRunInfo() );
		mainWindowFriend->updateConfigurationSavedStatus( false );
		uWaveAxialAgilent.agilentScript.updateScriptNameText( mainWindowFriend->getProfileSettings().categoryPath );

	}
	catch (Error& err)
	{
		sendErr( err.what() );
	}
}


void AuxiliaryWindow::saveAxialUwaveAgilentScript()
{
	try
	{
		uWaveAxialAgilent.agilentScript.saveScript( mainWindowFriend->getProfileSettings().categoryPath,
												   mainWindowFriend->getRunInfo() );
		uWaveAxialAgilent.agilentScript.updateScriptNameText( mainWindowFriend->getProfileSettings().categoryPath );

	}
	catch (Error& err)
	{
		sendErr( err.what() );
	}
}


void AuxiliaryWindow::saveAxialUwaveAgilentScriptAs( CWnd* parent )
{
	try
	{
		std::string extensionNoPeriod = topBottomAgilent.agilentScript.getExtension();
		if (extensionNoPeriod.size() == 0)
		{
			return;
		}
		extensionNoPeriod = extensionNoPeriod.substr( 1, extensionNoPeriod.size() );
		std::string newScriptAddress = saveWithExplorer( parent, extensionNoPeriod,
															 mainWindowFriend->getProfileSettings() );
		topBottomAgilent.agilentScript.saveScriptAs( newScriptAddress, mainWindowFriend->getRunInfo() );
		mainWindowFriend->updateConfigurationSavedStatus( false );
		topBottomAgilent.agilentScript.updateScriptNameText( mainWindowFriend->getProfileSettings().categoryPath );
	}
	catch (Error& err)
	{
		sendErr( err.what() );
	}
}


void AuxiliaryWindow::newFlashingAgilentScript()
{
	try
	{
		flashingAgilent.agilentScript.checkSave( mainWindowFriend->getProfileSettings().categoryPath, mainWindowFriend->getRunInfo() );
		flashingAgilent.agilentScript.newScript( );
		mainWindowFriend->updateConfigurationSavedStatus( false );
		flashingAgilent.agilentScript.updateScriptNameText( mainWindowFriend->getProfileSettings().categoryPath );
		flashingAgilent.agilentScript.colorEntireScript( getAllVariables(), mainWindowFriend->getRgbs(),
														 getTtlNames(), getDacNames() );
	}
	catch (Error& err)
	{
		sendErr( err.what() );
	}
}


void AuxiliaryWindow::openFlashingAgilentScript( CWnd* parent )
{
	try
	{
		flashingAgilent.agilentScript.checkSave( mainWindowFriend->getProfileSettings().categoryPath, mainWindowFriend->getRunInfo() );
		std::string openFileName = openWithExplorer( parent, AGILENT_SCRIPT_EXTENSION );
		flashingAgilent.agilentScript.openParentScript( openFileName,
														  mainWindowFriend->getProfileSettings().categoryPath,
														  mainWindowFriend->getRunInfo() );
		mainWindowFriend->updateConfigurationSavedStatus( false );
		flashingAgilent.agilentScript.updateScriptNameText( mainWindowFriend->getProfileSettings().categoryPath );


	}
	catch (Error& err)
	{
		sendErr( err.what() );
	}
}


void AuxiliaryWindow::saveFlashingAgilentScript()
{
	try
	{
		flashingAgilent.agilentScript.saveScript( mainWindowFriend->getProfileSettings().categoryPath,
												   mainWindowFriend->getRunInfo() );
		flashingAgilent.agilentScript.updateScriptNameText( mainWindowFriend->getProfileSettings().categoryPath );
	}
	catch (Error& err)
	{
		sendErr( err.what() );
	}
}


void AuxiliaryWindow::saveFlashingAgilentScriptAs( CWnd* parent )
{
	try
	{
		std::string extensionNoPeriod = flashingAgilent.agilentScript.getExtension();
		if (extensionNoPeriod.size() == 0)
		{
			return;
		}
		extensionNoPeriod = extensionNoPeriod.substr( 1, extensionNoPeriod.size() );
		std::string newScriptAddress = saveWithExplorer( parent, extensionNoPeriod,
															 mainWindowFriend->getProfileSettings() );
		flashingAgilent.agilentScript.saveScriptAs( newScriptAddress, mainWindowFriend->getRunInfo() );
		mainWindowFriend->updateConfigurationSavedStatus( false );
		flashingAgilent.agilentScript.updateScriptNameText( mainWindowFriend->getProfileSettings().categoryPath );
	}
	catch (Error& err)
	{
		sendErr( err.what() );
	}
}



void AuxiliaryWindow::OnTimer( UINT_PTR eventID )
{
	uWaveAxialAgilent.agilentScript.handleTimerCall( getAllVariables(), mainWindowFriend->getRgbs(),
													 getTtlNames(), getDacNames());
	flashingAgilent.agilentScript.handleTimerCall( getAllVariables(), mainWindowFriend->getRgbs(),
												   getTtlNames(), getDacNames());
	topBottomAgilent.agilentScript.handleTimerCall( getAllVariables(), mainWindowFriend->getRgbs(),
													getTtlNames(), getDacNames() );
}


void AuxiliaryWindow::handleTopBottomEditChange()
{
	try
	{
		topBottomAgilent.agilentScript.handleEditChange();
		SetTimer( SYNTAX_TIMER_ID, SYNTAX_TIMER_LENGTH, NULL );
	}
	catch (Error& err)
	{
		sendErr( err.what() );
	}
}


void AuxiliaryWindow::handleAxialUwaveEditChange()
{
	try
	{
		uWaveAxialAgilent.agilentScript.handleEditChange();
		SetTimer( SYNTAX_TIMER_ID, SYNTAX_TIMER_LENGTH, NULL );
	}
	catch (Error& err)
	{
		sendErr( err.what() );
	}
}


void AuxiliaryWindow::handleFlashingEditChange()
{
	try
	{
		flashingAgilent.agilentScript.handleEditChange();
		SetTimer( SYNTAX_TIMER_ID, SYNTAX_TIMER_LENGTH, NULL );
	}
	catch (Error& err)
	{
		sendErr( err.what() );
	}
}



void AuxiliaryWindow::passTopBottomTekProgram()
{
	try
	{
		topBottomTek.handleProgram();
		sendStat( "Programmed Top/Bottom Tektronics Generator.\r\n" );
	}
	catch (Error& exception)
	{
		sendErr( "Error while programing top/Bottom Tektronics generator: " + exception.whatStr() + "\r\n" );
	}
}


void AuxiliaryWindow::passEoAxialTekProgram()
{
	try
	{
		eoAxialTek.handleProgram();
		sendStat( "Programmed E.O.M / Axial Tektronics Generator.\r\n" );
	}
	catch (Error& exception)
	{
		sendErr( "Error while programing E.O.M. / Axial Tektronics generator: " + exception.whatStr() + "\r\n" );
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
	dacBoards.handleNewConfig( newFile );
	topBottomAgilent.handleNewConfig( newFile );
	uWaveAxialAgilent.handleNewConfig( newFile );
	flashingAgilent.handleNewConfig( newFile );
	topBottomTek.handleNewConfig( newFile );
	eoAxialTek.handleNewConfig( newFile );
}


void AuxiliaryWindow::handleSaveConfig(std::ofstream& saveFile)
{
	// order matters.
	configVariables.handleSaveConfig(saveFile);
	ttlBoard.handleSaveConfig(saveFile);
	dacBoards.handleSaveConfig(saveFile);
	topBottomAgilent.handleSavingConfig(saveFile, mainWindowFriend->getProfileSettings().categoryPath, 
										 mainWindowFriend->getRunInfo());
	uWaveAxialAgilent.handleSavingConfig(saveFile, mainWindowFriend->getProfileSettings( ).categoryPath,
										  mainWindowFriend->getRunInfo( ) );
	flashingAgilent.handleSavingConfig(saveFile, mainWindowFriend->getProfileSettings( ).categoryPath,
										mainWindowFriend->getRunInfo( ) );
	topBottomTek.handleSaveConfig(saveFile);
	eoAxialTek.handleSaveConfig(saveFile);
}


void AuxiliaryWindow::handleOpeningConfig(std::ifstream& configFile, double version)
{
	ttlBoard.prepareForce( );
	dacBoards.prepareForce( );

	configVariables.handleOpenConfig(configFile, version);

	ttlBoard.handleOpenConfig(configFile, version);
	dacBoards.handleOpenConfig(configFile, version, &ttlBoard);

	topBottomAgilent.readConfigurationFile(configFile, version);
	topBottomAgilent.updateSettingsDisplay( 1, mainWindowFriend->getProfileSettings().categoryPath, mainWindowFriend->getRunInfo() );
	uWaveAxialAgilent.readConfigurationFile(configFile, version );
	uWaveAxialAgilent.updateSettingsDisplay( 1, mainWindowFriend->getProfileSettings().categoryPath, mainWindowFriend->getRunInfo() );
	flashingAgilent.readConfigurationFile(configFile, version );
	flashingAgilent.updateSettingsDisplay( 1, mainWindowFriend->getProfileSettings().categoryPath, mainWindowFriend->getRunInfo() );

	topBottomTek.handleOpeningConfig(configFile, version);
	eoAxialTek.handleOpeningConfig(configFile, version);
}


UINT AuxiliaryWindow::getNumberOfDacs()
{
	return dacBoards.getNumberOfDacs();
}


std::array<std::array<std::string, 16>, 4> AuxiliaryWindow::getTtlNames()
{
	return ttlBoard.getAllNames();
}


std::array<std::string, 24> AuxiliaryWindow::getDacNames()
{
	return dacBoards.getAllNames();
}


void AuxiliaryWindow::drawVariables(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	if (id == IDC_GLOBAL_VARS_LISTVIEW)
	{
		globalVariables.handleDraw(pNMHDR, pResult, mainWindowFriend->getRgbs());
	}
	else
	{
		configVariables.handleDraw(pNMHDR, pResult, mainWindowFriend->getRgbs());
	}
}


void AuxiliaryWindow::ConfigVarsDblClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	std::vector<Script*> scriptList;
	try
	{
		configVariables.updateVariableInfo(scriptList, mainWindowFriend, this, &ttlBoard, &dacBoards);
	}
	catch (Error& exception)
	{
		sendErr("Variables Double Click Handler : " + exception.whatStr() + "\r\n");
	}
	mainWindowFriend->updateConfigurationSavedStatus(false);
}


void AuxiliaryWindow::ConfigVarsRClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	try
	{
		configVariables.deleteVariable();
	}
	catch (Error& exception)
	{
		sendErr("Variables Right Click Handler : " + exception.whatStr() + "\r\n");
	}
	mainWindowFriend->updateConfigurationSavedStatus(false);
}

std::vector<variableType> AuxiliaryWindow::getAllVariables()
{
	std::vector<variableType> vars = configVariables.getEverything();
	std::vector<variableType> vars2 = globalVariables.getEverything();
	vars.insert(vars.end(), vars2.begin(), vars2.end());
	return vars;
}


void AuxiliaryWindow::GlobalVarDblClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	std::vector<Script*> scriptList;
	try
	{
		globalVariables.updateVariableInfo(scriptList, mainWindowFriend, this, &ttlBoard, &dacBoards);
	}
	catch (Error& exception)
	{
		sendErr("Global Variables Double Click Handler : " + exception.whatStr() + "\r\n");
	}
}


void AuxiliaryWindow::GlobalVarRClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	try
	{
		globalVariables.deleteVariable();
	}
	catch (Error& exception)
	{
		sendErr("Global Variables Right Click Handler : " + exception.whatStr() + "\r\n");
	}
	//masterConfig.save(&ttlBoard, &dacBoards, &globalVariables);
}



void AuxiliaryWindow::ConfigVarsColumnClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	try
	{
		configVariables.handleColumnClick(pNotifyStruct, result);
	}
	catch (Error& exception)
	{
		sendErr("Handling config variable listview click : " + exception.whatStr() + "\r\n");
	}
	mainWindowFriend->updateConfigurationSavedStatus(false);
}


void AuxiliaryWindow::clearVariables()
{
	configVariables.clearVariables();
}


void AuxiliaryWindow::addVariable(std::string name, bool constant, double value, int item)
{
	variableType var;
	var.name = name;
	var.constant = constant;
	var.ranges.push_back({ value, 0, 1, false, true });
	configVariables.addConfigVariable(var, item);
}


void AuxiliaryWindow::passCommonCommand(UINT id)
{
	try
	{
		commonFunctions::handleCommonMessage(id, this, mainWindowFriend, scriptingWindowFriend, cameraWindowFriend, this);
	}
	catch (Error& err)
	{
		// catch any extra errors that handleCommonMessage doesn't explicitly handle.
		errBox(err.what());
	}
}

void AuxiliaryWindow::loadFriends(MainWindow* mainWin, ScriptingWindow* scriptWin, CameraWindow* camWin)
{
	mainWindowFriend = mainWin;
	scriptingWindowFriend = scriptWin;
	cameraWindowFriend = camWin;
}


void AuxiliaryWindow::passRoundToDac()
{
	dacBoards.handleRoundToDac(menu);
}


void AuxiliaryWindow::handleTektronicsButtons(UINT id)
{
	if (id >= TOP_ON_OFF && id <= BOTTOM_FSK)
	{
		topBottomTek.handleButtons(id - TOP_ON_OFF);
	}
	if (id >= EO_ON_OFF && id <= AXIAL_FSK)
	{
		eoAxialTek.handleButtons(id - EO_ON_OFF);
	}
	mainWindowFriend->updateConfigurationSavedStatus(false);
}


void AuxiliaryWindow::handleEnter()
{
	errBox("Hello, there!");
}


void AuxiliaryWindow::setConfigActive(bool active)
{
	configVariables.setActive(active);
}


UINT AuxiliaryWindow::getTotalVariationNumber()
{
	return configVariables.getTotalVariationNumber();
}


void AuxiliaryWindow::OnSize(UINT nType, int cx, int cy)
{
	SetRedraw( false );
	topBottomTek.rearrange(cx, cy, getFonts());
	eoAxialTek.rearrange(cx, cy, getFonts());

	topBottomAgilent.rearrange(cx, cy, getFonts());
	uWaveAxialAgilent.rearrange(cx, cy, getFonts());
	flashingAgilent.rearrange(cx, cy, getFonts());

	RhodeSchwarzGenerator.rearrange(cx, cy, getFonts());

	ttlBoard.rearrange(cx, cy, getFonts());
	dacBoards.rearrange(cx, cy, getFonts());

	configVariables.rearrange(cx, cy, getFonts());
	globalVariables.rearrange(cx, cy, getFonts());

	statusBox.rearrange( cx, cy, getFonts());
	SetRedraw();
	RedrawWindow();
}


fontMap AuxiliaryWindow::getFonts()
{
	return mainWindowFriend->getFonts();
}


void AuxiliaryWindow::handleAgilentOptions( UINT id )
{
	// zero the id.
	id -= IDC_TOP_BOTTOM_CHANNEL1_BUTTON;
	int agilentNum = id / 10;
	// figure out which box it was.
	Agilent* agilent = NULL;
	if (agilentNum == 0)
	{
		agilent = &topBottomAgilent;
	}
	else if (agilentNum == 1)
	{
		agilent = &uWaveAxialAgilent;
	}
	else if (agilentNum == 2)
	{
		agilent = &flashingAgilent;
	}
	else
	{
		errBox( "4th agilent???" );
		return;
	}
	// call the correct function.
	if (id % 7 == 0)
	{
		// channel 1
		agilent->handleChannelPress( 1, mainWindowFriend->getProfileSettings().categoryPath, 
									 mainWindowFriend->getRunInfo() );
	}
	else if (id % 7 == 1)
	{
		// channel 2
		agilent->handleChannelPress( 2, mainWindowFriend->getProfileSettings().categoryPath, 
									 mainWindowFriend->getRunInfo() );
	}
	else if (id % 7 == 3)
	{
		// TODO:
		// handle sync 
		//agilent->handleSync();
	}
	else if (id % 7 == 6)
	{
		try
		{
			agilent->handleInput( mainWindowFriend->getProfileSettings().categoryPath, mainWindowFriend->getRunInfo() );
			agilent->setAgilent();
			sendStat( "Programmed Agilent " + agilent->getName() + ".\r\n" );
		}
		catch (Error& err)
		{
			sendErr( "Error while programming agilent " + agilent->getName() + ": " + err.what() + "\r\n" );
		}
	}
	// else it's a combo or edit that must be handled separately, not in an ON_COMMAND handling.
}


void AuxiliaryWindow::handleTopBottomAgilentCombo()
{
	try
	{
		topBottomAgilent.handleInput( mainWindowFriend->getProfileSettings().categoryPath, mainWindowFriend->getRunInfo() );
		topBottomAgilent.handleCombo();
		topBottomAgilent.updateSettingsDisplay( mainWindowFriend->getProfileSettings().categoryPath, mainWindowFriend->getRunInfo() );
	}
	catch (Error& err)
	{
		sendErr( "ERROR: error while handling agilent combo change: " + err.whatStr() );
	}
}


void AuxiliaryWindow::handleAxialUWaveAgilentCombo()
{
	try
	{
		uWaveAxialAgilent.handleInput( mainWindowFriend->getProfileSettings().categoryPath, mainWindowFriend->getRunInfo() );
		uWaveAxialAgilent.handleCombo();
		uWaveAxialAgilent.updateSettingsDisplay( mainWindowFriend->getProfileSettings().categoryPath, mainWindowFriend->getRunInfo() );
	}
	catch (Error& err)
	{
		sendErr( "ERROR: error while handling agilent combo change: " + err.whatStr() );
	}
}


void AuxiliaryWindow::handleFlashingAgilentCombo()
{
	try
	{
		flashingAgilent.handleInput( mainWindowFriend->getProfileSettings().categoryPath, mainWindowFriend->getRunInfo() );
		flashingAgilent.handleCombo();
		flashingAgilent.updateSettingsDisplay( mainWindowFriend->getProfileSettings().categoryPath, mainWindowFriend->getRunInfo() );
	}
	catch (Error& err)
	{
		sendErr( "ERROR: error while handling agilent combo change: " + err.whatStr() );
	}
}


void AuxiliaryWindow::sendErr(std::string msg)
{
	mainWindowFriend->getComm()->sendError(msg);
}


void AuxiliaryWindow::sendStat(std::string msg)
{
	mainWindowFriend->getComm()->sendStatus(msg);
}


void AuxiliaryWindow::zeroDacs()
{
	try
	{
		dacBoards.resetDacEvents();
		ttlBoard.resetTtlEvents();
		dacBoards.prepareForce();
		ttlBoard.prepareForce();
		for (int dacInc = 0; dacInc < 24; dacInc++)
		{
			dacBoards.prepareDacForceChange( dacInc, 0, &ttlBoard );
		}
		dacBoards.organizeDacCommands(0);
		dacBoards.makeFinalDataFormat(0);
		dacBoards.stopDacs(); 
		dacBoards.configureClocks(0);
		dacBoards.writeDacs(0);
		dacBoards.startDacs();
		ttlBoard.organizeTtlCommands(0);
		ttlBoard.convertToFinalFormat(0);
		ttlBoard.writeTtlData(0);
		ttlBoard.startBoard();
		ttlBoard.waitTillFinished(0);
		sendStat( "Zero'd DACs.\r\n");
	}
	catch (Error& exception)
	{
		sendStat( "Failed to Zero DACs!!!\r\n" );
		sendErr( exception.what() );
	}
}


void AuxiliaryWindow::zeroTtls()
{
	try
	{
		ttlBoard.zeroBoard();
		sendStat( "Zero'd TTLs.\r\n" );
	}
	catch (Error& exception)
	{
		sendStat( "Failed to Zero TTLs!!!\r\n" );
		sendErr( exception.what() );
	}
}


void AuxiliaryWindow::loadMotSettings(MasterThreadInput* input)
{
	try
	{
		sendStat("Loading MOT Configuration...\r\n" );
		input->quiet = true;
		input->ttls = &ttlBoard;
		input->dacs = &dacBoards;
		input->globalControl = &globalVariables;
		input->comm = mainWindowFriend->getComm();
		input->settings = { 0,0,0 };
		input->debugOptions = { 0, 0, 0, 0, 0, 0, 0, "", 0, 0, 0 };
		// don't get configuration variables. The MOT shouldn't depend on config variables.
		input->variables = globalVariables.getEverything();
		// Only set it once, clearly.
		input->repetitionNumber = 1;
		input->masterScriptAddress = MOT_ROUTINE_ADDRESS;
		input->rsg = &RhodeSchwarzGenerator;
		input->agilents.push_back( &topBottomAgilent );
		input->agilents.push_back( &uWaveAxialAgilent );
		input->agilents.push_back( &flashingAgilent );
		input->intensityAgilentNumber = -1;
		input->topBottomTek = &topBottomTek;
		input->eoAxialTek = &eoAxialTek;
		input->runMaster = true;
		input->runNiawg = false;
	}
	catch (Error& exception)
	{
		sendStat(": " + exception.whatStr() + " " + exception.whatStr() + "\r\n" );
	}
}


// Gets called after alt-f4 or X button is pressed.
void AuxiliaryWindow::OnCancel()
{
	passCommonCommand(ID_FILE_MY_EXIT);
}


void AuxiliaryWindow::fillMasterThreadInput(MasterThreadInput* input)
{
	input->ttls = &ttlBoard;
	input->dacs = &dacBoards;
	input->globalControl = &globalVariables;

	// load the variables. This little loop is for letting configuration variables overwrite the globals.
	std::vector<variableType> configVars = configVariables.getEverything();
	std::vector<variableType> globals = globalVariables.getEverything();
	std::vector<variableType> experimentVars = configVars;

	for (auto& globalVar : globals)
	{
		globalVar.overwritten = false;
		bool nameExists = false;
		for (auto& configVar : experimentVars)
		{
			if (configVar.name == globalVar.name)
			{
				globalVar.overwritten = true;
				configVar.overwritten = true;
			}
		}
		if (!globalVar.overwritten)
		{
			experimentVars.push_back(globalVar);
		}
	}
	input->variables = experimentVars;
	globalVariables.setUsages(globals);
	input->rsg = &RhodeSchwarzGenerator;
	input->agilents.push_back(&topBottomAgilent);
	input->agilents.push_back(&uWaveAxialAgilent);
	input->agilents.push_back( &flashingAgilent );
	topBottomTek.getSettings();
	eoAxialTek.getSettings();
	input->topBottomTek = &topBottomTek;
	input->eoAxialTek = &eoAxialTek;
}


void AuxiliaryWindow::changeBoxColor(systemInfo<char> colors)
{
	statusBox.changeColor(colors);
}


void AuxiliaryWindow::handleAbort()
{
	ttlBoard.unshadeTtls();
	dacBoards.unshadeDacs();
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
	for (UINT dacInc = 0; dacInc < dacBoards.getNumberOfDacs(); dacInc++)
	{
		std::string name = dacBoards.getName(dacInc);
		std::pair<double, double> minMax = dacBoards.getDacRange(dacInc);
		if (name == "")
		{
			// then the name hasn't been set, so create the default name
			name = "Dac" + str(dacInc);
		}
		configStream << name << "\n";
		configStream << minMax.first << " - " << minMax.second << "\n";
		configStream << dacBoards.getDefaultValue(dacInc) << "\n";
	}

	// Number of Variables
	configStream << globalVariables.getCurrentNumberOfVariables() << "\n";
	/// Variables

	for (UINT varInc = 0; varInc < globalVariables.getCurrentNumberOfVariables(); varInc++)
	{
		variableType info = globalVariables.getVariableInfo(varInc);
		configStream << info.name << " ";
		configStream << info.ranges.front().initialValue << "\n";
		// all globals are constants, no need to output anything else.
	}

}


void AuxiliaryWindow::handleMasterConfigOpen(std::stringstream& configStream, double version)
{
	ttlBoard.resetTtlEvents();
	ttlBoard.prepareForce();
	dacBoards.resetDacEvents();
	dacBoards.prepareForce();
	// save info
	for (UINT ttlRowInc = 0; ttlRowInc < ttlBoard.getTtlBoardSize().first; ttlRowInc++)
	{
		for (UINT ttlNumberInc = 0; ttlNumberInc < ttlBoard.getTtlBoardSize().second; ttlNumberInc++)
		{
			std::string name;
			std::string statusString;
			bool status;
			configStream >> name;
			configStream >> statusString;
			try
			{
				// should actually be zero or one, but just just convert to bool
				status = std::stoi(statusString);
			}
			catch (std::invalid_argument&)
			{
				thrower("ERROR: Failed to load one of the default ttl values!");
			}

			ttlBoard.setName(ttlRowInc, ttlNumberInc, name, toolTips, this);
			ttlBoard.forceTtl(ttlRowInc, ttlNumberInc, status);
			ttlBoard.updateDefaultTtl(ttlRowInc, ttlNumberInc, status);
		}
	}
	// getting dacs.
	for (UINT dacInc = 0; dacInc < dacBoards.getNumberOfDacs(); dacInc++)
	{
		std::string name;
		std::string defaultValueString;
		double defaultValue;
		std::string minString;
		std::string maxString;
		double min;
		double max;
		configStream >> name;
		if (version >= 1.2)
		{
			configStream >> minString;
			std::string trash;
			configStream >> trash;
			if (trash != "-")
			{
				thrower("ERROR: Expected \"-\" in config file between min and max values!");
			}
			configStream >> maxString;
		}
		configStream >> defaultValueString;
		try
		{
			defaultValue = std::stod(defaultValueString);
			if (version >= 1.2)
			{
				min = std::stod(minString);
				max = std::stod(maxString);
			}
			else
			{
				min = -10;
				max = 10;
			}
		}
		catch (std::invalid_argument&)
		{
			thrower("ERROR: Failed to load one of the default DAC values!");
		}
		dacBoards.setName(dacInc, name, toolTips, this);
		dacBoards.setMinMax(dacInc, min, max);
		dacBoards.prepareDacForceChange(dacInc, defaultValue, &ttlBoard);
		dacBoards.setDefaultValue(dacInc, defaultValue);
	}
	// variables.
	if (version >= 1.1)
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
			variableType tempVar;
			tempVar.constant = true;
			tempVar.overwritten = false;
			tempVar.active = false;
			double value;
			configStream >> tempVar.name;
			configStream >> value;
			tempVar.ranges.push_back({ value, value, 0, false, true });
			globalVariables.addGlobalVariable(tempVar, varInc);
		}
	}
	variableType tempVar;
	tempVar.name = "";
	globalVariables.addGlobalVariable(tempVar, -1);
}


void AuxiliaryWindow::SetDacs()
{
	// have the dac values change
	try
	{
		sendStat("----------------------\r\n");
		dacBoards.resetDacEvents();
		ttlBoard.resetTtlEvents();
		sendStat( "Setting Dacs...\r\n" );
		dacBoards.handleButtonPress( &ttlBoard );
		dacBoards.organizeDacCommands(0);
		dacBoards.makeFinalDataFormat(0);
		// start the boards which actually sets the dac values.
		dacBoards.stopDacs();
		dacBoards.configureClocks(0);
		sendStat( "Writing New Dac Settings...\r\n" );
		dacBoards.writeDacs(0);
		dacBoards.startDacs();
		ttlBoard.organizeTtlCommands(0);
		ttlBoard.convertToFinalFormat(0);
		ttlBoard.writeTtlData(0);
		ttlBoard.startBoard();
		ttlBoard.waitTillFinished(0);
		sendStat( "Finished Setting Dacs.\r\n" );
		/*
		dacBoards.analyzeDacCommands(0);
		dacBoards.makeFinalDataFormat(0);
		dacBoards.stopDacs();
		dacBoards.configureClocks(0);
		dacBoards.writeDacs(0);
		dacBoards.startDacs();
		ttlBoard.organizeTtlCommands(0);
		ttlBoard.convertToFinalFormat(0);
		ttlBoard.writeTtlData(0);
		ttlBoard.startBoard();
		ttlBoard.waitTillFinished(0);
		sendStat( "Zero'd DACs.\r\n");
		*/
	}
	catch (Error& exception)
	{
		errBox( exception.what() );
		sendStat( ": " + exception.whatStr() + "\r\n" );
		sendErr( exception.what() );
	}
	mainWindowFriend->updateConfigurationSavedStatus(false);
}


void AuxiliaryWindow::DacEditChange(UINT id)
{
	try
	{
		dacBoards.handleEditChange(id - ID_DAC_FIRST_EDIT);
	}
	catch (Error& err)
	{
		sendErr(err.what());
	}
}


void AuxiliaryWindow::handleTtlPush(UINT id)
{
	try
	{
		ttlBoard.handleTTLPress( id );
	}
	catch (Error& exception)
	{
		sendErr( "TTL Press Handler Failed: " + exception.whatStr() + "\r\n" );
	}
	mainWindowFriend->updateConfigurationSavedStatus(false);
}


void AuxiliaryWindow::handlTtlHoldPush()
{
	try
	{
		ttlBoard.handleHoldPress();
	}
	catch (Error& exception)
	{
		sendErr( "TTL Hold Handler Failed: " + exception.whatStr() + "\r\n" );
	}
	mainWindowFriend->updateConfigurationSavedStatus(false);
}


void AuxiliaryWindow::ViewOrChangeTTLNames()
{
	ttlInputStruct input;
	input.ttls = &ttlBoard;
	input.toolTips = toolTips;
	TtlSettingsDialog dialog(&input, IDD_VIEW_AND_CHANGE_TTL_NAMES);
	dialog.DoModal();
}


void AuxiliaryWindow::ViewOrChangeDACNames()
{
	dacInputStruct input;
	input.dacs = &dacBoards;
	input.toolTips = toolTips;
	DacSettingsDialog dialog(&input, IDD_VIEW_AND_CHANGE_DAC_NAMES);
	dialog.DoModal();
}


void AuxiliaryWindow::Exit()
{
	EndDialog(0);
}


HBRUSH AuxiliaryWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	brushMap brushes = mainWindowFriend->getBrushes();
	rgbMap rgbs = mainWindowFriend->getRgbs();
	HBRUSH result = flashingAgilent.handleColorMessage( pWnd, brushes, rgbs, pDC );
	if (result != NULL)
	{
		return result;
	}
	result = uWaveAxialAgilent.handleColorMessage( pWnd, brushes, rgbs, pDC );
	if (result != NULL)
	{
		return result;
	}
	result = topBottomAgilent.handleColorMessage( pWnd, brushes, rgbs, pDC );
	if (result != NULL)
	{
		return result;
	}
	result = ttlBoard.handleColorMessage(pWnd, brushes, rgbs, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = dacBoards.handleColorMessage(pWnd, brushes, rgbs, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = topBottomTek.handleColorMessage(pWnd, brushes, rgbs, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = eoAxialTek.handleColorMessage(pWnd, brushes, rgbs, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = *statusBox.handleColoring(pWnd->GetDlgCtrlID(), pDC, brushes, rgbs);
	if (result != NULL)
	{
		return result;
	}

	// default colors
	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{
			pDC->SetTextColor(rgbs["Solarized Yellow"]);
			pDC->SetBkColor(rgbs["Medium Grey"]);
			return *brushes["Medium Grey"];
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor(rgbs["White"]);
			pDC->SetBkColor(rgbs["Dark Grey"]);
			return *brushes["Dark Grey"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor(rgbs["White"]);
			pDC->SetBkColor(rgbs["Dark Grey"]);
			return *brushes["Dark Grey"];
		}
		default:
			return *brushes["Solarized Base04"];
	}
}


BOOL AuxiliaryWindow::PreTranslateMessage(MSG* pMsg)
{
	for (UINT toolTipInc = 0; toolTipInc < toolTips.size(); toolTipInc++)
	{
		toolTips[toolTipInc]->RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}


BOOL AuxiliaryWindow::OnInitDialog()
{
	// don't redraw until the first OnSize.
	SetRedraw( false );

	int id = 4000;
	POINT controlLocation{ 0, 0 };
	try
	{
		statusBox.initialize( controlLocation, id, this, 480, toolTips );
		ttlBoard.initialize( controlLocation, toolTips, this, id );
		dacBoards.initialize( controlLocation, toolTips, this, id );

		POINT statusLoc = { 960, 0 };
		topBottomTek.initialize( statusLoc, this, id, "Top-Bottom-Tek", "Top", "Bottom", 480,
								 {TOP_BOTTOM_PROGRAM, TOP_ON_OFF, TOP_FSK, BOTTOM_ON_OFF, BOTTOM_FSK} );
		eoAxialTek.initialize( statusLoc, this, id, "EO / Axial", "EO", "Axial", 480, { EO_AXIAL_PROGRAM,
							   EO_ON_OFF, EO_FSK, AXIAL_ON_OFF, AXIAL_FSK } );
		RhodeSchwarzGenerator.initialize( controlLocation, toolTips, this, id );
		controlLocation = POINT{ 480, 0 };
		topBottomAgilent.initialize( controlLocation, toolTips, this, id, "Top-Bottom-Agilent", 120, 
									{ IDC_TOP_BOTTOM_CHANNEL1_BUTTON, IDC_TOP_BOTTOM_CHANNEL2_BUTTON,
									  IDC_TOP_BOTTOM_SYNC_BUTTON, IDC_TOP_BOTTOM_CALIBRATION_BUTTON, 
									 IDC_TOP_BOTTOM_PROGRAM, IDC_TOP_BOTTOM_AGILENT_COMBO,
									 IDC_TOP_BOTTOM_FUNCTION_COMBO, IDC_TOP_BOTTOM_EDIT},
									 mainWindowFriend->getRgbs()["Solarized Base03"] );
		uWaveAxialAgilent.initialize( controlLocation, toolTips, this, id, 
									  "Microwave-Axial-Agilent",   120, { IDC_AXIAL_UWAVE_CHANNEL1_BUTTON, 
									  IDC_AXIAL_UWAVE_CHANNEL2_BUTTON, IDC_AXIAL_UWAVE_SYNC_BUTTON, 
									  IDC_AXIAL_UWAVE_CALIBRATION_BUTTON,
									  IDC_AXIAL_UWAVE_PROGRAM, IDC_AXIAL_UWAVE_AGILENT_COMBO, 
									  IDC_AXIAL_UWAVE_FUNCTION_COMBO, IDC_AXIAL_UWAVE_EDIT },
									  mainWindowFriend->getRgbs()["Solarized Base03"] );
		flashingAgilent.initialize( controlLocation, toolTips, this, id, 
									"Flashing-Agilent",  120, {IDC_FLASHING_CHANNEL1_BUTTON, 
									IDC_FLASHING_CHANNEL2_BUTTON, IDC_FLASHING_SYNC_BUTTON, 
									IDC_FLASHING_CALIBRATION_BUTTON, IDC_FLASHING_PROGRAM, 
									IDC_FLASHING_AGILENT_COMBO, IDC_FLASHING_FUNCTION_COMBO, IDC_FLASHING_EDIT}, 
									mainWindowFriend->getRgbs()["Solarized Base03"] );
		controlLocation = POINT{ 1440, 0 };
		globalVariables.initialize( controlLocation, toolTips, this, id, "GLOBAL VARIABLES",
									mainWindowFriend->getRgbs(), IDC_GLOBAL_VARS_LISTVIEW );
		configVariables.initialize( controlLocation, toolTips, this, id, "CONFIGURATION VARIABLES",
									mainWindowFriend->getRgbs(), IDC_CONFIG_VARS_LISTVIEW);
		configVariables.setActive( false );

	}
	catch (Error& exeption)
	{
		errBox( exeption.what() );
	}

	menu.LoadMenu( IDR_MAIN_MENU );
	SetMenu( &menu );
	return TRUE;
}


std::string AuxiliaryWindow::getSystemStatusMsg()
{
	// controls are done. Report the initialization status...
	std::string msg;
	msg += " >>> TTL System <<<\n";
	if (!DIO_SAFEMODE)
	{
		msg += "Code System is active!\n";
		msg += ttlBoard.getSystemInfo() + "\n";
	}
	else
	{
		msg += "Code System is disabled! Enable in \"constants.h\"\n";
	}


	msg += "\n>>> DAC System <<<\n";
	if (!DAQMX_SAFEMODE)
	{
		msg += "Code System is Active!\n";
		msg += dacBoards.getDacSystemInfo() + "\n";
	}
	else
	{
		msg += "Code System is disabled! Enable in \"constants.h\"\n";
	}

	msg += ">>>>>> VISA Devices <<<<<<<\n\n";
	msg += "Tektronics 1: " + topBottomTek.queryIdentity() + "\n";
	msg += "Tektronics 2: " + eoAxialTek.queryIdentity() + "\n";
	msg += "\n\n>>> Agilents <<<\n";
	msg += "Code System is Active!\n";
	msg += "Top / Bottom Agilent: " + topBottomAgilent.getDeviceIdentity();
	msg += "U Wave / Axial Agilent: " + uWaveAxialAgilent.getDeviceIdentity();
	msg += "Flashing Agilent: " + flashingAgilent.getDeviceIdentity();

	msg += "\n>>> GPIB System <<<\n";
	msg += "Code System is Active!\n";
	msg += "RSG: " + RhodeSchwarzGenerator.getIdentity() + "\n";

	// 
	return msg;
}


