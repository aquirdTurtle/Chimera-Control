#include "stdafx.h"
#include "ScriptingWindow.h"
#include "afxwin.h"
#include "MainWindow.h"
#include "getFileName.h"
#include "saveTextFileFromEdit.h"
#include "commonMessages.h"

IMPLEMENT_DYNAMIC(ScriptingWindow, CDialog)

BEGIN_MESSAGE_MAP(ScriptingWindow, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
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

void ScriptingWindow::handleHorizontalScriptComboChange()
{
	this->horizontalNIAWGScript.childComboChangeHandler(this, this->mainWindowFriend);
	return;
}

void ScriptingWindow::handleVerticalScriptComboChange()
{
	this->verticalNIAWGScript.childComboChangeHandler(this, this->mainWindowFriend);
	return;
}

void ScriptingWindow::handleAgilentScriptComboChange()
{
	this->intensityAgilentScript.childComboChangeHandler(this, this->mainWindowFriend);
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
	verticalNIAWGScript.initializeControls(640, 1000, startLocaiton, this, "Vertical NIAWG", id);
	startLocaiton = { 640, 28 };
	horizontalNIAWGScript.initializeControls(640, 1000, startLocaiton, this, "Horizontal NIAWG", id);
	startLocaiton = { 1280, 28 };
	intensityAgilentScript.initializeControls(640, 1000, startLocaiton, this, "Agilent", id);
	statusBox.initialize({ 1880, 3 }, id, this);
	this->profileDisplay.initialize({ 0,3 }, id, this);
	// legacy... to be removed soon.
	//initializeScriptingWindow(*this);

	CMenu menu;
	menu.LoadMenu(IDR_MAIN_MENU);
	this->SetMenu(&menu);
	
	this->ShowWindow(SW_MAXIMIZE);
	return TRUE;
}

void ScriptingWindow::OnTimer(UINT_PTR eventID)
{
	this->horizontalNIAWGScript.handleTimerCall(this->mainWindowFriend->getCurentProfileSettings(), this->mainWindowFriend->getAllVariables());
	this->intensityAgilentScript.handleTimerCall(this->mainWindowFriend->getCurentProfileSettings(), this->mainWindowFriend->getAllVariables());
	this->verticalNIAWGScript.handleTimerCall(this->mainWindowFriend->getCurentProfileSettings(), this->mainWindowFriend->getAllVariables());
	return;
}

bool ScriptingWindow::checkScriptSaves()
{
	if (this->horizontalNIAWGScript.checkSave(this->getCurrentProfileSettings()))
	{
		return true;
	}
	if (this->verticalNIAWGScript.checkSave(this->getCurrentProfileSettings()))
	{
		return true;
	}
	if (this->intensityAgilentScript.checkSave(this->getCurrentProfileSettings()))
	{
		return true;
	}
	return false;
}

void ScriptingWindow::getFriends(MainWindow* mainWindowPointer)
{
	this->mainWindowFriend = mainWindowPointer;
}

/*
	This function retuns the names (just the names) of currently active scripts.
*/
scriptInfo<std::string> ScriptingWindow::getScriptNames()
{
	scriptInfo<std::string> names;
	// Order matters!
	names.horizontalNIAWG = this->horizontalNIAWGScript.getScriptName();
	names.verticalNIAWG = this->verticalNIAWGScript.getScriptName();
	names.intensityAgilent = this->intensityAgilentScript.getScriptName();
	return names;
}

/*
	This function returns indicators of whether a given script has been saved or not.
*/
scriptInfo<bool> ScriptingWindow::getScriptSavedStatuses()
{
	scriptInfo<bool> status;
	status.horizontalNIAWG = this->horizontalNIAWGScript.savedStatus();
	status.verticalNIAWG = this->verticalNIAWGScript.savedStatus();
	status.intensityAgilent = this->intensityAgilentScript.savedStatus();
	return status;
}

/*
	This function returns the current addresses of all files in all scripts.
*/
scriptInfo<std::string> ScriptingWindow::getScriptAddresses()
{
	scriptInfo<std::string> addresses;
	addresses.horizontalNIAWG = this->horizontalNIAWGScript.getScriptAddress();
	addresses.verticalNIAWG = this->verticalNIAWGScript.getScriptAddress();
	addresses.intensityAgilent = this->intensityAgilentScript.getScriptAddress();
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
			int num = (pWnd->GetDlgCtrlID());
			HBRUSH result = this->statusBox.handleColoring(num, pDC);
			if (result)
			{
				return result;
			}
			else
			{
				pDC->SetTextColor(scriptRGBs["White"]);
				pDC->SetBkColor(scriptRGBs["Dark Grey Red"]);
				return scriptBrushes["Dark Grey Red"];
			}
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor(scriptRGBs["White"]);
			pDC->SetBkColor(scriptRGBs["Dark Red"]);
			return scriptBrushes["Dark Red"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor(scriptRGBs["White"]);
			pDC->SetBkColor(scriptRGBs["Dark Grey"]);
			return scriptBrushes["Dark Grey"];
		}
		default:
		{
			return scriptBrushes["Light Grey"];
		}
	}
}

