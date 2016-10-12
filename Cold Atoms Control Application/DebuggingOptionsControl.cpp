#include "stdafx.h"
#include "fonts.h"
#include "DebuggingOptionsControl.h"

void DebuggingOptionsControl::initialize(int& idStart, POINT& loc)
{
	/*
	TODO:
	HwndControl outputNiawgScript;
	HwndControl outputAgilentScript;
	*/
	RECT box;
	///
	outputNiawgMachineScript.ID = idStart++;
	box = outputNiawgMachineScript.position = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	outputNiawgMachineScript.hwnd = CreateWindowEx(NULL, "BUTTON", "Output Machine NIAWG Script?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT,
		box.left, box.top, box.right - box.left, box.bottom - box.top, eMainWindowHandle, (HMENU)outputNiawgMachineScript.ID, GetModuleHandle(NULL), NULL);
	SendMessage(outputNiawgMachineScript.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	CheckDlgButton(eMainWindowHandle, outputNiawgMachineScript.ID, BST_CHECKED);
	this->currentOptions.outputNiawgMachineScriptSetting = true;
	loc.y += 20;
	///
	outputAgilentScript.ID = idStart++;
	box = outputAgilentScript.position = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	outputAgilentScript.hwnd = CreateWindowEx(NULL, "BUTTON", "Output Agilent Script?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT,
		box.left, box.top, box.right - box.left, box.bottom - box.top, eMainWindowHandle, (HMENU)outputAgilentScript.ID, GetModuleHandle(NULL), NULL);
	SendMessage(outputAgilentScript.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	CheckDlgButton(eMainWindowHandle, outputAgilentScript.ID, BST_CHECKED);
	this->currentOptions.outputAgilentScript = true;
	loc.y += 20;
	///
	outputNiawgScript.ID = idStart++;
	box = outputNiawgScript.position = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	outputNiawgScript.hwnd = CreateWindowEx(NULL, "BUTTON", "Output Human NIAWG Script?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT,
		box.left, box.top, box.right - box.left, box.bottom - box.top, eMainWindowHandle, (HMENU)outputNiawgScript.ID, GetModuleHandle(NULL), NULL);
	SendMessage(outputNiawgScript.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	CheckDlgButton(eMainWindowHandle, outputNiawgScript.ID, BST_CHECKED);
	this->currentOptions.outputNiawgHumanScript = true;
	
	return;
}

bool DebuggingOptionsControl::handleEvent(HWND parent, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg != WM_COMMAND)
	{
		return false;
	}
	int id = LOWORD(wParam);
	if (id == this->outputNiawgMachineScript.ID)
	{
		BOOL checked = IsDlgButtonChecked(parent, outputNiawgMachineScript.ID);
		if (checked) 
		{
			CheckDlgButton(parent, outputNiawgMachineScript.ID, BST_UNCHECKED);
			this->currentOptions.outputNiawgMachineScriptSetting = false;
		}
		else 
		{
			CheckDlgButton(parent, outputNiawgMachineScript.ID, BST_CHECKED);
			this->currentOptions.outputNiawgMachineScriptSetting = true;
		}
		eProfile.updateExperimentSavedStatus(false);
		return true;
	}
	if (id == this->outputNiawgScript.ID)
	{
		BOOL checked = IsDlgButtonChecked(parent, outputNiawgScript.ID);
		if (checked)
		{
			CheckDlgButton(parent, outputNiawgScript.ID, BST_UNCHECKED);
			this->currentOptions.outputNiawgHumanScript = false;
		}
		else
		{
			CheckDlgButton(parent, outputNiawgScript.ID, BST_CHECKED);
			this->currentOptions.outputNiawgHumanScript = true;
		}
		eProfile.updateExperimentSavedStatus(false);
		return true;
	}
	if (id == this->outputAgilentScript.ID)
	{
		BOOL checked = IsDlgButtonChecked(parent, outputAgilentScript.ID);
		if (checked)
		{
			CheckDlgButton(parent, outputAgilentScript.ID, BST_UNCHECKED);
			this->currentOptions.outputAgilentScript = false;
		}
		else
		{
			CheckDlgButton(parent, outputAgilentScript.ID, BST_CHECKED);
			this->currentOptions.outputAgilentScript = true;
		}
		eProfile.updateExperimentSavedStatus(false);
		return true;
	}
	return false;
}

debuggingOptionsList DebuggingOptionsControl::getOptions()
{
	return this->currentOptions;
}