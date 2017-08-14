#include "stdafx.h"
#include "ScriptingWindow.h"
#include "afxwin.h"
#include "MainWindow.h"
#include "getFileName.h"
#include "saveTextFileFromEdit.h"
#include "commonFunctions.h"
#include "textPromptDialog.h"
#include "DeviceWindow.h"

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

	// menu stuff
	ON_COMMAND_RANGE(MENU_ID_RANGE_BEGIN, MENU_ID_RANGE_END, &ScriptingWindow::passCommonCommand)
	// 
	ON_CBN_SELENDOK(IDC_VERTICAL_NIAWG_FUNCTION_COMBO, &ScriptingWindow::handleVerticalScriptComboChange)
	ON_CBN_SELENDOK(IDC_HORIZONTAL_NIAWG_FUNCTION_COMBO, &ScriptingWindow::handleHorizontalScriptComboChange)
	ON_CBN_SELENDOK(IDC_INTENSITY_FUNCTION_COMBO, &ScriptingWindow::handleAgilentScriptComboChange)
	//ON_CBN_SELENDOK(IDC_MASTER_FUNCTION_COMBO, &ScriptingWindow::handleMasterScriptComboChange)
END_MESSAGE_MAP()


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
	verticalNiawgScript.rearrange(cx, cy, mainWindowFriend->getFonts());
	horizontalNiawgScript.rearrange(cx, cy, mainWindowFriend->getFonts());
	intensityAgilentScript.rearrange(cx, cy, mainWindowFriend->getFonts());
	masterScript.rearrange(cx, cy, mainWindowFriend->getFonts());
	statusBox.rearrange("", "", cx, cy, mainWindowFriend->getFonts());
	profileDisplay.rearrange(cx, cy, mainWindowFriend->getFonts());

	verticalNiawgScript.colorEntireScript(deviceWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(), 
										  deviceWindowFriend->getTtlNames(), deviceWindowFriend->getDacNames());
	horizontalNiawgScript.colorEntireScript(deviceWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
											deviceWindowFriend->getTtlNames(), deviceWindowFriend->getDacNames());
	intensityAgilentScript.colorEntireScript(deviceWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
											 deviceWindowFriend->getTtlNames(), deviceWindowFriend->getDacNames());
	masterScript.colorEntireScript(deviceWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
								   deviceWindowFriend->getTtlNames(), deviceWindowFriend->getDacNames());
}

BOOL ScriptingWindow::PreTranslateMessage(MSG* pMsg)
{
	for (int toolTipInc = 0; toolTipInc < tooltips.size(); toolTipInc++)
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
	//verticalNiawgScript.childComboChangeHandler( mainWindowFriend, deviceWindowFriend );
}


void ScriptingWindow::handleAgilentScriptComboChange()
{
	//intensityAgilentScript.childComboChangeHandler( mainWindowFriend, deviceWindowFriend);
}


// this gets called when closing. The purpose here is to redirect the default, very abrupt close that would normally happen.
void ScriptingWindow::OnCancel()
{
	passCommonCommand(ID_FILE_MY_EXIT);
}


BOOL ScriptingWindow::OnInitDialog()
{
	// ADD MORE INITIALIZATIONS HERE
	int id = 2000;
	POINT startLocaiton = { 0, 28 };
	verticalNiawgScript.initialize( 480, 1000, startLocaiton, tooltips, this, id, "Vertical NIAWG");
	startLocaiton = { 480, 28 };
	horizontalNiawgScript.initialize( 480, 1000, startLocaiton, tooltips, this,  id, "Horizontal NIAWG");
	startLocaiton = { 960, 28 };
	intensityAgilentScript.initialize( 480, 1000, startLocaiton, tooltips, this, id, "Agilent");
	startLocaiton = { 1440, 28 };
	masterScript.initialize( 480, 1000, startLocaiton, tooltips, this, id, "Master");
	startLocaiton = { 1700, 3 };
	statusBox.initialize(startLocaiton, id, this, 220, mainWindowFriend->getFonts(), tooltips);
	profileDisplay.initialize({ 0,3 }, id, this, mainWindowFriend->getFonts(), tooltips);

	CMenu menu;
	menu.LoadMenu(IDR_MAIN_MENU);
	SetMenu(&menu);
	//ShowWindow(SW_MAXIMIZE);
	return TRUE;
}

