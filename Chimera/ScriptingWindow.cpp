// created by Mark O. Brown
#include "stdafx.h"
#include "ScriptingWindow.h"
#include "afxwin.h"
#include "MainWindow.h"
#include "openWithExplorer.h"
#include "saveWithExplorer.h"
#include "commonFunctions.h"
#include "textPromptDialog.h"
#include "AuxiliaryWindow.h"
#include "AndorWindow.h"
#include "BaslerWindow.h"
#include "MainWindow.h"
#include "Agilent.h"
#include "Thrower.h"
#include "AgilentSettings.h"

ScriptingWindow::ScriptingWindow() : CDialog(), intensityAgilent( INTENSITY_AGILENT_SETTINGS )
{}


IMPLEMENT_DYNAMIC(ScriptingWindow, CDialog)

BEGIN_MESSAGE_MAP(ScriptingWindow, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_WM_SIZE()

	ON_EN_CHANGE(IDC_NIAWG_EDIT, &ScriptingWindow::niawgEditChange)
	ON_EN_CHANGE(IDC_INTENSITY_EDIT, &ScriptingWindow::agilentEditChange)
	ON_EN_CHANGE(IDC_MASTER_EDIT, &ScriptingWindow::masterEditChange)

	ON_COMMAND(IDOK, &ScriptingWindow::catchEnter)

	ON_COMMAND_RANGE(IDC_INTENSITY_CHANNEL1_BUTTON, IDC_INTENSITY_PROGRAM, &ScriptingWindow::handleIntensityButtons)
	ON_CBN_SELENDOK( IDC_INTENSITY_AGILENT_COMBO, &ScriptingWindow::handleIntensityCombo )

	ON_COMMAND_RANGE(MENU_ID_RANGE_BEGIN, MENU_ID_RANGE_END, &ScriptingWindow::passCommonCommand)

	ON_CBN_SELENDOK(IDC_NIAWG_FUNCTION_COMBO, &ScriptingWindow::handleNiawgScriptComboChange)
	ON_CBN_SELENDOK(IDC_INTENSITY_FUNCTION_COMBO, &ScriptingWindow::handleAgilentScriptComboChange)
	
	ON_CBN_SELENDOK( IDC_MASTER_FUNCTION_COMBO, &ScriptingWindow::handleMasterFunctionChange )
	ON_WM_RBUTTONUP( )
	ON_WM_LBUTTONUP( )
	ON_NOTIFY_EX_RANGE( TTN_NEEDTEXTA, 0, 0xFFFF, ScriptingWindow::OnToolTipText )
END_MESSAGE_MAP()

void ScriptingWindow::OnRButtonUp( UINT stuff, CPoint clickLocation )
{
	camWin->stopSound( );
}

void ScriptingWindow::OnLButtonUp( UINT stuff, CPoint clickLocation )
{
	camWin->stopSound( );
}



void ScriptingWindow::handleMasterFunctionChange( )
{
	try
	{
		masterScript.functionChangeHandler(mainWin->getProfileSettings().categoryPath);
		masterScript.colorEntireScript( auxWin->getAllVariables( ), auxWin->getTtlNames( ), auxWin->getDacInfo ( ) );
		masterScript.updateSavedStatus( true );
	}
	catch ( Error& err )
	{
		errBox( err.trace( ) );
	}
}


void ScriptingWindow::handleIntensityCombo()
{
	intensityAgilent.handleInput( mainWin->getProfileSettings().categoryPath, mainWin->getRunInfo() );
	intensityAgilent.handleModeCombo();
	intensityAgilent.updateSettingsDisplay(mainWin->getProfileSettings().categoryPath, mainWin->getRunInfo());
}


void ScriptingWindow::handleIntensityButtons( UINT id )
{
	id -= IDC_INTENSITY_CHANNEL1_BUTTON;
	if (id % 7 == 0)
	{
		// channel 1
		intensityAgilent.handleChannelPress( 1, mainWin->getProfileSettings().categoryPath, 
											 mainWin->getRunInfo() );
	}
	else if (id % 7 == 1)
	{
		// channel 2
		intensityAgilent.handleChannelPress( 2, mainWin->getProfileSettings().categoryPath,
											 mainWin->getRunInfo() );
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
			intensityAgilent.handleInput(mainWin->getProfileSettings().categoryPath, mainWin->getRunInfo());
			intensityAgilent.setAgilent();
			comm()->sendStatus( "Programmed Agilent " + intensityAgilent.configDelim + ".\r\n" );
		}
		catch (Error&)
		{
			comm()->sendError( "Error while programming agilent " + intensityAgilent.configDelim + "\r\n" );
		}
	}
	// else it's a combo or edit that must be handled separately, not in an ON_COMMAND handling.
}


