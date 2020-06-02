// created by Mark O. Brown
#include "stdafx.h"

#include "afxwin.h"
#include "GeneralUtilityFunctions/commonFunctions.h"
#include "ExcessDialogs/openWithExplorer.h"
#include "ExcessDialogs/saveWithExplorer.h"
#include "PrimaryWindows/AuxiliaryWindow.h"
#include "PrimaryWindows/AndorWindow.h"
#include "PrimaryWindows/ScriptingWindow.h"
#include "PrimaryWindows/BaslerWindow.h"
#include "PrimaryWindows/MainWindow.h"
#include "Agilent/Agilent.h"
#include "Agilent/AgilentSettings.h"

ScriptingWindow::ScriptingWindow() : IChimeraWindow(),
intensityAgilent( INTENSITY_AGILENT_SETTINGS ), 
niawg (DoRows::which::B, 14, NIAWG_SAFEMODE)
{
	statBox = new ColorBox ();
}

IMPLEMENT_DYNAMIC(ScriptingWindow, IChimeraWindow)

BEGIN_MESSAGE_MAP(ScriptingWindow, IChimeraWindow)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_WM_SIZE()

	ON_EN_CHANGE( IDC_NIAWG_EDIT, &niawgEditChange)
	ON_EN_CHANGE( IDC_INTENSITY_EDIT, &agilentEditChange)
	ON_EN_CHANGE( IDC_MASTER_EDIT, &masterEditChange)

	ON_COMMAND (IDC_CONTROL_NIAWG_CHECK, &handleControlNiawgCheck)

	ON_COMMAND_RANGE( IDC_INTENSITY_CHANNEL1_BUTTON, IDC_INTENSITY_PROGRAM, &handleIntensityButtons)
	ON_CBN_SELENDOK( IDC_INTENSITY_AGILENT_COMBO, &handleIntensityCombo )

	ON_CBN_SELENDOK( IDC_NIAWG_FUNCTION_COMBO, &handleNiawgScriptComboChange)
	ON_CBN_SELENDOK( IDC_INTENSITY_FUNCTION_COMBO, &handleAgilentScriptComboChange)
	
	ON_CBN_SELENDOK( IDC_MASTER_FUNCTION_COMBO, &handleMasterFunctionChange )
	ON_COMMAND (IDC_RERNG_EXPERIMENT_BUTTON, &passExperimentRerngButton)
	ON_CBN_SELENDOK (IDC_RERNG_MODE_COMBO, &passRerngModeComboChange)

	ON_WM_RBUTTONUP( )
	ON_WM_LBUTTONUP( )
	ON_NOTIFY_EX_RANGE( TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText )
END_MESSAGE_MAP()


void ScriptingWindow::handleControlNiawgCheck ()
{
	niawg.updateWindowEnabled ();
}

void ScriptingWindow::loadCameraCalSettings (ExperimentThreadInput* input)
{
}


void ScriptingWindow::OnRButtonUp( UINT stuff, CPoint clickLocation )
{
	andorWin->stopSound( );
}

void ScriptingWindow::OnLButtonUp( UINT stuff, CPoint clickLocation )
{
	andorWin->stopSound( );
}



