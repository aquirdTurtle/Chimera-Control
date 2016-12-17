#include "stdafx.h"
#include "fonts.h"
#include "DebuggingOptionsControl.h"

void DebuggingOptionsControl::initialize(int& id, POINT& loc, CWnd* parent)
{
	// Debugging Options Title
	header.ID = id++;
	header.position = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	header.Create("DEBUGGING OPTIONS", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, header.position, parent, header.ID);
	header.SetFont(&eHeadingFont);
	loc.y += 20;
	///
	niawgMachineScript.ID = id++;
	niawgMachineScript.position = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	niawgMachineScript.Create("Output Machine NIAWG Script?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, niawgMachineScript.position, parent, niawgMachineScript.ID);
	niawgMachineScript.SetFont(&eNormalFont);
	niawgMachineScript.SetCheck(BST_CHECKED);
	this->currentOptions.outputNiawgMachineScript = true;
	loc.y += 20;
	///
	outputAgilentScript.ID = id++;
	outputAgilentScript.position = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	outputAgilentScript.Create("Output Agilent Script?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, outputAgilentScript.position, parent, outputAgilentScript.ID);
	outputAgilentScript.SetFont(&eNormalFont);
	outputAgilentScript.SetCheck(BST_CHECKED);
	this->currentOptions.outputAgilentScript = true;
	loc.y += 20;
	///
	niawgScript.ID = id++;
	niawgScript.position = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	niawgScript.Create("Output Human NIAWG Script?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, niawgScript.position, parent, niawgScript.ID);
	niawgScript.SetFont(&eNormalFont);
	niawgScript.SetCheck(BST_CHECKED);
	this->currentOptions.outputNiawgHumanScript = true;

	loc.y += 20;
	///
	this->readProgress.ID = id++;
	readProgress.position = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	readProgress.Create("Output Waveform Read Progress?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, readProgress.position, parent, readProgress.ID);
	readProgress.SetFont(&eNormalFont);
	readProgress.SetCheck(BST_CHECKED);
	this->currentOptions.showReadProgress = true;
	loc.y += 20;
	///
	this->writeProgress.ID = id++;
	writeProgress.position = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	writeProgress.Create("Output Waveform Write Progress?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, writeProgress.position, parent, writeProgress.ID);
	writeProgress.SetFont(&eNormalFont);
	writeProgress.SetCheck(BST_CHECKED);
	this->currentOptions.showWriteProgress = true;
	loc.y += 20;
	///
	this->correctionTimes.ID = id++;
	correctionTimes.position = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	correctionTimes.Create("Output Phase Correction Waveform Times?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, correctionTimes.position, parent, correctionTimes.ID);
	correctionTimes.SetFont(&eNormalFont);
	correctionTimes.SetCheck(BST_CHECKED);
	this->currentOptions.showCorrectionTimes= true;
	loc.y += 20;
	///
	this->excessInfo.ID = id++;
	excessInfo.position = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	excessInfo.Create("Output Excess Run Info?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, correctionTimes.position, parent, correctionTimes.ID);
	excessInfo.SetFont(&eNormalFont);
	excessInfo.SetCheck(BST_CHECKED);
	this->currentOptions.outputExcessInfo = true;	
	/*
	/// DEBUGGING OPTIONS
	eOutputMoreInfoCheckButton = CreateWindowEx(NULL, "BUTTON", "Output More Run Info?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT,
												1440, 545, 480, 20, thisWindowHandle, (HMENU)IDC_OUTPUT_MORE_RUN_INFO, GetModuleHandle(NULL), NULL);
	SendMessage(eOutputMoreInfoCheckButton, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	CheckDlgButton(thisWindowHandle, IDC_OUTPUT_MORE_RUN_INFO, BST_UNCHECKED);
	eOutputRunInfo = false;
	*/

	return;
}

bool DebuggingOptionsControl::handleEvent(HWND parent, UINT msg, WPARAM wParam, LPARAM lParam, MainWindow* mainWin)
{
	if (msg != WM_COMMAND)
	{
		return false;
	}
	int id = LOWORD(wParam);
	if (id == this->niawgMachineScript.ID)
	{
		BOOL checked = niawgMachineScript.GetCheck();
		if (checked) 
		{
			niawgMachineScript.SetCheck(BST_UNCHECKED);
			this->currentOptions.outputNiawgMachineScript = false;
		}
		else 
		{
			niawgMachineScript.SetCheck(BST_UNCHECKED);
			this->currentOptions.outputNiawgMachineScript = true;
		}
		mainWin->updateConfigurationSavedStatus(false);
		return true;
	}
	if (id == this->niawgScript.ID)
	{
		BOOL checked = niawgScript.GetCheck();
		if (checked)
		{
			niawgScript.SetCheck(BST_UNCHECKED);
			this->currentOptions.outputNiawgHumanScript = false;
		}
		else
		{
			niawgScript.SetCheck(BST_CHECKED);
			this->currentOptions.outputNiawgHumanScript = true;
		}
		mainWin->updateConfigurationSavedStatus(false);
		return true;
	}
	if (id == this->outputAgilentScript.ID)
	{
		BOOL checked = outputAgilentScript.GetCheck();
		if (checked)
		{
			outputAgilentScript.SetCheck(0);
			this->currentOptions.outputAgilentScript = false;
		}
		else
		{
			outputAgilentScript.SetCheck(1);
			this->currentOptions.outputAgilentScript = true;
		}
		mainWin->updateConfigurationSavedStatus(false);
		return true;
	}
	return false;
}

debugOptions DebuggingOptionsControl::getOptions()
{
	return this->currentOptions;
}

void DebuggingOptionsControl::setOptions(debugOptions options)
{
	this->currentOptions = options;
	return;
}