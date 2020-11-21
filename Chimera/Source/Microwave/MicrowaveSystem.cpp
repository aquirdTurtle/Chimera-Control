// created by Mark O. Brown
#include "stdafx.h"
#include "Microwave/MicrowaveSystem.h"
#include "GeneralFlumes/GpibFlume.h"
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

/*
 * The controls in this class only display information about what get's programmed to the RSG. They do not
 * (by design) provide an interface for which the user to change the programming of the RSG directly. The
 * user is to do this by using the "rsg:" command in a script.
 */
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
		[this](const QPoint& pos) {this->handleContextMenu (pos); });
	uwListListview->setGeometry (px, py, 480, 120);
	uwListListview->setColumnWidth (0, 40);
	uwListListview->setColumnWidth (1, 240);
	uwListListview->setColumnWidth (2, 140);
	

	uwListListview->setShowGrid (true);
	uwListListview->connect (
		uwListListview, &QTableWidget::cellChanged, [this](int row, int col) {
			if (currentList.size () <= row) {
				return;
			}
			auto qtxt = uwListListview->item (row, col)->text ();
			switch (col) {
			case 1:
				currentList[row].frequency = str (qtxt);
				break;
			case 2:
				currentList[row].power = str (qtxt);
				break;
			}
		});
	refreshListview ();
	py += 100;
}

void MicrowaveSystem::handleReadPress (){
	auto res = core.uwFlume.read ();
	readTxt->setText (cstr (res));
	errBox (res);
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

