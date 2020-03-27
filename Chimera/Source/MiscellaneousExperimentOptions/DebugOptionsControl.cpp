// created by Mark O. Brown
#include "stdafx.h"
#include "DebugOptionsControl.h"
#include "ExperimentThread/Communicator.h"
#include "ConfigurationSystems/ProfileSystem.h"
#include "PrimaryWindows/MainWindow.h"
#include <boost/lexical_cast.hpp>

void DebugOptionsControl::initialize( int& id, POINT& loc, CWnd* parent, cToolTips& tooltips )
{
	// Debugging Options Title
	header.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 25 };
	header.Create( "DEBUGGING OPTIONS", NORM_HEADER_OPTIONS, header.sPos, parent, id++ );
	header.fontType = fontTypes::HeadingFont;
	UINT count = 0;
	niawgMachineScript.sPos = { loc.x, loc.y, loc.x + 240, loc.y += 20 };
	niawgMachineScript.Create( "Show Machine NIAWG Script?", NORM_CHECK_OPTIONS, niawgMachineScript.sPos, parent, 
							   IDC_DEBUG_OPTIONS_RANGE_BEGIN + count++ );
	niawgMachineScript.SetCheck( BST_CHECKED );

	currentOptions.outputNiawgMachineScript = true;
	outputAgilentScript.sPos = { loc.x, loc.y, loc.x + 240, loc.y += 20 };
	outputAgilentScript.Create( "Show Agilent Script?", NORM_CHECK_OPTIONS, outputAgilentScript.sPos, parent, 
								IDC_DEBUG_OPTIONS_RANGE_BEGIN + count++ );
	outputAgilentScript.SetCheck( BST_CHECKED );
	currentOptions.outputAgilentScript = true;
	///
	niawgScript.sPos = { loc.x, loc.y, loc.x + 240, loc.y += 20 };
	niawgScript.Create( "Show Human NIAWG Script?", NORM_CHECK_OPTIONS, niawgScript.sPos, parent, 
						IDC_DEBUG_OPTIONS_RANGE_BEGIN + count++ );
	niawgScript.SetCheck( BST_CHECKED );
	currentOptions.outputNiawgHumanScript = true;

	///
	readProgress.sPos = { loc.x, loc.y, loc.x + 240, loc.y += 20 };
	readProgress.Create( "Show Wvfm Read Progress?", NORM_CHECK_OPTIONS, readProgress.sPos, parent, 
						 IDC_DEBUG_OPTIONS_RANGE_BEGIN + count++ );
	readProgress.SetCheck( BST_CHECKED );
	currentOptions.showReadProgress = true;
	///
	writeProgress.sPos = { loc.x, loc.y, loc.x + 240, loc.y += 20 };
	writeProgress.Create( "Show Wvfm Write Progress?", NORM_CHECK_OPTIONS, writeProgress.sPos, parent, 
						  IDC_DEBUG_OPTIONS_RANGE_BEGIN + count++ );
	writeProgress.SetCheck( BST_CHECKED );
	currentOptions.showWriteProgress = true;
	loc.y += -100;
	///
	correctionTimes.sPos = { loc.x + 240, loc.y, loc.x + 480, loc.y += 20 };
	correctionTimes.Create( "Show Phase Correction Wvfm Times?", NORM_CHECK_OPTIONS, correctionTimes.sPos,
							parent, IDC_DEBUG_OPTIONS_RANGE_BEGIN + count++ );
	correctionTimes.SetCheck( BST_CHECKED );
	correctionTimes.fontType = fontTypes::SmallFont;
	currentOptions.showCorrectionTimes = true;
	///
	excessInfo.sPos = { loc.x + 240, loc.y, loc.x + 480, loc.y += 20 };
	excessInfo.Create( "Show Excess Run Info?", NORM_CHECK_OPTIONS, correctionTimes.sPos, parent, 
					   IDC_DEBUG_OPTIONS_RANGE_BEGIN + count++ );
	excessInfo.SetCheck( BST_CHECKED );
	currentOptions.outputExcessInfo = true;

	outputNiawgWavesToText.sPos = { loc.x + 240, loc.y, loc.x + 480, loc.y += 20 };
	outputNiawgWavesToText.Create( "Output Niawg Wvfms to .txt?", NORM_CHECK_OPTIONS, outputNiawgWavesToText.sPos, 
								   parent, IDC_DEBUG_OPTIONS_RANGE_BEGIN + count++ );

	showTtlsButton.sPos = { loc.x + 240, loc.y, loc.x + 480, loc.y += 20 };
	showTtlsButton.Create( "Show All TTL Events", NORM_CHECK_OPTIONS, showTtlsButton.sPos, parent, IDC_SHOW_TTLS );

	showDacsButton.sPos = { loc.x + 240, loc.y, loc.x + 480, loc.y += 20 };
	showDacsButton.Create( "Show All Dac Events", NORM_CHECK_OPTIONS, showDacsButton.sPos, parent, IDC_SHOW_DACS );

	pauseText.sPos = { loc.x + 200, loc.y, loc.x + 480, loc.y + 20 };
	pauseText.Create( "Pause Btwn Variations (ms)", NORM_STATIC_OPTIONS, pauseText.sPos, parent, id++ );

	pauseEdit.sPos = { loc.x, loc.y, loc.x + 200, loc.y += 20 };
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


void DebugOptionsControl::handleSaveConfig(ConfigStream& saveFile)
{
	saveFile << "DEBUGGING_OPTIONS"
			 << "\n/*Output Agilent Script?*/\t\t\t" << currentOptions.outputAgilentScript
			 << "\n/*Output Excess Info?*/\t\t\t\t" << currentOptions.outputExcessInfo
			 << "\n/*Output Human Niawg Script?*/\t\t" << currentOptions.outputNiawgHumanScript
			 << "\n/*Output Niawg Machine Script?*/\t" << currentOptions.outputNiawgMachineScript
			 << "\n/*Show Correction Times?*/\t\t\t" << currentOptions.showCorrectionTimes
			 << "\n/*Show Dacs?*/\t\t\t\t\t\t" << currentOptions.showDacs
			 << "\n/*Show Ttls?*/\t\t\t\t\t\t" << currentOptions.showTtls
			 << "\n/*Show Read Progress?*/\t\t\t\t" << currentOptions.showReadProgress
			 << "\n/*Show Write Progress?*/\t\t\t" << currentOptions.showWriteProgress
			 << "\n/*Sleep Time:*/\t\t\t\t\t\t" << currentOptions.sleepTime
			 << "\n/*Output Niawg Waveforms to Text?*/\t" << currentOptions.outputNiawgWavesToText
			 << "\nEND_DEBUGGING_OPTIONS\n";
}


void DebugOptionsControl::handleOpenConfig(ConfigStream& openFile, Version ver )
{
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
		currentOptions.sleepTime = boost::lexical_cast<long>(sleep);
	}
	catch ( boost::bad_lexical_cast&)
	{
		currentOptions.sleepTime = 0;
	}
	if (ver > Version("2.8" ) )
	{
		openFile >> currentOptions.outputNiawgWavesToText;
	}
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
	try
	{
		currentOptions.sleepTime = boost::lexical_cast<long>( str ( text ) );
	}
	catch ( boost::bad_lexical_cast& )
	{
		throwNested ( "could not convert sleep time to long!" );
	}
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
