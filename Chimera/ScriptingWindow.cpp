#include "stdafx.h"
#include "ScriptingWindow.h"
#include "afxwin.h"
#include "MainWindow.h"
#include "openWithExplorer.h"
#include "saveTextFileFromEdit.h"
#include "commonFunctions.h"
#include "textPromptDialog.h"
#include "AuxiliaryWindow.h"


ScriptingWindow::ScriptingWindow() : CDialog(), intensityAgilent(INTENSITY_SAFEMODE, INTENSITY_AGILENT_USB_ADDRESS)
{}

IMPLEMENT_DYNAMIC(ScriptingWindow, CDialog)

BEGIN_MESSAGE_MAP(ScriptingWindow, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_WM_SIZE()

	ON_EN_CHANGE(IDC_HORIZONTAL_NIAWG_EDIT, &ScriptingWindow::horizontalEditChange)
	ON_EN_CHANGE(IDC_VERTICAL_NIAWG_EDIT, &ScriptingWindow::verticalEditChange)
	ON_EN_CHANGE(IDC_INTENSITY_EDIT, &ScriptingWindow::agilentEditChange)
	ON_EN_CHANGE(IDC_MASTER_EDIT, &ScriptingWindow::masterEditChange)

	ON_COMMAND(IDOK, &ScriptingWindow::catchEnter)

	ON_COMMAND_RANGE(IDC_INTENSITY_CHANNEL1_BUTTON, IDC_INTENSITY_PROGRAM, &ScriptingWindow::handleIntensityButtons)
	ON_CBN_SELENDOK( IDC_INTENSITY_AGILENT_COMBO, &ScriptingWindow::handleIntensityCombo )


	ON_COMMAND_RANGE(MENU_ID_RANGE_BEGIN, MENU_ID_RANGE_END, &ScriptingWindow::passCommonCommand)

	ON_CBN_SELENDOK(IDC_VERTICAL_NIAWG_FUNCTION_COMBO, &ScriptingWindow::handleVerticalScriptComboChange)
	ON_CBN_SELENDOK(IDC_HORIZONTAL_NIAWG_FUNCTION_COMBO, &ScriptingWindow::handleHorizontalScriptComboChange)
	ON_CBN_SELENDOK(IDC_INTENSITY_FUNCTION_COMBO, &ScriptingWindow::handleAgilentScriptComboChange)
	
	ON_CBN_SELENDOK( IDC_MASTER_FUNCTION_COMBO, &ScriptingWindow::handleMasterFunctionChange )

	ON_NOTIFY_EX_RANGE( TTN_NEEDTEXTA, 0, 0xFFFF, ScriptingWindow::OnToolTipText )
END_MESSAGE_MAP()


void ScriptingWindow::handleMasterFunctionChange( )
{
	try
	{
		masterScript.functionChangeHandler(mainWindowFriend->getProfileSettings().categoryPath);
		masterScript.colorEntireScript( auxWindowFriend->getAllVariables( ), mainWindowFriend->getRgbs( ),
										auxWindowFriend->getTtlNames( ), auxWindowFriend->getDacNames( ) );
		masterScript.updateSavedStatus( true );
	}
	catch ( Error& err )
	{
		errBox( err.what( ) );
	}
}


void ScriptingWindow::handleIntensityCombo()
{
	intensityAgilent.handleInput( mainWindowFriend->getProfileSettings().categoryPath, mainWindowFriend->getRunInfo() );
	intensityAgilent.handleCombo();
	intensityAgilent.updateSettingsDisplay(mainWindowFriend->getProfileSettings().categoryPath, mainWindowFriend->getRunInfo());
}


void ScriptingWindow::handleIntensityButtons( UINT id )
{
	id -= IDC_INTENSITY_CHANNEL1_BUTTON;
	if (id % 7 == 0)
	{
		// channel 1
		intensityAgilent.handleChannelPress( 1, mainWindowFriend->getProfileSettings().categoryPath, 
											 mainWindowFriend->getRunInfo() );
	}
	else if (id % 7 == 1)
	{
		// channel 2
		intensityAgilent.handleChannelPress( 2, mainWindowFriend->getProfileSettings().categoryPath,
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
			intensityAgilent.handleInput(mainWindowFriend->getProfileSettings().categoryPath, mainWindowFriend->getRunInfo());
			intensityAgilent.setAgilent();
			comm()->sendStatus( "Programmed Agilent " + intensityAgilent.getName() + ".\r\n" );
		}
		catch (Error& err)
		{
			comm()->sendError( "Error while programming agilent " + intensityAgilent.getName()
													+ ": " + err.what() + "\r\n" );
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
		comm()->sendError(err.what());
	}
}


Communicator* ScriptingWindow::comm()
{
	return mainWindowFriend->getComm();
}

void ScriptingWindow::catchEnter()
{
	errBox("Secret Message!");
}


void ScriptingWindow::OnSize(UINT nType, int cx, int cy)
{
	SetRedraw( false );
	verticalNiawgScript.rearrange(cx, cy, mainWindowFriend->getFonts());
	horizontalNiawgScript.rearrange(cx, cy, mainWindowFriend->getFonts());
	intensityAgilent.rearrange( cx, cy, mainWindowFriend->getFonts() );
	masterScript.rearrange(cx, cy, mainWindowFriend->getFonts());
	statusBox.rearrange( cx, cy, mainWindowFriend->getFonts());
	profileDisplay.rearrange(cx, cy, mainWindowFriend->getFonts());
	verticalNiawgScript.colorEntireScript(auxWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(), 
										  auxWindowFriend->getTtlNames(), auxWindowFriend->getDacNames());
	horizontalNiawgScript.colorEntireScript(auxWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
											auxWindowFriend->getTtlNames(), auxWindowFriend->getDacNames());
	
	intensityAgilent.agilentScript.colorEntireScript(auxWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
											 auxWindowFriend->getTtlNames(), auxWindowFriend->getDacNames());
	masterScript.colorEntireScript(auxWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
								   auxWindowFriend->getTtlNames(), auxWindowFriend->getDacNames());
	SetRedraw( true );
	RedrawWindow();
}

// special handling for long tooltips.
BOOL ScriptingWindow::OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
	try
	{
		verticalNiawgScript.handleToolTip( pNMHDR , pResult);
		horizontalNiawgScript.handleToolTip( pNMHDR, pResult );
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


void ScriptingWindow::handleHorizontalScriptComboChange()
{
	//horizontalNiawgScript.childComboChangeHandler();
}


void ScriptingWindow::handleVerticalScriptComboChange()
{
	//verticalNiawgScript.childComboChangeHandler( mainWindowFriend, auxWindowFriend );
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
	EnableToolTips( TRUE );
	// don't redraw until the first OnSize.
	SetRedraw( false );

	int id = 2000;

	POINT startLocation = { 0, 28 };
	verticalNiawgScript.initialize( 480, 900, startLocation, tooltips, this, id, "Vertical NIAWG", 
									"Vertical NIAWG Script",
									{ IDC_VERTICAL_NIAWG_FUNCTION_COMBO, IDC_VERTICAL_NIAWG_EDIT }, 
									mainWindowFriend->getRgbs()["Solarized Base04"] );

	startLocation = { 480, 28 };
	horizontalNiawgScript.initialize( 480, 900, startLocation, tooltips, this,  id, "Horizontal NIAWG",
									  "Horizontal NIAWG Script", { IDC_HORIZONTAL_NIAWG_FUNCTION_COMBO, 
									  IDC_HORIZONTAL_NIAWG_EDIT }, mainWindowFriend->getRgbs()["Solarized Base03"]);
	startLocation = { 960, 28 };
	intensityAgilent.initialize( startLocation, tooltips, this, id, 
								 "Intensity Agilent", 865, { IDC_INTENSITY_CHANNEL1_BUTTON,
								 IDC_INTENSITY_CHANNEL2_BUTTON, IDC_INTENSITY_SYNC_BUTTON, 
								 IDC_INTENSITY_CALIBRATION_BUTTON, IDC_INTENSITY_PROGRAM, 
								 IDC_INTENSITY_AGILENT_COMBO, IDC_INTENSITY_FUNCTION_COMBO, IDC_INTENSITY_EDIT }, 
								 mainWindowFriend->getRgbs()["Solarized Base03"] );
	startLocation = { 1440, 28 };
	masterScript.initialize( 480, 900, startLocation, tooltips, this, id, "Master", "Master Script",
	                         { IDC_MASTER_FUNCTION_COMBO, IDC_MASTER_EDIT }, mainWindowFriend->getRgbs()["Solarized Base04"] );
	startLocation = { 1700, 3 };
	statusBox.initialize(startLocation, id, this, 300, tooltips);
	profileDisplay.initialize({ 0,3 }, id, this, tooltips);
	
	CMenu menu;
	menu.LoadMenu(IDR_MAIN_MENU);
	SetMenu(&menu);
	try
	{
		// I only do this for the intensity agilent at the moment.
		intensityAgilent.setDefault( 1 );
	}
	catch (Error& err)
	{
		errBox( "ERROR: Failed to initialize intensity agilent: " + err.whatStr() );
	}
	SetRedraw( true );
	return TRUE;
}


void ScriptingWindow::fillMasterThreadInput( MasterThreadInput* input )
{
	input->agilents.push_back( &intensityAgilent );
	input->intensityAgilentNumber = input->agilents.size() - 1;
}


void ScriptingWindow::OnTimer(UINT_PTR eventID)
{
	horizontalNiawgScript.handleTimerCall( auxWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(), 
										   auxWindowFriend->getTtlNames(), auxWindowFriend->getDacNames() );
	intensityAgilent.agilentScript.handleTimerCall(auxWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
										   auxWindowFriend->getTtlNames(), auxWindowFriend->getDacNames());
	verticalNiawgScript.handleTimerCall(auxWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
										auxWindowFriend->getTtlNames(), auxWindowFriend->getDacNames());
	masterScript.handleTimerCall(auxWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
								 auxWindowFriend->getTtlNames(), auxWindowFriend->getDacNames());
}


void ScriptingWindow::checkScriptSaves()
{
	horizontalNiawgScript.checkSave(getProfile().categoryPath, mainWindowFriend->getRunInfo());
	verticalNiawgScript.checkSave(getProfile().categoryPath, mainWindowFriend->getRunInfo());
	intensityAgilent.checkSave( getProfile( ).categoryPath, mainWindowFriend->getRunInfo( ) );
	masterScript.checkSave( getProfile( ).categoryPath, mainWindowFriend->getRunInfo( ) );
}


std::string ScriptingWindow::getSystemStatusString()
{
	return "\nIntensity Agilent: " + intensityAgilent.getDeviceIdentity();	
}


void ScriptingWindow::loadFriends(MainWindow* mainWin, CameraWindow* camWin, AuxiliaryWindow* masterWin)
{
	mainWindowFriend = mainWin;
	cameraWindowFriend = camWin;
	auxWindowFriend = masterWin;
}


/*
	This function retuns the names (just the names) of currently active scripts.
*/
scriptInfo<std::string> ScriptingWindow::getScriptNames()
{
	scriptInfo<std::string> names;
	// Order matters!
	names.horizontalNIAWG = horizontalNiawgScript.getScriptName();
	names.verticalNIAWG = verticalNiawgScript.getScriptName();
	names.intensityAgilent = intensityAgilent.agilentScript.getScriptName();
	return names;
}

/*
	This function returns indicators of whether a given script has been saved or not.
*/
scriptInfo<bool> ScriptingWindow::getScriptSavedStatuses()
{
	scriptInfo<bool> status;
	status.horizontalNIAWG = horizontalNiawgScript.savedStatus();
	status.verticalNIAWG = verticalNiawgScript.savedStatus();
	status.intensityAgilent = intensityAgilent.agilentScript.savedStatus();
	return status;
}

/*
	This function returns the current addresses of all files in all scripts.
*/
scriptInfo<std::string> ScriptingWindow::getScriptAddresses()
{
	scriptInfo<std::string> addresses;
	addresses.horizontalNIAWG = horizontalNiawgScript.getScriptPathAndName();
	addresses.verticalNIAWG = verticalNiawgScript.getScriptPathAndName();
	addresses.intensityAgilent = intensityAgilent.agilentScript.getScriptPathAndName();
	addresses.master = masterScript.getScriptPathAndName();
	return addresses;
}

/*
	This function handles the coloring of all controls on this window.
*/
HBRUSH ScriptingWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	brushMap brushes = mainWindowFriend->getBrushes();
	rgbMap rgbs = mainWindowFriend->getRgbs();
	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{
			int num = (pWnd->GetDlgCtrlID());
			CBrush* result = statusBox.handleColoring(num, pDC, brushes, rgbs);
			if (result)
			{
				return *result;
			}
			else
			{
				pDC->SetTextColor(rgbs["Solarized Orange"]);
				pDC->SetBkColor( rgbs["Dark Grey"] );
				return *brushes["Dark Grey"];
			}
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor(rgbs["Solarized Orange"]);
			pDC->SetBkColor(rgbs["Solarized Base02"]);
			return *brushes["Solarized Base02"];
		}
		default:
		{
			return *brushes["LigSolarized Base02"];
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
		comm( )->sendError( err.what( ) );
	}
}


void ScriptingWindow::horizontalEditChange()
{
	horizontalNiawgScript.handleEditChange( );
	SetTimer( SYNTAX_TIMER_ID, SYNTAX_TIMER_LENGTH, NULL );
}

void ScriptingWindow::agilentEditChange()
{
	intensityAgilent.agilentScript.handleEditChange();
	SetTimer(SYNTAX_TIMER_ID, SYNTAX_TIMER_LENGTH, NULL);
}

void ScriptingWindow::verticalEditChange()
{
	verticalNiawgScript.handleEditChange();
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
		intensityAgilent.checkSave( getProfile().categoryPath, mainWindowFriend->getRunInfo() );
		intensityAgilent.agilentScript.newScript( );
		updateConfigurationSavedStatus( false );
		intensityAgilent.agilentScript.updateScriptNameText( mainWindowFriend->getProfileSettings().categoryPath );
		intensityAgilent.agilentScript.colorEntireScript( auxWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
														  auxWindowFriend->getTtlNames(), auxWindowFriend->getDacNames() );
	}
	catch (Error& err)
	{
		comm()->sendError( err.what() );
	}
}


void ScriptingWindow::openIntensityScript( CWnd* parent )
{
	try
	{
		intensityAgilent.checkSave( getProfile().categoryPath, mainWindowFriend->getRunInfo() );
		std::string intensityOpenName = openWithExplorer( parent, AGILENT_SCRIPT_EXTENSION );
		intensityAgilent.agilentScript.openParentScript( intensityOpenName, getProfile().categoryPath, mainWindowFriend->getRunInfo() );
		updateConfigurationSavedStatus( false );
		intensityAgilent.agilentScript.updateScriptNameText( getProfile().categoryPath );
		intensityAgilent.agilentScript.colorEntireScript(auxWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
			auxWindowFriend->getTtlNames(), auxWindowFriend->getDacNames());
	}
	catch (Error& err)
	{
		comm()->sendError( err.what() );
	}
}

void ScriptingWindow::saveIntensityScript()
{
	try
	{
		// channel 0 is the intensity channel, the 4th option is the scripting option.
		if ( intensityAgilent.getOutputInfo( ).channel[0].option == 4 )
		{
			intensityAgilent.agilentScript.saveScript( getProfile( ).categoryPath, mainWindowFriend->getRunInfo( ) );
			intensityAgilent.agilentScript.updateScriptNameText( getProfile( ).categoryPath );
		}
	}
	catch (Error& err)
	{
		comm()->sendError( err.what() );
	}
}


void ScriptingWindow::saveIntensityScriptAs(CWnd* parent)
{
	try
	{
		std::string extensionNoPeriod = intensityAgilent.agilentScript.getExtension();
		if (extensionNoPeriod.size() == 0)
		{
			return;
		}
		extensionNoPeriod = extensionNoPeriod.substr( 1, extensionNoPeriod.size() );
		std::string newScriptAddress = saveWithExplorer( parent, extensionNoPeriod, getProfileSettings() );
		intensityAgilent.agilentScript.saveScriptAs( newScriptAddress, mainWindowFriend->getRunInfo() );
		updateConfigurationSavedStatus( false );
		intensityAgilent.agilentScript.updateScriptNameText( getProfile().categoryPath );
	}
	catch (Error& err)
	{
		comm()->sendError( err.what() );
	}

}


void ScriptingWindow::newVerticalScript()
{
	try
	{
		verticalNiawgScript.checkSave( getProfile().categoryPath, mainWindowFriend->getRunInfo() );
		verticalNiawgScript.newScript( );
		updateConfigurationSavedStatus( false );
		verticalNiawgScript.updateScriptNameText( getProfile().categoryPath );
		verticalNiawgScript.colorEntireScript( auxWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
											   auxWindowFriend->getTtlNames(), auxWindowFriend->getDacNames() );
	}
	catch (Error& err)
	{
		comm()->sendError( err.what() );
	}

}


// just a quick shortcut.
profileSettings ScriptingWindow::getProfile()
{
	return mainWindowFriend->getProfileSettings();
}


void ScriptingWindow::openVerticalScript(CWnd* parent)
{
	try
	{
		verticalNiawgScript.checkSave( getProfile().categoryPath, mainWindowFriend->getRunInfo() );
		std::string intensityOpenName = openWithExplorer( parent, NIAWG_SCRIPT_EXTENSION );
		verticalNiawgScript.openParentScript( intensityOpenName, getProfile().categoryPath, 
											  mainWindowFriend->getRunInfo() );
		updateConfigurationSavedStatus( false );
		verticalNiawgScript.updateScriptNameText( getProfile().categoryPath );
		verticalNiawgScript.colorEntireScript(auxWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
			auxWindowFriend->getTtlNames(), auxWindowFriend->getDacNames());
	}
	catch (Error& err)
	{
		comm()->sendError( err.what() );
	}

}


void ScriptingWindow::saveVerticalScript()
{
	try
	{
		verticalNiawgScript.saveScript(getProfile().categoryPath, mainWindowFriend->getRunInfo());
		verticalNiawgScript.updateScriptNameText(getProfile().categoryPath);
	}
	catch (Error& err)
	{
		comm()->sendError( err.what() );
	}

}


void ScriptingWindow::saveVerticalScriptAs(CWnd* parent)
{
	try
	{
		std::string extensionNoPeriod = verticalNiawgScript.getExtension();
		if (extensionNoPeriod.size() == 0)
		{
			return;
		}
		extensionNoPeriod = extensionNoPeriod.substr(1, extensionNoPeriod.size());
		std::string newScriptAddress = saveWithExplorer(parent, extensionNoPeriod, getProfileSettings());
		verticalNiawgScript.saveScriptAs(newScriptAddress, mainWindowFriend->getRunInfo() );
		updateConfigurationSavedStatus(false);
		verticalNiawgScript.updateScriptNameText(getProfile().categoryPath);
	}
	catch (Error& err)
	{
		comm()->sendError( err.what() );
	}

}


void ScriptingWindow::newHorizontalScript()
{
	try
	{
		horizontalNiawgScript.checkSave( getProfile().categoryPath, mainWindowFriend->getRunInfo() );
		horizontalNiawgScript.newScript( );
		updateConfigurationSavedStatus( false );
		horizontalNiawgScript.updateScriptNameText( getProfile().categoryPath );
		horizontalNiawgScript.colorEntireScript( auxWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
												 auxWindowFriend->getTtlNames(), auxWindowFriend->getDacNames() );
	}
	catch (Error& err)
	{
		comm()->sendError( err.what() );
	}

}


void ScriptingWindow::openHorizontalScript(CWnd* parent)
{
	try
	{
		horizontalNiawgScript.checkSave( getProfile().categoryPath, mainWindowFriend->getRunInfo() );
		std::string horizontalOpenName = openWithExplorer( parent, NIAWG_SCRIPT_EXTENSION );
		horizontalNiawgScript.openParentScript( horizontalOpenName, getProfile().categoryPath, mainWindowFriend->getRunInfo() );
		updateConfigurationSavedStatus( false );
		horizontalNiawgScript.updateScriptNameText( getProfile().categoryPath );
		horizontalNiawgScript.colorEntireScript(auxWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
			auxWindowFriend->getTtlNames(), auxWindowFriend->getDacNames());
	}
	catch (Error& err)
	{
		comm()->sendError( err.what() );
	}

}


void ScriptingWindow::saveHorizontalScript()
{
	try
	{
		horizontalNiawgScript.saveScript( getProfile().categoryPath, mainWindowFriend->getRunInfo() );
		horizontalNiawgScript.updateScriptNameText( getProfile().categoryPath );
	}
	catch (Error& err)
	{
		comm()->sendError( err.what() );
	}
}


void ScriptingWindow::saveHorizontalScriptAs(CWnd* parent)
{
	std::string extensionNoPeriod = horizontalNiawgScript.getExtension();
	if (extensionNoPeriod.size() == 0)
	{
		return;
	}
	extensionNoPeriod = extensionNoPeriod.substr(1, extensionNoPeriod.size());
	std::string newScriptAddress = saveWithExplorer(parent, extensionNoPeriod, 
														getProfileSettings());
	horizontalNiawgScript.saveScriptAs(newScriptAddress, mainWindowFriend->getRunInfo() );
	updateConfigurationSavedStatus(false);
	horizontalNiawgScript.updateScriptNameText(getProfile().categoryPath);
}


void ScriptingWindow::updateScriptNamesOnScreen()
{
	horizontalNiawgScript.updateScriptNameText(getProfile().categoryPath);
	verticalNiawgScript.updateScriptNameText(getProfile().categoryPath);
	intensityAgilent.agilentScript.updateScriptNameText(getProfile().categoryPath);
}


void ScriptingWindow::recolorScripts()
{
	verticalNiawgScript.colorEntireScript( auxWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
										   auxWindowFriend->getTtlNames(), auxWindowFriend->getDacNames());
	horizontalNiawgScript.colorEntireScript(auxWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
											auxWindowFriend->getTtlNames(), auxWindowFriend->getDacNames());
	intensityAgilent.agilentScript.colorEntireScript(auxWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
											 auxWindowFriend->getTtlNames(), auxWindowFriend->getDacNames());
	masterScript.colorEntireScript(auxWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
								   auxWindowFriend->getTtlNames(), auxWindowFriend->getDacNames());
}


void ScriptingWindow::openIntensityScript(std::string name)
{
	intensityAgilent.agilentScript.openParentScript(name, getProfile().categoryPath, mainWindowFriend->getRunInfo());
}


void ScriptingWindow::handleOpenConfig(std::ifstream& configFile, double version)
{
	ProfileSystem::checkDelimiterLine(configFile, "SCRIPTS");
	// the reading for the scripts is simple enough at the moment that I just read everything here.
	configFile.get();
	std::string vertName, horName, masterName;
	getline(configFile, vertName);
	getline(configFile, horName);
	getline(configFile, masterName);
	ProfileSystem::checkDelimiterLine(configFile, "END_SCRIPTS");

	intensityAgilent.readConfigurationFile(configFile, version );
	intensityAgilent.updateSettingsDisplay(1, mainWindowFriend->getProfileSettings().categoryPath, mainWindowFriend->getRunInfo());
	openVerticalScript(vertName);
	openHorizontalScript(horName);
	openMasterScript(masterName);
	considerScriptLocations();
	recolorScripts();

}


void ScriptingWindow::newMasterScript()
{
	masterScript.checkSave(getProfile().categoryPath, mainWindowFriend->getRunInfo());
	masterScript.newScript( );
	updateConfigurationSavedStatus(false);
	masterScript.updateScriptNameText(getProfile().categoryPath);
	masterScript.colorEntireScript(auxWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
								   auxWindowFriend->getTtlNames(), auxWindowFriend->getDacNames());
}

void ScriptingWindow::openMasterScript(CWnd* parent)
{
	try
	{
		masterScript.checkSave( getProfile( ).categoryPath, mainWindowFriend->getRunInfo( ) );
		std::string horizontalOpenName = openWithExplorer( parent, MASTER_SCRIPT_EXTENSION );
		masterScript.openParentScript( horizontalOpenName, getProfile( ).categoryPath, mainWindowFriend->getRunInfo( ) );
		updateConfigurationSavedStatus( false );
		masterScript.updateScriptNameText( getProfile( ).categoryPath );
		masterScript.colorEntireScript( auxWindowFriend->getAllVariables( ), mainWindowFriend->getRgbs( ),
										auxWindowFriend->getTtlNames( ), auxWindowFriend->getDacNames( ) );
	}
	catch ( Error& err )
	{
		comm( )->sendError( "New Master function Failed: " + err.whatStr( ) + "\r\n" );
	}
}


void ScriptingWindow::saveMasterScript()
{
	masterScript.saveScript(getProfile().categoryPath, mainWindowFriend->getRunInfo());
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
	masterScript.saveScriptAs(newScriptAddress, mainWindowFriend->getRunInfo());
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
		comm()->sendError("New Master function Failed: " + exception.whatStr() + "\r\n");
	}
}


