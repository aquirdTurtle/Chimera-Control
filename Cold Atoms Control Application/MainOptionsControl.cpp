
#include "stdafx.h"
#include "MainOptionsControl.h"

void MainOptionsControl::initialize(int& id, POINT& loc, CWnd* parent)
{
	header.ID = id++;
	header.position = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	header.Create("MAIN OPTIONS", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, header.position, parent, header.ID);
	header.SetFont(&eHeadingFont);
	loc.y += 20;
	//
	this->connectToMaster.ID = id++;
	connectToMaster.position = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	connectToMaster.Create("Connect to Master Computer?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, connectToMaster.position, parent, connectToMaster.ID);
	connectToMaster.SetFont(&eNormalFont);
	connectToMaster.SetCheck(0);
	currentOptions.connectToMaster = false;
	loc.y += 20;
	//
	this->getVariables.ID = id++;
	getVariables.position = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	getVariables.Create("Get Variables from Master Computer?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, getVariables.position, parent, getVariables.ID);
	getVariables.SetFont(&eNormalFont);
	getVariables.SetCheck(0);
	currentOptions.getVariables= false;
	loc.y += 20;
	//
	this->controlIntensity.ID = id++;
	controlIntensity.position = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	controlIntensity.Create("Program Intensity?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, controlIntensity.position, parent, controlIntensity.ID);
	controlIntensity.SetFont(&eNormalFont);
	controlIntensity.SetCheck(0);
	currentOptions.programIntensity = false;


	/*
	/// SETUP / EXPERIMENTAL PARAMETERS
	// Program intensity option
	eProgramIntensityOptionButton = CreateWindowEx(NULL, "BUTTON", "Program Intensity?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT,
		1440, 420, 480, 20, thisWindowHandle, (HMENU)IDC_PROGRAM_INTENSITY_BOX, GetModuleHandle(NULL), NULL);
	SendMessage(eProgramIntensityOptionButton, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	CheckDlgButton(thisWindowHandle, IDC_LOG_SCRIPT_PARAMS, BST_CHECKED);
	eProgramIntensityOption = true;

	*/
	return;
}


bool MainOptionsControl::handleEvent(HWND parent, UINT msg, WPARAM wParam, LPARAM lParam, MainWindow* mainWin)
{

	return TRUE;
}

mainOptions MainOptionsControl::getOptions()
{
	return this->currentOptions;
}

void MainOptionsControl::setOptions(mainOptions options)
{
	return;
}