void ScriptingWindow::OnTimer(UINT_PTR eventID)
{
	horizontalNiawgScript.handleTimerCall( deviceWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(), 
										   deviceWindowFriend->getTtlNames(), deviceWindowFriend->getDacNames() );
	intensityAgilentScript.handleTimerCall(deviceWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
										   deviceWindowFriend->getTtlNames(), deviceWindowFriend->getDacNames());
	verticalNiawgScript.handleTimerCall(deviceWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
										deviceWindowFriend->getTtlNames(), deviceWindowFriend->getDacNames());
	masterScript.handleTimerCall(deviceWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
								 deviceWindowFriend->getTtlNames(), deviceWindowFriend->getDacNames());
}


bool ScriptingWindow::checkScriptSaves()
{
	horizontalNiawgScript.checkSave(getProfile().categoryPath, mainWindowFriend->getRunInfo());
	verticalNiawgScript.checkSave(getProfile().categoryPath, mainWindowFriend->getRunInfo());
	intensityAgilentScript.checkSave(getProfile().categoryPath, mainWindowFriend->getRunInfo());
	return false;
}


void ScriptingWindow::getFriends(MainWindow* mainWin, CameraWindow* camWin, DeviceWindow* masterWin)
{
	mainWindowFriend = mainWin;
	cameraWindowFriend = camWin;
	deviceWindowFriend = masterWin;
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
	names.intensityAgilent = intensityAgilentScript.getScriptName();
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
	status.intensityAgilent = intensityAgilentScript.savedStatus();
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
	addresses.intensityAgilent = intensityAgilentScript.getScriptPathAndName();
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
				pDC->SetTextColor(rgbs["White"]);
				pDC->SetBkColor(rgbs["Dark Grey Red"]);
				return *brushes["Dark Grey Red"];
			}
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor(rgbs["White"]);
			pDC->SetBkColor(rgbs["Dark Red"]);
			return *brushes["Dark Red"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor(rgbs["White"]);
			pDC->SetBkColor(rgbs["Dark Grey"]);
			return *brushes["Dark Grey"];
		}
		default:
		{
			return *brushes["Light Grey"];
		}
	}
}

void ScriptingWindow::horizontalEditChange()
{
	horizontalNiawgScript.handleEditChange();
	SetTimer(SYNTAX_TIMER_ID, SYNTAX_TIMER_LENGTH, NULL);
}

void ScriptingWindow::agilentEditChange()
{
	intensityAgilentScript.handleEditChange();
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
	intensityAgilentScript.checkSave(getProfile().categoryPath, mainWindowFriend->getRunInfo());
	intensityAgilentScript.newScript(getProfile().orientation);
	updateConfigurationSavedStatus(false);
	intensityAgilentScript.updateScriptNameText(mainWindowFriend->getCurrentProfileSettings().categoryPath);
	intensityAgilentScript.colorEntireScript(deviceWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
											 deviceWindowFriend->getTtlNames(), deviceWindowFriend->getDacNames());
}


void ScriptingWindow::openIntensityScript(CWnd* parent)
{
	intensityAgilentScript.checkSave(getProfile().categoryPath, mainWindowFriend->getRunInfo());
	std::string intensityOpenName = getFileNameDialog(parent->GetSafeHwnd());
	intensityAgilentScript.openParentScript(intensityOpenName, getProfile().categoryPath, mainWindowFriend->getRunInfo());
	updateConfigurationSavedStatus(false);
	intensityAgilentScript.updateScriptNameText(getProfile().categoryPath);
}


void ScriptingWindow::saveIntensityScript()
{
	intensityAgilentScript.saveScript(getProfile().categoryPath, mainWindowFriend->getRunInfo());
	intensityAgilentScript.updateScriptNameText(getProfile().categoryPath);
}


void ScriptingWindow::saveIntensityScriptAs(CWnd* parent)
{
	std::string extensionNoPeriod = intensityAgilentScript.getExtension();
	if (extensionNoPeriod.size() == 0)
	{
		return;
	}
	extensionNoPeriod = extensionNoPeriod.substr(1, extensionNoPeriod.size());
	std::string newScriptAddress = saveTextFileFromEdit(parent->GetSafeHwnd(), extensionNoPeriod, getCurrentProfileSettings());
	intensityAgilentScript.saveScriptAs(newScriptAddress, mainWindowFriend->getRunInfo() );
	updateConfigurationSavedStatus(false);
	intensityAgilentScript.updateScriptNameText(getProfile().categoryPath);
}