void ScriptingWindow::handleMasterFunctionChange( )
{
	try
	{
		masterScript.functionChangeHandler(mainWin->getProfileSettings().configLocation);
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
	intensityAgilent.checkSave (mainWin->getProfileSettings ().configLocation, mainWin->getRunInfo ());
	intensityAgilent.readGuiSettings(  );
	intensityAgilent.handleModeCombo();
	intensityAgilent.updateSettingsDisplay(mainWin->getProfileSettings().configLocation, mainWin->getRunInfo());
}


void ScriptingWindow::handleIntensityButtons( UINT id )
{
	id -= IDC_INTENSITY_CHANNEL1_BUTTON;
	if (id % 7 == 0)
	{
		// channel 1
		intensityAgilent.handleChannelPress( 1, mainWin->getProfileSettings().configLocation, 
											 mainWin->getRunInfo() );
	}
	else if (id % 7 == 1)
	{
		// channel 2
		intensityAgilent.handleChannelPress( 2, mainWin->getProfileSettings().configLocation,
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
			intensityAgilent.checkSave (mainWin->getProfileSettings ().configLocation, mainWin->getRunInfo ());
			intensityAgilent.programAgilentNow(auxWin->getUsableConstants());
			reportStatus( "Programmed Agilent " + intensityAgilent.getConfigDelim()+ ".\r\n" );
		}
		catch (Error& err)
		{
			reportErr( "Error while programming agilent " + intensityAgilent.getConfigDelim() + ": " + err.trace() 
					   + "\r\n" );
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
		reportErr(err.trace());
	}
}


void ScriptingWindow::OnSize(UINT nType, int cx, int cy)
{
	bool intSaved = intensityAgilent.getSavedStatus(), niawgSaved = niawg.niawgScript.savedStatus (), masterSaved = masterScript.savedStatus();
	
	SetRedraw( false );
	
	intensityAgilent.rearrange( cx, cy, mainWin->getFonts() );
	masterScript.rearrange(cx, cy, mainWin->getFonts());
	statBox->rearrange( cx, cy, mainWin->getFonts());
	profileDisplay.rearrange(cx, cy, mainWin->getFonts());
	niawg.rearrange (cx, cy, mainWin->getFonts ());
	recolorScripts ( );
	SetRedraw( true );
	RedrawWindow();
	intensityAgilent.updateSavedStatus (intSaved);
	niawg.niawgScript.updateSavedStatus (niawgSaved);
	masterScript.updateSavedStatus (masterSaved);
}

// special handling for long tooltips.
BOOL ScriptingWindow::OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
	try
	{
		niawg.niawgScript.handleToolTip( pNMHDR , pResult);
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
	// don't redraw until the first OnSize.
	qtp = new QWinWidget ((CWnd*)this);
	qtp->setStyleSheet (mainWin->getStyleSheet ());
	qtp->move (0, 0);
	int id = 2000;
	/// initialize niawg.
	try	{
		niawg.core.initialize ();
	}
	catch (Error & except)	{
		errBox ("NIAWG failed to Initialize! Error: " + except.trace ());
	}
	try	{
		niawg.core.setDefaultWaveforms ();
		// but the default starts in the horizontal configuration, so switch back and start in this config.
		restartNiawgDefaults ();
	}
	catch (Error & exception)	{
		errBox ( "Failed to start niawg default waveforms! Niawg gave the following error message: " 
				 + exception.trace () );
	}
	POINT startLocation = { 0, 25 };
	//niawg.initialize (startLocation, this);
	niawg.niawgScript.setEnabled ( true, false );
	startLocation = { 640, 25 };
	
	intensityAgilent.initialize( startLocation, "Tweezer Intensity Agilent", 865, this, 640 );
	startLocation = { 2*640, 25 };
	masterScript.initialize( 640, 900, startLocation, this, "Master", "Master Script" );
	startLocation = { 1200, 3 };
	statBox->initialize(startLocation, this, 700, mainWin->getDevices ());
	//profileDisplay.initialize({ 0,3 }, this);
	qtp->show ();
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
	return IChimeraWindow::OnInitDialog ();
}

void ScriptingWindow::fillMotInput (ExperimentThreadInput* input){}
void ScriptingWindow::fillMasterThreadInput( ExperimentThreadInput* input ){}


void ScriptingWindow::OnTimer(UINT_PTR eventID)
{
	try
	{
		intensityAgilent.agilentScript.handleTimerCall (auxWin->getAllVariables (), auxWin->getTtlNames (), auxWin->getDacInfo ());
		niawg.niawgScript.handleTimerCall (auxWin->getAllVariables (), auxWin->getTtlNames (), auxWin->getDacInfo ());
		masterScript.handleTimerCall (auxWin->getAllVariables (), auxWin->getTtlNames (), auxWin->getDacInfo ());
	}
	catch (Error & err)
	{
		reportErr(err.trace ());
	}
}


void ScriptingWindow::checkScriptSaves()
{
	niawg.niawgScript.checkSave(getProfile().configLocation, mainWin->getRunInfo() );
	intensityAgilent.checkSave ( getProfile ( ).configLocation, mainWin->getRunInfo ( ) );
	masterScript.checkSave( getProfile( ).configLocation, mainWin->getRunInfo( ), mainWin->getComm ( ) );
}


std::string ScriptingWindow::getSystemStatusString()
{
	std::string status = "Intensity Agilent:\n\t" + intensityAgilent.getDeviceIdentity();	
	status = "NIAWG:\n";
	if (!NIAWG_SAFEMODE)
	{
		status += "\tCode System is Active!\n";
		try
		{
			status += "\t" + niawg.core.fgenConduit.getDeviceInfo ();
		}
		catch (Error & err)
		{
			status += "\tFailed to get device info! Error: " + err.trace ();
		}
	}
	else
	{
		status += "\tCode System is disabled! Enable in \"constants.h\"\n";
	}
	return status;
}

void ScriptingWindow::sendNiawgSoftwareTrig ()
{
	niawg.core.fgenConduit.sendSoftwareTrigger ();
}

void ScriptingWindow::streamNiawgWaveform ()
{
	niawg.core.streamWaveform ();
}


/*
	This function retuns the names (just the names) of currently active scripts.
*/
scriptInfo<std::string> ScriptingWindow::getScriptNames()
{
	scriptInfo<std::string> names;
	names.niawg = niawg.niawgScript.getScriptName();
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
	status.niawg = niawg.niawgScript.savedStatus();
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
	addresses.niawg = niawg.niawgScript.getScriptPathAndName();
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
		}
		case CTLCOLOR_EDIT:
		{
			HBRUSH result = intensityAgilent.handleColorMessage ( pWnd, pDC );
			if ( result != NULL )
			{
				return result;
			}
		}
	}
	return IChimeraWindow::OnCtlColor (pDC, pWnd, nCtlColor);
}