void ScriptingWindow::horizontalEditChange()
{
	this->horizontalNIAWGScript.handleEditChange();
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
	this->verticalNIAWGScript.handleEditChange();
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
	if (this->intensityAgilentScript.checkSave(this->getCurrentProfileSettings()))
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
	if (intensityAgilentScript.checkSave(this->getCurrentProfileSettings()))
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
	intensityAgilentScript.saveScript(this->getCurrentProfileSettings());
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
	intensityAgilentScript.saveScriptAs(newScriptAddress);
	this->updateConfigurationSavedStatus(false);
	intensityAgilentScript.updateScriptNameText();
	return 0;
}
int ScriptingWindow::newVerticalScript()
{
	if (this->verticalNIAWGScript.checkSave(this->getCurrentProfileSettings()))
	{
		return true;
	}
	verticalNIAWGScript.newScript(this->getCurrentProfileSettings(), this->mainWindowFriend->getAllVariables());
	this->updateConfigurationSavedStatus(false);
	verticalNIAWGScript.updateScriptNameText();
	return 0;
}
int ScriptingWindow::openVerticalScript(HWND parentWindow)
{
	if (verticalNIAWGScript.checkSave(this->getCurrentProfileSettings()))
	{
		return true;
	}
	std::string intensityOpenName = getFileNameDialog(parentWindow);
	verticalNIAWGScript.openParentScript(intensityOpenName, this->getCurrentProfileSettings(), this->mainWindowFriend->getAllVariables());
	this->updateConfigurationSavedStatus(false);
	verticalNIAWGScript.updateScriptNameText();
	return 0;
}
int ScriptingWindow::saveVerticalScript()
{
	verticalNIAWGScript.saveScript(this->getCurrentProfileSettings());
	verticalNIAWGScript.updateScriptNameText();
	return 0;
}
int ScriptingWindow::saveVerticalScriptAs(HWND parentWindow)
{
	std::string extensionNoPeriod = verticalNIAWGScript.getExtension();
	if (extensionNoPeriod.size() == 0)
	{
		return -1;
	}
	extensionNoPeriod = extensionNoPeriod.substr(1, extensionNoPeriod.size());
	std::string newScriptAddress = saveTextFileFromEdit(parentWindow, extensionNoPeriod, this->getCurrentProfileSettings());
	verticalNIAWGScript.saveScriptAs(newScriptAddress);
	this->updateConfigurationSavedStatus(false);
	verticalNIAWGScript.updateScriptNameText();
	return 0;
}

int ScriptingWindow::newHorizontalScript()
{
	if (this->horizontalNIAWGScript.checkSave(this->getCurrentProfileSettings()))
	{
		return true;
	}
	horizontalNIAWGScript.newScript(this->getCurrentProfileSettings(), this->mainWindowFriend->getAllVariables());
	this->updateConfigurationSavedStatus(false);
	horizontalNIAWGScript.updateScriptNameText();
	return 0;
}