void ScriptingWindow::masterEditChange()
{
	try
	{
		masterScript.handleEditChange();
		SetTimer(SYNTAX_TIMER_ID, SYNTAX_TIMER_LENGTH, NULL);
	}
	catch (Error& err)
	{
		comm()->sendError(err.trace());
	}
}


Communicator* ScriptingWindow::comm()
{
	return mainWin->getComm();
}

void ScriptingWindow::catchEnter()
{
	errBox("Secret Message!");
}


void ScriptingWindow::OnSize(UINT nType, int cx, int cy)
{
	SetRedraw( false );
	niawgScript.rearrange(cx, cy, mainWin->getFonts());
	intensityAgilent.rearrange( cx, cy, mainWin->getFonts() );
	masterScript.rearrange(cx, cy, mainWin->getFonts());
	statusBox.rearrange( cx, cy, mainWin->getFonts());
	profileDisplay.rearrange(cx, cy, mainWin->getFonts());
	recolorScripts ( );
	SetRedraw( true );
	RedrawWindow();
}

// special handling for long tooltips.
BOOL ScriptingWindow::OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
	try
	{
		niawgScript.handleToolTip( pNMHDR , pResult);
		intensityAgilent.agilentScript.handleToolTip( pNMHDR, pResult );
		masterScript.handleToolTip( pNMHDR, pResult );
	}
	catch(Error&)
	{
		// worked.
		return TRUE;
	}
	return FALSE;
}