void ScriptingWindow::setIntensityDefault()
{
	try
	{
		intensityAgilent.setDefault( 1 );
	}
	catch ( Error& err )
	{
		reportErr( err.trace( ) );
	}
}


void ScriptingWindow::niawgEditChange()
{
	niawg.niawgScript.handleEditChange( );
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
		intensityAgilent.checkSave( getProfile().configLocation, mainWin->getRunInfo() );
		intensityAgilent.agilentScript.newScript( );
		updateConfigurationSavedStatus( false );
		intensityAgilent.agilentScript.updateScriptNameText( mainWin->getProfileSettings().configLocation );
		intensityAgilent.agilentScript.colorEntireScript( auxWin->getAllVariables(), 
														  auxWin->getTtlNames(), auxWin->getDacInfo () );
	}
	catch (Error& err)
	{
		reportErr( err.trace() );
	}
}


void ScriptingWindow::openIntensityScript( CWnd* parent )
{
	try
	{
		intensityAgilent.verifyScriptable ( );
		intensityAgilent.checkSave( getProfile().configLocation, mainWin->getRunInfo() );
		std::string intensityOpenName = openWithExplorer( parent, AGILENT_SCRIPT_EXTENSION );
		intensityAgilent.agilentScript.openParentScript( intensityOpenName, getProfile().configLocation,
														 mainWin->getRunInfo() );
		updateConfigurationSavedStatus( false );
		intensityAgilent.agilentScript.updateScriptNameText( getProfile().configLocation );
		intensityAgilent.agilentScript.colorEntireScript(auxWin->getAllVariables(), 
														  auxWin->getTtlNames(), 
														  auxWin->getDacInfo ());
	}
	catch (Error& err)
	{
		reportErr( err.trace() );
	}
}

void ScriptingWindow::saveIntensityScript()
{
	try
	{
		// channel 0 is the intensity channel, the 4th option is the scripting option.
		if ( intensityAgilent.getOutputInfo( ).channel[0].option == AgilentChannelMode::which::Script )
		{
			intensityAgilent.agilentScript.saveScript( getProfile( ).configLocation, mainWin->getRunInfo( ) );
			intensityAgilent.agilentScript.updateScriptNameText( getProfile( ).configLocation );
		}
	}
	catch (Error& err)
	{
		reportErr( err.trace() );
	}
}


void ScriptingWindow::saveIntensityScriptAs(CWnd* parent )
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
		intensityAgilent.agilentScript.updateScriptNameText( getProfile().configLocation );
	}
	catch (Error& err)
	{
		reportErr( err.trace() );
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
		niawg.niawgScript.checkSave( getProfile().configLocation, mainWin->getRunInfo() );
		niawg.niawgScript.newScript( );
		updateConfigurationSavedStatus( false );
		niawg.niawgScript.updateScriptNameText( getProfile().configLocation );
		niawg.niawgScript.colorEntireScript( auxWin->getAllVariables(), auxWin->getTtlNames(), auxWin->getDacInfo () );
	}
	catch (Error& err)
	{
		reportErr( err.trace() );
	}
}


void ScriptingWindow::openNiawgScript(CWnd* parent )
{
	try
	{
		niawg.niawgScript.checkSave( getProfile().configLocation, mainWin->getRunInfo() );
		std::string horizontalOpenName = openWithExplorer( parent, NIAWG_SCRIPT_EXTENSION );
		niawg.niawgScript.openParentScript( horizontalOpenName, getProfile().configLocation, mainWin->getRunInfo() );
		updateConfigurationSavedStatus( false );
		niawg.niawgScript.updateScriptNameText( getProfile().configLocation );
		niawg.niawgScript.colorEntireScript(auxWin->getAllVariables(), auxWin->getTtlNames(), auxWin->getDacInfo ());
	}
	catch (Error& err)
	{
		reportErr( err.trace() );
	}

}


