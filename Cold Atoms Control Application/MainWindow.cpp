#include "stdafx.h"
#include "MainWindow.h"
#include "initializeMainWindow.h"
#include "commonMessages.h"


IMPLEMENT_DYNAMIC(MainWindow, CDialog)

BEGIN_MESSAGE_MAP(MainWindow, CDialog)
	ON_WM_CTLCOLOR()
	ON_COMMAND_RANGE(MENU_ID_RANGE_BEGIN, MENU_ID_RANGE_END, &MainWindow::passCommonCommand)
END_MESSAGE_MAP()

BOOL MainWindow::OnInitDialog()
{
	/// PROFILE DATA
	int id = 10190;
	POINT configStart = { 960, 0 };
	this->profile.initializeControls(configStart, this, id);
	POINT notesStart = { 960, 250 };
	this->notes.initializeControls(notesStart, this, id);
	POINT controlLocation = { 1440, 95 };
	this->variables.initializeControls(controlLocation, this, id);
	POINT settingsLocation = { 1440, 315 };
	this->settings.initialize(id, settingsLocation, this);

	POINT loc = { 1440, 565 };
	this->debugger.initialize(id, loc, this);
	
	initializeMainWindow(*this);
	CMenu menu;
	menu.LoadMenu(IDR_MAIN_MENU);
	this->SetMenu(&menu);
	this->ShowWindow(SW_MAXIMIZE);
	return TRUE;
}

void MainWindow::getFriends(ScriptingWindow* scriptingWindowPointer)
{
	this->scriptingWindowFriend = scriptingWindowPointer;
}

HBRUSH MainWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{
			pDC->SetTextColor(mainRGBs["White"]);
			pDC->SetBkColor(mainRGBs["Medium Grey"]);
			return mainBrushes["Medium Grey"];
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor(mainRGBs["White"]);
			pDC->SetBkColor(mainRGBs["Dark Blue"]);
			return mainBrushes["Dark Blue"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor(mainRGBs["White"]);
			pDC->SetBkColor(mainRGBs["Dark Grey"]);
			return mainBrushes["Dark Grey"];
		}
		default:
		{
			return mainBrushes["Light Grey"];
		}
	}
	return NULL;
}

void MainWindow::passCommonCommand(UINT id)
{
	// pass the command id to the common function, filling in the pointers to the windows which own objects needed.
	commonMessages::handleCommonMessage(id, this, this, this->scriptingWindowFriend);
	return;
}

profileSettings MainWindow::getCurentProfileSettings()
{
	return this->profile.getCurrentProfileSettings();
}

bool MainWindow::checkProfileReady()
{
	return this->profile.allSettingsReadyCheck(this->scriptingWindowFriend, this);
}

bool MainWindow::checkProfileSave()
{
	return this->profile.checkSaveEntireProfile(this->scriptingWindowFriend, this);
}

bool MainWindow::setOrientation(std::string orientation)
{
	return this->profile.setOrientation(orientation);
}

void MainWindow::updateConfigurationSavedStatus(bool status)
{
	this->profile.updateConfigurationSavedStatus(status);
	return;
}

std::string MainWindow::getNotes(std::string whichLevel)
{
	std::transform(whichLevel.begin(), whichLevel.end(), whichLevel.begin(), ::tolower);
	if (whichLevel == "experiment")
	{
		return this->notes.getExperimentNotes();
	}
	else if (whichLevel == "category")
	{
		return this->notes.getCategoryNotes();
	}
	else if (whichLevel == "configuration")
	{
		return this->notes.getConfigurationNotes();
	}
	else
	{
		throw std::invalid_argument(("The Main window's getNotes function was called with a bad argument:" 
			+ whichLevel + ". Acceptable arguments are \"experiment\", \"category\", and \"configuration\". "
			"This throw can be continued successfully, the notes will just be recorded.").c_str());
	}
	return "";
}
void MainWindow::setNotes(std::string whichLevel, std::string notes)
{
	std::transform(whichLevel.begin(), whichLevel.end(), whichLevel.begin(), ::tolower);
	if (whichLevel == "experiment")
	{
		this->notes.setExperimentNotes(notes);
	}
	else if (whichLevel == "category")
	{
		this->notes.setCategoryNotes(notes);
	}
	else if (whichLevel == "configuration")
	{
		this->notes.setConfigurationNotes(notes);
	}
	else
	{
		throw std::invalid_argument(("The Main window's setNotes function was called with a bad argument:"
			+ whichLevel + ". Acceptable arguments are \"experiment\", \"category\", and \"configuration\". "
			"This throw can be continued successfully, the notes will just not load.").c_str());
	}
	return;
}

std::vector<variable> MainWindow::getAllVariables()
{
	return this->variables.getAllVariables();
}

void MainWindow::clearVariables()
{
	this->variables.clearVariables();
	return;
}

void MainWindow::addVariable(std::string name, bool timelike, bool singleton, double value, int item)
{
	this->variables.addVariable(name, timelike, singleton, value, item);
	return;
}

debugOptions MainWindow::getDebuggingOptions()
{
	return this->debugger.getOptions();
}

void MainWindow::setDebuggingOptions(debugOptions options)
{
	this->debugger.setOptions(options);
	return;
}

mainOptions MainWindow::getMainOptions()
{
	return this->settings.getOptions();
}
