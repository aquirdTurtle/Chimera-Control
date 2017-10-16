#include "stdafx.h"
#include "DebugOptionsControl.h"


void DebugOptionsControl::initialize( int& id, POINT& loc, CWnd* parent, cToolTips& tooltips )
{
	// Debugging Options Title
	header.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 25 };
	header.Create( "DEBUGGING OPTIONS", NORM_HEADER_OPTIONS, header.sPos, parent, id++ );
	header.fontType = HeadingFont;
	UINT count = 0;
	niawgMachineScript.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	niawgMachineScript.Create( "Output Machine NIAWG Script?", NORM_CHECK_OPTIONS, niawgMachineScript.sPos, parent, 
							   IDC_DEBUG_OPTIONS_RANGE_BEGIN + count++ );
	niawgMachineScript.SetCheck( BST_CHECKED );

	currentOptions.outputNiawgMachineScript = true;
	outputAgilentScript.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	outputAgilentScript.Create( "Output Agilent Script?", NORM_CHECK_OPTIONS, outputAgilentScript.sPos, parent, 
								IDC_DEBUG_OPTIONS_RANGE_BEGIN + count++ );
	outputAgilentScript.SetCheck( BST_CHECKED );
	currentOptions.outputAgilentScript = true;
	///
	niawgScript.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	niawgScript.Create( "Output Human NIAWG Script?", NORM_CHECK_OPTIONS, niawgScript.sPos, parent, 
						IDC_DEBUG_OPTIONS_RANGE_BEGIN + count++ );
	niawgScript.SetCheck( BST_CHECKED );
	currentOptions.outputNiawgHumanScript = true;

	///
	readProgress.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	readProgress.Create( "Output Waveform Read Progress?", NORM_CHECK_OPTIONS, readProgress.sPos, parent, 
						 IDC_DEBUG_OPTIONS_RANGE_BEGIN + count++ );
	readProgress.SetCheck( BST_CHECKED );
	currentOptions.showReadProgress = true;
	///
	writeProgress.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	writeProgress.Create( "Output Waveform Write Progress?", NORM_CHECK_OPTIONS, writeProgress.sPos, parent, 
						  IDC_DEBUG_OPTIONS_RANGE_BEGIN + count++ );
	writeProgress.SetCheck( BST_CHECKED );
	currentOptions.showWriteProgress = true;
	///
	correctionTimes.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	correctionTimes.Create( "Output Phase Correction Waveform Times?", NORM_CHECK_OPTIONS, correctionTimes.sPos,
							parent, IDC_DEBUG_OPTIONS_RANGE_BEGIN + count++ );
	correctionTimes.SetCheck( BST_CHECKED );
	currentOptions.showCorrectionTimes = true;
	///
	excessInfo.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	excessInfo.Create( "Output Excess Run Info?", NORM_CHECK_OPTIONS, correctionTimes.sPos, parent, 
					   IDC_DEBUG_OPTIONS_RANGE_BEGIN + count++ );
	excessInfo.SetCheck( BST_CHECKED );
	currentOptions.outputExcessInfo = true;

	outputNiawgWavesToText.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	outputNiawgWavesToText.Create( "Output Niawg Waveforms to .txt?", NORM_CHECK_OPTIONS, outputNiawgWavesToText.sPos, 
								   parent, IDC_DEBUG_OPTIONS_RANGE_BEGIN + count++ );

	showTtlsButton.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	showTtlsButton.Create( "Show All TTL Events", NORM_CHECK_OPTIONS, showTtlsButton.sPos, parent, IDC_SHOW_TTLS );

	showDacsButton.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	showDacsButton.Create( "Show All Dac Events", NORM_CHECK_OPTIONS, showDacsButton.sPos, parent, IDC_SHOW_DACS );

	pauseText.sPos = { loc.x + 240, loc.y, loc.x + 480, loc.y + 20 };
	pauseText.Create( "Pause Between Variations (ms)", NORM_STATIC_OPTIONS, pauseText.sPos, parent, id++ );

	pauseEdit.sPos = { loc.x, loc.y, loc.x + 240, loc.y += 20 };
	pauseEdit.Create( NORM_EDIT_OPTIONS, pauseEdit.sPos, parent, id++ );
	pauseEdit.SetWindowTextA( "0" );
}


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
	outputNiawgWavesToText.rearrange( width, height, fonts );
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
	saveFile << currentOptions.outputNiawgWavesToText << "\n";
	saveFile << "END_DEBUGGING_OPTIONS\n";
}


void DebugOptionsControl::handleOpenConfig(std::ifstream& openFile, int versionMajor, int versionMinor )
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
	if ( (versionMajor == 2 && versionMinor > 8) || versionMajor > 2)
	{
		openFile >> currentOptions.outputNiawgWavesToText;
	}
	ProfileSystem::checkDelimiterLine(openFile, "END_DEBUGGING_OPTIONS");
	setOptions( currentOptions );
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
	currentOptions.outputNiawgWavesToText = outputNiawgWavesToText.GetCheck( );
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
	outputNiawgWavesToText.SetCheck( currentOptions.outputNiawgWavesToText );
}