void ScriptingWindow::saveNiawgScript()
{
	try
	{
		niawg.niawgScript.saveScript( getProfile().configLocation, mainWin->getRunInfo() );
		niawg.niawgScript.updateScriptNameText( getProfile().configLocation );
	}
	catch (Error& err)
	{
		reportErr( err.trace() );
	}
}


void ScriptingWindow::saveNiawgScriptAs(CWnd* parent )
{
	std::string extensionNoPeriod = niawg.niawgScript.getExtension();
	if (extensionNoPeriod.size() == 0)
	{
		return;
	}
	extensionNoPeriod = extensionNoPeriod.substr(1, extensionNoPeriod.size());
	std::string newScriptAddress = saveWithExplorer(parent, extensionNoPeriod, 
														getProfileSettings());
	niawg.niawgScript.saveScriptAs(newScriptAddress, mainWin->getRunInfo() );
	updateConfigurationSavedStatus(false);
	niawg.niawgScript.updateScriptNameText(getProfile().configLocation);
}


void ScriptingWindow::updateScriptNamesOnScreen()
{
	niawg.niawgScript.updateScriptNameText(getProfile().configLocation);
	niawg.niawgScript.updateScriptNameText(getProfile().configLocation);
	intensityAgilent.agilentScript.updateScriptNameText(getProfile().configLocation);
}


void ScriptingWindow::recolorScripts()
{
	niawg.niawgScript.colorEntireScript( auxWin->getAllVariables(), auxWin->getTtlNames(), auxWin->getDacInfo ());
	intensityAgilent.agilentScript.colorEntireScript(auxWin->getAllVariables(), auxWin->getTtlNames(), auxWin->getDacInfo ());
	masterScript.colorEntireScript(auxWin->getAllVariables(), auxWin->getTtlNames(), auxWin->getDacInfo ());
}


void ScriptingWindow::openIntensityScript(std::string name)
{
	intensityAgilent.agilentScript.openParentScript(name, getProfile().configLocation, mainWin->getRunInfo());
}


void ScriptingWindow::windowOpenConfig(ConfigStream& configFile)
{
	try
	{
		ProfileSystem::initializeAtDelim ( configFile, "SCRIPTS");
	}
	catch ( Error&)
	{
		errBox ( "Failed to initialize configuration file at scripting window entry point \"SCRIPTS\"." );
		return;
	}		
	try
	{
		configFile.get ( );
		auto getlineFunc = ProfileSystem::getGetlineFunc (configFile.ver);
		std::string niawgName, masterName;
		if ( configFile.ver.versionMajor < 3 )
		{
			std::string extraNiawgName;
			getlineFunc (configFile, extraNiawgName);
		}
		getlineFunc (configFile, niawgName);
		getlineFunc (configFile, masterName);
		ProfileSystem::checkDelimiterLine ( configFile, "END_SCRIPTS" );
		deviceOutputInfo info;
		ProfileSystem::stdGetFromConfig ( configFile, intensityAgilent.getCore(), info, Version ("4.0") );
		intensityAgilent.setOutputSettings (info);
		intensityAgilent.updateSettingsDisplay (1, mainWin->getProfileSettings ().configLocation, mainWin->getRunInfo ());
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
		niawg.handleOpenConfig (configFile);
		niawg.updateWindowEnabled ();
	}
	catch ( Error& e )
	{
		errBox ( "Scripting Window failed to read parameters from the configuration file.\n\n" + e.trace() );
	}
}


void ScriptingWindow::newMasterScript()
{
	masterScript.checkSave(getProfile().configLocation, mainWin->getRunInfo());
	masterScript.newScript( );
	updateConfigurationSavedStatus(false);
	masterScript.updateScriptNameText(getProfile().configLocation);
	masterScript.colorEntireScript(auxWin->getAllVariables(), auxWin->getTtlNames(), auxWin->getDacInfo ());
}

void ScriptingWindow::openMasterScript(CWnd* parent )
{
	try
	{
		masterScript.checkSave( getProfile( ).configLocation, mainWin->getRunInfo( ) );
		std::string openName = openWithExplorer( parent, MASTER_SCRIPT_EXTENSION );
		masterScript.openParentScript( openName, getProfile( ).configLocation, mainWin->getRunInfo( ) );
		updateConfigurationSavedStatus( false );
		masterScript.updateScriptNameText( getProfile( ).configLocation );
		masterScript.colorEntireScript( auxWin->getAllVariables( ), auxWin->getTtlNames( ), auxWin->getDacInfo ( ) );
	}
	catch ( Error& err )
	{
		reportErr( "Open Master Script Failed: " + err.trace( ) + "\r\n" );
	}
}


