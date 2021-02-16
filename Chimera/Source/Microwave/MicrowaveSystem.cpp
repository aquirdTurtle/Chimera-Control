// created by Mark O. Brown
#include "stdafx.h"
#include "Microwave/MicrowaveSystem.h"
#include "LowLevel/constants.h"
#include "PrimaryWindows/QtAuxiliaryWindow.h"
#include <qheaderview.h>
#include <qmenu.h>
#include "PrimaryWindows/QtMainWindow.h"
#include <QTableWidget.h>

MicrowaveSystem::MicrowaveSystem(IChimeraQtWindow* parent) : IChimeraSystem(parent) {}

std::string MicrowaveSystem::getIdentity(){ 
	return core.queryIdentity();
}

void MicrowaveSystem::handleContextMenu (const QPoint& pos){
	QTableWidgetItem* item = uwListListview->itemAt (pos);
	QMenu menu;
	menu.setStyleSheet (chimeraStyleSheets::stdStyleSheet ());
	auto* deleteAction = new QAction ("Delete This Item", uwListListview);
	uwListListview->connect (deleteAction, &QAction::triggered, [this, item]() {uwListListview->removeRow (item->row ()); });
	auto* newPerson = new QAction ("New Item", uwListListview);
	uwListListview->connect (newPerson, &QAction::triggered,
		[this]() {currentList.push_back (microwaveListEntry ()); refreshListview (); });
	if (item) { menu.addAction (deleteAction); }
	menu.addAction (newPerson);
	menu.exec (uwListListview->mapToGlobal (pos));
}

void MicrowaveSystem::initialize( QPoint& pos, IChimeraQtWindow* parent ){
	auto& px = pos.rx (), & py = pos.ry ();
	header = new QLabel ("MICROWAVE SYSTEM", parent); 
	header->setGeometry (px, py, 240, 25);
	
	controlOptionCheck = new QCheckBox ("Control?", parent);
	controlOptionCheck->setGeometry (px+240, py, 120, 20);

	programNowPush = new QPushButton ("Program Now", parent);
	programNowPush->setGeometry (px + 360, py, 120, 20);

	parent->connect (programNowPush, &QPushButton::released, [this, parent]() {
		try	{
			programNow (parent->auxWin->getUsableConstants ());
		}
		catch (ChimeraError& err) {
			parent->reportErr ("Failed to program microwave system! " + err.qtrace ());
		}
	});

	writeNow = new QPushButton ("Write Now", parent);
	writeNow->setGeometry (px, py += 20, 80, 20);
	writeNow->connect (writeNow, &QPushButton::pressed, this, &MicrowaveSystem::handleWritePress);

	writeTxt = new QLineEdit (parent);
	writeTxt->setGeometry (px+80, py, 160, 20);

	readNow = new QPushButton("Read Now", parent);
	readNow->setGeometry (px+240, py, 80, 20);
	readNow->connect (writeNow, &QPushButton::pressed, this, &MicrowaveSystem::handleReadPress);

	readTxt = new QLabel("", parent);
	readTxt->setGeometry (px+320, py, 160, 20);

	triggerStepTimeLabel = new QLabel ("Trig. Time (ms)", parent);
	triggerStepTimeLabel->setGeometry (px, py += 20, 240,20);
	triggerStepTimeEdit = new QLineEdit (parent);
	triggerStepTimeEdit->setGeometry (px+240, py, 240, 20);
	triggerStepTimeEdit->connect (triggerStepTimeEdit, &QLineEdit::textChanged, [this, parent]() {
		try {
			auto time = boost::lexical_cast<double>(str(triggerStepTimeEdit->text ()));
			core.setTrigTime (time);
		}
		catch (boost::bad_lexical_cast &) {} // probably just happens while user is trying to type
		});
	
	uwListListview = new QTableWidget (parent);
	uwListListview->setGeometry (px, py += 20, 480, 100);
	uwListListview->setColumnCount (3);
	QStringList labels;
	labels << "#" << "Frequency (GHz)" << "Power (dBm)";
	uwListListview->setHorizontalHeaderLabels (labels);
	uwListListview->horizontalHeader ()->setFixedHeight (20);
	uwListListview->verticalHeader ()->setFixedWidth (25);
	uwListListview->verticalHeader ()->setDefaultSectionSize (20);
	uwListListview->setContextMenuPolicy (Qt::CustomContextMenu);
	parent->connect (uwListListview, &QTableWidget::customContextMenuRequested,
		[this](const QPoint& pos) {handleContextMenu (pos); });
	uwListListview->setGeometry (px, py, 480, 120);
	uwListListview->setColumnWidth (0, 40);
	uwListListview->setColumnWidth (1, 240);
	uwListListview->setColumnWidth (2, 140);

	uwListListview->setShowGrid (true);
	refreshListview ();
	py += 100;
}

void MicrowaveSystem::refreshCurrentUwList () {
	currentList.resize (uwListListview->rowCount ());
	for (auto rowI : range (uwListListview->rowCount ())) {
		try {
			currentList[rowI].frequency = str (uwListListview->item (rowI, 1)->text ());
			currentList[rowI].power = str (uwListListview->item (rowI, 2)->text ());
		}
		catch (ChimeraError&) {
			throwNested ("Failed to convert microwave table data to uw list structure!");
		}
	}
}

void MicrowaveSystem::handleReadPress (){
	auto res = core.uwFlume.read ();
	readTxt->setText (qstr (res));
	readTxt->setToolTip (qstr (res));
	//errBox (res);
}

void MicrowaveSystem::handleWritePress (){
	auto txt = writeTxt->text ();
	core.uwFlume.write(str(txt));
}


void MicrowaveSystem::programNow(std::vector<parameterType> constants){
	// ignore the check if the user literally presses program now.
	core.experimentSettings.control = true;
	core.experimentSettings.list = currentList;
	core.experimentActive = true;
	std::string warnings;

	core.calculateVariations (constants, nullptr);
	core.programVariation (0, constants, nullptr);
	emit notification ("Finished programming microwave system!\n");
}


void MicrowaveSystem::handleSaveConfig (ConfigStream& saveFile){
	refreshCurrentUwList ();
	saveFile << core.configDelim
		<< "\n/*Control?*/ " << controlOptionCheck->isChecked ()
		<< "\n/*List Size:*/ " << currentList.size ();
	for (auto listElem : currentList){
		saveFile << "\n/*Freq:*/ " << listElem.frequency 
				 << "\n/*Power:*/ " << listElem.power;
	}
	saveFile << "\nEND_" << core.configDelim << "\n";
}

void MicrowaveSystem::setMicrowaveSettings (microwaveSettings settings){
	controlOptionCheck->setChecked (settings.control);
	currentList = settings.list;
	refreshListview ();
}


void MicrowaveSystem::refreshListview (){
	unsigned count = 0;
	uwListListview->setRowCount (0);
	for (auto listElem : currentList){
		auto ind = uwListListview->rowCount ();
		uwListListview->insertRow (ind);
		uwListListview->setItem (ind, 0, new QTableWidgetItem (cstr(count)));
		uwListListview->item (ind, 0)->setFlags (uwListListview->item (ind, 0)->flags () ^ Qt::ItemIsEnabled);
		uwListListview->setItem (ind, 1, new QTableWidgetItem (cstr (listElem.frequency.expressionStr)));
		uwListListview->setItem (ind, 2, new QTableWidgetItem (cstr (listElem.power.expressionStr)));
		count++;
	}
}

MicrowaveCore& MicrowaveSystem::getCore (){
	return core;
}

