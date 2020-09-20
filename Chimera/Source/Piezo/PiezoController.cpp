#include "stdafx.h"
#include "PiezoController.h"
#include <qtimer.h>
#include <boost/lexical_cast.hpp>
#include <PrimaryWindows/QtMainWindow.h>

PiezoController::PiezoController (IChimeraQtWindow* parent, piezoSetupInfo info) : IChimeraSystem(parent), core(info), 
expActive(info.expActive), isReflective(info.editReflective) {}

std::string PiezoController::getConfigDelim ( ){
	return core.configDelim;
}
 
PiezoCore& PiezoController::getCore ( ){
	return core;
}

void PiezoController::handleProgramNowPress ( ){
	double xval, yval, zval;
	try	{
		xval = boost::lexical_cast<double>(str(edits.x->text()));
		yval = boost::lexical_cast<double>(str (edits.y->text ()));
		zval = boost::lexical_cast<double>(str (edits.z->text ()));
	}
	catch( boost::bad_lexical_cast& ){
		thrower ( "ERROR: failed to convert one of the edit texts to a double!" );
	}
	core.programXNow ( xval );
	core.programYNow ( yval );
	core.programZNow ( zval );
	updateCurrentValues ( );
}

void PiezoController::updateCurrentValues ( ){
	currentVals.x->setText ( qstr ( core.getCurrentXVolt ( ), 9, true) );
	currentVals.y->setText ( qstr ( core.getCurrentYVolt ( ), 9, true) );
	currentVals.z->setText ( qstr ( core.getCurrentZVolt ( ), 9, true) );
	if (isReflective) {
		edits.x->setText (qstr (core.getCurrentXVolt (), 9, true));
		edits.y->setText (qstr (core.getCurrentYVolt (), 9, true));
		edits.z->setText (qstr (core.getCurrentZVolt (), 9, true));
	}
}

void PiezoController::handleOpenConfig (ConfigStream& configFile){
	if (configFile.ver > Version ("4.5")) {
		auto configVals = core.getSettingsFromConfig (configFile);
		edits.x->setText (qstr (configVals.pztValues.x.expressionStr, 9, true));
		edits.y->setText (qstr (configVals.pztValues.y.expressionStr, 9, true));
		edits.z->setText (qstr (configVals.pztValues.z.expressionStr, 9, true));
		ctrlButton->setChecked (configVals.ctrlPzt);
		updateCtrl ();
	}
}

void PiezoController::handleSaveConfig (ConfigStream& configFile ){
	configFile << core.configDelim;
	configFile << "\n/*X-Value:*/ " << Expression(str(edits.x->text()));
	configFile << "\n/*Y-Value:*/ " << Expression(str(edits.y->text()));
	configFile << "\n/*Z-Value:*/ " << Expression (str (edits.z->text ()))
			   << "\n/*Control?*/ " << ctrlButton->isChecked()
			   << "\nEND_" + core.configDelim << "\n";
}

std::string PiezoController::getDeviceInfo ( ){
	return core.getDeviceInfo( );
}

std::string PiezoController::getPiezoDeviceList ( ){
	return core.getDeviceList ( );
}

void PiezoController::updateCtrl ( ){
	auto ctrl = ctrlButton->isChecked ( );
	core.experimentActive = ctrl;
	edits.x->setEnabled( ctrl );
	edits.y->setEnabled ( ctrl );
	edits.z->setEnabled ( ctrl );
}

void PiezoController::initialize ( POINT& pos, IChimeraQtWindow* parent, LONG width, piezoChan<std::string> names ){
	core.initialize ( );

	programNowButton = new QPushButton ("Program Pzt Now", parent);
	programNowButton->setGeometry (pos.x, pos.y, 6 * width / 8, 25);
	parent->connect (programNowButton, &QPushButton::released, [this, parent]() {
		try	{
			handleProgramNowPress ();
			updateCurrentValues ();
		}
		catch (ChimeraError& err) {
			parent->reportErr (err.qtrace ());
		}
	});
	ctrlButton = new QCheckBox ("Ctrl?", parent);
	ctrlButton->setGeometry (pos.x + 3*width/4, pos.y, width/4, 25);
	ctrlButton->setChecked (true);
	if (!expActive) {
		// never enabled. You should be able to always modify the piezo values in the middle of the experiment safely. 
		ctrlButton->setEnabled (false);
		ctrlButton->setText ("(Not Used in Exp)");
	}
	parent->connect (ctrlButton, &QCheckBox::stateChanged, [this, parent]() {
		try	{
			updateCtrl ();
			parent->configUpdated ();
		}
		catch (ChimeraError& err) {
			parent->reportErr (err.qtrace ());
		}
	});

	labels.x = new QLabel (names.x.c_str (), parent);
	labels.x->setGeometry (pos.x, pos.y += 25, width / 3, 20);
	labels.y = new QLabel (names.y.c_str (), parent);
	labels.y->setGeometry (pos.x + width/3, pos.y, width / 3, 20);
	labels.z = new QLabel (names.z.c_str (), parent);
	labels.z->setGeometry (pos.x+ 2*width / 3, pos.y, width / 3, 20);

	edits.x = new QLineEdit (parent);
	edits.x->setGeometry (pos.x, pos.y += 20, width / 3, 20);
	parent->connect (edits.x, &QLineEdit::textChanged, [parent]() { parent->configUpdated (); });
	edits.y = new QLineEdit (parent);
	edits.y->setGeometry (pos.x + width / 3, pos.y, width / 3, 20);
	parent->connect (edits.y, &QLineEdit::textChanged, [parent]() { parent->configUpdated (); });
	edits.z = new QLineEdit (parent);
	edits.z->setGeometry (pos.x + 2 * width / 3, pos.y, width / 3, 20);
	parent->connect (edits.z, &QLineEdit::textChanged, [parent]() { parent->configUpdated (); });

	currentVals.x = new QLabel (parent);
	currentVals.x->setGeometry (pos.x, pos.y += 20, width / 3, 20);
	currentVals.y = new QLabel (parent);
	currentVals.y->setGeometry (pos.x + width / 3, pos.y, width / 3, 20);
	currentVals.z = new QLabel (parent);
	currentVals.z->setGeometry (pos.x + 2 * width / 3, pos.y, width / 3, 20);
	pos.y += 20;
	updateCurrentValues ( );
	QTimer* timer = new QTimer (this);
	connect (timer, &QTimer::timeout, [this, parent]() {
		try {
			if (!parent->mainWin->expIsRunning()){
				updateCurrentValues ();
			}
		}
		catch(ChimeraError&){}
		});
	// could probably make this time a front panel option.
	timer->start (10000);
}
