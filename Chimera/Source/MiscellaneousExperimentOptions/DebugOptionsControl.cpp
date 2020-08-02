// created by Mark O. Brown
#include "stdafx.h"
#include "DebugOptionsControl.h"
#include "ConfigurationSystems/ProfileSystem.h"
#include <boost/lexical_cast.hpp>

void DebugOptionsControl::initialize( POINT& loc, IChimeraQtWindow* parent ){
	// Debugging Options Title
	header = new QLabel ("DEBUGGING OPTIONS", parent);
	header->setGeometry (loc.x, loc.y, 480, 25);
	loc.y += 25;
	unsigned count = 0;
	niawgMachineScript = new QCheckBox ("Show Machine NIAWG Script?", parent);
	niawgMachineScript->setGeometry (loc.x, loc.y, 240, 20);
	niawgMachineScript->setChecked( true );
	auto configUpdate = [parent]() {parent->configUpdated (); };
	parent->connect (niawgMachineScript, &QCheckBox::stateChanged, configUpdate);
	currentOptions.outputNiawgMachineScript = true;
	loc.y += 20;
	outputAgilentScript = new QCheckBox ("Show Agilent Script?", parent);
	outputAgilentScript->setGeometry (loc.x, loc.y, 240, 20);
	outputAgilentScript->setChecked( true );
	parent->connect (outputAgilentScript, &QCheckBox::stateChanged, configUpdate);
	currentOptions.outputAgilentScript = true;
	loc.y += 20;
	///
	niawgScript = new QCheckBox ("Show Human NIAWG Script", parent);
	niawgScript->setChecked( true );
	niawgScript->setGeometry (loc.x, loc.y, 240, 20);
	parent->connect (niawgScript, &QCheckBox::stateChanged, configUpdate);
	currentOptions.outputNiawgHumanScript = true;
	loc.y += 20;
	///
	readProgress = new QCheckBox ("Show Wvfm Read Progress?", parent);
	readProgress->setGeometry (loc.x, loc.y, 240, 20);
	readProgress->setChecked( true );
	parent->connect (readProgress, &QCheckBox::stateChanged, configUpdate);
	currentOptions.showReadProgress = true;
	loc.y += 20;
	///
	writeProgress = new QCheckBox ("Show Wvfm Write Progress", parent);
	writeProgress->setGeometry (loc.x, loc.y, 240, 20);
	writeProgress->setChecked( true );
	parent->connect (writeProgress, &QCheckBox::stateChanged, configUpdate);
	currentOptions.showWriteProgress = true;
	loc.y += -80;
	///
	correctionTimes = new QCheckBox ("Show Phase Correction Wvfm Times?", parent);
	correctionTimes->setGeometry (loc.x+240, loc.y, 240, 20);
	correctionTimes->setChecked( true );
	parent->connect (correctionTimes, &QCheckBox::stateChanged, configUpdate);
	currentOptions.showCorrectionTimes = true;
	loc.y += 20;
	///
	excessInfo = new QCheckBox ("Show Excess Run Info?", parent);
	excessInfo->setGeometry (loc.x + 240, loc.y, 240, 20);
	excessInfo->setChecked( true );
	parent->connect (excessInfo, &QCheckBox::stateChanged, configUpdate);
	currentOptions.outputExcessInfo = true;
	loc.y += 20;

	outputNiawgWavesToText = new QCheckBox ("Output Niawg Wvfms to .txt?", parent);
	outputNiawgWavesToText->setGeometry (loc.x + 240, loc.y, 240, 20);
	parent->connect (outputNiawgWavesToText, &QCheckBox::stateChanged, configUpdate);

	loc.y += 20;
	showTtlsButton = new QCheckBox ("Show All TTL Events?", parent);
	showTtlsButton->setGeometry (loc.x + 240, loc.y, 240, 20);
	parent->connect (showTtlsButton, &QCheckBox::stateChanged, configUpdate);

	loc.y += 20;
	showDacsButton = new QCheckBox ("Show all DAC Events?", parent);
	showDacsButton->setGeometry (loc.x + 240, loc.y, 240, 20);
	parent->connect (showDacsButton, &QCheckBox::stateChanged, configUpdate);

	loc.y += 20;
	pauseText = new QLabel ("Pause Btwn Variations (ms):", parent);
	pauseText->setGeometry (loc.x, loc.y, 280, 20);

	pauseEdit = new QLineEdit ("0", parent);
	pauseEdit->setGeometry (loc.x + 280, loc.y, 200, 20);
	parent->connect (pauseEdit, &QLineEdit::textChanged, configUpdate);

	loc.y += 20;
}


void DebugOptionsControl::handleSaveConfig(ConfigStream& saveFile){
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


void DebugOptionsControl::handleOpenConfig(ConfigStream& openFile )
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
	if (openFile.ver > Version("2.8" ) )
	{
		openFile >> currentOptions.outputNiawgWavesToText;
	}
	setOptions( currentOptions );
}


void DebugOptionsControl::handleEvent(unsigned id, MainWindow* comm)
{
	/*
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
	*/
}


debugInfo DebugOptionsControl::getOptions()
{
	currentOptions.outputNiawgMachineScript = niawgMachineScript->isChecked();
	currentOptions.outputNiawgHumanScript = niawgScript->isChecked ();
	currentOptions.outputAgilentScript = outputAgilentScript->isChecked ();
	currentOptions.showReadProgress = readProgress->isChecked ();
	currentOptions.showWriteProgress = writeProgress->isChecked ();
	currentOptions.showCorrectionTimes = correctionTimes->isChecked ();
	currentOptions.showTtls = showTtlsButton->isChecked ();
	currentOptions.showDacs = showDacsButton->isChecked ();
	currentOptions.outputNiawgWavesToText = outputNiawgWavesToText->isChecked ();
	try
	{
		currentOptions.sleepTime = boost::lexical_cast<long>( str ( pauseEdit->text()) );
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
	readProgress->setChecked( currentOptions.showReadProgress );
	writeProgress->setChecked ( currentOptions.showWriteProgress );
	correctionTimes->setChecked (currentOptions.showCorrectionTimes);
	niawgScript->setChecked (currentOptions.outputNiawgHumanScript);
	outputAgilentScript->setChecked ( currentOptions.outputAgilentScript );
	niawgMachineScript->setChecked ( currentOptions.outputNiawgMachineScript);
	excessInfo->setChecked ( currentOptions.outputExcessInfo);
	showTtlsButton->setChecked ( currentOptions.showTtls);
	showDacsButton->setChecked ( currentOptions.showDacs);
	pauseEdit->setText( cstr( currentOptions.sleepTime ) );
	outputNiawgWavesToText->setChecked ( currentOptions.outputNiawgWavesToText );
}
