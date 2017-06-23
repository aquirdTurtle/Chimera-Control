#include "stdafx.h"
#include "ScriptingWindow.h"
#include "afxwin.h"
#include "MainWindow.h"
#include "getFileName.h"
#include "saveTextFileFromEdit.h"
#include "commonFunctions.h"

IMPLEMENT_DYNAMIC(ScriptingWindow, CDialog)

BEGIN_MESSAGE_MAP(ScriptingWindow, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_EN_CHANGE(IDC_HORIZONTAL_SCRIPT_EDIT, &ScriptingWindow::horizontalEditChange)
	ON_EN_CHANGE(IDC_VERTICAL_SCRIPT_EDIT, &ScriptingWindow::verticalEditChange)
	ON_EN_CHANGE(IDC_AGILENT_SCRIPT_EDIT, &ScriptingWindow::agilentEditChange)
	// menu stuff
	ON_COMMAND_RANGE(MENU_ID_RANGE_BEGIN, MENU_ID_RANGE_END, &ScriptingWindow::passCommonCommand)
	// 
	ON_CBN_SELENDOK(IDC_VERTICAL_SCRIPT_COMBO, &ScriptingWindow::handleVerticalScriptComboChange)
	ON_CBN_SELENDOK(IDC_HORIZONTAL_SCRIPT_COMBO, &ScriptingWindow::handleHorizontalScriptComboChange)
	ON_CBN_SELENDOK(IDC_AGILENT_SCRIPT_COMBO, &ScriptingWindow::handleAgilentScriptComboChange)
END_MESSAGE_MAP()

void ScriptingWindow::OnSize(UINT nType, int cx, int cy)
{
	verticalNiawgScript.rearrange(cx, cy, mainWindowFriend->getFonts());
	horizontalNiawgScript.rearrange(cx, cy, mainWindowFriend->getFonts());
	intensityAgilentScript.rearrange(cx, cy, mainWindowFriend->getFonts());
	statusBox.rearrange("", "", cx, cy, mainWindowFriend->getFonts());
	profileDisplay.rearrange(cx, cy, mainWindowFriend->getFonts());

	verticalNiawgScript.colorEntireScript(getCurrentProfileSettings(), mainWindowFriend->getAllVariables());
	horizontalNiawgScript.colorEntireScript(getCurrentProfileSettings(), mainWindowFriend->getAllVariables());
	intensityAgilentScript.colorEntireScript(getCurrentProfileSettings(), mainWindowFriend->getAllVariables());
}

BOOL ScriptingWindow::PreTranslateMessage(MSG* pMsg)
{
	for (int toolTipInc = 0; toolTipInc < this->tooltips.size(); toolTipInc++)
	{
		this->tooltips[toolTipInc]->RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void ScriptingWindow::handleHorizontalScriptComboChange()
{
	this->horizontalNiawgScript.childComboChangeHandler(this->mainWindowFriend);
	return;
}

void ScriptingWindow::handleVerticalScriptComboChange()
{
	this->verticalNiawgScript.childComboChangeHandler(this->mainWindowFriend);
	return;
}

void ScriptingWindow::handleAgilentScriptComboChange()
{
	this->intensityAgilentScript.childComboChangeHandler( this->mainWindowFriend );
	return;
}

// this gets called when closing. The purpose here is to redirect the default, very abrupt close that would normally happen.
void ScriptingWindow::OnCancel()
{
	passCommonCommand(ID_FILE_MY_EXIT);
	return;
}

BOOL ScriptingWindow::OnInitDialog()
{
	// ADD MORE INITIALIZATIONS HERE
	int id = 2000;
	POINT startLocaiton = { 0, 28 };
	verticalNiawgScript.initializeControls(640, 1000, startLocaiton, this, "Vertical NIAWG", id, 
		mainWindowFriend->getFonts(), tooltips);
	startLocaiton = { 640, 28 };
	horizontalNiawgScript.initializeControls(640, 1000, startLocaiton, this, "Horizontal NIAWG", id, 
		mainWindowFriend->getFonts(), tooltips);
	startLocaiton = { 1280, 28 };
	intensityAgilentScript.initializeControls(640, 1000, startLocaiton, this, "Agilent", id, 
		mainWindowFriend->getFonts(), tooltips);
	startLocaiton = { 1700, 3 };
	statusBox.initialize(startLocaiton, id, this, 220, mainWindowFriend->getFonts(), tooltips);
	this->profileDisplay.initialize({ 0,3 }, id, this, mainWindowFriend->getFonts(), tooltips);
	CMenu menu;
	menu.LoadMenu(IDR_MAIN_MENU);
	this->SetMenu(&menu);
	this->ShowWindow(SW_MAXIMIZE);
	return TRUE;
}

void ScriptingWindow::OnTimer(UINT_PTR eventID)
{
	this->horizontalNiawgScript.handleTimerCall(this->mainWindowFriend->getCurentProfileSettings(), this->mainWindowFriend->getAllVariables());
	this->intensityAgilentScript.handleTimerCall(this->mainWindowFriend->getCurentProfileSettings(), this->mainWindowFriend->getAllVariables());
	this->verticalNiawgScript.handleTimerCall(this->mainWindowFriend->getCurentProfileSettings(), this->mainWindowFriend->getAllVariables());
	return;
}

bool ScriptingWindow::checkScriptSaves()
{
	if (this->horizontalNiawgScript.checkSave(this->getCurrentProfileSettings(), this->mainWindowFriend->niawgIsRunning() ))
	{
		return true;
	}
	if (this->verticalNiawgScript.checkSave(this->getCurrentProfileSettings(), this->mainWindowFriend->niawgIsRunning() ))
	{
		return true;
	}
	if (this->intensityAgilentScript.checkSave(this->getCurrentProfileSettings(), this->mainWindowFriend->niawgIsRunning() ))
	{
		return true;
	}
	return false;
}

void ScriptingWindow::getFriends(MainWindow* mainWindowPointer, CameraWindow* camWin)
{
	this->mainWindowFriend = mainWindowPointer;
	this->cameraWindowFriend = camWin;
}

/*
	This function retuns the names (just the names) of currently active scripts.
*/
scriptInfo<std::string> ScriptingWindow::getScriptNames()
{
	scriptInfo<std::string> names;
	// Order matters!
	names.horizontalNIAWG = this->horizontalNiawgScript.getScriptName();
	names.verticalNIAWG = this->verticalNiawgScript.getScriptName();
	names.intensityAgilent = this->intensityAgilentScript.getScriptName();
	return names;
}

/*
	This function returns indicators of whether a given script has been saved or not.
*/
scriptInfo<bool> ScriptingWindow::getScriptSavedStatuses()
{
	scriptInfo<bool> status;
	status.horizontalNIAWG = this->horizontalNiawgScript.savedStatus();
	status.verticalNIAWG = this->verticalNiawgScript.savedStatus();
	status.intensityAgilent = this->intensityAgilentScript.savedStatus();
	return status;
}

/*
	This function returns the current addresses of all files in all scripts.
*/
scriptInfo<std::string> ScriptingWindow::getScriptAddresses()
{
	scriptInfo<std::string> addresses;
	addresses.horizontalNIAWG = this->horizontalNiawgScript.getScriptAddress();
	addresses.verticalNIAWG = this->verticalNiawgScript.getScriptAddress();
	addresses.intensityAgilent = this->intensityAgilentScript.getScriptAddress();
	return addresses;
}

/*
	This function handles the coloring of all controls on this window.
*/
HBRUSH ScriptingWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	brushMap brushes = mainWindowFriend->getBrushes();
	rgbMap rgbs = mainWindowFriend->getRGB();
	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{
			int num = (pWnd->GetDlgCtrlID());
			CBrush* result = this->statusBox.handleColoring(num, pDC, brushes, rgbs);
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
	this->horizontalNiawgScript.handleEditChange();
	this->SetTimer(SYNTAX_TIMER_ID, SYNTAX_TIMER_LENGTH, NULL);
	return;
}

void ScriptingWindow::agilentEditChange()
{
	this->intensityAgilentScript.handleEditChange();
	this->SetTimer(SYNTAX_TIMER_ID, SYNTAX_TIMER_LENGTH, NULL);
	return;
}

void ScriptingWindow::verticalEditChange()
{
	this->verticalNiawgScript.handleEditChange();
	this->SetTimer(SYNTAX_TIMER_ID, SYNTAX_TIMER_LENGTH, NULL);
	return;
}

/// Commonly Called Functions
/*
	The following set of functions, mostly revolving around saving etc. of the script files, are called by all of the
	window objects because they are associated with the menu at the top of each screen
*/
/// 
int ScriptingWindow::newIntensityScript()
{
	if (this->intensityAgilentScript.checkSave(this->getCurrentProfileSettings(), this->mainWindowFriend->niawgIsRunning() ))
	{
		return true;
	}
	intensityAgilentScript.newScript(this->getCurrentProfileSettings(), this->mainWindowFriend->getAllVariables());
	this->updateConfigurationSavedStatus(false);
	intensityAgilentScript.updateScriptNameText();
	return 0;
}


int ScriptingWindow::openIntensityScript(HWND parentWindow)
{
	if (intensityAgilentScript.checkSave(this->getCurrentProfileSettings(), this->mainWindowFriend->niawgIsRunning() ))
	{
		return true;
	}

	std::string intensityOpenName = getFileNameDialog(parentWindow);
	intensityAgilentScript.openParentScript(intensityOpenName, this->getCurrentProfileSettings(), this->mainWindowFriend->getAllVariables());
	this->updateConfigurationSavedStatus(false);
	intensityAgilentScript.updateScriptNameText();
	return 0;
}


int ScriptingWindow::saveIntensityScript()
{
	intensityAgilentScript.saveScript(this->getCurrentProfileSettings(), this->mainWindowFriend->niawgIsRunning() );
	intensityAgilentScript.updateScriptNameText();
	return 0;
}


int ScriptingWindow::saveIntensityScriptAs(HWND parentWindow)
{
	std::string extensionNoPeriod = intensityAgilentScript.getExtension();
	if (extensionNoPeriod.size() == 0)
	{
		return -1;
	}
	extensionNoPeriod = extensionNoPeriod.substr(1, extensionNoPeriod.size());
	std::string newScriptAddress = saveTextFileFromEdit(parentWindow, extensionNoPeriod, this->getCurrentProfileSettings());
	intensityAgilentScript.saveScriptAs(newScriptAddress, this->mainWindowFriend->niawgIsRunning() );
	this->updateConfigurationSavedStatus(false);
	intensityAgilentScript.updateScriptNameText();
	return 0;
}


int ScriptingWindow::newVerticalScript()
{
	if (verticalNiawgScript.checkSave(getCurrentProfileSettings(), mainWindowFriend->niawgIsRunning() ))
	{
		return true;
	}
	verticalNiawgScript.newScript(getCurrentProfileSettings(), mainWindowFriend->getAllVariables());
	updateConfigurationSavedStatus(false);
	verticalNiawgScript.updateScriptNameText();
	return 0;
}


int ScriptingWindow::openVerticalScript(HWND parentWindow)
{
	if (verticalNiawgScript.checkSave(this->getCurrentProfileSettings(), this->mainWindowFriend->niawgIsRunning() ))
	{
		return true;
	}
	std::string intensityOpenName = getFileNameDialog(parentWindow);
	verticalNiawgScript.openParentScript(intensityOpenName, this->getCurrentProfileSettings(), this->mainWindowFriend->getAllVariables());
	updateConfigurationSavedStatus(false);
	verticalNiawgScript.updateScriptNameText();
	return 0;
}


int ScriptingWindow::saveVerticalScript()
{
	verticalNiawgScript.saveScript(this->getCurrentProfileSettings(), this->mainWindowFriend->niawgIsRunning());
	verticalNiawgScript.updateScriptNameText();
	return 0;
}
int ScriptingWindow::saveVerticalScriptAs(HWND parentWindow)
{
	std::string extensionNoPeriod = verticalNiawgScript.getExtension();
	if (extensionNoPeriod.size() == 0)
	{
		return -1;
	}
	extensionNoPeriod = extensionNoPeriod.substr(1, extensionNoPeriod.size());
	std::string newScriptAddress = saveTextFileFromEdit(parentWindow, extensionNoPeriod, this->getCurrentProfileSettings());
	verticalNiawgScript.saveScriptAs(newScriptAddress, this->mainWindowFriend->niawgIsRunning() );
	this->updateConfigurationSavedStatus(false);
	verticalNiawgScript.updateScriptNameText();
	return 0;
}

int ScriptingWindow::newHorizontalScript()
{
	if (this->horizontalNiawgScript.checkSave(this->getCurrentProfileSettings(), this->mainWindowFriend->niawgIsRunning() ))
	{
		return true;
	}
	horizontalNiawgScript.newScript(this->getCurrentProfileSettings(), this->mainWindowFriend->getAllVariables());
	this->updateConfigurationSavedStatus(false);
	horizontalNiawgScript.updateScriptNameText();
	return 0;
}


int ScriptingWindow::openHorizontalScript(HWND parentWindow)
{
	if (horizontalNiawgScript.checkSave(getCurrentProfileSettings(), mainWindowFriend->niawgIsRunning() ))
	{
		return true;
	}
	std::string horizontalOpenName = getFileNameDialog(parentWindow);
	horizontalNiawgScript.openParentScript(horizontalOpenName, getCurrentProfileSettings(), mainWindowFriend->getAllVariables());
	this->updateConfigurationSavedStatus(false);
	horizontalNiawgScript.updateScriptNameText();
	return 0;
}


int ScriptingWindow::saveHorizontalScript()
{
	horizontalNiawgScript.saveScript(getCurrentProfileSettings(), mainWindowFriend->niawgIsRunning() );
	horizontalNiawgScript.updateScriptNameText();
	return 0;
}


int ScriptingWindow::saveHorizontalScriptAs(HWND parentWindow)
{
	std::string extensionNoPeriod = horizontalNiawgScript.getExtension();
	if (extensionNoPeriod.size() == 0)
	{
		return -1;
	}
	extensionNoPeriod = extensionNoPeriod.substr(1, extensionNoPeriod.size());
	std::string newScriptAddress = saveTextFileFromEdit(parentWindow, extensionNoPeriod, getCurrentProfileSettings());
	horizontalNiawgScript.saveScriptAs(newScriptAddress, mainWindowFriend->niawgIsRunning() );
	this->updateConfigurationSavedStatus(false);
	horizontalNiawgScript.updateScriptNameText();
	return 0;
}

void ScriptingWindow::updateScriptNamesOnScreen()
{
	this->horizontalNiawgScript.updateScriptNameText();
	this->verticalNiawgScript.updateScriptNameText();
	this->intensityAgilentScript.updateScriptNameText();
	return;
}

void ScriptingWindow::recolorScripts()
{
	this->verticalNiawgScript.colorEntireScript(getCurrentProfileSettings(), mainWindowFriend->getAllVariables());
	horizontalNiawgScript.colorEntireScript(getCurrentProfileSettings(), mainWindowFriend->getAllVariables());
	intensityAgilentScript.colorEntireScript(getCurrentProfileSettings(), mainWindowFriend->getAllVariables());
	return;
}

int ScriptingWindow::openIntensityScript(std::string name)
{
	return this->intensityAgilentScript.openParentScript(name, getCurrentProfileSettings(), mainWindowFriend->getAllVariables());
}

int ScriptingWindow::openVerticalScript(std::string name)
{
	return this->verticalNiawgScript.openParentScript(name, getCurrentProfileSettings(), mainWindowFriend->getAllVariables());
}

int ScriptingWindow::openHorizontalScript(std::string name)
{
	return horizontalNiawgScript.openParentScript(name, getCurrentProfileSettings(), mainWindowFriend->getAllVariables());
}

void ScriptingWindow::considerScriptLocations()
{
	verticalNiawgScript.considerCurrentLocation(getCurrentProfileSettings());
	horizontalNiawgScript.considerCurrentLocation(getCurrentProfileSettings());
	intensityAgilentScript.considerCurrentLocation(getCurrentProfileSettings());
}

/// End common functions
void ScriptingWindow::passCommonCommand(UINT id)
{
	// pass the command id to the common function, filling in the pointers to the windows which own objects needed.
	commonFunctions::handleCommonMessage(id, this, this->mainWindowFriend, this, this->cameraWindowFriend);
}

void ScriptingWindow::changeBoxColor( colorBoxes<char> colors )
{
	statusBox.changeColor(colors);
}

void ScriptingWindow::updateProfile(std::string text)
{
	profileDisplay.update(text);
}

profileSettings ScriptingWindow::getCurrentProfileSettings()
{	
	return mainWindowFriend->getCurentProfileSettings();
}

void ScriptingWindow::updateConfigurationSavedStatus(bool status)
{
	mainWindowFriend->updateConfigurationSavedStatus(status);
}