BOOL ScriptingWindow::PreTranslateMessage(MSG* pMsg)
{
	for (UINT toolTipInc = 0; toolTipInc < tooltips.size(); toolTipInc++)
	{
		 tooltips[toolTipInc]->RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}


void ScriptingWindow::handleNiawgScriptComboChange()
{
	//horizontalNiawgScript.childComboChangeHandler();
}

void ScriptingWindow::handleAgilentScriptComboChange()
{
	//intensityAgilent.agilentScript.childComboChangeHandler( mainWindowFriend, auxWindowFriend);
}


// this gets called when closing. The purpose here is to redirect the default, very abrupt close that would normally happen.
void ScriptingWindow::OnCancel()
{
	passCommonCommand(ID_FILE_MY_EXIT);
}


BOOL ScriptingWindow::OnInitDialog()
{
	SetWindowText ( "Scripting Window" );
	EnableToolTips( TRUE );
	// don't redraw until the first OnSize.
	SetRedraw( false );

	int id = 2000;

	POINT startLocation = { 0, 28 };
	niawgScript.initialize( 640, 900, startLocation, tooltips, this,  id, "NIAWG",
							"NIAWG Script", { IDC_NIAWG_FUNCTION_COMBO, 
							IDC_NIAWG_EDIT }, _myRGBs["Interactable-Bkgd"]);
	niawgScript.setEnabled ( true, false );

	startLocation = { 640, 28 };
	
	intensityAgilent.initialize( startLocation, tooltips, this, id, "Intensity Agilent", 865, 
								 _myRGBs["Interactable-Bkgd"], 640 );
	startLocation = { 2*640, 28 };
	masterScript.initialize( 640, 900, startLocation, tooltips, this, id, "Master", "Master Script",
	                         { IDC_MASTER_FUNCTION_COMBO, IDC_MASTER_EDIT }, _myRGBs["Interactable-Bkgd"] );
	startLocation = { 1600, 3 };
	statusBox.initialize(startLocation, id, this, 300, tooltips);
	profileDisplay.initialize({ 0,3 }, id, this, tooltips);
	
	
	menu.LoadMenu(IDR_MAIN_MENU);
	SetMenu(&menu);
	try
	{
		// I only do this for the intensity agilent at the moment.
		intensityAgilent.setDefault( 1 );
	}
	catch (Error& err)
	{
		errBox( "ERROR: Failed to initialize intensity agilent: " + err.trace() );
	}
	SetRedraw( true );
	return TRUE;
}

void ScriptingWindow::setMenuCheck ( UINT menuItem, UINT itemState )
{
	menu.CheckMenuItem ( menuItem, itemState );
}


void ScriptingWindow::fillMasterThreadInput( ExperimentThreadInput* input )
{
	input->agilents.push_back( &intensityAgilent );
	input->intensityAgilentNumber = input->agilents.size() - 1;
}


void ScriptingWindow::OnTimer(UINT_PTR eventID)
{
	intensityAgilent.agilentScript.handleTimerCall(auxWin->getAllVariables(), auxWin->getTtlNames(), auxWin->getDacInfo ());
	niawgScript.handleTimerCall(auxWin->getAllVariables(), auxWin->getTtlNames(), auxWin->getDacInfo());
	masterScript.handleTimerCall(auxWin->getAllVariables(), auxWin->getTtlNames(), auxWin->getDacInfo ());
}


void ScriptingWindow::checkScriptSaves()
{
	niawgScript.checkSave(getProfile().categoryPath, mainWin->getRunInfo() );
	intensityAgilent.checkSave ( getProfile ( ).categoryPath, mainWin->getRunInfo ( ) );
	masterScript.checkSave( getProfile( ).categoryPath, mainWin->getRunInfo( ), comm ( ) );
}


std::string ScriptingWindow::getSystemStatusString()
{
	return "Intensity Agilent:\n\t" + intensityAgilent.getDeviceIdentity();	
}


void ScriptingWindow::loadFriends(MainWindow* mainWin_, AndorWindow* camWin_, AuxiliaryWindow* auxWin_, BaslerWindow* basWin_)
{
	mainWin = mainWin_;
	camWin = camWin_;
	auxWin = auxWin_;
	basWin = basWin_;
}

/*
	This function retuns the names (just the names) of currently active scripts.
*/
scriptInfo<std::string> ScriptingWindow::getScriptNames()
{
	scriptInfo<std::string> names;
	names.niawg = niawgScript.getScriptName();
	names.intensityAgilent = intensityAgilent.agilentScript.getScriptName();
	names.master = masterScript.getScriptName( );
	return names;
}

/*
	This function returns indicators of whether a given script has been saved or not.
*/
scriptInfo<bool> ScriptingWindow::getScriptSavedStatuses()
{
	scriptInfo<bool> status;
	status.niawg = niawgScript.savedStatus();
	status.intensityAgilent = intensityAgilent.agilentScript.savedStatus();
	status.master = masterScript.savedStatus( );
	return status;
}

/*
	This function returns the current addresses of all files in all scripts.
*/
scriptInfo<std::string> ScriptingWindow::getScriptAddresses()
{
	scriptInfo<std::string> addresses;
	addresses.niawg = niawgScript.getScriptPathAndName();
	addresses.intensityAgilent = intensityAgilent.agilentScript.getScriptPathAndName();
	addresses.master = masterScript.getScriptPathAndName();
	return addresses;
}

/*
	This function handles the coloring of all controls on this window.
*/
HBRUSH ScriptingWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{
			int num = ( pWnd->GetDlgCtrlID ( ) );
			HBRUSH result = intensityAgilent.handleColorMessage ( pWnd, pDC );
			if ( result != NULL )
			{
				return result;
			}
			CBrush* resultc = statusBox.handleColoring(num, pDC );
			if ( resultc )
			{
				return *resultc;
			}
			pDC->SetTextColor( _myRGBs["Text"]);
			pDC->SetBkColor( _myRGBs["Static-Bkgd"] );
			return *_myBrushes["Static-Bkgd"];
		}
		case CTLCOLOR_EDIT:
		{
			HBRUSH result = intensityAgilent.handleColorMessage ( pWnd, pDC );
			if ( result != NULL )
			{
				return result;
			}
			pDC->SetTextColor ( _myRGBs[ "ScriptWin-Text" ] );
			pDC->SetBkColor ( _myRGBs[ "Interactable-Bkgd" ] );
			return *_myBrushes[ "Interactable-Bkgd" ];
		}

		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor( _myRGBs["ScriptWin-Text"]);
			pDC->SetBkColor( _myRGBs["Interactable-Bkgd"]);
			return *_myBrushes["Interactable-Bkgd"];
		}
		default:
		{
			return *_myBrushes["Main-Bkgd"];
		}
	}
}


