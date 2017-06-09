#include "stdafx.h"
#include "fonts.h"
#include "DebuggingOptionsControl.h"


void DebuggingOptionsControl::rearrange(int width, int height, fontMap fonts)
{
	header.rearrange("", "", width, height, fonts);
	readProgress.rearrange("", "", width, height, fonts);
	writeProgress.rearrange("", "", width, height, fonts);
	correctionTimes.rearrange("", "", width, height, fonts);
	niawgScript.rearrange("", "", width, height, fonts);
	outputAgilentScript.rearrange("", "", width, height, fonts);
	niawgMachineScript.rearrange("", "", width, height, fonts);
	excessInfo.rearrange("", "", width, height, fonts);
}

void DebuggingOptionsControl::initialize(int& id, POINT& loc, CWnd* parent, fontMap fonts,  
										 std::vector<CToolTipCtrl*>& tooltips)
{
	// Debugging Options Title
	header.ID = id++;
	header.sPos = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	header.Create("DEBUGGING OPTIONS", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, header.sPos, parent, header.ID);	
	header.SetFont(fonts["Heading Font"]);
	loc.y += 20;
	///
	if (id != IDC_DEBUG_OPTIONS_RANGE_BEGIN)
	{
		throw;
	}
	niawgMachineScript.ID = id++;
	niawgMachineScript.sPos = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	niawgMachineScript.Create("Output Machine NIAWG Script?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT,
							  niawgMachineScript.sPos, parent, niawgMachineScript.ID);
	niawgMachineScript.SetFont(fonts["Normal Font"]);
	niawgMachineScript.SetCheck(BST_CHECKED);
	this->currentOptions.outputNiawgMachineScript = true;
	loc.y += 20;
	///
	outputAgilentScript.ID = id++;
	outputAgilentScript.sPos = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	outputAgilentScript.Create("Output Agilent Script?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, outputAgilentScript.sPos, parent, outputAgilentScript.ID);
	outputAgilentScript.SetFont(fonts["Normal Font"]);
	outputAgilentScript.SetCheck(BST_CHECKED);
	this->currentOptions.outputAgilentScript = true;
	loc.y += 20;
	///
	niawgScript.ID = id++;
	niawgScript.sPos = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	niawgScript.Create("Output Human NIAWG Script?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, niawgScript.sPos, parent, niawgScript.ID);
	niawgScript.SetFont(fonts["Normal Font"]);
	niawgScript.SetCheck(BST_CHECKED);
	this->currentOptions.outputNiawgHumanScript = true;

	loc.y += 20;
	///
	this->readProgress.ID = id++;
	readProgress.sPos = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	readProgress.Create("Output Waveform Read Progress?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, readProgress.sPos, parent, readProgress.ID);
	readProgress.SetFont(fonts["Normal Font"]);
	readProgress.SetCheck(BST_CHECKED);
	this->currentOptions.showReadProgress = true;
	loc.y += 20;
	///
	this->writeProgress.ID = id++;
	writeProgress.sPos = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	writeProgress.Create("Output Waveform Write Progress?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, writeProgress.sPos, parent, writeProgress.ID);
	writeProgress.SetFont(fonts["Normal Font"]);
	writeProgress.SetCheck(BST_CHECKED);
	this->currentOptions.showWriteProgress = true;
	loc.y += 20;
	///
	this->correctionTimes.ID = id++;
	correctionTimes.sPos = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	correctionTimes.Create("Output Phase Correction Waveform Times?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, correctionTimes.sPos, parent, correctionTimes.ID);
	correctionTimes.SetFont(fonts["Normal Font"]);
	correctionTimes.SetCheck(BST_CHECKED);
	this->currentOptions.showCorrectionTimes= true;
	loc.y += 20;
	///
	this->excessInfo.ID = id++;
	excessInfo.sPos = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	excessInfo.Create("Output Excess Run Info?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, correctionTimes.sPos, parent, correctionTimes.ID);
	excessInfo.SetFont(fonts["Normal Font"]);
	excessInfo.SetCheck(BST_CHECKED);
	this->currentOptions.outputExcessInfo = true;	
	if (id - 1 != IDC_DEBUG_OPTIONS_RANGE_END)
	{
		throw;
	}
}

void DebuggingOptionsControl::handleEvent(UINT id, MainWindow* comm)
{
	if (id == this->niawgMachineScript.ID)
	{
		BOOL checked = niawgMachineScript.GetCheck();
		if (checked) 
		{
			niawgMachineScript.SetCheck(0);
			currentOptions.outputNiawgMachineScript = false;
		}
		else 
		{
			niawgMachineScript.SetCheck(1);
			this->currentOptions.outputNiawgMachineScript = true;
		}
		comm->updateConfigurationSavedStatus(false);
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
		comm->updateConfigurationSavedStatus(false);
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
		comm->updateConfigurationSavedStatus(false);
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
		comm->updateConfigurationSavedStatus(false);
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
		comm->updateConfigurationSavedStatus(false);
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
		comm->updateConfigurationSavedStatus(false);
	}
}


debugInfo DebuggingOptionsControl::getOptions()
{
	return this->currentOptions;
}

void DebuggingOptionsControl::setOptions(debugInfo options)
{
	currentOptions = options;
}