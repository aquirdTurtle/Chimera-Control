#include "stdafx.h"
#include "DebuggingOptionsControl.h"


void DebugOptionsControl::rearrange(int width, int height, fontMap fonts)
{
	header.rearrange(width, height, fonts);
	readProgress.rearrange(width, height, fonts);
	writeProgress.rearrange(width, height, fonts);
	correctionTimes.rearrange(width, height, fonts);
	niawgScript.rearrange(width, height, fonts);
	outputAgilentScript.rearrange(width, height, fonts);
	niawgMachineScript.rearrange(width, height, fonts);
	excessInfo.rearrange(width, height, fonts);
	showTtlsButton.rearrange(width, height, fonts);
	showDacsButton.rearrange(width, height, fonts);
	pauseText.rearrange(width, height, fonts);
	pauseEdit.rearrange(width, height, fonts);
}


void DebugOptionsControl::handleNewConfig( std::ofstream& newFile )
{
	newFile << "DEBUGGING_OPTIONS\n";
	newFile << 0 << "\n";
	newFile << 0 << "\n";
	newFile << 0 << "\n";
	newFile << 0 << "\n";
	newFile << 0 << "\n";
	newFile << 1 << "\n";
	newFile << 1 << "\n";
	newFile << 0 << "\n";
	newFile << 0 << "\n";
	newFile << 0 << "\n";
	newFile << "END_DEBUGGING_OPTIONS\n";
}


void DebugOptionsControl::handleSaveConfig(std::ofstream& saveFile)
{
	saveFile << "DEBUGGING_OPTIONS\n";
	saveFile << currentOptions.outputAgilentScript << "\n";
	saveFile << currentOptions.outputExcessInfo << "\n";
	saveFile << currentOptions.outputNiawgHumanScript << "\n";
	saveFile << currentOptions.outputNiawgMachineScript << "\n";
	saveFile << currentOptions.showCorrectionTimes << "\n";
	saveFile << currentOptions.showDacs << "\n";
	saveFile << currentOptions.showTtls << "\n";
	saveFile << currentOptions.showReadProgress << "\n";
	saveFile << currentOptions.showWriteProgress << "\n";
	saveFile << currentOptions.sleepTime << "\n";
	saveFile << "END_DEBUGGING_OPTIONS\n";
}


void DebugOptionsControl::handleOpenConfig(std::ifstream& openFile, double version)
{
	ProfileSystem::checkDelimiterLine(openFile, "DEBUGGING_OPTIONS");
	openFile >> currentOptions.outputAgilentScript;
	openFile >> currentOptions.outputExcessInfo;
	openFile >> currentOptions.outputNiawgHumanScript;
	openFile >> currentOptions.outputNiawgMachineScript;
	openFile >> currentOptions.showCorrectionTimes;
	openFile >> currentOptions.showDacs;
	openFile >> currentOptions.showTtls;
	openFile >> currentOptions.showReadProgress;
	openFile >> currentOptions.showWriteProgress;
	std::string sleep;
	openFile >> sleep;
	try
	{
		currentOptions.sleepTime = std::stol(sleep);
	}
	catch (std::invalid_argument&)
	{
		currentOptions.sleepTime = 0;
	}
	ProfileSystem::checkDelimiterLine(openFile, "END_DEBUGGING_OPTIONS");
	setOptions( currentOptions );
}