void ScriptingWindow::setIntensityDefault()
{
	try
	{
		intensityAgilent.setDefault( 1 );
	}
	catch ( Error& err )
	{
		comm( )->sendError( err.trace( ) );
	}
}


void ScriptingWindow::niawgEditChange()
{
	niawgScript.handleEditChange( );
	SetTimer( SYNTAX_TIMER_ID, SYNTAX_TIMER_LENGTH, NULL );
}

void ScriptingWindow::agilentEditChange()
{
	intensityAgilent.agilentScript.handleEditChange();
	SetTimer(SYNTAX_TIMER_ID, SYNTAX_TIMER_LENGTH, NULL);
}


/// Commonly Called Functions
/*
	The following set of functions, mostly revolving around saving etc. of the script files, are called by all of the
	window objects because they are associated with the menu at the top of each screen
*/
/// 
void ScriptingWindow::newIntensityScript()
{
	try
	{
		intensityAgilent.verifyScriptable ( );
		intensityAgilent.checkSave( getProfile().categoryPath, mainWin->getRunInfo() );
		intensityAgilent.agilentScript.newScript( );
		updateConfigurationSavedStatus( false );
		intensityAgilent.agilentScript.updateScriptNameText( mainWin->getProfileSettings().categoryPath );
		intensityAgilent.agilentScript.colorEntireScript( auxWin->getAllVariables(), 
														  auxWin->getTtlNames(), auxWin->getDacInfo () );
	}
	catch (Error& err)
	{
		comm()->sendError( err.trace() );
	}
}


void ScriptingWindow::openIntensityScript( CWnd* parent )
{
	try
	{
		intensityAgilent.verifyScriptable ( );
		intensityAgilent.checkSave( getProfile().categoryPath, mainWin->getRunInfo() );
		std::string intensityOpenName = openWithExplorer( parent, AGILENT_SCRIPT_EXTENSION );
		intensityAgilent.agilentScript.openParentScript( intensityOpenName, getProfile().categoryPath,
														 mainWin->getRunInfo() );
		updateConfigurationSavedStatus( false );
		intensityAgilent.agilentScript.updateScriptNameText( getProfile().categoryPath );
		intensityAgilent.agilentScript.colorEntireScript(auxWin->getAllVariables(), 
														  auxWin->getTtlNames(), 
														  auxWin->getDacInfo ());
	}
	catch (Error& err)
	{
		comm()->sendError( err.trace() );
	}
}

void ScriptingWindow::saveIntensityScript()
{
	try
	{
		// channel 0 is the intensity channel, the 4th option is the scripting option.
		if ( intensityAgilent.getOutputInfo( ).channel[0].option == AgilentChannelMode::which::Script )
		{
			intensityAgilent.agilentScript.saveScript( getProfile( ).categoryPath, mainWin->getRunInfo( ) );
			intensityAgilent.agilentScript.updateScriptNameText( getProfile( ).categoryPath );
		}
	}
	catch (Error& err)
	{
		comm()->sendError( err.trace() );
	}
}