void ScriptingWindow::newVerticalScript()
{
	verticalNiawgScript.checkSave(getProfile().categoryPath, mainWindowFriend->getRunInfo());
	verticalNiawgScript.newScript(getProfile().orientation);
	updateConfigurationSavedStatus(false);
	verticalNiawgScript.updateScriptNameText(getProfile().categoryPath);
	verticalNiawgScript.colorEntireScript(deviceWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
											deviceWindowFriend->getTtlNames(), deviceWindowFriend->getDacNames());
}


// just a quick shortcut.
profileSettings ScriptingWindow::getProfile()
{
	return mainWindowFriend->getCurrentProfileSettings();
}


void ScriptingWindow::openVerticalScript(CWnd* parent)
{
	verticalNiawgScript.checkSave(getProfile().categoryPath, mainWindowFriend->getRunInfo());
	std::string intensityOpenName = getFileNameDialog(parent->GetSafeHwnd());
	verticalNiawgScript.openParentScript(intensityOpenName, getProfile().categoryPath,
										 mainWindowFriend->getRunInfo());
	updateConfigurationSavedStatus(false);
	verticalNiawgScript.updateScriptNameText(getProfile().categoryPath);
}


void ScriptingWindow::saveVerticalScript()
{
	verticalNiawgScript.saveScript(getProfile().categoryPath, mainWindowFriend->getRunInfo());
	verticalNiawgScript.updateScriptNameText(getProfile().categoryPath);
}


void ScriptingWindow::saveVerticalScriptAs(CWnd* parent)
{
	std::string extensionNoPeriod = verticalNiawgScript.getExtension();
	if (extensionNoPeriod.size() == 0)
	{
		return;
	}
	extensionNoPeriod = extensionNoPeriod.substr(1, extensionNoPeriod.size());
	std::string newScriptAddress = saveTextFileFromEdit(parent->GetSafeHwnd(), extensionNoPeriod, getCurrentProfileSettings());
	verticalNiawgScript.saveScriptAs(newScriptAddress, mainWindowFriend->getRunInfo() );
	updateConfigurationSavedStatus(false);
	verticalNiawgScript.updateScriptNameText(getProfile().categoryPath);
}


void ScriptingWindow::newHorizontalScript()
{
	horizontalNiawgScript.checkSave(getProfile().categoryPath, mainWindowFriend->getRunInfo());
	horizontalNiawgScript.newScript( getProfile().orientation );
	updateConfigurationSavedStatus(false);
	horizontalNiawgScript.updateScriptNameText(getProfile().categoryPath);
	horizontalNiawgScript.colorEntireScript(deviceWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
											deviceWindowFriend->getTtlNames(), deviceWindowFriend->getDacNames());
}


void ScriptingWindow::openHorizontalScript(CWnd* parent)
{
	horizontalNiawgScript.checkSave(getProfile().categoryPath, mainWindowFriend->getRunInfo());
	std::string horizontalOpenName = getFileNameDialog(parent->GetSafeHwnd());
	horizontalNiawgScript.openParentScript(horizontalOpenName, getProfile().categoryPath, mainWindowFriend->getRunInfo());
	updateConfigurationSavedStatus(false);
	horizontalNiawgScript.updateScriptNameText(getProfile().categoryPath);
}


void ScriptingWindow::saveHorizontalScript()
{
	horizontalNiawgScript.saveScript(getProfile().categoryPath, mainWindowFriend->getRunInfo());
	horizontalNiawgScript.updateScriptNameText(getProfile().categoryPath);
}


void ScriptingWindow::saveHorizontalScriptAs(CWnd* parent)
{
	std::string extensionNoPeriod = horizontalNiawgScript.getExtension();
	if (extensionNoPeriod.size() == 0)
	{
		return;
	}
	extensionNoPeriod = extensionNoPeriod.substr(1, extensionNoPeriod.size());
	std::string newScriptAddress = saveTextFileFromEdit(parent->GetSafeHwnd(), extensionNoPeriod, 
														getCurrentProfileSettings());
	horizontalNiawgScript.saveScriptAs(newScriptAddress, mainWindowFriend->getRunInfo() );
	updateConfigurationSavedStatus(false);
	horizontalNiawgScript.updateScriptNameText(getProfile().categoryPath);
}


