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
	if (id != IDC_DEBUG_OPTIONS_RANGE_BEGIN)
	{
		throw;
	}
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
	if (id - 1 != IDC_DEBUG_OPTIONS_RANGE_END)
	{
		throw;
	}
	return;
}

bool DebuggingOptionsControl::handleEvent(UINT id, MainWindow* mainWin)
{
	if (id == this->niawgMachineScript.ID)
	{
		BOOL checked = niawgMachineScript.GetCheck();
		if (checked) 
		{
			niawgMachineScript.SetCheck(0);
			this->currentOptions.outputNiawgMachineScript = false;
		}
		else 
		{
			niawgMachineScript.SetCheck(1);
			this->currentOptions.outputNiawgMachineScript = true;
		}
		mainWin->updateConfigurationSavedStatus(false);
		return true;
	}
	else if (id == this->niawgScript.ID)
	{
		BOOL checked = niawgScript.GetCheck();
		if (checked)
		{
			niawgScript.SetCheck(0);
			this->currentOptions.outputNiawgHumanScript = false;
		}
		else
		{
			niawgScript.SetCheck(1);
			this->currentOptions.outputNiawgHumanScript = true;
		}
		mainWin->updateConfigurationSavedStatus(false);
		return true;
	}
	else if (id == this->outputAgilentScript.ID)
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
	else if (id == this->readProgress.ID)
	{
		BOOL checked = readProgress.GetCheck();
		if (checked)
		{
			readProgress.SetCheck(0);
			this->currentOptions.showReadProgress= false;
		}
		else
		{
			readProgress.SetCheck(1);
			this->currentOptions.showReadProgress= true;
		}
		mainWin->updateConfigurationSavedStatus(false);
	}
	else if (id == this->writeProgress.ID)
	{
		BOOL checked = writeProgress.GetCheck();
		if (checked)
		{
			writeProgress.SetCheck(0);
			this->currentOptions.showWriteProgress= false;
		}
		else
		{
			writeProgress.SetCheck(1);
			this->currentOptions.showWriteProgress = true;
		}
		mainWin->updateConfigurationSavedStatus(false);
	}
	else if (id == this->correctionTimes.ID)
	{
		BOOL checked = correctionTimes.GetCheck();
		if (checked)
		{
			correctionTimes.SetCheck(0);
			this->currentOptions.showCorrectionTimes= false;
		}
		else
		{
			correctionTimes.SetCheck(1);
			this->currentOptions.showCorrectionTimes = true;
		}
		mainWin->updateConfigurationSavedStatus(false);
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