void ScriptingWindow::saveMasterFunction()
{
	try
	{
		masterScript.saveAsFunction();
	}
	catch (Error& exception)
	{
		comm()->sendError("Save Master Script Function Failed: " + exception.whatStr() + "\r\n");
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
	saveFile << addresses.verticalNIAWG << "\n";
	saveFile << addresses.horizontalNIAWG << "\n";
	saveFile << addresses.master << "\n";
	saveFile << "END_SCRIPTS\n";
	intensityAgilent.handleSavingConfig(saveFile, mainWindowFriend->getProfileSettings().categoryPath, 
										 mainWindowFriend->getRunInfo());
}


void ScriptingWindow::checkMasterSave()
{
	masterScript.checkSave(getProfile().categoryPath, mainWindowFriend->getRunInfo());
}


void ScriptingWindow::openMasterScript(std::string name)
{
	masterScript.openParentScript(name, getProfile().categoryPath, mainWindowFriend->getRunInfo());
}


void ScriptingWindow::openVerticalScript(std::string name)
{
	try
	{
		verticalNiawgScript.openParentScript(name, getProfile().categoryPath, mainWindowFriend->getRunInfo());
	}
	catch(Error& err)
	{
		comm()->sendError(err.what());
	}
}


void ScriptingWindow::openHorizontalScript(std::string name)
{
	try
	{
		horizontalNiawgScript.openParentScript(name, getProfile().categoryPath, mainWindowFriend->getRunInfo());
	}
	catch (Error& err)
	{
		comm()->sendError(err.what());
	}
}


void ScriptingWindow::considerScriptLocations()
{
	verticalNiawgScript.considerCurrentLocation(getProfile().categoryPath, mainWindowFriend->getRunInfo());
	horizontalNiawgScript.considerCurrentLocation(getProfile().categoryPath, mainWindowFriend->getRunInfo());
	intensityAgilent.agilentScript.considerCurrentLocation(getProfile().categoryPath, mainWindowFriend->getRunInfo());
}


/// End common functions
void ScriptingWindow::passCommonCommand(UINT id)
{
	// pass the command id to the common function, filling in the pointers to the windows which own objects needed.
	commonFunctions::handleCommonMessage( id, this, mainWindowFriend, this, cameraWindowFriend, auxWindowFriend );
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
	return mainWindowFriend->getProfileSettings();
}


void ScriptingWindow::updateConfigurationSavedStatus(bool status)
{
	mainWindowFriend->updateConfigurationSavedStatus(status);
}

