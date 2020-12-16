// created by Mark O. Brown

#include "stdafx.h"
#include "MainOptionsControl.h"
#include "ConfigurationSystems/ConfigSystem.h"
#include "PrimaryWindows/QtMainWindow.h"
#include <boost/lexical_cast.hpp>
#include <qdebug.h>

void MainOptionsControl::initialize( QPoint& loc, IChimeraQtWindow* parent ){
	auto& px = loc.rx (), & py = loc.ry ();
	header = new QLabel ("MAIN OPTIONS", parent);
	header->setGeometry (QRect (px, py, 480, 20));
	py += 20;
	auto configUpdate = [parent]() {parent->configUpdated (); };
	randomizeVariationsButton = new QCheckBox ("Randomize Variations?", parent);
	randomizeVariationsButton->setGeometry (QRect (px, py, 480, 20));
	parent->connect (randomizeVariationsButton, &QCheckBox::stateChanged, configUpdate);
	py += 20;

	delayAutoCal = new QCheckBox ("Delay Auto-Calibration", parent);
	delayAutoCal->setGeometry (px, py, 480, 20);
	delayAutoCal->setChecked (true);
	py += 20;

	atomThresholdForSkipText = new QLabel ("Atom Threshold for Load Skip:", parent);
	atomThresholdForSkipText->setGeometry (px, py, 240, 25);
	atomThresholdForSkipEdit = new QLineEdit ("-1", parent);
	atomThresholdForSkipEdit->setGeometry (px+240, py, 240, 25);
	parent->connect (atomThresholdForSkipEdit, &QLineEdit::textChanged, configUpdate);

	py += 25;
	currentOptions.randomizeVariations = true;
}

void MainOptionsControl::handleSaveConfig(ConfigStream& saveFile){
	saveFile << "MAIN_OPTIONS"
			 << "\n/*Randomize Variations?*/ " << randomizeVariationsButton->isChecked()
			 << "\n/*Atom Threshold for Load Skip*/ " << str(atomThresholdForSkipEdit->text());
	saveFile << "\nEND_MAIN_OPTIONS\n";
}

mainOptions MainOptionsControl::getSettingsFromConfig (ConfigStream& openFile ){
	mainOptions options;
	openFile >> options.randomizeVariations;
	std::string txt;
	openFile >> txt;
	try{
		options.atomSkipThreshold = boost::lexical_cast<unsigned long>( txt );
	}
	catch ( boost::bad_lexical_cast& ){
		errBox ( "atom threshold for load skip failed to convert to an unsigned long! The code will force "
					"the threshold to the maximum threshold." );
		options.atomSkipThreshold = -1;
	}
	return options;
}

void MainOptionsControl::setOptions ( mainOptions opts ){
	currentOptions = opts;
	randomizeVariationsButton->setChecked( currentOptions.randomizeVariations );
	atomThresholdForSkipEdit->setText( cstr ( currentOptions.atomSkipThreshold ) );
}

mainOptions MainOptionsControl::getOptions(){
	currentOptions.randomizeVariations = randomizeVariationsButton->isChecked();
	currentOptions.delayAutoCal = delayAutoCal->isChecked ();
	try{
		currentOptions.atomSkipThreshold = boost::lexical_cast<unsigned long>( str( atomThresholdForSkipEdit->text() ) );
	}
	catch ( boost::bad_lexical_cast& ){
		throwNested ( "failed to convert atom threshold for load-skip to an unsigned long!" );
	}
	return currentOptions;
}