void ScriptingWindow::saveIntensityScriptAs(CWnd* parent)
{
	try
	{
		intensityAgilent.verifyScriptable ( );
		std::string extensionNoPeriod = intensityAgilent.agilentScript.getExtension();
		if (extensionNoPeriod.size() == 0)
		{
			return;
		}
		extensionNoPeriod = extensionNoPeriod.substr( 1, extensionNoPeriod.size() );
		std::string newScriptAddress = saveWithExplorer( parent, extensionNoPeriod, getProfileSettings() );
		intensityAgilent.agilentScript.saveScriptAs( newScriptAddress, mainWin->getRunInfo() );
		updateConfigurationSavedStatus( false );
		intensityAgilent.agilentScript.updateScriptNameText( getProfile().categoryPath );
	}
	catch (Error& err)
	{
		comm()->sendError( err.trace() );
	}
} 
 

// just a quick shortcut.
profileSettings ScriptingWindow::getProfile()
{
	return mainWin->getProfileSettings();
}


void ScriptingWindow::newNiawgScript()
{
	try
	{
		niawgScript.checkSave( getProfile().categoryPath, mainWin->getRunInfo() );
		niawgScript.newScript( );
		updateConfigurationSavedStatus( false );
		niawgScript.updateScriptNameText( getProfile().categoryPath );
		niawgScript.colorEntireScript( auxWin->getAllVariables(), auxWin->getTtlNames(), auxWin->getDacInfo () );
	}
	catch (Error& err)
	{
		comm()->sendError( err.trace() );
	}
}


void ScriptingWindow::openNiawgScript(CWnd* parent)
{
	try
	{
		niawgScript.checkSave( getProfile().categoryPath, mainWin->getRunInfo() );
		std::string horizontalOpenName = openWithExplorer( parent, NIAWG_SCRIPT_EXTENSION );
		niawgScript.openParentScript( horizontalOpenName, getProfile().categoryPath, mainWin->getRunInfo() );
		updateConfigurationSavedStatus( false );
		niawgScript.updateScriptNameText( getProfile().categoryPath );
		niawgScript.colorEntireScript(auxWin->getAllVariables(), auxWin->getTtlNames(), auxWin->getDacInfo ());
	}
	catch (Error& err)
	{
		comm()->sendError( err.trace() );
	}

}


void ScriptingWindow::saveNiawgScript()
{
	try
	{
		niawgScript.saveScript( getProfile().categoryPath, mainWin->getRunInfo() );
		niawgScript.updateScriptNameText( getProfile().categoryPath );
	}
	catch (Error& err)
	{
		comm()->sendError( err.trace() );
	}
}


void ScriptingWindow::saveNiawgScriptAs(CWnd* parent)
{
	std::string extensionNoPeriod = niawgScript.getExtension();
	if (extensionNoPeriod.size() == 0)
	{
		return;
	}
	extensionNoPeriod = extensionNoPeriod.substr(1, extensionNoPeriod.size());
	std::string newScriptAddress = saveWithExplorer(parent, extensionNoPeriod, 
														getProfileSettings());
	niawgScript.saveScriptAs(newScriptAddress, mainWin->getRunInfo() );
	updateConfigurationSavedStatus(false);
	niawgScript.updateScriptNameText(getProfile().categoryPath);
}


void ScriptingWindow::updateScriptNamesOnScreen()
{
	niawgScript.updateScriptNameText(getProfile().categoryPath);
	niawgScript.updateScriptNameText(getProfile().categoryPath);
	intensityAgilent.agilentScript.updateScriptNameText(getProfile().categoryPath);
}