void DebugOptionsControl::initialize( int& id, POINT& loc, CWnd* parent, cToolTips& tooltips)
{
	// Debugging Options Title
	header.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 25 };
	header.Create("DEBUGGING OPTIONS", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, header.sPos, parent, id++);	
	header.fontType = HeadingFont;
	///
	UINT runningCount = 0;
	niawgMachineScript.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	niawgMachineScript.Create("Output Machine NIAWG Script?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT,
							  niawgMachineScript.sPos, parent, IDC_DEBUG_OPTIONS_RANGE_BEGIN + runningCount++ );
	niawgMachineScript.SetCheck(BST_CHECKED);

	currentOptions.outputNiawgMachineScript = true;
	///
	outputAgilentScript.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	outputAgilentScript.Create("Output Agilent Script?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, 
							   outputAgilentScript.sPos, parent, IDC_DEBUG_OPTIONS_RANGE_BEGIN + runningCount++ );
	outputAgilentScript.SetCheck(BST_CHECKED);
	currentOptions.outputAgilentScript = true;
	///
	niawgScript.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	niawgScript.Create("Output Human NIAWG Script?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, niawgScript.sPos, 
					   parent, IDC_DEBUG_OPTIONS_RANGE_BEGIN + runningCount++ );
	niawgScript.SetCheck(BST_CHECKED);
	currentOptions.outputNiawgHumanScript = true;

	///
	readProgress.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	readProgress.Create("Output Waveform Read Progress?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, 
						readProgress.sPos, parent, IDC_DEBUG_OPTIONS_RANGE_BEGIN + runningCount++ );
	readProgress.SetCheck(BST_CHECKED);
	currentOptions.showReadProgress = true;
	///
	writeProgress.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	writeProgress.Create("Output Waveform Write Progress?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, 
						 writeProgress.sPos, parent, IDC_DEBUG_OPTIONS_RANGE_BEGIN + runningCount++ );
	writeProgress.SetCheck(BST_CHECKED);
	currentOptions.showWriteProgress = true;
	///
	correctionTimes.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	correctionTimes.Create("Output Phase Correction Waveform Times?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, 
						   correctionTimes.sPos, parent, IDC_DEBUG_OPTIONS_RANGE_BEGIN + runningCount++ );
	correctionTimes.SetCheck(BST_CHECKED);
	currentOptions.showCorrectionTimes= true;
	///
	excessInfo.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	excessInfo.Create( "Output Excess Run Info?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, correctionTimes.sPos, 
					   parent, IDC_DEBUG_OPTIONS_RANGE_BEGIN + runningCount++ );
	excessInfo.SetCheck(BST_CHECKED);
	currentOptions.outputExcessInfo = true;	

	showTtlsButton.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	showTtlsButton.Create( "Show All TTL Events", WS_CHILD | WS_VISIBLE | SS_CENTER | BS_CHECKBOX | BS_RIGHT,
						   showTtlsButton.sPos, parent, IDC_SHOW_TTLS );

	showDacsButton.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	showDacsButton.Create("Show All Dac Events", WS_CHILD | WS_VISIBLE | SS_CENTER | BS_CHECKBOX | BS_RIGHT,
						  showDacsButton.sPos, parent, IDC_SHOW_DACS );

	pauseText.sPos = { loc.x + 240, loc.y, loc.x + 480, loc.y + 20 };
	pauseText.Create("Pause Between Variations (ms)", WS_CHILD | WS_VISIBLE | ES_RIGHT, pauseText.sPos, parent, id++);

	pauseEdit.sPos = { loc.x, loc.y, loc.x + 240, loc.y += 20 };
	pauseEdit.Create(WS_CHILD | WS_VISIBLE , pauseEdit.sPos, parent, id++ );
	pauseEdit.SetWindowTextA("0");
}

