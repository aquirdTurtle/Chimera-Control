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
			 << "\n/*Sleep Time:*/\t\t\t\t\t\t" << currentOptions.sleepTime
			 << "\nEND_DEBUGGING_OPTIONS\n";
}

void DebugOptionsControl::handleOpenConfig(ConfigStream& openFile ){
	std::string sleep;
	openFile >> sleep;
	try{
		currentOptions.sleepTime = boost::lexical_cast<long>(sleep);
	}
	catch ( boost::bad_lexical_cast&){
		currentOptions.sleepTime = 0;
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