void ScriptingWindow::recolorScripts()
{
	niawgScript.colorEntireScript( auxWin->getAllVariables(), auxWin->getTtlNames(), auxWin->getDacInfo ());
	intensityAgilent.agilentScript.colorEntireScript(auxWin->getAllVariables(), auxWin->getTtlNames(), auxWin->getDacInfo ());
	masterScript.colorEntireScript(auxWin->getAllVariables(), auxWin->getTtlNames(), auxWin->getDacInfo ());
}


void ScriptingWindow::openIntensityScript(std::string name)
{
	intensityAgilent.agilentScript.openParentScript(name, getProfile().categoryPath, mainWin->getRunInfo());
}


void ScriptingWindow::handleOpenConfig(std::ifstream& configFile, Version ver)
{
	try
	{
		ProfileSystem::initializeAtDelim ( configFile, "SCRIPTS", ver );
	}
	catch ( Error&)
	{
		errBox ( "Failed to initialize configuration file at scripting window entry point \"SCRIPTS\"." );
		return;
	}		
	try
	{
		configFile.get ( );
		std::string niawgName, masterName;
		if ( ver.versionMajor < 3 )
		{
			std::string extraNiawgName;
			getline ( configFile, extraNiawgName );
		}
		getline ( configFile, niawgName );
		getline ( configFile, masterName );
		ProfileSystem::checkDelimiterLine ( configFile, "END_SCRIPTS" );
		ProfileSystem::standardOpenConfig ( configFile, intensityAgilent.configDelim, &intensityAgilent, Version("4.0") );
		intensityAgilent.updateSettingsDisplay ( 1, mainWin->getProfileSettings ( ).categoryPath,
												 mainWin->getRunInfo ( ) );
		try
		{
			openNiawgScript ( niawgName );
		}
		catch ( Error& err )
		{
			int answer = promptBox ( "ERROR: Failed to open NIAWG script file: " + niawgName + ", with error \r\n"
									 + err.trace ( ) + "\r\nAttempt to find file yourself?", MB_YESNO );
			if ( answer == IDYES )
			{
				openNiawgScript ( openWithExplorer ( NULL, "nScript" ) );
			}
		}
		try
		{
			openMasterScript ( masterName );
		}
		catch ( Error& err )
		{
			int answer = promptBox ( "ERROR: Failed to open master script file: " + masterName + ", with error \r\n"
									 + err.trace ( ) + "\r\nAttempt to find file yourself?", MB_YESNO );
			if ( answer == IDYES )
			{
				openMasterScript ( openWithExplorer ( NULL, "mScript" ) );
			}
		}
		considerScriptLocations ( );
		recolorScripts ( );
	}
	catch ( Error& e )
	{
		errBox ( "Scripting Window failed to read parameters from the configuration file.\n\n" + e.trace() );
	}
}


void ScriptingWindow::newMasterScript()
{
	masterScript.checkSave(getProfile().categoryPath, mainWin->getRunInfo());
	masterScript.newScript( );
	updateConfigurationSavedStatus(false);
	masterScript.updateScriptNameText(getProfile().categoryPath);
	masterScript.colorEntireScript(auxWin->getAllVariables(), auxWin->getTtlNames(), auxWin->getDacInfo ());
}

void ScriptingWindow::openMasterScript(CWnd* parent)
{
	try
	{
		masterScript.checkSave( getProfile( ).categoryPath, mainWin->getRunInfo( ) );
		std::string openName = openWithExplorer( parent, MASTER_SCRIPT_EXTENSION );
		masterScript.openParentScript( openName, getProfile( ).categoryPath, mainWin->getRunInfo( ) );
		updateConfigurationSavedStatus( false );
		masterScript.updateScriptNameText( getProfile( ).categoryPath );
		masterScript.colorEntireScript( auxWin->getAllVariables( ), auxWin->getTtlNames( ), auxWin->getDacInfo ( ) );
	}
	catch ( Error& err )
	{
		comm( )->sendError( "Open Master Script Failed: " + err.trace( ) + "\r\n" );
	}
}