void DebugOptionsControl::handleEvent(UINT id, MainWindow* comm)
{
	if (id == niawgMachineScript.GetDlgCtrlID())
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
			currentOptions.outputNiawgMachineScript = true;
		}
		comm->updateConfigurationSavedStatus(false);
	}
	else if (id == niawgScript.GetDlgCtrlID())
	{
		BOOL checked = niawgScript.GetCheck();
		if (checked)
		{
			niawgScript.SetCheck(0);
			currentOptions.outputNiawgHumanScript = false;
		}
		else
		{
			niawgScript.SetCheck(1);
			currentOptions.outputNiawgHumanScript = true;
		}
		comm->updateConfigurationSavedStatus(false);
	}
	else if (id == outputAgilentScript.GetDlgCtrlID())
	{
		BOOL checked = outputAgilentScript.GetCheck();
		if (checked)
		{
			outputAgilentScript.SetCheck(0);
			currentOptions.outputAgilentScript = false;
		}
		else
		{
			outputAgilentScript.SetCheck(1);
			currentOptions.outputAgilentScript = true;
		}
		comm->updateConfigurationSavedStatus(false);
	}
	else if (id == readProgress.GetDlgCtrlID())
	{
		BOOL checked = readProgress.GetCheck();
		if (checked)
		{
			readProgress.SetCheck(0);
			currentOptions.showReadProgress= false;
		}
		else
		{
			readProgress.SetCheck(1);
			currentOptions.showReadProgress= true;
		}
		comm->updateConfigurationSavedStatus(false);
	}
	else if (id == writeProgress.GetDlgCtrlID())
	{
		BOOL checked = writeProgress.GetCheck();
		if (checked)
		{
			writeProgress.SetCheck(0);
			currentOptions.showWriteProgress= false;
		}
		else
		{
			writeProgress.SetCheck(1);
			currentOptions.showWriteProgress = true;
		}
		comm->updateConfigurationSavedStatus(false);
	}
	else if (id == correctionTimes.GetDlgCtrlID())
	{
		BOOL checked = correctionTimes.GetCheck();
		if (checked)
		{
			correctionTimes.SetCheck(0);
			currentOptions.showCorrectionTimes= false;
		}
		else
		{
			correctionTimes.SetCheck(1);
			currentOptions.showCorrectionTimes = true;
		}
		comm->updateConfigurationSavedStatus(false);
	}
	else if ( id == excessInfo.GetDlgCtrlID( ) )
	{
		BOOL checked = excessInfo.GetCheck( );
		if ( checked )
		{
			excessInfo.SetCheck( 0 );
			currentOptions.outputExcessInfo = false;
		}
		else
		{
			excessInfo.SetCheck( 1 );
			currentOptions.outputExcessInfo = true;
		}
		comm->updateConfigurationSavedStatus( false );
	}
	else if (id == showTtlsButton.GetDlgCtrlID())
	{
		BOOL checked = showTtlsButton.GetCheck();
		if (checked)
		{
			showTtlsButton.SetCheck(0);
			currentOptions.showTtls = false;
		}
		else
		{
			showTtlsButton.SetCheck(1);
			currentOptions.showTtls = true;
		}
		comm->updateConfigurationSavedStatus(false);
	}
	else if (id == showDacsButton.GetDlgCtrlID())
	{
		BOOL checked = showDacsButton.GetCheck();
		if (checked)
		{
			showDacsButton.SetCheck(0);
			currentOptions.showDacs = false;
		}
		else
		{
			showDacsButton.SetCheck(1);
			currentOptions.showDacs = true;
		}
		comm->updateConfigurationSavedStatus(false);
	}
	else if (id == pauseEdit.GetDlgCtrlID())
	{
		comm->updateConfigurationSavedStatus(false);
	}
}


debugInfo DebugOptionsControl::getOptions()
{
	currentOptions.outputNiawgMachineScript = niawgMachineScript.GetCheck();
	currentOptions.outputNiawgHumanScript = niawgScript.GetCheck();
	currentOptions.outputAgilentScript = outputAgilentScript.GetCheck();
	currentOptions.showReadProgress = readProgress.GetCheck();
	currentOptions.showWriteProgress = writeProgress.GetCheck();
	currentOptions.showCorrectionTimes = correctionTimes.GetCheck();
	currentOptions.showTtls = showTtlsButton.GetCheck();
	currentOptions.showDacs = showDacsButton.GetCheck();
	CString text;
	pauseEdit.GetWindowTextA(text);
	currentOptions.sleepTime = std::stol(str(text));
	return currentOptions;
}


void DebugOptionsControl::setOptions(debugInfo options)
{
	currentOptions = options;
	readProgress.SetCheck( currentOptions.showReadProgress );
	writeProgress.SetCheck( currentOptions.showWriteProgress );
	correctionTimes.SetCheck(currentOptions.showCorrectionTimes);
	niawgScript.SetCheck(currentOptions.outputNiawgHumanScript);
	outputAgilentScript.SetCheck( currentOptions.outputAgilentScript );
	niawgMachineScript.SetCheck( currentOptions.outputNiawgMachineScript);
	excessInfo.SetCheck( currentOptions.outputExcessInfo);
	showTtlsButton.SetCheck( currentOptions.showTtls);
	showDacsButton.SetCheck( currentOptions.showDacs);
	pauseEdit.SetWindowTextA( cstr( currentOptions.sleepTime ) );
}