void ScriptingWindow::updateScriptNamesOnScreen()
{
	horizontalNiawgScript.updateScriptNameText(getProfile().categoryPath);
	verticalNiawgScript.updateScriptNameText(getProfile().categoryPath);
	intensityAgilentScript.updateScriptNameText(getProfile().categoryPath);
}


void ScriptingWindow::recolorScripts()
{
	verticalNiawgScript.colorEntireScript( deviceWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
										   deviceWindowFriend->getTtlNames(), deviceWindowFriend->getDacNames() );
	horizontalNiawgScript.colorEntireScript(deviceWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
											deviceWindowFriend->getTtlNames(), deviceWindowFriend->getDacNames());
	intensityAgilentScript.colorEntireScript(deviceWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
											 deviceWindowFriend->getTtlNames(), deviceWindowFriend->getDacNames());
	masterScript.colorEntireScript(deviceWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
								   deviceWindowFriend->getTtlNames(), deviceWindowFriend->getDacNames());
}


void ScriptingWindow::openIntensityScript(std::string name)
{
	intensityAgilentScript.openParentScript(name, getProfile().categoryPath, mainWindowFriend->getRunInfo());
}


void ScriptingWindow::handleOpenConfig(std::ifstream& configFile, double version)
{
	ProfileSystem::checkDelimiterLine(configFile, "SCRIPTS");
	// the reading for the scripts is simple enough at the moment that I just read everything here.
	configFile.get();
	std::string vertName, horName, intensityName, masterName;
	getline(configFile, vertName);
	getline(configFile, horName);
	getline(configFile, intensityName);
	getline(configFile, masterName);

	openVerticalScript(vertName);
	openHorizontalScript(horName);
	openIntensityScript(intensityName);
	openMasterScript(masterName);
	considerScriptLocations();
	recolorScripts();

	ProfileSystem::checkDelimiterLine(configFile, "END_SCRIPTS");
}


void ScriptingWindow::newMasterScript()
{
	masterScript.checkSave(getProfile().categoryPath, mainWindowFriend->getRunInfo());
	masterScript.newScript(getProfile().orientation);
	updateConfigurationSavedStatus(false);
	masterScript.updateScriptNameText(getProfile().categoryPath);
	masterScript.colorEntireScript(deviceWindowFriend->getAllVariables(), mainWindowFriend->getRgbs(),
											deviceWindowFriend->getTtlNames(), deviceWindowFriend->getDacNames());
}

void ScriptingWindow::openMasterScript(CWnd* parent)
{
	masterScript.checkSave(getProfile().categoryPath, mainWindowFriend->getRunInfo());
	std::string horizontalOpenName = getFileNameDialog(parent->GetSafeHwnd());
	masterScript.openParentScript(horizontalOpenName, getProfile().categoryPath, mainWindowFriend->getRunInfo());
	updateConfigurationSavedStatus(false);
	masterScript.updateScriptNameText(getProfile().categoryPath);
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
	std::string newScriptAddress = saveTextFileFromEdit(parent->GetSafeHwnd(), extensionNoPeriod,
														getCurrentProfileSettings());
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


void ScriptingWindow::handleSavingConfig(std::ofstream& saveFile)
{
	scriptInfo<std::string> addresses = getScriptAddresses();
	// order matters!
	saveFile << "SCRIPTS\n";
	saveFile << addresses.verticalNIAWG << "\n";
	saveFile << addresses.horizontalNIAWG << "\n";
	saveFile << addresses.intensityAgilent << "\n";	
	saveFile << addresses.master << "\n";
	saveFile << "END_SCRIPTS\n";
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
	intensityAgilentScript.considerCurrentLocation(getProfile().categoryPath, mainWindowFriend->getRunInfo());
}


/// End common functions
void ScriptingWindow::passCommonCommand(UINT id)
{
	// pass the command id to the common function, filling in the pointers to the windows which own objects needed.
	commonFunctions::handleCommonMessage( id, this, mainWindowFriend, this, cameraWindowFriend, deviceWindowFriend );
}


void ScriptingWindow::changeBoxColor( systemInfo<char> colors )
{
	statusBox.changeColor(colors);
}


void ScriptingWindow::updateProfile(std::string text)
{
	profileDisplay.update(text);
}


profileSettings ScriptingWindow::getCurrentProfileSettings()
{	
	return mainWindowFriend->getCurrentProfileSettings();
}


void ScriptingWindow::updateConfigurationSavedStatus(bool status)
{
	mainWindowFriend->updateConfigurationSavedStatus(status);
}