void ScriptingWindow::saveMasterScript()
{
	if ( masterScript.isFunction ( ) )
	{
		masterScript.saveAsFunction ( mainWin->getComm() );
		return;
	}
	masterScript.saveScript(getProfile().configLocation, mainWin->getRunInfo());
	masterScript.updateScriptNameText(getProfile().configLocation);
}


void ScriptingWindow::saveMasterScriptAs(CWnd* parent )
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
	masterScript.updateScriptNameText(getProfile().configLocation);
}


void ScriptingWindow::newMasterFunction()
{
	try
	{
		masterScript.newFunction();
	}
	catch (Error& exception)
	{
		reportErr("New Master function Failed: " + exception.trace() + "\r\n");
	}
}


void ScriptingWindow::saveMasterFunction()
{
	try
	{
		masterScript.saveAsFunction(mainWin->getComm());
	}
	catch (Error& exception)
	{
		reportErr("Save Master Script Function Failed: " + exception.trace() + "\r\n");
	}
}


void ScriptingWindow::deleteMasterFunction()
{
	// todo. Right now you can just delete the file itself...
}


void ScriptingWindow::windowSaveConfig(ConfigStream& saveFile)
{
	scriptInfo<std::string> addresses = getScriptAddresses();
	// order matters!
	saveFile << "SCRIPTS\n";
	saveFile << "/*NIAWG Script Address:*/ " << addresses.niawg << "\n";
	saveFile << "/*Master Script Address:*/ " << addresses.master << "\n";
	saveFile << "END_SCRIPTS\n";
	intensityAgilent.handleSavingConfig(saveFile, mainWin->getProfileSettings().configLocation, mainWin->getRunInfo());
	niawg.handleSaveConfig (saveFile);
}


void ScriptingWindow::checkMasterSave()
{
	masterScript.checkSave(getProfile().configLocation, mainWin->getRunInfo());
}


void ScriptingWindow::openMasterScript(std::string name)
{
	masterScript.openParentScript(name, getProfile().configLocation, mainWin->getRunInfo());
}


void ScriptingWindow::openNiawgScript(std::string name)
{
	niawg.niawgScript.openParentScript(name, getProfile().configLocation, mainWin->getRunInfo());
}


void ScriptingWindow::considerScriptLocations()
{
	niawg.niawgScript.considerCurrentLocation(getProfile().configLocation, mainWin->getRunInfo());
	intensityAgilent.agilentScript.considerCurrentLocation(getProfile().configLocation, mainWin->getRunInfo());
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

void ScriptingWindow::setNiawgRunningState (bool newRunningState)
{
	niawg.core.setRunningState (newRunningState);
}

bool ScriptingWindow::niawgIsRunning () { return niawg.core.niawgIsRunning (); }
void ScriptingWindow::setNiawgDefaults () { niawg.core.setDefaultWaveforms (); }
void ScriptingWindow::restartNiawgDefaults () { niawg.core.restartDefault (); }
NiawgCore& ScriptingWindow::getNiawg () { return niawg.core; }
void ScriptingWindow::stopRearranger () { niawg.core.turnOffRerng (); }
void ScriptingWindow::waitForRearranger () { niawg.core.waitForRerng (true); }
void ScriptingWindow::stopNiawg () { niawg.core.turnOff (); }

void ScriptingWindow::passNiawgIsOnPress ()
{
	if (niawg.core.isOn ())
	{
		niawg.core.turnOff ();
		mainWin->checkAllMenus (ID_NIAWG_NIAWGISON, MF_UNCHECKED);
	}
	else
	{
		niawg.core.turnOn ();
		mainWin->checkAllMenus (ID_NIAWG_NIAWGISON, MF_CHECKED);
	}
}

std::string ScriptingWindow::getNiawgErr ()
{
	return niawg.core.fgenConduit.getErrorMsg ();
}

void ScriptingWindow::passRerngModeComboChange () 
{
	niawg.rearrangeCtrl.updateActive (); 
}

void ScriptingWindow::passExperimentRerngButton ()
{
	niawg.rearrangeCtrl.updateActive ();
}

void ScriptingWindow::fillExpDeviceList (DeviceList& list)
{
	list.list.push_back (niawg.core);
	list.list.push_back (intensityAgilent.getCore ());
}
