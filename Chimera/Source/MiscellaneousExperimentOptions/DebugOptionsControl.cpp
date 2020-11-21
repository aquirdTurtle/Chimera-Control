// created by Mark O. Brown
#include "stdafx.h"
#include "DebugOptionsControl.h"
#include "ConfigurationSystems/ConfigSystem.h"
#include <boost/lexical_cast.hpp>

void DebugOptionsControl::initialize( QPoint& loc, IChimeraQtWindow* parent ){
	auto& px = loc.rx (), & py = loc.ry ();
	// Debugging Options Title
	header = new QLabel ("DEBUGGING OPTIONS", parent);
	header->setGeometry (px, py, 480, 25);
	py += 25;
	auto configUpdate = [parent]() {parent->configUpdated (); };
	pauseText = new QLabel ("Pause Btwn Variations (ms):", parent);
	pauseText->setGeometry (px, py, 280, 20);

	pauseEdit = new QLineEdit ("0", parent);
	pauseEdit->setGeometry (px + 280, py, 200, 20);
	parent->connect (pauseEdit, &QLineEdit::textChanged, configUpdate);

	py += 20;
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

void DebugOptionsControl::handleOpenConfig(ConfigStream& openFile ){
	bool trashOptions;
	openFile >> trashOptions >> trashOptions >> trashOptions >> trashOptions >> trashOptions >> trashOptions
		>> trashOptions >> trashOptions >> trashOptions;
	std::string sleep;
	openFile >> sleep;
	try{
		currentOptions.sleepTime = boost::lexical_cast<long>(sleep);
	}
	catch ( boost::bad_lexical_cast&){
		currentOptions.sleepTime = 0;
	}
	if (openFile.ver > Version("2.8" ) ){
		openFile >> trashOptions;
	}
	setOptions( currentOptions );
}

debugInfo DebugOptionsControl::getOptions(){
	try{
		currentOptions.sleepTime = boost::lexical_cast<long>( str ( pauseEdit->text()) );
	}
	catch ( boost::bad_lexical_cast& ){
		throwNested ( "could not convert sleep time to long!" );
	}
	return currentOptions;
}

void DebugOptionsControl::setOptions(debugInfo options){
	currentOptions = options;
	pauseEdit->setText( cstr( currentOptions.sleepTime ) );
}
