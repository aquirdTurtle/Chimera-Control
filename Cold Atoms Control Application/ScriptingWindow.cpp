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
	ON_EN_CHANGE(0, &ScriptingWindow::horizontalEditChange)
	ON_EN_CHANGE(0, &ScriptingWindow::agilentEditChange)
	ON_EN_CHANGE(0, &ScriptingWindow::agilentEditChange)
	ON_COMMAND_RANGE(MENU_ID_RANGE_BEGIN, MENU_ID_RANGE_END, &ScriptingWindow::passCommonCommand)
	ON_COMMAND(eVariableStatusMessageID, &ScriptingWindow::redrawBox)
	ON_COMMAND(eGreenMessageID, &ScriptingWindow::redrawBox)
	ON_COMMAND(eFatalErrorMessageID, &ScriptingWindow::redrawBox)
	ON_COMMAND(eNormalFinishMessageID, &ScriptingWindow::redrawBox)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

void ScriptingWindow::OnClose()
{
	if (this->horizontalNIAWGScript.checkSave(this->getCurrentProfileSettings()))
	{
		return;
	}
	if (this->verticalNIAWGScript.checkSave(this->getCurrentProfileSettings()))
	{
		return;
	}
	if (this->intensityAgilentScript.checkSave(this->getCurrentProfileSettings()))
	{
		return;
	}
	PostQuitMessage(0);
	CDialog::OnClose();
	return;
}

BOOL ScriptingWindow::OnInitDialog()
{
	// ADD MORE INITIALIZATIONS HERE
	int id = 101234;
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
	addresses.horizontalNIAWG = this->verticalNIAWGScript.getScriptAddress();
	addresses.horizontalNIAWG = this->intensityAgilentScript.getScriptAddress();
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
			bool result = this->statusBox.isColoringThisBox(num);
			if (result)
			{
				if (eGenStatusColor == "G")
				{
					// Color Green. This is the "Ready to give next waveform" color. During this color you can also press esc to exit.
					pDC->SetTextColor(RGB(255, 255, 255));
					pDC->SetBkColor(RGB(0, 120, 0));
					return eGreenBrush;
				}
				else if (eGenStatusColor == "Y")
				{
					// Color Yellow. This is the "Working" Color.
					pDC->SetTextColor(RGB(255, 255, 255));
					pDC->SetBkColor(RGB(104, 104, 0));
					return eYellowBrush;
				}
				else if (eGenStatusColor == "R")
				{
					// Color Red. This is a big visual signifier for when the program exited with error.
					pDC->SetTextColor(RGB(255, 255, 255));
					pDC->SetBkColor(RGB(120, 0, 0));
					return eRedBrush;
				}
				else
				{
					// color Blue. This is the default, ready for user input color.
					pDC->SetTextColor(RGB(255, 255, 255));
					pDC->SetBkColor(RGB(0, 0, 120));
					return blueBrush;
				}
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
	return;
}

void ScriptingWindow::agilentEditChange()
{
	return;
}

void ScriptingWindow::verticalEditChange()
{
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

void ScriptingWindow::redrawBox()
{
	this->statusBox.redraw();
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