void ScriptingWindow::saveMasterScript()
{
	if ( masterScript.isFunction ( ) )
	{
		masterScript.saveAsFunction ( comm () );
		return;
	}
	masterScript.saveScript(getProfile().categoryPath, mainWin->getRunInfo());
	masterScript.updateScriptNameText(getProfile().categoryPath);
}


void ScriptingWindow::saveMasterScriptAs(CWnd* parent)
{
	std::string extensionNoPeriod = masterScript.getExtension();
	if (extensionNoPeriod.size() == 0)
	{
		return;
	}
	extensionNoPeriod = extensionNoPeriod.substr(1, extensionNoPeriod.size());
	std::string newScriptAddress = saveWithExplorer(parent, extensionNoPeriod, getProfileSettings());
	masterScript.saveScriptAs(newScriptAddress, mainWin->getRunInfo());
	updateConfigurationSavedStatus(false);
	masterScript.updateScriptNameText(getProfile().categoryPath);
}


void ScriptingWindow::newMasterFunction()
{
	try
	{
		masterScript.newFunction();
	}
	catch (Error& exception)
	{
		comm()->sendError("New Master function Failed: " + exception.trace() + "\r\n");
	}
}


void ScriptingWindow::saveMasterFunction()
{
	try
	{
		masterScript.saveAsFunction(comm());
	}
	catch (Error& exception)
	{
		comm()->sendError("Save Master Script Function Failed: " + exception.trace() + "\r\n");
	}
}


void ScriptingWindow::deleteMasterFunction()
{
	// todo. Right now you can just delete the file itself...
}


void ScriptingWindow::handleNewConfig( std::ofstream& saveFile )
{
	saveFile << "SCRIPTS\n";
	saveFile << "NONE" << "\n";
	saveFile << "NONE" << "\n";
	saveFile << "NONE" << "\n";
	saveFile << "END_SCRIPTS\n";
	intensityAgilent.handleNewConfig( saveFile );
}


void ScriptingWindow::handleSavingConfig(std::ofstream& saveFile)
{
	scriptInfo<std::string> addresses = getScriptAddresses();
	// order matters!
	saveFile << "SCRIPTS\n";
	saveFile << addresses.niawg << "\n";
	saveFile << addresses.master << "\n";
	saveFile << "END_SCRIPTS\n";
	intensityAgilent.handleSavingConfig(saveFile, mainWin->getProfileSettings().categoryPath, 
										 mainWin->getRunInfo());
}


void ScriptingWindow::checkMasterSave()
{
	masterScript.checkSave(getProfile().categoryPath, mainWin->getRunInfo());
}


void ScriptingWindow::openMasterScript(std::string name)
{
	masterScript.openParentScript(name, getProfile().categoryPath, mainWin->getRunInfo());
}


void ScriptingWindow::openNiawgScript(std::string name)
{
	niawgScript.openParentScript(name, getProfile().categoryPath, mainWin->getRunInfo());
}


void ScriptingWindow::considerScriptLocations()
{
	niawgScript.considerCurrentLocation(getProfile().categoryPath, mainWin->getRunInfo());
	intensityAgilent.agilentScript.considerCurrentLocation(getProfile().categoryPath, mainWin->getRunInfo());
}


/// End common functions
void ScriptingWindow::passCommonCommand(UINT id)
{
	// pass the command id to the common function, filling in the pointers to the windows which own objects needed.
	commonFunctions::handleCommonMessage( id, this, mainWin, this, camWin, auxWin, basWin );
}


void ScriptingWindow::changeBoxColor( systemInfo<char> colors )
{
	statusBox.changeColor(colors);
}


void ScriptingWindow::updateProfile(std::string text)
{
	profileDisplay.update(text);
}


profileSettings ScriptingWindow::getProfileSettings()
{	
	return mainWin->getProfileSettings();
}


void ScriptingWindow::updateConfigurationSavedStatus(bool status)
{
	mainWin->updateConfigurationSavedStatus(status);
}