int ScriptingWindow::openHorizontalScript(HWND parentWindow)
{
	if (horizontalNIAWGScript.checkSave(this->getCurrentProfileSettings()))
	{
		return true;
	}
	std::string horizontalOpenName = getFileNameDialog(parentWindow);
	horizontalNIAWGScript.openParentScript(horizontalOpenName, this->getCurrentProfileSettings(), this->mainWindowFriend->getAllVariables());
	this->updateConfigurationSavedStatus(false);
	horizontalNIAWGScript.updateScriptNameText();
	return 0;
}


int ScriptingWindow::saveHorizontalScript()
{
	horizontalNIAWGScript.saveScript(this->getCurrentProfileSettings());
	horizontalNIAWGScript.updateScriptNameText();
	return 0;
}


int ScriptingWindow::saveHorizontalScriptAs(HWND parentWindow)
{
	std::string extensionNoPeriod = this->horizontalNIAWGScript.getExtension();
	if (extensionNoPeriod.size() == 0)
	{
		return -1;
	}
	extensionNoPeriod = extensionNoPeriod.substr(1, extensionNoPeriod.size());
	std::string newScriptAddress = saveTextFileFromEdit(parentWindow, extensionNoPeriod, this->getCurrentProfileSettings());
	horizontalNIAWGScript.saveScriptAs(newScriptAddress);
	this->updateConfigurationSavedStatus(false);
	horizontalNIAWGScript.updateScriptNameText();
	return 0;
}

void ScriptingWindow::updateScriptNamesOnScreen()
{
	this->horizontalNIAWGScript.updateScriptNameText();
	this->verticalNIAWGScript.updateScriptNameText();
	this->intensityAgilentScript.updateScriptNameText();
	return;
}

void ScriptingWindow::recolorScripts()
{
	this->verticalNIAWGScript.colorEntireScript(this->getCurrentProfileSettings(), this->mainWindowFriend->getAllVariables());
	horizontalNIAWGScript.colorEntireScript(this->getCurrentProfileSettings(), this->mainWindowFriend->getAllVariables());
	intensityAgilentScript.colorEntireScript(this->getCurrentProfileSettings(), this->mainWindowFriend->getAllVariables());
	return;
}

int ScriptingWindow::openIntensityScript(std::string name)
{
	return this->intensityAgilentScript.openParentScript(name, this->getCurrentProfileSettings(), this->mainWindowFriend->getAllVariables());
}

int ScriptingWindow::openVerticalScript(std::string name)
{
	return this->verticalNIAWGScript.openParentScript(name, this->getCurrentProfileSettings(), this->mainWindowFriend->getAllVariables());
}

int ScriptingWindow::openHorizontalScript(std::string name)
{
	return this->horizontalNIAWGScript.openParentScript(name, this->getCurrentProfileSettings(), this->mainWindowFriend->getAllVariables());
}

void ScriptingWindow::considerScriptLocations()
{
	verticalNIAWGScript.considerCurrentLocation(this->getCurrentProfileSettings());
	horizontalNIAWGScript.considerCurrentLocation(this->getCurrentProfileSettings());
	intensityAgilentScript.considerCurrentLocation(this->getCurrentProfileSettings());
	return;
}

/// End common functions
void ScriptingWindow::passCommonCommand(UINT id)
{
	// pass the command id to the common function, filling in the pointers to the windows which own objects needed.
	commonMessages::handleCommonMessage(id, this, this->mainWindowFriend, this);
	return;
}

void ScriptingWindow::changeBoxColor(std::string color)
{
	this->statusBox.changeColor(color);
	return;
}

void ScriptingWindow::updateProfile(std::string text)
{
	this->profileDisplay.update(text);
	return;
}

profileSettings ScriptingWindow::getCurrentProfileSettings()
{	
	return this->mainWindowFriend->getCurentProfileSettings();
}

void ScriptingWindow::updateConfigurationSavedStatus(bool status)
{
	this->mainWindowFriend->updateConfigurationSavedStatus(status);
	return;
}
